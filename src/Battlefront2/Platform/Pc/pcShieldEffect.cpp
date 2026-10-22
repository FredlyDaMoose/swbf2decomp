#include "Battlefront2/Source/ShieldEffect.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcShieldShader.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

#include <math.h>
#include "PebbleFL/Common/PblMath.h"
#include "pcSunFlare.h"

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

struct ShieldVertexC
{
    short pos[4];
    unsigned char n[4];
    short uv[2];

    void SetPos(const PblVector3& v)
    {
        pos[0] = (short)(int)((v.x - CompressedPos::s_vMin.x) * CompressedPos::s_vScaleInv.x - 32767.5f);
        pos[1] = (short)(int)((v.y - CompressedPos::s_vMin.y) * CompressedPos::s_vScaleInv.y - 32767.5f);
        pos[2] = (short)(int)((v.z - CompressedPos::s_vMin.z) * CompressedPos::s_vScaleInv.z - 32767.5f);
    }

    void SetNormal(const PblVector3& v)
    {
        n[2] = (unsigned char)(int)((v.x + 1.0f) * 127.5f);
        n[1] = (unsigned char)(int)((v.y + 1.0f) * 127.5f);
        n[0] = (unsigned char)(int)((v.z + 1.0f) * 127.5f);
    }

    void SetUV(float u, float v)
    {
        uv[0] = (short)(int)(u * 2048.0f);
        uv[1] = (short)(int)(v * 2048.0f);
    }
};

struct ShieldVertexU
{
    PblVector3 pos;
    PblVector3 n;
    float u;
    float v;

    void SetPos(const PblVector3& v) { pos = v; }
    void SetNormal(const PblVector3& v) { n = v; }
    void SetUV(float fU, float fV) { u = fU; v = fV; }
};

struct DroidekaVertexU
{
    PblVector3 pos;
    PblVector3 n;
    RedColor color;

    void SetPos(const PblVector3& v) { pos = v; }
    void SetNormal(const PblVector3& v) { n = v; }
    void SetColor(const RedColor& c) { color = c; }
};

 
 
 

// FUNCTION: SWBF2 0x004285c0
const PblBox& ShieldEffect::GetBBox() const
{
    if (!m_pClass->m_pPrim)
    {
        return *(const PblBox*)0;
    }
    return m_pClass->m_pPrim->m_bbox;
}

extern "C" PblMatrix* __stdcall D3DXMatrixScaling(PblMatrix* pOut, float fSx, float fSy, float fSz);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);

// FUNCTION: SWBF2 0x004281e0
void ShieldEffectClass::PlatformCleanup()
{
    if (m_pPrim)
    {
        m_pPrim->Release();
        m_pPrim = 0;
    }
}

// FUNCTION: SWBF2 0x004282c0
void ShieldEffect::PlatformCleanup()
{
    if (m_pShader)
    {
        m_pShader->Release();
        m_pShader = 0;
    }
}

// FUNCTION: SWBF2 0x00428310
void ShieldEffect::PlatformRender(const PblMatrix& mMatrix, unsigned char ucViewport, unsigned int uiFlags)
{
    if (!RedRenderer::pcGetRenderCaps().pipeline && !m_pClass->m_type)
    {
        PblMatrix mBillboard = RedRenderer::GetCamera()->GetMatrix();
        const PblVector3& vX = mMatrix.GetXAxis();
        const float fScale = sqrtf(vX.x * vX.x + vX.y * vX.y + vX.z * vX.z);
        PblMatrix mScale;
        D3DXMatrixMultiply(&mBillboard, &mBillboard, D3DXMatrixScaling(&mScale, fScale, fScale, fScale));
        mBillboard.m[3][0] = mMatrix.m[3][0];
        mBillboard.m[3][1] = mMatrix.m[3][1];
        mBillboard.m[3][2] = mMatrix.m[3][2];
        mBillboard.m[3][3] = 1.0f;

        const PblVector3& vCamera = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
        const float fX = vCamera.x - mMatrix.m[3][0];
        const float fY = vCamera.y - mMatrix.m[3][1];
        const float fZ = vCamera.z - mMatrix.m[3][2];
        const float fRadius = m_pClass->m_fRadius;
        m_pShader->m_fVertexMinDistSqr = 0.0f;
        m_pShader->m_fViewAngleFactor = fRadius * fRadius > fY * fY + fX * fX + fZ * fZ ? 0.2f : 0.0f;
        RedRenderer::pcRenderPrimitive(m_pClass->m_pPrim, m_pShader,
                                       mBillboard, RedColor::WHITE,
                                       uiFlags, 0, 0, 0);
        return;
    }

    const float fZoom = RedRenderer::GetCamera()->GetZoom();
    pcShieldShader::s_fFadeStart = FLRenderer::GetNearSceneFadeStart() * fZoom;
    pcShieldShader::s_fFadeEnd = FLRenderer::GetNearSceneFadeEnd() * fZoom;
    float fOffsetU = m_pClass->m_fScrollRate * m_fScrollTimer;
    fOffsetU -= (float)(int)fOffsetU;
    float fOffsetV;
    if (m_pClass->m_type == 1)
    {
        fOffsetV = 1.0f - m_fTurnOnFactor;
    }
    else
    {
        const PblVector3& vCamera = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
        const float fX = vCamera.x - mMatrix.m[3][0];
        const float fY = vCamera.y - mMatrix.m[3][1];
        const float fZ = vCamera.z - mMatrix.m[3][2];
        const float fRadius = m_pClass->m_fRadius;
        fOffsetV = 0.0f;
        if (fRadius * fRadius > fY * fY + fX * fX + fZ * fZ)
        {
            m_pShader->m_fViewAngleFactor = 0.2f;
        }
        else
        {
            m_pShader->m_fViewAngleFactor = m_pClass->m_fViewAngleFactor;
        }
        m_pShader->m_fVertexMinDistSqr = 0.0f;
    }
    m_pShader->m_fOffsetU = fOffsetU;
    m_pShader->m_fOffsetV = fOffsetV;
    RedRenderer::pcRenderPrimitive(m_pClass->m_pPrim, m_pShader,
                                   mMatrix, RedColor::WHITE,
                                   uiFlags, 0, 0, 0);
}

