#include "RedEngineFL/Util/RedCommandConsole.h"

#include <float.h>
#include "PebbleFL/Common/PblVector.h"
#include <string.h>

#include "EntityWalker.h"
#include "EnergyBar.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "GameLoop.h"
#include "WalkerLegPair.h"
#include "PebbleFL/Common/PblAngle.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "Explosion.h"
#include "FirstPerson.h"
#include "GameModel.h"
#include "MountedTurret.h"
#include "NetGame.h"
#include "PassengerSlot.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "Zephyr/Source/ZephyrSkeletonShared.h"
#include <new>
#include "Aimer.h"
#include "CameraManager.h"
#include "ChaseCamera.h"
#include "DeathCamera.h"
#include "Character.h"
#include "EntityCarrier.h"
#include "EntityHover.h"
#include "EntitySoldier.h"
#include "FLEffect.h"
#include "FoleyFXWalker.h"
#include "PlayerController.h"
#include "Restrictor.h"
#include "TargetManager.h"
#include "Tracker.h"
#include "UnitMobileController.h"
#include "VehicleManager.h"
#include "WaterEffect.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Application/pc/pcRedDropShadow.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedLight.h"

#include "AIUtil.h"
#include "AILowLevel.h"
#include "CommandWalker.h"
#include "LockOnManager.h"
#include "MedalsMgr.h"
#include "StatusDisplay.h"
#include "ToolTips.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"

// FUNCTION: SWBF2 0x0047f9b0
void EntityWalker::SetVelocity(const PblVector3& vVelocity)
{
    mVelocity = vVelocity;
}

// FUNCTION: SWBF2 0x0047fb30
Trackable* EntityWalker::GetTrackable()
{
    return static_cast<Trackable*>(this);
}

// FUNCTION: SWBF2 0x0047fc70
bool EntityWalker::IsForcedThirdPerson() const
{
    return true;
}

// FUNCTION: SWBF2 0x00501a80
float EntityWalker::GetAimTurnRate() const
{
    return mClass->mMaxPitchSpeed;
}

// FUNCTION: SWBF2 0x00501a90
void EntityWalker::SetWeaponIndex(int iChannel, int iIndex)
{
    mWeaponIndex[iIndex] = iChannel;
}

// FUNCTION: SWBF2 0x00505390
ExplosionClass* EntityWalker::GetExplosionClass()
{
    return mClass ? mClass->mExplosion : 0;
}

// FUNCTION: SWBF2 0x005053b0
bool EntityWalker::ShouldShowReticule() const
{
    return GetCurWpn() != 0;
}

// FUNCTION: SWBF2 0x005074b0
const PblVector3& EntityWalker::GetAttachOffset() const
{
    return mClass->mAttachOffset;
}

// FUNCTION: SWBF2 0x005074c0
CameraTrackSetting* EntityWalker::GetCameraTrackSettings()
{
    return &mClass->mCameraSettings;
}

// GLOBAL: SWBF2 0x007e6654
static int sCurrentSection = -1;
// GLOBAL: SWBF2 0x007e6658
static int sCurPair = -1;
// GLOBAL: SWBF2 0x01e51e1c
static int sWeaponSection;
// GLOBAL: SWBF2 0x01e51e12
static bool s_bSetZephyrSkeletonShared;
// GLOBAL: SWBF2 0x01e51e11
static bool sPassengerSlotSection;
// GLOBAL: SWBF2 0x01e51e20
static unsigned int sTowCableCollisionMask;

// FUNCTION: SWBF2 0x005076e0
EntityWalkerClass::EntityWalkerClass(unsigned int uHash)
    : GameObjectClass(uHash)
{
    m_bDynamicLighting = true;
    mHealthType = HEALTH_TYPE_VEHICLE;
    mPilotType = PILOT_VEHICLE;
    mLODClass = 1;
    mHoldForPlayer = true;

    mAnimObj[0] = 0;
    mAnimObj[1] = 0;
    mAnimObj[2] = 0;
    mNumAnimObjs = 0;
    m_pZephyrSkeletonShared = 0;
    mSkeletonRootScale = 1.0f;
    mAnimationCount = 0;
    mAnimations = 0;
    m_bAnimationBlending = false;
    m_uiNumAttackAnims = 0;

    sCurrentSection = -1;
    sWeaponSection = 0;
    sCurPair = -1;
    s_bSetZephyrSkeletonShared = false;

    memset(m_aAttackAnims, 0, sizeof(m_aAttackAnims));

    mVehicleType = 9;
    mAcceleration = 5.0f;
    mMaxSpeed = 20.0f;
    mBoostSpeed = 0.0f;
    mJumpVerticalSpeed = 0.0f;
    mJumpForwardSpeedBoost = 1.0f;
    mMaxTurnSpeed = 2.0f;
    mMaxYawSpeed = 2.0f;
    mMinYaw = 0.0f;
    mMaxYaw = 0.0f;
    mMaxPitchSpeed = 0.5f;
    mMinPitch = -0.5f;
    mMaxPitch = 0.5f;
    mStoppedTurnSpeed = 0.1f;
    mForwardTurnSpeed = 0.1f;
    mBoostTurnSpeed = 0.01f;
    mTurnThreshold = 0.3f;
    mMaxTerrainAngle = 0.9f;
    mOrientRoll = false;
    mWidth = 1.0f;

    mEnergyBarClass.m_fEnergyMax = 0.0f;
    mEnergyBarClass.m_fEnergyMin = 0.0f;
    mEnergyBarClass.m_fEnergyOverheat = 0.0f;
    mEnergyBarClass.Init(100.0f, 20.0f);

    mEnergyAutoRestore = 0.0f;
    mEnergyBoostDrain = 1.0f;
    mEnergyCostJump = 0.0f;
    mCollisionThreshold[0] = 7.5f;
    mCollisionThreshold[1] = 2.5f;
    mCollisionThreshold[2] = 0.0f;
    mCollisionScale[0] = 1.0f;
    mCollisionScale[1] = 0.0f;
    mCollisionScale[2] = 0.0f;
    mCollisionInflict = 1.0f;

    for (int i = 0; i < 4; ++i)
    {
        mWeaponClass[i] = 0;
        mWeaponAmmo[i].uiAmmo = 0;
        mWeaponAmmo[i].uiHeat = 0;
        mWeaponAmmo[i].bFlagAmmoInfinite = 1;
        mWeaponAmmo[i].bFlagShareHeat = 0;
        mWeaponAmmo[i].bFlagShareAmmo = 0;
        mWeaponChannel[i] = i;
    }

    mWeaponCount = 0;
    mStompRumbleLight = 0.0f;
    mStompRumbleHeavy = 0.0f;
    mStompRumbleLightDuration = 0.0f;
    mStompRumbleHeavyDuration = 0.0f;
    mStompEffect = 0;
    mStompDecal = 0;
    mFootWaterSplashEffect = 0xAEF1787Bu;
    mStompDecalSize = 2.0f;
    mNoiseFactor = 5.0f;

    mCameraSettings.mCameraMoveTensionL = 12.0f;
    mCameraSettings.mCameraMoveTensionR = 12.0f;
    mCameraSettings.mCameraMoveTensionU = 12.0f;
    mCameraSettings.mCameraMoveTensionD = 12.0f;
    mCameraSettings.mCameraMoveTensionF = 12.0f;
    mCameraSettings.mCameraMoveTensionB = 12.0f;
    mCameraSettings.mCameraAimTension = FLT_MAX;

    mPassengerSlotClass = 0;
    mNumPassengerSlots = 0;

    for (int i = 0; i < 8; ++i)
    {
        mTurretClass[i] = 0;
    }

    mExplosion = 0;
    mExplosionOffset.x = 0.0f;
    mExplosionOffset.y = 0.0f;
    mExplosionOffset.z = 0.0f;
    mFinalExplosion = 0;
    mFinalExplosionOffset.x = 0.0f;
    mFinalExplosionOffset.y = 0.0f;
    mFinalExplosionOffset.z = 0.0f;
    mDeathAnimationExplosionCount = 0;
    mDeathAnimationExplosionTime = 0.4f;
    mDeathDustClassHash = 0;
    mDeathDustEffect = 0;
    mDeathDustDelay = 0.0f;
    mDeathDustOffset.x = 0.0f;
    mDeathDustOffset.y = 0.0f;
    mDeathDustOffset.z = 0.0f;
    mDeathShakeDelay = -1.0f;
    mDeathShakeForce = 1.0f;
    mDeathShakeDuration = 1.0f;
    mDeathShakeRadius = 1.0f;
    mDeathFadeTimeStart = 0.0f;
    mDeathFadeTimeEnd = 0.0f;
    m_fThrustAngleToStrafe_Tan = 0.0f;
    m_fThrustAngleToTurn_Tan = 0.0f;
    m_fThrustAngleToStop_Tan = 5.0f;
    m_fSteerTowardAimRate = 0.0f;

    mAIUseHoverPhysics = false;
    mSteerAtYawLimit = true;
    mSteerAtStrafeLimit = true;
    mCorrectAimForSteering = true;
    mReverseBackwardSteering = false;
    mRenderPilotFlag = false;
    mSmashParkedFlyers = false;

    mStompDetectionType = 0;
    mNumFeet = 0;

    for (int i = 0; i < 6; ++i)
    {
        mFootPrimitives[i] = -1;
    }

    mStompThreshold = 0.15f;
    mLegRayHitLength = 2.5f;
    mLegPairCount = 0;
    mLegPairInfo = 0;
    mAttachHash = 0;
    mAttachOffset.x = 0.0f;
    mAttachOffset.y = 0.0f;
    mAttachOffset.z = 0.0f;
    mWaterDamageInterval = 1.0f;
    mWaterDamageAmount = 0.0f;
    mHydraulicSoundHeight = 1.0f;
    mHydrualicLowerHeight = 1.0f;
    mFPWalkScale = 1.0f;
    mFPPoseHash = 0x16E27226u;

    memset(mTowCableCollisionBodyID, 0xFF, sizeof(mTowCableCollisionBodyID));
    memset(mTowCableCollisionOffset, 0, sizeof(mTowCableCollisionOffset));

    mNumTowCableCollisions = 0;
    mFirstPersonModelHandle = -1;
    mIgnorableCollisions = 0;
    mDropShadowSize = 0.0f;
    mTimeRequiredToEject = 12.0f;
    mEjectResist = 0.1f;
    mTimeTilReboard = 10.0f;
}

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);

// GLOBAL: SWBF2 0x01ebc25c
// SYNTHETIC: SWBF2 0x00402ef0
// `dynamic initializer for 'EntityWalkerClass::rttiHashEntityWalkerClass''
PblHash EntityWalkerClass::rttiHashEntityWalkerClass("EntityWalkerClass");
char EntityWalkerClass::rttiNameEntityWalkerClass[] = "EntityWalkerClass";

// GLOBAL: SWBF2 0x01ebc2b0
// SYNTHETIC: SWBF2 0x00402ee0
// `dynamic initializer for 'EntityWalker::rttiHashEntityWalker''
PblHash EntityWalker::rttiHashEntityWalker("EntityWalker");
char EntityWalker::rttiNameEntityWalker[] = "EntityWalker";

// GLOBAL: SWBF2 0x01ebc260
// SYNTHETIC: SWBF2 0x00402ec0
// `dynamic initializer for 'EntityWalker::sMemoryPool''
// SYNTHETIC: SWBF2 0x00767010
// `dynamic atexit destructor for 'EntityWalker::sMemoryPool''
MemoryPool EntityWalker::sMemoryPool("EntityWalker", 0x2100);

// FUNCTION: SWBF2 0x0047fb20
float EntityWalker::GetMaxStrafe() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x0047fb50
const Controllable* EntityWalker::GetControllable() const
{
    return this;
}

// FUNCTION: SWBF2 0x0047fb60
Controllable* EntityWalker::GetControllable()
{
    return this;
}

// FUNCTION: SWBF2 0x0047fbd0
GameObjectType EntityWalker::GetGameObjectType()
{
    return (GameObjectType)(GO_VEHICLE | GO_WALKER);
}

// FUNCTION: SWBF2 0x0047fbe0
void EntityWalker::ReserveForPlayer(float fTime)
{
    mReserveForPlayer = GameLoop::GetMissionTime() + fTime;
}

// FUNCTION: SWBF2 0x0047fc00
bool EntityWalker::IsReservedForPlayer() const
{
    return mReserveForPlayer > GameLoop::GetMissionTime();
}

// FUNCTION: SWBF2 0x0047fc20
void EntityWalker::SetEjectTimer(float fTime)
{
    mEjectTimer = fTime;
}

// FUNCTION: SWBF2 0x0047fc40
float EntityWalker::GetEjectTimer() const
{
    return mEjectTimer;
}

// FUNCTION: SWBF2 0x0047fc50
void EntityWalker::ResetTimeSinceLastHack()
{
    mTimeSinceLastHack = 0.0f;
}

// FUNCTION: SWBF2 0x0047fc80
bool EntityWalkerClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityWalkerClass || GameObjectClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0047fcc0
int EntityWalkerClass::GetVehicleType()
{
    return mVehicleType;
}

// FUNCTION: SWBF2 0x0047fcd0
int EntityWalkerClass::GetWeaponCount()
{
    return mWeaponCount;
}

// FUNCTION: SWBF2 0x0047fce0
const WeaponClass* EntityWalkerClass::GetWeaponClass(int iIndex)
{
    return mWeaponClass[iIndex];
}

// FUNCTION: SWBF2 0x0047fd00
float EntityWalkerClass::GetCollisionInflict()
{
    return mCollisionInflict;
}

// FUNCTION: SWBF2 0x0047fd10
float EntityWalkerClass::GetTimeRequiredToEject()
{
    return mTimeRequiredToEject;
}

// FUNCTION: SWBF2 0x0047fd20
float EntityWalkerClass::GetReboardTime()
{
    return mTimeTilReboard;
}

// FUNCTION: SWBF2 0x0047fd30
const void* EntityWalkerClass::GetControllableClass()
{
    return static_cast<const ControllableClass*>(this);
}

// FUNCTION: SWBF2 0x004ff4e0
unsigned int EntityWalker::GetDerivedRtti() const
{
    return rttiHashEntityWalker;
}

// FUNCTION: SWBF2 0x004ff4f0
const char* EntityWalker::GetDerivedRttiName() const
{
    return rttiNameEntityWalker;
}

// FUNCTION: SWBF2 0x004ff510
void EntityWalker::RotateY(const PblAngle& rAngle)
{
    const float fCos = rAngle.mCos * rAngle.mCos - rAngle.mSin * rAngle.mSin;
    const float fSin = rAngle.mCos * 2.0f * rAngle.mSin;

    PblMatrix kRot;
    kRot.m[0][0] = fCos;
    kRot.m[0][1] = 0.0f;
    kRot.m[0][2] = -fSin;
    kRot.m[0][3] = 0.0f;
    kRot.m[1][0] = 0.0f;
    kRot.m[1][1] = 1.0f;
    kRot.m[1][2] = 0.0f;
    kRot.m[1][3] = 0.0f;
    kRot.m[2][0] = fSin;
    kRot.m[2][1] = 0.0f;
    kRot.m[2][2] = fCos;
    kRot.m[2][3] = 0.0f;
    kRot.m[3][0] = 0.0f;
    kRot.m[3][1] = 0.0f;
    kRot.m[3][2] = 0.0f;
    kRot.m[3][3] = 1.0f;

    PblMatrix kOut;
    D3DXMatrixMultiply(&kOut, &kRot, &mMatrix);
    mMatrix = kOut;
}

// FUNCTION: SWBF2 0x004ff5f0
EntityType EntityWalker::GetEntityType() const
{
    return ENT_WALKER;
}

// FUNCTION: SWBF2 0x004ff660
unsigned int EntityWalkerClass::GetDerivedRtti()
{
    return rttiHashEntityWalkerClass;
}

// FUNCTION: SWBF2 0x004ff670
const char* EntityWalkerClass::GetDerivedRttiName()
{
    return rttiNameEntityWalkerClass;
}

// FUNCTION: SWBF2 0x004ff690
WalkerAnimationIdle::~WalkerAnimationIdle()
{
}

// FUNCTION: SWBF2 0x004ff6d0
WalkerLegPairInfo::WalkerLegPairInfo()
{
    for (int i = 0; i < 2; ++i)
    {
        mFootIndices[i] = -1;
        mFootCollisionBody[i] = -1;
        mLegJoints[i] = -1;
        mLegMasks[i].SetAll();
        mLegTopMasks[i].SetAll();
    }
}

// SYNTHETIC: SWBF2 0x005085f0
// EntityWalkerClass::~EntityWalkerClass

// SYNTHETIC: SWBF2 0x00507d30
// EntityWalkerClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00507d60
EntityWalkerClass::EntityWalkerClass(EntityWalkerClass* pBase, unsigned int uHash)
    : GameObjectClass(pBase, uHash),
      ControllableClass(pBase),
      VehicleMusicClass(pBase),
      VehicleEngineClass(pBase)
{
    mEnergyBarClass.m_fEnergyMax = 0.0f;
    mEnergyBarClass.m_fEnergyMin = 0.0f;
    mEnergyBarClass.m_fEnergyOverheat = 0.0f;

    m_bDynamicLighting = true;
    mHealthType = HEALTH_TYPE_VEHICLE;
    mPilotType = PILOT_VEHICLE;
    mLODClass = 1;
    mHoldForPlayer = true;

    mNumAnimObjs = pBase->mNumAnimObjs;
    for (int i = 0; i < 3; ++i)
    {
        mAnimObj[i] = pBase->mAnimObj[i];
    }
    m_pZephyrSkeletonShared = pBase->m_pZephyrSkeletonShared;
    mSkeletonRootScale = pBase->mSkeletonRootScale;
    mAnimationCount = pBase->mAnimationCount;

    sCurrentSection = -1;
    sWeaponSection = 0;
    sCurPair = -1;
    s_bSetZephyrSkeletonShared = false;

    mAnimations = new WalkerAnimation*[mAnimationCount];
    memcpy(mAnimations, pBase->mAnimations, mAnimationCount * sizeof(WalkerAnimation*));

    m_bAnimationBlending = pBase->m_bAnimationBlending;
    m_uiNumAttackAnims = pBase->m_uiNumAttackAnims;
    for (int i = 0; i < 4; ++i)
    {
        m_aAttackAnims[i] = pBase->m_aAttackAnims[i];
    }

    mVehicleType = pBase->mVehicleType;
    mAcceleration = pBase->mAcceleration;
    mMaxSpeed = pBase->mMaxSpeed;
    mBoostSpeed = pBase->mBoostSpeed;
    mJumpVerticalSpeed = pBase->mJumpVerticalSpeed;
    mJumpForwardSpeedBoost = pBase->mJumpForwardSpeedBoost;
    mMaxTurnSpeed = pBase->mMaxTurnSpeed;
    mMaxYawSpeed = pBase->mMaxYawSpeed;
    mMinYaw = pBase->mMinYaw;
    mMaxYaw = pBase->mMaxYaw;
    mMaxPitchSpeed = pBase->mMaxPitchSpeed;
    mMinPitch = pBase->mMinPitch;
    mMaxPitch = pBase->mMaxPitch;
    mStoppedTurnSpeed = pBase->mStoppedTurnSpeed;
    mForwardTurnSpeed = pBase->mForwardTurnSpeed;
    mBoostTurnSpeed = pBase->mBoostTurnSpeed;
    mTurnThreshold = pBase->mTurnThreshold;
    mMaxTerrainAngle = pBase->mMaxTerrainAngle;
    mOrientRoll = pBase->mOrientRoll;
    mWidth = pBase->mWidth;
    mEnergyBarClass = pBase->mEnergyBarClass;
    mEnergyAutoRestore = pBase->mEnergyAutoRestore;
    mEnergyBoostDrain = pBase->mEnergyBoostDrain;
    mEnergyCostJump = pBase->mEnergyCostJump;
    for (int i = 0; i < 3; ++i)
    {
        mCollisionThreshold[i] = pBase->mCollisionThreshold[i];
    }
    for (int i = 0; i < 3; ++i)
    {
        mCollisionScale[i] = pBase->mCollisionScale[i];
    }
    mCollisionInflict = pBase->mCollisionInflict;

    for (int i = 0; i < 4; ++i)
    {
        mWeaponClass[i] = pBase->mWeaponClass[i];
    }
    for (int i = 0; i < 4; ++i)
    {
        mWeaponAmmo[i] = pBase->mWeaponAmmo[i];
    }
    for (int i = 0; i < 4; ++i)
    {
        mWeaponChannel[i] = pBase->mWeaponChannel[i];
    }
    mWeaponCount = pBase->mWeaponCount;

    mStompRumbleLight = pBase->mStompRumbleLight;
    mStompRumbleHeavy = pBase->mStompRumbleHeavy;
    mStompRumbleLightDuration = pBase->mStompRumbleLightDuration;
    mStompRumbleHeavyDuration = pBase->mStompRumbleHeavyDuration;
    mStompEffect = pBase->mStompEffect;
    mStompDecal = pBase->mStompDecal;
    mFootWaterSplashEffect = pBase->mFootWaterSplashEffect;
    mStompDecalSize = pBase->mStompDecalSize;
    mNoiseFactor = pBase->mNoiseFactor;

    mCameraSettings.mCameraMoveTensionL = pBase->mCameraSettings.mCameraMoveTensionL;
    mCameraSettings.mCameraMoveTensionR = pBase->mCameraSettings.mCameraMoveTensionR;
    mCameraSettings.mCameraMoveTensionU = pBase->mCameraSettings.mCameraMoveTensionU;
    mCameraSettings.mCameraMoveTensionD = pBase->mCameraSettings.mCameraMoveTensionD;
    mCameraSettings.mCameraMoveTensionF = pBase->mCameraSettings.mCameraMoveTensionF;
    mCameraSettings.mCameraMoveTensionB = pBase->mCameraSettings.mCameraMoveTensionB;
    mCameraSettings.mCameraAimTension = pBase->mCameraSettings.mCameraAimTension;

    mPassengerSlotClass = pBase->mPassengerSlotClass;
    mNumPassengerSlots = pBase->mNumPassengerSlots;
    for (int i = 0; i < 8; ++i)
    {
        mTurretClass[i] = pBase->mTurretClass[i];
    }

    mExplosion = pBase->mExplosion;
    mExplosionOffset = pBase->mExplosionOffset;
    mFinalExplosion = pBase->mFinalExplosion;
    mFinalExplosionOffset = pBase->mFinalExplosionOffset;
    mDeathAnimationExplosionCount = pBase->mDeathAnimationExplosionCount;
    memcpy(mDeathAnimationExplosions, pBase->mDeathAnimationExplosions,
           mDeathAnimationExplosionCount * sizeof(unsigned int));
    mDeathAnimationExplosionTime = pBase->mDeathAnimationExplosionTime;
    mDeathDustClassHash = pBase->mDeathDustClassHash;
    mDeathDustEffect = pBase->mDeathDustEffect;
    mDeathDustDelay = pBase->mDeathDustDelay;
    mDeathDustOffset = pBase->mDeathDustOffset;
    mDeathShakeDelay = pBase->mDeathShakeDelay;
    mDeathShakeForce = pBase->mDeathShakeForce;
    mDeathShakeDuration = pBase->mDeathShakeDuration;
    mDeathShakeRadius = pBase->mDeathShakeRadius;
    mDeathFadeTimeStart = pBase->mDeathFadeTimeStart;
    mDeathFadeTimeEnd = pBase->mDeathFadeTimeEnd;
    m_fThrustAngleToStrafe_Tan = pBase->m_fThrustAngleToStrafe_Tan;
    m_fThrustAngleToTurn_Tan = pBase->m_fThrustAngleToTurn_Tan;
    m_fThrustAngleToStop_Tan = pBase->m_fThrustAngleToStop_Tan;
    m_fSteerTowardAimRate = pBase->m_fSteerTowardAimRate;

    mAIUseHoverPhysics = pBase->mAIUseHoverPhysics;
    mSteerAtYawLimit = pBase->mSteerAtYawLimit;
    mSteerAtStrafeLimit = pBase->mSteerAtStrafeLimit;
    mCorrectAimForSteering = pBase->mCorrectAimForSteering;
    mReverseBackwardSteering = pBase->mReverseBackwardSteering;
    mRenderPilotFlag = pBase->mRenderPilotFlag;
    mSmashParkedFlyers = pBase->mSmashParkedFlyers;

    mStompDetectionType = pBase->mStompDetectionType;
    mNumFeet = pBase->mNumFeet;
    for (int i = 0; i < 6; ++i)
    {
        mFootPrimitives[i] = pBase->mFootPrimitives[i];
    }
    mStompThreshold = pBase->mStompThreshold;
    mLegRayHitLength = pBase->mLegRayHitLength;
    mLegPairCount = pBase->mLegPairCount;
    mLegPairInfo = pBase->mLegPairInfo;
    mAttachHash = pBase->mAttachHash;
    mAttachOffset = pBase->mAttachOffset;
    mWaterDamageInterval = pBase->mWaterDamageInterval;
    mWaterDamageAmount = pBase->mWaterDamageAmount;
    mHydraulicSound = pBase->mHydraulicSound;
    mHydraulicSoundHeight = pBase->mHydraulicSoundHeight;
    mHydraulicLowerSound = pBase->mHydraulicLowerSound;
    mHydrualicLowerHeight = pBase->mHydrualicLowerHeight;
    for (int i = 0; i < 6; ++i)
    {
        mFootstepSounds[i] = pBase->mFootstepSounds[i];
    }
    mFPWalkScale = pBase->mFPWalkScale;
    mFPPoseHash = pBase->mFPPoseHash;
    for (int i = 0; i < 12; ++i)
    {
        mTowCableCollisionBodyID[i] = pBase->mTowCableCollisionBodyID[i];
    }
    for (int i = 0; i < 12; ++i)
    {
        mTowCableCollisionOffset[i] = pBase->mTowCableCollisionOffset[i];
    }
    mNumTowCableCollisions = pBase->mNumTowCableCollisions;
    mFirstPersonModelHandle = pBase->mFirstPersonModelHandle;
    mIgnorableCollisions = pBase->mIgnorableCollisions;
    mDropShadowSize = pBase->mDropShadowSize;
    mTimeRequiredToEject = pBase->mTimeRequiredToEject;
    mEjectResist = pBase->mEjectResist;
    mTimeTilReboard = pBase->mTimeTilReboard;
}

