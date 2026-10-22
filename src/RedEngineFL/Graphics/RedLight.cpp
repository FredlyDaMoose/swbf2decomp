#include <float.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

#include "RedLight.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedAmbientLight.h"
#include "RedCamera.h"
#include "RedDirectionalLight.h"
#include "RedOmniLight.h"
#include "RedShadowRegion.h"
#include "RedSpotLight.h"
#include "RedEngineFL/Util/RedRegion.h"
#include "PC/pcRedRenderer.h"
#include "PC/pcRedTexture.h"

namespace FLRenderer
{
unsigned int GetViewportCount();
}

extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);

// GLOBAL: SWBF2 0x007deff0
float RedShadowMap::_fShadowMapScale = 1.0f;

// GLOBAL: SWBF2 0x007deff4
unsigned int RedShadowMap::_uiShadowMapWrapMode = 1;

// GLOBAL: SWBF2 0x007deffc
bool bProjectionEnable = true;

// GLOBAL: SWBF2 0x007df000
float RedLight::m_sGlobalDirectionalFactor[2] = { 1.0f, 1.0f };

// GLOBAL: SWBF2 0x007df014
PblList<RedLight> RedLight::s_GlobalList;

// GLOBAL: SWBF2 0x007df028
PblList<RedLight> RedLight::s_VisibleOmniLightList;

// GLOBAL: SWBF2 0x007df03c
PblList<RedLight> RedLight::s_VisibleSpotLightList;

// GLOBAL: SWBF2 0x007df090
PblList<RedLight> RedLight::s_VisibleDirectionalLightList;

// GLOBAL: SWBF2 0x007df0a4
PblVector4 RedShadowMap::_vShadowMapTransformU = { 1.0f, 0.0f, 0.0f, 0.0f };

// GLOBAL: SWBF2 0x007df0b4
PblVector4 RedShadowMap::_vShadowMapTransformV = { 0.0f, 0.0f, 1.0f, 0.0f };

// GLOBAL: SWBF2 0x007df0c4
PblVector4 RedShadowMap::_vShadowMapTransformW = { 0.0f, 0.0f, 0.0f, 1.0f };

static const float LIGHT_MAX_PREFERED_CAMERA_DISTANCE = 60.0f;

// GLOBAL: SWBF2 0x007df0d4
float LIGHT_MAX_PREFERED_CAMERA_DISTANCE_INVSQR =
    1.0f / (LIGHT_MAX_PREFERED_CAMERA_DISTANCE * LIGHT_MAX_PREFERED_CAMERA_DISTANCE);

// GLOBAL: SWBF2 0x00960f18
unsigned int RedShadowMap::_uiShadowMapHash;

// GLOBAL: SWBF2 0x00960f20
unsigned int RedShadowMap::_fShadowMapOffsetX;

// GLOBAL: SWBF2 0x00960f24
unsigned int RedShadowMap::_fShadowMapOffsetZ;

// GLOBAL: SWBF2 0x00960f28
bool RedShadowMap::_bShadowMapEnable;

// GLOBAL: SWBF2 0x00960f30
RedShadowRegionFactory* RedLight::s_pShadowRegionFactory;

// GLOBAL: SWBF2 0x00960f34
RedDirectionalLight* RedLight::m_sGlobalDirectionals[2];

// GLOBAL: SWBF2 0x00960f3c
RedAmbientLight* RedLight::m_sGlobalAmbient;

// GLOBAL: SWBF2 0x00960f40
unsigned int RedLight::m_sGlobalEnvMapHash;

// FUNCTION: SWBF2 0x006c6ae0
template void PblList<RedLight>::RemoveAll();

// FUNCTION: SWBF2 0x006c6a60
RedLight::~RedLight()
{
    Deactivate();
}

// FUNCTION: SWBF2 0x006c6aa0
void RedLight::Activate()
{
    if (m_uiFlags & LIGHT_FLAG_ACTIVE)
    {
        return;
    }

    s_GlobalList.Add(&m_GlobalNode, this);
    m_uiFlags |= LIGHT_FLAG_ACTIVE;
}

// FUNCTION: SWBF2 0x006c6b20
void RedLight::Deactivate()
{
    if (!(m_uiFlags & LIGHT_FLAG_ACTIVE))
    {
        return;
    }

    s_GlobalList.Remove(&m_GlobalNode);
    if (m_VisibleNode._pList)
    {
        m_VisibleNode._pList->Remove(&m_VisibleNode);
    }

    m_uiFlags &= ~LIGHT_FLAG_ACTIVE;
}

// FUNCTION: SWBF2 0x006c6b90
void RedLight::DeinitSys()
{
    s_GlobalList.RemoveAll();
    s_VisibleOmniLightList.RemoveAll();
    s_VisibleSpotLightList.RemoveAll();
    s_VisibleDirectionalLightList.RemoveAll();

    RedShadowRegion::Cleanup();

    delete s_pShadowRegionFactory;
    s_pShadowRegionFactory = 0;
    delete m_sGlobalDirectionals[0];
    m_sGlobalDirectionals[0] = 0;
    delete m_sGlobalDirectionals[1];
    m_sGlobalDirectionals[1] = 0;
    delete m_sGlobalAmbient;
    m_sGlobalAmbient = 0;
}

// FUNCTION: SWBF2 0x006c6c30
const RedLight* RedLight::Find(unsigned int uiName)
{
    if (!uiName)
    {
        return 0;
    }

    for (PblList<RedLight>::Itor it = s_GlobalList.Begin(); it != s_GlobalList.End(); ++it)
    {
        if (it->m_uiName == uiName)
        {
            return it;
        }
    }

    if (m_sGlobalDirectionals[0]->m_uiName == uiName)
    {
        return m_sGlobalDirectionals[0];
    }
    if (m_sGlobalDirectionals[1]->m_uiName == uiName)
    {
        return m_sGlobalDirectionals[1];
    }
    return 0;
}

// FUNCTION: SWBF2 0x006c6c90
void RedLight::GetGlobalDirectionalAmbientColors(RedColorValue& topColor,
                                                 RedColorValue& bottomColor)
{
    topColor = m_sGlobalAmbient->m_topColor;
    bottomColor = m_sGlobalAmbient->m_bottomColor;
}

