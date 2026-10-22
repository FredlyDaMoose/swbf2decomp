#pragma once

#include <stddef.h>

#include "Weapon.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x007b1158
class WeaponLaserClass : public WeaponClass
{
public:
    WeaponLaserClass(unsigned int uHash);
    WeaponLaserClass(WeaponLaserClass* pBase, unsigned int uHash);
    virtual ~WeaponLaserClass();

    static PblHash rttiHashWeaponLaserClass;
    static char rttiNameWeaponLaserClass[];

    virtual WeaponClass* Derive(unsigned int uHash);
    virtual Weapon* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    float mShotDelay;
    float mSalvoTime;
};
  
 
 

#include "WeaponCannonInstance.h"

// VTABLE: SWBF2 0x007b1190
class WeaponLaser : public Weapon
{
public:
    WeaponLaser(const WeaponLaserClass* pClass, const WeaponDesc& desc);
    virtual ~WeaponLaser();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool IsBusy() const;
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

protected:
    virtual void EnterFire();
    virtual bool UpdateFire(float fDeltaTime);
    virtual void ExitFire();
    virtual void EnterEmpty();
    virtual bool CheckFire() const;
    virtual bool FireDone() const;

    bool Fire(float fTime);

public:
    static PblHash rttiHashWeaponLaser;
    static char rttiNameWeaponLaser[];

    float mSalvoTimer;
};
  
 
