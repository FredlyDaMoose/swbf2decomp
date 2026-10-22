#include "RedEngineFL/Util/RedCommandConsole.h"

#include <stdio.h>
#include "PebbleFL/Common/PblVector.h"
#include <stdlib.h>

#include "EntityFlyer.h"
#include "CollisionBody.h"
#include "CollisionMesh.h"
#include "CollisionObject.h"
#include "CollisionPrimitive.h"
#include "Explosion.h"
#include "FirstPerson.h"
#include "GameModel.h"
#include "MountedTurret.h"
#include "PassengerSlot.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "CameraManager.h"
#include "MotionBlurEffect.h"
#include "ChaseCamera.h"
#include "DeathCamera.h"
#include "EntityCarrier.h"
#include "EventManager_Character.h"
#include "NetGame.h"
#include "Aimer.h"
#include <string.h>
#include "CollisionManager.h"
#include "PathFinder.h"
#include "TargetManager.h"
#include "VehicleManager.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblRandom.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "Character.h"
#include "EntityPath.h"
#include "PlayerController.h"
#include "UnitController.h"
#include "UnitFlyController.h"
#include "FLEffect.h"
#include "FlagItem.h"
#include "ParticleEmitterObject.h"
#include "Tracker.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "AIDifficulty.h"
#include "Controller.h"
#include "EntitySoldier.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedEngineFL/Application/RedWater.h"
#include "LockOnManager.h"
#include "MedalsMgr.h"
#include "SpaceDustEffect.h"
#include "Team.h"
#include "ToolTips.h"
#include "TriggerRegionManager.h"
#include "CommandFlyer.h"
#include "EntityPortableTurret.h"
#include "Restrictor.h"
#include "UnitThreatManager.h"
extern "C" PblMatrix* __stdcall D3DXMatrixRotationX(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationZ(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut,
    const PblMatrix* pM1, const PblMatrix* pM2);

// FUNCTION: SWBF2 0x004b6320
EnergyMode EntityFlyer::GetEnergyMode() const
{
    if (mTrick <= 0.0f)
    {
        return static_cast<EnergyMode>(0);
    }
    return static_cast<EnergyMode>(mTrickDoubleclick == 0.0f ? 1 : 2);
}

// FUNCTION: SWBF2 0x004ab800
float EntityFlyer::GetAimTurnRate() const
{
    return mClass->mPitchRate;
}

// FUNCTION: SWBF2 0x004ab810
void EntityFlyer::SetWeaponIndex(int iWeaponIndex, int iChannel)
{
    mWeaponIndex[iChannel] = iWeaponIndex;
}

// FUNCTION: SWBF2 0x004ac390
bool EntityFlyer::IsForcedThirdPerson() const
{
    return mInRoll || mInFlip;
}

// FUNCTION: SWBF2 0x004b6040
void EntityFlyer::DeactivatePhysics()
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

// FUNCTION: SWBF2 0x004aaf20
void EntityFlyer::ClientKill()
{
    static_cast<Damageable*>(this)->Kill();
    if (mState == LANDED || mState == CRASHED)
    {
        DamageOwner owner(mCharacter, this, GetTeam(), mClass, 0, 1.0f);
        mClass->mExplosionDestruct->CreateExplosion(
            mCollisionSphere.GetCenter(), owner, false, (CollisionObjectType)0);
    }
}

// FUNCTION: SWBF2 0x004b6350
PblVector3 EntityFlyer::GetTargetPointMissVector() const
{
    if (mSideRoll.x == 0.0f && mSideRoll.y == 0.0f && !mInFlip)
    {
        return PblVector3(0.0f, 0.0f, 0.0f);
    }

    PblMatrix unbanked, unpitched, matrix;
    D3DXMatrixRotationZ(&unbanked, -mBankRollAngle);
    D3DXMatrixMultiply(&unbanked, &unbanked,
                       D3DXMatrixRotationX(&unpitched, -mBankPitchAngle));
    D3DXMatrixMultiply(&matrix, &unbanked, &mMatrix);

    if (mSideRoll.x == 0.0f && mSideRoll.y == 0.0f)
    {
        float scale;
        int row;
        switch (mLastTrickPerformed)
        {
        case FLIP_UP:         row = 1; scale = -10.0f; break;
        case FLIP_DOWN:       row = 1; scale = 10.0f; break;
        case FLIP_UP_RIGHT:
        case FLIP_DOWN_RIGHT: row = 0; scale = 10.0f; break;
        case FLIP_UP_LEFT:
        case FLIP_DOWN_LEFT:  row = 0; scale = -10.0f; break;
        default: return PblVector3(0.0f, 0.0f, 0.0f);
        }
        return PblVector3(matrix.m[row][0] * scale,
                          matrix.m[row][1] * scale,
                          matrix.m[row][2] * scale);
    }
    return PblVector3((matrix.m[0][0] * mSideRoll.x + matrix.m[1][0] * mSideRoll.y) * 17.0f,
                      (matrix.m[0][1] * mSideRoll.x + matrix.m[1][1] * mSideRoll.y) * 17.0f,
                      (matrix.m[0][2] * mSideRoll.x + matrix.m[1][2] * mSideRoll.y) * 17.0f);
}

// FUNCTION: SWBF2 0x004ac370
ExplosionClass* EntityFlyer::GetExplosionClass()
{
    return mClass ? mClass->mExplosionCritical : 0;
}

// FUNCTION: SWBF2 0x004b6300
float EntityFlyer::GetMaxStrafe() const
{
    return mClass->mStrafeSpeed;
}

// FUNCTION: SWBF2 0x004b65d0
bool EntityFlyer::ShouldShowReticule() const
{
    return mIsAlive && !mInFlip && (!mClass->mGuidedMissile || mEnergyBar.m_fEnergy > 0.0f);
}

// FUNCTION: SWBF2 0x004b9720
bool EntityFlyerClass::InitAnimations(const char* pszName)
{
    mAnimObj = RedAnimation::Find(PblHash(pszName));
    if (!mAnimObj)
    {
        return false;
    }

    mAnimTakeoff = const_cast<ZephyrAnim*>(mAnimObj->FindAnimation("takeoff"));
    mAnimFins = const_cast<ZephyrAnim*>(mAnimObj->FindAnimation("fins"));
    return true;
}

// GLOBAL: SWBF2 0x007e633c
static int sCurrentSection = -1;
// GLOBAL: SWBF2 0x007e6500
static int sCurThrustEffect = -1;
// GLOBAL: SWBF2 0x007e6504
static int sCurContrailEffect = -1;
// GLOBAL: SWBF2 0x01e51da8
static int sWeaponSection;
// GLOBAL: SWBF2 0x01e3075e
static bool sPassengerSlotSection;
// GLOBAL: SWBF2 0x01e51dac
static unsigned int sSoftCollisionMask;
// GLOBAL: SWBF2 0x01e51da4
static unsigned int sOrdnanceCollisionMask;

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x004b6670
EntityFlyerClass::EntityFlyerClass(unsigned int uHash)
    : GameObjectClass(uHash)
{
    m_bDynamicLighting = true;
    mLODClass = 1;
    mCollisionModel.mDenyFlyerLand = true;
    mMapParams.mViewRangeMin = 1200.0f;
    mMapParams.mViewRangeMax = 1200.0f;
    mHoldForPlayer = true;
    mHealthType = HEALTH_TYPE_VEHICLE;
    mAIType = 32;
    mVORadius = 500.0f;
    mPilotType = PILOT_VEHICLE;

    mFlyingCollisionInited = false;
    mAnimObj = 0;
    mAnimTakeoff = 0;
    mAnimFins = 0;
    mAcceleration = 5.0f;
    mBoostAcceleration = 0.0f;
    mMinFlyerSpeed = 10.0f;
    mMidFlyerSpeed = 20.0f;
    mMaxFlyerSpeed = 30.0f;
    mBoostFlyerSpeed = 0.0f;
    mStrafeSpeed = 0.0f;
    mPitchRate = 1.0f;
    mTurnRate = 2.0f;
    mPitchFilter = 10.0f;
    mPitchFilterDecel = 10.0f;
    mTurnFilter = 10.0f;
    mTurnFilterDecel = 10.0f;
    mPitchBuildupMultiplier = 1.0f;
    mTurnBuildupMultiplier = 1.0f;
    mThrustPitchAngle = 0.0f;
    mStrafeRollAngle = 0.0f;
    mBankAngle = 0.5f;
    mBankFilter = 2.0f;
    mLevelFilter = 3.0f;
    mLevelFilterLanding = 5.0f;
    mRollRate = 3.15f;
    mRollRateAccel = 4.0f;
    mTakeoffHeight = 5.0f;
    mTakeoffTime = 5.0f;
    mTakeoffSpeed = 3.0f;
    mLandingTime = 5.0f;
    mLandingSpeed = 3.0f;
    mLandedHeight = 0.0f;

    sCurrentSection = -1;
    sWeaponSection = 0;

    mCollisionThreshold[0] = 7.5f;
    mCollisionThreshold[1] = 2.5f;
    mCollisionThreshold[2] = 0.0f;
    mCollisionScale[0] = 1.0f;
    mCollisionScale[1] = 0.0f;
    mCollisionScale[2] = 0.0f;
    mCollisionInflict = 1.0f;
    mCrashShakeStart = 1.0f;
    mCrashShakeEnd = 0.5f;
    mCrashShakeLength = 2.0f;
    mMinCrashRollRate = 3.0f;
    mMaxCrashRollRate = 6.0f;

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
    mBlurEffect = 0.0f;
    mBlurStartV = 1.0f;
    mFOVEffect3rd = 0.0f;
    mFOVEFFectMinCamOffset3rd.x = 0.0f;
    mFOVEFFectMinCamOffset3rd.y = 0.0f;
    mFOVEFFectMinCamOffset3rd.z = 0.0f;
    mFOVEFFectMaxCamOffset3rd.x = 0.0f;
    mFOVEFFectMaxCamOffset3rd.y = 0.0f;
    mFOVEFFectMaxCamOffset3rd.z = 0.0f;
    mFOVEffect1st = 0.0f;

    mEnergyBarClass.m_fEnergyMax = 0.0f;
    mEnergyBarClass.m_fEnergyMin = 0.0f;
    mEnergyBarClass.m_fEnergyOverheat = 0.0f;
    mEnergyBarClass.Init(100.0f, 20.0f);

    mEnergyAutoRestore = 0.0f;
    mEnergyBoostDrain = 1.0f;
    mEnergyTrickDrainSingleTap = 15.0f;
    mEnergyTrickDrainDoubleTap = 50.0f;
    mTrickRollSpeed = 1.0f;
    mTrickFlipSpeed = 1.0f;
    mTrickFlipCameraDetach = 0.24f;
    mTrickSideRollCameraDetach = 0.48f;
    mTrickSideRollStrafeSpeed = 0.0f;
    mMomentumFilter = 8.0f;
    mBarrierX = -1.0f;
    mBarrierZ = -1.0f;
    mBarrierOX = 0.0f;
    mBarrierOZ = 0.0f;

    mPassengerSlotClass = 0;
    mNumPassengerSlots = 0;

    for (int i = 0; i < 8; ++i)
    {
        mTurretClass[i] = 0;
    }

    mExplosionCritical = 0;
    mExplosionDestruct = 0;

    for (int i = 0; i < 8; ++i)
    {
        mThrustEffectName[i] = 0;
        mThrustEffectHardPoint[i] = 0;
        mThrustEffectAttachOffset[i].x = 0.0f;
        mThrustEffectAttachOffset[i].y = 0.0f;
        mThrustEffectAttachOffset[i].z = 0.0f;
        mThrustEffectMinScale[i] = 1.0f;
        mThrustEffectMaxScale[i] = 1.0f;
        mThrustEffectScaleStart[i] = 0.0f;
    }

    for (int i = 0; i < 12; ++i)
    {
        mContrailEffectName[i] = 0;
        mContrailEffectHardPoint[i] = 0;
        mContrailEffectAttachOffset[i].x = 0.0f;
        mContrailEffectAttachOffset[i].y = 0.0f;
        mContrailEffectAttachOffset[i].z = 0.0f;
        mContrailEffectMinScale[i] = 1.0f;
        mContrailEffectMaxScale[i] = 1.0f;
        mContrailEffectMinSpeed[i] = 1.1f;
        mContrailEffectMaxSpeed[i] = 1.6f;
    }

    mNeverCrashWhenUnpiloted = false;
    mGuidedMissile = false;

    mCockpitModelHandle = -1;
    mFreeFlyTime = 2.0f;
    mTrickTimeMid = 1.0f;
    mTrickTimeRange = 1.0f;
    mCircleATATFrequency = 0.25f;
    mSquadronMaxAngle = 0.0f;
    mSquadronSquaredDist = 0.0f;
    mTimeRequiredToEject = 4.0f;
    mEjectResist = 0.1f;
    mTimeTilReboard = 5.0f;

    mGuidedMissileDamage = 500.0f;
    mGuidedMissileDamageBoost = 2000.0f;

    for (int i = 0; i < 8; ++i)
    {
        mGuidedMissileHealthScale[i] = 1.0f;
    }

    mGuidedMissileShieldScale = 1.0f;
    mGuidedMissileDeathCamPause = 1.0f;
    mGuidedMissileDeathCamPullbackSpeed = 60.0f;
    mGuidedMissileDeathCamMinDistance = 20.0f;
    mGuidedMissileBoostExplosionSpeedThreshold = 0.8f;
}

// GLOBAL: SWBF2 0x01ebbcb8
// SYNTHETIC: SWBF2 0x00402ae0
// `dynamic initializer for 'EntityFlyerClass::rttiHashEntityFlyerClass''
PblHash EntityFlyerClass::rttiHashEntityFlyerClass("EntityFlyerClass");
char EntityFlyerClass::rttiNameEntityFlyerClass[] = "EntityFlyerClass";

// GLOBAL: SWBF2 0x01ebbc64
// SYNTHETIC: SWBF2 0x00402ad0
// `dynamic initializer for 'EntityFlyer::rttiHashEntityFlyer''
PblHash EntityFlyer::rttiHashEntityFlyer("EntityFlyer");

// FUNCTION: SWBF2 0x004a9cd0
unsigned int EntityFlyer::GetDerivedRtti() const
{
    return rttiHashEntityFlyer;
}

// FUNCTION: SWBF2 0x004a9ce0
const char* EntityFlyer::GetDerivedRttiName() const
{
    return "EntityFlyer";
}

// GLOBAL: SWBF2 0x01ebbc68
// SYNTHETIC: SWBF2 0x00402ab0
// `dynamic initializer for 'EntityFlyer::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f50
// `dynamic atexit destructor for 'EntityFlyer::sMemoryPool''
MemoryPool EntityFlyer::sMemoryPool("EntityFlyer", 0x1d90);

// FUNCTION: SWBF2 0x00477680
bool EntityFlyer::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityFlyer ||
           GameObject::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004ab7c0
const ControllableClass* EntityFlyer::GetControllableClass() const
{
    return mClass ? static_cast<const ControllableClass*>(mClass) : 0;
}

// FUNCTION: SWBF2 0x004ab7e0
ControllableClass* EntityFlyer::GetControllableClass()
{
    return mClass ? static_cast<ControllableClass*>(mClass) : 0;
}

// FUNCTION: SWBF2 0x004ab830
int EntityFlyer::GetWeaponCount() const
{
    return mClass->mWeaponCount;
}

// FUNCTION: SWBF2 0x004ab840
void EntityFlyer::SetTowCableWalker(GameObject* pWalker)
{
    if (mTowCableWalker != pWalker)
    {
        for (unsigned int i = 0; i < CameraManager::sInstance->GetNumCameras(); i++)
        {
            Trackable* pTarget = CameraManager::sInstance->GetChaseCameraTarget(i);
            if (pTarget && pTarget->GetGameObject() == this)
            {
                CameraManager::sInstance->mChaseCamera[i]->mTimer = 0.0f;
                break;
            }
        }
    }
    mTowCableWalker = pWalker;
}

// FUNCTION: SWBF2 0x004ac3a0
float EntityFlyer::GetFlyerMaxSpeed() const
{
    float speed = mClass->mMaxFlyerSpeed;
    if (mInLandingRegionFactor != 0.0f && speed >= 60.0f)
    {
        speed = 60.0f;
    }
    return speed;
}

// FUNCTION: SWBF2 0x004ac3e0
float EntityFlyer::GetFlyerMidSpeed() const
{
    float speed = mClass->mMidFlyerSpeed;
    if (mInLandingRegionFactor != 0.0f && speed >= 20.0f)
    {
        speed = 20.0f;
    }
    return speed;
}

// FUNCTION: SWBF2 0x004ac420
float EntityFlyer::GetFlyerMinSpeed() const
{
    float speed = mClass->mMinFlyerSpeed;
    if (mInLandingRegionFactor != 0.0f)
    {
        speed *= 0.2f;
        if (speed >= 10.0f)
        {
            speed = 10.0f;
        }
    }
    return speed;
}

// FUNCTION: SWBF2 0x004b62f0
float EntityFlyer::GetMaxSpeed() const
{
    return mClass->mMaxFlyerSpeed;
}

// FUNCTION: SWBF2 0x004b6310
int EntityFlyer::GetReservableSlots(ReserveType) const
{
    return mControllableList._iCount;
}

// FUNCTION: SWBF2 0x00477ac0
bool EntityFlyerClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityFlyerClass || GameObjectClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004a9d00
bool EntityFlyer::PostCollision::Update(float fDeltaTime)
{
    return mOwner->PostCollisionUpdate(fDeltaTime);
}

// SYNTHETIC: SWBF2 0x004a9d50
// EntityFlyer::PostCollision::~PostCollision

// SYNTHETIC: SWBF2 0x004a9d60
// EntityFlyer::PostCollision::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004a9d90
unsigned int EntityFlyerClass::GetDerivedRtti()
{
    return rttiHashEntityFlyerClass;
}

// FUNCTION: SWBF2 0x004a9da0
const char* EntityFlyerClass::GetDerivedRttiName()
{
    return rttiNameEntityFlyerClass;
}

// FUNCTION: SWBF2 0x004a9db0
Trackable* CameraManager::GetChaseCameraTarget(int iCam)
{
    if (mCurGameCamera[iCam] == mDeathCamera[iCam])
    {
        return mDeathCamera[iCam]->mOwner;
    }
    return mChaseCamera[iCam]->mOwner;
}

// FUNCTION: SWBF2 0x004b6f60
EntityFlyerClass::EntityFlyerClass(EntityFlyerClass* pSrc, unsigned int uHash)
    : GameObjectClass(pSrc, uHash),
      ControllableClass(pSrc),
      VehicleMusicClass(pSrc),
      VehicleEngineClass(pSrc)
{
    mEnergyBarClass.m_fEnergyMax = 0.0f;
    mEnergyBarClass.m_fEnergyMin = 0.0f;
    mEnergyBarClass.m_fEnergyOverheat = 0.0f;

    mFlyingCollisionInited = false;
    sCurrentSection = -1;
    sWeaponSection = 0;

    mAnimObj = pSrc->mAnimObj;
    mAnimTakeoff = pSrc->mAnimTakeoff;
    mAnimFins = pSrc->mAnimFins;
    mAcceleration = pSrc->mAcceleration;
    mBoostAcceleration = pSrc->mBoostAcceleration;
    mMinFlyerSpeed = pSrc->mMinFlyerSpeed;
    mMidFlyerSpeed = pSrc->mMidFlyerSpeed;
    mMaxFlyerSpeed = pSrc->mMaxFlyerSpeed;
    mBoostFlyerSpeed = pSrc->mBoostFlyerSpeed;
    mStrafeSpeed = pSrc->mStrafeSpeed;
    mPitchRate = pSrc->mPitchRate;
    mTurnRate = pSrc->mTurnRate;
    mPitchFilter = pSrc->mPitchFilter;
    mPitchFilterDecel = pSrc->mPitchFilterDecel;
    mTurnFilter = pSrc->mTurnFilter;
    mTurnFilterDecel = pSrc->mTurnFilterDecel;
    mPitchBuildupMultiplier = pSrc->mPitchBuildupMultiplier;
    mTurnBuildupMultiplier = pSrc->mTurnBuildupMultiplier;
    mThrustPitchAngle = pSrc->mThrustPitchAngle;
    mStrafeRollAngle = pSrc->mStrafeRollAngle;
    mBankAngle = pSrc->mBankAngle;
    mBankFilter = pSrc->mBankFilter;
    mLevelFilter = pSrc->mLevelFilter;
    mLevelFilterLanding = pSrc->mLevelFilterLanding;
    mRollRate = pSrc->mRollRate;
    mRollRateAccel = pSrc->mRollRateAccel;
    mTakeoffHeight = pSrc->mTakeoffHeight;
    mTakeoffTime = pSrc->mTakeoffTime;
    mTakeoffSpeed = pSrc->mTakeoffSpeed;
    mLandingTime = pSrc->mLandingTime;
    mLandingSpeed = pSrc->mLandingSpeed;
    mLandedHeight = pSrc->mLandedHeight;

    mBlurEffect = pSrc->mBlurEffect;
    mBlurStartV = pSrc->mBlurStartV;
    mFOVEffect1st = pSrc->mFOVEffect1st;
    mFOVEffect3rd = pSrc->mFOVEffect3rd;
    mFOVEFFectMinCamOffset3rd = pSrc->mFOVEFFectMinCamOffset3rd;
    mFOVEFFectMaxCamOffset3rd = pSrc->mFOVEFFectMaxCamOffset3rd;
    mEnergyBarClass = pSrc->mEnergyBarClass;
    mEnergyAutoRestore = pSrc->mEnergyAutoRestore;
    mEnergyBoostDrain = pSrc->mEnergyBoostDrain;
    mEnergyTrickDrainSingleTap = pSrc->mEnergyTrickDrainSingleTap;
    mEnergyTrickDrainDoubleTap = pSrc->mEnergyTrickDrainDoubleTap;
    mTrickRollSpeed = pSrc->mTrickRollSpeed;
    mTrickFlipSpeed = pSrc->mTrickFlipSpeed;
    mTrickFlipCameraDetach = pSrc->mTrickFlipCameraDetach;
    mTrickSideRollCameraDetach = pSrc->mTrickSideRollCameraDetach;
    mTrickSideRollStrafeSpeed = pSrc->mTrickSideRollStrafeSpeed;
    mMomentumFilter = pSrc->mMomentumFilter;

    for (int i = 0; i < 3; ++i)
    {
        mCollisionThreshold[i] = pSrc->mCollisionThreshold[i];
        mCollisionScale[i] = pSrc->mCollisionScale[i];
    }
    mCollisionInflict = pSrc->mCollisionInflict;
    mCrashShakeStart = pSrc->mCrashShakeStart;
    mCrashShakeEnd = pSrc->mCrashShakeEnd;
    mCrashShakeLength = pSrc->mCrashShakeLength;
    mMinCrashRollRate = pSrc->mMinCrashRollRate;
    mMaxCrashRollRate = pSrc->mMaxCrashRollRate;

    for (int i = 0; i < 4; ++i)
    {
        mWeaponClass[i] = pSrc->mWeaponClass[i];
        mWeaponAmmo[i] = pSrc->mWeaponAmmo[i];
        mWeaponChannel[i] = pSrc->mWeaponChannel[i];
    }
    mWeaponCount = pSrc->mWeaponCount;
    mVehicleType = VEHICLE_UNKNOWN;
    for (int i = 0; i < 4; ++i)
    {
        mAimerClass[i] = pSrc->mAimerClass[i];
    }

    mExplosionCritical = pSrc->mExplosionCritical;
    mExplosionDestruct = pSrc->mExplosionDestruct;
    mCameraSettings = pSrc->mCameraSettings;
    for (int i = 0; i < 8; ++i)
    {
        mTurretClass[i] = pSrc->mTurretClass[i];
    }
    mPassengerSlotClass = pSrc->mPassengerSlotClass;
    mNumPassengerSlots = pSrc->mNumPassengerSlots;

    mNoiseFactor = 1.0f;
    mBarrierX = pSrc->mBarrierX;
    mBarrierZ = pSrc->mBarrierZ;
    mBarrierOX = pSrc->mBarrierOX;
    mBarrierOZ = pSrc->mBarrierOZ;
    mLODClass = 1;

    for (int i = 0; i < 8; ++i)
    {
        mThrustEffectName[i] = pSrc->mThrustEffectName[i];
        mThrustEffectHardPoint[i] = pSrc->mThrustEffectHardPoint[i];
        mThrustEffectAttachOffset[i] = pSrc->mThrustEffectAttachOffset[i];
        mThrustEffectMinScale[i] = pSrc->mThrustEffectMinScale[i];
        mThrustEffectMaxScale[i] = pSrc->mThrustEffectMaxScale[i];
        mThrustEffectScaleStart[i] = pSrc->mThrustEffectScaleStart[i];
    }

    for (int i = 0; i < 12; ++i)
    {
        mContrailEffectName[i] = pSrc->mContrailEffectName[i];
        mContrailEffectHardPoint[i] = pSrc->mContrailEffectHardPoint[i];
        mContrailEffectAttachOffset[i] = pSrc->mContrailEffectAttachOffset[i];
        mContrailEffectMinScale[i] = pSrc->mContrailEffectMinScale[i];
        mContrailEffectMaxScale[i] = pSrc->mContrailEffectMaxScale[i];
        mContrailEffectMinSpeed[i] = pSrc->mContrailEffectMinSpeed[i];
        mContrailEffectMaxSpeed[i] = pSrc->mContrailEffectMaxSpeed[i];
    }

    mCockpitModelHandle = -1;
    mCollisionModel.mDenyFlyerLand = true;
    mFreeFlyTime = pSrc->mFreeFlyTime;
    mTrickTimeMid = pSrc->mTrickTimeMid;
    mTrickTimeRange = pSrc->mTrickTimeRange;
    mCircleATATFrequency = pSrc->mCircleATATFrequency;
    mSquadronMaxAngle = pSrc->mSquadronMaxAngle;
    mSquadronSquaredDist = pSrc->mSquadronSquaredDist;
    mTimeRequiredToEject = pSrc->mTimeRequiredToEject;
    mEjectResist = pSrc->mEjectResist;
    mTimeTilReboard = pSrc->mTimeTilReboard;
    mNeverCrashWhenUnpiloted = pSrc->mNeverCrashWhenUnpiloted;
    mGuidedMissile = pSrc->mGuidedMissile;
    mGuidedMissileDamage = pSrc->mGuidedMissileDamage;
    mGuidedMissileDamageBoost = pSrc->mGuidedMissileDamageBoost;
    for (int i = 0; i < 8; ++i)
    {
        mGuidedMissileHealthScale[i] = pSrc->mGuidedMissileHealthScale[i];
    }
    mGuidedMissileShieldScale = pSrc->mGuidedMissileShieldScale;
    mGuidedMissileDeathCamPause = pSrc->mGuidedMissileDeathCamPause;
    mGuidedMissileDeathCamPullbackSpeed = pSrc->mGuidedMissileDeathCamPullbackSpeed;
    mGuidedMissileDeathCamMinDistance = pSrc->mGuidedMissileDeathCamMinDistance;
    mGuidedMissileBoostExplosionSpeedThreshold = pSrc->mGuidedMissileBoostExplosionSpeedThreshold;
    mSoundTrick = pSrc->mSoundTrick;
    mSoundFlip = pSrc->mSoundFlip;
}

// FUNCTION: SWBF2 0x004b7a60
void EntityFlyerClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x4cbe283c)
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
    case 0x7b001920:
        sscanf(pszValue, "%f", &mBoostAcceleration);
        break;
    case 0xfbe2eb62:
        sscanf(pszValue, "%f", &mMinFlyerSpeed);
        break;
    case 0x838bda20:
        sscanf(pszValue, "%f", &mMidFlyerSpeed);
        break;
    case 0x86df0364:
        sscanf(pszValue, "%f", &mMaxFlyerSpeed);
        break;
    case 0xa5900707:
        sscanf(pszValue, "%f", &mBoostFlyerSpeed);
        break;
    case 0xfe1ce511:
        sscanf(pszValue, "%f", &mStrafeSpeed);
        break;
    case 0xb9b8abc3:
        sscanf(pszValue, "%f", &mPitchRate);
        break;
    case 0x0f77c4e8:
        sscanf(pszValue, "%f", &mTurnRate);
        break;
    case 0x9b0ca8c3:
        sscanf(pszValue, "%f", &mPitchFilter);
        break;
    case 0x53d5cdc8:
        sscanf(pszValue, "%f", &mPitchFilterDecel);
        break;
    case 0xde721cb4:
        sscanf(pszValue, "%f", &mTurnFilter);
        break;
    case 0x1795170d:
        sscanf(pszValue, "%f", &mTurnFilterDecel);
        break;
    case 0x4d32015d:
        sscanf(pszValue, "%f", &mPitchBuildupMultiplier);
        if (mPitchBuildupMultiplier < 1.0f)
        {
            mPitchBuildupMultiplier = 1.0f;
        }
        break;
    case 0x482c5704:
        sscanf(pszValue, "%f", &mTurnBuildupMultiplier);
        if (mTurnBuildupMultiplier < 1.0f)
        {
            mTurnBuildupMultiplier = 1.0f;
        }
        break;
    case 0x4c5b08a6:
        sscanf(pszValue, "%f", &mThrustPitchAngle);
        break;
    case 0xf1a4896c:
        sscanf(pszValue, "%f", &mStrafeRollAngle);
        break;
    case 0x6d7c9f7e:
        sscanf(pszValue, "%f", &mBankAngle);
        break;
    case 0x35de28f1:
        sscanf(pszValue, "%f", &mBankFilter);
        break;
    case 0xf4f5f38f:
        sscanf(pszValue, "%f", &mLevelFilter);
        break;
    case 0xbe55a77e:
        sscanf(pszValue, "%f", &mLevelFilterLanding);
        break;
    case 0x24eaf25c:
        sscanf(pszValue, "%f", &mRollRate);
        break;
    case 0x61151c4c:
        sscanf(pszValue, "%f", &mRollRateAccel);
        break;
    case 0x5107fafc:
        sscanf(pszValue, "%f", &mTakeoffHeight);
        break;
    case 0x61195c0a:
        sscanf(pszValue, "%f", &mTakeoffTime);
        break;
    case 0x8255424e:
        sscanf(pszValue, "%f", &mTakeoffSpeed);
        break;
    case 0x1283209d:
        sscanf(pszValue, "%f", &mLandingTime);
        break;
    case 0x6ae3a957:
        sscanf(pszValue, "%f", &mLandingSpeed);
        break;
    case 0x7642977a:
        sscanf(pszValue, "%f", &mLandedHeight);
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

    case 0xb9282909:
        sscanf(pszValue, "%f", &mBlurEffect);
        break;
    case 0x04afc6a4:
        sscanf(pszValue, "%f", &mBlurStartV);
        break;
    case 0xaaa6d95c:
        sscanf(pszValue, "%f", &mFOVEffect3rd);
        mFOVEffect3rd *= 0.017453292f;
        break;
    case 0x5dbcabb0:
    {
        float x, y, z;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mFOVEFFectMinCamOffset3rd.x = x;
        mFOVEFFectMinCamOffset3rd.y = y;
        mFOVEFFectMinCamOffset3rd.z = z;
        break;
    }
    case 0x8d0cfc36:
    {
        float x, y, z;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mFOVEFFectMaxCamOffset3rd.x = x;
        mFOVEFFectMaxCamOffset3rd.y = y;
        mFOVEFFectMaxCamOffset3rd.z = z;
        break;
    }
    case 0xd158caab:
        sscanf(pszValue, "%f", &mFOVEffect1st);
        mFOVEffect1st *= 0.017453292f;
        break;

    case 0x4f9d8bb4:
        sscanf(pszValue, "%f", &mEnergyAutoRestore);
        break;
    case 0x645f56f6:
        sscanf(pszValue, "%f", &mEnergyBoostDrain);
        break;
    case 0x4cd7e18f:
        sscanf(pszValue, "%f", &mEnergyTrickDrainSingleTap);
        mEnergyBarClass.SetPropertyEnergyCost(mEnergyTrickDrainSingleTap);
        break;
    case 0x89c72ee4:
        sscanf(pszValue, "%f", &mEnergyTrickDrainDoubleTap);
        mEnergyBarClass.SetPropertyEnergyCost(mEnergyTrickDrainDoubleTap);
        break;
    case 0xa69ed102:
        sscanf(pszValue, "%f", &mTrickRollSpeed);
        break;
    case 0xb9ab019a:
        sscanf(pszValue, "%f", &mTrickFlipSpeed);
        break;
    case 0x77eb7aa7:
        sscanf(pszValue, "%f", &mTrickFlipCameraDetach);
        break;
    case 0xc4ae94d0:
        sscanf(pszValue, "%f", &mTrickSideRollCameraDetach);
        break;
    case 0x3dfbe65e:
        sscanf(pszValue, "%f", &mTrickSideRollStrafeSpeed);
        break;
    case 0xd937dce9:
        sscanf(pszValue, "%f", &mMomentumFilter);
        break;
    case 0x7e6b2efb:
        mSoundTrick.SetID(pszValue, true, false);
        break;
    case 0x3da7ba35:
        mSoundFlip.SetID(pszValue, true, false);
        break;

    case 0x02bb1fe0:
    case 0x9b30e763:
    case 0x8a323f32:
        mExplosionDestruct = ExplosionClass::Find(PblHash(pszValue));
        break;
    case 0xe04f08c1:
        mExplosionCritical = ExplosionClass::Find(PblHash(pszValue));
        break;

    case 0x64c511a4:
    {
        mPassengerSlotClass = new PassengerSlotClass(mId);
        int iNumSlots = atoi(pszValue);
        sPassengerSlotSection = true;
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
        mCockpitModelHandle = FirstPerson::AddGeometry(pszValue);
        break;
    case 0x98557a5e:
        InitAnimations(pszValue);
        break;
    case 0xef563a98:
        if (mAnimObj)
        {
            mAnimFins = const_cast<ZephyrAnim*>(mAnimObj->FindAnimation(pszValue));
        }
        break;

    case 0x0894d756:
        sCurThrustEffect = -1;
        for (int i = 0; i < 8; ++i)
        {
            if (!mThrustEffectName[i])
            {
                sCurThrustEffect = i;
                break;
            }
        }
        if (sCurThrustEffect != -1)
        {
            mThrustEffectName[sCurThrustEffect] = PblHash(pszValue);
        }
        break;
    case 0x2e0557f0:
        if (sCurThrustEffect != -1)
        {
            mThrustEffectHardPoint[sCurThrustEffect] = PblTEMPHash(pszValue);
        }
        break;
    case 0x34775769:
        if (sCurThrustEffect != -1)
        {
            float x, y, z;
            sscanf(pszValue, "%f %f %f", &x, &y, &z);
            mThrustEffectAttachOffset[sCurThrustEffect].x = x;
            mThrustEffectAttachOffset[sCurThrustEffect].y = y;
            mThrustEffectAttachOffset[sCurThrustEffect].z = z;
        }
        break;
    case 0xf174a3f8:
        if (sCurThrustEffect != -1)
        {
            mThrustEffectMinScale[sCurThrustEffect] = (float)atof(pszValue);
        }
        break;
    case 0xc3fd784a:
        if (sCurThrustEffect != -1)
        {
            mThrustEffectMaxScale[sCurThrustEffect] = (float)atof(pszValue);
        }
        break;
    case 0xe97041b8:
        if (sCurThrustEffect != -1)
        {
            mThrustEffectScaleStart[sCurThrustEffect] = (float)atof(pszValue);
        }
        break;

    case 0x050dfa58:
        sCurContrailEffect = -1;
        for (int i = 0; i < 12; ++i)
        {
            if (!mContrailEffectName[i])
            {
                sCurContrailEffect = i;
                break;
            }
        }
        if (sCurContrailEffect != -1)
        {
            mContrailEffectName[sCurContrailEffect] = PblHash(pszValue);
        }
        break;
    case 0x58b81b76:
        if (sCurContrailEffect != -1)
        {
            mContrailEffectHardPoint[sCurContrailEffect] = PblTEMPHash(pszValue);
        }
        break;
    case 0x84c7e963:
        if (sCurContrailEffect != -1)
        {
            float x, y, z;
            sscanf(pszValue, "%f %f %f", &x, &y, &z);
            mContrailEffectAttachOffset[sCurContrailEffect].x = x;
            mContrailEffectAttachOffset[sCurContrailEffect].y = y;
            mContrailEffectAttachOffset[sCurContrailEffect].z = z;
        }
        break;
    case 0x86caf00e:
        if (sCurContrailEffect != -1)
        {
            mContrailEffectMinScale[sCurContrailEffect] = (float)atof(pszValue);
        }
        break;
    case 0x72a09574:
        if (sCurContrailEffect != -1)
        {
            mContrailEffectMaxScale[sCurContrailEffect] = (float)atof(pszValue);
        }
        break;
    case 0x75ee599f:
        if (sCurContrailEffect != -1)
        {
            mContrailEffectMinSpeed[sCurContrailEffect] = (float)atof(pszValue);
        }
        break;
    case 0x64e60f49:
        if (sCurContrailEffect != -1)
        {
            mContrailEffectMaxSpeed[sCurContrailEffect] = (float)atof(pszValue);
        }
        break;

    case 0xc80a91f1:
        sscanf(pszValue, "%f", &mFreeFlyTime);
        break;
    case 0xc4605e25:
        sscanf(pszValue, "%f", &mTrickTimeMid);
        break;
    case 0x2dc13b12:
        sscanf(pszValue, "%f", &mTrickTimeRange);
        break;
    case 0x0490b19d:
        sscanf(pszValue, "%f", &mCircleATATFrequency);
        break;
    case 0x459ce661:
        sscanf(pszValue, "%f", &mSquadronMaxAngle);
        mSquadronMaxAngle = PblMath::Cos(mSquadronMaxAngle * 0.017453292f);
        break;
    case 0x14989eaf:
        sscanf(pszValue, "%f", &mSquadronSquaredDist);
        mSquadronSquaredDist *= mSquadronSquaredDist;
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
    case 0xdf92a1d2:
        mNeverCrashWhenUnpiloted = atoi(pszValue) != 0;
        break;
    case 0xc6e9c80b:
        mGuidedMissile = atoi(pszValue) != 0;
        if (mGuidedMissile)
        {
            mPilotType = PILOT_REMOTE;
        }
        break;
    case 0x376b238a:
        sscanf(pszValue, "%f", &mGuidedMissileDamage);
        break;
    case 0x487b2329:
        sscanf(pszValue, "%f", &mGuidedMissileDamageBoost);
        break;
    case 0xc37970cf:
    {
        float fScale;
        sscanf(pszValue, "%f", &fScale);
        for (int i = 0; i < 3; ++i)
        {
            mGuidedMissileHealthScale[i] = fScale;
        }
        break;
    }
    case 0x0bb94602:
        sscanf(pszValue, "%f", &mGuidedMissileHealthScale[0]);
        break;
    case 0x9aa17a49:
        sscanf(pszValue, "%f", &mGuidedMissileHealthScale[1]);
        break;
    case 0x338a07cd:
        sscanf(pszValue, "%f", &mGuidedMissileHealthScale[2]);
        break;
    case 0x6ccdc5ef:
        sscanf(pszValue, "%f", &mGuidedMissileHealthScale[3]);
        break;
    case 0xa0bb60ce:
    {
        float fScale;
        sscanf(pszValue, "%f", &fScale);
        for (int i = 3; i < 7; ++i)
        {
            mGuidedMissileHealthScale[i] = fScale;
        }
        break;
    }
    case 0x02168f61:
    {
        float fScale;
        sscanf(pszValue, "%f", &fScale);
        for (int i = 4; i < 7; ++i)
        {
            mGuidedMissileHealthScale[i] = fScale;
        }
        break;
    }
    case 0xd0545c72:
        sscanf(pszValue, "%f", &mGuidedMissileShieldScale);
        break;
    case 0x9bdfa490:
        sscanf(pszValue, "%f", &mGuidedMissileDeathCamPause);
        break;
    case 0x962e0745:
        sscanf(pszValue, "%f", &mGuidedMissileDeathCamPullbackSpeed);
        break;
    case 0x325bf479:
        sscanf(pszValue, "%f", &mGuidedMissileDeathCamMinDistance);
        break;
    case 0xbb3082af:
        sscanf(pszValue, "%f", &mGuidedMissileBoostExplosionSpeedThreshold);
        break;

    case 0x47c86b4a:
    {
        GameObjectClass::SetProperty(uProperty, pszValue);
        sSoftCollisionMask = 0;
        sOrdnanceCollisionMask = 0;
        int iNumBodies = mCollisionModel.GetNumBodies();
        const CollisionMesh* pMesh = CollisionMesh::Find(PblHash(pszValue));
        if (pMesh)
        {
            mCollisionModel.AddBody(new CollisionBodyMesh(pMesh, "CollisionMesh", _IdentityMatrix, 0));
            int iMaskId = mCollisionModel.GetMaskId(0x29173102);
            if (iMaskId >= 0)
            {
                sSoftCollisionMask |= 1 << iMaskId;
                sOrdnanceCollisionMask |= 1 << iMaskId;
            }
            PblBox box = pMesh->GetWorldspaceAABB(_IdentityMatrix);
            mBarrierX = box._vMax.x - box._vMin.x;
            mBarrierZ = box._vMax.z - box._vMin.z;
            mBarrierOX = (box._vMin.x + box._vMax.x) * 0.5f;
            mBarrierOZ = (box._vMin.z + box._vMax.z) * 0.5f;
        }
        if (mModel)
        {
            mCameraSettings.mCameraSafePoint.x = 0.0f;
            mCameraSettings.mCameraSafePoint.y = mModel->_mModel->_TotalBoundingBox._vMax.y * 0.9f;
            mCameraSettings.mCameraSafePoint.z = 0.0f;
        }
        if (iNumBodies == 0 && mModel)
        {
            const PblBox& box = mModel->_mModel->_TotalBoundingBox;
            float fCenterX = (box._vMin.x + box._vMax.x) * 0.5f;
            float fCenterY = (box._vMin.y + box._vMax.y) * 0.5f;
            float fCenterZ = (box._vMin.z + box._vMax.z) * 0.5f;
            float fHalfX = (box._vMax.x - box._vMin.x) * 0.5f;
            float fHalfY = (box._vMax.y - box._vMin.y) * 0.5f;
            float fHalfZ = (box._vMax.z - box._vMin.z) * 0.5f;
            CollisionPrimitive primitive;
            PblMatrix matrix = _IdentityMatrix;
            primitive.SetAACylinder((fHalfZ + fHalfX) * 0.5f, fHalfY * 2.0f);
            matrix.m[3][0] = fCenterX;
            matrix.m[3][1] = fCenterY;
            matrix.m[3][2] = fCenterZ;
            matrix.m[3][3] = 1.0f;
            mCollisionModel.AddBody(new CollisionBodyPrimitive(primitive, "main_body", matrix, 0));
        }
        if (pMesh)
        {
            CollisionMask softMask = mCollisionModel.GetCollisionMask(COLL_SOFT);
            CollisionMask ordnanceMask = mCollisionModel.GetCollisionMask(COLL_ORDNANCE);
            for (int i = 0; i < 32; ++i)
            {
                if (sSoftCollisionMask & (1u << i))
                {
                    softMask.SetId(i);
                }
                if (sOrdnanceCollisionMask & (1u << i))
                {
                    ordnanceMask.SetId(i);
                }
            }
            mCollisionModel.SetCollisionMask(COLL_SOFT, softMask);
            mCollisionModel.SetCollisionMask(COLL_ORDNANCE, ordnanceMask);
        }
        if (mModel)
        {
            mLandedHeight = -mModel->_mModel->_TotalBoundingBox._vMin.y;
        }
        mCollisionModel.mIsFlyerFlag = true;
        break;
    }
    case 0x5dfdc07f:
    {
        int iMaskId = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (iMaskId >= 0)
        {
            sSoftCollisionMask |= 1 << iMaskId;
            mCollisionModel.SetCollisionMask(COLL_SOFT, CollisionMask(sSoftCollisionMask));
        }
        break;
    }
    case 0xfb2bdf07:
    {
        int iMaskId = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (iMaskId >= 0)
        {
            sOrdnanceCollisionMask |= 1 << iMaskId;
            mCollisionModel.SetCollisionMask(COLL_ORDNANCE, CollisionMask(sOrdnanceCollisionMask));
        }
        break;
    }

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
            !EntityPathFollowerClass::SetProperty(uProperty, pszValue) &&
            !mAimerClass[sWeaponSection].SetProperty(uProperty, pszValue, pModel, 0) &&
            !mCameraSettings.SetProperty(uProperty, pszValue) &&
            !ControllableClass::SetProperty(uProperty, pszValue, pModel))
        {
            GameObjectClass::SetProperty(uProperty, pszValue);
        }
        break;
    }
    }
}

