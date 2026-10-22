#include <stdio.h>
#include <stdlib.h>

#include "WeaponLauncher.h"
#include "Controllable.h"
#include "GameObject.h"
#include "OrdnanceMissile.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

namespace AIUtil
{

    bool CanSeePosition(const PblVector3& vFrom, Controllable* pMe, const PblVector3& vPos, const GameObject* pObj);

    float GetRandomFloat(float fMin, float fMax);
}

// GLOBAL: SWBF2 0x01f9a8e8
// SYNTHETIC: SWBF2 0x004071f0
// `dynamic initializer for 'WeaponLauncherClass::rttiHashWeaponLauncherClass''
PblHash WeaponLauncherClass::rttiHashWeaponLauncherClass("WeaponLauncherClass");
char WeaponLauncherClass::rttiNameWeaponLauncherClass[] = "WeaponLauncherClass";

// FUNCTION: SWBF2 0x00687990
WeaponLauncherClass::WeaponLauncherClass(unsigned int uHash)
    : WeaponCannonClass(uHash)
{
    mReloadTime = 0.6f;

    mLockTime = 0.0f;

    mClearLockOnFire = true;
    mHasTrackingAmmo = false;
}

// FUNCTION: SWBF2 0x00687340
bool WeaponLauncherClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponLauncherClass || WeaponCannonClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00687370
unsigned int WeaponLauncherClass::GetDerivedRtti()
{
    return rttiHashWeaponLauncherClass;
}

// FUNCTION: SWBF2 0x00687380
const char* WeaponLauncherClass::GetDerivedRttiName()
{
    return rttiNameWeaponLauncherClass;
}

// SYNTHETIC: SWBF2 0x006879d0
// WeaponLauncherClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00687a00
WeaponLauncherClass::WeaponLauncherClass(WeaponLauncherClass* pBase, unsigned int uHash)
    : WeaponCannonClass(pBase, uHash)
{
    mLockTime = pBase->mLockTime;
    mClearLockOnFire = pBase->mClearLockOnFire;
    mHasTrackingAmmo = pBase->mHasTrackingAmmo;
}

// FUNCTION: SWBF2 0x00687a50
WeaponLauncherClass::~WeaponLauncherClass()
{
}

// FUNCTION: SWBF2 0x00687a60
void WeaponLauncherClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x448FBCE3:
        sscanf(pszValue, "%f", &mLockTime);
        break;
    case 0xC61C7CCC:
        mClearLockOnFire = atoi(pszValue) != 0;
        break;
    case 0xE8345A43:
        break;
    case 0x399BF05D:
    case 0xF8899C5E:
        WeaponCannonClass::SetProperty(uProperty, pszValue);
        mHasTrackingAmmo = mOrdnanceClass &&
                           mOrdnanceClass->IsRtti(OrdnanceMissileClass::rttiHashOrdnanceMissileClass);
        break;
    default:
        WeaponCannonClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00687b10
WeaponClass* WeaponLauncherClass::Derive(unsigned int uHash)
{
    return new WeaponLauncherClass(this, uHash);
}

// FUNCTION: SWBF2 0x00687b80
WeaponLauncher* WeaponLauncherClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponLauncher(this, desc);
}

// GLOBAL: SWBF2 0x01f9a8e4
// SYNTHETIC: SWBF2 0x004071e0
// `dynamic initializer for 'WeaponLauncher::rttiHashWeaponLauncher''
PblHash WeaponLauncher::rttiHashWeaponLauncher("WeaponLauncher");
char WeaponLauncher::rttiNameWeaponLauncher[] = "WeaponLauncher";

// FUNCTION: SWBF2 0x00687390
WeaponLauncher::WeaponLauncher(const WeaponLauncherClass* pClass, const WeaponDesc& desc)
    : WeaponCannon(pClass, desc)
{
    mCurTarget = 0;
    mCurTargetBodyID = -1;
    mLocked = !static_cast<const WeaponLauncherClass*>(mClass)->mHasTrackingAmmo;
}

// FUNCTION: SWBF2 0x006872a0
bool WeaponLauncher::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponLauncher || reinterpret_cast<const Weapon*>(this)->Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x006872d0
unsigned int WeaponLauncher::GetDerivedRtti() const
{
    return rttiHashWeaponLauncher;
}

// FUNCTION: SWBF2 0x006872e0
const char* WeaponLauncher::GetDerivedRttiName() const
{
    return rttiNameWeaponLauncher;
}

// FUNCTION: SWBF2 0x006872f0
bool WeaponLauncher::IsLocked() const
{
    return mLocked;
}

// FUNCTION: SWBF2 0x00687300
const GameObject* WeaponLauncher::GetLocked() const
{
    PblHandle<const GameObject>& h = const_cast<WeaponLauncher*>(this)->mCurTarget;
    if (h.mObject && h.mObject->mHandleId != h.mSavedHandleId)
    {
        h.mObject = 0;
        h.mSavedHandleId = 0;
    }
    return h.mObject;
}

// FUNCTION: SWBF2 0x00687330
int WeaponLauncher::GetLockedTargetBodyId() const
{
    return mCurTargetBodyID;
}

