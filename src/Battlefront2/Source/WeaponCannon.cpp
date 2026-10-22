#include "WeaponCannon.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AILowLevel.h"
#include "AIUtil.h"
#include "Aimer.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Ordnance.h"
#include "Trigger.h"
#include "UnitController.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblRandom.h"
#include "SoundFL/source/sndvoicevirtualbus.h"

extern int gDebugCount;
extern bool netIsLocalTurn;

extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV,
                                                         const PblMatrix* pM);

namespace CombatUtil { void SendDamageEvent(const Controllable* pOwner, const GameObject* pTarget, const Trigger* pTrigger); }
namespace GameLoop { extern float sClientDeltaTime; }

class AmmoCounter
{
public:
    bool CanExpendRounds(float fRounds) const;
    void ExpendRounds(float fRounds);
};

namespace
{

    const PblVector3& GetVelocity(GameObject* pObject)
    {
        return pObject->GetVelocity();
    }

    const PblMatrix& GetCollisionMatrix(const GameObject* pObject)
    {
        return static_cast<const CollisionObject*>(pObject)->GetMatrix();
    }

    bool IsAlive(const GameObject* pObject)
    {
        return pObject->IsAlive();
    }

    float GetCurHealth(const GameObject* pObject)
    {
        return pObject->mCurHealth;
    }

    const PblVector3& GetTranslation(const PblMatrix& m)
    {
        return *reinterpret_cast<const PblVector3*>(m.m[3]);
    }

    int GetTeam(const GameObject* pObject)
    {
        return pObject->GetTeam();
    }

    EntityClass* GetEntityClass(GameObject* pObject)
    {
        return pObject->GetEntityClass();
    }

    const GameObject* GetHandleObject(const PblHandle<const GameObject>& handle)
    {
        PblHandle<const GameObject>& h = const_cast<PblHandle<const GameObject>&>(handle);
        if (h.mObject &&
            h.mObject->mHandleId != h.mSavedHandleId)
        {
            h.mObject = 0;
            h.mSavedHandleId = 0;
        }
        return h.mObject;
    }

    float Clamp(float f, float fMin, float fMax)
    {
        if (fMin > f)
        {
            return fMin;
        }
        if (f > fMax)
        {
            return fMax;
        }
        return f;
    }
}

// FUNCTION: SWBF2 0x00680050
WeaponCannonClass::WeaponCannonClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    mTriggerSingle = false;

    mMaxPressedTime = 0.0f;
    mShotDelayMin = 0.0f;
    mShotDelayMax = 0.0f;
    mPitchSpread = 0.0f;
    mYawSpread = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            mStanceSpread[i][j] = 0.0f;
        }
    }

    mSpreadPerShot = 0.0f;
    mSpreadRecoverRate = 0.0f;
    mSpreadThreshold = 0.0f;
    mSpreadLimit = 0.0f;
    mKickStrength = 0.0f;
    mKickBuildup = 2.0f;
    mSalvoCount = 1;
    mShotsPerSalvo = 1;
    mSalvoDelayMin = 0.0f;
    mSalvoDelayMax = 0.0f;
    mInitialSalvoDelay = 0.0f;
    mPatternArray = 0;
    mPatternCount = 0;
    mWarmUpTime = 0.0f;
    mCoolDownTime = 0.0f;
    mWarmUpSoundStartPitch = 1.0f;
    mWarmUpSoundFadeInTime = 0.0f;
    mWarmUpSoundFadeOutTime = 0.0f;
    mBarrelSoundFadeInTime = 0.0f;
    mBarrelSoundFadeOutTime = 0.0f;
    mCoolDownSoundEndPitch = 1.0f;
    mCoolDownSoundFadeInTime = 0.0f;
    mCoolDownSoundFadeOutTime = 0.0f;
    mSecondaryOrdnanceClass = 0;
    mSecondaryOrdnancePeriod = 0;
}

// GLOBAL: SWBF2 0x01f9a894
// SYNTHETIC: SWBF2 0x004070a0
// `dynamic initializer for 'WeaponCannon::rttiHashWeaponCannon''
PblHash WeaponCannon::rttiHashWeaponCannon("WeaponCannon");
char WeaponCannon::rttiNameWeaponCannon[] = "WeaponCannon";

// GLOBAL: SWBF2 0x01f9a898
// SYNTHETIC: SWBF2 0x004070b0
// `dynamic initializer for 'WeaponCannonClass::rttiHashWeaponCannonClass''
PblHash WeaponCannonClass::rttiHashWeaponCannonClass("WeaponCannonClass");
char WeaponCannonClass::rttiNameWeaponCannonClass[] = "WeaponCannonClass";

// FUNCTION: SWBF2 0x0067e3c0
bool WeaponCannon::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponCannon || Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0067e3f0
unsigned int WeaponCannon::GetDerivedRtti() const
{
    return rttiHashWeaponCannon;
}

// FUNCTION: SWBF2 0x0067e400
const char* WeaponCannon::GetDerivedRttiName() const
{
    return rttiNameWeaponCannon;
}

// FUNCTION: SWBF2 0x0067e440
bool WeaponCannonClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponCannonClass || WeaponClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0067e470
unsigned int WeaponCannonClass::GetDerivedRtti()
{
    return rttiHashWeaponCannonClass;
}

// FUNCTION: SWBF2 0x0067e480
const char* WeaponCannonClass::GetDerivedRttiName()
{
    return rttiNameWeaponCannonClass;
}