// FUNCTION: SWBF2 0x004b9520
EntityClass* EntityFlyerClass::Derive(unsigned int uHash)
{
    return new EntityFlyerClass(this, uHash);
}

namespace EventManager
{

// FUNCTION: SWBF2 0x004b9820
template <>
void Event<Character, GameObject>::Callback::Trigger(Character& item, GameObject& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild->IsAlive())
            {
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
                if (!pChild)
                {
                    break;
                }
            }
            if (pChild)
            {
                pChild->Release();
            }
        }
    }
    Release();
}
}

namespace EventManager
{

// FUNCTION: SWBF2 0x004b97e0
template <>
void Event<Character, GameObject>::Trigger(Character& item, GameObject& param)
{
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        mRoot.Trigger(item, param);
    }
}
}

extern PblVector3 gVectorZero;

void* EntityFlyer::operator new(unsigned int uSize)
{
    return sMemoryPool.Allocate(uSize);
}

void EntityFlyer::operator delete(void* pBlock)
{
    sMemoryPool.Free(pBlock);
}

// FUNCTION: SWBF2 0x004776c0
GameObject* EntityFlyer::GetGameObject()
{
    return this;
}

// FUNCTION: SWBF2 0x004776d0
const GameObject* EntityFlyer::GetGameObject() const
{
    return this;
}

// FUNCTION: SWBF2 0x004776e0
Entity* EntityFlyer::GetEntity()
{
    return this;
}

// FUNCTION: SWBF2 0x004776f0
const Entity* EntityFlyer::GetEntity() const
{
    return this;
}

// FUNCTION: SWBF2 0x00477700
const PblMatrix& EntityFlyer::GetMatrix() const
{
    return mMatrix;
}

// SYNTHETIC: SWBF2 0x00478584
// [thunk]:EntityFlyer::GetGameObject`adjustor{600}'

// SYNTHETIC: SWBF2 0x004785a5
// [thunk]:EntityFlyer::GetGameObject`adjustor{12}'

// SYNTHETIC: SWBF2 0x00478524
// [thunk]:EntityFlyer::GetMatrix`adjustor{564}'

// SYNTHETIC: SWBF2 0x0047856e
// [thunk]:EntityFlyer::GetGameObject`adjustor{320}'

// SYNTHETIC: SWBF2 0x0047858f
// [thunk]:EntityFlyer::GetGameObject`adjustor{600}'

// SYNTHETIC: SWBF2 0x004785b5
// [thunk]:EntityFlyer::GetEntity`adjustor{12}'

// SYNTHETIC: SWBF2 0x004785c5
// [thunk]:EntityFlyer::GetEntity`adjustor{576}'

