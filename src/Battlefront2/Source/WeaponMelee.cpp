#include "PebbleFL/Common/PblHashTable.h"

#include <new>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "WeaponMelee.h"
#include "WeaponMeleeInstance.h"
#include "WeaponMeleeThrow.h"
#include "LightsaberTrailEffect.h"
#include "ParticleSystem.h"
#include "Aimer.h"
#include "Combo.h"
#include "Controllable.h"
#include "EntitySoldier.h"
#include "SoldierAnimationBank.h"
#include "SoldierAnimationData.h"
#include "SoldierAnimator.h"
#include "SoldierAnimatorClass.h"
#include "SoldierAnimType.h"
#include "Trigger.h"
#include "Explosion.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "Battlefront2/Platform/Pc/pcLightBeam.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "RedEngineFL/Application/pc/pcRedParticle.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblMath.h"
#include "EnergyBar.h"
#include "FLEffect.h"
#include "Ordnance.h"
#include "OrdnanceBeam.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "Damageable.h"
#include "Battlefront2/Platform/Pc/pcSunFlare.h"

const RedColor Interpolate(const RedColor& a, const RedColor& b, float t);

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                     const PblMatrix* pM2);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut,
                                                          const PblVector3* pV, const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut,
                                                           const PblVector3* pV, const PblMatrix* pM);
extern bool netInShell;
extern bool netEnabled;
extern bool netEnabledNext;
extern bool netIsLocalTurn;

extern bool gAltMeleeHitEffects;

struct MeleeDamageSample
{
    float fTime;
    PblVector3 vPos;
    PblVector3 vDir;
};

// FUNCTION: SWBF2 0x0068f260
void _RenderLightSabre(const PblVector3& vPosition, const PblVector3& vDirection,
                       unsigned int uiTexture, unsigned int uiGlowTexture,
                       float fLength, float fWidth, unsigned int uiFlags)
{
    if (!uiTexture || fLength == 0.0f || fWidth == 0.0f)
    {
        return;
    }

    RedRenderer::GetCamera();
    const float fExtent = (fLength + fWidth) * 2.0f;
    PblVector3 vMin(vPosition.x - fExtent, vPosition.y - fExtent, vPosition.z - fExtent);
    PblVector3 vMax(vPosition.x + fExtent, vPosition.y + fExtent, vPosition.z + fExtent);
    RedParticleRenderer::SubmitBounds(vMin, vMax);
    RedParticleRenderer::SubmitTexture(uiTexture, BLEND_ADDITIVE, uiFlags);

    const PblVector3 vRight(1.0f, 0.0f, 0.0f);
    const PblVector3 vUp(0.0f, 1.0f, 0.0f);
    RedParticleRenderer::TexCoords texCoords = {};
    texCoords.u1 = texCoords.v1 = 255;
    PblVector3 vTip(vPosition.x + vDirection.x * fLength,
                    vPosition.y + vDirection.y * fLength,
                    vPosition.z + vDirection.z * fLength);
    RedParticleRenderer::SubmitParticle(PIT_LASER_START, vTip, RedColor::WHITE,
                                        fWidth * 0.8f, 0.0f, texCoords, vRight, vUp);
    const float fBaseOffset = fLength * 0.04f;
    PblVector3 vBase(vPosition.x - vDirection.x * fBaseOffset,
                     vPosition.y - vDirection.y * fBaseOffset,
                     vPosition.z - vDirection.z * fBaseOffset);
    RedParticleRenderer::SubmitParticle(PIT_LASER_END, vBase, RedColor::WHITE,
                                        fWidth, 0.0f, texCoords, vRight, vUp);

    if (uiGlowTexture)
    {
        const float fGlowWidth = (PblRandom::GetGlobalFloat() + 1.5f) * fWidth;
        RedParticleRenderer::SubmitTexture(uiGlowTexture, BLEND_ADDITIVE, uiFlags);
        RedColor glow = {255, 255, 255, 0x33};
        RedParticleRenderer::SubmitParticle(PIT_LASER_START, vTip, glow,
                                            fGlowWidth * 0.8f, 0.0f, texCoords, vRight, vUp);
        RedParticleRenderer::SubmitParticle(PIT_LASER_END, vPosition, glow,
                                            fGlowWidth, 0.0f, texCoords, vRight, vUp);
    }
}

// FUNCTION: SWBF2 0x0068a050
void WeaponMelee::Render(const PblMatrix& matrix, const RedPose* pPose,
                         const RedColor& color, unsigned int uiFlags, bool bRenderModel) const
{
    uiFlags |= 1;
    bool bThrown = false;
    if (m_pMeleeThrow)
    {
        bThrown = (pPose && m_pMeleeThrow->m_bThrow) || m_pMeleeThrow->m_bReturnedThisFrame;
    }

    const WeaponMeleeClass* pClass = static_cast<const WeaponMeleeClass*>(mRenderClass);
    RedModel* pModel = bRenderModel && pClass->mHighResModel ?
                       pClass->mHighResModel : pClass->mModel;
    const PblMatrix* apMatrix[4] = {};
    PblMatrix aMatrix[4];
    if (pModel && !bThrown)
    {
        if (pPose)
        {
            const PblMatrix* pBone = pPose->Find(pClass->m_auiHardpoint[0]);
            if (pBone)
            {
                D3DXMatrixMultiply(&aMatrix[0], pBone, &matrix);
                apMatrix[0] = &aMatrix[0];
            }
        }
        else
        {
            apMatrix[0] = &matrix;
        }
        if (apMatrix[0] && bRenderModel)
        {
            pModel->Render(apMatrix[0], 0, color, uiFlags, 0);
        }
    }

    for (unsigned int i = 1; i < pClass->m_uiNumWeapons; ++i)
    {
        RedModel* pWeaponModel = pClass->m_apWeaponModel[i - 1];
        if (!pWeaponModel || !pPose)
        {
            continue;
        }
        const PblMatrix* pBone = pPose->Find(pClass->m_auiHardpoint[i]);
        if (!pBone)
        {
            continue;
        }
        D3DXMatrixMultiply(&aMatrix[i], pBone, &matrix);
        apMatrix[i] = &aMatrix[i];
        if (bRenderModel)
        {
            pWeaponModel->Render(apMatrix[i], 0, color, uiFlags, 0);
        }
    }

    bool bRenderedBlade = false;
    for (unsigned int i = bThrown ? 1 : 0; i < pClass->m_uiNumDamageEdges; ++i)
    {
        const WeaponMeleeClass::DamageEdge& edge = pClass->m_aDamageEdge[i];
        if (!edge.uiTextureHash)
        {
            continue;
        }
        const PblMatrix* pEdgeMatrix = apMatrix[edge.uiAttachToWeapon];
        if (!pEdgeMatrix)
        {
            continue;
        }
        if (!bThrown && edge.colorTrail.a && !(uiFlags & 0x100) && bRenderModel)
        {
            bool bTrailOn = (m_uiTrailOnMask & (1u << i)) && IsOn(edge.uiAttachToWeapon);
            PblMatrix mTrail = *pEdgeMatrix;
            const_cast<WeaponMelee*>(this)->RenderLightsaberTrail(i, &mTrail, bTrailOn, uiFlags);
        }
        PblVector3 vPosition, vDirection;
        D3DXVec3TransformCoord(&vPosition, &edge.vOffsetPos, pEdgeMatrix);
        D3DXVec3TransformNormal(&vDirection, &edge.vOffsetDir, pEdgeMatrix);
        const float fLength = GetLightSaberLengthFactor(edge.uiAttachToWeapon) * edge.fLength;
        _RenderLightSabre(vPosition, vDirection, edge.uiTextureHash,
                           bRenderModel ? edge.uiGlowTextureHash : 0,
                           fLength, edge.fWidth, uiFlags);
        bRenderedBlade = true;
    }
    if (bRenderedBlade && (uiFlags & 0x4000000))
    {
        ParticleSystem::FlushParticleCache();
        RedParticleRenderer::RenderAll();
    }
}

// FUNCTION: SWBF2 0x0068e8f0
void WeaponMeleeClass::Render(float fTurnOn, const PblMatrix& matrix, const RedPose* pPose,
                              const RedColor& color, unsigned int  , bool  ) const
{
    const unsigned int uiRenderFlags = 0x04000000;
    const PblMatrix* apMatrix[4] = {};
    PblMatrix aMatrix[4];
    if (mModel)
    {
        if (pPose)
        {
            const PblMatrix* pBone = pPose->Find(m_auiHardpoint[0]);
            if (pBone)
            {
                D3DXMatrixMultiply(&aMatrix[0], pBone, &matrix);
                apMatrix[0] = &aMatrix[0];
            }
        }
        else
        {
            apMatrix[0] = &matrix;
        }
        if (apMatrix[0])
        {
            mModel->Render(apMatrix[0], 0, color, uiRenderFlags, 0);
        }
    }

    for (unsigned int i = 1; i < m_uiNumWeapons; ++i)
    {
        if (!m_apWeaponModel[i - 1] || !pPose)
        {
            continue;
        }
        const PblMatrix* pBone = pPose->Find(m_auiHardpoint[i]);
        if (!pBone)
        {
            continue;
        }
        D3DXMatrixMultiply(&aMatrix[i], pBone, &matrix);
        apMatrix[i] = &aMatrix[i];
        m_apWeaponModel[i - 1]->Render(apMatrix[i], 0, color, uiRenderFlags, 0);
    }

    bool bRenderedBlade = false;
    for (unsigned int i = 0; i < m_uiNumDamageEdges; ++i)
    {
        const DamageEdge& edge = m_aDamageEdge[i];
        if (!edge.uiTextureHash)
        {
            continue;
        }
        const PblMatrix* pEdgeMatrix = apMatrix[edge.uiAttachToWeapon];
        if (!pEdgeMatrix)
        {
            continue;
        }
        PblVector3 vPosition, vDirection;
        D3DXVec3TransformCoord(&vPosition, &edge.vOffsetPos, pEdgeMatrix);
        D3DXVec3TransformNormal(&vDirection, &edge.vOffsetDir, pEdgeMatrix);
        _RenderLightSabre(vPosition, vDirection, edge.uiTextureHash, edge.uiGlowTextureHash,
                           edge.fLength * fTurnOn, edge.fWidth, uiRenderFlags);
        bRenderedBlade = true;
    }
    if (bRenderedBlade)
    {
        ParticleSystem::FlushParticleCache();
        RedParticleRenderer::RenderAll();
    }
}

// FUNCTION: SWBF2 0x00574bb0
int LightsaberTrail::GetTotalNumSteps(float fMaxTimeStep, float fMaxDistanceStep,
                                      unsigned char* pSteps, float* pTotalDistance) const
{
    const unsigned int uiCount = m_uiNumSamples;
    if (!uiCount)
    {
        return 0;
    }
    if (pSteps)
    {
        pSteps[0] = 1;
    }
    if (pTotalDistance)
    {
        *pTotalDistance = 0.0f;
    }

    unsigned int uiStepCount = 1;
    float fPreviousTime = m_afTime[m_uiSampleFirst];
    for (unsigned int i = 1; i < uiCount; ++i)
    {
        const unsigned int uiIndex = (m_uiSampleFirst + i) & 7;
        const float fDistance = m_afDist[uiIndex];
        unsigned int uiSteps = 1;
        if (fDistance > 0.0f)
        {
            const float fElapsed = m_afTime[uiIndex] - fPreviousTime;
            if (fElapsed > fMaxTimeStep)
            {
                uiSteps = static_cast<unsigned int>(fElapsed / fMaxTimeStep) + 1;
            }
            if (fDistance > fMaxDistanceStep)
            {
                const unsigned int uiDistanceSteps =
                    static_cast<unsigned int>(fDistance / fMaxDistanceStep) + 1;
                if (uiSteps < uiDistanceSteps)
                {
                    uiSteps = uiDistanceSteps;
                }
            }
            if (pTotalDistance)
            {
                *pTotalDistance += fDistance;
            }
        }
        if (pSteps)
        {
            pSteps[i] = static_cast<unsigned char>(uiSteps);
        }
        fPreviousTime = m_afTime[uiIndex];
        uiStepCount += uiSteps;
    }
    return uiStepCount;
}

// FUNCTION: SWBF2 0x00574d00
PblBox LightsaberTrail::GetBoundingBox(float fLength) const
{
    PblBox box;
    box._vMin = m_avPos[m_uiSampleFirst];
    box._vMax = m_avPos[m_uiSampleFirst];
    unsigned char uiPrev = m_uiSampleFirst;
    for (unsigned char i = 1; i < m_uiNumSamples; ++i)
    {
        const unsigned char uiIndex =
            static_cast<unsigned char>((m_uiSampleFirst + i) % LightsaberTrail::MAX_SAMPLES_PER_TRAIL);
        const PblVector3& vPos = m_avPos[uiIndex];
        const PblVector3& vDir = m_avDir[uiIndex];
        PblVector3 vTip(vPos.x + vDir.x * fLength, vPos.y + vDir.y * fLength, vPos.z + vDir.z * fLength);
        box.AddPoint(vPos);
        box.AddPoint(vTip);
        if (m_afDist[uiIndex] > 0.0f)
        {
            const float fHalfStep = (m_afTime[uiIndex] - m_afTime[uiPrev]) * 0.5f;
            const PblVector3& vPrevPos = m_avPos[uiPrev];
            const PblVector3& vPrevVel = m_avVel[uiPrev];
            PblVector3 vAfter(vPrevPos.x + vPrevVel.x * fHalfStep, vPrevPos.y + vPrevVel.y * fHalfStep,
                              vPrevPos.z + vPrevVel.z * fHalfStep);
            box.AddPoint(vAfter);
            const PblVector3& vVel = m_avVel[uiIndex];
            PblVector3 vBefore(vPos.x - vVel.x * fHalfStep, vPos.y - vVel.y * fHalfStep,
                               vPos.z - vVel.z * fHalfStep);
            box.AddPoint(vBefore);
        }
        uiPrev = uiIndex;
    }
    const float fPad = fLength * 1.5f;
    box._vMax = PblVector3(box._vMax.x + fPad, box._vMax.y + fPad, box._vMax.z + fPad);
    box._vMin = PblVector3(box._vMin.x - fPad, box._vMin.y - fPad, box._vMin.z - fPad);
    return box;
}

// GLOBAL: SWBF2 0x01e56fa1
unsigned char LightsaberTrail::s_uiSample = 0;
// GLOBAL: SWBF2 0x01e56fa2
unsigned char LightsaberTrail::s_uiSamplePrev = 0;
// GLOBAL: SWBF2 0x01e573e8
float LightsaberTrail::s_fDeltaTimeInv = 0.0f;

// FUNCTION: SWBF2 0x00575140
int LightsaberTrail::_SetSample(float fTime) const
{
    unsigned char uiEnd = LightsaberTrail::s_uiSample;
    if (LightsaberTrail::s_uiSample == LightsaberTrail::s_uiSamplePrev || m_afTime[LightsaberTrail::s_uiSamplePrev] > fTime)
    {
        LightsaberTrail::s_uiSamplePrev = m_uiSampleFirst;
        uiEnd = (LightsaberTrail::s_uiSample + 1) & 7;
        LightsaberTrail::s_uiSample = uiEnd;
        LightsaberTrail::s_fDeltaTimeInv = 1.0f / (m_afTime[uiEnd] - m_afTime[LightsaberTrail::s_uiSamplePrev]);
    }
    int iResult = uiEnd;
    if (fTime > m_afTime[uiEnd])
    {
        do
        {
            const unsigned char uiNext = (uiEnd + 1) & 7;
            iResult = uiNext;
            if (uiNext == m_uiSampleNext)
            {
                break;
            }
            const unsigned char uiPrevious = uiEnd;
            uiEnd = uiNext;
            LightsaberTrail::s_uiSamplePrev = uiPrevious;
            LightsaberTrail::s_uiSample = uiEnd;
            const float fNextTime = m_afTime[uiEnd];
            iResult = uiPrevious;
            LightsaberTrail::s_fDeltaTimeInv = 1.0f / (fNextTime - m_afTime[uiPrevious]);
            if (fTime <= fNextTime)
            {
                break;
            }
        }
        while (true);
    }
    return iResult;
}

static PblVector3* InterpolateTrailVector(const LightsaberTrail* pTrail, float fTime,
                                         PblVector3* pResult, const PblVector3* pSamples,
                                         const PblVector3* pVelocities)
{
    pTrail->_SetSample(fTime);
    const unsigned int uiBegin = LightsaberTrail::s_uiSamplePrev;
    const unsigned int uiEnd = LightsaberTrail::s_uiSample;
    const float fElapsed = fTime - pTrail->m_afTime[uiBegin];
    const float fFraction = LightsaberTrail::s_fDeltaTimeInv * fElapsed;
    const float fRemaining = 1.0f - fFraction;
    const float fBeginVelocityWeight = fRemaining * fRemaining * fElapsed;
    const float fEndVelocityWeight = fRemaining * fElapsed * fFraction;
    const float fEndWeight = (3.0f - 2.0f * fFraction) * fFraction * fFraction;
    const float fBeginWeight = 1.0f - fEndWeight;
    pResult->x = (pSamples[uiBegin].x * fBeginWeight + pSamples[uiEnd].x * fEndWeight)
               + pVelocities[uiBegin].x * fBeginVelocityWeight
               - pVelocities[uiEnd].x * fEndVelocityWeight;
    pResult->y = (pSamples[uiBegin].y * fBeginWeight + pSamples[uiEnd].y * fEndWeight)
               + pVelocities[uiBegin].y * fBeginVelocityWeight
               - pVelocities[uiEnd].y * fEndVelocityWeight;
    pResult->z = (pSamples[uiBegin].z * fBeginWeight + pSamples[uiEnd].z * fEndWeight)
               + pVelocities[uiBegin].z * fBeginVelocityWeight
               - pVelocities[uiEnd].z * fEndVelocityWeight;
    return pResult;
}

// FUNCTION: SWBF2 0x00575230
PblVector3* LightsaberTrail::GetInterpolatedPos(float fTime, PblVector3* pResult) const
{
    return InterpolateTrailVector(this, fTime, pResult, m_avPos, m_avVel);
}

// FUNCTION: SWBF2 0x005753b0
PblVector3* LightsaberTrail::GetInterpolatedDir(float fTime, PblVector3* pResult) const
{
    return InterpolateTrailVector(this, fTime, pResult, m_avDir, m_avOmg);
}

 
 
 
 

namespace GameLoop { extern float sClientDeltaTime; }

// GLOBAL: SWBF2 0x01fac398
static unsigned int s_uiODFComboId;

namespace DefaultCombo
{
    enum ComboState
    {
        IDLE = 0,
        ATTACK1 = 1,
        RECOVER1 = 2,
        ATTACK2 = 3,
        RECOVER2 = 4,
        ATTACK3 = 5,
        RECOVER3 = 6,
        NUM_STATES = 7
    };

    struct DeflectAnimation
    {
        char* szName;
        unsigned char uiDirections;
    };

    static const unsigned int NUM_DEFLECT_ANIMATIONS = 8;

    // GLOBAL: SWBF2 0x0078c0fc
    char* aszAnimationName[2] = { "stand_attack1a", "stand_attack1b" };
    // GLOBAL: SWBF2 0x0078c104
    int saiAnimForState[NUM_STATES] = { -1, 0, 0, 1, 1, 0, 0 };
    // GLOBAL: SWBF2 0x0078c164
    char* aszStateName[NUM_STATES] = { "IDLE", "ATTACK1", "RECOVER1", "ATTACK2",
                                       "RECOVER2", "ATTACK3", "RECOVER3" };
}

static const float LIGHTSABER_DEFAULT_LENGTH = 1.0f;
static const float LIGHTSABER_DEFAULT_WIDTH = 0.08f;
static const float MELEE_DEFAULT_STATE_DURATION = 1.0f;
static const float MELEE_DEFAULT_DAMAGE_TIME = 0.2f;
static const float MELEE_DEFAULT_DAMAGE = 1000.0f;
static const float MELEE_DEFAULT_DAMAGE_DISTANCE = 1.2f;
static const float MELEE_DEFAULT_TURNONDURATION = 0.2f;

// GLOBAL: SWBF2 0x01f9a8f8
RedColor LIGHTSABER_DEFAULT_TRAIL_COLOR;

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

namespace
{

struct VertexU
{
    PblVector3 pos;
    RedColor color;

    void SetPos(const PblVector3& v) { pos = v; }
    void SetColor(const RedColor& c) { color = c; }
};
 

const PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });
}