// FUNCTION: SWBF2 0x006c6cc0
const RedLightingState* RedLight::GetLightingState(const PblBox& box, int iMaxOmniLights,
                                                   bool bNoShadowRegions, bool bStatic,
                                                   bool bNoProjectedLights)
{
    if (!RedRenderer::GetLightingEnable())
    {
        return &RedRenderer::GetDefaultLightingState();
    }

    int iMaxSpotLights = 1;
    if (RedRenderer::pcGetRenderOptions().mGraphicsTier == 0)
    {
        iMaxOmniLights = 0;
        iMaxSpotLights = 0;
    }

    PblVector3 vCenter((box._vMin.x + box._vMax.x) * 0.5f, (box._vMin.y + box._vMax.y) * 0.5f,
                       (box._vMin.z + box._vMax.z) * 0.5f);
    float fSizeX = box._vMax.x - box._vMin.x;
    float fSizeY = box._vMax.y - box._vMin.y;
    float fSizeZ = box._vMax.z - box._vMin.z;
    float fRadius = (float)sqrt((double)(fSizeX * fSizeX + fSizeY * fSizeY + fSizeZ * fSizeZ)) * 0.5f;

    float afDirectionalPriority[2] = { 0.0f, 0.0f };
    const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
    bool bDefault = true;

    RedLightingState state;
    memset(&state, 0, sizeof(state));
    state.ambientLight = *m_sGlobalAmbient;
    if (m_sGlobalEnvMapHash)
    {
        state.pEnvMap = RedTexture::Find(m_sGlobalEnvMapHash);
    }
    state.envMapFactor = 1.0f;
    state.bStaticLighting = bStatic;

    bool bShadowed = false;
    if (!bStatic && !bNoShadowRegions)
    {
        RedAmbientLight ambient;
        float afDirectional[2];
        unsigned int uiEnvMapHash;
        float fEnvMapFactor;
        if (RedShadowRegion::GetShadowValues(vCenter, fRadius, afDirectional, ambient, uiEnvMapHash,
                                             fEnvMapFactor))
        {
            bShadowed = true;
            bDefault = false;
            afDirectional[0] *= m_sGlobalDirectionalFactor[0];
            afDirectional[1] *= m_sGlobalDirectionalFactor[1];
            for (int i = 0; i < 2; ++i)
            {
                if (m_sGlobalDirectionals[i] && afDirectional[i] > 0.0f)
                {
                    state.pDirectionalLights[state.numDirectionalLights] = m_sGlobalDirectionals[i];
                    state.pDirectionalFactor[state.numDirectionalLights] = afDirectional[i];
                    ++state.numDirectionalLights;
                }
            }
            state.ambientLight = ambient;
            state.pEnvMap = RedTexture::Find(uiEnvMapHash);
            state.envMapFactor = fEnvMapFactor;
        }
    }
    if (!bShadowed)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (m_sGlobalDirectionals[i])
            {
                float fFactor = m_sGlobalDirectionalFactor[i];
                if (fFactor < 1.0f)
                {
                    bDefault = false;
                }
                if (fFactor > 0.0f)
                {
                    state.pDirectionalLights[state.numDirectionalLights] = m_sGlobalDirectionals[i];
                    afDirectionalPriority[state.numDirectionalLights] = FLT_MAX;
                    state.pDirectionalFactor[state.numDirectionalLights] = fFactor;
                    ++state.numDirectionalLights;
                }
            }
        }
    }

    for (PblList<RedLight>::Itor it = s_VisibleDirectionalLightList.Begin();
         it != s_VisibleDirectionalLightList.End(); ++it)
    {
        RedDirectionalLight* pLight = static_cast<RedDirectionalLight*>((RedLight*)it);
        if (bStatic && (pLight->m_uiFlags & LIGHT_FLAG_STATIC) &&
            !(pLight->m_uiFlags & LIGHT_FLAG_SPECULARCASTER))
        {
            break;
        }
        if (bNoProjectedLights && pLight->m_uiProjTexHash)
        {
            continue;
        }

        float fPriority = FLT_MAX;
        float fFactor = 1.0f;
        const RedRegion* pRegion = pLight->m_boundingRegion;
        if (pRegion)
        {
            fFactor = pRegion->GetIntersectionFactor(vCenter, fRadius);
            if (fFactor <= 0.0f)
            {
                continue;
            }
            float fDX = pRegion->mMatrix.m[3][0] - mCamera.m[3][0];
            float fDY = pRegion->mMatrix.m[3][1] - mCamera.m[3][1];
            float fDZ = pRegion->mMatrix.m[3][2] - mCamera.m[3][2];
            float fCamera = (fDX * fDX + fDY * fDY + fDZ * fDZ) *
                            LIGHT_MAX_PREFERED_CAMERA_DISTANCE_INVSQR;
            float fDivisor = 1.0f;
            if (fCamera > 1.0f)
            {
                fDivisor = fCamera;
            }
            fPriority = pLight->GetIntensity() / fDivisor;
        }

        int i = 0;
        for (; i < state.numDirectionalLights; ++i)
        {
            if (fPriority > afDirectionalPriority[i])
            {
                break;
            }
        }
        if (i >= 2)
        {
            continue;
        }
        if (state.numDirectionalLights < 2)
        {
            ++state.numDirectionalLights;
        }
        for (int j = state.numDirectionalLights - 1; j > i; --j)
        {
            state.pDirectionalLights[j] = state.pDirectionalLights[j - 1];
            state.pDirectionalFactor[j] = state.pDirectionalFactor[j - 1];
            afDirectionalPriority[j] = afDirectionalPriority[j - 1];
        }
        state.pDirectionalLights[i] = pLight;
        state.pDirectionalFactor[i] = fFactor;
        afDirectionalPriority[i] = fPriority;
    }
    if (state.numDirectionalLights)
    {
        bDefault = false;
        for (int i = 0; i < state.numDirectionalLights; ++i)
        {
            if (state.pDirectionalLights[i]->m_uiProjTexHash)
            {
                state.pProjectedLight = state.pDirectionalLights[i];
                state.pDirectionalLights[i] = state.pDirectionalLights[0];
                state.pDirectionalLights[0] = static_cast<RedDirectionalLight*>(state.pProjectedLight);
                float fFactor = state.pDirectionalFactor[i];
                state.pDirectionalFactor[i] = state.pDirectionalFactor[0];
                state.pDirectionalFactor[0] = fFactor;
                break;
            }
        }
        for (int i = 0; i < state.numDirectionalLights; ++i)
        {
            if (state.pDirectionalLights[i]->IsSpecularCaster())
            {
                ++state.numSpecularLights;
            }
        }
    }

    if (iMaxOmniLights > 0)
    {
        float afOmniPriority[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        if (iMaxOmniLights > 4)
        {
            iMaxOmniLights = 4;
        }
        for (PblList<RedLight>::Itor it = s_VisibleOmniLightList.Begin();
             it != s_VisibleOmniLightList.End(); ++it)
        {
            RedOmniLight* pLight = static_cast<RedOmniLight*>((RedLight*)it);
            if (bStatic && (pLight->m_uiFlags & LIGHT_FLAG_STATIC))
            {
                break;
            }
            if (bNoProjectedLights && pLight->m_uiProjTexHash)
            {
                continue;
            }

            const PblVector3& vPos = pLight->m_pos;
            float fDistSq = 0.0f;
            float fD;
            if (box._vMin.x > vPos.x || vPos.x > box._vMax.x)
            {
                fD = vPos.x - (box._vMin.x > vPos.x ? box._vMin.x : box._vMax.x);
                fDistSq = fD * fD;
            }
            if (box._vMin.y > vPos.y || vPos.y > box._vMax.y)
            {
                fD = vPos.y - (box._vMin.y > vPos.y ? box._vMin.y : box._vMax.y);
                fDistSq += fD * fD;
            }
            if (box._vMin.z > vPos.z || vPos.z > box._vMax.z)
            {
                fD = vPos.z - (box._vMin.z > vPos.z ? box._vMin.z : box._vMax.z);
                fDistSq += fD * fD;
            }
            float fRadiusSq = pLight->m_radius * pLight->m_radius;
            if (fDistSq > fRadiusSq)
            {
                continue;
            }

            float fDivisor = 1.0f;
            float fDist4 = fDistSq * 4.0f;
            float fIntensity = pLight->GetIntensity();
            if (fDist4 > fRadiusSq)
            {
                fIntensity *= fRadiusSq;
                fDivisor = fDist4;
            }
            float fDX = vPos.x - mCamera.m[3][0];
            float fDY = vPos.y - mCamera.m[3][1];
            float fDZ = vPos.z - mCamera.m[3][2];
            float fCamera = (fDX * fDX + fDY * fDY + fDZ * fDZ) *
                            LIGHT_MAX_PREFERED_CAMERA_DISTANCE_INVSQR;
            if (fCamera > 1.0f)
            {
                fDivisor *= fCamera;
            }
            float fPriority = fIntensity / fDivisor;

            int i = 0;
            for (; i < state.numOmniLights; ++i)
            {
                if (fPriority > afOmniPriority[i])
                {
                    break;
                }
            }
            if (i >= iMaxOmniLights)
            {
                continue;
            }
            if (state.numOmniLights < iMaxOmniLights)
            {
                ++state.numOmniLights;
            }
            for (int j = state.numOmniLights - 1; j > i; --j)
            {
                state.pOmniLights[j] = state.pOmniLights[j - 1];
                afOmniPriority[j] = afOmniPriority[j - 1];
            }
            state.pOmniLights[i] = pLight;
            afOmniPriority[i] = fPriority;
        }
        if (state.numOmniLights)
        {
            bDefault = false;
            for (int i = 0; i < state.numOmniLights; ++i)
            {
                if (state.pOmniLights[i]->m_uiProjTexHash)
                {
                    state.pProjectedLight = state.pOmniLights[i];
                    state.pOmniLights[i] = state.pOmniLights[0];
                    state.pOmniLights[0] = static_cast<RedOmniLight*>(state.pProjectedLight);
                    break;
                }
            }
            for (int i = 0; i < state.numOmniLights; ++i)
            {
                if (state.pOmniLights[i]->IsSpecularCaster())
                {
                    ++state.numSpecularLights;
                }
            }
        }
    }

    if (iMaxSpotLights > 0)
    {
        float afSpotPriority[1] = { 0.0f };
        if (iMaxSpotLights > 1)
        {
            iMaxSpotLights = 1;
        }
        for (PblList<RedLight>::Itor it = s_VisibleSpotLightList.Begin();
             it != s_VisibleSpotLightList.End(); ++it)
        {
            RedSpotLight* pLight = static_cast<RedSpotLight*>((RedLight*)it);
            if (bStatic && (pLight->m_uiFlags & LIGHT_FLAG_STATIC))
            {
                break;
            }
            if (bNoProjectedLights && pLight->m_uiProjTexHash)
            {
                continue;
            }

            const PblVector3& vPos = pLight->m_vPos;
            float fDistSq = 0.0f;
            float fD;
            if (box._vMin.x > vPos.x || vPos.x > box._vMax.x)
            {
                fD = vPos.x - (box._vMin.x > vPos.x ? box._vMin.x : box._vMax.x);
                fDistSq = fD * fD;
            }
            if (box._vMin.y > vPos.y || vPos.y > box._vMax.y)
            {
                fD = vPos.y - (box._vMin.y > vPos.y ? box._vMin.y : box._vMax.y);
                fDistSq += fD * fD;
            }
            if (box._vMin.z > vPos.z || vPos.z > box._vMax.z)
            {
                fD = vPos.z - (box._vMin.z > vPos.z ? box._vMin.z : box._vMax.z);
                fDistSq += fD * fD;
            }
            float fRadiusSq = pLight->m_fRadius * pLight->m_fRadius;
            if (fDistSq > fRadiusSq)
            {
                continue;
            }
            if (!pLight->SphereIntersect(vCenter, fRadius))
            {
                continue;
            }

            float fDivisor = 1.0f;
            float fDist4 = fDistSq * 4.0f;
            float fIntensity = pLight->GetIntensity();
            if (fDist4 > fRadiusSq)
            {
                fIntensity *= fRadiusSq;
                fDivisor = fDist4;
            }
            float fDX = vPos.x - mCamera.m[3][0];
            float fDY = vPos.y - mCamera.m[3][1];
            float fDZ = vPos.z - mCamera.m[3][2];
            float fCamera = (fDX * fDX + fDY * fDY + fDZ * fDZ) *
                            LIGHT_MAX_PREFERED_CAMERA_DISTANCE_INVSQR;
            if (fCamera > 1.0f)
            {
                fDivisor *= fCamera;
            }
            float fPriority = fIntensity / fDivisor;

            int i = 0;
            for (; i < state.numSpotLights; ++i)
            {
                if (fPriority > afSpotPriority[i])
                {
                    break;
                }
            }
            if (i >= iMaxSpotLights)
            {
                continue;
            }
            if (state.numSpotLights < iMaxSpotLights)
            {
                ++state.numSpotLights;
            }
            for (int j = state.numSpotLights - 1; j > i; --j)
            {
                state.pSpotLights[j] = state.pSpotLights[j - 1];
                afSpotPriority[j] = afSpotPriority[j - 1];
            }
            state.pSpotLights[i] = pLight;
            afSpotPriority[i] = fPriority;
        }
        if (state.numSpotLights)
        {
            for (int i = 0; i < state.numSpotLights; ++i)
            {
                if (state.pSpotLights[i]->m_uiProjTexHash)
                {
                    state.pProjectedLight = state.pSpotLights[i];
                    state.pSpotLights[i] = state.pSpotLights[0];
                    state.pSpotLights[0] = static_cast<RedSpotLight*>(state.pProjectedLight);
                    break;
                }
            }
            for (int i = 0; i < state.numSpotLights; ++i)
            {
                if (state.pSpotLights[i]->IsSpecularCaster())
                {
                    ++state.numSpecularLights;
                }
            }
        }
    }

    RedLightingState* pState;
    if (bDefault)
    {
        pState = const_cast<RedLightingState*>(&RedRenderer::GetDefaultLightingState());
    }
    else
    {
        pState = RedRenderer::AllocLightingStateInCache();
        *pState = state;
    }
    if (FLRenderer::GetViewportCount() > 1 || !bProjectionEnable)
    {
        pState->pProjectedLight = 0;
    }
    return pState;
}

