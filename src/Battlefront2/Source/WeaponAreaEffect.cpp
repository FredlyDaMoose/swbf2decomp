#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "WeaponAreaEffect.h"
#include "Aimer.h"
#include "CollisionManager.h"
#include "Controllable.h"
#include "Damageable.h"
#include "EnergyBar.h"
#include "EntitySoldier.h"
#include "GameObject.h"
#include "NetGame.h"
#include "SoldierAnimationBank.h"
#include "Trigger.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include <new>

extern bool netIsLocalTurn;
extern int gDebugCount;

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

namespace
{
    EntityClass* GetEntityClass(GameObject* pObject)
    {
        typedef EntityClass* (__thiscall *Fn)(GameObject*);
        return pObject->GetEntityClass();
    }
}

// GLOBAL: SWBF2 0x01f9a884
// SYNTHETIC: SWBF2 0x00407060
// `dynamic initializer for 'WeaponAreaEffect::rttiHashWeaponAreaEffect''
PblHash WeaponAreaEffect::rttiHashWeaponAreaEffect("WeaponAreaEffect");

// FUNCTION: SWBF2 0x0067c170
bool WeaponAreaEffect::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponAreaEffect || Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0067c1a0
unsigned int WeaponAreaEffect::GetDerivedRtti() const
{
    return rttiHashWeaponAreaEffect;
}

// FUNCTION: SWBF2 0x0067c1b0
const char* WeaponAreaEffect::GetDerivedRttiName() const
{
    return "WeaponAreaEffect";
}

// FUNCTION: SWBF2 0x0067c1c0
void WeaponAreaEffect::SetOrdnance(Ordnance*)
{
}

// FUNCTION: SWBF2 0x0067c1d0
Ordnance* WeaponAreaEffect::CreateOrdnance(OrdnanceDesc&)
{
    return 0;
}

// FUNCTION: SWBF2 0x0067c260
WeaponAreaEffect::WeaponAreaEffect(const WeaponAreaEffectClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    for (int i = 0; i < MAX_TARGETS; ++i)
    {
        m_apTarget[i] = 0;
        m_afTargetRange[i] = 0.0f;
    }
    m_fStrength = 0.0f;
    m_iNumTargets = 0;
    m_bFired = false;
}

// FUNCTION: SWBF2 0x0067c360
WeaponAreaEffect::~WeaponAreaEffect()
{
}

