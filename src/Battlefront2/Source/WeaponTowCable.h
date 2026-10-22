#pragma once

#include <stddef.h>

#include "GameSound.h"
#include "WeaponCannon.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblHashTable.h"

class WeaponTowCableClass;

// VTABLE: SWBF2 0x007b1c58
class WeaponTowCable : public WeaponCannon
{
public:
    WeaponTowCable(const WeaponTowCableClass* pClass, const WeaponDesc& desc);
    virtual ~WeaponTowCable();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

protected:
    virtual bool CheckFire() const;

public:
    virtual void SetOrdnance(Ordnance* pOrdnance);
    virtual Ordnance* CreateOrdnance(OrdnanceDesc& desc);

    bool HasOrdnance() const;
    void KillMyCable();

    static PblHash rttiHashWeaponTowCable;
    static char rttiNameWeaponTowCable[];

    Ordnance* mMyCable;
    bool mCableOut;
};
  
 
 

// VTABLE: SWBF2 0x007b1c20
class WeaponTowCableClass : public WeaponCannonClass
{
public:
    WeaponTowCableClass(unsigned int uHash);

    WeaponTowCableClass(WeaponTowCableClass* pBase, unsigned int uHash);
    virtual ~WeaponTowCableClass();

    static PblHash rttiHashWeaponTowCableClass;
    static char rttiNameWeaponTowCableClass[];

    // FUNCTION: SWBF2 0x005b4240
    static unsigned int GetRtti() { return rttiHashWeaponTowCableClass; }

    virtual WeaponClass* Derive(unsigned int uHash);

    virtual WeaponTowCable* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    GameSound mWhir;
    GameSound mDetatch;
};
  
 
 