// FUNCTION: SWBF2 0x00427120
void ShieldEffectClass::PlatformInit()
{
    m_pPrim = 0;

    if (m_type == 0)
    {
        float afSin[33];
        float afCos[33];
        for (int i = 0; i < 33; ++i)
        {
            PblMath::SinCos(&afSin[i], &afCos[i], (float)i * (PblMath::fTwoPi / 32.0f));
        }

        if (RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED)
        {
            m_pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0xA2, sizeof(DroidekaVertexU), 13 * 33, 801, true);
        }
        else if (RedRenderer::pcGetGeometryMode())
        {
            m_pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0xD222, sizeof(ShieldVertexC), 13 * 33, 801, true);
        }
        else
        {
            m_pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x222, sizeof(ShieldVertexU), 13 * 33, 801, true);
        }

        if (RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED)
        {
            m_pPrim->m_bbox._vMin = PblVector3(-m_fRadius, -m_fRadius, -m_fHeight);
            m_pPrim->m_bbox._vMax = PblVector3(m_fRadius, m_fRadius, m_fHeight);
            CompressedPos_SetBoundingBox(m_pPrim->m_bbox);

            DroidekaVertexU* pVertex = (DroidekaVertexU*)m_pPrim->BeginVertexData();
            if (!pVertex)
            {
                return;
            }

            RedColor color;
            color.b = 255;
            color.g = 255;
            color.r = 255;
            for (int j = 0; j < 13; ++j)
            {
                float fPhi = (float)j * (PblMath::fPi / 12.0f);
                float fSin = PblMath::Sin(fPhi);
                float fCos = PblMath::Cos(fPhi);
                float fRingRadius = m_fRadius * fSin;
                float fRingHeight = m_fHeight * fCos;
                float fInvLen = 1.0f / sqrtf(m_fHeight * m_fHeight * fSin * fSin +
                                             m_fRadius * m_fRadius * fCos * fCos);
                float fNormalRadius = m_fHeight * fSin * fInvLen;
                float fNormalHeight = m_fRadius * fCos * fInvLen;
                float fFade = 1.0f - fCos * fCos;
                color.a = (unsigned char)(int)(fFade * fFade * fFade * 255.0f);
                for (int i = 0; i < 33; ++i)
                {
                    pVertex->SetPos(PblVector3(afCos[i] * fRingRadius, afSin[i] * fRingRadius, fRingHeight));
                    pVertex->SetNormal(PblVector3(afCos[i] * fNormalRadius, afSin[i] * fNormalRadius, fNormalHeight));
                    pVertex->SetColor(color);
                    ++pVertex;
                }
            }
        }
        else
        {
            m_pPrim->m_bbox._vMin = PblVector3(-m_fRadius, -m_fHeight, -m_fRadius);
            m_pPrim->m_bbox._vMax = PblVector3(m_fRadius, m_fHeight, m_fRadius);
            CompressedPos_SetBoundingBox(m_pPrim->m_bbox);

            void* pVertices = m_pPrim->BeginVertexData();
            if (!pVertices)
            {
                return;
            }

            if (RedRenderer::pcGetGeometryMode())
            {
                ShieldVertexC* pVertex = (ShieldVertexC*)pVertices;
                for (int j = 0; j < 13; ++j)
                {
                    float fPhi = (float)j * (PblMath::fPi / 12.0f);
                    float fSin = PblMath::Sin(fPhi);
                    float fCos = PblMath::Cos(fPhi);
                    float fRingRadius = m_fRadius * fSin;
                    float fRingHeight = m_fHeight * fCos;
                    float fInvLen = 1.0f / sqrtf(m_fHeight * m_fHeight * fSin * fSin +
                                                 m_fRadius * m_fRadius * fCos * fCos);
                    float fNormalRadius = m_fHeight * fSin * fInvLen;
                    float fNormalHeight = m_fRadius * fCos * fInvLen;
                    for (int i = 0; i < 33; ++i)
                    {
                        pVertex->SetPos(PblVector3(afCos[i] * fRingRadius, fRingHeight, afSin[i] * fRingRadius));
                        pVertex->SetNormal(PblVector3(afCos[i] * fNormalRadius, fNormalHeight, afSin[i] * fNormalRadius));
                        pVertex->SetUV((float)i * 2.0f / 32.0f, (float)j / 12.0f);
                        ++pVertex;
                    }
                }
            }
            else
            {
                ShieldVertexU* pVertex = (ShieldVertexU*)pVertices;
                for (int j = 0; j < 13; ++j)
                {
                    float fPhi = (float)j * (PblMath::fPi / 12.0f);
                    float fSin = PblMath::Sin(fPhi);
                    float fCos = PblMath::Cos(fPhi);
                    float fRingRadius = m_fRadius * fSin;
                    float fRingHeight = m_fHeight * fCos;
                    float fInvLen = 1.0f / sqrtf(m_fHeight * m_fHeight * fSin * fSin +
                                                 m_fRadius * m_fRadius * fCos * fCos);
                    float fNormalRadius = m_fHeight * fSin * fInvLen;
                    float fNormalHeight = m_fRadius * fCos * fInvLen;
                    for (int i = 0; i < 33; ++i)
                    {
                        pVertex->SetPos(PblVector3(afCos[i] * fRingRadius, fRingHeight, afSin[i] * fRingRadius));
                        pVertex->SetNormal(PblVector3(afCos[i] * fNormalRadius, fNormalHeight, afSin[i] * fNormalRadius));
                        pVertex->SetUV((float)i * 2.0f / 32.0f, (float)j / 12.0f);
                        ++pVertex;
                    }
                }
            }
        }

        m_pPrim->EndVertexData();

        unsigned short* pIndex = m_pPrim->BeginIndexData();
        for (unsigned short k = 0; k < 32; ++k)
        {
            if ((k & 1) == 0)
            {
                for (unsigned short j = 0; j < 13; ++j)
                {
                    *pIndex++ = k + j * 33;
                    *pIndex++ = k + 1 + j * 33;
                }
            }
            else
            {
                for (unsigned short j = 12; j > 0; --j)
                {
                    *pIndex++ = k + 1 + j * 33;
                    *pIndex++ = k + (j - 1) * 33;
                }
                if (k == 31)
                {
                    *pIndex++ = k + 1;
                }
            }
        }
        m_pPrim->EndIndexData();
        return;
    }

    const float fStep = PblMath::fHalfPi / 12.0f;

    float afSin[33];
    float afCos[33];
    float afWave[33];
    for (int i = 0; i < 33; ++i)
    {
        float fAngle = (float)i * (PblMath::fTwoPi / 32.0f);
        PblMath::SinCos(&afSin[i], &afCos[i], fAngle);
        afWave[i] = PblMath::Cos(fAngle * 2.0f + PblMath::fTwoPi * 0.73f) * 0.3f
                  + PblMath::Cos(PblMath::fTwoPi * 0.21f + fAngle) * 0.4f
                  + PblMath::Cos(fAngle * 3.0f + PblMath::fTwoPi * 0.57f) * 0.2f
                  + PblMath::Cos(fAngle * 5.0f + PblMath::fTwoPi * 0.07f) * 0.1f;
    }

    if (RedRenderer::pcGetGeometryMode())
    {
        m_pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0xD222, sizeof(ShieldVertexC), 16 * 33, 1023, true);
    }
    else
    {
        m_pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x222, sizeof(ShieldVertexU), 16 * 33, 1023, true);
    }

    m_pPrim->m_bbox._vMin = PblVector3(m_fRadius * -1.01f,
                                       PblMath::Sin(fStep * 3.0f) * m_fHeight * -1.01f,
                                       m_fRadius * -1.01f);
    m_pPrim->m_bbox._vMax = PblVector3(m_fRadius * 1.01f, m_fHeight * 1.01f, m_fRadius * 1.01f);
    CompressedPos_SetBoundingBox(m_pPrim->m_bbox);

    void* pVertices = m_pPrim->BeginVertexData();
    if (!pVertices)
    {
        return;
    }

    if (RedRenderer::pcGetGeometryMode())
    {
        ShieldVertexC* pVertex = (ShieldVertexC*)pVertices;
        int iRing = -3;
        for (unsigned short j = 0; j <= 15; ++j, ++iRing)
        {
            float fPhi = (float)iRing * fStep;
            float fCos = PblMath::Cos(fPhi);
            float fSin = PblMath::Sin(fPhi);
            float fRingRadius = fCos * m_fRadius;
            float fRingHeight = m_fHeight * fSin;
            float fInvLen = 1.0f / sqrtf(m_fHeight * m_fHeight * fCos * fCos +
                                         m_fRadius * m_fRadius * fSin * fSin);
            float fNormalRadius = m_fHeight * fCos * fInvLen;
            float fNormalHeight = fSin * m_fRadius * fInvLen;
            float fWaveScale = j < 15 ? 0.08f : 0.0f;
            float fV = (float)(iRing + 3) / 15.0f;
            for (int i = 0; i < 33; ++i)
            {
                pVertex->SetPos(PblVector3(afCos[i] * fRingRadius, fRingHeight, afSin[i] * fRingRadius));
                pVertex->SetNormal(PblVector3(afCos[i] * fNormalRadius, fNormalHeight, afSin[i] * fNormalRadius));
                pVertex->SetUV((float)i * 4.0f / 32.0f, afWave[i] * fWaveScale + 1.0f - fV);
                ++pVertex;
            }
        }
    }
    else
    {
        ShieldVertexU* pVertex = (ShieldVertexU*)pVertices;
        int iRing = -3;
        for (unsigned short j = 0; j <= 15; ++j, ++iRing)
        {
            float fPhi = (float)iRing * fStep;
            float fCos = PblMath::Cos(fPhi);
            float fSin = PblMath::Sin(fPhi);
            float fRingRadius = fCos * m_fRadius;
            float fRingHeight = fSin * m_fHeight;
            float fInvLen = 1.0f / sqrtf(m_fHeight * m_fHeight * fCos * fCos +
                                         m_fRadius * m_fRadius * fSin * fSin);
            float fNormalRadius = fCos * m_fHeight * fInvLen;
            float fNormalHeight = fSin * m_fRadius * fInvLen;
            float fWaveScale = j < 15 ? 0.08f : 0.0f;
            float fV = (float)(iRing + 3) / 15.0f;
            for (int i = 0; i < 33; ++i)
            {
                pVertex->SetPos(PblVector3(fRingRadius * afCos[i], fRingHeight, fRingRadius * afSin[i]));
                pVertex->SetNormal(PblVector3(fNormalRadius * afCos[i], fNormalHeight, fNormalRadius * afSin[i]));
                pVertex->SetUV((float)i * 4.0f / 32.0f, afWave[i] * fWaveScale + 1.0f - fV);
                ++pVertex;
            }
        }
    }

    m_pPrim->EndVertexData();

    unsigned short* pIndex = m_pPrim->BeginIndexData();
    for (unsigned short k = 0; k < 32; ++k)
    {
        if ((k & 1) == 0)
        {
            for (unsigned short j = 0; j < 16; ++j)
            {
                *pIndex++ = k + j * 33;
                *pIndex++ = k + 1 + j * 33;
            }
        }
        else
        {
            for (unsigned short j = 15; j > 0; --j)
            {
                *pIndex++ = k + 1 + j * 33;
                *pIndex++ = k + (j - 1) * 33;
            }
            *pIndex++ = k + 1;
            if (k != 31)
            {
                *pIndex++ = k + 1;
            }
        }
    }
    m_pPrim->EndIndexData();
}

// FUNCTION: SWBF2 0x00428220
void ShieldEffect::PlatformInit()
{
    pcRedShader* pShader;
    if (RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED && m_pClass->m_type == 0)
    {
        pShader = pcRedShader::Create(0x337519B0, 0xA2, &m_pClass->m_uiTextureHash, 1, 0x100C0,
                                      m_pClass->m_color, m_pClass->m_colorSpecular, 32.0f);
    }
    else if (RedRenderer::pcGetGeometryMode())
    {
        pShader = pcRedShader::Create(0x337519B0, 0xD222, &m_pClass->m_uiTextureHash, 1, 0x100C4,
                                      m_pClass->m_color, m_pClass->m_colorSpecular, 32.0f);
    }
    else
    {
        pShader = pcRedShader::Create(0x337519B0, 0x222, &m_pClass->m_uiTextureHash, 1, 0x100C4,
                                      m_pClass->m_color, m_pClass->m_colorSpecular, 32.0f);
    }

    m_pShader = (pcShieldShader*)pShader;
    m_pShader->m_fViewAngleFactor = m_pClass->m_fViewAngleFactor;
    m_pShader->m_fVertexMinDistSqr = 0.0f;
}
