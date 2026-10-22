#pragma once

#include <stddef.h>

#include "EntityBuildingArmedDynamic.h"

// VTABLE: SWBF2 0x0079a778
class EntityDefenseGridTurretClass : public EntityBuildingArmedDynamicClass
{
public:
    EntityDefenseGridTurretClass(unsigned int uHash);

    EntityDefenseGridTurretClass(EntityDefenseGridTurretClass* pBase, unsigned int uHash);

    virtual ~EntityDefenseGridTurretClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    float mRespawnTime;

    static PblHash rttiHashEntityDefenseGridTurretClass;

    static unsigned int GetRtti() { return rttiHashEntityDefenseGridTurretClass; }
    static char rttiNameEntityDefenseGridTurretClass[];
};
  
 

class MemoryPool;

// VTABLE: SWBF2 0x0079a544
class EntityDefenseGridTurret : public EntityBuildingArmedDynamic
{
public:

    EntityDefenseGridTurret(EntityDefenseGridTurretClass* pClass, const EntityDesc& desc);
    virtual ~EntityDefenseGridTurret();
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool Update(float fDeltaTime);

    static void* operator new(unsigned int uSize);
    static void operator delete(void* pBlock);

    static MemoryPool sMemoryPool;
    static PblHash rttiHashEntityDefenseGridTurret;
    static char rttiNameEntityDefenseGridTurret[];

    EntityDefenseGridTurretClass* mClass;
    float mRespawnTimer;
};
 
 
 
