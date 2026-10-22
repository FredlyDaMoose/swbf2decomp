#include "Battlefront2/Source/LensFlare.h"
#include <math.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexBuffer.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcFlareShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcSampleShader.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

// GLOBAL: SWBF2 0x01f9a924
static RedTexture* s_sampleTex;
// GLOBAL: SWBF2 0x01f9ae40
static RedTexture* s_intensityTex;
// GLOBAL: SWBF2 0x01f9ae48
static pcRedShader* s_pClearShader;
// GLOBAL: SWBF2 0x01f9ae3c
static pcSampleShader* s_pSampleShader;
// GLOBAL: SWBF2 0x01f9ae50
static pcRedPrimitive* s_samplePoints;

// GLOBAL: SWBF2 0x01f9ae44
static pcFlareShader* s_pFlareShader;
// GLOBAL: SWBF2 0x01f9ae4c
static pcFlareShader* s_pFillShader;

struct CompressedPosXY
{
    short x;
    short y;
    static PblVector3 s_vMin;
    static PblVector3 s_vScale;
    static PblVector3 s_vScaleInv;
};
// GLOBAL: SWBF2 0x007f587c
PblVector3 CompressedPosXY::s_vMin;
// GLOBAL: SWBF2 0x007de288
PblVector3 CompressedPosXY::s_vScale = { 1.0f / 65535.0f, 1.0f / 65535.0f, 1.0f / 65535.0f };
// GLOBAL: SWBF2 0x007de294
PblVector3 CompressedPosXY::s_vScaleInv = { 65535.0f, 65535.0f, 65535.0f };

// GLOBAL: SWBF2 0x007eb728
static PblBox s_boundingBox(-2200.0f, -2200.0f, -2200.0f, 2200.0f, 2200.0f, 2200.0f);

static const float kfFadeDrawDistance = 0.9f;
static const float kfFadeDrawDistanceSquared = kfFadeDrawDistance * kfFadeDrawDistance;
static const float kfFadeDrawDistanceFactor = 1.0f / (1.0f - kfFadeDrawDistanceSquared);

struct CompressedPos
{
    short x;
    short y;
    short z;
    short w;

    static PblVector3 s_vMin;
    static PblVector3 s_vScale;
    static PblVector3 s_vScaleInv;
};
struct CompressedTexCoord
{
    short u;
    short v;
};
struct FlareCompressedVertex
{
    CompressedPos pos;
    RedColor color;
    CompressedTexCoord uv;
};
 

struct PosXYZ
{
    float x;
    float y;
    float z;
};

struct TexCoord
{
    float u;
    float v;
};

struct FlareVertex
{
    PosXYZ pos;
    RedColor color;
    TexCoord uv;
};
 

// FUNCTION: SWBF2 0x0041f1d0
template <>
void LensFlare::EmitRing<FlareVertex>(FlareVertex*& pVertices, const PblVector3& pos, float angle,
                float innerRadius, float middleRadius, float outerRadius,
                const RedColor& innerColor, const RedColor& middleColor,
                const RedColor& outerColor, int, float fU, float fV)
{
    FlareVertex* const first = pVertices;
    float s, c;
    PblMath::SinCos(&s, &c, angle);
    first[0].pos.x = pos.x + c * innerRadius;
    first[0].pos.y = pos.y + s * innerRadius;
    first[0].pos.z = pos.z;

    for (int i = 0; i <= 12; ++i)
    {
        PblMath::SinCos(&s, &c, ((float)i * 6.2831855f) / 12.0f + angle);
        FlareVertex* p = first + 1 + 2 * i;
        p[0].pos.x = pos.x + c * innerRadius;
        p[0].pos.y = pos.y + s * innerRadius;
        p[0].pos.z = pos.z;
        p[0].color = innerColor;
        p[0].uv.u = fU;
        p[0].uv.v = fV;

        p[1].pos.x = pos.x + c * middleRadius;
        p[1].pos.y = pos.y + s * middleRadius;
        p[1].pos.z = pos.z;
        p[1].color = middleColor;
        p[1].uv.u = fU;
        p[1].uv.v = fV;
        if (i == 0)
        {
            p[26] = p[1];
        }
        p[27] = p[1];

        p[28].pos.x = pos.x + c * outerRadius;
        p[28].pos.y = pos.y + s * outerRadius;
        p[28].pos.z = pos.z;
        p[28].color = outerColor;
        p[28].uv.u = fU;
        p[28].uv.v = fV;
    }
    PblMath::SinCos(&s, &c, angle);
    first[54].pos.x = pos.x + c * outerRadius;
    first[54].pos.y = pos.y + s * outerRadius;
    first[54].pos.z = pos.z;
    pVertices = first + 55;
}

static void CompressedPos_SetBoundingBox(const PblBox& box)
{
    PblVector3 vMin = { box._vMin.x, box._vMin.y, box._vMin.z };
    PblVector3 vExtent = { box._vMax.x - box._vMin.x, box._vMax.y - box._vMin.y,
                                box._vMax.z - box._vMin.z };
    CompressedPos::s_vMin = vMin;
    CompressedPos::s_vScale = vExtent * (1.0f / 65535.0f);
    CompressedPos::s_vScaleInv.x = 1.0f / CompressedPos::s_vScale.x;
    CompressedPos::s_vScaleInv.y = 1.0f / CompressedPos::s_vScale.y;
    CompressedPos::s_vScaleInv.z = 1.0f / CompressedPos::s_vScale.z;
}

