#include <stdio.h>
#include <stdlib.h>

#include "WeaponGrenade.h"
#include "Aimer.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "EntitySoldier.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Ordnance.h"
#include "SoldierAnimatorClass.h"
#include "Trigger.h"

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

extern bool netIsLocalTurn;

class AmmoCounter
{
public:
    bool CanExpendRounds(float fRounds) const;
    void ExpendRounds(float fRounds);
};

namespace GameLoop { extern float sClientDeltaTime; }

// GLOBAL: SWBF2 0x01f9a8cc
// SYNTHETIC: SWBF2 0x00407190
// `dynamic initializer for 'WeaponGrenadeClass::rttiHashWeaponGrenadeClass''
PblHash WeaponGrenadeClass::rttiHashWeaponGrenadeClass("WeaponGrenadeClass");
char WeaponGrenadeClass::rttiNameWeaponGrenadeClass[] = "WeaponGrenadeClass";

// FUNCTION: SWBF2 0x00685770
WeaponGrenadeClass::WeaponGrenadeClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    mLockOnAngle = 0.0f;
    mReloadTime = 0.6f;

    mShotDelay = 0.125f;
    mInitialSalvoDelay[0] = 0.5f;
    mInitialSalvoDelay[1] = 0.36f;
    mInitialSalvoDelay[2] = 0.584f;
    mShotElevate = 0.3f;
    mMaxPressedTime = 0.0f;

    mHideOnFire = true;
    mForcedFireAnim = -1;

    for (int i = 0; i < 3; ++i)
    {
        m_avFireOffset[i].x = 0.0f;
        m_avFireOffset[i].y = 0.0f;
        m_avFireOffset[i].z = 0.0f;
    }
}

// FUNCTION: SWBF2 0x00684f90
bool WeaponGrenadeClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponGrenadeClass || WeaponClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00684fc0
unsigned int WeaponGrenadeClass::GetDerivedRtti()
{
    return rttiHashWeaponGrenadeClass;
}

// FUNCTION: SWBF2 0x00684fd0
const char* WeaponGrenadeClass::GetDerivedRttiName()
{
    return rttiNameWeaponGrenadeClass;
}

// FUNCTION: SWBF2 0x00685840
WeaponGrenadeClass::WeaponGrenadeClass(WeaponGrenadeClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    mShotDelay = pBase->mShotDelay;
    mInitialSalvoDelay[0] = pBase->mInitialSalvoDelay[0];
    mInitialSalvoDelay[1] = pBase->mInitialSalvoDelay[1];
    mInitialSalvoDelay[2] = pBase->mInitialSalvoDelay[2];
    mShotElevate = pBase->mShotElevate;
    mMaxPressedTime = pBase->mMaxPressedTime;
    mHideOnFire = pBase->mHideOnFire;
    mForcedFireAnim = pBase->mForcedFireAnim;
    for (int i = 0; i < 3; ++i)
    {
        m_avFireOffset[i] = pBase->m_avFireOffset[i];
    }
}

// FUNCTION: SWBF2 0x006858f0
WeaponGrenadeClass::~WeaponGrenadeClass()
{
}

// SYNTHETIC: SWBF2 0x00685810
// WeaponGrenadeClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00685900
void WeaponGrenadeClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0xD9356908)
    {
        sscanf(pszValue, "%f", &mShotDelay);
        if (mShotDelay < 0.01f)
        {
            mShotDelay = 0.01f;
        }
    }
    else if (uProperty == 0xC8388D9D)
    {
        sscanf(pszValue, "%f", &mInitialSalvoDelay[1]);
        mInitialSalvoDelay[0] = mInitialSalvoDelay[1];
        mInitialSalvoDelay[2] = mInitialSalvoDelay[1];
    }
    else if (uProperty == 0x2A708F20)
    {
        sscanf(pszValue, "%f", &mInitialSalvoDelay[0]);
    }
    else if (uProperty == 0x36F0BFCD)
    {
        sscanf(pszValue, "%f", &mInitialSalvoDelay[2]);
    }
    else if (uProperty == 0xAE3DB60D)
    {
        sscanf(pszValue, "%f", &mShotElevate);
    }
    else if (uProperty == 0x2819FFA0)
    {
        sscanf(pszValue, "%f", &mMaxPressedTime);
    }
    else if (uProperty == 0x5578D44C)
    {
        mHideOnFire = atoi(pszValue) != 0;
    }
    else if (uProperty == 0x079C1D86)
    {
        mForcedFireAnim = (char)atoi(pszValue);
        if (mForcedFireAnim < 0 || mForcedFireAnim > 1)
        {
            mForcedFireAnim = -1;
        }
    }
    else
    {
        WeaponClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x00685a50
WeaponClass* WeaponGrenadeClass::Derive(unsigned int uHash)
{
    return new WeaponGrenadeClass(this, uHash);
}

// FUNCTION: SWBF2 0x00685ac0
Weapon* WeaponGrenadeClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponGrenade(this, desc);
}