// FUNCTION: SWBF2 0x0067e490
WeaponCannon::WeaponCannon(const WeaponCannonClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    mStrength = 0.0f;
    mSalvoCount = 0;
    mSalvoDelay = 0.0f;
    mPatternIndex = 0;
    if (mOwner->mPlayerId >= 0)
    {
        mKickSpread = -pClass->mSpreadThreshold;
    }
    else
    {
        mKickSpread = 0.0f;
    }
    mWarmUp = false;
    mKickSpreadFactor = 1.0f;
    mYawSpreadFactor = 1.0f;
    mStanceSpread = 0.0f;
    mWarmCoolTimer = 0.0f;
    mOrdCounter = 0;
}

// FUNCTION: SWBF2 0x0067e590
WeaponCannon::~WeaponCannon()
{
    StopWarmUpSound();
    StopBarrelSound();
    StopCoolDownSound();
}

// SYNTHETIC: SWBF2 0x0067e410
// WeaponCannon::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0067e620
bool WeaponCannon::Update(float fDeltaTime)
{
    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);

    if (mOwner->mPlayerId >= 0)
    {
        ++gDebugCount;
    }

    GameObject* pObject = mOwner->GetGameObject();
    const PblVector3* pPos;
    if (mAimer)
    {
        pPos = &mAimer->mFirePos;
    }
    else
    {
        pPos = &GetTranslation(GetCollisionMatrix(pObject));
    }
    const PblVector3* pVel = &GetVelocity(pObject);

    if (mSoundControlWarmUp.mFlags & GameSoundControllable::flags_playing)
    {
        mSoundControlWarmUp.Update(&mSoundPropsWarmUp, pPos, pVel);
    }
    if (mSoundControlBarrel.mFlags & GameSoundControllable::flags_playing)
    {
        mSoundControlBarrel.Update(&mSoundPropsBarrel, pPos, pVel);
    }
    if (mSoundControlCoolDown.mFlags & GameSoundControllable::flags_playing)
    {
        mSoundControlCoolDown.Update(&mSoundPropsCoolDown, pPos, pVel);
    }

    if (!(mWarmCoolTimer > 0.0f))
    {
        mKickSpread -= pClass->mSpreadRecoverRate * fDeltaTime;
        if (mOwner->mPlayerId >= 0)
        {
            float fMin = -pClass->mSpreadThreshold;
            if (fMin > mKickSpread)
            {
                mKickSpread = fMin;
            }
        }
        else if (mKickSpread < 0.0f)
        {
            mKickSpread = 0.0f;
        }
    }

    GameObject* pOwnerObject = mOwner->GetGameObject();
    if (!pOwnerObject || !IsAlive(pOwnerObject) || !(GetCurHealth(pOwnerObject) > 0.0f))
    {
        StopWarmUpSound();
        StopBarrelSound();
        StopCoolDownSound();
    }
    return Weapon::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x0067e7a0
bool WeaponCannon::IsBusy() const
{
    if (mState != FIRE && mState != FIRE2)
    {
        return false;
    }
    return mSalvoCount > 0 || mStateLimit > mStateTimer + GameLoop::sClientDeltaTime;
}

// FUNCTION: SWBF2 0x0067e7e0
void WeaponCannon::NotifySoldierState(int iState, float fTime, bool b0, bool b1,
                                      const PblVector3& vec)
{
    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);

    float fStrafe = fabsf(mOwner->mControlStrafe);
    float fMove = fabsf(mOwner->mControlMove);
    float fBlend = fMove > fStrafe ? fMove : fStrafe;

    int iStance = 0;
    if (iState < 3)
    {
        iStance = iState;
    }
    int iColumn = b1 ? 2 : 0;
    const float fRest = pClass->mStanceSpread[iStance][iColumn];
    mStanceSpread = (pClass->mStanceSpread[iStance][1] - fRest) * fBlend + fRest;

    Weapon::NotifySoldierState(iState, fTime, b0, b1, vec);
}

// FUNCTION: SWBF2 0x0067e880
float WeaponCannon::GetYawSpread() const
{
    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);

    float fYaw = pClass->mYawSpread;
    if (fYaw == 0.0f && mOwner->mPlayerId < 0 && pClass->mTriggerSingle && pClass->mZoomMax >= 6.0f &&
        UnitControllerPtr(mOwner->mCtrl) &&
        reinterpret_cast<AILowLevel*>(UnitControllerPtr(mOwner->mCtrl)->mLowLevel)->GetTarget())
    {
        const GameObject* pTarget =
            reinterpret_cast<AILowLevel*>(UnitControllerPtr(mOwner->mCtrl)->mLowLevel)->GetTarget();
        const PblMatrix& mOwnerMatrix = mOwner->GetMatrix();
        const PblMatrix& mTargetMatrix = GetCollisionMatrix(pTarget);
        float fX = mTargetMatrix.m[3][0] - mOwnerMatrix.m[3][0];
        float fY = mTargetMatrix.m[3][1] - mOwnerMatrix.m[3][1];
        float fZ = mTargetMatrix.m[3][2] - mOwnerMatrix.m[3][2];
        float fDistSq = fX * fX + fY * fY + fZ * fZ;

        float fSpread;
        if (fDistSq < 400.0f)
        {
            fSpread = 0.0997f;
        }
        else if (fDistSq < 1600.0f)
        {
            fSpread = 0.0499f;
        }
        else if (fDistSq < 3600.0f)
        {
            fSpread = 0.0333f;
        }
        else if (fDistSq < 6400.0f)
        {
            fSpread = 0.025f;
        }
        else if (fDistSq < 10000.0f)
        {
            fSpread = 0.02f;
        }
        else if (fDistSq < 14400.0f)
        {
            fSpread = 0.0167f;
        }
        else
        {
            fSpread = 0.0143f;
        }
        fYaw = mYawSpreadFactor * fSpread;
    }

    float fKick = mKickSpread > 0.0f ? mKickSpread : 0.0f;
    return mStanceSpread + fKick + fYaw;
}

