#pragma once

#include <stddef.h>

#include "EntityItem.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class Character;
class EntityLight;
class ExplosionClass;
class FLEffectObject;
class StickInfo;

// VTABLE: SWBF2 0x0079c394
class EntityMineClass : public EntityItemClass
{
public:
    EntityMineClass(unsigned int uHash);
    EntityMineClass(EntityMineClass* pBase, unsigned int uHash);
    virtual ~EntityMineClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    GameSound mTick;
    float mTickPitch;
    ExplosionClass* mExplosionTrigger;
    ExplosionClass* mExplosionExpire;
    ExplosionClass* mExplosionDeath;
    bool mTriggerContact;
    float mTriggerRadius;
    float mSuppressRadius;

    static PblHash rttiHashEntityMineClass;
    static char rttiNameEntityMineClass[];
};
  
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x0079c274
class EntityMine : public EntityItem
{
public:
    EntityMine(EntityMineClass* pClass, const EntityDesc& desc);
    virtual ~EntityMine();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void ClientKill();
    virtual EntityType GetEntityType() const;
    virtual void Kill();
    virtual bool Update(float fDeltaTime);
    virtual bool CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor);
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    static PblHash rttiHashEntityMine;
    static char rttiNameEntityMine[];
    static MemoryPool sMemoryPool;

    bool mNetDead;
    int mPrevTimerVal;
    StickInfo* mStickInfo;
    PblVector3 mOldPosition;
    int mHitSurface;
    bool mTrigger;
};
 
 
 
 
 
 
 
 
