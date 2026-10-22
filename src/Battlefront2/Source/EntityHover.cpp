#include <float.h>
#include "PebbleFL/Common/PblVector.h"

#include "EntityHover.h"
#include "EnergyBar.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "CollisionObject.h"
#include "GameLoop.h"
#include "GameModel.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "CollisionBody.h"
#include "CollisionManager.h"
#include "CollisionMesh.h"
#include "CollisionPrimitive.h"
#include "Explosion.h"
#include "FirstPerson.h"
#include "MountedTurret.h"
#include "NetGame.h"
#include "PassengerSlot.h"
#include "PathFinder.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include <new>
#include <string.h>
#include "FLEffect.h"
#include "TargetManager.h"
#include "VehicleManager.h"
#include "WeaponCannonInstance.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"

#include "PlayerController.h"
#include "Tracker.h"
#include "CameraManager.h"
#include "MotionBlurEffect.h"
#include "UnitMobileController.h"
#include "AILowLevel.h"
#include "Character.h"
#include "LockOnManager.h"
#include "MedalsMgr.h"
#include "Team.h"
#include "ToolTips.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedEngineFL/Application/RedWater.h"

#include "AIUtil.h"
#include "ConnectivityGraph.h"
#include "EntitySoldier.h"
#include "EntityWalker.h"
#include "Restrictor.h"
#include "StatusDisplay.h"
#include "UnitCmdInfo.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedShadingState.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);

namespace
{

    void UpdateCollisionSphere(EntityHover* pHover)
    {
        PblVector3 center;
        D3DXVec3TransformCoord(&center, &pHover->mModel->_mModel->_BoundingSphere.GetCenter(),
                               &pHover->mMatrix);
        pHover->SetCollisionCenter(center);
        pHover->RecalculateAABB();
    }
}

// GLOBAL: SWBF2 0x01ebbdd0
// SYNTHETIC: SWBF2 0x00402ba0
// `dynamic initializer for 'EntityHover::rttiHashEntityHover''
PblHash EntityHover::rttiHashEntityHover("EntityHover");
char EntityHover::rttiNameEntityHover[] = "EntityHover";

// GLOBAL: SWBF2 0x01ebbdd8
// SYNTHETIC: SWBF2 0x00402b80
// `dynamic initializer for 'EntityHover::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f60
// `dynamic atexit destructor for 'EntityHover::sMemoryPool''
MemoryPool EntityHover::sMemoryPool("EntityHover", 0x1d10);

// GLOBAL: SWBF2 0x01ebbdd4
// SYNTHETIC: SWBF2 0x00402bb0
// `dynamic initializer for 'EntityHoverClass::rttiHashEntityHoverClass''
PblHash EntityHoverClass::rttiHashEntityHoverClass("EntityHoverClass");
char EntityHoverClass::rttiNameEntityHoverClass[] = "EntityHoverClass";

// FUNCTION: SWBF2 0x00478a50
bool EntityHover::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityHover || GameObject::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00478a90
GameObject* EntityHover::GetGameObject()
{
    return this;
}

// FUNCTION: SWBF2 0x00478aa0
const GameObject* EntityHover::GetGameObject() const
{
    return this;
}

// FUNCTION: SWBF2 0x00478ab0
Entity* EntityHover::GetEntity()
{
    return this;
}

// FUNCTION: SWBF2 0x00478ac0
const Entity* EntityHover::GetEntity() const
{
    return this;
}

// FUNCTION: SWBF2 0x00478ad0
const PblMatrix& EntityHover::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x00478ae0
const PblVector3& EntityHover::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x00478af0
void EntityHover::SetVelocity(const PblVector3& vVelocity)
{
    mVelocity = vVelocity;
}

// FUNCTION: SWBF2 0x00478b10
void EntityHover::SetPosition(const PblVector3& vPosition)
{
    mMatrix.m[3][0] = vPosition.x;
    mMatrix.m[3][1] = vPosition.y;
    mMatrix.m[3][2] = vPosition.z;
    mMatrix.m[3][3] = 1.0f;
    UpdateCollisionSphere(this);
}

// FUNCTION: SWBF2 0x00478bb0
void EntityHover::SetMatrix(const PblMatrix& matrix)
{
    mMatrix = matrix;
    UpdateCollisionSphere(this);
}

// FUNCTION: SWBF2 0x00478c60
Trackable* EntityHover::GetTrackable()
{
    return this;
}

// FUNCTION: SWBF2 0x00478c80
const Controllable* EntityHover::GetControllable() const
{
    return this;
}

// FUNCTION: SWBF2 0x00478c90
Controllable* EntityHover::GetControllable()
{
    return this;
}

// FUNCTION: SWBF2 0x00478ca0
int EntityHover::GetWeaponIndex(int iChannel) const
{
    return mWeaponIndex[iChannel];
}

// FUNCTION: SWBF2 0x00478cc0
Weapon* EntityHover::GetWeaponPtr(int iIndex) const
{
    return mWeapon[iIndex];
}

// FUNCTION: SWBF2 0x00478ce0
const Aimer* EntityHover::GetAimer(int iIndex) const
{
    return mAimer[iIndex];
}

// FUNCTION: SWBF2 0x00478d00
GameObjectType EntityHover::GetGameObjectType()
{
    return (GameObjectType)(GO_VEHICLE | GO_HOVER);
}

// FUNCTION: SWBF2 0x00478d10
const RedPose* EntityHover::GetPose() const
{
    return &mPose;
}

// FUNCTION: SWBF2 0x00478d20
void EntityHover::ReserveForPlayer(float fSeconds)
{
    mReserveForPlayer = GameLoop::GetMissionTime() + fSeconds;
}

// FUNCTION: SWBF2 0x00478d40
bool EntityHover::IsReservedForPlayer() const
{
    return mReserveForPlayer > GameLoop::GetMissionTime();
}

// FUNCTION: SWBF2 0x00478d60
EnergyBar* EntityHover::GetEnergyBar()
{
    return (EnergyBar*)mEnergyBar;
}

// FUNCTION: SWBF2 0x00478d70
void EntityHover::SetEjectTimer(float fSeconds)
{
    mEjectTimer = fSeconds;
}

// FUNCTION: SWBF2 0x00478d90
float EntityHover::GetEjectTimer() const
{
    return mEjectTimer;
}

// FUNCTION: SWBF2 0x00478da0
void EntityHover::ResetTimeSinceLastHack()
{
    mTimeSinceLastHack = 0.0f;
}

// FUNCTION: SWBF2 0x00478db0
bool EntityHover::IsForcedThirdPerson() const
{
    return true;
}

// FUNCTION: SWBF2 0x00478dc0
bool EntityHoverClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityHoverClass || GameObjectClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00478e00
int EntityHoverClass::GetVehicleType()
{
    return mVehicleType;
}

// FUNCTION: SWBF2 0x00478e10
int EntityHoverClass::GetWeaponCount()
{
    return mWeaponCount;
}

// FUNCTION: SWBF2 0x00478e20
const WeaponClass* EntityHoverClass::GetWeaponClass(int iIndex)
{
    return mWeaponClass[iIndex];
}

// FUNCTION: SWBF2 0x00478e40
float EntityHoverClass::GetTimeRequiredToEject()
{
    return mTimeRequiredToEject;
}

// FUNCTION: SWBF2 0x00478e50
float EntityHoverClass::GetReboardTime()
{
    return mTimeTilReboard;
}

// FUNCTION: SWBF2 0x00478e60
const void* EntityHoverClass::GetControllableClass()
{
    return this ? static_cast<const ControllableClass*>(this) : 0;
}

// FUNCTION: SWBF2 0x004c0920
unsigned int EntityHover::GetDerivedRtti() const
{
    return rttiHashEntityHover;
}

// FUNCTION: SWBF2 0x004c0930
const char* EntityHover::GetDerivedRttiName() const
{
    return rttiNameEntityHover;
}

// FUNCTION: SWBF2 0x004c0950
EntityType EntityHover::GetEntityType() const
{
    return ENT_HOVER;
}

// FUNCTION: SWBF2 0x004c0960
bool EntityHover::PostCollision::Update(float fDeltaTime)
{
    return mOwner->PostCollisionUpdate(fDeltaTime);
}

// FUNCTION: SWBF2 0x004c09b0
EntityHover::PostCollision::~PostCollision()
{
}

// SYNTHETIC: SWBF2 0x004c09c0
// EntityHover::PostCollision::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004c09f0
unsigned int EntityHoverClass::GetDerivedRtti()
{
    return rttiHashEntityHoverClass;
}

// FUNCTION: SWBF2 0x004c0a00
const char* EntityHoverClass::GetDerivedRttiName()
{
    return rttiNameEntityHoverClass;
}

// GLOBAL: SWBF2 0x007e6514
static int sCurrentSection = -1;
// GLOBAL: SWBF2 0x007e6518
static float sBodySpringLengthRadiusMultiplier = 0.5f;
// GLOBAL: SWBF2 0x01e51dcc
static int sWeaponSection;
// GLOBAL: SWBF2 0x01e51dd0
static int sWheelSection;
// GLOBAL: SWBF2 0x01e3075f
static bool sPassengerSlotSection;
// GLOBAL: SWBF2 0x01e51ddc
static unsigned int sSoftCollisionMask;
// GLOBAL: SWBF2 0x01e51dc8
static unsigned int sOrdnanceCollisionMask;
// GLOBAL: SWBF2 0x01e51dd8
static EntityHoverClass* sDbgHov;

// FUNCTION: SWBF2 0x004c81c0
EntityHoverClass::EntityHoverClass(unsigned int uHash)
    : GameObjectClass(uHash)
{
    m_bDynamicLighting = true;
    mHealthType = HEALTH_TYPE_VEHICLE;
    mPilotType = PILOT_VEHICLE;
    mLODClass = 1;
    mHoldForPlayer = true;
    mCollisionModel.mDenyFlyerLand = true;

    mSetAltitude = 1.0f;
    mGravityScale = 1.0f;
    mLiftSpring = 5.0f;
    mLiftDamp = 2.0f;
    mAcceleration = 10.0f;
    mDeceleration = 5.0f;
    mTraction = 10.0f;
    mForwardSpeed = 20.0f;
    mReverseSpeed = 10.0f;
    mStrafeSpeed = 15.0f;
    mSpinRate = 3.0f;
    mTurnRate = 2.0f;
    mYawRate = 1.0f;
    mPitchRate = 0.5f;
    mTurnFilter = 10.0f;
    mYawMin = 0.0f;
    mYawMax = 0.0f;
    mPitchMin = -0.5f;
    mPitchMax = 0.5f;
    mThrustPitchAngle = 0.0f;
    mStrafeRollAngle = 0.0f;
    mBankAngle = 0.0f;
    mBankFilter = 2.0f;
    mLevelSpring = 10.0f;
    mLevelDamp = 3.0f;
    mCollisionThreshold[0] = 7.5f;
    mCollisionThreshold[1] = 2.5f;
    mCollisionThreshold[2] = 0.0f;
    mCollisionScale[0] = 1.0f;
    mCollisionScale[1] = 0.0f;
    mCollisionScale[2] = 0.0f;
    mCollisionInflict = 1.0f;

    sCurrentSection = -1;
    sWeaponSection = 0;
    sWheelSection = 0;

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
    mAnimObj = 0;
    mAnimFins = 0;
    mNoiseFactor = 1.0f;
    mBarrierX = -1.0f;
    mBarrierZ = -1.0f;
    mBarrierOX = 0.0f;
    mBarrierOZ = 0.0f;

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
    mAttachHash = 0;
    mAttachOffset.x = 0.0f;
    mAttachOffset.y = 0.0f;
    mAttachOffset.z = 0.0f;
    mWakeEffect = 0;
    mWaterEffect = 0;
    mBlurEffect = 0.0f;
    mBlurStartV = 1.0f;

    for (int i = 0; i < 6; ++i)
    {
        mWakeEmitterPrimitives[i] = -1;
    }

    mNumWakeEmitters = 0;
    mSoundCloseToGroundHeight = 0.9f;

    mMaxSuspensionOffset = 0.0f;
    mMidSuspensionOffset = 0.0f;
    mSuspensionCenterHash = 0;
    mSuspensionLeftArmHash = 0;
    mSuspensionRightArmHash = 0;

    mFirstPersonModelHandle = -1;
    mFPModelAttachedtoCam = true;
    mMovingTurnOnly = false;
    mFloatsOnWater = true;
    mRandomSpring = true;
    mNumColliderBodies = 0;

    mEnergyBarClass.m_fEnergyMax = 0.0f;
    mEnergyBarClass.m_fEnergyMin = 0.0f;
    mEnergyBarClass.m_fEnergyOverheat = 0.0f;
    mEnergyBarClass.Init(100.0f, 20.0f);

    mEnergyAutoRestore = 0.0f;
    mEnergyBoostDrain = 0.0f;
    mBoostSpeed = 0.0f;
    mBoostAcceleration = 0.0f;
    mBoostFOV = 0.0f;
    mBoostTurnMultiplier = 0.5f;
    mTimeRequiredToEject = 3.0f;
    mEjectResist = 0.1f;
    mTimeTilReboard = 5.0f;
    mJumpTimeMin = 0.0f;
    mJumpTimeMax = 0.0f;
    mJumpForce = 0.0f;
    mJumpMinSpeedMult = 0.3f;
    mJumpEnergyPerSec = 0.0f;
    mWheelCount = 0;

    for (int i = 0; i < 4; ++i)
    {
        mWheelTextureId[i] = 0;
        mWheelVelocToU[i] = 0.0f;
        mWheelVelocToV[i] = 0.0f;
        mWheelOmegaToU[i] = 0.0f;
        mWheelOmegaToV[i] = 0.0f;
    }
}

// FUNCTION: SWBF2 0x004ca820
EntityClass* EntityHoverClass::Derive(unsigned int uHash)
{
    return new EntityHoverClass(this, uHash);
}

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

void* EntityHover::operator new(unsigned int uSize)
{
    return sMemoryPool.Allocate(uSize);
}

void EntityHover::operator delete(void* pBlock)
{
    sMemoryPool.Free(pBlock);
}