static void CompressedPosXY_SetBoundingBox(const PblBox& box)
{
    PblVector3 vMin = { box._vMin.x, box._vMin.y, box._vMin.z };
    PblVector3 vExtent = { box._vMax.x - box._vMin.x, box._vMax.y - box._vMin.y,
                                box._vMax.z - box._vMin.z };
    CompressedPosXY::s_vMin = vMin;
    CompressedPosXY::s_vScale = vExtent * (1.0f / 65535.0f);
    CompressedPosXY::s_vScaleInv.x = 1.0f / CompressedPosXY::s_vScale.x;
    CompressedPosXY::s_vScaleInv.y = 1.0f / CompressedPosXY::s_vScale.y;
    CompressedPosXY::s_vScaleInv.z = 1.0f / CompressedPosXY::s_vScale.z;
}

static short CompressedPos_Encode(float f, float fMin, float fScaleInv)
{
    return (short)((f - fMin) * fScaleInv - 32767.5f);
}

static void CompressedPos_SetPos(CompressedPos& pos, float fX, float fY, float fZ)
{
    pos.x = CompressedPos_Encode(fX, CompressedPos::s_vMin.x, CompressedPos::s_vScaleInv.x);
    pos.y = CompressedPos_Encode(fY, CompressedPos::s_vMin.y, CompressedPos::s_vScaleInv.y);
    pos.z = CompressedPos_Encode(fZ, CompressedPos::s_vMin.z, CompressedPos::s_vScaleInv.z);
}

template <>
void LensFlareEffect::LensFlareList::Render<FlareCompressedVertex, 37506>() const;
template <>
void LensFlareEffect::LensFlareList::Render<FlareVertex, 642>() const;

