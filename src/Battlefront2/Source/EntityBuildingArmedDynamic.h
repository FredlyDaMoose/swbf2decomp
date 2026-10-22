#pragma once

#include <stddef.h>

#include "EntityBuildingArmed.h"
#include "PebbleFL/Common/PblHashTable.h"

class MemoryPool;

// VTABLE: SWBF2 0x0079a03c
class EntityBuildingArmedDynamicClass : public EntityBuildingArmedClass
{
public:
    EntityBuildingArmedDynamicClass(unsigned int uHash);

    EntityBuildingArmedDynamicClass(EntityBuildingArmedDynamicClass* pBase, unsigned int uHash);

    virtual ~EntityBuildingArmedDynamicClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();

    static PblHash rttiHashEntityBuildingArmedDynamicClass;
    static char rttiNameEntityBuildingArmedDynamicClass[];
};
  
 

class EntityBuildingArmedDynamic : public EntityBuildingArmed
{
public:

    EntityBuildingArmedDynamic(EntityBuildingArmedDynamicClass* pClass, const EntityDesc& desc);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    virtual EntityType GetEntityType() const;
    virtual void AddSends(void (*pfnSend)(GameObject*, int), int iArg);
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);

    static void* operator new(unsigned int uSize);
    static void operator delete(void* pBlock);

    static PblHash rttiHashEntityBuildingArmedDynamic;
    static MemoryPool sMemoryPool;
    static char rttiNameEntityBuildingArmedDynamic[];
};
 