// FUNCTION: SWBF2 0x006c7b20
const RedLightingState* RedLight::GetLightingState(const PblSphere& sphere, int iMaxOmniLights,
                                                   bool bNoShadowRegions, bool bStatic,
                                                   bool bNoProjectedLights)
{
    if (!RedRenderer::GetLightingEnable())
    {
        return &RedRenderer::GetDefaultLightingState();
    }

    int iMaxSpotLights = 1;
    if (RedRenderer::pcGetRenderOptions().mGraphicsTier == 0)
    {
        iMaxOmniLights = 0;
        iMaxSpotLights = 0;
    }

    float afDirectionalPriority[2] = { 0.0f, 0.0f };
    const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
    bool bDefault = true;

    RedLightingState state;
    memset(&state, 0, sizeof(state));
    state.ambientLight = *m_sGlobalAmbient;
    if (m_sGlobalEnvMapHash)
    {
        state.pEnvMap = RedTexture::Find(m_sGlobalEnvMapHash);
    }
    state.envMapFactor = 1.0f;
    state.bStaticLighting = bStatic;

    bool bShadowed = false;
    if (!bStatic && !bNoShadowRegions)
    {
        RedAmbientLight ambient;
        float afDirectional[2];
        unsigned int uiEnvMapHash;
        float fEnvMapFactor;
        if (RedShadowRegion::GetShadowValues(sphere.GetCenter(), sphere.GetRadius(), afDirectional,
                                             ambient, uiEnvMapHash, fEnvMapFactor))
        {
            bShadowed = true;
            bDefault = false;
            afDirectional[0] *= m_sGlobalDirectionalFactor[0];
            afDirectional[1] *= m_sGlobalDirectionalFactor[1];
            for (int i = 0; i < 2; ++i)
            {
                if (m_sGlobalDirectionals[i] && afDirectional[i] > 0.0f)
                {
                    state.pDirectionalLights[state.numDirectionalLights] = m_sGlobalDirectionals[i];
                    state.pDirectionalFactor[state.numDirectionalLights] = afDirectional[i];
                    ++state.numDirectionalLights;
                }
            }
            state.ambientLight = ambient;
            state.pEnvMap = RedTexture::Find(uiEnvMapHash);
            state.envMapFactor = fEnvMapFactor;
        }
    }
    if (!bShadowed)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (m_sGlobalDirectionals[i])
            {
                float fFactor = m_sGlobalDirectionalFactor[i];
                if (fFactor < 1.0f)
                {
                    bDefault = false;
                }
                if (fFactor > 0.0f)
                {
                    state.pDirectionalLights[state.numDirectionalLights] = m_sGlobalDirectionals[i];
                    afDirectionalPriority[state.numDirectionalLights] = FLT_MAX;
                    state.pDirectionalFactor[state.numDirectionalLights] = fFactor;
                    ++state.numDirectionalLights;
                }
            }
        }
    }

    for (PblList<RedLight>::Itor it = s_VisibleDirectionalLightList.Begin();
         it != s_VisibleDirectionalLightList.End(); ++it)
    {
        RedDirectionalLight* pLight = static_cast<RedDirectionalLight*>((RedLight*)it);
        if (bStatic && (pLight->m_uiFlags & LIGHT_FLAG_STATIC) &&
            !(pLight->m_uiFlags & LIGHT_FLAG_SPECULARCASTER))
        {
            break;
        }
        if (bNoProjectedLights && pLight->m_uiProjTexHash)
        {
            continue;
        }

        float fPriority = FLT_MAX;
        float fFactor = 1.0f;
        const RedRegion* pRegion = pLight->m_boundingRegion;
        if (pRegion)
        {
            fFactor = pRegion->GetIntersectionFactor(sphere.GetCenter(), sphere.GetRadius());
            if (fFactor <= 0.0f)
            {
                continue;
            }
            float fDX = pRegion->mMatrix.m[3][0] - mCamera.m[3][0];
            float fDY = pRegion->mMatrix.m[3][1] - mCamera.m[3][1];
            float fDZ = pRegion->mMatrix.m[3][2] - mCamera.m[3][2];
            float fCamera = (fDX * fDX + fDY * fDY + fDZ * fDZ) *
                            LIGHT_MAX_PREFERED_CAMERA_DISTANCE_INVSQR;
            float fDivisor = 1.0f;
            if (fCamera > 1.0f)
            {
                fDivisor = fCamera;
            }
            fPriority = pLight->GetIntensity() / fDivisor;
        }

        int i = 0;
        for (; i < state.numDirectionalLights; ++i)
        {
            if (fPriority > afDirectionalPriority[i])
            {
                break;
            }
        }
        if (i >= 2)
        {
            continue;
        }
        if (state.numDirectionalLights < 2)
        {
            ++state.numDirectionalLights;
        }
        for (int j = state.numDirectionalLights - 1; j > i; --j)
        {
            state.pDirectionalLights[j] = state.pDirectionalLights[j - 1];
            state.pDirectionalFactor[j] = state.pDirectionalFactor[j - 1];
            afDirectionalPriority[j] = afDirectionalPriority[j - 1];
        }
        state.pDirectionalLights[i] = pLight;
        state.pDirectionalFactor[i] = fFactor;
        afDirectionalPriority[i] = fPriority;
    }
    if (state.numDirectionalLights)
    {
        bDefault = false;
        for (int i = 0; i < state.numDirectionalLights; ++i)
        {
            if (state.pDirectionalLights[i]->m_uiProjTexHash)
            {
                state.pProjectedLight = state.pDirectionalLights[i];
                state.pDirectionalLights[i] = state.pDirectionalLights[0];
                state.pDirectionalLights[0] = static_cast<RedDirectionalLight*>(state.pProjectedLight);
                float fFactor = state.pDirectionalFactor[i];
                state.pDirectionalFactor[i] = state.pDirectionalFactor[0];
                state.pDirectionalFactor[0] = fFactor;
                break;
            }
        }
        for (int i = 0; i < state.numDirectionalLights; ++i)
        {
            if (state.pDirectionalLights[i]->IsSpecularCaster())
            {
                ++state.numSpecularLights;
            }
        }
    }

    if (iMaxOmniLights > 0)
    {
        float afOmniPriority[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        if (iMaxOmniLights > 4)
        {
            iMaxOmniLights = 4;
        }
        for (PblList<RedLight>::Itor it = s_VisibleOmniLightList.Begin();
             it != s_VisibleOmniLightList.End(); ++it)
        {
            RedOmniLight* pLight = static_cast<RedOmniLight*>((RedLight*)it);
            if (bStatic && (pLight->m_uiFlags & LIGHT_FLAG_STATIC))
            {
                break;
            }
            if (bNoProjectedLights && pLight->m_uiProjTexHash)
            {
                continue;
            }

            float fRadius = pLight->m_radius;
            float fDX = pLight->m_pos.x - sphere.x;
            float fDY = pLight->m_pos.y - sphere.y;
            float fDZ = pLight->m_pos.z - sphere.z;
            float fDistSq = fDX * fDX + fDY * fDY + fDZ * fDZ;
            float fReach = sphere.GetRadius() + fRadius;
            if (fDistSq >= fReach * fReach)
            {
                continue;
            }

            float fRadiusSq = fRadius * fRadius;
            float fDivisor = 1.0f;
            float fDist4 = fDistSq * 4.0f;
            float fIntensity = pLight->GetIntensity();
            if (fDist4 > fRadiusSq)
            {
                fIntensity *= fRadiusSq;
                fDivisor = fDist4;
            }
            fDX = pLight->m_pos.x - mCamera.m[3][0];
            fDY = pLight->m_pos.y - mCamera.m[3][1];
            fDZ = pLight->m_pos.z - mCamera.m[3][2];
            float fCamera = (fDX * fDX + fDY * fDY + fDZ * fDZ) *
                            LIGHT_MAX_PREFERED_CAMERA_DISTANCE_INVSQR;
            if (fCamera > 1.0f)
            {
                fDivisor *= fCamera;
            }
            float fPriority = fIntensity / fDivisor;

            int i = 0;
            for (; i < state.numOmniLights; ++i)
            {
                if (fPriority > afOmniPriority[i])
                {
                    break;
                }
            }
            if (i >= iMaxOmniLights)
            {
                continue;
            }
            if (state.numOmniLights < iMaxOmniLights)
            {
                ++state.numOmniLights;
            }
            for (int j = state.numOmniLights - 1; j > i; --j)
            {
                state.pOmniLights[j] = state.pOmniLights[j - 1];
                afOmniPriority[j] = afOmniPriority[j - 1];
            }
            state.pOmniLights[i] = pLight;
            afOmniPriority[i] = fPriority;
        }
        if (state.numOmniLights)
        {
            bDefault = false;
            for (int i = 0; i < state.numOmniLights; ++i)
            {
                if (state.pOmniLights[i]->m_uiProjTexHash)
                {
                    state.pProjectedLight = state.pOmniLights[i];
                    state.pOmniLights[i] = state.pOmniLights[0];
                    state.pOmniLights[0] = static_cast<RedOmniLight*>(state.pProjectedLight);
                    break;
                }
            }
            for (int i = 0; i < state.numOmniLights; ++i)
            {
                if (state.pOmniLights[i]->IsSpecularCaster())
                {
                    ++state.numSpecularLights;
                }
            }
        }
    }

    if (iMaxSpotLights > 0)
    {
        float afSpotPriority[1] = { 0.0f };
        if (iMaxSpotLights > 1)
        {
            iMaxSpotLights = 1;
        }
        for (PblList<RedLight>::Itor it = s_VisibleSpotLightList.Begin();
             it != s_VisibleSpotLightList.End(); ++it)
        {
            RedSpotLight* pLight = static_cast<RedSpotLight*>((RedLight*)it);
            if (bStatic && (pLight->m_uiFlags & LIGHT_FLAG_STATIC))
            {
                break;
            }
            if (bNoProjectedLights && pLight->m_uiProjTexHash)
            {
                continue;
            }

            float fRadius = sphere.GetRadius();
            float fReach = fRadius + pLight->m_fRadius;
            float fDX = pLight->m_vPos.x - sphere.x;
            float fDY = pLight->m_vPos.y - sphere.y;
            float fDZ = pLight->m_vPos.z - sphere.z;
            float fDistSq = fDX * fDX + fDY * fDY + fDZ * fDZ;
            if (fDistSq >= fReach * fReach)
            {
                continue;
            }
            if (!pLight->SphereIntersect(sphere.GetCenter(), fRadius))
            {
                continue;
            }

            float fRadiusSq = pLight->m_fRadius * pLight->m_fRadius;
            float fDivisor = 1.0f;
            float fDist4 = fDistSq * 4.0f;
            float fIntensity = pLight->GetIntensity();
            if (fDist4 > fRadiusSq)
            {
                fIntensity *= fRadiusSq;
                fDivisor = fDist4;
            }
            fDX = pLight->m_vPos.x - mCamera.m[3][0];
            fDY = pLight->m_vPos.y - mCamera.m[3][1];
            fDZ = pLight->m_vPos.z - mCamera.m[3][2];
            float fCamera = (fDX * fDX + fDY * fDY + fDZ * fDZ) *
                            LIGHT_MAX_PREFERED_CAMERA_DISTANCE_INVSQR;
            if (fCamera > 1.0f)
            {
                fDivisor *= fCamera;
            }
            float fPriority = fIntensity / fDivisor;

            int i = 0;
            for (; i < state.numSpotLights; ++i)
            {
                if (fPriority > afSpotPriority[i])
                {
                    break;
                }
            }
            if (i >= iMaxSpotLights)
            {
                continue;
            }
            if (state.numSpotLights < iMaxSpotLights)
            {
                ++state.numSpotLights;
            }
            for (int j = state.numSpotLights - 1; j > i; --j)
            {
                state.pSpotLights[j] = state.pSpotLights[j - 1];
                afSpotPriority[j] = afSpotPriority[j - 1];
            }
            state.pSpotLights[i] = pLight;
            afSpotPriority[i] = fPriority;
        }
        if (state.numSpotLights)
        {
            for (int i = 0; i < state.numSpotLights; ++i)
            {
                if (state.pSpotLights[i]->m_uiProjTexHash)
                {
                    state.pProjectedLight = state.pSpotLights[i];
                    state.pSpotLights[i] = state.pSpotLights[0];
                    state.pSpotLights[0] = static_cast<RedSpotLight*>(state.pProjectedLight);
                    break;
                }
            }
            for (int i = 0; i < state.numSpotLights; ++i)
            {
                if (state.pSpotLights[i]->IsSpecularCaster())
                {
                    ++state.numSpecularLights;
                }
            }
        }
    }

    RedLightingState* pState;
    if (bDefault)
    {
        pState = const_cast<RedLightingState*>(&RedRenderer::GetDefaultLightingState());
    }
    else
    {
        pState = RedRenderer::AllocLightingStateInCache();
        *pState = state;
    }
    if (FLRenderer::GetViewportCount() > 1 || !bProjectionEnable)
    {
        pState->pProjectedLight = 0;
    }
    return pState;
}