// FUNCTION: SWBF2 0x0041bc00
void LensFlareEffect::PlatformInit()
{
    s_sampleTex = new RedTexture(64, 64, D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                                 D3DPOOL_DEFAULT, 0);
    s_intensityTex = new RedTexture(16, 16, D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                                    D3DPOOL_DEFAULT, 0);
    s_pClearShader = pcRedShader::Create(PblHash("Clear"), 3, 0, 0, 0x80000,
                                         RedColor::WHITE, RedColor::BLACK, 32.0f);

    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_UNCOMPRESSED)
    {
        s_samplePoints = pcRedPrimitive::Create(D3DPT_POINTLIST, 2, 12, 256, 0, true);
        float* v = (float*)s_samplePoints->BeginVertexData();
        if (!v)
        {
            return;
        }
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                v[0] = (float)i;
                v[1] = (float)j;
                v[2] = 0.0f;
                v += 3;
            }
        }
        s_samplePoints->EndVertexData();

        s_pFlareShader = (pcFlareShader*)pcRedShader::Create(
            PblHash("flare"), 0x282, 0, 0, 0x130040, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pFlareShader->SetTexture(s_intensityTex);
        s_pFillShader = (pcFlareShader*)pcRedShader::Create(
            PblHash("flare"), 0x82, 0, 0, 0x10040, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pSampleShader = (pcSampleShader*)pcRedShader::Create(
            PblHash("sample"), 2, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
    else
    {
        s_samplePoints = pcRedPrimitive::Create(D3DPT_POINTLIST, 0x1002, 8, 256, 0, true);
        const PblBox box(0.0f, 0.0f, 0.0f, 17.0f, 17.0f, 0.0f);
        s_samplePoints->m_bbox = box;
        CompressedPos_SetBoundingBox(box);
        short* v = (short*)s_samplePoints->BeginVertexData();
        if (!v)
        {
            return;
        }
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                v[0] = CompressedPos_Encode((float)i, CompressedPos::s_vMin.x,
                                            CompressedPos::s_vScaleInv.x);
                v[1] = CompressedPos_Encode((float)j, CompressedPos::s_vMin.y,
                                            CompressedPos::s_vScaleInv.y);
                v[2] = CompressedPos_Encode(0.0f, CompressedPos::s_vMin.z,
                                            CompressedPos::s_vScaleInv.z);
                v += 4;
            }
        }
        s_samplePoints->EndVertexData();

        s_pFlareShader = (pcFlareShader*)pcRedShader::Create(
            PblHash("flare"), 0x9282, 0, 0, 0x130040, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pFlareShader->SetTexture(s_intensityTex);
        s_pFillShader = (pcFlareShader*)pcRedShader::Create(
            PblHash("flare"), 0x1082, 0, 0, 0x10040, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pSampleShader = (pcSampleShader*)pcRedShader::Create(
            PblHash("sample"), 0x1002, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }

    s_pSampleShader->SetOutputPosition(0.0f, 0.0f);
    s_pSampleShader->SetScale(1.0f / 255.0f);
    s_pSampleShader->SetInputTexture(s_sampleTex);
}

// FUNCTION: SWBF2 0x0041c000
void LensFlareEffect::PlatformCleanup()
{
    if (s_samplePoints)
    {
        s_samplePoints->Release();
        s_samplePoints = 0;
    }
    delete s_sampleTex;
    delete s_intensityTex;
    if (s_pClearShader)
    {
        s_pClearShader->Release();
        s_pClearShader = 0;
    }
    if (s_pFlareShader)
    {
        s_pFlareShader->Release();
        s_pFlareShader = 0;
    }
    if (s_pSampleShader)
    {
        s_pSampleShader->Release();
        s_pSampleShader = 0;
    }
    if (s_pFillShader)
    {
        s_pFillShader->Release();
        s_pFillShader = 0;
    }
    _List.Cleanup();
}

// FUNCTION: SWBF2 0x0041c170
void LensFlareEffect::Render(int value)
{
    if (value & 0x200000)
    {
        return;
    }
    if (RedRenderer::pcGetGeometryMode() != GEOMETRYMODE_UNCOMPRESSED)
    {
        _List.Render<FlareCompressedVertex, 37506>();
    }
    else
    {
        _List.Render<FlareVertex, 642>();
    }
}

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

inline const PblVector3 RedRenderer::TransformCameraPointToScreenSpace(const PblVector3& vPoint)
{
    return TransformProjectionPointToScreenSpace(TransformCameraPointToProjectionSpace(vPoint));
}

static bool IsFlareVisible(const LensFlare* pFlare, unsigned int uiView, const PblMatrix& mView,
                           float fNear, float fFar, float fWidth, float fHeight,
                           PblVector3& vPosition, float& fSize, float& fSquaredDistance)
{
    if (pFlare->GetColor().a == 0)
    {
        return false;
    }
    if (pFlare->IsHiddenInView(uiView))
    {
        return false;
    }

    D3DXVec3TransformCoord(&vPosition, &pFlare->m_mWorldMatrix.GetTranslation(), &mView);
    fSize = pFlare->GetSize();

    if (fNear > -vPosition.z || -vPosition.z > fFar)
    {
        return false;
    }

    if (pFlare->GetSquaredDrawDistance() > 0.0f)
    {
        fSquaredDistance = vPosition.x * vPosition.x + vPosition.y * vPosition.y +
                           vPosition.z * vPosition.z;
        if (!(fSquaredDistance < pFlare->GetSquaredDrawDistance()))
        {
            return false;
        }
    }
    else
    {
        fSquaredDistance = 0.0f;
    }

    if (((float)fabs(vPosition.x) - fSize) * fNear * 2.0f > -(vPosition.z * fWidth))
    {
        return false;
    }
    if (((float)fabs(vPosition.y) - fSize) * fNear * 2.0f > -(vPosition.z * fHeight))
    {
        return false;
    }
    return true;
}

static void SetClearVertex(float* pVertex, float fX, float fY)
{
    pVertex[0] = fX;
    pVertex[1] = fY;
    pVertex[2] = 0.0f;
    pVertex[3] = 1.0f;
}

static void SetFillVertex(FlareCompressedVertex& vertex, float fX, float fY, float fZ)
{
    CompressedPos_SetPos(vertex.pos, fX, fY, fZ);
    vertex.color = RedColor::WHITE;
}

static void SetFlareVertex(FlareCompressedVertex& vertex, float x, float y, float z,
                           const RedColor& color, short u, short v)
{
    CompressedPos_SetPos(vertex.pos, x, y, z);
    vertex.color = color;
    vertex.uv.u = u;
    vertex.uv.v = v;
}

// FUNCTION: SWBF2 0x0041f480
template <>
void LensFlare::EmitRing<FlareCompressedVertex>(
    FlareCompressedVertex*& pVertices, const PblVector3& pos, float angle,
    float innerRadius, float middleRadius, float outerRadius,
    const RedColor& innerColor, const RedColor& middleColor,
    const RedColor& outerColor, int, float fU, float fV)
{
    const short u = (short)(int)(fU * 2048.0f);
    const short v = (short)(int)(fV * 2048.0f);
    FlareCompressedVertex* const first = pVertices;
    float s, c;
    PblMath::SinCos(&s, &c, angle);
    CompressedPos_SetPos(first[0].pos, pos.x + c * innerRadius,
                         pos.y + s * innerRadius, pos.z);

    for (int i = 0; i <= 12; ++i)
    {
        PblMath::SinCos(&s, &c, ((float)i * 6.2831855f) / 12.0f + angle);
        FlareCompressedVertex* p = first + 1 + 2 * i;
        SetFlareVertex(p[0], pos.x + c * innerRadius, pos.y + s * innerRadius,
                       pos.z, innerColor, u, v);
        SetFlareVertex(p[1], pos.x + c * middleRadius, pos.y + s * middleRadius,
                       pos.z, middleColor, u, v);
        if (i == 0)
        {
            SetFlareVertex(p[26], pos.x + c * middleRadius, pos.y + s * middleRadius,
                           pos.z, middleColor, u, v);
        }
        SetFlareVertex(p[27], pos.x + c * middleRadius, pos.y + s * middleRadius,
                       pos.z, middleColor, u, v);
        SetFlareVertex(p[28], pos.x + c * outerRadius, pos.y + s * outerRadius,
                       pos.z, outerColor, u, v);
    }
    PblMath::SinCos(&s, &c, angle);
    CompressedPos_SetPos(first[54].pos, pos.x + c * outerRadius,
                         pos.y + s * outerRadius, pos.z);
    pVertices = first + 55;
}

// FUNCTION: SWBF2 0x0041e980
template <>
void LensFlare::Emit<FlareCompressedVertex>(FlareCompressedVertex*& pVertices,
                                           const PblVector3& pos, float fU,
                                           float fV, float fFade)
{
    const float size = _fSize * 2.0f;
    const float angle = ((pos.y * 2.0f * 3.1415901f + pos.x * 3.1415901f) / pos.z)
                      + _fSize * 1.0472f + _fRandomRotOffset;
    float s1, c1, s2, c2;
    PblMath::SinCos(&s1, &c1, angle);
    PblMath::SinCos(&s2, &c2, 1.0472f - angle);

    const float x0 = pos.x + c1 * size;
    const float y0 = pos.y + s1 * size * 2.0f;
    const float x1 = pos.x + (c1 * -0.5f - s1 * 0.866f) * size;
    const float y1 = pos.y + (c1 * 0.866f - s1 * 0.5f) * size * 2.0f;
    const float x2 = pos.x + (c1 * -0.5f + s1 * 0.866f) * size;
    const float y2 = pos.y + (s1 * -0.5f - c1 * 0.866f) * size * 2.0f;
    const float x3 = pos.x + c2 * size * 3.5f;
    const float y3 = pos.y + s2 * size;
    const float x4 = pos.x + (c2 * -0.5f - s2 * 0.866f) * size * 3.5f;
    const float y4 = pos.y + (c2 * 0.866f - s2 * 0.5f) * size;
    const float x5 = pos.x + (c2 * -0.5f + s2 * 0.866f) * size * 3.5f;
    const float y5 = pos.y + (s2 * -0.5f - c2 * 0.866f) * size;

    RedColor faded = _color;
    faded.a = (unsigned char)(int)((float)_color.a * fFade);
    const RedColor black = { 0, 0, 0, 0 };
    const short u = (short)(int)(fU * 2048.0f);
    const short v = (short)(int)(fV * 2048.0f);
    SetFlareVertex(pVertices[0], x0, y0, pos.z, black, u, v);
    pVertices[1] = pVertices[0];
    SetFlareVertex(pVertices[2], x1, y1, pos.z, black, u, v);
    SetFlareVertex(pVertices[3], pos.x, pos.y, pos.z, faded, u, v);
    SetFlareVertex(pVertices[4], x2, y2, pos.z, black, u, v);
    SetFlareVertex(pVertices[5], x0, y0, pos.z, black, u, v);
    pVertices[6] = pVertices[5];
    SetFlareVertex(pVertices[7], x3, y3, pos.z, black, u, v);
    pVertices[8] = pVertices[7];
    SetFlareVertex(pVertices[9], x4, y4, pos.z, black, u, v);
    SetFlareVertex(pVertices[10], pos.x, pos.y, pos.z, faded, u, v);
    SetFlareVertex(pVertices[11], x5, y5, pos.z, black, u, v);
    SetFlareVertex(pVertices[12], x3, y3, pos.z, black, u, v);
    pVertices[13] = pVertices[12];
    pVertices += 14;

    RedColor half = _color;
    half.a >>= 1;
    RedColor clear = _color;
    clear.a = 0;
    EmitRing(pVertices, pos, angle, 0.0f, _fSize, _fSize * 4.0f,
             _color, half, clear, 12, fU, fV);
}

static void SetFillVertex(FlareVertex& vertex, float fX, float fY, float fZ)
{
    vertex.pos.x = fX;
    vertex.pos.y = fY;
    vertex.pos.z = fZ;
    vertex.color = RedColor::WHITE;
}

static void SetFlareVertex(FlareVertex& vertex, float x, float y, float z,
                           const RedColor& color, float u, float v)
{
    vertex.pos.x = x;
    vertex.pos.y = y;
    vertex.pos.z = z;
    vertex.color = color;
    vertex.uv.u = u;
    vertex.uv.v = v;
}

// FUNCTION: SWBF2 0x0041e3b0
template <>
void LensFlare::Emit<FlareVertex>(FlareVertex*& pVertices, const PblVector3& pos, float fU,
                                  float fV, float fFade)
{
    const float angle = ((pos.y * 2.0f * 3.1415901f + pos.x * 3.1415901f) / pos.z)
                      + _fSize * 1.0472f + _fRandomRotOffset;
    float s1, c1, s2, c2;
    PblMath::SinCos(&s1, &c1, angle);
    PblMath::SinCos(&s2, &c2, 1.0472f - angle);

    const float size = _fSize * 2.0f;
    const float z = pos.z + 0.0f;
    const float x0 = pos.x + c1 * size;
    const float y0 = pos.y + s1 * (size * 2.0f);
    const float x1 = pos.x + (c1 * -0.5f - s1 * 0.866f) * size;
    const float y1 = pos.y + (c1 * 0.866f - s1 * 0.5f) * (size * 2.0f);
    const float x2 = pos.x + (c1 * -0.5f - s1 * -0.866f) * size;
    const float y2 = pos.y + (s1 * -0.5f - c1 * 0.866f) * (size * 2.0f);
    const float x3 = pos.x + c2 * (size * 3.5f);
    const float y3 = pos.y + s2 * size;
    const float x4 = pos.x + (c2 * -0.5f - s2 * 0.866f) * (size * 3.5f);
    const float y4 = pos.y + (c2 * 0.866f - s2 * 0.5f) * size;
    const float x5 = pos.x + (c2 * -0.5f - s2 * -0.866f) * (size * 3.5f);
    const float y5 = pos.y + (s2 * -0.5f - c2 * 0.866f) * size;

    RedColor faded = _color;
    faded.a = (unsigned char)(int)((float)_color.a * fFade);
    RedColor black = RedColor::BLACK;
    black.a = 0;
    SetFlareVertex(pVertices[0], x0, y0, z, black, fU, fV);
    pVertices[1] = pVertices[0];
    SetFlareVertex(pVertices[2], x1, y1, z, black, fU, fV);
    SetFlareVertex(pVertices[3], pos.x, pos.y, pos.z, faded, fU, fV);
    SetFlareVertex(pVertices[4], x2, y2, z, black, fU, fV);
    SetFlareVertex(pVertices[5], x0, y0, z, black, fU, fV);
    pVertices[6] = pVertices[5];
    SetFlareVertex(pVertices[7], x3, y3, z, black, fU, fV);
    pVertices[8] = pVertices[7];
    SetFlareVertex(pVertices[9], x4, y4, z, black, fU, fV);
    SetFlareVertex(pVertices[10], pos.x, pos.y, pos.z, faded, fU, fV);
    SetFlareVertex(pVertices[11], x5, y5, z, black, fU, fV);
    SetFlareVertex(pVertices[12], x3, y3, z, black, fU, fV);
    pVertices[13] = pVertices[12];
    pVertices += 14;

    RedColor half = _color;
    half.a >>= 1;
    RedColor clear = _color;
    clear.a = 0;
    EmitRing(pVertices, pos, angle, 0.0f, _fSize, _fSize * 4.0f,
               _color, half, clear, 12, fU, fV);
}

// FUNCTION: SWBF2 0x0041c1b0
template <>
void LensFlareEffect::LensFlareList::Render<FlareVertex, 642>() const
{
    const PblMatrix& mView = RedRenderer::GetCamera()->GetMatrixInverse();
    RedRenderer::GetCamera();
    float fNear;
    float fFar;
    float fWidth;
    float fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    const unsigned int uiView = FLRenderer::GetCurrentViewport();
    float fViewMinX;
    float fViewMinY;
    float fViewMaxX;
    float fViewMaxY;
    RedRenderer::pcGetViewportExtents(fViewMinX, fViewMinY, fViewMaxX, fViewMaxY);

    PblVector3 vPosition;
    float fSize = 0.0f;
    float fSquaredDistance = 0.0f;
    Itor it(*this);
    for (; it; ++it)
    {
        if (IsFlareVisible(it, uiView, mView, fNear, fFar, fWidth, fHeight, vPosition, fSize,
                           fSquaredDistance))
        {
            break;
        }
    }
    if (!it)
    {
        return;
    }

    do
    {
        int iNumFlares = 0;
        pcRedVertexBuffer* pFlareBuffer = pcRedVertexBuffer::Create(1920, sizeof(FlareVertex), false, true);
        pcRedVertexBuffer* pFillBuffer = pcRedVertexBuffer::Create(96, sizeof(FlareVertex), false, true);
        pcRedVertexBuffer* pClearBuffer = pcRedVertexBuffer::Create(96, 4 * sizeof(float), false, true);

        CompressedPos_SetBoundingBox(s_boundingBox);
        CompressedPosXY_SetBoundingBox(PblBox(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));

        unsigned int uiFlareStart;
        unsigned int uiFillStart;
        unsigned int uiClearStart;
        FlareVertex* pFlareVertices = (FlareVertex*)pFlareBuffer->Lock(1920, uiFlareStart);
        FlareVertex* const pFlareBase = pFlareVertices;
        FlareVertex* pFillVertices = (FlareVertex*)pFillBuffer->Lock(96, uiFillStart);
        FlareVertex* const pFillBase = pFillVertices;
        float* pClearVertices = (float*)pClearBuffer->Lock(96, uiClearStart);
        float* const pClearBase = pClearVertices;

        RECT destRects[16];
        RECT sourceRects[16];
        float fSampleY = 0.0f;
        float fSampleX = 0.0f;
        float fU = 0.03125f;
        float fV = 0.03125f;
        do
        {
            float fFade = 1.0f;
            if (fSquaredDistance > it->GetSquaredDrawDistance() * kfFadeDrawDistanceSquared)
            {
                fFade = (1.0f - fSquaredDistance / it->GetSquaredDrawDistance()) * kfFadeDrawDistanceFactor;
            }

            const float fMinX = vPosition.x - fSize;
            const float fMinY = vPosition.y - fSize;
            const PblVector3 vScreenMin = RedRenderer::TransformCameraPointToScreenSpace(
                PblVector3(fMinX, fMinY, vPosition.z));
            const float fMaxX = vPosition.x + fSize;
            const float fMaxY = vPosition.y + fSize;
            const PblVector3 vScreenMax = RedRenderer::TransformCameraPointToScreenSpace(
                PblVector3(fMaxX, fMaxY, vPosition.z));

            SetClearVertex(pClearVertices + 0, vScreenMin.x - 2.0f, vScreenMax.y - 2.0f);
            SetClearVertex(pClearVertices + 4, vScreenMin.x - 2.0f, vScreenMin.y + 2.0f);
            SetClearVertex(pClearVertices + 8, vScreenMax.x + 2.0f, vScreenMax.y - 2.0f);
            SetClearVertex(pClearVertices + 12, vScreenMin.x - 2.0f, vScreenMin.y + 2.0f);
            SetClearVertex(pClearVertices + 16, vScreenMax.x + 2.0f, vScreenMax.y - 2.0f);
            SetClearVertex(pClearVertices + 20, vScreenMax.x + 2.0f, vScreenMin.y + 2.0f);
            pClearVertices += 24;

            const float fZ = vPosition.z + fSize;
            SetFillVertex(pFillVertices[0], fMinX, fMaxY, fZ);
            SetFillVertex(pFillVertices[1], fMaxX, fMaxY, fZ);
            SetFillVertex(pFillVertices[2], fMaxX, fMinY, fZ);
            SetFillVertex(pFillVertices[3], fMinX, fMaxY, fZ);
            SetFillVertex(pFillVertices[4], fMaxX, fMinY, fZ);
            SetFillVertex(pFillVertices[5], fMinX, fMinY, fZ);
            pFillVertices += 6;

            RECT source;
            source.left = (int)vScreenMin.x;
            source.top = (int)vScreenMax.y;
            source.right = (int)vScreenMax.x + 1;
            source.bottom = (int)vScreenMin.y + 1;
            RECT dest;
            dest.left = (int)fSampleX;
            dest.top = (int)fSampleY;
            fSampleX += 16.0f;
            dest.right = (int)fSampleX;
            dest.bottom = (int)(fSampleY + 16.0f);
            if (fViewMinX > (float)source.left)
            {
                source.left = (int)fViewMinX;
            }
            if ((float)source.right > fViewMaxX)
            {
                source.right = (int)fViewMaxX;
            }
            if (fViewMinY > (float)source.top)
            {
                source.top = (int)fViewMinY;
            }
            if ((float)source.bottom > fViewMaxY)
            {
                source.bottom = (int)fViewMaxY;
            }
            sourceRects[iNumFlares] = source;
            destRects[iNumFlares] = dest;

            it->Emit(pFlareVertices, vPosition, fU, fV, fFade);

            fU += 0.0625f;
            if (fU >= 1.0f)
            {
                fV += 0.0625f;
                fU = 0.03125f;
            }
            if (fSampleX >= 64.0f)
            {
                fSampleY += 16.0f;
                fSampleX = 0.0f;
            }

            ++iNumFlares;
            for (++it; it; ++it)
            {
                if (IsFlareVisible(it, uiView, mView, fNear, fFar, fWidth, fHeight, vPosition,
                                   fSize, fSquaredDistance))
                {
                    break;
                }
            }
        } while (iNumFlares < 16 && it);

        const int iNumFillVertices = pFillVertices - pFillBase;
        const int iNumFlareVertices = pFlareVertices - pFlareBase;
        const int iNumClearVertices = (pClearVertices - pClearBase) / 4;
        pFillBuffer->Unlock();
        pFlareBuffer->Unlock();
        pClearBuffer->Unlock();

        pcRedPrimitive* pFlarePrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLESTRIP, iNumFlareVertices - 2, 642, iNumFlareVertices, uiFlareStart,
            pFlareBuffer, 0, 0);
        pFlarePrimitive->m_bbox = s_boundingBox;
        pcRedPrimitive* pFillPrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLELIST, iNumFillVertices / 3, 642, iNumFillVertices, uiFillStart,
            pFillBuffer, 0, 0);
        pFillPrimitive->m_bbox = s_boundingBox;
        pcRedPrimitive* pClearPrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLELIST, iNumClearVertices / 3, 3, iNumClearVertices, uiClearStart,
            pClearBuffer, 0, 0);

        RedColor clearColor = { 0, 0, 0, 0 };
        RedRenderer::pcRenderPrimitive(pClearPrimitive, s_pClearShader, gMatrixIdentity,
                                       clearColor, 0x200, 0, 0, 0);
        RedColor fillColor = RedColor::BLACK;
        fillColor.a = 255;
        RedRenderer::pcRenderPrimitive(pFillPrimitive, s_pFillShader,
                                       RedRenderer::GetCamera()->GetMatrix(), fillColor, 0x200, 0, 0, 0);

        IDirect3DSurface9* pBackBuffer;
        RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
        IDirect3DSurface9* pSampleSurface;
        ((IDirect3DTexture9*)s_sampleTex->m_pTexture)->GetSurfaceLevel(0, &pSampleSurface);
        RedRenderer::pcGetDevice()->ColorFill(pSampleSurface, 0, 0);
        for (int i = 0; i < iNumFlares; i++)
        {
            RedRenderer::pcGetDevice()->StretchRect(pBackBuffer, &sourceRects[i], pSampleSurface,
                                                    &destRects[i], D3DTEXF_NONE);
        }
        pBackBuffer->Release();
        pSampleSurface->Release();

        RedTexture* pRenderTarget = RedRenderer::pcGetRenderTarget();
        IDirect3DSurface9* pDepthTarget = RedRenderer::pcGetDepthTarget();
        RedViewport viewport;
        RedRenderer::pcGetViewport(viewport);
        RedRenderer::pcSetRenderTarget(s_intensityTex, 0);
        RedColor intensityClear = RedColor::BLACK;
        intensityClear.a = 0;
        RedRenderer::pcClear(1, intensityClear, 1.0f, 0);

        float fOutputX = 0.0f;
        float fOutputY = 0.0f;
        float fInputX = 0.0f;
        float fInputY = 0.0f;
        for (int i = 0; i < iNumFlares; i++)
        {
            s_pSampleShader->SetOutputPosition(fOutputX, fOutputY);
            s_pSampleShader->SetInputOffset(fInputX, fInputY);
            RedRenderer::pcRenderPrimitive(s_samplePoints, s_pSampleShader,
                                           gMatrixIdentity, RedColor::WHITE, 0x200, 0, 0, 0);
            fOutputX += 1.0f;
            if (fOutputX >= 16.0f)
            {
                fOutputY += 1.0f;
                fOutputX = 0.0f;
            }
            fInputX += 16.0f;
            if (fInputX >= 64.0f)
            {
                fInputY += 16.0f;
                fInputX = 0.0f;
            }
        }

        RedRenderer::pcSetRenderTarget(pRenderTarget, pDepthTarget);
        RedRenderer::pcSetViewport(viewport);
        RedRenderer::pcRenderPrimitive(pFlarePrimitive, s_pFlareShader,
                                       RedRenderer::GetCamera()->GetMatrix(), RedColor::WHITE, 0x200, 0, 0, 0);

        pFlarePrimitive->Release();
        pFillPrimitive->Release();
        pClearPrimitive->Release();
        pFlareBuffer->Release();
        pFillBuffer->Release();
        pClearBuffer->Release();
    } while (it);
}

