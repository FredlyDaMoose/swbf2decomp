#include <math.h>
#include <d3dx9math.h>

#include "Battlefront2/Source/LightBeam.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRef.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "pcSunFlare.h"
#include "pcLightBeam.h"

// GLOBAL: SWBF2 0x01f9ae60
unsigned short s_auiBeamIndices[38];
// GLOBAL: SWBF2 0x01f9af40
float s_afBeamAlpha[13];

// GLOBAL: SWBF2 0x01eaff80
// SYNTHETIC: SWBF2 0x004020a0
// `dynamic initializer for 's_avBeamRadius''

PblVector3 s_avBeamRadius[6];
// GLOBAL: SWBF2 0x01f9aeb0
unsigned short s_auiHaloIndices[66];
// GLOBAL: SWBF2 0x01eafe00
// SYNTHETIC: SWBF2 0x004020c0
// `dynamic initializer for 's_avHaloRadius''

PblVector3 s_avHaloRadius[32];

// FUNCTION: SWBF2 0x0041fa70
static bool InitStaticData()
{
    unsigned short* pIndex = s_auiBeamIndices;
    unsigned short i;

    *pIndex++ = 8;
    *pIndex++ = 7;
    for (i = 0; i < 3; i++)
    {
        *pIndex++ = 2 - i;
        *pIndex++ = 6 - i;
    }
    *pIndex++ = 3;
    *pIndex++ = 3;
    *pIndex++ = 9;
    *pIndex++ = 9;
    for (i = 10; i < 16; i++)
    {
        *pIndex++ = i - 7;
        *pIndex++ = i;
    }
    *pIndex++ = 15;
    *pIndex++ = 15;
    for (i = 0; i < 6; i++)
    {
        *pIndex++ = 21 - i;
        *pIndex++ = 14 - i;
    }

    s_afBeamAlpha[0] = 0.0f;
    for (int iRing = 0; iRing < 6; iRing++)
    {
        const float f = (float)iRing;

        float x = (f + 0.5f) * (1.0f / 3.0f) - 1.0f;
        if (x < 0.001f)
        {
            x -= 1.0f / 18.0f;
        }
        else if (x > 0.001f)
        {
            x += 1.0f / 18.0f;
        }
        s_avBeamRadius[iRing] = PblVector3(x, (float)sqrt((double)(1.0f - x * x)), 0.0f);

        float a = f * 0.4f - 1.0f;
        s_afBeamAlpha[2 * iRing + 1] = (float)sqrt((double)(1.0f - a * a));
        a = (f + 1.0f) * (1.0f / 3.0f) - 1.0f;
        s_afBeamAlpha[2 * iRing + 2] = (float)sqrt((double)(1.0f - a * a));
    }

    pIndex = s_auiHaloIndices;
    *pIndex++ = 1;
    *pIndex++ = 2;
    *pIndex++ = 0;
    for (i = 3; i < 17; i += 2)
    {
        *pIndex++ = i;
        *pIndex++ = i;
        *pIndex++ = i + 1;
        *pIndex++ = 0;
    }
    *pIndex++ = 1;
    for (i = 17; i < 33; i++)
    {
        *pIndex++ = i - 16;
        *pIndex++ = i;
    }
    *pIndex++ = 1;
    *pIndex++ = 17;

    const float fStep = PblMath::fTwoPi * (1.0f / 16.0f);
    for (int iSide = 0; iSide < 16; iSide++)
    {
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, (float)iSide * fStep);
        s_avHaloRadius[iSide] = PblVector3(-fSin, fCos, 0.0f);
        PblMath::SinCos(&fSin, &fCos, ((float)iSide + 0.5f) * fStep);
        s_avHaloRadius[16 + iSide] = PblVector3(-fSin, fCos, 0.0f);
    }

    return true;
}

// FUNCTION: SWBF2 0x0041fdb0
void LightBeamEffect::_PlatformInit()
{
    InitStaticData();

    if (m_pShader)
    {
        m_pShader->Release();
    }

    m_pShader = pcRedShader::Create(
        PblHash("LightBeam"),
        RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED ? 0x1082 : 0x82,
        0, 0, 0x10040, RedColor::WHITE, RedColor::BLACK, 32.0f);
}

