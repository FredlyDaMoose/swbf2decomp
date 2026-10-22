#pragma once

#include <stddef.h>
#include "Entity.h"
#include "GameSound.h"
#include "ListPool.h"
#include "Thread.h"
#include "RedEngineFL/Util/RedRegion.h"

class ChaseCamera;
class PblVector3;
class RumbleEffectClass;
class RumbleRegion;

// VTABLE: SWBF2 0x007ae080
class RumbleEffect : public Entity
{
public:

    RumbleEffect(RumbleEffectClass* pClass, const PblMatrix& matrix);

    virtual ~RumbleEffect();

    // SYNTHETIC: SWBF2 0x00631340
    // RumbleEffect::`scalar deleting destructor'

    PblMatrix mMatrix;
    unsigned int mNameHash;
    unsigned int mEffectHash;
};
 
 
 
 

// VTABLE: SWBF2 0x007ae0b4
class RumbleRegionManager : public RedRegionFactory
{
public:

    RumbleRegionManager();

    virtual ~RumbleRegionManager();

    // SYNTHETIC: SWBF2 0x00630ad0
    // RumbleRegionManager::`scalar deleting destructor'

    virtual void* CreateRegion(RedRegionDesc& desc, const char* pcName) const;

    static void Reset();

    static void Update(float fDeltaTime);

    static void CheckForRumble(unsigned int uPlayer, const PblVector3& vPos,
                               ChaseCamera* pCamera, float fDist);

    // GLOBAL: SWBF2 0x007ec298
    static ListPool<RumbleRegion*, int> sRegions;
};
  

class RumbleRegionDesc
{
public:
    float mLight[2];
    float mHeavy[2];
    float mHeavyDecay[2];
    float mLightDecay[2];
    float mDelayLight[2];
    float mDelayHeavy[2];
    float mTimeLeftHeavy[2];
    float mTimeLeftLight[2];
    float mInitialTime[2];
    float mShakeAmt[2];
    float mShakeLen[2];
    unsigned int mParticleID;
    GameSound mRumbleSound;
};
 
 
 

// VTABLE: SWBF2 0x007ae068
// VTABLE: SWBF2 0x007ae044
class RumbleEffectClass : public Thread, public EntityClass
{
public:
    RumbleEffectClass(unsigned int uHash);

    virtual bool Update(float fDeltaTime);
    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    // SYNTHETIC: SWBF2 0x00630a70
    // RumbleEffectClass::`scalar deleting destructor'

    static RumbleEffectClass* FindRumbleEffectClass(unsigned int uHash);

    static void StartEffects(unsigned int uNameHash);

    static void Reset();

    // GLOBAL: SWBF2 0x007ec288
    static ListPool<RumbleEffectClass*, int> s_RumbleEffectClassList;
    // GLOBAL: SWBF2 0x007ec278
    static ListPool<RumbleEffect*, int> s_RumbleEffectList;

    RumbleRegionDesc mRumbleRegionDesc;
    unsigned int mRumbleId;
};
   
 
 

class RumbleRegion : public RedRegion
{
public:

    RumbleRegion(RedRegionDesc& desc) : RedRegion(desc) {}

    float mTimer;
    int mCurRumbleID;
    unsigned int mEffectHash;
    RumbleEffectClass* m_pREClass;
};
 
 
 
