#include <math.h>

#include "LightsaberTrailEffect.h"

// GLOBAL: SWBF2 0x01e573ec
LightsaberTrailEffect* LightsaberTrailEffect::s_pInstance;

// FUNCTION: SWBF2 0x00573ea0
void LightsaberTrailEffect::Create()
{
    s_pInstance = new LightsaberTrailEffect();
}

// FUNCTION: SWBF2 0x00573e30
void LightsaberTrailEffect::Init()
{
    m_fLIGHTSABER_TRAIL_PERSIST_TIME = 0.12f;
    m_fLIGHTSABER_TRAIL_FADE_TIME = 0.02f;
    m_fLIGHTSABER_TRAIL_FADE_DISTANCE = 0.35f;
    m_fLIGHTSABER_TRAIL_CORE_MAX_TIME = 0.4f;
    m_fLIGHTSABER_TRAIL_CORE_MAX_DISTANCE = 0.8f;
    m_fLIGHTSABER_TRAIL_CORE_WIDTH = 0.3f;
    m_fLIGHTSABER_TRAIL_MAX_TIME_PER_STEP = 0.01f;
    m_fLIGHTSABER_TRAIL_MAX_DISTANCE_PER_STEP = 0.1f;
    _PlatformInit();
}

LightsaberTrailEffect::~LightsaberTrailEffect()
{
}

// SYNTHETIC: SWBF2 0x00573d30
// LightsaberTrailEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00573d20
void LightsaberTrailEffect::Render(int)
{
}

// FUNCTION: SWBF2 0x00573e70
void LightsaberTrailEffect::Cleanup()
{
    _PlatformCleanup();
}

// FUNCTION: SWBF2 0x00573e80
void LightsaberTrailEffect::ReadConfig(PblConfig&)
{
}

// FUNCTION: SWBF2 0x00573e90
FLEffectClass* LightsaberTrailEffect::BuildClass(PblConfig&) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00573f40
void LightsaberTrailEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV,
                                                         const PblMatrix* pM);

// GLOBAL: SWBF2 0x01e56058
namespace GameLoop { float sClientDeltaTime; }

// FUNCTION: SWBF2 0x00573f60
LightsaberTrail::LightsaberTrail(const LightsaberTrailClass* pClass)
    : m_pClass(const_cast<LightsaberTrailClass*>(pClass))
    , m_vPosPrev(3.402823466e+38f, 3.402823466e+38f, 3.402823466e+38f)
    , m_vDirPrev(3.402823466e+38f, 3.402823466e+38f, 3.402823466e+38f)
{
    m_bTrailOn = false;
    m_bTrailWasOn = false;
    m_bEffectActive = true;
    m_fTimer = 0.0f;
    m_uiSampleNext = 0;
    m_uiSampleFirst = 0;
    m_uiNumSamples = 0;
}

LightsaberTrail::~LightsaberTrail()
{
    m_bEffectActive = false;
}

// SYNTHETIC: SWBF2 0x00573df0
// LightsaberTrail::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0057552c
// [thunk]:LightsaberTrail::`scalar deleting destructor' adjustor{24}

// FUNCTION: SWBF2 0x00573da0
bool LightsaberTrail::IsEffectActive() const
{
    return m_bEffectActive;
}

// FUNCTION: SWBF2 0x00573db0
void LightsaberTrail::ActivateEffect()
{
    m_bEffectActive = true;
}

// FUNCTION: SWBF2 0x00573dc0
void LightsaberTrail::DeactivateEffect()
{
    m_bEffectActive = false;
}

// FUNCTION: SWBF2 0x00573dd0
void LightsaberTrail::AddToRenderList()
{
}

// FUNCTION: SWBF2 0x00573de0
void LightsaberTrail::RemoveFromRenderList()
{
}

// FUNCTION: SWBF2 0x00574040
void LightsaberTrail::UpdatedWorldMatrix()
{
    const float fDeltaTime = GameLoop::sClientDeltaTime;
    if (fDeltaTime > 0.0f)
    {
        PblVector3 vPos, vDir;
        D3DXVec3TransformCoord(&vPos, &m_pClass->m_vOffsetPos, &m_mWorldMatrix);
        D3DXVec3TransformNormal(&vDir, &m_pClass->m_vOffsetDir, &m_mWorldMatrix);

        const float dx = vPos.x - m_vPosPrev.x;
        const float dy = vPos.y - m_vPosPrev.y;
        const float dz = vPos.z - m_vPosPrev.z;
        if (dx * dx + dy * dy + dz * dz > 100.0f)
        {
            m_vPosPrev = vPos;
            m_vDirPrev = vDir;
        }

        _Update(fDeltaTime, vPos, vDir, m_vPosPrev, m_vDirPrev, m_bTrailOn && m_bEffectActive);
        m_vPosPrev = vPos;
        m_vDirPrev = vDir;
    }
}

