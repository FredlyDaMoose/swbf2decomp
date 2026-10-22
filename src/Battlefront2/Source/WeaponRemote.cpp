#include <stdlib.h>

#include "WeaponRemote.h"
#include "Controllable.h"
#include "Damageable.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Trigger.h"

// GLOBAL: SWBF2 0x01f9a904
// SYNTHETIC: SWBF2 0x00407270
// `dynamic initializer for 'WeaponRemoteClass::rttiHashWeaponRemoteClass''
PblHash WeaponRemoteClass::rttiHashWeaponRemoteClass("WeaponRemoteClass");
char WeaponRemoteClass::rttiNameWeaponRemoteClass[] = "WeaponRemoteClass";

// GLOBAL: SWBF2 0x01f9a908
// SYNTHETIC: SWBF2 0x00407260
// `dynamic initializer for 'WeaponRemote::rttiHashWeaponRemote''
PblHash WeaponRemote::rttiHashWeaponRemote("WeaponRemote");
char WeaponRemote::rttiNameWeaponRemote[] = "WeaponRemote";

// FUNCTION: SWBF2 0x0068ff10
bool WeaponRemoteClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponRemoteClass || WeaponDispenserClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0068ff40
unsigned int WeaponRemoteClass::GetDerivedRtti()
{
    return rttiHashWeaponRemoteClass;
}

// FUNCTION: SWBF2 0x0068ff50
const char* WeaponRemoteClass::GetDerivedRttiName()
{
    return rttiNameWeaponRemoteClass;
}

// FUNCTION: SWBF2 0x006903b0
WeaponRemoteClass::WeaponRemoteClass(unsigned int uHash)
    : WeaponDispenserClass(uHash)
{
    mSwitchImmediately = false;
    mAllowDestruct = true;
}

// SYNTHETIC: SWBF2 0x006903e0
// WeaponRemoteClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00690410
WeaponRemoteClass::WeaponRemoteClass(WeaponRemoteClass* pBase, unsigned int uHash)
    : WeaponDispenserClass(pBase, uHash)
{
    mSwitchImmediately = pBase->mSwitchImmediately;
    mAllowDestruct = pBase->mAllowDestruct;
}

// FUNCTION: SWBF2 0x00690450
WeaponRemoteClass::~WeaponRemoteClass()
{
}

// FUNCTION: SWBF2 0x00690460
void WeaponRemoteClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0xC43B77DD)
    {
        mSwitchImmediately = atoi(pszValue) != 0;
    }
    else if (uProperty == 0xED422F60)
    {
        mAllowDestruct = atoi(pszValue) != 0;
    }
    else
    {
        WeaponDispenserClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x006904c0
WeaponDispenserClass* WeaponRemoteClass::Derive(unsigned int uHash)
{
    return new WeaponRemoteClass(this, uHash);
}

// FUNCTION: SWBF2 0x00690530
WeaponDispenser* WeaponRemoteClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponRemote(this, desc);
}

// FUNCTION: SWBF2 0x0068fec0
bool WeaponRemote::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponRemote || WeaponDispenser::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0068fef0
unsigned int WeaponRemote::GetDerivedRtti() const
{
    return rttiHashWeaponRemote;
}

// FUNCTION: SWBF2 0x0068ff00
const char* WeaponRemote::GetDerivedRttiName() const
{
    return "WeaponRemote";
}

// FUNCTION: SWBF2 0x0068ff60
WeaponRemote::WeaponRemote(const WeaponRemoteClass* pClass, const WeaponDesc& desc)
    : WeaponDispenser(pClass, desc)
{
}

// FUNCTION: SWBF2 0x006900b0
bool WeaponRemote::Update(float fDeltaTime)
{
    if (mSelectedFlag && mItem && !(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        Controllable* pOwner = mOwner;
        if (mTrigger->IsPressed() && pOwner->mPilot)
        {
            if (pOwner->mPlayerId >= 0)
            {
                pOwner->mPilot->EnterRemote(mItem);
            }
        }
        else if (mReload && mReload->IsPressed() && ((const WeaponRemoteClass*)mClass)->mAllowDestruct)
        {
            DamageOwner owner(pOwner->mCharacter, pOwner->GetGameObject(), mItem->GetTeam(),
                              mItem->GetEntityClass(), mClass, 1.0f);
            mItem->InstantDeath(owner, false);
        }
    }
    return WeaponDispenser::Update(fDeltaTime);
}

// SYNTHETIC: SWBF2 0x0068ffa0
// WeaponRemote::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0068ffd0
WeaponRemote::~WeaponRemote()
{
    if (mItem)
    {
        mItem->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), false);
    }
}

// FUNCTION: SWBF2 0x00690220
bool WeaponRemote::IsBusy() const
{
    return WeaponDispenser::IsBusy();
}

// FUNCTION: SWBF2 0x00690230
bool WeaponRemote::CheckCharge() const
{
    if (mItem)
    {
        return false;
    }
    return WeaponDispenser::CheckCharge();
}

// FUNCTION: SWBF2 0x00690270
bool WeaponRemote::CheckFire() const
{
    if (mItem)
    {
        return false;
    }
    return WeaponDispenser::CheckFire();
}

// FUNCTION: SWBF2 0x006902b0
bool WeaponRemote::FireDone() const
{
    return mFired && mStateTimer >= mStateLimit;
}

// FUNCTION: SWBF2 0x006902d0
Entity* WeaponRemote::Fire()
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return 0;
    }

    Entity* pEntity = WeaponDispenser::Fire();
    if (pEntity)
    {
        if (GameObject* pObject = pEntity->GetGameObject())
        {
            if (!mItem)
            {
                mItem = pObject;
            }
            if (static_cast<const WeaponRemoteClass*>(mClass)->mSwitchImmediately && mItem)
            {
                if (Controllable* pPilot = mOwner->mPilot)
                {
                    pPilot->EnterRemote(mItem);
                }
            }
        }
    }
    return pEntity;
}