// SYNTHETIC: SWBF2 0x0067c1e0
// WeaponAreaEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0067c3c0
bool WeaponAreaEffect::Update(float fDeltaTime)
{
    return Weapon::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x0067c3e0
void WeaponAreaEffect::NotifySoldierState(int iState, float fTime, bool b0, bool b1, const PblVector3& vec)
{
    Weapon::NotifySoldierState(iState, fTime, b0, b1, vec);
}

// FUNCTION: SWBF2 0x0067c410
bool WeaponAreaEffect::OverrideSoldierControls()
{
    if (mState == FIRE && _GetClass()->m_fThrustFactor != 1.0f)
    {
        mOwner->mControlMove *= _GetClass()->m_fThrustFactor;
        mOwner->mControlStrafe *= _GetClass()->m_fThrustFactor;
        Weapon::OverrideSoldierControls();
        return true;
    }
    return Weapon::OverrideSoldierControls();
}

// FUNCTION: SWBF2 0x0067c480
bool WeaponAreaEffect::OverrideSoldierEnergyRestore(float& fRestore) const
{
    const WeaponAreaEffectClass* pClass = _GetClass();
    if (pClass->m_fEnergyDrainRate != 0.0f)
    {
        switch (pClass->m_eFireType)
        {
        case WeaponAreaEffectClass::FIRE_HOLD:
            if (mState == FIRE && m_bFired)
            {
                fRestore = -pClass->m_fEnergyDrainRate;
                return true;
            }
            break;
        case WeaponAreaEffectClass::FIRE_CHARGE:
            if (mState == CHARGE)
            {
                fRestore = -pClass->m_fEnergyDrainRate;
                return true;
            }
            break;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0067c4e0
void WeaponAreaEffect::EnterFire()
{
    if (NetGame::IsClient())
    {
        ++gDebugCount;
    }
    Weapon::EnterFire();
    const WeaponAreaEffectClass* pClass = _GetClass();
    if (pClass->m_eFireType == WeaponAreaEffectClass::FIRE_CHARGE && pClass->m_fChargeMaxTime > mStateTimer)
    {
        m_fStrength = (1.0f - pClass->m_fChargeMinStrength) * mStateTimer / pClass->m_fChargeMaxTime +
                      pClass->m_fChargeMinStrength;
    }
    else
    {
        m_fStrength = 1.0f;
    }
    mStateTimer = 0.0f;
    const float fEnergyDrain = pClass->mEnergyDrain;
    if (EnergyBar* pEnergy = mOwner->GetEnergyBar())
    {
        pEnergy->SpendEnergy(fEnergyDrain);
    }
    m_bFired = false;
    if (pClass->mInstantPlayFireAnim)
    {
        mFiredFlag = 1;
    }
    if (pClass->mTriggerSingle && pClass->m_eFireType != WeaponAreaEffectClass::FIRE_HOLD)
    {
        mTrigger->ClearUntilPressed();
    }
}

// FUNCTION: SWBF2 0x0067c5d0
bool WeaponAreaEffect::UpdateFire(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    if (!m_bFired)
    {
        if (_GetClass()->m_fInitialSalvoDelay > mStateTimer)
        {
            return true;
        }
        if (!_GetClass()->mInstantPlayFireAnim)
        {
            mFiredFlag = 1;
        }
        Fire();
        if (!(NetGame::IsEnabled() && NetGame::IsClient()) && SetTargets())
        {
            ApplyEffectsToTargets(fDeltaTime);
        }
        mStateTimer = 0.0f;
        m_bFired = true;
    }
    else if (!(NetGame::IsEnabled() && NetGame::IsClient()) && _GetClass()->m_eFireType == WeaponAreaEffectClass::FIRE_HOLD)
    {
        if (mStateTimer > 0.3333f)
        {
            UpdateTargets();
            mStateTimer = 0.0f;
        }
        if (m_iNumTargets)
        {
            ApplyEffectsToTargets(fDeltaTime);
        }
    }
    return Weapon::UpdateFire(fDeltaTime);
}

// FUNCTION: SWBF2 0x0067c700
void WeaponAreaEffect::ExitFire()
{
    m_iNumTargets = 0;
    m_fStrength = 0.0f;
    Weapon::ExitFire();
    if (_GetClass()->mTriggerSingle && _GetClass()->m_eFireType == WeaponAreaEffectClass::FIRE_HOLD)
    {
        mTrigger->ClearUntilPressed();
    }
    m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot);
}

// FUNCTION: SWBF2 0x0067c760
void WeaponAreaEffect::EnterCharge()
{
    mStateTimer = 0.0f;
    mStateLimit = _GetClass()->m_fChargeMaxTime;
    Weapon::EnterCharge();
}

// FUNCTION: SWBF2 0x0067c780
bool WeaponAreaEffect::UpdateCharge(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    if (mStateTimer >= mStateLimit)
    {
        mStateTimer = mStateLimit;
    }
    return Weapon::UpdateCharge(fDeltaTime);
}

// FUNCTION: SWBF2 0x0067c7c0
void WeaponAreaEffect::ExitCharge()
{
    Weapon::ExitCharge();
}

// FUNCTION: SWBF2 0x0067c7d0
void WeaponAreaEffect::EnterEmpty()
{
    Weapon::EnterEmpty();
}

// FUNCTION: SWBF2 0x0067c7e0
bool WeaponAreaEffect::CheckCharge() const
{
    if (!mTrigger || _GetClass()->m_eFireType != WeaponAreaEffectClass::FIRE_CHARGE || CheckOverheat())
    {
        return false;
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x0067c810
bool WeaponAreaEffect::CheckFire() const
{
    if (!mTrigger)
    {
        return false;
    }
    const float fEnergyDrain = _GetClass()->mEnergyDrain;
    const EnergyBar* pEnergy = mOwner->GetEnergyBar();
    if (pEnergy && pEnergy->IsOverheating() && fEnergyDrain > 0.0f)
    {
        return false;
    }
    if (_GetClass()->m_fEnergyDrainRate > 0.0f)
    {
        mOwner->GetEnergyBar();
    }
    if (CheckOverheat())
    {
        return false;
    }
    switch (_GetClass()->m_eFireType)
    {
    case WeaponAreaEffectClass::FIRE_HOLD:
        return mTrigger->IsDown();
    case WeaponAreaEffectClass::FIRE_CHARGE:
        return mTrigger->IsRelease();
    default:
        return mTrigger->IsDown();
    }
}

// FUNCTION: SWBF2 0x0067c8b0
bool WeaponAreaEffect::FireDone() const
{
    if (!m_bFired)
    {
        return false;
    }
    if (_GetClass()->m_eFireType == WeaponAreaEffectClass::FIRE_HOLD && !CheckEmpty() && mTrigger->IsDown() &&
        m_iNumTargets)
    {
        return false;
    }
    return Weapon::FireDone();
}

// FUNCTION: SWBF2 0x0067c900
bool WeaponAreaEffect::CheckEmpty() const
{
    const float fEnergyDrain = _GetClass()->mEnergyDrain;
    const EnergyBar* pEnergy = mOwner->GetEnergyBar();
    if (pEnergy && pEnergy->IsOverheating() && fEnergyDrain > 0.0f)
    {
        return true;
    }
    if (_GetClass()->m_fEnergyDrainRate > 0.0f)
    {
        mOwner->GetEnergyBar();
    }
    return Weapon::CheckEmpty();
}

// FUNCTION: SWBF2 0x0067c960
bool WeaponAreaEffect::ReloadDone() const
{
    return Weapon::ReloadDone();
}

// FUNCTION: SWBF2 0x0067c970
int WeaponAreaEffect::SetTargets()
{
    const WeaponAreaEffectClass* pClass = _GetClass();
    PblVector3 vCenter;
    float fRadius;
    switch (pClass->m_eAreaType)
    {
    case WeaponAreaEffectClass::AREA_CYLINDER:
    {
        const float fRange = pClass->m_fAreaRange;
        vCenter.x = mAimer->mDirection.x * fRange + mAimer->mFirePos.x;
        vCenter.y = mAimer->mDirection.y * fRange + mAimer->mFirePos.y;
        vCenter.z = mAimer->mDirection.z * fRange + mAimer->mFirePos.z;
        fRadius = (float)sqrt(pClass->m_fAreaHeight * pClass->m_fAreaHeight * 0.25f +
                              pClass->m_fAreaRadius * pClass->m_fAreaRadius);
        break;
    }
    case WeaponAreaEffectClass::AREA_ARC:
    case WeaponAreaEffectClass::AREA_RAY:
    {
        const float fHalfAngle = pClass->m_fAreaRadius * 0.017453292f;
        float fDist;
        if (fHalfAngle >= 1.5707964f)
        {
            vCenter = mAimer->mFirePos;
            fDist = pClass->m_fAreaRange;
        }
        else
        {
            const float fCos = (float)cos(fHalfAngle);
            if (fHalfAngle >= 0.78539819f)
            {
                fDist = pClass->m_fAreaRange * fCos;
            }
            else
            {
                fDist = pClass->m_fAreaRange * 0.5f / fCos;
            }
            vCenter.x = mAimer->mDirection.x * fDist + mAimer->mFirePos.x;
            vCenter.y = mAimer->mDirection.y * fDist + mAimer->mFirePos.y;
            vCenter.z = mAimer->mDirection.z * fDist + mAimer->mFirePos.z;
        }
        fRadius = (float)sqrt(pClass->m_fAreaHeight * pClass->m_fAreaHeight * 0.25f + fDist * fDist);
        break;
    }
    default:
    {
        const float fRange = pClass->m_fAreaRange;
        vCenter.x = mAimer->mDirection.x * fRange + mAimer->mFirePos.x;
        vCenter.y = mAimer->mDirection.y * fRange + mAimer->mFirePos.y;
        vCenter.z = mAimer->mDirection.z * fRange + mAimer->mFirePos.z;
        fRadius = pClass->m_fAreaRadius;
        break;
    }
    }

    GameObject* apObjects[10];
    const int iCount = CollisionManager::GetObjectsInRange(vCenter, fRadius, apObjects, 10, mOwner->GetGameObject(), 0x3a);
    return UpdateTargets(apObjects, iCount);
}

// FUNCTION: SWBF2 0x0067cbd0
int WeaponAreaEffect::UpdateTargets()
{
    GameObject* apTargets[MAX_TARGETS];
    int iCount = 0;
    for (int i = 0; i < MAX_TARGETS; ++i)
    {
        if (GameObject* pTarget = m_apTarget[i])
        {
            apTargets[iCount++] = pTarget;
        }
    }
    return UpdateTargets(apTargets, iCount);
}

// FUNCTION: SWBF2 0x0067cc30
int WeaponAreaEffect::UpdateTargets(GameObject** ppTargets, int iNumTargets)
{
    static const float fInner = 0.5625f;
    static const float fScale = 1.0f / (1.0f - fInner);
    static const float fOffset = -(fInner / (1.0f - fInner));

    m_iNumTargets = 0;
    if (iNumTargets <= 0)
    {
        return 0;
    }

    const GameObject* apIgnore[2];
    apIgnore[0] = mOwner->GetGameObject();
    apIgnore[1] = 0;
    CollisionObject* pHit;

    const WeaponAreaEffectClass* pClass = _GetClass();
    switch (pClass->m_eAreaType)
    {
    case WeaponAreaEffectClass::AREA_SPHERE:
    {
        const float fRange = pClass->m_fAreaRange;
        PblVector3 vCenter;
        vCenter.x = mAimer->mFirePos.x + fRange * mAimer->mDirection.x;
        vCenter.y = mAimer->mFirePos.y + mAimer->mDirection.y * fRange;
        vCenter.z = mAimer->mFirePos.z + mAimer->mDirection.z * fRange;
        const float fInvRadiusSq = 1.0f / (pClass->m_fAreaRadius * pClass->m_fAreaRadius);
        for (int i = 0; i < iNumTargets; ++i)
        {
            const PblVector3& vTarget = ppTargets[i]->GetCollisionCenter();
            apIgnore[1] = ppTargets[i];
            if (CollisionManager::RayHit(mAimer->mFirePos, vTarget, pHit, 0, apIgnore, 2, 0x98, true) < 1.0f)
            {
                continue;
            }
            const float dx = vTarget.x - vCenter.x;
            const float dy = vTarget.y - vCenter.y;
            const float dz = vTarget.z - vCenter.z;
            AddTarget(ppTargets[i], (dy * dy + dx * dx + dz * dz) * fInvRadiusSq);
        }
        return m_iNumTargets;
    }

    case WeaponAreaEffectClass::AREA_CYLINDER:
    {
        const float fRange = pClass->m_fAreaRange;
        PblVector3 vCenter;
        vCenter.x = mAimer->mDirection.x * fRange + mAimer->mFirePos.x;
        vCenter.y = mAimer->mFirePos.y + mAimer->mDirection.y * fRange;
        vCenter.z = mAimer->mFirePos.z + mAimer->mDirection.z * fRange;
        const float fInvRadiusSq = 1.0f / (pClass->m_fAreaRadius * pClass->m_fAreaRadius);
        const float fInvHalfHeightSq = 4.0f / (pClass->m_fAreaHeight * pClass->m_fAreaHeight);
        for (int i = 0; i < iNumTargets; ++i)
        {
            GameObject* pTarget = ppTargets[i];
            const PblVector3& vTarget = pTarget->GetCollisionCenter();
            const float fTargetRadius = pTarget->GetCollisionRadius();
            const float fHeight = fabsf(vTarget.y - vCenter.y);
            if (fHeight > _GetClass()->m_fAreaHeight * 0.5f + fTargetRadius)
            {
                continue;
            }
            const float dx = vTarget.x - vCenter.x;
            const float dz = vTarget.z - vCenter.z;
            const float fDistSq = dx * dx + dz * dz;

            const float fReach = _GetClass()->m_fAreaRange + fTargetRadius;
            if (fDistSq > fReach * fReach)
            {
                continue;
            }
            apIgnore[1] = pTarget;
            if (CollisionManager::RayHit(mAimer->mFirePos, vTarget, pHit, 0, apIgnore, 2, 0x98, true) < 1.0f)
            {
                continue;
            }
            float fTargetRange = fDistSq * fInvRadiusSq;
            const float fOver = fHeight - fTargetRadius;
            const float fHeightRange = (float)(fOver > 0.0f) * fOver * fOver * fInvHalfHeightSq * fScale + fOffset;
            if (fHeightRange > fTargetRange)
            {
                fTargetRange = fHeightRange;
            }
            AddTarget(ppTargets[i], fTargetRange);
        }
        return m_iNumTargets;
    }

    case WeaponAreaEffectClass::AREA_ARC:
    {
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, pClass->m_fAreaRadius * 0.017453292f);
        const Aimer* pAimer = mAimer;
        const PblMatrix& mOwnerMatrix = mOwner->GetMatrix();
        const PblVector3& vDir = pAimer->mDirection;
        PblVector3 vNormal;
        vNormal.x = vDir.y * mOwnerMatrix.m[0][2] - vDir.z * mOwnerMatrix.m[0][1];
        vNormal.y = mOwnerMatrix.m[0][0] * vDir.z - vDir.x * mOwnerMatrix.m[0][2];
        vNormal.z = vDir.x * mOwnerMatrix.m[0][1] - mOwnerMatrix.m[0][0] * vDir.y;
        const float fInvRangeSq = 1.0f / (_GetClass()->m_fAreaRange * _GetClass()->m_fAreaRange);
        const float fInvHalfHeightSq = 4.0f / (_GetClass()->m_fAreaHeight * _GetClass()->m_fAreaHeight);
        for (int i = 0; i < iNumTargets; ++i)
        {
            GameObject* pTarget = ppTargets[i];
            const PblVector3& vTarget = pTarget->GetCollisionCenter();
            const float dx = vTarget.x - pAimer->mFirePos.x;
            const float dy = vTarget.y - pAimer->mFirePos.y;
            const float dz = vTarget.z - pAimer->mFirePos.z;
            const float fTargetRadius = pTarget->GetCollisionRadius();
            const float fHeight = fabsf(dx * vNormal.x + vNormal.y * dy + vNormal.z * dz);
            if (fHeight > _GetClass()->m_fAreaHeight * 0.5f + fTargetRadius)
            {
                continue;
            }
            const float fForward = vDir.x * dx + vDir.y * dy + vDir.z * dz;
            const float fSide = fabsf(mOwnerMatrix.m[0][1] * dy + mOwnerMatrix.m[0][0] * dx + mOwnerMatrix.m[0][2] * dz);
            if (fSide * fCos - fTargetRadius > fForward * fSin)
            {
                continue;
            }
            const float fDistSq = fSide * fSide + fForward * fForward;
            const float fReach = _GetClass()->m_fAreaRange + fTargetRadius;
            if (fDistSq > fReach * fReach)
            {
                continue;
            }
            apIgnore[1] = pTarget;
            if (CollisionManager::RayHit(pAimer->mFirePos, vTarget, pHit, 0, apIgnore, 2, 0x98, true) < 1.0f)
            {
                continue;
            }
            float fTargetRange = fDistSq * fInvRangeSq;
            if (fSide > 0.001f)
            {
                float fEdge = fSide / ((1.0f - fCos) * fSide + fForward * fSin);
                fEdge = fEdge * fEdge * fScale + fOffset;
                if (fEdge > fTargetRange)
                {
                    fTargetRange = fEdge;
                }
            }
            const float fOver = fHeight - fTargetRadius;
            const float fHeightRange = (float)(fOver > 0.0f) * fOver * fOver * fInvHalfHeightSq * fScale + fOffset;
            if (fHeightRange > fTargetRange)
            {
                fTargetRange = fHeightRange;
            }
            AddTarget(ppTargets[i], fTargetRange);
        }
        return m_iNumTargets;
    }

    case WeaponAreaEffectClass::AREA_RAY:
    {
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, pClass->m_fAreaRadius * 0.017453292f);
        const Aimer* pAimer = mAimer;
        const PblMatrix& mOwnerMatrix = mOwner->GetMatrix();
        const PblVector3& vDir = pAimer->mDirection;
        PblVector3 vNormal;
        vNormal.x = vDir.y * mOwnerMatrix.m[0][2] - vDir.z * mOwnerMatrix.m[0][1];
        vNormal.y = mOwnerMatrix.m[0][0] * vDir.z - vDir.x * mOwnerMatrix.m[0][2];
        vNormal.z = vDir.x * mOwnerMatrix.m[0][1] - mOwnerMatrix.m[0][0] * vDir.y;
        const float fInvRangeSq = 1.0f / (_GetClass()->m_fAreaRange * _GetClass()->m_fAreaRange);
        const float fInvHalfHeightSq = 4.0f / (_GetClass()->m_fAreaHeight * _GetClass()->m_fAreaHeight);
        for (int i = 0; i < iNumTargets; ++i)
        {
            GameObject* pTarget = ppTargets[i];
            const PblVector3& vTarget = pTarget->GetCollisionCenter();
            const float dx = vTarget.x - pAimer->mFirePos.x;
            const float dy = vTarget.y - pAimer->mFirePos.y;
            const float dz = vTarget.z - pAimer->mFirePos.z;
            const float fTargetRadius = pTarget->GetCollisionRadius();
            const float fHeight = fabsf(dx * vNormal.x + vNormal.y * dy + vNormal.z * dz);
            if (fHeight > _GetClass()->m_fAreaHeight * 0.5f + fTargetRadius)
            {
                continue;
            }
            const float fForward = vDir.x * dx + vDir.y * dy + vDir.z * dz;
            const float fSide = fabsf(dx * mOwnerMatrix.m[0][0] + dy * mOwnerMatrix.m[0][1] + mOwnerMatrix.m[0][2] * dz);
            if (fSide * fCos - fTargetRadius > fForward * fSin)
            {
                continue;
            }
            const float fDistSq = fSide * fSide + fForward * fForward;
            const float fReach = _GetClass()->m_fAreaRange + fTargetRadius;
            if (fDistSq > fReach * fReach)
            {
                continue;
            }
            apIgnore[1] = pTarget;
            if (CollisionManager::RayHit(pAimer->mFirePos, vTarget, pHit, 0, apIgnore, 2, 0x98, true) < 1.0f)
            {
                continue;
            }
            float fTargetRange = fDistSq * fInvRangeSq * fScale + fOffset;
            float fEdge = 0.0f;
            if (fSide > 0.001f)
            {
                fEdge = fSide / ((1.0f - fCos) * fSide + fForward * fSin);
            }
            fEdge *= fEdge;
            if (fEdge > fTargetRange)
            {
                fTargetRange = fEdge;
            }
            const float fOver = fHeight - fTargetRadius;
            const float fHeightRange = (float)(fOver > 0.0f) * fOver * fOver * fInvHalfHeightSq * fScale + fOffset;
            if (fHeightRange > fTargetRange)
            {
                fTargetRange = fHeightRange;
            }
            AddTarget(ppTargets[i], fTargetRange);
        }
        return m_iNumTargets;
    }
    }
    return m_iNumTargets;
}

// FUNCTION: SWBF2 0x0067d6b0
bool WeaponAreaEffect::AddTarget(GameObject* pTarget, float fRange)
{
    if (mOwner->GetTargetLockedObj() == pTarget)
    {
        fRange *= 0.0001f;
    }
    const int iMax = _GetClass()->m_iMaxTargets;
    int iSlot;
    for (iSlot = 0; iSlot < m_iNumTargets; ++iSlot)
    {
        if (m_afTargetRange[iSlot] > fRange)
        {
            break;
        }
    }
    if (iSlot >= iMax)
    {
        return false;
    }
    int iLast = m_iNumTargets;
    if (iLast >= iMax)
    {
        --iLast;
    }
    for (int i = iLast; i > iSlot; --i)
    {
        m_apTarget[i] = m_apTarget[i - 1];
        m_afTargetRange[i] = m_afTargetRange[i - 1];
    }
    m_apTarget[iSlot] = pTarget;
    m_afTargetRange[iSlot] = fRange;
    if (m_iNumTargets < iMax)
    {
        ++m_iNumTargets;
    }
    return true;
}

// FUNCTION: SWBF2 0x0067d7c0
void WeaponAreaEffect::ApplyEffectsToTargets(float fDeltaTime)
{
    float fStrength = m_fStrength;
    if (_GetClass()->m_eFireType == WeaponAreaEffectClass::FIRE_HOLD)
    {
        fStrength *= fDeltaTime;
    }
    for (int i = 0; i < MAX_TARGETS; ++i)
    {
        GameObject* pTarget = m_apTarget[i];
        if (!pTarget)
        {
            continue;
        }
        float fEffect = fStrength;
        const float fFalloff = _GetClass()->m_fEffectFalloff;
        if (1.0f > fFalloff)
        {
            const float fRange = m_afTargetRange[i];
            if (1.0f > fRange)
            {
                const float fNear = (1.0f - fRange) * (1.0f - fRange);
                fEffect = ((1.0f - fNear) * fFalloff + fNear) * fStrength;
            }
            else
            {
                fEffect = fFalloff * fStrength;
            }
        }
        const PblMatrix& mOwnerMatrix = mOwner->GetMatrix();
        const PblMatrix& mTargetMatrix = pTarget->GetMatrix();
        PblVector3 vDelta;
        vDelta.x = mTargetMatrix.m[3][0] - mOwnerMatrix.m[3][0];
        vDelta.y = mTargetMatrix.m[3][1] - mOwnerMatrix.m[3][1];
        vDelta.z = mTargetMatrix.m[3][2] - mOwnerMatrix.m[3][2];
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vDelta);
        ApplyEffectsToTarget(m_apTarget[i], vDir, fEffect);
    }
}

// FUNCTION: SWBF2 0x0067d930
void WeaponAreaEffect::ApplyEffectsToTarget(GameObject* pTarget, const PblVector3& vDir, float fStrength)
{
    const WeaponAreaEffectClass* pClass = _GetClass();
    if (pClass->m_fDamage > 0.0f)
    {
        const float fDamage = pClass->m_fDamage * fStrength;
        GameObject* pOwnerObject = mOwner->GetGameObject();
        const int iTeam = mOwner->GetGameObject()->GetTeam();
        DamageDesc damage(DamageOwner(mOwner->mCharacter, mOwner->GetGameObject(), iTeam,
                                      GetEntityClass(pOwnerObject), mClass, 1.0f),
                          fDamage);
        pTarget->ApplyDamage(damage, vDir, pTarget->GetCollisionCenter(), -1, (unsigned int)-1);
    }

    pClass = _GetClass();
    if (pClass->m_fEffectStrength > 0.0f)
    {
        const float fEffect = pClass->m_fEffectStrength * fStrength;
        switch (pClass->m_eEffectType)
        {
        case WeaponAreaEffectClass::EFFECT_PUSH:
            pTarget->ApplyPush(vDir, fEffect, false);
            break;
        case WeaponAreaEffectClass::EFFECT_PULL:
        {
            PblVector3 vPull;
            vPull.x = -vDir.x;
            vPull.y = -vDir.y;
            vPull.z = -vDir.z;
            pTarget->ApplyPush(vPull, fEffect, false);
            break;
        }
        case WeaponAreaEffectClass::EFFECT_CHOKE:
            if (pTarget->GetEntityType() == ENT_SOLDIER)
            {
                static_cast<EntitySoldier*>(pTarget->GetEntity())
                    ->ApplyChoke(vDir, fEffect, (SoldierAnimationBank::ANIMATION)pClass->m_eSoldierAnimation);
            }
            break;
        }
    }
}

// FUNCTION: SWBF2 0x0067db20
void WeaponAreaEffect::Fire()
{
    if (!NetGame::IsEnabled() || (NetGame::IsClient() && netIsLocalTurn))
    {
        mClass->CreateMuzzleFlash(mFirePointMatrix);
    }
}

// GLOBAL: SWBF2 0x01f9a888
// SYNTHETIC: SWBF2 0x00407070
// `dynamic initializer for 'WeaponAreaEffectClass::rttiHashWeaponAreaEffectClass''
PblHash WeaponAreaEffectClass::rttiHashWeaponAreaEffectClass("WeaponAreaEffectClass");
char WeaponAreaEffectClass::rttiNameWeaponAreaEffectClass[] = "WeaponAreaEffectClass";

// FUNCTION: SWBF2 0x0067db70
WeaponAreaEffectClass::WeaponAreaEffectClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    m_eFireType = FIRE_REPEAT;
    m_eEffectType = EFFECT_PUSH;
    m_eAreaType = AREA_SPHERE;
    m_fInitialSalvoDelay = 0.0f;
    m_fAreaRange = 3.0f;
    m_fAreaRadius = 3.0f;
    m_fAreaHeight = 2.0f;
    m_iMaxTargets = 4;
    m_fDamage = 0.0f;
    m_fEffectStrength = 0.0f;
    m_fEffectFalloff = 1.0f;
    m_fEnergyDrainRate = 0.0f;
    m_fChargeMaxTime = 0.0f;
    m_fChargeMinStrength = 0.0f;
    m_fThrustFactor = 1.0f;
    m_bTargetLock = false;
    m_eSoldierAnimation = -1;
}

// FUNCTION: SWBF2 0x0067dc70
WeaponAreaEffectClass::WeaponAreaEffectClass(WeaponAreaEffectClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    m_eFireType = pBase->m_eFireType;
    m_eEffectType = pBase->m_eEffectType;
    m_eAreaType = pBase->m_eAreaType;
    m_fInitialSalvoDelay = pBase->m_fInitialSalvoDelay;
    m_fAreaRange = pBase->m_fAreaRange;
    m_fAreaRadius = pBase->m_fAreaRadius;
    m_fAreaHeight = pBase->m_fAreaHeight;
    m_iMaxTargets = pBase->m_iMaxTargets;
    m_fDamage = pBase->m_fDamage;
    m_fEffectStrength = pBase->m_fEffectStrength;
    m_fEffectFalloff = pBase->m_fEffectFalloff;
    m_fEnergyDrainRate = pBase->m_fEnergyDrainRate;
    m_fChargeMaxTime = pBase->m_fChargeMaxTime;
    m_fChargeMinStrength = pBase->m_fChargeMinStrength;
    m_fThrustFactor = pBase->m_fThrustFactor;
    m_bTargetLock = pBase->m_bTargetLock;
    m_eSoldierAnimation = pBase->m_eSoldierAnimation;
}

// FUNCTION: SWBF2 0x0067dd60
WeaponAreaEffectClass::~WeaponAreaEffectClass()
{
}

// SYNTHETIC: SWBF2 0x0067dc40
// WeaponAreaEffectClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0067c210
bool WeaponAreaEffectClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponAreaEffectClass || WeaponClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0067c240
unsigned int WeaponAreaEffectClass::GetDerivedRtti()
{
    return rttiHashWeaponAreaEffectClass;
}

// FUNCTION: SWBF2 0x0067c250
const char* WeaponAreaEffectClass::GetDerivedRttiName()
{
    return rttiNameWeaponAreaEffectClass;
}

// FUNCTION: SWBF2 0x0067dd70
void WeaponAreaEffectClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x32f380d1:
        switch (PblHash(pszValue))
        {
        case 0xadc649b8:
            m_eFireType = FIRE_HOLD;
            break;
        case 0xc0ca2371:
            m_eFireType = FIRE_CHARGE;
            break;
        default:
            m_eFireType = FIRE_REPEAT;
            break;
        }
        break;
    case 0x5c1fc8bc:
        switch (PblHash(pszValue))
        {
        case 0x812d107e:
            m_eEffectType = EFFECT_PULL;
            break;
        case 0x533a03d7:
            m_eEffectType = EFFECT_CHOKE;
            break;
        case 0x3f40cd38:
            m_eEffectType = EFFECT_MINDTRICK;
            break;
        case 0xdd6cdf21:
            m_eEffectType = EFFECT_STUN;
            break;
        case 0x876fffdd:
        default:
            m_eEffectType = EFFECT_PUSH;
            break;
        }
        break;
    case 0xda78204c:
        switch (PblHash(pszValue))
        {
        case 0xad1ebfef:
            m_eAreaType = AREA_CYLINDER;
            break;
        case 0x3a6f2adb:
            m_eAreaType = AREA_ARC;
            break;
        case 0x3dff097f:
            m_eAreaType = AREA_RAY;
            break;
        default:
            m_eAreaType = AREA_SPHERE;
            break;
        }
        break;
    case 0xc8388d9d:
        sscanf(pszValue, "%f", &m_fInitialSalvoDelay);
        break;
    case 0x2efc37e9:
        sscanf(pszValue, "%f", &m_fAreaRange);
        break;
    case 0x3891df5a:
        sscanf(pszValue, "%f", &m_fAreaRadius);
        break;
    case 0x894a0e77:
        sscanf(pszValue, "%f", &m_fAreaHeight);
        break;
    case 0xa00a48cd:
        sscanf(pszValue, "%d", &m_iMaxTargets);
        if (m_iMaxTargets > 4)
        {
            m_iMaxTargets = 4;
        }
        break;
    case 0xd2ea8a45:
        sscanf(pszValue, "%f", &m_fDamage);
        break;
    case 0x53a1abbd:
        sscanf(pszValue, "%f", &m_fEffectStrength);
        break;
    case 0x3b5eb7fa:
        sscanf(pszValue, "%f", &m_fEffectFalloff);
        break;
    case 0x8909415b:
        sscanf(pszValue, "%f", &m_fEnergyDrainRate);
        break;
    case 0x2668aef4:
        sscanf(pszValue, "%f", &m_fChargeMaxTime);
        break;
    case 0xd193ab6e:
        sscanf(pszValue, "%f", &m_fChargeMinStrength);
        break;
    case 0x0ec601dc:
        sscanf(pszValue, "%f", &m_fThrustFactor);
        break;
    case 0xe2e7efdf:
        m_bTargetLock = atoi(pszValue) != 0;
        break;
    case 0x5f0ce10d:
        m_eSoldierAnimation = SoldierAnimationBank::AddAnimation(pszValue);
        break;
    default:
        WeaponClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x0067e040
WeaponClass* WeaponAreaEffectClass::Derive(unsigned int uHash)
{
    return new WeaponAreaEffectClass(this, uHash);
}

// FUNCTION: SWBF2 0x0067e0b0
Weapon* WeaponAreaEffectClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    void* pMemory = Weapon::sMemoryPool.Allocate(0x150);
    return pMemory ? reinterpret_cast<Weapon*>(::new (pMemory) WeaponAreaEffect(this, desc)) : 0;
}
