#pragma once

#include <stddef.h>

#include "EntityBuilding.h"
#include "Thread.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "Zephyr/Source/ZephyrSkeleton.h"

class RedAnimation;
class ZephyrAnim;

// VTABLE: SWBF2 0x007996d0
class EntityBuildingAnimatedClass : public EntityBuildingClass
{
public:
    EntityBuildingAnimatedClass(unsigned int uHash);

    EntityBuildingAnimatedClass(EntityBuildingAnimatedClass* pBase, unsigned int uHash);

    virtual ~EntityBuildingAnimatedClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    bool InitAnimations(const char* pszName);
    bool SetupAnimation(ZephyrPoseDyn<32>& kPose, int iAnim, float fTime, bool bLoop);

    static PblHash rttiHashEntityBuildingAnimatedClass;
    static char rttiNameEntityBuildingAnimatedClass[];

    ZephyrAnim* mAnimations[2];
    RedAnimation* mAnimObj;
    bool mDeathExplosions;
    unsigned int mDeathEffectHash[5];
    unsigned int mDeathEffectNodeHash[5];
};
  
 
 
 
 
 

class EntityBuildingAnimatedClass;
struct EntityDesc;

class EntityBuildingAnimated : public EntityBuilding, public Thread
{
public:

    EntityBuildingAnimated(EntityBuildingAnimatedClass* pClass, const EntityDesc& desc);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~EntityBuildingAnimated();
    virtual void Init();
    virtual EntityClass* GetEntityClass();
    virtual const EntityClass* GetEntityClass() const;

    virtual const RedPose* GetPose() const;
    virtual void ActivatePhysics();
    virtual void DeactivatePhysics();
    virtual void Render(unsigned char uFlags, float fDist, unsigned int uMask);
    virtual void Kill();
    virtual bool Update(float fDeltaTime);

    void SetAnimation(unsigned int uAnimationId, bool bLoop);

    static PblHash rttiHashEntityBuildingAnimated;
    static char rttiNameEntityBuildingAnimated[];

    EntityBuildingAnimatedClass* mAnimatedClass;
    float mAnimationTime;
    float mDeathExplosionTime;
    unsigned int mAnimationId;
    RedAnimation* mAnimObj;
    ZephyrSkeleton<32> mZephyrSkeleton;
    ZephyrPoseDyn<32> mZephyrPoseDyn;
    RedPose mPose;
};
 
 
 
 
 
 
 
 
