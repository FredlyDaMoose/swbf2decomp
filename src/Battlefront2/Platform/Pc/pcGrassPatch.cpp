#include "pcGrassPatch.h"
#include "pcDynamicSprite.h"

#include <math.h>

#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedIndexBuffer.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexBuffer.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include <new>

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// GLOBAL: SWBF2 0x007eb6e0
static PblBox sBox(-100.0f, -100.0f, -100.0f, 100.0f, 100.0f, 100.0f);

namespace
{

struct GrassVertexCompressed
{
    short pos[3];
    short pad;
    unsigned char normal[3];
    unsigned char padNormal;
    RedColor color;
    GrassPatchRenderable::CompressedTexCoord uv;
};
struct GrassVertexUncompressed
{
    float pos[3];
    float normal[3];
    RedColor color;
    GrassPatchRenderable::TexCoord uv;
};

inline void RenderPrimitive(const pcRedPrimitive* pPrimitive, const pcRedShader* pShader,
                            const PblMatrix* pMatrix, const RedColor* pColor, unsigned int uiFlags)
{
    RedRenderer::pcRenderPrimitive((pcRedPrimitive*)pPrimitive, (pcRedShader*)pShader,
                                   *pMatrix, *pColor, uiFlags,
                                   0, 0, 0);
}

inline RedColor ToRedColor(const RedColorValue& c)
{
    RedColor out;
    out.b = (unsigned char)(int)(c.b * 255.0f);
    out.g = (unsigned char)(int)(c.g * 255.0f);
    out.r = (unsigned char)(int)(c.r * 255.0f);
    out.a = (unsigned char)(int)(c.a * 255.0f);
    return out;
}
}

// FUNCTION: SWBF2 0x00417280
GrassPatchRenderable::GrassPatchRenderable(const GrassPatchClass* pClass)
    : Renderable(2), m_fDistanceFarShader(-3.402823466e38f),
      m_pPrimitive(0), m_pPrimitiveFar(0), m_pClass(const_cast<GrassPatchClass*>(pClass)),
      m_pVertices(0)
{
    const unsigned int vertexFormat = RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_UNCOMPRESSED
        ? 0x2a2 : 0xd2a2;
    m_pShader = pcRedShader::Create(0xe68b9c52, vertexFormat, &pClass->mTextureHash,
                                    1, 0x10006, RedColor::WHITE, RedColor::BLACK, 32.0f);
    m_pShaderFar = pcRedShader::Create(0xe68b9c52, vertexFormat, &pClass->mTextureHash,
                                       1, 0x10002, RedColor::WHITE, RedColor::BLACK, 32.0f);
    m_pShaderTransparent = pcRedShader::Create(0xe68b9c52, vertexFormat, &pClass->mTextureHash,
                                                1, 0x10004, RedColor::WHITE, RedColor::BLACK, 32.0f);
    m_bReflection = false;
    m_pVB = 0;
    const int count = pClass->mNumParticles;
    m_pIBReverse = pcRedIndexBuffer::Create(6 * count, true, true);
    unsigned short* indices = m_pIBReverse->Lock(6 * count, m_startIndexReverse);
    for (int i = count - 1, j = 0; i >= 0; --i, j += 6)
    {
        const int base = i * 4;
        indices[j] = (unsigned short)base;
        indices[j + 1] = (unsigned short)(base + 1);
        indices[j + 2] = (unsigned short)(base + 2);
        indices[j + 3] = (unsigned short)base;
        indices[j + 4] = (unsigned short)(base + 2);
        indices[j + 5] = (unsigned short)(base + 3);
    }
    m_pIBReverse->Unlock();
    m_pIBForward = pcRedIndexBuffer::Create(6 * count, true, true);
    indices = m_pIBForward->Lock(6 * count, m_startIndexForward);
    for (int i = 0, j = 0; i < count; ++i, j += 6)
    {
        const int base = i * 4;
        indices[j] = (unsigned short)base;
        indices[j + 1] = (unsigned short)(base + 1);
        indices[j + 2] = (unsigned short)(base + 2);
        indices[j + 3] = (unsigned short)base;
        indices[j + 4] = (unsigned short)(base + 2);
        indices[j + 5] = (unsigned short)(base + 3);
    }
    m_pIBForward->Unlock();
    mHeap = new PblHeap<int, float>(count, 1000000.0f, 0);
    m_iTexCoordType = pClass->mNumParts == 4 ? 1 : pClass->mNumParts == 6 ? 2 : 0;
    Activate(0);
}