// FUNCTION: SWBF2 0x00508690
EntityClass* EntityWalkerClass::Derive(unsigned int uHash)
{
    return new EntityWalkerClass(this, uHash);
}

// FUNCTION: SWBF2 0x00508700
Entity* EntityWalkerClass::Build(const EntityDesc& desc)
{
    if (EntityWalker::sMemoryPool.IsFull())
    {
        return 0;
    }

    EntityWalker* pWalker = new EntityWalker(this, desc);
    if (pWalker)
    {
        NetGame::Created(pWalker);
        CollisionManager::AddRigidBody(pWalker);
        pWalker->ActivatePhysics();
        pWalker->BuildEffects();
    }
    return pWalker;
}

namespace
{

void ReadThrustAngleTan(const char* pszValue, float& rTan)
{
    sscanf(pszValue, "%f", &rTan);
    rTan *= 0.017453292f;
    if (rTan < 0.0f)
    {
        rTan = -rTan;
    }
    const float fMax = PblMath::fHalfPi * 0.99f;
    if (rTan >= fMax)
    {
        rTan = (float)tan(fMax);
    }
    else if (rTan > 0.0f)
    {
        rTan = (float)tan(rTan);
    }
}

void ReadAngleLimits(const char* pszValue, float& rMin, float& rMax)
{
    sscanf(pszValue, "%f %f", &rMin, &rMax);
    rMin *= 0.017453292f;
    rMax *= 0.017453292f;
}
}

// FUNCTION: SWBF2 0x005087b0
void EntityWalkerClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x1cd9f762)
    {
        switch (PblHash(pszValue))
        {
        case 0xfb2aa4b7:
        case 0xea20fef2:
            sCurrentSection = 0;
            break;
        case 0xe920fd5f:
            sCurrentSection = 1;
            break;
        case 0xe820fbcc:
            sCurrentSection = 2;
            break;
        case 0xe720fa39:
            sCurrentSection = 3;
            break;
        case 0xe620f8a6:
            sCurrentSection = 4;
            break;
        case 0xe520f713:
            sCurrentSection = 5;
            break;
        case 0xe420f580:
            sCurrentSection = 6;
            break;
        case 0xf3210d1d:
            sCurrentSection = 7;
            break;
        case 0xdbaa7975:
        default:
            sCurrentSection = -1;
            return;
        }
        if (!mTurretClass[sCurrentSection])
        {
            sPassengerSlotSection = false;
            mTurretClass[sCurrentSection] = new MountedTurretClass(mId);
        }
        return;
    }

    if (sCurrentSection >= 0 &&
        mTurretClass[sCurrentSection]->SetProperty(uProperty, pszValue, mModel->GetRedModel(0)))
    {
        return;
    }

    switch (uProperty)
    {
    case 0x2fa0ba9d:
    case 0x9f8a0040:
        sscanf(pszValue, "%f", &mAcceleration);
        break;
    case 0x86df0364:
        sscanf(pszValue, "%f", &mMaxSpeed);
        break;
    case 0xa5900707:
        sscanf(pszValue, "%f", &mBoostSpeed);
        break;
    case 0xc415d23a:
    {
        float fHeight;
        sscanf(pszValue, "%f", &fHeight);
        float fSpeed = 0.0f;
        if (fHeight > 0.0f)
        {
            fSpeed = (float)sqrt((double)(fHeight * 19.6f));
        }
        mJumpVerticalSpeed = fSpeed;
        break;
    }
    case 0x0761451c:
        sscanf(pszValue, "%f", &mJumpForwardSpeedBoost);
        break;
    case 0xf0f20aba:
        sscanf(pszValue, "%f", &mJumpMinSpeed);
        break;
    case 0x5433f589:
        sscanf(pszValue, "%f", &mMaxTurnSpeed);
        break;
    case 0x7335aa87:
        sscanf(pszValue, "%f", &mMaxYawSpeed);
        if (mMaxTurnSpeed > mMaxYawSpeed)
        {
            mMaxYawSpeed = mMaxTurnSpeed;
        }
        break;
    case 0x2c3e8078:
        ReadAngleLimits(pszValue, mMinYaw, mMaxYaw);
        break;
    case 0xb5534be0:
        sscanf(pszValue, "%f", &mMaxPitchSpeed);
        break;
    case 0x3403b139:
        ReadAngleLimits(pszValue, mMinPitch, mMaxPitch);
        break;
    case 0x7c8f02c0:
        sscanf(pszValue, "%f", &mStoppedTurnSpeed);
        break;
    case 0x7eab3c84:
        sscanf(pszValue, "%f", &mForwardTurnSpeed);
        break;
    case 0x8f5adace:
        sscanf(pszValue, "%f", &mBoostTurnSpeed);
        break;
    case 0x46bba6d5:
        sscanf(pszValue, "%f", &mTurnThreshold);
        mTurnThreshold = PblMath::Sin(mTurnThreshold * 0.5f);
        break;
    case 0x47a95951:
        sscanf(pszValue, "%f", &mMaxTerrainAngle);
        mMaxTerrainAngle = PblMath::Cos(mMaxTerrainAngle * 0.017453292f);
        break;
    case 0x65587105:
    {
        int iRoll = 0;
        sscanf(pszValue, "%d", &iRoll);
        mOrientRoll = iRoll != 0;
        break;
    }
    case 0x4aee3e6b:
    case 0xc33a9f04:
    {
        int iMaskId = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (iMaskId >= 0)
        {
            mIgnorableCollisions |= 1 << iMaskId;
        }
    }

    case 0x0b9bb9c9:
        sscanf(pszValue, "%f", &mWidth);
        break;

    case 0x4f9d8bb4:
        sscanf(pszValue, "%f", &mEnergyAutoRestore);
        break;
    case 0x645f56f6:
        sscanf(pszValue, "%f", &mEnergyBoostDrain);
        break;
    case 0x0b4a7fd4:
        sscanf(pszValue, "%f", &mEnergyCostJump);
        mEnergyBarClass.SetPropertyEnergyCost(mEnergyCostJump);
        break;
    case 0x2cdb588a:
        sscanf(pszValue, "%f %f %f", &mCollisionThreshold[0], &mCollisionThreshold[1], &mCollisionThreshold[2]);
        break;
    case 0x1a87bacf:
        sscanf(pszValue, "%f %f %f", &mCollisionScale[0], &mCollisionScale[1], &mCollisionScale[2]);
        break;
    case 0x2ef03732:
        sscanf(pszValue, "%f", &mCollisionInflict);
        break;
    case 0x28adda49:
        mCombatInterrupts = atoi(pszValue) == 0;
        break;

    case 0xd0329e80:
        sWeaponSection = atoi(pszValue) - 1;
        break;
    case 0x6f332041:
    case 0xfbf47dba:
    {
        mWeaponClass[sWeaponSection] = WeaponClass::Find(PblHash(pszValue));
        int iCount = sWeaponSection + 1;
        if (mWeaponCount > iCount)
        {
            iCount = mWeaponCount;
        }
        mWeaponCount = iCount;
        break;
    }
    case 0x16472328:
        sscanf(pszValue, "%d", &mWeaponChannel[sWeaponSection]);
        break;
    case 0xdc19bd13:
    {
        int iAmmo = 0;
        sscanf(pszValue, "%d", &iAmmo);
        WeaponAmmoDesc& ammo = mWeaponAmmo[sWeaponSection];
        ammo.bFlagShareAmmo = 0;
        ammo.bFlagAmmoInfinite = iAmmo <= 0;
        if (!ammo.bFlagAmmoInfinite)
        {
            ammo.uiAmmo = iAmmo;
        }
        break;
    }
    case 0x8585bb8a:
    {
        int iWeapon = 0;
        sscanf(pszValue, "%d", &iWeapon);
        WeaponAmmoDesc& ammo = mWeaponAmmo[sWeaponSection];
        if (iWeapon > 0 && iWeapon <= sWeaponSection)
        {
            ammo.bFlagAmmoInfinite = 0;
            ammo.bFlagShareAmmo = 1;
            ammo.uiAmmo = iWeapon - 1;
        }
        else
        {
            ammo.bFlagShareAmmo = 0;
            ammo.bFlagAmmoInfinite = 1;
        }
        break;
    }
    case 0x9455a446:
    {
        int iWeapon = 0;
        sscanf(pszValue, "%d", &iWeapon);
        WeaponAmmoDesc& ammo = mWeaponAmmo[sWeaponSection];
        if (iWeapon > 0 && iWeapon <= sWeaponSection)
        {
            ammo.bFlagShareHeat = 1;
            ammo.uiHeat = iWeapon - 1;
        }
        else
        {
            ammo.bFlagShareHeat = 0;
        }
        break;
    }
    case 0x5233b817:
        mVehicleType = FindVehicleType(pszValue);
        break;

    case 0xba714b21:
        sscanf(pszValue, "%f", &mStompRumbleLight);
        break;
    case 0x8aeb89da:
        sscanf(pszValue, "%f", &mStompRumbleHeavy);
        break;
    case 0x3799c3f1:
        sscanf(pszValue, "%f", &mStompRumbleLightDuration);
        break;
    case 0x4e7dc4e6:
        sscanf(pszValue, "%f", &mStompRumbleHeavyDuration);
        break;
    case 0x07ff5a07:
        mStompEffect = PblHash(pszValue);
        break;
    case 0x9b650367:
        mStompDecal = PblHash(pszValue);
        break;
    case 0x057266ee:
        mFootWaterSplashEffect = PblHash(pszValue);
        break;
    case 0xe611ed7e:
        sscanf(pszValue, "%f", &mStompDecalSize);
        break;
    case 0xb32208ed:
    {
        int iType;
        sscanf(pszValue, "%d", &iType);
        mStompDetectionType = (unsigned char)iType;
        break;
    }
    case 0x4f08e1f5:
        sscanf(pszValue, "%f", &mStompThreshold);
        break;

    case 0x02bb1fe0:
    case 0x9b30e763:
        mExplosion = ExplosionClass::Find(PblHash(pszValue));
        break;
    case 0xc5637083:
        if (*pszValue)
        {
            float x = 0.0f, y = 0.0f, z = 0.0f;
            sscanf(pszValue, "%f %f %f", &x, &y, &z);
            mExplosionOffset.x = x;
            mExplosionOffset.y = y;
            mExplosionOffset.z = z;
        }
        break;
    case 0x58e1ba4e:
    case 0xe1b7b2ad:
        mFinalExplosion = ExplosionClass::Find(PblHash(pszValue));
        break;
    case 0xa47728f9:
        if (*pszValue)
        {
            float x = 0.0f, y = 0.0f, z = 0.0f;
            sscanf(pszValue, "%f %f %f", &x, &y, &z);
            mFinalExplosionOffset.x = x;
            mFinalExplosionOffset.y = y;
            mFinalExplosionOffset.z = z;
        }
        break;
    case 0x07fe74d0:
        if (mDeathAnimationExplosionCount < 4)
        {
            mDeathAnimationExplosions[mDeathAnimationExplosionCount++] = PblHash(pszValue);
        }
        break;
    case 0x871eb169:
        mDeathAnimationExplosionTime = (float)atof(pszValue);
        break;
    case 0x7c8816fc:
        if (*pszValue)
        {
            mDeathDustClassHash = PblHash(pszValue);
        }
        break;
    case 0xae640110:
        sscanf(pszValue, "%f", &mDeathDustDelay);
        break;
    case 0x060c07e2:
        if (*pszValue)
        {
            float x = 0.0f, y = 0.0f, z = 0.0f;
            sscanf(pszValue, "%f %f %f", &x, &y, &z);
            mDeathDustOffset.x = x;
            mDeathDustOffset.y = y;
            mDeathDustOffset.z = z;
        }
        break;
    case 0x0c13fc70:
        sscanf(pszValue, "%f", &mDeathShakeDelay);
        break;
    case 0x94b9538c:
        sscanf(pszValue, "%f", &mDeathShakeForce);
        break;
    case 0xbb80d985:
        sscanf(pszValue, "%f", &mDeathShakeDuration);
        break;
    case 0xb3af758b:
        sscanf(pszValue, "%f", &mDeathShakeRadius);
        break;
    case 0x5a2d2bca:
        sscanf(pszValue, "%f", &mDeathFadeTimeStart);
        break;
    case 0x954a0a1f:
        sscanf(pszValue, "%f", &mDeathFadeTimeEnd);
        break;

    case 0xea47dd5c:
        ReadThrustAngleTan(pszValue, m_fThrustAngleToStrafe_Tan);
        break;
    case 0xafb0caf4:
        ReadThrustAngleTan(pszValue, m_fThrustAngleToTurn_Tan);
        break;
    case 0xbed8917f:
        ReadThrustAngleTan(pszValue, m_fThrustAngleToStop_Tan);
        break;
    case 0xd128c668:
        sscanf(pszValue, "%f", &m_fSteerTowardAimRate);
        break;
    case 0x55bc9af9:
        mAIUseHoverPhysics = atoi(pszValue) != 0;
        break;
    case 0xe57fbb1d:
        mSteerAtYawLimit = atoi(pszValue) != 0;
        break;
    case 0xba804df3:
        mSteerAtStrafeLimit = atoi(pszValue) != 0;
        break;
    case 0x8ebd30be:
        mCorrectAimForSteering = atoi(pszValue) != 0;
        break;
    case 0x3f164e6b:
        mReverseBackwardSteering = atoi(pszValue) != 0;
        break;
    case 0x7fdeedf1:
        mSmashParkedFlyers = atoi(pszValue) != 0;
        break;

    case 0x64c511a4:
    {
        sPassengerSlotSection = true;
        mPassengerSlotClass = new PassengerSlotClass(mId);
        int iNumSlots = atoi(pszValue);
        if (iNumSlots < 0)
        {
            iNumSlots = 0;
        }
        else if (iNumSlots > 4)
        {
            iNumSlots = 4;
        }
        mNumPassengerSlots = (unsigned char)iNumSlots;
        break;
    }
    case 0x6363d774:
        mFirstPersonModelHandle = FirstPerson::AddGeometry(pszValue);
        break;
    case 0x46c8d867:
        mFPPoseHash = PblTEMPHash(pszValue);
        break;
    case 0x05b435b4:
        sscanf(pszValue, "%f", &mFPWalkScale);
        break;

    case 0x7012f6cd:
        s_bSetZephyrSkeletonShared = true;
        m_pZephyrSkeletonShared = RedAnimation::Find(PblHash(pszValue))->GetZephyrSkeletonShared();
        break;
    case 0x4cacc3d0:
        sscanf(pszValue, "%f", &mSkeletonRootScale);
        break;
    case 0x98557a5e:
        InitAnimations(pszValue);
        break;
    case 0x7f5a5828:
        m_bAnimationBlending = atoi(pszValue) != 0;
        break;
    case 0xaf8812e5:
        if (m_uiNumAttackAnims < 4)
        {
            m_aAttackAnims[m_uiNumAttackAnims++].uiAnimName = PblTEMPHash(pszValue);
        }
        break;
    case 0x389c392f:
        if (m_uiNumAttackAnims && m_uiNumAttackAnims <= 4)
        {
            sscanf(pszValue, "%d", &m_aAttackAnims[m_uiNumAttackAnims - 1].uiControls);
        }
        break;
    case 0x02171472:
        if (m_uiNumAttackAnims && m_uiNumAttackAnims <= 4)
        {
            float x, y, z;
            sscanf(pszValue, "%f %f %f", &x, &y, &z);
            PblVector3& vPosition = m_aAttackAnims[m_uiNumAttackAnims - 1].vPosition;
            vPosition.x = x;
            vPosition.y = y;
            vPosition.z = z;
        }
        break;
    case 0x05bf62dc:
        if (m_uiNumAttackAnims && m_uiNumAttackAnims <= 4)
        {
            sscanf(pszValue, "%f", &m_aAttackAnims[m_uiNumAttackAnims - 1].fTime);
        }
        break;
    case 0xcc4110db:
        if (m_uiNumAttackAnims && m_uiNumAttackAnims <= 4)
        {
            sscanf(pszValue, "%f", &m_aAttackAnims[m_uiNumAttackAnims - 1].fRadius);
        }
        break;
    case 0xf34add68:
        if (m_uiNumAttackAnims && m_uiNumAttackAnims <= 4)
        {
            sscanf(pszValue, "%f", &m_aAttackAnims[m_uiNumAttackAnims - 1].fDamage);
        }
        break;
    case 0x529c3d65:
        if (m_uiNumAttackAnims && m_uiNumAttackAnims <= 4)
        {
            sscanf(pszValue, "%f", &m_aAttackAnims[m_uiNumAttackAnims - 1].fPush);
        }
        break;

    case 0x153289ae:
        sscanf(pszValue, "%d", &mLegPairCount);
        mLegPairInfo = new WalkerLegPairInfo[mLegPairCount];
        sCurPair = -1;
        break;
    case 0x2aea4add:
        ++sCurPair;
        break;
    case 0xd382edb2:
        if (m_pZephyrSkeletonShared)
        {
            mLegPairInfo[sCurPair].mFootIndices[0] = m_pZephyrSkeletonShared->GetJointIdxByCrc(PblTEMPHash(pszValue));
        }
        break;
    case 0xa2f4825f:
        if (m_pZephyrSkeletonShared)
        {
            mLegPairInfo[sCurPair].mFootIndices[1] = m_pZephyrSkeletonShared->GetJointIdxByCrc(PblTEMPHash(pszValue));
        }
        break;
    case 0x21322adb:
    {
        WalkerLegPairInfo& rPair = mLegPairInfo[sCurPair];
        rPair.mFootCollisionBody[0] = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (rPair.mFootCollisionBody[0] >= 0)
        {
            mFootPrimitives[mNumFeet++] = rPair.mFootCollisionBody[0];
        }
        break;
    }
    case 0xb62deb24:
    {
        WalkerLegPairInfo& rPair = mLegPairInfo[sCurPair];
        rPair.mFootCollisionBody[1] = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (rPair.mFootCollisionBody[1] >= 0)
        {
            mFootPrimitives[mNumFeet++] = rPair.mFootCollisionBody[1];
        }
        break;
    }
    case 0x03fd0638:
        if (m_pZephyrSkeletonShared)
        {
            int iJoint = m_pZephyrSkeletonShared->GetJointIdxByCrc(PblTEMPHash(pszValue));
            if (iJoint >= 0)
            {
                mLegPairInfo[sCurPair].mLegJoints[0] = iJoint;
                mLegPairInfo[sCurPair].mLegMasks[0] = HierarchyJointMask(mAnimObj[0]->GetZephyrSkeletonShared(), iJoint);
            }
        }
        break;
    case 0xc42be48d:
        if (m_pZephyrSkeletonShared)
        {
            int iJoint = m_pZephyrSkeletonShared->GetJointIdxByCrc(PblTEMPHash(pszValue));
            if (iJoint >= 0)
            {
                mLegPairInfo[sCurPair].mLegJoints[1] = iJoint;
                mLegPairInfo[sCurPair].mLegMasks[1] = HierarchyJointMask(mAnimObj[0]->GetZephyrSkeletonShared(), iJoint);
            }
        }
        break;
    case 0x580f4d75:
        if (m_pZephyrSkeletonShared)
        {
            int iJoint = m_pZephyrSkeletonShared->GetJointIdxByCrc(PblTEMPHash(pszValue));
            if (iJoint >= 0)
            {
                mLegPairInfo[sCurPair].mLegTopMasks[0] = HierarchyJointMask(mAnimObj[0]->GetZephyrSkeletonShared(), iJoint);
            }
        }
        break;
    case 0x9eb9fc8e:
        if (m_pZephyrSkeletonShared)
        {
            int iJoint = m_pZephyrSkeletonShared->GetJointIdxByCrc(PblTEMPHash(pszValue));
            if (iJoint >= 0)
            {
                mLegPairInfo[sCurPair].mLegTopMasks[1] = HierarchyJointMask(mAnimObj[0]->GetZephyrSkeletonShared(), iJoint);
            }
        }
        break;
    case 0x391bbcda:
        sscanf(pszValue, "%f", &mLegRayHitLength);
        break;

    case 0x18be5123:
    {
        mAttachHash = PblTEMPHash(pszValue);

        if (mModel)
        {
            const RedSkeleton* pSkeleton = mModel->GetRedModel(0)->GetSkeleton();
            if (pSkeleton)
            {
                for (unsigned int i = 0; i < 128; ++i)
                {
                    if (pSkeleton->GetBoneNameHash(i) == mAttachHash)
                    {
                        const PblMatrix& mBone = pSkeleton->GetDefaultPoseMatrix(i);
                        mAttachOffset = *(const PblVector3*)&mBone.m[3][0];
                        break;
                    }
                }
            }
        }
        break;
    }
    case 0x5b491acc:
        sscanf(pszValue, "%f", &mWaterDamageInterval);
        break;
    case 0x829299ad:
        sscanf(pszValue, "%f", &mWaterDamageAmount);
        break;

    case 0x069134e5:
        mHydraulicSound.SetID(pszValue, true, false);
        break;
    case 0x8ae91462:
        sscanf(pszValue, "%f", &mHydraulicSoundHeight);
        break;
    case 0x0379fd4a:
        mHydraulicLowerSound.SetID(pszValue, true, false);
        break;
    case 0x08080fd4:
        sscanf(pszValue, "%f", &mHydrualicLowerHeight);
        break;
    case 0x97157af6:
        mFootstepSounds[0].SetID(pszValue, true, false);
        break;
    case 0x98157c89:
        mFootstepSounds[1].SetID(pszValue, true, false);
        break;
    case 0x951577d0:
        mFootstepSounds[2].SetID(pszValue, true, false);
        break;
    case 0x96157963:
        mFootstepSounds[3].SetID(pszValue, true, false);
        break;
    case 0x9b158142:
        mFootstepSounds[4].SetID(pszValue, true, false);
        break;
    case 0x9c1582d5:
        mFootstepSounds[5].SetID(pszValue, true, false);
        break;

    case 0x2436d722:
    {
        char szBody[68];
        float x, y, z;
        sscanf(pszValue, "%s %f %f %f", szBody, &x, &y, &z);
        int iMaskId = mCollisionModel.GetMaskId(PblHash(szBody));
        if (iMaskId >= 0)
        {
            sTowCableCollisionMask |= 1 << iMaskId;
            mCollisionModel.SetCollisionMask(COLL_TOWCABLE, CollisionMask(sTowCableCollisionMask));
            for (int i = 0; i < 12; ++i)
            {
                if (mTowCableCollisionBodyID[i] < 0)
                {
                    mTowCableCollisionBodyID[i] = iMaskId;
                    mTowCableCollisionOffset[i].x = x;
                    mTowCableCollisionOffset[i].y = y;
                    mTowCableCollisionOffset[i].z = z;
                    ++mNumTowCableCollisions;
                    break;
                }
            }
        }
        break;
    }
    case 0x82af5399:
        sscanf(pszValue, "%f", &mDropShadowSize);
        break;
    case 0xbb9297d5:
        sscanf(pszValue, "%f", &mTimeRequiredToEject);
        break;
    case 0xa6aa3d07:
        sscanf(pszValue, "%f", &mEjectResist);
        break;
    case 0x165e67e0:
        sscanf(pszValue, "%f", &mTimeTilReboard);
        break;

    default:
    {
        if (mEnergyBarClass.SetProperty(uProperty, pszValue))
        {
            break;
        }
        const RedModel* pModel = mModel ? mModel->GetRedModel(0) : 0;

        if ((!mPassengerSlotClass || !sPassengerSlotSection ||
             !mPassengerSlotClass->SetProperty(uProperty, pszValue, pModel)) &&
            !VehicleEngineClass::SetProperty(uProperty, pszValue) &&
            !VehicleMusicClass::SetProperty(uProperty, pszValue) &&
            !mAimerClass[sWeaponSection].SetProperty(uProperty, pszValue, pModel, 0) &&
            !mCameraSettings.SetProperty(uProperty, pszValue) &&
            !ControllableClass::SetProperty(uProperty, pszValue, pModel))
        {
            GameObjectClass::SetProperty(uProperty, pszValue);
            if (uProperty == 0x47c86b4a)
            {
                if (mModel)
                {
                    mCameraSettings.mCameraSafePoint.x = 0.0f;
                    mCameraSettings.mCameraSafePoint.y = mModel->_mModel->_TotalBoundingBox._vMax.y * 0.8f;
                    mCameraSettings.mCameraSafePoint.z = 0.0f;
                }
                sTowCableCollisionMask = 0;
                mNumTowCableCollisions = 0;
                if (mModel)
                {
                    mAttachOffset.x = 0.0f;
                    mAttachOffset.y = mModel->_mModel->_TotalBoundingBox._vMax.y;
                    mAttachOffset.z = 0.0f;
                }
            }
        }
        break;
    }
    }
}