// FUNCTION: SWBF2 0x0040cbe0
void __cdecl CompressedPos::SetBoundingBox(const PblBox* pBox)
{
    PblVector3 vMin = { pBox->_vMin.x, pBox->_vMin.y, pBox->_vMin.z };
    PblVector3 vExtent = { pBox->_vMax.x - pBox->_vMin.x, pBox->_vMax.y - pBox->_vMin.y,
                                pBox->_vMax.z - pBox->_vMin.z };
    CompressedPos::s_vMin = vMin;
    CompressedPos::s_vScale = vExtent * (1.0f / 65535.0f);
    CompressedPos::s_vScaleInv.x = 1.0f / CompressedPos::s_vScale.x;
    CompressedPos::s_vScaleInv.y = 1.0f / CompressedPos::s_vScale.y;
    CompressedPos::s_vScaleInv.z = 1.0f / CompressedPos::s_vScale.z;
}

// FUNCTION: SWBF2 0x00410ba0
void VertexC::SetPos(const PblVector3& v)
{
    pos.x = (short)((v.x - CompressedPos::s_vMin.x) * CompressedPos::s_vScaleInv.x - 32767.5f);
    pos.y = (short)((v.y - CompressedPos::s_vMin.y) * CompressedPos::s_vScaleInv.y - 32767.5f);
    pos.z = (short)((v.z - CompressedPos::s_vMin.z) * CompressedPos::s_vScaleInv.z - 32767.5f);
}

// FUNCTION: SWBF2 0x004171f0
PblVector3* __cdecl CrossProduct(PblVector3* pResult, const PblVector3* pA, const PblVector3* pB)
{
    float x = pA->y * pB->z - pA->z * pB->y;
    float y = pA->z * pB->x - pA->x * pB->z;
    float z = pA->x * pB->y - pA->y * pB->x;
    pResult->x = x;
    pResult->y = y;
    pResult->z = z;
    return pResult;
}

namespace
{

struct VertexU
{
    PblVector3 pos;
    RedColor color;

    void SetPos(const PblVector3& v) { pos = v; }
    void SetColor(const RedColor& c) { color = c; }
};
 
}