// SYNTHETIC: SWBF2 0x004785d0
// [thunk]:EntityFlyer::GetEntity`adjustor{576}'

// SYNTHETIC: SWBF2 0x004b9942
// [thunk]:EntityFlyer::`scalar deleting destructor'`adjustor{576}'

// FUNCTION: SWBF2 0x004ac0c0
bool EntityFlyer::ResetSquadronTime()
{
    if (mProcedurallyAnimated || mAIEng.ai.mSquadronFormationTime > 0.0f)
    {
        return false;
    }
    mAIEng.ai.mSquadronFormationTime = PblRandom::GetGlobalFloatRange(7.0f, 3.0f);
    return true;
}

// FUNCTION: SWBF2 0x004b6610
void EntityFlyer::SendLandedEvent()
{
    if (mCharacter)
    {
        GameObject* pObject = GetGameObject();
        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            EventManager::CharacterEvent::mLandedFlyer.Trigger(*mCharacter, *pObject);
        }
    }
}

// FUNCTION: SWBF2 0x004aa960
void EntityFlyer::InitAsLanded()
{
    mOmega = gVectorZero;
    mVelocity.z = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.x = 0.0f;
    mSetSpeed = 0.0f;
    mSetStrafe = 0.0f;
    mSetStrafeVertical = 0.0f;
    mState = LANDED;
    SendLandedEvent();
    mWorldRoll = 0.0f;
    mAutoLevelingAllowed = 15.0f;
    mFlightRatio = 0.0f;
    mCrashTimer = 0.0f;
    const float fHeight = mClass->mLandedHeight;
    const PblVector3& vPos = GetPosition();
    PblVector3 vLanded;
    vLanded.x = vPos.x + 0.0f;
    vLanded.y = vPos.y + fHeight;
    vLanded.z = vPos.z + 0.0f;
    SetPosition(vLanded);
}

// FUNCTION: SWBF2 0x004a9de0
EntityFlyer::EntityFlyer(EntityFlyerClass* pClass, const EntityDesc& desc)
    : GameObject(pClass, desc, COLL_RIGID, &pClass->mModel->_mLodData)
    , Controllable(pClass, desc.mPlayerId)
    , EntityPathFollower(pClass)
    , VehicleMusic(pClass)
    , VehicleEngine(pClass)
    , mEnergyBar(&pClass->mEnergyBarClass)
    , mTurbulance(0.0f)
    , mObstacle(0)
    , mPostCollision(this)
{
    mControllableList.AddLast(&this->Controllable::mNode, this);
    mTrickEngineValue = 0.0f;
    memset(mThrustEffect, 0, sizeof(mThrustEffect));
    memset(mContrailEffect, 0, sizeof(mContrailEffect));

    mNumTurrets = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (pClass->mTurretClass[i])
        {
            mTurret[mNumTurrets] = new MountedTurret(this, pClass->mTurretClass[i], &mPose);
            mControllableList.AddLast(&mTurret[mNumTurrets]->mNode, mTurret[mNumTurrets]);
            ++mNumTurrets;
        }
    }

    mClass = pClass;
    if (mClass->mPassengerSlotClass)
    {
        for (int i = 0; i < mClass->mNumPassengerSlots; ++i)
        {
            mPassengerSlots[i] = new PassengerSlot(this, mClass->mPassengerSlotClass, &mPose);
            mControllableList.AddLast(&mPassengerSlots[i]->mNode, mPassengerSlots[i]);
        }
    }

    mEyeDir = mMatrix.GetZAxis();
    mEyePoint = mClass->mCameraSettings.CalcEyePoint(mMatrix);
    mAnimObj = pClass->mAnimObj;
    AddToScene();

    if (mClass->mGuidedMissile)
    {
        InitAsFlying();
    }
    else
    {
        InitAsLanded();
    }

    mInRoll = false;
    mBankPitchAngle = 0.0f;
    mBankPitchAngleDelta = 0.0f;
    mBankRollAngle = 0.0f;
    mBankRollAngleDelta = 0.0f;
    mLastAltitude = 0.0f;
    mRoll = 0.0f;
    mRollTarget = 0.0f;
    mSideRoll.x = 0.0f;
    mSideRoll.y = 0.0f;
    mSideRoll.z = 0.0f;
    mInFlip = false;
    mLastTrickPerformed = TRICK_MAX;
    mPitchControllerPrevUpdate = 0.0f;
    mTurnControllerPrevUpdate = 0.0f;
    mFlip = 0.0f;
    mFlipTarget = 0.0f;
    mBoost = false;
    mGetSpeedSpeed = 0.0f;
    mTrickCameraInterpDetach = 0.0f;
    mTrickCameraInterpDir = 0.0f;
    mTrickCameraDetach = 0.0f;
    mTrick = -1.0f;
    mTrickDoubleclick = 0.0f;
    mInLandingRegionFlag = false;
    mInLandingRegionFlag_ToolTip = true;
    mInLandingRegionFactor = 0.0f;
    mFlyerAutoCorrectingToStayInBattlefield = false;
    mAutoLevelingAllowed = 15.0f;
    mWorldRoll = 0.0f;
    mCrashTimer = 0.0f;
    mFollowingPath = false;
    mLandedHeight = mClass->mLandedHeight;
    ResetSquadronTime();

    mAmbientShift.x = 0.0f;
    mAmbientShift.y = 0.0f;
    mAmbientShift.z = 0.0f;
    mAmbientShiftTarget.x = 0.1f;
    mAmbientShiftTarget.y = 0.08f;
    mAmbientShiftTarget.z = 0.06f;
    mAmbientShiftSpeedMax = 0.4f;
    mAmbientShiftSpeed.x = 0.0f;
    mAmbientShiftSpeed.y = 0.0f;
    mAmbientShiftSpeed.z = 0.0f;
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
        mAimer[i] = new Aimer(&pClass->mAimerClass[i], this, &mMatrix, &mPose, 0);
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

    TargetManager::AddTarget(this, true);
    ClearControllable();
    VehicleManager::Add(this);

    if (mAnimObj)
    {
        ZephyrPoseStatic<32> kPoseFinal;
        mZephyrSkeleton.Open(*mClass->mAnimObj->GetZephyrSkeletonShared(), kPoseFinal);
        kPoseFinal.Open(mZephyrSkeleton);
        mZephyrPoseDyn.Open(mZephyrSkeleton);
        if (mClass->mAnimTakeoff)
        {
            mZephyrPoseDyn.SetAnimation(mClass->mAnimTakeoff, 30.0f);
            mZephyrPoseDyn.SetLoop(false);
            mZephyrPoseDyn.SetFrameNum(0);
            kPoseFinal.Set(mZephyrPoseDyn, true);
            mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
            mPose.ConvertFromZephyrPose(mZephyrSkeleton);
        }
    }
    mModel->GetRedModel(0)->SetupPose(mPose);
    NetGame::Created(this);

    mSpeedPrev = 0.0f;
    mNextBarrierCheck = -1.0f;
    mTowCableWalker = 0;
    mReserveForPlayer = 0.0f;
    memset(&mAIEng, 0, sizeof(mAIEng));
    if (!mProcedurallyAnimated)
    {
        mAIEng.ai.mFreeFlyTime = 0.0f;
        mAIEng.ai.mTrickTime = 0.0f;
        mAIEng.ai.mTimeToNextTrick = 0.0f;
        mAIEng.ai.mReactionTime = 0.0f;
        mAIEng.ai.mReactionThinkTime = 0.0f;
        mAIEng.ai.mSquadronFormationTime = 0.0f;
        mAIEng.ai.mTimeSinceLastHack = 0.0f;
    }
    mTotalUpdateDt = 0.0f;
    mJustRendered = false;
    mLastDistanceToTarget = mClass->mProximityMax;
}

// SYNTHETIC: SWBF2 0x004a9d20
// EntityFlyer::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004aab30
EntityFlyer::~EntityFlyer()
{
    if (mFollowingPath)
    {
        EntityPathFollower::Reset(mClass);
    }
    NetGame::Destroyed(this);
    mControllableList.Remove(&this->Controllable::mNode);
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
    TargetManager::RemoveTarget(this);
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        delete mWeapon[i];
        delete mAimer[i];
    }
    VehicleManager::Remove(this);
}

// FUNCTION: SWBF2 0x004aa790
void EntityFlyer::Init()
{
    mBoost = false;
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
    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        for (PblList<Controllable>::Itor it(list); it; ++it)
        {
            it->SetupController();
        }
    }
    PathFinder::CreateGameObjectBarrier(mObstacle, this, mClass->mBarrierX * 0.5f, mClass->mBarrierZ * 0.5f,
                                        mClass->mBarrierOX, mClass->mBarrierOZ, 0);
}

// FUNCTION: SWBF2 0x004b5f70
void EntityFlyer::ActivatePhysics()
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

// FUNCTION: SWBF2 0x004b9590
Entity* EntityFlyerClass::Build(const EntityDesc& desc)
{
    if (!mFlyingCollisionInited)
    {
        mFlyingCollisionModel = mCollisionModel;
        CollisionMask mask = mCollisionModel.GetCollisionMask(COLL_ORDNANCE);
        CollisionMask special = mCollisionModel.GetCollisionMask(COLL_SPECIAL);
        for (int i = 0; i < 64; ++i)
        {
            if (special.IsSet(i))
            {
                mask.SetId(i);
            }
        }
        mFlyingCollisionModel.SetCollisionMask(COLL_ORDNANCE, mask);
        mFlyingCollisionInited = true;
    }

    if (EntityFlyer::sMemoryPool.IsFull())
    {
        return 0;
    }

    EntityFlyer* pFlyer = new EntityFlyer(this, desc);
    if (pFlyer)
    {
        CollisionManager::AddSoftBody(pFlyer);
        pFlyer->ActivatePhysics();
        pFlyer->BuildEffects();
    }
    return pFlyer;
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// FUNCTION: SWBF2 0x004b4eb0
void EntityFlyer::SetupController()
{
    Controller* pController = 0;
    if (mPlayerId >= 0)
    {
        pController = new PlayerController(this);
        Track(mPlayerId);
        mFollowingPath = false;
    }
    else if (mPilot)
    {
        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            pController = new UnitFlyController(this);
        }
        mFollowingPath = EntityPath::AllPathsIsClassSupported(mClass->mEntityClassID);
    }
    if (!mCtrl && pController)
    {
        AddController(pController, mPilot->mPlayerId < 0);
    }
    ClearControllable();
}

 
 

// FUNCTION: SWBF2 0x004b37e0
bool EntityFlyer::UpdateIndirect(float fDeltaTime)
{
    AILowLevel& lowLevel = UnitControllerPtr(mCtrl)->GetLowLevel();
    lowLevel.UpdateIndirect(fDeltaTime);
    if (!mCtrl)
    {
        return true;
    }

    if (mState == TAKEOFF || mState == FLYING)
    {
        if (!mFollowingPath || mState != FLYING || !IsAlive() || mAIEng.ai.mFreeFlyTime > 0.0f || mInRoll ||
            mInFlip)
        {
            float fThrust = 1.0f;
            float fStrafe = 0.0f;
            float fTurn = 0.0f;
            float fPitch = 0.0f;
            float fSpeedScale = 1.0f;
            if (mState != TAKEOFF)
            {
                ControllableDestination dest;
                PblVector3 vPos;
                PblVector3 vDir;
                if (CalculateDest(fDeltaTime, &dest, &vPos, &fSpeedScale, &vDir))
                {
                    dest.mHasDest = true;
                    dest.mPos = vPos;
                    fPitch = mControlPitch;
                    fTurn = mControlZoom.IsDown() ? 0.0f : mControlTurn;
                    AIUtil::UpdatePhysics(this, fDeltaTime, dest, lowLevel.mHeading, GetFlyerMaxSpeed(),
                               mClass->mStrafeSpeed, mClass->mTurnRate, mClass->mPitchRate, &fThrust, &fStrafe,
                               &fTurn, &fPitch);
                }
            }

            float fRatio = fSpeedScale;
            float fMinRatio = lowLevel.mMinControlSpeed / GetFlyerMaxSpeed();
            if (fMinRatio > fRatio)
            {
                fRatio = fMinRatio;
            }
            float fFloor = AIUtil::GetTargetSpeedForRealSpeed(5.0f, GetFlyerMaxSpeed());
            if (!(fRatio > fFloor))
            {
                fRatio = fFloor;
            }
            float fSpeed = AIUtil::GetThrustForTargetSpeed(fRatio, GetFlyerMinSpeed(), GetFlyerMidSpeed(), GetFlyerMaxSpeed());
            mControlStrafe = fSpeed * fStrafe;
            mControlPitch = fPitch;
            mControlMove = fSpeed * fThrust;
            mControlTurn = fTurn;
        }
    }

    if (!mInRoll && !mInFlip && lowLevel.GetTarget() &&
        (!mCharacter || !mCharacter->mCarriedFlagPtr) && !AIUtil::gGroundFlyerMap)
    {
        const PblMatrix& mTarget = lowLevel.GetTarget()->GetMatrix();
        const PblMatrix& mMine = GetMatrix();
        PblVector3 vDelta(mTarget.GetTranslation().x - mMine.GetTranslation().x,
                          mTarget.GetTranslation().y - mMine.GetTranslation().y,
                          mTarget.GetTranslation().z - mMine.GetTranslation().z);
        float fDistSq = vDelta.y * vDelta.y + vDelta.x * vDelta.x + vDelta.z * vDelta.z;
        if (40000.0f > fDistSq)
        {
            bool bReact = true;
            if (!(1600.0f > fDistSq))
            {
                PblVector3 vDir;
                D3DXVec3Normalize(&vDir, &vDelta);
                const PblVector3& vFront = mMine.GetZAxis();
                if (!(0.965925f > vFront.y * vDir.y + vFront.x * vDir.x + vFront.z * vDir.z))
                {
                    bReact = false;
                }
            }
            if (bReact)
            {
                const PblVector3& vFront = mMine.GetZAxis();
                float fDot = vFront.y * vDelta.y + vFront.x * vDelta.x + vFront.z * vDelta.z;
                PblVector3 vSide(vDelta.x - vFront.x * fDot, vDelta.y - vFront.y * fDot,
                                 vDelta.z - vFront.z * fDot);
                mSideRoll = AIUtil::GetSafeNormal(vSide);
                DoTrick(SIDE_ROLL);
            }
        }
    }

    if (mState == FLYING)
    {
        if (mFireTimer > 0.0f && mClass->mbFlyerVsFlyer)
        {
            mFireTimer -= fDeltaTime;
            if (0.6f > mFireTimer)
            {
                lowLevel.SetFireRequest(true, false);
                if (0.0001f >= mFireTimer)
                {
                    mFireTimer = PblRandom::GetGlobalFloatRange(6.0f, 1.5f);
                }
            }
        }

        lowLevel.ProcessFire(fDeltaTime, &lowLevel.mHeading);
    }
    return true;
}

extern bool netFrameLock;

extern bool bRenderContrails;

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);

// FUNCTION: SWBF2 0x004aae10
void EntityFlyer::HealthChangeCallback(bool bArg, float fDelta)
{
    if (bArg)
    {
        mState = CRASHED;
    }
    Damageable::HealthChangeCallback(bArg, fDelta);
}

// FUNCTION: SWBF2 0x004aae40
void EntityFlyer::Kill()
{
    mIsAlive = false;
    if (mObstacle)
    {
        PathFinder::DeleteGameObjectBarrier(mObstacle);
    }
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

// FUNCTION: SWBF2 0x004aafa0
bool EntityFlyer::RenderTrackable(unsigned int uView) const
{
    for (int i = 0; i < mNumTurrets; ++i)
    {
        if (!mTurret[i]->RenderTrackable(mTurret[i]->mPlayerId))
        {
            return false;
        }
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

// FUNCTION: SWBF2 0x004ab040
void EntityFlyer::Render(unsigned char renderMode, float lod, unsigned int flags)
{
    if (!RenderTrackable(mPlayerId))
    {
        return;
    }

    if (mClass->m_bDynamicLighting)
    {
        flags &= ~0x80;
    }
    else
    {
        flags |= 0x80;
    }

    if (NetGame::IsEnabled() && !netFrameLock)
    {
        mJustRendered = true;
    }

    if (mAnimObj)
    {
        if (mClass->mAnimTakeoff)
        {
            float fRatio = mFlightRatio;
            if (NetGame::IsEnabled() && !netFrameLock && fRatio > 0.0001f && fRatio < 0.9999f)
            {
                float fStep = (1.0f - NetGame::GetTurnRatio()) * mTotalUpdateDt;
                if (mState == LANDING && mClass->mLandingTime > 0.0f)
                {
                    fRatio += fStep / mClass->mLandingTime;
                    if (0.0f > fRatio)
                    {
                        fRatio = 0.0f;
                    }
                    else if (fRatio > 1.0f)
                    {
                        fRatio = 1.0f;
                    }
                }
                else if (mState == TAKEOFF && mClass->mTakeoffTime > 0.0f)
                {
                    fRatio -= fStep / mClass->mTakeoffTime;
                    if (0.0f > fRatio)
                    {
                        fRatio = 0.0f;
                    }
                    else if (fRatio > 1.0f)
                    {
                        fRatio = 1.0f;
                    }
                }
            }
            mZephyrPoseDyn.SetAnimation(mClass->mAnimTakeoff, 30.0f);
            mZephyrPoseDyn.SetFrameNumFloat((float)(mZephyrPoseDyn.GetNumFrames() - 1) * fRatio);
            ZephyrPoseStatic<32> kPose;
            mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
            kPose.Open(mZephyrSkeleton);
            kPose.Set(mZephyrPoseDyn, true);
            mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
            mPose.ConvertFromZephyrPose(mZephyrSkeleton);
        }
        else if (mClass->mAnimFins)
        {
            float fTurn = mControlZoom.IsDown() ? 0.0f : mControlTurn;
            float fSpeed = mSetSpeed;
            float fBlend;
            if (GetFlyerMinSpeed() >= fSpeed)
            {
                fBlend = -1.0f;
            }
            else if (GetFlyerMidSpeed() > fSpeed)
            {
                fBlend = (fSpeed - GetFlyerMidSpeed()) / (GetFlyerMidSpeed() - GetFlyerMinSpeed());
            }
            else if (GetFlyerMaxSpeed() > fSpeed)
            {
                fBlend = (fSpeed - GetFlyerMidSpeed()) / (GetFlyerMaxSpeed() - GetFlyerMidSpeed());
            }
            else
            {
                fBlend = 1.0f;
            }
            mZephyrPoseDyn.SetAnimation(mClass->mAnimFins, 30.0f);
            mZephyrPoseDyn.SetLoop(0);
            mZephyrPoseDyn.SetFrameNumFloat(fTurn + 4.0f);
            ZephyrPoseStatic<32> kPose;
            mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
            kPose.Open(mZephyrSkeleton);
            kPose.Set(mZephyrPoseDyn, true);
            mZephyrPoseDyn.SetFrameNumFloat(fBlend > 0.0f ? fTurn + 1.0f : fTurn + 7.0f);
            kPose.Blend(mZephyrPoseDyn, (float)fabs((double)fBlend));
            mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
            mPose.ConvertFromZephyrPose(mZephyrSkeleton);
        }
    }

    PblMatrix mWorld = GetSmoothedMatrix();
    if (!netFrameLock && NetGame::IsEnabled())
    {
        gameObjectInterpolator.GetMatrix(mNetUniqueId, mWorld);
    }
    const PblMatrix mInverse = mWorld.GetInverseFast();

    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mAimer[i])
        {
            mAimer[i]->SetupPose(mInverse, mPose, true);
        }
    }

    PblMatrix aTurretSave[8];
    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->Render(mWorld, mInverse, mPose, flags, aTurretSave[i]);
    }

    SetupShadow(this, mModel, renderMode, flags, 0);
    mModel->Render(renderMode, lod, &mWorld, &mPose, RedColor::WHITE, flags, 0);
    PblMatrix mReflection;
    if (!(flags & 0x200000) && FLRenderer::IsReflected(GetRenderPos(), _Sphere.w, mReflection, false))
    {
        PblMatrix mat;
        D3DXMatrixMultiply(&mat, &mWorld, &mReflection);
        mReflection = mat;
        mModel->Render(renderMode, lod, &mReflection, &mPose, RedColor::WHITE,
                       (flags & ~0x10000) | 0x10000100, 0);
    }

    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->PostRender(aTurretSave[i]);
    }

    FlagItem* pFlag = GetGameObject()->GetCarriedFlagItem();
    if (pFlag)
    {
        const FlagItemClass* pFlagClass = static_cast<const FlagItemClass*>(pFlag->mClass);
        PblVector3 vPos;
        D3DXVec3TransformCoord(&vPos, &pFlagClass->mCarriedOffset, &mWorld);
        mWorld.m[3][0] = vPos.x;
        mWorld.m[3][1] = vPos.y;
        mWorld.m[3][2] = vPos.z;
        mWorld.m[3][3] = 1.0f;
        reinterpret_cast<GameObject*>(pFlag)->SetMatrix(mWorld);
        reinterpret_cast<GameObject*>(pFlag)->SetVelocity(GetVelocity());
        pFlag->RenderHelper(renderMode, lod, flags);
    }
}

// FUNCTION: SWBF2 0x004ab930
void EntityFlyer::PlayDustEffect()
{
    PblVector3 vStart = mMatrix.GetTranslation();
    vStart.y += 1.0f;
    const PblVector3 vDown(0.0f, -1.0f, 0.0f);
    CollisionObject* pHit = 0;
    const GameObject* pIgnore = this;
    float fDist = CollisionManager::RayHit(vStart, vDown, 1024.0f, pHit, 0, &pIgnore, 1, 0x90, false) * 1024.0f;
    if (!pHit)
    {
        return;
    }
    if (pHit->GetCollisionType() != COLL_TERRAIN && RedTerrain::_bEnable)
    {
        return;
    }
    FLEffectClass* pEffectClass = FLEffect::Get(0xd63ee91a);
    if (!pEffectClass)
    {
        return;
    }
    FLEffectObject* pEffect = pEffectClass->BuildEffect();
    if (!pEffect)
    {
        return;
    }
    PblVector3 vHit(vStart.x - 0.0f, vStart.y - fDist, vStart.z - 0.0f);
    pEffect->AttachEffectToVector(&vHit, false);
}

// FUNCTION: SWBF2 0x004aba50
void EntityFlyer::TurnOnThrustEffects()
{
    for (int i = 0; i < 8; ++i)
    {
        if (!mClass->mThrustEffectName[i])
        {
            return;
        }
        if (mThrustEffect[i])
        {
            continue;
        }
        FLEffectClass* pEffectClass = FLEffect::Get(mClass->mThrustEffectName[i]);
        if (!pEffectClass)
        {
            continue;
        }
        mThrustEffect[i] = static_cast<ParticleEmitterObject*>(pEffectClass->BuildEffect());
        if (!mThrustEffect[i])
        {
            continue;
        }
        mThrustEffect[i]->ScaleSet(mClass->mThrustEffectMinScale[i]);
        PblMatrix mOffset = _IdentityMatrix;
        mOffset.m[3][0] = mClass->mThrustEffectAttachOffset[i].x;
        mOffset.m[3][1] = mClass->mThrustEffectAttachOffset[i].y;
        mOffset.m[3][2] = mClass->mThrustEffectAttachOffset[i].z;
        mOffset.m[3][3] = 1.0f;
        if (!mClass->mThrustEffectHardPoint[i])
        {
            mThrustEffect[i]->AttachEffectToObject(this, mOffset, true);
        }
        else
        {
            mThrustEffect[i]->AttachEffectToObject(this, mClass->mThrustEffectHardPoint[i], mOffset, true);
        }
    }
}