// FUNCTION: SWBF2 0x0050a620
void EntityWalkerClass::PostReadSetup()
{
    if (mSkeletonRootScale != 1.0f)
    {
        mExplosionOffset.x *= mSkeletonRootScale;
        mExplosionOffset.y *= mSkeletonRootScale;
        mExplosionOffset.z *= mSkeletonRootScale;
        mFinalExplosionOffset.x *= mSkeletonRootScale;
        mFinalExplosionOffset.y *= mSkeletonRootScale;
        mFinalExplosionOffset.z *= mSkeletonRootScale;
        mHydraulicSoundHeight *= mSkeletonRootScale;
        mHydrualicLowerHeight *= mSkeletonRootScale;
        mLegRayHitLength *= mSkeletonRootScale;
        mWidth *= mSkeletonRootScale;
        mDropShadowSize *= mSkeletonRootScale;
        for (unsigned int i = 0; i < m_uiNumAttackAnims; ++i)
        {
            m_aAttackAnims[i].fRadius *= mSkeletonRootScale;
            m_aAttackAnims[i].vPosition.x *= mSkeletonRootScale;
            m_aAttackAnims[i].vPosition.y *= mSkeletonRootScale;
            m_aAttackAnims[i].vPosition.z *= mSkeletonRootScale;
        }
    }

    if (mAnimations)
    {
        delete[] mAnimations;
        mAnimations = 0;
    }

    if (mNumAnimObjs > 0)
    {
        mAnimationCount = m_uiNumAttackAnims + EntityWalker::StateCount;
        mAnimations = new WalkerAnimation*[mAnimationCount];

        mAnimations[EntityWalker::Idle] =
            new WalkerAnimationIdle(mAnimObj, mNumAnimObjs, "idle idle_leftup idle_rightup", false);
        mAnimations[EntityWalker::IdleToLeft] =
            new WalkerAnimationIdle(mAnimObj, mNumAnimObjs,
                                    "idle_to_leftfoot idle_to_leftfoot_leftup idle_to_leftfoot_rightup", false);
        mAnimations[EntityWalker::IdleToRight] =
            new WalkerAnimationIdle(mAnimObj, mNumAnimObjs,
                                    "idle_to_rightfoot idle_to_rightfoot_leftup idle_to_rightfoot_rightup", false);
        mAnimations[EntityWalker::LeftToRight] =
            new WalkerAnimationIdle(mAnimObj, mNumAnimObjs,
                                    "walk_leftfoot_rightfoot walk_leftfoot_rightfoot_leftup "
                                    "walk_leftfoot_rightfoot_rightup",
                                    true);
        mAnimations[EntityWalker::RightToLeft] =
            new WalkerAnimationIdle(mAnimObj, mNumAnimObjs,
                                    "walk_rightfoot_leftfoot walk_rightfoot_leftfoot_leftup "
                                    "walk_rightfoot_leftfoot_rightup",
                                    true);
        mAnimations[EntityWalker::LeftToIdle] =
            new WalkerAnimationIdle(mAnimObj, mNumAnimObjs,
                                    "leftfoot_to_idle leftfoot_to_idle_leftup leftfoot_to_idle_rightup", false);
        mAnimations[EntityWalker::RightToIdle] =
            new WalkerAnimationIdle(mAnimObj, mNumAnimObjs,
                                    "rightfoot_to_idle rightfoot_to_idle_leftup rightfoot_to_idle_rightup", false);
        mAnimations[EntityWalker::TurnLeft] =
            new WalkerAnimationIdle(mAnimObj, mNumAnimObjs, "turnl turnl_leftup turnl_rightup", false);
        mAnimations[EntityWalker::TurnRight] =
            new WalkerAnimationIdle(mAnimObj, mNumAnimObjs, "turnr turnr_leftup turnr_rightup", false);
        mAnimations[EntityWalker::Dying] = new WalkerAnimation(mAnimObj, mNumAnimObjs, "death01", false);
        mAnimations[EntityWalker::Dead] = 0;

        for (int i = 0; i < m_uiNumAttackAnims; ++i)
        {
            mAnimations[EntityWalker::StateCount + i] =
                new WalkerAnimation(mAnimObj, mNumAnimObjs, m_aAttackAnims[i].uiAnimName);
        }
    }
}

// FUNCTION: SWBF2 0x0050aba0
bool EntityWalkerClass::InitAnimations(const char* pszName)
{
    mAnimationCount = 0;
    mNumAnimObjs = 0;
    mAnimObj[0] = RedAnimation::Find(PblHash(pszName));
    if (mAnimObj[0] && !s_bSetZephyrSkeletonShared)
    {
        m_pZephyrSkeletonShared = mAnimObj[0]->GetZephyrSkeletonShared();
    }

    while (mAnimObj[mNumAnimObjs])
    {
        char szName[128];
        _snprintf(szName, 127, "%s%d", pszName, ++mNumAnimObjs);
        szName[127] = 0;
        mAnimObj[mNumAnimObjs] = RedAnimation::Find(PblHash(szName));
    }
    return mNumAnimObjs > 0;
}

// FUNCTION: SWBF2 0x0050aca0
PblBitVector<64> EntityWalkerClass::HierarchyJointMask(const ZephyrSkeletonShared* pSkeleton, int iJoint)
{
    PblBitVector<64> mask;
    mask.ClearAll();
    if (iJoint >= 0)
    {
        mask.SetBit(iJoint);
        for (int iChild = pSkeleton->m_pJoints[iJoint].m_iChild; iChild >= 0;
             iChild = pSkeleton->m_pJoints[iChild].m_iSibling)
        {
            mask.Or(HierarchyJointMask(pSkeleton, iChild));
        }
    }
    return mask;
}

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

extern bool netFrameLock;
extern bool netIsLocalTurn;

// GLOBAL: SWBF2 0x01e51e24
static bool sBipedHack;

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

void* WalkerLegPair::operator new(unsigned int uSize)
{
    return sMemoryPool.Allocate(uSize);
}

void WalkerLegPair::operator delete(void* pBlock)
{
    sMemoryPool.Free(pBlock);
}

void WalkerLegPair::SetInterpolate(bool bInterpolate)
{
    mBlend->SetInterpolate(bInterpolate);
}

namespace
{

void UpdateCollisionSphere(EntityWalker* pWalker)
{
    PblVector3 center;
    D3DXVec3TransformCoord(&center, &pWalker->mModel->_mModel->_BoundingSphere.GetCenter(), &pWalker->mMatrix);
    pWalker->SetCollisionCenter(center);
    pWalker->RecalculateAABB();
}

void SmashParkedFlyer(EntityWalker* pWalker, CollisionObject* pOther, Entity* pEntity)
{
    if (pOther->GetCollisionType() == COLL_STATIC && pWalker->mClass->mSmashParkedFlyers && pEntity &&
        pEntity->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        GameObject* pObject = pOther->GetGameObject();
        if (pObject)
        {
            Character* pCharacter = pWalker->mPilot ? pWalker->mPilot->mCharacter : 0;
            pObject->InstantDeath(DamageOwner(pCharacter, 0, 0, 0, 0, 1.0f), true);
        }
    }
}
}

// FUNCTION: SWBF2 0x0047f900
bool EntityWalker::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityWalker || GameObject::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0047f940
const RedPose* EntityWalker::GetPose() const
{
    return &mPose;
}

// FUNCTION: SWBF2 0x0047f950
GameObject* EntityWalker::GetGameObject()
{
    return this;
}

// FUNCTION: SWBF2 0x0047f960
const GameObject* EntityWalker::GetGameObject() const
{
    return this;
}

// FUNCTION: SWBF2 0x0047f970
Entity* EntityWalker::GetEntity()
{
    return this;
}

// FUNCTION: SWBF2 0x0047f980
const Entity* EntityWalker::GetEntity() const
{
    return this;
}

// FUNCTION: SWBF2 0x0047f990
const PblMatrix& EntityWalker::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x0047f9a0
const PblVector3& EntityWalker::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x0047f9d0
void EntityWalker::SetPosition(const PblVector3& vPosition)
{
    mMatrix.m[3][0] = vPosition.x;
    mMatrix.m[3][1] = vPosition.y;
    mMatrix.m[3][2] = vPosition.z;
    mMatrix.m[3][3] = 1.0f;
    UpdateCollisionSphere(this);
}

// FUNCTION: SWBF2 0x0047fa70
void EntityWalker::SetMatrix(const PblMatrix& matrix)
{
    mMatrix = matrix;
    UpdateCollisionSphere(this);
}

// FUNCTION: SWBF2 0x005027a0
int EntityWalker::GetFirstPersonIndex()
{
    if (mPlayerId < 0 || !mTracker || !mTracker->IsFirstPersonView())
    {
        return -1;
    }
    if (!NetGame::IsEnabled() || NetGame::IsClient() ||
        (NetGame::IsHost() && NetGame::IsOneWorld() && !NetGame::IsDedicated()))
    {
        return NetGame::GetJoystickIndex(mPlayerId);
    }
    return -1;
}

// FUNCTION: SWBF2 0x005075a0
void EntityWalker::DeactivatePhysics()
{
    for (int i = 0; i < mClass->mNumPassengerSlots; ++i)
    {
        if (mPassengerSlots[i])
        {
            mPassengerSlots[i]->DeactivatePhysics();
        }
    }
    for (int i = 0; i < mNumTurrets; ++i)
    {
        if (mTurret[i])
        {
            mTurret[i]->DeactivatePhysics();
        }
    }
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mAimer[i])
        {
            mAimer[i]->DeactivatePhysics();
        }
    }
    mPostCollision.DeactivateThread();
    static_cast<Controllable*>(this)->DeactivateThread();
}

// FUNCTION: SWBF2 0x00507280
void EntityWalker::SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix)
{
    if (IsAlive() && (mTargetInfo.mIsAiming || (mTracker && mTracker->IsFirstPersonView())))
    {
        int iFirstPerson = GetFirstPersonIndex();
        if (iFirstPerson >= 0 && !mTargetInfo.mIsAiming)
        {
            FirstPerson::UpdateAimer(iFirstPerson, rMatrix);
        }
    }
}

// FUNCTION: SWBF2 0x005072d0
void EntityWalker::SetupCameraFOV(float& rFOV)
{
    if (!IsAlive())
    {
        rFOV = GetDefaultFOV();
        return;
    }
    const ControllableClass* pSettings = GetControllableClass();
    rFOV = mTargetInfo.mIsAiming || (mTracker && mTracker->IsFirstPersonView())
        ? pSettings->mFirstPersonFOV : pSettings->mThirdPersonFOV;
}

// FUNCTION: SWBF2 0x00507340
void EntityWalker::SetupCameraZoomFactor(float& rZoom)
{
    if (mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0)
    {
        Weapon* pWeapon = mWeapon[mWeaponIndex[0]];
        if (pWeapon)
        {
            rZoom = pWeapon->mZoom;
            return;
        }
    }
    rZoom = 1.0f;
}

// FUNCTION: SWBF2 0x00500cc0
bool EntityWalker::Jump()
{
    if (m_bIsJumping || mClass->mJumpVerticalSpeed <= 0.0f ||
        (!m_bIsGrounded && m_fGroundedTimer > 0.3f))
    {
        return false;
    }

    float fForwardSpeed = mVelocity.x * mMatrix.m[2][0] +
                          mVelocity.y * mMatrix.m[2][1] +
                          mVelocity.z * mMatrix.m[2][2];
    if (fForwardSpeed < mClass->mJumpMinSpeed ||
        !((EnergyBar*)mEnergyBar)->SpendEnergy(mClass->mEnergyCostJump))
    {
        return false;
    }

    mVelocity.y += mClass->mJumpVerticalSpeed;
    if (mClass->mJumpForwardSpeedBoost > 1.0f && fForwardSpeed > 0.0f)
    {
        float fBoost = (mClass->mJumpForwardSpeedBoost - 1.0f) * fForwardSpeed;
        mVelocity.x += fBoost * mMatrix.m[2][0];
        mVelocity.y += fBoost * mMatrix.m[2][1];
        mVelocity.z += fBoost * mMatrix.m[2][2];
    }
    m_bIsGrounded = false;
    m_bIsJumping = true;
    m_fGroundedTimer = 0.0f;
    return true;
}

// FUNCTION: SWBF2 0x0047fb70
int EntityWalker::GetWeaponIndex(int iChannel) const
{
    return mWeaponIndex[iChannel];
}

// FUNCTION: SWBF2 0x0047fb90
Weapon* EntityWalker::GetWeaponPtr(int iIndex) const
{
    return mWeapon[iIndex];
}

// FUNCTION: SWBF2 0x0047fbb0
const Aimer* EntityWalker::GetAimer(int iIndex) const
{
    return mAimer[iIndex];
}

// FUNCTION: SWBF2 0x0047fc60
EnergyBar* EntityWalker::GetEnergyBar()
{
    return (EnergyBar*)mEnergyBar;
}

// SYNTHETIC: SWBF2 0x004803a5
// [thunk]:EntityWalker::GetGameObject`adjustor{600}'

// SYNTHETIC: SWBF2 0x004803b0
// [thunk]:EntityWalker::GetGameObject`adjustor{600}'

// SYNTHETIC: SWBF2 0x004803c6
// [thunk]:EntityWalker::GetGameObject`adjustor{12}'

// SYNTHETIC: SWBF2 0x004803d6
// [thunk]:EntityWalker::GetEntity`adjustor{576}'

// SYNTHETIC: SWBF2 0x004803e1
// [thunk]:EntityWalker::GetEntity`adjustor{576}'

// SYNTHETIC: SWBF2 0x0048040d
// [thunk]:EntityWalker::GetMatrix`adjustor{564}'

// SYNTHETIC: SWBF2 0x0048044c
// [thunk]:EntityWalker::GetEntity`adjustor{12}'

