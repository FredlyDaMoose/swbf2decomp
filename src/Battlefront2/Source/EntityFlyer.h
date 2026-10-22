#pragma once

#include <stddef.h>

#include "Aimer.h"
#include "CollisionModel.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "EntityPathFollower.h"
#include "GameObject.h"
#include "GameSound.h"
#include "Trackable.h"
#include "VehicleEngine.h"
#include "VehicleMusic.h"
#include "Weapon.h"

class ExplosionClass;
class MountedTurretClass;
class PassengerSlotClass;
class RedAnimation;
class ZephyrAnim;

// VTABLE: SWBF2 0x0079b278
class EntityFlyerClass : public GameObjectClass,
                        public ControllableClass,
                        public VehicleMusicClass,
                        public VehicleEngineClass,
                        public EntityPathFollowerClass
{
public:
    EntityFlyerClass(unsigned int uHash);
    // SYNTHETIC: SWBF2 0x004b6f10
    // EntityFlyerClass::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x004b79c0
    virtual ~EntityFlyerClass() {}

    EntityFlyerClass(EntityFlyerClass* pSrc, unsigned int uHash);

private:

    friend void test_sweep_entityflyer_animation();

    bool InitAnimations(const char* pszName);

public:

    float GetTurnRate() const { return mTurnRate; }

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual int GetVehicleType();
    virtual const void* GetControllableClass();
    virtual int GetWeaponCount();
    virtual const WeaponClass* GetWeaponClass(int iIndex);
    virtual float GetTimeRequiredToEject();
    virtual float GetReboardTime();

    static PblHash rttiHashEntityFlyerClass;
    static char rttiNameEntityFlyerClass[];

    CollisionModel mFlyingCollisionModel;
    bool mFlyingCollisionInited;
    RedAnimation* mAnimObj;
    ZephyrAnim* mAnimTakeoff;
    ZephyrAnim* mAnimFins;
    float mAcceleration;
    float mBoostAcceleration;
    float mMinFlyerSpeed;
    float mMidFlyerSpeed;
    float mMaxFlyerSpeed;
    float mBoostFlyerSpeed;
    float mStrafeSpeed;
    float mPitchRate;
    float mTurnRate;
    float mPitchFilter;
    float mPitchFilterDecel;
    float mTurnFilter;
    float mTurnFilterDecel;
    float mPitchBuildupMultiplier;
    float mTurnBuildupMultiplier;
    float mThrustPitchAngle;
    float mStrafeRollAngle;
    float mBankAngle;
    float mBankFilter;
    float mLevelFilter;
    float mLevelFilterLanding;
    float mRollRate;
    float mRollRateAccel;
    float mTakeoffHeight;
    float mTakeoffTime;
    float mTakeoffSpeed;
    float mLandingTime;
    float mLandingSpeed;
    float mLandedHeight;
    float mCollisionThreshold[3];
    float mCollisionScale[3];
    float mCollisionInflict;
    float mCrashShakeStart;
    float mCrashShakeEnd;
    float mCrashShakeLength;
    float mMinCrashRollRate;
    float mMaxCrashRollRate;
    AimerClass mAimerClass[4];
    WeaponClass* mWeaponClass[4];
    WeaponAmmoDesc mWeaponAmmo[4];
    int mWeaponChannel[4];
    int mWeaponCount;
    int mVehicleType;
    float mBlurEffect;
    float mBlurStartV;
    float mFOVEffect3rd;
    PblVector3 mFOVEFFectMinCamOffset3rd;
    PblVector3 mFOVEFFectMaxCamOffset3rd;
    float mFOVEffect1st;
    EnergyBarClass mEnergyBarClass;
    float mEnergyAutoRestore;
    float mEnergyBoostDrain;
    float mEnergyTrickDrainSingleTap;
    float mEnergyTrickDrainDoubleTap;
    float mTrickRollSpeed;
    float mTrickFlipSpeed;
    float mTrickFlipCameraDetach;
    float mTrickSideRollCameraDetach;
    float mTrickSideRollStrafeSpeed;
    float mMomentumFilter;
    float mNoiseFactor;
    float mBarrierX;
    float mBarrierZ;
    float mBarrierOX;
    float mBarrierOZ;
    CameraTrackSetting mCameraSettings;
    PassengerSlotClass* mPassengerSlotClass;
    unsigned char mNumPassengerSlots;
    MountedTurretClass* mTurretClass[8];
    ExplosionClass* mExplosionCritical;
    ExplosionClass* mExplosionDestruct;
    unsigned int mThrustEffectName[8];
    unsigned int mThrustEffectHardPoint[8];
    PblVector3 mThrustEffectAttachOffset[8];
    float mThrustEffectMinScale[8];
    float mThrustEffectMaxScale[8];
    float mThrustEffectScaleStart[8];
    unsigned int mContrailEffectName[12];
    unsigned int mContrailEffectHardPoint[12];
    PblVector3 mContrailEffectAttachOffset[12];
    float mContrailEffectMinScale[12];
    float mContrailEffectMaxScale[12];
    float mContrailEffectMinSpeed[12];
    float mContrailEffectMaxSpeed[12];
    int mCockpitModelHandle;
    float mFreeFlyTime;
    float mTrickTimeMid;
    float mTrickTimeRange;
    float mCircleATATFrequency;
    float mSquadronMaxAngle;
    float mSquadronSquaredDist;
    float mTimeRequiredToEject;
    float mEjectResist;
    float mTimeTilReboard;

    bool IsGuidedMissile() const { return mGuidedMissile; }
    bool mNeverCrashWhenUnpiloted : 1;
    bool mGuidedMissile : 1;
    float mGuidedMissileDamage;
    float mGuidedMissileDamageBoost;
    float mGuidedMissileHealthScale[8];
    float mGuidedMissileShieldScale;
    float mGuidedMissileDeathCamPause;
    float mGuidedMissileDeathCamPullbackSpeed;
    float mGuidedMissileDeathCamMinDistance;
    float mGuidedMissileBoostExplosionSpeedThreshold;
    GameSound mSoundTrick;
    GameSound mSoundFlip;
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

 
 
 
 