// FUNCTION: SWBF2 0x004abc70
void EntityFlyer::RecalculateSpeed()
{
    if (mFollowingPath && mState == FLYING && IsAlive() && mAIEng.ai.mFreeFlyTime <= 0.0f && !mInRoll && !mInFlip)
    {
        float fMax = mClass->mBoostFlyerSpeed;
        if (fMax == 0.0f)
        {
            fMax = GetFlyerMaxSpeed();
        }
        float fSpeed = GetFlyerMidSpeed() * mCurSpeed;
        float fMin = GetFlyerMinSpeed();
        if (fMin > fSpeed)
        {
            mGetSpeedSpeed = fMin;
        }
        else if (fSpeed > fMax)
        {
            mGetSpeedSpeed = fMax;
        }
        else
        {
            mGetSpeedSpeed = fSpeed;
        }
    }
    else
    {
        const PblVector3& vNose = mMatrix.GetZAxis();
        mGetSpeedSpeed = vNose.x * mVelocity.x + mVelocity.y * vNose.y + mVelocity.z * vNose.z;
    }
}

// FUNCTION: SWBF2 0x004abed0
void EntityFlyer::TurnOffContrailEffects()
{
    for (int i = 0; i < 12; ++i)
    {
        if (!mClass->mContrailEffectName[i])
        {
            return;
        }
        if (mContrailEffect[i])
        {
            mContrailEffect[i]->DeactivateEffect();
            mContrailEffect[i] = 0;
        }
    }
}

// FUNCTION: SWBF2 0x004abd40
bool EntityFlyer::TurnOnContrailEffect(int iIndex)
{
    if (!bRenderContrails)
    {
        return false;
    }
    if (mContrailEffect[iIndex])
    {
        return true;
    }

    FLEffectClass* pEffectClass = FLEffect::Get(mClass->mContrailEffectName[iIndex]);
    if (!pEffectClass)
    {
        return false;
    }
    mContrailEffect[iIndex] = static_cast<ParticleEmitterObject*>(pEffectClass->BuildEffect());
    if (!mContrailEffect[iIndex])
    {
        return false;
    }

    mContrailEffect[iIndex]->ScaleSet(mClass->mContrailEffectMinScale[iIndex]);
    PblMatrix mOffset = _IdentityMatrix;
    mOffset.m[3][0] = mClass->mContrailEffectAttachOffset[iIndex].x;
    mOffset.m[3][1] = mClass->mContrailEffectAttachOffset[iIndex].y;
    mOffset.m[3][2] = mClass->mContrailEffectAttachOffset[iIndex].z;
    mOffset.m[3][3] = 1.0f;
    if (!mClass->mContrailEffectHardPoint[iIndex])
    {
        mContrailEffect[iIndex]->AttachEffectToObject(this, mOffset, true);
        return false;
    }
    mContrailEffect[iIndex]->AttachEffectToObject(this, mClass->mContrailEffectHardPoint[iIndex], mOffset, true);
    return true;
}

// FUNCTION: SWBF2 0x004abf20
void EntityFlyer::UpdateContrailEffects()
{
    if (!bRenderContrails)
    {
        return;
    }

    float fSpeed = mGetSpeedSpeed;
    if (mState == FLYING)
    {
        float fBoost = mClass->mBoostFlyerSpeed;
        float fMax = mClass->mMaxFlyerSpeed;
        float fStart = (fBoost - fMax) * 0.1f + fMax;
        if (fStart >= fBoost)
        {
            fBoost = fStart * 1.5f;
        }
        if (!(fStart > fSpeed))
        {
            float fRatio;
            if (fBoost > fStart)
            {
                fRatio = (fSpeed - fStart) / (fBoost - fStart);
                if (0.0f > fRatio)
                {
                    fRatio = 0.0f;
                }
                else if (fRatio > 1.0f)
                {
                    fRatio = 1.0f;
                }
            }
            else
            {
                fRatio = 0.0f;
            }
            mTurbulance = fRatio;

            for (int i = 0; i < 12; ++i)
            {
                if (!mClass->mContrailEffectName[i])
                {
                    return;
                }
                if (0.5f > mClass->mContrailEffectMinSpeed[i])
                {
                    if (mContrailEffect[i])
                    {
                        mContrailEffect[i]->DeactivateEffect();
                        mContrailEffect[i] = 0;
                    }
                }
                else if (TurnOnContrailEffect(i))
                {
                    mContrailEffect[i]->ScaleSet(
                        (mClass->mContrailEffectMaxScale[i] - mClass->mContrailEffectMinScale[i]) * fRatio +
                        mClass->mContrailEffectMinScale[i]);
                }
            }
            return;
        }
    }
    mTurbulance = 0.0f;
    TurnOffContrailEffects();
}

// FUNCTION: SWBF2 0x004ac150
void EntityFlyer::UpdatePathFollower(float fDeltaTime, PblMatrix& rMatrix)
{

    if (mProcedurallyAnimated)
    {
        return;
    }

    BranchParams params = GetBranchParams();
    params.mFallbackToClosest = true;
    float fSpeedLimit = 0.0f;
    if (mClass->mBoostFlyerSpeed > 0.0f)
    {
        fSpeedLimit = GetFlyerMaxSpeed() / GetFlyerMidSpeed();
    }
    float fSpeedHold = 0.0f;
    if (!mEnergyBar.IsOverheating() && mEnergyBar.GetEnergyMax() > 0.0f)
    {
        fSpeedHold = mEnergyBar.GetEnergy() / mEnergyBar.GetEnergyMax();
    }

    if (EntityPathFollower::Update(params, fDeltaTime, mVelocity, &rMatrix, false, fSpeedLimit, fSpeedHold))
    {
        SetMatrix(mMatrix);
        if (!InSquadron())
        {
            mAIEng.ai.mSquadronFormationTime -= fDeltaTime;
            if (ResetSquadronTime())
            {
                float fSquareDist = mClass->mSquadronSquaredDist;
                if (fSquareDist == 0.0f)
                {
                    float fDist = mClass->GetModel()->_mModel->_BoundingSphere.GetRadius() * 30.0f;
                    fSquareDist = fDist * fDist;
                }
                if (!CreateSquadronForFlyby(params))
                {
                    PblVector3 vPos = mMatrix.GetTranslation();
                    PblVector3 vDir = mMatrix.GetZAxis();
                    CreateSquadronFromClosestEntities(vPos, vDir, fSquareDist, mClass->mSquadronMaxAngle);
                }
            }
        }
    }
    else
    {
        mAIEng.ai.mFreeFlyTime = 2.0f;
        if (!mInRoll && !mInFlip && mAIEng.ai.mTimeToNextTrick <= 0.0f)
        {
            mAIEng.ai.mTrickTime = PblRandom::GetGlobalFloatRange(2.25f, 1.5f);
            mAIEng.ai.mTimeToNextTrick = 5.0f;
        }
        EntityPathFollower::Reset(params.mPathClass);
        ResetSquadronTime();
    }

    if (mbLandNow)
    {
        mAIEng.ai.mFreeFlyTime = 0.5f;
        Land();
    }
}

// FUNCTION: SWBF2 0x004b10d0
void EntityFlyer::SideRollStop()
{
    mSideRoll.x = 0.0f;
    mSideRoll.y = 0.0f;
    mSideRoll.z = 0.0f;
    RollStop();
}

// FUNCTION: SWBF2 0x004b1100
void EntityFlyer::SideRollUpdate(float fDeltaTime)
{
    if (mState != FLYING && mState != CRASHING && mState != CRASHED)
    {
        SideRollStop();
        return;
    }
    if ((mSideRoll.x != 0.0f || mSideRoll.y != 0.0f) && mRoll == mRollTarget &&
        0.8f > (float)fabs((double)mBankRollAngleDelta))
    {
        mSideRoll.x = 0.0f;
        mSideRoll.y = 0.0f;
        mSideRoll.z = 0.0f;
    }
}

// FUNCTION: SWBF2 0x004b11d0
void EntityFlyer::RollUpdate(float fDeltaTime)
{
    const float fTwoPi = 6.2831855f;
    if (mState == CRASHING)
    {
        if (mRollTarget > 0.0f && fTwoPi * 2.0f > mRollTarget)
        {
            mRollTarget = fTwoPi * 2.0f;
        }
        else if (0.0f > mRollTarget && mRollTarget > fTwoPi * -2.0f)
        {
            mRollTarget = fTwoPi * -2.0f;
        }
        else if (mRollTarget == 0.0f)
        {
            mRollTarget = ((PblRandom::GetSim().GetInt() & 1) ? 1.0f : -1.0f) * fTwoPi * 2.0f;
        }
    }
    else if (mState != FLYING)
    {
        RollStop();
        return;
    }

    float fRollSpeed = mClass->mTrickRollSpeed;
    if (mRoll != mRollTarget)
    {
        mInRoll = true;
        if (mRollTarget >= mRoll)
        {
            mRoll += fRollSpeed * fDeltaTime;
            if (mRoll > 3.1415927f)
            {
                mRoll -= fTwoPi;
                mRollTarget -= fTwoPi;
                mBankRollAngle -= fTwoPi;
            }
            if (mRoll < mRollTarget)
            {
                return;
            }
        }
        else
        {
            mRoll -= fRollSpeed * fDeltaTime;
            if (-3.1415927f > mRoll)
            {
                mRoll += fTwoPi;
                mRollTarget += fTwoPi;
                mBankRollAngle += fTwoPi;
            }
            if (mRollTarget < mRoll)
            {
                return;
            }
        }
        if (0.001f > (float)fabs((double)mRollTarget))
        {
            RollStop();
            return;
        }
        mRoll = mRollTarget;
        return;
    }

    if (!(0.25f > (float)fabs((double)mBankRollAngleDelta)))
    {
        return;
    }
    float fAbsTarget = (float)fabs((double)mRollTarget);
    if (0.001f > (float)fabs((double)(fAbsTarget - 3.1415927f)))
    {
        float fSign = 0.0f > mBankRollAngleDelta ? -1.0f : 1.0f;
        if (mGetSpeedSpeed > GetFlyerMinSpeed() * 0.9f)
        {
            RollAdd(fSign * 10.210176f);
        }
        else
        {
            RollAdd(fSign * 3.926991f);
        }
        return;
    }
    if (1.5707964f >= fAbsTarget)
    {
        mRollTarget = 0.0f;
    }
    else
    {
        mInRoll = false;
    }
}

// FUNCTION: SWBF2 0x004b14b0
void EntityFlyer::RollStop()
{
    mRoll = 0.0f;
    mRollTarget = 0.0f;
    while (mBankRollAngle > 3.1415927f)
    {
        mBankRollAngle -= 6.2831855f;
    }
    while (-3.1415927f > mBankRollAngle)
    {
        mBankRollAngle += 6.2831855f;
    }
    mInRoll = false;
}

extern "C" PblMatrix* __stdcall D3DXMatrixRotationX(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationZ(PblMatrix* pOut, float fAngle);
extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);

extern bool netIsLocalTurn;
extern bool netIsLateTurn;
extern PblVector3 gVectorZero;

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

// FUNCTION: SWBF2 0x004b1550
void EntityFlyer::FlipUpdate(float fDeltaTime, PblMatrix& rMatrix)
{
    if (mState != FLYING)
    {
        FlipStop();
        return;
    }

    float fFlipSpeed = mClass->mTrickFlipSpeed;
    if (mFlip != mFlipTarget)
    {
        mInFlip = true;
        if (mFlipTarget >= mFlip)
        {
            mFlip += fFlipSpeed * fDeltaTime;
            if (mFlip > 3.1415927f)
            {
                mFlip -= 6.2831855f;
                mFlipTarget -= 6.2831855f;
                mBankPitchAngle -= 6.2831855f;
            }
            if (mFlip < mFlipTarget)
            {
                return;
            }
        }
        else
        {
            mFlip -= fFlipSpeed * fDeltaTime;
            if (-3.1415927f > mFlip)
            {
                mFlip += 6.2831855f;
                mFlipTarget += 6.2831855f;
                mBankPitchAngle += 6.2831855f;
            }
            if (mFlipTarget < mFlip)
            {
                return;
            }
        }
        if (0.001f > (float)fabs((double)mFlipTarget))
        {
            FlipStop();
            return;
        }
        mFlip = mFlipTarget;
        return;
    }

    if (mFlip == -3.1415927f || mFlip == 3.1415927f)
    {
        mInFlip = true;
        rMatrix.m[0][0] = -rMatrix.m[0][0];
        rMatrix.m[0][1] = -rMatrix.m[0][1];
        rMatrix.m[0][2] = -rMatrix.m[0][2];
        rMatrix.m[0][3] = 0.0f;
        rMatrix.m[2][0] = -rMatrix.m[2][0];
        rMatrix.m[2][1] = -rMatrix.m[2][1];
        rMatrix.m[2][2] = -rMatrix.m[2][2];
        rMatrix.m[2][3] = 0.0f;
        mFlip = 0.0f;
        mFlipTarget = 0.0f;
        mBankPitchAngle += 3.1415927f;
        while (mBankPitchAngle > 3.1415927f)
        {
            mBankPitchAngle -= 6.2831855f;
        }
        if (0.0f > mRollTarget)
        {
            mBankRollAngle = -(mBankRollAngle + 3.1415927f);
            mRollTarget = -(mRollTarget + 3.1415927f);
            mRoll = -(mRoll + 3.1415927f);
        }
        else
        {
            mBankRollAngle = -(mBankRollAngle - 3.1415927f);
            mRollTarget = -(mRollTarget - 3.1415927f);
            mRoll = -(mRoll - 3.1415927f);
        }
    }
    else if (mInFlip && 0.01f > (float)fabs((double)mBankPitchAngleDelta))
    {
        mInFlip = false;
    }
}

// FUNCTION: SWBF2 0x004b1870
void EntityFlyer::FlipStop()
{
    mFlip = 0.0f;
    mFlipTarget = 0.0f;
    while (mBankPitchAngle > 3.1415927f)
    {
        mBankPitchAngle -= 6.2831855f;
    }
    while (-3.1415927f > mBankPitchAngle)
    {
        mBankPitchAngle += 6.2831855f;
    }
    mInFlip = false;
}