// SYNTHETIC: SWBF2 0x00480467
// [thunk]:EntityWalker::GetGameObject`adjustor{320}'

// SYNTHETIC: SWBF2 0x0050b00e
// [thunk]:EntityWalker::`scalar deleting destructor'`adjustor{576}'

// FUNCTION: SWBF2 0x004ff600
bool EntityWalker::PostCollision::Update(float fDeltaTime)
{
    return mOwner->PostCollisionUpdate(fDeltaTime);
}

// SYNTHETIC: SWBF2 0x004ff620
// EntityWalker::PostCollision::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004ff650
EntityWalker::PostCollision::~PostCollision()
{
}

// SYNTHETIC: SWBF2 0x004ff720
// EntityWalker::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004ff750
EntityWalker::EntityWalker(EntityWalkerClass* pClass, const EntityDesc& desc)
    : GameObject(pClass, desc, COLL_RIGID, &pClass->mModel->_mLodData)
    , Controllable(pClass, desc.mPlayerId)
    , VehicleMusic(pClass)
    , VehicleEngine(pClass)
    , mClass(pClass)
    , mBlend(0)
    , mReserveForPlayer(0.0f)
    , mLegPairCount(0)
    , mFootState(0)
    , mFootWaterState(0)
    , mState(Idle)
    , mDirection(None)
    , mPostCollision(this)
{
    mMainMask.SetAll();
    mPivotOffset.x = 0.0f;
    mPivotOffset.y = 0.0f;
    mPivotOffset.z = 0.0f;
    new (mEnergyBar) EnergyBar(&pClass->mEnergyBarClass);
    mBoost = false;
    mSpeedPrev = 0.0f;
    mEjectTimer = 0.0f;
    mTimeSinceLastHack = 0.0f;
    mAttackControlTimer = 0.0f;

    const RedModel* pRedModel = mModel->GetRedModel(0);
    mStencilShadow = pRedModel ? pRedModel->_pStencilShadow : 0;

    VehicleManager::Add(this);
    TargetManager::AddTarget(this, true);
    mControllableList.AddLast(&this->Controllable::mNode, this);

    mNumTurrets = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (mClass->mTurretClass[i])
        {
            mTurret[mNumTurrets] = new MountedTurret(this, mClass->mTurretClass[i], &mPose);
            mControllableList.AddLast(&mTurret[mNumTurrets]->mNode, mTurret[mNumTurrets]);
            ++mNumTurrets;
        }
    }

    if (mClass->mPassengerSlotClass)
    {
        for (int i = 0; i < mClass->mNumPassengerSlots; ++i)
        {
            mPassengerSlots[i] = new PassengerSlot(this, mClass->mPassengerSlotClass, &mPose);
            mControllableList.AddLast(&mPassengerSlots[i]->mNode, mPassengerSlots[i]);
        }
    }

    mAnimObj = mClass->mAnimObj[0];
    AddToScene();

    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    mVelocityNoGrav.x = 0.0f;
    mVelocityNoGrav.y = 0.0f;
    mVelocityNoGrav.z = 0.0f;
    for (int i = 0; i < 4; ++i)
    {
        mAimer[i] = 0;
    }
    for (int i = 0; i < 4; ++i)
    {
        mWeapon[i] = 0;
    }

    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (!mClass->mWeaponClass[i])
        {
            continue;
        }
        mAimer[i] = new Aimer(&mClass->mAimerClass[i], this, &mMatrix, &mPose, 0);
        WeaponDesc weaponDesc;
        weaponDesc.mOwner = this;
        weaponDesc.mAimer = mAimer[i];
        weaponDesc.mTrigger = 0;
        weaponDesc.mReload = 0;
        weaponDesc.mNumClips = 0;
        weaponDesc.m_pAmmoCounter = 0;
        weaponDesc.m_pEnergyBar = 0;
        weaponDesc.mTrigger = &mControlFire[mClass->mWeaponChannel[i]];
        weaponDesc.mReload = &mControlReload;
        const WeaponAmmoDesc& ammo = mClass->mWeaponAmmo[i];
        if (ammo.bFlagShareAmmo)
        {
            weaponDesc.m_pAmmoCounter = mWeapon[ammo.uiAmmo]->m_pAmmoCounter;
        }
        else if (ammo.bFlagAmmoInfinite)
        {
            weaponDesc.mNumClips = 0x7fffffff;
        }
        else
        {
            weaponDesc.mNumClips = ammo.uiAmmo;
        }
        if (ammo.bFlagShareHeat)
        {
            weaponDesc.m_pEnergyBar = mWeapon[ammo.uiHeat]->m_pEnergyBar;
        }
        mWeapon[i] = mClass->mWeaponClass[i]->Build(weaponDesc);
    }
    for (int iChannel = 0; iChannel < 2; ++iChannel)
    {
        mWeaponIndex[iChannel] = -1;
        for (int i = 0; i < mClass->mWeaponCount; ++i)
        {
            if (mWeapon[i] && mClass->mWeaponChannel[i] == iChannel)
            {
                mWeaponIndex[iChannel] = i;
                break;
            }
        }
    }

    ClearControllable();
    mLegAngle = 0.0f;

    mZephyrSkeleton.Open(*mClass->m_pZephyrSkeletonShared, mZephyrPoseStatic);
    mZephyrPoseStatic.Open(mZephyrSkeleton);
    mBlend = new WalkerBlend(3, mZephyrSkeleton);
    mBlend->EnableTransitionBlending(mClass->m_bAnimationBlending);

    mLegPairCount = mClass->mLegPairCount;
    for (int i = 0; i < mLegPairCount; ++i)
    {
        mMainMask.And(~mClass->mLegPairInfo[i].mLegMasks[0]);
        mMainMask.And(~mClass->mLegPairInfo[i].mLegMasks[1]);
    }
    mBlend->SetJointMask(0, mMainMask);
    mBlend->SetJointMask(1, mMainMask);
    mBlend->SetJointMask(2, mMainMask);
    for (int i = 0; i < mLegPairCount; ++i)
    {
        mLegPairs[i] = new WalkerLegPair(this, mClass->mLegPairInfo[i]);
    }

    mZephyrPoseStatic.SetBasePose();
    mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
    mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    mModel->GetRedModel(0)->SetupPose(mPose);

    mIsDead = false;
    mExplodedDeath = false;
    mExplodedFinal = false;
    mVelocity.y = 0.0f;
    mVelocityNoGrav.y = 0.0f;
    mYawValue = 0.0f;
    mPitchValue = 0.0f;
    mEyeDir = mMatrix.GetZAxis();
    mEyePoint = mClass->mCameraSettings.CalcEyePoint(mMatrix);
    _uiRenderFlags &= 3;
    mDoneDustCloud = false;
    mDeathExplosionTime = 0.0f;
    mDeathTime = 0.0f;
    for (int i = 0; i < 6; ++i)
    {
        mLastFootHeight[i] = 0.0f;
        mMinFootHeight[i] = FLT_MAX;
    }
    mWaterDamageTimer = mClass->mWaterDamageInterval;
    mPrevAimerPos.x = -66.6f;
    mPrevAimerPos.y = 66.6f;
    mPrevAimerPos.z = -66.6f;
    mIsStopped = false;
    m_bIsGrounded = false;
    m_bIsJumping = false;
    m_uiAttackControls = 0;
    m_uiAttackAnim = 0;
    mBodyPitch = 0.0f;
    mBodyRoll = 0.0f;
    mStopTime = -100.0f;
    m_fGroundedTimer = 0.0f;
}

// FUNCTION: SWBF2 0x00500160
EntityWalker::~EntityWalker()
{
    NetGame::Destroyed(this);
    for (int i = 0; i < mLegPairCount; ++i)
    {
        delete mLegPairs[i];
    }
    delete mBlend;
    while (mNumTurrets)
    {
        --mNumTurrets;
        mControllableList.Remove(&mTurret[mNumTurrets]->mNode);
        delete mTurret[mNumTurrets];
    }
    for (int i = 0; i < mClass->mNumPassengerSlots; ++i)
    {
        mControllableList.Remove(&mPassengerSlots[i]->mNode);
        delete mPassengerSlots[i];
    }
    mControllableList.Remove(&this->Controllable::mNode);
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        delete mWeapon[i];
        delete mAimer[i];
    }
    TargetManager::RemoveTarget(this);
    VehicleManager::Remove(this);
    reinterpret_cast<EnergyBar*>(mEnergyBar)->~EnergyBar();
}

// FUNCTION: SWBF2 0x00500410
void EntityWalker::Init()
{
    GameObject::Init();
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mWeapon[i])
        {
            mWeapon[i]->Init();
        }
    }
    for (int i = 0; i < mNumTurrets; ++i)
    {
        if (mTurret[i])
        {
            mTurret[i]->Init();
        }
    }
    for (int i = 0; i < mClass->mNumPassengerSlots; ++i)
    {
        mPassengerSlots[i]->Init();
    }

    const PblList<Controllable>& list = GetControllableList();
    if (list.GetCount() > 1)
    {
        for (int i = 0; i < mClass->mWeaponCount; ++i)
        {
            if (mWeapon[i])
            {
                mWeapon[i]->Select(0, false);
            }
        }
    }
    else
    {
        for (int iChannel = 0; iChannel < 2; ++iChannel)
        {
            int iIndex = mWeaponIndex[iChannel];
            if (iIndex >= 0 && mWeapon[iIndex])
            {
                mWeapon[iIndex]->Select(iChannel, false);
            }
        }
    }

    NextState(0);
    for (int i = 0; i < mLegPairCount; ++i)
    {
        mLegPairs[i]->InitDone();
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        for (PblList<Controllable>::Itor it(list); it; ++it)
        {
            it->SetupController();
        }
    }
    m_uiAttackControls = 0;
    m_uiAttackAnim = 0;
    mAttackControlTimer = 0.0f;
}

// FUNCTION: SWBF2 0x005005c0
void EntityWalker::SetupController()
{
    NextState(0);
    Controller* pController = 0;
    if (mPlayerId >= 0)
    {
        pController = new PlayerController(this);
        Track(mPilot->mPlayerId);
    }
    else if (mPilot)
    {
        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            pController = new UnitMobileController(this);
        }
    }
    if (pController)
    {
        AddController(pController, mPilot->mPlayerId < 0);
    }
    ClearControllable();
}

// FUNCTION: SWBF2 0x00500700
FoleyFX* EntityWalker::GetFoleyFX() const
{
    return FoleyFXCollider::GetFoleyFX(0x5e9b9948);
}

// FUNCTION: SWBF2 0x00500710
void EntityWalker::DoFootImpactEffects()
{
    int iNumFeet = mClass->mNumFeet;
    unsigned int uBit = 1;
    for (int i = 0; i < iNumFeet; ++i)
    {
        bool bImpact = false;
        float fHalfHeight = mCollisionModel->GetHeightFromMaskID(mClass->mFootPrimitives[i]) * 0.5f;
        PblMatrix mFoot = mCollisionModel->GetWorldMatrix(mClass->mFootPrimitives[i], mMatrix, GetPose());
        mFoot.m[3][0] = mFoot.m[3][0] - mFoot.m[1][0] * fHalfHeight;
        mFoot.m[3][1] = mFoot.m[3][1] - mFoot.m[1][1] * fHalfHeight;
        mFoot.m[3][2] = mFoot.m[3][2] - mFoot.m[1][2] * fHalfHeight;
        mFoot.m[3][3] = 1.0f;
        PblVector3 vSplash(mFoot.m[3][0], mMatrix.m[3][1], mFoot.m[3][2]);
        const PblVector3& vFoot = mFoot.GetTranslation();
        float fHeight = mFoot.m[3][1] - mMatrix.m[3][1];

        if (!mClass->mStompDetectionType)
        {
            float fLowest = mClass->mStompThreshold + mMinFootHeight[i];
            if (mLastFootHeight[i] > fHeight && fLowest > fHeight && mLastFootHeight[i] > fLowest)
            {
                bImpact = true;
            }
        }
        else
        {
            unsigned int uLifted = (uBit << 24) | (uBit >> 8);
            float fDrop = mLastFootHeight[i] - fHeight;
            if (mFootState & uLifted)
            {
                if (fDrop > 0.1f)
                {
                    mFootState &= ~uLifted;
                }
            }
            else if (0.1f > fDrop)
            {
                mFootState |= uLifted;
                bImpact = true;
            }
        }
        if (mMinFootHeight[i] > fHeight)
        {
            mMinFootHeight[i] = fHeight;
        }

        unsigned int uRaised = (uBit << 8) | (uBit >> 24);
        unsigned int uLowered = (uBit << 16) | (uBit >> 16);
        if (fHeight > mLastFootHeight[i])
        {
            if (fHeight > mClass->mHydraulicSoundHeight && !(mFootState & uRaised))
            {
                mClass->mHydraulicSound.Play(&vFoot, &mVelocity, false, true);
                mFootState |= uRaised;
            }
        }
        else
        {
            mFootState &= ~uRaised;
        }
        if ((int)(fHeight * 10.0f) < (int)(mLastFootHeight[i] * 10.0f))
        {
            if (mClass->mHydrualicLowerHeight > fHeight && !(mFootState & uLowered))
            {
                mClass->mHydraulicLowerSound.Play(&vFoot, &mVelocity, false, true);
                mFootState |= uLowered;
            }
        }
        else
        {
            mFootState &= ~uLowered;
        }

        float fWater;
        if (RedTerrain::_bEnable && RedWater::GetWaterHeight(vFoot, fWater))
        {
            if (fWater > RedTerrain::GetInterpolatedHeight(vFoot.x, vFoot.z) && fWater > vFoot.y)
            {
                if (!(mFootWaterState & uBit))
                {
                    mFootWaterState |= uBit;
                    WaterEffect::AddWaterEffect(mClass->mFootWaterSplashEffect, vFoot);
                    RedWater::AddWaterRing(vFoot, 2.0f, 0.1f);
                }
            }
            else
            {
                mFootWaterState &= ~uBit;
            }
        }

        mLastFootHeight[i] = fHeight;
        if (bImpact)
        {

            GetControllableList();
            mFoot.m[3][0] = vSplash.x;
            mFoot.m[3][1] = vSplash.y;
            mFoot.m[3][2] = vSplash.z;
            mFoot.m[3][3] = 1.0f;
            float fSplashWater;
            if (RedWater::GetWaterHeight(vSplash, fSplashWater) && fSplashWater >= vSplash.y)
            {
                RedWater::AddWaterRing(vSplash, 3.0f, 0.5f);
            }
            if (FoleyFX* pFoley = GetFoleyFX())
            {
                static_cast<FoleyFXWalker*>(pFoley)->mFootstep[i].Play(&vFoot, 0, false, true);
            }
            mClass->mFootstepSounds[i].Play(&vFoot, 0, false, true);
            mFootState |= uBit;
            if (mClass->mStompEffect)
            {
                FLEffectClass* pEffectClass = FLEffect::s_EffectClasses.Find(mClass->mStompEffect);
                if (pEffectClass)
                {
                    FLEffectObject* pEffect = pEffectClass->BuildEffect();
                    if (pEffect)
                    {
                        pEffect->AttachEffectToMatrix(&mFoot, false);
                    }
                }
            }
        }
        else
        {
            mFootState &= ~uBit;
        }
        uBit = (uBit << 1) | (uBit >> 31);
    }
}

// FUNCTION: SWBF2 0x00500b80
void EntityWalker::Kill()
{
    mIsAlive = false;
    if (mTracker && mTracker->IsFirstPersonView())
    {
        int iJoystick = GetJoystickIndex();
        if (iJoystick >= 0)
        {
            FirstPerson::Activate(iJoystick, false);
            FirstPerson::Update(iJoystick, TYPE_COCKPIT, -2, 0, 0);
        }
    }
    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->Kill();
    }
    for (int i = 0; i < mClass->mNumPassengerSlots; ++i)
    {
        mPassengerSlots[i]->Kill();
    }
    GameObject::Kill();
    Controllable::Kill();
    DeactivateEngine();
}

extern PblVector3 gVectorZero;

// FUNCTION: SWBF2 0x00500c50
int EntityWalker::IsTowCableCollisionValid(int iBodyId, int iLastBodyId, int iPrevBodyId)
{
    return mClass->IsTowCableCollisionValid(iBodyId, iLastBodyId, iPrevBodyId);
}

const PblVector3& EntityWalkerClass::GetTowCableCollisionOffset(int iBodyID)
{
    for (int i = 0; i < 12; i++)
    {
        if (mTowCableCollisionBodyID[i] == iBodyID)
        {
            return mTowCableCollisionOffset[i];
        }
    }
    return gVectorZero;
}

// FUNCTION: SWBF2 0x00500c60
const PblVector3& EntityWalker::GetTowCableCollisionOffset(int iBodyID)
{
    return mClass->GetTowCableCollisionOffset(iBodyID);
}

