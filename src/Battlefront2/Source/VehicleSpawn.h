#pragma once

#include <stddef.h>

#include "Entity.h"
#include "Thread.h"
#include "CommandPost.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblList.h"
#include "RedEngineFL/Util/RedRegion.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class EntityFlyerClass;

// VTABLE: SWBF2 0x007b0098
class VehicleSpawnClass : public EntityClass
{
public:
    VehicleSpawnClass(unsigned int uHash, bool bIsPad);

    VehicleSpawnClass(VehicleSpawnClass* pParent, unsigned int uHash);

    virtual ~VehicleSpawnClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    bool mIsPad;
};
  
 

// VTABLE: SWBF2 0x007b0060 (the Entity subobject's 13 slots)
// VTABLE: SWBF2 0x007b0048 (the Thread subobject's 5 slots, base offset +4)
class VehicleSpawn : public Entity, public Thread
{
public:

    VehicleSpawn(VehicleSpawnClass* pClass, const EntityDesc& desc);

    virtual ~VehicleSpawn();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual const PblMatrix& GetMatrix() const;

    virtual bool Update(float fDeltaTime);

    void UpdateSpawn(float fDeltaTime);

    class VehicleTracker;

    void UpdateLive(float fDeltaTime, VehicleTracker& tracker);

    class VehicleTracker : public PblList<VehicleTracker>::Node
    {
    public:

        static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
        static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

        static void AddMemoryPoolCount(unsigned int uCount) { sMemoryPool.SetCount(sMemoryPool.GetCount() + uCount); }

        VehicleTracker() : mEmptyTimer(0.0f) {}

        static MemoryPool sMemoryPool;

        PblHandle<GameObject> mVehicle;
        float mEmptyTimer;
    };

    static PblList<VehicleSpawn> sVehicleSpawnList;
    static void Reset();

    static void gCalcBeginLandPos(const EntityFlyerClass& flyerClass, PblMatrix* pMatrix);
    static const PblList<VehicleSpawn>& GetVehicleSpawnList() { return sVehicleSpawnList; }

    PblList<VehicleSpawn>::Node m_ListNode;
    PblMatrix mMatrix;
    VehicleSpawnClass* mClass;
    ConstCommandPostHandle mCommandPost;
    unsigned int mNameId;
    int mSpawnCount;
    float mSpawnTime;
    float mExpireTimeEnemy;
    float mExpireTimeField;
    float mDecayTime;
    EntityClass* mSpawnClass[8];
    EntityFlyerClass* mFlyerClass[8];
    bool mUseCarrier[8];
    PblList<VehicleTracker> mTrackerList;
    PblHandle<GameObject> mCarrier;
    int mVehicleTeam;
    int mSpawnTeam;
    bool mSpawnTrigger;
    float mSpawnTimer;
    float mCycleTimer;
    RedRegion mSpawnRegion;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