// FUNCTION: SWBF2 0x004b1d80
int EntityFlyer::GetFirstPersonIndex()
{
    if (mPlayerId < 0)
    {
        return -1;
    }
    if (!mTracker || !mTracker->IsFirstPersonView())
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

// FUNCTION: SWBF2 0x004b1e00
int EntityFlyer::GetJoystickIndex()
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

// FUNCTION: SWBF2 0x004b1e70
bool EntityFlyer::PostCollisionUpdate(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }

    PblVector3 vPos = mMatrix.GetTranslation();
    float fWaterHeight;
    if (RedWater::GetWaterHeight(vPos, fWaterHeight) && fWaterHeight > vPos.y)
    {
        vPos.y = fWaterHeight;
        SetPosition(vPos);
        float fApproach = mVelocity.y;
        if (0.0f > fApproach)
        {
            float fUp = mMatrix.m[1][1];
            float fFront = mMatrix.m[2][1];
            float fThreshold = mClass->mCollisionThreshold[1] * fUp + mClass->mCollisionThreshold[0] +
                               mClass->mCollisionThreshold[2] * fFront;
            if (mState != LANDING)
            {
                FoleyFXCollider::CollisionCallback(COLL_WATER, 0, &GetPosition());
                PlayImpactEffects(&mMatrix.GetTranslation(), &mVelocity);
                PblVector3 vVel = mVelocity;
                PblVector3 vEngine = mMatrix.GetTranslation();
                VehicleEngine::Collision(mClass, vEngine, vVel);

                GetControllableList();
            }

            if (-fThreshold > fApproach)
            {
                float fExcess = fThreshold + fApproach;
                float fDamage = (mClass->mCollisionScale[1] * fUp + mClass->mCollisionScale[0] +
                                 mClass->mCollisionScale[2] * fFront) *
                                fExcess * fExcess;
                if (fDamage > 0.0f)
                {
                    Damageable::ApplyDamage(DamageDesc(DamageOwner(0, 0, 0, 0, 0, 1.0f), fDamage), 0xffffffff);
                }
            }

            if (!IsAlive() || mState == CRASHING)
            {
                mState = CRASHED;
                GetControllableList();
                InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), false);
            }
            else
            {
                mVelocity.x -= gVectorUnitY.x * fApproach;
                mVelocity.y -= gVectorUnitY.y * fApproach;
                mVelocity.z -= gVectorUnitY.z * fApproach;
                float fBounce = fApproach * -1.125f;
                float fDrag = fBounce * 0.25f /
                              sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z);
                if (!(1.0f > fDrag))
                {
                    fDrag = 1.0f;
                }
                mVelocity.x -= mVelocity.x * fDrag;
                mVelocity.y -= mVelocity.y * fDrag;
                mVelocity.z -= mVelocity.z * fDrag;
                float fRise = fBounce + fApproach;
                mVelocity.x += fRise * gVectorUnitY.x;
                mVelocity.y += fRise * gVectorUnitY.y;
                mVelocity.z += fRise * gVectorUnitY.z;

                if (mState != LANDING)
                {
                    float fSplash =
                        sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z) /
                            40.0f +
                        0.01f;
                    if (!(1.0f > fSplash))
                    {
                        fSplash = 1.0f;
                    }
                    if (fSplash > 0.0f)
                    {
                        float fBlend = fSplash * 0.3f;
                        const PblVector3& vNose = mMatrix.GetZAxis();
                        PblVector3 vDir(vNose.x * (1.0f - fBlend) + fBlend * gVectorUnitY.x,
                                        vNose.y * (1.0f - fBlend) + fBlend * gVectorUnitY.y,
                                        vNose.z * (1.0f - fBlend) + fBlend * gVectorUnitY.z);
                        if (mVelocity.x != 0.0f || mVelocity.y != 0.0f || mVelocity.z != 0.0f)
                        {
                            PblVector3 vHeading;
                            D3DXVec3Normalize(&vHeading, &mVelocity);
                            vDir.x += vHeading.x * 0.3f;
                            vDir.y += vHeading.y * 0.3f;
                            vDir.z += vHeading.z * 0.3f;
                        }
                        mMatrix = DirectionMatrix(vDir, mMatrix.GetYAxis(), mMatrix.GetTranslation());
                        mSetSpeed = mVelocity.y * mMatrix.m[2][1] + mVelocity.x * mMatrix.m[2][0] +
                                    mVelocity.z * mMatrix.m[2][2];
                        mSetStrafe = mVelocity.y * mMatrix.m[0][1] + mVelocity.x * mMatrix.m[0][0] +
                                     mVelocity.z * mMatrix.m[0][2];
                        mSetStrafeVertical = mVelocity.y * mMatrix.m[1][1] + mMatrix.m[1][0] * mVelocity.x +
                                             mVelocity.z * mMatrix.m[1][2];

                        CameraManager* pManager = CameraManager::sInstance;
                        if (pManager->GetChaseCameraTarget(0) &&
                            pManager->GetChaseCameraTarget(0)->GetGameObject() == this)
                        {
                            CameraManager::sInstance->ApplyShake(fSplash * 0.8f, fSplash * 0.7f);
                            if (fSplash > 0.5f)
                            {
                                CameraManager::sInstance->mChaseCamera[0]->mTimer = 0.1f;
                            }
                        }
                    }
                }
            }
        }
        if (mState == LANDING)
        {
            mState = TAKEOFF;
        }
    }

    PblMatrix mUnbank;
    PblMatrix mUnpitch;
    D3DXMatrixRotationZ(&mUnbank, -mBankRollAngle);
    D3DXMatrixMultiply(&mUnbank, &mUnbank, D3DXMatrixRotationX(&mUnpitch, -mBankPitchAngle));
    PblMatrix mBody;
    D3DXMatrixMultiply(&mBody, &mUnbank, &mMatrix);
    mEyeDir = mBody.GetZAxis();
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
            CollisionManager::RayHit(mTargetInfo.mAimStart, mEyeDir, 1024.0f, pHit, 0, &pIgnore, 1, 0x9a, true) *
            1024.0f;
        mTargetInfo.mAimPoint = PblVector3(mTargetInfo.mAimStart.x + mEyeDir.x * fDist,
                                           mTargetInfo.mAimStart.y + mEyeDir.y * fDist,
                                           mTargetInfo.mAimStart.z + mEyeDir.z * fDist);
    }
    else
    {
        mTargetInfo.mAimStart = mEyePoint;
        int iIndex = mWeaponIndex[0];
        if (iIndex >= 0 && mWeapon[iIndex] && mWeapon[iIndex]->mTarget)
        {

            const GameObject* pTarget = mWeapon[iIndex]->mTarget;
            PblVector3 vTarget = pTarget->GetTargetPoint(mEyePoint, mEyeDir, -1);
            float fBubble = 0.0f;
            if (pTarget->GetControllable() && pTarget->GetControllable()->mCtrl)
            {
                fBubble = pTarget->GetControllable()->mCtrl->mTargetBubbleSize_Cached;
            }
            float fLead = pTarget->IsRtti(EntityFlyer::rttiHashEntityFlyer) ? fBubble * 0.6f : 0.0f;
            PblVector3 vOffset = mCtrl ? mCtrl->mTargetBubbleOffset : gVectorZero;
            mTargetInfo.mAimPoint = PblVector3(vTarget.x + fLead * vOffset.x, vTarget.y + vOffset.y * fLead,
                                               vTarget.z + vOffset.z * fLead);
        }
        else
        {
            mTargetInfo.mAimPoint = PblVector3(mEyeDir.x * 1024.0f + mEyePoint.x, mEyePoint.y + mEyeDir.y * 1024.0f,
                                               mEyePoint.z + mEyeDir.z * 1024.0f);
        }
    }

    if (!mClass->IsGuidedMissile())
    {
        mClass->UpdatePilotAnimation(mPilot, &mPose, mMatrix, mVelocity, fDeltaTime, -1.0f, -1.0f, -1.0f);
    }

    if (Smooth(fDeltaTime))
    {
        const PblVector3& vCenter = GetCollisionCenter();
        const PblVector3& vPosition = GetPosition();
        const PblVector3& vSmoothed = GetSmoothedMatrix().GetTranslation();
        SetRenderPos(PblVector3(vSmoothed.x + vCenter.x - vPosition.x, vSmoothed.y + vCenter.y - vPosition.y,
                                vSmoothed.z + vCenter.z - vPosition.z));
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

// FUNCTION: SWBF2 0x004b2d40
bool EntityFlyer::ApplyDamage(const DamageDesc& damage, const PblVector3& vDir, const PblVector3& vPos,
                              int iBodyPart, unsigned int uFlags)
{
    if (NetGame::IsPreGame())
    {
        return true;
    }
    if (mPlayerId < 0)
    {

        if (!mbQueuePoint && !mbGoMinSpeed && mProcedurallyAnimated == 0 &&
            mAIEng.ai.mFreeFlyTime <= 0.0001f && PblRandom::GetGlobalFloat() < AIDifficulty::GetFlyerFreeFlyOnDamage() &&
            mClass->mbFlyerVsFlyer)
        {
            if (GetFollowingPath())
            {
                EntityPathFollower::Reset(mClass);
            }
            mAIEng.ai.mFreeFlyTime = 2.0f;
            if (!mInRoll && !mInFlip && mAIEng.ai.mTimeToNextTrick <= 0.0f)
            {
                mAIEng.ai.mTrickTime = PblRandom::GetGlobalFloatRange(2.25f, AIDifficulty::GetFlyerTrickOnDamage());
                mAIEng.ai.mTimeToNextTrick = 10.0f;
            }
        }
    }
    if (mPlayerId >= 0 && (mInRoll || mInFlip))
    {
        return true;
    }
    return Damageable::ApplyDamage(damage, vDir, vPos, iBodyPart, uFlags);
}

// FUNCTION: SWBF2 0x004b3c60
void EntityFlyer::TakeOff()
{
    if (mState == LANDED)
    {
        PlayDustEffect();
    }
    if (mState == LANDED || mState == LANDING)
    {
        if (mFollowingPath)
        {
            EntityPathFollower::Reset(mClass);
            float fMid = GetFlyerMidSpeed();
            mCurSpeed = fMid != 0.0f ? GetFlyerMinSpeed() / fMid : 0.0f;
        }
        TurnOnThrustEffects();
        mState = TAKEOFF;
        if (mProcedurallyAnimated == 0)
        {
            mAIEng.ai.mFreeFlyTime = 0.0f;
            mAIEng.ai.mTrickTime = 0.0f;
        }
        CollisionManager::RemoveBody(this);
        CollisionManager::AddSoftBody(this);
    }
    const PblVector3& vPosition = GetPosition();
    if (!IsWindingUp() || IsWindingDown())
    {
        WindUp(mClass, vPosition, mVelocity);
    }
}

// FUNCTION: SWBF2 0x004b6100
const EntitySquadronClass* EntityFlyer::GetEntitySquadronClass() const
{
    return mClass;
}

// FUNCTION: SWBF2 0x004b6120
EntityPathFollower::BranchParams EntityFlyer::GetBranchParams()
{
    UnitController* pController = UnitControllerPtr(mCtrl);
    float fBoost = mClass->mBoostFlyerSpeed;
    if (!(fBoost > 0.0f))
    {
        fBoost = GetFlyerMaxSpeed();
    }
    float fLow;
    float fHigh;
    AIDifficulty::GetFlyerSpeedRange(pController, GetFlyerMinSpeed(), GetFlyerMidSpeed(), GetFlyerMaxSpeed(), fBoost, fLow,
                             fHigh);
    float fBase = (mCharacter && mCharacter->mCarriedFlagPtr) ? (fHigh - fLow) * 0.5f + fLow : fLow;

    float fTime = GameLoop::GetMissionTime() + (float)(((unsigned int)this >> 2) & 0xf);
    float fWave = PblMath::Cos(fTime * 2.33f) * PblMath::Sin(fTime * 1.17f);
    float fBeat = PblMath::Sin(fTime * 0.43f) * fWave * 0.5f + 0.5f;
    float fSpeed = fBeat * fBeat * (fHigh - fBase) + fBase;

    return BranchParams(mClass, mMatrix, mClass->GetModel()->_mModel->_BoundingSphere.GetRadius(), this, fSpeed,
                        FLT_MAX, 0, -1.0f, false, false);
}

extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

namespace SquadCommands { void VehicleCommand(GameObject* pObject, Character* pCharacter); }

const PblVector3 __cdecl Normalize(const PblVector3& rv);

namespace PblMath
{
    inline float Length2Est(float fA, float fB)
    {
        const float fLength2EstAlpha = 0.94805944f;
        const float fLength2EstBeta = 0.39269909f;
        fA = (float)fabs((double)fA);
        fB = (float)fabs((double)fB);
        return fA > fB ? fA * fLength2EstAlpha + fB * fLength2EstBeta
                       : fA * fLength2EstBeta + fB * fLength2EstAlpha;
    }

    inline float Length3Est(float fX, float fY, float fZ)
    {
        return Length2Est(Length2Est(fX, fY), fZ);
    }
}

inline const float LengthEst(const PblVector3& rv)
{
    return PblMath::Length3Est(rv.x, rv.y, rv.z);
}

// FUNCTION: SWBF2 0x004ac460
bool EntityFlyer::Update(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }

    Controllable::Update(fDeltaTime);

    if (mInLandingRegionFlag)
    {
        mInLandingRegionFlag = false;
    }
    else
    {
        mInLandingRegionFactor = 0.0f;
    }
    mFlyerAutoCorrectingToStayInBattlefield = false;

    if (NetGame::IsEnabled() && NetGame::JustEndedPreGame() && !NetGame::IsClient())
    {
        Kill();
        return false;
    }

    mCollisionModel = mState == FLYING ? &mClass->mFlyingCollisionModel : &mClass->mCollisionModel;

    if (mClass->mGuidedMissile && (!mPilot || mEnergyBar.GetEnergy() <= 0.0f || mCrashTimer != 0.0f))
    {
        ExplosionClass* pExplosion = mClass->mExplosionDestruct;
        if (mClass->mBoostFlyerSpeed > 0.0f && mPilot && mClass->mExplosionCritical &&
            mGetSpeedSpeed >= mClass->mGuidedMissileBoostExplosionSpeedThreshold * mClass->mBoostFlyerSpeed)
        {
            pExplosion = mClass->mExplosionCritical;
        }

        if (!(NetGame::IsEnabled() && NetGame::IsClient()) && !pExplosion && mCrashTimer == 0.0f)
        {
            Kill();
            return false;
        }
        if (mCrashTimer == 0.0f)
        {
            TurnOffThrustEffects();
            TurnOffContrailEffects();
            DeactivateEngine();
            Deactivate();
            RemoveFromCollisionManager();
            mCrashTimer += fDeltaTime;
            if (pExplosion)
            {
                DamageOwner owner(mCharacter, this, GetTeam(), mClass, 0, 1.0f);
                pExplosion->CreateExplosion(mMatrix.GetTranslation(), owner, true, COLL_UNKNOWN);
            }
            return true;
        }
        mCrashTimer += fDeltaTime;
        if (!(NetGame::IsEnabled() && NetGame::IsClient()) &&
            (mCrashTimer > mClass->mGuidedMissileDeathCamPause || !mPilot))
        {
            Kill();
            return false;
        }
        return true;
    }

    if (netIsLocalTurn)
    {
        if (mJustRendered)
        {
            mTotalUpdateDt = fDeltaTime;
            mJustRendered = false;
        }
        else
        {
            mTotalUpdateDt += fDeltaTime;
        }
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()) && -5000.0f > mMatrix.GetTranslation().y)
    {
        Kill();
        return false;
    }

    if (mPlayerId >= 0)
    {
        NetGame::IsMouseMode(mPlayerId);
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
        pManager->SetAutoLoseLocked(10.0f, true, -1.0f, fLoseDist);
        pManager->SetTarget(0.5f, -1.0f, fLockDist, true);
        pManager->SetAuto(true, false);
        pManager->SetNumAttackerTargets(0);
        pManager->SetNumNearestTargets(64);

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

    RecalculateSpeed();

    if (mState == LANDED)
    {
        if (GetCollisionType() != COLL_RIGID)
        {
            RemoveFromCollisionManager();
            AddRigidToCollisionManager();
        }
    }
    else if (mState == FLYING && GetCollisionType() != COLL_SOFT)
    {
        RemoveFromCollisionManager();
        AddSoftToCollisionManager();
    }

    if (mState == TAKEOFF || mState == FLYING || (mState == CRASHING && mIsAlive))
    {
        int iPilots = 0;
        int iPlayerPilots = 0;
        for (PblList<Controllable>::Itor it(GetControllableList()); it; ++it)
        {
            Controllable* pControllable = it;
            if (pControllable->mPilot)
            {
                ++iPilots;
                if (pControllable->mPilot->mPlayerId >= 0)
                {
                    ++iPlayerPilots;
                }
            }
        }
        bool bNeverCrash = mClass && mClass->mNeverCrashWhenUnpiloted;
        if (mState == CRASHING)
        {
            if (mIsAlive && iPilots)
            {
                mState = TAKEOFF;
            }
        }
        else if (!bNeverCrash && (!iPilots || (!mPilot && !iPlayerPilots)))
        {
            SetCurHealth(0.0f, true, 0);
            mState = CRASHING;
            mCrashTimer = 0.5f;
        }
    }

    if (CameraManager::sInstance->IsChaseMode(iJoystick))
    {
        if (mPlayerId >= 0 && mGetSpeedSpeed + 0.0001f >= mClass->mMinFlyerSpeed)
        {
            SpaceDustEffect::Start(iJoystick);
        }
    }
    else
    {
        SpaceDustEffect::Stop(iJoystick);
        if (iJoystick < 0)
        {
            for (int i = 0; i < 2; ++i)
            {
                int iSpectator = NetGame::GetSpectator(NetGame::GetLocalIndex(i));
                if (iSpectator != -1 && iSpectator == mPlayerId)
                {
                    SpaceDustEffect::Start(i);
                }
            }
        }
    }

    if (!mInRoll && !mInFlip && mClass->mMaxFlyerSpeed >= mGetSpeedSpeed - 0.0001f)
    {
        float fRestore = mClass->mEnergyAutoRestore;
        if (fRestore < 0.0f)
        {
            mEnergyBar.SpendEnergy(-(fRestore * fDeltaTime));
        }
        else
        {
            float fMultiplier = mCharacter && mCharacter->mMedalsMgr &&
                                        mCharacter->mMedalsMgr->IsAwardAvailable(MedalsMgr::ENDURANCE)
                                    ? 1.5f
                                    : 1.0f;
            mEnergyBar.AddEnergy(fRestore * fMultiplier * fDeltaTime);
        }
    }

    float fThrust = mControlMove;
    float fRollInput;
    if (mInLandingRegionFactor == 0.0f)
    {
        fRollInput = mControlZoom.IsDown() ? mControlTurn : mControlStrafe;
    }
    else
    {
        fRollInput = 0.0f;
    }
    float fRoll;
    if (0.3f > (float)fabs((double)fRollInput))
    {
        fRoll = 0.0f;
    }
    else
    {
        fRoll = (fRollInput > 0.0f ? fRollInput - 0.3f : fRollInput + 0.3f) / 0.7f;
        if (-1.0f > fRoll)
        {
            fRoll = -1.0f;
        }
        else if (fRoll > 1.0f)
        {
            fRoll = 1.0f;
        }
    }
    float fPitch = mControlPitch;
    float fTurn = mControlZoom.IsDown() ? 0.0f : mControlTurn;
    float fAbsPitch = (float)fabs((double)fPitch);
    float fAbsTurn = (float)fabs((double)fTurn);

    if (GetFollowingPath())
    {
        mBoost = mClass->mBoostFlyerSpeed > 0.0f && mGetSpeedSpeed > mClass->mMaxFlyerSpeed &&
                 mEnergyBar.SpendEnergy(mClass->mEnergyBoostDrain * fDeltaTime);
    }
    else if (!(mClass->mBoostFlyerSpeed > 0.0f))
    {
        mBoost = false;
    }
    else if (mBoost)
    {
        mBoost = !mControlSprint.Tapped() && !mControlSprint.HeldAndReleased() && fThrust >= 0.2f &&
                 mEnergyBar.SpendEnergy(mClass->mEnergyBoostDrain * fDeltaTime);
    }
    else
    {
        mBoost = (mControlThrustFwd.TappedAndPressed() ||
                  ((mControlSprint.Tapped() || mControlSprint.Held()) &&
                   mGetSpeedSpeed >= mClass->mMinFlyerSpeed * 0.3f)) &&
                 fThrust >= 0.2f && mEnergyBar.SpendEnergy(mClass->mEnergyBoostDrain * fDeltaTime);
    }
    if (mPlayerId >= 0 && mBoost)
    {
        ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_FlyerBoost, 0);
    }

    UpdateContrailEffects();

    if (mInRoll || mInFlip)
    {
        if (mControlCrouch.IsPressed())
        {
            mControlCrouch.ClearUntilPressed();
        }
        if (mControlSpecialMove.IsPressed())
        {
            mControlSpecialMove.ClearUntilPressed();
        }
        mTrickDoubleclick = 0.0f;
        mTrick -= fDeltaTime;
    }
    else if (mClass->mEnergyTrickDrainDoubleTap != 0.0f && mControlSpecialMove.IsPressed())
    {
        mTrick = 1.0f;
        mTrickDoubleclick = 1.0f;
    }
    else if (mControlCrouch.IsPressed() && mClass->mEnergyTrickDrainSingleTap != 0.0f)
    {
        mTrick = 1.0f;
        mTrickDoubleclick = mControlCrouch.TappedAndPressed() && mClass->mEnergyTrickDrainDoubleTap != 0.0f;
    }
    else
    {
        mTrick -= fDeltaTime;
    }
    if (-1.0f > mTrick)
    {
        mTrick = -1.0f;
    }

    float fTopSpeed = mClass->mMaxFlyerSpeed > mClass->mBoostFlyerSpeed ? mClass->mMaxFlyerSpeed
                                                                       : mClass->mBoostFlyerSpeed;
    float fPitchRate = mClass->mPitchRate;
    float fTurnRate = mClass->mTurnRate;
    float fMinRate = fPitchRate <= fTurnRate ? fPitchRate : fTurnRate;
    float fTurnRateNow = fMinRate;
    if (fAbsTurn >= 0.7f)
    {
        float fT = (fAbsTurn - 0.7f) / 0.3f;
        if (fT >= 0.0f)
        {
            if (fT > 1.0f)
            {
                fT = 1.0f;
            }
        }
        else
        {
            fT = 0.0f;
        }
        fTurnRateNow = (1.0f - fT) * fMinRate + fTurnRate * fT;
    }
    float fPitchRateNow = fMinRate;
    if (fAbsPitch >= 0.7f)
    {
        float fT = (fAbsPitch - 0.7f) / 0.3f;
        if (fT >= 0.0f)
        {
            if (fT > 1.0f)
            {
                fT = 1.0f;
            }
        }
        else
        {
            fT = 0.0f;
        }
        fPitchRateNow = (1.0f - fT) * fMinRate + fPitchRate * fT;
    }

    float fPitchScale;
    float fTurnScale;
    if (mPlayerId >= 0 && !mInRoll && !mInFlip &&
        (mClass->mBoostFlyerSpeed == 0.0f || GetFlyerMaxSpeed() >= mGetSpeedSpeed - 0.0001f))
    {
        fPitchScale = mPitchBuildup > 0.5f
                          ? (mClass->mPitchBuildupMultiplier - 1.0f) * (mPitchBuildup - 0.5f) * 2.0f + 1.0f
                          : 1.0f;
        fTurnScale = mTurnBuildup > 0.5f ? (mClass->mTurnBuildupMultiplier - 1.0f) * (mTurnBuildup - 0.5f) * 2.0f + 1.0f
                                         : 1.0f;
    }
    else if (mClass->mBoostFlyerSpeed == 0.0f || mGetSpeedSpeed <= GetFlyerMaxSpeed())
    {
        fPitchScale = 1.0f;
        mPitchBuildup = 0.0f;
        mTurnBuildup = 0.0f;
        fTurnScale = 1.0f;
    }
    else
    {
        float fMinScale = IsRtti(CommandFlyer::rttiHashCommandFlyer) ? 0.5f : 0.7f;
        float fT = (mGetSpeedSpeed - GetFlyerMaxSpeed()) / (mClass->mBoostFlyerSpeed - GetFlyerMaxSpeed());
        if (fT >= 0.0f)
        {
            if (fT > 1.0f)
            {
                fT = 1.0f;
            }
        }
        else
        {
            fT = 0.0f;
        }
        mPitchBuildup = 0.0f;
        mTurnBuildup = 0.0f;
        fPitchScale = 1.0f - (1.0f - fMinScale) * fT;
        fTurnScale = fPitchScale;
    }

    if (!mProcedurallyAnimated)
    {
        if (mAIEng.ai.mTimeSinceLastHack > -0.0001f)
        {
            mAIEng.ai.mTimeSinceLastHack += fDeltaTime;
        }
        if (mAIEng.ai.mTimeSinceLastHack > 1.0f)
        {
            mAIEng.ai.mEjectTimer -= (0.04f - mClass->mEjectResist) * 50.0f * fDeltaTime;
        }
        if (0.0001f > mAIEng.ai.mEjectTimer)
        {
            mAIEng.ai.mTimeSinceLastHack = -1.0f;
        }
    }

    Controllable* pControllable = GetControllable();
    if (!mProcedurallyAnimated)
    {
        if (mAIEng.ai.mReactionThinkTime > 0.0f)
        {
            mAIEng.ai.mReactionThinkTime -= fDeltaTime;
        }
        if (mAIEng.ai.mReactionTime > 0.0f)
        {
            mAIEng.ai.mReactionTime -= fDeltaTime;
        }
    }

    if (pControllable && pControllable->mControlFire[0].IsDown())
    {
        const GameObject* pTarget = 0;
        if (pControllable->mAIReticuleTarget[0] &&
            pControllable->mAIReticuleTarget[0]->GetGameObject()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            pTarget = pControllable->mAIReticuleTarget[0];
        }
        else if (pControllable->mReticuleTarget[0] &&
                 pControllable->mReticuleTarget[0]->GetGameObject()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            pTarget = pControllable->mReticuleTarget[0];
        }

        if (pTarget)
        {
            EntityFlyer* pFlyer = (EntityFlyer*)pTarget->GetEntity();
            if (pFlyer && pFlyer->mPlayerId < 0 &&
                !(pFlyer->mPilotType == PILOT_SELF ||
                  (pFlyer->mPilotType == PILOT_VEHICLESELF && pFlyer->mPilot == pFlyer)) &&
                pFlyer->mPilot && pFlyer->mClass->mbFlyerVsFlyer && pFlyer->GetFollowingPath() &&
                !pFlyer->mbQueuePoint && !pFlyer->mbGoMinSpeed && !mProcedurallyAnimated)
            {
                if (0.0001f > mAIEng.ai.mReactionThinkTime)
                {
                    mAIEng.ai.mReactionThinkTime = 2.0f;
                    if (0.0001f > mAIEng.ai.mReactionTime)
                    {
                        mAIEng.ai.mReactionTime = AIDifficulty::GetFlyerReactionTime();
                    }
                }
                if (!(mAIEng.ai.mReactionTime > 0.0001f))
                {
                    pFlyer->TargetReact(pControllable);
                    mAIEng.ai.mReactionThinkTime = 2.0f;
                }
            }
        }
    }
    if (!mProcedurallyAnimated && mAIEng.ai.mTimeToNextTrick > 0.0f)
    {
        mAIEng.ai.mTimeToNextTrick -= fDeltaTime;
    }

    PblMatrix matrix;
    if (GetFollowingPath())
    {
        UpdatePathFollower(fDeltaTime, matrix);
    }
    if (!GetFollowingPath())
    {
        EntityPathFollower::Reset(mClass);
        PblMatrix mUnbank;
        PblMatrix mUnpitch;
        D3DXMatrixRotationZ(&mUnbank, -mBankRollAngle);
        D3DXMatrixMultiply(&mUnbank, &mUnbank, D3DXMatrixRotationX(&mUnpitch, -mBankPitchAngle));
        D3DXMatrixMultiply(&matrix, &mUnbank, &mMatrix);
        RollUpdate(fDeltaTime);
        FlipUpdate(fDeltaTime, matrix);
        SideRollUpdate(fDeltaTime);

        if (mInFlip)
        {
            fPitch = 0.0f;
            fAbsPitch = 0.0f;
            fTurn = 0.0f;
            fAbsTurn = 0.0f;
            fRoll = 0.0f;
        }
        else if (mSideRoll.x == 0.0f && mSideRoll.y == 0.0f)
        {
            if (fAbsTurn > 0.3f && mAutoLevelingAllowed == 0.0f)
            {
                mAutoLevelingAllowed = 0.01f;
            }
        }
        else
        {
            fRoll = 0.0f;
        }

        if (!mProcedurallyAnimated && mAIEng.ai.mFreeFlyTime > 0.0f)
        {
            if (mAIEng.ai.mTrickTime > 0.0f)
            {
                mAIEng.ai.mTrickTime -= fDeltaTime;
                if (0.0f >= mAIEng.ai.mTrickTime)
                {
                    float fTrick = PblRandom::GetGlobalFloat() * 7.0f;
                    if (0.0f > fTrick)
                    {
                        fTrick = 0.0f;
                    }
                    else if (fTrick > 6.0f)
                    {
                        fTrick = 6.0f;
                    }
                    Trick eTrick = (Trick)(int)fTrick;
                    if (eTrick == SIDE_ROLL)
                    {
                        mSideRoll.x = PblRandom::GetGlobalFloat() - 0.5f;
                        if (mSideRoll.x == 0.0f)
                        {
                            mSideRoll.x = 0.01f;
                        }
                        mSideRoll.y = PblRandom::GetGlobalFloat() - 0.5f;
                        PblVector3 vSideRoll;
                        D3DXVec3Normalize(&vSideRoll, &mSideRoll);
                        mSideRoll = vSideRoll;
                    }
                    DoTrick(eTrick);
                }
            }
            mAIEng.ai.mFreeFlyTime -= fDeltaTime;
        }
    }

    if (!mIsAlive)
    {
        if (mState == CRASHED || mState == LANDED)
        {

            if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mDamageOwner.mCharacter)
            {
                Controllable* pPilot =
                    (mPilotType == PILOT_SELF || (mPilotType == PILOT_VEHICLESELF && mPilot == this)) ? 0 : mPilot;
                AwardVehicleKillPoints(mClass->GetVehicleType(), pPilot == 0);
            }
            if (!(NetGame::IsEnabled() && NetGame::IsClient()))
            {
                CreateChunkCluster(1.0f);
                Kill();
                if (mClass->mExplosionDestruct)
                {
                    DamageOwner owner(mCharacter, this, GetTeam(), mClass, 0, 1.0f);
                    mClass->mExplosionDestruct->CreateExplosion(mMatrix.GetTranslation(), owner, false, COLL_UNKNOWN);
                }
                return false;
            }
        }
        else if (mState != CRASHING)
        {

            if (mClass->mExplosionCritical)
            {
                DamageOwner owner(mCharacter, this, GetTeam(), mClass, 0, 1.0f);
                mClass->mExplosionCritical->CreateExplosion(mMatrix.GetTranslation(), owner, true, COLL_UNKNOWN);
            }
            SetCurHealth(0.0f, true, 0);
            mState = CRASHING;
            if (mClass->GetNumChunks() > 0)
            {
                mCrashTimer = PblRandom::GetSim().GetFloat() * 2.0f + 0.2f;
            }
            else
            {
                mCrashTimer = 600.0f;
            }
        }
    }

    UpdateStatus(fDeltaTime);
    TriggerDisabledEffect();
    if (mState != LANDED && !IsOn())
    {
        TurnOn(mClass, mMatrix.GetTranslation(), mVelocity, mMaxHealth != 0.0f ? mCurHealth / mMaxHealth : 0.0f);
    }

    if (!(mPilotType == PILOT_SELF || (mPilotType == PILOT_VEHICLESELF && mPilot == this)) && mPilot)
    {
        Controllable* pPilot = mPilot;
        if (mControlView.IsPressed() && netIsLocalTurn && !IsForcedThirdPerson() && !IsForcedFirstPerson() &&
            mClass->mForceMode != 1 && mClass->mForceMode != 2 && mTracker)
        {
            SwitchControlView(mPlayerId);
        }
        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            if ((mState == LANDED || mClass->mGuidedMissile) && (mControlUse.IsPressed() || IsDisabled()))
            {
                SpaceDustEffect::Stop(GetJoystickIndex());
                if (mPilotType == PILOT_REMOTE || mClass->mGuidedMissile)
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
    }

    if (mState == LANDED && mControlSquadCommand.IsPressed())
    {
        SquadCommands::VehicleCommand(this, mCharacter);
    }

    if (mIsAlive && mPlayerId >= 0)
    {
        if (mState == LANDED)
        {
            ToolTipsDisplay::sInstance->ShowTip(TTT_AfterBoardFlyer, 0);
        }
        if (mState == FLYING)
        {
            ToolTipsDisplay::sInstance->ShowTip(TTT_FlyerControls, 0);
            ToolTipsDisplay::sInstance->ShowTip(TTT_FlyerFirePrim, 0);
            ToolTipsDisplay::sInstance->ShowTip(TTT_FlyerFireSec, 0);
            ToolTipsDisplay::sInstance->ShowTip(TTT_FlyerStunt, 0);
            ToolTipsDisplay::sInstance->ShowTip(TTT_FlyerBoost, 0);
            ToolTipsDisplay::sInstance->ShowTip(TTT_FlyerTargetLock, 0);
            ToolTipsDisplay::sInstance->ShowTip(TTT_FlyerTargetClear, 0);
            if (mControlFire[0].IsPressed())
            {
                ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_FlyerFirePrim, 0);
            }
            if (mControlFire[1].IsPressed())
            {
                ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_FlyerFireSec, 0);
            }
            if (!mInLandingRegionFlag_ToolTip && mInLandingRegionFactor > 0.0f)
            {
                ToolTipsDisplay::sInstance->ShowTip(TTT_FlyerLandingZone, 0);
            }
            mInLandingRegionFlag_ToolTip = mInLandingRegionFactor > 0.0f;
        }
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
                if (mControlSwitch[iChannel] >= 0.0f)
                {
                    if (++iNew > mClass->mWeaponCount - 1)
                    {
                        iNew = 0;
                    }
                }
                else
                {
                    if (--iNew < 0)
                    {
                        iNew = mClass->mWeaponCount - 1;
                    }
                }
            } while (iNew != mWeaponIndex[iChannel] && mClass->mWeaponChannel[iNew] != iChannel);

            if (iNew == iIndex)
            {
                if (GetJoystickIndex() >= 0)
                {
                    WeaponClass::sWeaponChangeUnableSound.Play(&mMatrix.GetTranslation(), 0, false, true);
                }
            }
            else
            {
                mWeapon[iIndex]->Deselect(iChannel, false);
                mWeaponIndex[iChannel] = iNew;
                mWeapon[iNew]->Select(iChannel, false);
                mTargetInfo.mIsAiming = false;
            }
        }
    }

    if (mPlayerId >= 0)
    {
        int iIndex = mWeaponIndex[0];
        if (iIndex >= 0 && mWeapon[iIndex] && mWeapon[iIndex]->mClass->mZoomMax > 1.0f && mControlZoom.Tapped())
        {
            mTargetInfo.mIsAiming = mWeapon[iIndex]->CycleZoom(mTargetInfo.mIsAiming);
        }
    }

    float fGroundDist = FLT_MAX;
    PblVector3 vNormal = gVectorUnitY;
    switch (mState)
    {
    case LANDED:
        mFlightRatio = 0.0f;
        if (mControlJump.IsPressed())
        {
            TakeOff();
            ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_AfterBoardFlyer, 0);
        }
        break;

    case TAKEOFF:
        if (mControlJump.IsPressed() && mFlightRatio > 0.75f)
        {

            GetPosition();
            mState = LANDING;
            WindDown(mClass, mMatrix.GetTranslation(), mVelocity);
        }
        else
        {
            PblVector3 vDown(0.0f, -1.0f, 0.0f);
            CollisionObject* pHit = 0;
            const GameObject* pIgnore = this;
            float fDist = CollisionManager::RayHit(mMatrix.GetTranslation(), vDown, 1024.0f, pHit, &vNormal, &pIgnore,
                                                   1, 400, false) *
                          1024.0f;
            mLastAltitude = fDist;
            fGroundDist = fDist;
            if (fDist > mClass->mTakeoffHeight + mLandedHeight)
            {
                mFlightRatio += fDeltaTime / mClass->mTakeoffTime;
                if (mFlightRatio >= 1.0f)
                {
                    mFlightRatio = 1.0f;
                    mWorldRoll = 0.0f;
                    mState = FLYING;
                }
            }
        }
        break;

    case FLYING:
        mFlightRatio = 1.0f;
        if (mControlJump.IsPressed() && !mClass->mGuidedMissile &&
            (mInLandingRegionFactor > 0.99f || !TriggerRegionManager::sInstance->mLandingRegionsExist))
        {
            GetPosition();
            mState = LANDING;
            mLastAltitude = FLT_MAX;
            WindDown(mClass, mMatrix.GetTranslation(), mVelocity);
            ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_FlyerLandingZone, 0);
        }
        break;

    case LANDING:
        if (mControlJump.IsPressed() && 0.05f > mFlightRatio)
        {
            mState = TAKEOFF;
        }
        else
        {
            mFlightRatio -= fDeltaTime / mClass->mLandingTime;
            if (0.0f > mFlightRatio)
            {
                mFlightRatio = 0.0f;
            }

            const PblVector3& vCenter = GetCollisionCenter();
            PblVector3 vDown(0.0f, -1.0f, 0.0f);
            CollisionObject* pHit = 0;
            const GameObject* pIgnore = this;
            float fCenterY = vCenter.y;
            float fBodyY = matrix.m[3][1];
            float fDist = CollisionManager::RayHit(vCenter, vDown, 1024.0f, pHit, &vNormal, &pIgnore, 1, 400, false) *
                              1024.0f -
                          (fCenterY - fBodyY);
            fGroundDist = fDist;
            if (mLandedHeight + GetCollisionRadius() >= fDist && mLastAltitude > mLandedHeight + GetCollisionRadius())
            {
                PlayDustEffect();
            }
            if (netIsLocalTurn)
            {
                mLastAltitude = fDist;
            }
            if (mLandedHeight > fDist &&
                matrix.m[1][1] * vNormal.y + matrix.m[1][0] * vNormal.x + matrix.m[1][2] * vNormal.z > 0.9375f)
            {

                bool bDeny = false;
                if (pHit)
                {
                    bDeny = pHit->GetCollisionType() == COLL_WATER;
                    if (pHit->mCollisionModel)
                    {
                        bDeny |= pHit->mCollisionModel->mDenyFlyerLand;
                    }
                }
                if (bDeny)
                {
                    mState = TAKEOFF;
                }
                else
                {
                    mState = LANDED;
                    SendLandedEvent();
                    TurnOffThrustEffects();
                    AIUtil::TellAIToExit(this);
                }
            }
        }
        break;

    case CRASHING:
        mFlightRatio -= fDeltaTime * 0.2f;
        if (0.0f > mFlightRatio)
        {
            mFlightRatio = 0.0f;
        }
        break;
    }

    float fAcceleration =
        mClass->mBoostFlyerSpeed > 0.0f && (mBoost || mGetSpeedSpeed > GetFlyerMaxSpeed()) ? mClass->mBoostAcceleration
                                                                                           : mClass->mAcceleration;
    float fAccelStep = fAcceleration * fDeltaTime;
    if (!GetFollowingPath())
    {
        if (mBoost && mClass->mBoostFlyerSpeed != 0.0f)
        {
            mSetSpeed = mClass->mBoostFlyerSpeed;
        }
        else
        {
            if (0.0f > fThrust)
            {
                mSetSpeed = GetFlyerMidSpeed() - (GetFlyerMinSpeed() - GetFlyerMidSpeed()) * fThrust;
            }
            else
            {
                mSetSpeed = (GetFlyerMaxSpeed() - GetFlyerMidSpeed()) * fThrust + GetFlyerMidSpeed();
            }
            float fMin = GetFlyerMinSpeed();
            float fMax = GetFlyerMaxSpeed();
            if (fMin > mSetSpeed)
            {
                mSetSpeed = fMin;
            }
            else if (mSetSpeed > fMax)
            {
                mSetSpeed = fMax;
            }
        }
        float fDecel = mInLandingRegionFactor == 0.0f ? 1.0f : 3.0f;
        float fLow = mGetSpeedSpeed - fDecel * fAccelStep;
        float fHigh = mGetSpeedSpeed + fAccelStep;
        if (fLow > mSetSpeed)
        {
            mSetSpeed = fLow;
        }
        else if (mSetSpeed > fHigh)
        {
            mSetSpeed = fHigh;
        }

        float fStepX = mSideRoll.x == 0.0f ? 0.033333335f : 1.0f;
        float fStepY = mSideRoll.y == 0.0f ? 0.033333335f : 1.0f;
        float fStrafe = mClass->mStrafeSpeed * 0.0f + mSideRoll.x * mClass->mTrickSideRollStrafeSpeed;
        fLow = mSetStrafe - mClass->mAcceleration * fStepX;
        fHigh = mClass->mAcceleration * fStepX + mSetStrafe;
        if (fLow > fStrafe)
        {
            mSetStrafe = fLow;
        }
        else if (fStrafe > fHigh)
        {
            mSetStrafe = fHigh;
        }
        else
        {
            mSetStrafe = fStrafe;
        }
        float fStrafeVertical = mSideRoll.y * mClass->mTrickSideRollStrafeSpeed;
        fLow = mSetStrafeVertical - mClass->mAcceleration * fStepY;
        fHigh = mClass->mAcceleration * fStepX + mSetStrafeVertical;
        if (fLow > fStrafeVertical)
        {
            mSetStrafeVertical = fLow;
        }
        else if (fStrafeVertical > fHigh)
        {
            mSetStrafeVertical = fHigh;
        }
        else
        {
            mSetStrafeVertical = fStrafeVertical;
        }
    }

    const PblVector3& vX = matrix.GetXAxis();
    const PblVector3& vY = matrix.GetYAxis();
    const PblVector3& vZ = matrix.GetZAxis();
    switch (mState)
    {
    case LANDED:
        mVelocity = gVectorZero;
        mOmega = gVectorZero;
        mBankPitchAngle = 0.0f;
        mBankPitchAngleDelta = 0.0f;
        mBankRollAngle = 0.0f;
        mBankRollAngleDelta = 0.0f;
        break;

    case TAKEOFF:
    case LANDING:
    {

        float fHeight = fGroundDist / mClass->mTakeoffHeight;
        if (fHeight >= 1.0f)
        {
            fHeight = 1.0f;
        }
        float fLevel = 1.0f - (1.0f - mFlightRatio) * mClass->mLevelFilterLanding * fDeltaTime;
        if (0.0f > fLevel)
        {
            fLevel = 0.0f;
        }
        mOmega.x = fLevel * mOmega.x;
        mOmega.x = (vZ.y * vNormal.y + vNormal.x * vZ.x + vNormal.z * vZ.z) * (mClass->mLevelFilterLanding * 0.5f) *
                       (1.0f - mFlightRatio) +
                   mOmega.x;
        float fPitchFilter = mClass->mPitchFilter * fDeltaTime;
        if (fPitchFilter > 1.0f)
        {
            fPitchFilter = 1.0f;
        }
        mOmega.x = (-(fPitchRateNow * fPitch * mFlightRatio) - mOmega.x) * fPitchFilter + mOmega.x;
        float fTurnFilter = mClass->mTurnFilter * fDeltaTime;
        if (fTurnFilter > 1.0f)
        {
            fTurnFilter = 1.0f;
        }
        mOmega.y = (-(fTurnRateNow * fTurn * fHeight) - mOmega.y) * fTurnFilter + mOmega.y;
        mOmega.z = -((vX.y * vNormal.y + vNormal.x * vX.x + vNormal.z * vX.z) * mClass->mLevelFilterLanding);

        PblVector3 vDrive;
        vDrive.x = mSetSpeed * vZ.x - mSetStrafe * vX.x - mSetStrafeVertical * vY.x;
        vDrive.y = vZ.y * mSetSpeed - vX.y * mSetStrafe - vY.y * mSetStrafeVertical;
        vDrive.z = mSetSpeed * vZ.z - mSetStrafe * vX.z - vY.z * mSetStrafeVertical;
        float fClimb = (fHeight * 0.875f + 0.125f) *
                       (mState == TAKEOFF ? mClass->mTakeoffSpeed : -mClass->mLandingSpeed);
        mVelocity = Interpolate(PblVector3(0.0f, fClimb, 0.0f), vDrive, mFlightRatio);

        PblMatrix mInverse = matrix.GetInverseFast();
        PblVector3 vLocal;
        D3DXVec3TransformNormal(&vLocal, &mVelocity, &mInverse);

        float fBankScale = mState == TAKEOFF ? 1.0f / (mClass->mTurnRate * fTopSpeed) : 1.0f / fTopSpeed;
        float fPitchTarget = mClass->mThrustPitchAngle * fThrust * mFlightRatio +
                             (vLocal.y * mOmega.x - vLocal.x * mOmega.y) * fBankScale * mClass->mBankAngle;
        mBankPitchAngle = (fPitchTarget - mBankPitchAngle) * (mClass->mBankFilter * fDeltaTime) + mBankPitchAngle;
        mBankPitchAngleDelta = fPitchTarget - mBankPitchAngle;
        float fRollTarget = mClass->mStrafeRollAngle * 0.0f * mFlightRatio -
                            (vLocal.z * mOmega.y - mOmega.z * vLocal.y) * fBankScale * mClass->mBankAngle;
        mBankRollAngle = (fRollTarget - mBankRollAngle) * (mClass->mBankFilter * fDeltaTime) + mBankRollAngle;
        mBankRollAngleDelta = fRollTarget - mBankRollAngle;
        break;
    }

    case FLYING:
    {

        mWorldRoll = mClass->mRollRateAccel * fDeltaTime * (fRoll - mWorldRoll) + mWorldRoll;
        if ((float)fabs((double)mWorldRoll) > 0.0001f)
        {
            PblMatrix mRoll;
            PblMatrix mRolled;
            D3DXMatrixRotationZ(&mRoll, mClass->mRollRate * mWorldRoll * fDeltaTime);
            D3DXMatrixMultiply(&mRolled, &mRoll, &matrix);
            matrix = mRolled;
            if (fRoll != 0.0f)
            {
                mAutoLevelingAllowed = 0.0f;
            }
        }
        else
        {
            mWorldRoll = 0.0f;
        }
        if (GetFollowingPath())
        {
            break;
        }

        if (mInLandingRegionFactor == 0.0f)
        {

            if (mControlStrafeLeft.DoubleTapped())
            {
                mSideRoll.x = -1.0f;
                mSideRoll.y = 0.0f;
                DoTrick(SIDE_ROLL);
            }
            else if (mControlStrafeRight.DoubleTapped())
            {
                mSideRoll.x = 1.0f;
                mSideRoll.y = 0.0f;
                DoTrick(SIDE_ROLL);
            }
            else if (mControlThrustBack.DoubleTapped())
            {
                DoTrick(FLIP_UP);
            }
            else if (mTrick > 0.0f)
            {
                if (mTrickDoubleclick == 0.0f)
                {
                    if (fAbsPitch > 0.2f || fAbsTurn > 0.2f)
                    {
                        mSideRoll.x = fTurn;
                        mSideRoll.y = -fPitch;
                        mSideRoll = Normalize(mSideRoll);
                        DoTrick(SIDE_ROLL);
                    }
                }
                else if (fAbsPitch > 0.2f || fAbsTurn > 0.2f)
                {
                    if (fAbsPitch <= fAbsTurn)
                    {
                        DoTrick(fTurn >= 0.0f ? FLIP_UP_RIGHT : FLIP_UP_LEFT);
                    }
                    else
                    {
                        DoTrick(fPitch < 0.0f ? FLIP_DOWN : FLIP_UP);
                    }
                }
            }

            if (mTrick > -0.2f)
            {
                if (fAbsPitch > (float)fabs((double)mPitchControllerPrevUpdate))
                {
                    fPitch = mPitchControllerPrevUpdate;
                    fAbsPitch = (float)fabs((double)fPitch);
                }
                if (fAbsTurn > (float)fabs((double)mTurnControllerPrevUpdate))
                {
                    fTurn = mTurnControllerPrevUpdate;
                    fAbsTurn = (float)fabs((double)fTurn);
                }
            }
        }

        if (mPlayerId >= 0 || !mCtrl)
        {
            float fY = mMatrix.GetTranslation().y;
            float fMinY = AIUtil::gMinPlayerFlyHeight;
            float fMaxY = AIUtil::gMaxPlayerFlyHeight;
            if (fMinY > fY || fY > fMaxY)
            {
                mFlyerAutoCorrectingToStayInBattlefield = true;
            }
            float fT = 1.0f;
            PblVector3 vUp = gVectorUnitY;
            bool bCorrect = false;
            if (0.0f > mVelocity.y)
            {
                fT = (fMinY - fY) * 0.5f / mVelocity.y;
                if (0.0f > fT)
                {
                    fT = 0.0f;
                }
                bCorrect = fT < 1.0f;
            }
            else if (mVelocity.y > 0.0f)
            {
                float fTime = (fMaxY - fY) * 0.5f / mVelocity.y;
                if (0.0f > fTime)
                {
                    fT = 0.0f;
                }
                else if (!(fTime > 1.0f))
                {
                    fT = fTime;
                }
                vUp.x = -gVectorUnitY.x;
                vUp.y = -gVectorUnitY.y;
                vUp.z = -gVectorUnitY.z;
                bCorrect = fT < 1.0f;
            }
            if (bCorrect)
            {
                float fDot = vZ.y * vUp.y + vZ.x * vUp.x + vUp.z * vZ.z;
                if ((mVelocity.y * vZ.y + vZ.x * mVelocity.x + mVelocity.z * vZ.z) * fDot <= 0.0f)
                {
                    if (vY.y * vUp.y + vY.x * vUp.x + vUp.z * vY.z > 0.0f)
                    {
                        fDot = -fDot;
                    }
                    float fCorrect = fDot / (mClass->mPitchRate * fDeltaTime);
                    if (-1.0f > fCorrect)
                    {
                        fCorrect = -1.0f;
                    }
                    else if (fCorrect > 1.0f)
                    {
                        fCorrect = 1.0f;
                    }
                    fPitch = (fPitch - fCorrect) * (fT * fT) + fCorrect;
                    fAbsPitch = (float)fabs((double)fPitch);
                }
            }
        }

        float fPitchCommand = -(fPitchRateNow * fPitch * fPitchScale);
        float fPitchFilter = (mOmega.x >= 0.0f && fPitchCommand >= mOmega.x) ||
                                     (mOmega.x <= 0.0f && fPitchCommand <= mOmega.x)
                                 ? mClass->mPitchFilter
                                 : mClass->mPitchFilterDecel;
        float fTurnCommand = -(fTurnRateNow * fTurn * fTurnScale);
        float fTurnFilter = (mOmega.y > 0.0f && fTurnCommand > mOmega.y) || (mOmega.y < 0.0f && fTurnCommand < mOmega.y)
                                ? mClass->mTurnFilter
                                : mClass->mTurnFilterDecel;
        float fOmegaX = mOmega.x;
        if (fPitchCommand > fOmegaX)
        {
            fOmegaX += fPitchFilter * fDeltaTime;
            if (fOmegaX > fPitchCommand)
            {
                fOmegaX = fPitchCommand;
            }
        }
        else if (fOmegaX > fPitchCommand)
        {
            fOmegaX -= fPitchFilter * fDeltaTime;
            if (fPitchCommand > fOmegaX)
            {
                fOmegaX = fPitchCommand;
            }
        }
        float fOmegaY = mOmega.y;
        if (fTurnCommand > fOmegaY)
        {
            fOmegaY += fTurnFilter * fDeltaTime;
            if (fOmegaY > fTurnCommand)
            {
                fOmegaY = fTurnCommand;
            }
        }
        else if (fOmegaY > fTurnCommand)
        {
            fOmegaY -= fTurnFilter * fDeltaTime;
            if (fTurnCommand > fOmegaY)
            {
                fOmegaY = fTurnCommand;
            }
        }
        mOmega.x = fOmegaX;
        mOmega.y = fOmegaY;

        if (mInLandingRegionFactor != 0.0f)
        {
            mAutoLevelingAllowed = 15.0f;
        }
        if (mAutoLevelingAllowed != 0.0f && !mInRoll && !mInFlip)
        {
            mAutoLevelingAllowed += fDeltaTime;
            if (mAutoLevelingAllowed > 15.0f)
            {
                mAutoLevelingAllowed = 15.0f;
            }
            if (14.0f > mAutoLevelingAllowed && (fAbsPitch > 0.1f || fAbsTurn > 0.1f))
            {
                mAutoLevelingAllowed = 0.01f;
            }
        }
        if (!mInRoll && !mInFlip && mAutoLevelingAllowed >= 14.0f)
        {
            mWorldRoll = 0.0f;
            mOmega.z = -(mClass->mLevelFilter * vX.y);
        }
        else
        {
            mOmega.z = 0.0f;
        }

        const PblVector3& vForward = mMatrix.GetZAxis();
        PblVector3 vDrive;
        vDrive.x = vForward.x * mSetSpeed - vX.x * mSetStrafe - vY.x * mSetStrafeVertical;
        vDrive.y = vForward.y * mSetSpeed - vX.y * mSetStrafe - vY.y * mSetStrafeVertical;
        vDrive.z = vForward.z * mSetSpeed - vX.z * mSetStrafe - vY.z * mSetStrafeVertical;
        float fMomentum = mClass->mMomentumFilter * fDeltaTime;
        if (fMomentum >= 0.0f)
        {
            if (fMomentum > 1.0f)
            {
                fMomentum = 1.0f;
            }
        }
        else
        {
            fMomentum = 0.0f;
        }
        mVelocity.x = (1.0f - fMomentum) * mVelocity.x + fMomentum * vDrive.x;
        mVelocity.y = mVelocity.y * (1.0f - fMomentum) + vDrive.y * fMomentum;
        mVelocity.z = mVelocity.z * (1.0f - fMomentum) + vDrive.z * fMomentum;
        float fSpeed = mSetSpeed;
        float fForwardSpeed = mVelocity.y * vForward.y + mVelocity.x * vForward.x + mVelocity.z * vForward.z;
        if (fForwardSpeed > fSpeed || (fSpeed > fForwardSpeed && fForwardSpeed > 0.0f))
        {
            float fScale = fSpeed / fForwardSpeed;
            mVelocity.x *= fScale;
            mVelocity.y *= fScale;
            mVelocity.z *= fScale;
        }

        PblMatrix mInverse = matrix.GetInverseFast();
        PblVector3 vLocal;
        D3DXVec3TransformNormal(&vLocal, &mVelocity, &mInverse);
        float fBankScale = 1.0f / fTopSpeed;
        float fRollSlip = (vLocal.z * mOmega.y - mOmega.z * vLocal.y) * fBankScale;
        float fPitchTarget = mClass->mThrustPitchAngle * fThrust + mFlip +
                             (vLocal.y * mOmega.x - vLocal.x * mOmega.y) * fBankScale * mClass->mBankAngle;
        mBankPitchAngle = (fPitchTarget - mBankPitchAngle) * (mClass->mBankFilter * fDeltaTime) + mBankPitchAngle;
        mBankPitchAngleDelta = fPitchTarget - mBankPitchAngle;
        float fRollTarget = mClass->mStrafeRollAngle * 0.0f + mRoll - mClass->mBankAngle * fRollSlip;
        mBankRollAngle = (fRollTarget - mBankRollAngle) * (mClass->mBankFilter * fDeltaTime) + mBankRollAngle;
        mBankRollAngleDelta = fRollTarget - mBankRollAngle;
        break;
    }

    case CRASHING:
    {

        if (mVelocity.x * mVelocity.x + mVelocity.z * mVelocity.z > 1.0f)
        {
            PblVector3 vDir;
            D3DXVec3Normalize(&vDir, &mVelocity);
            mOmega.x = mOmega.x - (vY.y * vDir.y + vY.x * vDir.x + vY.z * vDir.z) * mClass->mLevelFilter *
                                      (1.0f - mFlightRatio);
            float fPitchFilter = mClass->mPitchFilter * mFlightRatio * fDeltaTime;
            if (fPitchFilter > 1.0f)
            {
                fPitchFilter = 1.0f;
            }
            mOmega.x = (fPitchRateNow * fPitch * -0.5f - mOmega.x) * fPitchFilter + mOmega.x;
        }
        float fTurnFilter = mClass->mTurnFilter * mFlightRatio * fDeltaTime;
        if (fTurnFilter > 1.0f)
        {
            fTurnFilter = 1.0f;
        }
        mOmega.y = (fTurnRateNow * fTurn * -0.5f - mOmega.y) * fTurnFilter + mOmega.y;
        float fDrift = (mFlightRatio * 0.75f + 0.25f) * mClass->mLevelFilter * fDeltaTime;
        if (fDrift > 1.0f)
        {
            fDrift = 1.0f;
        }
        PblVector3 vDrive;
        vDrive.x = mSetSpeed * vZ.x - mSetStrafe * vX.x - mSetStrafeVertical * vY.x;
        vDrive.y = vZ.y * mSetSpeed - vX.y * mSetStrafe - mSetStrafeVertical * vY.y;
        vDrive.z = mSetSpeed * vZ.z - mSetStrafe * vX.z - mSetStrafeVertical * vY.z;
        mVelocity.x = (vDrive.x - mVelocity.x) * fDrift + mVelocity.x;
        mVelocity.y = (vDrive.y - mVelocity.y) * fDrift + mVelocity.y;
        mVelocity.z = (vDrive.z - mVelocity.z) * fDrift + mVelocity.z;
        mVelocity.y = (1.0f - mFlightRatio) * -9.8f * fDeltaTime + mVelocity.y;

        float fFree = 1.0f - mFlightRatio;
        float fOmegaZ = (mVelocity.y * vZ.x - vZ.y * mVelocity.x) * 0.015625f * fFree * fDeltaTime + mOmega.z;
        float fOmegaX = mOmega.x + fFree * ((vZ.y * mVelocity.z - mVelocity.y * vZ.z) * 0.015625f) * fDeltaTime;
        mOmega.y = (mVelocity.x * vZ.z - mVelocity.z * vZ.x) * 0.015625f * fFree * fDeltaTime + mOmega.y;
        mOmega.z = fOmegaZ;
        mOmega.x = fOmegaX;
        mOmega.x = (PblRandom::GetSim().GetFloat() - 0.5f) * 0.25f + vZ.y * 0.0625f + 0.0625f + mOmega.x;
        mOmega.y = (PblRandom::GetSim().GetFloat() - 0.5f) * 0.25f + mOmega.y;
        mOmega.z = (PblRandom::GetSim().GetFloat() - 0.5f) * 0.5f + mOmega.z;

        PblMatrix mInverse = matrix.GetInverseFast();
        PblVector3 vLocal;
        D3DXVec3TransformNormal(&vLocal, &mVelocity, &mInverse);
        float fBankScale = 1.0f / fTopSpeed;
        float fRollSlip = (vLocal.z * mOmega.y - mOmega.z * vLocal.y) * fBankScale;
        float fPitchTarget = mClass->mThrustPitchAngle * fThrust * mFlightRatio + mFlip +
                             (vLocal.y * mOmega.x - vLocal.x * mOmega.y) * fBankScale * mClass->mBankAngle;
        mBankPitchAngle = (fPitchTarget - mBankPitchAngle) * (mClass->mBankFilter * fDeltaTime) + mBankPitchAngle;
        mBankPitchAngleDelta = fPitchTarget - mBankPitchAngle;
        float fRollTarget = mClass->mStrafeRollAngle * 0.0f * mFlightRatio + mRoll - mClass->mBankAngle * fRollSlip;
        mCrashTimer -= fDeltaTime;
        mBankRollAngle = (fRollTarget - mBankRollAngle) * (mClass->mBankFilter * fDeltaTime) + mBankRollAngle;
        mBankRollAngleDelta = fRollTarget - mBankRollAngle;
        if (0.0f > mCrashTimer)
        {
            mState = CRASHED;
        }
        break;
    }
    }

    if (mState != FLYING || mInLandingRegionFactor != 0.0f)
    {
        mControlFire[0].Clear();
        mControlFire[1].Clear();
        for (int i = 0; i < mNumTurrets; ++i)
        {
            if (mTurret[i])
            {
                mTurret[i]->mControlFire[0].Clear();
                mTurret[i]->mControlFire[1].Clear();
            }
        }
    }

    if (!GetFollowingPath())
    {
        PblQuaternion qBody(matrix);
        PblVector3 vPos = matrix.GetTranslation();
        float fSpin = sqrtf(mOmega.x * mOmega.x + mOmega.y * mOmega.y + mOmega.z * mOmega.z);
        if (fSpin > 0.0001f)
        {
            float fInvSpin = 1.0f / fSpin;
            float fAxisX = mOmega.x * fInvSpin;
            float fAxisY = mOmega.y * fInvSpin;
            float fAxisZ = mOmega.z * fInvSpin;
            float fSin;
            float fCos;
            PblMath::SinCos(&fSin, &fCos, fSpin * fDeltaTime * 0.5f);
            qBody = qBody * PblQuaternion(fAxisX * fSin, fAxisY * fSin, fAxisZ * fSin, fCos);
        }
        vPos.x += mVelocity.x * fDeltaTime;
        vPos.y += mVelocity.y * fDeltaTime;
        vPos.z += mVelocity.z * fDeltaTime;
        float fMinY = mPlayerId >= 0 ? AIUtil::gMinPlayerFlyHeight : AIUtil::gMinFlyHeight;
        float fMaxY = mPlayerId >= 0 ? AIUtil::gMaxPlayerFlyHeight : AIUtil::gMaxFlyHeight;
        if (vPos.y > fMaxY + 0.2f)
        {
            vPos.y = fMaxY + 0.2f;
        }
        else if (fMinY - 0.2f > vPos.y)
        {
            vPos.y = fMinY - 0.2f;
        }

        PblQuaternion qUnit;
        PblMatrix mFrame;
        D3DXMatrixRotationQuaternion(&mFrame, D3DXQuaternionNormalize(&qUnit, &qBody));
        mFrame.m[3][0] = vPos.x;
        mFrame.m[3][1] = vPos.y;
        mFrame.m[3][2] = vPos.z;
        mFrame.m[3][3] = 1.0f;
        matrix = mFrame;
        PblMatrix mBank;
        PblMatrix mBankRoll;
        D3DXMatrixRotationX(&mBank, mBankPitchAngle);
        D3DXMatrixMultiply(&mBank, &mBank, D3DXMatrixRotationZ(&mBankRoll, mBankRollAngle));
        D3DXMatrixMultiply(&mFrame, &mBank, &matrix);
        SetMatrix(mFrame);
    }

    for (int i = 0; i < 8; ++i)
    {
        if (!mClass->mThrustEffectName[i])
        {
            break;
        }
        ParticleEmitterObject* pEffect = mThrustEffect[i];
        if (!pEffect)
        {
            continue;
        }
        float fMaxSpeed = mClass->mMaxFlyerSpeed;
        if (fMaxSpeed == 0.0f)
        {
            fMaxSpeed = 1.0f;
        }
        float fMinSpeed = mClass->mMinFlyerSpeed;
        float fStart = (fMaxSpeed - fMinSpeed) * mClass->mThrustEffectScaleStart[i] + fMinSpeed;
        float fTop = mClass->mBoostFlyerSpeed > fMaxSpeed ? mClass->mBoostFlyerSpeed : fMaxSpeed;
        float fT = (mGetSpeedSpeed - fStart) / (fTop - fStart);
        if (0.0f > fT)
        {
            fT = 0.0f;
        }
        pEffect->ScaleSet((mClass->mThrustEffectMaxScale[i] - mClass->mThrustEffectMinScale[i]) * fT +
                          mClass->mThrustEffectMinScale[i]);
    }

    int iFirstPerson = GetFirstPersonIndex();
    if (mIsAlive && iFirstPerson >= 0)
    {
        FirstPerson::Update(iFirstPerson, TYPE_COCKPIT, mClass->mCockpitModelHandle, this, 0);
    }

    if (fDeltaTime > 0.0f)
    {
        float fHealth = mMaxHealth != 0.0f ? mCurHealth / mMaxHealth : 0.0f;
        if (mPilot || mProcedurallyAnimated)
        {
            if (!IsOn())
            {
                TurnOn(mClass, mMatrix.GetTranslation(), mVelocity, fHealth);
            }
        }
        else if (IsOn() && mState == LANDED)
        {
            TurnOff(mClass, mMatrix.GetTranslation(), mVelocity);
        }

        if (mSoundTrick.GetPlaying())
        {
            const GameSound* pSound =
                mSideRoll.x == 0.0f && mSideRoll.y == 0.0f ? &mClass->mSoundFlip : &mClass->mSoundTrick;
            mSoundTrick.Update(pSound, &GetMatrix().GetTranslation(), &GetVelocity());
        }

        if (!IsOn())
        {
            VehicleMusic::Update(mClass, fDeltaTime, false, 0.0f, 0);
        }
        else
        {

            bool bFirstFrame = false;
            const PblVector3* pVelocity;
            if (mProcedurallyAnimated)
            {
                PblVector3& vEngineVelocity = *(PblVector3*)mAIEng.eng.velocity;
                PblVector3& vPrevPos = *reinterpret_cast<PblVector3*>(mAIEng.eng.prevPos.mPosition);
                float& fPrevPosValid = mAIEng.eng.prevPos.mValid;
                if (fPrevPosValid == 0.0f)
                {
                    fPrevPosValid = 1.0f;
                    bFirstFrame = true;
                }
                else
                {
                    vEngineVelocity.x = mMatrix.GetTranslation().x - vPrevPos.x;
                    vEngineVelocity.y = mMatrix.GetTranslation().y - vPrevPos.y;
                    vEngineVelocity.z = mMatrix.GetTranslation().z - vPrevPos.z;
                }
                PblVector3 vDir = NormalizeEst(vEngineVelocity);
                float fClimb = mMatrix.m[1][0] * vDir.x + mMatrix.m[1][1] * vDir.y + mMatrix.m[1][2] * vDir.z;
                if (fClimb > 0.8f)
                {
                    if (!IsOn())
                    {
                        PblVector3 vPos = mMatrix.GetTranslation();
                        PblVector3 vVel = vEngineVelocity;
                        TurnOn(mClass, vPos, vVel, fHealth);
                    }
                }
                else if (-0.8f > fClimb && IsOn() && !IsWindingDown())
                {
                    PblVector3 vPos = mMatrix.GetTranslation();
                    PblVector3 vVel = vEngineVelocity;
                    WindDown(mClass, vPos, vVel);
                }
                vPrevPos = mMatrix.GetTranslation();
                pVelocity = &vEngineVelocity;
            }
            else
            {
                pVelocity = &mVelocity;
            }

            float fSpeed = LengthEst(*pVelocity);
            float fTop = mClass->mBoostFlyerSpeed;
            if (!(fTop > 0.0f))
            {
                fTop = GetFlyerMaxSpeed() == 0.0f ? 1.0f : GetFlyerMaxSpeed();
            }
            float fSpeedRatio = fSpeed / fTop;
            if (0.0f > fSpeedRatio)
            {
                fSpeedRatio = 0.0f;
            }
            else if (fSpeedRatio > 1.0f)
            {
                fSpeedRatio = 1.0f;
            }

            if (mProcedurallyAnimated)
            {
                if (0.05f > fSpeed)
                {
                    if (!bFirstFrame && IsOn() && IsWindingDown())
                    {
                        PblVector3 vPos = mMatrix.GetTranslation();
                        PblVector3 vVel = *pVelocity;
                        TurnOff(mClass, vPos, vVel);
                    }
                }
                else if (IsOn() && !IsWindingUp() && !IsWindingDown() && !IsTurningOff())
                {
                    PblVector3 vPos = mMatrix.GetTranslation();
                    PblVector3 vVel = *pVelocity;
                    WindUp(mClass, vPos, vVel);
                }
            }

            float fAccelRatio = (fSpeedRatio - mSpeedPrev) / fDeltaTime;
            if (!mInRoll && !mInFlip)
            {
                if (mTrickEngineValue > 0.0f)
                {
                    float fValue = mTrickEngineValue - fDeltaTime * 0.5f;
                    if (0.0f > fValue)
                    {
                        fValue = 0.0f;
                    }
                    else if (fValue > 1.0f)
                    {
                        fValue = 1.0f;
                    }
                    mTrickEngineValue = fValue;
                }
            }
            else if (0.0f > mTrickEngineValue)
            {
                float fValue = fDeltaTime * 2.0f + mTrickEngineValue;
                if (0.0f > fValue)
                {
                    fValue = 0.0f;
                }
                else if (fValue > 1.0f)
                {
                    fValue = 1.0f;
                }
                mTrickEngineValue = fValue;
            }
            if (fAccelStep == 0.0f)
            {
                fAccelStep = 1.0f;
            }
            fAccelRatio /= fAccelStep;

            Weapon* pWeapon = mWeapon[0];
            PblHandle<const GameObject> target = pWeapon && pWeapon->mTarget ? pWeapon->mTarget : mReticuleTarget[0];
            float fDistance;
            const GameObject* pTarget = target;
            if (pTarget)
            {
                const PblVector3& vTarget = pTarget->GetMatrix().GetTranslation();
                fDistance = PblMath::Length3Est(vTarget.x - mMatrix.GetTranslation().x,
                                                vTarget.y - mMatrix.GetTranslation().y,
                                                vTarget.z - mMatrix.GetTranslation().z);
            }
            else
            {
                fDistance = mClass->mProximityMax;
            }
            mLastDistanceToTarget = (fDistance - mLastDistanceToTarget) * (fDeltaTime * 6.0f) + mLastDistanceToTarget;

            bool bFirstPerson = mTracker && mTracker->IsFirstPersonView();
            bool bCockpit = mPlayerId >= 0 &&
                            (NetGame::GetLocalPlayer(0) == mCharacter || NetGame::GetLocalPlayer(1) == mCharacter);
            float fBoost = mInRoll || mInFlip ? 0.0f : mTurbulance;
            VehicleEngine::Update(mClass, fDeltaTime, mMatrix.GetTranslation(), *pVelocity, fSpeedRatio, fAccelRatio,
                                  fHealth, fBoost, 0.0f, 0.0f, mTrickEngineValue, bCockpit, bFirstPerson,
                                  mLastDistanceToTarget);
            mSpeedPrev = fSpeedRatio;

            int iSide = 0;
            if (mPilot && mPilot->mCharacter)
            {
                iSide = mPilot->mCharacter->GetTeamPtr()->GetSide();
            }
            bool bPiloted = mPilot && mPilot->mPlayerId >= 0 && GameLoop::GetNumCameras() == 1;
            VehicleMusic::Update(mClass, fDeltaTime, bPiloted, fSpeedRatio, iSide);
        }
    }

    if (mObstacle)
    {
        mObstacle->UpdatePosition(GetMatrix(), mClass->mBarrierOX, mClass->mBarrierOZ, GetVelocity());
    }

    mPitchControllerPrevUpdate = fPitch;
    mTurnControllerPrevUpdate = fTurn;
    return true;
}