// FUNCTION: SWBF2 0x0067ea60
float WeaponCannon::GetPitchSpread() const
{
    float fKick = mKickSpread > 0.0f ? mKickSpread : 0.0f;
    return mStanceSpread + fKick + static_cast<const WeaponCannonClass*>(mClass)->mPitchSpread;
}

// FUNCTION: SWBF2 0x0067eaa0
void WeaponCannon::SetKickSpreadFactor(float fFactor)
{
    mKickSpreadFactor = fFactor;
}

// FUNCTION: SWBF2 0x0067eac0
void WeaponCannon::SetYawSpreadFactor(float fFactor)
{
    mYawSpreadFactor = fFactor;
}

// FUNCTION: SWBF2 0x0067eae0
void WeaponCannon::EnterFire()
{
    if (netEnabledNext && netClientActive)
    {
        ++gDebugCount;
    }

    mWarmUp = true;
    mWarmCoolTimer = static_cast<const WeaponCannonClass*>(mClass)->mWarmUpTime;
    if (mWarmCoolTimer > 0.0f)
    {
        PlayWarmUpSound();
    }
    else
    {
        Weapon::EnterFire();
    }

    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);
    if (pClass->mMaxPressedTime > 0.0f)
    {
        mStrength = mStateTimer / pClass->mMaxPressedTime;
    }
    else
    {
        mStrength = 1.0f;
    }
    mStateTimer = 0.0f;
    mStateLimit = AIUtil::GetRandomFloat(pClass->mShotDelayMin, pClass->mShotDelayMax);
    mSalvoCount = pClass->mSalvoCount;
    mSalvoDelay = pClass->mInitialSalvoDelay;
    if (pClass->mInstantPlayFireAnim)
    {
        mFiredFlag = 1;
    }
    if (pClass->mTriggerSingle)
    {
        mTrigger->ClearUntilPressed();
    }
}

// FUNCTION: SWBF2 0x0067ebd0
bool WeaponCannon::UpdateFire(float fDeltaTime)
{
    float fRemaining = fDeltaTime;
    if (!mSelectedFlag)
    {
        mSalvoCount = 0;
    }

    while (fRemaining > 0.000030517578f)
    {
        UpdateWarmUpSound(fRemaining);
        UpdateBarrelSound(fRemaining);
        UpdateCoolDownSound(fRemaining);

        const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);
        if (!mWarmUp && pClass->mCoolDownTime > 0.0f)
        {
            PlayCoolDownSound();
        }

        if (mWarmCoolTimer > 0.0f)
        {
            mWarmCoolTimer -= fRemaining;
            if (mWarmCoolTimer < 0.0f)
            {
                mWarmCoolTimer = 0.0f;
            }

            pClass = static_cast<const WeaponCannonClass*>(mClass);
            float fTimer;
            if (mWarmUp)
            {
                fTimer = (pClass->mWarmUpTime - mWarmCoolTimer) * mStateLimit / pClass->mWarmUpTime;
            }
            else
            {
                mSalvoCount = 0;
                fTimer = (pClass->mCoolDownTime - mWarmCoolTimer) * mStateLimit / pClass->mCoolDownTime;
            }
            if (fTimer < 0.0f)
            {
                fTimer = 0.0f;
            }
            else if (fTimer > mStateLimit)
            {
                fTimer = mStateLimit;
            }
            mStateTimer = fTimer;
            break;
        }

        if (mWarmUp && static_cast<const WeaponCannonClass*>(mClass)->mWarmUpTime > 0.0f)
        {
            PlayBarrelSound();
        }

        if (mSalvoCount > 0)
        {
            while (!(mSalvoDelay > 0.0f))
            {
                pClass = static_cast<const WeaponCannonClass*>(mClass);
                bool bFirstShot = mSalvoCount == pClass->mSalvoCount;
                for (int iShots = pClass->mShotsPerSalvo; iShots > 0; --iShots)
                {
                    const WeaponCannonClass* pShotClass = static_cast<const WeaponCannonClass*>(mClass);
                    if (!m_pAmmoCounter->CanExpendRounds(pShotClass->mRoundsPerShot) ||
                        (m_pEnergyBar->IsOverheating() && pShotClass->mHeatPerShot > 0.0f))
                    {
                        mSalvoCount = 0;
                        break;
                    }

                    bool bFired = false;
                    if (mAimer->WeaponShouldFire() && Fire())
                    {
                        bFired = true;
                        if (netEnabledNext && netClientActive)
                        {
                            ++gDebugCount;
                        }
                        if (!bFirstShot && static_cast<const WeaponCannonClass*>(mClass)->mSalvoDelayMin > 0.0f)
                        {
                            PlayFireSound(false);
                        }
                        bFirstShot = false;
                        m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot);
                        m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot);
                    }
                    mAimer->UpdateFire(mAimer, bFired);
                }

                pClass = static_cast<const WeaponCannonClass*>(mClass);
                mKickSpread += pClass->mSpreadPerShot * mKickSpreadFactor;
                if (mKickSpread > pClass->mSpreadLimit)
                {
                    mKickSpread = pClass->mSpreadLimit;
                }
                --mSalvoCount;
                mSalvoDelay += AIUtil::GetRandomFloat(pClass->mSalvoDelayMin, pClass->mSalvoDelayMax);
                if (mSalvoCount <= 0)
                {
                    break;
                }
            }

            float fStep = mSalvoDelay > fRemaining ? fRemaining : mSalvoDelay;
            fRemaining -= fStep;
            mSalvoDelay -= fStep;
        }
        else
        {
            float fStep = mStateLimit - mStateTimer;
            if (fStep > fRemaining)
            {
                fStep = fRemaining;
            }
            fRemaining -= fStep;
            mStateTimer += fStep;
            if (mStateTimer < mStateLimit || static_cast<const WeaponCannonClass*>(mClass)->mTriggerSingle ||
                !CheckFire() || CheckReload() || CheckEmpty())
            {
                break;
            }

            mStateTimer -= mStateLimit;
            Weapon::ExitFire();
            Weapon::EnterFire();
            pClass = static_cast<const WeaponCannonClass*>(mClass);
            mSalvoCount = pClass->mSalvoCount;
            mSalvoDelay = pClass->mInitialSalvoDelay;
        }
    }
    return Weapon::UpdateFire(fDeltaTime);
}

