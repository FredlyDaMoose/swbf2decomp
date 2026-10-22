#pragma once

#include <stddef.h>

#include "EntityBuildingArmedDynamic.h"
#include "GameSound.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class Character;

// VTABLE: SWBF2 0x0079c66c
class EntityPortableTurretClass : public EntityBuildingArmedDynamicClass
{
public:
    EntityPortableTurretClass(unsigned int uHash);

    EntityPortableTurretClass(EntityPortableTurretClass* pBase, unsigned int uHash);

    virtual ~EntityPortableTurretClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    GameSound mCollisionSound;
    GameSound mCollisionFoliageSound;
    GameSound mCollisionWaterSound;
    float mLifeTime;
    float mGravity;
    float mRebound;
    float mFriction;

    static PblHash rttiHashEntityPortableTurretClass;
    static char rttiNameEntityPortableTurretClass[];
};
  
 
 
 
 
 
 
 

class CollisionResult;
class Restrictor;

class EntityPortableTurret : public EntityBuildingArmedDynamic
{
public:

    EntityPortableTurret(EntityPortableTurretClass* pClass, const EntityDesc& desc);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual const PblVector3& GetVelocity() const;
    virtual void SetVelocity(const PblVector3& vVelocity);
    virtual void RecalculateAABB();
    virtual bool CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor);
    virtual bool Update(float fDeltaTime);

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    static PblHash rttiHashEntityPortableTurret;
    static char rttiNameEntityPortableTurret[];
    static MemoryPool sMemoryPool;

    EntityPortableTurretClass* mClass;
    float mLifeTimer;
    PblVector3 mVelocity;
    Character* mOwnerChar;
    bool mStationary;
    bool mBecomeStationaryNow;

    char _tailPad[6];
};

 
 
 
 
 
 
 