// FUNCTION: SWBF2 0x0050ad20
int EntityWalkerClass::IsTowCableCollisionValid(int iBodyId, int iLastBodyId, int iPrevBodyId)
{
    int iNum = mNumTowCableCollisions;
    int iLegSize = (unsigned char)(mNumTowCableCollisions >> 2);
    int iValid[12];
    for (int i = 0; i < iNum; i++)
    {
        iValid[i] = 1;
    }

    for (int iLeg = 0; iLeg + iLegSize - 1 < iNum; iLeg += iLegSize)
    {
        bool bFound = false;
        for (int j = 0; j < iLegSize; j++)
        {
            bFound |= mTowCableCollisionBodyID[iLeg + j] == iPrevBodyId;
        }
        if (bFound)
        {
            for (int j = 0; j < iLegSize; j++)
            {
                iValid[iLeg + j] = -1;
            }
        }
    }

    for (int iLeg = 0; iLeg + iLegSize - 1 < iNum; iLeg += iLegSize)
    {
        bool bFound = false;
        for (int j = 0; j < iLegSize; j++)
        {
            bFound |= mTowCableCollisionBodyID[iLeg + j] == iLastBodyId;
        }
        if (!bFound)
        {
            continue;
        }

        for (int j = 0; j < iLegSize; j++)
        {
            iValid[iLeg + j] = 0;
        }

        int iMark;
        int iBefore = iLeg - iLegSize;
        if (iBefore < 0)
        {
            iBefore += mNumTowCableCollisions;
        }
        if (iValid[iBefore] == -1)
        {
            iMark = iBefore - iLegSize;
            if (iMark < 0)
            {
                iMark += mNumTowCableCollisions;
            }
        }
        else
        {
            int iAfter = iLeg + iLegSize;
            if (iAfter >= mNumTowCableCollisions)
            {
                iAfter -= mNumTowCableCollisions;
            }
            if (iValid[iAfter] != -1)
            {
                continue;
            }
            iMark = iAfter + iLegSize;
            if (iMark >= mNumTowCableCollisions)
            {
                iMark -= mNumTowCableCollisions;
            }
        }
        for (int j = 0; j < iLegSize; j++)
        {
            iValid[iMark + j] = -1;
        }
    }

    for (int i = 0; i < iNum; i++)
    {
        if (mTowCableCollisionBodyID[i] == iBodyId)
        {
            return iValid[i];
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x00500ca0
int EntityWalker::GetReservableSlots(ReserveType eType) const
{
    if (eType == RT_FORMATION)
    {
        return 4;
    }
    return GetControllableList().GetCount();
}

// FUNCTION: SWBF2 0x00500e20
void EntityWalker::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (!RenderTrackable(mPlayerId))
    {
        return;
    }
    for (int i = 0; i < mNumTurrets; ++i)
    {
        if (mTurret[i] && !mTurret[i]->RenderTrackable(mTurret[i]->mPlayerId))
        {
            return;
        }
    }

    if (mClass->m_bDynamicLighting)
    {
        uiFlags &= ~0x80;
    }
    else
    {
        uiFlags |= 0x80;
    }

    if (!ucViewport)
    {
        mBlend->SetInterpolate(true);
        for (int i = 0; i < mLegPairCount; ++i)
        {
            mLegPairs[i]->SetInterpolate(true);
        }
    }

    PblMatrix mWorld = GetSmoothedMatrix();
    if (!netFrameLock && NetGame::IsEnabled())
    {
        gameObjectInterpolator.GetMatrix(mNetUniqueId, mWorld);
    }
    const PblMatrix mInverse = mWorld.GetInverseFast();

    float fAlpha = 1.0f;
    if (!netFrameLock && NetGame::IsEnabled())
    {
        bool bWrapped;
        float fDir = mDirection == Backward ? -1.0f : 1.0f;
        float fBlendState = gameObjectInterpolator.GetLoopingFloat(mNetUniqueId, 0, mBlend->GetStateLimit(), fDir,
                                                                    bWrapped);
        float fLegState = gameObjectInterpolator.GetLoopingFloat(mNetUniqueId, 1, mLegPairs[0]->GetStateLimit(),
                                                                  mDirection == Backward ? -1.0f : 1.0f, bWrapped);
        mBlend->SetFrameState(fBlendState);
        mLegPairs[0]->SetFrameState(fLegState);
        if (mLegPairCount > 1)
        {
            fDir = -1.0f;
            if (mDirection != Backward)
            {
                fDir = 1.0f;
            }
            mLegPairs[1]->SetFrameState(gameObjectInterpolator.GetLoopingFloat(
                mNetUniqueId, 2, mLegPairs[1]->GetStateLimit(), fDir, bWrapped));
        }
        BuildWorldMatrices();
    }

    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (!mAimer[i])
        {
            continue;
        }
        bool bRestore = false;
        PblVector3 vSaved;
        if (i == 0 && !netFrameLock && NetGame::IsEnabled())
        {
            bRestore = true;
            vSaved = mAimer[0]->mDirection;
            PblVector3 vDirection = vSaved;
            gameObjectInterpolator.GetVector(mNetUniqueId, vDirection);
            mAimer[0]->mDirection = vDirection;
        }
        mAimer[i]->SetupPose(mInverse, mPose, true);
        if (bRestore)
        {
            mAimer[i]->mDirection = vSaved;
        }
    }

    if (mIsDead && mState != Dead)
    {
        if (mClass->mDeathAnimationExplosionCount > 0)
        {
            mDeathExplosionTime += GameLoop::GetDeltaTime();
            if (mDeathExplosionTime > mClass->mDeathAnimationExplosionTime)
            {
                mDeathExplosionTime = 0.0f;
                unsigned int uEffect = mClass->mDeathAnimationExplosions[PblRandom::GetGlobalInt() %
                                                                         mClass->mDeathAnimationExplosionCount];
                FLEffectClass* pEffectClass = FLEffect::s_EffectClasses.Find(uEffect);
                if (pEffectClass)
                {
                    FLEffectObject* pEffect = pEffectClass->BuildEffect();
                    if (pEffect)
                    {
                        unsigned int uRandom = PblRandom::GetGlobalInt();
                        unsigned int uBone = mCollisionModel->GetParentNameFromMaskId(
                            uRandom % mCollisionModel->GetNumBodies());
                        const PblMatrix* pBone = GetPose()->Find(uBone);

                        if (pBone)
                        {
                            pEffect->AttachEffectToObject(this, pBone, false);
                        }
                        else
                        {
                            pEffect->AttachEffectToMatrix(&mWorld, false);
                        }
                    }
                }
            }
        }

        float fLastDeathTime = mDeathTime;
        mDeathTime += GameLoop::GetDeltaTime();
        if (!mDoneDustCloud && mDeathTime > mClass->mDeathDustDelay)
        {
            mDoneDustCloud = true;
            if (mClass->mDeathDustClassHash && !mClass->mDeathDustEffect)
            {
                mClass->mDeathDustEffect = FLEffect::s_EffectClasses.Find(mClass->mDeathDustClassHash);
            }
            if (mClass->mDeathDustEffect)
            {
                FLEffectObject* pEffect = mClass->mDeathDustEffect->BuildEffect();
                if (pEffect)
                {
                    PblMatrix mDust = _IdentityMatrix;
                    mDust.m[3][0] = mClass->mDeathDustOffset.x;
                    mDust.m[3][1] = mClass->mDeathDustOffset.y;
                    mDust.m[3][2] = mClass->mDeathDustOffset.z;
                    mDust.m[3][3] = 1.0f;
                    PblMatrix mTemp;
                    D3DXMatrixMultiply(&mTemp, &mDust, &mWorld);
                    mDust = mTemp;
                    pEffect->AttachEffectToMatrix(&mDust, false);
                }
            }
        }

        if (mClass->mDeathShakeDelay >= fLastDeathTime && mDeathTime > mClass->mDeathShakeDelay)
        {

            CameraManager* pManager = CameraManager::sInstance;
            Trackable* pTrackable = pManager->mCurGameCamera[0] == pManager->mDeathCamera[0]
                                        ? pManager->mDeathCamera[0]->mOwner
                                        : pManager->mChaseCamera[0]->mOwner;
            if (pTrackable)
            {
                GameObject* pTarget = pTrackable->GetGameObject();
                if (pTarget)
                {
                    const PblMatrix& mTarget = pTarget->GetMatrix();
                    float fDX = mWorld.m[3][0] - mTarget.m[3][0];
                    float fDY = mWorld.m[3][1] - mTarget.m[3][1];
                    float fDZ = mWorld.m[3][2] - mTarget.m[3][2];
                    float fRadius2 = mClass->mDeathShakeRadius * mClass->mDeathShakeRadius;
                    float fDist2 = fDX * fDX + fDY * fDY + fDZ * fDZ;
                    if (fRadius2 > fDist2)
                    {
                        float fScale = 1.0f - fDist2 / fRadius2;
                        CameraManager::sInstance->ApplyShake(mClass->mDeathShakeForce * fScale,
                                                             mClass->mDeathShakeDuration * fScale);
                    }
                }
            }
        }

        if (mClass->mDeathFadeTimeEnd > 0.0f)
        {
            if (mDeathTime > mClass->mDeathFadeTimeEnd)
            {
                return;
            }
            if (mDeathTime > mClass->mDeathFadeTimeStart)
            {
                fAlpha = (mClass->mDeathFadeTimeEnd - mDeathTime) /
                         (mClass->mDeathFadeTimeEnd - mClass->mDeathFadeTimeStart);
            }
        }
    }

    PblMatrix aTurretSave[8];
    if (fAlpha >= 0.8f)
    {
        for (int i = 0; i < mNumTurrets; ++i)
        {
            if (mTurret[i])
            {
                mTurret[i]->Render(mWorld, mInverse, mPose, uiFlags, aTurretSave[i]);
            }
        }
    }

    SetupShadow(this, mModel, ucViewport, uiFlags, 0);
    RedColor color = RedColor::WHITE;
    color.a = (unsigned char)(int)(fAlpha * 255.0f);
    mModel->Render(ucViewport, fDist, &mWorld, &mPose, color, uiFlags, 0);

    if (!netFrameLock && NetGame::IsEnabled())
    {
        mBlend->ResetTurnState();
        mLegPairs[0]->ResetTurnState();
        if (mLegPairCount > 1)
        {
            mLegPairs[1]->ResetTurnState();
        }
        BuildWorldMatrices();
    }

    if (fAlpha >= 0.8f)
    {
        for (int i = 0; i < mNumTurrets; ++i)
        {
            if (mTurret[i])
            {
                mTurret[i]->PostRender(aTurretSave[i]);
            }
        }
    }

    if (!(uiFlags & 0x230000) && mClass->mDropShadowSize > 0.0f)
    {
        RedDirectionalLight* pLight = RedLight::GetGlobalDirectionalLight(0);
        if (!pLight->IsShadowCaster() && RedLight::GetGlobalDirectionalLight(1)->IsShadowCaster())
        {
            pLight = RedLight::GetGlobalDirectionalLight(1);
        }
        const PblVector3& vDir = pLight->m_vDirection;
        PblVector3 vShadowNormal(0.0f - vDir.x * 0.2f, 0.8f - vDir.y * 0.2f, 0.0f - vDir.z * 0.2f);

        RedDropShadowManager::AddShadow(0x410f19a9, mWorld.GetTranslation(), vShadowNormal, vDir, 0.0f,
                                        mClass->mDropShadowSize, mClass->mDropShadowSize, fAlpha);
    }

    int iNumFeet = mClass->mNumFeet;
    if (iNumFeet > 0 && mClass->mStompDecal && fAlpha > 0.8f)
    {
        unsigned int uBit = 1;
        for (int i = 0; i < iNumFeet; ++i)
        {
            if (mFootState & uBit)
            {

                mCollisionModel->GetHeightFromMaskID(mClass->mFootPrimitives[i]);
                mCollisionModel->GetWorldMatrix(mClass->mFootPrimitives[i], mWorld, GetPose());
            }
            uBit = (uBit << 1) | (uBit >> 31);
        }
    }
}

// FUNCTION: SWBF2 0x00501a40
const ControllableClass* EntityWalker::GetControllableClass() const
{
    return mClass;
}

// FUNCTION: SWBF2 0x00501a60
ControllableClass* EntityWalker::GetControllableClass()
{
    return mClass;
}

// FUNCTION: SWBF2 0x00501ab0
int EntityWalker::GetWeaponCount() const
{
    return mClass->mWeaponCount;
}

// FUNCTION: SWBF2 0x00501ac0
PilotDeathType EntityWalker::GetPilotDeathType()
{
    return mClass->mPilotDeathType;
}

// FUNCTION: SWBF2 0x00501ad0
bool EntityWalker::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                     Restrictor* pRestrictor)
{
    Entity* pEntity = pOther->GetEntity();
    if (pEntity && pEntity->IsRtti(EntityCarrier::rttiHashEntityCarrier))
    {
        return true;
    }
    if ((1 << pResult->mColliderId) & mClass->mIgnorableCollisions)
    {
        return true;
    }

    bool bFoot = false;
    for (int i = 0; i < mClass->mNumFeet; ++i)
    {
        mCollisionModel->GetWorldMatrix(mClass->mFootPrimitives[i], mMatrix, GetPose());
        int iFoot = mClass->mFootPrimitives[i];
        if (pResult->mColliderId == iFoot ||
            mCollisionModel->GetParentNameFromMaskId(pResult->mColliderId) ==
                mCollisionModel->GetParentNameFromMaskId(iFoot))
        {
            bFoot = true;
        }
    }

    const PblVector3& vSeparation = pResult->mSeparationNormal;
    switch (pOther->GetCollisionType())
    {
    case COLL_SOFT:
        if (vSeparation.y > 0.9f && bFoot)
        {
            GameObject* pObject = pOther->GetGameObject();
            if (pObject && pObject->IsRtti(EntitySoldier::rttiHashEntitySoldier))
            {
                pObject->InstantDeath(DamageOwner(mCharacter, this, GetTeam(), mClass, 0, 1.0f), true);
            }
        }
        break;
    case COLL_TERRAIN:
        for (int i = 0; i < mLegPairCount; ++i)
        {
            if (mLegPairs[i]->FootTerrainCollision(pResult))
            {
                if (pRestrictor)
                {
                    pRestrictor->AddRestriction(PblVector3(0.0f, 1.0f, 0.0f));
                }
                return true;
            }
        }
        m_bIsGrounded = true;
        mVelocity.y = 0.0f;
        mVelocityNoGrav.y = 0.0f;
        break;
    }

    if (pOther->GetCollisionType() == COLL_RIGID)
    {
        if (AIUtil::ResolveCollision(GetGameObject(), pOther->GetGameObject()))
        {
            mStopTime = GameLoop::GetMissionTime();
        }
        PblVector3 vRelVel = mVelocity;
        if (pOther->GetGameObject())
        {
            const PblVector3& vOtherVel = pOther->GetGameObject()->GetVelocity();
            vRelVel.x -= vOtherVel.x;
            vRelVel.y -= vOtherVel.y;
            vRelVel.z -= vOtherVel.z;
        }
        float fApproach = vSeparation.y * vRelVel.y + vSeparation.x * vRelVel.x + vSeparation.z * vRelVel.z;
        if (vSeparation.y > 0.7071f)
        {
            m_bIsGrounded = true;
        }
        if (fApproach < 0.0f)
        {
            float fUp =
                mMatrix.m[1][1] * vSeparation.y + mMatrix.m[1][0] * vSeparation.x + mMatrix.m[1][2] * vSeparation.z;
            float fFront =
                mMatrix.m[2][1] * vSeparation.y + mMatrix.m[2][0] * vSeparation.x + mMatrix.m[2][2] * vSeparation.z;
            float fThreshold = mClass->mCollisionThreshold[1] * fUp + mClass->mCollisionThreshold[0] +
                               mClass->mCollisionThreshold[2] * fFront;

            FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(), pOther, &GetPosition());
            PlayImpactEffects(&mMatrix.GetTranslation(), &mVelocity);
            PblVector3 vPos = mMatrix.GetTranslation();
            PblVector3 vVel = mVelocity;
            VehicleEngine::Collision(mClass, vPos, vVel);

            if (-fThreshold > fApproach)
            {
                float fExcess = fThreshold + fApproach;
                float fDamage = (mClass->mCollisionScale[1] * fUp + mClass->mCollisionScale[0] +
                                 mClass->mCollisionScale[2] * fFront) *
                                fExcess * fExcess;
                if (pOther->GetGameObject())
                {
                    fDamage = pOther->GetGameObject()->GetGameObjectClass()->GetCollisionInflict() * fDamage;
                }
                if (fDamage > 0.0f)
                {
                    DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
                    if (pOther->GetGameObject() && pOther->GetGameObject()->GetControllable())
                    {
                        owner.mCharacter = pOther->GetGameObject()->GetControllable()->mCharacter;
                        owner.mGameObject = pOther->GetGameObject();
                        owner.mTeam = pOther->GetGameObject()->GetTeam();
                        owner.mClass = pOther->GetGameObject()->GetGameObjectClass();
                    }
                    GameObject* pObject = pOther->GetGameObject();
                    if (!pObject || (!pObject->IsRtti(EntityHover::rttiHashEntityHover) &&
                                     !pObject->IsRtti(EntityWalker::rttiHashEntityWalker)))
                    {
                        Damageable::ApplyDamage(DamageDesc(owner, fDamage), 0xffffffff);
                    }
                }
            }
        }
    }

    if (pOther->GetCollisionType() == COLL_STATIC)
    {
        SmashParkedFlyer(this, pOther, pEntity);
        if (!(0.2f > vSeparation.y) && bFoot)
        {
            mVelocity.y = 0.0f;
            mVelocityNoGrav.y = 0.0f;
            if (vSeparation.y > 0.7071f)
            {
                m_bIsGrounded = true;
            }
            return false;
        }
        float fOverlap = pResult->mOverlap;
        float fHorizontal = sqrtf((vSeparation.x * fOverlap) * (vSeparation.x * fOverlap) +
                                  (vSeparation.z * fOverlap) * (vSeparation.z * fOverlap));
        if (fHorizontal > 0.0f)
        {
            float fDist = fOverlap * fOverlap / fHorizontal;
            PblVector3 vFlat(vSeparation.x, 0.0f, vSeparation.z);
            PblVector3 vDir;
            D3DXVec3Normalize(&vDir, &vFlat);
            float fMax = fHorizontal * 2.0f;
            if (!(fMax > fDist))
            {
                fDist = fMax;
            }
            RestrictObject(vDir, fDist, pRestrictor);
            if (pRestrictor)
            {
                pRestrictor->AddRestriction(vSeparation);
            }
            SmashParkedFlyer(this, pOther, pEntity);
            return true;
        }
    }

    bool bResult = CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
    if (bResult && pRestrictor)
    {
        pRestrictor->AddRestriction(vSeparation);
    }
    return bResult;
}

// FUNCTION: SWBF2 0x00502400
void EntityWalker::SetTerrainHeight(int iIndex, float fHeight)
{
    for (int i = 0; i < mLegPairCount; ++i)
    {
        mLegPairs[i]->SetTerrainHeight(iIndex, fHeight);
    }
}

// FUNCTION: SWBF2 0x00502440
void EntityWalker::Die()
{
    mIsDead = true;
    CollisionManager::RemoveBody(this);
    AddStaticToCollisionManager();
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mWeapon[i])
        {
            mWeapon[i]->DeactivateThread();
        }
    }
    if (mClass->mExplosion && !mExplodedDeath)
    {
        mExplodedDeath = true;
        DamageOwner owner(mDamageOwner.mCharacter, mDamageOwner.mGameObject, mDamageOwner.mTeam, mClass, 0, 1.0f);
        PblVector3 vPos;
        D3DXVec3TransformCoord(&vPos, &mClass->mExplosionOffset, &mMatrix);
        mClass->mExplosion->CreateExplosion(vPos, owner, false, COLL_UNKNOWN);
    }
}

// FUNCTION: SWBF2 0x00502560
void EntityWalker::UpdatePilot()
{
    if (mPilotType == PILOT_SELF || (mPilotType == PILOT_VEHICLESELF && mPilot == this))
    {
        return;
    }
    Controllable* pPilot = mPilot;
    if (!pPilot)
    {
        return;
    }
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    if (mControlUse.IsPressed() || IsDisabled())
    {
        if (mPilotType == PILOT_REMOTE)
        {
            pPilot->ExitRemote();
        }
        else if (mPilotType == PILOT_VEHICLE || mPilotType == PILOT_VEHICLESELF)
        {
            if (pPilot->mCharacter && pPilot->mCharacter->mRemote)
            {
                pPilot->ExitRemote();
            }
            pPilot->ExitVehicle(true, true);
        }
    }
    else if (GetControllableList().GetCount() > 1 && mControlSwitch[0] != 0.0f)
    {
        pPilot->CyclePosition(mControlSwitch[0] > 0.0f);
    }
}

// FUNCTION: SWBF2 0x00502670
void EntityWalker::UpdatePlayer()
{
    if (mPlayerId < 0 || !IsAlive())
    {
        return;
    }

    if (mControlView.IsPressed() && netIsLocalTurn && !IsForcedThirdPerson() && !IsForcedFirstPerson() &&
        mClass->mForceMode != 1 && mClass->mForceMode != 2 && mTracker)
    {
        SwitchControlView(mPlayerId);
    }

    int iIndex = mWeaponIndex[0];
    if (iIndex >= 0 && mWeapon[iIndex] && mWeapon[iIndex]->mClass->mZoomMax > 1.0f && mControlZoom.IsPressed())
    {
        mTargetInfo.mIsAiming = mWeapon[iIndex]->CycleZoom(mTargetInfo.mIsAiming);
    }

    if (mPlayerId >= 0 && mTracker && mTracker->IsFirstPersonView())
    {
        int iJoystick = GetJoystickIndex();
        if (iJoystick >= 0)
        {
            FirstPerson::Update(iJoystick, TYPE_COCKPIT, mClass->mFirstPersonModelHandle, this, 0);
        }
    }
}

// FUNCTION: SWBF2 0x00502820
int EntityWalker::GetJoystickIndex()
{
    if (mPlayerId < 0)
    {
        return -1;
    }
    if (!NetGame::IsEnabled() || NetGame::IsClient() ||
        (NetGame::IsHost() && NetGame::IsOneWorld() && !NetGame::IsDedicated()))
    {
        return NetGame::GetJoystickIndex(mPlayerId);
    }
    return -1;
}

// FUNCTION: SWBF2 0x005053d0
void EntityWalker::MoveForward(float fDistance)
{
    PblVector3 vPos = mMatrix.GetTranslation();
    float fAngle = 0.0f;
    if (!sBipedHack)
    {
        fAngle = mBodyPitch * 2.0f;
    }
    float fCos = cosf(fAngle);
    float fSin = sinf(fAngle);
    if (1.0f > mTurnOffset.mCos)
    {
        float fTurnCos = mTurnOffset.mCos * mTurnOffset.mCos - mTurnOffset.mSin * mTurnOffset.mSin;
        float fTurnSin = mTurnOffset.mCos * 2.0f * mTurnOffset.mSin;

        PblMatrix mTurn(
            PblVector4{ fTurnCos, 0.0f, -fTurnSin, 0.0f },
            PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
            PblVector4{ fTurnSin, 0.0f, fTurnCos, 0.0f },
            PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });
        D3DXMatrixMultiply(&mTurn, &mTurn, &mMatrix);
        vPos.x = (mTurn.m[2][0] * fCos - mTurn.m[1][0] * fSin) * fDistance + vPos.x;
        vPos.y = vPos.y + (mTurn.m[2][1] * fCos - mTurn.m[1][1] * fSin) * fDistance;
        vPos.z = vPos.z + (mTurn.m[2][2] * fCos - mTurn.m[1][2] * fSin) * fDistance;
    }
    else
    {
        vPos.x = (fCos * mMatrix.m[2][0] - fSin * mMatrix.m[1][0]) * fDistance + vPos.x;
        vPos.y = (mMatrix.m[2][1] * fCos - mMatrix.m[1][1] * fSin) * fDistance + vPos.y;
        vPos.z = (mMatrix.m[2][2] * fCos - mMatrix.m[1][2] * fSin) * fDistance + vPos.z;
    }
    SetPosition(vPos);
}

