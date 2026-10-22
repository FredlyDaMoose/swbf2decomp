#include "Battlefront2/Source/Godray.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcParticleShader.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "pcSunFlare.h"

// FUNCTION: SWBF2 0x00415cf0
void GodrayManager::PlatformInit()
{
    if (m_pShader)
    {
        m_pShader->Release();
    }

    m_pShader = pcRedShader::Create(
        PblHash("Particle"),
        RedRenderer::pcGetGeometryMode() != GEOMETRYMODE_UNCOMPRESSED ? 0x9282 : 0x282,
        &m_uiTextureHash, 1, 0x10044, RedColor::WHITE, RedColor::BLACK, 32.0f);
}

// FUNCTION: SWBF2 0x00415d70
void GodrayManager::PlatformCleanup()
{
    if (m_pShader)
    {
        m_pShader->Release();
        m_pShader = 0;
    }
}

static const PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x00796510

static const unsigned short sRayStripIndices[20] =
{
    5, 1, 4, 0, 3, 2, 2, 6, 3, 7, 4, 8, 5, 9, 9, 8, 11, 7, 10, 6
};

namespace
{

struct CompressedTexCoord
{
    short u, v;
};

struct GodrayVertex
{
    struct VertexC
    {
        CompressedPos pos;
        RedColor color;
        CompressedTexCoord uv;

        void SetPos(const PblVector3& v) { pos.SetPos(v.x, v.y, v.z); }
        void SetColor(const RedColor& c) { color = c; }
        void SetUV(float fU, float fV)
        {
            uv.u = (short)(int)(fU * 2048.0f);
            uv.v = (short)(int)(fV * 2048.0f);
        }
    };

    struct VertexU
    {
        PblVector3 pos;
        RedColor color;
        float u;
        float v;