// FUNCTION: SWBF2 0x006c86f0
const RedLightingState* RedLight::GetLightingState(bool bStatic)
{
    return &RedRenderer::GetDefaultLightingState();
}

// FUNCTION: SWBF2 0x006c8700
void RedLight::InitSys()
{
    s_GlobalList.RemoveAll();
    s_VisibleOmniLightList.RemoveAll();
    s_VisibleSpotLightList.RemoveAll();
    s_VisibleDirectionalLightList.RemoveAll();

    m_sGlobalDirectionals[0] = new RedDirectionalLight();
    m_sGlobalDirectionals[1] = new RedDirectionalLight();
    m_sGlobalAmbient = new RedAmbientLight(RedColorValue::WHITE, RedColorValue::WHITE,
                                           RedColorValue::WHITE);
    m_sGlobalEnvMapHash = 0;

    RedShadowRegion::Init();
    s_pShadowRegionFactory = new RedShadowRegionFactory();

    RedShadowMap::InitSys();
}

// FUNCTION: SWBF2 0x006c8990
void RedLight::PostLoad()
{
    for (PblList<RedLight>::Itor it = s_GlobalList.Begin(); it != s_GlobalList.End(); ++it)
    {
        if (it->m_eType == TYPE_DIRECTIONAL)
        {
            RedDirectionalLight* pLight = static_cast<RedDirectionalLight*>((RedLight*)it);
            pLight->m_boundingRegion = RedRegion::Find(pLight->m_uiRegionName);
        }
    }
}