// FUNCTION: SWBF2 0x00574150
void LightsaberTrail::_Update(float fDeltaTime, const PblVector3& vPos, const PblVector3& vDir,
                              const PblVector3& vPosPrev, const PblVector3& vDirPrev, bool bTrailOn)
{
    if (fDeltaTime > 0.0f)
    {
        m_fTimer += fDeltaTime;

        unsigned char i;
        for (i = 0; i < m_uiNumSamples; ++i)
        {
            unsigned char uiSample = (unsigned char)((m_uiSampleFirst + i) % MAX_SAMPLES_PER_TRAIL);
            if (m_afTime[uiSample] >=
                m_fTimer - LightsaberTrailEffect::s_pInstance->m_fLIGHTSABER_TRAIL_PERSIST_TIME)
            {
                if (m_afDist[uiSample] > 0.0f)
                {
                    --i;
                }
                break;
            }
            m_afDist[uiSample] = 0.0f;
        }

        if (i)
        {
            if (i < m_uiNumSamples)
            {
                m_uiNumSamples -= i;
            }
            else
            {
                m_uiNumSamples = m_afTime[m_uiNumSamples - 1] >= m_fTimer - fDeltaTime * 1.5f;
            }
            m_uiSampleFirst = (unsigned char)((m_uiSampleNext - m_uiNumSamples + MAX_SAMPLES_PER_TRAIL) %
                                              MAX_SAMPLES_PER_TRAIL);
        }

        if (bTrailOn || m_bTrailWasOn)
        {
            if (!m_uiNumSamples || m_fTimer - GetLastTime() > fDeltaTime * 1.5f)
            {
                _AddSample(m_fTimer - fDeltaTime, vPosPrev, vDirPrev, false);
            }
            _AddSample(m_fTimer, vPos, vDir, true);
        }
        m_bTrailWasOn = bTrailOn;
    }
}

extern PblVector3 gVectorZero;