// FUNCTION: SWBF2 0x00505670
void EntityWalker::UpdateOrientation(float fDeltaTime)
{
    float fMaxPush = -1.0f;
    float fHeightDifference = 0.0f;
    for (int i = 0; i < mLegPairCount; ++i)
    {
        WalkerLegPair* pLegPair = mLegPairs[i];
        PblVector3 vFeet[2];
        vFeet[0] = mCollisionModel->GetLocalMatrix(pLegPair->mFootCollisionBody[0], GetPose()).GetTranslation();
        vFeet[1] = mCollisionModel->GetLocalMatrix(pLegPair->mFootCollisionBody[1], GetPose()).GetTranslation();
        pLegPair->SetupGroundCollision(mMatrix, mBodyPitch, vFeet);
        if (pLegPair->mPushValue > fMaxPush)
        {
            fMaxPush = pLegPair->mPushValue;
        }
        fHeightDifference += pLegPair->mFootHeightDifference / (float)mLegPairCount;
        pLegPair->SetInterpolate(false);
    }

    float fGravity = m_bIsJumping ? -9.8f : -29.4f;
    if (mLegPairCount <= 1)
    {
        if (fMaxPush >= 0.0f)
        {
            float fLift = fMaxPush - 0.1f;
            if (fLift > 0.0f)
            {
                SetPosition(PblVector3(mMatrix.m[3][0] + 0.0f, mMatrix.m[3][1] + fLift, mMatrix.m[3][2] + 0.0f));
            }
            m_bIsGrounded = true;
            mVelocityNoGrav.y = 0.0f;
            mVelocity.y = 0.0f;
        }
        else
        {
            mVelocity.y = fGravity * fDeltaTime + mVelocity.y;
        }
    }
    else if (sBipedHack)
    {
        if (fMaxPush > 0.0f)
        {
            mBodyPitch = (-0.62f - mBodyPitch) * 0.05f + mBodyPitch;
            mPitchOffset = PblAngle(mBodyPitch);
            SetPosition(PblVector3(mMatrix.m[3][0] + 0.0f, mMatrix.m[3][1] + fMaxPush, mMatrix.m[3][2] + 0.0f));
            m_bIsGrounded = true;
            mVelocityNoGrav.y = 0.0f;
            mVelocity.y = 0.0f;
        }
        else
        {
            mVelocity.y = fGravity * fDeltaTime + mVelocity.y;
        }
    }
    else
    {
        if (fMaxPush > 0.0f)
        {
            float fPushDifference = mLegPairs[1]->mPushValue - mLegPairs[0]->mPushValue;
            if (fPushDifference != 0.0f)
            {
                float fPitch = fPushDifference * 0.01f + mBodyPitch;
                if (-0.29552022f > fPitch)
                {
                    fPitch = -0.29552022f;
                }
                else if (fPitch > 0.29552022f)
                {
                    fPitch = 0.29552022f;
                }
                mBodyPitch = fPitch;
                mPitchOffset = PblAngle(fPitch);
            }
            float fPush0 = mLegPairs[0]->mPushValue;
            if (!(fPush0 > -0.1f))
            {
                fPush0 = -0.1f;
            }
            float fPush1 = mLegPairs[1]->mPushValue;
            if (!(fPush1 > -0.1f))
            {
                fPush1 = -0.1f;
            }
            float fLift = (fPush1 + fPush0) * 0.5f;
            if (fLift != 0.0f)
            {
                SetPosition(PblVector3(mMatrix.m[3][0] + 0.0f, mMatrix.m[3][1] + fLift, mMatrix.m[3][2] + 0.0f));
            }
            m_bIsGrounded = true;
            mVelocity.y = 0.0f;
            mVelocityNoGrav.y = 0.0f;
        }
        else
        {
            mVelocity.y = fGravity * fDeltaTime + mVelocity.y;
        }

        if (mClass->mOrientRoll && (fHeightDifference > 0.0001f || -0.0001f > fHeightDifference))
        {
            float fInvLength = 1.0f / (float)sqrt(mClass->mWidth * mClass->mWidth +
                                                  fHeightDifference * fHeightDifference);
            float fTargetCos = mClass->mWidth * fInvLength;
            float fTargetSin = fInvLength * fHeightDifference;
            float fDelta = mRollOffset.mCos * fTargetSin - mRollOffset.mSin * fTargetCos;
            float fSin = fDelta * mRollOffset.mCos * 0.05f + mRollOffset.mSin;
            float fCos = mRollOffset.mCos - fDelta * mRollOffset.mSin * 0.05f;
            mRollOffset.mSin = fSin;
            mRollOffset.mCos = fCos;
            mRollOffset.Renormalize();
            if (mRollOffset.mSin > 0.29552022f)
            {
                mRollOffset.mCos = 0.95533651f;
                mRollOffset.mSin = 0.29552022f;
            }
            else if (-0.29552022f > mRollOffset.mSin)
            {
                mRollOffset.mCos = 0.95533651f;
                mRollOffset.mSin = -0.29552022f;
            }
        }
    }

    mVelocityNoGrav = mVelocity;
    mVelocityNoGrav.y = 0.0f;
}

// FUNCTION: SWBF2 0x005074d0
void EntityWalker::ActivatePhysics()
{
    ActivateThread(0, PER_TURN);
    mPostCollision.ActivateThread(-15, PER_TURN);
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mAimer[i])
        {
            mAimer[i]->ActivatePhysics();
        }
    }
    for (int i = 0; i < mNumTurrets; ++i)
    {
        if (mTurret[i])
        {
            mTurret[i]->ActivatePhysics();
        }
    }
    for (int i = 0; i < mClass->mNumPassengerSlots; ++i)
    {
        if (mPassengerSlots[i])
        {
            mPassengerSlots[i]->ActivatePhysics();
        }
    }
}

extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);

extern bool netIsLateTurn;
extern PblVector3 gVectorZero;

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

Controllable* GetControllablePilot(Controllable* pControllable);

namespace SquadCommands { void VehicleCommand(GameObject* pObject, Character* pCharacter); }

static const float ATTACK_CONTROL_DELAY = 0.1f;
static const float ATTACK_REPEAT_DELAY = 0.3f;

// GLOBAL: SWBF2 0x007e6520
EntityWalker::StateTableEntry EntityWalker::sStateTable[StateCount] = {
    {{Idle, Idle, IdleToLeft, LeftToIdle, TurnLeft, TurnRight, Dying}},
    {{Idle, Idle, IdleToLeft, LeftToIdle, Idle, Idle, Dying}},
    {{Idle, Idle, IdleToLeft, RightToIdle, Idle, Idle, Dying}},
    {{Dead, Dead, Dead, Dead, Dead, Dead, Dead}},
    {{Dead, Dead, Dead, Dead, Dead, Dead, Dead}},
    {{LeftToIdle, Idle, LeftToRight, RightToIdle, LeftToIdle, LeftToIdle, Dying}},
    {{RightToIdle, Idle, RightToLeft, LeftToIdle, RightToIdle, RightToIdle, Dying}},
    {{RightToIdle, IdleToLeft, RightToLeft, RightToLeft, RightToIdle, RightToIdle, Dying}},
    {{LeftToIdle, IdleToRight, LeftToRight, LeftToRight, LeftToIdle, LeftToIdle, Dying}},
    {{Idle, IdleToLeft, IdleToLeft, RightToLeft, TurnLeft, TurnRight, Dying}},
    {{Idle, IdleToRight, IdleToLeft, LeftToRight, TurnLeft, TurnRight, Dying}},
};

// FUNCTION: SWBF2 0x00503900
void EntityWalker::ApplyAttackDamage(const PblVector3& vPosition, float fRadius, float fDamage, float fPush) const
{
    PblVector3 vAttack;
    D3DXVec3TransformCoord(&vAttack, &vPosition, &mMatrix);
    GameObject* pObjects[10];
    const int iCount = CollisionManager::GetObjectsInRange(vAttack, fRadius, pObjects, 10, this, 0x3a);
    if (!iCount)
    {
        return;
    }

    DamageDesc damage(mDamageOwner, fDamage);
    PblVector3 vDir;
    PblVector3 vNormal;
    D3DXVec3TransformNormal(&vNormal, &vPosition, &mMatrix);
    D3DXVec3Normalize(&vDir, &vNormal);
    for (int i = 0; i < iCount; ++i)
    {
        const PblVector3& vCenter = pObjects[i]->GetCollisionCenter();
        PblVector3 vDelta;
        vDelta.x = vCenter.x - (vAttack.x - vDir.x * fRadius * 1.5f);
        vDelta.y = vCenter.y - (vAttack.y - vDir.y * fRadius * 1.5f);
        vDelta.z = vCenter.z - (vAttack.z - vDir.z * fRadius * 1.5f);
        D3DXVec3Normalize(&vNormal, &vDelta);
        if (fDamage > 0.0f)
        {
            static_cast<Damageable*>(pObjects[i])->ApplyDamage(damage, vNormal, pObjects[i]->GetCollisionCenter(),
                                                               -1, 0xffffffff);
        }
        if (fPush > 0.0f)
        {
            pObjects[i]->ApplyPush(vNormal, fPush, false);
        }
    }
}

// FUNCTION: SWBF2 0x00503ae0
void EntityWalker::SetState(int iState)
{
    if (iState == Dead)
    {
        mState = Dead;
        return;
    }
    if (iState <= Dead || iState >= StateCount)
    {
        mDirection = None;
    }

    WalkerAnimation* pAnim = mClass->mAnimations[iState];
    pAnim->Setup(*mBlend, 0.0f,
                 mClass->GetAnimationBlendDuration((eState)mState, (eState)iState), false,
                 mDirection == Backward);
    for (int i = 0; i < mLegPairCount; ++i)
    {
        pAnim->Setup(*mLegPairs[i]->mBlend, 0.0f, 0.0f, false, mDirection == Backward);
    }

    if (mState != iState)
    {
        mBlend->mUnits[0]->mWeight = mState != Idle ? 1.0f : 0.0f;
        for (int i = 1; i < mBlend->mCount; ++i)
        {
            mBlend->mUnits[i]->mWeight = 0.0f;
        }
        mBlend->SetJointMask(0, mMainMask);
        mBlend->SetJointMask(1, mMainMask);
        mBlend->SetJointMask(2, mMainMask);
        for (int i = 0; i < mLegPairCount; ++i)
        {
            mLegPairs[i]->StateChanged(iState);
        }
    }
    BuildWorldMatrices();
    mState = iState;
}

// FUNCTION: SWBF2 0x00503cc0
void EntityWalker::NextState(int iInput)
{
    switch (iInput)
    {
    case ToIdle:
        if (mDirection == Backward)
        {
            iInput = ToIdleBackwards;
        }
        break;
    case ToForward:
        mDirection = Forward;
        break;
    case ToBackward:
        mDirection = Backward;
        break;
    }
    SetState(sStateTable[mState < StateCount ? mState : Idle].next[iInput]);
}

// FUNCTION: SWBF2 0x00503d30
void EntityWalker::ResetRotation()
{
    mTurnOffset -= PblAngle(1.0f, mTurnOffset.mSin * 0.1f);
    mTurnOffset.Renormalize();
}

// FUNCTION: SWBF2 0x00503dd0
void EntityWalker::AdjustRotation(PblAngle angle)
{
    RotateY(PblAngle(angle.mCos, -angle.mSin));
    mTurnOffset += angle;
}

// FUNCTION: SWBF2 0x00503e50
void EntityWalker::CheckTurn(float fMove, float fStrafe)
{
    float fThreshold;
    if (mDirection != None || (fMove == 0.0f && fStrafe == 0.0f))
    {
        fThreshold = mClass->mTurnThreshold;
    }
    else
    {
        fThreshold = 0.1f;
    }

    if (mTurnOffset.mSin > fThreshold)
    {
        mTotalTurn = -atan2f(mTurnOffset.mSin, mTurnOffset.mCos);
        NextState(ToRight);
    }
    else if (-fThreshold > mTurnOffset.mSin)
    {
        mTotalTurn = -atan2f(mTurnOffset.mSin, mTurnOffset.mCos);
        NextState(ToLeft);
    }
    else if (mState != Idle)
    {
        NextState(ToIdle);
    }
}

// FUNCTION: SWBF2 0x00503f70
void EntityWalker::ApplyBlend()
{
    mBlend->Blend(mZephyrPoseStatic);
    for (int i = 0; i < mLegPairCount; ++i)
    {
        mLegPairs[i]->ApplyBlend();
    }
}

// FUNCTION: SWBF2 0x00503fb0
void EntityWalker::CorrectJoints()
{
    if (mBodyPitch != 0.0f)
    {
        for (int i = 0; i < mLegPairCount; ++i)
        {
            mLegPairs[i]->CorrectJoints();
        }
    }

    ZephyrTransform* pJoints = mZephyrPoseStatic.m_pJoints;
    pJoints[0].kQuat = pJoints[0].kQuat * PblQuaternion(0.0f, mTurnOffset.mSin, 0.0f, mTurnOffset.mCos);

    unsigned short uiRoot = (unsigned short)mZephyrSkeleton.m_pShared->GetJointIdxByCrc(0x16e27226);
    ZephyrTransform* pRoot = &pJoints[uiRoot];
    if (mClass->mSkeletonRootScale != 1.0f)
    {
        mZephyrSkeleton.m_pShared->SetJointScale(uiRoot, mClass->mSkeletonRootScale);
    }
    if (mBodyPitch != 0.0f && pRoot)
    {
        pRoot->kQuat = pRoot->kQuat * PblQuaternion(mPitchOffset.mSin, 0.0f, 0.0f, mPitchOffset.mCos);
    }
    if (mRollOffset.mCos < 1.0f && pRoot)
    {
        pRoot->kQuat = pRoot->kQuat * PblQuaternion(0.0f, 0.0f, mRollOffset.mSin, mRollOffset.mCos);
    }
}

// FUNCTION: SWBF2 0x00504170
void EntityWalker::RestoreJoints()
{
    ZephyrTransform* pJoints = mZephyrPoseStatic.m_pJoints;
    unsigned short uiRoot = (unsigned short)mZephyrSkeleton.m_pShared->GetJointIdxByCrc(0x16e27226);
    ZephyrTransform* pRoot = &pJoints[uiRoot];
    if (mClass->mSkeletonRootScale != 1.0f)
    {
        mZephyrSkeleton.m_pShared->SetJointScale(uiRoot, 1.0f);
    }
    if (mRollOffset.mCos < 1.0f && pRoot)
    {
        pRoot->kQuat = pRoot->kQuat * PblQuaternion(0.0f, 0.0f, -mRollOffset.mSin, mRollOffset.mCos);
    }
    if (mBodyPitch != 0.0f && pRoot)
    {
        pRoot->kQuat = pRoot->kQuat * PblQuaternion(-mPitchOffset.mSin, 0.0f, 0.0f, mPitchOffset.mCos);
    }
    pJoints[0].kQuat =
        pJoints[0].kQuat * PblQuaternion(0.0f, mTurnOffset.mSin, 0.0f, mTurnOffset.mCos).Inverse();

    if (mBodyPitch != 0.0f)
    {
        for (int i = 0; i < mLegPairCount; ++i)
        {
            mLegPairs[i]->RestoreJoints();
        }
    }
}

// FUNCTION: SWBF2 0x00504350
void EntityWalker::UpdateDone()
{
    for (int i = 0; i < mLegPairCount; ++i)
    {
        mLegPairs[i]->UpdateDone();
    }
}

// FUNCTION: SWBF2 0x00504380
void EntityWalker::BuildWorldMatrices()
{
    ApplyBlend();
    CorrectJoints();
    mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
    RestoreJoints();
}

// FUNCTION: SWBF2 0x00502890
void EntityWalker::UpdateState(float fDeltaTime, float fMove, float fStrafe, float& rTurnSpeed, float fTurn)
{
    const float fFrameTime = fDeltaTime;
    float fAbsMove = fabsf(fMove);
    float fAbsStrafe = fabsf(fStrafe);

    unsigned int uiAttackControls = mAttackControlTimer >= ATTACK_CONTROL_DELAY ? m_uiAttackControls : 0;
    bool bAttacking = uiAttackControls != 0 || m_uiAttackAnim != 0;
    EntityWalkerClass* pClass = mClass;
    bool bStopped = fAbsStrafe >= pClass->m_fThrustAngleToStop_Tan * fAbsMove ||
                    fMove * fMove + fStrafe * fStrafe <= 0.01f;
    bool bWalk = m_bIsJumping || (!bStopped && !bAttacking);
    if (mPlayerId >= 0 && !bWalk && !pClass->mSteerAtStrafeLimit && pClass->mCorrectAimForSteering &&
        fAbsStrafe > 0.1f)
    {
        float fTan = pClass->m_fThrustAngleToStop_Tan;
        fAbsMove = 1.0f / sqrtf(fTan * fTan + 1.0f);
        fMove = fAbsMove;
        if (mDirection == Backward)
        {
            fMove = -fAbsMove;
        }
        bStopped = false;
        bWalk = true;
    }

    rTurnSpeed = pClass->mStoppedTurnSpeed;
    float fSpeed = mClass->mMaxSpeed;
    float fDistance = 0.0f;
    if (m_bIsGrounded)
    {
        m_fGroundedTimer = 0.0f;
        m_bIsJumping = false;
    }
    else
    {
        m_fGroundedTimer += fDeltaTime;
    }
    if (!bAttacking && mControlJump.IsPressed())
    {
        Jump();
    }

    EnergyBar* pEnergyBar = reinterpret_cast<EnergyBar*>(mEnergyBar);
    if (m_bIsJumping)
    {
        mAttackControlTimer = 0.0f;
        m_uiAttackControls = 0;
        m_uiAttackAnim = 0;
        fSpeed = mVelocity.x * mMatrix.m[2][0] + mVelocity.y * mMatrix.m[2][1] + mVelocity.z * mMatrix.m[2][2];
        rTurnSpeed = mClass->mBoostTurnSpeed;
        mBoost = false;
    }
    else
    {
        bool bBoost = false;
        if (mClass->mBoostSpeed > mClass->mMaxSpeed && !bAttacking && mDirection == Forward && fMove > 0.0f &&
            fMove * fMove + fStrafe * fStrafe > 0.81f)
        {
            if (mPlayerId >= 0)
            {
                if (mBoost)
                {
                    bBoost = !mControlSprint.Tapped() && !mControlSprint.HeldAndReleased();
                }
                else
                {
                    bBoost = mControlThrustFwd.TappedAndPressed() || mControlSprint.Tapped() ||
                             mControlSprint.Held();
                }
            }
            else
            {
                bBoost = mControlSprint.IsDown() ||
                         (mCtrl && UnitControllerPtr(mCtrl)->GetLowLevel().GetHeight() == HEIGHT_SPRINT);
            }
        }
        if (bBoost && pEnergyBar->SpendEnergy(mClass->mEnergyBoostDrain * fDeltaTime))
        {
            fSpeed = mClass->mBoostSpeed;
            rTurnSpeed = mClass->mBoostTurnSpeed;
            mBoost = true;
        }
        else
        {
            mBoost = false;
            float fMultiplier = mCharacter && mCharacter->mMedalsMgr &&
                                        mCharacter->mMedalsMgr->IsAwardAvailable(MedalsMgr::ENDURANCE)
                                    ? 1.5f
                                    : 1.0f;
            pEnergyBar->AddEnergy(mClass->mEnergyAutoRestore * fMultiplier * fDeltaTime);
            if (bAttacking)
            {
                rTurnSpeed = 0.0f;
            }
            else if (bWalk)
            {
                rTurnSpeed = mClass->mForwardTurnSpeed;
            }
        }
    }

    if (mPlayerId >= 0 && mClass->mReverseBackwardSteering && !bStopped && fMove < 0.0f)
    {
        fStrafe = -fStrafe;
    }

    float fHeading = atan2f(mTurnOffset.mSin, mTurnOffset.mCos) * 2.0f;
    pClass = mClass;
    float fMinYaw = pClass->mMinYaw;
    float fMaxYaw = pClass->mMaxYaw;
    if (fTurn > 0.0f)
    {
        fMinYaw = (1.0f - fTurn * 0.8f) * fMinYaw;
    }
    else
    {
        fMaxYaw = (fTurn * 0.8f + 1.0f) * fMaxYaw;
    }
    fMinYaw += fHeading;
    fMaxYaw += fHeading;
    float fMaxTurn = fDeltaTime * rTurnSpeed;

    float fYaw;
    bool bDefault = true;
    if (mPlayerId >= 0)
    {
        if (pClass->mSteerAtYawLimit && (fMinYaw > mYawValue || mYawValue > fMaxYaw))
        {
            fYaw = ((fMinYaw > mYawValue ? fMinYaw : fMaxYaw) - mYawValue) * 2.0f;
            bDefault = false;
        }
        else if (pClass->mCorrectAimForSteering)
        {
            if (bWalk)
            {
                if ((mYawValue <= 0.0f && fAbsMove * pClass->m_fThrustAngleToStrafe_Tan > -fStrafe) ||
                    (mYawValue >= 0.0f && fAbsMove * pClass->m_fThrustAngleToStrafe_Tan > fStrafe))
                {
                    fYaw = (atan2f(fStrafe, fAbsMove) - mYawValue) * 0.95f;
                    bDefault = false;
                }
                else if (!pClass->mSteerAtStrafeLimit)
                {
                    fYaw = (atan2f(fStrafe * pClass->m_fThrustAngleToStrafe_Tan, fAbsStrafe) - mYawValue) * 0.95f;
                    bDefault = false;
                }
            }
            else if (!pClass->mSteerAtStrafeLimit)
            {
                if (fAbsStrafe > 0.1f)
                {
                    fYaw = (atan2f(fStrafe * pClass->m_fThrustAngleToStrafe_Tan, fAbsStrafe) - mYawValue) * 0.95f;
                }
                else
                {
                    fYaw = 0.0f;
                }
                bDefault = false;
            }
        }
    }
    if (bDefault)
    {
        fYaw = fMaxTurn * fStrafe;
        if (bWalk && pClass->m_fThrustAngleToTurn_Tan * fAbsMove > fAbsStrafe)
        {
            float fFraction = (fAbsStrafe / fAbsMove - pClass->m_fThrustAngleToStrafe_Tan) /
                              (pClass->m_fThrustAngleToTurn_Tan - pClass->m_fThrustAngleToStrafe_Tan);
            if (-fMaxTurn > fYaw)
            {
                fYaw = -fMaxTurn;
            }
            else if (fYaw > fMaxTurn)
            {
                fYaw = fMaxTurn;
            }
            fYaw = fYaw * fFraction;
        }
        if (pClass->m_fSteerTowardAimRate * fAbsStrafe > 0.0f)
        {
            fYaw -= (1.0f - fAbsStrafe) * pClass->m_fSteerTowardAimRate * mYawValue;
        }
    }

    if (!pClass->mSteerAtStrafeLimit && fTurn == 0.0f &&
        (mDirection == None || (mDirection == Forward && fMove < 0.5f) || (mDirection == Backward && fMove > -0.5f)) &&
        (pClass->mMinYaw * 0.5f >= fYaw || fYaw >= pClass->mMaxYaw * 0.5f))
    {
        bWalk = false;
    }
    if (fYaw != 0.0f)
    {
        if (-fMaxTurn > fYaw)
        {
            fYaw = -fMaxTurn;
        }
        else if (fYaw > fMaxTurn)
        {
            fYaw = fMaxTurn;
        }
    }
    if (mPlayerId >= 0 && pClass->mCorrectAimForSteering)
    {
        mYawValue = fYaw * 2.0f + mYawValue;
    }
    AdjustRotation(PblAngle(fYaw));

    if (mState != Idle)
    {
        DoFootImpactEffects();
    }
    else if (mIsDead)
    {
        NextState(ToDying);
        m_uiAttackControls = 0;
        m_uiAttackAnim = 0;
        mAttackControlTimer = 0.0f;
        mFootState = 0;
    }
    else if (m_uiAttackAnim != 0)
    {
        mFootState = 0;
    }
    else if (uiAttackControls != 0)
    {

        int iAttack = 0;
        for (int i = 0; i < mClass->m_uiNumAttackAnims; ++i)
        {
            unsigned int uiControls = mClass->m_aAttackAnims[i].uiControls;
            if (uiControls == uiAttackControls)
            {
                iAttack = i;
                break;
            }
            if ((uiControls & uiAttackControls) == uiControls)
            {
                iAttack = i;
            }
        }
        mAttackControlTimer = -ATTACK_REPEAT_DELAY;
        m_uiAttackAnim = iAttack + 1;
        m_uiAttackControls = 0;
        SetState(StateCount + iAttack);
        mFootState = 0;
    }
    else if (!bWalk)
    {
        CheckTurn(fMove, fStrafe);
        mFootState = 0;
    }
    else
    {
        NextState(fMove >= 0.0f || m_bIsJumping ? ToForward : ToBackward);
        mFootState = 0;
    }

    if (fDeltaTime > 0.0f)
    {
        float fRootScale = mClass->mSkeletonRootScale;
        if (m_bIsJumping)
        {

            if (mBlend->IsFullWalkAnim())
            {
                if (mBlend->GetTimeRemaining() < 0.0001f)
                {
                    fDeltaTime = 0.0f;
                }
                else
                {
                    float fAnimSpeed = fRootScale * mBlend->GetSpeed();
                    if (fSpeed > fAnimSpeed && fAnimSpeed > 0.0f)
                    {
                        fDeltaTime = fSpeed / fAnimSpeed * fDeltaTime;
                    }
                    if (fDeltaTime > mBlend->GetTimeRemaining() - 0.0001f)
                    {
                        fDeltaTime = mBlend->GetTimeRemaining() - 0.0001f;
                    }
                }
            }
        }
        else if (bWalk && mDirection != None)
        {
            if (mDirection == Forward)
            {
                if (fMove < 0.0f)
                {
                    mBlend->Reverse();
                    for (int i = 0; i < mLegPairCount; ++i)
                    {
                        mLegPairs[i]->GetBlend()->Reverse();
                    }
                    mDirection = Backward;
                }
            }
            else if (fMove > 0.0f)
            {
                mBlend->Reverse();
                for (int i = 0; i < mLegPairCount; ++i)
                {
                    mLegPairs[i]->GetBlend()->Reverse();
                }
                mDirection = Forward;
            }

            float fWalkSpeed = fRootScale * mClass->mAnimations[LeftToRight]->mSpeed;
            float fInput = sqrtf(fMove * fMove + fStrafe * fStrafe) * fSpeed;
            if (fInput > 0.5f * fWalkSpeed)
            {
                fDeltaTime = fInput / fWalkSpeed * fDeltaTime;
            }
            else
            {
                fDeltaTime = 0.5f * fDeltaTime;
            }
        }
        else if (m_uiAttackAnim != 0)
        {
            const EntityWalkerClass::WalkerAttackAnim& attack = mClass->m_aAttackAnims[m_uiAttackAnim - 1];
            float fTime = mBlend->GetTime();
            if (attack.fTime >= fTime && fTime + fDeltaTime > attack.fTime)
            {
                ApplyAttackDamage(attack.vPosition, attack.fRadius, attack.fDamage, attack.fPush);
            }
        }
        else if (uiAttackControls != 0)
        {
            fDeltaTime = fDeltaTime * 4.0f;
        }

        float fRemaining = mBlend->GetTimeRemaining();
        if (mState == Dying)
        {
            fRemaining = mClass->mDeathFadeTimeEnd + fRemaining;
        }
        if (fDeltaTime >= fRemaining)
        {

            mBlend->Update(fRemaining);
            for (int i = 0; i < mLegPairCount; ++i)
            {
                mLegPairs[i]->Update(fRemaining);
            }
            if (!m_bIsJumping)
            {
                float fStep = mBlend->GetMovement().z * fRootScale;
                fDistance = fStep + fDistance;
                MoveForward(fStep);
            }
            if (mState == TurnLeft || mState == TurnRight)
            {
                mTurnOffset += PblAngle(fRemaining * mTotalTurn / mClass->mAnimations[mState]->mTimeTotal);
            }
            fDeltaTime -= fRemaining;
            m_uiAttackAnim = 0;
            if (mIsDead)
            {
                NextState(ToDying);
            }
            else if (bWalk && mDirection == Forward)
            {
                NextState(ToForward);
            }
            else if (bWalk && mDirection == Backward)
            {
                NextState(ToBackward);
            }
            else
            {
                NextState(ToIdle);
            }
        }

        if (mBlend->GetReversed() && fDeltaTime > mBlend->GetTimeRemaining())
        {
            fDeltaTime = mBlend->GetTimeRemaining();
        }
        for (int i = 0; i < mLegPairCount; ++i)
        {
            mLegPairs[i]->Update(fDeltaTime);
        }
        mBlend->Update(fDeltaTime);
        if (!m_bIsJumping)
        {
            float fStep = mBlend->GetMovement().z * fRootScale;
            float fWalkSpeed = (fStep + fDistance) / fFrameTime;
            mVelocity.x = mMatrix.m[2][0] * fWalkSpeed;
            mVelocity.z = mMatrix.m[2][2] * fWalkSpeed;
            MoveForward(fStep);
        }
        if (mState != Idle && mState < StateCount)
        {
            if (mState > TurnRight)
            {
                ResetRotation();
            }
            else
            {
                mTurnOffset += PblAngle(fDeltaTime * mTotalTurn / mClass->mAnimations[mState]->mTimeTotal);
            }
        }
    }

    if (m_bIsJumping)
    {
        PblVector3 vStep(mVelocity.x * fFrameTime, mVelocity.y * fFrameTime, mVelocity.z * fFrameTime);
        const PblVector3& vPos = GetPosition();
        SetPosition(PblVector3(vStep.x + vPos.x, vPos.y + vStep.y, vPos.z + vStep.z));
    }
    BuildWorldMatrices();
    m_bIsGrounded = false;
}