// FUNCTION: SWBF2 0x0067f020
void WeaponCannon::ExitFire()
{
    StopWarmUpSound();
    StopBarrelSound();
    StopCoolDownSound();
    mWarmUp = false;
    Weapon::ExitFire();
}

// FUNCTION: SWBF2 0x0067f050
void WeaponCannon::EnterCharge()
{
    mStateTimer = 0.0f;
    mStateLimit = static_cast<const WeaponCannonClass*>(mClass)->mMaxPressedTime;
    Weapon::EnterCharge();
}

// FUNCTION: SWBF2 0x0067f070
bool WeaponCannon::UpdateCharge(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    if (mStateTimer >= mStateLimit)
    {
        mStateTimer = mStateLimit;
    }
    Weapon::UpdateCharge(fDeltaTime);
    return true;
}

// FUNCTION: SWBF2 0x0067f0c0
void WeaponCannon::ExitCharge()
{
    Weapon::ExitCharge();
}

// FUNCTION: SWBF2 0x0067f0d0
void WeaponCannon::EnterEmpty()
{
    Weapon::EnterEmpty();
    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);
    mStateLimit = AIUtil::GetRandomFloat(pClass->mShotDelayMin, pClass->mShotDelayMax);
}

// FUNCTION: SWBF2 0x0067f100
bool WeaponCannon::CheckCharge() const
{
    if (!mTrigger || !(static_cast<const WeaponCannonClass*>(mClass)->mMaxPressedTime > 0.0f))
    {
        return false;
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x0067f120
bool WeaponCannon::CheckFire() const
{
    if (!mTrigger)
    {
        return false;
    }

    float fEnergyDrain = mClass->mEnergyDrain;
    EnergyBar* pBar = mOwner->GetEnergyBar();
    if (pBar && pBar->IsOverheating() && fEnergyDrain > 0.0f)
    {
        return false;
    }
    if (CheckOverheat())
    {
        return false;
    }

    if (static_cast<const WeaponCannonClass*>(mClass)->mMaxPressedTime > 0.0f)
    {
        return mTrigger->IsRelease();
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x0067f1a0
bool WeaponCannon::FireDone() const
{
    WeaponCannon* pThis = const_cast<WeaponCannon*>(this);
    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);

    if (mWarmCoolTimer > 0.0f)
    {

        if (!CheckFire() && mWarmUp)
        {
            pThis->mWarmUp = false;
            pThis->mWarmCoolTimer =
                (pClass->mWarmUpTime - mWarmCoolTimer) * pClass->mCoolDownTime / pClass->mWarmUpTime;
        }
        return false;
    }

    if (!mWarmUp)
    {
        return true;
    }
    if (mSalvoCount > 0 || mStateTimer < mStateLimit)
    {
        return false;
    }
    if (!pClass->mTriggerSingle && CheckFire() && !CheckReload() && !CheckEmpty())
    {
        return false;
    }

    if (mWarmUp)
    {
        pThis->mWarmUp = false;
        pThis->mWarmCoolTimer = pClass->mCoolDownTime;
        return !(pClass->mCoolDownTime > 0.0f);
    }
    return true;
}

// FUNCTION: SWBF2 0x0067f2a0
bool WeaponCannon::CheckEmpty() const
{
    float fEnergyDrain = mClass->mEnergyDrain;
    EnergyBar* pBar = mOwner->GetEnergyBar();
    if (pBar && pBar->IsOverheating() && fEnergyDrain > 0.0f)
    {
        return true;
    }
    return Weapon::CheckEmpty();
}

// FUNCTION: SWBF2 0x0067f2f0
bool WeaponCannon::ReloadDone() const
{
    return Weapon::ReloadDone();
}

// FUNCTION: SWBF2 0x0067f300
void WeaponCannon::SetOrdnance(Ordnance* pOrdnance)
{
}

// FUNCTION: SWBF2 0x0067f310
Ordnance* WeaponCannon::CreateOrdnance(OrdnanceDesc& desc)
{
    return mClass->mOrdnanceClass->Build(desc);
}

// FUNCTION: SWBF2 0x0067f320
bool WeaponCannon::Fire()
{
    if (!mClass->mOrdnanceClass)
    {
        return false;
    }

    PblMatrix mFire = mAimer->GetMatrix();

    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);
    float fPitch = 0.0f;
    float fYaw = 0.0f;
    if (pClass->mPatternCount > 0)
    {
        fPitch = pClass->mPatternArray[mPatternIndex].mPitch;
        fYaw = pClass->mPatternArray[mPatternIndex].mYaw;
        if (++mPatternIndex >= pClass->mPatternCount)
        {
            mPatternIndex = 0;
        }
    }
    fPitch += (PblRandom::GetSim().GetFloat() - PblRandom::GetSim().GetFloat()) * GetPitchSpread();
    fYaw += (PblRandom::GetSim().GetFloat() - PblRandom::GetSim().GetFloat()) * GetYawSpread();

    float fCosPitch = (float)cos(fPitch);
    float fSinPitch = (float)sin(fPitch);
    float fCosYaw = (float)cos(fYaw);
    float fSinYaw = (float)sin(fYaw);

    float fEnergyDrain = mClass->mEnergyDrain;
    if (EnergyBar* pBar = mOwner->GetEnergyBar())
    {
        pBar->SpendEnergy(fEnergyDrain);
    }

    OrdnanceDesc desc;
    desc.mPosition = mAimer->mFirePos;
    PblVector3 vDir(fSinYaw * fCosPitch, -fSinPitch, fCosYaw * fCosPitch);
    PblVector3 vWorldDir;
    D3DXVec3TransformNormal(&vWorldDir, &vDir, &mFire);
    desc.mDirection = vWorldDir;
    desc.mStrength = mStrength;
    desc.mOwner = mOwner->GetGameObject();
    desc.mDamageOwner = DamageOwner(mOwner->mCharacter, mOwner->GetGameObject(), GetTeam(mOwner->GetGameObject()),
                                    GetEntityClass(mOwner->GetGameObject()), mClass,
                                    mOwner->GetOffenseMultiplier());
    desc.mTarget = IsLocked() ? const_cast<GameObject*>(GetLocked()) : 0;
    desc.mBodyID = IsLocked() ? GetLockedTargetBodyId() : -1;
    desc.mFireWeapon = this;

    pClass = static_cast<const WeaponCannonClass*>(mClass);
    bool bSecondary;
    if (pClass->mSecondaryOrdnancePeriod && pClass->mSecondaryOrdnanceClass)
    {
        bSecondary = mOrdCounter++ % pClass->mSecondaryOrdnancePeriod == 0;
    }
    else
    {
        bSecondary = false;
    }

    bool bFlash = true;
    if (NetGame::IsEnabled())
    {
        if (NetGame::IsClient())
        {
            if (!netIsLocalTurn || NetGame::GetJoystickIndex(mOwner->mPlayerId) < 0)
            {
                return true;
            }
        }
        else
        {
            bFlash = NetGame::IsOneWorld() && !NetGame::IsDedicated();
            NetGame::CreateEvent_CreateOrdnance(pClass->mOrdnanceClass, desc);
            if (bSecondary)
            {
                NetGame::CreateEvent_CreateOrdnance(
                    static_cast<const WeaponCannonClass*>(mClass)->mSecondaryOrdnanceClass, desc);
            }
        }
    }
    if (bFlash)
    {
        mClass->CreateMuzzleFlash(mFire);
    }

    Ordnance* pOrdnance = CreateOrdnance(desc);
    if (mClass->mInstantPlayFireAnim)
    {
        mFiredFlag = 0;
    }
    SetOrdnance(pOrdnance);
    if (bSecondary)
    {
        static_cast<const WeaponCannonClass*>(mClass)->mSecondaryOrdnanceClass->Build(desc);
    }

    pClass = static_cast<const WeaponCannonClass*>(mClass);
    if (pClass->mKickStrength != 0.0f)
    {
        mOwner->ApplyWeaponKick(pClass->mKickStrength,
                                (1.0f / pClass->mKickBuildup + 2.0f) * pClass->mShotDelayMin);
    }

    CombatUtil::SendDamageEvent(mOwner, GetHandleObject(mTarget), mTrigger);
    return true;
}

// FUNCTION: SWBF2 0x0067f920
void WeaponCannon::PlayWarmUpSound()
{
    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);
    if (!pClass->mWarmUpSound.GetProperties() || mSoundControlWarmUp.GetPlaying())
    {
        return;
    }

    GameObject* pObject = mOwner->GetGameObject();
    const PblMatrix& mMatrix = GetCollisionMatrix(pObject);
    const PblVector3& vVel = GetVelocity(pObject);
    mSoundPropsWarmUp = pClass->mWarmUpSound;
    mSoundControlWarmUp.Play(&mSoundPropsWarmUp, &GetTranslation(mMatrix), &vVel, true);
    if (Snd::VoiceVirtual* pVoice = mSoundControlWarmUp.GetVoiceVirtual())
    {
        static_cast<Snd::VoiceVirtualBus*>(pVoice)->SetGain(0.0f);
        pVoice->SetPitch(pClass->mWarmUpSoundStartPitch);
    }
}

// FUNCTION: SWBF2 0x0067f9e0
void WeaponCannon::StopWarmUpSound()
{
    mSoundControlWarmUp.Stop(true);
}

// FUNCTION: SWBF2 0x0067f9f0
void WeaponCannon::UpdateWarmUpSound(float fDeltaTime)
{
    if (!mSoundControlWarmUp.GetPlaying())
    {
        return;
    }
    Snd::VoiceVirtual* pVoice = mSoundControlWarmUp.GetVoiceVirtual();
    if (!pVoice)
    {
        return;
    }
    Snd::VoiceVirtualBus* pBus = static_cast<Snd::VoiceVirtualBus*>(pVoice);

    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);
    float fGain = pBus->GetGain();
    if (mWarmUp)
    {
        if (mWarmCoolTimer > 0.0f && pClass->mWarmUpTime != 0.0f)
        {
            float t = Clamp((pClass->mWarmUpTime - mWarmCoolTimer) / pClass->mWarmUpTime, 0.0f, 1.0f);
            pVoice->SetPitch(mSoundPropsWarmUp.GetPitch() * t + (1.0f - t) * pClass->mWarmUpSoundStartPitch);
        }
        else
        {
            pVoice->SetPitch(mSoundPropsWarmUp.GetPitch());
        }

        if (pClass->mWarmUpSoundFadeInTime > 0.0f)
        {
            fGain = Clamp(mSoundPropsWarmUp.GetGain() * fDeltaTime / pClass->mWarmUpSoundFadeInTime + fGain, 0.0f,
                          mSoundPropsWarmUp.GetGain());
        }
        else
        {
            fGain = mSoundPropsWarmUp.GetGain();
        }
    }
    else
    {
        if (pClass->mCoolDownTime != 0.0f)
        {
            float fPitch;
            if (mWarmCoolTimer > 0.0f)
            {
                float t = Clamp((pClass->mCoolDownTime - mWarmCoolTimer) / pClass->mCoolDownTime, 0.0f, 1.0f);
                fPitch = mSoundPropsWarmUp.GetPitch() * (1.0f - t) + pClass->mWarmUpSoundStartPitch * t;
            }
            else
            {
                fPitch = pClass->mWarmUpSoundStartPitch;
            }
            pVoice->SetPitch(fPitch);
        }

        float fFadeOut = pClass->mWarmUpSoundFadeOutTime;
        if (fFadeOut >= mWarmCoolTimer)
        {
            if (fFadeOut > 0.0f)
            {
                fGain = Clamp(fGain - mSoundPropsWarmUp.GetGain() * fDeltaTime / fFadeOut, 0.0f,
                              mSoundPropsWarmUp.GetGain());
            }
            else
            {
                fGain = 0.0f;
            }
            if (fGain == 0.0f)
            {
                StopWarmUpSound();
                return;
            }
        }
    }
    pBus->SetGain(fGain);
}

