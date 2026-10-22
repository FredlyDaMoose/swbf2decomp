#pragma once

#include <stddef.h>

#include "WeaponDispenser.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x007b0a60
class WeaponDetonatorClass : public WeaponDispenserClass
{
public:
    WeaponDetonatorClass(unsigned int uHash);
    WeaponDetonatorClass(WeaponDetonatorClass* pBase, unsigned int uHash);

    virtual WeaponClass* Derive(unsigned int uHash);
    virtual WeaponDispenser* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    unsigned int mPlantedHash;

    static PblHash rttiHashWeaponDetonatorClass;
    static char rttiNameWeaponDetonatorClass[];
};
  
 

class WeaponDetonator : public WeaponDispenser
{
public:
    WeaponDetonator(const WeaponDetonatorClass* pClass, const WeaponDesc& desc);

    virtual bool Update(float fDeltaTime);
    virtual bool IsRtti(unsigned int uRtti) const;

    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);
    virtual unsigned int GetNameHash() const;

    bool HasItemsDispensed() const;

    void NetAttachItem(Entity* pAttach);

    virtual bool CheckCharge() const;
    virtual bool CheckFire() const;

    static PblHash rttiHashWeaponDetonator;
    static char rttiNameWeaponDetonator[];
};
 
