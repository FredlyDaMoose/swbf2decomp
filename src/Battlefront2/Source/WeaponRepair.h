#pragma once

#include <stddef.h>

#include "Weapon.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x007b1950
class WeaponRepairClass : public WeaponClass
{
public:
    WeaponRepairClass(unsigned int uHash);
    WeaponRepairClass(WeaponRepairClass* pBase, unsigned int uHash);
    virtual ~WeaponRepairClass();

    static PblHash rttiHashWeaponRepairClass;
    static char rttiNameWeaponRepairClass[];

    virtual WeaponClass* Derive(unsigned int uHash);
    virtual Weapon* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    float mShotDelay;
    float mAddHealth[8];
    float mAddAmmo[8];
    float mAddBuild;
    float mAddRebuild;
};
  
 
 
 
 
 

// VTABLE: SWBF2 0x007b1988
class WeaponRepair : public Weapon
{
public:
    WeaponRepair(const WeaponRepairClass* pClass, const WeaponDesc& desc);

    virtual ~WeaponRepair();
    bool Update(float fDeltaTime);
    bool IsRtti(unsigned int uRtti) const;
    unsigned int GetDerivedRtti() const;
    const char* GetDerivedRttiName() const;
    bool IsLocked() const;
    void Select(unsigned int uFlags, bool bOffhand);
    void Deselect(unsigned int uFlags, bool bOffhand);
    bool IsBusy() const;

protected:
    void EnterFire();
    bool UpdateFire(float fDeltaTime);
    void ExitFire();
    bool CheckFire() const;
    bool FireDone() const;
    bool Fire(float fDeltaTime);

public:

    static PblHash rttiHashWeaponRepair;
    static char rttiNameWeaponRepair[];

    bool mFired;

    char mUnmodeledTail[0x120 - 0x111];
};
 
 
 
 
 