// FUNCTION: SWBF2 0x004c0a10
EntityHover::EntityHover(EntityHoverClass* pClass, const EntityDesc& desc)
    : GameObject(pClass, desc, COLL_RIGID, &pClass->mModel->_mLodData)
    , Controllable(pClass, desc.mPlayerId)
    , VehicleMusic(pClass)
    , VehicleEngine(pClass)
    , mObstacle(0)
    , mPostCollision(this)
{
    new (mEnergyBar) EnergyBar(&pClass->mEnergyBarClass);
    mBoost = false;

    mControllableList.AddLast(&this->Controllable::mNode, this);

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

    PblVector3 vLift(0.0f, 0.0f, 0.0f);
    vLift.y += mClass->mSetAltitude;
    const float fX = mMatrix.m[3][0] + vLift.x;
    const float fY = mMatrix.m[3][1] + vLift.y;
    const float fZ = mMatrix.m[3][2] + vLift.z;
    mMatrix.m[3][0] = fX;
    mMatrix.m[3][1] = fY;
    mMatrix.m[3][2] = fZ;
    mMatrix.m[3][3] = 1.0f;
    AddToScene();

    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    mOmega.x = 0.0f;
    mOmega.y = 0.0f;
    mOmega.z = 0.0f;
    mGroundRatio = 1.0f;
    mBankPitchAngle = 0.0f;
    mBankRollAngle = 0.0f;
    mFinPitchValue = 0.0f;
    mFinRollValue = 0.0f;
    mYawValue = 0.0f;
    mPitchValue = 0.0f;
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

    TargetManager::AddTarget(this, true);
    ClearControllable();
    VehicleManager::Add(this);
    mEyeDir = mMatrix.GetZAxis();
    mEyePoint = mClass->mCameraSettings.CalcEyePoint(mMatrix);
    _uiRenderFlags &= 3;
    for (int i = 0; i < 4; ++i)
    {
        mWheelTextureU[i] = 0.0f;
    }
    for (int i = 0; i < 4; ++i)
    {
        mWheelTextureV[i] = 0.0f;
    }
    memset(mWakeEmitters, 0, sizeof(mWakeEmitters));
    mSpeedPrev = 0.0f;
    mGroundRatioPrev = 1.0f;
    mCloseToGround = 0;
    mStrain = 0.0f;
    mAccel = 0.0f;

    if (mClass->mAnimObj)
    {
        ZephyrPoseStatic<32> kPoseFinal;
        mZephyrSkeleton.Open(*mClass->mAnimObj->GetZephyrSkeletonShared(), kPoseFinal);
        kPoseFinal.Open(mZephyrSkeleton);
        mZephyrPoseDyn.Open(mZephyrSkeleton);
        if (mClass->mAnimFins)
        {
            mZephyrPoseDyn.SetAnimation(mClass->mAnimFins, 30.0f);
            mZephyrPoseDyn.SetLoop(false);
            mZephyrPoseDyn.SetFrameNum(4);
            mZephyrPoseDyn.Update(0.0f);
            kPoseFinal.Set(mZephyrPoseDyn, false);
            mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
            mPose.ConvertFromZephyrPose(mZephyrSkeleton);
        }
    }
    mModel->GetRedModel(0)->SetupPose(mPose);

    mForceFrameNum1 = -1.0f;
    mForceFrameNum2 = -1.0f;
    mForceFrameBlend = -1.0f;
    mCurrSuspensionOffset = mClass->mMidSuspensionOffset;
    mNextSuspensionOffset = mClass->mMidSuspensionOffset;
    mLastSuspensionOffset = mClass->mMidSuspensionOffset;
    mSuspensionVelocity = 0.0f;
    mStuckState = 0;
    mStuckCycle = -100.0f;
    mWaterEmitter = NULL;
    mIsDead = false;
    mReserveForPlayer = 0.0f;
    mGroundPitch = 0.0f;
    mGroundBank = 0.0f;
    mGroundPitchVel = 0.0f;
    mGroundBankVel = 0.0f;
    for (int i = 0; i < mClass->mNumColliderBodies; ++i)
    {
        mColliderBody[i].mOverlap = 0.0f;
        mColliderBody[i].mOldOverlap = 0.0f;
        mColliderBody[i].mNormal.x = 0.0f;
        mColliderBody[i].mNormal.y = 0.0f;
        mColliderBody[i].mNormal.z = 0.0f;
    }
    mJumpMode = false;
    mEjectTimer = 0.0f;
    mTimeSinceLastHack = 0.0f;
    mJumpTimer = 0.0f;
}

// FUNCTION: SWBF2 0x004c7f60
void EntityHover::ActivatePhysics()
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

// FUNCTION: SWBF2 0x004c1600
void EntityHover::Init()
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

// SYNTHETIC: SWBF2 0x004c8dc0
// EntityHoverClass::~EntityHoverClass

// SYNTHETIC: SWBF2 0x004c86f0
// EntityHoverClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004c8720
EntityHoverClass::EntityHoverClass(EntityHoverClass* pBase, unsigned int uHash)
    : GameObjectClass(pBase, uHash),
      ControllableClass(pBase),
      VehicleMusicClass(pBase),
      VehicleEngineClass(pBase)
{
    mEnergyBarClass.m_fEnergyMax = 0.0f;
    mEnergyBarClass.m_fEnergyMin = 0.0f;
    mEnergyBarClass.m_fEnergyOverheat = 0.0f;

    mSetAltitude = pBase->mSetAltitude;
    mGravityScale = pBase->mGravityScale;
    mLiftSpring = pBase->mLiftSpring;
    mLiftDamp = pBase->mLiftDamp;
    mAcceleration = pBase->mAcceleration;
    mDeceleration = pBase->mDeceleration;
    mTraction = pBase->mTraction;
    mForwardSpeed = pBase->mForwardSpeed;
    mReverseSpeed = pBase->mReverseSpeed;
    mStrafeSpeed = pBase->mStrafeSpeed;
    mBoostSpeed = pBase->mBoostSpeed;
    mBoostAcceleration = pBase->mBoostAcceleration;
    mBoostFOV = pBase->mBoostFOV;
    mBoostTurnMultiplier = pBase->mBoostTurnMultiplier;
    mSpinRate = pBase->mSpinRate;
    mTurnRate = pBase->mTurnRate;
    mTurnFilter = pBase->mTurnFilter;
    mYawRate = pBase->mYawRate;
    mYawMin = pBase->mYawMin;
    mYawMax = pBase->mYawMax;
    mPitchRate = pBase->mPitchRate;
    mPitchMin = pBase->mPitchMin;
    mPitchMax = pBase->mPitchMax;
    mThrustPitchAngle = pBase->mThrustPitchAngle;
    mStrafeRollAngle = pBase->mStrafeRollAngle;

    sCurrentSection = -1;
    sWeaponSection = 0;
    sWheelSection = 0;

    mBankAngle = pBase->mBankAngle;
    mBankFilter = pBase->mBankFilter;
    mLevelSpring = pBase->mLevelSpring;
    mLevelDamp = pBase->mLevelDamp;
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
    mVehicleType = VEHICLE_UNKNOWN;
    mNoiseFactor = pBase->mNoiseFactor;
    mBarrierX = pBase->mBarrierX;
    mBarrierZ = pBase->mBarrierZ;
    mBarrierOX = pBase->mBarrierOX;
    mBarrierOZ = pBase->mBarrierOZ;
    for (int i = 0; i < 4; ++i)
    {
        mAimerClass[i] = pBase->mAimerClass[i];
    }
    mCameraSettings = pBase->mCameraSettings;

    for (int i = 0; i < 8; ++i)
    {
        mTurretClass[i] = pBase->mTurretClass[i];
    }
    mPassengerSlotClass = pBase->mPassengerSlotClass;
    mNumPassengerSlots = pBase->mNumPassengerSlots;
    mExplosion = pBase->mExplosion;
    mAttachHash = pBase->mAttachHash;
    mAttachOffset = pBase->mAttachOffset;
    mWakeEffect = pBase->mWakeEffect;
    mWaterEffect = pBase->mWaterEffect;
    mBlurEffect = pBase->mBlurEffect;
    mBlurStartV = pBase->mBlurStartV;
    mNumWakeEmitters = pBase->mNumWakeEmitters;
    for (int i = 0; i < 6; ++i)
    {
        mWakeEmitterPrimitives[i] = pBase->mWakeEmitterPrimitives[i];
    }
    mLODClass = 1;
    mSoundCloseToGroundHeight = pBase->mSoundCloseToGroundHeight;
    mSoundCloseToGround = pBase->mSoundCloseToGround;

    mAnimObj = pBase->mAnimObj;
    mAnimFins = pBase->mAnimFins;
    mFirstPersonModelHandle = pBase->mFirstPersonModelHandle;
    mMaxSuspensionOffset = pBase->mMaxSuspensionOffset;
    mSuspensionCenterHash = pBase->mSuspensionCenterHash;
    mSuspensionLeftArmHash = pBase->mSuspensionLeftArmHash;
    mSuspensionRightArmHash = pBase->mSuspensionRightArmHash;
    mMovingTurnOnly = pBase->mMovingTurnOnly;
    mFloatsOnWater = pBase->mFloatsOnWater;
    mFPModelAttachedtoCam = pBase->mFPModelAttachedtoCam;

    mVelocitySpring = 15.0f;
    mVelocityDamp = 5.0f;
    mOmegaXSpring = 3.0f;
    mOmegaXDamp = 1.0f;
    mOmegaZSpring = 3.0f;
    mOmegaZDamp = 1.0f;
    mNumColliderBodies = 0;
    mCollisionModel.mDenyFlyerLand = true;
    mRandomSpring = pBase->mRandomSpring;

    mEnergyBarClass = pBase->mEnergyBarClass;
    mEnergyAutoRestore = pBase->mEnergyAutoRestore;
    mEnergyBoostDrain = pBase->mEnergyBoostDrain;
    mTimeRequiredToEject = pBase->mTimeRequiredToEject;
    mEjectResist = pBase->mEjectResist;
    mTimeTilReboard = pBase->mTimeTilReboard;
    mJumpTimeMin = pBase->mJumpTimeMin;
    mJumpTimeMax = pBase->mJumpTimeMax;
    mJumpForce = pBase->mJumpForce;
    mJumpEnergyPerSec = pBase->mJumpEnergyPerSec;
    mJumpMinSpeedMult = pBase->mJumpMinSpeedMult;
    mWheelCount = pBase->mWheelCount;
    for (int i = 0; i < 4; ++i)
    {
        mWheelTextureId[i] = pBase->mWheelTextureId[i];
    }
    for (int i = 0; i < 4; ++i)
    {
        mWheelVelocToU[i] = pBase->mWheelVelocToU[i];
    }
    for (int i = 0; i < 4; ++i)
    {
        mWheelVelocToV[i] = pBase->mWheelVelocToV[i];
    }
    for (int i = 0; i < 4; ++i)
    {
        mWheelOmegaToU[i] = pBase->mWheelOmegaToU[i];
    }
    for (int i = 0; i < 4; ++i)
    {
        mWheelOmegaToV[i] = pBase->mWheelOmegaToV[i];
    }
}

// FUNCTION: SWBF2 0x004ca890
Entity* EntityHoverClass::Build(const EntityDesc& desc)
{
    if (EntityHover::sMemoryPool.IsFull())
    {
        return 0;
    }

    EntityHover* pHover = new EntityHover(this, desc);
    if (pHover)
    {
        NetGame::Created(pHover);
        CollisionManager::AddRigidBody(static_cast<CollisionObject*>(pHover));
        pHover->ActivatePhysics();
        pHover->BuildEffects();
    }
    return pHover;
}

// FUNCTION: SWBF2 0x004ca940
bool EntityHoverClass::InitAnimations(const char* pszName)
{
    mAnimObj = RedAnimation::Find(PblHash(pszName));
    return mAnimObj != 0;
}

// FUNCTION: SWBF2 0x004ca980
void EntityHoverClass::SetupDefaultColliderBodyFactors(ColliderBodyInfo& info, int iBodyId)
{
    info.mCollisionBodyId = iBodyId;
    info.mVelocitySpringFactor = 1.0f;
    info.mVelocityDampFactor = 1.0f;
    info.mOmegaXDampFactor = 1.0f;
    info.mOmegaZDampFactor = 1.0f;

    const CollisionBodyPrimitive* pBody = (const CollisionBodyPrimitive*)mCollisionModel.GetBody(iBodyId);
    PblMatrix matrix = pBody->GetLocalMatrix(0);
    info.mSpringLength = pBody->GetRadius() * sBodySpringLengthRadiusMultiplier;

    const PblVector3& vOffset = matrix.GetTranslation();
    if (vOffset.z > 0.1f)
    {
        info.mOmegaXSpringFactor = -1.0f;
    }
    else if (vOffset.z < -0.1f)
    {
        info.mOmegaXSpringFactor = 1.0f;
    }
    else
    {
        info.mOmegaXSpringFactor = 0.0f;
    }

    if (vOffset.x > 0.1f)
    {
        info.mOmegaZSpringFactor = 1.0f;
    }
    else if (vOffset.x < -0.1f)
    {
        info.mOmegaZSpringFactor = -1.0f;
    }
    else
    {
        info.mOmegaZSpringFactor = 0.0f;
    }
}

namespace
{

void ReadSuspensionNode(const GameModel* pModel, unsigned int uNodeHash, PblVector3& rPos)
{
    RedPose pose;
    pModel->GetRedModel(0)->SetupPose(pose);
    const PblMatrix* pNode = pose.Find(uNodeHash);
    if (pNode)
    {
        rPos = pNode->GetTranslation();
    }
}
}