// FUNCTION: SWBF2 0x004b3de0
bool EntityFlyer::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                    Restrictor* pRestrictor)
{
    const PblVector3& vNormal = pResult->mSeparationNormal;

    if (mClass->mGuidedMissile)
    {
        mMatrix.m[3][0] += vNormal.x * pResult->mOverlap;
        mMatrix.m[3][1] += vNormal.y * pResult->mOverlap;
        mMatrix.m[3][2] += vNormal.z * pResult->mOverlap;
        mMatrix.m[3][3] = 1.0f;
        if (mEnergyBar.GetEnergy() > 0.0f)
        {
            GameObject* pObject = pOther->GetGameObject();
            if (pObject)
            {
                DamageOwner owner(mCharacter, this, GetTeam(), mClass, 0, 1.0f);
                float fDamage;
                if (mClass->mBoostFlyerSpeed > 0.0f)
                {
                    float fBoost = (mGetSpeedSpeed - mClass->mMaxFlyerSpeed) /
                                   (mClass->mBoostFlyerSpeed - mClass->mMaxFlyerSpeed);
                    if (0.0f > fBoost)
                    {
                        fBoost = 0.0f;
                    }
                    else if (fBoost > 1.0f)
                    {
                        fBoost = 1.0f;
                    }
                    fDamage = (1.0f - fBoost) * mClass->mGuidedMissileDamage +
                              mClass->mGuidedMissileDamageBoost * fBoost;
                }
                else
                {
                    fDamage = mClass->mGuidedMissileDamage;
                }
                DamageDesc damage(owner, fDamage);
                damage.mShieldScale = mClass->mGuidedMissileShieldScale;
                for (int i = 0; i < 8; ++i)
                {
                    damage.mHealthScale[i] = mClass->mGuidedMissileHealthScale[i];
                }
                PblVector3 vDir(-vNormal.x, -vNormal.y, -vNormal.z);
                pObject->ApplyDamage(damage, vDir, pResult->mCollisionPosition, pResult->mReceiverId, 0xffffffff);
            }
            mEnergyBar.SetEnergy(0.0f);
        }
        return true;
    }

    if (mState == LANDING)
    {
        const CollisionModel* pModel = pOther->mCollisionModel;
        if (pModel)
        {
            if (pModel->mDenyFlyerLand)
            {
                mState = TAKEOFF;
            }
            else if (pModel->mDeathOnFlyerLand && vNormal.y > 0.6f)
            {
                GameObject* pObject = pOther->GetGameObject();
                if (pObject)
                {
                    pObject->InstantDeath(DamageOwner(mCharacter, this, GetTeam(), 0, 0, 1.0f), true);
                }
            }
        }
    }
    else if (mState == TAKEOFF)
    {
        if (pOther->GetCollisionType() == COLL_SOFT)
        {
            return true;
        }
        if (pOther->GetCollisionType() == COLL_ITEM)
        {
            return true;
        }
        if (pOther->GetGameObject() &&
            pOther->GetGameObject()->IsRtti(EntityPortableTurret::rttiHashEntityPortableTurret))
        {
            pOther->GetGameObject()->InstantDeath(DamageOwner(mCharacter, this, GetTeam(), 0, 0, 1.0f), false);
            return true;
        }
        if (((CollisionObject*)pOther)->GetCollisionType() >= GetCollisionType() && -0.7f > vNormal.y)
        {
            mState = LANDING;
        }
    }

    if (mState == LANDED && pOther->GetGameObject() &&
        pOther->GetGameObject()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        EntityFlyer* pFlyer = static_cast<EntityFlyer*>(pOther->GetGameObject());
        if (pFlyer->mState != LANDED)
        {
            Damageable::ApplyDamage(DamageDesc(DamageOwner(0, 0, 0, 0, 0, 1.0f), mMaxHealth * 0.2f), 0xffffffff);
        }
        else
        {
            float fDamage = mMaxHealth > pFlyer->mMaxHealth ? mMaxHealth : pFlyer->mMaxHealth;
            Damageable::ApplyDamage(DamageDesc(DamageOwner(0, 0, 0, 0, 0, 1.0f), fDamage), 0xffffffff);
        }
    }
    else if (pOther->GetCollisionType() >= GetCollisionType())
    {
        PblVector3 vRelVel = mVelocity;
        if (pOther->GetGameObject())
        {
            const PblVector3& vOtherVel = pOther->GetGameObject()->GetVelocity();
            vRelVel.x -= vOtherVel.x;
            vRelVel.y -= vOtherVel.y;
            vRelVel.z -= vOtherVel.z;
        }
        float fApproach = vNormal.y * vRelVel.y + vRelVel.x * vNormal.x + vNormal.z * vRelVel.z;
        if (0.0f > fApproach)
        {
            float fUp = mMatrix.m[1][1] * vNormal.y + mMatrix.m[1][0] * vNormal.x + mMatrix.m[1][2] * vNormal.z;
            float fFront = mMatrix.m[2][1] * vNormal.y + mMatrix.m[2][0] * vNormal.x + mMatrix.m[2][2] * vNormal.z;
            float fThreshold = mClass->mCollisionThreshold[1] * fUp + mClass->mCollisionThreshold[0] +
                               mClass->mCollisionThreshold[2] * fFront;
            if (mState != LANDING)
            {
                FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(), pOther, &GetPosition());
                PlayImpactEffects(&mMatrix.GetTranslation(), &mVelocity);
                PblVector3 vVel = mVelocity;
                PblVector3 vEngine = mMatrix.GetTranslation();
                VehicleEngine::Collision(mClass, vEngine, vVel);
                GetControllableList();
            }

            float fExcess = fThreshold + fApproach;
            if (fExcess > 0.0f)
            {
                fExcess = 0.0f;
            }
            float fDamage = (mClass->mCollisionScale[1] * fUp + mClass->mCollisionScale[0] +
                             mClass->mCollisionScale[2] * fFront) *
                            fExcess * fExcess;
            if (pOther->GetGameObject())
            {
                fDamage = pOther->GetGameObject()->GetGameObjectClass()->GetCollisionInflict() * fDamage;
            }
            if (mState == FLYING)
            {
                float fHeadOn = mMatrix.m[2][1] * -vNormal.y + -vNormal.x * mMatrix.m[2][0] +
                                mMatrix.m[2][2] * -vNormal.z;
                if (!(fHeadOn > 0.0f))
                {
                    fHeadOn = 0.0f;
                }
                fDamage = mMaxHealth * 0.5f * fHeadOn + fDamage;
            }

            float fApplied;
            if (!IsRtti(CommandFlyer::rttiHashCommandFlyer) && pOther->GetGameObject() &&
                pOther->GetGameObject()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
            {
                fApplied = mMaxHealth * 0.2f;
            }
            else if (pOther->GetGameObject())
            {
                float fCap = pOther->GetGameObject()->mMaxHealth * 0.5f;
                fApplied = fCap > fDamage ? fDamage : fCap;
            }
            else
            {
                fApplied = fDamage;
            }
            if (fApplied > 0.0f)
            {
                Damageable::ApplyDamage(DamageDesc(DamageOwner(0, 0, 0, 0, 0, 1.0f), fApplied), 0xffffffff);
            }

            if (!IsAlive() || mState == CRASHING)
            {
                mState = CRASHED;
                GetControllableList();
                InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), false);
            }
            else
            {
                float fBounce = fApproach * -1.4f;
                if (pOther->GetCollisionType() == GetCollisionType())
                {
                    float fInvMass = 1.0f / mMass;
                    fBounce = fInvMass / (1.0f / pOther->mMass + fInvMass) * fBounce;
                }

                mVelocity = vRelVel;
                mVelocity.x -= vNormal.x * fApproach;
                mVelocity.y -= vNormal.y * fApproach;
                mVelocity.z -= vNormal.z * fApproach;
                float fDrag = fBounce * 0.125f /
                              sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z);
                if (!(1.0f > fDrag))
                {
                    fDrag = 1.0f;
                }
                mVelocity.x -= mVelocity.x * fDrag;
                mVelocity.y -= mVelocity.y * fDrag;
                mVelocity.z -= mVelocity.z * fDrag;
                float fPush = fBounce + fApproach;
                mVelocity.x += fPush * vNormal.x;
                mVelocity.y += vNormal.y * fPush;
                mVelocity.z += vNormal.z * fPush;
                if (pOther->GetGameObject())
                {
                    const PblVector3& vOtherVel = pOther->GetGameObject()->GetVelocity();
                    mVelocity.x += vOtherVel.x;
                    mVelocity.y = vOtherVel.y + mVelocity.y;
                    mVelocity.z = vOtherVel.z + mVelocity.z;
                }

                if (mState == LANDING)
                {
                    if (vNormal.y > 0.7f &&
                        mMatrix.m[1][1] * vNormal.y + mMatrix.m[1][0] * vNormal.x + vNormal.z * mMatrix.m[1][2] >
                            0.9375f &&
                        0.0001f > mFlightRatio)
                    {
                        TurnOffThrustEffects();
                        SendLandedEvent();
                        mState = LANDED;
                        AIUtil::TellAIToExit(this);
                    }
                }
                else
                {
                    float fImpact =
                        sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z) /
                            40.0f +
                        0.01f;
                    if (!(1.0f > fImpact))
                    {
                        fImpact = 1.0f;
                    }
                    if (fImpact > 0.0f)
                    {
                        float fBlend = fImpact * 0.3f;
                        const PblVector3& vNose = mMatrix.GetZAxis();
                        PblVector3 vDir(vNose.x * (1.0f - fBlend) + fBlend * vNormal.x,
                                        vNose.y * (1.0f - fBlend) + vNormal.y * fBlend,
                                        vNose.z * (1.0f - fBlend) + vNormal.z * fBlend);
                        if (mVelocity.x != 0.0f || mVelocity.y != 0.0f || mVelocity.z != 0.0f)
                        {
                            PblVector3 vHeading;
                            D3DXVec3Normalize(&vHeading, &mVelocity);
                            vDir.x += vHeading.x * 0.3f;
                            vDir.y += vHeading.y * 0.3f;
                            vDir.z += vHeading.z * 0.3f;
                        }
                        mMatrix = DirectionMatrix(vDir, mMatrix.GetYAxis(), mMatrix.GetTranslation());
                        mSetSpeed = mVelocity.y * mMatrix.m[2][1] + mVelocity.x * mMatrix.m[2][0] +
                                    mVelocity.z * mMatrix.m[2][2];
                        mSetStrafe = mVelocity.y * mMatrix.m[0][1] + mVelocity.x * mMatrix.m[0][0] +
                                     mVelocity.z * mMatrix.m[0][2];
                        mSetStrafeVertical = mVelocity.y * mMatrix.m[1][1] + mMatrix.m[1][0] * mVelocity.x +
                                             mVelocity.z * mMatrix.m[1][2];

                        CameraManager* pManager = CameraManager::sInstance;
                        if (pManager->GetChaseCameraTarget(0) &&
                            pManager->GetChaseCameraTarget(0)->GetGameObject() == this)
                        {
                            CameraManager::sInstance->ApplyShake(fImpact * 0.8f, fImpact * 0.7f);
                            if (fImpact > 0.5f)
                            {
                                CameraManager::sInstance->mChaseCamera[0]->mTimer = 0.1f;
                            }
                        }
                    }
                }
            }
        }
    }

    bool bResult = CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
    if (bResult && pRestrictor)
    {
        pRestrictor->AddRestriction(vNormal);
        if (mState != LANDING && mState != TAKEOFF && mState != LANDED)
        {
            FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(), pOther, &GetPosition());
            PlayImpactEffects(&mMatrix.GetTranslation(), &mVelocity);
            PblVector3 vVel = mVelocity;
            PblVector3 vEngine = mMatrix.GetTranslation();
            VehicleEngine::Collision(mClass, vEngine, vVel);
        }
    }
    return bResult;
}

