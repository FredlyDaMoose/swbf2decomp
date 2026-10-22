#include <stdio.h>

#include "WeaponLaser.h"
#include "Aimer.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Ordnance.h"
#include "Trigger.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include <new>

extern bool netIsLocalTurn;
namespace GameLoop { extern float sClientDeltaTime; }

namespace CombatUtil { void SendDamageEvent(const Controllable* pOwner, const GameObject* pTarget, const Trigger* pTrigger); }

class AmmoCounter
{
public:
    void ExpendRounds(float fRounds);
};

namespace
{
    EntityClass* GetEntityClass(GameObject* pObject)
    {
        typedef EntityClass* (__thiscall *Fn)(GameObject*);
        return pObject->GetEntityClass();
    }
}

// GLOBAL: SWBF2 0x01f9a8e0
// SYNTHETIC: SWBF2 0x004071c0
// `dynamic initializer for 'WeaponLaser::rttiHashWeaponLaser''
PblHash WeaponLaser::rttiHashWeaponLaser("WeaponLaser");
char WeaponLaser::rttiNameWeaponLaser[] = "WeaponLaser";

// FUNCTION: SWBF2 0x00686a10
bool WeaponLaser::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponLaser || Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00686a40
unsigned int WeaponLaser::GetDerivedRtti() const
{
    return rttiHashWeaponLaser;
}

// FUNCTION: SWBF2 0x00686a50
const char* WeaponLaser::GetDerivedRttiName() const
{
    return rttiNameWeaponLaser;
}

// FUNCTION: SWBF2 0x00686ae0
WeaponLaser::WeaponLaser(const WeaponLaserClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    mSalvoTimer = 0.0f;
}

// FUNCTION: SWBF2 0x00686b10
WeaponLaser::~WeaponLaser()
{
}

// SYNTHETIC: SWBF2 0x00686a60
// WeaponLaser::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00686b20
bool WeaponLaser::IsBusy() const
{
    if (mState != FIRE && mState != FIRE2)
    {
        return false;
    }
    return mSalvoTimer - GameLoop::sClientDeltaTime > 0.0f ||
           mStateLimit > mStateTimer + GameLoop::sClientDeltaTime;
}

// FUNCTION: SWBF2 0x00686b70
void WeaponLaser::EnterFire()
{
    Weapon::EnterFire();
    const WeaponLaserClass* pClass = static_cast<const WeaponLaserClass*>(mClass);
    mStateTimer = 0.0f;
    mStateLimit = pClass->mShotDelay;
    mSalvoTimer = pClass->mSalvoTime;
    if (pClass->mTriggerSingle)
    {
        mTrigger->ClearUntilPressed();
    }
}

// FUNCTION: SWBF2 0x00686bc0
bool WeaponLaser::UpdateFire(float fDeltaTime)
{
    float fRemaining = fDeltaTime;
    if (!mSelectedFlag)
    {
        mSalvoTimer = 0.0f;
    }
    while (fRemaining > 0.0f)
    {
        float fStep = mSalvoTimer;
        if (fStep > 0.0f)
        {
            if (fStep > fRemaining)
            {
                fStep = fRemaining;
            }
            Aimer* pFirst = mAimer;
            do
            {
                if (mAimer->WeaponShouldFire())
                {
                    Fire(fStep);
                    const float fShare = fStep / static_cast<const WeaponLaserClass*>(mClass)->mSalvoTime;
                    m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot * fShare);
                    m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot * fShare);
                }
            } while (!mAimer->UpdateFire(mAimer, true) || mAimer != pFirst);
            fRemaining -= fStep;
            mSalvoTimer -= fStep;
        }
        else
        {
            const float fLimit = mStateLimit;
            float fWait = fLimit - mStateTimer;
            if (fWait > fRemaining)
            {
                fWait = fRemaining;
            }
            fRemaining -= fWait;
            mStateTimer += fWait;
            if (mStateTimer < fLimit || mClass->mTriggerSingle || !CheckFire() || CheckReload() || CheckEmpty())
            {
                mSoundControl.Stop(true);
                break;
            }
            mStateTimer -= mStateLimit;
            if (static_cast<const WeaponLaserClass*>(mClass)->mShotDelay > 0.0f)
            {
                Weapon::ExitFire();
                Weapon::EnterFire();
            }
            mSalvoTimer = static_cast<const WeaponLaserClass*>(mClass)->mSalvoTime;
        }
    }
    return Weapon::UpdateFire(fDeltaTime);
}