// FUNCTION: SWBF2 0x004175a0
GrassPatchRenderable::~GrassPatchRenderable()
{
    Deactivate();
    if (m_pPrimitive)
    {
        m_pPrimitive->Release();
    }
    if (m_pPrimitiveFar)
    {
        m_pPrimitiveFar->Release();
    }
    if (m_pVB)
    {
        m_pVB->Release();
    }
    if (m_pIBForward)
    {
        m_pIBForward->Release();
    }
    if (m_pIBReverse)
    {
        m_pIBReverse->Release();
    }
    if (m_pShader)
    {
        m_pShader->Release();
    }
    if (m_pShaderFar)
    {
        m_pShaderFar->Release();
    }
    if (m_pShaderTransparent)
    {
        m_pShaderTransparent->Release();
    }
    delete mHeap;
}

// FUNCTION: SWBF2 0x00419000
void GrassPatchClass::PlatformRender(const PblMatrix& matrix, const RedColorValue& color,
                                     unsigned int uiFlags) const
{
    if (!GrassPatch::IsEnabled())
    {
        return;
    }
    RedColor packed = ToRedColor(color);
    mRenderData->RenderInstance(matrix, packed, uiFlags);
}

// FUNCTION: SWBF2 0x00419080
void GrassPatchClass::CreateRenderData()
{
    if (GrassPatch::IsEnabled())
    {
        pcDynamicSprite::Init();
        mRenderData = new GrassPatchRenderable(this);
    }
}

// FUNCTION: SWBF2 0x00418e10
void GrassPatchClass::RenderMaxDist(const PblMatrix& matrix, float fMaxDist,
                                    const RedColorValue& color, unsigned int uiFlags) const
{
    if (!GrassPatch::IsEnabled())
    {
        return;
    }
    fMaxDist = fMaxDist * 20.0f / 20.0f;
    const PblMatrix& cam = RedRenderer::GetCamera()->_Matrix;
    const float distance = cam.m[2][0] * (cam.m[3][0] - matrix.m[3][0]) +
                           cam.m[2][1] * (cam.m[3][1] - matrix.m[3][1]) +
                           cam.m[2][2] * (cam.m[3][2] - matrix.m[3][2]);
    if (distance < -mBoundingRadius || distance - mBoundingRadius > fMaxDist)
    {
        return;
    }
    static float FADE = 20.0f;
    static float inverseFADE = 1.0f / FADE;
    float alpha = 1.0f;
    if (uiFlags & 0x20000000)
    {
        float end = RedRenderer::pcGetNearSceneEnd();
        float start = RedRenderer::pcGetNearSceneStart();
        if (end > fMaxDist)
        {
            end = fMaxDist;
        }
        if (start > fMaxDist - FADE)
        {
            start = fMaxDist - FADE;
        }
        if (distance > end)
        {
            return;
        }
        if (distance > start)
        {
            alpha = (end - distance) / (end - start);
        }
    }
    else if (distance > fMaxDist - FADE)
    {
        alpha = 1.0f - (distance - (fMaxDist - FADE)) * inverseFADE;
        if (alpha < 0.0f)
        {
            alpha = 0.0f;
        }
        if (alpha > 1.0f)
        {
            alpha = 1.0f;
        }
    }
    RedColorValue faded = color;
    faded.a *= alpha;
    PlatformRender(matrix, faded, uiFlags);
}

// FUNCTION: SWBF2 0x00417790
void GrassPatchRenderable::Render(int value)
{
    if (GrassPatch::IsEnabled())
    {
        m_bReflection = ((unsigned int)value & 0x200000) != 0;
        Update();
    }
}