// FUNCTION: SWBF2 0x0067fc10
void WeaponCannon::PlayBarrelSound()
{
    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);
    if (!pClass->mBarrelSound.GetProperties() || mSoundControlBarrel.GetPlaying())
    {
        return;
    }

    GameObject* pObject = mOwner->GetGameObject();
    const PblMatrix& mMatrix = GetCollisionMatrix(pObject);
    const PblVector3& vVel = GetVelocity(pObject);
    mSoundPropsBarrel = pClass->mBarrelSound;
    mSoundControlBarrel.Play(&mSoundPropsBarrel, &GetTranslation(mMatrix), &vVel, true);
    if (Snd::VoiceVirtual* pVoice = mSoundControlBarrel.GetVoiceVirtual())
    {
        static_cast<Snd::VoiceVirtualBus*>(pVoice)->SetGain(0.0f);
    }
}

// FUNCTION: SWBF2 0x0067fcb0
void WeaponCannon::StopBarrelSound()
{
    mSoundControlBarrel.Stop(true);
}

// FUNCTION: SWBF2 0x0067fcc0
void WeaponCannon::UpdateBarrelSound(float fDeltaTime)
{
    if (!mSoundControlBarrel.GetPlaying())
    {
        return;
    }
    Snd::VoiceVirtual* pVoice = mSoundControlBarrel.GetVoiceVirtual();
    if (!pVoice)
    {
        return;
    }
    Snd::VoiceVirtualBus* pBus = static_cast<Snd::VoiceVirtualBus*>(pVoice);

    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);
    float fGain = pBus->GetGain();
    if (mWarmUp)
    {
        if (pClass->mBarrelSoundFadeInTime > 0.0f)
        {
            fGain = Clamp(mSoundPropsBarrel.GetGain() * fDeltaTime / pClass->mBarrelSoundFadeInTime + fGain, 0.0f,
                          mSoundPropsBarrel.GetGain());
        }
        else
        {
            fGain = mSoundPropsBarrel.GetGain();
        }
    }
    else
    {
        if (pClass->mBarrelSoundFadeOutTime > 0.0f)
        {
            fGain = Clamp(fGain - mSoundPropsBarrel.GetGain() * fDeltaTime / pClass->mBarrelSoundFadeOutTime, 0.0f,
                          mSoundPropsBarrel.GetGain());
        }
        else
        {
            fGain = 0.0f;
        }
        if (fGain == 0.0f)
        {
            StopBarrelSound();
            return;
        }
    }
    pBus->SetGain(fGain);
}