// FUNCTION: SWBF2 0x00685b30
bool WeaponGrenadeClass::PostLoadInit()
{
    if (!WeaponClass::PostLoadInit())
    {
        return false;
    }

    PblMatrix matrix;
    if (mIsOffhand)
    {
        if (SoldierAnimatorClass::Instance()->GetAnimationBoneMatrix(
                SoldierAnimationBank::HUMAN_RIFLE, SoldierAnimType::STAND, SoldierAnimType::SHOOT_SECONDARY,
                0xb7ec1d31, mInitialSalvoDelay[0], matrix))
        {
            m_avFireOffset[0] = matrix.GetTranslation();
            m_avFireOffset[0].x = 0.0f;
        }
        if (SoldierAnimatorClass::Instance()->GetAnimationBoneMatrix(
                SoldierAnimationBank::HUMAN_RIFLE, SoldierAnimType::STAND, SoldierAnimType::SHOOT_SECONDARY2,
                0xb7ec1d31, mInitialSalvoDelay[1], matrix))
        {
            m_avFireOffset[1] = matrix.GetTranslation();
            m_avFireOffset[1].x = 0.0f;
        }
    }
    else
    {
        PblVector3 v;
        if (SoldierAnimatorClass::Instance()->GetAnimationBoneMatrix(
                SoldierAnimationBank::HUMAN_RIFLE, SoldierAnimType::STAND, SoldierAnimType::SHOOT_SECONDARY,
                0x2b960099, mInitialSalvoDelay[0], matrix))
        {
            D3DXVec3TransformCoord(&v, &mFirePointOffset, &matrix);
            m_avFireOffset[0] = v;
        }
        if (SoldierAnimatorClass::Instance()->GetAnimationBoneMatrix(
                SoldierAnimationBank::HUMAN_RIFLE, SoldierAnimType::STAND, SoldierAnimType::SHOOT_SECONDARY2,
                0x2b960099, mInitialSalvoDelay[1], matrix))
        {
            D3DXVec3TransformCoord(&v, &mFirePointOffset, &matrix);
            m_avFireOffset[1] = v;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00684fe0
WeaponGrenade::WeaponGrenade(const WeaponGrenadeClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    mNetFiredFlag = false;
    mFired = false;
    mStrength = 0.0f;
    m_iFirePos = 0;
}

// GLOBAL: SWBF2 0x01f9a8d0
// SYNTHETIC: SWBF2 0x00407180
// `dynamic initializer for 'WeaponGrenade::rttiHashWeaponGrenade''
PblHash WeaponGrenade::rttiHashWeaponGrenade("WeaponGrenade");
char WeaponGrenade::rttiNameWeaponGrenade[] = "WeaponGrenade";

// FUNCTION: SWBF2 0x00684f40
bool WeaponGrenade::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponGrenade || reinterpret_cast<const Weapon*>(this)->Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00684f70
unsigned int WeaponGrenade::GetDerivedRtti() const
{
    return rttiHashWeaponGrenade;
}

// FUNCTION: SWBF2 0x00684f80
const char* WeaponGrenade::GetDerivedRttiName() const
{
    return rttiNameWeaponGrenade;
}

// FUNCTION: SWBF2 0x00685060
bool WeaponGrenade::IsBusy() const
{
    if (mState != FIRE && mState != FIRE2)
    {
        return false;
    }
    return !mFired || mStateLimit > mStateTimer + GameLoop::sClientDeltaTime;
}

// FUNCTION: SWBF2 0x006850d0
void WeaponGrenade::EnterFire()
{
    const WeaponGrenadeClass* pClass = (const WeaponGrenadeClass*)mClass;

    if (pClass->mHideOnFire)
    {
        mHideWeapon = 1;
    }
    mFired = false;

    if (pClass->mMaxPressedTime > 0.0f)
    {
        mStrength = mStateTimer / pClass->mMaxPressedTime;
    }
    else
    {
        mStrength = 1.0f;
    }

    char iFireAnim = pClass->mForcedFireAnim;
    if (iFireAnim < 0)
    {
        iFireAnim = mStrength > 0.4f;
    }
    mState = (WeaponState)(FIRE + iFireAnim);

    if (m_iSoldierState == 2)
    {
        m_iFirePos = 2;
    }
    else
    {
        m_iFirePos = iFireAnim;
    }

    mStateTimer = 0.0f;
    mStateLimit = pClass->mInitialSalvoDelay[m_iFirePos] + pClass->mShotDelay;
    mFiredFlag = 1;
    mNetFiredFlag = true;

    if (pClass->mTriggerSingle)
    {
        mTrigger->ClearUntilPressed();
    }
}

// FUNCTION: SWBF2 0x00685330
bool WeaponGrenade::CheckCharge() const
{
    if (!mTrigger || !(((const WeaponGrenadeClass*)mClass)->mMaxPressedTime > 0.0f))
    {
        return false;
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x00685350
bool WeaponGrenade::CheckFire() const
{
    if (!mTrigger || reinterpret_cast<const Weapon*>(this)->CheckOverheat())
    {
        return false;
    }
    if (((const WeaponGrenadeClass*)mClass)->mMaxPressedTime > 0.0f)
    {
        return mTrigger->IsRelease();
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x00685050
WeaponGrenade::~WeaponGrenade()
{
}

// SYNTHETIC: SWBF2 0x00685020
// WeaponGrenade::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006850a0
void WeaponGrenade::SignalFire()
{
    unsigned int bFiredFlag = mFiredFlag;
    Weapon::SignalFire();
    mFiredFlag = bFiredFlag;
}

// FUNCTION: SWBF2 0x006851d0
bool WeaponGrenade::UpdateFire(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    if (!mSelectedFlag)
    {
        mFired = true;
    }
    if (!mFired)
    {
        const WeaponGrenadeClass* pClass = (const WeaponGrenadeClass*)mClass;
        if (mStateTimer >= pClass->mInitialSalvoDelay[m_iFirePos])
        {
            if (m_pAmmoCounter->CanExpendRounds(pClass->mRoundsPerShot) &&
                (!m_pEnergyBar->m_bOverheat || !(pClass->mHeatPerShot > 0.0f)) && Fire())
            {
                m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot);
                m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot);
            }
            mNetFiredFlag = false;
            mFired = true;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00685290
void WeaponGrenade::ExitFire()
{
    Weapon::ExitFire();
}

// FUNCTION: SWBF2 0x006852a0
void WeaponGrenade::EnterCharge()
{
    mStateTimer = 0.0f;
    mStateLimit = ((const WeaponGrenadeClass*)mClass)->mMaxPressedTime;
    Weapon::EnterCharge();
}

// FUNCTION: SWBF2 0x006852c0
bool WeaponGrenade::UpdateCharge(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    if (mStateTimer >= mStateLimit)
    {
        mStateTimer = mStateLimit;
    }
    Weapon::UpdateCharge(fDeltaTime);
    return true;
}

// FUNCTION: SWBF2 0x00685310
void WeaponGrenade::ExitCharge()
{
    Weapon::ExitCharge();
}

// FUNCTION: SWBF2 0x00685320
void WeaponGrenade::ExitReload()
{
    mHideWeapon = 0;
    Weapon::ExitReload();
}

// FUNCTION: SWBF2 0x006853a0
bool WeaponGrenade::FireDone() const
{
    if (!mFired || !(mStateTimer >= mStateLimit))
    {
        return false;
    }
    return mClass->mTriggerSingle || !CheckFire() || CheckReload() || CheckEmpty();
}

// FUNCTION: SWBF2 0x00685400
void WeaponGrenade::EnterEmpty()
{
    Weapon::EnterEmpty();
    mStateLimit = ((const WeaponGrenadeClass*)mClass)->mShotDelay;
}

// FUNCTION: SWBF2 0x00685420
bool WeaponGrenade::Fire()
{
    if (!NetGame::IsEnabled())
    {
        PlayFireSound(false);
    }

    OrdnanceDesc desc;
    if (mOwner)
    {
        PblVector3 vOffset = ((const WeaponGrenadeClass*)mClass)->m_avFireOffset[m_iFirePos];
        if (mOwner->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            float fScale = static_cast<EntitySoldier*>(mOwner->GetGameObject())->mClass->mSkeletonRootScale;
            if (fScale != 1.0f)
            {
                vOffset.x *= fScale;
                vOffset.y *= fScale;
                vOffset.z *= fScale;
            }
        }
        PblVector3 vPos;
        D3DXVec3TransformCoord(&vPos, &vOffset, &mOwner->GetMatrix());
        desc.mPosition = vPos;
    }
    else
    {
        desc.mPosition = mAimer->mFirePos;
    }

    desc.mDirection = mAimer->mDirection;
    if (mOwner->mPlayerId >= 0)
    {
        desc.mDirection.y += ((const WeaponGrenadeClass*)mClass)->mShotElevate;
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &desc.mDirection);
        desc.mDirection = vDir;
    }
    desc.mStrength = mStrength;
    desc.mOwner = mOwner->GetGameObject();
    desc.mDamageOwner = DamageOwner(mOwner->mCharacter, mOwner->GetGameObject(), mOwner->GetGameObject()->GetTeam(),
                                    mOwner->GetGameObject()->GetEntityClass(), mClass,
                                    mOwner->GetOffenseMultiplier());
    desc.mTarget = const_cast<GameObject*>(static_cast<const GameObject*>(mTarget));
    desc.mBodyID = mTargetBodyID;
    desc.mFireWeapon = this;

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
            NetGame::CreateEvent_CreateOrdnance(mClass->mOrdnanceClass, desc);
        }
    }
    if (bFlash)
    {
        mClass->CreateMuzzleFlash(mFirePointMatrix);
    }

    mClass->mOrdnanceClass->Build(desc);
    if (((const WeaponGrenadeClass*)mClass)->mInitialSalvoDelay[m_iFirePos] > 0.0f)
    {
        mFiredFlag = 0;
    }
    return true;
}