namespace
{

inline float ArcLengthEstimate(const PblVector3& v0, const PblVector3& v1)
{
    const float fLen0 = sqrtf(v0.x * v0.x + v0.y * v0.y + v0.z * v0.z);
    const float fLen1 = sqrtf(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
    float fLen = (fLen1 + fLen0) * 0.5f;
    if (fLen0 > 0.001f && fLen1 > 0.001f)
    {
        fLen = ((v0.x * v1.x * 0.25f + v0.y * v1.y * 0.25f + v0.z * v1.z * 0.25f) / (fLen1 * fLen0) + 0.75f) *
               fLen;
    }
    return fLen;
}
}

// FUNCTION: SWBF2 0x005742f0
void LightsaberTrail::_AddSample(float fTime, const PblVector3& vPos, const PblVector3& vDir, bool bHead)
{
    m_afTime[m_uiSampleNext] = fTime;
    m_avPos[m_uiSampleNext] = vPos;
    m_avDir[m_uiSampleNext] = vDir;

    if (m_uiNumSamples && bHead)
    {
        const int iCur = m_uiSampleNext;
        const int iPrev = (iCur + MAX_SAMPLES_PER_TRAIL - 1) % MAX_SAMPLES_PER_TRAIL;
        const float fDeltaTime = fTime - m_afTime[iPrev];
        const float fInvDeltaTime = 1.0f / fDeltaTime;

        PblVector3 vVel;
        vVel.x = (vPos.x - m_avPos[iPrev].x) * fInvDeltaTime;
        vVel.y = (vPos.y - m_avPos[iPrev].y) * fInvDeltaTime;
        vVel.z = (vPos.z - m_avPos[iPrev].z) * fInvDeltaTime;

        const PblVector3& vDirPrev = m_avDir[iPrev];
        const float fDot = vDir.x * vDirPrev.x + vDir.y * vDirPrev.y + vDir.z * vDirPrev.z;

        PblVector3 vOmgPrev;
        if (fDot >= 0.999f)
        {
            m_avOmg[iCur].x = (vDir.x - vDirPrev.x) * fInvDeltaTime;
            m_avOmg[iCur].y = (vDir.y - vDirPrev.y) * fInvDeltaTime;
            m_avOmg[iCur].z = (vDir.z - vDirPrev.z) * fInvDeltaTime;
            vOmgPrev = m_avOmg[m_uiSampleNext];
        }
        else
        {
            const float fLen = sqrtf(fDot * 2.0f + 2.0f);
            const float fA = ((2.0f - fLen) * (fInvDeltaTime * 4.0f)) / ((1.0f - fDot) * fLen);
            const float fB = fA * fDot;
            m_avOmg[iCur].x = vDir.x * fB - vDirPrev.x * fA;
            m_avOmg[iCur].y = vDir.y * fB - vDirPrev.y * fA;
            m_avOmg[iCur].z = vDir.z * fB - vDirPrev.z * fA;
            vOmgPrev.x = vDir.x * fA - vDirPrev.x * fB;
            vOmgPrev.y = vDir.y * fA - vDirPrev.y * fB;
            vOmgPrev.z = vDir.z * fA - vDirPrev.z * fB;
        }

        if (m_uiNumSamples == 1 || !(m_afDist[iPrev] > 0.0f))
        {
            m_avVel[iPrev] = vVel;
            m_avOmg[iPrev] = vOmgPrev;
        }
        else
        {
            const float fT = (fDeltaTime * 0.5f) /
                             (fTime - m_afTime[(m_uiSampleNext + MAX_SAMPLES_PER_TRAIL - 1) % MAX_SAMPLES_PER_TRAIL]);
            const float fU = 1.0f - fT;
            m_avVel[iPrev].x = m_avVel[iPrev].x * fT + vVel.x * fU;
            m_avVel[iPrev].y = m_avVel[iPrev].y * fT + vVel.y * fU;
            m_avVel[iPrev].z = m_avVel[iPrev].z * fT + vVel.z * fU;
            m_avOmg[iPrev].x = m_avOmg[iPrev].x * fT + vOmgPrev.x * fU;
            m_avOmg[iPrev].y = m_avOmg[iPrev].y * fT + vOmgPrev.y * fU;
            m_avOmg[iPrev].z = m_avOmg[iPrev].z * fT + vOmgPrev.z * fU;
        }

        PblVector3& vVelCur = m_avVel[m_uiSampleNext];
        vVelCur.x = vVel.x * 2.0f - m_avVel[iPrev].x;
        vVelCur.y = vVel.y * 2.0f - m_avVel[iPrev].y;
        vVelCur.z = vVel.z * 2.0f - m_avVel[iPrev].z;

        const int iNext = m_uiSampleNext;
        const float fDistVel = ArcLengthEstimate(m_avVel[iNext], m_avVel[iPrev]) * fDeltaTime;

        PblVector3 vTip, vTipPrev;
        vTip.x = m_avVel[iNext].x + m_avOmg[iNext].x;
        vTip.y = m_avVel[iNext].y + m_avOmg[iNext].y;
        vTip.z = m_avVel[iNext].z + m_avOmg[iNext].z;
        vTipPrev.x = m_avVel[iPrev].x + m_avOmg[iPrev].x;
        vTipPrev.y = m_avVel[iPrev].y + m_avOmg[iPrev].y;
        vTipPrev.z = m_avVel[iPrev].z + m_avOmg[iPrev].z;
        const float fDistTip = ArcLengthEstimate(vTip, vTipPrev) * fDeltaTime;

        float fDist = fDistVel;
        if (fDistTip > fDist)
        {
            fDist = fDistTip;
        }
        if (fDist < 0.001f)
        {
            fDist = 0.001f;
        }
        m_afDist[iNext] = fDist;
    }
    else
    {
        m_avOmg[m_uiSampleNext] = gVectorZero;
        m_avVel[m_uiSampleNext] = m_avOmg[m_uiSampleNext];
        m_afDist[m_uiSampleNext] = 0.0f;
    }

    m_uiSampleNext = (unsigned char)((m_uiSampleNext + 1) % MAX_SAMPLES_PER_TRAIL);
    if (m_uiNumSamples < MAX_SAMPLES_PER_TRAIL)
    {
        ++m_uiNumSamples;
    }
    else
    {
        m_uiSampleFirst = (unsigned char)((m_uiSampleFirst + 1) % MAX_SAMPLES_PER_TRAIL);
    }
}

// FUNCTION: SWBF2 0x00574b70
float LightsaberTrail::GetLastTime() const
{
    if (m_uiNumSamples)
    {
        return m_afTime[(m_uiSampleNext + MAX_SAMPLES_PER_TRAIL - 1) % MAX_SAMPLES_PER_TRAIL];
    }
    return -3.402823466e+38f;
}