// FUNCTION: SWBF2 0x0067fde0
void WeaponCannon::PlayCoolDownSound()
{
    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);
    if (!pClass->mCoolDownSound.GetProperties() || mSoundControlCoolDown.GetPlaying())
    {
        return;
    }

    GameObject* pObject = mOwner->GetGameObject();
    const PblMatrix& mMatrix = GetCollisionMatrix(pObject);
    const PblVector3& vVel = GetVelocity(pObject);
    mSoundPropsCoolDown = pClass->mCoolDownSound;
    mSoundControlCoolDown.Play(&mSoundPropsCoolDown, &GetTranslation(mMatrix), &vVel, true);
    if (Snd::VoiceVirtual* pVoice = mSoundControlCoolDown.GetVoiceVirtual())
    {
        static_cast<Snd::VoiceVirtualBus*>(pVoice)->SetGain(0.0f);
    }
}

// FUNCTION: SWBF2 0x0067fe80
void WeaponCannon::StopCoolDownSound()
{
    mSoundControlCoolDown.Stop(true);
}

// FUNCTION: SWBF2 0x0067fe90
void WeaponCannon::UpdateCoolDownSound(float fDeltaTime)
{
    if (!mSoundControlCoolDown.GetPlaying())
    {
        return;
    }
    Snd::VoiceVirtual* pVoice = mSoundControlCoolDown.GetVoiceVirtual();
    if (!pVoice || mWarmUp)
    {
        return;
    }
    Snd::VoiceVirtualBus* pBus = static_cast<Snd::VoiceVirtualBus*>(pVoice);

    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);
    if (!(pClass->mCoolDownTime > 0.0f))
    {
        return;
    }

    float fPitch;
    if (mWarmCoolTimer > 0.0f)
    {
        float t = Clamp((pClass->mCoolDownTime - mWarmCoolTimer) / pClass->mCoolDownTime, 0.0f, 1.0f);
        fPitch = mSoundPropsCoolDown.GetPitch() * (1.0f - t) + pClass->mCoolDownSoundEndPitch * t;
    }
    else
    {
        fPitch = pClass->mCoolDownSoundEndPitch;
    }
    pVoice->SetPitch(fPitch);

    float fGain = pBus->GetGain();
    if (mWarmCoolTimer > pClass->mCoolDownSoundFadeOutTime)
    {
        if (pClass->mCoolDownSoundFadeInTime > 0.0f)
        {
            fGain = Clamp(mSoundPropsCoolDown.GetGain() * fDeltaTime / pClass->mCoolDownSoundFadeInTime + fGain, 0.0f,
                          mSoundPropsCoolDown.GetGain());
        }
        else
        {
            fGain = mSoundPropsCoolDown.GetGain();
        }
    }
    else
    {
        if (pClass->mCoolDownSoundFadeOutTime > 0.0f)
        {
            fGain = Clamp(fGain - mSoundPropsCoolDown.GetGain() * fDeltaTime / pClass->mCoolDownSoundFadeOutTime,
                          0.0f, mSoundPropsCoolDown.GetGain());
        }
        else
        {
            fGain = 0.0f;
        }
        if (fGain == 0.0f)
        {
            StopCoolDownSound();
            return;
        }
    }
    pBus->SetGain(fGain);
}