// FUNCTION: SWBF2 0x005043b0
bool EntityWalker::Update(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }

    Controllable::Update(fDeltaTime);
    if (NetGame::IsEnabled() && NetGame::JustEndedPreGame() && !NetGame::IsClient())
    {
        return false;
    }

    int iJoystick = GetJoystickIndex();
    bool bTargetHandled = false;
    if (iJoystick >= 0 &&
        (!NetGame::IsEnabled() || (NetGame::IsClient() && netIsLocalTurn) ||
         (NetGame::IsHost() && NetGame::IsOneWorld() && netIsLocalTurn)))
    {
        LockOnManager* pManager = LockOnManager::GetByPlayerIndex(iJoystick);
        int iTypes = 0;
        float fLockDist = 2000.0f;
        float fLoseDist = 2500.0f;
        for (int i = 0; i < 2; ++i)
        {
            if (mWeaponIndex[i] != -1)
            {
                const WeaponClass* pClass = mWeapon[mWeaponIndex[i]]->mClass;
                iTypes = pClass->mTargetTypes;
                fLockDist = pClass->mTargetLockMaxDistance;
                fLoseDist = pClass->mTargetLockMaxDistanceLose;
                break;
            }
        }
        pManager->SetAllowedTypes(iTypes);
        mAllowedLockTypes = iTypes;
        pManager->SetAutoLose(2.0f, true, 0.4f, fLoseDist);
        pManager->SetAutoLoseLocked(2.0f, true, -1.0f, fLoseDist);
        pManager->SetTarget(0.5f, 0.8f, fLockDist, true);
        pManager->SetAuto(true, false);
        pManager->SetNumAttackerTargets(0);
        pManager->SetNumNearestTargets(pManager->GetDisplayIconsEverything() ? 64 : 2);

        if (mControlLockTarget.IsPressed())
        {
            pManager->ForceNewTargetLock(0);
            ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_FlyerTargetLock, 0);
            bTargetHandled = true;
        }
        else if (mControlLockTarget.Held())
        {
            pManager->RemoveTargetLock();
            ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_FlyerTargetClear, 0);
            bTargetHandled = true;
        }
    }
    else if (mPlayerId >= 0 && iJoystick < 0 && NetGame::IsEnabled() && !NetGame::IsClient())
    {

        int iTypes = 0;
        for (int i = 0; i < 2; ++i)
        {
            if (mWeaponIndex[i] != -1)
            {
                iTypes = mWeapon[mWeaponIndex[i]]->mClass->mTargetTypes;
                break;
            }
        }
        mAllowedLockTypes = iTypes;
    }

    if (mPlayerId >= 0 && !(NetGame::IsEnabled() && NetGame::IsClient()) && !bTargetHandled)
    {
        GameObject* pTarget = GetTargetLockedObj();
        if (pTarget &&
            !LockOnManager::IsTargetObjectTypeAllowed(pTarget, GetGameObject(), false, mAllowedLockTypes, 0, false))
        {
            pTarget = GetVehicleIfAttached(pTarget);
            if (pTarget &&
                LockOnManager::IsTargetObjectTypeAllowed(pTarget, GetGameObject(), false, mAllowedLockTypes, 0, false))
            {
                SetTargetLockedObj(pTarget);
            }
            else
            {
                SetTargetLockedObj(0);
            }
        }
    }

    if (mIsDead)
    {
        mTargetInfo.mIsAiming = false;
    }
    else
    {
        float fWaterHeight;
        if (RedWater::GetWaterHeight(GetPosition(), fWaterHeight) &&
            fWaterHeight >= GetPosition().y + GetCollisionRadius())
        {
            mWaterDamageTimer -= fDeltaTime;
            if (0.0f > mWaterDamageTimer)
            {
                Damageable::ApplyDamage(DamageDesc(DamageOwner(0, 0, 0, 0, 0, 1.0f), mClass->mWaterDamageAmount),
                                        0xffffffff);
            }
        }
        else
        {
            mWaterDamageTimer = mClass->mWaterDamageInterval;
        }

        if (!IsAlive())
        {
            mTargetInfo.mIsAiming = false;
            Kill();
            Die();
        }
        else
        {
            if (!(NetGame::IsEnabled() && NetGame::IsClient()))
            {
                PblVector3 vPos = mMatrix.GetTranslation();
                if (-500.0f > vPos.y && !(NetGame::IsEnabled() && NetGame::IsClient()))
                {
                    Kill();
                    return false;
                }
            }
            UpdateStatus(fDeltaTime);
        }
    }
    TriggerDisabledEffect();

    if (mTimeSinceLastHack > -0.0001f)
    {
        mTimeSinceLastHack += fDeltaTime;
    }
    if (mTimeSinceLastHack > 1.0f)
    {
        mEjectTimer -= (0.04f - mClass->mEjectResist) * 50.0f * fDeltaTime;
    }
    if (0.0001f > mEjectTimer)
    {
        mTimeSinceLastHack = -1.0f;
    }

    if (mControlSquadCommand.IsPressed())
    {
        SquadCommands::VehicleCommand(this, mCharacter);
    }

    if (GetControllableList().GetCount() == 1)
    {
        for (int iChannel = 0; iChannel < 2; ++iChannel)
        {
            int iIndex = mWeaponIndex[iChannel];
            if (iIndex < 0 || mControlSwitch[iChannel] == 0.0f)
            {
                continue;
            }

            int iNew = iIndex;
            do
            {
                if (mControlSwitch[iChannel] < 0.0f)
                {
                    if (--iNew < 0)
                    {
                        iNew = mClass->mWeaponCount - 1;
                    }
                }
                else
                {
                    if (++iNew > mClass->mWeaponCount - 1)
                    {
                        iNew = 0;
                    }
                }
            } while (iNew != mWeaponIndex[iChannel] && mClass->mWeaponChannel[iNew] != iChannel);

            if (iNew != iIndex)
            {
                mWeapon[iIndex]->Deselect(iChannel, false);
                mWeaponIndex[iChannel] = iNew;
                mWeapon[iNew]->Select(iChannel, false);
                mTargetInfo.mIsAiming = false;
            }
            else if (NetGame::GetJoystickIndex(mPlayerId) >= 0)
            {
                WeaponClass::sWeaponChangeUnableSound.Play(&mMatrix.GetTranslation(), 0, false, true);
            }
        }
    }

    if (mClass->m_uiNumAttackAnims && mClass->mHealthType == HEALTH_TYPE_ANIMAL)
    {
        if (!m_bIsGrounded && m_fGroundedTimer > 0.3f)
        {
            mAttackControlTimer = 0.0f;
            m_uiAttackControls = 0;
            m_uiAttackAnim = 0;
        }
        else
        {
            mAttackControlTimer += fDeltaTime;
            if (mAttackControlTimer > 0.0f)
            {
                if (mControlFire[0].IsDown())
                {
                    mControlFire[0].ClearUntilPressed();
                    m_uiAttackControls |= 1;
                }
                if (mControlFire[1].IsDown())
                {
                    mControlFire[1].ClearUntilPressed();
                    m_uiAttackControls |= 2;
                }
                if (m_uiAttackControls == 0)
                {
                    mAttackControlTimer = 0.0f;
                }
                else if (m_uiAttackControls == 3)
                {
                    mAttackControlTimer = ATTACK_CONTROL_DELAY;
                }
            }
        }
    }

    UpdatePilot();
    UpdatePlayer();

    bool bMouse = NetGame::IsMouseMode(mPlayerId);
    float fTurn = mControlTurn;
    float fScale = fDeltaTime;
    if (mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]])
    {
        fScale = fDeltaTime / mWeapon[mWeaponIndex[0]]->mZoomTurnScale;
    }
    float fYawRate = mClass->mMaxYawSpeed;
    float fPitchRate;
    if (bMouse)
    {
        fPitchRate = fYawRate;
    }
    else
    {
        fPitchRate = mClass->mMaxPitchSpeed;
        if (mPlayerId >= 0)
        {
            fYawRate = (fYawRate - fPitchRate) * mTurnBuildup + fPitchRate;
        }
    }
    mPitchValue = fPitchRate * mControlPitch * fScale + mPitchValue;

    if (mIsDead && mIsDead && mState == Dead)
    {
        if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mDamageOwner.mCharacter)
        {
            AwardVehicleKillPoints(mClass->GetVehicleType(),
                                   GetControllablePilot(this) == 0 && !IsRtti(CommandWalker::rttiHashCommandWalker));
        }
        ExplosionClass* pExplosion = mClass->mFinalExplosion;
        if (!pExplosion)
        {
            pExplosion = mClass->mExplosion;
        }
        if (pExplosion && !mExplodedFinal)
        {
            mExplodedFinal = true;
            DamageOwner owner(mDamageOwner.mCharacter, mDamageOwner.mGameObject, mDamageOwner.mTeam, mClass, 0, 1.0f);
            PblVector3 vPos;
            D3DXVec3TransformCoord(&vPos, &mClass->mFinalExplosionOffset, &mMatrix);
            pExplosion->CreateExplosion(vPos, owner, false, COLL_UNKNOWN);
        }
        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            CreateChunkCluster(1.0f);
            return false;
        }
        return true;
    }

    float fMove = mControlMove;
    if (-0.25f > mBodyPitch)
    {
        fMove = -1.0f;
    }
    else if (-0.18f > mBodyPitch && fMove > 0.0f)
    {
        fMove = 0.0f;
    }
    if (mBodyPitch > 0.25f)
    {
        fMove = 1.0f;
    }
    else if (mBodyPitch > 0.18f && fMove < 0.0f)
    {
        fMove = 0.0f;
    }
    float fTurnSpeed = mClass->mForwardTurnSpeed;
    UpdateState(fDeltaTime, fMove, mControlStrafe, fTurnSpeed, fTurn);

    if (VehicleEngine::IsOn())
    {
        if (!mPilot)
        {
            TurnOff(mClass, mMatrix.GetTranslation(), mVelocity);
        }
    }
    else if (mPilot)
    {
        TurnOn(mClass, mMatrix.GetTranslation(), mVelocity, mMaxHealth != 0.0f ? mCurHealth / mMaxHealth : 0.0f);
    }
    if (fDeltaTime > 0.0f && VehicleEngine::IsOn())
    {
        float fSpeed = sqrtf(mVelocityNoGrav.x * mVelocityNoGrav.x + mVelocityNoGrav.y * mVelocityNoGrav.y +
                             mVelocityNoGrav.z * mVelocityNoGrav.z);
        bool bFirstPerson = mTracker && mTracker->IsFirstPersonView();
        bool bLocal = mPlayerId >= 0 &&
                      (NetGame::GetLocalPlayer(0) == mCharacter || NetGame::GetLocalPlayer(1) == mCharacter);
        float fHealth = mMaxHealth != 0.0f ? mCurHealth / mMaxHealth : 0.0f;
        VehicleEngine::Update(mClass, fDeltaTime, mMatrix.GetTranslation(), mVelocityNoGrav, fSpeed,
                              mSpeedPrev - fSpeed / fDeltaTime, fHealth, 0.0f, 0.0f, 0.0f, 0.0f, bLocal,
                              bFirstPerson, 0.0f);
        mSpeedPrev = fSpeed;
    }
    int iSide = 0;
    if (mPilot && mPilot->mCharacter)
    {
        iSide = mPilot->mCharacter->mTeamPtr->mSide;
    }
    VehicleMusic::Update(mClass, fDeltaTime, mPlayerId >= 0 && NetGame::GetNumLocalPlayers() == 1, mSpeedPrev,
                         iSide);

    float fHeading = atan2f(mTurnOffset.mSin, mTurnOffset.mCos) * 2.0f;
    float fMinYaw = mClass->mMinYaw + fHeading;
    float fMaxYaw = mClass->mMaxYaw + fHeading;
    float fMaxRoom = 1.0f;
    if (0.0f > fTurn && fMaxYaw > 0.0f)
    {
        float fRoom = fMaxYaw - mYawValue;
        if (0.0f > fRoom)
        {
            fRoom = 0.0f;
        }
        fMaxRoom = fRoom / fMaxYaw;
        if (0.0f > fMaxRoom)
        {
            fMaxRoom = 0.0f;
        }
        else if (fMaxRoom > 1.0f)
        {
            fMaxRoom = 1.0f;
        }
    }
    float fMinRoom = 1.0f;
    if (fTurn > 0.0f && 0.0f > fMinYaw)
    {
        float fRoom = mYawValue - fMinYaw;
        if (0.0f > fRoom)
        {
            fRoom = 0.0f;
        }
        fMinRoom = -(fRoom / fMinYaw);
        if (0.0f > fMinRoom)
        {
            fMinRoom = 0.0f;
        }
        else if (fMinRoom > 1.0f)
        {
            fMinRoom = 1.0f;
        }
    }
    float fRoom = fMinRoom > fMaxRoom ? fMaxRoom : fMinRoom;
    float fRate = fYawRate;
    if (fYawRate > fTurnSpeed)
    {
        fRate = (1.0f - fRoom) * fTurnSpeed + fRoom * fYawRate;
    }
    mYawValue = mYawValue - fRate * fTurn * fScale;
    mPitchValue = mClass->mMinPitch > mPitchValue
                      ? mClass->mMinPitch
                      : (mPitchValue > mClass->mMaxPitch ? mClass->mMaxPitch : mPitchValue);

    if (!mIsDead && !m_bIsJumping && 0.0f > mVelocity.y)
    {
        const PblVector3& vPos = mMatrix.GetTranslation();
        SetPosition(PblVector3(vPos.x + 0.0f, vPos.y + mVelocity.y * fDeltaTime, vPos.z + 0.0f));
    }
    return true;
}