// FUNCTION: SWBF2 0x006c8900
void RedShadowMap::InitSys()
{
    _vShadowMapTransformU.x = 1.0f;
    _vShadowMapTransformU.y = 0.0f;
    _vShadowMapTransformU.z = 0.0f;
    _vShadowMapTransformU.w = 0.0f;
    RedShadowMap::_uiShadowMapHash = 0;
    _vShadowMapTransformV.x = 0.0f;
    _vShadowMapTransformV.y = 0.0f;
    _vShadowMapTransformV.z = 1.0f;
    _vShadowMapTransformV.w = 0.0f;
    _fShadowMapScale = 1.0f;
    RedShadowMap::_fShadowMapOffsetX = 0;
    RedShadowMap::_fShadowMapOffsetZ = 0;
    _vShadowMapTransformW.x = 0.0f;
    _vShadowMapTransformW.y = 0.0f;
    _vShadowMapTransformW.z = 0.0f;
    _vShadowMapTransformW.w = 1.0f;
    _bShadowMapEnable = false;
    _uiShadowMapWrapMode = 1;
}

// FUNCTION: SWBF2 0x006c8970
bool RedShadowMap::IsShadowMapEnabled()
{
    return _bShadowMapEnable && RedRenderer::pcGetRenderOptions().shadowQuality == QUALITY2_HIGH;
}