// FUNCTION: SWBF2 0x004177c0
void GrassPatchRenderable::RenderInstance(const PblMatrix& matrix, const RedColor& color,
                                          unsigned int uiFlags) const
{
    if (!GrassPatch::IsEnabled() || m_bReflection != ((uiFlags & 0x200000) != 0))
    {
        return;
    }

    PblSphere sphere;
    sphere.x = matrix.m[3][0];
    sphere.y = matrix.m[3][1];
    sphere.z = matrix.m[3][2];
    sphere.w = m_pClass->mBoundingRadius;
    RedRenderer::SetLightingState(*RedLight::GetLightingState(sphere, 4, false, false, false));

    const PblMatrix& camMat = RedRenderer::GetCamera()->_Matrix;
    PblVector3 vDelta(matrix.m[3][0] - camMat.m[3][0], matrix.m[3][1] - camMat.m[3][1],
                      matrix.m[3][2] - camMat.m[3][2]);
    const float* vForward = RedRenderer::GetCamera()->_Matrix.m[2];
    float fDist = -(vForward[0] * vDelta.x + vForward[1] * vDelta.y + vForward[2] * vDelta.z);

    if (m_bReflection)
    {
        if (m_pPrimitiveFar && color.a == 0xff)
        {
            RenderPrimitive(m_pPrimitiveFar, m_pShaderFar, &matrix, &color, uiFlags);
        }
        else if (m_pPrimitive)
        {
            RenderPrimitive(m_pPrimitive, m_pShader, &matrix, &color, uiFlags);
        }
    }
    else if (m_pClass->mTransparentFlag)
    {
        if (m_pPrimitive)
        {
            RenderPrimitive(m_pPrimitive, m_pShaderTransparent, &matrix, &color,
                                              uiFlags | 8);
        }
    }
    else if (fDist < m_fDistanceFarShader)
    {
        if (m_pPrimitive)
        {
            RenderPrimitive(m_pPrimitive, m_pShader, &matrix, &color, uiFlags);
        }
    }
    else
    {
        PblMatrix mat = matrix;
        float fAlpha = 1.0f - (float)color.a * (1.0f / 255.0f);
        float fFade = 1.0f - fAlpha * fAlpha;
        mat.m[1][0] *= fFade;
        mat.m[1][1] *= fFade;
        mat.m[1][2] *= fFade;
        mat.m[1][3] = 0.0f;

        RedColor opaque;
        opaque.b = color.b;
        opaque.g = color.g;
        opaque.r = color.r;
        opaque.a = 0xff;

        if (m_pPrimitiveFar)
        {
            RenderPrimitive(m_pPrimitiveFar, m_pShaderFar, &mat, &opaque, uiFlags);
        }
    }
}

// FUNCTION: SWBF2 0x00417a40
void GrassPatchRenderable::Update()
{
    if (!GrassPatch::IsEnabled())
    {
        return;
    }

    const float* vForward = RedRenderer::GetCamera()->_Matrix.m[2];
    for (int i = 0; i < m_pClass->mNumParticles; ++i)
    {
        const PblVector3& vPos = m_pClass->mParticle[i].mPos;
        float fKey = -(vPos.x * vForward[0] + vPos.y * vForward[1] + vPos.z * vForward[2]);
        mHeap->Push(i, fKey);
    }

    Begin();

    while (!mHeap->IsEmpty())
    {
        int iIndex = mHeap->Get(0);
        AddParticle(m_pClass->mParticle[iIndex], iIndex, m_pClass);
        mHeap->Pop();
    }

    m_pVertices = 0;
    m_pVB->Unlock();
    m_pVB->Release();
    m_pVB = 0;
}

