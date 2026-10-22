#include "WeaponTowCable.h"
#include "Controllable.h"
#include "NetGame.h"
#include "OrdnanceTowCable.h"
#include "CollisionManager.h"
#include "EntityCarrier.h"
#include "GameObject.h"

namespace
{

    const PblMatrix& GetCollisionMatrix(GameObject* pObject)
    {
        typedef const PblMatrix& (__thiscall * Fn)(void*);
        return static_cast<CollisionObject*>(pObject)->GetMatrix();
    }

    const PblVector3& GetVelocity(GameObject* pObject)
    {
        typedef const PblVector3& (__thiscall * Fn)(GameObject*);
        return pObject->GetVelocity();
    }

    const PblVector3& GetTranslation(const PblMatrix& m)
    {
        return *reinterpret_cast<const PblVector3*>(m.m[3]);
    }
}

// GLOBAL: SWBF2 0x01f9a91c
// SYNTHETIC: SWBF2 0x004072c0
// `dynamic initializer for 'WeaponTowCable::rttiHashWeaponTowCable''
PblHash WeaponTowCable::rttiHashWeaponTowCable("WeaponTowCable");
char WeaponTowCable::rttiNameWeaponTowCable[] = "WeaponTowCable";

// GLOBAL: SWBF2 0x01f9a920
// SYNTHETIC: SWBF2 0x004072d0
// `dynamic initializer for 'WeaponTowCableClass::rttiHashWeaponTowCableClass''
PblHash WeaponTowCableClass::rttiHashWeaponTowCableClass("WeaponTowCableClass");
char WeaponTowCableClass::rttiNameWeaponTowCableClass[] = "WeaponTowCableClass";

// FUNCTION: SWBF2 0x00692a30
WeaponTowCableClass::WeaponTowCableClass(unsigned int uHash)
    : WeaponCannonClass(uHash)
{
}

// FUNCTION: SWBF2 0x00692aa0
WeaponTowCableClass::WeaponTowCableClass(WeaponTowCableClass* pBase, unsigned int uHash)
    : WeaponCannonClass(pBase, uHash)
{
    mWhir = pBase->mWhir;
    mDetatch = pBase->mDetatch;
}

// FUNCTION: SWBF2 0x00692b30
WeaponTowCableClass::~WeaponTowCableClass()
{
}

// SYNTHETIC: SWBF2 0x00692a70
// WeaponTowCableClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006927b0
bool WeaponTowCableClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponTowCableClass || WeaponCannonClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x006927e0
unsigned int WeaponTowCableClass::GetDerivedRtti()
{
    return rttiHashWeaponTowCableClass;
}

// FUNCTION: SWBF2 0x006927f0
const char* WeaponTowCableClass::GetDerivedRttiName()
{
    return rttiNameWeaponTowCableClass;
}

// FUNCTION: SWBF2 0x00692b40
void WeaponTowCableClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x5EDE4703)
    {
        mDetatch.SetID(pszValue, false, false);
    }
    else if (uProperty == 0x96A1687E)
    {
        mWhir.SetID(pszValue, false, false);
    }
    else
    {
        WeaponCannonClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x00692b90
WeaponClass* WeaponTowCableClass::Derive(unsigned int uHash)
{
    return new WeaponTowCableClass(this, uHash);
}

// FUNCTION: SWBF2 0x00692c00
WeaponTowCable* WeaponTowCableClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponTowCable(this, desc);
}

// FUNCTION: SWBF2 0x00692800
WeaponTowCable::WeaponTowCable(const WeaponTowCableClass* pClass, const WeaponDesc& desc)
    : WeaponCannon(pClass, desc)
{
    mCableOut = false;
    mMyCable = 0;
}

// SYNTHETIC: SWBF2 0x00692780
// WeaponTowCable::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00692830
WeaponTowCable::~WeaponTowCable()
{
}

// FUNCTION: SWBF2 0x00692730
bool WeaponTowCable::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponTowCable || WeaponCannon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x006929b0
bool WeaponTowCable::CheckFire() const
{
    if (mCableOut)
    {
        return false;
    }
    return WeaponCannon::CheckFire();
}

// FUNCTION: SWBF2 0x00692760
unsigned int WeaponTowCable::GetDerivedRtti() const
{
    return rttiHashWeaponTowCable;
}

// FUNCTION: SWBF2 0x00692770
const char* WeaponTowCable::GetDerivedRttiName() const
{
    return rttiNameWeaponTowCable;
}

// FUNCTION: SWBF2 0x00692840
void WeaponTowCable::SetOrdnance(Ordnance* pOrdnance)
{
    if (pOrdnance ? mCableOut : !mCableOut)
    {
        return;
    }

    GameObject* pObject = mOwner->GetGameObject();
    const WeaponTowCableClass* pClass = static_cast<const WeaponTowCableClass*>(mClass);
    mSoundControl.Stop(true);
    mSoundProps = mCableOut ? pClass->mDetatch : pClass->mWhir;
    mSoundControl.Play(&mSoundProps, &GetTranslation(GetCollisionMatrix(pObject)), &GetVelocity(pObject), true);
    mMyCable = pOrdnance;
    mCableOut = pOrdnance != 0;
}

// FUNCTION: SWBF2 0x00692900
bool WeaponTowCable::HasOrdnance() const
{
    return mMyCable != 0;
}

// FUNCTION: SWBF2 0x00692910
void WeaponTowCable::KillMyCable()
{
    if (mMyCable && NetGame::IsEnabled() && NetGame::IsClient())
    {
        CollisionManager::RemoveOrdnance(mMyCable);
        if (mMyCable->mOwner)
        {
            ((EntityFlyer*)mMyCable->GetOwner()->GetEntity())->SetTowCableWalker(0);
        }
        mMyCable->KillOrdnance();
        SetOrdnance(0);
    }
}

// FUNCTION: SWBF2 0x006929d0
Ordnance* WeaponTowCable::CreateOrdnance(OrdnanceDesc& desc)
{
    Ordnance* pOrdnance = WeaponCannon::CreateOrdnance(desc);
    if (pOrdnance)
    {
        static_cast<OrdnanceTowCable*>(pOrdnance)->SetFakeCable(NetGame::IsEnabled() && NetGame::IsClient());
    }
    return pOrdnance;
}