// FUNCTION: SWBF2 0x006c9810
void RedShadowMap::SetScreenSpaceShadowMap(RedTexture* pTexture, const PblMatrix& matrix)
{
    _pShadowMapTexture = pTexture;
    RedShadowMap::_uiShadowMapHash = pTexture ? pTexture->GetName() : 0;

    _vShadowMapTransformU.x = matrix.m[0][0];
    _vShadowMapTransformU.y = matrix.m[1][0];
    _vShadowMapTransformU.z = matrix.m[2][0];
    _vShadowMapTransformU.w = matrix.m[3][0];
    _vShadowMapTransformV.x = matrix.m[0][1];
    _vShadowMapTransformV.y = matrix.m[1][1];
    _vShadowMapTransformV.z = matrix.m[2][1];
    _vShadowMapTransformV.w = matrix.m[3][1];
    _vShadowMapTransformW.x = matrix.m[0][3];
    _vShadowMapTransformW.y = matrix.m[1][3];
    _vShadowMapTransformW.z = matrix.m[2][3];
    _vShadowMapTransformW.w = matrix.m[3][3];
    RedRenderer::pcSetShadowMapTransform(_vShadowMapTransformU,
                                         _vShadowMapTransformV,
                                         _vShadowMapTransformW);
}

static const float kf1by255 = 1.0f / 255.0f;

