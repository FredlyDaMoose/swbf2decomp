#pragma once

#include <stddef.h>

#include "EntityProp.h"
#include "Thread.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "Zephyr/Source/ZephyrSkeleton.h"

class RedAnimation;
class ZephyrAnim;

// VTABLE: SWBF2 0x0079a7e8
class EntityDoorClass : public EntityPropClass
{
public:
    struct AnimationTrigger
    {
        unsigned int mTriggerPoint;
        float mTriggerRadius;
    };

    EntityDoorClass(unsigned int uHash);
    EntityDoorClass(EntityDoorClass* pBase, unsigned int uHash);

    virtual ~EntityDoorClass();
    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    bool InitAnimations(const char* pszName);

    static PblHash rttiHashEntityDoorClass;
    static char rttiNameEntityDoorClass[];

    ZephyrAnim* mAnimation;
    RedAnimation* mAnimObj;
    AnimationTrigger mAnimationTriggers[3];
    GameSound mOpenSound;
    GameSound mCloseSound;
    GameSound mLockedSound;
};
  
 
 
 
 
 
 
 

class EntityDoorClass;
class EntityClass;

class EntityDoor : public EntityProp, public Thread
{
public:

    EntityDoor(EntityDoorClass* pClass, const EntityDesc& desc);

    static void ResetDoorList();

    static void EnterClient();
    static void LeaveClient();

    static EntityDoor* Find(unsigned int uHash);
    static void BeginIter();
    static EntityDoor* GetCurrentIter();
    static void IncrementIter();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~EntityDoor();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual EntityClass* GetEntityClass();
    virtual const EntityClass* GetEntityClass() const;
    static unsigned int GetRtti();

    virtual const RedPose* GetPose() const;

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);
    virtual bool Update(float fDeltaTime);

    static PblHash rttiHashEntityDoor;
    static char rttiNameEntityDoor[];

    static int msHostDoorsNumEntries;
    static PblHashTable<EntityDoor, 32> msHostDoors;
    static int msClientDoorsNumEntries;
    static PblHashTable<EntityDoor, 32> msClientDoors;
    static PblHashTable<EntityDoor, 32>* mspDoors;
    static PblHashTable<EntityDoor, 32>::Itor msDoorIter;

    void* portal_;
    bool hasPortal_;
    float mRenderDt;
    float mLastAnimTime;
    EntityDoorClass* mAnimatedClass;
    RedAnimation* mAnimObj;
    ZephyrSkeleton<32> mZephyrSkeleton;
    ZephyrPoseDyn<32> mZephyrPoseDyn;
    RedPose mPose;
    bool mOpenTrigger : 1;
    bool mIsOpen : 1;
    bool mIsLocked : 1;
    bool mJustRendered : 1;
    bool mTriggerSkipLastResult : 1;
    unsigned char mTriggerSkipCnt : 2;
    float mTimeSinceLastTriggered;
};
 
 
 
 
 
 
 
 
 
 