// FUNCTION: SWBF2 0x004b5f40
float EntityFlyer::GetSoundRadius() const
{
    if (mSoundEngine.GetPlayState() == Snd::SourceTransport::playState_playing && mSoundEngine.mProperties)
    {
        return mClass->mNoiseFactor;
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x004aaa60
void EntityFlyer::InitAsFlying()
{
    mOmega = gVectorZero;
    mControlMove = -1.0f;
    const PblVector3& vNose = mMatrix.GetZAxis();
    float fMin = GetFlyerMinSpeed();
    mVelocity.x = vNose.x * fMin;
    mVelocity.y = vNose.y * fMin;
    mVelocity.z = vNose.z * fMin;
    mSetSpeed = GetFlyerMinSpeed();
    mSetStrafe = 0.0f;
    mSetStrafeVertical = 0.0f;
    TurnOnThrustEffects();
    mState = FLYING;
    mWorldRoll = 0.0f;
    mAutoLevelingAllowed = 15.0f;
    mFlightRatio = 1.0f;
    mCrashTimer = 0.0f;
}

// FUNCTION: SWBF2 0x004abc20
void EntityFlyer::TurnOffThrustEffects()
{
    for (int i = 0; i < 8; ++i)
    {
        if (!mClass->mThrustEffectName[i])
        {
            return;
        }
        if (mThrustEffect[i])
        {
            mThrustEffect[i]->DeactivateEffect();
            mThrustEffect[i] = 0;
        }
    }
}

// FUNCTION: SWBF2 0x004b10a0
void EntityFlyer::SideRoll()
{
    const float fTwoPi = 6.2831855f;
    if (0.0f > mSideRoll.x)
    {
        RollAdd(fTwoPi * -1.1f);
    }
    else
    {
        RollAdd(fTwoPi * 1.1f);
    }
}

// FUNCTION: SWBF2 0x004b11b0
void EntityFlyer::RollAdd(float fRoll)
{
    mRollTarget += fRoll;
    mInRoll = true;
}

// FUNCTION: SWBF2 0x004b1530
void EntityFlyer::FlipAdd(float fFlip)
{
    mFlipTarget += fFlip;
    mInFlip = true;
}

// FUNCTION: SWBF2 0x004b18f0
void EntityFlyer::DoTrick(Trick eTrick)
{
    if (mClass->mEnergyTrickDrainSingleTap == 0.0f)
    {
        return;
    }
    if (mClass->mEnergyTrickDrainDoubleTap == 0.0f)
    {
        eTrick = SIDE_ROLL;
    }
    ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_FlyerStunt, 0);
    mTrick = 0.0f;
    mControlCrouch.ClearUntilPressed();
    mControlSpecialMove.ClearUntilPressed();
    float fEnergy = eTrick == SIDE_ROLL ? mClass->mEnergyTrickDrainSingleTap : mClass->mEnergyTrickDrainDoubleTap;
    mTrickDoubleclick = 0.0f;
    if (!mInRoll && !mInFlip && mEnergyBar.SpendEnergy(fEnergy))
    {
        mSoundTrick.Stop(true);
        const GameSound* pSound = &mClass->mSoundFlip;
        switch (eTrick)
        {
        case SIDE_ROLL:
            SideRoll();
            pSound = &mClass->mSoundTrick;
            break;
        case FLIP_UP:
            FlipAdd(-3.1415927f);
            break;
        case FLIP_DOWN:
            FlipAdd(3.1415927f);
            break;
        case FLIP_UP_RIGHT:
            FlipAdd(-3.1415927f);
            if (mGetSpeedSpeed >= (GetFlyerMinSpeed() + GetFlyerMidSpeed()) * 0.5f)
            {
                RollAdd(10.210176f);
            }
            else
            {
                RollAdd(3.926991f);
            }
            break;
        case FLIP_DOWN_RIGHT:
            FlipAdd(3.1415927f);
            if (mGetSpeedSpeed >= (GetFlyerMinSpeed() + GetFlyerMidSpeed()) * 0.5f)
            {
                RollAdd(-10.210176f);
            }
            else
            {
                RollAdd(-3.926991f);
            }
            break;
        case FLIP_UP_LEFT:
            FlipAdd(-3.1415927f);
            if (mGetSpeedSpeed >= (GetFlyerMinSpeed() + GetFlyerMidSpeed()) * 0.5f)
            {
                RollAdd(-10.210176f);
            }
            else
            {
                RollAdd(-3.926991f);
            }
            break;
        case FLIP_DOWN_LEFT:
            FlipAdd(3.1415927f);
            if (mGetSpeedSpeed >= (GetFlyerMinSpeed() + GetFlyerMidSpeed()) * 0.5f)
            {
                RollAdd(10.210176f);
            }
            else
            {
                RollAdd(3.926991f);
            }
            break;
        default:
            break;
        }
        const PblMatrix& mMine = GetMatrix();
        mSoundTrick.Play(pSound, &mMine.GetTranslation(), &GetVelocity(), true);
        mLastTrickPerformed = eTrick;
        if (mCtrl)
        {
            mCtrl->IncreaseTargetBubbleSize(AI::TBI_FLYERTRICK);
        }
        return;
    }
    mTrick = -1.0f;
    mSideRoll.x = 0.0f;
    mSideRoll.y = 0.0f;
    mSideRoll.z = 0.0f;
    if (mPlayerId >= 0)
    {
        ToolTipsDisplay::sInstance->ShowTip(TTT_FlyerNoEnergy, 0);
    }
}

// FUNCTION: SWBF2 0x004b1b90
void EntityFlyer::TargetReact(Controllable* pAttacker)
{
    unsigned int uTeam = GetAllegiance();
    if (!pAttacker || !pAttacker->mCharacter || uTeam == (unsigned int)pAttacker->mCharacter->mTeamNumber ||
        mbQueuePoint || mbGoMinSpeed)
    {
        return;
    }
    const PblVector3& vTheirs = pAttacker->GetMatrix().GetTranslation();
    const PblVector3& vMine = mMatrix.GetTranslation();
    float fDx = vTheirs.x - vMine.x;
    float fDy = vTheirs.y - vMine.y;
    float fDz = vTheirs.z - vMine.z;
    float fDistSq = fDy * fDy + fDx * fDx + fDz * fDz;
    if (fDistSq > 490000.0f)
    {
        return;
    }
    if (mCharacter && mCharacter->mCarriedFlagPtr)
    {
        return;
    }
    int iRoll = PblRandom::GetGlobalInt() % 100;
    if (fDistSq > 90000.0f)
    {
        int iTurn = (int)(AIDifficulty::GetFlyerTurnAndAttackFrequency() * 100.0f);
        int iEvade = (int)(AIDifficulty::GetFlyerEvasiveFrequency() * 100.0f);
        if (iRoll < iTurn)
        {
            if (mProcedurallyAnimated == 0)
            {
                mAIEng.ai.mFreeFlyTime = 0.0f;
                mAIEng.ai.mTrickTime = 0.0f;
            }
            mAttacker = pAttacker->mCharacter;
            if (UnitControllerPtr(mCtrl))
            {
                reinterpret_cast<AI::UnitThreatManager*>(UnitControllerPtr(mCtrl)->mThreatManager)->AddThreat(
                           static_cast<Trackable*>(pAttacker)->GetGameObject());
            }
            return;
        }
        if (iRoll >= iTurn + iEvade)
        {
            return;
        }
    }
    else if (!((float)(int)AIDifficulty::GetFlyerEvasiveFrequency() * 100.0f > (float)iRoll))
    {
        return;
    }
    if (mProcedurallyAnimated == 0)
    {
        mAIEng.ai.mFreeFlyTime = 0.0f;
        mAIEng.ai.mTrickTime = 0.0f;
    }
    if (0.0001f > mStrafeTimer)
    {
        mStrafeTimer = 6.2831855f * 2.0f;
    }
}

// FUNCTION: SWBF2 0x004b2e60
static float sGetHeight(const PblVector3& vPos, const PblVector3& vDir, float fDist, float fAngle)
{
    if (!RedTerrain::_bEnable)
    {
        return AIUtil::gMinFlyHeight;
    }
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    float fX = vPos.x + (vDir.x * fCos - vDir.z * fSin) * fDist;
    float fZ = vPos.z + (vDir.z * fCos + vDir.x * fSin) * fDist;
    float fHeight = RedTerrain::GetInterpolatedHeight(fX, fZ);
    return fHeight > AIUtil::gMinFlyHeight ? fHeight : AIUtil::gMinFlyHeight;
}

// FUNCTION: SWBF2 0x004b2f20
void EntityFlyer::RaiseHeight(const PblVector3& vPos, bool bTaper, PblVector3* pDest, float* pfSpeedScale,
                              float fTaperFactor)
{
    PblVector3 vNose = GetMatrix().GetZAxis();
    float fDx = pDest->x - vPos.x;
    float fDy = pDest->y - vPos.y;
    float fDz = pDest->z - vPos.z;
    float fDist = sqrtf(fDy * fDy + fDx * fDx + fDz * fDz);
    float fLook = mSetSpeed * 1.5f;
    if (!(fLook > 1.0f))
    {
        fLook = 1.0f;
    }

    float fHeight = sGetHeight(vPos, vNose, fLook, 0.0f);
    float fH1 = sGetHeight(vPos, vNose, fLook * 0.7f, 0.7853982f);
    float fH2 = sGetHeight(vPos, vNose, fLook * 0.7f, -0.7853982f);
    float fH3 = sGetHeight(vPos, vNose, fLook * 0.5f, 1.5707964f);
    float fH4 = sGetHeight(vPos, vNose, fLook * 0.5f, -1.5707964f);
    float fMax = fH3 > fH4 ? fH3 : fH4;
    fMax = fH2 > fMax ? fH2 : fMax;
    fMax = fH1 > fMax ? fH1 : fMax;
    if (!(fHeight > fMax))
    {
        fHeight = fMax;
    }

    CollisionObject* pHit = 0;
    const GameObject* pIgnore = this;
    PblVector3 vStart = GetMatrix().GetTranslation();
    vStart.y -= GetCollisionRadius() * 1.5f;
    float fHit = CollisionManager::RayHit(vStart, vNose, fLook, pHit, 0, &pIgnore, 1, 0x18, false);
    if (1.0f > fHit)
    {
        float fTop = pHit->GetCollisionCenter().y + pHit->GetCollisionRadius();
        float fMyY = GetMatrix().GetTranslation().y;
        if (fTop - fMyY > 0.0f)
        {
            fTop = (fTop - fMyY) * fLook / (fHit * fLook) + fMyY;
        }
        if (!(fHeight > fTop))
        {
            fHeight = fTop;
        }
        *pfSpeedScale = AIUtil::GetTargetSpeedForRealSpeed(20.0f, GetFlyerMaxSpeed());
    }

    float fGround = RedTerrain::_bEnable ? RedTerrain::GetInterpolatedHeight(vPos.x, vPos.z) : -FLT_MAX;
    float fWater;
    if (RedWater::GetWaterHeight(vPos, fWater) && fWater > fGround)
    {
        fGround = fWater;
    }
    if (!(fGround > fHeight))
    {
        fGround = fHeight;
    }

    float fHoriz = sqrtf(fDz * fDz + fDx * fDx);
    float fY = fHoriz * (fGround + 25.0f - vPos.y) / fLook + vPos.y;
    if (bTaper)
    {
        float fT = fTaperFactor;
        if (!(fTaperFactor >= 0.0f))
        {
            float fRange = mSetSpeed / mClass->mTurnRate * 2.0f;
            if (!(fRange > 2.0f))
            {
                fRange = 2.0f;
            }
            float fR = fDist;
            if (1.0f > fR)
            {
                fR = 1.0f;
            }
            else if (fR > fRange)
            {
                fR = fRange;
            }
            fT = (fR - 1.0f) / (fRange - 1.0f);
        }
        fY = (fY - pDest->y) * fT + pDest->y;
    }
    if (fY > pDest->y)
    {
        pDest->y = fY;
    }
}

// FUNCTION: SWBF2 0x004b3310
bool EntityFlyer::AtSlideDest() const
{
    float fDirX = mSlidePos.x - mSlideBeginPos.x;
    float fDirZ = mSlidePos.z - mSlideBeginPos.z;
    const PblVector3& vPos = GetMatrix().GetTranslation();
    return (vPos.x - mSlidePos.x) * fDirX + 0.0f + (vPos.z - mSlidePos.z) * fDirZ > 0.0f;
}

// FUNCTION: SWBF2 0x004b3390
bool EntityFlyer::CalculateDest(float fDeltaTime, ControllableDestination* pDest, PblVector3* pPos,
                                float* pfSpeedScale, PblVector3* pDir)
{
    AILowLevel& lowLevel = UnitControllerPtr(mCtrl)->GetLowLevel();
    *pfSpeedScale = lowLevel.mDest.mMaxSpeed;
    PblVector3* pPosition = pDir;
    *pPosition = GetMatrix().GetTranslation();

    if (GameLoop::GetMissionTime() > mNextBarrierCheck + 1.0f || lowLevel.mFlyerIgnoreBarriers)
    {
        mSliding = false;
    }
    if (mSliding)
    {
        if (AtSlideDest())
        {
            mSliding = false;
        }
        else
        {
            pDest->mHasDest = true;
            pDest->mPos = mSlidePos;
            pDest->mOriginalPos = mSlidePos;
            PblVector3 vDelta(mSlidePos.x - pPosition->x, mSlidePos.y - pPosition->y, mSlidePos.z - pPosition->z);
            pDest->mDirection = AIUtil::GetSafeNormal(vDelta);
            pDest->mSafeHeightEnforce = SH_FULL;
        }
    }
    if (!mSliding)
    {
        if (lowLevel.IsAtDest() && lowLevel.mLandAtDest)
        {
            Land();
            return false;
        }
        *pDest = lowLevel.mDest;
    }
    *pPos = pDest->mPos;

    if (!lowLevel.HasPath() || !lowLevel.mFlyerIgnoreBarriers)
    {
        float fTime = GameLoop::GetMissionTime();
        if (fTime > mNextBarrierCheck)
        {
            mNextBarrierCheck = fTime + 0.1f;
            if (AIUtil::AvoidBarriers(this, *pPos, pPos))
            {
                mSliding = true;
                mSlidePos = *pPos;
                mSlideBeginPos = *pPosition;
            }
        }
    }

    if (!lowLevel.HasPath() || !lowLevel.mFlyerIgnoreSafeHeight || pDest->mSafeHeightEnforce == SH_FULL ||
        pDest->mSafeHeightEnforce == SH_TAPER || pDest->mSafeHeightEnforce == SH_TAPER_FACTOR)
    {
        float fTaperFactor = pDest->mSafeHeightEnforce == SH_TAPER_FACTOR ? pDest->mTaperFactor : -1.0f;
        bool bTaper = pDest->mSafeHeightEnforce == SH_TAPER || pDest->mSafeHeightEnforce == SH_TAPER_FACTOR;
        RaiseHeight(*pPosition, bTaper, pPos, pfSpeedScale, fTaperFactor);
        if (pPos->y > AIUtil::gMaxFlyHeight)
        {
            pPos->y = AIUtil::gMaxFlyHeight;
        }
    }

    PblVector3 vDelta(pPos->x - pPosition->x, pPos->y - pPosition->y, pPos->z - pPosition->z);
    PblVector3 vToDest;
    D3DXVec3Normalize(&vToDest, &vDelta);
    float fScale = *pfSpeedScale;
    const PblVector3& vNose = GetMatrix().GetZAxis();
    float fAhead = vNose.x * vToDest.x + vNose.y * vToDest.y + vNose.z * vToDest.z;
    if (!(fAhead > 0.9f))
    {
        fAhead = 0.9f;
    }
    *pfSpeedScale = (fAhead - 0.9f) / (1.0f - 0.9f) * *pfSpeedScale;

    float fDx = pPos->x - pPosition->x;
    float fDy = pPos->y - pPosition->y;
    float fDz = pPos->z - pPosition->z;
    float fDist = sqrtf(fDy * fDy + fDx * fDx + fDz * fDz);
    float fMax = GetFlyerMaxSpeed();
    float fFar = fMax * 3.0f;
    if (fMax > fDist)
    {
        fDist = fMax;
    }
    else if (fDist > fFar)
    {
        fDist = fFar;
    }
    float fNear = (fDist - fMax) / (fFar - fMax) * 0.7f + 0.3f;

    const PblVector3& vNose2 = GetMatrix().GetZAxis();
    float fAlign = vNose2.x * pDest->mDirection.x + vNose2.y * pDest->mDirection.y + vNose2.z * pDest->mDirection.z;
    if (!(fAlign > 0.5f))
    {
        fAlign = 0.5f;
    }
    float fSpeed = ((fAlign * 2.0f - 1.0f) * (1.0f - fNear) + fNear) * *pfSpeedScale;
    *pfSpeedScale = fSpeed;
    float fFloor = AIUtil::GetTargetSpeedForRealSpeed(20.0f, GetFlyerMaxSpeed());
    if (fScale > fSpeed)
    {
        *pfSpeedScale = fSpeed > fFloor ? fSpeed : fFloor;
    }
    return true;
}

// FUNCTION: SWBF2 0x004b3d50
void EntityFlyer::Land()
{
    if (mState == FLYING || mState == TAKEOFF || mState == LANDING)
    {
        mState = LANDING;
    }
    if (!IsWindingDown() || IsWindingUp())
    {
        WindDown(mClass, mMatrix.GetTranslation(), mVelocity);
    }
    mLastAltitude = 0.0f;
    GetPosition();
}

// FUNCTION: SWBF2 0x004b5b10
void EntityFlyer::SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix)
{
    if (NetGame::IsEnabled())
    {
        float fMaxSpeed = mClass->mMaxFlyerSpeed;
        if (fMaxSpeed < mClass->mBoostFlyerSpeed)
        {
            fMaxSpeed = mClass->mBoostFlyerSpeed;
        }
        if (mPlayerId >= 0 && mGetSpeedSpeed > mClass->mBlurStartV)
        {
            float fRatio = (mGetSpeedSpeed - mClass->mBlurStartV) /
                           (fMaxSpeed - mClass->mBlurStartV);
            if (fRatio < 0.0f)
            {
                fRatio = 0.0f;
            }
            else if (fRatio > 1.0f)
            {
                fRatio = 1.0f;
            }
            const float fBlur = mClass->mBlurEffect * fRatio;
            if (fBlur > 0.0f)
            {
                MotionBlurEffect::Start(0, fBlur, CameraManager::Instance()->mUpdatedCameraId);
            }
        }
    }
    if (IsAlive() && (mTargetInfo.mIsAiming || (mTracker && mTracker->IsFirstPersonView())))
    {
        const int iFirstPerson = GetFirstPersonIndex();
        if (iFirstPerson >= 0 && !mTargetInfo.mIsAiming)
        {
            FirstPerson::UpdateAimer(iFirstPerson, rMatrix);
        }
    }
}