// FUNCTION: SWBF2 0x00680210
WeaponCannonClass::WeaponCannonClass(WeaponCannonClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    mMaxPressedTime = pBase->mMaxPressedTime;
    mShotDelayMin = pBase->mShotDelayMin;
    mShotDelayMax = pBase->mShotDelayMax;
    mPitchSpread = pBase->mPitchSpread;
    mYawSpread = pBase->mYawSpread;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            mStanceSpread[i][j] = pBase->mStanceSpread[i][j];
        }
    }
    mSpreadPerShot = pBase->mSpreadPerShot;
    mSpreadRecoverRate = pBase->mSpreadRecoverRate;
    mSpreadThreshold = pBase->mSpreadThreshold;
    mSpreadLimit = pBase->mSpreadLimit;
    mKickStrength = pBase->mKickStrength;
    mKickBuildup = pBase->mKickBuildup;
    mSalvoCount = pBase->mSalvoCount;
    mShotsPerSalvo = pBase->mShotsPerSalvo;
    mSalvoDelayMin = pBase->mSalvoDelayMin;
    mSalvoDelayMax = pBase->mSalvoDelayMax;
    mInitialSalvoDelay = pBase->mInitialSalvoDelay;
    mPatternArray = 0;
    mPatternCount = 0;
    mWarmUpTime = pBase->mWarmUpTime;
    mCoolDownTime = pBase->mCoolDownTime;
    mWarmUpSound = pBase->mWarmUpSound;
    mWarmUpSoundStartPitch = pBase->mWarmUpSoundStartPitch;
    mWarmUpSoundFadeInTime = pBase->mWarmUpSoundFadeInTime;
    mWarmUpSoundFadeOutTime = pBase->mWarmUpSoundFadeOutTime;
    mBarrelSound = pBase->mBarrelSound;
    mBarrelSoundFadeInTime = pBase->mBarrelSoundFadeInTime;
    mBarrelSoundFadeOutTime = pBase->mBarrelSoundFadeOutTime;
    mCoolDownSound = pBase->mCoolDownSound;
    mCoolDownSoundEndPitch = pBase->mCoolDownSoundEndPitch;
    mCoolDownSoundFadeInTime = pBase->mCoolDownSoundFadeInTime;
    mCoolDownSoundFadeOutTime = pBase->mCoolDownSoundFadeOutTime;
    mSecondaryOrdnanceClass = pBase->mSecondaryOrdnanceClass;
    mSecondaryOrdnancePeriod = pBase->mSecondaryOrdnancePeriod;
}