// FUNCTION: SWBF2 0x00417bd0
void GrassPatchRenderable::Begin()
{
    if (!GrassPatch::IsEnabled())
    {
        return;
    }

    unsigned int uiNumParticles = (unsigned int)m_pClass->mNumParticles;
    unsigned int uiPrimCount = 2 * uiNumParticles;
    unsigned int uiNumVertices = 4 * uiNumParticles;

    unsigned int uiVertexFormat;
    unsigned int uiVertexStride;
    float fScale;
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_UNCOMPRESSED)
    {
        fScale = 1.0f;
        uiVertexFormat = 0x2A2;
        uiVertexStride = 36;
    }
    else
    {
        fScale = 327.67499f;
        uiVertexFormat = 0xD2A2;
        uiVertexStride = 20;
    }

    unsigned int uiStartVertex = 0;
    m_pVB = pcRedVertexBuffer::Create(uiNumVertices, uiVertexStride, false, true);
    m_pVertices = m_pVB->Lock(4 * m_pClass->mNumParticles, uiStartVertex);

    if (m_pPrimitive)
    {
        m_pPrimitive->Release();
        m_pPrimitive = 0;
    }
    if (m_pPrimitiveFar)
    {
        m_pPrimitiveFar->Release();
        m_pPrimitiveFar = 0;
    }

    m_pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLELIST, uiPrimCount, uiVertexFormat, uiNumVertices,
                                          uiStartVertex, m_pVB, m_startIndexForward, m_pIBForward);
    m_pPrimitive->m_bbox = sBox;
    m_pPrimitiveFar = pcRedPrimitive::Create(D3DPT_TRIANGLELIST, uiPrimCount, uiVertexFormat, uiNumVertices,
                                             uiStartVertex, m_pVB, m_startIndexReverse, m_pIBReverse);
    m_pPrimitiveFar->m_bbox = sBox;

    const PblMatrix& mat = RedRenderer::GetCamera()->_Matrix;

    m_vUp = PblVector3(0.0f, 1.0f, 0.0f);
    float fDot = (mat.m[2][0] * m_vUp.x + mat.m[2][1] * m_vUp.y + mat.m[2][2] * m_vUp.z) * 0.7f;
    m_vUp.x -= mat.m[2][0] * fDot;
    m_vUp.y -= mat.m[2][1] * fDot;
    m_vUp.z -= mat.m[2][2] * fDot;
    PblVector3 vNorm;
    D3DXVec3Normalize(&vNorm, &m_vUp);
    m_vUp = PblVector3(vNorm.x * fScale, vNorm.y * fScale, vNorm.z * fScale);

    PblVector3 vFront(mat.m[2][0] * fScale, mat.m[2][1] * fScale, mat.m[2][2] * fScale);
    PblVector3 vRight(mat.m[0][0] * fScale, mat.m[0][1] * fScale, mat.m[0][2] * fScale);
    for (int i = 0; i < 10; ++i)
    {

        float f = (float)(i % 5) * (1.0f - (float)(i & 1) * 2.0f) * 0.05f;
        float fRight = (1.0f - fabsf(f)) * 1.1f;
        float fFront = f * 1.1f;
        m_avRight[i] = PblVector3(vFront.x * fFront + vRight.x * fRight, vFront.y * fFront + vRight.y * fRight,
                                  vFront.z * fFront + vRight.z * fRight);
    }

    m_vFlatFront = PblVector3(-mat.m[2][0], -mat.m[2][1], -mat.m[2][2]);
    m_vFlatFront.y = 0.0f;
    if (m_vFlatFront.x == 0.0f && m_vFlatFront.z == 0.0f)
    {
        m_vFlatFront.x = 1.0f;
    }
    else
    {
        D3DXVec3Normalize(&vNorm, &m_vFlatFront);
        m_vFlatFront = vNorm;
    }
}