#ifdef GetObject
#undef GetObject
#endif
// FUNCTION: SWBF2 0x00428e40
template ParticleEmitterObject* PblHandle<ParticleEmitterObject>::GetObject() const;

// SYNTHETIC: SWBF2 0x0047852f
// [thunk]:CommandFlyer::`scalar deleting destructor'`adjustor{320}'
// SYNTHETIC: SWBF2 0x0047853a
// [thunk]:CommandFlyer::`scalar deleting destructor'`adjustor{972}'
// SYNTHETIC: SWBF2 0x00478545
// [thunk]:CommandFlyer::`scalar deleting destructor'`adjustor{600}'
// SYNTHETIC: SWBF2 0x00478550
// [thunk]:CommandFlyer::`scalar deleting destructor'`adjustor{12}'
// SYNTHETIC: SWBF2 0x00478558
// [thunk]:CommandFlyer::`scalar deleting destructor'`adjustor{512}'
// SYNTHETIC: SWBF2 0x00478563
// [thunk]:CommandFlyer::`scalar deleting destructor'`adjustor{576}'
// SYNTHETIC: SWBF2 0x0047859a
// [thunk]:CommandFlyer::`scalar deleting destructor'`adjustor{148}'
// SYNTHETIC: SWBF2 0x00478607
// [thunk]:CommandFlyer::Kill`adjustor{256}'
// SYNTHETIC: SWBF2 0x004b9937
// [thunk]:EntityFlyer::`scalar deleting destructor'`adjustor{512}'
// SYNTHETIC: SWBF2 0x004b994d
// [thunk]:EntityFlyer::`scalar deleting destructor'`adjustor{148}'
// SYNTHETIC: SWBF2 0x004b9958
// [thunk]:EntityFlyer::Kill`adjustor{256}'
// SYNTHETIC: SWBF2 0x004b9963
// [thunk]:EntityFlyer::`scalar deleting destructor'`adjustor{320}'
// SYNTHETIC: SWBF2 0x004b996e
// [thunk]:EntityFlyer::`scalar deleting destructor'`adjustor{972}'
// SYNTHETIC: SWBF2 0x004b9979
// [thunk]:EntityFlyer::`scalar deleting destructor'`adjustor{600}'
// SYNTHETIC: SWBF2 0x004b9984
// [thunk]:EntityFlyer::`scalar deleting destructor'`adjustor{12}'

// GLOBAL: SWBF2 0x007e6323
bool bRenderContrails = true;

// SYNTHETIC: SWBF2 0x00402af0
// `dynamic initializer for 'cvRenderContrails''
RedCommandConsole cvRenderContrails("flyer.contrailsActive", MultiVar(&bRenderContrails));