// FUNCTION: SWBF2 0x00680460
void WeaponCannonClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    const float DEG2RAD = 0.017453292f;
    float* pAngle;

    switch (uProperty)
    {
    case 0x2819ffa0:
        sscanf(pszValue, "%f", &mMaxPressedTime);
        return;

    case 0xd9356908:
        mShotDelayMin = 0.0f;
        mShotDelayMax = 0.0f;
        if (sscanf(pszValue, "%f %f", &mShotDelayMin, &mShotDelayMax) == 1)
        {
            mShotDelayMax = mShotDelayMin;
        }
        if (mShotDelayMin < 0.01f)
        {
            mShotDelayMin = 0.01f;
        }
        if (mShotDelayMin > mShotDelayMax)
        {
            mShotDelayMax = mShotDelayMin;
        }
        return;

    case 0xaedcbd12:
    {
        float fSpread;
        if (sscanf(pszValue, "%f", &fSpread))
        {
            mYawSpread = fSpread * 0.05f;
            mPitchSpread = fSpread * 0.025f;
        }
        return;
    }

    case 0x8d0cb6ca:
        pAngle = &mPitchSpread;
        break;
    case 0xebefdcd7:
        pAngle = &mYawSpread;
        break;
    case 0x4c081d30:
        pAngle = &mStanceSpread[0][0];
        break;
    case 0xf7561531:
        pAngle = &mStanceSpread[0][1];
        break;
    case 0x5bc0137f:
        pAngle = &mStanceSpread[0][2];
        break;
    case 0x81fec758:
        pAngle = &mStanceSpread[1][0];
        break;
    case 0xfbde8e79:
        pAngle = &mStanceSpread[1][1];
        break;
    case 0x67bfc1d7:
        pAngle = &mStanceSpread[1][2];
        break;
    case 0x4a29a3ca:
        pAngle = &mStanceSpread[2][0];
        break;
    case 0x73608c73:
        pAngle = &mStanceSpread[2][1];
        break;
    case 0x7d9da8bd:
        pAngle = &mStanceSpread[2][2];
        break;
    case 0xef4935b0:
    case 0x43ab2c7d:
        pAngle = &mSpreadPerShot;
        break;
    case 0xccedb638:
        pAngle = &mSpreadRecoverRate;
        break;
    case 0x2613b9b9:
        pAngle = &mSpreadThreshold;
        break;
    case 0x85a6b6fd:
        pAngle = &mSpreadLimit;
        break;
    case 0x16e4db4a:
        pAngle = &mKickStrength;
        break;

    case 0x33841a8a:
        sscanf(pszValue, "%f", &mSpreadRecoverRate);
        mSpreadRecoverRate = mSpreadPerShot * mSpreadRecoverRate;
        mSpreadLimit = mSpreadRecoverRate / mShotDelayMin;
        return;

    case 0x069ad4d0:
        sscanf(pszValue, "%f", &mKickBuildup);
        return;
    case 0x9260ea63:
        sscanf(pszValue, "%i", &mSalvoCount);
        return;
    case 0x03b38558:
        sscanf(pszValue, "%i", &mShotsPerSalvo);
        return;

    case 0xd76f37db:
        mSalvoDelayMin = 0.0f;
        mSalvoDelayMax = 0.0f;
        if (sscanf(pszValue, "%f %f", &mSalvoDelayMin, &mSalvoDelayMax) == 1)
        {
            mSalvoDelayMax = mSalvoDelayMin;
        }
        if (mSalvoDelayMin > mSalvoDelayMax)
        {
            mSalvoDelayMax = mSalvoDelayMin;
        }
        return;

    case 0xc8388d9d:
        sscanf(pszValue, "%f", &mInitialSalvoDelay);
        return;

    case 0xa4dfafc0:
    {
        int iCount = 0;
        sscanf(pszValue, "%i", &iCount);
        delete[] mPatternArray;
        mPatternCount = 0;
        if (iCount > 0)
        {
            mPatternArray = new ShotPattern[iCount];
            memset(mPatternArray, 0, iCount * sizeof(ShotPattern));
        }
        return;
    }

    case 0xa4120aa8:
        if (sscanf(pszValue, "%f %f", &mPatternArray[mPatternCount].mPitch, &mPatternArray[mPatternCount].mYaw))
        {
            mPatternArray[mPatternCount].mPitch *= DEG2RAD;
            mPatternArray[mPatternCount].mYaw *= DEG2RAD;
            ++mPatternCount;
        }
        return;

    case 0x44edeee4:
        sscanf(pszValue, "%f", &mWarmUpTime);
        return;
    case 0x9ecac3a1:
        sscanf(pszValue, "%f", &mCoolDownTime);
        return;

    case 0xca333e94:
        mWarmUpSound.SetID(pszValue, false, false);
        return;
    case 0x1a2034be:
        sscanf(pszValue, "%f", &mWarmUpSoundStartPitch);
        return;
    case 0x22e5153c:
        sscanf(pszValue, "%f", &mWarmUpSoundFadeInTime);
        return;
    case 0x25820dbf:
        sscanf(pszValue, "%f", &mWarmUpSoundFadeOutTime);
        return;

    case 0x2298cae8:
        mBarrelSound.SetID(pszValue, false, false);
        return;
    case 0x674fc158:
        sscanf(pszValue, "%f", &mBarrelSoundFadeInTime);
        return;
    case 0x0f1ca143:
        sscanf(pszValue, "%f", &mBarrelSoundFadeOutTime);
        return;

    case 0x940f9407:
        mCoolDownSound.SetID(pszValue, false, false);
        return;
    case 0x3a3d88da:
        sscanf(pszValue, "%f", &mCoolDownSoundEndPitch);
        return;
    case 0x7236488b:
        sscanf(pszValue, "%f", &mCoolDownSoundFadeInTime);
        return;
    case 0x4e159f7a:
        sscanf(pszValue, "%f", &mCoolDownSoundFadeOutTime);
        return;

    case 0x6e364fc1:
    case 0xdddb253a:
        mSecondaryOrdnanceClass = Factory<Ordnance, OrdnanceClass, OrdnanceDesc>::Find(PblHash(pszValue));
        return;
    case 0x82e86218:
        mSecondaryOrdnancePeriod = atoi(pszValue);
        return;

    default:
        WeaponClass::SetProperty(uProperty, pszValue);
        return;
    }

    if (sscanf(pszValue, "%f", pAngle))
    {
        *pAngle *= DEG2RAD;
    }
}

// FUNCTION: SWBF2 0x00680a70
WeaponClass* WeaponCannonClass::Derive(unsigned int uHash)
{
    return new WeaponCannonClass(this, uHash);
}

// FUNCTION: SWBF2 0x00680ae0
Weapon* WeaponCannonClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponCannon(this, desc);
}

// FUNCTION: SWBF2 0x00680b50
float WeaponCannonClass::GetHeatPerSalvo()
{
    return (float)mShotsPerSalvo * mHeatPerShot;
}

// FUNCTION: SWBF2 0x00680b80
float WeaponCannonClass::GetShotsPerSalvo()
{
    return (float)mShotsPerSalvo;
}

// FUNCTION: SWBF2 0x00680ba0
int WeaponCannonClass::GetBarrageMin()
{
    if (mTriggerSingle)
    {
        return 1;
    }
    return WeaponClass::GetBarrageMin();
}

// FUNCTION: SWBF2 0x00680bc0
int WeaponCannonClass::GetBarrageMax()
{
    if (mTriggerSingle)
    {
        return 1;
    }
    return WeaponClass::GetBarrageMax();
}

// SYNTHETIC: SWBF2 0x006801e0
// WeaponCannonClass::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00680450
// WeaponCannonClass::~WeaponCannonClass