// FUNCTION: SWBF2 0x00505c80
bool EntityWalker::PostCollisionUpdate(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime) || (mIsDead && mState == Dead))
    {
        return true;
    }

    if (RedTerrain::_bEnable)
    {
        float fHeight = mClass->mLegRayHitLength * 0.85f;
        PblVector3 vStart(mMatrix.m[3][0] + 0.0f, mMatrix.m[3][1] + fHeight, mMatrix.m[3][2] + 0.0f);
        PblVector3 vFlat(mMatrix.m[2][0], 0.0f, mMatrix.m[2][2]);
        PblVector3 vFront;
        D3DXVec3Normalize(&vFront, &vFlat);
        PblVector3 vSide(-vFront.z, 0.0f, vFront.x);
        float fLength = fHeight;
        if (mLegPairCount > 1)
        {
            fLength = fHeight * 1.3f;
            float fOffset = mLegPairs[mDirection == Backward]->mLocalZOffset;
            vStart.x = fOffset * vFront.x + vStart.x;
            vStart.y = fOffset * vFront.y + vStart.y;
            vStart.z = fOffset * vFront.z + vStart.z;
        }
        PblVector3 vDir = vFront;
        if (mDirection == Backward)
        {
            vDir.x = vFront.x * -1.0f;
            vDir.y = vFront.y * -1.0f;
            vDir.z = vFront.z * -1.0f;
        }
        float fAhead = fLength * 0.8f;
        float fAside = fLength * 0.7f;

        PblVector3 vRay(fAhead * vDir.x + vSide.x * fAside, vDir.y * fAhead + fAside * vSide.y,
                        vDir.z * fAhead + fAside * vSide.z);
        PblVector3 vEnd(vRay.x + vStart.x, vRay.y + vStart.y, vRay.z + vStart.z);
        float fHit = RedTerrain::RayHit(vStart, vEnd, 0.1f, 3.0f);
        if (1.0f > fHit)
        {
            float fBack = 1.0f - fHit;
            SetPosition(PblVector3(mMatrix.m[3][0] - vRay.x * fBack, mMatrix.m[3][1] - vRay.y * fBack,
                                   mMatrix.m[3][2] - vRay.z * fBack));
        }

        vRay = PblVector3(fAhead * vDir.x - vSide.x * fAside, fAhead * vDir.y - fAside * vSide.y,
                          fAhead * vDir.z - fAside * vSide.z);
        vEnd = PblVector3(vRay.x + vStart.x, vRay.y + vStart.y, vRay.z + vStart.z);
        fHit = RedTerrain::RayHit(vStart, vEnd, 0.1f, 3.0f);
        if (1.0f > fHit)
        {
            float fBack = 1.0f - fHit;
            SetPosition(PblVector3(mMatrix.m[3][0] - vRay.x * fBack, mMatrix.m[3][1] - vRay.y * fBack,
                                   mMatrix.m[3][2] - vRay.z * fBack));
        }
    }

    if (!mIsDead)
    {
        UpdateOrientation(fDeltaTime);
    }
    UpdateDone();
    CorrectJoints();
    mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
    RestoreJoints();

    float fSinPitch;
    float fCosPitch;
    float fSinYaw;
    float fCosYaw;
    PblMath::SinCos(&fSinPitch, &fCosPitch, mPitchValue);
    PblMath::SinCos(&fSinYaw, &fCosYaw, mYawValue);
    PblVector3 vLocal(fSinYaw * fCosPitch, fSinPitch, fCosYaw * fCosPitch);
    PblVector3 vEyeDir;
    D3DXVec3TransformNormal(&vEyeDir, &vLocal, &mMatrix);
    mEyeDir = vEyeDir;
    mEyePoint = mClass->mCameraSettings.CalcEyePoint(mMatrix);

    if (mPlayerId >= 0 && mTracker)
    {
        if (mTargetInfo.mIsAiming || mTracker->IsFirstPersonView())
        {
            mTargetInfo.mAimStart = mEyePoint;
        }
        else
        {

            PblMatrix mLook =
                DirectionMatrix(PblVector3(-mEyeDir.x, -mEyeDir.y, -mEyeDir.z), mMatrix.GetYAxis(), gVectorZero);
            PblQuaternion qLook = PblQuaternion(mLook);
            float fSin;
            float fCos;
            PblMath::SinCos(&fSin, &fCos, -mClass->mCameraSettings.GetCameraTiltValue() * 0.5f);
            PblQuaternion qTilt;
            qTilt._fX = fSin * gVectorUnitX.x;
            qTilt._fY = gVectorUnitX.y * fSin;
            qTilt._fZ = gVectorUnitX.z * fSin;
            qTilt._fS = fCos;
            PblQuaternion qCamera = qLook * qTilt;

            PblQuaternion qUnit;
            PblMatrix mCamera;
            D3DXMatrixRotationQuaternion(&mCamera, D3DXQuaternionNormalize(&qUnit, &qCamera));
            mCamera.m[3][0] = gVectorZero.x;
            mCamera.m[3][1] = gVectorZero.y;
            mCamera.m[3][2] = gVectorZero.z;
            mCamera.m[3][3] = 1.0f;

            mTargetInfo.mAimStart = mClass->mCameraSettings.CalcCameraPoint(mMatrix, mCamera);
            float fAlong = mEyeDir.y * (mEyePoint.y - mTargetInfo.mAimStart.y) +
                           mEyeDir.x * (mEyePoint.x - mTargetInfo.mAimStart.x) +
                           mEyeDir.z * (mEyePoint.z - mTargetInfo.mAimStart.z);
            mTargetInfo.mAimStart.x = mTargetInfo.mAimStart.x + mEyeDir.x * fAlong;
            mTargetInfo.mAimStart.y = mEyeDir.y * fAlong + mTargetInfo.mAimStart.y;
            mTargetInfo.mAimStart.z = mEyeDir.z * fAlong + mTargetInfo.mAimStart.z;
        }
        const GameObject* pIgnore = this;
        CollisionObject* pHit;
        float fDist =
            CollisionManager::RayHit(mTargetInfo.mAimStart, mEyeDir, 512.0f, pHit, 0, &pIgnore, 1, 0x9a, true) *
            512.0f;
        mTargetInfo.mAimPoint = PblVector3(mTargetInfo.mAimStart.x + mEyeDir.x * fDist,
                                           mTargetInfo.mAimStart.y + mEyeDir.y * fDist,
                                           mTargetInfo.mAimStart.z + mEyeDir.z * fDist);
    }
    else
    {
        mTargetInfo.mAimStart = mEyePoint;
        float fDist = 512.0f;
        int iIndex = mWeaponIndex[0];
        if (iIndex >= 0 && mWeapon[iIndex] && mWeapon[iIndex]->mTarget)
        {
            PblVector3 vTarget = mWeapon[iIndex]->mTarget->GetTargetPoint(mEyePoint, mEyeDir, -1);
            fDist = sqrtf((vTarget.x - mEyePoint.x) * (vTarget.x - mEyePoint.x) +
                          (vTarget.y - mEyePoint.y) * (vTarget.y - mEyePoint.y) +
                          (vTarget.z - mEyePoint.z) * (vTarget.z - mEyePoint.z));
        }
        mTargetInfo.mAimPoint = PblVector3(mEyeDir.x * fDist + mEyePoint.x, mEyePoint.y + mEyeDir.y * fDist,
                                           mEyePoint.z + mEyeDir.z * fDist);
    }

    mClass->UpdatePilotAnimation(mPilot, &mPose, mMatrix, mVelocity, fDeltaTime, -1.0f, -1.0f, -1.0f);
    mBlend->SetInterpolate(false);

    if (Smooth(fDeltaTime))
    {
        const PblVector3& vCenter = GetCollisionCenter();
        const PblVector3& vPos = GetPosition();
        const PblVector3& vSmoothed = GetSmoothedMatrix().GetTranslation();
        const RedDirectionalLight* pLight = 0;
        if (mStencilShadow)
        {
            if (RedLight::m_sGlobalDirectionals[0]->IsShadowCaster())
            {
                pLight = RedLight::m_sGlobalDirectionals[0];
            }
            else if (RedLight::m_sGlobalDirectionals[1]->IsShadowCaster())
            {
                pLight = RedLight::m_sGlobalDirectionals[1];
            }
        }
        if (pLight)
        {

            float fOffset;
            if (-0.5f > pLight->m_vDirection.y)
            {
                fOffset = GetCollisionRadius() / (pLight->m_vDirection.y * -2.0f);
            }
            else
            {
                fOffset = GetCollisionRadius();
            }
            SetRenderPos(PblVector3(vSmoothed.x + vCenter.x - vPos.x + fOffset * pLight->m_vDirection.x,
                                    vSmoothed.y + vCenter.y - vPos.y + pLight->m_vDirection.y * fOffset,
                                    vSmoothed.z + vCenter.z - vPos.z + pLight->m_vDirection.z * fOffset));
            SetRenderRadius(GetCollisionRadius() + fOffset);
            SetTestBBox(false);
        }
        else
        {
            SetRenderPos(PblVector3(vSmoothed.x + vCenter.x - vPos.x, vSmoothed.y + vCenter.y - vPos.y,
                                    vSmoothed.z + vCenter.z - vPos.z));
        }
    }

    if (!netFrameLock && NetGame::IsEnabled())
    {
        bool bFeed = true;
        if (!(NetGame::IsClient() && netIsLocalTurn))
        {
            if (!NetGame::IsHost() || !NetGame::IsOneWorld() || netIsLateTurn)
            {
                bFeed = false;
            }
            else if (mNetUniqueId >= 0)
            {
                mNetUniqueId = NetGame::GetHostWorldUniqueId(mNetUniqueId, this);
            }
        }
        if (bFeed)
        {
            gameObjectInterpolator.SetNext(mNetUniqueId, &GetSmoothedMatrix());
            gameObjectInterpolator.SetNextFloat(mNetUniqueId, 0, mBlend->GetTurnState());
            gameObjectInterpolator.SetNextFloat(mNetUniqueId, 1, mLegPairs[0]->GetTurnState());
            gameObjectInterpolator.SetNextFloat(mNetUniqueId, 2,
                                                mLegPairCount > 1 ? mLegPairs[1]->GetTurnState() : 0.0f);
            if (mAimer[0])
            {
                gameObjectInterpolator.SetNextVector(mNetUniqueId, &mAimer[0]->mDirection);
            }
        }
    }

    if (mPilot && mPilot->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        GameObject* pSoldier = mPilot->GetGameObject();
        PblMatrix mInverse = GetMatrix().GetInverseFast();
        PblMatrix mSeat;
        D3DXMatrixMultiply(&mSeat, &pSoldier->GetMatrix(), &mInverse);
        PblMatrix mLocal = mSeat;
        PblMatrix mSmoothed = GetSmoothedMatrix();
        PblMatrix mRender;
        D3DXMatrixMultiply(&mRender, &mLocal, &mSmoothed);
        pSoldier->SetRenderPos(mRender.GetTranslation());
    }
    return true;
}

// FUNCTION: SWBF2 0x00506a40
bool EntityWalker::ApplyDamage(const DamageDesc& damage, const PblVector3& vDir, const PblVector3& vPos,
                               int iBodyPart, unsigned int uFlags)
{
    if (NetGame::IsPreGame())
    {
        return true;
    }
    if (mPlayerId >= 0)
    {
        int iJoystick = NetGame::GetJoystickIndex(mPlayerId);
        if (iJoystick >= 0)
        {
            PblMatrix mInverse = mMatrix.GetInverseFast();
            PblVector3 vLocal;
            D3DXVec3TransformNormal(&vLocal, &vDir, &mInverse);
            StatusDisplay::Instance(iJoystick)->SetDamageDirection(vLocal);
        }
    }
    return Damageable::ApplyDamage(damage, vDir, vPos, iBodyPart, uFlags);
}

// FUNCTION: SWBF2 0x00506ae0
bool EntityWalker::UpdateIndirect(float fDeltaTime)
{
    AILowLevel& lowLevel = UnitControllerPtr(mCtrl)->GetLowLevel();
    if (!lowLevel.UpdateIndirect(fDeltaTime) || !mCtrl)
    {
        return true;
    }

    if (mMaxHealth * 0.2f > mCurHealth && (UnitControllerPtr(mCtrl)->GetRandomSeed() & 3) == 0)
    {
        AIUtil::TellAIToExit(this);
        return true;
    }

    mIsStopped = mStopTime + 7.5f > GameLoop::GetMissionTime();
    ControllableDestination dest;
    if (mIsStopped)
    {
        dest.mHasDest = true;
        dest.mPos = GetMatrix().GetTranslation();
        dest.mOriginalPos = GetMatrix().GetTranslation();
        dest.mDirection = GetMatrix().GetZAxis();
        dest.mSafeHeightEnforce = (SafeHeight)0;
        dest.mTaperFactor = -1.0f;
        dest.mMaxSpeed = 1.0f;
    }
    else
    {
        dest = lowLevel.mDest;
    }
    ControllableHeading heading = lowLevel.mHeading;
    lowLevel.ProcessFire(fDeltaTime, &heading);

    if (mClass->mAIUseHoverPhysics)
    {
        const PblMatrix& mMe = GetMatrix();
        PblVector3 vToDest(dest.mPos.x - GetMatrix().m[3][0], dest.mPos.y - GetMatrix().m[3][1],
                           dest.mPos.z - GetMatrix().m[3][2]);
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vToDest);
        if (mMe.m[2][1] * vDir.y + vDir.x * mMe.m[2][0] + mMe.m[2][2] * vDir.z > 0.8660254f)
        {
            float fStrafe = mControlStrafe;
            float fThrust = mControlMove;
            float fTurn = mControlTurn;
            float fPitch = mControlPitch;
            AIUtil::UpdatePhysics(this, fDeltaTime, dest, heading, mClass->mMaxSpeed, mClass->mMaxSpeed * 0.4f,
                       mClass->mMaxTurnSpeed, mClass->mMaxPitchSpeed, &fThrust, &fStrafe, &fTurn, &fPitch);
            mControlMove = fThrust >= 0.0f ? fThrust : 0.0f;
            mControlStrafe = -fStrafe;
            AIUtil::CalculateHeadingControls(this, fDeltaTime, dest, heading, mClass->mMaxYawSpeed, &fTurn,
                                             mClass->mMaxPitchSpeed, &fPitch, true);
            mControlTurn = fTurn;
            mControlPitch = fPitch;
            return true;
        }
    }

    float fThrust = mControlMove;
    float fStrafe = mControlStrafe;
    float fTurn = mControlTurn;
    float fPitch = mControlPitch;
    float fUnusedStrafe;
    float fUnusedPitch;
    AIUtil::UpdatePhysics(this, fDeltaTime, dest, heading, mClass->mMaxSpeed, 0.0f, mClass->mMaxTurnSpeed, 0.0f, &fThrust,
               &fUnusedStrafe, &fStrafe, &fUnusedPitch);
    AIUtil::CalculateHeadingControls(this, fDeltaTime, dest, heading, mClass->mMaxYawSpeed, &fTurn,
                                     mClass->mMaxPitchSpeed, &fPitch, true);
    mControlMove = fThrust;
    mControlStrafe = fStrafe;
    mControlTurn = fTurn;
    mControlPitch = fPitch;
    return true;
}

// FUNCTION: SWBF2 0x00507380
float EntityWalker::GetSoundRadius() const
{
    float fMute = 0.0f;
    float fRadius = 0.0f;
    for (unsigned int i = 0; i < mClass->mNumFeet; ++i)
    {
        if (mFootState & (1 << i))
        {
            if (FoleyFX* pFoley = GetFoleyFX())
            {
                fMute = static_cast<FoleyFXWalker*>(pFoley)->mFootstep[i].GetMuteDistance();
            }
        }
        if (mFootState & (1 << (i + 8)))
        {
            fMute = mClass->mHydraulicSound.GetMuteDistance();
        }
        if (mFootState & (1 << (i + 16)))
        {
            fMute = mClass->mHydraulicLowerSound.GetMuteDistance();
        }
        if (fMute > fRadius)
        {
            fRadius = fMute;
        }
    }
    float fEngine = VehicleEngine::GetMuteDistance();
    if (fEngine > fRadius)
    {
        fRadius = fEngine;
    }
    return mClass->mNoiseFactor * fRadius;
}

// FUNCTION: SWBF2 0x00507660
bool EntityWalker::RenderTrackable(unsigned int uView) const
{
    if (!IsAlive())
    {
        return true;
    }

    if (mTargetInfo.mIsAiming)
    {
        int iIndex = mWeaponIndex[0];
        if (iIndex >= 0)
        {
            Weapon* pWeapon = mWeapon[iIndex];
            if (pWeapon && pWeapon->ZoomFirstPerson() &&
                NetGame::GetJoystickIndex(uView) == (int)FLRenderer::GetCurrentViewport())
            {
                return false;
            }
        }
    }
    return Trackable::RenderTrackable(uView);
}

// FUNCTION: SWBF2 0x005076d0
float EntityWalker::GetMaxSpeed() const
{
    return mClass->mMaxSpeed;
}

// FUNCTION: SWBF2 0x0050af10
float EntityWalkerClass::GetAnimationBlendDuration(EntityWalker::eState eFrom, EntityWalker::eState eTo) const
{
    if (!m_bAnimationBlending || eFrom == eTo)
    {
        return 0.0f;
    }

    int iToFoot = -1;
    int iFromFoot = -1;
    if (eTo >= EntityWalker::StateCount)
    {
        iToFoot = 0;
    }
    else
    {
        switch (eTo)
        {
        case EntityWalker::Idle:
        case EntityWalker::TurnLeft:
        case EntityWalker::TurnRight:
        case EntityWalker::Dying:
        case EntityWalker::IdleToLeft:
        case EntityWalker::IdleToRight:
            iToFoot = 0;
            break;
        case EntityWalker::LeftToRight:
        case EntityWalker::LeftToIdle:
            iToFoot = 1;
            break;
        case EntityWalker::RightToLeft:
        case EntityWalker::RightToIdle:
            iToFoot = 2;
            break;
        default:
            break;
        }
    }
    if (eFrom >= EntityWalker::StateCount)
    {
        iFromFoot = 0;
    }
    else
    {
        switch (eTo)
        {
        case EntityWalker::TurnLeft:
        case EntityWalker::TurnRight:
        case EntityWalker::LeftToIdle:
        case EntityWalker::RightToIdle:
            iFromFoot = 0;
            break;
        case EntityWalker::IdleToLeft:
        case EntityWalker::RightToLeft:
            iFromFoot = 1;
            break;
        case EntityWalker::IdleToRight:
        case EntityWalker::LeftToRight:
            iFromFoot = 2;
            break;
        default:
            break;
        }
    }
    if (iToFoot == iFromFoot && iToFoot != -1)
    {
        return 0.0f;
    }
    return 0.2f;
}

// FUNCTION: SWBF2 0x00506f70
void EntityWalker::SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime)
{
    PblMatrix matrix = GetSmoothedMatrix();
    if (!IsAlive() ||
        (!mTargetInfo.mIsAiming && (!mTracker || !mTracker->IsFirstPersonView())))
    {
        mClass->mCameraSettings.SetupCameraTrackMatrix(this, rMatrix, fDeltaTime,
                                                       matrix, mEyeDir, 1.0f, 1.0f, this);
        return;
    }

    float fSinPitch, fCosPitch, fSinYaw, fCosYaw;
    PblMath::SinCos(&fSinPitch, &fCosPitch, mPitchValue);
    PblMath::SinCos(&fSinYaw, &fCosYaw, mYawValue);
    PblVector3 vLocal(fSinYaw * fCosPitch, fSinPitch, fCosYaw * fCosPitch);
    PblVector3 vAim;
    D3DXVec3TransformNormal(&vAim, &vLocal, &matrix);
    PblVector3 vEye = mClass->mCameraSettings.CalcEyePoint(matrix);

    const PblMatrix* pJoint = mPose.Find(mClass->mFPPoseHash);
    const PblVector3 vJoint = pJoint->GetTranslation();
    if (mPrevAimerPos.x == -66.6f && mPrevAimerPos.y == 66.6f &&
        mPrevAimerPos.z == -66.6f)
    {
        mPrevAimerPos = vJoint;
    }

    const float fScale = mClass->mFPWalkScale * 2.0f;
    vEye.x += (vJoint.x - mPrevAimerPos.x) * fScale;
    vEye.y += (vJoint.y - mPrevAimerPos.y) * fScale;
    vEye.z += (vJoint.z - mPrevAimerPos.z) * fScale;
    rMatrix = DirectionMatrix(PblVector3(-vAim.x, -vAim.y, -vAim.z),
                              matrix.GetYAxis(), vEye);

    int iFirstPerson = GetFirstPersonIndex();
    if (iFirstPerson >= 0 && !mTargetInfo.mIsAiming)
    {
        FirstPerson::UpdateCamera(iFirstPerson, rMatrix);
    }
}

// SYNTHETIC: SWBF2 0x004803ce
// [thunk]:EntityWalker::GetGameObject`adjustor{12}'
// SYNTHETIC: SWBF2 0x004803ec
// [thunk]:EntityWalker::GetControllable`adjustor{600}'
// SYNTHETIC: SWBF2 0x004803f7
// [thunk]:EntityWalker::GetControllable`adjustor{600}'
// SYNTHETIC: SWBF2 0x00480402
// [thunk]:CommandWalker::`scalar deleting destructor'`adjustor{148}'
// SYNTHETIC: SWBF2 0x00480418
// [thunk]:CommandWalker::Kill`adjustor{256}'
// SYNTHETIC: SWBF2 0x00480423
// [thunk]:CommandWalkerClass::`scalar deleting destructor'`adjustor{3536}'
// SYNTHETIC: SWBF2 0x0048042e
// [thunk]:CommandWalker::`scalar deleting destructor'`adjustor{320}'
// SYNTHETIC: SWBF2 0x00480439
// [thunk]:CommandWalker::`scalar deleting destructor'`adjustor{600}'
// SYNTHETIC: SWBF2 0x00480444
// [thunk]:CommandWalker::`scalar deleting destructor'`adjustor{12}'
// SYNTHETIC: SWBF2 0x00480454
// [thunk]:EntityWalker::GetEntity`adjustor{12}'
// SYNTHETIC: SWBF2 0x0048045c
// [thunk]:CommandWalker::`scalar deleting destructor'`adjustor{512}'
// SYNTHETIC: SWBF2 0x00480472
// [thunk]:EntityWalker::GetGameObject`adjustor{320}'
// SYNTHETIC: SWBF2 0x0050afda
// [thunk]:EntityWalker::Kill`adjustor{256}'
// SYNTHETIC: SWBF2 0x0050afe5
// [thunk]:EntityWalker::`scalar deleting destructor'`adjustor{320}'
// SYNTHETIC: SWBF2 0x0050aff0
// [thunk]:EntityWalker::`scalar deleting destructor'`adjustor{600}'
// SYNTHETIC: SWBF2 0x0050affb
// [thunk]:EntityWalker::`scalar deleting destructor'`adjustor{12}'
// SYNTHETIC: SWBF2 0x0050b003
// [thunk]:EntityWalker::`scalar deleting destructor'`adjustor{512}'
// SYNTHETIC: SWBF2 0x0050b019
// [thunk]:EntityWalker::`scalar deleting destructor'`adjustor{148}'

// GLOBAL: SWBF2 0x01e51e13
bool g_bWalkerShowDamage;

// SYNTHETIC: SWBF2 0x00402f00
// `dynamic initializer for 'g_WalkerShowDamage''
RedCommandConsole g_WalkerShowDamage("walker.ShowDamage", MultiVar(&g_bWalkerShowDamage));
