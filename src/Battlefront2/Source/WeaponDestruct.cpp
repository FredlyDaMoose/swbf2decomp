#include <stdio.h>
#include <new>

#include "WeaponDestruct.h"
#include "Explosion.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "EntityDroid.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Ordnance.h"

#include "Trigger.h"

extern bool netIsLocalTurn;

class AmmoCounter
{
public:
    void ExpendRounds(float fRounds);
};

// GLOBAL: SWBF2 0x01f9a8a4
// SYNTHETIC: SWBF2 0x004070e0
// `dynamic initializer for 'WeaponDestruct::rttiHashWeaponDestruct''
PblHash WeaponDestruct::rttiHashWeaponDestruct("WeaponDestruct");
char WeaponDestruct::rttiNameWeaponDestruct[] = "WeaponDestruct";

// FUNCTION: SWBF2 0x00681a10
bool WeaponDestruct::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponDestruct || Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00681a40
unsigned int WeaponDestruct::GetDerivedRtti() const
{
    return rttiHashWeaponDestruct;
}

// FUNCTION: SWBF2 0x00681a50
const char* WeaponDestruct::GetDerivedRttiName() const
{
    return rttiNameWeaponDestruct;
}

// GLOBAL: SWBF2 0x01f9a8a8
// SYNTHETIC: SWBF2 0x004070f0
// `dynamic initializer for 'WeaponDestructClass::rttiHashWeaponDestructClass''
PblHash WeaponDestructClass::rttiHashWeaponDestructClass("WeaponDestructClass");
char WeaponDestructClass::rttiNameWeaponDestructClass[] = "WeaponDestructClass";

// FUNCTION: SWBF2 0x00682190
WeaponDestructClass::WeaponDestructClass(unsigned int uHash)
    : WeaponClass(uHash), OrbitalStrikeClass(uHash)
{
    mTargetSides = 0;
    mTargetTypes = 0;
    mAITargetTypes = 0;

    mMaxPressedTime = 0.0f;
    mExplosionClass = 0;
}

// FUNCTION: SWBF2 0x00682220
WeaponDestructClass::WeaponDestructClass(WeaponDestructClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash), OrbitalStrikeClass(pBase, uHash)
{
    mMaxPressedTime = pBase->mMaxPressedTime;
    mExplosionClass = pBase->mExplosionClass;
}

// FUNCTION: SWBF2 0x00682280
WeaponDestructClass::~WeaponDestructClass()
{
}

// SYNTHETIC: SWBF2 0x006821f0
// WeaponDestructClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00681a60
bool WeaponDestructClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponDestructClass || WeaponClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00681a90
unsigned int WeaponDestructClass::GetDerivedRtti()
{
    return rttiHashWeaponDestructClass;
}

// FUNCTION: SWBF2 0x00681aa0
const char* WeaponDestructClass::GetDerivedRttiName()
{
    return rttiNameWeaponDestructClass;
}

// FUNCTION: SWBF2 0x00682290
void WeaponDestructClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x2819FFA0)
    {
        sscanf(pszValue, "%f", &mMaxPressedTime);
    }
    else if (uProperty == 0x4C5AA5AA)
    {
        sscanf(pszValue, "%f", &mChargeSoundPitch);
    }
    else if (uProperty == 0xCA0C1378)
    {
        mExplosionClass = ExplosionClass::Find(PblHash(pszValue));
    }
    else if (!OrbitalStrikeClass::SetProperty(uProperty, pszValue))
    {
        WeaponClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x00682320
WeaponClass* WeaponDestructClass::Derive(unsigned int uHash)
{
    return new WeaponDestructClass(this, uHash);
}

// FUNCTION: SWBF2 0x00682390
Weapon* WeaponDestructClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponDestruct(this, desc);
}

// FUNCTION: SWBF2 0x00681ab0
WeaponDestruct::WeaponDestruct(const WeaponDestructClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
}

// SYNTHETIC: SWBF2 0x00681af0
// WeaponDestruct::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00681b20
WeaponDestruct::~WeaponDestruct()
{
}

// FUNCTION: SWBF2 0x006820f0
bool WeaponDestruct::CheckCharge() const
{
    if (mState == IDLE)
    {
        return mTrigger && mTrigger->IsPressed();
    }
    return mStateLimit > mStateTimer;
}

// FUNCTION: SWBF2 0x00682130
bool WeaponDestruct::CheckFire() const
{
    return mState == CHARGE && mStateTimer >= mStateLimit;
}