// FUNCTION: SWBF2 0x004c8e60
void EntityHoverClass::SetProperty(unsigned int uProperty, const char* pszValue)
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
    case 0x4f358485:
        sscanf(pszValue, "%f", &mSetAltitude);
        break;
    case 0x97f2e0f1:
        sscanf(pszValue, "%f", &mGravityScale);
        break;
    case 0x1b5071f9:
        sscanf(pszValue, "%f", &mLiftSpring);
        break;
    case 0xa37b2ec6:
        sscanf(pszValue, "%f", &mLiftDamp);
        break;
    case 0x2fa0ba9d:
        sscanf(pszValue, "%f", &mAcceleration);
        break;
    case 0x8b12c806:
        sscanf(pszValue, "%f", &mDeceleration);
        break;
    case 0x4fe8d3f1:
        sscanf(pszValue, "%f", &mTraction);
        break;
    case 0xd42834bd:
        sscanf(pszValue, "%f", &mForwardSpeed);
        break;
    case 0x1f069ec0:
        sscanf(pszValue, "%f", &mReverseSpeed);
        break;
    case 0xfe1ce511:
        sscanf(pszValue, "%f", &mStrafeSpeed);
        break;
    case 0xa5900707:
        sscanf(pszValue, "%f", &mBoostSpeed);
        break;
    case 0x7b001920:
        sscanf(pszValue, "%f", &mBoostAcceleration);
        break;
    case 0x3a1b2aef:
        sscanf(pszValue, "%f", &mBoostFOV);
        mBoostFOV *= 0.017453292f;
        break;
    case 0x2c358e68:
        sscanf(pszValue, "%f", &mBoostTurnMultiplier);
        break;
    case 0xddd0e74b:
        sscanf(pszValue, "%f", &mSpinRate);
        break;
    case 0x0f77c4e8:
        sscanf(pszValue, "%f", &mTurnRate);
        break;
    case 0xde721cb4:
        sscanf(pszValue, "%f", &mTurnFilter);
        break;
    case 0xb9b8abc3:
        sscanf(pszValue, "%f", &mPitchRate);
        break;
    case 0x3403b139:
        sscanf(pszValue, "%f %f", &mPitchMin, &mPitchMax);
        mPitchMin *= 0.017453292f;
        mPitchMax *= 0.017453292f;
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
    case 0x02e46404:
        sscanf(pszValue, "%f", &mLevelSpring);
        break;
    case 0x252dfaab:
        sscanf(pszValue, "%f", &mLevelDamp);
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

    case 0x98557a5e:
        InitAnimations(pszValue);
        break;
    case 0xef563a98:
        if (mAnimObj)
        {
            mAnimFins = const_cast<ZephyrAnim*>(mAnimObj->FindAnimation(pszValue));
        }
        break;

    case 0x02bb1fe0:
    case 0x9b30e763:
        mExplosion = ExplosionClass::Find(PblHash(pszValue));
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
                        mAttachOffset = pSkeleton->GetDefaultPoseMatrix(i).GetTranslation();
                        break;
                    }
                }
            }
        }
        break;
    }
    case 0x26d7a21c:
        mWakeEffect = PblHash(pszValue);
        break;
    case 0x87733801:
        mWaterEffect = PblHash(pszValue);
        break;
    case 0xb9282909:
        sscanf(pszValue, "%f", &mBlurEffect);
        break;
    case 0x04afc6a4:
        sscanf(pszValue, "%f", &mBlurStartV);
        break;
    case 0x14d902a6:

        mSoundCloseToGround.SetID(pszValue, true, false);
        break;
    case 0x3a3614a0:
        sscanf(pszValue, "%f", &mSoundCloseToGroundHeight);
        mSoundCloseToGroundHeight = 1.0f - mSoundCloseToGroundHeight;
        break;

    case 0xf0856c5d:
        sscanf(pszValue, "%f", &mMaxSuspensionOffset);
        break;
    case 0x8eda7a09:
        sscanf(pszValue, "%f", &mMidSuspensionOffset);
        break;
    case 0xe534c2cb:
        mSuspensionCenterHash = PblTEMPHash(pszValue);
        ReadSuspensionNode(mModel, mSuspensionCenterHash, mBaseSuspensionPos);
        break;
    case 0xabb24116:
        mSuspensionLeftArmHash = PblTEMPHash(pszValue);
        ReadSuspensionNode(mModel, mSuspensionLeftArmHash, mLeftSuspensionPos);
        break;
    case 0x580b459f:
        mSuspensionRightArmHash = PblTEMPHash(pszValue);
        ReadSuspensionNode(mModel, mSuspensionRightArmHash, mRightSuspensionPos);
        break;

    case 0x6363d774:
        mFirstPersonModelHandle = FirstPerson::AddGeometry(pszValue);
        break;
    case 0x337b1504:
    {
        int iAttached;
        sscanf(pszValue, "%i", &iAttached);
        mFPModelAttachedtoCam = iAttached > 0;
        break;
    }
    case 0x3c45420e:
        mMovingTurnOnly = atoi(pszValue) != 0;
        break;
    case 0xefb6b10a:
        mFloatsOnWater = atoi(pszValue) != 0;
        break;

    case 0xdf0d2c6f:
        sscanf(pszValue, "%f", &mVelocitySpring);
        break;
    case 0x661d8524:
        sscanf(pszValue, "%f", &mVelocityDamp);
        break;
    case 0x9184393d:
        sscanf(pszValue, "%f", &mOmegaXSpring);
        break;
    case 0x9d59000a:
        sscanf(pszValue, "%f", &mOmegaXDamp);
        break;
    case 0x37949663:
        sscanf(pszValue, "%f", &mOmegaZSpring);
        break;
    case 0xaa764468:
        sscanf(pszValue, "%f", &mOmegaZDamp);
        break;
    case 0x7a5de0fc:
        mRandomSpring = false;
        break;
    case 0x77d6d149:
        sscanf(pszValue, "%f", &sBodySpringLengthRadiusMultiplier);
        break;
    case 0x5f1b7721:
    {
        sDbgHov = this;
        float x, y, z, fRadius;
        sscanf(pszValue, "%f %f %f %f", &x, &y, &z, &fRadius);
        if (mNumColliderBodies < 6)
        {
            CollisionPrimitive primitive;
            primitive.SetSphere(fRadius);
            PblMatrix matrix = _IdentityMatrix;
            matrix.m[3][0] = x;
            matrix.m[3][1] = y;
            matrix.m[3][2] = z;
            matrix.m[3][3] = 1.0f;
            char szName[32];
            _snprintf(szName, 31, "SpringBody%d", mNumColliderBodies + 1);
            szName[31] = 0;
            mCollisionModel.AddBody(new CollisionBodyPrimitive(primitive, szName, matrix, 0));

            const PblVector3& vCenter = mModel->_mModel->_BoundingSphere.GetCenter();
            float dx = vCenter.x - x;
            float dy = vCenter.y - y;
            float dz = vCenter.z - z;
            float fReach = sqrtf(dy * dy + dx * dx + dz * dz) + fRadius;
            mPotentialCollisionRadius = mPotentialCollisionRadius > fReach ? mPotentialCollisionRadius : fReach;

            SetupDefaultColliderBodyFactors(mColliderBodyInfo[mNumColliderBodies],
                                            mCollisionModel.GetMaskId(PblHash(szName)));
            AddBuildingCollision(szName);
            AddTerrainCollision(szName);
            ++mNumColliderBodies;
        }
        break;
    }
    case 0x577b739e:
    {
        sDbgHov = this;
        int iBodyId = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (iBodyId >= 0 && mNumColliderBodies < 6)
        {
            SetupDefaultColliderBodyFactors(mColliderBodyInfo[mNumColliderBodies], iBodyId);
            AddBuildingCollision(pszValue);
            AddTerrainCollision(pszValue);
            ++mNumColliderBodies;
        }
        break;
    }
    case 0xeafe81d0:
        sscanf(pszValue, "%f", &mColliderBodyInfo[mNumColliderBodies - 1].mSpringLength);
        break;
    case 0xf8911430:
        sscanf(pszValue, "%f", &mColliderBodyInfo[mNumColliderBodies - 1].mVelocitySpringFactor);
        break;
    case 0xd3bf70df:
        sscanf(pszValue, "%f", &mColliderBodyInfo[mNumColliderBodies - 1].mVelocityDampFactor);
        break;
    case 0xa8a2ecbe:
        sscanf(pszValue, "%f", &mColliderBodyInfo[mNumColliderBodies - 1].mOmegaXSpringFactor);
        break;
    case 0xbf3448ed:
        sscanf(pszValue, "%f", &mColliderBodyInfo[mNumColliderBodies - 1].mOmegaXDampFactor);
        break;
    case 0x29be15fc:
        sscanf(pszValue, "%f", &mColliderBodyInfo[mNumColliderBodies - 1].mOmegaZSpringFactor);
        break;
    case 0x29041feb:
        sscanf(pszValue, "%f", &mColliderBodyInfo[mNumColliderBodies - 1].mOmegaZDampFactor);
        break;

    case 0x4f9d8bb4:
        sscanf(pszValue, "%f", &mEnergyAutoRestore);
        break;
    case 0x645f56f6:
        sscanf(pszValue, "%f", &mEnergyBoostDrain);
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
    case 0xbff09d80:
        sscanf(pszValue, "%f", &mJumpTimeMin);
        break;
    case 0xd5dd386a:
        sscanf(pszValue, "%f", &mJumpTimeMax);
        break;
    case 0x4aaefdcc:
        sscanf(pszValue, "%f", &mJumpForce);
        break;
    case 0x534dfe50:
        sscanf(pszValue, "%f", &mJumpMinSpeedMult);
        break;
    case 0x391e36a1:
        sscanf(pszValue, "%f", &mJumpEnergyPerSec);
        break;

    case 0x1a6593db:
        sWheelSection = atoi(pszValue) - 1;
        break;
    case 0x80841697:
    {
        mWheelTextureId[sWheelSection] = PblHash(pszValue);
        int iCount = sWheelSection + 1;
        if (mWheelCount > iCount)
        {
            iCount = mWheelCount;
        }
        mWheelCount = iCount;
        break;
    }
    case 0x4b8c0d9f:
        sscanf(pszValue, "%f", &mWheelVelocToU[sWheelSection]);
        break;
    case 0x4c8c0f32:
        sscanf(pszValue, "%f", &mWheelVelocToV[sWheelSection]);
        break;
    case 0xba991e3d:
        sscanf(pszValue, "%f", &mWheelOmegaToU[sWheelSection]);
        break;
    case 0xb7991984:
        sscanf(pszValue, "%f", &mWheelOmegaToV[sWheelSection]);
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

    case 0x47c86b4a:
    {
        GameObjectClass::SetProperty(uProperty, pszValue);
        sSoftCollisionMask = 0;
        sOrdnanceCollisionMask = 0;
        sBodySpringLengthRadiusMultiplier = 0.5f;
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
            const PblBox& box = mModel->_mModel->_TotalBoundingBox;
            mCameraSettings.mCameraSafePoint.x = 0.0f;
            mCameraSettings.mCameraSafePoint.y = box._vMax.y * 0.9f;
            mCameraSettings.mCameraSafePoint.z = 0.0f;
            if (iNumBodies == 0)
            {
                float fCenterX = (box._vMin.x + box._vMax.x) * 0.5f;
                float fCenterY = (box._vMin.y + box._vMax.y) * 0.5f;
                float fCenterZ = (box._vMin.z + box._vMax.z) * 0.5f;
                float fHalfX = (box._vMax.x - box._vMin.x) * 0.5f;
                float fHalfY = (box._vMax.y - box._vMin.y) * 0.5f;
                float fHalfZ = (box._vMax.z - box._vMin.z) * 0.5f;
                CollisionPrimitive primitive;
                PblMatrix matrix = _IdentityMatrix;
                primitive.SetAACylinder((fHalfX + fHalfZ) * 0.5f, fHalfY * 2.0f);
                matrix.m[3][0] = fCenterX;
                matrix.m[3][1] = fCenterY;
                matrix.m[3][2] = fCenterZ;
                matrix.m[3][3] = 1.0f;
                mCollisionModel.AddBody(new CollisionBodyPrimitive(primitive, "main_body", matrix, 0));
            }
            mAttachOffset.x = 0.0f;
            mAttachOffset.y = box._vMax.y;
            mAttachOffset.z = 0.0f;
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
    case 0xafe693ce:
    {

        GameObjectClass::SetProperty(uProperty, pszValue);
        int iMaskId = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (iMaskId >= 0)
        {
            mWakeEmitterPrimitives[mNumWakeEmitters++] = iMaskId;
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

// SYNTHETIC: SWBF2 0x004c0980
// EntityHover::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004c1300
EntityHover::~EntityHover()
{
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
    if (mWaterEmitter)
    {
        mWaterEmitter->DeactivateEffect();
    }
    for (int i = 0; i < 6; ++i)
    {
        FLEffectObject* pEmitter = mWakeEmitters[i];
        if (pEmitter)
        {
            pEmitter->DeactivateEffect();
        }
    }
    reinterpret_cast<EnergyBar*>(mEnergyBar)->~EnergyBar();
}

// FUNCTION: SWBF2 0x004c76d0
void EntityHover::SetupController()
{
    Controller* pController = 0;
    if (mPlayerId >= 0)
    {
        sDbgHov = mClass;
        pController = new PlayerController(this);
        Track(mPlayerId);
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

// FUNCTION: SWBF2 0x004c32b0
int EntityHover::GetFirstPersonIndex()
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

// FUNCTION: SWBF2 0x004c3330
int EntityHover::GetJoystickIndex()
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

extern "C" PblMatrix* __stdcall D3DXMatrixRotationX(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationZ(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);

extern bool netIsLocalTurn;

extern PblVector3 gVectorZero;

Controllable* GetControllablePilot(Controllable* pControllable);

PblMatrix NormalMatrix(const PblVector3& vDir, const PblVector3& vNormal, const PblVector3& vPos);

namespace SquadCommands { void VehicleCommand(GameObject* pObject, Character* pCharacter); }

// FUNCTION: SWBF2 0x004c2230
bool EntityHover::Update(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }

    Controllable::Update(fDeltaTime);

    if (NetGame::IsEnabled() && NetGame::JustEndedPreGame() && !NetGame::IsClient())
    {
        mIsAlive = false;
        mIsDead = true;
    }

    if (!IsAlive())
    {
        if (!mIsDead)
        {
            int iFirstPerson = GetFirstPersonIndex();
            if (iFirstPerson >= 0)
            {
                FirstPerson::Activate(iFirstPerson, false);
            }
            if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mDamageOwner.mCharacter)
            {
                AwardVehicleKillPoints(mClass->GetVehicleType(), GetControllablePilot(this) == 0);
            }
            if (mClass->mExplosion)
            {
                DamageOwner owner(mCharacter, this, GetTeam(), mClass, 0, 1.0f);
                mClass->mExplosion->CreateExplosion(mMatrix.GetTranslation(), owner, false, COLL_UNKNOWN);
            }
        }
        if (!NetGame::IsEnabled() || !mIsDead)
        {
            CreateChunkCluster(1.0f);
            Kill();
        }
        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            return false;
        }
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        PblVector3 vPos = mMatrix.GetTranslation();
        if (-500.0f > vPos.y)
        {
            Kill();
            return false;
        }
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

    UpdateStatus(fDeltaTime);
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

    if (mClass->mSuspensionCenterHash)
    {
        mSuspensionVelocity =
            ((mCurrSuspensionOffset - mClass->mMidSuspensionOffset) * -205.0f - mSuspensionVelocity * 6.5f) *
                fDeltaTime +
            mSuspensionVelocity;
        mCurrSuspensionOffset = mSuspensionVelocity * fDeltaTime + mCurrSuspensionOffset;
        if (mCurrSuspensionOffset > 0.0f)
        {
            mCurrSuspensionOffset = 0.0f;
            mSuspensionVelocity = 0.0f;
        }
        if (mClass->mMaxSuspensionOffset > mCurrSuspensionOffset)
        {
            mCurrSuspensionOffset = mClass->mMaxSuspensionOffset;
            mSuspensionVelocity = 0.0f;
        }
        bool bTurn = true;
        if (mClass->mMidSuspensionOffset > mNextSuspensionOffset)
        {
            if (mNextSuspensionOffset > mCurrSuspensionOffset)
            {
                mCurrSuspensionOffset = mNextSuspensionOffset;
            }
            else if (!(mSuspensionVelocity > 0.0f))
            {
                bTurn = false;
            }
        }
        else
        {
            if (mCurrSuspensionOffset > mNextSuspensionOffset)
            {
                mCurrSuspensionOffset = mNextSuspensionOffset;
            }
            else if (!(0.0f > mSuspensionVelocity))
            {
                bTurn = false;
            }
        }
        if (bTurn)
        {
            mLastSuspensionOffset = mCurrSuspensionOffset;
            mNextSuspensionOffset = mClass->mMidSuspensionOffset * 2.0f - mCurrSuspensionOffset;
            if (0.0f > mSuspensionVelocity)
            {
                mSuspensionVelocity = 0.0f;
            }
        }
    }

    if (mPilotType == PILOT_SELF || (mPilotType == PILOT_VEHICLESELF && mPilot == this) || !mPilot)
    {
        if (IsOn())
        {
            TurnOff(mClass, mMatrix.GetTranslation(), mVelocity);
        }
    }
    else
    {
        Controllable* pPilot = mPilot;
        if (!IsOn() || IsTurningOff())
        {
            TurnOn(mClass, mMatrix.GetTranslation(), mVelocity, mMaxHealth != 0.0f ? mCurHealth / mMaxHealth : 0.0f);
        }
        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
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
                    if (mClass->mIsPilotAnimation9Pose)
                    {
                        mClass->UpdatePilotAnimation(pPilot, &mPose, mMatrix, mVelocity, fDeltaTime, -1.0f, -1.0f,
                                                     -1.0f);
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

    if (mControlSquadCommand.IsPressed())
    {
        SquadCommands::VehicleCommand(this, mCharacter);
    }

    const PblList<Controllable>& list = GetControllableList();
    if (list.GetCount() == 1)
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

    if (IsAlive() && mPlayerId >= 0)
    {
        int iIndex = mWeaponIndex[0];
        if (iIndex >= 0 && mWeapon[iIndex] && mWeapon[iIndex]->mClass->mZoomMax > 1.0f && mControlZoom.IsPressed())
        {
            mTargetInfo.mIsAiming = mWeapon[iIndex]->CycleZoom(mTargetInfo.mIsAiming);
        }
        if (mTracker)
        {
            if (mControlView.IsPressed() && netIsLocalTurn && !IsForcedThirdPerson() && !IsForcedFirstPerson() &&
                mClass->mForceMode != 1 && mClass->mForceMode != 2 && mTracker)
            {
                SwitchControlView(mPlayerId);
            }
            int iFirstPerson = GetFirstPersonIndex();
            if (iFirstPerson >= 0)
            {
                FirstPerson::Update(iFirstPerson, TYPE_HOVER, mClass->mFirstPersonModelHandle, this, 0);
            }
        }
        if (list.GetFirst()->_pObject->mPlayerId >= 0)
        {
            ToolTipsDisplay::sInstance->ShowTip(TTT_HoverSprint, 0);
        }
        if (list.GetCount() > 1)
        {
            ToolTipsDisplay::sInstance->ShowTip(TTT_AfterBoardMultiseat, 0);
        }
    }

    bool bMouse = false;
    if (mPlayerId >= 0)
    {
        bMouse = NetGame::IsMouseMode(mPlayerId);
    }
    float fThrust = mControlMove;
    float fStrafe = mControlStrafe;
    float fTurn = mControlTurn;
    float fPitch = mControlPitch;

    if (mPlayerId >= 0 &&
        (mBoost ? (!mControlSprint.Tapped() && !mControlSprint.HeldAndReleased() && fThrust >= 0.9f)
                : (mControlThrustFwd.TappedAndPressed() ||
                   ((mControlSprint.Tapped() || mControlSprint.Held()) && fThrust >= 0.9f))))
    {
        ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_HoverSprint, 0);
        mBoost = true;
    }
    else
    {
        mBoost = mCtrl && mPlayerId < 0 && UnitControllerPtr(mCtrl)->GetLowLevel().GetHeight() == HEIGHT_SPRINT;
    }

    float fScale = fDeltaTime;
    if (mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]])
    {
        fScale = fDeltaTime / mWeapon[mWeaponIndex[0]]->mZoomTurnScale;
    }
    float fYawRate = mClass->mYawRate;
    float fPitchRate;
    if (bMouse)
    {
        fPitchRate = fYawRate;
    }
    else
    {
        fPitchRate = mClass->mPitchRate;
        if (mPlayerId >= 0)
        {
            fYawRate = (fYawRate - fPitchRate) * mTurnBuildup + fPitchRate;
        }
    }
    mYawValue = mYawValue - fYawRate * fTurn * (fDeltaTime * 30.0f) * fScale;
    mPitchValue = fPitchRate * fPitch * (fDeltaTime * 30.0f) * fScale + mPitchValue;

    PblVector3 vRight = mMatrix.GetXAxis();
    PblVector3 vUp = mMatrix.GetYAxis();
    Move(fDeltaTime, fThrust, fStrafe, fTurn, mBoost);
    mYawValue -= mMatrix.m[2][0] * vRight.x + mMatrix.m[2][1] * vRight.y + mMatrix.m[2][2] * vRight.z;
    mPitchValue -= mMatrix.m[2][0] * vUp.x + mMatrix.m[2][1] * vUp.y + mMatrix.m[2][2] * vUp.z;
    if (mClass->mYawMin > mYawValue)
    {
        mYawValue = mClass->mYawMin;
    }
    else if (mYawValue > mClass->mYawMax)
    {
        mYawValue = mClass->mYawMax;
    }
    if (mClass->mPitchMin > mPitchValue)
    {
        mPitchValue = mClass->mPitchMin;
    }
    else if (mPitchValue > mClass->mPitchMax)
    {
        mPitchValue = mClass->mPitchMax;
    }

    if (mObstacle)
    {
        mObstacle->UpdatePosition(GetMatrix(), mClass->mBarrierOX, mClass->mBarrierOZ, GetVelocity());
    }

    float fWaterHeight;
    if (RedWater::GetWaterHeight(GetPosition(), fWaterHeight))
    {
        mWaterEmitterPos = GetPosition();
        mWaterEmitterPos.y = fWaterHeight + 0.02f;
        if (!mWaterEmitter && mClass->mWaterEffect)
        {
            FLEffectClass* pClass = FLEffect::s_EffectClasses.Find(mClass->mWaterEffect);
            if (pClass)
            {
                mWaterEmitter = pClass->BuildEffect();
                if (mWaterEmitter)
                {
                    mWaterEmitter->AttachEffectToVector(&mWaterEmitterPos, true);
                }
            }
        }
    }
    else if (mWaterEmitter)
    {
        mWaterEmitter->StopAndFinish();
    }
    return true;
}

// FUNCTION: SWBF2 0x004c3fa0
void EntityHover::Move(float fDeltaTime, float fThrust, float fStrafe, float fTurn, bool& rBoost)
{
    if (0.0001f > mClass->mStrafeSpeed)
    {
        fStrafe = 0.0f;
    }
    float fHeight = 0.0f;

    PblMatrix mUnbank;
    PblMatrix mUnpitch;
    D3DXMatrixRotationZ(&mUnbank, -mBankRollAngle);
    D3DXMatrixMultiply(&mUnbank, &mUnbank, D3DXMatrixRotationX(&mUnpitch, -mBankPitchAngle));
    PblMatrix mBody;
    D3DXMatrixMultiply(&mBody, &mUnbank, &mMatrix);

    const GameObject* pIgnore = this;
    float fInput = fabsf(fStrafe) > fabsf(fThrust) ? fabsf(fStrafe) : fabsf(fThrust);

    EnergyBar* pEnergy = reinterpret_cast<EnergyBar*>(mEnergyBar);
    if (rBoost && !(mClass->mBoostSpeed > 0.0f && pEnergy->SpendEnergy(mClass->mEnergyBoostDrain * fDeltaTime)))
    {
        rBoost = false;
    }
    if (!rBoost && !mJumpMode)
    {
        float fMultiplier = mCharacter && mCharacter->mMedalsMgr &&
                                    mCharacter->mMedalsMgr->IsAwardAvailable(MedalsMgr::ENDURANCE)
                                ? 1.5f
                                : 1.0f;
        pEnergy->AddEnergy(mClass->mEnergyAutoRestore * fMultiplier * fDeltaTime);
    }

    PblMatrix mGround;
    const PblMatrix* pDrive;
    if (mClass->mNumColliderBodies > 0)
    {
        mVelocity.y = (mClass->mGravityScale * -9.8f - (1.0f - mClass->mGravityScale) * mGroundRatio * 9.8f) *
                          fDeltaTime +
                      mVelocity.y;
        float fSpring = mClass->mLevelSpring * fDeltaTime;
        if (fSpring > 1.0f)
        {
            fSpring = 1.0f;
        }
        mOmega.x = mBody.m[2][1] * fabsf(mBody.m[2][1]) * fSpring + mOmega.x;
        fSpring = mClass->mLevelSpring * fDeltaTime;
        if (fSpring > 1.0f)
        {
            fSpring = 1.0f;
        }
        mOmega.z = mOmega.z - fabsf(mBody.m[0][1]) * mBody.m[0][1] * fSpring;
        pDrive = &mBody;
    }
    else
    {
        CollisionObject* pHit;
        PblVector3 vUp;
        if (mClass->mSetAltitude > 0.0f)
        {
            PblVector3 vStart = GetCollisionCenter();
            PblVector3 vDown(0.0f, -1.0f, 0.0f);
            float fLength = mClass->mSetAltitude * 10.0f + GetCollisionRadius();
            PblVector3 vNormal = gVectorUnitY;
            float fHit = CollisionManager::RayHit(vStart, vDown, fLength, pHit, &vNormal, &pIgnore, 1, 0x190, false);
            mGroundNormal = vNormal;
            fHeight = fHit * fLength + mBody.m[3][1] - vStart.y;

            if (mVelocity.x * mVelocity.x + mVelocity.z * mVelocity.z > 25.0f)
            {
                PblVector3 vAhead(vDown.x * fLength + mVelocity.x, mVelocity.y + vDown.y * fLength,
                                  mVelocity.z + vDown.z * fLength);
                float fAheadLength = sqrtf(vAhead.y * vAhead.y + vAhead.x * vAhead.x + vAhead.z * vAhead.z);
                float fInv = 1.0f / fAheadLength;
                PblVector3 vAheadDir(vAhead.x * fInv, vAhead.y * fInv, vAhead.z * fInv);
                float fAheadHit = CollisionManager::RayHit(vStart, vAheadDir, fAheadLength, pHit, &vNormal, &pIgnore,
                                                           1, 0x190, false);
                if (1.0f > fAheadHit)
                {
                    float fAheadDist = fAheadHit * fAheadLength;
                    float fDrop = -((vAheadDir.x * fAheadDist) * vNormal.x + (vAheadDir.y * fAheadDist) * vNormal.y +
                                    vNormal.z * (vAheadDir.z * fAheadDist));
                    if (!(fDrop > fHeight))
                    {
                        fHeight = fDrop;
                    }
                    if (vNormal.y > 0.0f)
                    {
                        float fBlend = (1.0f - fAheadHit) * vNormal.y;
                        PblVector3 vBlend((vNormal.x - mGroundNormal.x) * fBlend + mGroundNormal.x,
                                          (vNormal.y - mGroundNormal.y) * fBlend + mGroundNormal.y,
                                          (vNormal.z - mGroundNormal.z) * fBlend + mGroundNormal.z);
                        PblVector3 vGround;
                        D3DXVec3Normalize(&vGround, &vBlend);
                        mGroundNormal = vGround;
                    }
                }
            }

            float fRatio = 1.0f - (fHeight - mClass->mSetAltitude) / (mClass->mSetAltitude * 9.0f);
            if (0.0f > fRatio)
            {
                fRatio = 0.0f;
            }
            else if (fRatio > 1.0f)
            {
                fRatio = 1.0f;
            }
            mGroundRatio = fRatio;
            float fLift = mClass->mSetAltitude > 0.0f ? 0.1f : 0.0f;
            PblVector3 vTilt(mGroundNormal.x * fRatio, mGroundNormal.y + fLift, mGroundNormal.z * fRatio);
            D3DXVec3Normalize(&vUp, &vTilt);
        }
        else if (mGroundRatio == 1.0f)
        {
            PblVector3 vStart(mBody.m[3][0] + 0.0f, mBody.m[3][1] + GetCollisionRadius(), mBody.m[3][2] + 0.0f);
            PblVector3 vDown(0.0f, -1.0f, 0.0f);
            CollisionManager::RayHit(vStart, vDown, 32.0f, pHit, &mGroundNormal, &pIgnore, 1, 0x90, false);
            vUp = mGroundNormal;
        }
        else
        {
            vUp.x = 0.0f;
            vUp.y = 1.0f;
            vUp.z = 0.0f;
        }

        mGround = NormalMatrix(mBody.GetZAxis(), mGroundNormal, gVectorZero);
        mVelocity.y = (-9.8f - (mClass->mGravityScale - 1.0f) * mGroundRatio * 9.8f) * fDeltaTime + mVelocity.y;
        if (mClass->mSetAltitude > 0.0f)
        {
            float fLift = (mClass->mSetAltitude - fHeight) * mClass->mLiftSpring + mGroundRatio * 9.8f -
                          (mGroundNormal.y * mVelocity.y + mGroundNormal.x * mVelocity.x +
                           mGroundNormal.z * mVelocity.z) *
                              mClass->mLiftDamp;
            if (fLift > 0.0f)
            {
                fLift *= fDeltaTime;
                mVelocity.x = mVelocity.x + fLift * mGroundNormal.x;
                mVelocity.y = mGroundNormal.y * fLift + mVelocity.y;
                mVelocity.z = mGroundNormal.z * fLift + mVelocity.z;
            }
        }

        float fDamp = mClass->mLevelDamp * fDeltaTime;
        if (fDamp > 1.0f)
        {
            fDamp = 1.0f;
        }
        mOmega.x = (1.0f - fDamp) * mOmega.x;
        fDamp = mClass->mLevelDamp * fDeltaTime;
        if (fDamp > 1.0f)
        {
            fDamp = 1.0f;
        }
        mOmega.z = (1.0f - fDamp) * mOmega.z;
        float fSpring = mClass->mLevelSpring * fDeltaTime;
        if (fSpring > 1.0f)
        {
            fSpring = 1.0f;
        }
        mOmega.x = mOmega.x + (mBody.m[2][0] * vUp.x + mBody.m[2][1] * vUp.y + mBody.m[2][2] * vUp.z) * fSpring;
        fSpring = mClass->mLevelSpring * fDeltaTime;
        if (fSpring > 1.0f)
        {
            fSpring = 1.0f;
        }
        mOmega.z = mOmega.z - (mBody.m[0][0] * vUp.x + mBody.m[0][1] * vUp.y + mBody.m[0][2] * vUp.z) * fSpring;
        pDrive = &mGround;
    }

    const PblMatrix& mDrive = *pDrive;
    float fClimb = mBody.m[1][1] > 0.0f ? mBody.m[1][1] : 0.0f;
    float fGrip = fClimb * mGroundRatio * fDeltaTime;
    float fAccel = (rBoost ? mClass->mBoostAcceleration
                           : (mClass->mAcceleration - mClass->mDeceleration) * fInput + mClass->mDeceleration) *
                   fGrip;
    float fTraction = mClass->mTraction * fGrip;
    float fSpeed;
    if (0.0f > fThrust)
    {
        fSpeed = mClass->mReverseSpeed;
    }
    else if (rBoost)
    {
        fSpeed = mClass->mBoostSpeed;
    }
    else
    {
        fSpeed = mClass->mForwardSpeed;
    }
    float fStrafeSpeed = mClass->mStrafeSpeed;
    float fStrafeDelta = -(fStrafeSpeed * fStrafe) - (mVelocity.y * mDrive.m[0][1] + mDrive.m[0][0] * mVelocity.x +
                                                      mVelocity.z * mDrive.m[0][2]);
    float fForwardDelta =
        fSpeed * fThrust - (mVelocity.y * mDrive.m[2][1] + mDrive.m[2][0] * mVelocity.x + mVelocity.z * mDrive.m[2][2]);
    if (-fAccel > fForwardDelta)
    {
        fForwardDelta = -fAccel;
    }
    else if (fForwardDelta > fAccel)
    {
        fForwardDelta = fAccel;
    }
    mVelocity.x = fForwardDelta * mDrive.m[2][0] + mVelocity.x;
    mVelocity.y = fForwardDelta * mDrive.m[2][1] + mVelocity.y;
    mVelocity.z = fForwardDelta * mDrive.m[2][2] + mVelocity.z;
    if (0.0001f > fStrafeSpeed)
    {

        if (-fTraction > fStrafeDelta)
        {
            fStrafeDelta = -fTraction;
        }
        else if (fStrafeDelta > fTraction)
        {
            fStrafeDelta = fTraction;
        }
        mVelocity.x = mDrive.m[0][0] * fStrafeDelta + mVelocity.x;
        mVelocity.y = mDrive.m[0][1] * fStrafeDelta + mVelocity.y;
        mVelocity.z = mDrive.m[0][2] * fStrafeDelta + mVelocity.z;
    }
    else
    {
        if (-fAccel > fStrafeDelta)
        {
            fStrafeDelta = -fAccel;
        }
        else if (fStrafeDelta > fAccel)
        {
            fStrafeDelta = fAccel;
        }
        mVelocity.x = mDrive.m[0][0] * fStrafeDelta + mVelocity.x;
        mVelocity.y = mDrive.m[0][1] * fStrafeDelta + mVelocity.y;
        mVelocity.z = mDrive.m[0][2] * fStrafeDelta + mVelocity.z;
        if (fInput > 0.0001f)
        {

            PblVector3 vSpin(mDrive.m[1][0] * mOmega.y, mDrive.m[1][1] * mOmega.y, mDrive.m[1][2] * mOmega.y);
            PblVector3 vDrag((vSpin.y * mVelocity.z - vSpin.z * mVelocity.y) * fDeltaTime,
                             (vSpin.z * mVelocity.x - vSpin.x * mVelocity.z) * fDeltaTime,
                             (vSpin.x * mVelocity.y - vSpin.y * mVelocity.x) * fDeltaTime);
            float fMaxDrag = fTraction - fAccel;
            float fDragSq = vDrag.x * vDrag.x + vDrag.y * vDrag.y + vDrag.z * vDrag.z;
            if (fDragSq > fMaxDrag * fMaxDrag)
            {
                float fScale = fMaxDrag / (float)sqrt((double)fDragSq);
                vDrag.x *= fScale;
                vDrag.y *= fScale;
                vDrag.z *= fScale;
            }
            mVelocity.x = vDrag.x * fInput + mVelocity.x;
            mVelocity.y = vDrag.y * fInput + mVelocity.y;
            mVelocity.z = vDrag.z * fInput + mVelocity.z;
        }
    }

    if (mClass->mJumpForce > 0.0f)
    {
        if (!mJumpMode)
        {
            float fMinSpeed = mClass->mJumpMinSpeedMult * mClass->mForwardSpeed;
            if (mControlJump.IsDown() && mGroundRatio > 0.9f &&
                mVelocity.x * mMatrix.m[2][0] + mVelocity.y * mMatrix.m[2][1] + mVelocity.z * mMatrix.m[2][2] >
                    fMinSpeed)
            {
                mJumpMode = true;
            }
            mJumpTimer = 0.0f;
        }
        else if ((mClass->mJumpTimeMin > mJumpTimer || mControlJump.IsDown()) &&
                 pEnergy->SpendEnergy(mClass->mJumpEnergyPerSec * fDeltaTime))
        {
            mJumpTimer += fDeltaTime;
            float fForward =
                mVelocity.x * mMatrix.m[2][0] + mVelocity.y * mMatrix.m[2][1] + mVelocity.z * mMatrix.m[2][2];
            float fMinSpeed = mClass->mJumpMinSpeedMult * mClass->mForwardSpeed;
            float fRatio = (fForward - fMinSpeed) / (mClass->mForwardSpeed - fMinSpeed);
            if (0.0f > fRatio)
            {
                fRatio = 0.0f;
            }
            else if (fRatio > 1.0f)
            {
                fRatio = 1.0f;
            }
            if ((1.0f - fRatio) * mClass->mJumpTimeMin + mClass->mJumpTimeMax * fRatio > mJumpTimer)
            {
                mVelocity.x = mClass->mJumpForce * mMatrix.m[1][0] * fDeltaTime + mVelocity.x;
                mVelocity.y = mMatrix.m[1][1] * mClass->mJumpForce * fDeltaTime + mVelocity.y;
                mVelocity.z = mMatrix.m[1][2] * mClass->mJumpForce * fDeltaTime + mVelocity.z;
            }
            else
            {
                mJumpMode = false;
            }
        }
        else
        {
            mJumpMode = false;
        }
    }

    float fVelocity = sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z);
    float fForward = mVelocity.x * mMatrix.m[2][0] + mVelocity.y * mMatrix.m[2][1] + mVelocity.z * mMatrix.m[2][2];
    bool bMouse = false;
    if (mPlayerId >= 0)
    {
        bMouse = NetGame::IsMouseMode(mPlayerId);
    }
    float fTopSpeed;
    float fTurnRate;
    if (mPlayerId >= 0 && !bMouse)
    {
        fTopSpeed = mClass->mForwardSpeed;
        float fRatio = fForward / fTopSpeed;
        if (0.0f > fRatio)
        {
            fRatio = 0.0f;
        }
        else if (fRatio > 1.0f)
        {
            fRatio = 1.0f;
        }
        fTurnRate = ((mClass->mTurnRate - mClass->mSpinRate) * fRatio + mClass->mSpinRate - mClass->mPitchRate) *
                        mTurnBuildup +
                    mClass->mPitchRate;
    }
    else
    {
        fTopSpeed = mClass->mForwardSpeed;
        float fRatio = fForward / fTopSpeed;
        if (0.0f > fRatio)
        {
            fRatio = 0.0f;
        }
        else if (fRatio > 1.0f)
        {
            fRatio = 1.0f;
        }
        fTurnRate = (mClass->mTurnRate - mClass->mSpinRate) * fRatio + mClass->mSpinRate;
    }
    fTurnRate = -fTurnRate;
    if (mClass->mBoostSpeed > 0.0f && fVelocity > fTopSpeed)
    {
        float fBoost = (fVelocity - fTopSpeed) / (mClass->mBoostSpeed - fTopSpeed);
        if (0.0f > fBoost)
        {
            fBoost = 0.0f;
        }
        else if (fBoost > 1.0f)
        {
            fBoost = 1.0f;
        }
        fTurnRate *= 1.0f - (1.0f - mClass->mBoostTurnMultiplier) * fBoost;
    }
    float fTurnTarget = fTurnRate * fTurn;
    if (fTurnTarget > mOmega.y)
    {
        float fLimit = mClass->mTurnFilter * fDeltaTime + mOmega.y;
        mOmega.y = fLimit > fTurnTarget ? fTurnTarget : fLimit;
    }
    else
    {
        float fLimit = mOmega.y - mClass->mTurnFilter * fDeltaTime;
        mOmega.y = fTurnTarget > fLimit ? fTurnTarget : fLimit;
    }

    PblQuaternion qOrient(mMatrix);
    PblVector3 vPos = mMatrix.GetTranslation();
    float fRate = sqrtf(mOmega.x * mOmega.x + mOmega.y * mOmega.y + mOmega.z * mOmega.z);
    if (fRate > 0.0001f)
    {
        float fInv = 1.0f / fRate;
        PblVector3 vAxis(fInv * mOmega.x, mOmega.y * fInv, mOmega.z * fInv);
        float fSin;
        float fCos;
        PblMath::SinCos(&fSin, &fCos, fRate * fDeltaTime * 0.5f);
        PblQuaternion qDelta;
        qDelta._fX = vAxis.x * fSin;
        qDelta._fY = vAxis.y * fSin;
        qDelta._fZ = vAxis.z * fSin;
        qDelta._fS = fCos;
        qOrient = qOrient * qDelta;
    }

    PblVector3 vDir;
    if (fVelocity > mClass->mForwardSpeed * 0.1f)
    {
        D3DXVec3Normalize(&vDir, &mVelocity);
    }
    PblVector3 vNewPos(vPos.x + mVelocity.x * fDeltaTime, vPos.y + mVelocity.y * fDeltaTime,
                       vPos.z + mVelocity.z * fDeltaTime);

    PblVector3 vLocal;
    D3DXVec3TransformNormal(&vLocal, &mVelocity, &mBody.GetInverseFast());
    float fInvSpeed = 1.0f / mClass->mForwardSpeed;
    float fSlipSide = (vLocal.z * mOmega.y - mOmega.z * vLocal.y) * fInvSpeed;
    float fSlipFront = (mOmega.x * vLocal.y - mOmega.y * vLocal.x) * fInvSpeed;
    mBankPitchAngle = (mClass->mThrustPitchAngle * fThrust + mClass->mBankAngle * fSlipFront - mBankPitchAngle) *
                          (mClass->mBankFilter * fDeltaTime) +
                      mBankPitchAngle;
    mBankRollAngle = (mClass->mStrafeRollAngle * fStrafe - mClass->mBankAngle * fSlipSide - mBankRollAngle) *
                         (mClass->mBankFilter * fDeltaTime) +
                     mBankRollAngle;

    PblQuaternion qUnit;
    PblMatrix mNew;
    D3DXMatrixRotationQuaternion(&mNew, D3DXQuaternionNormalize(&qUnit, &qOrient));
    mNew.m[3][0] = vNewPos.x;
    mNew.m[3][1] = vNewPos.y;
    mNew.m[3][2] = vNewPos.z;
    mNew.m[3][3] = 1.0f;
    PblMatrix mBank;
    PblMatrix mBankRoll;
    D3DXMatrixRotationX(&mBank, mBankPitchAngle);
    D3DXMatrixMultiply(&mBank, &mBank, D3DXMatrixRotationZ(&mBankRoll, mBankRollAngle));
    D3DXMatrixMultiply(&mBankRoll, &mBank, &mNew);
    SetMatrix(mBankRoll);

    if (mGroundRatio > mGroundRatioPrev && mGroundRatio <= mClass->mSoundCloseToGroundHeight && !mCloseToGround)
    {
        mClass->mSoundCloseToGround.Play(&mMatrix.GetTranslation(), &mVelocity, false, true);
        mCloseToGround = 1;
    }
    else if (mGroundRatioPrev > mGroundRatio)
    {
        mCloseToGround = 0;
    }
    mGroundRatioPrev = mGroundRatio;

    if (fDeltaTime > 0.0f)
    {
        float fEngineTop = fThrust > 0.0f ? mClass->mForwardSpeed : mClass->mReverseSpeed;
        float fThrottle = fEngineTop * fThrust;
        float fStrain = 0.0f;
        if (fEngineTop == 0.0f)
        {
            fEngineTop = 1.0f;
        }
        fForward /= fEngineTop;
        fThrottle /= fEngineTop;
        float fRevs = (float)fabs((double)fForward);
        float fAcceleration = (fRevs - mSpeedPrev) / fDeltaTime;
        if ((fAcceleration <= 0.0f && fThrottle > fForward) || (fAcceleration >= 0.0f && fForward > fThrottle))
        {
            fStrain = (float)fabs((double)(fThrottle - fForward));
        }
        mStrain = (fStrain - mStrain) * 0.1f + mStrain;
        mAccel = (fAcceleration - mAccel) * 0.01f + mAccel;
        float fHealth = mMaxHealth != 0.0f ? mCurHealth / mMaxHealth : 0.0f;
        VehicleEngine::Update(mClass, fDeltaTime, mMatrix.GetTranslation(), mVelocity, fRevs, mAccel, fHealth, 0.0f,
                              0.0f, mGroundRatio, mStrain, false, false, 0.0f);
        mSpeedPrev = fRevs;

        int iSide = 0;
        if (mPilot && mPilot->mCharacter)
        {
            iSide = mPilot->mCharacter->mTeamPtr->mSide;
        }
        VehicleMusic::Update(mClass, fDeltaTime,
                             mPilot && mPilot->mPlayerId >= 0 && GameLoop::GetNumCameras() == 1, fForward, iSide);
    }

    if (mPilot && mClass->mAnimObj && mClass->mAnimFins)
    {
        float fRate = fDeltaTime * 5.0f;
        mFinPitchValue = (fThrust - mFinPitchValue) * fRate + mFinPitchValue;
        float fRoll = fStrafe * 0.5f + fTurn;
        if (-1.0f > fRoll)
        {
            fRoll = -1.0f;
        }
        else if (fRoll > 1.0f)
        {
            fRoll = 1.0f;
        }
        mFinRollValue = (fRoll - mFinRollValue) * fRate + mFinRollValue;
        mForceFrameNum1 = mFinRollValue + 4.0f;
        mForceFrameBlend = (float)fabs((double)mFinPitchValue);
        mForceFrameNum2 = mFinPitchValue > 0.0f ? mFinRollValue + 1.0f : mFinRollValue + 7.0f;

        ZephyrPoseStatic<32> kPose;
        mZephyrSkeleton.Open(*mClass->mAnimObj->GetZephyrSkeletonShared(), kPose);
        kPose.Open(mZephyrSkeleton);
        mZephyrPoseDyn.SetLoop(0);
        mZephyrPoseDyn.SetFrameNumFloat(mForceFrameNum1);
        mZephyrPoseDyn.Update(0.0f);
        kPose.Set(mZephyrPoseDyn, true);
        mZephyrPoseDyn.SetFrameNumFloat(mForceFrameNum2);
        mZephyrPoseDyn.Update(0.0f);
        kPose.Blend(mZephyrPoseDyn, mForceFrameBlend);
        mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
        mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    }

    if (mClass->mWheelCount > 0)
    {
        float fWheelForward =
            mVelocity.x * mMatrix.m[2][0] + mVelocity.y * mMatrix.m[2][1] + mVelocity.z * mMatrix.m[2][2];
        float fWheelTurn = mOmega.y;
        for (int i = 0; i < mClass->mWheelCount; ++i)
        {
            mWheelTextureU[i] =
                (mClass->mWheelVelocToU[i] * fWheelForward + mClass->mWheelOmegaToU[i] * fWheelTurn) * fDeltaTime +
                mWheelTextureU[i];
            if (0.0f > mWheelTextureU[i])
            {
                mWheelTextureU[i] += 1.0f;
            }
            else if (mWheelTextureU[i] > 1.0f)
            {
                mWheelTextureU[i] -= 1.0f;
            }
            mWheelTextureV[i] =
                (mClass->mWheelVelocToV[i] * fWheelForward + mClass->mWheelOmegaToV[i] * fWheelTurn) * fDeltaTime +
                mWheelTextureV[i];
            if (0.0f > mWheelTextureV[i])
            {
                mWheelTextureV[i] += 1.0f;
            }
            else if (mWheelTextureV[i] > 1.0f)
            {
                mWheelTextureV[i] -= 1.0f;
            }
        }
    }

    if (!(mClass->mSetAltitude > 0.0f))
    {
        mGroundRatio = 0.0f;
    }
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

extern bool netFrameLock;
extern bool netIsLateTurn;
extern int gDebugCount;

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

// GLOBAL: SWBF2 0x01e51dd4
static bool sRenderHoverDebug;

// FUNCTION: SWBF2 0x004c17c0
void EntityHover::Kill()
{
    mIsAlive = false;
    mIsDead = true;
    PathFinder::DeleteGameObjectBarrier(mObstacle);
    int iJoystick = GetFirstPersonIndex();
    if (iJoystick >= 0)
    {
        FirstPerson::Activate(iJoystick, false);
        FirstPerson::Update(iJoystick, TYPE_HOVER, -2, 0, 0);
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

// FUNCTION: SWBF2 0x004c1890
void EntityHover::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (!RenderTrackable(mPlayerId) || mIsDead)
    {
        return;
    }

    if (mClass->m_bDynamicLighting)
    {
        uiFlags &= ~0x80;
    }
    else
    {
        uiFlags |= 0x80;
    }

    if (mClass->mSuspensionCenterHash)
    {
        const PblMatrix* pCenter = mPose.Find(mClass->mSuspensionCenterHash);
        if (pCenter)
        {
            PblMatrix& mCenter = mProceduralPoseMatrix[0];
            if (pCenter != &mCenter)
            {
                mPose.Remove(mClass->mSuspensionCenterHash);
                mPose.Store(mClass->mSuspensionCenterHash, &mCenter);
                mCenter = *pCenter;
            }
            PblVector3 vPos = mClass->mBaseSuspensionPos;
            vPos.y += mCurrSuspensionOffset;
            mCenter.m[3][0] = vPos.x;
            mCenter.m[3][1] = vPos.y;
            mCenter.m[3][2] = vPos.z;
            mCenter.m[3][3] = 1.0f;
        }

        const PblMatrix* pLeft = mClass->mSuspensionLeftArmHash ? mPose.Find(mClass->mSuspensionLeftArmHash) : 0;
        if (pLeft)
        {
            PblMatrix& mLeft = mProceduralPoseMatrix[1];
            if (pLeft != &mLeft)
            {
                mPose.Remove(mClass->mSuspensionLeftArmHash);
                mPose.Store(mClass->mSuspensionLeftArmHash, &mLeft);
            }
            const PblVector3& vPivot = mClass->mLeftSuspensionPos;
            PblVector3 vTarget(0.0f, vPivot.y + mCurrSuspensionOffset, vPivot.z);
            PblMatrix mArm = DirectionMatrix(
                PblVector3(vTarget.x - vPivot.x, vTarget.y - vPivot.y, vTarget.z - vPivot.z), gVectorUnitY, vPivot);
            mLeft.m[0][0] = -mArm.m[2][0];
            mLeft.m[0][1] = -mArm.m[2][1];
            mLeft.m[0][2] = -mArm.m[2][2];
            mLeft.m[0][3] = 0.0f;
            mLeft.m[1][0] = mArm.m[1][0];
            mLeft.m[1][1] = mArm.m[1][1];
            mLeft.m[1][2] = mArm.m[1][2];
            mLeft.m[1][3] = 0.0f;
            mLeft.m[2][0] = mArm.m[0][0];
            mLeft.m[2][1] = mArm.m[0][1];
            mLeft.m[2][2] = mArm.m[0][2];
            mLeft.m[2][3] = 0.0f;
            mLeft.m[3][0] = mClass->mLeftSuspensionPos.x;
            mLeft.m[3][1] = mClass->mLeftSuspensionPos.y;
            mLeft.m[3][2] = mClass->mLeftSuspensionPos.z;
            mLeft.m[3][3] = 1.0f;
        }

        const PblMatrix* pRight = mClass->mSuspensionRightArmHash ? mPose.Find(mClass->mSuspensionRightArmHash) : 0;
        if (pRight)
        {
            PblMatrix& mRight = mProceduralPoseMatrix[2];
            if (pRight != &mRight)
            {
                mPose.Remove(mClass->mSuspensionRightArmHash);
                mPose.Store(mClass->mSuspensionRightArmHash, &mRight);
            }
            const PblVector3& vPivot = mClass->mRightSuspensionPos;
            PblVector3 vTarget(0.0f, vPivot.y + mCurrSuspensionOffset, vPivot.z);
            PblMatrix mArm = DirectionMatrix(
                PblVector3(vTarget.x - vPivot.x, vTarget.y - vPivot.y, vTarget.z - vPivot.z), gVectorUnitY, vPivot);
            mRight.m[0][0] = mArm.m[2][0];
            mRight.m[0][1] = mArm.m[2][1];
            mRight.m[0][2] = mArm.m[2][2];
            mRight.m[0][3] = 0.0f;
            mRight.m[1][0] = mArm.m[1][0];
            mRight.m[1][1] = mArm.m[1][1];
            mRight.m[1][2] = mArm.m[1][2];
            mRight.m[1][3] = 0.0f;
            mRight.m[2][0] = -mArm.m[0][0];
            mRight.m[2][1] = -mArm.m[0][1];
            mRight.m[2][2] = -mArm.m[0][2];
            mRight.m[2][3] = 0.0f;
            mRight.m[3][0] = mClass->mRightSuspensionPos.x;
            mRight.m[3][1] = mClass->mRightSuspensionPos.y;
            mRight.m[3][2] = mClass->mRightSuspensionPos.z;
            mRight.m[3][3] = 1.0f;
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
        mTurret[i]->Render(mWorld, mInverse, mPose, uiFlags, aTurretSave[i]);
    }

    SetupShadow(this, mModel, ucViewport, uiFlags, 0);

    if (mPilot && mClass->mAnimObj && mClass->mAnimFins && !netFrameLock && NetGame::IsEnabled())
    {
        ZephyrPoseStatic<32> kPose;
        mZephyrSkeleton.Open(*mClass->mAnimObj->GetZephyrSkeletonShared(), kPose);
        kPose.Open(mZephyrSkeleton);
        float fPitch = gameObjectInterpolator.GetFloat(mNetUniqueId, 0);
        float fRoll = gameObjectInterpolator.GetFloat(mNetUniqueId, 1);
        float fFrame1 = fRoll + 4.0f;
        float fBlend = (float)fabs((double)fPitch);
        float fFrame2 = fPitch > 0.0f ? fRoll + 1.0f : fRoll + 7.0f;
        mZephyrPoseDyn.SetLoop(0);
        mZephyrPoseDyn.SetFrameNumFloat(fFrame1);
        mZephyrPoseDyn.Update(0.0f);
        kPose.Set(mZephyrPoseDyn, true);
        mZephyrPoseDyn.SetFrameNumFloat(fFrame2);
        mZephyrPoseDyn.Update(0.0f);
        kPose.Blend(mZephyrPoseDyn, fBlend);
        mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
        mPose.ConvertFromZephyrPose(kPose);
    }

    if (mClass->mWheelCount > 0 && !ucViewport)
    {

        RedShadingPose shadingPose;
        shadingPose.m_pFirstEntry = 0;
        shadingPose.m_pModel = 0;
        shadingPose.BindToModel(mModel->GetRedModel(0));
        for (int i = 0; i < mClass->mWheelCount; ++i)
        {
            RedShadingState* pState = shadingPose.CreateShadingState(mClass->mWheelTextureId[i]);
            if (pState)
            {

                RedTextureTransform& xform = pState->GetTextureTransformParam(0x3f0d728a);
                float fU = mWheelTextureU[i];
                float fV = mWheelTextureV[i];
                if (!xform.m_bTransform)
                {
                    xform.m_bTransform = 1;
                    xform.m_Transform[0][0] = 1.0f;
                    xform.m_Transform[1][0] = 0.0f;
                    xform.m_Transform[0][1] = 0.0f;
                    xform.m_Transform[1][1] = 1.0f;
                }
                xform.m_Transform[2][0] = fU;
                xform.m_Transform[2][1] = fV;
            }
        }
        mModel->Render(0, fDist, &mWorld, &mPose, RedColor::WHITE, uiFlags, &shadingPose);
        shadingPose.Reset();
    }
    else
    {
        mModel->Render(ucViewport, fDist, &mWorld, &mPose, RedColor::WHITE, uiFlags, 0);
    }

    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->PostRender(aTurretSave[i]);
    }

    if (sRenderHoverDebug && mPlayerId >= 0)
    {
        for (int i = 0; i < mClass->mNumColliderBodies; ++i)
        {
            PblMatrix mBody = mCollisionModel->GetBody(mClass->mColliderBodyInfo[i].mCollisionBodyId)
                                  ->GetWorldMatrix(mMatrix, 0);
        }
    }
}

// FUNCTION: SWBF2 0x004c33a0
bool EntityHover::PostCollisionUpdate(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }
    if (mPlayerId >= 0)
    {
        ++gDebugCount;
    }

    for (int i = 0; i < mClass->mNumColliderBodies; ++i)
    {
        UpdateColliderBody(i, fDeltaTime);
    }

    mGroundNormal = mMatrix.GetYAxis();
    mGroundRatio -= fDeltaTime;
    if (0.0f > mGroundRatio)
    {
        mGroundRatio = 0.0f;
    }

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

    PblVector3 vLowest = GetModel()->_mModel->_TotalBoundingBox._vMin;
    for (int i = 0; i < mClass->mNumWakeEmitters; ++i)
    {
        FLEffectClass* pEffectClass = 0;
        if (mClass->mWakeEffect)
        {
            pEffectClass = FLEffect::s_EffectClasses.Find(mClass->mWakeEffect);
        }
        if (pEffectClass && fabsf(mControlMove) + fabsf(mControlStrafe) > 0.01f)
        {
            PblMatrix mPrimitive = mCollisionModel->GetLocalMatrix(mClass->mWakeEmitterPrimitives[i], GetPose());
            PblMatrix mWake = mMatrix;
            mWake.m[3][0] = mPrimitive.m[3][0];
            mWake.m[3][1] = vLowest.y;
            mWake.m[3][2] = mPrimitive.m[3][2];
            mWake.m[3][3] = 1.0f;
            if (!mWakeEmitters[i])
            {

                mWakeEmitters[i] = pEffectClass->BuildEffect();
                if (mWakeEmitters[i])
                {
                    mWakeEmitters[i]->AttachEffectToObject(this, mWake, true);
                }
            }
        }
        else if (mWakeEmitters[i])
        {
            mWakeEmitters[i]->DeactivateEffect();
            mWakeEmitters[i] = 0;
        }
    }

    if (mClass->mIsPilotAnimation9Pose)
    {
        mClass->UpdatePilotAnimation(mPilot, &mPose, mMatrix, mVelocity, fDeltaTime, mForceFrameNum1,
                                     mForceFrameNum2, mForceFrameBlend);
    }
    else
    {
        mClass->UpdatePilotAnimation(mPilot, &mPose, mMatrix, mVelocity, fDeltaTime, -1.0f, -1.0f, -1.0f);
    }

    if (Smooth(fDeltaTime))
    {
        const PblVector3& vCenter = GetCollisionCenter();
        const PblVector3& vPos = GetPosition();
        const PblVector3& vSmoothed = GetSmoothedMatrix().GetTranslation();
        SetRenderPos(PblVector3(vSmoothed.x + vCenter.x - vPos.x, vSmoothed.y + vCenter.y - vPos.y,
                                vSmoothed.z + vCenter.z - vPos.z));
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
            gameObjectInterpolator.SetNextFloat(mNetUniqueId, 0, mFinPitchValue);
            gameObjectInterpolator.SetNextFloat(mNetUniqueId, 1, mFinRollValue);
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

// FUNCTION: SWBF2 0x004c3f00
bool EntityHover::ApplyDamage(const DamageDesc& damage, const PblVector3& vDir, const PblVector3& vPos,
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

// FUNCTION: SWBF2 0x004c5e60
const ControllableClass* EntityHover::GetControllableClass() const
{
    return mClass;
}

// FUNCTION: SWBF2 0x004c5e80
ControllableClass* EntityHover::GetControllableClass()
{
    return mClass;
}

// FUNCTION: SWBF2 0x004c5ed0
int EntityHover::GetWeaponCount() const
{
    return mClass->mWeaponCount;
}

// FUNCTION: SWBF2 0x004c5ee0
int EntityHover::UpdateCollisionOnColliderBody(unsigned int uBodyId, float fOverlap, const PblVector3& vNormal)
{
    for (int i = 0; i < mClass->mNumColliderBodies; ++i)
    {
        if (uBodyId == mClass->mColliderBodyInfo[i].mCollisionBodyId)
        {
            PblVector3 vUp = mMatrix.GetYAxis();
            float fUp = vNormal.x * vUp.x + vNormal.y * vUp.y + vNormal.z * vUp.z;
            if (fUp > 0.5f && vNormal.y > 0.5f)
            {
                float fCompress = fUp * fOverlap;
                float fLength = mClass->mColliderBodyInfo[i].mSpringLength;
                if (!(fLength > fCompress))
                {
                    fCompress = fLength;
                }
                float fRatio = fCompress / fLength;
                if (!(mColliderBody[i].mOverlap > fRatio))
                {
                    mColliderBody[i].mOverlap = fRatio;
                }
            }
            return i;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x004c5fd0
void EntityHover::UpdateColliderBody(int iBody, float fDeltaTime)
{
    CollisionBody* pBody = mCollisionModel->GetBody(mClass->mColliderBodyInfo[iBody].mCollisionBodyId);
    if (!pBody)
    {
        return;
    }

    PblMatrix mBody = pBody->GetWorldMatrix(mMatrix, 0);
    if (mClass->mFloatsOnWater)
    {
        float fRadius = static_cast<CollisionBodyPrimitive*>(pBody)->GetRadius();
        float fWaterHeight;
        if (RedWater::GetWaterHeight(mBody.GetTranslation(), fWaterHeight))
        {
            float fDepth = fWaterHeight - (mBody.m[3][1] - fRadius);
            if (fDepth > 0.0f)
            {
                float fRatio = fDepth / mClass->mColliderBodyInfo[iBody].mSpringLength;
                float fCompress = 1.0f > fRatio ? fRatio : 1.0f;
                if (!(mColliderBody[iBody].mOverlap > fCompress))
                {
                    mColliderBody[iBody].mOverlap = fCompress;
                }
                if (fRatio > 1.0f && 0.0f > mVelocity.y)
                {
                    mVelocity.y *= 0.5f;
                    mVelocity.x *= 0.9f;
                    mVelocity.z *= 0.9f;
                    mOmega.x *= 0.9f;
                    mOmega.y *= 0.9f;
                    mOmega.z *= 0.9f;
                }
            }
        }
    }

    if (mColliderBody[iBody].mOverlap > 0.0f)
    {
        if (mClass->mRandomSpring)
        {
            mColliderBody[iBody].mOverlap *= PblRandom::GetGlobalFloat() * 0.1f + 0.9f;
        }
        mGroundRatio = 1.0f;

        const EntityHoverClass::ColliderBodyInfo& info = mClass->mColliderBodyInfo[iBody];
        PblVector3 vUp = mMatrix.GetYAxis();
        float fOverlap = mColliderBody[iBody].mOverlap;
        float fClosing = mVelocity.y * vUp.y + mVelocity.x * vUp.x + mVelocity.z * vUp.z;
        if (!(0.0f > fClosing))
        {
            fClosing = 0.0f;
        }
        float fSpring = info.mVelocitySpringFactor * fOverlap * mClass->mVelocitySpring -
                        info.mVelocityDampFactor * (1.0f - fOverlap) * mClass->mVelocityDamp * fClosing;
        mVelocity.y = vUp.y * fSpring * fDeltaTime + mVelocity.y;
        mVelocity.x = mVelocity.x + vUp.x * fSpring * fDeltaTime;
        mVelocity.z = vUp.z * fSpring * fDeltaTime + mVelocity.z;
        mOmega.x = (info.mOmegaXSpringFactor * fOverlap * mClass->mOmegaXSpring -
                    (1.0f - fOverlap) * mOmega.x * info.mOmegaXDampFactor * mClass->mOmegaXDamp) *
                       fDeltaTime +
                   mOmega.x;
        mOmega.z = (info.mOmegaZSpringFactor * fOverlap * mClass->mOmegaZSpring -
                    mOmega.z * (1.0f - fOverlap) * info.mOmegaZDampFactor * mClass->mOmegaZDamp) *
                       fDeltaTime +
                   mOmega.z;
    }

    mColliderBody[iBody].mNormal.z = 0.0f;
    mColliderBody[iBody].mNormal.y = 0.0f;
    mColliderBody[iBody].mNormal.x = 0.0f;
    mColliderBody[iBody].mOldOverlap = mColliderBody[iBody].mOverlap;
    mColliderBody[iBody].mOverlap = 0.0f;
}

// FUNCTION: SWBF2 0x004c63b0
void EntityHover::ApplyCollisionImpact(float fApproach, const PblVector3& vNormal, CollisionObject* pOther)
{
    GameObject* pObject = pOther->GetGameObject();
    if (pObject && (pObject->IsRtti(EntityHover::rttiHashEntityHover) ||
                    pObject->IsRtti(EntityWalker::rttiHashEntityWalker)))
    {
        return;
    }

    float fUp = mMatrix.m[1][1] * vNormal.y + vNormal.x * mMatrix.m[1][0] + mMatrix.m[1][2] * vNormal.z;
    float fFront = mMatrix.m[2][1] * vNormal.y + vNormal.x * mMatrix.m[2][0] + mMatrix.m[2][2] * vNormal.z;
    if (!(0.0f > fApproach))
    {
        return;
    }
    float fThreshold =
        mClass->mCollisionThreshold[1] * fUp + mClass->mCollisionThreshold[0] + mClass->mCollisionThreshold[2] * fFront;
    if (!(-fThreshold > fApproach))
    {
        return;
    }

    float fExcess = fThreshold + fApproach;
    float fDamage = (mClass->mCollisionScale[1] * fUp + mClass->mCollisionScale[0] +
                     mClass->mCollisionScale[2] * fFront) *
                    fExcess * fExcess;
    if (pOther->GetGameObject())
    {
        fDamage = pOther->GetGameObject()->GetGameObjectClass()->GetCollisionInflict() * fDamage;
    }
    if (!(fDamage > 0.0f))
    {
        return;
    }

    FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(), pOther, &GetPosition());
    PlayImpactEffects(&mMatrix.GetTranslation(), &mVelocity);
    PblVector3 vPos = mMatrix.GetTranslation();
    PblVector3 vVel = mVelocity;
    VehicleEngine::Collision(mClass, vPos, vVel);

    DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
    if (pOther->GetGameObject() && pOther->GetGameObject()->GetControllable())
    {
        owner.mCharacter = pOther->GetGameObject()->GetControllable()->mCharacter;
        owner.mGameObject = pOther->GetGameObject();
        owner.mTeam = pOther->GetGameObject()->GetTeam();
        owner.mClass = pOther->GetGameObject()->GetGameObjectClass();
    }
    Damageable::ApplyDamage(DamageDesc(owner, fDamage), 0xffffffff);
}

// FUNCTION: SWBF2 0x004c66a0
bool EntityHover::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                    Restrictor* pRestrictor)
{
    if (pOther->GetCollisionType() >= GetCollisionType())
    {
        PblVector3 vRelVel = mVelocity;
        if (pOther->GetGameObject())
        {
            const PblVector3& vOtherVel = pOther->GetGameObject()->GetVelocity();
            vRelVel.x -= vOtherVel.x;
            vRelVel.y -= vOtherVel.y;
            vRelVel.z -= vOtherVel.z;
        }
        const PblVector3& vNormal = pResult->mSeparationNormal;
        float fApproach = vNormal.y * vRelVel.y + vRelVel.x * vNormal.x + vNormal.z * vRelVel.z;

        if (pOther->GetCollisionType() != COLL_SOFT)
        {
            if (pOther->GetCollisionType() == COLL_RIGID)
            {
                ApplyCollisionImpact(fApproach, vNormal, pOther);
            }
            else
            {
                int iBody = UpdateCollisionOnColliderBody(pResult->mColliderId, pResult->mOverlap, vNormal);
                if (iBody >= 0)
                {
                    float fSpring = mClass->mColliderBodyInfo[iBody].mSpringLength;
                    if (fSpring >= pResult->mOverlap)
                    {
                        return false;
                    }
                    ApplyCollisionImpact(fApproach, vNormal, pOther);
                    if (0.0f > fApproach && IsAlive())
                    {
                        mVelocity.x -= vNormal.x * fApproach;
                        mVelocity.y -= vNormal.y * fApproach;
                        mVelocity.z -= vNormal.z * fApproach;
                        mVelocity.x -= vNormal.x * fApproach * 0.1f;
                        mVelocity.y -= 0.0f;
                        mVelocity.z -= vNormal.z * fApproach * 0.1f;
                    }
                    RestrictObject(vNormal, pResult->mOverlap - fSpring, pRestrictor);
                    if (pRestrictor)
                    {
                        pRestrictor->AddRestriction(vNormal);
                    }
                    return true;
                }
            }

            if (0.0f > fApproach && IsAlive())
            {
                float fFriction = 0.25f;
                if (mClass->mSetAltitude == 0.0f)
                {
                    float fUp = vNormal.y * mMatrix.m[1][1] + vNormal.x * mMatrix.m[1][0] +
                                vNormal.z * mMatrix.m[1][2];
                    if (0.0f > fUp)
                    {
                        fUp = 0.0f;
                    }
                    fFriction = (1.0f - fUp * fUp * fUp * fUp) * 0.25f;
                }
                float fBounce = fApproach * -1.125f;
                if (pOther->GetCollisionType() == GetCollisionType())
                {
                    float fMass = mMass;
                    float fOtherMass = pOther->mMass;
                    float fClosing = -(mVelocity.y * vNormal.y + vNormal.x * mVelocity.x + mVelocity.z * vNormal.z);
                    if (!(fClosing > 0.0f))
                    {
                        fClosing = 0.0f;
                    }
                    float fOtherClosing = 0.0f;
                    if (pOther->GetGameObject())
                    {
                        const PblVector3& vOtherVel = pOther->GetGameObject()->GetVelocity();
                        fOtherClosing = vOtherVel.y * vNormal.y + vOtherVel.x * vNormal.x + vOtherVel.z * vNormal.z;
                        if (!(fOtherClosing > 0.0f))
                        {
                            fOtherClosing = 0.0f;
                        }
                    }
                    float fMine = (fClosing * 0.5f + 1.0f) * fOtherMass;
                    fBounce = fMine / ((fOtherClosing * 0.5f + 1.0f) * fMass + fMine) * fBounce;
                }

                mVelocity = vRelVel;
                mVelocity.x -= vNormal.x * fApproach;
                mVelocity.y -= vNormal.y * fApproach;
                mVelocity.z -= vNormal.z * fApproach;
                float fSlide = fBounce * fFriction /
                               sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                                     mVelocity.z * mVelocity.z);
                if (!(1.0f > fSlide))
                {
                    fSlide = 1.0f;
                }
                mVelocity.x -= fSlide * mVelocity.x;
                mVelocity.y -= mVelocity.y * fSlide;
                mVelocity.z -= mVelocity.z * fSlide;
                float fPush = fBounce + fApproach;
                mVelocity.x = vNormal.x * fPush + mVelocity.x;
                mVelocity.y = vNormal.y * fPush + mVelocity.y;
                mVelocity.z = vNormal.z * fPush + mVelocity.z;
                if (pOther->GetGameObject())
                {
                    const PblVector3& vOtherVel = pOther->GetGameObject()->GetVelocity();
                    mVelocity.x = vOtherVel.x + mVelocity.x;
                    mVelocity.y = vOtherVel.y + mVelocity.y;
                    mVelocity.z = vOtherVel.z + mVelocity.z;
                }

                if (mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z >
                        mClass->mForwardSpeed * 0.0001f * mClass->mForwardSpeed &&
                    mClass->mSuspensionCenterHash)
                {
                    float fUp = mMatrix.m[1][1] * vNormal.y + mMatrix.m[1][0] * vNormal.x +
                                mMatrix.m[1][2] * vNormal.z;
                    CollisionObjectType eTerrain = COLL_TERRAIN;
                    if (mCollisionModel->GetCollisionMask(eTerrain).IsSet(pResult->mReceiverId))
                    {
                        mSuspensionVelocity += fUp * fApproach;
                    }
                    else
                    {
                        mSuspensionVelocity -= fUp * fApproach;
                    }
                    if (0.0f >= mSuspensionVelocity)
                    {
                        mNextSuspensionOffset = mClass->mMaxSuspensionOffset;
                    }
                    else
                    {
                        mNextSuspensionOffset = mClass->mMidSuspensionOffset * 2.0f - mClass->mMaxSuspensionOffset;
                    }
                }
            }

            if (0.0f >= mClass->mSetAltitude && vNormal.y > 0.5f)
            {
                mGroundNormal = vNormal;
                mGroundRatio = 1.0f;
            }
        }
    }

    bool bResult = CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
    if (pRestrictor && bResult)
    {
        pRestrictor->AddRestriction(pResult->mSeparationNormal);
    }
    return bResult;
}

// FUNCTION: SWBF2 0x004c7670
ControllableDestination::ControllableDestination(const ControllableDestination& other)
    : mHasDest(other.mHasDest),
      mPos(other.mPos),
      mOriginalPos(other.mOriginalPos),
      mDirection(other.mDirection),
      mSafeHeightEnforce(other.mSafeHeightEnforce),
      mTaperFactor(other.mTaperFactor),
      mMaxSpeed(other.mMaxSpeed)
{
}

// FUNCTION: SWBF2 0x004c6ef0
bool EntityHover::UpdateIndirect(float fDeltaTime)
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

    ControllableDestination dest(lowLevel.mDest);
    float fThrustScale = 1.0f;
    if (dest.mHasDest && GetGameObjectClass()->mAIType != ConnectivityGraph::CGT_FLAG_HUGE)
    {
        const PblVector3& vVel = static_cast<GameObject*>(mCtrl->mOwner->GetEntity())->GetVelocity();
        float fLook = sqrtf(vVel.y * vVel.y + vVel.x * vVel.x + vVel.z * vVel.z) * 1.5f;
        if (fLook > 5.0f)
        {
            PblVector3 vHitNormal(0.0f, 0.0f, 1.0f);
            CollisionObject* pHit;
            const GameObject* pIgnore[2];
            pIgnore[0] = this;
            pIgnore[1] = GetControllablePilot(this)->GetGameObject();
            PblVector3 vDest = dest.mPos;
            PblVector3 vPos = GetMatrix().GetTranslation();
            PblVector3 vToDest(vDest.x - vPos.x, 0.0001f, vDest.z - vPos.z);
            PblVector3 vDir;
            D3DXVec3Normalize(&vDir, &vToDest);
            PblVector3 vStart(vPos.x + 0.0f, vPos.y + 1.0f, vPos.z + 0.0f);
            PblVector3 vEnd(vDir.x * fLook + vStart.x, vDir.y * fLook + vStart.y, vDir.z * fLook + vStart.z);
            PblVector3 vRay(vEnd.x - vStart.x, vEnd.y - vStart.y, vEnd.z - vStart.z);
            float fLength = sqrtf(vRay.x * vRay.x + vRay.y * vRay.y + vRay.z * vRay.z);
            PblVector3 vRayDir;
            D3DXVec3Normalize(&vRayDir, &vRay);
            if (1.0f > CollisionManager::RayHit(vStart, vRayDir, fLength, pHit, &vHitNormal, pIgnore, 2, 0x43b,
                                                true))
            {
                PblVector3 vSide(-vDir.z, 0.0f, vDir.x);
                PblVector3 vOffset(vSide.x * fLook * 0.25f, vSide.y * fLook * 0.25f, vSide.z * fLook * 0.25f);
                if (vSide.x * vHitNormal.x + vHitNormal.y * vSide.y + vSide.z * vHitNormal.z > 0.0f)
                {
                    vDest.x += vOffset.x;
                    vDest.y += vOffset.y;
                    vDest.z += vOffset.z;
                }
                else
                {
                    vDest.x -= vOffset.x;
                    vDest.y -= vOffset.y;
                    vDest.z -= vOffset.z;
                }
                fThrustScale = 0.5f;
                dest.mPos = vDest;
                dest.mHasDest = true;
            }
        }
    }

    float fStrafe = mControlStrafe;
    float fThrust = mControlMove;
    float fTurn = mControlTurn;
    float fPitch = mControlPitch;
    ControllableHeading heading = lowLevel.mHeading;
    lowLevel.ProcessFire(fDeltaTime, &heading);
    AIUtil::UpdatePhysics(this, fDeltaTime, dest, heading, mClass->mForwardSpeed, mClass->mStrafeSpeed, mClass->mTurnRate,
               mClass->mPitchRate, &fThrust, &fStrafe, &fTurn, &fPitch);
    mControlStrafe = fStrafe;

    float fMove = fThrust;
    if (mClass->mMovingTurnOnly && lowLevel.mInCombat && (float)fabs((double)fTurn) > 0.0001f)
    {
        if (fThrust >= 0.0f)
        {
            if (0.5f > fThrust)
            {
                fMove = 0.5f;
            }
        }
        else if (fThrust > -0.5f)
        {
            fMove = -0.5f;
        }
    }
    fMove *= fThrustScale;

    if (mStuckState == 0 && lowLevel.IsStuck())
    {
        if (GameLoop::GetMissionTime() > mStuckCycle + 15.0f)
        {
            mStuckState = 1;
            mStuckDir = GetMatrix().GetZAxis();
        }
    }
    if (mStuckState == 1)
    {
        const PblVector3& vFront = GetMatrix().GetZAxis();
        if (0.0f > mStuckDir.y * vFront.y + mStuckDir.x * vFront.x + mStuckDir.z * vFront.z)
        {
            mStuckState = 2;
            mStuckTimer = GameLoop::GetMissionTime();
        }
        fMove = -1.0f;
        fTurn = 0.5f;
    }
    if (mStuckState == 2)
    {
        if (GameLoop::GetMissionTime() > mStuckTimer + 3.0f)
        {
            mStuckState = 0;
            mStuckCycle = GameLoop::GetMissionTime();
            lowLevel.mOwner->ForceCommand(UnitCmdInfo(CMD_WAIT, PblHandle<GameObject>(), PblVector3(0.0f, 0.0f, 0.0f),
                                                      PblHandle<GameObject>()));
        }
        fMove = 1.0f;
        fTurn = 0.5f;
    }
    if (0.1f > (float)fabs((double)fMove))
    {
        fTurn *= 0.6f;
    }
    mControlMove = fMove;
    mControlTurn = fTurn;
    mControlPitch = fPitch;
    return true;
}

// FUNCTION: SWBF2 0x004c7f20
float EntityHover::GetSoundRadius() const
{
    float fRadius = 0.0f;
    float fEngine = VehicleEngine::GetMuteDistance();
    if (fEngine > fRadius)
    {
        fRadius = fEngine;
    }
    return fRadius;
}

// FUNCTION: SWBF2 0x004c80f0
bool EntityHover::RenderTrackable(unsigned int uView) const
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

// FUNCTION: SWBF2 0x004c8180
float EntityHover::GetMaxSpeed() const
{
    return mClass->mForwardSpeed;
}

// FUNCTION: SWBF2 0x004c8190
float EntityHover::GetMaxStrafe() const
{
    return mClass->mStrafeSpeed;
}

// FUNCTION: SWBF2 0x004c81a0
int EntityHover::GetReservableSlots(ReserveType eType) const
{
    if (eType == RT_FORMATION)
    {
        return 4;
    }
    return GetControllableList().GetCount();
}

// FUNCTION: SWBF2 0x004c3ee0
ExplosionClass* EntityHover::GetExplosionClass()
{
    return mClass ? mClass->mExplosion : 0;
}

// FUNCTION: SWBF2 0x004c5ea0
float EntityHover::GetAimTurnRate() const
{
    return mClass->mPitchRate;
}

// FUNCTION: SWBF2 0x004c5eb0
void EntityHover::SetWeaponIndex(int iIndex, int iChannel)
{
    mWeaponIndex[iChannel] = iIndex;
}

// FUNCTION: SWBF2 0x004c6ed0
CameraTrackSetting* EntityHover::GetCameraTrackSettings()
{
    return &mClass->mCameraSettings;
}

// FUNCTION: SWBF2 0x004c6ee0
const PblVector3& EntityHover::GetAttachOffset() const
{
    return mClass->mAttachOffset;
}

// FUNCTION: SWBF2 0x004c8160
CollisionModel* EntityHover::GetDefaultPilotCollision()
{
    CollisionModel* pModel = &mClass->mDefaultPilotCollision;
    return pModel->GetNumBodies() > 0 ? pModel : 0;
}

// FUNCTION: SWBF2 0x004c7ee0
void EntityHover::SetupCameraZoomFactor(float& rZoom)
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

// FUNCTION: SWBF2 0x004c7cd0
void EntityHover::SetupCameraFOV(float& rFOV)
{
    if (!IsAlive())
    {
        rFOV = GetDefaultFOV();
        return;
    }

    const bool bFirstPerson = mTargetInfo.mIsAiming || (mTracker && mTracker->IsFirstPersonView());
    const ControllableClass* pSettings = GetControllableClass();
    const float fNormalFOV = bFirstPerson ? pSettings->mFirstPersonFOV : pSettings->mThirdPersonFOV;
    const float fForwardSpeed = mVelocity.x * mMatrix.m[2][0]
                              + mVelocity.y * mMatrix.m[2][1]
                              + mVelocity.z * mMatrix.m[2][2];
    if (fForwardSpeed <= mClass->mForwardSpeed || mClass->mBoostSpeed <= mClass->mForwardSpeed ||
        mClass->mBoostFOV == 0.0f)
    {
        rFOV = fNormalFOV;
        return;
    }
    float fBlend = (fForwardSpeed - mClass->mForwardSpeed) /
                   (mClass->mBoostSpeed - mClass->mForwardSpeed);
    if (fBlend < 0.0f)
    {
        fBlend = 0.0f;
    }
    else if (fBlend > 1.0f)
    {
        fBlend = 1.0f;
    }
    rFOV = (1.0f - fBlend) * fNormalFOV + fBlend * mClass->mBoostFOV;
}

// FUNCTION: SWBF2 0x004c7bc0
void EntityHover::SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix)
{
    if (NetGame::IsEnabled() && mPlayerId >= 0 && mClass->mBlurEffect > 0.0f)
    {
        const float fSpeed = sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                                   mVelocity.z * mVelocity.z);
        if (fSpeed > mClass->mBlurStartV * mClass->mForwardSpeed)
        {
            MotionBlurEffect::Start(0, mClass->mBlurEffect, CameraManager::Instance()->mUpdatedCameraId);
        }
    }
    if (IsAlive() && (mTargetInfo.mIsAiming || (mTracker && mTracker->IsFirstPersonView())))
    {
        int iFirstPerson = GetFirstPersonIndex();
        if (iFirstPerson >= 0 && !mTargetInfo.mIsAiming)
        {
            FirstPerson::UpdateAimer(iFirstPerson, rMatrix);
        }
    }
}

// FUNCTION: SWBF2 0x004c7800
void EntityHover::SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime)
{
    PblMatrix mTarget = GetSmoothedMatrix();
    if (IsAlive() && (mTargetInfo.mIsAiming || (mTracker && mTracker->IsFirstPersonView())))
    {
        const PblVector3 vBack(-mEyeDir.x, -mEyeDir.y, -mEyeDir.z);
        rMatrix = DirectionMatrix(vBack, mTarget.GetYAxis(), mEyePoint);
        int iFirstPerson = GetFirstPersonIndex();
        if (iFirstPerson >= 0 && !mTargetInfo.mIsAiming)
        {
            FirstPerson::UpdateCamera(iFirstPerson, rMatrix);
        }
    }
    else
    {
        if (mClass->mSuspensionCenterHash)
        {
            const float fRise = (mCurrSuspensionOffset - mClass->mMidSuspensionOffset) * 1.75f;
            mTarget.m[3][0] += mTarget.m[1][0] * fRise;
            mTarget.m[3][1] += mTarget.m[1][1] * fRise;
            mTarget.m[3][2] += mTarget.m[1][2] * fRise;
        }
        mTarget.m[3][3] = 1.0f;
        PblMatrix mUnbank;
        PblMatrix mUnpitch;
        D3DXMatrixRotationZ(&mUnbank, -mBankRollAngle);
        D3DXMatrixMultiply(&mUnbank, &mUnbank, D3DXMatrixRotationX(&mUnpitch, -mBankPitchAngle));
        PblMatrix mBody;
        D3DXMatrixMultiply(&mBody, &mUnbank, &mTarget);
        mClass->mCameraSettings.SetupCameraTrackMatrix(static_cast<Trackable*>(this), rMatrix, fDeltaTime,
                                                        mBody, mEyeDir, 1.0f, 1.0f, this);
    }

    if (!NetGame::IsEnabled() && mPlayerId >= 0 && mClass->mBlurEffect > 0.0f)
    {
        const float fSpeed = sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                                   mVelocity.z * mVelocity.z);
        if (fSpeed > mClass->mBlurStartV * mClass->mForwardSpeed)
        {
            MotionBlurEffect::Start(0, mClass->mBlurEffect, CameraManager::Instance()->mUpdatedCameraId);
        }
    }
}

// FUNCTION: SWBF2 0x004c8030
void EntityHover::DeactivatePhysics()
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

// SYNTHETIC: SWBF2 0x004794d0
// [thunk]:EntityHover::GetEntity`adjustor{12}'

// SYNTHETIC: SWBF2 0x004794e0
// [thunk]:EntityHover::GetGameObject`adjustor{320}'

// SYNTHETIC: SWBF2 0x00479501
// [thunk]:EntityHover::GetGameObject`adjustor{600}'

// SYNTHETIC: SWBF2 0x0047950c
// [thunk]:EntityHover::GetGameObject`adjustor{600}'

// SYNTHETIC: SWBF2 0x00479517
// [thunk]:EntityHover::GetGameObject`adjustor{12}'

// SYNTHETIC: SWBF2 0x00479527
// [thunk]:EntityHover::GetEntity`adjustor{576}'

// SYNTHETIC: SWBF2 0x00479532
// [thunk]:EntityHover::GetEntity`adjustor{576}'

// SYNTHETIC: SWBF2 0x00479587
// [thunk]:EntityHover::GetMatrix`adjustor{564}'

// SYNTHETIC: SWBF2 0x004caaad
// [thunk]:EntityHover::`scalar deleting destructor'`adjustor{576}'

// SYNTHETIC: SWBF2 0x004794c5
// [thunk]:CommandHover::`scalar deleting destructor'`adjustor{148}'
// SYNTHETIC: SWBF2 0x004794d8
// [thunk]:EntityHover::GetEntity`adjustor{12}'
// SYNTHETIC: SWBF2 0x004794eb
// [thunk]:EntityHover::GetGameObject`adjustor{320}'
// SYNTHETIC: SWBF2 0x0047951f
// [thunk]:EntityHover::GetGameObject`adjustor{12}'
// SYNTHETIC: SWBF2 0x0047953d
// [thunk]:EntityHover::GetControllable`adjustor{600}'
// SYNTHETIC: SWBF2 0x00479548
// [thunk]:EntityHover::GetControllable`adjustor{600}'
// SYNTHETIC: SWBF2 0x00479553
// [thunk]:CommandHover::Kill`adjustor{256}'
// SYNTHETIC: SWBF2 0x0047955e
// [thunk]:CommandHover::`scalar deleting destructor'`adjustor{320}'
// SYNTHETIC: SWBF2 0x00479569
// [thunk]:CommandHover::`scalar deleting destructor'`adjustor{600}'
// SYNTHETIC: SWBF2 0x00479574
// [thunk]:CommandHover::`scalar deleting destructor'`adjustor{12}'
// SYNTHETIC: SWBF2 0x0047957c
// [thunk]:CommandHover::`scalar deleting destructor'`adjustor{512}'
// SYNTHETIC: SWBF2 0x00479592
// [thunk]:CommandHover::`scalar deleting destructor'`adjustor{576}'
// SYNTHETIC: SWBF2 0x004caa79
// [thunk]:EntityHover::Kill`adjustor{256}'
// SYNTHETIC: SWBF2 0x004caa84
// [thunk]:EntityHover::`scalar deleting destructor'`adjustor{320}'
// SYNTHETIC: SWBF2 0x004caa8f
// [thunk]:EntityHover::`scalar deleting destructor'`adjustor{600}'
// SYNTHETIC: SWBF2 0x004caa9a
// [thunk]:EntityHover::`scalar deleting destructor'`adjustor{12}'
// SYNTHETIC: SWBF2 0x004caaa2
// [thunk]:EntityHover::`scalar deleting destructor'`adjustor{512}'
// SYNTHETIC: SWBF2 0x004caab8
// [thunk]:EntityHover::`scalar deleting destructor'`adjustor{148}'