// FUNCTION: SWBF2 0x0041fe80
void LightBeamEffect::RenderLightBeam(const LightBeam* pBeam, unsigned int uiFlags) const
{
    float fNear, fFar, fWidth, fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
    const PblMatrix& mCameraInverse = RedRenderer::GetCamera()->GetMatrixInverse();
    const PblVector3& vEye = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    const unsigned int uiViewport = FLRenderer::GetCurrentViewport();

    if (pBeam->_color.a == 0)
    {
        return;
    }
    if (const_cast<LightBeam*>(pBeam)->IsHiddenInView(uiViewport))
    {
        return;
    }

    PblVector3 vHead;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vHead, (const D3DXVECTOR3*)&pBeam->_vHeadPosition,
                           (const D3DXMATRIX*)&pBeam->m_mWorldMatrix);
    const float fDistSq = (vHead.x - vEye.x) * (vHead.x - vEye.x)
                        + (vHead.y - vEye.y) * (vHead.y - vEye.y)
                        + (vHead.z - vEye.z) * (vHead.z - vEye.z);
    if (pBeam->_fSquaredDrawDistance > 0.0f && fDistSq >= pBeam->_fSquaredDrawDistance)
    {
        return;
    }

    PblVector3 vView;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vView, (const D3DXVECTOR3*)&vHead,
                           (const D3DXMATRIX*)&mCameraInverse);

    float fRadius;
    if (pBeam->_fLength > 0.0f)
    {
        const float fLength = pBeam->_fLength + pBeam->_fFadeLength;
        float fSize = pBeam->_fTailSizeZ;
        if (pBeam->_fTailSizeX > fSize)
        {
            fSize = pBeam->_fTailSizeX;
        }
        fSize *= 2.0f;
        if (fLength > fSize)
        {
            fRadius = fSize * 0.5f * fSize / fLength + fLength;
        }
        else
        {
            fRadius = fLength * 0.5f * fLength / fSize + fSize;
        }
        if (fNear >= fRadius - vView.z)
        {
            return;
        }
    }
    else
    {
        fRadius = pBeam->_fTailSizeX + pBeam->_fFadeLength;
        if (fNear >= -vView.z)
        {
            return;
        }
    }
    if (-vView.z - fRadius >= fFar)
    {
        return;
    }
    if (((float)fabs(vView.x) - fRadius) * fNear * 2.0f > -(vView.z * fWidth))
    {
        return;
    }
    if (((float)fabs(vView.y) - fRadius) * fNear * 2.0f > -(vView.z * fHeight))
    {
        return;
    }

    LensFlare& flare = const_cast<LensFlare&>(pBeam->_Flare);

    if (pBeam->_fLength > 0.0f)
    {

        __declspec(align(16)) PblMatrix mBeam;
        PblVector3 vDir;
        D3DXVec3TransformNormal((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&pBeam->_vDirection,
                                (const D3DXMATRIX*)&pBeam->m_mWorldMatrix);
        mBeam.m[1][0] = vDir.x;
        mBeam.m[1][1] = vDir.y;
        mBeam.m[1][2] = vDir.z;
        mBeam.m[1][3] = 0.0f;

        const PblVector3& vCamPos = mCamera.GetTranslation();
        PblVector3 vSide(vHead.x - vCamPos.x, vHead.y - vCamPos.y, vHead.z - vCamPos.z);
        PblVector3 vCross;
        D3DXVec3Normalize((D3DXVECTOR3*)&vSide,
                          (const D3DXVECTOR3*)CrossProduct(&vCross, &vSide, &vDir));
        mBeam.m[0][0] = vSide.x;
        mBeam.m[0][1] = vSide.y;
        mBeam.m[0][2] = vSide.z;
        mBeam.m[0][3] = 0.0f;

        const PblVector3* pFront = CrossProduct(&vCross, (const PblVector3*)mBeam.m[1],
                                              (const PblVector3*)mBeam.m[0]);
        mBeam.m[2][0] = pFront->x;
        mBeam.m[2][1] = pFront->y;
        mBeam.m[2][2] = pFront->z;
        mBeam.m[2][3] = 0.0f;
        mBeam.m[3][0] = vHead.x;
        mBeam.m[3][1] = vHead.y;
        mBeam.m[3][2] = vHead.z;
        mBeam.m[3][3] = 1.0f;

        float fHeadSize = pBeam->_fHeadSizeX;
        float fTailSize = pBeam->_fTailSizeX;
        if (pBeam->_fHeadSizeX != pBeam->_fHeadSizeZ || pBeam->_fTailSizeX != pBeam->_fTailSizeZ)
        {
            const PblMatrix& mWorld = pBeam->m_mWorldMatrix;
            const float fAcrossX = (float)fabs(mWorld.m[0][0] * vSide.x + mWorld.m[0][1] * vSide.y
                                               + mWorld.m[0][2] * vSide.z);
            const float fAcrossZ = (float)fabs(mWorld.m[2][0] * vSide.x + mWorld.m[2][1] * vSide.y
                                               + mWorld.m[2][2] * vSide.z);
            if (pBeam->_fHeadSizeX != pBeam->_fHeadSizeZ)
            {
                fHeadSize = (float)sqrt((double)(pBeam->_fHeadSizeX * pBeam->_fHeadSizeX * fAcrossX * fAcrossX
                                                 + pBeam->_fHeadSizeZ * pBeam->_fHeadSizeZ * fAcrossZ * fAcrossZ));
            }
            if (pBeam->_fTailSizeX != pBeam->_fTailSizeZ)
            {
                fTailSize = (float)sqrt((double)(pBeam->_fTailSizeX * pBeam->_fTailSizeX * fAcrossX * fAcrossX
                                                 + pBeam->_fTailSizeZ * pBeam->_fTailSizeZ * fAcrossZ * fAcrossZ));
            }
        }

        const float fAlong = vDir.x * (vCamPos.x - vHead.x) + vDir.y * (vCamPos.y - vHead.y)
                           + vDir.z * (vCamPos.z - vHead.z);
        const float fAlongT = fAlong / pBeam->_fLength;
        const float fOffSq = fDistSq - fAlong * fAlong;
        const float fOffDist = (float)sqrt((double)fOffSq) - fNear;
        const float fDist = (float)sqrt((double)fDistSq);
        const float fCos = fDist > fHeadSize ? fAlong / fDist : fAlong / fHeadSize;

        float fWidthAtEye;
        if (fAlong > 0.0f && fHeadSize <= fTailSize)
        {
            fWidthAtEye = (fTailSize - fHeadSize) * fAlongT + fHeadSize;
        }
        else
        {
            fWidthAtEye = fHeadSize;
        }

        RedColor flareColor = pBeam->_color;
        float fFlareSize = fHeadSize * 0.5f;
        flareColor.a = 0;
        if (fCos > 0.0f && pBeam->_fFlareIntensity > 0.0f)
        {
            float fConeCos = pBeam->_fFlareCosAngle;
            const float fConeStart = fConeCos > 0.5f ? fConeCos * 0.5f : 0.0f;
            if (!(fConeCos > 0.1f))
            {
                fConeCos = 0.1f;
            }
            if (fConeStart < fCos)
            {
                float fFlare;
                if (fConeCos >= fCos)
                {
                    const float t = (fCos - fConeStart) / (fConeCos - fConeStart);
                    fFlare = t * 0.5f * t;
                }
                else
                {
                    const float t = (1.0f - fCos) / (1.0f - fConeCos);
                    fFlare = 1.0f - t * 0.5f * t;
                }
                const float fFlareRange = (pBeam->_fLength + pBeam->_fFadeLength) * 2.0f;
                if (fDist > fFlareRange)
                {
                    fFlare = fFlareRange / fDist * fFlare;
                }
                flareColor.a = (unsigned char)PblMath::Round(pBeam->_color.a * fFlare * pBeam->_fFlareIntensity);
                if (fTailSize > fHeadSize)
                {
                    const float fOff = fOffSq / (fWidthAtEye * fWidthAtEye);
                    if (fOff < 4.0f)
                    {
                        fFlareSize = (1.0f - fOff * 0.25f) * ((fTailSize - fHeadSize) * 0.5f) + fHeadSize * 0.5f;
                    }
                }
            }
        }
        flare.SetColor(flareColor);
        flare.SetSize(fFlareSize);

        float fIntensity = (1.0f - fCos * fCos) * pBeam->_fBeamIntensity;
        if (fOffDist < 0.0f)
        {
            fIntensity = 0.0f;
        }
        else if (fWidthAtEye > fOffDist)
        {
            fIntensity *= fOffDist / fWidthAtEye;
        }
        if (pBeam->_fSquaredDrawDistance > 0.0f && fDistSq > pBeam->_fSquaredDrawDistance * (0.9f * 0.9f))
        {
            fIntensity *= (1.0f - fDistSq / pBeam->_fSquaredDrawDistance) * (1.0f / (1.0f - 0.9f * 0.9f));
        }
        if (pBeam->_color.a * fIntensity <= 0.5f / 255.0f)
        {
            return;
        }

        RedColor edgeColor = pBeam->_color;
        edgeColor.a = 0;
        const float fFadeWidth = (float)fabs(fCos) * (fTailSize - pBeam->_fFadeLength) + pBeam->_fFadeLength;
        const float fFadeIntensity = pBeam->_fFadeAmount * fIntensity;
        const float fMaxSize = (fTailSize > fHeadSize ? fTailSize : fHeadSize) * 1.01f;
        const PblBox box(-fMaxSize, -0.01f, -1.0f,
                         fMaxSize, (pBeam->_fLength + fFadeWidth) * 1.01f, 1.0f);

        pcRedPrimitive* pPrimitive;
        if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
        {
            pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x1082, sizeof(VertexC), 22, 38, false);
            CompressedPos::SetBoundingBox(&box);
        }
        else
        {
            pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x82, sizeof(VertexU), 22, 38, false);
        }
        pPrimitive->m_bbox = box;
        unsigned short* pIndices = pPrimitive->BeginIndexData();
        void* pVertexData = pPrimitive->BeginVertexData();
        if (!pVertexData)
        {
            return;
        }

        int i;
        if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
        {
            VertexC* pVertex = (VertexC*)pVertexData;
            for (i = 0; i < 3; i++, pVertex++)
            {
                pVertex->SetPos(PblVector3(((float)i * 0.59999973f - 0.6f) * fHeadSize, 0.0f, 0.0f));
                pVertex->SetColor(edgeColor);
            }
            for (i = 0; i < 6; i++, pVertex++)
            {
                pVertex->SetPos(PblVector3(((float)i * 0.4f - 1.0f) * fHeadSize, fHeadSize * 0.2f, 0.0f));
                RedColor color = pBeam->_color;
                color.a = (unsigned char)(int)(pBeam->_color.a * fIntensity * s_afBeamAlpha[2 * i + 1]);
                pVertex->SetColor(color);
            }
            for (i = 0; i < 7; i++, pVertex++)
            {
                pVertex->SetPos(PblVector3(((float)i * (1.0f / 3.0f) - 1.0f) * fTailSize, pBeam->_fLength, 0.0f));
                RedColor color = pBeam->_color;
                color.a = (unsigned char)(int)(pBeam->_color.a * fFadeIntensity * s_afBeamAlpha[2 * i]);
                pVertex->SetColor(color);
            }
            for (i = 0; i < 6; i++, pVertex++)
            {
                pVertex->SetPos(PblVector3(s_avBeamRadius[i].x * fTailSize,
                                           s_avBeamRadius[i].y * fFadeWidth + pBeam->_fLength, 0.0f));
                pVertex->SetColor(edgeColor);
            }
        }
        else
        {
            VertexU* pVertex = (VertexU*)pVertexData;
            for (i = 0; i < 3; i++, pVertex++)
            {
                pVertex->SetPos(PblVector3(((float)i * 0.59999973f - 0.6f) * fHeadSize, 0.0f, 0.0f));
                pVertex->SetColor(edgeColor);
            }
            for (i = 0; i < 6; i++, pVertex++)
            {
                pVertex->SetPos(PblVector3(((float)i * 0.4f - 1.0f) * fHeadSize, fHeadSize * 0.2f, 0.0f));
                RedColor color = pBeam->_color;
                color.a = (unsigned char)(int)(pBeam->_color.a * fIntensity * s_afBeamAlpha[2 * i + 1]);
                pVertex->SetColor(color);
            }
            for (i = 0; i < 7; i++, pVertex++)
            {
                pVertex->SetPos(PblVector3(((float)i * (1.0f / 3.0f) - 1.0f) * fTailSize, pBeam->_fLength, 0.0f));
                RedColor color = pBeam->_color;
                color.a = (unsigned char)(int)(pBeam->_color.a * fFadeIntensity * s_afBeamAlpha[2 * i]);
                pVertex->SetColor(color);
            }
            for (i = 0; i < 6; i++, pVertex++)
            {
                pVertex->SetPos(PblVector3(s_avBeamRadius[i].x * fTailSize,
                                           s_avBeamRadius[i].y * fFadeWidth + pBeam->_fLength, 0.0f));
                pVertex->SetColor(edgeColor);
            }
        }
        for (i = 0; i < 38; i++)
        {
            pIndices[i] = s_auiBeamIndices[i];
        }

        pPrimitive->EndIndexData();
        pPrimitive->EndVertexData();
        RedRenderer::pcRenderPrimitive(pPrimitive, m_pShader,
                                       mBeam, RedColor::WHITE,
                                       uiFlags, 0, 0, 0);
        pPrimitive->Release();
    }
    else
    {

        float fFade = 1.0f;
        const float fDepth = -vView.z - fNear;
        const float fOuterRadius = pBeam->_fTailSizeX + pBeam->_fFadeLength;
        if (fOuterRadius > fDepth)
        {
            if (pBeam->_fTailSizeX > fDepth)
            {
                return;
            }
            fFade = (fDepth - pBeam->_fTailSizeX) / (fOuterRadius - pBeam->_fTailSizeX);
        }
        const float fAlpha = pBeam->_color.a * fFade;
        RedColor flareColor = pBeam->_color;
        flareColor.a = (unsigned char)PblMath::Round(pBeam->_fFlareIntensity * fAlpha);
        flare.SetColor(flareColor);
        flare.SetSize(pBeam->_fHeadSizeX);

        const unsigned char ucAlpha = (unsigned char)PblMath::Round(pBeam->_fBeamIntensity * fAlpha);
        if (!ucAlpha)
        {
            return;
        }

        RedColor centerColor = pBeam->_color;
        centerColor.a = ucAlpha;
        RedColor innerColor = pBeam->_color;
        innerColor.a = (unsigned char)(int)(ucAlpha * pBeam->_fFadeAmount);
        RedColor edgeColor = pBeam->_color;
        edgeColor.a = 0;
        const PblVector3 vCenter(0.0f, 0.0f, pBeam->_fTailSizeX);
        const float fBound = fOuterRadius * 1.01f;
        const PblBox box(-fBound, -fBound, -fBound, fBound, fBound, fBound);

        pcRedPrimitive* pPrimitive;
        if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
        {
            pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x1082, sizeof(VertexC), 33, 66, false);
            CompressedPos::SetBoundingBox(&box);
        }
        else
        {
            pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x82, sizeof(VertexU), 33, 66, false);
        }
        pPrimitive->m_bbox = box;
        unsigned short* pIndices = pPrimitive->BeginIndexData();
        void* pVertexData = pPrimitive->BeginVertexData();
        if (!pVertexData)
        {
            return;
        }

        const float fInner = pBeam->_fTailSizeX;
        int i;
        if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
        {
            VertexC* pVertex = (VertexC*)pVertexData;
            pVertex->SetPos(vCenter);
            pVertex->SetColor(centerColor);
            pVertex++;
            for (i = 0; i < 16; i++, pVertex++)
            {
                const PblVector3& r = s_avHaloRadius[i];
                pVertex->SetPos(PblVector3(r.x * fInner + vCenter.x, r.y * fInner + vCenter.y,
                                           vCenter.z + r.z * fInner));
                pVertex->SetColor(innerColor);
            }
            for (i = 0; i < 16; i++, pVertex++)
            {
                const PblVector3& r = s_avHaloRadius[16 + i];
                pVertex->SetPos(PblVector3(r.x * fOuterRadius, r.y * fOuterRadius, fOuterRadius * r.z));
                pVertex->SetColor(edgeColor);
            }
        }
        else
        {
            VertexU* pVertex = (VertexU*)pVertexData;
            pVertex->SetPos(vCenter);
            pVertex->SetColor(centerColor);
            pVertex++;
            for (i = 0; i < 16; i++, pVertex++)
            {
                const PblVector3& r = s_avHaloRadius[i];
                pVertex->SetPos(PblVector3(r.x * fInner + vCenter.x, r.y * fInner + vCenter.y,
                                           vCenter.z + r.z * fInner));
                pVertex->SetColor(innerColor);
            }
            for (i = 0; i < 16; i++, pVertex++)
            {
                const PblVector3& r = s_avHaloRadius[16 + i];
                pVertex->SetPos(PblVector3(r.x * fOuterRadius, r.y * fOuterRadius, fOuterRadius * r.z));
                pVertex->SetColor(edgeColor);
            }
        }
        for (i = 0; i < 66; i++)
        {
            pIndices[i] = s_auiHaloIndices[i];
        }

        pPrimitive->EndIndexData();
        pPrimitive->EndVertexData();

        __declspec(align(16)) PblMatrix mHalo = mCamera;
        mHalo.m[3][0] = vHead.x;
        mHalo.m[3][1] = vHead.y;
        mHalo.m[3][2] = vHead.z;
        mHalo.m[3][3] = 1.0f;
        RedRenderer::pcRenderPrimitive(pPrimitive, m_pShader,
                                       mHalo, RedColor::WHITE,
                                       uiFlags, 0, 0, 0);
        pPrimitive->Release();
    }
}

// FUNCTION: SWBF2 0x0041fe40
void LightBeamEffect::_PlatformCleanup()
{
    if (m_pShader)
    {
        m_pShader->Release();
        m_pShader = 0;
    }
}