// FUNCTION: SWBF2 0x006c8af0
void RedLight::ReadGlobals(PblConfig& config)
{
    unsigned int uiLight1 = 0;
    unsigned int uiLight2 = 0;
    unsigned int uiEnvMap = 0;

    RedColorValue topColor;
    topColor.r = RedColor::CYAN.r * kf1by255;
    topColor.g = RedColor::CYAN.g * kf1by255;
    topColor.b = RedColor::CYAN.b * kf1by255;
    topColor.a = RedColor::CYAN.a * kf1by255;
    RedColorValue bottomColor;
    bottomColor.r = RedColor::YELLOW.r * kf1by255;
    bottomColor.g = RedColor::YELLOW.g * kf1by255;
    bottomColor.b = RedColor::YELLOW.b * kf1by255;
    bottomColor.a = RedColor::YELLOW.a * kf1by255;

    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0xa710dc3c:
            topColor.r = data.GetUint8Arg(0) * kf1by255;
            topColor.g = data.GetUint8Arg(1) * kf1by255;
            topColor.b = data.GetUint8Arg(2) * kf1by255;
            topColor.a = 1.0f;
            break;
        case 0x4ea76b2a:
            bottomColor.r = data.GetUint8Arg(0) * kf1by255;
            bottomColor.g = data.GetUint8Arg(1) * kf1by255;
            bottomColor.b = data.GetUint8Arg(2) * kf1by255;
            bottomColor.a = 1.0f;
            break;
        case 0xbb6dd0da:
            uiEnvMap = PblHash(data.GetStringArg(0));
            break;
        case 0xdb56693a:
            uiLight1 = PblHash(data.GetStringArg(0));
            break;
        case 0xda5667a7:
            uiLight2 = PblHash(data.GetStringArg(0));
            break;
        }
    }

    RedLight::m_sGlobalAmbient->m_topColor = topColor;
    RedLight::m_sGlobalAmbient->m_bottomColor = bottomColor;
    RedColorValue mean;
    mean.r = (topColor.r + bottomColor.r) * 0.5f;
    mean.g = (topColor.g + bottomColor.g) * 0.5f;
    mean.b = (topColor.b + bottomColor.b) * 0.5f;
    mean.a = (topColor.a + bottomColor.a) * 0.5f;
    RedLight::m_sGlobalAmbient->m_color = mean;

    RedLight::m_sGlobalEnvMapHash = uiEnvMap;
    RedLight::m_sGlobalDirectionalFactor[0] = 0.0f;
    RedLight::m_sGlobalDirectionalFactor[1] = 0.0f;

    for (PblList<RedLight>::Itor it = RedLight::s_GlobalList.Begin(); it != RedLight::s_GlobalList.End(); ++it)
    {
        RedLight* pLight = it;
        if (uiLight1 && pLight->m_uiName == uiLight1)
        {
            if (pLight->m_eType == RedLight::TYPE_DIRECTIONAL)
            {
                RedLight::m_sGlobalDirectionals[0] = (RedDirectionalLight*)pLight;
                RedLight::m_sGlobalDirectionalFactor[0] = 1.0f;
                pLight->Deactivate();
            }
        }
        else if (uiLight2 && pLight->m_uiName == uiLight2 && pLight->m_eType == RedLight::TYPE_DIRECTIONAL)
        {
            RedLight::m_sGlobalDirectionals[1] = (RedDirectionalLight*)pLight;
            RedLight::m_sGlobalDirectionalFactor[1] = 1.0f;
            pLight->Deactivate();
        }
    }
}

// FUNCTION: SWBF2 0x006c89d0
void RedLight::Read(PblFileChunk& chunk)
{
    PblConfig config(chunk);
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();

        PblConfig scope(config.ReadNextScope(), true);
        if (data.GetId() == 0xe29d1e2f)
        {
            ReadLight(scope, PblHash(data.GetStringArg(0)));
        }
        else if (data.GetId() == 0xf7548f33)
        {
            ReadGlobals(scope);
        }
    }
}

// FUNCTION: SWBF2 0x006c9030
void RedLight::ReadLight(PblConfig& config, unsigned int uiName)
{
    PblQuaternion qRotation(0.0f, 0.0f, 0.0f, 1.0f);
    PblVector3 vPosition;
    RedColorValue color;
    color.r = RedColor::BLACK.r * kf1by255;
    color.g = RedColor::BLACK.g * kf1by255;
    color.b = RedColor::BLACK.b * kf1by255;
    color.a = RedColor::BLACK.a * kf1by255;
    unsigned int uiType = 0;
    unsigned int uiRegion = 0;
    unsigned int uiTexture = 0;
    unsigned int uiWrapMode = 3;
    float fRange = 0.0f;
    float fInnerCone = 0.0f;
    float fOuterCone = 0.0f;
    float fFalloff = 1.0f;
    float fTileU = 1.0f;
    float fTileV = 1.0f;
    float fOffsetU = 0.0f;
    float fOffsetV = 0.0f;
    bool bBidirectional = false;
    bool bCastShadow = false;
    bool bCastSpecular = false;
    bool bStatic = false;
    bool bOverbright = false;

    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x21ac415f:
            qRotation._fX = -data.GetFloatArg(3);
            qRotation._fY = data.GetFloatArg(0);
            qRotation._fZ = -data.GetFloatArg(1);
            qRotation._fS = data.GetFloatArg(2);
            break;
        case 0x044c2100:
            bBidirectional = data.GetUint32Arg(0) == 1;
            break;
        case 0x031afe7d:
            uiRegion = PblHash(data.GetStringArg(0));
            break;
        case 0x3c6468f4:
            uiTexture = PblHash(data.GetStringArg(0));
            uiWrapMode = data.GetInt32Arg(1) ? 3 : 1;
            break;
        case 0x5127f14d:
            uiType = data.GetUint32Arg(0);
            break;
        case 0x489162bb:
            bCastSpecular = data.GetNumArgs() > 0 && data.GetInt32Arg(0) != 0;
            break;
        case 0x3d7e6258:
            color.r = data.GetFloatArg(0);
            color.g = data.GetFloatArg(1);
            color.b = data.GetFloatArg(2);
            color.a = 1.0f;
            break;
        case 0x55b86d2c:
            bOverbright = true;
            break;
        case 0x6ac627c1:
            fOffsetU = data.GetFloatArg(0);
            fOffsetV = data.GetFloatArg(1);
            break;
        case 0x934f4e0a:
            vPosition.x = data.GetFloatArg(0);
            vPosition.y = data.GetFloatArg(1);
            vPosition.z = -data.GetFloatArg(2);
            break;
        case 0xbfe60da6:
            fTileU = data.GetFloatArg(0);
            fTileV = data.GetFloatArg(1);
            break;
        case 0xc9b2bcc5:
            fFalloff = data.GetFloatArg(0);
            break;
        case 0xdd404b6a:
            fInnerCone = data.GetFloatArg(0);
            fOuterCone = data.GetFloatArg(1);
            break;
        case 0xd290c23b:
            bStatic = true;
            break;
        case 0xe7e90aee:
            bCastShadow = true;
            break;
        case 0xfadc0cd2:
            fRange = data.GetFloatArg(0);
            break;
        }
    }

    PblQuaternion qUnit;
    PblMatrix mTransform;
    D3DXMatrixRotationQuaternion(&mTransform, D3DXQuaternionNormalize(&qUnit, &qRotation));
    mTransform.m[3][0] = vPosition.x;
    mTransform.m[3][1] = vPosition.y;
    mTransform.m[3][2] = vPosition.z;
    mTransform.m[3][3] = 1.0f;
    const PblVector3& vDirection = mTransform.GetZAxis();

    RedLight* pLight = 0;
    switch (uiType)
    {
    case TYPE_DIRECTIONAL:
    {
        RedDirectionalLight* pDirectional = new RedDirectionalLight(vDirection, color);
        pDirectional->m_uiRegionName = uiRegion;
        pDirectional->SetOverbright(bOverbright);
        pDirectional->SetColor(color);
        pDirectional->SetProjectionTextureTile(fTileU, fTileV);
        pDirectional->SetProjectionTextureOffset(fOffsetU, fOffsetV);
        pLight = pDirectional;
        break;
    }
    case TYPE_POINT:
    {
        RedOmniLight* pOmni = new RedOmniLight(vPosition, fRange, color);
        pOmni->SetOverbright(bOverbright);
        pOmni->SetColor(color);
        pLight = pOmni;
        break;
    }
    case TYPE_SPOT:
    {
        RedSpotLight* pSpot = new RedSpotLight(color, vDirection, vPosition, fRange, fFalloff);
        pSpot->SetInnerConeAngle(fInnerCone);
        pSpot->SetOuterConeAngle(fOuterCone);
        pSpot->SetOverbright(bOverbright);
        pSpot->SetColor(color);
        pSpot->SetBidirectional(bBidirectional);
        pLight = pSpot;
        break;
    }
    default:
        return;
    }

    if (pLight)
    {
        pLight->SetShadowCaster(bCastShadow);
        pLight->SetSpecularCaster(bCastSpecular);
        if (uiTexture)
        {
            pLight->SetProjectionTexture(uiTexture);
            pLight->SetProjectionTextureWrapMode(uiWrapMode);
        }
        pLight->SetName(uiName);
        pLight->SetStatic(bStatic);
        pLight->Activate();
    }
}

