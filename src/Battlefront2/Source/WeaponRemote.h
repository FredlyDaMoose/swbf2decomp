#pragma once

#include <stddef.h>

#include "WeaponDispenser.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x007b1800
class WeaponRemoteClass : public WeaponDispenserClass
{
public:
    WeaponRemoteClass(unsigned int uHash);

    WeaponRemoteClass(WeaponRemoteClass* pBase, unsigned int uHash);

    virtual ~WeaponRemoteClass();

    static PblHash rttiHashWeaponRemoteClass;
    static char rttiNameWeaponRemoteClass[];

    virtual WeaponDispenserClass* Derive(unsigned int uHash);
    virtual WeaponDispenser* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    bool mSwitchImmediately;
    bool mAllowDestruct;
};
  
 
 

class WeaponRemote : public WeaponDispenser
{
public:
    WeaponRemote(const WeaponRemoteClass* pClass, const WeaponDesc& desc);

    virtual bool Update(float fDeltaTime);
    virtual bool IsRtti(unsigned int uRtti) const;

    virtual ~WeaponRemote();
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool IsBusy() const;
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

protected:

    virtual bool CheckCharge() const;
    virtual bool CheckFire() const;
    virtual bool FireDone() const;
    virtual Entity* Fire();

public:

    static PblHash rttiHashWeaponRemote;
    static char rttiNameWeaponRemote[];

    PblHandle<GameObject> mItem;
};
 
 