// FUNCTION: SWBF2 0x00421480
void LightsaberTrailEffect::RenderLightsaberTrail(const LightsaberTrail* pTrail, unsigned int uiFlags) const
{
    unsigned char auiNumSteps[LightsaberTrail::MAX_SAMPLES_PER_TRAIL];
    float fTotalDist = 0.0f;
    unsigned int uiNumSteps = pTrail->GetTotalNumSteps(m_fLIGHTSABER_TRAIL_MAX_TIME_PER_STEP,
                                         m_fLIGHTSABER_TRAIL_MAX_DISTANCE_PER_STEP,
                                         auiNumSteps, &fTotalDist);
    if (!uiNumSteps)
    {
        return;
    }
    if (pTrail->m_bTrailWasOn)
    {
        uiNumSteps += 2;
    }

    const LightsaberTrailClass* pClass = pTrail->m_pClass;
    RedColor colorTrail;
    colorTrail.a = 0xff;
    const RedColor colorHead = pClass->m_colorHead;
    const float fWidth = pClass->m_fWidth;
    const float fLength = pClass->m_fLength;
    colorTrail.b = (unsigned char)(int)((pClass->m_colorTrail.b * pClass->m_colorTrail.a + 0.5f) * (1.0f / 255.0f));
    colorTrail.g = (unsigned char)(int)((pClass->m_colorTrail.g * pClass->m_colorTrail.a + 0.5f) * (1.0f / 255.0f));
    colorTrail.r = (unsigned char)(int)((pClass->m_colorTrail.r * pClass->m_colorTrail.a + 0.5f) * (1.0f / 255.0f));
    PblBox bbox = pTrail->GetBoundingBox(fLength);
    const PblVector3& vCamZ = RedRenderer::GetCamera()->GetMatrix().GetZAxis();
    const float fPersistTime = m_fLIGHTSABER_TRAIL_PERSIST_TIME;
    const float fFadeTime = m_fLIGHTSABER_TRAIL_FADE_TIME;
    const float fFadeTimeInv = 1.0f / (fPersistTime - fFadeTime);
    const float fFadeDist = m_fLIGHTSABER_TRAIL_FADE_DISTANCE;
    const float fFadeDistInv = 1.0f / fFadeDist;
    const unsigned int uiNumVertices = 6 * uiNumSteps - 2;

    const bool bCompressed = RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED;
    pcRedPrimitive* pPrim;
    if (bCompressed)
    {
        pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x1082, sizeof(VertexC), uiNumVertices, 0, false);
        CompressedPos::SetBoundingBox(&bbox);
    }
    else
    {
        pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x82, sizeof(VertexU), uiNumVertices, 0, false);
    }
    pPrim->m_bbox = bbox;
    RedColor colorEdge = colorTrail;
    colorEdge.a = 0;
    void* pData = pPrim->BeginVertexData();
    float fTime;
    if (bCompressed)
    {
        if (!pData)
        {
            return;
        }
        VertexC* pFront = (VertexC*)pData;
        VertexC* pBack = pFront + 4 * uiNumSteps - 2;
        VertexC* pThird = pBack + 1;

        unsigned int uiSample = (pTrail->m_uiSampleNext - pTrail->m_uiNumSamples) & 7;
        PblVector3 vDir = pTrail->m_avDir[uiSample];
        PblVector3 vPos = pTrail->m_avPos[uiSample];
        PblVector3 vPosNext(vPos.x + vDir.x * fLength, vPos.y + vDir.y * fLength, vPos.z + vDir.z * fLength);
        float fDot = vDir.x * vCamZ.x + vDir.y * vCamZ.y + vDir.z * vCamZ.z;
        PblVector3 vCap(vDir.x - fDot * vCamZ.x, vDir.y - fDot * vCamZ.y, vDir.z - fDot * vCamZ.z);
        if (vCap.x * vCap.x + vCap.y * vCap.y + vCap.z * vCap.z <= 0.01f)
        {
            vCap.x *= 10.0f;
            vCap.y *= 10.0f;
            vCap.z *= 10.0f;
        }
        else
        {
            PblVector3 vNormal;
            D3DXVec3Normalize(&vNormal, &vCap);
            vCap = vNormal;
        }
        pFront[0].SetPos(PblVector3(vPos.x - vCap.x * fWidth, vPos.y - vCap.y * fWidth, vPos.z - vCap.z * fWidth));
        pFront[0].SetColor(colorEdge);
        pFront[1].SetPos(vPos);
        pFront[1].SetColor(colorEdge);
        pFront += 2;
        pBack[0].SetPos(vPosNext);
        pBack[0].SetColor(colorEdge);
        pBack[-1].SetPos(vPos);
        pBack[-1].SetColor(colorEdge);
        pBack -= 2;
        pThird[0].SetPos(PblVector3(vPosNext.x + vCap.x * fWidth, vPosNext.y + vCap.y * fWidth,
                                    vPosNext.z + vCap.z * fWidth));
        pThird[0].SetColor(colorEdge);
        pThird += 1;

        bool bCoreDone = !pTrail->m_bTrailWasOn;
        float fTimePrev = pTrail->m_afTime[(pTrail->m_uiSampleNext - pTrail->m_uiNumSamples) & 7];
        float fAgePrev = pTrail->m_fTimer - fTimePrev;
        float fDist = 0.0f;
        for (unsigned int i = 1; i < pTrail->m_uiNumSamples; ++i)
        {
            uiSample = (i + pTrail->m_uiSampleNext - pTrail->m_uiNumSamples) & 7;
            const float fSampleDist = pTrail->m_afDist[uiSample];
            const bool bMoved = fSampleDist > 0.0f;
            bool bNextVisible;
            if (i == pTrail->m_uiNumSamples - 1)
            {
                bNextVisible = pTrail->m_bTrailWasOn;
            }
            else
            {
                bNextVisible = auiNumSteps[i + 1] != 0;
            }
            const float fSampleTime = pTrail->m_afTime[uiSample];
            const int iNumSteps = auiNumSteps[i];
            const float fStepInv = 1.0f / iNumSteps;
            const float fTimeStep = (fSampleTime - fTimePrev) * fStepInv;
            const float fDistStep = fStepInv * fSampleDist;
            for (int j = 0; j < iNumSteps; ++j)
            {
                fTime = fTimePrev + fTimeStep;
                fDist += fDistStep;
                RedColor color = colorTrail;
                const float fAge = pTrail->m_fTimer - fTime;
                pTrail->GetInterpolatedDir(fTime, &vDir);
                fDot = vDir.x * vCamZ.x + vDir.y * vCamZ.y + vDir.z * vCamZ.z;
                vCap = PblVector3(vDir.x - fDot * vCamZ.x, vDir.y - fDot * vCamZ.y, vDir.z - fDot * vCamZ.z);
                if (vCap.x * vCap.x + vCap.y * vCap.y + vCap.z * vCap.z <= 0.01f)
                {
                    vCap.x *= 10.0f;
                    vCap.y *= 10.0f;
                    vCap.z *= 10.0f;
                }
                else
                {
                    PblVector3 vNormal;
                    D3DXVec3Normalize(&vNormal, &vCap);
                    vCap = vNormal;
                }

                if (bCoreDone || m_fLIGHTSABER_TRAIL_CORE_MAX_TIME < fAge ||
                    fDist <= fTotalDist - m_fLIGHTSABER_TRAIL_CORE_MAX_DISTANCE)
                {
                    PblVector3 vResult;
                    vPos = *pTrail->GetInterpolatedPos(fTime, &vResult);
                    vPosNext = PblVector3(vPos.x + vDir.x * fLength, vPos.y + vDir.y * fLength,
                                          vPos.z + vDir.z * fLength);
                }
                else
                {

                    PblVector3 vPosNew;
                    pTrail->GetInterpolatedPos(fTime, &vPosNew);
                    const PblVector3 vPosNextNew(vPos.x + vDir.x * fLength, vPos.y + vDir.y * fLength,
                                                 vPos.z + vDir.z * fLength);
                    const float fPosMove = sqrtf((vPosNew.x - vPos.x) * (vPosNew.x - vPos.x) +
                                                 (vPosNew.y - vPos.y) * (vPosNew.y - vPos.y) +
                                                 (vPosNew.z - vPos.z) * (vPosNew.z - vPos.z));
                    const float fTipMove = sqrtf((vPosNextNew.x - vPosNext.x) * (vPosNextNew.x - vPosNext.x) +
                                                 (vPosNextNew.y - vPosNext.y) * (vPosNextNew.y - vPosNext.y) +
                                                 (vPosNextNew.z - vPosNext.z) * (vPosNextNew.z - vPosNext.z));
                    float fCore;
                    if (fAgePrev <= m_fLIGHTSABER_TRAIL_CORE_MAX_TIME)
                    {
                        fCore = (fDist - (fTotalDist - m_fLIGHTSABER_TRAIL_CORE_MAX_DISTANCE)) /
                                (fPosMove > fTipMove ? fPosMove : fTipMove);
                        if (fCore > 1.0f)
                        {
                            fCore = 1.0f;
                        }
                    }
                    else
                    {
                        fCore = (m_fLIGHTSABER_TRAIL_CORE_MAX_TIME - fAge) / (fAgePrev - fAge);
                    }
                    float fPosWidth = 0.0f;
                    if (fPosMove > 0.01f)
                    {
                        fPosWidth = m_fLIGHTSABER_TRAIL_CORE_WIDTH * fWidth * 0.5f / fPosMove;
                    }
                    float fTipWidth = 0.0f;
                    if (fTipMove > 0.01f)
                    {
                        fTipWidth = m_fLIGHTSABER_TRAIL_CORE_WIDTH * fWidth * 0.5f / fTipMove;
                    }

                    const PblVector3 vPosDelta(vPos.x - vPosNew.x, vPos.y - vPosNew.y, vPos.z - vPosNew.z);
                    const PblVector3 vTipDelta(vPosNext.x - vPosNextNew.x, vPosNext.y - vPosNextNew.y,
                                               vPosNext.z - vPosNextNew.z);
                    float fPos = fPosWidth + fCore;
                    float fTip = fTipWidth + fCore;
                    PblVector3 vCorePos(vPosDelta.x * fPos + vPosNew.x, vPosDelta.y * fPos + vPosNew.y,
                                        vPosDelta.z * fPos + vPosNew.z);
                    PblVector3 vCoreTip(vTipDelta.x * fTip + vPosNextNew.x, vTipDelta.y * fTip + vPosNextNew.y,
                                        vTipDelta.z * fTip + vPosNextNew.z);
                    pFront[0].SetPos(PblVector3(vCorePos.x - vCap.x * fWidth, vCorePos.y - vCap.y * fWidth,
                                                vCorePos.z - vCap.z * fWidth));
                    pFront[0].SetColor(colorEdge);
                    pFront[1].SetPos(vCorePos);
                    pFront[1].SetColor(colorTrail);
                    pBack[0].SetPos(vCoreTip);
                    pBack[0].SetColor(colorTrail);
                    pBack[-1].SetPos(vCorePos);
                    pBack[-1].SetColor(colorTrail);
                    pBack -= 2;
                    pThird[0].SetPos(vCoreTip);
                    pThird[0].SetColor(colorTrail);
                    pThird[1].SetPos(PblVector3(vCoreTip.x + vCap.x * fWidth, vCoreTip.y + vCap.y * fWidth,
                                                vCoreTip.z + vCap.z * fWidth));
                    pThird[1].SetColor(colorEdge);
                    pThird += 2;

                    if (i == pTrail->m_uiNumSamples - 1 && j == iNumSteps - 1)
                    {
                        if (fPosWidth > fCore)
                        {
                            fPosWidth = fCore;
                        }
                        if (fTipWidth > fCore)
                        {
                            fTipWidth = fCore;
                        }
                    }
                    fPos = fCore - fPosWidth;
                    fTip = fCore - fTipWidth;
                    vCorePos = PblVector3(vPosDelta.x * fPos + vPosNew.x, vPosDelta.y * fPos + vPosNew.y,
                                          vPosDelta.z * fPos + vPosNew.z);
                    vCoreTip = PblVector3(vTipDelta.x * fTip + vPosNextNew.x, vTipDelta.y * fTip + vPosNextNew.y,
                                          vTipDelta.z * fTip + vPosNextNew.z);
                    pFront[2].SetPos(PblVector3(vCorePos.x - vCap.x * fWidth, vCorePos.y - vCap.y * fWidth,
                                                vCorePos.z - vCap.z * fWidth));
                    pFront[2].SetColor(colorEdge);
                    pFront[3].SetPos(vCorePos);
                    pFront[3].SetColor(colorHead);
                    pFront += 4;
                    pBack[0].SetPos(vCoreTip);
                    pBack[0].SetColor(colorHead);
                    pBack[-1].SetPos(vCorePos);
                    pBack[-1].SetColor(colorHead);
                    pBack -= 2;
                    pThird[0].SetPos(vCoreTip);
                    pThird[0].SetColor(colorHead);
                    pThird[1].SetPos(PblVector3(vCoreTip.x + vCap.x * fWidth, vCoreTip.y + vCap.y * fWidth,
                                                vCoreTip.z + vCap.z * fWidth));
                    pThird[1].SetColor(colorEdge);
                    pThird += 2;

                    vPos = fPos > 0.0f ? vPosNew : vCorePos;
                    vPosNext = fTip > 0.0f ? vPosNextNew : vCoreTip;
                    bCoreDone = true;
                }

                if (bCoreDone && pTrail->m_bTrailWasOn)
                {
                    color = colorHead;
                }
                else if (fAge < fPersistTime && bMoved && (bNextVisible || j != iNumSteps - 1))
                {
                    float fFade = 1.0f;
                    if (fAge > fFadeTime)
                    {
                        fFade = (fPersistTime - fAge) * fFadeTimeInv * ((fPersistTime - fAge) * fFadeTimeInv);
                    }
                    if (fFadeDist > fDist)
                    {
                        fFade = fDist * fFadeDistInv * fFade;
                    }
                    if (fFade < 1.0f)
                    {
                        color.a = (unsigned char)(int)(color.a * fFade);
                    }
                }
                else
                {
                    color.a = 0;
                }

                pFront[0].SetPos(PblVector3(vPos.x - vCap.x * fWidth, vPos.y - vCap.y * fWidth, vPos.z - vCap.z * fWidth));
                pFront[0].SetColor(colorEdge);
                pFront[1].SetPos(vPos);
                pFront[1].SetColor(color);
                pFront += 2;
                pBack[0].SetPos(vPosNext);
                pBack[0].SetColor(color);
                pBack[-1].SetPos(vPos);
                pBack[-1].SetColor(color);
                pBack -= 2;
                pThird[0].SetPos(vPosNext);
                pThird[0].SetColor(color);
                pThird[1].SetPos(PblVector3(vPosNext.x + vCap.x * fWidth, vPosNext.y + vCap.y * fWidth,
                                            vPosNext.z + vCap.z * fWidth));
                pThird[1].SetColor(colorEdge);
                pThird += 2;

                fAgePrev = fAge;
                fTimePrev = fTime;
            }
            fTimePrev = fSampleTime;
        }
    }
    else
    {
        if (!pData)
        {
            return;
        }
        VertexU* pFront = (VertexU*)pData;
        VertexU* pBack = pFront + 4 * uiNumSteps - 2;
        VertexU* pThird = pBack + 1;

        unsigned int uiSample = (pTrail->m_uiSampleNext - pTrail->m_uiNumSamples) & 7;
        PblVector3 vDir = pTrail->m_avDir[uiSample];
        PblVector3 vPos = pTrail->m_avPos[uiSample];
        PblVector3 vPosNext(vPos.x + vDir.x * fLength, vPos.y + vDir.y * fLength, vPos.z + vDir.z * fLength);
        float fDot = vDir.x * vCamZ.x + vDir.y * vCamZ.y + vDir.z * vCamZ.z;
        PblVector3 vCap(vDir.x - fDot * vCamZ.x, vDir.y - fDot * vCamZ.y, vDir.z - fDot * vCamZ.z);
        if (vCap.x * vCap.x + vCap.y * vCap.y + vCap.z * vCap.z <= 0.01f)
        {
            vCap.x *= 10.0f;
            vCap.y *= 10.0f;
            vCap.z *= 10.0f;
        }
        else
        {
            PblVector3 vNormal;
            D3DXVec3Normalize(&vNormal, &vCap);
            vCap = vNormal;
        }
        pFront[0].SetPos(PblVector3(vPos.x - vCap.x * fWidth, vPos.y - vCap.y * fWidth, vPos.z - vCap.z * fWidth));
        pFront[0].SetColor(colorEdge);
        pFront[1].SetPos(vPos);
        pFront[1].SetColor(colorEdge);
        pFront += 2;
        pBack[0].SetPos(vPosNext);
        pBack[0].SetColor(colorEdge);
        pBack[-1].SetPos(vPos);
        pBack[-1].SetColor(colorEdge);
        pBack -= 2;
        pThird[0].SetPos(PblVector3(vPosNext.x + vCap.x * fWidth, vPosNext.y + vCap.y * fWidth,
                                    vPosNext.z + vCap.z * fWidth));
        pThird[0].SetColor(colorEdge);
        pThird += 1;

        bool bCoreDone = !pTrail->m_bTrailWasOn;
        float fTimePrev = pTrail->m_afTime[(pTrail->m_uiSampleNext - pTrail->m_uiNumSamples) & 7];
        float fAgePrev = pTrail->m_fTimer - fTimePrev;
        float fDist = 0.0f;
        for (unsigned int i = 1; i < pTrail->m_uiNumSamples; ++i)
        {
            uiSample = (i + pTrail->m_uiSampleNext - pTrail->m_uiNumSamples) & 7;
            const float fSampleDist = pTrail->m_afDist[uiSample];
            const bool bMoved = fSampleDist > 0.0f;
            bool bNextVisible;
            if (i == pTrail->m_uiNumSamples - 1)
            {
                bNextVisible = pTrail->m_bTrailWasOn;
            }
            else
            {
                bNextVisible = auiNumSteps[i + 1] != 0;
            }
            const float fSampleTime = pTrail->m_afTime[uiSample];
            const int iNumSteps = auiNumSteps[i];
            const float fStepInv = 1.0f / iNumSteps;
            const float fTimeStep = (fSampleTime - fTimePrev) * fStepInv;
            const float fDistStep = fStepInv * fSampleDist;
            for (int j = 0; j < iNumSteps; ++j)
            {
                fTime = fTimePrev + fTimeStep;
                fDist += fDistStep;
                RedColor color = colorTrail;
                const float fAge = pTrail->m_fTimer - fTime;
                pTrail->GetInterpolatedDir(fTime, &vDir);
                fDot = vDir.x * vCamZ.x + vDir.y * vCamZ.y + vDir.z * vCamZ.z;
                vCap = PblVector3(vDir.x - fDot * vCamZ.x, vDir.y - fDot * vCamZ.y, vDir.z - fDot * vCamZ.z);
                if (vCap.x * vCap.x + vCap.y * vCap.y + vCap.z * vCap.z <= 0.01f)
                {
                    vCap.x *= 10.0f;
                    vCap.y *= 10.0f;
                    vCap.z *= 10.0f;
                }
                else
                {
                    PblVector3 vNormal;
                    D3DXVec3Normalize(&vNormal, &vCap);
                    vCap = vNormal;
                }

                if (bCoreDone || m_fLIGHTSABER_TRAIL_CORE_MAX_TIME < fAge ||
                    fDist <= fTotalDist - m_fLIGHTSABER_TRAIL_CORE_MAX_DISTANCE)
                {
                    PblVector3 vResult;
                    vPos = *pTrail->GetInterpolatedPos(fTime, &vResult);
                    vPosNext = PblVector3(vPos.x + vDir.x * fLength, vPos.y + vDir.y * fLength,
                                          vPos.z + vDir.z * fLength);
                }
                else
                {

                    PblVector3 vPosNew;
                    pTrail->GetInterpolatedPos(fTime, &vPosNew);
                    const PblVector3 vPosNextNew(vPos.x + vDir.x * fLength, vPos.y + vDir.y * fLength,
                                                 vPos.z + vDir.z * fLength);
                    const float fPosMove = sqrtf((vPosNew.x - vPos.x) * (vPosNew.x - vPos.x) +
                                                 (vPosNew.y - vPos.y) * (vPosNew.y - vPos.y) +
                                                 (vPosNew.z - vPos.z) * (vPosNew.z - vPos.z));
                    const float fTipMove = sqrtf((vPosNextNew.x - vPosNext.x) * (vPosNextNew.x - vPosNext.x) +
                                                 (vPosNextNew.y - vPosNext.y) * (vPosNextNew.y - vPosNext.y) +
                                                 (vPosNextNew.z - vPosNext.z) * (vPosNextNew.z - vPosNext.z));
                    float fCore;
                    if (fAgePrev <= m_fLIGHTSABER_TRAIL_CORE_MAX_TIME)
                    {
                        fCore = (fDist - (fTotalDist - m_fLIGHTSABER_TRAIL_CORE_MAX_DISTANCE)) /
                                (fPosMove > fTipMove ? fPosMove : fTipMove);
                        if (fCore > 1.0f)
                        {
                            fCore = 1.0f;
                        }
                    }
                    else
                    {
                        fCore = (m_fLIGHTSABER_TRAIL_CORE_MAX_TIME - fAge) / (fAgePrev - fAge);
                    }
                    float fPosWidth = 0.0f;
                    if (fPosMove > 0.01f)
                    {
                        fPosWidth = m_fLIGHTSABER_TRAIL_CORE_WIDTH * fWidth * 0.5f / fPosMove;
                    }
                    float fTipWidth = 0.0f;
                    if (fTipMove > 0.01f)
                    {
                        fTipWidth = m_fLIGHTSABER_TRAIL_CORE_WIDTH * fWidth * 0.5f / fTipMove;
                    }

                    const PblVector3 vPosDelta(vPos.x - vPosNew.x, vPos.y - vPosNew.y, vPos.z - vPosNew.z);
                    const PblVector3 vTipDelta(vPosNext.x - vPosNextNew.x, vPosNext.y - vPosNextNew.y,
                                               vPosNext.z - vPosNextNew.z);
                    float fPos = fPosWidth + fCore;
                    float fTip = fTipWidth + fCore;
                    PblVector3 vCorePos(vPosDelta.x * fPos + vPosNew.x, vPosDelta.y * fPos + vPosNew.y,
                                        vPosDelta.z * fPos + vPosNew.z);
                    PblVector3 vCoreTip(vTipDelta.x * fTip + vPosNextNew.x, vTipDelta.y * fTip + vPosNextNew.y,
                                        vTipDelta.z * fTip + vPosNextNew.z);
                    pFront[0].SetPos(PblVector3(vCorePos.x - vCap.x * fWidth, vCorePos.y - vCap.y * fWidth,
                                                vCorePos.z - vCap.z * fWidth));
                    pFront[0].SetColor(colorEdge);
                    pFront[1].SetPos(vCorePos);
                    pFront[1].SetColor(colorTrail);
                    pBack[0].SetPos(vCoreTip);
                    pBack[0].SetColor(colorTrail);
                    pBack[-1].SetPos(vCorePos);
                    pBack[-1].SetColor(colorTrail);
                    pBack -= 2;
                    pThird[0].SetPos(vCoreTip);
                    pThird[0].SetColor(colorTrail);
                    pThird[1].SetPos(PblVector3(vCoreTip.x + vCap.x * fWidth, vCoreTip.y + vCap.y * fWidth,
                                                vCoreTip.z + vCap.z * fWidth));
                    pThird[1].SetColor(colorEdge);
                    pThird += 2;

                    if (i == pTrail->m_uiNumSamples - 1 && j == iNumSteps - 1)
                    {
                        if (fPosWidth > fCore)
                        {
                            fPosWidth = fCore;
                        }
                        if (fTipWidth > fCore)
                        {
                            fTipWidth = fCore;
                        }
                    }
                    fPos = fCore - fPosWidth;
                    fTip = fCore - fTipWidth;
                    vCorePos = PblVector3(vPosDelta.x * fPos + vPosNew.x, vPosDelta.y * fPos + vPosNew.y,
                                          vPosDelta.z * fPos + vPosNew.z);
                    vCoreTip = PblVector3(vTipDelta.x * fTip + vPosNextNew.x, vTipDelta.y * fTip + vPosNextNew.y,
                                          vTipDelta.z * fTip + vPosNextNew.z);
                    pFront[2].SetPos(PblVector3(vCorePos.x - vCap.x * fWidth, vCorePos.y - vCap.y * fWidth,
                                                vCorePos.z - vCap.z * fWidth));
                    pFront[2].SetColor(colorEdge);
                    pFront[3].SetPos(vCorePos);
                    pFront[3].SetColor(colorHead);
                    pFront += 4;
                    pBack[0].SetPos(vCoreTip);
                    pBack[0].SetColor(colorHead);
                    pBack[-1].SetPos(vCorePos);
                    pBack[-1].SetColor(colorHead);
                    pBack -= 2;
                    pThird[0].SetPos(vCoreTip);
                    pThird[0].SetColor(colorHead);
                    pThird[1].SetPos(PblVector3(vCoreTip.x + vCap.x * fWidth, vCoreTip.y + vCap.y * fWidth,
                                                vCoreTip.z + vCap.z * fWidth));
                    pThird[1].SetColor(colorEdge);
                    pThird += 2;

                    vPos = fPos > 0.0f ? vPosNew : vCorePos;
                    vPosNext = fTip > 0.0f ? vPosNextNew : vCoreTip;
                    bCoreDone = true;
                }

                if (bCoreDone && pTrail->m_bTrailWasOn)
                {
                    color = colorHead;
                }
                else if (fAge < fPersistTime && bMoved && (bNextVisible || j != iNumSteps - 1))
                {
                    float fFade = 1.0f;
                    if (fAge > fFadeTime)
                    {
                        fFade = (fPersistTime - fAge) * fFadeTimeInv * ((fPersistTime - fAge) * fFadeTimeInv);
                    }
                    if (fFadeDist > fDist)
                    {
                        fFade = fDist * fFadeDistInv * fFade;
                    }
                    if (fFade < 1.0f)
                    {
                        color.a = (unsigned char)(int)(color.a * fFade);
                    }
                }
                else
                {
                    color.a = 0;
                }

                pFront[0].SetPos(PblVector3(vPos.x - vCap.x * fWidth, vPos.y - vCap.y * fWidth, vPos.z - vCap.z * fWidth));
                pFront[0].SetColor(colorEdge);
                pFront[1].SetPos(vPos);
                pFront[1].SetColor(color);
                pFront += 2;
                pBack[0].SetPos(vPosNext);
                pBack[0].SetColor(color);
                pBack[-1].SetPos(vPos);
                pBack[-1].SetColor(color);
                pBack -= 2;
                pThird[0].SetPos(vPosNext);
                pThird[0].SetColor(color);
                pThird[1].SetPos(PblVector3(vPosNext.x + vCap.x * fWidth, vPosNext.y + vCap.y * fWidth,
                                            vPosNext.z + vCap.z * fWidth));
                pThird[1].SetColor(colorEdge);
                pThird += 2;

                fAgePrev = fAge;
                fTimePrev = fTime;
            }
            fTimePrev = fSampleTime;
        }
    }
    pPrim->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrim, m_pShader,
                                   _IdentityMatrix, RedColor::WHITE,
                                   uiFlags, 0, 0, 0);
    pPrim->Release();
}

