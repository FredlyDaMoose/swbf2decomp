#include "Battlefront2/Source/BirdEffect.h"
#include "pcSunFlare.h"
#include "PebbleFL/Common/PblRef.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

// FUNCTION: SWBF2 0x0040cde0
void BirdParticle::Render(unsigned char, float, unsigned int)
{
    mTimeOffCamera = 0.0f;

    pcRedPrimitive* pPrimitive;
    if (RedRenderer::pcGetGeometryMode())
    {
        pPrimitive = CreateCompressedGeometry();
    }
    else
    {
        pPrimitive = CreateGeometry();
    }

    RedRenderer::pcRenderPrimitive(pPrimitive,
                                   mType.m_pShader,
                                   mMatrix,
                                   RedColor::WHITE,
                                   0x01000010, 0, 0, 0);
    pPrimitive->Release();
}

// SYNTHETIC: SWBF2 0x0040cb80
// `vector constructor iterator'

// FUNCTION: SWBF2 0x0040cd00
void BirdParticleManager::PlatformInit()
{
    unsigned int uiFormat =
        RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED ? 0x9282 : 0x282;
    for (int i = 0; i < sNumBirdTypes; i++)
    {
        sBirdTypes[i].m_pShader = pcRedShader::Create(PblHash("Normal"), uiFormat, &sBirdTypes[i].mTexture, 1,
                                                 0x10004, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
}

// FUNCTION: SWBF2 0x0040cd70
void BirdParticleManager::PlatformCleanup()
{
    for (int i = 0; i < sNumBirdTypes; i++)
    {
        if (sBirdTypes[i].m_pShader)
        {
            sBirdTypes[i].m_pShader->Release();
            sBirdTypes[i].m_pShader = 0;
        }
    }
    sNumBirdTypes = 0;
}

// GLOBAL: SWBF2 0x0078c2e4
const RedColor kBirdColor = { 0xcc, 0xcc, 0xcc, 0xff };

namespace
{

struct TexCoord
{
    float u, v;
    void SetUV(float fU, float fV) { u = fU; v = fV; }
};

struct CompressedTexCoord
{
    short u, v;
    void SetUV(float fU, float fV) { u = (short)(fU * 2048.0f); v = (short)(fV * 2048.0f); }
};

struct BirdVertex
{
    PblVector3 pos;
    RedColor color;
    TexCoord uv;
};
 

struct BirdCompressedVertex
{
    CompressedPos pos;
    RedColor color;
    CompressedTexCoord uv;
};
 

inline void SetBirdVertex(BirdVertex& v, const PblVector3& vPos, float fU, float fV)
{
    v.pos = vPos;
    v.color = kBirdColor;
    v.uv.SetUV(fU, fV);
}

inline void SetBirdVertex(BirdCompressedVertex& v, const PblVector3& vPos, float fU, float fV)
{
    v.pos.SetPos(vPos.x, vPos.y, vPos.z);
    v.color = kBirdColor;
    v.uv.SetUV(fU, fV);
}

inline PblVector3 Add(const PblVector3& a, const PblVector3& b)
{
    return PblVector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline PblVector3 Sub(const PblVector3& a, const PblVector3& b)
{
    return PblVector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline PblVector3 Scale(const PblVector3& a, float f)
{
    return PblVector3(a.x * f, a.y * f, a.z * f);
}

struct BirdWing
{
    PblVector3 vElbow;
    PblVector3 vTip;
    PblVector3 vWidth;
};

inline BirdWing MakeBirdWing(const BirdParticle& bird, const PblVector3& vShoulder, float fSize, bool bMirror)
{
    PblVector3 vWing1 = bird.mWing1Matrix.GetXAxis();
    PblVector3 vWidth = bird.mWing1Matrix.GetZAxis();
    PblVector3 vWing2 = bird.mWing2Matrix.GetXAxis();
    if (bMirror)
    {
        vWing1.x = -vWing1.x;
        vWidth.x = -vWidth.x;
        vWing2.x = -vWing2.x;
    }
    BirdWing wing;
    wing.vWidth = Scale(vWidth, fSize * 0.5f);
    wing.vElbow = Add(Scale(vWing1, fSize), vShoulder);
    wing.vTip = Add(wing.vElbow, Scale(vWing2, fSize));
    return wing;
}
}

// FUNCTION: SWBF2 0x0040ce80
pcRedPrimitive* BirdParticle::CreateCompressedGeometry()
{
    float fSize = mType.mSize * 0.5f;
    float fExtent = mType.mSize * 2.0f * 0.5f;

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLELIST, 0x9282,
                                                        sizeof(BirdCompressedVertex), 30, 0, false);
    PblBox box;
    box._vMin = PblVector3(-fExtent, -fExtent, -fExtent);
    box._vMax = PblVector3(fExtent, fExtent, fExtent);
    pPrimitive->m_bbox = box;
    CompressedPos::SetBoundingBox(&box);

    BirdCompressedVertex* pVerts = (BirdCompressedVertex*)pPrimitive->BeginVertexData();
    if (pVerts)
    {
        float fHalf = fSize * 0.5f;
        SetBirdVertex(pVerts[0], PblVector3(0.0f, fSize * -0.5f, -fSize), 0.0f, 1.0f);
        SetBirdVertex(pVerts[1], PblVector3(0.0f, fSize * -0.5f, fSize), 1.0f, 1.0f);
        SetBirdVertex(pVerts[2], PblVector3(0.0f, fHalf, fSize), 1.0f, 0.5f);
        SetBirdVertex(pVerts[3], PblVector3(0.0f, fSize * -0.5f, -fSize), 0.0f, 1.0f);
        SetBirdVertex(pVerts[4], PblVector3(0.0f, fHalf, fSize), 1.0f, 0.5f);
        SetBirdVertex(pVerts[5], PblVector3(0.0f, fHalf, -fSize), 0.0f, 0.5f);

        const PblVector3 vShoulder(0.0f, 0.0f, fSize * 0.65f);
        BirdWing r = MakeBirdWing(*this, vShoulder, fSize, false);
        BirdWing l = MakeBirdWing(*this, vShoulder, fSize, true);

        SetBirdVertex(pVerts[6], Sub(vShoulder, r.vWidth), 0.0f, 0.5f);
        SetBirdVertex(pVerts[7], Add(vShoulder, r.vWidth), 0.0f, 0.0f);
        SetBirdVertex(pVerts[8], Add(r.vElbow, r.vWidth), 0.5f, 0.0f);
        SetBirdVertex(pVerts[9], Sub(vShoulder, r.vWidth), 0.0f, 0.5f);
        SetBirdVertex(pVerts[10], Add(r.vElbow, r.vWidth), 0.5f, 0.0f);
        SetBirdVertex(pVerts[11], Sub(r.vElbow, r.vWidth), 0.5f, 0.5f);

        SetBirdVertex(pVerts[12], Sub(vShoulder, l.vWidth), 0.0f, 0.5f);
        SetBirdVertex(pVerts[13], Add(vShoulder, l.vWidth), 0.0f, 0.0f);
        SetBirdVertex(pVerts[14], Add(l.vElbow, l.vWidth), 0.5f, 0.0f);
        SetBirdVertex(pVerts[15], Sub(vShoulder, l.vWidth), 0.0f, 0.5f);
        SetBirdVertex(pVerts[16], Add(l.vElbow, l.vWidth), 0.5f, 0.0f);
        SetBirdVertex(pVerts[17], Sub(l.vElbow, l.vWidth), 0.5f, 0.5f);

        SetBirdVertex(pVerts[18], Add(r.vElbow, r.vWidth), 0.5f, 0.0f);
        SetBirdVertex(pVerts[19], Sub(r.vElbow, r.vWidth), 0.5f, 0.5f);
        SetBirdVertex(pVerts[20], Add(r.vTip, r.vWidth), 1.0f, 0.0f);
        SetBirdVertex(pVerts[21], Sub(r.vElbow, r.vWidth), 0.5f, 0.5f);
        SetBirdVertex(pVerts[22], Add(r.vTip, r.vWidth), 1.0f, 0.0f);
        SetBirdVertex(pVerts[23], Sub(r.vTip, r.vWidth), 1.0f, 0.5f);

        SetBirdVertex(pVerts[24], Sub(l.vElbow, l.vWidth), 0.5f, 0.5f);
        SetBirdVertex(pVerts[25], Add(l.vElbow, l.vWidth), 0.5f, 0.0f);
        SetBirdVertex(pVerts[26], Add(l.vTip, l.vWidth), 1.0f, 0.0f);
        SetBirdVertex(pVerts[27], Sub(l.vElbow, l.vWidth), 0.5f, 0.5f);
        SetBirdVertex(pVerts[28], Add(l.vTip, l.vWidth), 1.0f, 0.0f);
        SetBirdVertex(pVerts[29], Sub(l.vTip, l.vWidth), 1.0f, 0.5f);

        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}

// FUNCTION: SWBF2 0x0040e710
pcRedPrimitive* BirdParticle::CreateGeometry()
{
    float fSize = mType.mSize * 0.5f;

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLELIST, 0x282, sizeof(BirdVertex), 30, 0, false);
    if (pPrimitive)
    {
        BirdVertex* pVerts = (BirdVertex*)pPrimitive->BeginVertexData();
        if (pVerts)
        {
            float fHalf = fSize * 0.5f;
            SetBirdVertex(pVerts[0], PblVector3(0.0f, fSize * -0.5f, -fSize), 0.0f, 1.0f);
            SetBirdVertex(pVerts[1], PblVector3(0.0f, fSize * -0.5f, fSize), 1.0f, 1.0f);
            SetBirdVertex(pVerts[2], PblVector3(0.0f, fHalf, fSize), 1.0f, 0.5f);
            SetBirdVertex(pVerts[3], PblVector3(0.0f, fSize * -0.5f, -fSize), 0.0f, 1.0f);
            SetBirdVertex(pVerts[4], PblVector3(0.0f, fHalf, fSize), 1.0f, 0.5f);
            SetBirdVertex(pVerts[5], PblVector3(0.0f, fHalf, -fSize), 0.0f, 0.5f);

            const PblVector3 vShoulder(0.0f, 0.0f, fSize * 0.65f);
            BirdWing r = MakeBirdWing(*this, vShoulder, fSize, false);
            BirdWing l = MakeBirdWing(*this, vShoulder, fSize, true);

            SetBirdVertex(pVerts[6], Sub(vShoulder, r.vWidth), 0.0f, 0.5f);
            SetBirdVertex(pVerts[7], Add(vShoulder, r.vWidth), 0.0f, 0.0f);
            SetBirdVertex(pVerts[8], Add(r.vElbow, r.vWidth), 0.5f, 0.0f);
            SetBirdVertex(pVerts[9], Sub(vShoulder, r.vWidth), 0.0f, 0.5f);
            SetBirdVertex(pVerts[10], Add(r.vElbow, r.vWidth), 0.5f, 0.0f);
            SetBirdVertex(pVerts[11], Sub(r.vElbow, r.vWidth), 0.5f, 0.5f);

            SetBirdVertex(pVerts[12], Sub(vShoulder, l.vWidth), 0.0f, 0.5f);
            SetBirdVertex(pVerts[13], Add(vShoulder, l.vWidth), 0.0f, 0.0f);
            SetBirdVertex(pVerts[14], Add(l.vElbow, l.vWidth), 0.5f, 0.0f);
            SetBirdVertex(pVerts[15], Sub(vShoulder, l.vWidth), 0.0f, 0.5f);
            SetBirdVertex(pVerts[16], Add(l.vElbow, l.vWidth), 0.5f, 0.0f);
            SetBirdVertex(pVerts[17], Sub(l.vElbow, l.vWidth), 0.5f, 0.5f);

            SetBirdVertex(pVerts[18], Sub(r.vElbow, r.vWidth), 0.5f, 0.5f);
            SetBirdVertex(pVerts[19], Add(r.vElbow, r.vWidth), 0.5f, 0.0f);
            SetBirdVertex(pVerts[20], Add(r.vTip, r.vWidth), 1.0f, 0.0f);
            SetBirdVertex(pVerts[21], Sub(r.vElbow, r.vWidth), 0.5f, 0.5f);
            SetBirdVertex(pVerts[22], Add(r.vTip, r.vWidth), 1.0f, 0.0f);
            SetBirdVertex(pVerts[23], Sub(r.vTip, r.vWidth), 1.0f, 0.5f);

            SetBirdVertex(pVerts[24], Sub(l.vElbow, l.vWidth), 0.5f, 0.5f);
            SetBirdVertex(pVerts[25], Add(l.vElbow, l.vWidth), 0.5f, 0.0f);
            SetBirdVertex(pVerts[26], Add(l.vTip, l.vWidth), 1.0f, 0.0f);
            SetBirdVertex(pVerts[27], Sub(l.vElbow, l.vWidth), 0.5f, 0.5f);
            SetBirdVertex(pVerts[28], Add(l.vTip, l.vWidth), 1.0f, 0.0f);
            SetBirdVertex(pVerts[29], Sub(l.vTip, l.vWidth), 1.0f, 0.5f);

            pPrimitive->EndVertexData();
        }
    }
    return pPrimitive;
}