// FUNCTION: SWBF2 0x00686de0
void WeaponLaser::ExitFire()
{
    Weapon::ExitFire();
}

// FUNCTION: SWBF2 0x00686df0
bool WeaponLaser::CheckFire() const
{
    if (!mTrigger || CheckOverheat())
    {
        return false;
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x00686e10
bool WeaponLaser::FireDone() const
{
    return mSalvoTimer <= 0.0f && mStateTimer >= mStateLimit &&
           (mClass->mTriggerSingle || !CheckFire() || CheckReload() || CheckEmpty());
}

// FUNCTION: SWBF2 0x00686e80
bool WeaponLaser::Fire(float fTime)
{
    OrdnanceDesc desc;
    desc.mPosition = mAimer->mFirePos;
    desc.mDirection = mAimer->mDirection;
    desc.mStrength = fTime / static_cast<const WeaponLaserClass*>(mClass)->mSalvoTime;
    GameObject* pOwnerObject = mOwner->GetGameObject();
    desc.mOwner = pOwnerObject;
    desc.mDamageOwner = DamageOwner(mOwner->mCharacter, mOwner->GetGameObject(), mOwner->GetGameObject()->GetTeam(),
                                    GetEntityClass(mOwner->GetGameObject()), mClass, mOwner->GetOffenseMultiplier());
    desc.mTarget = const_cast<GameObject*>(mTarget.GetValidObject());
    desc.mBodyID = mTargetBodyID;

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
    CombatUtil::SendDamageEvent(mOwner, mTarget.GetValidObject(), mTrigger);
    return true;
}

// FUNCTION: SWBF2 0x006870a0
void WeaponLaser::EnterEmpty()
{
    Weapon::EnterEmpty();
    mStateLimit = static_cast<const WeaponLaserClass*>(mClass)->mShotDelay;
}

// GLOBAL: SWBF2 0x01f9a8dc
// SYNTHETIC: SWBF2 0x004071d0
// `dynamic initializer for 'WeaponLaserClass::rttiHashWeaponLaserClass''
PblHash WeaponLaserClass::rttiHashWeaponLaserClass("WeaponLaserClass");
char WeaponLaserClass::rttiNameWeaponLaserClass[] = "WeaponLaserClass";

// FUNCTION: SWBF2 0x006870c0
WeaponLaserClass::WeaponLaserClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    mShotDelay = 1.0f;
    mSalvoTime = 0.5f;
}

// FUNCTION: SWBF2 0x00686a90
bool WeaponLaserClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponLaserClass || WeaponClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00686ac0
unsigned int WeaponLaserClass::GetDerivedRtti()
{
    return rttiHashWeaponLaserClass;
}

// FUNCTION: SWBF2 0x00686ad0
const char* WeaponLaserClass::GetDerivedRttiName()
{
    return rttiNameWeaponLaserClass;
}

// SYNTHETIC: SWBF2 0x006870f0
// WeaponLaserClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00687120
WeaponLaserClass::WeaponLaserClass(WeaponLaserClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    mShotDelay = pBase->mShotDelay;
    mSalvoTime = pBase->mSalvoTime;
}

// FUNCTION: SWBF2 0x00687160
WeaponLaserClass::~WeaponLaserClass()
{
}

// FUNCTION: SWBF2 0x00687170
void WeaponLaserClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0xD9356908)
    {
        sscanf(pszValue, "%f", &mShotDelay);
    }
    else if (uProperty == 0x4225631D)
    {
        sscanf(pszValue, "%f", &mSalvoTime);
    }
    else
    {
        WeaponClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x006871c0
WeaponClass* WeaponLaserClass::Derive(unsigned int uHash)
{
    return new WeaponLaserClass(this, uHash);
}

// FUNCTION: SWBF2 0x00687230
Weapon* WeaponLaserClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    void* pMemory = Weapon::sMemoryPool.Allocate(0x120);
    return pMemory ? reinterpret_cast<Weapon*>(::new (pMemory) WeaponLaser(this, desc)) : 0;
}