// FUNCTION: SWBF2 0x006874b0
void WeaponLauncher::SetupTarget(const GameObject** ppTarget, int* pBodyId) const
{
    if (mOwner && mOwner->mPlayerId >= 0)
    {
        if (ppTarget)
        {
            const GameObject* pTarget = mOwner->GetTargetLockedObj();
            *ppTarget = pTarget;
            if (pTarget)
            {
                if (!(mClass->mTargetTypes & (1 << pTarget->mHealthType)))
                {
                    *ppTarget = 0;
                }
                else
                {
                    const PblVector3& vPos = pTarget->GetCollisionCenter();
                    PblVector3 vDelta;
                    vDelta.x = vPos.x - mOwner->mEyePoint.x;
                    vDelta.y = vPos.y - mOwner->mEyePoint.y;
                    vDelta.z = vPos.z - mOwner->mEyePoint.z;
                    PblVector3 vDir;
                    D3DXVec3Normalize(&vDir, &vDelta);
                    float fDot = mOwner->mEyeDir.x * vDir.x + mOwner->mEyeDir.y * vDir.y +
                                 mOwner->mEyeDir.z * vDir.z;
                    if (fDot < 0.9f ||
                        !AIUtil::CanSeePosition(mOwner->mEyePoint, mOwner, (*ppTarget)->GetCollisionCenter(),
                                                *ppTarget))
                    {
                        *ppTarget = 0;
                    }
                }
            }
        }
        if (pBodyId)
        {
            *pBodyId = -1;
        }
    }
    else
    {
        if (ppTarget)
        {
            *ppTarget = mTarget;
        }
        if (pBodyId)
        {
            *pBodyId = mTargetBodyID;
        }
    }
}

// FUNCTION: SWBF2 0x00687920
bool WeaponLauncher::CheckCharge() const
{
    if (static_cast<const WeaponLauncherClass*>(mClass)->mHasTrackingAmmo)
    {
        const GameObject* pTarget;
        SetupTarget(&pTarget, 0);
        if (!pTarget)
        {
            return false;
        }
        if (static_cast<const WeaponLauncherClass*>(mClass)->mLockTime > 0.0f)
        {
            return true;
        }
    }
    return WeaponCannon::CheckCharge();
}

// FUNCTION: SWBF2 0x00687980
bool WeaponLauncher::CheckFire() const
{
    return WeaponCannon::CheckFire();
}

// SYNTHETIC: SWBF2 0x00687400
// WeaponLauncher::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00687430
WeaponLauncher::~WeaponLauncher()
{
}

// FUNCTION: SWBF2 0x00687440
void WeaponLauncher::EnterIdle()
{
    mStateTimer = 0.0f;
    mLocked = !static_cast<const WeaponLauncherClass*>(mClass)->mHasTrackingAmmo;
    mCurTarget = 0;
    mCurTargetBodyID = -1;
    Weapon::EnterIdle();
}

// FUNCTION: SWBF2 0x00687480
void WeaponLauncher::ExitFire()
{
    const WeaponLauncherClass* pClass = static_cast<const WeaponLauncherClass*>(mClass);
    if (pClass->mHasTrackingAmmo && pClass->mClearLockOnFire)
    {
        mLocked = false;
    }
    WeaponCannon::ExitFire();
}

// FUNCTION: SWBF2 0x00687630
void WeaponLauncher::EnterCharge()
{
    if (!static_cast<const WeaponLauncherClass*>(mClass)->mHasTrackingAmmo)
    {
        WeaponCannon::EnterCharge();
        return;
    }
    Weapon::EnterCharge();

    const GameObject* pTarget;
    int iBodyId;
    SetupTarget(&pTarget, &iBodyId);

    const WeaponLauncherClass* pClass = static_cast<const WeaponLauncherClass*>(mClass);
    mStateLimit = pClass->mLockTime > 0.0f ? pClass->mLockTime : pClass->mMaxPressedTime;
    if (!pClass->mClearLockOnFire && pTarget && mCurTarget == pTarget && mCurTargetBodyID == mTargetBodyID)
    {
        mStateTimer = mStateLimit;
        mLocked = true;
    }
    else
    {
        mStateTimer = 0.0f;
        mLocked = false;
    }
    mCurTarget = pTarget;
    mCurTargetBodyID = iBodyId;
}

// FUNCTION: SWBF2 0x00687740
bool WeaponLauncher::UpdateCharge(float fDeltaTime)
{
    if (!static_cast<const WeaponLauncherClass*>(mClass)->mHasTrackingAmmo)
    {
        return WeaponCannon::UpdateCharge(fDeltaTime);
    }

    mStateTimer += fDeltaTime;
    if (mStateTimer >= mStateLimit)
    {
        mStateTimer = mStateLimit;
        mLocked = true;
    }

    const GameObject* pTarget;
    int iBodyId;
    SetupTarget(&pTarget, &iBodyId);
    if (!pTarget || mCurTarget != pTarget || mCurTargetBodyID != iBodyId)
    {
        mStateTimer = 0.0f;
        mCurTarget = pTarget;
        mCurTargetBodyID = iBodyId;
        mLocked = false;
    }
    return Weapon::UpdateCharge(fDeltaTime);
}

// FUNCTION: SWBF2 0x00687840
void WeaponLauncher::ExitCharge()
{
    if (!static_cast<const WeaponLauncherClass*>(mClass)->mHasTrackingAmmo)
    {
        WeaponCannon::ExitCharge();
        return;
    }

    const GameObject* pTarget;
    int iBodyId;
    SetupTarget(&pTarget, &iBodyId);
    if (!pTarget || mCurTarget != pTarget || mCurTargetBodyID != iBodyId)
    {
        mStateTimer = 0.0f;
        mCurTarget = pTarget;
        mCurTargetBodyID = iBodyId;
        mLocked = false;
    }
    Weapon::ExitCharge();
}

// FUNCTION: SWBF2 0x006878f0
void WeaponLauncher::EnterEmpty()
{
    WeaponCannon::EnterEmpty();
    const WeaponCannonClass* pClass = static_cast<const WeaponCannonClass*>(mClass);
    mStateLimit = AIUtil::GetRandomFloat(pClass->mShotDelayMin, pClass->mShotDelayMax);
}