// FUNCTION: SWBF2 0x00418400
void GrassPatchRenderable::AddParticle(const GrassPatchClass::GrassParticle& particle, int iParticleIndex,
                                       const GrassPatchClass* pClass)
{
    if (!GrassPatch::IsEnabled())
    {
        return;
    }

    GeometryModeT eMode = RedRenderer::pcGetGeometryMode();
    float fScale = eMode == GEOMETRYMODE_UNCOMPRESSED ? 1.0f : 327.67499f;

    PblVector3 vPos(particle.mPos.x * fScale, particle.mPos.y * fScale, particle.mPos.z * fScale);
    PblVector3 vRight;
    PblVector3 vTop;
    PblVector3 vBottom;

    if (particle.mFlatType)
    {
        float fSwing = pClass->mFlatGrassSwingFlag ? particle.mOffset * 0.03f : 0.0f;
        float fFace = pClass->mFlatFaceFactor + fSwing;
        PblVector3 vDir(0.0f - m_vFlatFront.x * fFace, 1.0f - m_vFlatFront.y * fFace,
                        0.0f - m_vFlatFront.z * fFace);

        const PblVector3& u = particle.mUp;

        PblVector3 vSide(u.y * vDir.z - u.z * vDir.y, u.z * vDir.x - u.x * vDir.z, u.x * vDir.y - u.y * vDir.x);
        vSide.y = u.y + vSide.y;
        PblVector3 n1;
        D3DXVec3Normalize(&n1, &vSide);

        PblVector3 vCross(n1.y * vDir.z - n1.z * vDir.y, n1.z * vDir.x - n1.x * vDir.z,
                          n1.x * vDir.y - n1.y * vDir.x);
        PblVector3 n2;
        D3DXVec3Normalize(&n2, &vCross);

        float fSize = particle.mSize * fScale;
        vRight = PblVector3(n1.x * fSize, n1.y * fSize, n1.z * fSize);
        PblVector3 vHalfUp(n2.x * fSize * 0.5f, n2.y * fSize * 0.5f, n2.z * fSize * 0.5f);
        PblVector3 vHalfRight(vRight.x * 0.5f, vRight.y * 0.5f, vRight.z * 0.5f);

        vTop = PblVector3(vHalfUp.x + vPos.x - vHalfRight.x, vHalfUp.y + vPos.y - vHalfRight.y,
                          vHalfUp.z + vPos.z - vHalfRight.z);
        vBottom = PblVector3(vPos.x - vHalfUp.x - vHalfRight.x, vPos.y - vHalfUp.y - vHalfRight.y,
                             vPos.z - vHalfUp.z - vHalfRight.z);
    }
    else
    {
        float fUp = particle.mUpFactor;
        float fHeight = particle.mHeight;
        PblVector3 vUp(((m_vUp.x - m_vFlatFront.x) * fUp + m_vFlatFront.x) * fHeight,
                       ((m_vUp.y - m_vFlatFront.y) * fUp + m_vFlatFront.y) * fHeight,
                       ((m_vUp.z - m_vFlatFront.z) * fUp + m_vFlatFront.z) * fHeight);
        float fShift = particle.mOffset * 0.08f - 0.5f;

        const PblVector3& vRow = m_avRight[iParticleIndex % 10];
        vRight = PblVector3(vRow.x * particle.mSize, vRow.y * particle.mSize, vRow.z * particle.mSize);

        vTop = PblVector3(vUp.x + vPos.x + vRight.x * fShift, vUp.y + vPos.y + vRight.y * fShift,
                          vUp.z + vPos.z + vRight.z * fShift);
        vBottom = PblVector3(vPos.x - vRight.x * 0.5f, vPos.y - vRight.y * 0.5f, vPos.z - vRight.z * 0.5f);
    }

    float fClip = 0.0f;
    if (particle.mPos.y < 0.0f)
    {
        fClip = -(particle.mPos.y / particle.mSize);
        vBottom = PblVector3((vTop.x - vBottom.x) * fClip + vBottom.x, (vTop.y - vBottom.y) * fClip + vBottom.y,
                             (vTop.z - vBottom.z) * fClip + vBottom.z);
    }

    PblVector3 corners[4] = {
        vTop,
        PblVector3(vTop.x + vRight.x, vTop.y + vRight.y, vTop.z + vRight.z),
        PblVector3(vBottom.x + vRight.x, vBottom.y + vRight.y, vBottom.z + vRight.z),
        vBottom,
    };

    if (eMode != GEOMETRYMODE_UNCOMPRESSED)
    {
        const CompressedTexCoord* pUV = s_CompressedTexCoords[m_iTexCoordType][particle.mTexture];
        unsigned short usV = (unsigned short)pUV[0].v;
        if (fClip != 0.0f)
        {
            usV = (unsigned short)(int)(((float)pUV[2].v - (float)pUV[0].v) * fClip + (float)pUV[0].v);
        }

        CompressedTexCoord uv[4] = { pUV[2], pUV[3], { pUV[1].u, (short)usV }, { pUV[0].u, (short)usV } };

        for (int i = 0; i < 4; ++i)
        {
            GrassVertexCompressed* pv = (GrassVertexCompressed*)m_pVertices;
            pv->pos[0] = (short)(int)corners[i].x;
            pv->pos[1] = (short)(int)corners[i].y;
            pv->pos[2] = (short)(int)corners[i].z;
            pv->normal[0] = 0x7f;
            pv->normal[1] = 0xff;
            pv->normal[2] = 0x7f;
            pv->color = ToRedColor(particle.mColor);
            pv->uv = uv[i];
            m_pVertices = pv + 1;
        }
    }
    else
    {
        const TexCoord* pUV = s_TexCoords[m_iTexCoordType][particle.mTexture];
        float fV = pUV[0].v;
        if (fClip != 0.0f)
        {
            fV = (pUV[2].v - fV) * fClip + fV;
        }

        TexCoord uv[4] = { pUV[2], pUV[3], { pUV[1].u, fV }, { pUV[0].u, fV } };

        for (int i = 0; i < 4; ++i)
        {
            GrassVertexUncompressed* pv = (GrassVertexUncompressed*)m_pVertices;
            pv->pos[0] = corners[i].x;
            pv->pos[1] = corners[i].y;
            pv->pos[2] = corners[i].z;
            pv->normal[0] = 0.0f;
            pv->normal[1] = 1.0f;
            pv->normal[2] = 0.0f;
            pv->color = ToRedColor(particle.mColor);
            pv->uv = uv[i];
            m_pVertices = pv + 1;
        }
    }
}

