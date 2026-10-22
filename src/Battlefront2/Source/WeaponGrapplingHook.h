#pragma once

#include "WeaponCannon.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x007b0d68
class WeaponGrapplingHookClass : public WeaponCannonClass
{
public:
    WeaponGrapplingHookClass(unsigned int uHash);
    WeaponGrapplingHookClass(WeaponGrapplingHookClass* pBase, unsigned int uHash);
    virtual ~WeaponGrapplingHookClass();

    static PblHash rttiHashWeaponGrapplingHookClass;
    static char rttiNameWeaponGrapplingHookClass[];

    virtual WeaponClass* Derive(unsigned int uHash);
    virtual Weapon* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
};
 class Ordnance;
class NetPktGroup;

// VTABLE: SWBF2 0x007b0da0
class WeaponGrapplingHook : public WeaponCannon
{
public:
    WeaponGrapplingHook(const WeaponGrapplingHookClass* pClass, const WeaponDesc& desc);
    virtual ~WeaponGrapplingHook();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

protected:
    virtual bool CheckFire() const;

public:
    virtual void SetOrdnance(Ordnance* pOrdnance);

    static PblHash rttiHashWeaponGrapplingHook;
    static char rttiNameWeaponGrapplingHook[];

    bool mCableOut;
};
  

 
