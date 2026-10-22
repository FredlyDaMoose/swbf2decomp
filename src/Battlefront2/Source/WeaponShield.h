#pragma once

#include <stddef.h>

#include "CollisionModel.h"
#include "CollisionObject.h"
#include "GameSound.h"
#include "Weapon.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblHashTable.h"

class CollisionBodyPrimitive;
class FLEffectClass;

// VTABLE: SWBF2 0x007b1be8
class WeaponShieldClass : public WeaponClass
{
public:
    WeaponShieldClass(unsigned int uHash);
    WeaponShieldClass(WeaponShieldClass* pBase, unsigned int uHash);
    virtual ~WeaponShieldClass();

    static PblHash rttiHashWeaponShieldClass;
    static char rttiNameWeaponShieldClass[];

    virtual WeaponClass* Derive(unsigned int uHash);
    virtual Weapon* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void PostReadInit();

    float mMaxShield;
    float mAddShield;
    float mAddShieldOff;
    PblVector3 m_vShieldOffset;
    PblVector3 m_vShieldSize;
    CollisionBodyPrimitive* mCollisionBody;
    CollisionModel mCollisionModel;
    FLEffectClass* mShieldEffect;
    GameSound mSoundShield;
    GameSound mSoundShieldOff;
};
  
 
 
 
 
 
 
 
 
 
 

class FLEffectObject;
class CollisionResult;
class Restrictor;

// VTABLE: SWBF2 0x007b1a9c
// VTABLE: SWBF2 0x007b1b94 CollisionObject
class WeaponShield : public Weapon, public CollisionObject
{
public:
    WeaponShield(const WeaponShieldClass* pClass, const WeaponDesc& desc);
    virtual ~WeaponShield();
    virtual bool Update(float fDeltaTime);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool IsBusy() const;

protected:
    virtual void EnterFire();
    virtual bool UpdateFire(float fDeltaTime);
    virtual void ExitFire();
    virtual bool CheckFire() const;
    virtual bool FireDone() const;

public:

    virtual const CollisionObject* GetCollisionObject() const;
    virtual CollisionObject* GetCollisionObject();

    virtual const PblVector3& GetPosition() const;
    virtual bool CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor);
    virtual const PblMatrix& GetMatrix() const;

    static PblHash rttiHashWeaponShield;

    static unsigned int GetRtti() { return rttiHashWeaponShield; }
    static char rttiNameWeaponShield[];

    FLEffectObject* mShieldEffect;
};
 
 