// FUNCTION: SWBF2 0x0041d1e0
template <>
void LensFlareEffect::LensFlareList::Render<FlareCompressedVertex, 37506>() const
{
    const PblMatrix& mView = RedRenderer::GetCamera()->GetMatrixInverse();
    RedRenderer::GetCamera();
    float fNear;
    float fFar;
    float fWidth;
    float fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    const unsigned int uiView = FLRenderer::GetCurrentViewport();
    float fViewMinX;
    float fViewMinY;
    float fViewMaxX;
    float fViewMaxY;
    RedRenderer::pcGetViewportExtents(fViewMinX, fViewMinY, fViewMaxX, fViewMaxY);

    PblVector3 vPosition;
    float fSize = 0.0f;
    float fSquaredDistance = 0.0f;
    Itor it(*this);
    for (; it; ++it)
    {
        if (IsFlareVisible(it, uiView, mView, fNear, fFar, fWidth, fHeight, vPosition, fSize,
                           fSquaredDistance))
        {
            break;
        }
    }
    if (!it)
    {
        return;
    }

    do
    {
        int iNumFlares = 0;
        pcRedVertexBuffer* pFlareBuffer = pcRedVertexBuffer::Create(1920, sizeof(FlareCompressedVertex), false, true);
        pcRedVertexBuffer* pFillBuffer = pcRedVertexBuffer::Create(96, sizeof(FlareCompressedVertex), false, true);
        pcRedVertexBuffer* pClearBuffer = pcRedVertexBuffer::Create(96, 4 * sizeof(float), false, true);

        CompressedPos_SetBoundingBox(s_boundingBox);
        CompressedPosXY_SetBoundingBox(PblBox(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));

        unsigned int uiFlareStart;
        unsigned int uiFillStart;
        unsigned int uiClearStart;
        FlareCompressedVertex* pFlareVertices = (FlareCompressedVertex*)pFlareBuffer->Lock(1920, uiFlareStart);
        FlareCompressedVertex* const pFlareBase = pFlareVertices;
        FlareCompressedVertex* pFillVertices = (FlareCompressedVertex*)pFillBuffer->Lock(96, uiFillStart);
        FlareCompressedVertex* const pFillBase = pFillVertices;
        float* pClearVertices = (float*)pClearBuffer->Lock(96, uiClearStart);
        float* const pClearBase = pClearVertices;

        RECT destRects[16];
        RECT sourceRects[16];
        float fSampleY = 0.0f;
        float fSampleX = 0.0f;
        float fU = 0.03125f;
        float fV = 0.03125f;
        do
        {
            float fFade = 1.0f;
            if (fSquaredDistance > it->GetSquaredDrawDistance() * kfFadeDrawDistanceSquared)
            {
                fFade = (1.0f - fSquaredDistance / it->GetSquaredDrawDistance()) * kfFadeDrawDistanceFactor;
            }

            const float fMinX = vPosition.x - fSize;
            const float fMinY = vPosition.y - fSize;
            const PblVector3 vScreenMin = RedRenderer::TransformCameraPointToScreenSpace(
                PblVector3(fMinX, fMinY, vPosition.z));
            const float fMaxX = vPosition.x + fSize;
            const float fMaxY = vPosition.y + fSize;
            const PblVector3 vScreenMax = RedRenderer::TransformCameraPointToScreenSpace(
                PblVector3(fMaxX, fMaxY, vPosition.z));

            SetClearVertex(pClearVertices + 0, vScreenMin.x - 2.0f, vScreenMax.y - 2.0f);
            SetClearVertex(pClearVertices + 4, vScreenMin.x - 2.0f, vScreenMin.y + 2.0f);
            SetClearVertex(pClearVertices + 8, vScreenMax.x + 2.0f, vScreenMax.y - 2.0f);
            SetClearVertex(pClearVertices + 12, vScreenMin.x - 2.0f, vScreenMin.y + 2.0f);
            SetClearVertex(pClearVertices + 16, vScreenMax.x + 2.0f, vScreenMax.y - 2.0f);
            SetClearVertex(pClearVertices + 20, vScreenMax.x + 2.0f, vScreenMin.y + 2.0f);
            pClearVertices += 24;

            const float fZ = vPosition.z + fSize;
            SetFillVertex(pFillVertices[0], fMinX, fMaxY, fZ);
            SetFillVertex(pFillVertices[1], fMaxX, fMaxY, fZ);
            SetFillVertex(pFillVertices[2], fMaxX, fMinY, fZ);
            SetFillVertex(pFillVertices[3], fMinX, fMaxY, fZ);
            SetFillVertex(pFillVertices[4], fMaxX, fMinY, fZ);
            SetFillVertex(pFillVertices[5], fMinX, fMinY, fZ);
            pFillVertices += 6;

            RECT source;
            source.left = (int)vScreenMin.x;
            source.top = (int)vScreenMax.y;
            source.right = (int)vScreenMax.x + 1;
            source.bottom = (int)vScreenMin.y + 1;
            RECT dest;
            dest.left = (int)fSampleX;
            dest.top = (int)fSampleY;
            fSampleX += 16.0f;
            dest.right = (int)fSampleX;
            dest.bottom = (int)(fSampleY + 16.0f);
            if (fViewMinX > (float)source.left)
            {
                source.left = (int)fViewMinX;
            }
            if ((float)source.right > fViewMaxX)
            {
                source.right = (int)fViewMaxX;
            }
            if (fViewMinY > (float)source.top)
            {
                source.top = (int)fViewMinY;
            }
            if ((float)source.bottom > fViewMaxY)
            {
                source.bottom = (int)fViewMaxY;
            }
            sourceRects[iNumFlares] = source;
            destRects[iNumFlares] = dest;

            it->Emit(pFlareVertices, vPosition, fU, fV, fFade);

            fU += 0.0625f;
            if (fU >= 1.0f)
            {
                fV += 0.0625f;
                fU = 0.03125f;
            }
            if (fSampleX >= 64.0f)
            {
                fSampleY += 16.0f;
                fSampleX = 0.0f;
            }

            ++iNumFlares;
            for (++it; it; ++it)
            {
                if (IsFlareVisible(it, uiView, mView, fNear, fFar, fWidth, fHeight, vPosition,
                                   fSize, fSquaredDistance))
                {
                    break;
                }
            }
        } while (iNumFlares < 16 && it);

        const int iNumFillVertices = pFillVertices - pFillBase;
        const int iNumFlareVertices = pFlareVertices - pFlareBase;
        const int iNumClearVertices = (pClearVertices - pClearBase) / 4;
        pFillBuffer->Unlock();
        pFlareBuffer->Unlock();
        pClearBuffer->Unlock();

        pcRedPrimitive* pFlarePrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLESTRIP, iNumFlareVertices - 2, 37506, iNumFlareVertices, uiFlareStart,
            pFlareBuffer, 0, 0);
        pFlarePrimitive->m_bbox = s_boundingBox;
        pcRedPrimitive* pFillPrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLELIST, iNumFillVertices / 3, 37506, iNumFillVertices, uiFillStart,
            pFillBuffer, 0, 0);
        pFillPrimitive->m_bbox = s_boundingBox;
        pcRedPrimitive* pClearPrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLELIST, iNumClearVertices / 3, 3, iNumClearVertices, uiClearStart,
            pClearBuffer, 0, 0);

        RedColor clearColor = { 0, 0, 0, 0 };
        RedRenderer::pcRenderPrimitive(pClearPrimitive, s_pClearShader, gMatrixIdentity,
                                       clearColor, 0x200, 0, 0, 0);
        RedColor fillColor = RedColor::BLACK;
        fillColor.a = 255;
        RedRenderer::pcRenderPrimitive(pFillPrimitive, s_pFillShader,
                                       RedRenderer::GetCamera()->GetMatrix(), fillColor, 0x200, 0, 0, 0);

        IDirect3DSurface9* pBackBuffer;
        RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
        IDirect3DSurface9* pSampleSurface;
        ((IDirect3DTexture9*)s_sampleTex->m_pTexture)->GetSurfaceLevel(0, &pSampleSurface);
        RedRenderer::pcGetDevice()->ColorFill(pSampleSurface, 0, 0);
        for (int i = 0; i < iNumFlares; i++)
        {
            RedRenderer::pcGetDevice()->StretchRect(pBackBuffer, &sourceRects[i], pSampleSurface,
                                                    &destRects[i], D3DTEXF_NONE);
        }
        pBackBuffer->Release();
        pSampleSurface->Release();

        RedTexture* pRenderTarget = RedRenderer::pcGetRenderTarget();
        IDirect3DSurface9* pDepthTarget = RedRenderer::pcGetDepthTarget();
        RedViewport viewport;
        RedRenderer::pcGetViewport(viewport);
        RedRenderer::pcSetRenderTarget(s_intensityTex, 0);
        RedColor intensityClear = RedColor::BLACK;
        intensityClear.a = 0;
        RedRenderer::pcClear(1, intensityClear, 1.0f, 0);

        float fOutputX = 0.0f;
        float fOutputY = 0.0f;
        float fInputX = 0.0f;
        float fInputY = 0.0f;
        for (int i = 0; i < iNumFlares; i++)
        {
            s_pSampleShader->SetOutputPosition(fOutputX, fOutputY);
            s_pSampleShader->SetInputOffset(fInputX, fInputY);
            RedRenderer::pcRenderPrimitive(s_samplePoints, s_pSampleShader,
                                           gMatrixIdentity, RedColor::WHITE, 0x200, 0, 0, 0);
            fOutputX += 1.0f;
            if (fOutputX >= 16.0f)
            {
                fOutputY += 1.0f;
                fOutputX = 0.0f;
            }
            fInputX += 16.0f;
            if (fInputX >= 64.0f)
            {
                fInputY += 16.0f;
                fInputX = 0.0f;
            }
        }

        RedRenderer::pcSetRenderTarget(pRenderTarget, pDepthTarget);
        RedRenderer::pcSetViewport(viewport);
        RedRenderer::pcRenderPrimitive(pFlarePrimitive, s_pFlareShader,
                                       RedRenderer::GetCamera()->GetMatrix(), RedColor::WHITE, 0x200, 0, 0, 0);

        pFlarePrimitive->Release();
        pFillPrimitive->Release();
        pClearPrimitive->Release();
        pFlareBuffer->Release();
        pFillBuffer->Release();
        pClearBuffer->Release();
    } while (it);
}