// FUNCTION: SWBF2 0x00689d10
bool WeaponMelee::WillBecomeBusy() const
{
    return IsBusy();
}

// FUNCTION: SWBF2 0x00689d80
void WeaponMelee::TurnOn(float fDuration)
{
    if (fDuration < 0.0f)
    {
        fDuration = static_cast<const WeaponMeleeClass*>(mClass)->m_fTurnOnDuration;
    }

    if (m_bLightsaberOn)
    {
        if (m_fTurnOnDuration > m_fTurnOnTimer && m_fTurnOnDuration > fDuration)
        {
            m_fTurnOnTimer = fDuration / m_fTurnOnDuration * m_fTurnOnTimer;
            m_fTurnOnDuration = fDuration;
        }
    }
    else
    {
        if (m_fTurnOnDuration <= m_fTurnOnTimer)
        {
            m_fTurnOnTimer = 0.0f;
        }
        else
        {
            m_fTurnOnTimer = (1.0f - m_fTurnOnTimer / m_fTurnOnDuration) * fDuration;
        }
        m_bLightsaberOn = 1;
        m_fTurnOnDuration = fDuration;
    }
}

// FUNCTION: SWBF2 0x00689e30
void WeaponMelee::TurnOff(float fDuration)
{
    if (m_pMeleeThrow && m_pMeleeThrow->m_bThrow)
    {
        return;
    }

    if (fDuration < 0.0f)
    {
        fDuration = static_cast<const WeaponMeleeClass*>(mClass)->m_fTurnOnDuration;
    }

    if (!m_bLightsaberOn)
    {
        if (m_fTurnOnDuration > m_fTurnOnTimer && m_fTurnOnDuration > fDuration)
        {
            m_fTurnOnTimer = fDuration / m_fTurnOnDuration * m_fTurnOnTimer;
            m_fTurnOnDuration = fDuration;
        }
    }
    else
    {
        if (m_fTurnOnDuration <= m_fTurnOnTimer)
        {
            m_fTurnOnTimer = 0.0f;
        }
        else
        {
            m_fTurnOnTimer = (1.0f - m_fTurnOnTimer / m_fTurnOnDuration) * fDuration;
        }
        m_bLightsaberOn = 0;
        m_fTurnOnDuration = fDuration;
    }
}

// FUNCTION: SWBF2 0x00689d20
float WeaponMelee::GetLightSaberLengthFactor() const
{
    float fTimer = m_fTurnOnTimer;
    float fDuration = m_fTurnOnDuration;
    if (m_bLightsaberOn)
    {
        return fDuration <= fTimer ? 1.0f : fTimer / fDuration;
    }
    return fDuration <= fTimer ? 0.0f : 1.0f - fTimer / fDuration;
}

// FUNCTION: SWBF2 0x00689f00
bool WeaponMelee::IsOn(unsigned int uiBlade) const
{
    if (uiBlade >= static_cast<const WeaponMeleeClass*>(mClass)->m_uiNumWeapons ||
        (!m_bLightsaberOn && m_fTurnOnDuration <= m_fTurnOnTimer))
    {
        return false;
    }
    unsigned int uiMask = 1u << uiBlade;
    return (m_uiLightsaberOnMask & uiMask) != 0 ||
           ((m_uiLightsaberWasOnMask & uiMask) != 0 && m_fComboStateTimer < 0.1f);
}

// FUNCTION: SWBF2 0x00689f80
float WeaponMelee::GetLightSaberLengthFactor(unsigned int uiBlade) const
{
    if (uiBlade >= static_cast<const WeaponMeleeClass*>(mClass)->m_uiNumWeapons)
    {
        return 0.0f;
    }
    float fOn = GetLightSaberLengthFactor();
    if (fOn <= 0.0f)
    {
        return 0.0f;
    }
    unsigned int uiMask = 1u << uiBlade;
    bool bOn = (m_uiLightsaberOnMask & uiMask) != 0;
    bool bWasOn = (m_uiLightsaberWasOnMask & uiMask) != 0;
    if (bOn == bWasOn)
    {
        return (bOn ? 1.0f : 0.0f) * fOn;
    }
    if (bOn)
    {
        return m_fComboStateTimer < 0.1f ?
               m_fComboStateTimer / 0.1f * fOn : fOn;
    }
    return m_fComboStateTimer < 0.1f ?
           fOn - m_fComboStateTimer / 0.1f * fOn : 0.0f;
}

// FUNCTION: SWBF2 0x0068a410
void WeaponMelee::RenderLightsaberTrail(unsigned int uiEdge, const PblMatrix* pMatrix, bool bTrailOn,
                                        unsigned int uiFlags)
{
    WeaponMelee::TrailData** ppData = &m_pTrailData;
    WeaponMelee::TrailData* pSelected = 0;
    while (*ppData)
    {
        WeaponMelee::TrailData* pData = *ppData;
        if (pData->m_pTrail->m_uiNumSamples)
        {
            if (pData->m_uiEdge == uiEdge)
            {
                pSelected = pData;
            }
            ppData = &pData->pNext;
        }
        else
        {
            *ppData = pData->pNext;
            WeaponMelee::_FreeTrailData(pData);
        }
    }

    const WeaponMeleeClass* pClass = static_cast<const WeaponMeleeClass*>(mClass);
    const WeaponMeleeClass::DamageEdge& edge = pClass->m_aDamageEdge[uiEdge];
    if (!pSelected)
    {
        if (!bTrailOn)
        {
            return;
        }
        pSelected = WeaponMelee::_AllocTrailData();
        if (!pSelected)
        {
            return;
        }
        const RedColor colorHead = Interpolate(RedColor::WHITE, edge.colorTrail, 0.5f);
        pSelected->Init(uiEdge, edge.vOffsetPos, edge.vOffsetDir,
                        edge.fLength, edge.fWidth, edge.colorTrail, colorHead);
        pSelected->pNext = m_pTrailData;
        m_pTrailData = pSelected;
    }

    LightsaberTrail* pTrail = pSelected->m_pTrail;
    if (pSelected->m_uiFrameLastUpdated != (unsigned char)RedRenderer::GetFrameNumber())
    {
        pTrail->m_bTrailOn = bTrailOn;
        pTrail->AttachEffectToMatrix(pMatrix, false);
    }
    if (pTrail->m_uiNumSamples)
    {
        LightsaberTrail::s_uiSamplePrev = 0;
        LightsaberTrail::s_uiSample = 0;
        LightsaberTrailEffect::s_pInstance->RenderLightsaberTrail(pTrail, uiFlags);
    }
}

