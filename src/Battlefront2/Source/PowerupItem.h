#pragma once

#include <stddef.h>

#include "EntityItem.h"

// VTABLE: SWBF2 0x007adae4
class PowerupItemClass : public EntityItemClass
{
public:
    PowerupItemClass(unsigned int uHash);
    PowerupItemClass(PowerupItemClass* pBase, unsigned int uHash);
    virtual ~PowerupItemClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    float mAddHealth[8];
    float mAddAmmo[8];
    float mAddEnergy[8];
    float mBuffOffenseTimer;
    float mBuffOffenseMult;
    float mBuffDefenseTimer;
    float mBuffDefenseMult;
    float mBuffHealthTimer;
    float mBuffHealthRate;
    float mDebuffDamageTimer;
    float mDebuffDamageRate;
    GameSound mPowerupSound;
    GameSound mAmbientSound;

    static PblHash rttiHashPowerupItemClass;
    static char rttiNamePowerupItemClass[];
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 

#include "GameSound.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

// VTABLE: SWBF2 0x007ad9bc
// VTABLE: SWBF2 0x007ad8d4 CollisionObject
// VTABLE: SWBF2 0x007ad928 RedSceneObject
// VTABLE: SWBF2 0x007ad990 Damageable
// VTABLE: SWBF2 0x007adadc PblHandled
// VTABLE: SWBF2 0x007adb30 Thread
class PowerupItem : public EntityItem
{
public:
    PowerupItem(PowerupItemClass* pClass, const EntityDesc& desc);
    virtual ~PowerupItem();

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual EntityType GetEntityType() const;
    virtual bool AddToTeamManager() const;
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);

    virtual bool CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor);

    virtual bool Update(float fDeltaTime);

    GameSoundControllable mAmbientSound;

    static unsigned int GetRtti() { return rttiHashPowerupItem; }

    static MemoryPool sMemoryPool;
    static PblHash rttiHashPowerupItem;
    static char rttiNamePowerupItem[];
};
 
 