        void SetPos(const PblVector3& vPos) { pos = vPos; }
        void SetColor(const RedColor& c) { color = c; }
        void SetUV(float fU, float fV) { u = fU; v = fV; }
    };
};
 
 

inline PblVector3 Add(const PblVector3& a, const PblVector3& b)
{
    return PblVector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

template <class VertexT>
VertexT* WriteRayVertices(VertexT* pVertex,
                          const PblVector3& vStart, const PblVector3& vStartIn,
                          const PblVector3& vEndIn, const PblVector3& vEnd,
                          const PblVector3& vA, const PblVector3& vB,
                          const PblVector3& vC, const PblVector3& vD,
                          const RedColor& edgeColor, const RedColor& rayColor,
                          const float afU[4], const float afV[4])
{
    pVertex[0].SetPos(Add(vStart, vB));    pVertex[0].SetColor(edgeColor);  pVertex[0].SetUV(afU[0], afV[1]);
    pVertex[1].SetPos(Add(vStart, vC));    pVertex[1].SetColor(edgeColor);  pVertex[1].SetUV(afU[0], afV[2]);
    pVertex[2].SetPos(Add(vStartIn, vA));  pVertex[2].SetColor(edgeColor);  pVertex[2].SetUV(afU[1], afV[0]);
    pVertex[3].SetPos(Add(vStartIn, vB));  pVertex[3].SetColor(rayColor);   pVertex[3].SetUV(afU[1], afV[1]);
    pVertex[4].SetPos(Add(vStartIn, vC));  pVertex[4].SetColor(rayColor);   pVertex[4].SetUV(afU[1], afV[2]);
    pVertex[5].SetPos(Add(vStartIn, vD));  pVertex[5].SetColor(edgeColor);  pVertex[5].SetUV(afU[1], afV[3]);
    pVertex[6].SetPos(Add(vEndIn, vA));    pVertex[6].SetColor(edgeColor);  pVertex[6].SetUV(afU[2], afV[0]);
    pVertex[7].SetPos(Add(vEndIn, vB));    pVertex[7].SetColor(rayColor);   pVertex[7].SetUV(afU[2], afV[1]);
    pVertex[8].SetPos(Add(vEndIn, vC));    pVertex[8].SetColor(rayColor);   pVertex[8].SetUV(afU[2], afV[2]);
    pVertex[9].SetPos(Add(vEndIn, vD));    pVertex[9].SetColor(edgeColor);  pVertex[9].SetUV(afU[2], afV[3]);
    pVertex[10].SetPos(Add(vEnd, vB));     pVertex[10].SetColor(edgeColor); pVertex[10].SetUV(afU[3], afV[1]);
    pVertex[11].SetPos(Add(vEnd, vC));     pVertex[11].SetColor(edgeColor); pVertex[11].SetUV(afU[3], afV[2]);
    return pVertex + 12;
}
}

// FUNCTION: SWBF2 0x00415dc0
void GodrayManager::PlatformRender(unsigned int uiFlags) const
{
    if (uiFlags & 0x200000)
    {
        return;
    }
    if (!m_uiNumVisibleGodrays)
    {
        return;
    }

    unsigned int uiNumVertices = 0;
    unsigned int uiNumIndices = 0;
    unsigned int i;
    for (i = 0; i < m_uiNumVisibleGodrays; i++)
    {
        if (m_aVisibleGodrays[i].fAlpha > 0.0f)
        {
            uiNumVertices += 12 * m_aVisibleGodrays[i].uiNumRays;
            uiNumIndices += 22 * m_aVisibleGodrays[i].uiNumRays;
        }
    }
    if (uiNumIndices <= 2)
    {
        return;
    }
    uiNumIndices -= 2;

    const PblVector3& vEye = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    const float fBoxRadius = m_fMaxViewRadius + m_fMaxGodrayLength;
    const PblBox box(vEye.x - fBoxRadius, vEye.y - fBoxRadius, vEye.z - fBoxRadius,
                     vEye.x + fBoxRadius, vEye.y + fBoxRadius, vEye.z + fBoxRadius);

    pcRedPrimitive* pPrimitive;
    if (RedRenderer::pcGetGeometryMode())
    {
        pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x9282, sizeof(GodrayVertex::VertexC),
                                            uiNumVertices, uiNumIndices, false);
        CompressedPos::SetBoundingBox(&box);
    }
    else
    {
        pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x282, sizeof(GodrayVertex::VertexU),
                                            uiNumVertices, uiNumIndices, false);
    }
    pPrimitive->m_bbox = box;
    unsigned short* pIndices = pPrimitive->BeginIndexData();
    void* pVertexData = pPrimitive->BeginVertexData();
    if (!pVertexData)
    {
        return;
    }

    unsigned short usBaseVertex = 0;
    const float fTexScale = 8.0f / (m_fTexScaleL * m_fMaxGodrayLength);
    ((pcParticleShader*)m_pShader)->SetTextureScale(1.0f / fTexScale);

    for (i = 0; i < m_uiNumVisibleGodrays; i++)
    {
        const VisibleGodray* pVisible = &m_aVisibleGodrays[i];
        if (!(pVisible->fAlpha > 0.0f))
        {
            continue;
        }

        const Godray* pGodray = pVisible->pGodray;
        const float fMaxRadius = pVisible->fLengthClipped * 0.33f;
        float fAlphaScale = 1.0f;
        float fRadius = pGodray->fRadius * 0.5f;
        if (fRadius > fMaxRadius)
        {
            fAlphaScale = fMaxRadius / fRadius;
            fRadius = fMaxRadius;
        }

        RedColor edgeColor = pGodray->color;
        edgeColor.a = 0;
        const PblVector3 vLength(m_vLengthDir.x * fRadius, m_vLengthDir.y * fRadius,
                                 m_vLengthDir.z * fRadius);

        for (unsigned int uiRay = 0; uiRay < pVisible->uiNumRays; uiRay++)
        {
            const VisibleGodray::Ray& ray = pVisible->aRays[uiRay];

            const float fOffset = ray.fPosZ * m_vWidthDir.z + ray.fPosX * m_vWidthDir.x;
            const PblVector3 vWidth(ray.fRadius * m_vWidthDir.x, m_vWidthDir.y * ray.fRadius,
                                    m_vWidthDir.z * ray.fRadius);
            const float fHalf = ray.fRadius * 0.5f;
            const PblVector3 vHalf(m_vWidthDir.x * fHalf, m_vWidthDir.y * fHalf, m_vWidthDir.z * fHalf);
            const PblVector3 vOffset(m_vWidthDir.x * fOffset, fOffset * m_vWidthDir.y,
                                     fOffset * m_vWidthDir.z);

            const float fStartL = pVisible->bStartClipped
                                      ? (pGodray->fLength - pVisible->fLengthClipped) * m_fTexScaleL
                                      : 0.0f;
            float fL = ray.fTexOffsetL0 + m_fTexOffsetL + fStartL;
            float fW = ray.fRadius0 * m_fTexOffsetAngle * 0.5f * m_fTexScaleW
                     + (ray.fTexOffsetW0 + m_fTexOffsetW) + m_fTexScaleW * fOffset;
            fL -= (float)(int)(fL + 8.0f);
            fW -= (float)(int)(fW + 8.0f);

            const PblVector3 vStartIn(pVisible->vStartPosClipped.x + vLength.x,
                                      pVisible->vStartPosClipped.y + vLength.y,
                                      pVisible->vStartPosClipped.z + vLength.z);
            const PblVector3 vEndIn(pVisible->vEndPosClipped.x - vLength.x,
                                    pVisible->vEndPosClipped.y - vLength.y,
                                    pVisible->vEndPosClipped.z - vLength.z);
            const PblVector3 vA(vOffset.x - vWidth.x, vOffset.y - vWidth.y, vOffset.z - vWidth.z);
            const PblVector3 vB(vOffset.x - vHalf.x, vOffset.y - vHalf.y, vOffset.z - vHalf.z);
            const PblVector3 vC(vOffset.x + vHalf.x, vOffset.y + vHalf.y, vOffset.z + vHalf.z);
            const PblVector3 vD(vOffset.x + vWidth.x, vOffset.y + vWidth.y, vOffset.z + vWidth.z);

            const float fWidthW = m_fTexScaleW * ray.fRadius;
            float afU[4];
            afU[0] = fL * fTexScale;
            afU[1] = (m_fTexScaleL * fRadius + fL) * fTexScale;
            afU[2] = ((pVisible->fLengthClipped - fRadius) * m_fTexScaleL + fL) * fTexScale;
            afU[3] = (pVisible->fLengthClipped * m_fTexScaleL + fL) * fTexScale;
            float afV[4];
            afV[0] = (fWidthW + fW) * fTexScale;
            afV[1] = (fWidthW * 0.5f + fW) * fTexScale;
            afV[2] = (fW - fWidthW * 0.5f) * fTexScale;
            afV[3] = (fW - fWidthW) * fTexScale;

            RedColor rayColor = pGodray->color;
            rayColor.a = (unsigned char)(int)(ray.fAlpha * pVisible->fAlpha * fAlphaScale * 255.0f);

            if (RedRenderer::pcGetGeometryMode())
            {
                pVertexData = WriteRayVertices((GodrayVertex::VertexC*)pVertexData,
                                               pVisible->vStartPosClipped, vStartIn, vEndIn,
                                               pVisible->vEndPosClipped, vA, vB, vC, vD,
                                               edgeColor, rayColor, afU, afV);
            }
            else
            {
                pVertexData = WriteRayVertices((GodrayVertex::VertexU*)pVertexData,
                                               pVisible->vStartPosClipped, vStartIn, vEndIn,
                                               pVisible->vEndPosClipped, vA, vB, vC, vD,
                                               edgeColor, rayColor, afU, afV);
            }

            if (usBaseVertex)
            {
                pIndices[0] = pIndices[-1];
                pIndices[1] = usBaseVertex + 5;
                pIndices += 2;
            }
            for (unsigned int uiIndex = 0; uiIndex < 20; uiIndex++)
            {
                *pIndices++ = usBaseVertex + sRayStripIndices[uiIndex];
            }
            usBaseVertex += 12;
        }
    }

    pPrimitive->EndIndexData();
    pPrimitive->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrimitive, m_pShader,
                                   _IdentityMatrix,
                                   m_color, 0x1000010, 0, 0, 0);
    pPrimitive->Release();
}