// FUNCTION: SWBF2 0x00681b30
bool WeaponDestruct::IsBusy() const
{
    return (mState == FIRE || mState == FIRE2) && mStrike.operator OrbitalStrike*() != 0;
}

// FUNCTION: SWBF2 0x00682150
bool WeaponDestruct::FireDone() const
{
    return mStrike.operator OrbitalStrike*() == 0;
}

// FUNCTION: SWBF2 0x00681b70
void WeaponDestruct::EnterCharge()
{
    Weapon::EnterCharge();
    mStateTimer = 0.0f;
    mStateLimit = ((WeaponDestructClass*)mClass)->mMaxPressedTime;
    m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot);
    m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot);
    GameObject* pObject = mOwner->GetGameObject();
    if (pObject && pObject->mAddHealth < 0.0f)
    {
        pObject->mAddHealth = 0.0f;
    }
}

// FUNCTION: SWBF2 0x00681c00
bool WeaponDestruct::UpdateCharge(float fDeltaTime)
{
    GameObject* pObject = mOwner->GetGameObject();
    if (pObject->IsRtti(EntityDroid::rttiHashEntityDroid))
    {
        ((EntityDroid*)pObject)->FreezeMovement();
    }
    mStateTimer += fDeltaTime;
    return Weapon::UpdateCharge(fDeltaTime);
}

// FUNCTION: SWBF2 0x00681c60
void WeaponDestruct::EnterFire()
{
    Weapon::ExitCharge();
    GameObject* pObject = mOwner->GetGameObject();
    WeaponDestructClass* pClass = (WeaponDestructClass*)mClass;
    const PblVector3& vPos = pObject->GetCollisionObject()->GetCollisionCenter();

    if (mClass->mOrdnanceClass)
    {
        OrdnanceDesc desc;
        desc.mPosition = vPos;
        desc.mOwner = pObject;
        desc.mDamageOwner.mCharacter = mOwner->mCharacter;
        desc.mDamageOwner.mGameObject = pObject;
        desc.mDamageOwner.mTeam = pObject->GetTeam();
        desc.mDamageOwner.mClass = pObject->GetEntityClass();
        desc.mDamageOwner.mWeapon = mClass;
        desc.mDamageOwner.mDamageMultiplier = mOwner->GetOffenseMultiplier();
        desc.mTarget = IsLocked() ? (GameObject*)GetLocked() : 0;
        desc.mBodyID = IsLocked() ? GetLockedTargetBodyId() : -1;
        desc.mFireWeapon = this;

        bool bBuild = true;
        bool bFlash = true;
        if (NetGame::IsEnabled())
        {
            if (NetGame::IsClient())
            {
                bBuild = netIsLocalTurn && NetGame::GetJoystickIndex(mOwner->mPlayerId) >= 0;
            }
            else
            {
                bFlash = netIsOneWorld && !netIsDedicated;
                NetGame::CreateEvent_CreateOrdnance(mClass->mOrdnanceClass, desc);
            }
        }
        if (bBuild)
        {
            if (bFlash)
            {
                mClass->CreateMuzzleFlash(pObject->GetCollisionObject()->GetMatrix());
            }
            mClass->mOrdnanceClass->Build(desc);
        }
    }

    if (pClass->mStrikeClass)
    {
        OrbitalStrikeDesc desc;
        desc.mPosition = vPos;
        desc.mDamageOwner = DamageOwner(mOwner->mCharacter, pObject, pObject->GetTeam(),
                                       pObject->GetEntityClass(), mClass,
                                       mOwner->GetOffenseMultiplier());
        mStrike = new OrbitalStrike(pClass, desc);
    }
}

// FUNCTION: SWBF2 0x00681fc0
void WeaponDestruct::ExitFire()
{
    Weapon::ExitFire();
    GameObject* pObject = mOwner->GetGameObject();
    DamageOwner owner(mOwner->mCharacter, pObject, pObject->GetTeam(),
                      pObject->GetEntityClass(), mClass, 1.0f);
    WeaponDestructClass* pClass = (WeaponDestructClass*)mClass;
    if (pClass->mExplosionClass)
    {
        CollisionObject* pCollision = pObject->GetCollisionObject();
        pClass->mExplosionClass->CreateExplosion(
            pCollision->GetCollisionCenter(), owner, false, COLL_UNKNOWN);
    }
    if (pObject->IsRtti(EntityDroid::rttiHashEntityDroid))
    {
        ((EntityDroid*)pObject)->KillDelay(1.0f);
    }
    else
    {
        pObject->InstantDeath(owner, false);
    }
}