// FUNCTION: SWBF2 0x006c97a0
void RedLight::SetGlobalAmbientLightColor(RedColorValue& color)
{
    m_sGlobalAmbient->SetColor(color);
}

// FUNCTION: SWBF2 0x006c97c0
void RedLight::SetGlobalDirectionalAmbientColors(RedColorValue& topColor,
                                                 RedColorValue& bottomColor)
{
    m_sGlobalAmbient->SetTopColor(topColor);
    m_sGlobalAmbient->SetBottomColor(bottomColor);
}

// FUNCTION: SWBF2 0x006c97f0
void RedLight::SetGlobalDirectionalLightColor(RedColorValue& color, int i)
{
    m_sGlobalDirectionals[i]->SetColor(color);
}

// FUNCTION: SWBF2 0x00719be0
void RedShadowRegion::UpdateAllVisibility(const RedCamera* pCamera)
{
    RedShadowRegion::s_VisibleList.RemoveAll();
    if (!pCamera)
    {
        return;
    }

    for (PblList<RedShadowRegion>::Itor it(RedShadowRegion::s_List); it != RedShadowRegion::s_List.End();
         ++it)
    {
        if (it->IsVisible(pCamera, 25.0f))
        {
            RedShadowRegion::s_VisibleList.Add(&it->m_VisibleNode, it);
        }
    }
}

// FUNCTION: SWBF2 0x006c98f0
void RedLight::UpdateAllVisibility(const RedCamera* pCamera, unsigned int uiView)
{
    s_VisibleOmniLightList.RemoveAll();
    s_VisibleSpotLightList.RemoveAll();
    s_VisibleDirectionalLightList.RemoveAll();
    RedShadowRegion::UpdateAllVisibility(pCamera);
    if (!pCamera)
    {
        return;
    }

    for (PblList<RedLight>::Itor it(s_GlobalList); it != s_GlobalList.End(); ++it)
    {
        if ((it->m_uiHideInView & (1 << uiView)) || !it->IsVisible(pCamera))
        {
            continue;
        }

        RedLight* pLight = it;
        switch (pLight->m_eType)
        {
        case TYPE_DIRECTIONAL:
            if ((pLight->m_uiFlags & LIGHT_FLAG_STATIC) && !(pLight->m_uiFlags & LIGHT_FLAG_SPECULARCASTER))
            {
                s_VisibleDirectionalLightList.AddLast(&pLight->m_VisibleNode, pLight);
            }
            else
            {
                s_VisibleDirectionalLightList.AddFirst(&pLight->m_VisibleNode, pLight);
            }
            break;
        case TYPE_POINT:
            if (pLight->m_uiFlags & LIGHT_FLAG_STATIC)
            {
                s_VisibleOmniLightList.AddLast(&pLight->m_VisibleNode, pLight);
            }
            else
            {
                s_VisibleOmniLightList.AddFirst(&pLight->m_VisibleNode, pLight);
            }
            break;
        case TYPE_SPOT:
            if (pLight->m_uiFlags & LIGHT_FLAG_STATIC)
            {
                s_VisibleSpotLightList.AddLast(&pLight->m_VisibleNode, pLight);
            }
            else
            {
                s_VisibleSpotLightList.AddFirst(&pLight->m_VisibleNode, pLight);
            }
            break;
        }
    }

    RedLightingState state;
    memset(&state, 0, sizeof(state));
    state.pDirectionalLights[0] = m_sGlobalDirectionals[0];
    state.pDirectionalLights[1] = m_sGlobalDirectionals[1];
    state.pDirectionalFactor[0] = m_sGlobalDirectionalFactor[0];
    state.pDirectionalFactor[1] = m_sGlobalDirectionalFactor[1];
    state.numDirectionalLights = 2;
    state.ambientLight = *m_sGlobalAmbient;
    state.pEnvMap = RedTexture::Find(m_sGlobalEnvMapHash);
    state.envMapFactor = 1.0f;
    if (state.pDirectionalLights[0]->IsSpecularCaster())
    {
        ++state.numSpecularLights;
    }
    if (state.pDirectionalLights[1]->IsSpecularCaster())
    {
        ++state.numSpecularLights;
    }
    RedRenderer::SetDefaultLightingState(state);
}

// GLOBAL: SWBF2 0x00960f1c
RedTexture* RedShadowMap::_pShadowMapTexture = 0;