// GLOBAL: SWBF2 0x00788290
const GrassPatchRenderable::TexCoord GrassPatchRenderable::s_TexCoords[3][6][4] = {
    {
        { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
        { { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f } },
        { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
        { { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f } },
        { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } },
        { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } },
    },
    {
        { { 0.0f, 1.0f }, { 0.5f, 1.0f }, { 0.0f, 0.5f }, { 0.5f, 0.5f } },
        { { 0.5f, 1.0f }, { 1.0f, 1.0f }, { 0.5f, 0.5f }, { 1.0f, 0.5f } },
        { { 0.0f, 0.5f }, { 0.5f, 0.5f }, { 0.0f, 0.0f }, { 0.5f, 0.0f } },
        { { 0.5f, 0.5f }, { 1.0f, 0.5f }, { 0.5f, 0.0f }, { 1.0f, 0.0f } },
        { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } },
        { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } },
    },
    {
        { { 0.0f, 0.5f }, { 0.25f, 0.5f }, { 0.0f, 0.0f }, { 0.25f, 0.0f } },
        { { 0.25f, 0.5f }, { 0.5f, 0.5f }, { 0.25f, 0.0f }, { 0.5f, 0.0f } },
        { { 0.5f, 0.5f }, { 0.75f, 0.5f }, { 0.5f, 0.0f }, { 0.75f, 0.0f } },
        { { 0.75f, 0.5f }, { 1.0f, 0.5f }, { 0.75f, 0.0f }, { 1.0f, 0.0f } },
        { { 0.0f, 0.5f }, { 0.5f, 0.5f }, { 0.0f, 1.0f }, { 0.5f, 1.0f } },
        { { 0.5f, 0.5f }, { 1.0f, 0.5f }, { 0.5f, 1.0f }, { 1.0f, 1.0f } },
    },
};

// GLOBAL: SWBF2 0x00788170
const GrassPatchRenderable::CompressedTexCoord GrassPatchRenderable::s_CompressedTexCoords[3][6][4] = {
    {
        { { 0, 2047 }, { 2047, 2047 }, { 0, 0 }, { 2047, 0 } },
        { { 2047, 2047 }, { 0, 2047 }, { 2047, 0 }, { 0, 0 } },
        { { 0, 2047 }, { 2047, 2047 }, { 0, 0 }, { 2047, 0 } },
        { { 2047, 2047 }, { 0, 2047 }, { 2047, 0 }, { 0, 0 } },
        { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
        { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    },
    {
        { { 0, 2047 }, { 1023, 2047 }, { 0, 1024 }, { 1023, 1024 } },
        { { 1024, 2047 }, { 2047, 2047 }, { 1024, 1024 }, { 2047, 1024 } },
        { { 0, 1023 }, { 1023, 1023 }, { 0, 0 }, { 1023, 0 } },
        { { 1024, 1023 }, { 2047, 1023 }, { 1024, 0 }, { 2047, 0 } },
        { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
        { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    },
    {
        { { 0, 1023 }, { 511, 1023 }, { 0, 0 }, { 511, 0 } },
        { { 512, 1023 }, { 1023, 1023 }, { 512, 0 }, { 1023, 0 } },
        { { 1024, 1023 }, { 1535, 1023 }, { 1024, 0 }, { 1535, 0 } },
        { { 1536, 1023 }, { 2047, 1023 }, { 1536, 0 }, { 2047, 0 } },
        { { 0, 1024 }, { 1023, 1024 }, { 0, 2047 }, { 1023, 2047 } },
        { { 1024, 1024 }, { 2047, 1024 }, { 1024, 2047 }, { 2047, 2047 } },
    },
};

// FUNCTION: SWBF2 0x004190f0
void GrassPatchClass::DestroyRenderData()
{
    if (mRenderData)
    {
        delete mRenderData;
        mRenderData = 0;
    }
}
