#pragma once

#include <stddef.h>

#include "EntityProp.h"
#include "Thread.h"
#include "PebbleFL/Common/PblHandle.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"
#include "Zephyr/Source/ZephyrSkeleton.h"

class RedAnimation;
class ZephyrAnim;
class GameObject;
class EntityPropAnimated;

// VTABLE: SWBF2 0x0079ca40
class EntityPropAnimatedClass : public EntityPropClass
{
public:
    struct AnimationTrigger
    {
        unsigned int mTriggerPoint;
        float mTriggerRadius;
        unsigned int mAnimationID;
    };

    struct AttachTrigger
    {
        unsigned int mAttachPoint;
        float mAttachRadius;
        unsigned int mAnimationID;
        int mSoldierAnimation;
        float mAnimationTime;
        int mPostAttachEvent;
    };

    EntityPropAnimatedClass(unsigned int uHash);

    EntityPropAnimatedClass(EntityPropAnimatedClass* pBase, unsigned int uHash);
    virtual ~EntityPropAnimatedClass();
    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    bool InitAnimations(const char* pszName);
    bool SetupAnimation(ZephyrPoseDyn<32>& kPose, int iAnim, float fTime, bool bLoop);

    static PblHash rttiHashEntityPropAnimatedClass;
    static char rttiNameEntityPropAnimatedClass[];

    ZephyrAnim* mAnimations[5];
    RedAnimation* mAnimObj;
    GameSound mAnimationSound[5];
    AnimationTrigger mAnimationTriggers[3];
    AttachTrigger mAttachTriggers[3];
    GameSound mKillSoldierSound;
    bool mDisableForCloneWars;
    float mIdleDelay;
    bool mTrackDeathOnAttach;
};
  
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x0079c9e0
class EntityPropAnimated : public EntityProp, public Thread
{
public:
    EntityPropAnimated(EntityPropAnimatedClass* pClass, const EntityDesc& desc);
    virtual ~EntityPropAnimated();

    virtual bool IsRtti(unsigned int uRtti) const;
    static unsigned int GetRtti();
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual EntityClass* GetEntityClass();
    virtual const EntityClass* GetEntityClass() const;

    virtual const RedPose* GetPose() const;
    virtual void Render(unsigned char ucLod, float fAlpha, unsigned int uFlags);
    virtual bool Update(float fDeltaTime);

    void SetAnimation(unsigned int uAnimationId, bool bLoop);

    static PblHash rttiHashEntityPropAnimated;
    static char rttiNameEntityPropAnimated[];

    EntityPropAnimatedClass* mAnimatedClass;
    float mAnimationTime;
    float mBlendFactor;
    unsigned int mAnimationId;
    bool mLooping;
    RedAnimation* mAnimObj;
    ZephyrSkeleton<32> mZephyrSkeleton;
    ZephyrPoseStatic<32> mZephyrPoseStatic;
    ZephyrPoseDyn<32> mZephyrPoseDyn;
    RedPose mPose;
    PblHandle<GameObject> mAttachedSoldier;
    int mAttachedSoldierAnimation;
    int mAttachEvent;
    PblMatrix* mAttachMatrix;
    PblMatrix mCameraMatrix;
    float mIdleDelayTimer;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
