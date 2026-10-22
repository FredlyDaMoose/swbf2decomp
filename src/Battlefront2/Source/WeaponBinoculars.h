#pragma once

#include "Weapon.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x007b0430
class WeaponBinocularsClass : public WeaponClass
{
public:
    WeaponBinocularsClass(unsigned int uHash);
    WeaponBinocularsClass(WeaponBinocularsClass* pBase, unsigned int uHash);
    virtual ~WeaponBinocularsClass();

    static PblHash rttiHashWeaponBinocularsClass;
    static char rttiNameWeaponBinocularsClass[];

    virtual WeaponClass* Derive(unsigned int uHash);
    virtual Weapon* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
};
  

#include "WeaponCannonInstance.h"

// VTABLE: SWBF2 0x007b0468
class WeaponBinoculars : public Weapon
{
public:
    WeaponBinoculars(const WeaponBinocularsClass* pClass, const WeaponDesc& desc);
    virtual ~WeaponBinoculars();
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    static PblHash rttiHashWeaponBinoculars;
    static char rttiNameWeaponBinoculars[];
};
  