// FUNCTION: SWBF2 0x0068a550
bool WeaponMelee::Deflect(const Ordnance* pOrdnance, const PblVector3& pos, const PblVector3& dir)
{
    const Combo::State& state = m_pCombo->m_aState[m_eComboState];
    const Combo::Deflect* pDeflect = state.m_pDeflect;
    if (!pDeflect)
    {
        return false;
    }

    PblVector3 vVelocity = mOwner->GetGameObject()->GetVelocity();
    vVelocity.y = 0.0f;
    if (vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z > 0.09f)
    {
        PblVector3 vNormal;
        D3DXVec3Normalize(&vNormal, &vVelocity);
        vVelocity = vNormal;
        const PblMatrix& mOwnerMatrix = mOwner->GetMatrix();
        if (vVelocity.x * mOwnerMatrix.m[2][0] + vVelocity.y * mOwnerMatrix.m[2][1] +
                vVelocity.z * mOwnerMatrix.m[2][2] <= 0.0f)
        {
            vVelocity = PblVector3(-vVelocity.x, -vVelocity.y, -vVelocity.z);
        }
    }
    else
    {
        const PblMatrix& mOwnerMatrix = mOwner->GetMatrix();
        vVelocity = PblVector3(mOwnerMatrix.m[2][0], mOwnerMatrix.m[2][1], mOwnerMatrix.m[2][2]);
    }

    unsigned char uiAnim;

    if (!((Combo::Deflect*)pDeflect)->TestDeflect(&mOwner->GetMatrix().m[0][0], &vVelocity.x, &dir.x, reinterpret_cast<int>(&pos), m_uiAnimIndex, &uiAnim))
    {
        m_uiAnimIndex = state.m_uiAnimIndex;
        return false;
    }

    const float fEnergyCost = pDeflect->m_uiEnergyCost * 0.1f;
    EnergyBar* pEnergyBar = mOwner->GetEnergyBar();
    if (pEnergyBar && !pEnergyBar->SpendEnergy(fEnergyCost))
    {
        m_uiAnimIndex = state.m_uiAnimIndex;
        return false;
    }
    m_uiAnimIndex = uiAnim;

    const WeaponMeleeClass* pClass = static_cast<const WeaponMeleeClass*>(mClass);
    PblVector3 vHitPos = pos;
    if (pClass->m_amDeflectMatrix)
    {
        const PblMatrix& mOwnerMatrix = mOwner->GetMatrix();
        PblVector3 vFront(mOwnerMatrix.m[2][0], mOwnerMatrix.m[2][1], mOwnerMatrix.m[2][2]);
        AlignVectorToThrust(vFront, false);
        const PblMatrix& mUpMatrix = mOwner->GetMatrix();
        PblVector3 vSide(mUpMatrix.m[1][1] * vFront.z - mUpMatrix.m[1][2] * vFront.y,
                         mUpMatrix.m[1][2] * vFront.x - mUpMatrix.m[1][0] * vFront.z,
                         mUpMatrix.m[1][0] * vFront.y - mUpMatrix.m[1][1] * vFront.x);
        PblVector3 vRight;
        D3DXVec3Normalize(&vRight, &vSide);
        PblVector3 vUp(vFront.y * vRight.z - vFront.z * vRight.y,
                       vFront.z * vRight.x - vFront.x * vRight.z,
                       vFront.x * vRight.y - vFront.y * vRight.x);
        const PblMatrix& mOriginMatrix = mOwner->GetMatrix();

        PblMatrix mDeflect;
        mDeflect.m[0][0] = vRight.x;
        mDeflect.m[0][1] = vRight.y;
        mDeflect.m[0][2] = vRight.z;
        mDeflect.m[0][3] = 0.0f;
        mDeflect.m[1][0] = vUp.x;
        mDeflect.m[1][1] = vUp.y;
        mDeflect.m[1][2] = vUp.z;
        mDeflect.m[1][3] = 0.0f;
        mDeflect.m[2][0] = vFront.x;
        mDeflect.m[2][1] = vFront.y;
        mDeflect.m[2][2] = vFront.z;
        mDeflect.m[2][3] = 0.0f;
        mDeflect.m[3][0] = mOriginMatrix.m[3][0];
        mDeflect.m[3][1] = mOriginMatrix.m[3][1];
        mDeflect.m[3][2] = mOriginMatrix.m[3][2];
        mDeflect.m[3][3] = 1.0f;

        for (unsigned int i = 0; i < pDeflect->m_uiNumDeflectAnimations; ++i)
        {
            if (pDeflect->m_aDeflectAnim[i].uiAnimIndex != uiAnim)
            {
                continue;
            }

            PblMatrix mBlock;
            D3DXMatrixMultiply(&mBlock, &pClass->m_amDeflectMatrix[i], &mDeflect);
            const float dx = pos.x - mBlock.m[3][0];
            const float dy = pos.y - mBlock.m[3][1];
            const float dz = pos.z - mBlock.m[3][2];
            const unsigned int uiWeapon = pDeflect->m_aDeflectAnim[i].uiWeapon;
            PblVector3 vLocal(mBlock.m[0][1] * dy + dx * mBlock.m[0][0] + mBlock.m[0][2] * dz,
                              mBlock.m[1][0] * dx + mBlock.m[1][1] * dy + mBlock.m[1][2] * dz,
                              mBlock.m[2][0] * dx + mBlock.m[2][1] * dy + mBlock.m[2][2] * dz);
            int iEdge = -1;
            float fBestDistSq = FLT_MAX;
            for (unsigned int e = 0; e < pClass->m_uiNumDamageEdges; ++e)
            {
                const WeaponMeleeClass::DamageEdge& edge = pClass->m_aDamageEdge[e];
                if (edge.uiAttachToWeapon != uiWeapon)
                {
                    continue;
                }
                const float fHalf = edge.fLength * 0.5f;
                const float cx = edge.vOffsetPos.x + edge.vOffsetDir.x * fHalf - vLocal.x;
                const float cy = edge.vOffsetPos.y + edge.vOffsetDir.y * fHalf - vLocal.y;
                const float cz = edge.vOffsetPos.z + edge.vOffsetDir.z * fHalf - vLocal.z;
                const float fDistSq = cy * cy + cx * cx + cz * cz;
                if (fDistSq < fBestDistSq)
                {
                    iEdge = e;
                    fBestDistSq = fDistSq;
                }
            }
            if (iEdge == -1)
            {
                vHitPos = PblVector3(mBlock.m[3][0], mBlock.m[3][1], mBlock.m[3][2]);
            }
            else
            {
                const WeaponMeleeClass::DamageEdge& edge = pClass->m_aDamageEdge[iEdge];
                float t = ((edge.vOffsetDir.y * (vLocal.y - edge.vOffsetPos.y) +
                            edge.vOffsetDir.x * (vLocal.x - edge.vOffsetPos.x) +
                            edge.vOffsetDir.z * (vLocal.z - edge.vOffsetPos.z)) /
                               edge.fLength +
                           1.0f) *
                          0.5f;
                if (t > 1.0f)
                {
                    t = 1.0f;
                }
                else if (t < 0.0f)
                {
                    t = 0.0f;
                }
                const float fAlong = t * edge.fLength;
                PblVector3 vEdgePoint(edge.vOffsetDir.x * fAlong + edge.vOffsetPos.x,
                                      edge.vOffsetPos.y + edge.vOffsetDir.y * fAlong,
                                      edge.vOffsetPos.z + edge.vOffsetDir.z * fAlong);
                D3DXVec3TransformCoord(&vHitPos, &vEdgePoint, &mBlock);
            }
            break;
        }
    }

    if (!pOrdnance)
    {
        m_fDeflectTimer = 0.0f;
    }
    else
    {
        float fSpread = 0.05f;
        if (m_fDeflectTimer < 1.0f)
        {
            fSpread = (1.0f - m_fDeflectTimer) * 0.25f + m_fDeflectTimer * 0.05f;
        }
        else if (m_fComboStateTimer < 0.4f)
        {
            fSpread = 0.02f;
        }

        PblVector3 vDeflectDir;
        if (pOrdnance->IsRtti(OrdnanceBeam::rttiHashOrdnanceBeam))
        {

            PblRandom rand(((unsigned int)pOrdnance ^ (unsigned int)pOrdnance->GetOwner() ^
                          pOrdnance->PblHandled::GetHandleId()) ^ 0xC71C2830u);
            const float fPhase = rand.GetFloat();
            const float fScatter = rand.GetFloat();
            float fSin = 0.0f;
            float fCos = 1.0f;
            PblMath::SinCos(&fSin, &fCos,
                            ((fScatter * 2.0f - 1.0f) * m_fComboStateTimer + fPhase) * 6.2831855f);
            fCos *= fSpread;
            fSin *= fSpread;
            const PblMatrix& mRightMatrix = mOwner->GetMatrix();
            const PblVector3& vAim = mAimer->mDirection;
            PblVector3 vCross((vAim.y * mRightMatrix.m[0][2] - vAim.z * mRightMatrix.m[0][1]) * fSin,
                              (vAim.z * mRightMatrix.m[0][0] - vAim.x * mRightMatrix.m[0][2]) * fSin,
                              (vAim.x * mRightMatrix.m[0][1] - vAim.y * mRightMatrix.m[0][0]) * fSin);
            const PblMatrix& mSideMatrix = mOwner->GetMatrix();
            PblVector3 vScattered(vCross.x + (mSideMatrix.m[0][0] * fCos + vAim.x),
                                  vAim.y + mSideMatrix.m[0][1] * fCos + vCross.y,
                                  vAim.z + mSideMatrix.m[0][2] * fCos + vCross.z);
            D3DXVec3Normalize(&vDeflectDir, &vScattered);

            m_fDeflectTimer -= GameLoop::sClientDeltaTime;
            if (m_fDeflectTimer >= 1.0f)
            {
                m_fDeflectTimer = 1.0f;
            }
            m_fDeflectTimer -= GameLoop::sClientDeltaTime * 2.0f;
            if (m_fDeflectTimer < 0.0f)
            {
                m_fDeflectTimer = 0.0f;
            }
        }
        else
        {
            const float fX = (PblRandom::_Global.GetFloat() * 2.0f - 1.0f) * fSpread;
            const float fY = (PblRandom::_Global.GetFloat() * 2.0f - 1.0f) * fSpread;
            const PblMatrix& mRightMatrix = mOwner->GetMatrix();
            const PblVector3& vAim = mAimer->mDirection;
            PblVector3 vCross((vAim.y * mRightMatrix.m[0][2] - vAim.z * mRightMatrix.m[0][1]) * fY,
                              (vAim.z * mRightMatrix.m[0][0] - vAim.x * mRightMatrix.m[0][2]) * fY,
                              (vAim.x * mRightMatrix.m[0][1] - vAim.y * mRightMatrix.m[0][0]) * fY);
            const PblMatrix& mSideMatrix = mOwner->GetMatrix();
            PblVector3 vScattered(vAim.x + mSideMatrix.m[0][0] * fX + vCross.x,
                                  vAim.y + mSideMatrix.m[0][1] * fX + vCross.y,
                                  vAim.z + mSideMatrix.m[0][2] * fX + vCross.z);
            D3DXVec3Normalize(&vDeflectDir, &vScattered);
            m_fDeflectTimer = 0.0f;
        }
        pOrdnance->CreateRicochet(vHitPos, vDeflectDir, mOwner->GetGameObject());
    }

    TurnOn(0.1f);
    GameObject* pObject = mOwner->GetGameObject();
    pClass->m_DeflectSound.Play(&mOwner->GetGameObject()->GetPosition(), &pObject->GetVelocity(),
                                false, true);
    FLEffectClass* pEffectClass = FLEffect::s_EffectClasses.Find(pClass->m_uiDeflectEffect);
    if (pEffectClass)
    {
        FLEffectObject* pEffect = pEffectClass->BuildEffect();
        if (pEffect)
        {
            pEffect->AttachEffectToVector(&vHitPos, false);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0068b850
int WeaponMelee::_ClearInputQueueToTime(WeaponMelee::InputQueue* pQueue, float fTime)
{
    if (!pQueue->m_uiCount || pQueue->m_aEventQueue[pQueue->m_uiStart & 7].fTime > fTime)
    {
        return 0;
    }
    int iConsumed = 1;
    while (iConsumed < pQueue->m_uiCount &&
           pQueue->m_aEventQueue[(pQueue->m_uiStart + iConsumed) & 7].fTime <= fTime)
    {
        ++iConsumed;
    }
    if (iConsumed == pQueue->m_uiCount)
    {
        pQueue->m_uiCount = 0;
        pQueue->m_uiStart = 0;
    }
    else
    {
        pQueue->m_uiCount -= iConsumed;
        pQueue->m_uiStart = (pQueue->m_uiStart + iConsumed) & 7;
    }
    return iConsumed;
}

// FUNCTION: SWBF2 0x0068b8d0
bool WeaponMelee::TestInputQueueForButtons(int iPressedMask, int iReleasedMask) const
{
    if (!iPressedMask && !iReleasedMask)
    {
        return true;
    }
    const WeaponMelee::InputQueue& queue = m_InputQueue;
    for (unsigned int i = 0; i < queue.m_uiCount; ++i)
    {
        const WeaponMelee::InputQueue::Event& event =
            queue.m_aEventQueue[(queue.m_uiStart + i) & 7];
        int iButton = 1 << event.uiButton;
        if ((iButton & iPressedMask) && !(event.uiButtonAction & 3))
        {
            return true;
        }
        if ((iButton & iReleasedMask) && (event.uiButtonAction & 1))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0068b400
void WeaponMelee::InputQueue::Update(float fTime, unsigned char uiButtons)
{
    for (unsigned char uiButton = 1; uiButton < 8; ++uiButton)
    {
        unsigned char uiMask = 1 << uiButton;
        bool bCurrent = (uiButtons & uiMask) != 0;
        bool bPrevious = (m_uiButtonDown & uiMask) != 0;
        if (!bCurrent && !bPrevious)
        {
            continue;
        }

        WeaponMelee::InputQueue::Event* pRecent = 0;
        for (unsigned int i = 0; i < m_uiCount; ++i)
        {
            unsigned int uiIndex = (m_uiStart + m_uiCount - i - 1) & 7;
            WeaponMelee::InputQueue::Event* pEvent = &m_aEventQueue[uiIndex];
            if (pEvent->uiButton == uiButton)
            {
                pRecent = pEvent;
                break;
            }
        }

        int iAction = -1;
        if (bCurrent)
        {
            if (bPrevious)
            {
                iAction = 2;
                if (pRecent)
                {
                    unsigned char uiOld = pRecent->uiButtonAction;
                    if (uiOld & 1)
                    {
                        iAction = 2;
                    }
                    else if ((uiOld & 2) || fTime <= pRecent->fTime + 0.3f)
                    {
                        iAction = -1;
                    }
                    else
                    {
                        iAction = uiOld | 2;
                    }
                }
            }
            else
            {
                iAction = 0;
                if (pRecent && (pRecent->uiButtonAction & 3) == 1 &&
                    pRecent->fTime + 0.25f >= fTime)
                {
                    iAction = 4;
                }
                m_uiButtonDown |= uiMask;
            }
        }
        else
        {
            if (pRecent && !(pRecent->uiButtonAction & 1))
            {
                unsigned char uiOld = pRecent->uiButtonAction;
                iAction = uiOld | ((uiOld & 2) || fTime > pRecent->fTime + 0.3f ? 3 : 1);
            }
            m_uiButtonDown &= ~uiMask;
        }
        if (iAction >= 0)
        {
            unsigned int uiIndex = (m_uiStart + m_uiCount) & 7;
            WeaponMelee::InputQueue::Event& event = m_aEventQueue[uiIndex];
            event.fTime = fTime;
            event.uiButton = uiButton;
            event.uiButtonAction = static_cast<unsigned char>(iAction);
            if (m_uiCount >= 8)
            {
                ++m_uiStart;
            }
            else
            {
                ++m_uiCount;
            }
        }
    }
}

// FUNCTION: SWBF2 0x0068b5f0
bool WeaponMelee::_TestInputQueueForButtonEvent(int iButton, WeaponMelee::InputQueue* pQueue, float fTime,
                int iAction, float fBegin, float fEnd, float* pfMatch)
{
    if (iAction == 10 || iAction == 8)
    {
        int iLastRelease = -1;
        bool bOpen = false;
        for (unsigned int i = 0; i < pQueue->m_uiCount; ++i)
        {
            const WeaponMelee::InputQueue::Event& event =
                pQueue->m_aEventQueue[(pQueue->m_uiStart + i) & 7];
            if (event.uiButton != iButton)
            {
                continue;
            }
            if (event.fTime > fEnd)
            {
                break;
            }
            if (event.fTime < fBegin)
            {
                if (iAction == 10)
                {
                    iLastRelease = (event.uiButtonAction & 1) ? -1 : i;
                }
                else
                {
                    iLastRelease = (event.uiButtonAction & 1) ? i : -1;
                }
                continue;
            }
            if (((event.uiButtonAction & 1) != 0) == (iAction == 8))
            {
                *pfMatch = event.fTime;
                return true;
            }
            if (iAction == 10 || !(event.uiButtonAction & 2))
            {
                bOpen = true;
                break;
            }
        }
        if (iLastRelease >= 0 || bOpen)
        {
            *pfMatch = fBegin;
            return true;
        }
        const bool bDown = ((1u << iButton) & pQueue->m_uiButtonDown) != 0;
        if (fTime >= fBegin && fTime <= fEnd && bDown == (iAction == 10))
        {
            *pfMatch = fBegin;
            return true;
        }
        return false;
    }

    unsigned int uiRequired = 0;
    unsigned int uiMask = 0;
    if (iAction == 9)
    {
        uiRequired = uiMask = 1;
    }
    else
    {
        uiRequired = iAction & 7;
        uiMask = 2 | (iAction & 5);
    }
    for (unsigned int i = 0; i < pQueue->m_uiCount; ++i)
    {
        const WeaponMelee::InputQueue::Event& event =
            pQueue->m_aEventQueue[(pQueue->m_uiStart + i) & 7];
        if (event.uiButton == iButton && (event.uiButtonAction & uiMask) == uiRequired)
        {
            if (event.fTime > fEnd)
            {
                return false;
            }
            if (fBegin <= event.fTime)
            {
                *pfMatch = event.fTime;
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0068b980
bool WeaponMelee::TestConditionExceptInputQueue(float fPreviousTime, float fCurrentTime,
                const Combo::Condition* pCondition)
{
    float fStart = pCondition->m_iTimeStart * 0.01f;
    if (pCondition->m_bTimeStartFromEnd)
    {
        fStart += fPreviousTime;
    }
    if (fStart > fCurrentTime)
    {
        return false;
    }

    float fEnd = pCondition->m_iTimeEnd * 0.01f;
    if (pCondition->m_bTimeEndFromEnd)
    {
        fEnd += fPreviousTime;
    }
    if (fCurrentTime > fEnd)
    {
        return false;
    }

    if (!pCondition->TestSoldierState(m_iSoldierState, m_fSoldierStateTimer))
    {
        return false;
    }
    EnergyBar* pEnergy = mOwner->GetEnergyBar();
    const float fEnergy = pEnergy ? pEnergy->GetEnergy() : 0.0f;
    if (!pCondition->TestEnergy(fEnergy))
    {
        return false;
    }
    if (!pCondition->TestInputThrust(m_fInputThrustZ, -m_fInputThrustX))
    {
        return false;
    }
    if (!pCondition->TestVelocity(m_vVelocityLocal.x,
                                  m_vVelocityLocal.y,
                                  m_vVelocityLocal.z))
    {
        return false;
    }
    if (pCondition->m_uiInputButtonCnd &&
        !TestInputQueueForButtons(pCondition->m_uiInputButtonsPressed,
                    pCondition->m_uiInputButtonsReleased))
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x0068baa0
bool WeaponMelee::TestCondition(float fPreviousTime, float fCurrentTime,
                const Combo::Condition* pCondition, float* pfMatchTime)
{
    if (!TestConditionExceptInputQueue(fPreviousTime, fCurrentTime, pCondition))
    {
        return false;
    }

    float fStart = pCondition->m_iTimeStart * 0.01f;
    if (pCondition->m_bTimeStartFromEnd)
    {
        fStart += fPreviousTime;
    }
    if (pCondition->m_uiButton)
    {
        float fEnd = pCondition->m_iTimeEnd * 0.01f;
        if (pCondition->m_bTimeEndFromEnd)
        {
            fEnd += fPreviousTime;
        }
        const float fOffset = m_fInputTimer - fCurrentTime;
        if (!WeaponMelee::_TestInputQueueForButtonEvent(pCondition->m_uiButton, &m_InputQueue,
                         m_fInputTimer, pCondition->m_uiButtonAction,
                         fStart + fOffset, fEnd + fOffset, pfMatchTime))
        {
            return false;
        }
        *pfMatchTime -= fOffset;
    }
    else
    {
        *pfMatchTime = fStart;
    }
    return true;
}

// FUNCTION: SWBF2 0x00689cb0
bool WeaponMelee::IsBusy() const
{
    const Combo::State& state = m_pCombo->m_aState[m_eComboState];
    if (!(state.m_uiInputLock & 4))
    {
        return false;
    }
    float fLockTime = state.m_uiInputLockDuration * 0.01f;
    return fLockTime <= 0.0f || fLockTime > m_fComboStateTimer;
}

// FUNCTION: SWBF2 0x0068b140
bool WeaponMelee::AlignVectorToThrust(PblVector3& v, bool bInitial) const
{
    float fX = m_fAlignX;
    float fZ = m_fAlignZ;
    float fLengthSquared = fX * fX + fZ * fZ;
    if (fLengthSquared < 0.09f)
    {
        return false;
    }
    if (!bInitial)
    {
        fX = -fX;
    }
    if (m_fInputThrustZ < 0.0f)
    {
        fX = -fX;
        fZ = -fZ;
    }

    float fCos, fSin;
    if (fX >= fZ)
    {
        fCos = 0.70709997f;
        fSin = 0.70709997f;
    }
    else if (-fX >= fZ)
    {
        fCos = 0.70709997f;
        fSin = -0.70709997f;
    }
    else
    {
        float fInvLength = 1.0f / static_cast<float>(sqrt(static_cast<double>(fLengthSquared)));
        fCos = fInvLength * fZ;
        fSin = fInvLength * fX;
    }
    float fOldX = v.x;
    float fOldZ = v.z;
    v.x = fOldX * fCos + fOldZ * fSin;
    v.z = fOldZ * fCos - fOldX * fSin;
    return true;
}

// FUNCTION: SWBF2 0x0068b2c0
Trigger* WeaponMelee::GetButton(unsigned char uiButton)
{
    switch (uiButton)
    {
    case 1: return &mOwner->mControlFire[0];
    case 2: return &mOwner->mControlFire[1];
    case 3: return &mOwner->mControlJump;
    case 4: return &mOwner->mControlSprint;
    case 5: return &mOwner->mControlCrouch;
    case 6: return &mOwner->mControlReload;
    default: return 0;
    }
}

// FUNCTION: SWBF2 0x0068b330
void WeaponMelee::ClearInput()
{
    m_fInputThrustX = 0.0f;
    m_fInputThrustZ = 0.0f;
    m_fInputTimer = 0.0f;
    m_uiButtonDown = 0;
    m_InputQueue.m_uiCount = 0;
    m_InputQueue.m_uiStart = 0;
    m_InputQueue.m_uiButtonDown = 0;
}

// FUNCTION: SWBF2 0x0068b370
void WeaponMelee::ReadButtons()
{
    if (m_bSoldierInputLock)
    {
        m_uiButtonDown = 0;
        return;
    }
    for (unsigned char uiButton = 1; uiButton < 8; ++uiButton)
    {
        unsigned char uiMask = 1 << uiButton;
        bool bDown = (m_bAI_Deflect && uiButton == m_pCombo->m_uiDeflectButton) ||
                     (uiButton == 7 ? (m_uiButtonDown & 6) == 6 : (GetButton(uiButton)->m_uiButtonState & 1) != 0);
        if (bDown)
        {
            m_uiButtonDown |= uiMask;
        }
        else
        {
            m_uiButtonDown &= ~uiMask;
        }
    }
}

// FUNCTION: SWBF2 0x0068bc00
void WeaponMelee::ReleaseDamageData()
{
    while (m_pDamageData)
    {
        DamageData* pData = m_pDamageData;
        m_pDamageData = pData->pNext;
        FreeDamageData(pData);
    }
}

// FUNCTION: SWBF2 0x0068b100
void WeaponMelee::SetNetworkState(ComboState eState, bool bAnimatedMove)
{
    if (eState != m_eComboState)
    {
        ExitState();
        EnterState(eState);
    }
    m_bAnimatedMove = bAnimatedMove;
}

// FUNCTION: SWBF2 0x0068bc30
void WeaponMelee::EnterState(ComboState eState)
{
    const Combo::State& state = m_pCombo->m_aState[eState];
    const WeaponMeleeClass* pClass = static_cast<const WeaponMeleeClass*>(mClass);
    m_eComboState = eState;
    m_fComboStateTimer = 0.0f;
    m_uiAnimIndex = state.m_uiAnimIndex;
    m_bAnimatedMove = state.m_bAnimatedMove;
    m_bGravityDone = 0;
    m_uiLightsaberWasOnMask = m_uiLightsaberOnMask;
    m_bUnreachableState_WeaponUpdate = 0;
    m_bUnreachableState_SoldierUpdate = 0;
    const unsigned int uiBladeMask = (1u << pClass->m_uiNumWeapons) - 1;
    m_uiLightsaberOnMask = uiBladeMask & ~state.m_uiLightsaberOffMask;
    m_bRestartAnimation = state.m_bRestartAnimation;

    if (state.m_bPlayExplosion && pClass->mImpactExplosion &&
        (!(netInShell ? netEnabled : netEnabledNext) || netIsLocalTurn))
    {
        GameObject* pObject = mOwner->GetGameObject();
        DamageOwner owner(mOwner->mCharacter, pObject, pObject->GetTeam(),
                          0, mClass, 1.0f);
        const PblVector3& vOwnerPos = mOwner->GetMatrix().GetTranslation();
        PblVector3 vPos(vOwnerPos.x, vOwnerPos.y + 0.2f, vOwnerPos.z);
        pClass->mImpactExplosion->CreateExplosion(vPos, owner, false, COLL_UNKNOWN);
    }

    if (state.m_sound.mSoundDescription && state.m_uiSoundTime == 0)
    {
        GameObject* pObject = mOwner->GetGameObject();
        state.m_sound.Play(&pObject->GetPosition(), &pObject->GetVelocity(), false, true);
    }

    if (eState == COMBO_IDLE)
    {
        m_InputQueue.m_uiCount = 0;
        m_InputQueue.m_uiStart = 0;
        mState = (WeaponState)MELEE_IDLE;
        EnterIdle();
    }
    else if (state.m_uiNumAttacks)
    {
        mState = (WeaponState)MELEE_ATTACK;
        if (state.m_pDeflect)
        {
            EnterDeflect();
        }
        EnterFire();
    }
    else if (state.m_pDeflect)
    {
        mState = (WeaponState)MELEE_DEFLECT;
        EnterDeflect();
    }
    else
    {
        mState = (WeaponState)MELEE_RECOVER;
        EnterRecover();
    }
}

// FUNCTION: SWBF2 0x0068bfa0
bool WeaponMelee::UpdateState(float fDeltaTime)
{
    const Combo::State& state = m_pCombo->m_aState[m_eComboState];
    if (state.m_sound.mSoundDescription)
    {
        float fSoundTime = state.m_uiSoundTime * 0.01f;
        if (fSoundTime > m_fComboStateTimer - fDeltaTime &&
            m_fComboStateTimer >= fSoundTime)
        {
            GameObject* pObject = mOwner->GetGameObject();
            state.m_sound.Play(&pObject->GetPosition(), &pObject->GetVelocity(), false, true);
        }
    }

    bool bResult;
    switch (GetMeleeState())
    {
    case MELEE_ATTACK:
        if (state.m_pDeflect)
        {
            UpdateDeflect(fDeltaTime);
            mStateTimer -= fDeltaTime;
        }
        bResult = UpdateFire(fDeltaTime);
        break;
    case MELEE_DEFLECT:
        bResult = UpdateDeflect(fDeltaTime);
        break;
    case MELEE_RECOVER:
        bResult = UpdateRecover(fDeltaTime);
        break;
    default:
        bResult = UpdateIdle(fDeltaTime);
        break;
    }

    DamageData** ppData = &m_pDamageData;
    while (*ppData)
    {
        DamageData* pData = *ppData;
        if (m_fComboStateTimer >= pData->fTime + 0.3f)
        {
            *ppData = pData->pNext;
            FreeDamageData(pData);
        }
        else
        {
            ppData = &pData->pNext;
        }
    }
    return bResult;
}

// FUNCTION: SWBF2 0x0068bef0
void WeaponMelee::ExitState()
{
    switch (GetMeleeState())
    {
    case MELEE_ATTACK:
        ExitFire();
        if (m_pCombo->m_aState[m_eComboState].m_pDeflect)
        {
            ExitDeflect();
        }
        break;
    case MELEE_DEFLECT:
        ExitDeflect();
        break;
    case MELEE_RECOVER:
        ExitRecover();
        break;
    default:
        ExitIdle();
        break;
    }
    for (DamageData* pData = m_pDamageData; pData; pData = pData->pNext)
    {
        pData->fTime -= m_fComboStateTimer;
    }
}

// FUNCTION: SWBF2 0x0068c140
void WeaponMelee::EnterIdle()
{
    mStateTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0068c150
void WeaponMelee::ExitIdle()
{
    mStateTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0068c160
bool WeaponMelee::UpdateIdle(float fDeltaTime)
{
    bool bDeactivate = mDeactivateScheduled != 0;
    mStateTimer += fDeltaTime;
    if (bDeactivate)
    {
        DeactivateThread();
    }
    return true;
}

// FUNCTION: SWBF2 0x0068c210
void WeaponMelee::ExitFire()
{
    m_uiTrailOnMask = 0;
    mStateTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0068c230
bool WeaponMelee::UpdateFire(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    const Combo::State& state = m_pCombo->m_aState[m_eComboState];
    if (!state.m_uiNumAttacks)
    {
        return true;
    }
    const WeaponMeleeClass* pClass = static_cast<const WeaponMeleeClass*>(mClass);
    const unsigned int uiNumEdges = pClass->m_uiNumDamageEdges;
    if (!uiNumEdges)
    {
        return true;
    }

    m_uiTrailOnMask = 0;
    for (unsigned int uiEdge = 0; uiEdge < uiNumEdges; ++uiEdge)
    {
        float fStart = FLT_MAX;
        float fEnd = -FLT_MAX;
        for (unsigned int a = 0; a < state.m_uiNumAttacks; ++a)
        {
            const Combo::Attack& attack = state.m_aAttack[a];
            if (attack.m_uiDamageEdge != uiEdge)
            {
                continue;
            }

            const float fAttackStart = attack.m_uiDamageTimeStart * 0.01f - 0.0f;
            const float fAttackEnd = attack.m_bDamageTimeStartFromAnim
                                         ? m_fComboStateTimer
                                         : attack.m_uiDamageTimeEnd * 0.01f + 0.05f;
            if (fAttackStart < fStart)
            {
                fStart = fAttackStart;
            }
            if (fAttackEnd > fEnd)
            {
                fEnd = fAttackEnd;
            }
            if (m_fComboStateTimer >= fStart && fEnd >= m_fComboStateTimer)
            {
                m_uiTrailOnMask |= 1 << uiEdge;
                break;
            }
        }
    }

    const float fPrevTime = m_fComboStateTimer - fDeltaTime;
    bool bMatrixReady = false;
    bool bHit = false;
    PblMatrix mAttack;
    for (unsigned int a = 0; a < state.m_uiNumAttacks; ++a)
    {
        const Combo::Attack& attack = state.m_aAttack[a];

        const float fDamage = attack.m_uiDamage * 1.0f;
        const float fPush = attack.m_uiSpecialType == 0 ? attack.m_uiSpecial * 0.1f : 0.0f;
        const float fStart = attack.m_uiDamageTimeStart * 0.01f;
        const float fEnd = attack.m_bDamageTimeStartFromAnim ? FLT_MAX : attack.m_uiDamageTimeEnd * 0.01f;
        if (fEnd < fPrevTime || m_fComboStateTimer <= fStart)
        {
            continue;
        }
        const unsigned int uiEdge = attack.m_uiDamageEdge;
        if (uiEdge >= uiNumEdges)
        {
            continue;
        }

        DamageData* pData;
        for (pData = m_pDamageData; pData; pData = pData->pNext)
        {
            const Combo::Attack* pOther = static_cast<const Combo::Attack*>(pData->pAttack);
            if (pOther == &attack)
            {
                break;
            }
            if (attack.m_uiAttackId && attack.m_uiAttackId == pOther->m_uiAttackId)
            {
                break;
            }
        }
        if (!pData)
        {
            pData = AllocDamageData();
            if (!pData)
            {
                continue;
            }
            pData->fTime = fEnd;
            pData->pAttack = const_cast<Combo::Attack*>(&attack);
            pData->iNumObjects = 0;
            pData->pNext = m_pDamageData;
            m_pDamageData = pData;
        }

        const WeaponMeleeClass::DamageEdge& edge = pClass->m_aDamageEdge[uiEdge];
        float fLength = attack.m_uiDamageLength * 0.01f;
        float fWidth = attack.m_uiDamageWidth * 0.01f;
        if (attack.m_bDamageLengthFromEdge)
        {
            fLength = edge.fLength * fLength;
        }
        if (attack.m_bDamageWidthFromEdge)
        {
            fWidth = edge.fWidth * fWidth;
        }
        const float fRayLength = fLength + 0.2f;

        const MeleeDamageSample* aSample = reinterpret_cast<const MeleeDamageSample*>(attack.m_aDamageSample);
        const unsigned int uiNumSamples = attack.m_uiNumDamageSamples;
        int iFirst = -1;
        PblBox box;
        unsigned int s;
        for (s = 0; s < uiNumSamples; ++s)
        {
            const MeleeDamageSample& sample = aSample[s];
            if (fPrevTime >= sample.fTime && s < uiNumSamples - 1)
            {
                continue;
            }
            if (sample.fTime > m_fComboStateTimer)
            {
                break;
            }
            PblVector3 vStart(sample.vPos.x - sample.vDir.x * 0.2f, sample.vPos.y - sample.vDir.y * 0.2f,
                              sample.vPos.z - sample.vDir.z * 0.2f);
            if (iFirst == -1)
            {
                iFirst = s;
                box._vMin = vStart;
                box._vMax = vStart;
            }
            else
            {
                box.AddPoint(vStart);
            }
            PblVector3 vEnd(sample.vDir.x * fRayLength + vStart.x, sample.vDir.y * fRayLength + vStart.y,
                            sample.vDir.z * fRayLength + vStart.z);
            box.AddPoint(vEnd);
        }
        const unsigned int uiEnd = s;
        if (iFirst == -1)
        {
            continue;
        }

        if (!bMatrixReady)
        {
            if (state.m_bAlignToThrust)
            {
                const PblMatrix& mOwnerMatrix = mOwner->GetMatrix();
                const PblVector3& vAim = mAimer->mDirection;
                PblVector3 vSide(mOwnerMatrix.m[1][1] * vAim.z - mOwnerMatrix.m[1][2] * vAim.y,
                                 mOwnerMatrix.m[1][2] * vAim.x - mOwnerMatrix.m[1][0] * vAim.z,
                                 mOwnerMatrix.m[1][0] * vAim.y - mOwnerMatrix.m[1][1] * vAim.x);
                PblVector3 vRight;
                D3DXVec3Normalize(&vRight, &vSide);
                const PblMatrix& mOriginMatrix = mOwner->GetMatrix();
                PblVector3 vUp(vAim.y * vRight.z - vAim.z * vRight.y, vAim.z * vRight.x - vAim.x * vRight.z,
                               vAim.x * vRight.y - vAim.y * vRight.x);

                mAttack.m[0][0] = vRight.x;
                mAttack.m[0][1] = vRight.y;
                mAttack.m[0][2] = vRight.z;
                mAttack.m[0][3] = 0.0f;
                mAttack.m[1][0] = vUp.x;
                mAttack.m[1][1] = vUp.y;
                mAttack.m[1][2] = vUp.z;
                mAttack.m[1][3] = 0.0f;
                mAttack.m[2][0] = vAim.x;
                mAttack.m[2][1] = vAim.y;
                mAttack.m[2][2] = vAim.z;
                mAttack.m[2][3] = 0.0f;
                mAttack.m[3][0] = mOriginMatrix.m[3][0];
                mAttack.m[3][1] = mOriginMatrix.m[3][1];
                mAttack.m[3][2] = mOriginMatrix.m[3][2];
                mAttack.m[3][3] = 1.0f;
            }
            else
            {
                mAttack = mOwner->GetMatrix();
            }
            bMatrixReady = true;
        }

        PblVector3 vLocalCenter((box._vMax.x + box._vMin.x) * 0.5f, (box._vMax.y + box._vMin.y) * 0.5f,
                                (box._vMax.z + box._vMin.z) * 0.5f);
        PblVector3 vCenter;
        D3DXVec3TransformCoord(&vCenter, &vLocalCenter, &mAttack);
        const float dx = box._vMax.x - box._vMin.x;
        const float dy = box._vMax.y - box._vMin.y;
        const float dz = box._vMax.z - box._vMin.z;
        const float fRadius = sqrtf(dx * dx + dy * dy + dz * dz) * 0.5f;
        GameObject* apObjects[8];
        const int iNumObjects =
            CollisionManager::GetObjectsInRange(vCenter, fRadius, apObjects, 8, mOwner->GetGameObject(), 0x3a);

        FLEffectClass* pHitEffectClass = 0;
        GameObject* pOwnerObject = mOwner->GetGameObject();
        DamageDesc damage(DamageOwner(mOwner->mCharacter, mOwner->GetGameObject(),
                                      mOwner->GetGameObject()->GetTeam(), pOwnerObject->GetEntityClass(),
                                      mClass, 1.0f),
                          fDamage);
        for (int o = 0; o < iNumObjects; ++o)
        {
            GameObject* pObject = apObjects[o];
            CollisionObject* pCollision = pObject->GetCollisionObject();
            if (!pCollision)
            {
                continue;
            }
            int k;
            for (k = 0; k < pData->iNumObjects; ++k)
            {
                if (pData->apObject[k] == pObject)
                {
                    break;
                }
            }
            if (k < pData->iNumObjects)
            {
                continue;
            }

            PblVector3 vStart, vDir;
            float t = 1.0f;
            unsigned int i;
            for (i = iFirst; i < uiEnd; ++i)
            {
                const MeleeDamageSample& sample = aSample[i];
                PblVector3 vSampleDir;
                D3DXVec3TransformNormal(&vSampleDir, &sample.vDir, &mAttack);
                vDir = vSampleDir;
                const float fBackX = vSampleDir.x * 0.2f;
                const float fBackY = vSampleDir.y * 0.2f;
                const float fBackZ = vSampleDir.z * 0.2f;
                PblVector3 vPos;
                D3DXVec3TransformCoord(&vPos, &sample.vPos, &mAttack);
                vStart = PblVector3(vPos.x - fBackX, vPos.y - fBackY, vPos.z - fBackZ);
                t = pCollision->RayHit(vStart, vDir, fRayLength, 0, 0, true, fWidth);
                if (t < 1.0f)
                {
                    break;
                }
            }
            if (i >= uiEnd || pData->iNumObjects >= 8)
            {
                continue;
            }

            pData->apObject[pData->iNumObjects++] = pObject;
            const float fAlong = t * fRayLength;
            PblVector3 vHit(vStart.x + vDir.x * fAlong, vStart.y + vDir.y * fAlong, vStart.z + vDir.z * fAlong);
            PblVector3 vSwing(mAttack.m[2][0], mAttack.m[2][1], mAttack.m[2][2]);

            if (pObject->Deflect(0, vSwing, vHit))
            {
                continue;
            }
            static_cast<Damageable*>(pObject)->ApplyDamage(damage, vSwing, vHit, -1, 0xffffffff);
            if (fPush > 0.0f)
            {
                pObject->ApplyPush(vSwing, fPush, false);
            }

            if (!pHitEffectClass && pClass->m_uiNumHitEffects)
            {
                unsigned int uiEffect;
                if (gAltMeleeHitEffects && !(netInShell ? netEnabled : netEnabledNext) &&
                    pClass->m_uiNumHitEffects != 1)
                {
                    uiEffect = pClass->m_uiHitEffects[1 + PblRandom::GetGlobalInt() %
                                                              (pClass->m_uiNumHitEffects - 1)];
                }
                else
                {
                    uiEffect = pClass->m_uiHitEffects[0];
                }
                if (uiEffect)
                {
                    pHitEffectClass = FLEffect::s_EffectClasses.Find(uiEffect);
                }
            }
            if (pHitEffectClass)
            {
                FLEffectObject* pEffect = pHitEffectClass->BuildEffect();
                if (pEffect)
                {
                    pEffect->AttachEffectToVector(&vHit, false);
                }
            }
            bHit = true;
        }
    }

    if (bHit)
    {
        GameObject* pObject = mOwner->GetGameObject();
        pClass->m_HitSound.Play(&mOwner->GetGameObject()->GetPosition(), &pObject->GetVelocity(), false,
                                true);
    }
    return true;
}

// FUNCTION: SWBF2 0x0068c190
void WeaponMelee::EnterFire()
{
    const Combo::State& state = m_pCombo->m_aState[m_eComboState];
    m_uiTrailOnMask = 0;
    mStateTimer = 0.0f;
    SignalFire();
    float fDuration = static_cast<const WeaponMeleeClass*>(mClass)->m_fTurnOnDuration;
    if (state.m_uiNumAttacks)
    {
        float fStart = state.m_aAttack[0].m_uiDamageTimeStart * 0.01f;
        if (fStart < fDuration)
        {
            fDuration = fStart;
        }
    }
    TurnOn(fDuration);
}

// FUNCTION: SWBF2 0x0068d180
void WeaponMelee::EnterRecover()
{
    mStateTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0068d190
void WeaponMelee::ExitRecover()
{
    mStateTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0068d1a0
bool WeaponMelee::UpdateRecover(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    return true;
}

// FUNCTION: SWBF2 0x0068d1c0
void WeaponMelee::EnterDeflect()
{
    mStateTimer = 0.0f;
    m_fDeflectTimer = 1.0f;
    TurnOn(-1.0f);
}

// FUNCTION: SWBF2 0x0068d1f0
void WeaponMelee::ExitDeflect()
{
    mStateTimer = 0.0f;
    m_fDeflectTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0068d210
bool WeaponMelee::UpdateDeflect(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    m_fDeflectTimer += fDeltaTime;
    unsigned char uiAnim = m_pCombo->m_aState[m_eComboState].m_uiAnimIndex;
    if (m_uiAnimIndex != uiAnim && m_fDeflectTimer > 1.0f)
    {
        m_uiAnimIndex = uiAnim;
    }
    return true;
}

// FUNCTION: SWBF2 0x0068ebb0
void WeaponMeleeClass::GetDamageEdgeRay(unsigned int uiEdge, unsigned int& uiAttach,
                                        PblVector3& vBegin, PblVector3& vEnd) const
{
    const DamageEdge& edge = m_aDamageEdge[uiEdge];
    uiAttach = m_auiHardpoint[edge.uiAttachToWeapon];
    vBegin = edge.vOffsetPos;
    vEnd = edge.vOffsetDir;
}

// FUNCTION: SWBF2 0x0068ecb0
Combo* WeaponMeleeClass::_CreateODFCombo(unsigned int  )
{
    if (m_pCombo)
    {
        return m_pComboODF;
    }
    m_pCombo = const_cast<Combo*>(Combo::Find(s_uiODFComboId));
    if (m_pCombo)
    {
        m_pComboODF = 0;
        return 0;
    }
    if (Combo::sMemoryPool.IsFull())
    {
        return 0;
    }
    m_pComboODF = new Combo;
    m_pComboODF->InitDefault(s_uiODFComboId);
    m_pCombo = m_pComboODF;
    return m_pComboODF;
}

// FUNCTION: SWBF2 0x0068e830
Weapon* WeaponMeleeClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    WeaponMelee* pWeapon = new WeaponMelee(this, desc);
    if (!m_amDeflectMatrix)
    {
        m_eDeflectMatrixMap = pWeapon->mSoldierAnimationMap;
        _InitDeflectMatrices(m_eDeflectMatrixMap);
    }
    return pWeapon;
}

// FUNCTION: SWBF2 0x0068f150
void WeaponMeleeClass::_InitDeflectMatrices(int eMap)
{
    if (!m_pCombo)
    {
        return;
    }
    const Combo::Deflect* pDeflect = 0;
    for (unsigned int i = 0; i < m_pCombo->m_uiNumStates; ++i)
    {
        pDeflect = m_pCombo->m_aState[i].m_pDeflect;
        if (pDeflect)
        {
            break;
        }
    }
    if (!pDeflect || !pDeflect->m_uiNumDeflectAnimations)
    {
        return;
    }

    unsigned int uiCount = pDeflect->m_uiNumDeflectAnimations;
    unsigned int uiSize = uiCount > 0x03ffffffu ? 0xffffffffu : uiCount * sizeof(PblMatrix);
    char* pRaw = uiSize > 0xffffffefu ? 0 : new char[uiSize + 16];
    if (pRaw)
    {
        unsigned char uiOffset = 16 - (reinterpret_cast<unsigned int>(pRaw) & 15);
        pRaw += uiOffset;
        pRaw[-1] = uiOffset;
    }
    m_amDeflectMatrix = reinterpret_cast<PblMatrix*>(pRaw);
    for (unsigned int i = 0; i < uiCount; ++i)
    {
        const Combo::Deflect::DEFLECT_ANIM& anim = pDeflect->m_aDeflectAnim[i];
        unsigned int uiHardpoint = m_auiHardpoint[0];
        if (anim.uiWeapon && anim.uiWeapon < m_uiNumWeapons)
        {
            uiHardpoint = m_auiHardpoint[anim.uiWeapon];
        }
        SoldierAnimatorClass::sInstance->GetAnimationBoneMatrix(
            static_cast<SoldierAnimationBank::MAP>(eMap), anim.uiAnimIndex,
            uiHardpoint, 0.1f, m_amDeflectMatrix[i]);
    }
}

// GLOBAL: SWBF2 0x0078c120
DefaultCombo::DeflectAnimation aDeflectAnimation[DefaultCombo::NUM_DEFLECT_ANIMATIONS] = {
    { "stand_block_front1", 1 },
    { "stand_block_front2", 1 },
    { "stand_block_right1", 2 },
    { "stand_block_right2", 2 },
    { "stand_block_left1", 8 },
    { "stand_block_left2", 8 },
    { "stand_block_rear1", 32 },
    { "stand_block_rear2", 32 },
};

// GLOBAL: SWBF2 0x01f9a8ec
// SYNTHETIC: SWBF2 0x00407200
// `dynamic initializer for 'WeaponMelee::rttiHashWeaponMelee''
PblHash WeaponMelee::rttiHashWeaponMelee("WeaponMelee");
char WeaponMelee::rttiNameWeaponMelee[] = "WeaponMelee";

// GLOBAL: SWBF2 0x01f9a8f4
// SYNTHETIC: SWBF2 0x00407210
// `dynamic initializer for 'WeaponMeleeClass::rttiHashWeaponMeleeClass''
PblHash WeaponMeleeClass::rttiHashWeaponMeleeClass("WeaponMeleeClass");
char WeaponMeleeClass::rttiNameWeaponMeleeClass[] = "WeaponMeleeClass";

// GLOBAL: SWBF2 0x01eafd70
WeaponMelee::TrailData* WeaponMelee::s_aTrailDataPool;
// GLOBAL: SWBF2 0x01eafd68
WeaponMelee::TrailData* WeaponMelee::s_pTrailDataFreeList;
// GLOBAL: SWBF2 0x01eafd6c
WeaponMelee::DamageData* WeaponMelee::s_aDamageDataPool;
// GLOBAL: SWBF2 0x01eafd64
WeaponMelee::DamageData* WeaponMelee::s_pDamageDataFreeList;

// FUNCTION: SWBF2 0x00687bf0
bool WeaponMelee::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponMelee || Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00687c20
unsigned int WeaponMelee::GetDerivedRtti() const
{
    return rttiHashWeaponMelee;
}

// FUNCTION: SWBF2 0x00687c30
const char* WeaponMelee::GetDerivedRttiName() const
{
    return rttiNameWeaponMelee;
}

// FUNCTION: SWBF2 0x00687c40
bool WeaponMelee::IsMelee() const
{
    return true;
}

// FUNCTION: SWBF2 0x00687c50
bool WeaponMelee::SoldierCanOperate() const
{
    return true;
}

// SYNTHETIC: SWBF2 0x00687c60
// WeaponMelee::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00687c90
bool WeaponMeleeClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponMeleeClass || WeaponClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00687cc0
unsigned int WeaponMeleeClass::GetDerivedRtti()
{
    return rttiHashWeaponMeleeClass;
}

// FUNCTION: SWBF2 0x00687cd0
const char* WeaponMeleeClass::GetDerivedRttiName()
{
    return rttiNameWeaponMeleeClass;
}

LightsaberTrailClass::LightsaberTrailClass()
{
}

LightsaberTrailClass::~LightsaberTrailClass()
{
}

// FUNCTION: SWBF2 0x00687cf0
FLEffectObject* LightsaberTrailClass::BuildEffect()
{
    return new LightsaberTrail(this);
}

// SYNTHETIC: SWBF2 0x00687d50
// LightsaberTrailClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00687d80
void Combo::InitDefault(unsigned int uiNameHash)
{
    m_uiNameHash = uiNameHash;
    m_eMap = -1;
    m_eCombo = SoldierAnimationBank::AddCombo(uiNameHash);
    m_pSkeleton = 0;
    if (m_eCombo == SoldierAnimationBank::INVALID_COMBO)
    {
        return;
    }

    if (s_ComboTable.Store(uiNameHash, this))
    {
        ++s_ComboTableNumEntries;
    }

    m_aState = 0;
    m_uiNumStates = 0;
    State* pState = AddState(PblHash(DefaultCombo::aszStateName[DefaultCombo::IDLE]));
    if (pState)
    {
        pState->SetInputLock(0x38, 0.15f);
        pState->SetEnergyRestoreRate(0.0f, true);
    }
}

// FUNCTION: SWBF2 0x00687e10
void Combo::AddDefaultAttackState()
{
    unsigned int uiNumStates = m_uiNumStates;
    if (uiNumStates >= DefaultCombo::NUM_STATES || !(m_uiNumStates & 1))
    {
        return;
    }

    unsigned char uiAnimIndex = 0xff;
    if (uiNumStates == DefaultCombo::ATTACK1)
    {
        Transition* pTransition = m_aState[DefaultCombo::IDLE].AddTransition(DefaultCombo::ATTACK1);
        if (!pTransition)
        {
            return;
        }
        Condition* pCondition = pTransition->AddCondition();
        if (!pCondition)
        {
            return;
        }
        pCondition->m_uiSoldierStates = 0x0b;
        pCondition->m_uiButtonAction = 1;
        pCondition->m_uiButton = 1;
    }
    else
    {
        unsigned int uiPrev = uiNumStates - 2;
        if (uiPrev > uiNumStates)
        {
            return;
        }
        State* pPrev = &m_aState[uiPrev];
        Transition* pTransition = pPrev->m_aTransition;
        if (!pTransition)
        {
            return;
        }
        pTransition->m_uiTargetState = uiNumStates;
        Condition* pCondition = pTransition->AddCondition();
        if (!pCondition)
        {
            return;
        }
        pCondition->m_uiSoldierStates = 0x01;
        pCondition->m_uiButtonAction = 0;
        pCondition->m_uiButton = 1;
        pPrev->AddTransition(m_uiNumStates - 1);
    }

    unsigned int uiState = m_uiNumStates;
    State* pState = AddState(PblHash(DefaultCombo::aszStateName[uiState]));
    if (!pState)
    {
        return;
    }
    pState->SetInputLock(0x7f, -1.0f);
    if (m_eCombo != SoldierAnimationBank::INVALID_COMBO)
    {
        SoldierAnimationBank::COMBO_ANIMATION eAnim = SoldierAnimationBank::AddComboAnimation(
            (SoldierAnimationBank::COMBO)m_eCombo,
            DefaultCombo::aszAnimationName[DefaultCombo::saiAnimForState[uiState]]);
        if (eAnim == SoldierAnimationBank::INVALID_COMBO_ANIMATION)
        {
            uiAnimIndex = 0xff;
        }
        else
        {
            uiAnimIndex = SoldierAnimationData::Find(eAnim, SoldierAnimType::UPPER)->GetAnimIndex();
        }
    }
    pState->m_uiAnimIndex = uiAnimIndex;
    pState->m_uiSoldierStates = 1;
    pState->AddAttack();
    if (!pState->AddTransition(uiState + 1))
    {
        return;
    }

    uiState = m_uiNumStates;
    pState = AddState(PblHash(DefaultCombo::aszStateName[uiState]));
    if (!pState)
    {
        return;
    }
    pState->SetInputLock(0x7f, -1.0f);
    if (m_eCombo != SoldierAnimationBank::INVALID_COMBO)
    {
        SoldierAnimationBank::COMBO_ANIMATION eAnim = SoldierAnimationBank::AddComboAnimation(
            (SoldierAnimationBank::COMBO)m_eCombo,
            DefaultCombo::aszAnimationName[DefaultCombo::saiAnimForState[uiState]]);
        if (eAnim == SoldierAnimationBank::INVALID_COMBO_ANIMATION)
        {
            uiAnimIndex = 0xff;
        }
        else
        {
            uiAnimIndex = SoldierAnimationData::Find(eAnim, SoldierAnimType::UPPER)->GetAnimIndex();
        }
    }
    pState->m_uiAnimIndex = uiAnimIndex;
    pState->m_uiSoldierStates = 1;
}

// FUNCTION: SWBF2 0x00687fd0
void Combo::AddDefaultDeflect()
{
    if (m_aState[DefaultCombo::IDLE].m_pDeflect)
    {
        return;
    }
    Deflect* pDeflect = m_aState[DefaultCombo::IDLE].AddDeflect();
    if (!pDeflect)
    {
        return;
    }
    for (unsigned int i = 0; i < DefaultCombo::NUM_DEFLECT_ANIMATIONS; ++i)
    {
        SoldierAnimationBank::COMBO_ANIMATION eAnim = SoldierAnimationBank::AddComboAnimation(
            (SoldierAnimationBank::COMBO)m_eCombo, aDeflectAnimation[i].szName);
        if (eAnim == SoldierAnimationBank::INVALID_COMBO_ANIMATION)
        {
            continue;
        }
        if (pDeflect->AddDeflectAnimation(
                SoldierAnimationData::Find(eAnim, SoldierAnimType::UPPER)->GetAnimIndex(),
                aDeflectAnimation[i].uiDirections, 0) >= 15)
        {
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00688040
WeaponMelee::TrailData::TrailData()
    : m_pTrail(0)
{
}

// FUNCTION: SWBF2 0x00688060
WeaponMelee::TrailData::~TrailData()
{
    delete m_pTrail;
}

// FUNCTION: SWBF2 0x006880c0
void WeaponMelee::TrailData::Init(unsigned int uiEdge, const PblVector3& vOffsetPos,
                                  const PblVector3& vOffsetDir, float fLength, float fWidth,
                                  const RedColor& colorTrail, const RedColor& colorHead)
{
    m_uiEdge = (unsigned char)uiEdge;
    m_TrailClass.m_vOffsetPos = vOffsetPos;
    m_TrailClass.m_vOffsetDir = vOffsetDir;
    m_TrailClass.m_fLength = fLength;
    m_TrailClass.m_fWidth = fWidth;
    m_TrailClass.m_colorTrail = colorTrail;
    m_TrailClass.m_colorHead = colorHead;
    delete m_pTrail;
    m_pTrail = (LightsaberTrail*)m_TrailClass.BuildEffect();
    m_uiFrameLastUpdated = (unsigned char)(RedRenderer::GetFrameNumber() - 1);
}

// FUNCTION: SWBF2 0x00688140
void WeaponMelee::_InitDamageDataPool()
{
    s_aDamageDataPool = new DamageData[16];
    for (int i = 0; i < 16; ++i)
    {
        s_aDamageDataPool[i].pNext = (i < 15) ? &s_aDamageDataPool[i + 1] : 0;
    }
    s_pDamageDataFreeList = s_aDamageDataPool;
}

// FUNCTION: SWBF2 0x00688180
WeaponMelee::DamageData* WeaponMelee::AllocDamageData()
{
    DamageData* pData = s_pDamageDataFreeList;
    if (pData)
    {
        s_pDamageDataFreeList = pData->pNext;
        pData->pNext = 0;
    }
    return pData;
}

// FUNCTION: SWBF2 0x006881a0
void WeaponMelee::FreeDamageData(DamageData* pData)
{
    pData->pNext = s_pDamageDataFreeList;
    s_pDamageDataFreeList = pData;
}

// FUNCTION: SWBF2 0x006881b0
void WeaponMelee::_InitTrailDataPool()
{
    s_aTrailDataPool = new TrailData[16];
    for (int i = 0; i < 16; ++i)
    {
        s_aTrailDataPool[i].pNext = (i < 15) ? &s_aTrailDataPool[i + 1] : 0;
    }
    s_pTrailDataFreeList = s_aTrailDataPool;
}

// FUNCTION: SWBF2 0x00688240
WeaponMelee::TrailData* WeaponMelee::_AllocTrailData()
{
    TrailData* pData = s_pTrailDataFreeList;
    if (pData)
    {
        s_pTrailDataFreeList = pData->pNext;
        pData->pNext = 0;
    }
    return pData;
}

// FUNCTION: SWBF2 0x00688260
void WeaponMelee::_FreeTrailData(TrailData* pData)
{
    pData->pNext = s_pTrailDataFreeList;
    s_pTrailDataFreeList = pData;
}

// FUNCTION: SWBF2 0x00688270
WeaponMelee::WeaponMelee(const WeaponMeleeClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    if (pClass->m_pCombo)
    {
        if (mSoldierAnimationMap == -1)
        {
            m_pCombo = pClass->m_pCombo;
        }
        else
        {
            const ZephyrSkeletonShared* pSkeleton = SoldierAnimatorClass::Instance()->GetDefaultSkeleton();
            if (mOwner->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
            {
                pSkeleton = ((EntitySoldier*)mOwner->GetGameObject())->GetClass()->mSpecialSkeleton;
            }
            m_pCombo = pClass->m_pCombo->ResolveForWeapon(this, pSkeleton);
        }
    }
    else
    {
        m_pCombo = 0;
    }
    m_pMeleeThrow = 0;
    mState = IDLE;
    m_eComboState = COMBO_IDLE;
    m_eComboStatePrev = COMBO_IDLE;
    m_fComboStateTimer = 0.0f;
    m_fComboStateDurationPrev = 0.0f;
    m_fDeflectTimer = 0.0f;
    EnterState(COMBO_IDLE);
    m_bInHeroFlee = 0;
    m_fVelocityOverrideFactor = 0.0f;
    m_fTurnOnTimer = 0.0f;
    m_fTurnOnDuration = 0.0f;
    m_uiLightsaberWasOnMask = m_uiLightsaberOnMask;
    m_uiLightsaberSoundOnMask = m_uiLightsaberOnMask;
    m_uiTrailOnMask = 0;
    m_bAnimatedMove = 0;
    m_bLightsaberOn = 1;
    m_bLightsaberSoundOn = 0;
    m_bAI_Deflect = 0;
    m_pTrailData = 0;
    m_pDamageData = 0;
    ClearInput();
}

// FUNCTION: SWBF2 0x00688430
WeaponMelee::~WeaponMelee()
{
    ReleaseDamageData();
    while (m_pTrailData)
    {
        TrailData* pData = m_pTrailData;
        m_pTrailData = pData->pNext;
        _FreeTrailData(pData);
    }
}

// FUNCTION: SWBF2 0x006884c0
void WeaponMelee::Select(unsigned int uFlags, bool bOffhand)
{
    Weapon::Select(uFlags, bOffhand);
    ClearInput();
}

// FUNCTION: SWBF2 0x006884e0
void WeaponMelee::Deselect(unsigned int  , bool bOffhand)
{
    mSelectedFlag = 0;
    ClearInput();
    m_iSoldierState = SoldierState::STAND;
    if (!bOffhand)
    {
        TurnOff(-1.0f);
    }
    if (m_eComboState == COMBO_IDLE)
    {
        return;
    }

    const Combo::State* pState = &m_pCombo->m_aState[m_eComboState];
    unsigned int i;
    for (i = 0; i < pState->m_uiNumTransitions; ++i)
    {
        if (pState->m_aTransition[i].m_uiTargetState == COMBO_IDLE)
        {
            break;
        }
    }
    if (i < pState->m_uiNumTransitions)
    {
        ExitState();
        EnterState(COMBO_IDLE);
        m_eComboStatePrev = m_eComboState;
        m_fComboStateDurationPrev = 0.0f;
        ReleaseDamageData();
    }
}

// FUNCTION: SWBF2 0x00688580
void WeaponMelee::NotifySoldierState(int iState, float fTime, bool b0, bool b1, const PblVector3& vec)
{
    Weapon::NotifySoldierState(iState, fTime, b0, b1, vec);
    m_fSoldierStateTimer = fTime;
    m_vVelocityLocal = vec;
    m_bSoldierInputLock = b0;
    if (m_iSoldierState == SoldierState::HERO_FLEE)
    {
        TurnOff(-1.0f);
        ReleaseDamageData();
        ClearInput();
        if (m_pCombo && m_pCombo->m_pTakeKneeState)
        {
            for (unsigned int i = 0; i < m_pCombo->m_uiNumStates; ++i)
            {
                if (&m_pCombo->m_aState[i] == m_pCombo->m_pTakeKneeState)
                {
                    if ((unsigned int)m_eComboState != i)
                    {
                        ExitState();
                        EnterState((ComboState)i);
                    }
                    break;
                }
            }
        }
        m_bInHeroFlee = 1;
        mState = (WeaponState)MELEE_ATTACK;
        return;
    }

    if (m_iSoldierState > SoldierState::JET_JUMP)
    {
        ClearInput();
    }
    if (mState == (WeaponState)MELEE_IDLE)
    {
        int iSoldierState = m_iSoldierState;
        if (iSoldierState == SoldierState::STAND || iSoldierState == SoldierState::JUMP ||
            iSoldierState == SoldierState::JET_JUMP || iSoldierState == SoldierState::SPRINT)
        {
            TurnOn(-1.0f);
        }
        else
        {
            TurnOff(-1.0f);
        }
    }
    else if (m_iSoldierState > SoldierState::JET_JUMP)
    {
        ExitState();
        EnterState(COMBO_IDLE);
        TurnOff(-1.0f);
        ReleaseDamageData();
    }
}

// FUNCTION: SWBF2 0x00688700
bool WeaponMelee::OverrideSoldierControls()
{
    m_fInputThrustZ = mOwner->mControlMove;
    m_fInputThrustX = mOwner->mControlStrafe;
    ReadButtons();

    const Combo::State* pState = &m_pCombo->m_aState[m_eComboState];
    if (!pState->m_bAlignToInitialThrust)
    {
        m_fAlignZ = m_fInputThrustZ;
        m_fAlignX = m_fInputThrustX;
    }

    if (pState->m_uiInputLock)
    {
        float fDuration = (float)pState->m_uiInputLockDuration * 0.01f;
        if (fDuration <= 0.0f || fDuration > m_fComboStateTimer)
        {
            if (pState->m_uiInputLock & 1)
            {
                mOwner->mControlMove = 0.0f;
                mOwner->mControlStrafe = 0.0f;
            }
            for (unsigned char uiButton = 1; uiButton < 7; ++uiButton)
            {
                if (pState->m_uiInputLock & (1 << uiButton))
                {
                    GetButton(uiButton)->Clear();
                }
            }
        }
    }

    mOwner->mControlMove = (float)pState->m_uiThrustFactor * 0.05f * mOwner->mControlMove;
    mOwner->mControlStrafe = (float)pState->m_uiStrafeFactor * 0.05f * mOwner->mControlStrafe;
    mOwner->mControlTurn = (float)pState->m_uiTurnFactor * 0.05f * mOwner->mControlTurn;
    mOwner->mControlPitch = (float)pState->m_uiPitchFactor * 0.05f * mOwner->mControlPitch;

    unsigned int uiSoldierStates = pState->m_uiSoldierStates;
    m_bUnreachableState_SoldierUpdate = 0;
    if (uiSoldierStates)
    {
        m_bUnreachableState_SoldierUpdate = 1;
        if (m_iSoldierState <= SoldierState::JUMP || m_iSoldierState == SoldierState::JET_JUMP)
        {
            m_bUnreachableState_SoldierUpdate = 0;
            while (!(uiSoldierStates & (1 << m_iSoldierState)) && !m_bUnreachableState_SoldierUpdate)
            {
                int iPrev = m_iSoldierState;
                switch (iPrev)
                {
                case SoldierState::STAND:
                    if ((uiSoldierStates & 8) && mOwner->Sprint())
                    {
                        m_iSoldierState = SoldierState::SPRINT;
                    }
                    else if ((uiSoldierStates & 2) && mOwner->Crouch())
                    {
                        m_iSoldierState = SoldierState::CROUCH;
                    }
                    else if ((uiSoldierStates & 16) && mOwner->Jump())
                    {
                        m_iSoldierState = SoldierState::JUMP;
                    }
                    else if ((uiSoldierStates & 32) && mOwner->Roll())
                    {
                        m_iSoldierState = SoldierState::ROLL;
                    }
                    else if ((uiSoldierStates & 64) && mOwner->Jump())
                    {
                        m_iSoldierState = SoldierState::JUMP;
                    }
                    else
                    {
                        m_bUnreachableState_SoldierUpdate = 1;
                    }
                    break;
                case SoldierState::CROUCH:
                    if ((uiSoldierStates & 1) && mOwner->Stand())
                    {
                        m_iSoldierState = SoldierState::STAND;
                    }
                    else if (mOwner->Stand())
                    {
                        m_iSoldierState = SoldierState::STAND;
                    }
                    else
                    {
                        m_bUnreachableState_SoldierUpdate = 1;
                    }
                    break;
                case SoldierState::PRONE:
                    if ((uiSoldierStates & 1) && mOwner->Stand())
                    {
                        m_iSoldierState = SoldierState::STAND;
                    }
                    else if ((uiSoldierStates & 2) && mOwner->Crouch())
                    {
                        m_iSoldierState = SoldierState::CROUCH;
                    }
                    else if (mOwner->Stand())
                    {
                        m_iSoldierState = SoldierState::STAND;
                    }
                    else
                    {
                        m_bUnreachableState_SoldierUpdate = 1;
                    }
                    break;
                case SoldierState::SPRINT:
                    if (mOwner->EndSprint())
                    {
                        m_iSoldierState = SoldierState::STAND;
                    }
                    else
                    {
                        m_bUnreachableState_SoldierUpdate = 1;
                    }
                    break;
                case SoldierState::JUMP:
                    if ((uiSoldierStates & 64) && mOwner->JetJump())
                    {
                        m_iSoldierState = SoldierState::JET_JUMP;
                    }
                    else
                    {
                        m_bUnreachableState_SoldierUpdate = 1;
                    }
                    break;
                case SoldierState::JET_JUMP:
                    if ((uiSoldierStates & 16) && mOwner->EndJetJump())
                    {
                        m_iSoldierState = SoldierState::JUMP;
                    }
                    else
                    {
                        m_bUnreachableState_SoldierUpdate = 1;
                    }
                    break;
                default:
                    break;
                }
                if (m_iSoldierState == iPrev)
                {
                    m_bUnreachableState_SoldierUpdate = 1;
                }
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00688b50
bool WeaponMelee::ShouldShowReticule() const
{
    return m_pCombo->m_aState[m_eComboState].m_pDeflect != 0;
}

// FUNCTION: SWBF2 0x00688b70
unsigned char WeaponMelee::GetWeaponMeleeAnimation() const
{
    return m_uiAnimIndex;
}

// FUNCTION: SWBF2 0x00688b80
unsigned char WeaponMelee::GetNextWeaponMeleeAnimation() const
{
    const Combo::State* aState = m_pCombo->m_aState;
    const Combo::State* pState = &aState[m_eComboState];
    if (GetMeleeState() != MELEE_DEFLECT)
    {
        unsigned int uiNumTransitions = pState->m_uiNumTransitions;
        if (uiNumTransitions == 0)
        {
            return 0xa4;
        }
        pState = &aState[pState->m_aTransition[uiNumTransitions - 1].m_uiTargetState];
    }
    return (unsigned char)pState->m_uiAnimIndex;
}

// FUNCTION: SWBF2 0x00688bc0
unsigned char WeaponMelee::GetOffhandWeaponMeleeAnimation(int eState) const
{
    return m_pCombo->GetOffhandAnimIndex(eState);
}

// FUNCTION: SWBF2 0x00688bd0
unsigned char WeaponMelee::GetWeaponMeleeThrowAnimation() const
{
    return (unsigned char)m_pCombo->m_uiThrowAnimIndex;
}

// FUNCTION: SWBF2 0x00688be0
bool WeaponMelee::IsAnimationVelocityAligned() const
{
    const Combo::State* pState = &m_pCombo->m_aState[m_eComboState];
    if (GetMeleeState() == MELEE_DEFLECT && m_uiAnimIndex != pState->m_uiAnimIndex)
    {
        return true;
    }
    return pState->m_bAlignToThrust;
}

// FUNCTION: SWBF2 0x00688c20
bool WeaponMelee::IsFullBodyAnimation() const
{
    return m_bAnimatedMove;
}

// FUNCTION: SWBF2 0x00688c30
bool WeaponMelee::OverrideSoldierVelocity(float& fVelocityZ, float& fVelocityX, float& fFactor,
                                          float& fVelocityY) const
{
    bool bOverride = false;
    const Combo::State* pState = &m_pCombo->m_aState[m_eComboState];
    int iGravity = pState->m_iGravity;
    if (iGravity == Combo::State::GRAVITY_IMPULSE)
    {
        if (!m_bGravityDone)
        {
            fVelocityY = (float)pState->m_iGravityVelocityYTarget * 0.1f;
            const_cast<WeaponMelee*>(this)->m_bGravityDone = 1;
            bOverride = true;
        }
    }
    else if (iGravity != 10)
    {
        float fGravity = (float)iGravity * 0.1f;
        int iTarget = pState->m_iGravityVelocityYTarget;
        if (iTarget != -2048)
        {
            if (m_bGravityDone || (fGravity > 0.0f) == ((float)iTarget * 0.1f >= fVelocityY))
            {
                const_cast<WeaponMelee*>(this)->m_bGravityDone = 1;
                fGravity = 1.0f - (float)pState->m_bGravityZeroAtTarget;
            }
        }
        if (fGravity != 1.0f)
        {
            fVelocityY -= (fGravity - 1.0f) * 18.0f * GameLoop::sClientDeltaTime;
            bOverride = true;
        }
    }

    float fOverride = m_fVelocityOverrideFactor;
    if (fOverride <= 0.0f)
    {
        return bOverride;
    }

    if (!pState->m_bAnimatedMove && m_eComboStatePrev != m_eComboState)
    {
        const Combo::State* pPrev = &m_pCombo->m_aState[m_eComboStatePrev];
        if (pPrev->m_bAnimatedMove)
        {
            pState = pPrev;
        }
    }
    if (!pState->m_bAnimatedMove)
    {
        return bOverride;
    }

    float fStateX = (float)pState->m_iVelocityX * 0.02f;
    float fStateZ = (float)pState->m_iVelocityZ * 0.02f;
    PblVector3 vThrust(-m_fInputThrustX, 0.0f, m_fInputThrustZ);
    float fX;
    float fZ;
    if (pState->m_bAlignToThrust)
    {
        AlignVectorToThrust(vThrust, true);
        PblVector3 vVelocity((float)pState->m_uiVelocityFromStrafe * 0.1f * vThrust.x + fStateX, 0.0f,
                             (float)pState->m_uiVelocityFromThrust * 0.1f * vThrust.z + fStateZ);
        AlignVectorToThrust(vVelocity, false);
        fZ = vVelocity.z;
        fX = vVelocity.x;
    }
    else
    {
        fX = (float)pState->m_uiVelocityFromStrafe * 0.1f * vThrust.x + fStateX;
        fZ = (float)pState->m_uiVelocityFromThrust * 0.1f * vThrust.z + fStateZ;
    }
    fFactor = fOverride;
    fVelocityZ = fZ;
    fVelocityX = fX;
    return true;
}

// FUNCTION: SWBF2 0x00688f10
bool WeaponMelee::OverrideSoldierEnergyRestore(float& fRestore) const
{
    const Combo::State* pState = &m_pCombo->m_aState[m_eComboState];
    if (!pState->m_bEnergyRestoreRateOverride && pState->m_iEnergyRestoreRate == 0)
    {
        return false;
    }
    if (pState->m_bEnergyRestoreRateOverride)
    {
        fRestore = (float)pState->m_iEnergyRestoreRate * 0.2f;
    }
    else
    {
        fRestore = (float)pState->m_iEnergyRestoreRate * 0.2f + fRestore;
    }
    return true;
}

// FUNCTION: SWBF2 0x00688f90
bool WeaponMelee::OverrideAimer()
{
    if (mOwner->mPlayerId < 0)
    {
        return false;
    }
    const Combo::State* pState = &m_pCombo->m_aState[m_eComboState];
    if (pState->m_pDeflect || !pState->m_bAlignToThrust)
    {
        return false;
    }
    Aimer* pAimer = mAimer;
    PblVector3 vDirection = pAimer->mDirection;
    if (!AlignVectorToThrust(vDirection, false))
    {
        return false;
    }
    pAimer->SetSoldierInfo(pAimer->mFirePos, vDirection);
    return true;
}

// FUNCTION: SWBF2 0x00689010
void WeaponMelee::UpdateEffects(float fDeltaTime, const PblVector3& vPos, const PblVector3& vVel,
                                const GameSound* pSound)
{
    const WeaponMeleeClass* pClass = (const WeaponMeleeClass*)mClass;
    bool bOn = m_bLightsaberOn && m_uiLightsaberOnMask;
    if (m_bLightsaberSoundOn != bOn)
    {
        if (bOn)
        {
            if (!(mSoundControl.mFlags & GameSoundControllable::flags_playing))
            {
                if (!pSound)
                {
                    pSound = &pClass->m_OnSound;
                }
                mSoundProps = *pSound;
                mSoundControl.Play(&mSoundProps, &vPos, &vVel, true);
            }
        }
        else if (mSoundControl.mFlags & GameSoundControllable::flags_playing)
        {
            mSoundControl.Stop(true);
        }
        m_bLightsaberSoundOn = bOn;
    }
    else if (m_bLightsaberSoundOn && (mSoundControl.mFlags & GameSoundControllable::flags_playing))
    {
        if (!pSound)
        {
            pSound = &pClass->m_OnSound;
        }
        if (mSoundProps.GetProperties() != pSound->GetProperties())
        {
            mSoundControl.Stop(true);
            mSoundProps = *pSound;
            mSoundControl.Play(&mSoundProps, &vPos, &vVel, true);
        }
    }

    unsigned int uiMask = bOn ? m_uiLightsaberOnMask : 0;
    if (m_uiLightsaberSoundOnMask != uiMask)
    {
        if (uiMask & ~(unsigned int)m_uiLightsaberSoundOnMask)
        {
            m_OnOffSound.Stop(true);
            m_OnOffSoundProps = pClass->m_TurnOnSound;
            m_OnOffSound.Play(&m_OnOffSoundProps, &vPos, &vVel, true);
        }
        if ((unsigned char)~uiMask & m_uiLightsaberSoundOnMask)
        {
            m_OnOffSound.Stop(true);
            m_OnOffSoundProps = pClass->m_TurnOffSound;
            m_OnOffSound.Play(&m_OnOffSoundProps, &vPos, &vVel, true);
        }
        m_uiLightsaberSoundOnMask = uiMask;
    }

    if (m_OnOffSound.mFlags & GameSoundControllable::flags_playing)
    {
        m_OnOffSound.Update(&m_OnOffSoundProps, &vPos, &vVel);
    }
    if (mSoundControl.mFlags & GameSoundControllable::flags_playing)
    {
        mSoundControl.Update(&mSoundProps, &vPos, &vVel);
    }
    if (mSoundControlFire.mFlags & GameSoundControllable::flags_playing)
    {
        mSoundControlFire.Update(&mSoundPropsFire, &vPos, &vVel);
    }
    if (mFoleyFXControl.mFlags & GameSoundControllable::flags_playing)
    {
        mFoleyFXControl.Update(&mFoleyFXProps, &vPos, &vVel);
    }
    m_fTurnOnTimer += fDeltaTime;
}

// FUNCTION: SWBF2 0x00689290
bool WeaponMelee::Update(float fDeltaTime)
{
    float fDt = fDeltaTime;
    if (mOwner->GetPlayerId() < 0)
    {
        if (mSkip)
        {
            mSkipTime = fDeltaTime;
            return true;
        }
        fDt += mSkipTime;
        mSkipTime = 0.0f;
    }

    GameObject* pObject = mOwner->GetGameObject();
    UpdateEffects(fDt, mOwner->GetGameObject()->GetPosition(), pObject->GetVelocity(), 0);
    m_bRestartAnimation = 0;

    if (m_pMeleeThrow)
    {
        if (m_pMeleeThrow->m_bReturnedThisFrame)
        {
            m_pMeleeThrow->m_bReturnedThisFrame = 0;
            if ((m_iSoldierState == SoldierState::STAND || m_iSoldierState == SoldierState::CROUCH) &&
                m_pCombo && m_pCombo->m_pCatchState)
            {
                for (unsigned int i = 0; i < m_pCombo->GetNumStates(); ++i)
                {
                    if (&m_pCombo->m_aState[i] == m_pCombo->m_pCatchState)
                    {
                        ExitState();
                        EnterState((ComboState)i);
                        break;
                    }
                }
            }
        }
        else if (m_pMeleeThrow->m_bThrow)
        {
            if (m_eComboState != COMBO_IDLE)
            {
                ExitState();
                EnterState(COMBO_IDLE);
            }
            m_fInputTimer += fDt;
            return true;
        }
    }

    if (m_bAnimatedMove)
    {
        if (m_fVelocityOverrideFactor < 1.0f)
        {
            m_fVelocityOverrideFactor += fDt * (1.0f / 0.15f);
            if (m_fVelocityOverrideFactor > 1.0f)
            {
                m_fVelocityOverrideFactor = 1.0f;
            }
        }
    }
    else if (m_fVelocityOverrideFactor > 0.0f)
    {
        m_fVelocityOverrideFactor -= fDt * (1.0f / 0.15f);
        if (m_fVelocityOverrideFactor < 0.0f)
        {
            m_fVelocityOverrideFactor = 0.0f;
        }
    }

    if (m_bSoldierInputLock)
    {
        m_uiButtonDown = 0;
        m_InputQueue.m_uiCount = 0;
        m_InputQueue.m_uiStart = 0;
    }
    else
    {
        m_InputQueue.Update(m_fInputTimer, m_uiButtonDown);
    }

    const Combo::State* pState = &m_pCombo->m_aState[m_eComboState];
    const Combo::State* pStatePrev =
        m_eComboStatePrev == m_eComboState ? 0 : &m_pCombo->m_aState[m_eComboStatePrev];
    m_fComboStateTimer += fDt;
    bool bDone = false;
    while (!bDone)
    {
        ComboState eNextState = m_eComboState;
        const Combo::Transition* pTransition = 0;
        const Combo::Condition* pCondition = 0;
        float fMatchTime = FLT_MAX;
        bool bOvertime = false;
        float fDuration = pState->m_uiDuration * 0.01f;
        const bool bNoDuration = fDuration <= 0.0f;
        if (bNoDuration)
        {
            fDuration = m_fComboStateTimer;
        }

        if (!bNoDuration && m_fComboStateTimer >= fDuration)
        {

            if (pState->m_uiNumTransitions)
            {
                bool bLater = false;
                for (unsigned int i = 0; i < pState->m_uiNumTransitions; ++i)
                {
                    const Combo::Transition& transition = pState->m_aTransition[i];
                    const float fCost = transition.m_uiEnergyCost * 0.1f;
                    const EnergyBar* pEnergy = mOwner->GetEnergyBar();
                    if (pEnergy && pEnergy->m_bOverheat && fCost > 0.0f)
                    {
                        continue;
                    }
                    if (!transition.m_uiNumConditions && fMatchTime > fDuration)
                    {
                        pTransition = &transition;
                        pCondition = 0;
                        fMatchTime = fDuration;
                    }
                    for (unsigned int j = 0; j < transition.m_uiNumConditions; ++j)
                    {
                        const Combo::Condition* pTest = &transition.m_aCondition[j];
                        float fTime;
                        if (TestCondition(fDuration, m_fComboStateTimer, pTest, &fTime) &&
                            fMatchTime > fTime)
                        {
                            pTransition = &transition;
                            pCondition = pTest;
                            fMatchTime = fTime;
                        }
                        float fEnd = pTest->m_iTimeEnd * 0.01f;
                        if (pTest->m_bTimeEndFromEnd)
                        {
                            fEnd += fDuration;
                        }
                        if (fEnd > fDuration)
                        {
                            bLater = true;
                        }
                    }
                }
                eNextState = (ComboState)pState->m_aTransition[pState->m_uiNumTransitions - 1].m_uiTargetState;
                bOvertime = !pCondition && bLater;
            }
            else
            {
                eNextState = COMBO_IDLE;
                fMatchTime = fDuration;
            }
            const float fOver = m_fComboStateTimer - fDuration;
            if (fDt > fOver)
            {
                UpdateState(fDt - fOver);
            }
        }
        else
        {

            if (pStatePrev)
            {
                const float fPrevTimer = m_fComboStateDurationPrev + m_fComboStateTimer;
                for (unsigned int i = 0; i < pStatePrev->m_uiNumTransitions; ++i)
                {
                    const Combo::Transition& transition = pStatePrev->m_aTransition[i];
                    const float fCost = transition.m_uiEnergyCost * 0.1f;
                    const EnergyBar* pEnergy = mOwner->GetEnergyBar();
                    if (pEnergy && pEnergy->m_bOverheat && fCost > 0.0f)
                    {
                        continue;
                    }
                    for (unsigned int j = 0; j < transition.m_uiNumConditions; ++j)
                    {
                        const Combo::Condition* pTest = &transition.m_aCondition[j];
                        float fTime;
                        if (TestCondition(m_fComboStateDurationPrev, fPrevTimer, pTest, &fTime))
                        {
                            fTime -= m_fComboStateDurationPrev;
                            if (fMatchTime > fTime)
                            {
                                pTransition = &transition;
                                pCondition = pTest;
                                fMatchTime = fTime;
                            }
                        }
                    }
                }
                if (pCondition && pCondition->m_bSyncIfOvertime)
                {
                    fDt = m_fComboStateTimer;
                }
            }

            if (!pTransition)
            {
                for (unsigned int i = 0; i < pState->m_uiNumTransitions; ++i)
                {
                    const Combo::Transition& transition = pState->m_aTransition[i];
                    const float fCost = transition.m_uiEnergyCost * 0.1f;
                    const EnergyBar* pEnergy = mOwner->GetEnergyBar();
                    if (pEnergy && pEnergy->m_bOverheat && fCost > 0.0f)
                    {
                        continue;
                    }
                    for (unsigned int j = 0; j < transition.m_uiNumConditions; ++j)
                    {
                        const Combo::Condition* pTest = &transition.m_aCondition[j];
                        float fTime;
                        if ((pTest->m_bBreak || bNoDuration) &&
                            pTest->m_uiBreakTime * Combo::Condition::TIME_SCALE <= m_fComboStateTimer &&
                            TestCondition(fDuration, m_fComboStateTimer, pTest, &fTime) &&
                            fMatchTime > fTime)
                        {
                            pTransition = &transition;
                            pCondition = pTest;
                            fMatchTime = fTime;
                        }
                    }
                }
            }

            if (!pTransition && m_bAnimatedMove)
            {
                for (unsigned int i = 0; i < pState->m_uiNumAnimatedMoveConditions; ++i)
                {
                    const Combo::Condition* pTest = &pState->m_aAnimatedMoveCondition[i];
                    float fTime;
                    if (pTest->m_uiBreakTime * Combo::Condition::TIME_SCALE <= m_fComboStateTimer &&
                        TestCondition(fDuration, m_fComboStateTimer, pTest, &fTime))
                    {
                        m_bAnimatedMove = 0;
                    }
                }
            }
        }

        if (pTransition)
        {
            eNextState = (ComboState)pTransition->m_uiTargetState;
            EnergyBar* pEnergy = mOwner->GetEnergyBar();
            if (pEnergy)
            {
                pEnergy->SpendEnergy(pTransition->m_uiEnergyCost * 0.1f);
            }
        }

        if (pTransition || eNextState != m_eComboState)
        {
            const float fTimer = m_fComboStateTimer;
            const unsigned char uiAnimIndex = pState->GetAnimIndex();
            if (bOvertime)
            {
                m_eComboStatePrev = m_eComboState;
                m_fComboStateDurationPrev = fDuration;
                pStatePrev = pState;
            }
            else
            {
                m_eComboStatePrev = eNextState;
                m_fComboStateDurationPrev = 0.0f;
                pStatePrev = 0;
            }
            WeaponMelee::_ClearInputQueueToTime(&m_InputQueue, m_fInputTimer + fMatchTime - fTimer);
            ExitState();
            EnterState(eNextState);
            m_fComboStateTimer += fDt;
            pState = &m_pCombo->m_aState[m_eComboState];
            if (uiAnimIndex == pState->GetAnimIndex())
            {
                m_fComboStateTimer = fTimer;
            }
            if (pState->m_bMustShowOneFrame)
            {
                bDone = UpdateState(fDt);
            }
            continue;
        }

        m_bUnreachableState_WeaponUpdate = 0;
        const unsigned char uiSoldierStates = (unsigned char)pState->m_uiSoldierStates;
        if (uiSoldierStates)
        {
            const int iSoldierState = m_iSoldierState;
            if (!((unsigned char)(1 << iSoldierState) & uiSoldierStates))
            {
                m_bUnreachableState_WeaponUpdate = 1;
                if (iSoldierState > SoldierState::JET_JUMP)
                {
                    m_bUnreachableState_SoldierUpdate = 1;
                }
                else if (iSoldierState == SoldierState::JUMP || iSoldierState == SoldierState::JET_JUMP)
                {
                    m_bUnreachableState_SoldierUpdate =
                        (pState->m_uiSoldierStates &
                         ((1 << SoldierState::JUMP) | (1 << SoldierState::JET_JUMP))) == 0;
                }
                else if (iSoldierState > SoldierState::SPRINT)
                {
                    m_bUnreachableState_SoldierUpdate = 1;
                }
            }
        }
        if (m_bUnreachableState_SoldierUpdate && m_bUnreachableState_WeaponUpdate && !m_bInHeroFlee)
        {
            ExitState();
            EnterState(COMBO_IDLE);
            m_fComboStateTimer += fDt;
            pState = &m_pCombo->m_aState[m_eComboState];
            m_eComboStatePrev = m_eComboState;
            pStatePrev = 0;
            m_fComboStateDurationPrev = 0.0f;
            ReleaseDamageData();
        }
        else
        {
            bDone = UpdateState(fDt);
        }
    }
    m_fInputTimer += fDt;
    return true;
}

// FUNCTION: SWBF2 0x0068d270
WeaponMeleeClass::WeaponMeleeClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
    mDamage.mDamageOwner = owner;
    mDamage.mBaseValue = 0.0f;
    for (int i = 0; i < 8; ++i)
    {
        mDamage.mHealthScale[i] = 1.0f;
    }
    mDamage.mShieldScale = 1.0f;

    m_uiNumHitEffects = 0;
    m_pComboODF = 0;
    m_pCombo = 0;
    mImpactExplosion = 0;
    for (int i = 0; i < 5; ++i)
    {
        m_uiHitEffects[i] = 0;
    }
    m_uiDeflectEffect = 0;
    m_uiNumDamageEdges = 0;
    m_aDamageEdge = 0;
    m_fTurnOnDuration = 0.2f;
    m_uiNumWeapons = 1;
    m_auiHardpoint[0] = 0;
    m_amDeflectMatrix = 0;
    m_eDeflectMatrixMap = -1;
}

// FUNCTION: SWBF2 0x0068d420
WeaponMeleeClass::WeaponMeleeClass(WeaponMeleeClass* aParent, unsigned int aId)
    : WeaponClass(aParent, aId)
{
    DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
    mDamage.mDamageOwner = owner;
    mDamage.mBaseValue = 0.0f;
    for (int i = 0; i < 8; ++i)
    {
        mDamage.mHealthScale[i] = 1.0f;
    }
    mDamage.mShieldScale = 1.0f;

    m_pCombo = aParent->m_pCombo;
    m_pComboODF = 0;
    m_uiNumHitEffects = aParent->m_uiNumHitEffects;
    for (int i = 0; i < 5; ++i)
    {
        m_uiHitEffects[i] = aParent->m_uiHitEffects[i];
    }
    m_HitSound = aParent->m_HitSound;
    m_uiDeflectEffect = aParent->m_uiDeflectEffect;
    m_DeflectSound = aParent->m_DeflectSound;
    m_OnSound = aParent->m_OnSound;
    m_TurnOnSound = aParent->m_TurnOnSound;
    m_TurnOffSound = aParent->m_TurnOffSound;
    m_fTurnOnDuration = aParent->m_fTurnOnDuration;
    m_uiNumDamageEdges = aParent->m_uiNumDamageEdges;
    if (m_uiNumDamageEdges)
    {
        m_aDamageEdge = new DamageEdge[m_uiNumDamageEdges];
        memcpy(m_aDamageEdge, aParent->m_aDamageEdge, m_uiNumDamageEdges * sizeof(DamageEdge));
    }
    else
    {
        m_aDamageEdge = 0;
    }
    for (int i = 0; i < 3; ++i)
    {
        m_apWeaponModel[i] = aParent->m_apWeaponModel[i];
    }
    for (int i = 0; i < 4; ++i)
    {
        m_auiHardpoint[i] = aParent->m_auiHardpoint[i];
    }
    m_uiNumWeapons = aParent->m_uiNumWeapons;
    m_eDeflectMatrixMap = -1;
    m_amDeflectMatrix = 0;
    mImpactExplosion = aParent->mImpactExplosion;
}

static int ClampColorChannel(int i)
{
    return i < 0 ? 0 : (i > 255 ? 255 : i);
}

// SYNTHETIC: SWBF2 0x0068d3f0
// WeaponMeleeClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0068d6f0
WeaponMeleeClass::~WeaponMeleeClass()
{
    delete[] m_aDamageEdge;
    m_aDamageEdge = 0;
    if (m_amDeflectMatrix)
    {
        unsigned char* pAligned = reinterpret_cast<unsigned char*>(m_amDeflectMatrix);
        delete[] (pAligned - pAligned[-1]);
    }
    m_amDeflectMatrix = 0;
}

// FUNCTION: SWBF2 0x0068d780
RedColor aToColor(const char* pcArg)
{
    int r, g, b, a = 255;
    int iCount = sscanf(pcArg, "%d %d %d %d", &r, &g, &b, &a);
    if (iCount < 3)
    {
        iCount = sscanf(pcArg, "%2x%2x%2x%2x", &r, &g, &b, &a);
    }
    RedColor color;
    if (iCount < 3)
    {
        *(unsigned int*)&color = 0;
        return color;
    }
    color.r = (unsigned char)ClampColorChannel(r);
    color.g = (unsigned char)ClampColorChannel(g);
    color.b = (unsigned char)ClampColorChannel(b);
    color.a = (unsigned char)(iCount == 4 ? ClampColorChannel(a) : 255);
    return color;
}

// FUNCTION: SWBF2 0x0068d880
void WeaponMeleeClass::SetProperty(unsigned int aId, const char* aValue)
{
    // GLOBAL: SWBF2 0x01faa75c
    static WeaponMeleeClass* s_pThis;
    // GLOBAL: SWBF2 0x01fac3a0
    static RedModel* s_pAttachToModel;
    // GLOBAL: SWBF2 0x01fac39c
    static unsigned int s_uiAttachToHP;
    // GLOBAL: SWBF2 0x01fac3a4
    static unsigned int s_uiEdgeHP;

    if (s_pThis != this)
    {
        s_pThis = this;
        s_uiODFComboId = 0;
        s_pAttachToModel = 0;
        s_uiAttachToHP = 0;
        s_uiEdgeHP = 0;
    }

    switch (aId)
    {
    case 0x02BB1FE0:
    case 0x9B30E763:
        mImpactExplosion = ExplosionClass::Find(PblHash(aValue));
        break;

    case 0x03B58AB8:
    {
        int iNum = atoi(aValue);
        if (iNum < 1)
        {
            iNum = 1;
        }
        else if (iNum > 4)
        {
            iNum = 4;
        }
        _CreateDamageEdges(iNum);
        break;
    }

    case 0x042DABAE:
        m_TurnOffSound.SetID(aValue, false, false);
        break;
    case 0xC5BDD741:
        m_HitSound.SetID(aValue, false, false);
        break;
    case 0xC20DFE6F:
        m_DeflectSound.SetID(aValue, false, false);
        break;
    case 0xDE71B19F:
        m_OnSound.SetID(aValue, false, false);
        break;
    case 0xEA603E52:
        m_TurnOnSound.SetID(aValue, false, false);
        break;

    case 0x15A6D28C:
    {
        int iEdge = _AttachDamageEdge(s_pAttachToModel, s_uiAttachToHP, s_uiEdgeHP);
        if (iEdge >= 0)
        {
            m_aDamageEdge[iEdge].uiGlowTextureHash = PblHash(aValue);
        }
        break;
    }
    case 0x3C6468F4:
    case 0xFF773107:
    {
        int iEdge = _AttachDamageEdge(s_pAttachToModel, s_uiAttachToHP, s_uiEdgeHP);
        if (iEdge >= 0)
        {
            m_aDamageEdge[iEdge].uiTextureHash = PblHash(aValue);
        }
        break;
    }
    case 0x643005E0:
    case 0xE2AF0205:
    {
        int iEdge = _AttachDamageEdge(s_pAttachToModel, s_uiAttachToHP, s_uiEdgeHP);
        if (iEdge >= 0)
        {
            float fLength = LIGHTSABER_DEFAULT_LENGTH;
            sscanf(aValue, "%f", &fLength);
            m_aDamageEdge[iEdge].fLength = fLength;
        }
        break;
    }
    case 0x1F56AD0F:
    case 0xC9CBCCD8:
    {
        int iEdge = _AttachDamageEdge(s_pAttachToModel, s_uiAttachToHP, s_uiEdgeHP);
        if (iEdge >= 0)
        {
            float fWidth = LIGHTSABER_DEFAULT_WIDTH;
            sscanf(aValue, "%f", &fWidth);
            m_aDamageEdge[iEdge].fWidth = fWidth;
        }
        break;
    }
    case 0xFB0C9787:
    {
        int iEdge = _AttachDamageEdge(s_pAttachToModel, s_uiAttachToHP, s_uiEdgeHP);
        if (iEdge >= 0)
        {
            m_aDamageEdge[iEdge].colorTrail = aToColor(aValue);
        }
        break;
    }

    case 0x18BDF340:
        if (m_uiNumHitEffects < 5)
        {
            m_uiHitEffects[m_uiNumHitEffects++] = PblHash(aValue);
        }
        break;
    case 0xC0929477:
        m_uiHitEffects[0] = PblHash(aValue);
        m_uiNumHitEffects = 1;
        break;
    case 0xF1820751:
        m_uiDeflectEffect = PblHash(aValue);
        break;

    case 0xA7DDE527:
        if (sscanf(aValue, "%f", &m_fTurnOnDuration) != 1)
        {
            m_fTurnOnDuration = MELEE_DEFAULT_TURNONDURATION;
        }
        else if (m_fTurnOnDuration < 0.0f)
        {
            m_fTurnOnDuration = 0.0f;
        }
        break;

    case 0x2D32CE43:
    case 0xA7C25382:
    {

        char acBuf[257];
        strlcpy(acBuf, aValue, sizeof(acBuf));
        char* pcParent = 0;
        char* pcCombo = 0;
        char* pcName = strtok(acBuf, " ");
        while (pcName && !*pcName)
        {
            pcName = strtok(0, " ");
        }
        if (pcName)
        {
            do
                pcParent = strtok(0, " ");
            while (pcParent && !*pcParent);
            if (pcParent)
            {
                do
                    pcCombo = strtok(0, " ");
                while (pcCombo && !*pcCombo);
            }
        }
        if (aId == 0x2D32CE43)
        {
            m_pComboODF = 0;
            m_pCombo = pcCombo ? (Combo*)Combo::s_ComboTable.Find(PblHash(pcCombo)) : 0;
            if (m_pCombo)
            {
                mSoldierAnimationWeapon = SoldierAnimationBank::AddWeaponCombo(pcName, pcParent, pcCombo);
            }
            else
            {
                mSoldierAnimationWeapon = SoldierAnimationBank::INVALID_WEAPON;
            }
        }
        else
        {

            mSoldierAnimationWeapon = SoldierAnimationBank::AddWeaponCombo(pcName, pcParent, mFilename);
            if (mSoldierAnimationWeapon != SoldierAnimationBank::INVALID_WEAPON)
            {
                s_uiODFComboId = SoldierAnimationBank::GetComboNameHash(SoldierAnimationBank::GetWeaponCombo(
                    (SoldierAnimationBank::WEAPON)mSoldierAnimationWeapon));
                _CreateODFCombo(s_uiODFComboId);
            }
        }
        break;
    }

    case 0xC1A67869:
        if (_CreateODFCombo(s_uiODFComboId) && m_pComboODF->m_uiNumStates < 7)
        {
            m_pComboODF->AddDefaultAttackState();
        }
        break;

    case 0x7C9C8AE0:
        if (_CreateODFCombo(s_uiODFComboId) && m_pComboODF->m_uiNumStates < 7)
        {
            unsigned int uiState = m_pComboODF->m_uiNumStates;
            m_pComboODF->AddDefaultAttackState();
            char soundID[256] = "";
            float fDuration = MELEE_DEFAULT_STATE_DURATION;
            float fDamageTime = -MELEE_DEFAULT_DAMAGE_TIME;
            sscanf(aValue, "%f %f %s", &fDuration, &fDamageTime, soundID);
            if (fDuration >= 0.0f && (fDamageTime < 0.0f || fDamageTime > fDuration))
            {
                fDamageTime = fDuration * 0.2f;
            }
            Combo::State& state = m_pComboODF->m_aState[uiState];
            state.SetDuration(fDuration);
            state.m_aAttack->SetDamageTimeStart(fDamageTime, false);
            if (fDuration > 0.0f)
            {
                state.m_aAttack->SetDamageTimeEnd(fDuration * 0.2f + fDamageTime, false);
            }
            else
            {
                state.m_aAttack->SetDamageTimeEnd(fDamageTime + 0.16666667f, false);
            }
            state.m_sound.SetID(soundID, false, false);
        }
        break;

    case 0xB0D14CE0:
        if (atoi(aValue) && m_pComboODF && m_pComboODF->m_uiNumStates > 0)
        {
            m_pComboODF->AddDefaultDeflect();
        }
        break;

    case 0x45C02D38:
        if (m_pComboODF && m_pComboODF->m_uiNumStates > 2)
        {
            float fComboContinueTime = 0.0f;
            float fComboEndTime = 0.0f;
            sscanf(aValue, "%f %f", &fComboContinueTime, &fComboEndTime);
            for (unsigned int i = 1; i < m_pComboODF->m_uiNumStates; i += 2)
            {
                Combo::Condition* pCondition = m_pComboODF->m_aState[i].m_aTransition->m_aCondition;
                pCondition->SetTimeStart(fComboContinueTime, false);
                pCondition->SetTimeEnd(fComboEndTime, true, false);
            }
        }
        break;

    case 0xD0537A7C:
        if (m_pComboODF && m_pComboODF->m_uiNumStates >= 2)
        {
            m_pComboODF->m_aState[m_pComboODF->m_uiNumStates - 2].m_sound.SetID(aValue, false, false);
        }
        break;

    case 0x49021F55:
    case 0x1E111A47:
        if (m_pComboODF && m_pComboODF->m_uiNumStates >= 2)
        {
            unsigned int uiState = m_pComboODF->m_uiNumStates - (aId == 0x49021F55 ? 2 : 1);
            float fDuration = MELEE_DEFAULT_STATE_DURATION;
            if (sscanf(aValue, "%f", &fDuration) != 1)
            {
                fDuration = -1.0f;
            }
            m_pComboODF->m_aState[uiState].SetDuration(fDuration);
        }
        break;

    case 0x91F410CB:
        if (m_pComboODF && m_pComboODF->m_uiNumStates >= 2)
        {
            Combo::State& state = m_pComboODF->m_aState[m_pComboODF->m_uiNumStates - 1];
            float fBreakOutTime = -1.0f;
            if (sscanf(aValue, "%f", &fBreakOutTime) == 1 && fBreakOutTime >= 0.0f)
            {
                Combo::Transition* pTransition = state.m_uiNumTransitions ? state.m_aTransition
                                                                         : state.AddTransition(0);
                if (!pTransition)
                {
                    break;
                }
                Combo::Condition* pCondition = pTransition->AddCondition();
                if (pCondition)
                {
                    pCondition->SetBreak(true);
                    pCondition->SetTimeStart(fBreakOutTime, false);
                    pCondition->SetTimeEnd(0.0f, true, false);
                    pCondition->SetInputMag(0.5477f, false);
                }
            }
        }
        break;

    case 0x308AD121:
        if (m_pComboODF && m_pComboODF->m_uiNumStates >= 2)
        {
            Combo::Attack* pAttack = m_pComboODF->m_aState[m_pComboODF->m_uiNumStates - 2].m_aAttack;
            float fDamageTime = -1.0f;
            if (sscanf(aValue, "%f", &fDamageTime) != 1)
            {
                fDamageTime = -1.0f;
            }
            if (fDamageTime < 0.0f)
            {
                pAttack->SetDamageTimeStart(-fDamageTime, true);
                pAttack->SetDamageTimeEnd(-fDamageTime - 0.16666667f, true);
            }
            else
            {
                pAttack->SetDamageTimeStart(fDamageTime, false);
                pAttack->SetDamageTimeEnd(fDamageTime + 0.16666667f, false);
            }
        }
        break;

    case 0x49F8B243:
        if (m_pComboODF && m_pComboODF->m_uiNumStates >= 2)
        {
            Combo::Attack* pAttack = m_pComboODF->m_aState[m_pComboODF->m_uiNumStates - 2].m_aAttack;
            float fDamageDistance = MELEE_DEFAULT_DAMAGE_DISTANCE;
            if (sscanf(aValue, "%f", &fDamageDistance) != 1)
            {
                fDamageDistance = MELEE_DEFAULT_DAMAGE_DISTANCE;
            }
            else if (fDamageDistance < 0.0f)
            {
                fDamageDistance = 0.0f;
            }
            pAttack->SetDamageLength(fDamageDistance, false);
        }
        break;

    case 0xF34ADD68:
        if (m_pComboODF && m_pComboODF->m_uiNumStates >= 2)
        {
            Combo::Attack* pAttack = m_pComboODF->m_aState[m_pComboODF->m_uiNumStates - 2].m_aAttack;
            float fDamage = MELEE_DEFAULT_DAMAGE;
            if (sscanf(aValue, "%f", &fDamage) != 1)
            {
                fDamage = MELEE_DEFAULT_DAMAGE;
            }
            else if (fDamage < 0.0f)
            {
                fDamage = 0.0f;
            }
            pAttack->SetDamage(fDamage);
        }
        break;

    case 0x4274FC96:
        WeaponClass::SetProperty(aId, aValue);
        s_pAttachToModel = mModel;
        if (s_pAttachToModel)
        {
            s_uiAttachToHP = 0x2B960099;
            s_uiEdgeHP = PblTEMPHash(aValue);
            if (!s_pAttachToModel->GetParentBone(s_uiEdgeHP))
            {
                s_uiEdgeHP = 0;
            }
        }
        else
        {
            s_uiEdgeHP = 0;
            s_uiAttachToHP = 0;
        }
        break;

    case 0x47C86B4A:
        WeaponClass::SetProperty(aId, aValue);
        s_pAttachToModel = mModel;
        if (s_pAttachToModel)
        {
            s_uiAttachToHP = 0x2B960099;
            s_uiEdgeHP = 0xB3EA0EE8;
            if (!s_pAttachToModel->GetParentBone(s_uiEdgeHP))
            {
                s_uiEdgeHP = 0;
            }
        }
        else
        {
            s_uiEdgeHP = 0;
            s_uiAttachToHP = 0;
        }
        break;

    case 0xCA67EAC4:
        if (m_uiNumWeapons >= 4)
        {
            break;
        }
        s_pAttachToModel = (RedModel*)RedModel::Find(PblHash(aValue));
        if (s_pAttachToModel)
        {
            s_uiAttachToHP = 0xB7EC1D31;
            s_uiEdgeHP = 0xB3EA0EE8;
            if (!s_pAttachToModel->GetParentBone(s_uiEdgeHP))
            {
                s_uiEdgeHP = 0;
            }
        }
        else
        {
            s_uiEdgeHP = 0;
            s_uiAttachToHP = 0;
        }
        break;

    case 0x9E10B228:
    {
        char acEdgeHP[64];
        char acAttachToHP[64];
        int iCount = sscanf(aValue, "%s %s", acEdgeHP, acAttachToHP);
        if (iCount < 1)
        {
            break;
        }
        if (s_pAttachToModel)
        {
            s_uiAttachToHP = iCount < 2 ? 0xB7EC1D31 : PblTEMPHash(acAttachToHP);
            s_uiEdgeHP = PblTEMPHash(acEdgeHP);
            if (!s_pAttachToModel->GetParentBone(s_uiEdgeHP))
            {
                s_uiEdgeHP = 0;
            }
        }
        else
        {
            s_uiEdgeHP = 0;
            s_uiAttachToHP = 0;
        }
        break;
    }

    case 0x94F22065:
    {
        char acAttachToHP[64];
        float fDirX = 0.0f, fDirY = 0.0f, fDirZ = 1.0f;
        float fX = 0.0f, fY = 0.0f, fZ = 0.0f;
        int iCount = sscanf(aValue, "%s %f %f %f %f %f %f", acAttachToHP, &fDirX, &fDirY, &fDirZ, &fX, &fY, &fZ);
        if (iCount < 1)
        {
            break;
        }
        s_pAttachToModel = 0;
        s_uiAttachToHP = PblTEMPHash(acAttachToHP);
        s_uiEdgeHP = 0;
        int iEdge = _AttachDamageEdge(0, s_uiAttachToHP, 0);
        if (iEdge < 0)
        {
            s_uiAttachToHP = 0;
            break;
        }
        s_uiEdgeHP = m_aDamageEdge[iEdge].uiEdgeHP;
        if (iCount >= 4)
        {
            PblVector3 vDir(fDirX, fDirY, fDirZ);
            PblVector3 vNormal;
            D3DXVec3Normalize(&vNormal, &vDir);
            m_aDamageEdge[iEdge].vOffsetDir = vNormal;
            if (iCount >= 7)
            {
                m_aDamageEdge[iEdge].vOffsetPos = PblVector3(fX, fY, fZ);
            }
        }
        break;
    }

    default:
        WeaponClass::SetProperty(aId, aValue);
        break;
    }
}

// FUNCTION: SWBF2 0x0068e7c0
WeaponClass* WeaponMeleeClass::Derive(unsigned int uHash)
{
    return new WeaponMeleeClass(this, uHash);
}

// FUNCTION: SWBF2 0x0068ec10
void WeaponMeleeClass::Init()
{
    Combo::Init();
}

// FUNCTION: SWBF2 0x0068ec20
void WeaponMeleeClass::PostLoad()
{
    Combo::PostLoad();
    WeaponMelee::_InitDamageDataPool();
    WeaponMelee::_InitTrailDataPool();
}

// FUNCTION: SWBF2 0x0068ec40
void WeaponMeleeClass::ReadCombo(PblFileChunk& Chunk)
{
    PblConfig config(Chunk);
    if (!Combo::s_ComboTable.Find(config.GetNameHash()) && Combo::sMemoryPool.mUsed < Combo::sMemoryPool.GetCount())
    {
        Combo* pCombo = new Combo;
        pCombo->ReadConfig(config);
    }
}

// FUNCTION: SWBF2 0x0068ed40
int WeaponMeleeClass::_AttachDamageEdge(const RedModel* pAttachToModel, unsigned int uiAttachModelToHP,
                                        unsigned int uiAttachToModelHP)
{
    static const unsigned int DEFAULT_NUM_DAMAGE_EDGES = 1;

    if (!uiAttachModelToHP)
    {
        return -1;
    }

    unsigned int uiWeapon = 0;
    PblVector3 vOffsetPos;
    PblVector3 vOffsetDir;
    if (pAttachToModel)
    {
        if (!uiAttachToModelHP)
        {
            return -1;
        }
        if (pAttachToModel == mModel && !m_auiHardpoint[0])
        {
            m_auiHardpoint[0] = uiAttachModelToHP;
        }
        else if (pAttachToModel != mModel || uiAttachModelToHP != m_auiHardpoint[0])
        {
            for (uiWeapon = 1; uiWeapon < m_uiNumWeapons; ++uiWeapon)
            {
                if (m_apWeaponModel[uiWeapon - 1] == pAttachToModel && m_auiHardpoint[uiWeapon] == uiAttachModelToHP)
                {
                    break;
                }
            }
            if (uiWeapon >= m_uiNumWeapons)
            {
                if (m_uiNumWeapons >= 4)
                {
                    return -1;
                }
                m_apWeaponModel[uiWeapon - 1] = (RedModel*)pAttachToModel;
                m_auiHardpoint[uiWeapon] = uiAttachModelToHP;
                ++m_uiNumWeapons;
            }
        }
        for (unsigned int i = 0; i < m_uiNumDamageEdges; ++i)
        {
            if (m_aDamageEdge[i].uiAttachToWeapon == uiWeapon && m_aDamageEdge[i].uiEdgeHP == uiAttachToModelHP)
            {
                return i;
            }
        }

        PblMatrix mHP;
        if (pAttachToModel->GetParentBoneAndOffsetMatrix(uiAttachToModelHP, mHP))
        {
            vOffsetPos = mHP.GetTranslation();
            vOffsetDir = mHP.GetZAxis();
        }
        else
        {
            vOffsetPos = PblVector3(0.0f, 0.0f, 0.0f);
            vOffsetDir = gVectorUnitZ;
        }
    }
    else
    {
        if (m_uiNumDamageEdges && uiAttachToModelHP > m_uiNumDamageEdges)
        {
            return -1;
        }
        for (uiWeapon = 1; uiWeapon < m_uiNumWeapons; ++uiWeapon)
        {
            if (!m_apWeaponModel[uiWeapon - 1] && m_auiHardpoint[uiWeapon] == uiAttachModelToHP)
            {
                break;
            }
        }
        if (uiWeapon >= m_uiNumWeapons)
        {
            if (m_uiNumWeapons >= 4)
            {
                return -1;
            }
            m_apWeaponModel[uiWeapon - 1] = 0;
            m_auiHardpoint[uiWeapon] = uiAttachModelToHP;
            ++m_uiNumWeapons;
        }
        if (!uiAttachToModelHP)
        {
            uiAttachToModelHP = 1;
            for (unsigned int i = 0; i < m_uiNumDamageEdges; ++i)
            {
                if (m_aDamageEdge[i].uiAttachToWeapon == uiWeapon)
                {
                    unsigned int uiEdge = m_aDamageEdge[i].uiEdgeHP;
                    if (uiEdge <= m_uiNumDamageEdges && uiAttachToModelHP <= uiEdge)
                    {
                        uiAttachToModelHP = uiEdge + 1;
                    }
                }
            }
        }
        else
        {
            for (unsigned int i = 0; i < m_uiNumDamageEdges; ++i)
            {
                if (m_aDamageEdge[i].uiAttachToWeapon == uiWeapon && m_aDamageEdge[i].uiEdgeHP == uiAttachToModelHP)
                {
                    return i;
                }
            }
        }
        vOffsetPos = PblVector3(0.0f, 0.0f, 0.0f);
        vOffsetDir = gVectorUnitZ;
    }

    int iEdge = 0;
    if (!m_aDamageEdge)
    {
        _CreateDamageEdges(m_uiNumDamageEdges ? m_uiNumDamageEdges : DEFAULT_NUM_DAMAGE_EDGES);
    }
    else
    {
        for (iEdge = 0; iEdge < (int)m_uiNumDamageEdges; ++iEdge)
        {
            if (!m_aDamageEdge[iEdge].uiEdgeHP)
            {
                break;
            }
        }
        if (iEdge >= (int)m_uiNumDamageEdges)
        {
            return -1;
        }
    }

    DamageEdge& edge = m_aDamageEdge[iEdge];
    edge.uiAttachToWeapon = uiWeapon;
    edge.uiEdgeHP = uiAttachToModelHP;
    edge.vOffsetPos = vOffsetPos;
    edge.vOffsetDir = vOffsetDir;
    edge.fLength = LIGHTSABER_DEFAULT_LENGTH;
    edge.fWidth = LIGHTSABER_DEFAULT_WIDTH;
    edge.uiTextureHash = 0;
    edge.uiGlowTextureHash = 0x79B932AB;
    edge.colorTrail = LIGHTSABER_DEFAULT_TRAIL_COLOR;
    return iEdge;
}

// FUNCTION: SWBF2 0x0068f0d0
WeaponMeleeClass::DamageEdge* WeaponMeleeClass::_CreateDamageEdges(unsigned int uiNumEdges)
{
    if (m_aDamageEdge && uiNumEdges != m_uiNumDamageEdges)
    {
        delete m_aDamageEdge;
        m_aDamageEdge = 0;
    }
    if (!m_aDamageEdge)
    {
        m_uiNumDamageEdges = uiNumEdges;
        m_aDamageEdge = new DamageEdge[m_uiNumDamageEdges];
        memset(m_aDamageEdge, 0, m_uiNumDamageEdges * sizeof(DamageEdge));
    }
    return m_aDamageEdge;
}

// FUNCTION: SWBF2 0x0068e8c0
bool WeaponMeleeClass::IsLightSaber() const
{
    for (unsigned int i = 0; i < m_uiNumDamageEdges; ++i)
    {
        if (m_aDamageEdge[i].uiTextureHash)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0068eb70
void WeaponMeleeClass::PlayTurnOnSound() const
{
    m_TurnOnSound.Play(0, 0, true, true);
}

// FUNCTION: SWBF2 0x0068eb90
void WeaponMeleeClass::PlayTurnOffSound() const
{
    m_TurnOffSound.Play(0, 0, true, true);
}

// GLOBAL: SWBF2 0x01f9a8f0
// SYNTHETIC: SWBF2 0x00407220
// `dynamic initializer for 'gDefaultHash''
unsigned int gDefaultHash = PblHash("default");
