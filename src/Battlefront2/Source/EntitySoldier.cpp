#include "RedEngineFL/Util/RedCommandConsole.h"

#include <float.h>
#include "PebbleFL/Common/PblVector.h"
#include <math.h>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PauseMenu.h"

#include "EntitySoldier.h"
#include "EnergyBar.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "CollisionObject.h"
#include "EventInfo.h"
#include "GameModel.h"
#include "WeaponCannonInstance.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "AnimatedAddon.h"
#include "CameraManager.h"
#include "ChaseCamera.h"
#include "DeathCamera.h"
#include "Explosion.h"
#include "FLEffect.h"
#include "FirstPerson.h"
#include "SoldierAnimatorClass.h"
#include "SoldierAnimatorLowRes.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "Zephyr/Source/ZephyrSkeletonShared.h"
#include "CollisionBody.h"
#include "CollisionPrimitive.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblBitVector.h"
#include "Character.h"
#include "EntityCarrier.h"
#include "EntityRemoteTerminal.h"
#include "MedalsMgr.h"
#include "OrdnanceGrapplingHook.h"
#include "PassengerSlot.h"
#include "PlayerController.h"
#include "TargetManager.h"
#include "TentacleSimulator.h"
#include "UnitMobileController.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "CollisionManager.h"
#include "CommandPost.h"
#include "CommandPostManager.h"
#include "GameLoop.h"
#include "Restrictor.h"
#include "ToolTips.h"
#include "Tracker.h"
#include "WeaponInvisibility.h"
#include "Controller.h"
#include "FoleyFXSoldier.h"
#include "SoldierAnimation.h"
#include "SoldierAnimationData.h"
#include "Team.h"
#include "UnitController.h"
#include "WeaponMeleeInstance.h"
#include "Cloth.h"
#include "FlagItem.h"
#include "WaterEffect.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Application/pc/pcRedDropShadow.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedStencilShadow.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedShadingState.h"
#include "RedEngineFL/Util/RedSceneManager.h"
#include "BonusManager.h"
#include "ControllerManager.h"
#include "EntityHover.h"
#include "EntityWalker.h"
#include "EventManager_Character.h"
#include "LockOnManager.h"
#include "MessageDisplay.h"
#include "NetComm.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "FLInputManager.h"
#include "MotionBlurEffect.h"
#include "ScopeDisplay.h"
#include "WeaponRepair.h"

#include "EventManager_Object.h"
#include "StatusDisplay.h"

// SYNTHETIC: SWBF2 0x004f5c90
// EntitySoldierClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004f66f0
EntitySoldierClass::~EntitySoldierClass()
{
    delete mAddon;
}

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV,
                                                         const PblMatrix* pM);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

PblMatrix NormalMatrix(const PblVector3& vDir, const PblVector3& vNormal, const PblVector3& vPos);

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x01ebc054
// SYNTHETIC: SWBF2 0x00402d60
// `dynamic initializer for 'EntitySoldier::rttiHashEntitySoldier''
PblHash EntitySoldier::rttiHashEntitySoldier("EntitySoldier");
char EntitySoldier::rttiNameEntitySoldier[] = "EntitySoldier";

// GLOBAL: SWBF2 0x01ebc100
// SYNTHETIC: SWBF2 0x00402d40
// `dynamic initializer for 'EntitySoldier::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766fb0
// `dynamic atexit destructor for 'EntitySoldier::sMemoryPool''
MemoryPool EntitySoldier::sMemoryPool("EntitySoldier", 0xfd0);

// GLOBAL: SWBF2 0x01ebc0f8
// SYNTHETIC: SWBF2 0x00402d70
// `dynamic initializer for 'EntitySoldierClass::rttiHashEntitySoldierClass''
PblHash EntitySoldierClass::rttiHashEntitySoldierClass("EntitySoldierClass");
static char rttiNameEntitySoldierClass[] = "EntitySoldierClass";

// GLOBAL: SWBF2 0x01ebc0a8
// SYNTHETIC: SWBF2 0x00402db0
// `dynamic initializer for 'AcklayData::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766fc0
// `dynamic atexit destructor for 'AcklayData::sMemoryPool''
MemoryPool AcklayData::sMemoryPool("AcklayData", 0x13d0);

// GLOBAL: SWBF2 0x01e51e04
static int sWeaponSection;
// GLOBAL: SWBF2 0x01e51e08
static int sCurrentCameraZoomed;
// GLOBAL: SWBF2 0x01e51e0c
static int sCurrentCameraStance;
// GLOBAL: SWBF2 0x01e51e14
static int sCurrentCameraSplit;
// GLOBAL: SWBF2 0x01e51e18
static int sCurDropItem;

// GLOBAL: SWBF2 0x01e51dd5
static bool gMidgetTime;

ZephyrSkeletonShared* SoldierAnimatorLowResClass::GetDefaultSkeleton()
{
    return s_pHead ? s_pHead->m_pZephyrSkeletonShared : 0;
}

// SYNTHETIC: SWBF2 0x0043cf20
// Aimer::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004de010
bool EntitySoldier::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntitySoldier || GameObject::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004de050
unsigned int EntitySoldier::GetDerivedRtti() const
{
    return rttiHashEntitySoldier;
}

// FUNCTION: SWBF2 0x004de060
const char* EntitySoldier::GetDerivedRttiName() const
{
    return rttiNameEntitySoldier;
}

// FUNCTION: SWBF2 0x004de080
const RedPose* EntitySoldier::GetPose() const
{
    return mAcklayData ? mAcklayData->GetPose() : 0;
}

// FUNCTION: SWBF2 0x004de0a0
bool EntitySoldier::CollideTrisWithAllPrimitives() const
{
    return true;
}

// FUNCTION: SWBF2 0x004de0b0
GameObject* EntitySoldier::GetGameObject()
{
    return this;
}

// SYNTHETIC: SWBF2 0x004fab77
// [thunk]:EntitySoldier::GetGameObject const adjustor{320}
// SYNTHETIC: SWBF2 0x004faba0
// [thunk]:EntitySoldier::GetGameObject const adjustor{12}
// FUNCTION: SWBF2 0x004de0c0
const GameObject* EntitySoldier::GetGameObject() const
{
    return this;
}

// FUNCTION: SWBF2 0x004de0d0
Entity* EntitySoldier::GetEntity()
{
    return this;
}

// SYNTHETIC: SWBF2 0x004fab64
// [thunk]:EntitySoldier::GetEntity const adjustor{12}
// FUNCTION: SWBF2 0x004de0e0
const Entity* EntitySoldier::GetEntity() const
{
    return this;
}

// FUNCTION: SWBF2 0x004de0f0
const PblMatrix& EntitySoldier::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x004de100
const PblVector3& EntitySoldier::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x004de110
void EntitySoldier::SetVelocity(const PblVector3& vVelocity)
{
    mVelocity = vVelocity;
}

// FUNCTION: SWBF2 0x004de130
void EntitySoldier::SetPosition(const PblVector3& vPosition)
{
    mMatrix.m[3][0] = vPosition.x;
    mMatrix.m[3][1] = vPosition.y;
    mMatrix.m[3][2] = vPosition.z;
    mMatrix.m[3][3] = 1.0f;

    PblVector3 center;
    D3DXVec3TransformCoord(&center, &mModel->_mModel->_BoundingSphere.GetCenter(), &mMatrix);
    SetCollisionCenter(center);
    RecalculateAABB();
}

// FUNCTION: SWBF2 0x004de1d0
void EntitySoldier::SetMatrix(const PblMatrix& matrix)
{
    mMatrix = matrix;

    PblVector3 center;
    D3DXVec3TransformCoord(&center, &mModel->_mModel->_BoundingSphere.GetCenter(), &mMatrix);
    SetCollisionCenter(center);
    RecalculateAABB();
}

// FUNCTION: SWBF2 0x004de280
TargetInfo* EntitySoldier::GetTargetInfo()
{
    return &mTargetInfo;
}

// FUNCTION: SWBF2 0x004de290
Trackable* EntitySoldier::GetTrackable()
{
    return this;
}

// FUNCTION: SWBF2 0x004de2b0
const Controllable* EntitySoldier::GetControllable() const
{
    return this;
}

// FUNCTION: SWBF2 0x004de2c0
Controllable* EntitySoldier::GetControllable()
{
    return this;
}

// FUNCTION: SWBF2 0x004de2d0
const PlayableEntitySounds* EntitySoldier::GetPlayableEntitySounds() const
{
    return this;
}

// FUNCTION: SWBF2 0x004de2e0
PlayableEntitySounds* EntitySoldier::GetPlayableEntitySounds()
{
    return this;
}

// FUNCTION: SWBF2 0x004de2f0
int EntitySoldier::GetWeaponIndex(int iChannel) const
{
    return mWeaponIndex[iChannel];
}

// FUNCTION: SWBF2 0x004de310
int EntitySoldier::GetActiveWeaponChannel()
{
    return mActiveChannel;
}

// FUNCTION: SWBF2 0x004de320
Weapon* EntitySoldier::GetWeaponPtr(int iIndex) const
{
    return mWeapon[iIndex];
}

// FUNCTION: SWBF2 0x004de340
EnergyBar* EntitySoldier::GetEnergyBar()
{
    return (EnergyBar*)mEnergyBar;
}

// FUNCTION: SWBF2 0x004de350
const Aimer* EntitySoldier::GetAimer(int iIndex) const
{
    return &mAimer;
}

// FUNCTION: SWBF2 0x004de360
GameObjectType EntitySoldier::GetGameObjectType()
{
    return GO_SOLDIER;
}

// FUNCTION: SWBF2 0x004de370
EntityType EntitySoldier::GetEntityType() const
{
    return ENT_SOLDIER;
}

// FUNCTION: SWBF2 0x004de380
bool EntitySoldier::IsFirstPersonAvailablePS2() const
{
    return true;
}

// FUNCTION: SWBF2 0x004de390
bool EntitySoldier::IsForcedThirdPerson() const
{
    for (int i = 0; i < 2; ++i)
    {
        int iIndex = mWeaponIndex[i];
        if (iIndex >= 0 && mWeapon[iIndex]->IsMelee())
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x004de3d0
bool EntitySoldier::PostCollision::Update(float fDeltaTime)
{
    return mOwner->PostCollisionUpdate(fDeltaTime);
}

// SYNTHETIC: SWBF2 0x004de3f0
// EntitySoldier::PostCollision::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004de420
EntitySoldier::PostCollision::~PostCollision()
{
}

// FUNCTION: SWBF2 0x004de430
bool EntitySoldierClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntitySoldierClass || GameObjectClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004de470
unsigned int EntitySoldierClass::GetDerivedRtti()
{
    return rttiHashEntitySoldierClass;
}

// FUNCTION: SWBF2 0x004de480
const char* EntitySoldierClass::GetDerivedRttiName()
{
    return rttiNameEntitySoldierClass;
}

// FUNCTION: SWBF2 0x004de490
int EntitySoldierClass::GetWeaponCount()
{
    return mWeaponCount;
}

// FUNCTION: SWBF2 0x004de4a0
const WeaponClass* EntitySoldierClass::GetWeaponClass(int iIndex)
{
    return mWeaponClass[iIndex];
}

// FUNCTION: SWBF2 0x004de4c0
int EntitySoldierClass::GetSoldierType()
{
    return mSoldierType;
}

// FUNCTION: SWBF2 0x004de4d0
const void* EntitySoldierClass::GetControllableClass()
{
    return static_cast<const ControllableClass*>(this);
}

// FUNCTION: SWBF2 0x004de4e0
AI::EventInfo::EventInfo(EventType eType, float fFloat, int iInt)
    : mNextCmd(CMD_NONE, PblHandle<GameObject>(), PblVector3(0, 0, 0), PblHandle<GameObject>()),
      mType(eType),
      mGameObject(),
      mPos(0, 0, 0),
      mFloat(fFloat),
      mInt(iInt),
      mVoidPtr(0)
{
}

// FUNCTION: SWBF2 0x004de570
void SoldierAnimator::ModifyLegMatrixByVehicle(const PblMatrix& lastVehicleMatrix, const PblMatrix& vehicleMatrix)
{
    PblMatrix ma;
    PblMatrix mab;
    D3DXMatrixMultiply(&ma, &mLegMatrix, &lastVehicleMatrix);
    D3DXMatrixMultiply(&mab, &ma, &vehicleMatrix);
    mLegMatrix = NormalMatrix(*(const PblVector3*)mab.m[2], *(const PblVector3*)mLegMatrix.m[1],
                            *(const PblVector3*)mab.m[3]);
}

// FUNCTION: SWBF2 0x004de5f0
bool IsNotFirstPersonState(SoldierState::SoldierState eState)
{
    return eState == SoldierState::FLY || eState == SoldierState::TUMBLE;
}

// FUNCTION: SWBF2 0x004de600
bool EntitySoldier::InNonFirstPersonState()
{
    return IsNotFirstPersonState(mState);
}

// FUNCTION: SWBF2 0x004de620
void AcklayData::Init()
{
    sMemoryPool.Setup(0, sMemoryPool.mSize, sMemoryPool.mGrow);
}

// FUNCTION: SWBF2 0x004de640
AcklayData::AcklayData()
{
    for (int i = 0; i < 4; ++i)
    {
        mFeetElevation[i] = 0.0f;
    }
    mFootSound[0].SetID("acklaystepleft", false, false);
    mFootSound[1].SetID("acklaystepright", false, false);
}

// FUNCTION: SWBF2 0x004de730
AcklayData::~AcklayData()
{
    mZSkeleton.Close();
    mZPoseStatic.Close();
}

// FUNCTION: SWBF2 0x004de7b0
void AcklayData::SetupBasePose(const ZephyrSkeletonShared* pSkeleton)
{
    mZSkeleton.Open(*pSkeleton, mZPoseStatic);
    mZPoseStatic.Open(mZSkeleton);
    mZPoseStatic.SetBasePose();
    mZSkeleton.BuildWorldMatrices(_IdentityMatrix);
    mPose.ConvertFromZephyrPose(mZSkeleton);
}

// FUNCTION: SWBF2 0x004e7bf0
FoleyFX* EntitySoldier::GetFoleyFX() const
{
    const CollisionObject* pCollision = static_cast<const CollisionObject*>(this);
    return pCollision->GetFoleyFX(0xd098618c);
}

// FUNCTION: SWBF2 0x004e7c00
void EntitySoldier::PlayFootstep(float fSpeed, bool bRightFoot, const PblVector3* pPos,
                               const PblVector3* pVel)
{
    bool bRun = false;
    if (mSoldierAnimator)
    {
        switch (mSoldierAnimator->mMoveAnimation)
        {
        case SoldierAnimType::RUN_FORWARD:
        case SoldierAnimType::RUN_BACKWARD:
        case SoldierAnimType::RUN_LEFT:
        case SoldierAnimType::RUN_RIGHT:
            bRun = true;
            break;
        default:
            break;
        }
    }
    else
    {
        bRun = fSpeed >= mClass->mRunSpeed;
    }

    FoleyFXSoldier* pFoley = static_cast<FoleyFXSoldier*>(GetFoleyFX());
    Weapon* pWeapon = mWeapon[mActiveIndex];
    if (bRun)
    {
        if (bRightFoot)
        {
            if (pFoley)
            {
                pFoley->mFootstepRightRun.Play(pPos, pVel, false, true);
            }
            if (pWeapon)
            {
                pWeapon->PlayFoleyFX(3);
            }
        }
        else
        {
            if (pFoley)
            {
                pFoley->mFootstepLeftRun.Play(pPos, pVel, false, true);
            }
            if (pWeapon)
            {
                pWeapon->PlayFoleyFX(2);
            }
        }
    }
    else if (bRightFoot)
    {
        if (pFoley)
        {
            pFoley->mFootstepRightWalk.Play(pPos, pVel, false, true);
        }
        if (pWeapon)
        {
            pWeapon->PlayFoleyFX(1);
        }
    }
    else
    {
        if (pFoley)
        {
            pFoley->mFootstepLeftWalk.Play(pPos, pVel, false, true);
        }
        if (pWeapon)
        {
            pWeapon->PlayFoleyFX(0);
        }
    }
}

// GLOBAL: SWBF2 0x007e329c
float gShockFoleyEnabled = 1.0f;

extern bool netIsLocalTurn;

// FUNCTION: SWBF2 0x004e7d20
void EntitySoldier::UpdateFoleyFX(float fDeltaTime)
{
    const PblVector3* pPos = &mMatrix.GetTranslation();
    mLongFoleySound.Update(&mCurFoleySound, pPos, &mVelocity);

    if (!(mFlags & flags_isGrounded))
    {
        return;
    }
    if (NetGame::IsEnabled() && !(NetGame::IsClient() && netIsLocalTurn))
    {
        return;
    }

    if (mSoundState & 0x10)
    {
        if (gShockFoleyEnabled == 0.0f)
        {
            mSoundState &= ~0x10;
        }
        mClass->mShockSound.Play(pPos, 0, false, true);
    }

    const bool bFirstPerson = mTracker && mTracker->IsFirstPersonView();
    if (mSoldierAnimator && !bFirstPerson)
    {
        return;
    }
    if (!(fDeltaTime > 0.0f) || mPlayerId < 0 || mState == SoldierState::ROLL)
    {
        return;
    }

    const float fSpeed = sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                               mVelocity.z * mVelocity.z);
    const bool bStopped = fSpeed < 0.0001f;
    mSoundState &= ~0xc;
    if (fSpeed > 0.0001f)
    {
        mDistanceMoved += fSpeed * fDeltaTime;
    }
    else
    {
        mDistanceMoved += (float)fabs(mControlTurn) * fDeltaTime * 0.0005f;
    }

    if (!(mSoundState & 3))
    {
        if (mDistanceMoved > mClass->mFootstepStride * 0.5f)
        {
            mClass->mClothingRustleSound.Play(pPos, &mVelocity, false, true);
            if (mFlags & flags_leftStep)
            {
                mSoundState |= 1;
                mFlags &= ~flags_leftStep;
            }
            else
            {
                mSoundState |= 2;
                mFlags |= flags_leftStep;
            }
        }
        return;
    }

    if (!(mDistanceMoved > mClass->mFootstepStride) &&
        !(bStopped && mDistanceMoved > 0.0001f))
    {
        return;
    }

    while (mDistanceMoved > mClass->mFootstepStride)
    {
        mDistanceMoved -= mClass->mFootstepStride;
    }

    if (bStopped)
    {
        mDistanceMoved = 0.0f;
        if (mFlags & flags_leftStep)
        {
            mSoundState |= 1;
            mFlags &= ~flags_leftStep;
        }
        else
        {
            mSoundState |= 2;
            mFlags |= flags_leftStep;
        }
    }

    bool bRightFoot;
    if (mSoundState & 1)
    {
        mSoundState = (mSoundState & ~1) | 4;
        bRightFoot = false;
    }
    else
    {
        mSoundState = (mSoundState & ~2) | 8;
        bRightFoot = true;
    }
    PlayFootstep(fSpeed, bRightFoot, pPos, &mVelocity);
}

// FUNCTION: SWBF2 0x004e8030
void EntitySoldier::UpdateFoleyFX(const RedPose& pose, const PblMatrix& mWorld)
{
    if (!(mFlags & flags_isGrounded))
    {
        return;
    }
    if (mTracker && mTracker->IsFirstPersonView())
    {
        return;
    }

    const float fLeftFoot = pose.Find(0x1af77461)->m[3][1];
    const float fRightFoot = pose.Find(0x2f06fa86)->m[3][1];
    PblMatrix mRoot;
    D3DXMatrixMultiply(&mRoot, pose.Find(0x16e27226), &mWorld);
    const PblVector3 vRoot = mRoot.GetTranslation();

    float fWaterHeight = -FLT_MAX;
    bool bInWater = false;
    const PblVector3& vVelocity = GetVelocity();
    if (vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z > 4.5f)
    {
        if (RedWater::GetWaterHeight(GetPosition(), fWaterHeight) &&
            fWaterHeight > GetPosition().y)
        {
            bInWater = vRoot.y + 0.5f > fWaterHeight;
        }
    }

    const PblVector3* pPos = &mWorld.GetTranslation();
    mSoundState &= ~0xc;

    if (mLeftFootElevation - fLeftFoot > 0.01f)
    {
        if (!(mSoundState & 1))
        {
            mSoundState |= 1;
            mClass->mClothingRustleSound.Play(pPos, &mVelocity, false, true);
        }
    }
    else if ((mSoundState & 1) && fLeftFoot - mLeftFootElevation > 0.01f)
    {
        mSoundState &= ~1;
        if (mClass->mMinFootHeight > mLeftFootElevation)
        {
            PlayFootstep(0.0f, false, pPos, &mVelocity);
            mSoundState |= 4;
            if (!NetGame::IsEnabled() || (NetGame::IsClient() && netIsLocalTurn))
            {
                if (RedTerrain::_bEnable)
                {
                    PblMatrix mFoot;
                    D3DXMatrixMultiply(&mFoot, pose.Find(0x1af77461), &mWorld);
                    RedTerrain::GetInterpolatedHeight(mFoot.m[3][0], mFoot.m[3][2]);
                }
                if (bInWater)
                {
                    PblVector3 vDir(GetVelocity().x, 0.0f, GetVelocity().z);
                    PblVector3 vNormal;
                    D3DXVec3Normalize(&vNormal, &vDir);
                    WaterEffect::AddDirectionalWaterEffect(mClass->mWakeWaterSplashEffect,
                                                           GetPosition(), vNormal);
                }
            }
        }
    }

    if (mRightFootElevation - fRightFoot > 0.01f)
    {
        if (!(mSoundState & 2))
        {
            mSoundState |= 2;
            mClass->mClothingRustleSound.Play(pPos, &mVelocity, false, true);
        }
    }
    else if ((mSoundState & 2) && fRightFoot - mRightFootElevation > 0.01f)
    {
        mSoundState &= ~2;
        if (mClass->mMinFootHeight > mRightFootElevation)
        {
            PlayFootstep(0.0f, true, pPos, &mVelocity);
            mSoundState |= 8;
            if (!NetGame::IsEnabled() || (NetGame::IsClient() && netIsLocalTurn))
            {
                if (RedTerrain::_bEnable)
                {
                    PblMatrix mFoot;
                    D3DXMatrixMultiply(&mFoot, pose.Find(0x2f06fa86), &mWorld);
                    RedTerrain::GetInterpolatedHeight(mFoot.m[3][0], mFoot.m[3][2]);
                }
                if (bInWater)
                {
                    PblVector3 vDir(GetVelocity().x, 0.0f, GetVelocity().z);
                    PblVector3 vNormal;
                    D3DXVec3Normalize(&vNormal, &vDir);
                    WaterEffect::AddDirectionalWaterEffect(mClass->mWakeWaterSplashEffect,
                                                           GetPosition(), vNormal);
                }
            }
        }
    }

    mLeftFootElevation = fLeftFoot;
    mRightFootElevation = fRightFoot;
}

// FUNCTION: SWBF2 0x00424b30
GameObject* ResolveGameObjectHandle(PblHandle<GameObject>& handle)
{
    GameObject* pObject = handle.mObject;
    if (pObject)
    {
        if (pObject->mHandleId == handle.mSavedHandleId)
        {
            return pObject;
        }
        handle.mObject = 0;
        handle.mSavedHandleId = 0;
    }
    return 0;
}

extern PblVector3 gVectorZero;

// GLOBAL: SWBF2 0x007895a4
static const float sCollisionOffset[4] = { 0.9f, 0.65f, 0.25f, 1.1f };
// GLOBAL: SWBF2 0x00789534
static const float sCollisionRadius[3] = { 0.72f, 0.45f, 0.8f };
// GLOBAL: SWBF2 0x00789568
static const float sCollisionHeight[3] = { 1.9f, 1.6f, 0.55f };

// GLOBAL: SWBF2 0x01f9c294
static float sCollisionRadius2[3];
// GLOBAL: SWBF2 0x01e51dd6
static bool sCollisionRadius2Valid;

static const float GRAVITY_FORCE = 18.0f;
static const float IN_AIR_DRAG_FORCE = 0.012f;

// FUNCTION: SWBF2 0x004de850
EntitySoldier::EntitySoldier(EntitySoldierClass* pClass, const EntityDesc& desc)
    : GameObject(pClass, desc, COLL_SOFT, &pClass->mLodData)
    , Controllable(pClass, desc.mPlayerId)
    , PlayableEntitySounds(pClass)
    , VehicleEngine(pClass)
    , mPostCollision(this)
    , mAimer(pClass->mAimerClass, this, &mMatrix, 0, 0)
    , mFlags(0)
    , mDebuffDamageOwner(0, 0, 0, 0, 0, 1.0f)
    , mSmolderDamageOwner(0, 0, 0, 0, 0, 1.0f)
{
    new (mEnergyBar) EnergyBar(&pClass->mEnergyBarClass);

    mStandingOnObj = NULL;
    mSoldierMusic = pClass->mSoldierMusic;
    mAISoldierMusic = pClass->mAISoldierMusic;
    memset(mCollisionMeshCache, 0, sizeof(mCollisionMeshCache));
    mControllableList.AddLast(&this->Controllable::mNode, this);
    mVanishing = false;
    RedSceneObject::SetRenderRadius(_Sphere.w + 1.0f);

    mMatrix = NormalMatrix(desc.mMatrix.GetZAxis(), PblVector3(0.0f, 1.0f, 0.0f),
                         desc.mMatrix.GetTranslation());
    mClass = pClass;
    if (!IsInScene())
    {
        AddToScene();
    }

    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    mFlags |= flags_footprintLeft | flags_isGrounded;
    mGroundNormal.x = 0.0f;
    mGroundNormal.y = 1.0f;
    mGroundNormal.z = 0.0f;
    mPitchValue = -0.07f;
    mPitchOffset = 0.0f;
    mPitchVelocity = 0.0f;
    mKickScale = 0.0f;
    mKickBuild = 0.0f;
    mTurnAngle = 0.0f;
    mPrevZoom = -1.0f;
    mSquadCommandClearTimer = 0.0f;

    for (int i = 0; i < 8; ++i)
    {
        mWeapon[i] = 0;
    }
    m_bPrimaryWeaponIsMelee = 0;
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (!mClass->mWeaponClass[i])
        {
            continue;
        }
        WeaponDesc weaponDesc;
        weaponDesc.mOwner = this;
        weaponDesc.mAimer = &mAimer;
        weaponDesc.mTrigger = 0;
        weaponDesc.mReload = 0;
        weaponDesc.mNumClips = 0;
        weaponDesc.m_pAmmoCounter = 0;
        weaponDesc.m_pEnergyBar = 0;
        if (m_bPrimaryWeaponIsMelee && mClass->mWeaponChannel[i] == 1)
        {
            weaponDesc.mTrigger = &mControlReload;
            weaponDesc.mReload = 0;
        }
        else
        {
            weaponDesc.mTrigger = &mControlFire[mClass->mWeaponChannel[i]];
            weaponDesc.mReload = &mControlReload;
        }
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
        if (mClass->mWeaponChannel[i] == 0 && mWeapon[i]->IsMelee())
        {
            m_bPrimaryWeaponIsMelee = 1;
        }
    }
    for (int iChannel = 0; iChannel < 2; ++iChannel)
    {
        mWeaponIndex[iChannel] = -1;
        for (int i = 0; i < mClass->mWeaponCount; ++i)
        {
            if (mWeapon[i] && mClass->mWeaponChannel[i] == iChannel)
            {
                mWeaponIndex[iChannel] = (char)i;
                break;
            }
        }
    }
    mActiveChannel = 0;
    mActiveIndex = mWeaponIndex[mActiveChannel];
    if (mActiveIndex >= 0)
    {
        mAimer.SetWeapon(mWeapon[mActiveIndex]);
    }

    TargetManager::AddTarget(this, !NetGame::IsEnabled());
    ClearControllable();
    mStanceIndex = 0;
    mLegAngleForward = 1;
    mState = SoldierState::STAND;
    mOldState = SoldierState::STAND;
    mStateTimer = 0.0f;
    mAlertTimer = 0.0f;
    mLegAngle = 0.0f;

    CollisionModel& rModel = mClass->mCollisionModels[0];
    float fScale = mClass->mCollisionRootScale;
    CollisionPrimitive primitive;
    PblMatrix matrix = _IdentityMatrix;
    primitive.SetSphere((fScale > 1.0f ? fScale : 1.0f) * 0.17f);
    matrix.m[3][0] = 0.0f;
    matrix.m[3][1] = fScale * 1.65f;
    matrix.m[3][2] = 0.16f;
    matrix.m[3][3] = 1.0f;
    mCollisionBodyHead.Init(primitive, "coll_head", matrix, 0);
    primitive.SetSphere(fScale * 0.4f);
    matrix.m[3][0] = 0.0f;
    matrix.m[3][1] = fScale * 1.15f;
    matrix.m[3][2] = 0.0f;
    matrix.m[3][3] = 1.0f;
    mCollisionBodyTorso.Init(primitive, "coll_torso", matrix, 0);
    mDynamicCollisionModel.AddBody(&mCollisionBodyHead);
    mDynamicCollisionModel.AddBody(&mCollisionBodyTorso);
    for (int i = 2; i < rModel.GetNumBodies(); ++i)
    {
        mDynamicCollisionModel.AddBody(rModel.GetBody(i));
    }
    mDynamicCollisionModel.SetCollisionMask(COLL_TERRAIN, rModel.GetCollisionMask(COLL_TERRAIN));
    mDynamicCollisionModel.SetCollisionMask(COLL_STATIC, rModel.GetCollisionMask(COLL_STATIC));
    mDynamicCollisionModel.SetCollisionMask(COLL_RIGID, rModel.GetCollisionMask(COLL_RIGID));
    mDynamicCollisionModel.SetCollisionMask(COLL_SOFT, rModel.GetCollisionMask(COLL_SOFT));
    mDynamicCollisionModel.SetCollisionMask(COLL_ORDNANCE, rModel.GetCollisionMask(COLL_ORDNANCE));
    RecalculateAABB();

    mSoldierAnimator = 0;
    mSoldierAnimatorLowRes = 0;
    mModelLowRes = pClass->mModelLowRes;

    int iCamera = (mState == SoldierState::SPRINT) ? 3 : mStanceIndex;
    mEyePoint = (&mClass->mCameraSettings[0][0])[iCamera * 2 + mTargetInfo.mIsAiming].CalcEyePoint(mMatrix);
    mCameraSettingPrev = iCamera;
    mCameraSettingTimer = 0.0f;
    mEyeDir = mMatrix.GetZAxis();
    mFlags &= ~flags_useBinocularsAI;
    m_uiInputLockMask = 0;
    m_bSlide = 0;
    mInputLockTime = 0.0f;
    _uiRenderFlags &= 3;
    NetGame::Created(this);

    m_uiLastDamageDir = BACKWARD;
    m_ePilotAnimation = (SoldierAnimationBank::ANIMATION)-1;
    m_fWeaponAnimTimer = 0.0f;
    mSoundState = 0;
    mFlags = 0;
    mSpeedPrev = 0.0f;
    mDistanceMoved = 0.0f;
    m_uiWeaponAnimIndex = 0;
    m_eSoldierAnimationMap = (SoldierAnimationBank::MAP)0;
    mAnimationHash = 0;
    mLeftFootElevation = 0.0f;
    mRightFootElevation = 0.0f;
    mFlyingInFirstPerson = 0;
    mSkip = 0;
    mDoProneRotation = 0;
    mDrownTime = mClass->mDrownTime;
    mAILastCollisionNormal.x = 0.0f;
    mAILastCollisionNormal.y = 0.0f;
    mAILastCollisionNormal.z = 0.0f;
    mFoliageCollisionTime = 0.0f;
    mForceFrameNum1 = -1.0f;
    mForceFrameNum2 = -1.0f;
    mForceFrameBlend = -1.0f;
    mHook = 0;
    mSkipTime = 0.0f;
    mSkipIndirectTime = 0.0f;
    mLastVehicleMatrix = _IdentityMatrix;
    mLastPilotInfoFrame = 99999999;
    mJetFuel = (mClass->mJetFuelRechargeRate > 0.0f) ? 0.5f : 0.0f;

    mHealthGainEffectHandle = NULL;
    mAmmoGainEffectHandle = NULL;
    mEnergyGainEffectHandle = NULL;
    mBuffHealthEffectHandle = NULL;
    mBuffOffenseEffectHandle = NULL;
    mBuffDefenseEffectHandle = NULL;
    mDebuffEffectHandle = NULL;
    mInvincibleEffectHandle = NULL;
    mFootSlideEffectRightHandle = NULL;
    mFootSlideEffectLeftHandle = NULL;
    mChokeEffectHandle = NULL;
    for (int i = 0; i < 5; ++i)
    {
        mSmolderEffectHandle[i] = NULL;
        mSmolderEffectBone[i] = 0;
        mSmolderEffect[i] = 0;
    }
    mNumFlyGroundCollisions = 0;
    mAvgFlyGroundCollisionNormal = gVectorZero;
    mMarkedForDeath = 0;
    mAttachedToProp = 0;
    mSafeMatrix = mMatrix;
    mCollisionState = COLLSTATE_NONE;

    for (int i = 0; i < 8; ++i)
    {
        mClothPiece[i] = 0;
    }
    for (unsigned int i = 0; i < mModel->mClothCount; ++i)
    {
        EntityClass* pClothClass = EntityClass::Find(mModel->mClothList[i]);
        mClothPiece[i] = pClothClass ? (EntityCloth*)pClothClass->Build(desc) : 0;
    }
    if (mModelLowRes)
    {
        for (unsigned int i = 0; i < mModelLowRes->mClothCount; ++i)
        {
            EntityClass* pClothClass = EntityClass::Find(mModelLowRes->mClothList[i]);
            mClothPiece[mModel->mClothCount + i] = pClothClass ? (EntityCloth*)pClothClass->Build(desc) : 0;
        }
    }

    mCanChangeClass = 0;
    mSmolderVanishDeath = false;
    m_uiInvisibilityAlpha = 0;
    mBuffOffenseTimer = 0.0f;
    mBuffOffenseMult = 0.0f;
    mBuffDefenseTimer = 0.0f;
    mBuffDefenseMult = 0.0f;
    mBuffHealthTimer = 0.0f;
    mBuffHealthRate = 0.0f;
    mDebuffDamageTimer = 0.0f;
    mDebuffDamageRate = 0.0f;
    mSmolderTimer = 0.0f;
    mSmolderDamageRate = 0.0f;
    mPrevRemoteControllable = 0;
    mPrevRemoteGameObj = NULL;

    SetPortalActive(true);
    pvs::IManager::GetInstance().RegisterDynamic(*this);

    if (mClass->m_numTentacles && !TentacleSimulator::sMemoryPool.IsFull())
    {
        mTentacles = new TentacleSimulator(mClass->m_numTentacles, mClass->m_bonesPerTentacle,
                                           mClass->m_tentacleCollType);
    }
    else
    {
        mTentacles = 0;
    }

    if (mClass->m_bIsAcklay)
    {
        mAcklayData = new AcklayData;
        mAcklayData->SetupBasePose(mClass->mSpecialSkeleton);
    }
    else
    {
        mAcklayData = 0;
    }
    mNetJustChangedState = 0;
}

// FUNCTION: SWBF2 0x004dfdd0
void EntitySoldier::Init()
{
    mVanishing = false;
    GameObject::Init();
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mWeapon[i])
        {
            mWeapon[i]->Init();
        }
    }
    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        for (PblList<Controllable>::Itor it(GetControllableList()); it; ++it)
        {
            it->SetupController();
        }
    }
    if (EntitySoldier::sEnableMusic)
    {
        (NetGame::GetJoystickIndex(mPlayerId) >= 0 ? mSoldierMusic : mAISoldierMusic).Play();
    }
    if (EntitySoldier::sEnableAnnouncementVO)
    {

        mClass->mSoldierAnnouncement.Play(NULL, NULL, false, true);
        if (mClass->mSoldierAnnouncement.GetProperties())
        {
            mClass->mSoundDeath.SetID(0u, false, false);
        }
    }
}

// FUNCTION: SWBF2 0x004e11a0
static void ComputeCollisionRadius2()
{
    for (int i = 0; i < 2; ++i)
    {
        float fRadius = sCollisionRadius[i];
        float fHeight = sCollisionHeight[i] + 0.2f;
        sCollisionRadius2[i] = sqrtf(2.0f * fRadius * fRadius + 0.25f * fHeight * fHeight);
    }
    float fProne = sCollisionRadius[2] * 0.3333f;
    sCollisionRadius2[2] = sqrtf(fProne * fProne + 1.6f * 1.6f);
}

// FUNCTION: SWBF2 0x004e1210
void EntitySoldier::RecalculateAABB()
{
    if (mAcklayData)
    {
        SetCollisionCenter(mMatrix.GetTranslation());
        SetCollisionRadius(3.0f);
        Set2DCollisionRadius(3.0f);
        float fRadius = GetCollisionRadius();
        const PblVector3& vCenter = GetCollisionCenter();
        mAABB = PblBox(vCenter.x - fRadius, vCenter.y - fRadius, vCenter.z - fRadius,
                       vCenter.x + fRadius, vCenter.y + fRadius, vCenter.z + fRadius);
        return;
    }

    if (!sCollisionRadius2Valid)
    {
        ComputeCollisionRadius2();
        sCollisionRadius2Valid = true;
    }

    if (mState == SoldierState::PRONE)
    {
        SetCollisionCenter(mMatrix.GetTranslation());
        SetCollisionRadius(sCollisionRadius2[2]);
        Set2DCollisionRadius(sCollisionRadius2[2]);
        float fRadius = GetCollisionRadius();
        const PblVector3& vCenter = GetCollisionCenter();
        mAABB = PblBox(vCenter.x - fRadius, vCenter.y - fRadius, vCenter.z - fRadius,
                       vCenter.x + fRadius, vCenter.y + fRadius, vCenter.z + fRadius);
    }
    else if (mState == SoldierState::JUMP || mState == SoldierState::FALL)
    {
        const PblVector3& vPos = mMatrix.GetTranslation();
        const PblVector3& vUp = mMatrix.GetYAxis();
        SetCollisionCenter(PblVector3(vPos.x + vUp.x, vPos.y + vUp.y, vPos.z + vUp.z));
        SetCollisionRadius(sCollisionRadius2[0]);
        Set2DCollisionRadius(sCollisionRadius2[0]);
    }
    else
    {
        float fOffset = sCollisionOffset[mStanceIndex] + 0.1f;
        const PblVector3& vPos = mMatrix.GetTranslation();
        const PblVector3& vUp = mMatrix.GetYAxis();
        SetCollisionCenter(PblVector3(fOffset * vUp.x + vPos.x, vPos.y + vUp.y * fOffset,
                                      vPos.z + vUp.z * fOffset));
        SetCollisionRadius(sCollisionRadius2[mStanceIndex]);
        Set2DCollisionRadius(sCollisionRadius2[mStanceIndex]);
        float fRadius = sCollisionRadius[mStanceIndex];
        float fHalfHeight = sCollisionHeight[mStanceIndex] * 0.5f + 0.1f;
        const PblVector3& vCenter = GetCollisionCenter();
        mAABB = PblBox(vCenter.x - fRadius, vCenter.y - fHalfHeight, vCenter.z - fRadius,
                       vCenter.x + fRadius, vCenter.y + fHalfHeight, vCenter.z + fRadius);
    }
}

// FUNCTION: SWBF2 0x004e1660
float EntitySoldier::GetSmallestCollisionDim() const
{
    if (mState == SoldierState::CROUCH)
    {
        return 0.9f;
    }
    if (mState == SoldierState::PRONE)
    {
        return 0.275f;
    }

    return 1.1400001f;
}

// FUNCTION: SWBF2 0x004e21f0
const ControllableClass* EntitySoldier::GetControllableClass() const
{
    return mClass;
}

// FUNCTION: SWBF2 0x004e2210
ControllableClass* EntitySoldier::GetControllableClass()
{
    return mClass;
}

// FUNCTION: SWBF2 0x004e2230
SoldierAnimationBank::BANK EntitySoldier::GetSoldierAnimationBank() const
{
    return mClass->mAnimationBank;
}

// FUNCTION: SWBF2 0x004e2290
int EntitySoldier::GetWeaponCount() const
{
    return mClass->mWeaponCount;
}

// FUNCTION: SWBF2 0x004e2300
void EntitySoldier::TurnOffJetEffect()
{
    if (VehicleEngine::IsOn())
    {
        PblVector3 vVelocity = mVelocity;
        PblVector3 vPosition = mMatrix.GetTranslation();
        VehicleEngine::TurnOff(mClass, vPosition, vVelocity);
    }
    if (mJetEffectHandle)
    {
        mJetEffectHandle->StopAndFinish();
        mJetEffectHandle = NULL;
    }
}

extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV,
                                                         const PblMatrix* pM);
extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);

extern bool netFrameLock;
extern bool netIsLocalTurn;
extern bool netIsLateTurn;
extern int netCurMaxPlayers;
extern bool gThreadIsEven;

PblVector3* __cdecl CrossProduct(PblVector3* pResult, const PblVector3* pA, const PblVector3* pB);
namespace VisionManager { void __fastcall AddFakeRayhitResult(UnitController* pUnit, const GameObject* pObject); }

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

// GLOBAL: SWBF2 0x007eb83c
static PblVector3 sEyePointOffset[3] = {
    PblVector3(0.06f, 1.7f, 0.0f), PblVector3(-0.1f, 1.2f, 0.0f), PblVector3(0.06f, 0.6f, 0.4f)
};
// GLOBAL: SWBF2 0x007eb860
static PblVector3 sEyePointRelativeWeaponOffset[3] = {
    PblVector3(-0.12f, -0.1f, 0.25f), PblVector3(-0.12f, -0.05f, 0.25f), PblVector3(-0.12f, -0.2f, 0.15f)
};

// GLOBAL: SWBF2 0x01faa468
bool gPlayerSoldierCollisionOccurred;

extern bool gShowSoldierPolyCount;
namespace GameLoop { extern float sClientDeltaTime; }

bool IsNeverDies(EntityType eType);

// GLOBAL: SWBF2 0x01eaedbe

bool gLowResModelsEnabled;

// FUNCTION: SWBF2 0x004e23d0
void EntitySoldier::Render(unsigned char ucLod, float fFade, unsigned int uiFlags)
{
    bool bOwnView = false;
    if (SpawnDisplay::Instance(FLRenderer::GetCurrentViewport())->mVisible &&
        SpawnDisplay::Instance(FLRenderer::GetCurrentViewport())->mMode == SpawnDisplay::eSPAWNDISPLAY)
    {
        ucLod = 1;
    }
    else if (mPlayerId >= 0 &&
             NetGame::GetJoystickIndex(mPlayerId) == (int)FLRenderer::GetCurrentViewport())
    {
        bOwnView = true;
        ucLod = 0;
    }
    if (mPlayerId >= 0)
    {
        int iSpectator = NetGame::GetSpectator(NetGame::GetLocalIndex(FLRenderer::GetCurrentViewport()));
        if (iSpectator != -1 && iSpectator == mPlayerId)
        {
            ucLod = 0;
        }
    }
    mLastRenderedLod = ucLod;

    float fAlpha = fFade;
    bool bHidden = false;
    if (!RenderTrackable(mPlayerId))
    {
        if (ucLod)
        {
            return;
        }
        fAlpha = 0.0f;
        bHidden = true;
    }

    bool bFirstPersonView = false;
    int iLocal = NetGame::GetLocalIndex(FLRenderer::GetCurrentViewport());
    if (iLocal >= 0 && iLocal < 64 && mTracker)
    {
        bFirstPersonView = Tracker::GetTracker(iLocal)->IsFirstPersonView();
        Character* pLocal = Character::GetPtr(iLocal);
        GameObject* pLocalVehicle = pLocal && pLocal->mVehicle ? pLocal->mVehicle->GetGameObject() : 0;
        if (bFirstPersonView && pLocalVehicle && mCharacter && mCharacter->mVehicle &&
            mCharacter->mVehicle->GetGameObject() == pLocalVehicle)
        {
            return;
        }
    }

    bool bOwnerView = NetGame::GetJoystickIndex(mInitialPlayerId) == (int)FLRenderer::GetCurrentViewport();
    if (bOwnerView && !mAttachedToProp &&
        (mState == SoldierState::ARMED_PASSENGER || mState == SoldierState::PILOT_ANIMATION))
    {
        Tracker* pTracker = Tracker::GetTracker(mPlayerId);
        if (pTracker && pTracker->IsFirstPersonView())
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

    if (mState == SoldierState::HERO_FLEE)
    {
        if (mVanishing)
        {
            return;
        }
        if (mStateTimer > 0.5f)
        {
            if (mStateTimer > 3.1f)
            {
                fAlpha *= 4.0f - mStateTimer;
            }
            if (fAlpha < 0.0f)
            {
                return;
            }
        }
    }
    if (mState == SoldierState::DEAD && !(bOwnerView && !(mSmolderVanishDeath && mSmolderTimer > 0.0f)))
    {
        if (mVanishing)
        {
            return;
        }
        if (mStateTimer > 4.0f)
        {
            return;
        }
        if (mSmolderVanishDeath && mSmolderTimer > 0.0f)
        {
            float fVanish = 0.5f - mStateTimer;
            if (fVanish < 0.0f)
            {
                fVanish = 0.0f;
            }
            fAlpha *= fVanish;
        }
        else if (mStateTimer > 3.0f)
        {
            fAlpha *= 4.0f - mStateTimer;
        }
    }

    bool bStencilShadow = true;
    bool bDropShadow = mState != SoldierState::PILOT_ANIMATION;
    if (m_uiInvisibilityAlpha || (fAlpha < 0.15f && !IsAlive()))
    {
        bStencilShadow = false;
        bDropShadow = false;
    }

    if (mState == SoldierState::JET_HOVER || mState == SoldierState::JET_JUMP)
    {
        TurnOffJetIdleEffect();
        if (mClass->mJetPackEffect && !mJetEffectHandle)
        {
            FLEffectObject* pEffect = mClass->mJetPackEffect->BuildEffect();
            if (pEffect && SoldierAnimatorClass::sInstance)
            {
                pEffect->AttachEffectToObject(this, SoldierAnimatorClass::sInstance->GetJetPackMatrix(), true);
                mJetEffectHandle = pEffect;
            }
        }
    }
    else
    {
        TurnOffJetEffect();
        if (mClass->mJetPackIdleEffect && mJetIdleEffectHandle == 0)
        {
            if (FLEffectObject* pEffect = mClass->mJetPackIdleEffect->BuildEffect())
            {
                pEffect->AttachEffectToObject(this, true);
                mJetIdleEffectHandle = pEffect;
            }
        }
    }

    PblMatrix mWorld = GetSmoothedMatrix();
    PblVector3 vAim = mAimer.GetDirection();
    if (!netFrameLock && NetGame::IsEnabled())
    {
        gameObjectInterpolator.GetMatrix(mNetUniqueId, mWorld);
        gameObjectInterpolator.GetVector(mNetUniqueId, vAim);
    }
    PblMatrix mCamera = RedRenderer::GetCamera()->GetMatrix();

    Controllable* pVehicle = mCharacter ? mCharacter->mVehicle : 0;
    if (pVehicle && NetGame::IsEnabled())
    {
        if (mLastPilotInfoFrame > NetGame::GetTurnNumber())
        {
            return;
        }
        if (NetGame::GetTurnNumber() - mLastPilotInfoFrame > 20)
        {
            return;
        }
        if (GameObject* pObject = pVehicle->GetGameObject())
        {
            PblMatrix mVehicle = pObject->GetSmoothedMatrix();
            if (!netFrameLock && NetGame::IsEnabled() && !IsNeverDies(pObject->GetEntityType()))
            {
                gameObjectInterpolator.GetMatrix(pObject->mNetUniqueId, mVehicle);
            }
            pVehicle->GetPilotMatrix(mVehicle, mWorld);
        }
    }

    if (!bHidden)
    {

        if (!ucLod && !bFirstPersonView)
        {
            const PblVector3& vOffset = mClass->mTargetPointOffset[mStanceIndex];
            float fDepth =
                (mCamera.m[3][1] - (vOffset.y + mWorld.m[3][1])) * mCamera.m[2][1] +
                (mCamera.m[3][0] - (vOffset.x + mWorld.m[3][0])) * mCamera.m[2][0] +
                (mCamera.m[3][2] - (vOffset.z + mWorld.m[3][2])) * mCamera.m[2][2];
            if (fDepth <= 0.0f)
            {
                fDepth = 0.0f;
            }
            if (fDepth < 2.2f)
            {
                float fNear = 0.0f;
                float fFar = 0.0f;
                float fWidth = 0.0f;
                float fHeight = 0.0f;
                RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
                float fClear = fDepth - fNear;
                if (fClear < 0.0f)
                {
                    return;
                }
                if (fAlpha >= 0.06f)
                {
                    if (fClear < 1.2f)
                    {
                        fAlpha = fClear / 1.2f * fAlpha;
                    }
                    if (fAlpha <= 0.06f)
                    {
                        fAlpha = 0.06f;
                    }
                }
            }
        }
        if (!RedSceneManager::InTransparentPhase() &&
            (m_uiInvisibilityAlpha || (!ucLod && fAlpha < 1.0f)))
        {
            RedSceneManager::AddTransparentObject(this);
            return;
        }
    }

    if (mSkip)
    {
        mWorld.m[3][0] = mVelocity.x * mSkipTime + mWorld.m[3][0];
        mWorld.m[3][1] = mVelocity.y * mSkipTime + mWorld.m[3][1];
        mWorld.m[3][2] = mVelocity.z * mSkipTime + mWorld.m[3][2];
        mWorld.m[3][3] = 1.0f;
    }

    float fLight = 1.0f;
    if (RedTerrain::_bEnable && RedTerrain::mTerrainColorDarkeningFlag)
    {
        fLight = RedTerrain::GetInterpolatedLight(mWorld.m[3][0], mWorld.m[3][2]);
    }
    float fBrightness = fLight * 1.2f + 0.3f;
    if (fBrightness >= 1.0f)
    {
        fBrightness = 1.0f;
    }

    GameModel* pModel = (ucLod || (gLowResModelsEnabled && !NetGame::IsEnabled())) ? mModelLowRes : mModel;
    if (!pModel)
    {
        return;
    }

    float fModelAlpha = 1.0f;
    float fAttachAlpha = 1.0f;
    RedShadingPose* pShadingPose = NULL;
    RedShadingPose shadingPose;
    shadingPose.m_pFirstEntry = 0;
    shadingPose.m_pModel = 0;

    if (m_uiInvisibilityAlpha)
    {
        unsigned char ucInvisibility = m_uiInvisibilityAlpha;
        unsigned char ucCloak = 0xff;
        fAttachAlpha = 0.0f;
        if (ucInvisibility < 0x80)
        {
            ucCloak = (unsigned char)(ucInvisibility * 2);
            fAttachAlpha = (255 - ucCloak) * (1.0f / 255.0f);
        }
        else
        {
            if (bOwnView && ucInvisibility > 0xd8)
            {
                ucInvisibility = 0xd8;
            }
            fModelAlpha = (255 - ucInvisibility) / 127.0f;
        }
        pShadingPose = &shadingPose;
        shadingPose.BindToModel(pModel->GetRedModel(0));
        if (RedShadingState* pState = shadingPose.CreateDefaultShadingState(0x4cc5882a))
        {
            RedColor color = RedColor::WHITE;
            color.a = ucCloak;
            pState->GetColorParam(0xbc6f8a5e) = color;
            pState->GetIntParam(0x7802a19a) = 1;
        }
    }

    RedColor colorModel;
    colorModel.a = (unsigned char)(int)(fModelAlpha * fAlpha * 255.0f);
    colorModel.b = colorModel.g = colorModel.r = (unsigned char)(int)(fBrightness * 255.0f);
    float fAttachFade = fAttachAlpha * fAlpha;
    RedColor colorAttach;
    colorAttach.b = colorAttach.g = colorAttach.r = colorModel.r;
    colorAttach.a = (unsigned char)(int)(fAttachFade * 255.0f);

    RedPose pose;

    bool bReflected = false;
    unsigned int uiReflectFlags = 0;
    PblMatrix mReflection;
    if (!ucLod && !(uiFlags & 0x200000) &&
        FLRenderer::IsReflected(*(const PblVector3*)&_Sphere, _Sphere.w, mReflection, false))
    {
        bReflected = true;
        uiReflectFlags = (uiFlags & ~0x10000) | 0x10000100;
        PblMatrix mTemp;
        D3DXMatrixMultiply(&mTemp, &mWorld, &mReflection);
        mReflection = mTemp;
    }

    Weapon* pActive = NULL;
    Weapon* pAnimated = NULL;
    bool bOffhand = false;
    bool bThrow = false;
    if (mActiveIndex >= 0)
    {
        pActive = pAnimated = mWeapon[mActiveIndex];
        if (pActive && !pActive->mClass->mIsOffhand)
        {
            if (pActive->IsMeleeThrow() && mActiveChannel == 1 && mWeaponIndex[0] >= 0 &&
                mWeapon[mWeaponIndex[0]])
            {
                pAnimated = mWeapon[mWeaponIndex[0]];
                bThrow = true;
            }
        }
        else if (mActiveChannel == 1 && mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]])
        {
            pAnimated = mWeapon[mWeaponIndex[0]];
            bOffhand = true;
        }
        else if (mActiveIndex != 0 && mWeapon[0])
        {
            pAnimated = mWeapon[0];
            bOffhand = true;
        }
    }

    SetupLightingState(uiFlags);

    const RedPose* pPose = NULL;
    unsigned int uiModelFlags;
    unsigned int uiPoseFlags;
    if (!ucLod)
    {
        if (!mSoldierAnimator || mSoldierAnimator->GetOwner() != this)
        {
            mSoldierAnimator = SoldierAnimatorClass::sInstance->GetAnimator(0.0f);
            if (mSoldierAnimator)
            {
                mSoldierAnimator->SetNewOwner(0, this, mClass->mSpecialSkeleton,
                                              *(const PblBitVector<32>*)mClass->mUpperBodyAnimMask,
                                              *(const PblBitVector<32>*)mClass->mLowerBodyAnimMask,
                                              mClass->mSkeletonRootScale);
            }
        }
        if (mSoldierAnimatorLowRes && mSoldierAnimatorLowRes->GetFramesSinceUsed() > 1)
        {
            mSoldierAnimatorLowRes->Release(*this);
            mSoldierAnimatorLowRes = NULL;
        }

        if (mSoldierAnimator)
        {
            pPose = &pose;
            mSoldierAnimator->SetAction(
                m_bSlide && mState == SoldierState::STAND ? SoldierState::SLIDE : mState, mWorld,
                mAnimationHash);

            if (pActive)
            {
                mSoldierAnimator->SetWeaponAnimationMap(
                    (SoldierAnimationBank::MAP)pAnimated->mSoldierAnimationMap);
                int eWeaponState = pActive->mState;
                mSoldierAnimator->SetAlert(mAlertTimer > 0.0f);
                bool bFiring = (unsigned int)(eWeaponState - 1) <= 1;
                if (!pActive->IsMelee())
                {
                    if (pActive->mFiredFlag)
                    {
                        if (NetGame::IsEnabled() && !bFiring)
                        {
                            eWeaponState = (pActive != pAnimated) + 1;
                        }
                        if (!bHidden)
                        {
                            pActive->mFiredFlag = 0;
                        }
                    }
                    else if (bFiring)
                    {
                        eWeaponState = 0;
                    }
                }

                if (!pAnimated->IsMelee())
                {
                    mSoldierAnimator->SetWeaponState(eWeaponState, pActive->mStateLimit, bFiring, bOffhand);
                    if ((unsigned int)(eWeaponState - 1) <= 2)
                    {
                        SoldierAnimType::WeaponAnimation eAnim =
                            SoldierAnimatorClass::WeaponStateToAnimation(eWeaponState, bOffhand);
                        if (eAnim != SoldierAnimType::WEAPON_IDLE)
                        {
                            m_uiWeaponAnimIndex =
                                SoldierAnimType::GetAnimIndex((SoldierAnimType::Posture)mStanceIndex, eAnim);
                            m_fWeaponAnimTimer = 1.0f;
                        }
                    }
                }
                else
                {
                    WeaponMelee* pMelee = (WeaponMelee*)pAnimated;
                    if (bOffhand)
                    {
                        m_uiWeaponAnimIndex = pMelee->GetOffhandWeaponMeleeAnimation(eWeaponState);
                        mSoldierAnimator->SetWeaponComboState(
                            eWeaponState, bFiring, pMelee->GetOffhandWeaponMeleeAnimation(eWeaponState), 0xa4,
                            false);
                        m_fWeaponAnimTimer = 0.0f;
                    }
                    else if (bThrow)
                    {
                        m_uiWeaponAnimIndex = pMelee->GetWeaponMeleeThrowAnimation();
                        mSoldierAnimator->SetWeaponComboState(
                            eWeaponState, bFiring, pMelee->GetWeaponMeleeThrowAnimation(), 0xa4, false);
                        m_fWeaponAnimTimer = 0.0f;
                    }
                    else if (eWeaponState != 0)
                    {
                        mSoldierAnimator->SetWeaponComboState(
                            eWeaponState, pMelee->m_bRestartAnimation, pMelee->GetWeaponMeleeAnimation(),
                            pMelee->GetNextWeaponMeleeAnimation(), pMelee->IsFullBodyAnimation());
                        if (eWeaponState == 1)
                        {
                            m_uiWeaponAnimIndex = pMelee->GetWeaponMeleeAnimation();
                        }
                        else if (eWeaponState == 4)
                        {
                            m_uiWeaponAnimIndex = pMelee->GetNextWeaponMeleeAnimation();
                        }
                        m_fWeaponAnimTimer = 0.0f;
                    }
                    else
                    {
                        mSoldierAnimator->SetWeaponComboState(0, false, 0xa4, 0xa4,
                                                              bFiring ? pMelee->IsFullBodyAnimation() : false);
                    }
                }
            }

            const PblVector3* pAim;
            if (mPlayerId < 0)
            {
                pAim = &mEyeDir;
            }
            else
            {
                Controllable* pControllable = mCharacter ? mCharacter->GetControllable() : 0;
                if (pControllable &&
                    pControllable->GetControllableClass()->mAnimatesUnitAsTurret)
                {
                    Weapon* pWeapon = pControllable->GetWeaponPtr(0);
                    pAim = pWeapon ? &pWeapon->mAimer->GetDirection() : &mAimer.GetDirection();
                }
                else
                {
                    pAim = &vAim;
                }
            }
            mSoldierAnimator->SetAimDirection(*pAim);

            mSoldierAnimator->mForceFrameNum1 = mForceFrameNum1;
            mSoldierAnimator->mForceFrameNum2 = mForceFrameNum2;
            mSoldierAnimator->mForceFrameBlend = mForceFrameBlend;
            ZephyrSkeletonShared* pSkeleton = mClass->mSpecialSkeleton;
            if (mForceFrameNum1 >= 0.0f && pSkeleton &&
                pSkeleton != SoldierAnimatorClass::sInstance->GetDefaultSkeleton() &&
                mClass->mSkeletonRootScale != 1.0f)
            {
                int iRoot = pSkeleton->GetJointIdxByCrc(0x16e27226);
                pSkeleton->SetJointScale(iRoot, 1.0f);
                mSoldierAnimator->SetupPose(pose);
                pSkeleton->SetJointScale(iRoot, mClass->mSkeletonRootScale);
            }
            else
            {
                mSoldierAnimator->SetupPose(pose);
            }
            mSoldierAnimator->mForceFrameNum1 = -1.0f;
            mSoldierAnimator->mForceFrameNum2 = -1.0f;
            mSoldierAnimator->mForceFrameBlend = -1.0f;

            if (mTentacles && !mClass->mAddon)
            {
                PblMatrix aTentacleMatrices[20];
                mTentacles->DoTentacles(&pose, GetMatrix(), GetVelocity(), aTentacleMatrices,
                                        (uiFlags & 0x200000) ? 0.0f : -1.0f);
            }

            if (mAcklayData)
            {
                UpdateAcklayFoleyFX(pose, mWorld);
                if (mAcklayData)
                {
                    mAcklayData->mAnimatedPose = pose;
                    mAcklayData->bHasAnimatedPose = true;
                }
            }
            else
            {

                PblMatrix mHeadLook;
                if (mCharacter && mCharacter->mVehicle && !mCharacter->mRemote &&
                    GetObjectData(0) == COLL_ATTACHED && IsAlive())
                {
                    if (const PblMatrix* pHead = pose.Find(0x8afdf5e4))
                    {
                        PblVector3 vDir = mCharacter->mVehicle->mEyeDir;
                        PblVector3 vLocal;
                        PblMatrix mInverse = mMatrix.GetInverseFast();
                        D3DXVec3TransformNormal(&vLocal, &vDir, &mInverse);
                        PblVector3 vUp(pHead->m[1][0], pHead->m[1][1], pHead->m[1][2]);
                        float fDot = vUp.y * vLocal.y + vLocal.x * vUp.x + vUp.z * vLocal.z;
                        if (fDot > -1.0f)
                        {
                            if (fDot < 0.4f)
                            {
                                PblVector3 vPerp(vLocal.x - fDot * vUp.x, vLocal.y - fDot * vUp.y,
                                                 vLocal.z - fDot * vUp.z);
                                PblVector3 vSide;
                                D3DXVec3Normalize(&vSide, &vPerp);
                                float fSin = (float)sqrt(1.0f - 0.4f * 0.4f);
                                vLocal = PblVector3(vUp.x * 0.4f + vSide.x * fSin, vUp.y * 0.4f + vSide.y * fSin,
                                                    vUp.z * 0.4f + vSide.z * fSin);
                            }
                            PblMatrix mDir = DirectionMatrix(vLocal, *(const PblVector3*)&pHead->m[0][0],
                                                             *(const PblVector3*)&pHead->m[3][0]);
                            mHeadLook.m[0][0] = mDir.m[1][0];
                            mHeadLook.m[0][1] = mDir.m[1][1];
                            mHeadLook.m[0][2] = mDir.m[1][2];
                            mHeadLook.m[0][3] = 0.0f;
                            mHeadLook.m[1][0] = mDir.m[2][0];
                            mHeadLook.m[1][1] = mDir.m[2][1];
                            mHeadLook.m[1][2] = mDir.m[2][2];
                            mHeadLook.m[1][3] = 0.0f;
                            mHeadLook.m[2][0] = mDir.m[0][0];
                            mHeadLook.m[2][1] = mDir.m[0][1];
                            mHeadLook.m[2][2] = mDir.m[0][2];
                            mHeadLook.m[2][3] = 0.0f;
                            mHeadLook.m[3][0] = mDir.m[3][0];
                            mHeadLook.m[3][1] = mDir.m[3][1];
                            mHeadLook.m[3][2] = mDir.m[3][2];
                            mHeadLook.m[3][3] = mDir.m[3][3];
                            pose.Remove(0x8afdf5e4);
                            pose.Store(0x8afdf5e4, &mHeadLook);
                        }
                    }
                }
                else
                {
                    UnitController* pUnit = UnitControllerPtr(mCtrl);
                    AILowLevel* pLowLevel = pUnit ? &pUnit->GetLowLevel() : 0;
                    if (pLowLevel && pUnit->mLodState >= 3   &&
                        ((mState == SoldierState::STAND && pLowLevel->GetHeadLookState() != AI::HEAD_LOOK_OFF) ||
                         (mState == SoldierState::CROUCH &&
                          (pLowLevel->GetHeadLookState() == AI::HEAD_LOOK_RIGHT ||
                           pLowLevel->GetHeadLookState() == AI::HEAD_LOOK_LEFT))) &&
                        pLowLevel->CalcHeadLookMatrix(pose.Find(0x8afdf5e4), &mHeadLook))
                    {
                        pose.Remove(0x8afdf5e4);
                        pose.Store(0x8afdf5e4, &mHeadLook);
                    }
                }

                if (mJetEffectHandle)
                {
                    PblMatrix mOffset = *pose.Find(0xe5b51072);
                    mJetEffectHandle->SetLocalOffsetMatrix(mOffset);
                }
                if (mJetIdleEffectHandle)
                {
                    PblMatrix mOffset = *pose.Find(0xe5b51072);
                    mJetIdleEffectHandle->SetLocalOffsetMatrix(mOffset);
                }
                for (int i = 0; i < 5; ++i)
                {
                    if (mSmolderEffectHandle[i])
                    {
                        PblMatrix mOffset = *pose.Find(mSmolderEffectBone[i]);
                        mSmolderEffectHandle[i]->SetLocalOffsetMatrix(mOffset);
                    }
                }

                UpdateFoleyFX(pose, mWorld);

                float fWaterHeight;
                if (RedWater::GetWaterHeight(GetPosition(), fWaterHeight))
                {
                    PblMatrix mFoot = mWorld;
                    if (const PblMatrix* pRightFoot = pose.Find(0x2f06fa86))
                    {
                        D3DXMatrixMultiply(&mFoot, pRightFoot, &mWorld);
                    }
                    PblVector3 vRightFoot = mFoot.GetTranslation();
                    const PblMatrix* pLeftFoot = pose.Find(0x1af77461);
                    if (!pLeftFoot)
                    {
                        pLeftFoot = &_IdentityMatrix;
                    }
                    D3DXMatrixMultiply(&mFoot, pLeftFoot, &mWorld);
                    PblVector3 vLeftFoot = mFoot.GetTranslation();
                    const PblVector3& vVelocity = GetVelocity();
                    float fSpeed2 = vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z;
                    if (fWaterHeight > vRightFoot.y)
                    {
                        if (!(mFlags & flags_rightFootInWater))
                        {
                            mFlags |= flags_rightFootInWater;
                            if (fSpeed2 > 0.0f)
                            {
                                WaterEffect::AddWaterEffect(mClass->mFootWaterSplashEffect, vRightFoot);
                            }
                        }
                    }
                    else
                    {
                        mFlags &= ~flags_rightFootInWater;
                    }
                    if (fWaterHeight > vLeftFoot.y)
                    {
                        if (!(mFlags & flags_leftFootInWater))
                        {
                            mFlags |= flags_leftFootInWater;
                            if (fSpeed2 > 0.0f)
                            {
                                WaterEffect::AddWaterEffect(mClass->mFootWaterSplashEffect, vLeftFoot);
                            }
                        }
                    }
                    else
                    {
                        mFlags &= ~flags_leftFootInWater;
                    }
                }
                else
                {
                    mFlags &= ~(flags_rightFootInWater | flags_leftFootInWater);
                }

                float fDeltaTime = GameLoop::sClientDeltaTime;
                if (mClass->mFootSlideEffect && fDeltaTime > 0.0f)
                {
                    PblMatrix mRightFoot;
                    D3DXMatrixMultiply(&mRightFoot, pose.Find(0x2f06fa86), &mWorld);
                    bool bSlide = false;
                    if (!(mFlags & flags_rightFootInWater) && (mFlags & flags_isGrounded) &&
                        m_vPosFootRight.x * m_vPosFootRight.x + m_vPosFootRight.y * m_vPosFootRight.y +
                                m_vPosFootRight.z * m_vPosFootRight.z > 0.0f)
                    {
                        float fInv = 1.0f / fDeltaTime;
                        float fX = (mRightFoot.m[3][0] - m_vPosFootRight.x) * fInv;
                        float fY = fInv * (mRightFoot.m[3][1] - m_vPosFootRight.y);
                        float fZ = fInv * (mRightFoot.m[3][2] - m_vPosFootRight.z);
                        bSlide = fX * fX + fY * fY + fZ * fZ >= 25.0f;
                    }
                    m_vPosFootRight = mRightFoot.GetTranslation();
                    if (bSlide && mFootSlideEffectRightHandle == 0)
                    {
                        if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mFootSlideEffect))
                        {
                            if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                            {
                                mFootSlideEffectRightHandle = pEffect;
                            }
                        }
                    }
                    if (mFootSlideEffectRightHandle != 0)
                    {
                        mFootSlideEffectRightHandle->AttachEffectToMatrix(&_IdentityMatrix, mRightFoot, false);
                        if (!bSlide)
                        {
                            mFootSlideEffectRightHandle->StopAndFinish();
                            mFootSlideEffectRightHandle = NULL;
                        }
                    }

                    PblMatrix mLeftFoot;
                    D3DXMatrixMultiply(&mLeftFoot, pose.Find(0x1af77461), &mWorld);
                    bSlide = false;
                    if (!(mFlags & flags_leftFootInWater) && (mFlags & flags_isGrounded) &&
                        m_vPosFootLeft.x * m_vPosFootLeft.x + m_vPosFootLeft.y * m_vPosFootLeft.y +
                                m_vPosFootLeft.z * m_vPosFootLeft.z > 0.0f)
                    {
                        float fInv = 1.0f / fDeltaTime;
                        float fX = (mLeftFoot.m[3][0] - m_vPosFootLeft.x) * fInv;
                        float fY = fInv * (mLeftFoot.m[3][1] - m_vPosFootLeft.y);
                        float fZ = fInv * (mLeftFoot.m[3][2] - m_vPosFootLeft.z);
                        bSlide = fX * fX + fY * fY + fZ * fZ >= 25.0f;
                    }
                    m_vPosFootLeft = mLeftFoot.GetTranslation();
                    if (bSlide && mFootSlideEffectLeftHandle == 0)
                    {
                        if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mFootSlideEffect))
                        {
                            if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                            {
                                mFootSlideEffectLeftHandle = pEffect;
                            }
                        }
                    }
                    if (mFootSlideEffectLeftHandle != 0)
                    {
                        mFootSlideEffectLeftHandle->AttachEffectToMatrix(&_IdentityMatrix, mLeftFoot, false);
                        if (!bSlide)
                        {
                            mFootSlideEffectLeftHandle->StopAndFinish();
                            mFootSlideEffectLeftHandle = NULL;
                        }
                    }
                }
            }
        }
        else if (mClass->mFootSlideEffect)
        {
            if (mFootSlideEffectRightHandle != 0)
            {
                mFootSlideEffectRightHandle->StopAndFinish();
                mFootSlideEffectRightHandle = NULL;
            }
            if (mFootSlideEffectLeftHandle != 0)
            {
                mFootSlideEffectLeftHandle->StopAndFinish();
                mFootSlideEffectLeftHandle = NULL;
            }
        }

        AnimatedAddon* pAddon = mClass->mAddon;
        if (pAddon && mState != SoldierState::JET_HOVER && mState != SoldierState::JET_JUMP)
        {
            unsigned int uiAddonFlags = bStencilShadow ? (uiFlags | 0x10000) : (uiFlags & ~0x10000);
            pAddon->Render(mTentacles, GetVelocity(), pPose, mWorld, colorAttach, uiAddonFlags);
            if (bReflected && pPose)
            {
                pAddon->Render(mTentacles, GetVelocity(), pPose, mReflection, colorAttach, uiReflectFlags);
            }
        }

        if (mClass->mChokeEffect)
        {
            if (mState == SoldierState::CHOKE && mChokeEffectHandle == 0)
            {
                if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mChokeEffect))
                {
                    if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                    {
                        mChokeEffectHandle = pEffect;
                    }
                }
            }
            if (mChokeEffectHandle != 0)
            {
                const PblMatrix* pBone = 0;
                if (pPose)
                {
                    pBone = pPose->Find(0xd2afb28a);
                    if (!pBone)
                    {
                        pBone = pPose->Find(0x8afdf5e4);
                    }
                }
                PblMatrix mChoke;
                if (pBone)
                {
                    D3DXMatrixMultiply(&mChoke, pBone, &mWorld);
                }
                else
                {
                    mChoke = mWorld;
                }
                mChokeEffectHandle->AttachEffectToMatrix(&_IdentityMatrix, mChoke, false);
                if (mState != SoldierState::CHOKE)
                {
                    mChokeEffectHandle->StopAndFinish();
                    mChokeEffectHandle = NULL;
                }
            }
        }

        uiModelFlags = uiFlags & ~0x10000;
        if (bStencilShadow && pModel->GetRedModel(0)->_pStencilShadow && RedStencilShadow::GetShadowEnable())
        {
            RedDirectionalLight* pLight = RedLight::GetGlobalDirectionalLight(0);
            if (!pLight->IsShadowCaster())
            {
                pLight = RedLight::GetGlobalDirectionalLight(1)->IsShadowCaster()
                             ? RedLight::GetGlobalDirectionalLight(1)
                             : NULL;
            }
            float fNear, fFar, fWidth, fHeight;
            RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
            const RedCamera* pCamera = RedRenderer::GetCamera();
            float fLength;
            uiModelFlags = uiFlags;
            if (pLight)
            {
                uiModelFlags = uiFlags | 0x10000;
                const PblVector3& vDir = pLight->m_vDirection;
                float fMinLength = _Sphere.w * -2.0f / vDir.y;
                PblVector3 vStart(_Sphere.x, _Sphere.w + _Sphere.y, _Sphere.z);
                const GameObject* pIgnore = this;
                CollisionObject* pHit;
                fLength = CollisionManager::RayHit(vStart, vDir, 50.0f, pHit, NULL, &pIgnore, 1, 0x90, false) * 50.0f;
                if (fMinLength > fLength)
                {
                    fLength = fMinLength;
                }
            }
            else
            {
                fLength = _Sphere.w * 2.0f;
            }
            RedRenderer::SetStencilShadowLength(fLength);
            const PblMatrix& mView = pCamera->GetMatrix();
            float fDX = mView.m[3][0] - fNear * mView.m[2][0] - _Sphere.x;
            float fDY = mView.m[3][1] - mView.m[2][1] * fNear - _Sphere.y;
            float fDZ = mView.m[3][2] - mView.m[2][2] * fNear - _Sphere.z;
            float fReach = _Sphere.w + fLength + fWidth * 1.33f;
            RedRenderer::SetStencilShadowZFail(fReach * fReach > fDX * fDX + fDY * fDY + fDZ * fDZ);
            bDropShadow = false;
        }
        uiPoseFlags = 0x40000000;
    }
    else
    {
        if (pModel != mModelLowRes)
        {
            shadingPose.Reset();
            return;
        }

        if (pActive)
        {
            int eWeaponState = pActive->mState;
            if (!pAnimated->IsMelee())
            {
                if ((unsigned int)(eWeaponState - 1) <= 2)
                {
                    SoldierAnimType::WeaponAnimation eAnim =
                        SoldierAnimatorClass::WeaponStateToAnimation(eWeaponState, bOffhand);
                    if (eAnim != SoldierAnimType::WEAPON_IDLE)
                    {
                        m_uiWeaponAnimIndex =
                            SoldierAnimType::GetAnimIndex((SoldierAnimType::Posture)mStanceIndex, eAnim);
                        m_fWeaponAnimTimer = 1.0f;
                    }
                }
            }
            else
            {

                WeaponMelee* pMelee = (WeaponMelee*)pAnimated;
                if (bOffhand)
                {
                    m_uiWeaponAnimIndex = pMelee->GetOffhandWeaponMeleeAnimation(eWeaponState);
                }
                else if (bThrow)
                {
                    m_uiWeaponAnimIndex = pMelee->GetWeaponMeleeThrowAnimation();
                }
                else if (eWeaponState == 1 || eWeaponState == 4)
                {
                    m_uiWeaponAnimIndex = pMelee->GetWeaponMeleeAnimation();
                }
                m_fWeaponAnimTimer = 0.0f;
            }
        }

        if (mSoldierAnimator && mSoldierAnimator->GetFramesSinceUsed() > 1)
        {
            if (mSoldierAnimator->mOwner == this)
            {
                mSoldierAnimator->mOwner = NULL;
            }
            mSoldierAnimator = NULL;
        }

        mSoldierAnimatorLowRes = SoldierAnimatorLowResClass::GetAnimator(*this);
        if (!mSoldierAnimatorLowRes)
        {
            shadingPose.Reset();
            return;
        }
        pPose = mSoldierAnimatorLowRes->GetPose();
        if (mAcklayData)
        {
            mAcklayData->mAnimatedPose = *pPose;
            mAcklayData->bHasAnimatedPose = true;
        }

        PblMatrix mRoot;
        if (mSoldierAnimatorLowRes->GetRootMatrixAnimation(*this, mRoot))
        {
            PblMatrix mTemp;
            D3DXMatrixMultiply(&mTemp, &mRoot, &mWorld);
            if (mState == SoldierState::FLY || mState == SoldierState::TUMBLE)
            {
                mRoot = mTemp;
                mWorld = NormalMatrix(mRoot.GetZAxis(), *(const PblVector3*)&mRoot.m[1][0], mRoot.GetTranslation());
            }
            else
            {
                mWorld = mTemp;
            }
        }

        if (mJetEffectHandle)
        {
            mJetEffectHandle->SetLocalOffsetMatrix(SoldierAnimatorClass::sInstance->GetJetPackMatrix());
        }
        if (mJetIdleEffectHandle && pPose)
        {
            PblMatrix mOffset = *pPose->Find(0xe5b51072);
            mJetIdleEffectHandle->SetLocalOffsetMatrix(mOffset);
        }
        if (const RedPose* pLowResPose = mSoldierAnimatorLowRes->GetPose())
        {
            for (int i = 0; i < 5; ++i)
            {
                if (mSmolderEffectHandle[i])
                {
                    PblMatrix mOffset = *pLowResPose->Find(mSmolderEffectBone[i]);
                    mSmolderEffectHandle[i]->SetLocalOffsetMatrix(mOffset);
                }
            }
        }

        uiModelFlags = uiFlags;
        uiPoseFlags = 0;
    }

    if (mClass->mOverrideTextureHash)
    {
        if (!pShadingPose)
        {
            pShadingPose = &shadingPose;
        }
        pShadingPose->BindToModel(pModel->GetRedModel(0));
        if (RedShadingState* pState = pShadingPose->CreateShadingState(0xdc5803e9))
        {
            pState->GetIntParam(0x891891e9) = mClass->mOverrideTextureHash;
        }
        if (mClass->mOverrideTextureHash2)
        {
            if (RedShadingState* pState = pShadingPose->CreateShadingState(0xb98e11c1))
            {
                pState->GetIntParam(0x891891e9) = mClass->mOverrideTextureHash2;
            }
        }
    }
    unsigned int uiRenderFlags = uiPoseFlags | uiModelFlags;
    pModel->GetRedModel(0)->Render(&mWorld, pPose, colorModel, uiRenderFlags, pShadingPose);
    if (bReflected)
    {
        pModel->GetRedModel(0)->Render(&mReflection, pPose, colorModel, uiReflectFlags | uiPoseFlags, pShadingPose);
    }

    if (pModel->mClothCount)
    {
        RedColor colorCloth = RedColor::WHITE;
        colorCloth.a = (unsigned char)(int)((ucLod ? fAttachAlpha : fAttachFade) * 255.0f);
        unsigned int uClothArg = (mPlayerId >= 0 && mState == SoldierState::PRONE) ? 5 : 1;
        for (unsigned int i = 0; i < pModel->mClothCount; ++i)
        {
            if (mClothPiece[i])
            {
                mClothPiece[i]->Render(mWorld, pPose, ucLod, colorCloth, uiModelFlags, uClothArg);
            }
        }
    }

    if (ucLod)
    {
        if (pActive && pActive->IsMelee())
        {
            pActive->Render(mWorld, pPose, colorModel, uiRenderFlags | 1, false);
        }
    }
    else if (!(mState == SoldierState::PILOT_ANIMATION && !mHook) && mActiveIndex >= 0 &&
             mWeapon[mActiveIndex])
    {
        Weapon* pOffhand = NULL;
        Weapon* pPrimary = mWeapon[mActiveIndex];
        if (pPrimary->mClass->mIsOffhand)
        {
            pOffhand = pPrimary;
            int iIndex = mWeaponIndex[!mActiveChannel];
            pPrimary = iIndex >= 0 ? mWeapon[iIndex] : NULL;
        }
        if (pPrimary)
        {
            pPrimary->Render(mWorld, pPose, colorModel, uiRenderFlags | 1, pPrimary->IsMelee());
            if (bReflected)
            {
                pPrimary->Render(mReflection, pPose, colorModel, uiReflectFlags | uiPoseFlags | 1,
                                 pPrimary->IsMelee());
            }
        }
        if (pOffhand)
        {
            pOffhand->Render(mWorld, pPose, colorModel, uiRenderFlags | 1, false);
            if (bReflected)
            {
                pOffhand->Render(mReflection, pPose, colorModel, uiReflectFlags | uiPoseFlags | 1, false);
            }
        }
    }

    if (bDropShadow)
    {
        static PblHash sPlayerShadow("player_shadow");
        if (!(uiModelFlags & 0x220000) && fAttachAlpha > 0.0f && pPose)
        {
            pPose->BindToSkeleton(GetModel()->GetRedModel(0)->_pSkeleton);
            if (const PblMatrix* pHead = pPose->Find(0x8afdf5e4))
            {
                float fHeight = pHead->m[3][1];
                if (fHeight > 0.3f)
                {
                    const PblVector3& vDir = RedLight::GetGlobalDirectionalLight(0)->m_vDirection;
                    float fShadowFade = fHeight < 0.8f ? (fHeight - 0.3f) * 2.0f : 1.0f;
                    if (mFlags & flags_isGrounded)
                    {
                        float fShadowAlpha =
                            (1.0f - RedRenderer::GetShadowIntensity()) * (fShadowFade * fAttachAlpha);
                        if (fShadowAlpha > 0.0f)
                        {
                            PblVector3 vPos = mWorld.GetTranslation();
                            PblVector3 vNormal(0.0f - vDir.x * 0.2f, 0.8f - vDir.y * 0.2f, 0.0f - vDir.z * 0.2f);
                            if (-(vNormal.x * vDir.x) - vDir.z * vNormal.z >
                                -(vDir.x * mGroundNormal.x) - vDir.z * mGroundNormal.z)
                            {
                                RedDropShadowManager::AddShadow(sPlayerShadow, vPos, vNormal, vDir, 0.0f, fHeight,
                                                                mClass->mSkeletonRootScale * 2.0f, fShadowAlpha);
                            }
                        }
                    }
                    else
                    {
                        const GameObject* pIgnore = this;
                        PblVector3 vDown(-gVectorUnitY.x, -gVectorUnitY.y, -gVectorUnitY.z);
                        PblVector3 vNormal = gVectorUnitY;
                        PblVector3 vStart(mWorld.m[3][0] - vDown.x * 0.5f, mWorld.m[3][1] - vDown.y * 0.5f,
                                          mWorld.m[3][2] - vDown.z * 0.5f);
                        CollisionObject* pHit;
                        float fHit =
                            CollisionManager::RayHit(vStart, vDown, 4.0f, pHit, &vNormal, &pIgnore, 1, 0x90, true);
                        if (fHit < 1.0f)
                        {
                            float fDrop = fHit * 4.0f;
                            PblVector3 vHit(vDown.x * fDrop + vStart.x, vDown.y * fDrop + vStart.y,
                                            vDown.z * fDrop + vStart.z);
                            PblVector3 vShadowNormal(0.0f - vDir.x * 0.2f, 0.8f - vDir.y * 0.2f,
                                                     0.0f - vDir.z * 0.2f);
                            if (-(vShadowNormal.x * vDir.x) - vDir.z * vShadowNormal.z >
                                -(vNormal.x * vDir.x) - vDir.z * vNormal.z)
                            {
                                float fStart = mWorld.m[3][1] - vHit.y;
                                float fShadowAlpha = (1.0f - RedRenderer::GetShadowIntensity()) *
                                                     ((1.0f - fHit) * (fShadowFade * fAttachAlpha));
                                RedDropShadowManager::AddShadow(sPlayerShadow, vHit, vShadowNormal, vDir, fStart,
                                                                fStart + fHeight, mClass->mSkeletonRootScale * 2.0f,
                                                                fShadowAlpha);
                            }
                        }
                    }
                }
            }
        }
    }

    if (gShowSoldierPolyCount)
    {
        GameModel::PrintDebugInfo(pModel->GetRedModel(0), mWorld.GetTranslation());
    }

    if (FlagItem* pFlag = GetCarriedFlagItem())
    {
        if (pPose)
        {
            if (const PblMatrix* pBone = pPose->Find(pFlag->GetCarriedSoldierBoneHash()))
            {
                PblMatrix mFlag;
                D3DXMatrixMultiply(&mFlag, pBone, &mWorld);
                reinterpret_cast<GameObject*>(pFlag)->SetMatrix(mFlag);
                pFlag->RenderHelper(ucLod, fAlpha, uiModelFlags);
            }
        }
    }

    shadingPose.Reset();
}

// FUNCTION: SWBF2 0x004e5380
bool EntitySoldier::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                      Restrictor* pRestrictor)
{
    const bool bFlyer = pOther->mCollisionModel && pOther->mCollisionModel->mIsFlyerFlag;

    DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
    if (pOther->GetGameObject() && pOther->GetGameObject()->GetControllable())
    {
        owner.mCharacter = pOther->GetGameObject()->GetControllable()->mCharacter;
        owner.mGameObject = pOther->GetGameObject();
        owner.mTeam = pOther->GetGameObject()->GetTeam();
        owner.mClass = pOther->GetGameObject()->GetGameObjectClass();
    }

    if (m_uiInvisibilityAlpha > 0 &&
        (pOther->GetCollisionType() & (COLL_SOFT | COLL_RIGID | COLL_ATTACHED | COLL_ORDNANCE | COLL_SHIELD)))
    {
        for (int i = 0; i < GetWeaponCount(); ++i)
        {
            if (GetWeaponPtr(i)->IsRtti(WeaponInvisibility::rttiHashWeaponInvisibility))
            {
                static_cast<WeaponInvisibility*>(GetWeaponPtr(i))->StartFlicker();
                break;
            }
        }
    }

    if (pOther->GetCollisionType() == COLL_SOFT)
    {
        VisionManager::AddFakeRayhitResult(UnitControllerPtr(mCtrl), pOther->GetGameObject());
    }

    if (pOther->GetCollisionType() == COLL_ITEM)
    {
        pOther->CollisionCallback(pResult, this, pRestrictor);
    }
    else
    {
        PblVector3 vRelVel = mVelocity;
        if (pOther->GetGameObject())
        {
            if (pOther->GetCollisionType() == COLL_RIGID)
            {
                vRelVel.x = 0.0f;
                vRelVel.z = 0.0f;
            }
            const PblVector3& vOtherVel = pOther->GetGameObject()->GetVelocity();
            vRelVel.x -= vOtherVel.x;
            vRelVel.y -= vOtherVel.y;
            vRelVel.z -= vOtherVel.z;
        }
        const PblVector3& vSeparation = pResult->mSeparationNormal;
        float fApproach = vSeparation.y * vRelVel.y + vRelVel.x * vSeparation.x + vSeparation.z * vRelVel.z;

        FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(), pOther, &GetPosition());

        float fImpact = fApproach;
        if (pOther->GetCollisionType() == COLL_STATIC)
        {
            fImpact = vSeparation.y * fApproach;
        }
        if (fImpact < 0.0f && (!mStandingOnObj || mStandingOnObj != pOther->GetGameObject()) &&
            (pOther->GetCollisionType() != COLL_SOFT || bFlyer))
        {
            float fUp = mMatrix.m[1][1] * vSeparation.y + mMatrix.m[1][0] * vSeparation.x +
                        mMatrix.m[1][2] * vSeparation.z;
            float fFront = mMatrix.m[2][1] * vSeparation.y + vSeparation.x * mMatrix.m[2][0] +
                           mMatrix.m[2][2] * vSeparation.z;
            float fThreshold = mClass->mCollisionThreshold[1] * fUp + mClass->mCollisionThreshold[0] +
                               mClass->mCollisionThreshold[2] * fFront;
            PlayImpactEffects(&mMatrix.GetTranslation(), &mVelocity);
            if (-fThreshold > fImpact)
            {
                float fExcess = fThreshold + fImpact;
                float fDamage = (mClass->mCollisionScale[1] * fUp + mClass->mCollisionScale[0] +
                                 mClass->mCollisionScale[2] * fFront) *
                                fExcess * fExcess;
                if (pOther->GetGameObject())
                {
                    fDamage = pOther->GetGameObject()->GetGameObjectClass()->GetCollisionInflict() * fDamage;
                }
                if (fDamage > 0.0f)
                {
                    Damageable::ApplyDamage(DamageDesc(owner, fDamage), 0xffffffff);
                }
            }
        }

        if (AIUtil::ShouldSoldierDodgeThisType(pOther->GetCollisionType()) && fabs(vSeparation.y) < 0.3f)
        {
            mAILastCollisionNormal.x += vSeparation.x;
            mAILastCollisionNormal.y += vSeparation.y;
            mAILastCollisionNormal.z += vSeparation.z;
        }

        if ((pOther->GetCollisionType() != COLL_SOFT || bFlyer) && pOther->GetCollisionType() != COLL_ATTACHED)
        {
            if (mStanceIndex == SoldierState::PRONE && vSeparation.y > 0.6f)
            {
                mDoProneRotation = true;
            }
            if (mPlayerId >= 0)
            {
                gPlayerSoldierCollisionOccurred = true;
            }

            PblVector3 vNormal = vSeparation;
            PblVector3 vPush = vSeparation;
            float fOverlap = pResult->mOverlap;
            if (pOther->GetGameObject())
            {
                float fAge = GameLoop::GetMissionTime() - pOther->GetGameObject()->mCreatedTime;
                if (fAge > 0.0f && fAge < 1.0f)
                {
                    fOverlap = fAge * fOverlap;
                }
            }

            bool bFlattenPush = false;
            if (mStanceIndex == SoldierState::PRONE)
            {
                if (vNormal.y > 0.6f)
                {
                    if (fApproach < 0.1f && (!(mFlags & flags_isGrounded) || vNormal.y > mGroundNormal.y))
                    {
                        PblVector3 vBlend = Interpolate(vNormal, mGroundNormal, 0.5f);
                        PblVector3 vGround;
                        D3DXVec3Normalize(&vGround, &vBlend);
                        mFlags |= flags_isGrounded;
                        mGroundNormal = vGround;
                    }
                }
                else
                {
                    float fDot = mGroundNormal.y * vNormal.y + vNormal.x * mGroundNormal.x +
                                 mGroundNormal.z * vNormal.z;
                    if (fDot < 0.0f)
                    {
                        vNormal.x -= mGroundNormal.x * fDot;
                        vNormal.y -= mGroundNormal.y * fDot;
                        vNormal.z -= mGroundNormal.z * fDot;
                        float fLength = sqrtf(vNormal.x * vNormal.x + vNormal.y * vNormal.y + vNormal.z * vNormal.z);
                        if (fLength > 0.0f)
                        {
                            float fInv = 1.0f / fLength;
                            vNormal.x = fInv * vNormal.x;
                            vNormal.y = fInv * vNormal.y;
                            vNormal.z = fInv * vNormal.z;
                            fOverlap = fOverlap / fLength;
                        }
                    }
                    bFlattenPush = true;
                }
            }
            else
            {
                int iBody = pResult->mColliderId;
                if (iBody == mClass->mLowerBodyId[mStanceIndex] || mAcklayData)
                {

                    if (vNormal.y > 0.0f)
                    {
                        float fWeight = 1.0f / (float)(mNumFlyGroundCollisions + 1);
                        float fKeep = 1.0f - fWeight;
                        ++mNumFlyGroundCollisions;
                        mAvgFlyGroundCollisionNormal.x = fKeep * mAvgFlyGroundCollisionNormal.x + vNormal.x * fWeight;
                        mAvgFlyGroundCollisionNormal.y = mAvgFlyGroundCollisionNormal.y * fKeep + vNormal.y * fWeight;
                        mAvgFlyGroundCollisionNormal.z = mAvgFlyGroundCollisionNormal.z * fKeep + vNormal.z * fWeight;
                    }
                    if (vNormal.y > 0.6f)
                    {
                        if (fApproach < 0.1f && (!(mFlags & flags_isGrounded) || vNormal.y > mGroundNormal.y))
                        {
                            mGroundNormal = vNormal;
                            mFlags |= flags_isGrounded;
                            if (pOther->GetCollisionType() == COLL_RIGID || bFlyer)
                            {

                                if (pOther->GetGameObject() != mStandingOnObj || pResult->mReceiverId != mStandingOnBody)
                                {
                                    mStandingOnObj = pOther->GetGameObject();
                                    mStandingOnBody = pResult->mReceiverId;
                                    if (mStandingOnObj)
                                    {
                                        mLastVehicleMatrix =
                                            mStandingOnObj->mCollisionModel
                                                ->GetWorldMatrix(mStandingOnBody, mStandingOnObj->GetMatrix(),
                                                                 mStandingOnObj->GetPose())
                                                .GetInverseFast();
                                        mStandingOnObjLastVelocity = mStandingOnObj->GetVelocity();
                                    }
                                }
                            }
                            else
                            {
                                mStandingOnObj = 0;
                            }
                        }
                    }
                    else
                    {
                        if (vNormal.y < 0.0f)
                        {
                            vNormal.y = 0.0f;
                            float fLength = sqrtf(vNormal.x * vNormal.x + 0.0f + vNormal.z * vNormal.z);
                            if (fLength > 0.0f)
                            {
                                float fInv = 1.0f / fLength;
                                fOverlap = fOverlap / fLength;
                                vNormal.x = vNormal.x * fInv;
                                vNormal.y = fInv * 0.0f;
                                vNormal.z = vNormal.z * fInv;
                            }
                        }
                        bFlattenPush = true;
                    }
                }
                else if (iBody == mClass->mUpperBodyId[mStanceIndex])
                {

                    if (vPush.y > 0.0f)
                    {
                        bFlattenPush = true;
                    }
                    else if (vNormal.y > 0.0f)
                    {
                        vNormal.y = 0.0f;
                        float fLength = sqrtf(vNormal.x * vNormal.x + 0.0f + vNormal.z * vNormal.z);
                        if (fLength > 0.0f)
                        {
                            float fInv = 1.0f / fLength;
                            fOverlap = fOverlap / fLength;
                            vNormal.x = vNormal.x * fInv;
                            vNormal.y = fInv * 0.0f;
                            vNormal.z = vNormal.z * fInv;
                        }
                    }
                    else if (vNormal.y < 0.0f && (mFlags & flags_isGrounded))
                    {
                        float fDot = mGroundNormal.y * vNormal.y + vNormal.x * mGroundNormal.x +
                                     mGroundNormal.z * vNormal.z;
                        vNormal.x -= mGroundNormal.x * fDot;
                        vNormal.y -= mGroundNormal.y * fDot;
                        vNormal.z -= mGroundNormal.z * fDot;
                        float fLength = sqrtf(vNormal.x * vNormal.x + vNormal.y * vNormal.y + vNormal.z * vNormal.z);
                        if (fLength > 0.0f)
                        {
                            float fInv = 1.0f / fLength;
                            fOverlap = fOverlap / fLength;
                            vNormal.x = vNormal.x * fInv;
                            vNormal.y = vNormal.y * fInv;
                            vNormal.z = vNormal.z * fInv;
                        }
                    }
                }
            }

            if (bFlattenPush)
            {
                vPush.y = 0.0f;
                if (vPush.x != 0.0f || vPush.z != 0.0f)
                {
                    PblVector3 vFlat;
                    D3DXVec3Normalize(&vFlat, &vPush);
                    vPush = vFlat;
                }
            }

            float fInto = mVelocity.y * vPush.y + vPush.x * mVelocity.x + mVelocity.z * vPush.z;
            if (fInto < 0.0f)
            {
                mVelocity.x -= vPush.x * fInto;
                mVelocity.y -= vPush.y * fInto;
                mVelocity.z -= vPush.z * fInto;
            }
            if (pOther->GetCollisionType() == COLL_RIGID || pOther->GetCollisionType() == COLL_STATIC || bFlyer)
            {
                mCollisionState = COLLSTATE_COLLIDE_WITH_RIGID;
            }
            if (fabs(mVelocity.x) < 0.1f)
            {
                mVelocity.x = 0.0f;
            }
            if (fabs(mVelocity.z) < 0.1f)
            {
                mVelocity.z = 0.0f;
            }

            PblVector3 vDir = pRestrictor ? pRestrictor->RestrictMovement(vNormal, fOverlap) : vNormal;
            if (fOverlap > 2.0f && fOverlap > pResult->mOverlap)
            {
                mCollisionState = COLLSTATE_REVERT_TO_SAFE;
                fOverlap = 0.0f;
            }
            PblVector3 vMove = pRestrictor ? pRestrictor->GetRestrictedMovement(vDir, fOverlap)
                                           : PblVector3(vDir.x * fOverlap, vDir.y * fOverlap, vDir.z * fOverlap);
            if (vMove.y > 0.0f)
            {
                vMove.y -= 0.02f;
                if (!(vMove.y > 0.0f))
                {
                    vMove.y = 0.0f;
                }
            }
            SetPosition(PblVector3(mMatrix.m[3][0] + vMove.x, mMatrix.m[3][1] + vMove.y, mMatrix.m[3][2] + vMove.z));
            if (pRestrictor)
            {
                pRestrictor->AddRestriction(vNormal);
            }
            mEyePoint.x += vMove.x;
            mEyePoint.y += vMove.y;
            mEyePoint.z += vMove.z;
            return true;
        }

        if (vSeparation.y < 0.0f)
        {
            return true;
        }
        if (vSeparation.y > 0.0f && mVelocity.y <= 0.0f)
        {
            if (pOther->GetCollisionType() == COLL_ATTACHED)
            {
                mVelocity.y = 0.0f;
                mGroundNormal.x = 0.0f;
                mGroundNormal.y = 1.0f;
                mGroundNormal.z = 0.0f;
                mFlags |= flags_isGrounded;
                RestrictObject(vSeparation, pResult->mOverlap, pRestrictor);
                return true;
            }
            PblVector3 vPush(vSeparation.x * pResult->mOverlap, 0.0f, vSeparation.z * pResult->mOverlap);
            RestrictObject(vPush, sqrtf(vPush.x * vPush.x + 0.0f + vPush.z * vPush.z), pRestrictor);
            return true;
        }
    }

    PblVector3 vOldPos = mMatrix.GetTranslation();
    bool bResult = CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
    if (bResult)
    {
        mEyePoint.x += vOldPos.x - mMatrix.m[3][0];
        mEyePoint.y += vOldPos.y - mMatrix.m[3][1];
        mEyePoint.z += vOldPos.z - mMatrix.m[3][2];
    }
    return bResult;
}

// FUNCTION: SWBF2 0x004e6660
bool EntitySoldier::PostCollisionUpdate(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }

    if (mCollisionState == COLLSTATE_REVERT_TO_SAFE)
    {
        mVelocity.x = 0.0f;
        mVelocity.z = 0.0f;
        SetMatrix(mSafeMatrix);
        mCollisionState = COLLSTATE_NONE;
        return true;
    }
    mCollisionState = COLLSTATE_NONE;

    if (mAvgFlyGroundCollisionNormal.x * mAvgFlyGroundCollisionNormal.x +
            mAvgFlyGroundCollisionNormal.y * mAvgFlyGroundCollisionNormal.y +
            mAvgFlyGroundCollisionNormal.z * mAvgFlyGroundCollisionNormal.z >
        0.0001f)
    {
        float fInv = 1.0f / (float)sqrt((double)(mAvgFlyGroundCollisionNormal.x * mAvgFlyGroundCollisionNormal.x +
                                                 mAvgFlyGroundCollisionNormal.y * mAvgFlyGroundCollisionNormal.y +
                                                 mAvgFlyGroundCollisionNormal.z * mAvgFlyGroundCollisionNormal.z));
        mAvgFlyGroundCollisionNormal.x = fInv * mAvgFlyGroundCollisionNormal.x;
        mAvgFlyGroundCollisionNormal.y = mAvgFlyGroundCollisionNormal.y * fInv;
        mAvgFlyGroundCollisionNormal.z = mAvgFlyGroundCollisionNormal.z * fInv;
        if (mAvgFlyGroundCollisionNormal.y > 0.6f && !(mFlags & flags_isGrounded))
        {
            mGroundNormal = mAvgFlyGroundCollisionNormal;
            mFlags |= flags_isGrounded;
        }
    }
    mNumFlyGroundCollisions = 0;
    mAvgFlyGroundCollisionNormal = gVectorZero;

    if (mStanceIndex == SoldierState::PRONE && mDoProneRotation)
    {
        mDoProneRotation = false;
        PblMatrix mHead = mCollisionModel->GetWorldMatrix(mClass->mUpperBodyId[SoldierState::PRONE], mMatrix, GetPose());
        PblMatrix mFeet = mCollisionModel->GetWorldMatrix(mClass->mLowerBodyId[SoldierState::PRONE], mMatrix, GetPose());
        PblVector3 vRight = GetMatrix().GetXAxis();
        PblVector3 vUp = GetMatrix().GetYAxis();
        PblVector3 vFront = GetMatrix().GetZAxis();

        PblVector3 vHead(mHead.m[3][0] + vFront.x * 0.275f, mHead.m[3][1] + vFront.y * 0.275f,
                         mHead.m[3][2] + vFront.z * 0.275f);
        PblVector3 vFeet(mFeet.m[3][0] - vFront.x * 0.275f, mFeet.m[3][1] - vFront.y * 0.275f,
                         mFeet.m[3][2] - vFront.z * 0.275f);
        const GameObject* pIgnore = this;
        CollisionObject* pHit;
        PblVector3 vHitNormal;
        float fHead = CollisionManager::RayHit(vHead, PblVector3(-gVectorUnitY.x, -gVectorUnitY.y, -gVectorUnitY.z),
                                               10.0f, pHit, &vHitNormal, &pIgnore, 1, 0x98, true);
        if (fHead < 1.0f)
        {
            fHead *= 10.0f;
            PblVector3 vHeadHit(vHead.x - fHead * gVectorUnitY.x, vHead.y - fHead * gVectorUnitY.y,
                                vHead.z - fHead * gVectorUnitY.z);
            float fFeet = CollisionManager::RayHit(vFeet, PblVector3(-gVectorUnitY.x, -gVectorUnitY.y, -gVectorUnitY.z),
                                                   10.0f, pHit, &vHitNormal, &pIgnore, 1, 0x98, true);
            if (fFeet < 1.0f)
            {
                fFeet *= 10.0f;
                PblVector3 vSlope(vHeadHit.x - (vFeet.x - fFeet * gVectorUnitY.x),
                                  vHeadHit.y - (vFeet.y - fFeet * gVectorUnitY.y),
                                  vHeadHit.z - (vFeet.z - fFeet * gVectorUnitY.z));
                if (vSlope.y * vSlope.y + vSlope.x * vSlope.x + vSlope.z * vSlope.z > 0.0001f)
                {

                    PblVector3 vSlopeDir;
                    D3DXVec3Normalize(&vSlopeDir, &vSlope);
                    PblVector3 vBlend = Interpolate(vFront, vSlopeDir, 0.1f);
                    PblVector3 vNewFront;
                    D3DXVec3Normalize(&vNewFront, &vBlend);
                    vFront = vNewFront;

                    PblVector3 vSide;
                    const PblVector3* pSide = CrossProduct(&vSide, &vFront, &mGroundNormal);
                    vBlend = Interpolate(vRight, PblVector3(-pSide->x, -pSide->y, -pSide->z), 0.1f);
                    PblVector3 vNewRight;
                    D3DXVec3Normalize(&vNewRight, &vBlend);
                    vRight = vNewRight;

                    vBlend = PblVector3(vNewFront.y * vNewRight.z - vNewFront.z * vNewRight.y,
                                        vNewRight.x * vNewFront.z - vNewFront.x * vNewRight.z,
                                        vNewFront.x * vNewRight.y - vNewRight.x * vNewFront.y);
                    D3DXVec3Normalize(&vNewFront, &vBlend);
                    vUp = vNewFront;

                    const PblVector3& vOrigin = mMatrix.GetTranslation();
                    PblMatrix mNew;
                    mNew.m[0][0] = vRight.x;
                    mNew.m[0][1] = vRight.y;
                    mNew.m[0][2] = vRight.z;
                    mNew.m[0][3] = 0.0f;
                    mNew.m[1][0] = vUp.x;
                    mNew.m[1][1] = vUp.y;
                    mNew.m[1][2] = vUp.z;
                    mNew.m[1][3] = 0.0f;
                    mNew.m[2][0] = vFront.x;
                    mNew.m[2][1] = vFront.y;
                    mNew.m[2][2] = vFront.z;
                    mNew.m[2][3] = 0.0f;
                    mNew.m[3][0] = vOrigin.x;
                    mNew.m[3][1] = vOrigin.y;
                    mNew.m[3][2] = vOrigin.z;
                    mNew.m[3][3] = 1.0f;
                    SetMatrix(mNew);
                    mFlags |= flags_isGrounded;
                    mGroundNormal = mMatrix.GetYAxis();
                    mVelocity.y = 0.0f;
                }
            }
        }
    }

    if (Smooth(fDeltaTime))
    {
        const PblVector3& vCenter = GetCollisionCenter();
        const PblVector3& vPos = GetPosition();
        const PblMatrix& mSmoothed = GetSmoothedMatrix();
        SetRenderPos(PblVector3(mSmoothed.m[3][0] + vCenter.x - vPos.x, mSmoothed.m[3][1] + vCenter.y - vPos.y,
                                mSmoothed.m[3][2] + vCenter.z - vPos.z));
    }

    if (!netFrameLock && NetGame::IsEnabled())
    {
        if (!NetGame::IsClient() || !netIsLocalTurn)
        {
            if (!NetGame::IsHost() || !NetGame::IsOneWorld() || netIsLateTurn)
            {
                return true;
            }
            if (mNetUniqueId >= 0)
            {
                mNetUniqueId = NetGame::GetHostWorldUniqueId(mNetUniqueId, this);
            }
        }
        gameObjectInterpolator.SetNext(mNetUniqueId, &GetSmoothedMatrix());
        gameObjectInterpolator.SetNextVector(mNetUniqueId, &mAimer.GetDirection());
    }
    return true;
}

// FUNCTION: SWBF2 0x004e6e70
void EntitySoldier::CheckForZoom(float fDeltaTime)
{
    if (mPlayerId < 0)
    {
        return;
    }

    if ((mActiveIndex >= 0 && mWeapon[mActiveIndex] && mWeapon[mActiveIndex]->IsMelee()) || mActiveIndex < 0 ||
        !mWeapon[mActiveIndex] || mWeapon[mActiveIndex]->mClass->mZoomMax <= 1.0f ||
        (mActiveChannel == 0 && mWeapon[mActiveIndex]->mState == Weapon::RELOAD))
    {
        mTargetInfo.mIsAiming = false;
        return;
    }

    if (mControlZoom.IsPressed())
    {
        mTargetInfo.mIsAiming = mWeapon[mActiveIndex]->CycleZoom(mTargetInfo.mIsAiming);
    }
    if (mWeapon[mActiveIndex]->mClass->mZoomRate != 0.0f)
    {
        mWeapon[mActiveIndex]->UpdateZoom(fDeltaTime, mControlMove);
    }
}

// FUNCTION: SWBF2 0x004e6f80
void EntitySoldier::UpdateWeaponAndAimer(float fDeltaTime)
{
    if (!mIsAlive)
    {
        return;
    }

    Controllable* pControl = mCharacter ? mCharacter->GetControllable() : 0;
    if (pControl && pControl->GetControllableClass()->mControlsUnit)
    {
        mEyeDir = pControl->mEyeDir;
    }
    else
    {
        float fSin;
        float fCos;
        PblMath::SinCos(&fSin, &fCos, mPitchOffset + mPitchValue);
        PblVector3 vLocal(0.0f, fSin, fCos);
        PblVector3 vEyeDir;
        D3DXVec3TransformNormal(&vEyeDir, &vLocal, &mMatrix);
        mEyeDir = vEyeDir;
    }

    PblVector3 vEyePoint;
    D3DXVec3TransformCoord(&vEyePoint, &sEyePointOffset[mStanceIndex], &mMatrix);
    mEyePoint = vEyePoint;

    float fForward = sEyePointRelativeWeaponOffset[mStanceIndex].z;
    PblVector3 vFire(fForward * mEyeDir.x + mEyePoint.x, mEyePoint.y + mEyeDir.y * fForward,
                     mEyePoint.z + mEyeDir.z * fForward);

    if (mPlayerId >= 0 && mTracker)
    {
        if (mTargetInfo.mIsAiming && mActiveIndex >= 0 && mWeapon[mActiveIndex] &&
            (mWeapon[mActiveIndex]->ZoomFirstPerson() || mTracker->IsFirstPersonView()))
        {
            mAimer.SetSoldierInfo(vFire, mEyeDir);
            if (mStanceIndex == SoldierState::PRONE)
            {
                float fRight = sEyePointRelativeWeaponOffset[mStanceIndex].x;
                mEyePoint.x += fRight * mMatrix.m[0][0];
                mEyePoint.y += mMatrix.m[0][1] * fRight;
                mEyePoint.z += mMatrix.m[0][2] * fRight;
            }
            else
            {
                mEyePoint = vFire;
            }
            mTargetInfo.mAimStart = vFire;
            mTargetInfo.mAimPoint = PblVector3(mEyeDir.x * 200.0f + vFire.x, mEyeDir.y * 200.0f + vFire.y,
                                               mEyeDir.z * 200.0f + vFire.z);
            return;
        }

        bool bCastRay = true;
        if (NetGame::IsEnabled())
        {
            int iFrames = (netCurMaxPlayers + 3) / 4;
            if (!iFrames)
            {
                iFrames = 1;
            }
            if (NetGame::GetTurnNumber() % iFrames != mPlayerId % iFrames)
            {
                bCastRay = false;
            }
        }

        PblVector3 vOldAimStart = mTargetInfo.mAimStart;
        PblVector3 vOldAimPoint = mTargetInfo.mAimPoint;
        const PblVector3& vOffset = sEyePointRelativeWeaponOffset[mStanceIndex];
        int iCamera = (mState == SoldierState::SPRINT) ? 3 : mStanceIndex;
        const PblVector3& vRight = mMatrix.GetXAxis();
        PblVector3 vUp(mEyeDir.y * vRight.z - mEyeDir.z * vRight.y, vRight.x * mEyeDir.z - mEyeDir.x * vRight.z,
                       mEyeDir.x * vRight.y - vRight.x * mEyeDir.y);
        vFire = PblVector3(vUp.x * vOffset.y + (vOffset.x * vRight.x + vFire.x),
                           vUp.y * vOffset.y + (vRight.y * vOffset.x + vFire.y),
                           vUp.z * vOffset.y + (vRight.z * vOffset.x + vFire.z));

        if (mTracker->IsFirstPersonView())
        {
            mTargetInfo.mAimStart = vFire;
        }
        else
        {

            CameraTrackSetting& rCamera = (&mClass->mCameraSettings[0][0])[iCamera * 2 + mTargetInfo.mIsAiming];
            mEyePoint = rCamera.CalcEyePoint(mMatrix);
            PblMatrix mLook = DirectionMatrix(PblVector3(-mEyeDir.x, -mEyeDir.y, -mEyeDir.z), mMatrix.GetYAxis(),
                                              gVectorZero);
            PblQuaternion qLook = PblQuaternion(mLook);
            float fSin;
            float fCos;
            PblMath::SinCos(&fSin, &fCos, -rCamera.GetCameraTiltValue() * 0.5f);
            PblQuaternion qTilt;
            qTilt._fX = fSin * gVectorUnitX.x;
            qTilt._fY = fSin * gVectorUnitX.y;
            qTilt._fZ = fSin * gVectorUnitX.z;
            qTilt._fS = fCos;
            PblQuaternion qCamera = qLook * qTilt;

            PblQuaternion qUnit;
            PblMatrix mCamera;
            D3DXMatrixRotationQuaternion(&mCamera, D3DXQuaternionNormalize(&qUnit, &qCamera));
            mCamera.m[3][0] = gVectorZero.x;
            mCamera.m[3][1] = gVectorZero.y;
            mCamera.m[3][2] = gVectorZero.z;
            mCamera.m[3][3] = 1.0f;

            mTargetInfo.mAimStart = rCamera.CalcCameraPoint(mMatrix, mCamera);
            float fAlong = mEyeDir.y * (mEyePoint.y - mTargetInfo.mAimStart.y) +
                           mEyeDir.x * (mEyePoint.x - mTargetInfo.mAimStart.x) +
                           mEyeDir.z * (mEyePoint.z - mTargetInfo.mAimStart.z);
            mTargetInfo.mAimStart.y = mEyeDir.y * fAlong + mTargetInfo.mAimStart.y;
            mTargetInfo.mAimStart.x = fAlong * mEyeDir.x + mTargetInfo.mAimStart.x;
            mTargetInfo.mAimStart.z = mEyeDir.z * fAlong + mTargetInfo.mAimStart.z;
        }

        float fBehind = (&mClass->mCameraSettings[0][0])[iCamera * 2 + mTargetInfo.mIsAiming].mCameraTrackOffset.z +
                        2.0f;
        if (bCastRay)
        {
            PblVector3 vStart(mEyeDir.x * fBehind + mTargetInfo.mAimStart.x,
                              mTargetInfo.mAimStart.y + mEyeDir.y * fBehind,
                              mTargetInfo.mAimStart.z + mEyeDir.z * fBehind);
            float fDist;
            if (NetGame::IsEnabled() && NetGame::IsClient() && NetGame::GetJoystickIndex(mPlayerId) < 0)
            {
                fDist = 20.0f;
            }
            else
            {
                const GameObject* pIgnore = this;
                CollisionObject* pHit;
                fDist = CollisionManager::RayHit(vStart, mEyeDir, 60.0f, pHit, 0, &pIgnore, 1, 0x9a, true) * 60.0f;
            }
            mTargetInfo.mAimPoint = PblVector3(fDist * mEyeDir.x + vStart.x, mEyeDir.y * fDist + vStart.y,
                                               mEyeDir.z * fDist + vStart.z);
        }
        else
        {

            PblVector3 vOldStart(mEyeDir.x * fBehind + vOldAimStart.x, vOldAimStart.y + mEyeDir.y * fBehind,
                                 vOldAimStart.z + mEyeDir.z * fBehind);
            float fDist = sqrtf((vOldAimPoint.x - vOldStart.x) * (vOldAimPoint.x - vOldStart.x) +
                                (vOldAimPoint.y - vOldStart.y) * (vOldAimPoint.y - vOldStart.y) +
                                (vOldAimPoint.z - vOldStart.z) * (vOldAimPoint.z - vOldStart.z));
            mTargetInfo.mAimPoint =
                PblVector3(fDist * mEyeDir.x + (mTargetInfo.mAimStart.x + mEyeDir.x * fBehind),
                           mEyeDir.y * fDist + (mTargetInfo.mAimStart.y + mEyeDir.y * fBehind),
                           mEyeDir.z * fDist + (mTargetInfo.mAimStart.z + mEyeDir.z * fBehind));
        }

        PblVector3 vAim(mTargetInfo.mAimPoint.x - vFire.x, mTargetInfo.mAimPoint.y - vFire.y,
                        mTargetInfo.mAimPoint.z - vFire.z);
        PblVector3 vAimDir;
        D3DXVec3Normalize(&vAimDir, &vAim);
        mAimer.SetSoldierInfo(vFire, vAimDir);
    }
    else
    {
        const PblVector3& vOffset = sEyePointRelativeWeaponOffset[mStanceIndex];
        const PblVector3& vRight = mMatrix.GetXAxis();
        PblVector3 vUp(mEyeDir.y * vRight.z - mEyeDir.z * vRight.y, vRight.x * mEyeDir.z - mEyeDir.x * vRight.z,
                       mEyeDir.x * vRight.y - vRight.x * mEyeDir.y);
        vFire = PblVector3(vUp.x * vOffset.y + (vOffset.x * vRight.x + vFire.x),
                           vUp.y * vOffset.y + (vRight.y * vOffset.x + vFire.y),
                           vUp.z * vOffset.y + (vRight.z * vOffset.x + vFire.z));

        if (mPlayerId < 0 && !mClass->mHeroFlag)
        {
            PblVector3 vAimDir = AIUtil::DumbDown(GetGameObject(), fDeltaTime, &mAimer, mEyeDir);
            mTargetInfo.mAimStart = vFire;
            mTargetInfo.mAimPoint = PblVector3(vAimDir.x * 200.0f + vFire.x, vAimDir.y * 200.0f + vFire.y,
                                               vAimDir.z * 200.0f + vFire.z);
            mAimer.SetSoldierInfo(vFire, vAimDir);
        }
        else
        {
            mTargetInfo.mAimStart = vFire;
            mTargetInfo.mAimPoint = PblVector3(mEyeDir.x * 200.0f + vFire.x, mEyeDir.y * 200.0f + vFire.y,
                                               mEyeDir.z * 200.0f + vFire.z);
            mAimer.SetSoldierInfo(vFire, mEyeDir);
        }
    }
}

// FUNCTION: SWBF2 0x004eeb90
void EntitySoldier::MoveOffGround(float fDeltaTime, float fTurn, float fForward, float fStrafe)
{
    if (mFlags & flags_isGrounded)
    {
        float fDot = mGroundNormal.x * mVelocity.x + mVelocity.y * mGroundNormal.y +
                     mVelocity.z * mGroundNormal.z;
        if (fDot < 0.0f)
        {
            mVelocity.x -= mGroundNormal.x * fDot;
            mVelocity.y -= mGroundNormal.y * fDot;
            mVelocity.z -= mGroundNormal.z * fDot;
        }
    }

    float fDrag = expf(sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                             mVelocity.z * mVelocity.z) *
                       -IN_AIR_DRAG_FORCE * fDeltaTime);
    mVelocity.x *= fDrag;
    mVelocity.y *= fDrag;
    mVelocity.z *= fDrag;

    float fPush = fDeltaTime * fForward;
    mVelocity.x += fPush * mMatrix.m[2][0];
    mVelocity.y += mMatrix.m[2][1] * fPush;
    mVelocity.z += mMatrix.m[2][2] * fPush;
    fPush = fDeltaTime * fStrafe;
    mVelocity.y += mMatrix.m[0][1] * fPush;
    mVelocity.z += mMatrix.m[0][2] * fPush;
    mVelocity.x += fPush * mMatrix.m[0][0];

    if (mHook)
    {
        mHook->MoveSoldier(this, fDeltaTime);
    }
    else if (!(mFlags & flags_isGrounded))
    {
        mVelocity.y += fDeltaTime * -GRAVITY_FORCE;
    }

    const PblVector3& vPos = mMatrix.GetTranslation();
    SetPosition(PblVector3(vPos.x + fDeltaTime * mVelocity.x, vPos.y + mVelocity.y * fDeltaTime,
                           vPos.z + mVelocity.z * fDeltaTime));

    float fAngle = fDeltaTime * fTurn;
    if (fAngle != 0.0f)
    {
        float fSin;
        float fCos;
        PblMath::SinCos(&fSin, &fCos, fAngle);
        PblMatrix mRotation;
        mRotation.m[0][0] = fCos;
        mRotation.m[0][1] = 0.0f;
        mRotation.m[0][2] = -fSin;
        mRotation.m[0][3] = 0.0f;
        mRotation.m[1][0] = 0.0f;
        mRotation.m[1][1] = 1.0f;
        mRotation.m[1][2] = 0.0f;
        mRotation.m[1][3] = 0.0f;
        mRotation.m[2][0] = fSin;
        mRotation.m[2][1] = 0.0f;
        mRotation.m[2][2] = fCos;
        mRotation.m[2][3] = 0.0f;
        mRotation.m[3][0] = 0.0f;
        mRotation.m[3][1] = 0.0f;
        mRotation.m[3][2] = 0.0f;
        mRotation.m[3][3] = 1.0f;
        PblMatrix mNew;
        D3DXMatrixMultiply(&mNew, &mRotation, &mMatrix);
        SetMatrix(mNew);
    }
    else
    {
        SetMatrix(mMatrix);
    }
}

// FUNCTION: SWBF2 0x004ef350
bool EntitySoldier::UpdateBoardVsChangeClass()
{
    int iJoystick = GetJoystickIndex();
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        if (iJoystick < 0 || !mControlUse.IsPressed() || !netIsLocalTurn || mClass->mHeroFlag)
        {
            return false;
        }
        mCanChangeClass = false;
        bool bVehicle = true;
        if (mClass->mCanEnterVehicles && TryBoard(true, bVehicle))
        {
            return false;
        }

        for (CommandPostItor itor = CommandPostManager::Instance()->GetItor(); itor; ++itor)
        {
            CommandPost* pPost = *itor;
            if (!(pPost->mHoldPlayerMask & (1 << mCharacter->mPlayerId)))
            {
                continue;
            }
            const PblVector3& vMe = GetCollisionCenter();
            const PblVector3& vPost = pPost->GetGameObject()->GetCollisionCenter();
            PblVector3 vDir(vPost.x - vMe.x, vPost.y - vMe.y, vPost.z - vMe.z);
            float fDist = sqrtf(vDir.y * vDir.y + vDir.x * vDir.x + vDir.z * vDir.z);
            if (fDist > 0.0001f)
            {
                float fInv = 1.0f / fDist;
                if (mEyeDir.y * (vDir.y * fInv) + vDir.x * fInv * mEyeDir.x + mEyeDir.z * (vDir.z * fInv) > 0.15f &&
                    pPost->CanTeamSpawnHere(GetTeam()))
                {
                    mCanChangeClass = true;
                    break;
                }
            }
        }
        if (!mCanChangeClass)
        {
            GameObject* aObjects[64];
            int iCount = CollisionManager::GetObjectsInRange(GetCollisionCenter(), 30.0f, aObjects, 64, this, 0x10);
            for (int i = 0; i < iCount; ++i)
            {
                const PblVector3& vMe = GetPosition();
                const PblVector3& vOther = aObjects[i]->GetPosition();
                PblVector3 vDir(vOther.x - vMe.x, vOther.y - vMe.y, vOther.z - vMe.z);
                float fDist = sqrtf(vDir.y * vDir.y + vDir.x * vDir.x + vDir.z * vDir.z);
                if (fDist > 0.0001f)
                {
                    float fInv = 1.0f / fDist;
                    if (aObjects[i]->GetGameObjectClass()->mSwitchClassRadius > fDist &&
                        mEyeDir.x * (vDir.x * fInv) + mEyeDir.y * (vDir.y * fInv) + mEyeDir.z * (vDir.z * fInv) >
                            0.15f &&
                        aObjects[i]->mTeam == mTeam)
                    {
                        mCanChangeClass = true;
                        break;
                    }
                }
            }
        }
        if (mCanChangeClass)
        {
            SpawnDisplay::Instance(iJoystick)->ChangeClass(mCharacter);
        }
        return false;
    }

    bool bVehicle = true;
    GameObject* pVehicle = (mPlayerId >= 0) ? TryBoard(true, bVehicle) : 0;
    if (iJoystick >= 0 && !NetGame::IsEnabled())
    {
        int iLocal = NetGame::GetLocalIndex(0);
        if (mCharacter == Character::FindPlayer(iLocal) &&
            (ToolTipsDisplay::sInstance->ShouldTestForToolTipState(TTT_ApproachVehicle) ||
             ToolTipsDisplay::sInstance->ShouldTestForToolTipState(TTT_ApproachTurret)) &&
            pVehicle)
        {
            ToolTipsDisplay::sInstance->ShowTip(bVehicle ? TTT_ApproachVehicle : TTT_ApproachTurret, 0);
        }
        ToolTipsDisplay::sInstance->ShowTip(TTT_SoldierFire, 0);
        ToolTipsDisplay::sInstance->ShowTip(TTT_SoldierSprint, 0);
        ToolTipsDisplay::sInstance->ShowTip(TTT_SoldierTarget, 0);
        if (GetWeaponCount() > 0 && GetWeaponPtr(0)->IsMelee())
        {
            ToolTipsDisplay::sInstance->ShowTip(TTT_JediFire, 0);
            ToolTipsDisplay::sInstance->ShowTip(TTT_JediBlock, 0);
            ToolTipsDisplay::sInstance->ShowTip(TTT_JediForceJump, 0);
            ToolTipsDisplay::sInstance->ShowTip(TTT_JediForcePower, 0);
            if (mControlFire[0].IsDown())
            {
                ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_JediFire, 0);
            }
            if (mControlFire[1].IsDown())
            {
                ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_JediBlock, 0);
            }
        }
        else if (mClass->mJetFuelRechargeRate > 0.0f)
        {
            ToolTipsDisplay::sInstance->ShowTip(TTT_JetTrooper_Jet, 0);
        }
    }

    mCanChangeClass = false;
    if (mPlayerId >= 0 && !pVehicle)
    {
        for (CommandPostItor itor = CommandPostManager::Instance()->GetItor(); itor; ++itor)
        {
            CommandPost* pPost = *itor;
            if (!(pPost->mHoldPlayerMask & (1 << mCharacter->mPlayerId)))
            {
                continue;
            }
            const PblVector3& vMe = GetCollisionCenter();
            const PblVector3& vPost = pPost->GetGameObject()->GetCollisionCenter();
            PblVector3 vDir(vPost.x - vMe.x, vPost.y - vMe.y, vPost.z - vMe.z);
            float fDist = sqrtf(vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z);
            if (fDist > 0.0001f)
            {
                float fInv = 1.0f / fDist;
                if (mEyeDir.x * (vDir.x * fInv) + mEyeDir.y * (vDir.y * fInv) + mEyeDir.z * (vDir.z * fInv) > 0.15f &&
                    pPost->CanTeamSpawnHere(GetTeam()))
                {
                    mCanChangeClass = true;
                    break;
                }
            }
        }
        if (!mCanChangeClass)
        {
            GameObject* aObjects[64];
            int iCount = CollisionManager::GetObjectsInRange(GetCollisionCenter(), 30.0f, aObjects, 64, this, 0x10);
            for (int i = 0; i < iCount; ++i)
            {
                const PblVector3& vMe = GetPosition();
                const PblVector3& vOther = aObjects[i]->GetPosition();
                PblVector3 vDir(vOther.x - vMe.x, vOther.y - vMe.y, vOther.z - vMe.z);
                float fDist = sqrtf(vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z);
                if (fDist > 0.0001f)
                {
                    float fInv = 1.0f / fDist;
                    if (aObjects[i]->GetGameObjectClass()->mSwitchClassRadius > fDist &&
                        mEyeDir.x * (vDir.x * fInv) + mEyeDir.y * (vDir.y * fInv) + mEyeDir.z * (vDir.z * fInv) >
                            0.15f &&
                        aObjects[i]->mTeam == mTeam)
                    {
                        mCanChangeClass = true;
                        break;
                    }
                }
            }
        }
    }
    if (mClass->mHeroFlag)
    {
        mCanChangeClass = false;
    }

    if (!mControlUse.IsPressed())
    {
        return false;
    }
    if (mClass->mCanEnterVehicles)
    {
        if (iJoystick < 0)
        {
            pVehicle = TryBoard(NetGame::IsEnabled() && NetGame::IsClient(), bVehicle);
        }
        else if (pVehicle && !(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            pVehicle = EnterVehicle(pVehicle) ? pVehicle : 0;
        }
        if (pVehicle)
        {
            if (mCharacter)
            {
                ToolTipsDisplay::sInstance->CancelTip(0);
                ToolTipsDisplay::sInstance->CancelTimeoutTip(bVehicle ? TTT_ApproachVehicle : TTT_ApproachTurret, 0);
            }
            return true;
        }
    }
    if (iJoystick >= 0 && mCanChangeClass)
    {
        SpawnDisplay::Instance(iJoystick)->ChangeClass(mCharacter);
    }
    return false;
}

// FUNCTION: SWBF2 0x004efcb0
GameObject* EntitySoldier::TryBoard(bool bArg, bool& rIsVehicle)
{
    GameObject* apObjects[16];
    const int iCount = CollisionManager::GetObjectsInRange(GetCollisionCenter(), GetCollisionRadius() + 0.5f,
                                                           apObjects, 16, this, 26);
    GameObject* pBest = 0;
    float fBestPrio = 3.402823466e+38F;
    for (int i = 0; i < iCount; ++i)
    {
        GameObject* pObject = apObjects[i];
        if (pObject->GetModel())
        {
            const PblBox& box = pObject->GetModel()->_mModel->_TotalBoundingBox;
            const PblVector3 vMin(box._vMin.x - (GetCollisionRadius() + 0.5f),
                                  box._vMin.y - (GetCollisionRadius() + 0.5f),
                                  box._vMin.z - (GetCollisionRadius() + 0.5f));
            const PblVector3 vMax(box._vMax.x + (GetCollisionRadius() + 0.5f),
                                  box._vMax.y + (GetCollisionRadius() + 0.5f),
                                  box._vMax.z + (GetCollisionRadius() + 0.5f));
            PblMatrix mInverse = pObject->GetMatrix().GetInverseFast();
            PblVector3 vLocal;
            D3DXVec3TransformCoord(&vLocal, &GetCollisionCenter(), &mInverse);
            if (vLocal.x < vMin.x || vMax.x < vLocal.x || vLocal.y < vMin.y || vMax.y < vLocal.y ||
                vLocal.z < vMin.z || vMax.z < vLocal.z)
            {
                continue;
            }
        }

        GameObject* pTerminal = 0;
        if (pObject->IsRtti(EntityRemoteTerminal::rttiHashEntityRemoteTerminal))
        {
            if (!CanEnterVehicle(pObject))
            {

                EntityRemoteTerminal* pRemote = (EntityRemoteTerminal*)pObject;
                pTerminal = (GameObject*)pRemote->GetNextAvailableTerminal(this);
                if (!pTerminal)
                {
                    pTerminal = (GameObject*)pRemote->GetPrevAvailableTerminal(this);
                }
                if (!pTerminal || !CanEnterVehicle(pTerminal))
                {
                    continue;
                }
            }
        }
        else if (!CanEnterVehicle(pObject))
        {
            continue;
        }

        const CollisionObject* pCollision = pObject->GetCollisionObject();
        float fPrio = TargetInfo::GetTargetPrio(mEyePoint, mEyeDir, pCollision->GetCollisionCenter(),
                                                pCollision->GetCollisionRadius(), 1.0f, 5.0f, false);
        if (fPrio < 0.0f || fBestPrio <= fPrio)
        {
            continue;
        }
        fBestPrio = fPrio;
        pBest = pTerminal ? pTerminal : pObject;
    }
    if (!pBest)
    {
        return 0;
    }
    AIUtil::EntityType eType = AIUtil::GetEntityType(pBest);
    rIsVehicle = eType != AIUtil::ENTITY_TURRET && eType != AIUtil::ENTITY_BUILDING_ARMED;
    if (bArg || EnterVehicle(pBest))
    {
        return pBest;
    }
    return 0;
}

// FUNCTION: SWBF2 0x004f00d0
int EntitySoldier::GetReservableSlots(ReserveType eType) const
{
    if (eType != RT_FORMATION)
    {
        return 1;
    }
    MedalsMgr* pMedals = mCharacter ? mCharacter->mMedalsMgr : 0;
    if (pMedals)
    {
        return pMedals->GetRank() + 1;
    }
    return 4;
}

// FUNCTION: SWBF2 0x004f0110
bool EntitySoldier::UpdateIndirect(float fDeltaTime)
{
    if (!mSkip && !(mHandleId & 1) == gThreadIsEven && !AIUtil::IsImportant(this))
    {
        mSkip = true;
        mSkipIndirectTime = fDeltaTime;
        for (int i = 0; i < GetWeaponCount(); ++i)
        {
            Weapon* pWeapon = GetWeaponPtr(i);
            if (pWeapon)
            {
                pWeapon->SetSkip(true);
            }
        }
        mAimer.SetSkip(true);
        return true;
    }

    fDeltaTime += mSkipIndirectTime;
    mSkip = false;
    for (int i = 0; i < GetWeaponCount(); ++i)
    {
        Weapon* pWeapon = GetWeaponPtr(i);
        if (pWeapon)
        {
            pWeapon->SetSkip(false);
        }
    }
    mAimer.SetSkip(false);
    mSkipIndirectTime = 0.0f;

    if (mState == SoldierState::FLY || mState == SoldierState::TUMBLE || mState == SoldierState::BOUNCE)
    {
        return true;
    }

    AILowLevel& lowLevel = UnitControllerPtr(mCtrl)->GetLowLevel();
    if (!lowLevel.UpdateIndirect(fDeltaTime) || !mCtrl)
    {
        return true;
    }
    if (mInputLockTime > 0.0f)
    {
        UnitControllerPtr(mCtrl)->GetLowLevel().mRoll = false;
    }
    if (mSoldierAnimator && mSoldierAnimator->GetOwner() != this)
    {
        mSoldierAnimator = 0;
    }

    if (mState != SoldierState::ARMED_PASSENGER)
    {
        if (mPlayAnim)
        {
            if (mState != SoldierState::PILOT_ANIMATION)
            {
                SetState(SoldierState::PILOT_ANIMATION);
                mAnimationHash = mAnimNameId;
                m_ePilotAnimation = (SoldierAnimationBank::ANIMATION)-1;
            }
        }
        else if (lowLevel.mDoJetJump || lowLevel.mJetJumping)
        {
            if (lowLevel.mJetJumpStop)
            {
                lowLevel.mJetJumpStop = false;
                EndJetJump();
            }
            lowLevel.mDoJetJump = false;
            if (mInputLockTime <= 0.0f)
            {
                if (mState == SoldierState::JUMP)
                {
                    JetJump();
                }
                else if (!lowLevel.mJetJumping)
                {
                    Jump();
                    lowLevel.mDoJetJump = false;
                    lowLevel.mJetJumping = true;
                }
                else if (mState != SoldierState::JET_HOVER && mState != SoldierState::JET_JUMP)
                {
                    lowLevel.mJetJumping = false;
                }
            }
        }
        else if (lowLevel.mDoJump)
        {
            mInputLockTime = 0.0f;
            Jump();
            lowLevel.mDoJump = false;
            lowLevel.mJumping = true;
            lowLevel.mAIJetJumpDir = PblVector3(0.0f, 0.0f, 0.0f);
        }
        else if (lowLevel.mJumping)
        {
            if (mState != SoldierState::JUMP)
            {
                lowLevel.mJumping = false;
            }
        }
        else if (lowLevel.mDoIdle && mSoldierAnimator)
        {
            mSoldierAnimator->PlayIdleAnimation(lowLevel.mDoIdle);
            lowLevel.mDoIdle = SoldierAnimType::IDLE;
        }
        else if (lowLevel.mRoll && !mClass->m_bIsAcklay)
        {
            if (!lowLevel.mRolling)
            {
                if (lowLevel.mRollDir.x * lowLevel.mRollDir.x + lowLevel.mRollDir.y * lowLevel.mRollDir.y +
                        lowLevel.mRollDir.z * lowLevel.mRollDir.z >
                    0.1f)
                {
                    mVelocity = lowLevel.mRollDir;
                }
                else
                {
                    mVelocity = GetMatrix().GetZAxis();
                }
                Roll();
            }
            lowLevel.mRoll = false;
            lowLevel.mRolling = true;
        }
        else
        {
            if (lowLevel.mRolling && mState != SoldierState::ROLL)
            {
                lowLevel.mRolling = false;
                lowLevel.SetHeight(HEIGHT_CROUCH);
            }
            if (!lowLevel.mRolling && mInputLockTime <= 0.0f && mState != SoldierState::CHOKE)
            {
                switch (lowLevel.mHeight)
                {
                case HEIGHT_STAND:
                case HEIGHT_WALK:
                    Stand();
                    break;
                case HEIGHT_CROUCH:
                case HEIGHT_PRONE:
                    Crouch();
                    break;
                case HEIGHT_SPRINT:
                    if (!Sprint())
                    {
                        Stand();
                    }
                    break;
                }
            }
        }
    }

    if (mState == SoldierState::JET_HOVER)
    {
        float fHoverHeight = lowLevel.mDesiredHoverHeight;
        if (fHoverHeight < FLT_MAX)
        {
            float fDiff = fHoverHeight - GetPosition().y;
            if (fDiff > 0.0f)
            {
                float fRise = fDiff * 0.35f;
                if (fRise >= fDeltaTime * 30.0f)
                {
                    fRise = fDeltaTime * 30.0f;
                }
                mMatrix.m[3][1] += fRise;
            }
            else if (fDiff < 0.0f)
            {
                float fRise = fDiff * 0.35f;
                if (fRise <= fDeltaTime * -30.0f)
                {
                    fRise = fDeltaTime * -30.0f;
                }
                mMatrix.m[3][1] += fRise;
            }
        }
    }

    if (lowLevel.GetAlert())
    {
        if (mSoldierAnimator && mActiveIndex >= 0 && mWeapon[mActiveIndex] &&
            mWeapon[mActiveIndex]->mSoldierAnimationMap != -1)
        {
            mSoldierAnimator->SetWeaponAnimationMap(
                (SoldierAnimationBank::MAP)mWeapon[mActiveIndex]->mSoldierAnimationMap);
        }
        SetAlert(true);
    }

    ControllableHeading heading = lowLevel.mHeading;
    lowLevel.ProcessFire(fDeltaTime, &heading);
    float fPitch = GetPitchAdjusted();
    float fStrafe = mControlStrafe;
    float fThrust = mControlMove;
    float fTurn = GetTurnAdjusted();
    AIUtil::UpdatePhysics(this, fDeltaTime, lowLevel.mDest, heading, mClass->mMaxSpeed, mClass->mMaxStrafeSpeed,
               GetClassMaxTurnSpeed(), mClass->mMaxPitchSpeed, &fThrust, &fStrafe, &fTurn, &fPitch);

    const PblVector3& vPos = GetPosition();
    float fStep = mClass->mMaxSpeed * fDeltaTime;
    float fDx = vPos.x - lowLevel.mDest.mPos.x;
    float fDz = vPos.z - lowLevel.mDest.mPos.z;
    float fDistSq = fDz * fDz + fDx * fDx;
    if (fStep * fStep > fDistSq)
    {
        float fScale = (float)sqrt((double)fDistSq) / fStep;
        fStrafe = fScale * fStrafe;
        fThrust = fScale * fThrust;
    }
    mControlStrafe = fStrafe;
    mControlTurn = fTurn;
    mControlMove = fThrust;
    mControlPitch = fPitch;
    return true;
}

// FUNCTION: SWBF2 0x004f0830
void EntitySoldier::SetupController()
{
    Controller* pController;
    if (mPlayerId >= 0)
    {
        pController = new PlayerController(this);
        Track(mPlayerId);
    }
    else
    {
        if (NetGame::IsEnabled() && NetGame::IsClient())
        {
            return;
        }
        pController = new UnitMobileController(this);
    }
    if (pController)
    {
        AddController(pController, mPlayerId < 0);
    }
}

// FUNCTION: SWBF2 0x004f09a0
bool EntitySoldier::CanEnterVehicle(const GameObject* pObject) const
{
    return CanEnterVehicleInternal(pObject, false);
}

static int GetFlyerState(const GameObject* pObject)
{
    return static_cast<const EntityFlyer*>(pObject)->mState;
}

// FUNCTION: SWBF2 0x004f09c0
bool EntitySoldier::CanEnterVehicleInternal(const GameObject* pObject, bool bRemote) const
{
    if (!pObject->IsAlive() || pObject->mDisableTime > 0.0f)
    {
        return false;
    }
    if (pObject->IsRtti(EntityRemoteTerminal::rttiHashEntityRemoteTerminal) &&
        !((EntityRemoteTerminal*)pObject)->IsTerminalAvailable(this))
    {
        return false;
    }
    if (pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer) && GetFlyerState(pObject) == 2)
    {
        return false;
    }
    int iTeam = pObject->GetTeam();
    if (iTeam > 0)
    {
        if (mPlayerId < 0)
        {
            if (iTeam != GetTeam())
            {
                return false;
            }
        }
        else if (!IsMyFriend(pObject))
        {
            return false;
        }
    }

    for (PblList<Controllable>::Itor it(pObject->GetControllableList()); it; ++it)
    {
        Controllable* pSeat = it;
        if (bRemote ? pSeat->mPilotType != PILOT_REMOTE
                    : (pSeat->mPilotType != PILOT_VEHICLE && pSeat->mPilotType != PILOT_VEHICLESELF))
        {
            continue;
        }
        if (!pSeat->GetPilot() &&
            (mPlayerId >= 0 || (pSeat->mPilotType != PILOT_SELF && pSeat->mPilotType != PILOT_VEHICLESELF)))
        {
            return true;
        }
        if (pSeat->GetPilot() && pSeat->GetPilot()->mPlayerId < 0 && mPlayerId >= 0 &&
            !pSeat->GetPilot()->mCharacter->mRemote)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x004f2290
void EntitySoldier::SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime)
{
    PblMatrix mAim = GetSmoothedMatrix();
    Controllable* pControllable = mCharacter ? mCharacter->GetControllable() : 0;
    PblVector3 vAimDir;
    if (pControllable && pControllable->GetControllableClass()->mControlsUnit)
    {
        vAimDir = pControllable->mEyeDir;
    }
    else
    {
        float fSin;
        float fCos;
        PblMath::SinCos(&fSin, &fCos, mPitchOffset + mPitchValue);
        PblVector3 vLocal(0.0f, fSin, fCos);
        D3DXVec3TransformNormal(&vAimDir, &vLocal, &mAim);
    }
    PblVector3 vEye;
    D3DXVec3TransformCoord(&vEye, &sEyePointOffset[mStanceIndex], &mAim);

    bool bSpectator = false;
    if (NetGame::IsEnabled() &&
        NetGame::GetSpectator(NetGame::GetLocalIndex(CameraManager::sInstance->mUpdatedCameraId)) >= 0)
    {
        bSpectator = true;
    }

    if (IsAlive() && !bSpectator &&
        ((mTargetInfo.mIsAiming && mActiveIndex >= 0 && mWeapon[mActiveIndex] &&
          mWeapon[mActiveIndex]->ZoomFirstPerson()) ||
         (mTracker && mTracker->IsFirstPersonView())) &&
        !mFlyingInFirstPerson)
    {
        rMatrix = DirectionMatrix(PblVector3(-vAimDir.x, -vAimDir.y, -vAimDir.z), mAim.GetYAxis(), vEye);
        const float fBack = mTargetInfo.mIsAiming ? 0.6f : 0.3f;
        rMatrix.m[3][0] -= vAimDir.x * fBack;
        rMatrix.m[3][1] -= vAimDir.y * fBack;
        rMatrix.m[3][2] -= vAimDir.z * fBack;
        rMatrix.m[3][3] = 1.0f;
        int iFirstPerson = GetFirstPersonIndex();
        if (iFirstPerson >= 0 && !mTargetInfo.mIsAiming)
        {
            FirstPerson::UpdateCamera(iFirstPerson, rMatrix);
        }
    }
    else
    {
        int iCamera = (mState == SoldierState::SPRINT) ? 3 : mStanceIndex;
        float fBlendTime = 0.0f;
        if (mCameraSettingPrev != iCamera)
        {
            fBlendTime = mClass->mCameraSettingBlendTime[(iCamera > mCameraSettingPrev ? iCamera : mCameraSettingPrev) - 1];
        }
        const float fMoveTension = (mState == SoldierState::PRONE) ? 0.5f : 1.0f;
        const float fAimTension = IsAlive() ? 1.0f : 0.1f;
        CameraTrackSetting* pSettings = &mClass->mCameraSettings[0][0];
        CameraTrackSetting& rCamera = pSettings[iCamera * 2 + mTargetInfo.mIsAiming];
        if (fBlendTime > mCameraSettingTimer)
        {
            rCamera.SetupCameraTrackMatrix(this, pSettings[mCameraSettingPrev * 2 + mTargetInfo.mIsAiming],
                                           mCameraSettingTimer / fBlendTime, rMatrix, fDeltaTime, mAim, vAimDir,
                                           fMoveTension, fAimTension, this);
        }
        else
        {
            rCamera.SetupCameraTrackMatrix(this, rMatrix, fDeltaTime, mAim, vAimDir, fMoveTension, fAimTension,
                                           this);
        }
    }

    if (NetGame::IsEnabled() || mPlayerId < 0)
    {
        return;
    }
    Weapon* pWeapon = (mActiveIndex >= 0) ? mWeapon[mActiveIndex] : 0;
    const bool bZoomBlur = mTargetInfo.mIsAiming && pWeapon && pWeapon->mClass->mZoomRate != 0.0f;
    const int iCam = CameraManager::sInstance->mUpdatedCameraId;
    MotionBlurEffect::Stop(0, iCam);
    if (Controllable* pVehicle = mCharacter->mVehicle)
    {
        if (pVehicle->GetEntity()->IsRtti(EntityHover::rttiHashEntityHover) &&
            ((EntityHover*)pVehicle->GetEntity())->mClass->mBlurEffect > 0.0f)
        {
            MotionBlurEffect::Start(0, 0.8f, iCam);
        }
    }
    else if (bZoomBlur)
    {
        MotionBlurEffect::Start(0, 0.6f, iCam);
    }
    else if (mClass->m_fBlurEffect > 0.0f &&
             (mState == SoldierState::SPRINT || mState == SoldierState::JET_JUMP ||
              mState == SoldierState::JUMP || mState == SoldierState::FALL))
    {
        const PblVector3& vForward = mMatrix.GetZAxis();
        float fSpeed = mVelocity.y * vForward.y + mVelocity.x * vForward.x + mVelocity.z * vForward.z;
        float fSprint = mClass->mThrustFactor[SoldierState::SPRINT] * mClass->mMaxSpeed;
        float fFrom = (mClass->mMaxSpeed + fSprint) * 0.5f;
        if (fSpeed > fFrom)
        {
            MotionBlurEffect::Start(0, (fSpeed - fFrom) / (fSprint - fFrom) * mClass->m_fBlurEffect, iCam);
        }
    }
}

// FUNCTION: SWBF2 0x004f2000
void EntitySoldier::SetupCameraTrackMatrixHUDTargeting(PblMatrix& rMatrix)
{
    if (IsAlive() &&
        ((mTargetInfo.mIsAiming && mActiveIndex >= 0 && mWeapon[mActiveIndex] &&
          mWeapon[mActiveIndex]->ZoomFirstPerson()) ||
         (mTracker && mTracker->IsFirstPersonView())) &&
        !mFlyingInFirstPerson)
    {
        rMatrix = DirectionMatrix(PblVector3(-mEyeDir.x, -mEyeDir.y, -mEyeDir.z),
                                  static_cast<CollisionObject*>(this)->GetMatrix().GetYAxis(), mEyePoint);
        const float fBack = mTargetInfo.mIsAiming ? 0.6f : 0.3f;
        rMatrix.m[3][0] -= mEyeDir.x * fBack;
        rMatrix.m[3][1] -= mEyeDir.y * fBack;
        rMatrix.m[3][2] -= mEyeDir.z * fBack;
        rMatrix.m[3][3] = 1.0f;
        return;
    }
    int iCamera = (mState == SoldierState::SPRINT) ? 3 : mStanceIndex;
    float fBlendTime = 0.0f;
    if (mCameraSettingPrev != iCamera)
    {
        fBlendTime = mClass->mCameraSettingBlendTime[(iCamera > mCameraSettingPrev ? iCamera : mCameraSettingPrev) - 1];
    }
    CameraTrackSetting* pSettings = &mClass->mCameraSettings[0][0];
    if (fBlendTime > mCameraSettingTimer)
    {
        pSettings[iCamera * 2 + mTargetInfo.mIsAiming].SetupCameraTrackMatrixHUDTargeting(
            pSettings[mCameraSettingPrev * 2 + mTargetInfo.mIsAiming], mCameraSettingTimer / fBlendTime,
            rMatrix, static_cast<CollisionObject*>(this)->GetMatrix(), mEyeDir);
    }
    else
    {
        pSettings[iCamera * 2 + mTargetInfo.mIsAiming].SetupCameraTrackMatrixHUDTargeting(
            rMatrix, static_cast<CollisionObject*>(this)->GetMatrix(), mEyeDir);
    }
}

// FUNCTION: SWBF2 0x004f28e0
void EntitySoldier::SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix)
{
    if (NetGame::IsEnabled() && mPlayerId >= 0)
    {
        const int iCam = CameraManager::sInstance->mUpdatedCameraId;
        Weapon* pWeapon = (mActiveIndex >= 0) ? mWeapon[mActiveIndex] : 0;
        const bool bZoomBlur = mTargetInfo.mIsAiming && pWeapon && pWeapon->mClass->mZoomRate != 0.0f;
        MotionBlurEffect::Stop(0, iCam);
        if (Controllable* pVehicle = mCharacter->mVehicle)
        {
            if (pVehicle->GetEntity()->IsRtti(EntityHover::rttiHashEntityHover) &&
                ((EntityHover*)pVehicle->GetEntity())->mClass->mBlurEffect > 0.0f)
            {
                MotionBlurEffect::Start(0, 0.8f, iCam);
            }
        }
        else if (bZoomBlur)
        {
            MotionBlurEffect::Start(0, 0.6f, iCam);
        }
        else if (mClass->m_fBlurEffect > 0.0f &&
                 (mState == SoldierState::SPRINT || mState == SoldierState::JET_JUMP ||
                  mState == SoldierState::JUMP || mState == SoldierState::FALL))
        {
            const PblVector3& vForward = mMatrix.GetZAxis();
            float fSpeed = mVelocity.y * vForward.y + mVelocity.x * vForward.x + mVelocity.z * vForward.z;
            float fSprint = mClass->mThrustFactor[SoldierState::SPRINT] * mClass->mMaxSpeed;
            float fFrom = (mClass->mMaxSpeed + fSprint) * 0.5f;
            if (fSpeed > fFrom)
            {
                MotionBlurEffect::Start(0, (fSpeed - fFrom) / (fSprint - fFrom) * mClass->m_fBlurEffect, iCam);
            }
        }
    }
    if (IsAlive() &&
        ((mTargetInfo.mIsAiming && mActiveIndex >= 0 && mWeapon[mActiveIndex] &&
          mWeapon[mActiveIndex]->ZoomFirstPerson()) ||
         (mTracker && mTracker->IsFirstPersonView())) &&
        !mFlyingInFirstPerson)
    {
        int iFirstPerson = GetFirstPersonIndex();
        if (iFirstPerson >= 0 && !mTargetInfo.mIsAiming)
        {
            FirstPerson::UpdateAimer(iFirstPerson, rMatrix);
        }
    }
}

// FUNCTION: SWBF2 0x004f2e70
void EntitySoldier::Move(const PblMatrix& mMatrix, const PblVector3& vVelocity)
{
    static_cast<CollisionObject*>(this)->SetMatrix(mMatrix);
    mVelocity = vVelocity;
    RedSceneObject* pScene = this;
    pScene->Deactivate();
    pScene->Activate();
}

// FUNCTION: SWBF2 0x004f2b10
void EntitySoldier::SetupCameraFOV(float& rFOV)
{
    if (!IsAlive())
    {
        rFOV = GetDefaultFOV();
        return;
    }
    if (mTargetInfo.mIsAiming || (mTracker && mTracker->IsFirstPersonView()))
    {
        rFOV = GetControllableClass()->mFirstPersonFOV;
    }
    else
    {
        rFOV = GetControllableClass()->mThirdPersonFOV;
    }
}

// FUNCTION: SWBF2 0x004f2b80
void EntitySoldier::SetupCameraZoomFactor(float& rZoom)
{
    Weapon* pWeapon;
    if (mTargetInfo.mIsAiming && mActiveIndex >= 0 && (pWeapon = mWeapon[mActiveIndex]) != 0 && IsAlive())
    {
        rZoom = pWeapon->mZoom;
        if (mPrevZoom != -1.0f && rZoom != mPrevZoom)
        {
            int iJoystick = NetGame::GetJoystickIndex(mPlayerId);
            if (iJoystick >= 0)
            {
                if (ScopeDisplay* pScope = ScopeDisplay::Instance(iJoystick))
                {
                    pScope->mIsZooming = false;
                }
            }
        }
        mPrevZoom = rZoom;
    }
    else
    {
        rZoom = 1.0f;
    }
}

// FUNCTION: SWBF2 0x004f2c40
void EntitySoldier::SetupCameraIconVisibility(unsigned int& rMask)
{
    rMask = 0;
    if (!IsAlive())
    {
        return;
    }
    int iCount = GetWeaponCount();
    for (int i = 0; i < iCount; ++i)
    {
        if (GetWeaponPtr(i) && GetWeaponPtr(i)->IsRtti(WeaponRepair::rttiHashWeaponRepair))
        {
            rMask |= 1;
            return;
        }
    }
}

// FUNCTION: SWBF2 0x004f2e00
bool EntitySoldier::ShouldShowReticule() const
{
    if (mActiveIndex >= 0 && mWeapon[mActiveIndex] && mWeapon[mActiveIndex]->ShouldShowReticule() &&
        (!mWeapon[mActiveIndex]->mClass->mReticuleInAimingOnly || mTargetInfo.mIsAiming))
    {
        return IsAlive();
    }
    return false;
}

// FUNCTION: SWBF2 0x004f2cb0
int EntitySoldier::GetTargetBodyID(const PblVector3& vPos, const PblVector3& vDir) const
{
    return -1;
}

// FUNCTION: SWBF2 0x004f2cc0
PblVector3 EntitySoldier::GetTargetPoint(const PblVector3& vPos, const PblVector3& vDir, int iBodyId) const
{
    PblVector3 vPoint;
    D3DXVec3TransformCoord(&vPoint, &mClass->mTargetPointOffset[mStanceIndex], &mMatrix);
    return vPoint;
}

// FUNCTION: SWBF2 0x004f2d00
float EntitySoldier::GetSoundRadius() const
{
    float fWeaponRadius = 0.0f;
    const Weapon* pWeapon = GetCurWpn();
    if (pWeapon)
    {
        fWeaponRadius = pWeapon->GetSoundRadius() * 0.5f;
    }
    const PblVector3& vVelocity = GetVelocity();
    float fRadius = sqrtf(vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z) *
                    mClass->mNoiseFactor;
    if (fRadius < 0.0f)
    {
        fRadius = 0.0f;
    }
    else if (fRadius > 50.0f)
    {
        fRadius = 50.0f;
    }
    return (fWeaponRadius > fRadius) ? fWeaponRadius : fRadius;
}

// FUNCTION: SWBF2 0x004f3290
void EntitySoldier::ActivatePhysics()
{
    ActivateThread(0, PER_TURN);
    mPostCollision.ActivateThread(-15, PER_TURN);
    for (int iChannel = 0; iChannel < 2; ++iChannel)
    {
        int iIndex = mWeaponIndex[iChannel];
        if (iIndex < 0)
        {
            continue;
        }
        Weapon* pWeapon = mWeapon[iIndex];
        if (pWeapon && !pWeapon->mSelectedFlag)
        {
            pWeapon->Select(iChannel, false);
        }
    }
}

// FUNCTION: SWBF2 0x004f34d0
bool EntitySoldier::RenderTrackable(unsigned int uView) const
{
    if (mFlyingInFirstPerson || !IsAlive() || mAttachedToProp)
    {
        return true;
    }
    if (mTargetInfo.mIsAiming && mActiveIndex >= 0)
    {
        const Weapon* pWeapon = mWeapon[mActiveIndex];
        if (pWeapon && pWeapon->ZoomFirstPerson() &&
            NetGame::GetJoystickIndex(uView) == (int)FLRenderer::GetCurrentViewport())
        {
            return false;
        }
    }
    const Controllable* pControllable = GetControllable();
    if (pControllable && pControllable->GetEntity()->IsRtti(PassengerSlot::rttiHashPassengerSlot))
    {
        return false;
    }
    return Trackable::RenderTrackable(uView);
}

// FUNCTION: SWBF2 0x004f3680
float EntitySoldier::GetTargetBubbleStanceModifier() const
{
    switch (mState)
    {
    case SoldierState::CROUCH:
        return 1.5f;
    case SoldierState::PRONE:
    case SoldierState::SPRINT:
    case SoldierState::JUMP:
        return 2.0f;
    case SoldierState::ROLL:
        return 3.0f;
    }
    return 1.0f;
}

// FUNCTION: SWBF2 0x004f36c0
float EntitySoldier::GetMaxSpeed() const
{
    return mClass->mMaxSpeed;
}

// FUNCTION: SWBF2 0x004f5000
EntitySoldierClass::EntitySoldierClass(unsigned int uHash)
    : GameObjectClass(uHash)
{
    m_bDynamicLighting = true;
    mLODClass = 2;
    mHealthType = HEALTH_TYPE_PERSON;
    mLockOnMinDist = 0.0f;
    mLockOnMaxDist = 15.0f;
    mLockOnMaxTrackDist = 500.0f;
    mPilotType = PILOT_SELF;

    mCanCapturePosts = true;
    mCanEnterVehicles = true;

    mModelLowRes = 0;
    mAcceleration = 70.0f;
    mMaxSpeed = 9.0f;
    mMaxStrafeSpeed = 7.0f;
    mMaxTurnSpeed = 2.0f;
    mMaxPitchSpeed = 1.0f;
    mMinPitch = -0.8f;
    mMaxPitch = 1.1f;
    mJumpVerticalSpeed = 8.0f;
    mJumpForwardSpeedFactor = 1.3f;
    mJumpStrafeSpeedFactor = 1.0f;
    mRollSpeedFactor = 1.7f;
    mSprintAccelerateTime = 0.5f;

    m_bRecoverFromTumble = 0;
    m_bUseDirectionalJumps = 1;
    mJetShowHud = 1;
    mJetTypeHover = 0;
    m_bIsAcklay = 0;
    m_bUseDirectionalDeaths = 1;
    m_numTentacles = 0;
    m_bonesPerTentacle = 0;
    m_tentacleCollType = 0;

    mThrustFactor[0] = 1.0f;
    mThrustFactor[1] = 0.7f;
    mThrustFactor[2] = 0.3f;
    mThrustFactor[3] = 1.5f;
    mThrustFactor[4] = 0.03f;
    mThrustFactor[5] = 0.02f;
    mThrustFactor[6] = 0.3f;
    mThrustFactor[7] = 0.0f;

    mStrafeFactor[0] = 1.0f;
    mStrafeFactor[1] = 0.6f;
    mStrafeFactor[2] = 0.2f;
    mStrafeFactor[3] = 0.5f;
    mStrafeFactor[4] = 0.03f;
    mStrafeFactor[5] = 0.02f;
    mStrafeFactor[6] = 0.3f;
    mStrafeFactor[7] = 0.0f;

    mTurnFactor[0] = 1.0f;
    mTurnFactor[1] = 1.0f;
    mTurnFactor[2] = 0.5f;
    mTurnFactor[3] = 0.2f;
    mTurnFactor[4] = 0.5f;
    mTurnFactor[5] = 0.6f;
    mTurnFactor[6] = 1.0f;
    mTurnFactor[7] = 0.1f;

    mCollisionThreshold[0] = 10.0f;
    mCollisionThreshold[1] = 3.0f;
    mCollisionThreshold[2] = 0.0f;
    mCollisionScale[0] = 1.0f;
    mCollisionScale[1] = 0.0f;
    mCollisionScale[2] = 0.0f;
    mCollisionInflict = 1.0f;

    for (int i = 0; i < 8; ++i)
    {
        mWeaponClass[i] = 0;
        mWeaponAmmo[i].uiAmmo = 0;
        mWeaponAmmo[i].uiHeat = 0;
        mWeaponAmmo[i].bFlagAmmoInfinite = 0;
        mWeaponAmmo[i].bFlagShareHeat = 0;
        mWeaponAmmo[i].bFlagShareAmmo = 0;
        mWeaponChannel[i] = 0;
    }

    mWeaponCount = 0;
    mOverrideTextureHash = 0;
    mOverrideTextureHash2 = 0;

    mEnergyBarClass.m_fEnergyMax = 0.0f;
    mEnergyBarClass.m_fEnergyMin = 0.0f;
    mEnergyBarClass.m_fEnergyOverheat = 0.0f;
    mEnergyBarClass.Init(100.0f, 20.0f);

    mEnergyRestore = 10.0f;
    mEnergyRestoreIdle = 15.0f;
    mEnergyDrainSprint = 20.0f;
    mEnergyCostJump = 10.0f;
    mEnergyCostJumpSprint = 30.0f;
    mEnergyCostRoll = 10.0f;
    mJetJump = 7.0f;
    mJetPush = 14.0f;
    mJetAcceleration = 20.0f;
    mJetFuelRechargeRate = 0.0f;
    mJetFuelCost = 0.2f;
    mJetFuelInitialCost = 0.0f;
    mJetFuelMinBorder = 0.1f;
    mJetEnergyDrain = 0.0f;
    mJetPackEffect = 0;
    mJetPackIdleEffect = 0;

    mHealthGainEffect = 0x9D26C2F2u;
    mAmmoGainEffect = 0x2A72AE31u;
    mEnergyGainEffect = 0x9796A1C5u;
    mBuffHealthEffect = 0xBB3E84E9u;
    mBuffOffenseEffect = 0xC17AB11Au;
    mBuffDefenseEffect = 0x28B9F3ACu;
    mDebuffEffect = 0xCAA14018u;
    mInvincibleEffect = 0x9796A1C5u;
    mFootSlideEffect = 0;
    mChokeEffect = 0;
    m_fBlurEffect = 0.0f;
    mWaterSplashEffect = 0xA40BAAA7u;
    mFootWaterSplashEffect = 0xA40BAAA7u;
    mWakeWaterSplashEffect = 0xA40BAAA7u;

    mNoiseFactor = 5.0f;
    mSoldierType = 5;
    mImmuneToMines = false;

    mAimerClass = new AimerClass();
    mAimerClass->mOffsetPos.x = 0.0f;
    mAimerClass->mOffsetPos.y = 2.0f;
    mAimerClass->mOffsetPos.z = 0.0f;

    mCameraSettings[0][0].mCameraEyePointOffset.x = 0.0f;
    mCameraSettings[0][0].mCameraEyePointOffset.y = 1.8f;
    mCameraSettings[0][0].mCameraEyePointOffset.z = 0.0f;
    mCameraSettings[0][0].mCameraTrackCenter.x = 0.0f;
    mCameraSettings[0][0].mCameraTrackCenter.y = 2.3f;
    mCameraSettings[0][0].mCameraTrackCenter.z = 0.0f;
    mCameraSettings[0][0].mCameraTrackOffset.x = 0.0f;
    mCameraSettings[0][0].mCameraTrackOffset.y = 0.0f;
    mCameraSettings[0][0].mCameraTrackOffset.z = 4.5f;
    mCameraSettings[0][0].mCameraMoveTensionL = 15.0f;
    mCameraSettings[0][0].mCameraMoveTensionR = 15.0f;
    mCameraSettings[0][0].mCameraMoveTensionU = 15.0f;
    mCameraSettings[0][0].mCameraMoveTensionD = 15.0f;
    mCameraSettings[0][0].mCameraMoveTensionF = 15.0f;
    mCameraSettings[0][0].mCameraMoveTensionB = 15.0f;
    mCameraSettings[0][0].mCameraAimTension = FLT_MAX;

    mCameraSettings[0][1].mCameraEyePointOffset = mCameraSettings[0][0].mCameraEyePointOffset;
    mCameraSettings[0][1].mCameraTrackCenter = mCameraSettings[0][0].mCameraEyePointOffset;
    mCameraSettings[0][1].mCameraTrackOffset = gVectorZero;
    mCameraSettings[0][1].mCameraMoveTensionL = FLT_MAX;
    mCameraSettings[0][1].mCameraMoveTensionR = FLT_MAX;
    mCameraSettings[0][1].mCameraMoveTensionU = FLT_MAX;
    mCameraSettings[0][1].mCameraMoveTensionD = FLT_MAX;
    mCameraSettings[0][1].mCameraMoveTensionF = FLT_MAX;
    mCameraSettings[0][1].mCameraMoveTensionB = FLT_MAX;
    mCameraSettings[0][1].mCameraAimTension = FLT_MAX;

    mCameraSettings[0][2].mCameraEyePointOffset.x = 0.0f;
    mCameraSettings[0][2].mCameraEyePointOffset.y = 1.2f;
    mCameraSettings[0][2].mCameraEyePointOffset.z = 0.0f;
    mCameraSettings[0][2].mCameraTrackCenter.x = 0.0f;
    mCameraSettings[0][2].mCameraTrackCenter.y = 1.3f;
    mCameraSettings[0][2].mCameraTrackCenter.z = 0.0f;
    mCameraSettings[0][2].mCameraTrackOffset.x = 0.0f;
    mCameraSettings[0][2].mCameraTrackOffset.y = 0.0f;
    mCameraSettings[0][2].mCameraTrackOffset.z = 3.5f;
    mCameraSettings[0][2].mCameraMoveTensionL = 15.0f;
    mCameraSettings[0][2].mCameraMoveTensionR = 15.0f;
    mCameraSettings[0][2].mCameraMoveTensionU = 15.0f;
    mCameraSettings[0][2].mCameraMoveTensionD = 15.0f;
    mCameraSettings[0][2].mCameraMoveTensionF = 15.0f;
    mCameraSettings[0][2].mCameraMoveTensionB = 15.0f;
    mCameraSettings[0][2].mCameraAimTension = FLT_MAX;

    mCameraSettings[0][3].mCameraEyePointOffset = mCameraSettings[0][2].mCameraEyePointOffset;
    mCameraSettings[0][3].mCameraTrackCenter = mCameraSettings[0][2].mCameraEyePointOffset;
    mCameraSettings[0][3].mCameraTrackOffset = gVectorZero;
    mCameraSettings[0][3].mCameraMoveTensionL = FLT_MAX;
    mCameraSettings[0][3].mCameraMoveTensionR = FLT_MAX;
    mCameraSettings[0][3].mCameraMoveTensionU = FLT_MAX;
    mCameraSettings[0][3].mCameraMoveTensionD = FLT_MAX;
    mCameraSettings[0][3].mCameraMoveTensionF = FLT_MAX;
    mCameraSettings[0][3].mCameraMoveTensionB = FLT_MAX;
    mCameraSettings[0][3].mCameraAimTension = FLT_MAX;

    mCameraSettings[1][0].mCameraEyePointOffset.x = 0.0f;
    mCameraSettings[1][0].mCameraEyePointOffset.y = 0.3f;
    mCameraSettings[1][0].mCameraEyePointOffset.z = 0.0f;
    mCameraSettings[1][0].mCameraTrackCenter.x = 0.0f;
    mCameraSettings[1][0].mCameraTrackCenter.y = 0.5f;
    mCameraSettings[1][0].mCameraTrackCenter.z = 0.0f;
    mCameraSettings[1][0].mCameraTrackOffset.x = 0.0f;
    mCameraSettings[1][0].mCameraTrackOffset.y = 0.0f;
    mCameraSettings[1][0].mCameraTrackOffset.z = 2.5f;
    mCameraSettings[1][0].mCameraMoveTensionL = 15.0f;
    mCameraSettings[1][0].mCameraMoveTensionR = 15.0f;
    mCameraSettings[1][0].mCameraMoveTensionU = 15.0f;
    mCameraSettings[1][0].mCameraMoveTensionD = 15.0f;
    mCameraSettings[1][0].mCameraMoveTensionF = 15.0f;
    mCameraSettings[1][0].mCameraMoveTensionB = 15.0f;
    mCameraSettings[1][0].mCameraAimTension = FLT_MAX;

    mCameraSettings[1][1].mCameraEyePointOffset = mCameraSettings[1][0].mCameraEyePointOffset;
    mCameraSettings[1][1].mCameraTrackCenter = mCameraSettings[1][0].mCameraEyePointOffset;
    mCameraSettings[1][1].mCameraTrackOffset = gVectorZero;
    mCameraSettings[1][1].mCameraMoveTensionL = FLT_MAX;
    mCameraSettings[1][1].mCameraMoveTensionR = FLT_MAX;
    mCameraSettings[1][1].mCameraMoveTensionU = FLT_MAX;
    mCameraSettings[1][1].mCameraMoveTensionD = FLT_MAX;
    mCameraSettings[1][1].mCameraMoveTensionF = FLT_MAX;
    mCameraSettings[1][1].mCameraMoveTensionB = FLT_MAX;
    mCameraSettings[1][1].mCameraAimTension = FLT_MAX;

    mCameraSettings[1][2].mCameraEyePointOffset.x = 0.0f;
    mCameraSettings[1][2].mCameraEyePointOffset.y = 1.8f;
    mCameraSettings[1][2].mCameraEyePointOffset.z = 0.0f;
    mCameraSettings[1][2].mCameraTrackCenter.x = 0.0f;
    mCameraSettings[1][2].mCameraTrackCenter.y = 2.3f;
    mCameraSettings[1][2].mCameraTrackCenter.z = 0.0f;
    mCameraSettings[1][2].mCameraTrackOffset.x = 0.0f;
    mCameraSettings[1][2].mCameraTrackOffset.y = 0.0f;
    mCameraSettings[1][2].mCameraTrackOffset.z = 4.5f;
    mCameraSettings[1][2].mCameraMoveTensionL = 15.0f;
    mCameraSettings[1][2].mCameraMoveTensionR = 15.0f;
    mCameraSettings[1][2].mCameraMoveTensionU = 15.0f;
    mCameraSettings[1][2].mCameraMoveTensionD = 15.0f;
    mCameraSettings[1][2].mCameraMoveTensionF = 15.0f;
    mCameraSettings[1][2].mCameraMoveTensionB = 15.0f;
    mCameraSettings[1][2].mCameraAimTension = FLT_MAX;

    mCameraSettings[1][3].mCameraEyePointOffset = mCameraSettings[1][2].mCameraEyePointOffset;
    mCameraSettings[1][3].mCameraTrackCenter = mCameraSettings[1][2].mCameraEyePointOffset;
    mCameraSettings[1][3].mCameraTrackOffset = gVectorZero;
    mCameraSettings[1][3].mCameraMoveTensionL = FLT_MAX;
    mCameraSettings[1][3].mCameraMoveTensionR = FLT_MAX;
    mCameraSettings[1][3].mCameraMoveTensionU = FLT_MAX;
    mCameraSettings[1][3].mCameraMoveTensionD = FLT_MAX;
    mCameraSettings[1][3].mCameraMoveTensionF = FLT_MAX;
    mCameraSettings[1][3].mCameraMoveTensionB = FLT_MAX;
    mCameraSettings[1][3].mCameraAimTension = FLT_MAX;

    mCameraSettingBlendTime[0] = 0.75f;
    mCameraSettingBlendTime[1] = 1.0f;
    mCameraSettingBlendTime[2] = 0.75f;

    for (int i = 0; i < 3; ++i)
    {
        mCollisionModels[i].mDeathOnFlyerLand = true;
    }

    m_vCollisionHeadOffset = gVectorZero;
    m_vCollisionTorsoOffset = gVectorZero;
    m_fCollisionHeadScale = 1.0f;
    m_fCollisionTorsoScale = 1.0f;

    mSpecialSkeleton = 0;
    for (int i = 0; i < 4; ++i)
    {
        mUpperBodyAnimMask[i] = 0;
        mLowerBodyAnimMask[i] = 0;
    }
    mSpecialSkeletonLowRes = 0;
    mAddon = 0;
    mFootstepStride = 1.75f;

    mShockFadeOutTime = 0.5f;
    mShockFadeInTime = 3.0f;
    mShockFadeOutGain = 1.0f;
    mLowHealthThreshold = 0.3f;

    mDropItemCount = 0;
    for (int i = 0; i < 8; ++i)
    {
        mDropItemClass[i] = 0;
        mDropItemProbability[i] = 0.0f;
    }

    mDrownTime = 3.0f;
    mChunkFrequency = -1.0f;
    mSkeletonRootScale = 1.0f;
    mCollisionRootScale = 1.0f;
    mSkeletonRootScaleLowRes = 1.0f;
    mMinFootHeight = 0.17f;
    mAnimationBank = SoldierAnimationBank::HUMAN;
    mFirstPersonModelHandle = -1;

    mSndHeroSelectable = 0;
    mSndHeroSpawned = 0;
    mSndHeroDefeated = 0;
    mSndForHeroKiller = 0;

    mFleeLikeAHero = 0;

    mTargetPointOffset[0].x = 0.0f;
    mTargetPointOffset[0].y = 1.4f;
    mTargetPointOffset[0].z = 0.0f;
    mTargetPointOffset[1].x = 0.0f;
    mTargetPointOffset[1].y = 0.6f;
    mTargetPointOffset[1].z = 0.0f;
    mTargetPointOffset[2].x = 0.0f;
    mTargetPointOffset[2].y = 0.2f;
    mTargetPointOffset[2].z = 0.0f;
}

// FUNCTION: SWBF2 0x004f5cc0
EntitySoldierClass::EntitySoldierClass(EntitySoldierClass* pBase, unsigned int uHash)
    : GameObjectClass(pBase, uHash),
      ControllableClass(pBase),
      VehicleEngineClass(pBase),
      PlayableEntitySoundsClass(pBase)
{
    mEnergyBarClass.m_fEnergyMax = 0.0f;
    mEnergyBarClass.m_fEnergyMin = 0.0f;
    mEnergyBarClass.m_fEnergyOverheat = 0.0f;

    mLODClass = 2;
    sCurrentCameraStance = 0;
    sCurrentCameraZoomed = 0;
    sWeaponSection = 0;
    sCurDropItem = 0;

    mLodData = pBase->mLodData;
    mModelLowRes = pBase->mModelLowRes;
    mAcceleration = pBase->mAcceleration;
    mMaxSpeed = pBase->mMaxSpeed;
    mMaxStrafeSpeed = pBase->mMaxStrafeSpeed;
    mMaxTurnSpeed = pBase->mMaxTurnSpeed;
    mMaxPitchSpeed = pBase->mMaxPitchSpeed;
    mMinPitch = pBase->mMinPitch;
    mMaxPitch = pBase->mMaxPitch;
    mJumpVerticalSpeed = pBase->mJumpVerticalSpeed;
    mJumpForwardSpeedFactor = pBase->mJumpForwardSpeedFactor;
    mJumpStrafeSpeedFactor = pBase->mJumpStrafeSpeedFactor;
    mRollSpeedFactor = pBase->mRollSpeedFactor;
    mSprintAccelerateTime = pBase->mSprintAccelerateTime;
    m_bRecoverFromTumble = pBase->m_bRecoverFromTumble;
    mJetShowHud = pBase->mJetShowHud;
    mJetTypeHover = pBase->mJetTypeHover;
    m_bUseDirectionalJumps = pBase->m_bUseDirectionalJumps;
    m_bUseDirectionalDeaths = pBase->m_bUseDirectionalDeaths;
    m_bonesPerTentacle = pBase->m_bonesPerTentacle;
    m_numTentacles = pBase->m_numTentacles;
    m_tentacleCollType = pBase->m_tentacleCollType;
    m_bIsAcklay = pBase->m_bIsAcklay;
    for (int i = 0; i < 8; ++i)
    {
        mThrustFactor[i] = pBase->mThrustFactor[i];
    }
    for (int i = 0; i < 8; ++i)
    {
        mStrafeFactor[i] = pBase->mStrafeFactor[i];
    }
    for (int i = 0; i < 8; ++i)
    {
        mTurnFactor[i] = pBase->mTurnFactor[i];
    }
    for (int i = 0; i < 3; ++i)
    {
        mCollisionThreshold[i] = pBase->mCollisionThreshold[i];
    }
    for (int i = 0; i < 3; ++i)
    {
        mCollisionScale[i] = pBase->mCollisionScale[i];
    }
    mCollisionInflict = pBase->mCollisionInflict;
    for (int i = 0; i < 8; ++i)
    {
        mWeaponClass[i] = pBase->mWeaponClass[i];
        mWeaponAmmo[i] = pBase->mWeaponAmmo[i];
        mWeaponChannel[i] = pBase->mWeaponChannel[i];
    }
    mWeaponCount = pBase->mWeaponCount;
    mOverrideTextureHash = pBase->mOverrideTextureHash;
    mOverrideTextureHash2 = pBase->mOverrideTextureHash2;
    mEnergyBarClass.Init(pBase->mEnergyBarClass.m_fEnergyMax, pBase->mEnergyBarClass.m_fEnergyOverheat);
    mEnergyRestore = pBase->mEnergyRestore;
    mEnergyRestoreIdle = pBase->mEnergyRestoreIdle;
    mEnergyDrainSprint = pBase->mEnergyDrainSprint;
    mEnergyCostJump = pBase->mEnergyCostJump;
    mEnergyCostJumpSprint = pBase->mEnergyCostJumpSprint;
    mEnergyCostRoll = pBase->mEnergyCostRoll;
    mJetJump = pBase->mJetJump;
    mJetPush = pBase->mJetPush;
    mJetAcceleration = pBase->mJetAcceleration;
    mJetFuelRechargeRate = pBase->mJetFuelRechargeRate;
    mJetFuelCost = pBase->mJetFuelCost;
    mJetFuelInitialCost = pBase->mJetFuelInitialCost;
    mJetFuelMinBorder = pBase->mJetFuelMinBorder;
    mJetEnergyDrain = pBase->mJetEnergyDrain;
    mJetPackEffect = pBase->mJetPackEffect;
    mJetPackIdleEffect = pBase->mJetPackIdleEffect;
    mHealthGainEffect = pBase->mHealthGainEffect;
    mAmmoGainEffect = pBase->mAmmoGainEffect;
    mEnergyGainEffect = pBase->mEnergyGainEffect;
    mBuffHealthEffect = pBase->mBuffHealthEffect;
    mBuffOffenseEffect = pBase->mBuffOffenseEffect;
    mBuffDefenseEffect = pBase->mBuffDefenseEffect;
    mDebuffEffect = pBase->mDebuffEffect;
    mInvincibleEffect = pBase->mInvincibleEffect;
    mFootSlideEffect = pBase->mFootSlideEffect;
    mChokeEffect = pBase->mChokeEffect;
    m_fBlurEffect = pBase->m_fBlurEffect;
    mWaterSplashEffect = pBase->mWaterSplashEffect;
    mFootWaterSplashEffect = pBase->mFootWaterSplashEffect;
    mWakeWaterSplashEffect = pBase->mFootWaterSplashEffect;
    mNoiseFactor = pBase->mNoiseFactor;
    mSoldierType = pBase->mSoldierType;
    mImmuneToMines = pBase->mImmuneToMines;
    mAimerClass = pBase->mAimerClass;
    for (int r = 0; r < 2; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            mCameraSettings[r][c] = pBase->mCameraSettings[r][c];
        }
    }
    for (int i = 0; i < 3; ++i)
    {
        mCameraSettingBlendTime[i] = pBase->mCameraSettingBlendTime[i];
    }
    for (int i = 0; i < 3; ++i)
    {
        mCollisionModels[i].mDeathOnFlyerLand = pBase->mCollisionModels[i].mDeathOnFlyerLand;
    }
    m_vCollisionHeadOffset = pBase->m_vCollisionHeadOffset;
    m_vCollisionTorsoOffset = pBase->m_vCollisionTorsoOffset;
    m_fCollisionHeadScale = pBase->m_fCollisionHeadScale;
    m_fCollisionTorsoScale = pBase->m_fCollisionTorsoScale;
    mSpecialSkeleton = pBase->mSpecialSkeleton;
    for (int i = 0; i < 4; ++i)
    {
        mUpperBodyAnimMask[i] = pBase->mUpperBodyAnimMask[i];
    }
    for (int i = 0; i < 4; ++i)
    {
        mLowerBodyAnimMask[i] = pBase->mLowerBodyAnimMask[i];
    }
    mSpecialSkeletonLowRes = pBase->mSpecialSkeletonLowRes;
    mAddon = pBase->mAddon;
    SoldierAnimatorClass::Create();
    mFootstepStride = pBase->mFootstepStride;
    mRunSpeed = pBase->mRunSpeed;
    mAcquiredTargetSound = pBase->mAcquiredTargetSound;
    mHidingSound = pBase->mHidingSound;
    mApproachingTargetSound = pBase->mApproachingTargetSound;
    mFleeSound = pBase->mFleeSound;
    mPreparingForDamageSound = pBase->mPreparingForDamageSound;
    mHeardEnemySound = pBase->mHeardEnemySound;
    mChargeSound = pBase->mChargeSound;
    mAmbientSound = pBase->mAmbientSound;
    mShockFadeOutTime = pBase->mShockFadeOutTime;
    mShockFadeInTime = pBase->mShockFadeInTime;
    mShockFadeOutGain = pBase->mShockFadeOutGain;
    mShockSound = pBase->mShockSound;
    mClothingRustleSound = pBase->mClothingRustleSound;
    mLowHealthSound = pBase->mLowHealthSound;
    mChokeSound = pBase->mChokeSound;
    mLowHealthThreshold = pBase->mLowHealthThreshold;
    mDropItemCount = pBase->mDropItemCount;
    for (int i = 0; i < 8; ++i)
    {
        mDropItemClass[i] = pBase->mDropItemClass[i];
    }
    for (int i = 0; i < 8; ++i)
    {
        mDropItemProbability[i] = pBase->mDropItemProbability[i];
    }
    mDrownTime = pBase->mDrownTime;
    mChunkFrequency = pBase->mChunkFrequency;
    mSkeletonRootScale = pBase->mSkeletonRootScale;
    mCollisionRootScale = pBase->mCollisionRootScale;
    mSkeletonRootScaleLowRes = pBase->mSkeletonRootScaleLowRes;
    mAnimationBank = pBase->mAnimationBank;
    mFirstPersonModelHandle = pBase->mFirstPersonModelHandle;
    mExplosion = pBase->mExplosion;
    for (int i = 0; i < 3; ++i)
    {
        mUpperBodyId[i] = pBase->mUpperBodyId[i];
    }
    for (int i = 0; i < 3; ++i)
    {
        mLowerBodyId[i] = pBase->mLowerBodyId[i];
    }
    mSndHeroSelectable = pBase->mSndHeroSelectable;
    mSndHeroSpawned = pBase->mSndHeroSpawned;
    mSndHeroDefeated = pBase->mSndHeroDefeated;
    mSndForHeroKiller = pBase->mSndForHeroKiller;
    mFleeLikeAHero = pBase->mFleeLikeAHero;
    mSoldierMusic = pBase->mSoldierMusic;
    mAISoldierMusic = pBase->mAISoldierMusic;
    mSoldierAnnouncement = pBase->mSoldierAnnouncement;
    for (int i = 0; i < 3; ++i)
    {
        mTargetPointOffset[i] = pBase->mTargetPointOffset[i];
    }
    mMinFootHeight = pBase->mMinFootHeight;
}

// FUNCTION: SWBF2 0x004f67c0
void EntitySoldierClass::PostReadSetup()
{
    if (gMidgetTime && !NetGame::IsEnabled())
    {
        char szBank[32] = "jawa";
        char szLowResBank[32] = "jawalz";

        RedAnimation* pAnimation = RedAnimation::Find(PblHash(szBank));
        if (!pAnimation || !pAnimation->_pSkeletonShared)
        {
            strlcpy(szBank, "human", sizeof(szBank));
            strlcpy(szLowResBank, "humanlz", sizeof(szLowResBank));
            pAnimation = RedAnimation::Find(PblHash(szBank));
        }
        mSpecialSkeleton = pAnimation->_pSkeletonShared;
        SoldierAnimatorClass::SetupJointTransformAnimationFlag(mSpecialSkeleton);
        SoldierAnimatorClass::SetupBodyMasks(mSpecialSkeleton, *(PblBitVector<32>*)mUpperBodyAnimMask,
                                             *(PblBitVector<32>*)mLowerBodyAnimMask);
        mSkeletonRootScale = 0.6f;
        if (mSpecialSkeleton && mSpecialSkeleton != SoldierAnimatorClass::Instance()->GetDefaultSkeleton())
        {
            int iJoint = (int)mSpecialSkeleton->m_kHashCRC2Joint.Find(0x16e27226) - 1;
            mSpecialSkeleton->SetJointScale(iJoint, mSkeletonRootScale);
        }

        pAnimation = RedAnimation::Find(PblHash(szLowResBank));
        if (!pAnimation || !pAnimation->_pSkeletonShared)
        {
            strlcpy(szLowResBank, "humanlz", sizeof(szLowResBank));
            pAnimation = RedAnimation::Find(PblHash(szLowResBank));
        }
        mSpecialSkeletonLowRes = pAnimation->_pSkeletonShared;
        SoldierAnimatorLowResClass::AddBank(szLowResBank, "humanlz");
    }

    for (int i = 0; i < 8; ++i)
    {
        const WeaponClass* pWeapon = mWeaponClass[i];
        if (pWeapon && !pWeapon->mIsOffhand)
        {
            SoldierAnimationBank::WEAPON eWeapon = (SoldierAnimationBank::WEAPON)pWeapon->mSoldierAnimationWeapon;
            if (eWeapon == SoldierAnimationBank::INVALID_WEAPON)
            {
                eWeapon = SoldierAnimationBank::RIFLE;
            }
            SoldierAnimationBank::AddMap(mAnimationBank, eWeapon);
        }
    }

    const float fScale = mCollisionRootScale;
    const float fLowerRadius = 0.57000005f;
    CollisionPrimitive primitive;
    PblMatrix matrix = _IdentityMatrix;
    float fHeight = fScale * 0.8f - 0.1925f;

    if (!m_bIsAcklay)
    {
        float fHeadScale = m_fCollisionHeadScale * 0.17f;
        float fTorsoRadius = m_fCollisionTorsoScale * 0.4f;
        float fHeadRadius = (fScale > 1.0f ? fScale : 1.0f) * fHeadScale;

        primitive.SetSphere(fHeadRadius);
        float fHeadY = fScale * 1.65f;
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = fHeadY;
        matrix.m[3][2] = 0.16f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[0].AddBody(new CollisionBodyPrimitive(primitive, "coll_head", matrix, 0));

        fTorsoRadius *= fScale;
        primitive.SetSphere(fTorsoRadius);
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = fScale * 1.15f;
        matrix.m[3][2] = 0.0f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[0].AddBody(new CollisionBodyPrimitive(primitive, "coll_torso", matrix, 0));

        primitive.SetAACylinder(0.35f, fScale * 1.1f);
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = fScale * 0.55f;
        matrix.m[3][2] = 0.0f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[0].AddBody(new CollisionBodyPrimitive(primitive, "coll_body", matrix, 0));

        primitive.SetAACylinder(fLowerRadius, fHeadY);
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = fScale * 0.9f;
        matrix.m[3][2] = 0.0f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[0].AddBody(new CollisionBodyPrimitive(primitive, "main_body", matrix, 0));

        primitive.SetSphere(fLowerRadius);
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = fLowerRadius;
        matrix.m[3][2] = 0.15f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[0].AddBody(new CollisionBodyPrimitive(primitive, "coll_body_lower", matrix, 0));

        matrix.m[3][3] = 1.0f;
        float fUpperY = fScale * 1.9f - fLowerRadius;
        if (fUpperY > fLowerRadius)
        {
            matrix.m[3][0] = 0.0f;
            matrix.m[3][1] = fUpperY + 0.15f;
            matrix.m[3][2] = 0.15f;
        }
        else
        {
            matrix.m[3][1] = 0.62700003f;
        }
        mCollisionModels[0].AddBody(new CollisionBodyPrimitive(primitive, "coll_body_upper", matrix, 0));
        mCameraSettings[0][0].mCameraSafePoint = matrix.GetTranslation();
        mCameraSettings[0][1].mCameraSafePoint = matrix.GetTranslation();
        mCameraSettings[1][2].mCameraSafePoint = matrix.GetTranslation();
        mCameraSettings[1][3].mCameraSafePoint = matrix.GetTranslation();

        primitive.SetSphere(fHeadRadius);
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = fScale * 1.2f;
        matrix.m[3][2] = 0.25f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[1].AddBody(new CollisionBodyPrimitive(primitive, "coll_head", matrix, 0));

        primitive.SetSphere(fTorsoRadius);
        float fTorsoY = fScale * 0.7f;
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = fTorsoY;
        matrix.m[3][2] = 0.0f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[1].AddBody(new CollisionBodyPrimitive(primitive, "coll_torso", matrix, 0));

        primitive.SetAACylinder(0.35f, fTorsoY);
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = fScale * 0.35f;
        matrix.m[3][2] = 0.0f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[1].AddBody(new CollisionBodyPrimitive(primitive, "coll_body", matrix, 0));

        primitive.SetAACylinder(0.45f, fScale * 1.45f);
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = fScale * 0.65f;
        matrix.m[3][2] = 0.0f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[1].AddBody(new CollisionBodyPrimitive(primitive, "main_body", matrix, 0));

        primitive.SetSphere(fLowerRadius);
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = fLowerRadius;
        matrix.m[3][2] = 0.15f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[1].AddBody(new CollisionBodyPrimitive(primitive, "coll_body_lower", matrix, 0));

        matrix.m[3][3] = 1.0f;
        fUpperY = fScale * 1.6f - fLowerRadius;
        if (fUpperY > fLowerRadius)
        {
            matrix.m[3][0] = 0.0f;
            matrix.m[3][1] = fUpperY + 0.15f;
            matrix.m[3][2] = 0.15f;
        }
        else
        {
            matrix.m[3][1] = 0.62700003f;
        }
        mCollisionModels[1].AddBody(new CollisionBodyPrimitive(primitive, "coll_body_upper", matrix, 0));
        mCameraSettings[0][2].mCameraSafePoint = matrix.GetTranslation();
        mCameraSettings[0][3].mCameraSafePoint = matrix.GetTranslation();

        primitive.SetSphere(fHeadRadius);
        matrix.m[3][0] = -0.07f;
        matrix.m[3][1] = fScale * 0.28f;
        matrix.m[3][2] = fScale * 0.75f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[2].AddBody(new CollisionBodyPrimitive(primitive, "coll_head", matrix, 0));

        primitive.SetSphere(0.01f);
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = 0.0f;
        matrix.m[3][2] = 0.0f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[2].AddBody(new CollisionBodyPrimitive(primitive, "coll_torso", matrix, 0));

        primitive.SetBox(0.25f, 0.2f, fScale * 0.87f, -1.0f);
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = 0.2f;
        matrix.m[3][2] = fScale * -0.25f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[2].AddBody(new CollisionBodyPrimitive(primitive, "coll_body", matrix, 0));

        primitive.SetAACylinder(0.8f, fScale * 0.45000002f);
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = fScale * 0.25f;
        matrix.m[3][2] = 0.0f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[2].AddBody(new CollisionBodyPrimitive(primitive, "main_body", matrix, 0));

        primitive.SetSphere(0.8f);
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = 0.72f;
        matrix.m[3][2] = fHeight * 0.333f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[2].AddBody(new CollisionBodyPrimitive(primitive, "coll_body_medium", matrix, 0));

        primitive.SetSphere(0.385f);
        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = 0.34649998f;
        matrix.m[3][2] = fHeight;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[2].AddBody(new CollisionBodyPrimitive(primitive, "coll_body_upper", matrix, 0));
        mCameraSettings[1][0].mCameraSafePoint = matrix.GetTranslation();
        mCameraSettings[1][1].mCameraSafePoint = matrix.GetTranslation();

        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = 0.34649998f;
        matrix.m[3][2] = -fHeight;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[2].AddBody(new CollisionBodyPrimitive(primitive, "coll_body_lower", matrix, 0));

        matrix.m[3][0] = 0.0f;
        matrix.m[3][1] = 0.34649998f;
        matrix.m[3][2] = fHeight * -0.333f;
        matrix.m[3][3] = 1.0f;
        mCollisionModels[2].AddBody(new CollisionBodyPrimitive(primitive, "coll_body_medium2", matrix, 0));

        if (mNumHitLocations == 0)
        {
            mHitLocations = new HitLocationData[1];
            mHitLocations[mNumHitLocations].mBodyId = 0;
            mHitLocations[mNumHitLocations].mDamageMultiplier = 3.0f;
            ++mNumHitLocations;
        }

        if (mAddon)
        {
            mAddon->PostReadSetup();
        }

        for (int i = 0; i < 3; ++i)
        {
            CollisionModel& model = mCollisionModels[i];
            mLowerBodyId[i] = model.GetMaskId(0x9458FB2E);
            mUpperBodyId[i] = model.GetMaskId(0x2C31EC5B);

            unsigned int uLegs = 0;
            if (mLowerBodyId[i] >= 0)
            {
                uLegs = 1 << mLowerBodyId[i];
            }
            if (mUpperBodyId[i] >= 0)
            {
                uLegs |= 1 << mUpperBodyId[i];
            }
            int iId = model.GetMaskId(0x0DB3CAEA);
            if (iId >= 0)
            {
                uLegs |= 1 << iId;
            }
            iId = model.GetMaskId(0x6A085208);
            if (iId >= 0)
            {
                uLegs |= 1 << iId;
            }
            model.SetCollisionMask(COLL_TERRAIN, CollisionMask(uLegs));
            model.SetCollisionMask(COLL_STATIC, CollisionMask(uLegs));
            model.SetCollisionMask(COLL_RIGID, CollisionMask(uLegs));

            model.SetCollisionMask(COLL_SOFT, CollisionMask(1 << model.GetMaskId(0xFE858165)));

            unsigned int uHit = 1 << model.GetMaskId(0xCA25B9F3);
            uHit |= 1 << model.GetMaskId(0xE8B9E01A);
            uHit |= 1 << model.GetMaskId(0x34FCD074);
            model.SetCollisionMask(COLL_ORDNANCE, CollisionMask(uHit));
        }
    }
    else
    {
        char Buffer[128];
        int i = 0;
        CollisionBodyPrimitive* pBody;
        do
        {
            _snprintf(Buffer, 127, "geo_inf_acklay%d", i);
            pBody = CollisionBodyPrimitive::_HashTable.Find(PblHash(Buffer));
            if (pBody)
            {
                mCollisionModels[0].AddBody(pBody);
            }
            ++i;
        } while (pBody);
    }

    primitive.SetSphere(0.8f);
    matrix.m[3][0] = 0.0f;
    matrix.m[3][1] = 0.72f;
    matrix.m[3][2] = fHeight * 0.333f;
    matrix.m[3][3] = 1.0f;
    mDeadCollision.AddBody(new CollisionBodyPrimitive(primitive, "coll_body_medium", matrix, 0));

    primitive.SetSphere(0.385f);
    matrix.m[3][0] = 0.0f;
    matrix.m[3][1] = 0.34649998f;
    matrix.m[3][2] = fHeight;
    matrix.m[3][3] = 1.0f;
    mDeadCollision.AddBody(new CollisionBodyPrimitive(primitive, "coll_body_upper", matrix, 0));

    matrix.m[3][0] = 0.0f;
    matrix.m[3][1] = 0.34649998f;
    matrix.m[3][2] = -fHeight;
    matrix.m[3][3] = 1.0f;
    mDeadCollision.AddBody(new CollisionBodyPrimitive(primitive, "coll_body_lower", matrix, 0));

    matrix.m[3][0] = 0.0f;
    matrix.m[3][1] = 0.34649998f;
    matrix.m[3][2] = fHeight * -0.333f;
    matrix.m[3][3] = 1.0f;
    mDeadCollision.AddBody(new CollisionBodyPrimitive(primitive, "coll_body_medium2", matrix, 0));

    int iLower = mDeadCollision.GetMaskId(0x9458FB2E);
    int iUpper = mDeadCollision.GetMaskId(0x2C31EC5B);
    unsigned int uDead = 0;
    if (iLower >= 0)
    {
        uDead = 1 << iLower;
    }
    if (iUpper >= 0)
    {
        uDead |= 1 << iUpper;
    }
    int iId = mDeadCollision.GetMaskId(0x0DB3CAEA);
    if (iId >= 0)
    {
        uDead |= 1 << iId;
    }
    iId = mDeadCollision.GetMaskId(0x6A085208);
    if (iId >= 0)
    {
        uDead |= 1 << iId;
    }
    mDeadCollision.SetCollisionMask(COLL_TERRAIN, CollisionMask(uDead));
    mDeadCollision.SetCollisionMask(COLL_STATIC, CollisionMask(uDead));
    mDeadCollision.SetCollisionMask(COLL_RIGID, CollisionMask(0));
    mDeadCollision.SetCollisionMask(COLL_SOFT, CollisionMask(0));
    mDeadCollision.SetCollisionMask(COLL_ORDNANCE, CollisionMask(0));

    mEnergyBarClass.SetPropertyEnergyCost(mEnergyCostJump);
    mEnergyBarClass.SetPropertyEnergyCost(mEnergyCostJumpSprint);
    mEnergyBarClass.SetPropertyEnergyCost(mEnergyCostRoll);
    mEnergyBarClass.SetPropertyEnergyCost(mJetEnergyDrain);
}

// FUNCTION: SWBF2 0x004f82e0
void EntitySoldierClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x2FA0BA9D:
    case 0x9F8A0040:
        sscanf(pszValue, "%f", &mAcceleration);
        break;
    case 0x86DF0364:
        sscanf(pszValue, "%f", &mMaxSpeed);
        break;
    case 0x3437A33D:
        sscanf(pszValue, "%f", &mMaxStrafeSpeed);
        break;
    case 0x5433F589:
        sscanf(pszValue, "%f", &mMaxTurnSpeed);
        break;
    case 0xB5534BE0:
        sscanf(pszValue, "%f", &mMaxPitchSpeed);
        break;
    case 0x3403B139:
        sscanf(pszValue, "%f %f", &mMinPitch, &mMaxPitch);
        mMinPitch *= 0.017453292f;
        mMaxPitch *= 0.017453292f;
        break;
    case 0xC415D23A:
    {
        float fHeight = 0.0f;
        sscanf(pszValue, "%f", &fHeight);
        mJumpVerticalSpeed = (float)sqrt(fHeight * 2.0f * 18.0f);
        break;
    }
    case 0x8EF41EF6:
        sscanf(pszValue, "%f", &mJumpForwardSpeedFactor);
        break;
    case 0x32505412:
        sscanf(pszValue, "%f", &mJumpStrafeSpeedFactor);
        break;
    case 0x07FF90DA:
        sscanf(pszValue, "%f", &mRollSpeedFactor);
        break;
    case 0x56389879:
        sscanf(pszValue, "%f", &mSprintAccelerateTime);
        break;

    case 0x50208DC0:
        m_bRecoverFromTumble = atoi(pszValue) != 0;
        break;
    case 0x3AF4F6FD:
        m_bUseDirectionalJumps = atoi(pszValue) != 0;
        break;
    case 0xAFC3ABEE:
        mJetShowHud = atoi(pszValue) != 0;
        break;
    case 0x91FABB5C:
        if (PblHash(pszValue) == 0x70CEAB2D)
        {
            mJetTypeHover = 1;
        }
        break;
    case 0x268DAC2D:
        mFleeLikeAHero = atoi(pszValue);
        break;
    case 0xC372B928:
    {
        int iAcklay = 0;
        sscanf(pszValue, "%d", &iAcklay);
        m_bIsAcklay = iAcklay != 0;
        break;
    }
    case 0x523E0533:
        m_bUseDirectionalDeaths = atoi(pszValue) != 0;
        break;
    case 0xE0359276:
    {
        int iNum;
        sscanf(pszValue, "%d", &iNum);
        m_numTentacles = iNum;
        break;
    }
    case 0x087A4DC9:
    {
        int iNum;
        sscanf(pszValue, "%d", &iNum);
        m_bonesPerTentacle = iNum;
        break;
    }
    case 0xF7AB24F5:
    {
        int iType;
        sscanf(pszValue, "%d", &iType);
        m_tentacleCollType = iType;
        break;
    }

    case 0x47BF39C1:
    {
        char acState[257];
        float fThrust = 1.0f;
        float fStrafe = 1.0f;
        float fTurn = 1.0f;
        int iCount = sscanf(pszValue, "%s %f %f %f", acState, &fThrust, &fStrafe, &fTurn);
        if (iCount < 2)
        {
            break;
        }
        int iState;
        switch (PblHash(acState))
        {
        case 0x75493323: iState = 0; break;
        case 0x4E4FA58B: iState = 1; break;
        case 0x6DD99A25: iState = 2; break;
        case 0x76F15155: iState = 3; break;
        case 0xA73F5C0D: iState = 4; break;
        case 0x16FF3D1E: iState = 5; break;
        case 0xD7E330F4: iState = 6; break;
        case 0x4C009BC2: iState = 7; break;
        default:
            return;
        }
        mThrustFactor[iState] = fThrust;
        if (iCount >= 3)
        {
            mStrafeFactor[iState] = fStrafe;
        }
        if (iCount >= 4)
        {
            mTurnFactor[iState] = fTurn;
        }
        break;
    }

    case 0x2CDB588A:
        sscanf(pszValue, "%f %f %f", &mCollisionThreshold[0], &mCollisionThreshold[1], &mCollisionThreshold[2]);
        break;
    case 0x1A87BACF:
        sscanf(pszValue, "%f %f %f", &mCollisionScale[0], &mCollisionScale[1], &mCollisionScale[2]);
        break;
    case 0x2EF03732:
        sscanf(pszValue, "%f", &mCollisionInflict);
        break;
    case 0x28ADDA49:
        mCombatInterrupts = atoi(pszValue) == 0;
        break;

    case 0xD0329E80:
        sWeaponSection = atoi(pszValue) - 1;
        break;
    case 0x6F332041:
    case 0xFBF47DBA:
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
    case 0xDC697015:
    case 0x2CE1A1D1:
        mWeaponClass[0] = WeaponClass::Find(PblHash(pszValue));
        mWeaponCount = mWeaponCount > 1 ? mWeaponCount : 1;
        break;
    case 0x4ADC9ACD:
    case 0x29E19D18:
        mWeaponClass[1] = WeaponClass::Find(PblHash(pszValue));
        mWeaponCount = mWeaponCount > 2 ? mWeaponCount : 2;
        break;
    case 0x2AE19EAB:
        mWeaponClass[2] = WeaponClass::Find(PblHash(pszValue));
        mWeaponCount = mWeaponCount > 3 ? mWeaponCount : 3;
        break;
    case 0x2FE1A68A:
        mWeaponClass[3] = WeaponClass::Find(PblHash(pszValue));
        mWeaponCount = mWeaponCount > 4 ? mWeaponCount : 4;
        break;

    case 0xDC19BD13:
    case 0x9E84BC86:
    case 0x9D84BAF3:
    case 0x9C84B960:
    case 0xA384C465:
    {
        int iWeapon = sWeaponSection;
        switch (uProperty)
        {
        case 0x9E84BC86: iWeapon = 0; break;
        case 0x9D84BAF3: iWeapon = 1; break;
        case 0x9C84B960: iWeapon = 2; break;
        case 0xA384C465: iWeapon = 3; break;
        }
        int iAmmo = 0;
        sscanf(pszValue, "%d", &iAmmo);
        WeaponAmmoDesc& ammo = mWeaponAmmo[iWeapon];
        ammo.bFlagShareAmmo = 0;
        ammo.bFlagAmmoInfinite = iAmmo <= 0;
        if (!ammo.bFlagAmmoInfinite)
        {
            ammo.uiAmmo = iAmmo;
        }
        break;
    }
    case 0x8585BB8A:
    case 0xEC868761:
    case 0xE98682A8:
    case 0xEA86843B:
    case 0xEF868C1A:
    {
        int iWeapon = sWeaponSection;
        switch (uProperty)
        {
        case 0xEC868761: iWeapon = 0; break;
        case 0xE98682A8: iWeapon = 1; break;
        case 0xEA86843B: iWeapon = 2; break;
        case 0xEF868C1A: iWeapon = 3; break;
        }
        int iShare = 0;
        sscanf(pszValue, "%d", &iShare);
        WeaponAmmoDesc& ammo = mWeaponAmmo[iWeapon];
        if (iShare > 0 && iShare <= iWeapon)
        {
            ammo.bFlagAmmoInfinite = 0;
            ammo.bFlagShareAmmo = 1;
            ammo.uiAmmo = iShare - 1;
        }
        else
        {
            ammo.bFlagShareAmmo = 0;
            ammo.bFlagAmmoInfinite = 1;
        }
        break;
    }
    case 0x9455A446:
    case 0xF9D1E755:
    case 0xF6D1E29C:
    case 0xF7D1E42F:
    case 0xF4D1DF76:
    {
        int iWeapon = sWeaponSection;
        switch (uProperty)
        {
        case 0xF9D1E755: iWeapon = 0; break;
        case 0xF6D1E29C: iWeapon = 1; break;
        case 0xF7D1E42F: iWeapon = 2; break;
        case 0xF4D1DF76: iWeapon = 3; break;
        }
        int iShare = 0;
        sscanf(pszValue, "%d", &iShare);
        WeaponAmmoDesc& ammo = mWeaponAmmo[iWeapon];
        if (iShare > 0 && iShare <= iWeapon)
        {
            ammo.bFlagShareHeat = 1;
            ammo.uiHeat = iShare - 1;
        }
        else
        {
            ammo.bFlagShareHeat = 0;
        }
        break;
    }
    case 0x16472328:
    {
        int iChannel;
        sscanf(pszValue, "%d", &iChannel);
        mWeaponChannel[sWeaponSection] = (unsigned int)iChannel <= 1 ? (char)iChannel : 0;
        break;
    }
    case 0x2AFC405B:
    {
        int iChannel;
        sscanf(pszValue, "%d", &iChannel);
        mWeaponChannel[0] = (unsigned int)iChannel <= 1 ? (char)iChannel : 0;
        break;
    }
    case 0x2BFC41EE:
    {
        int iChannel;
        sscanf(pszValue, "%d", &iChannel);
        mWeaponChannel[1] = (unsigned int)iChannel <= 1 ? (char)iChannel : 0;
        break;
    }
    case 0x2CFC4381:
    {
        int iChannel;
        sscanf(pszValue, "%d", &iChannel);
        mWeaponChannel[2] = (unsigned int)iChannel <= 1 ? (char)iChannel : 0;
        break;
    }
    case 0x2DFC4514:
    {
        int iChannel;
        sscanf(pszValue, "%d", &iChannel);
        mWeaponChannel[3] = (unsigned int)iChannel <= 1 ? (char)iChannel : 0;
        break;
    }

    case 0x09DB74AC:
        mOverrideTextureHash = PblHash(pszValue);
        break;
    case 0x227894BA:
        mOverrideTextureHash2 = PblHash(pszValue);
        break;

    case 0x942ED301:
        sscanf(pszValue, "%f", &mEnergyRestore);
        if (mEnergyRestore < 0.0f)
        {
            mEnergyRestore = 0.0f;
        }
        if (mEnergyRestore > mEnergyRestoreIdle)
        {
            mEnergyRestoreIdle = mEnergyRestore;
        }
        break;
    case 0x082EAF37:
        sscanf(pszValue, "%f", &mEnergyRestoreIdle);
        if (mEnergyRestore > mEnergyRestoreIdle)
        {
            mEnergyRestoreIdle = mEnergyRestore;
        }
        break;
    case 0x134361B9:
        sscanf(pszValue, "%f", &mEnergyDrainSprint);
        if (mEnergyDrainSprint < 0.0f)
        {
            mEnergyDrainSprint = 0.0f;
        }
        break;
    case 0x0B4A7FD4:
        sscanf(pszValue, "%f", &mEnergyCostJump);
        if (mEnergyCostJump < 0.0f)
        {
            mEnergyCostJump = 0.0f;
        }
        break;
    case 0xA44B8F28:
        sscanf(pszValue, "%f", &mEnergyCostJumpSprint);
        if (mEnergyCostJumpSprint < 0.0f)
        {
            mEnergyCostJumpSprint = 0.0f;
        }
        break;
    case 0xCA24CD33:
        sscanf(pszValue, "%f", &mEnergyCostRoll);
        if (mEnergyCostRoll < 0.0f)
        {
            mEnergyCostRoll = 0.0f;
        }
        break;

    case 0xC4B2CB20:
        sscanf(pszValue, "%f", &mJetJump);
        break;
    case 0xF0B75E90:
        sscanf(pszValue, "%f", &mJetPush);
        break;
    case 0x8AAC2908:
        sscanf(pszValue, "%f", &mJetAcceleration);
        break;
    case 0x0825C1BF:
        sscanf(pszValue, "%f", &mJetFuelRechargeRate);
        break;
    case 0x36D9DE4D:
        sscanf(pszValue, "%f", &mJetFuelCost);
        break;
    case 0x1EBB5841:
        sscanf(pszValue, "%f", &mJetFuelInitialCost);
        break;
    case 0xF8A37C8A:
        sscanf(pszValue, "%f", &mJetFuelMinBorder);
        break;
    case 0x0C2425EA:
        sscanf(pszValue, "%f", &mJetEnergyDrain);
        break;
    case 0x69C4D0DD:
        mJetPackEffect = FLEffect::Get(PblHash(pszValue));
        break;
    case 0xBC5EDAD3:
        mJetPackIdleEffect = FLEffect::Get(PblHash(pszValue));
        break;

    case 0x68C9C2D9:
        mHealthGainEffect = PblHash(pszValue);
        break;
    case 0x8D8743F9:
        mAmmoGainEffect = PblHash(pszValue);
        break;
    case 0x80A20AE3:
        mEnergyGainEffect = PblHash(pszValue);
        break;
    case 0x32839537:
        mBuffHealthEffect = PblHash(pszValue);
        break;
    case 0x5B84DF23:
        mBuffOffenseEffect = PblHash(pszValue);
        break;
    case 0x8B003201:
        mBuffDefenseEffect = PblHash(pszValue);
        break;
    case 0x1F3FEA32:
        mDebuffEffect = PblHash(pszValue);
        break;
    case 0x9B485275:
        mInvincibleEffect = PblHash(pszValue);
        break;
    case 0xB8F82137:
        mFootSlideEffect = PblHash(pszValue);
        break;
    case 0xAD9245D2:
        mChokeEffect = PblHash(pszValue);
        break;
    case 0xB9282909:
        sscanf(pszValue, "%f", &m_fBlurEffect);
        break;
    case 0xDEA64CFA:
        mWaterSplashEffect = PblHash(pszValue);
        break;
    case 0x057266EE:
        mFootWaterSplashEffect = PblHash(pszValue);
        break;
    case 0xF44FEE52:
        mWakeWaterSplashEffect = PblHash(pszValue);
        break;

    case 0x54A7AEDF:
        mSoldierType = FindSoldierType(pszValue);
        break;
    case 0x62A8F10F:
        mImmuneToMines = atoi(pszValue) != 0;
        break;

    case 0xBACA6BF1:
        switch (PblHash(pszValue))
        {
        case 0x75493323:
            sCurrentCameraSplit = 0; sCurrentCameraStance = 0; sCurrentCameraZoomed = 0;
            break;
        case 0x94D7CDEC:
            sCurrentCameraSplit = 0; sCurrentCameraStance = 0; sCurrentCameraZoomed = 1;
            break;
        case 0x4E4FA58B:
            sCurrentCameraSplit = 0; sCurrentCameraStance = 1; sCurrentCameraZoomed = 0;
            break;
        case 0xF648B5F4:
            sCurrentCameraSplit = 0; sCurrentCameraStance = 1; sCurrentCameraZoomed = 1;
            break;
        case 0x6DD99A25:
            sCurrentCameraSplit = 0; sCurrentCameraStance = 2; sCurrentCameraZoomed = 0;
            break;
        case 0x0016DA92:
            sCurrentCameraSplit = 0; sCurrentCameraStance = 2; sCurrentCameraZoomed = 1;
            break;
        case 0x76F15155:
            sCurrentCameraSplit = 0; sCurrentCameraStance = 3; sCurrentCameraZoomed = 0;
            break;
        case 0xD8AECCCD:
            sCurrentCameraSplit = 1; sCurrentCameraStance = 0; sCurrentCameraZoomed = 0;
            break;
        case 0x1643517A:
            sCurrentCameraSplit = 1; sCurrentCameraStance = 0; sCurrentCameraZoomed = 1;
            break;
        case 0xCA780CF9:
            sCurrentCameraSplit = 1; sCurrentCameraStance = 1; sCurrentCameraZoomed = 0;
            break;
        case 0x37E029EE:
            sCurrentCameraSplit = 1; sCurrentCameraStance = 1; sCurrentCameraZoomed = 1;
            break;
        case 0xF6D39CCB:
            sCurrentCameraSplit = 1; sCurrentCameraStance = 2; sCurrentCameraZoomed = 0;
            break;
        case 0xFE5CFC34:
            sCurrentCameraSplit = 1; sCurrentCameraStance = 2; sCurrentCameraZoomed = 1;
            break;
        case 0x4ABADB57:
            sCurrentCameraSplit = 1; sCurrentCameraStance = 3; sCurrentCameraZoomed = 0;
            break;
        default:
            sCurrentCameraSplit = 0; sCurrentCameraStance = 0; sCurrentCameraZoomed = 0;
            break;
        }
        break;
    case 0xC3E3DCFC:
        if (sCurrentCameraStance > 0 && !sCurrentCameraZoomed &&
            !(sCurrentCameraSplit && CameraManager::sInstance->m_iNumCam <= 1))
        {
            sscanf(pszValue, "%f", &mCameraSettingBlendTime[sCurrentCameraStance - 1]);
        }
        break;

    case 0x47C86B4A:
    {
        GameObjectClass::SetProperty(uProperty, pszValue);
        mLodData = SoldierLodData(mModel ? mModel->GetRedModel(0) : 0, 0);
        SoldierAnimatorClass* pAnimatorClass = SoldierAnimatorClass::Instance();
        mSpecialSkeleton = pAnimatorClass->GetDefaultSkeleton();
        mSpecialSkeletonLowRes = SoldierAnimatorLowResClass::GetDefaultSkeleton();
        reinterpret_cast<PblBitVector<32>&>(mUpperBodyAnimMask) = pAnimatorClass->GetDefaultUpperBodyAnimMask();
        reinterpret_cast<PblBitVector<32>&>(mLowerBodyAnimMask) = pAnimatorClass->GetDefaultLowerBodyAnimMask();
        break;
    }
    case 0xA490EBA1:
        mModelLowRes = const_cast<GameModel*>(GameModel::Find(PblHash(pszValue)));
        mLodData = SoldierLodData(mModel ? mModel->GetRedModel(0) : 0, mModelLowRes->GetRedModel(0));
        break;
    case 0x6363D774:
        mFirstPersonModelHandle = FirstPerson::AddGeometry(pszValue);
        break;

    case 0x7012F6CD:
    case 0x98557A5E:
    {
        char acBuffer[257];
        strlcpy(acBuffer, pszValue, sizeof(acBuffer));
        char* pcParent = 0;
        char* pcName = strtok(acBuffer, " ");
        while (pcName && !*pcName)
        {
            pcName = strtok(0, " ");
        }
        if (pcName)
        {
            do
                pcParent = strtok(0, " ");
            while (pcParent && !*pcParent);
        }
        RedAnimation* pAnimation = RedAnimation::Find(PblHash(pcName));
        if (pAnimation && pAnimation->GetZephyrSkeletonShared())
        {
            mSpecialSkeleton = pAnimation->GetZephyrSkeletonShared();
            SoldierAnimatorClass::SetupJointTransformAnimationFlag(mSpecialSkeleton);
            SoldierAnimatorClass::SetupBodyMasks(mSpecialSkeleton,
                                                 reinterpret_cast<PblBitVector<32>&>(mUpperBodyAnimMask),
                                                 reinterpret_cast<PblBitVector<32>&>(mLowerBodyAnimMask));
            mAnimationBank = SoldierAnimationBank::AddBank(pcName, pcParent);
        }
        break;
    }
    case 0x55F297D2:
    case 0x1ECC7E0D:
        if (!gMidgetTime)
        {
            char acBuffer[257];
            strlcpy(acBuffer, pszValue, sizeof(acBuffer));
            char* pcParent = 0;
            char* pcName = strtok(acBuffer, " ");
            while (pcName && !*pcName)
            {
                pcName = strtok(0, " ");
            }
            if (pcName)
            {
                do
                    pcParent = strtok(0, " ");
                while (pcParent && !*pcParent);
            }
            RedAnimation* pAnimation = RedAnimation::Find(PblHash(pcName));
            if (pAnimation && pAnimation->GetZephyrSkeletonShared())
            {
                mSpecialSkeletonLowRes = pAnimation->GetZephyrSkeletonShared();
                SoldierAnimationBank::AddLowResBank(mAnimationBank, pcName, pcParent);
                SoldierAnimatorLowResClass::AddBank(pcName, pcParent);
            }
        }
        break;
    case 0xE8888B42:
        mAddon = new AnimatedAddon(*mSpecialSkeleton);
        break;
    case 0x94272331:
    case 0xA423E008:
    case 0xDDDCDE45:
        mAddon->SetProperty(uProperty, pszValue);
        break;

    case 0x33AB42BA:
    {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        m_vCollisionHeadOffset.x = x;
        m_vCollisionHeadOffset.y = y;
        m_vCollisionHeadOffset.z = z;
        break;
    }
    case 0x463A5C7D:
    {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        m_vCollisionTorsoOffset.x = x;
        m_vCollisionTorsoOffset.y = y;
        m_vCollisionTorsoOffset.z = z;
        break;
    }
    case 0x4F90C3A1:
        sscanf(pszValue, "%f", &m_fCollisionHeadScale);
        break;
    case 0xB199E320:
        sscanf(pszValue, "%f", &m_fCollisionTorsoScale);
        break;
    case 0xD641F0D6:
        sscanf(pszValue, "%f", &mFootstepStride);
        break;
    case 0x44C8744D:
        sscanf(pszValue, "%f", &mRunSpeed);
        break;

    case 0x399EE477:
        mAcquiredTargetSound.SetID(pszValue, false, false);
        break;
    case 0xC007B675:
        mHidingSound.SetID(pszValue, false, false);
        break;
    case 0xEB6089A1:
        mApproachingTargetSound.SetID(pszValue, false, false);
        break;
    case 0x94353BF8:
        mFleeSound.SetID(pszValue, false, false);
        break;
    case 0x130971AE:
        mPreparingForDamageSound.SetID(pszValue, false, false);
        break;
    case 0xD3C54780:
        mHeardEnemySound.SetID(pszValue, false, false);
        break;
    case 0x4EE58508:
        mChargeSound.SetID(pszValue, false, false);
        break;
    case 0x65A920BA:
        mAmbientSound.SetID(pszValue, false, false);
        break;
    case 0x20CD1D00:
        sscanf(pszValue, "%f", &mShockFadeOutTime);
        break;
    case 0x716ADADD:
        sscanf(pszValue, "%f", &mShockFadeInTime);
        break;
    case 0xECF4A152:
        sscanf(pszValue, "%f", &mShockFadeOutGain);
        break;
    case 0xF66784C8:
        mShockSound.SetID(pszValue, false, false);
        break;
    case 0x5625FC0B:
        mClothingRustleSound.SetID(pszValue, false, false);
        break;
    case 0xF8D3E5D2:
        mLowHealthSound.SetID(pszValue, false, false);
        break;
    case 0xC865523E:
        sscanf(pszValue, "%f", &mLowHealthThreshold);
        break;
    case 0xDF0FB8AE:
        mChokeSound.SetID(pszValue, false, false);
        break;

    case 0x5EA04066:
        ++sCurDropItem;
        break;
    case 0x406D2B75:
    {
        mDropItemClass[sCurDropItem] = EntityClass::Find(PblHash(pszValue));
        int iCount = sCurDropItem + 1;
        if (mDropItemCount > iCount)
        {
            iCount = mDropItemCount;
        }
        mDropItemCount = iCount;
        break;
    }
    case 0x26996DAE:
    {
        sscanf(pszValue, "%f", &mDropItemProbability[sCurDropItem]);
        int iCount = sCurDropItem + 1;
        if (mDropItemCount > iCount)
        {
            iCount = mDropItemCount;
        }
        mDropItemCount = iCount;
        break;
    }

    case 0x43B50014:
        sscanf(pszValue, "%f", &mChunkFrequency);
        break;
    case 0x4CACC3D0:
        sscanf(pszValue, "%f", &mSkeletonRootScale);
        if (mSpecialSkeleton && mSpecialSkeleton != SoldierAnimatorClass::Instance()->GetDefaultSkeleton() &&
            mSkeletonRootScale != 1.0f)
        {
            int iJoint = (int)mSpecialSkeleton->m_kHashCRC2Joint.Find(0x16E27226) - 1;
            mSpecialSkeleton->SetJointScale(iJoint, mSkeletonRootScale);
        }
        break;
    case 0xC64A53AB:
        sscanf(pszValue, "%f", &mCollisionRootScale);
        break;
    case 0x26239CD4:
        sscanf(pszValue, "%f", &mSkeletonRootScaleLowRes);
        if (mSpecialSkeletonLowRes && mSpecialSkeletonLowRes != SoldierAnimatorLowResClass::GetDefaultSkeleton() &&
            mSkeletonRootScaleLowRes != 1.0f)
        {
            int iJoint = (int)mSpecialSkeletonLowRes->m_kHashCRC2Joint.Find(0xE6E47876) - 1;
            mSpecialSkeletonLowRes->SetJointScale(iJoint, mSkeletonRootScaleLowRes);
        }
        break;
    case 0x91415FA0:
        sscanf(pszValue, "%f", &mMinFootHeight);
        break;
    case 0x02BB1FE0:
    case 0x9B30E763:
        mExplosion = ExplosionClass::Find(PblHash(pszValue));
        break;

    case 0x4AC2B220:
        mSndHeroSelectable = PblHash(pszValue);
        break;
    case 0xE4DEE4EE:
        mSndHeroSpawned = PblHash(pszValue);
        break;
    case 0xD3A93472:
        mSndHeroDefeated = PblHash(pszValue);
        break;
    case 0xEF9E29F7:
        mSndForHeroKiller = PblHash(pszValue);
        break;
    case 0xF5B46BE4:
        mSoldierMusic.SetID(pszValue);
        break;
    case 0x3BEC98DA:
        mAISoldierMusic.SetID(pszValue);
        break;
    case 0x99257468:
        mSoldierAnnouncement.SetID(pszValue, false, false);
        break;

    case 0x71735CF7:
    {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mTargetPointOffset[0].x = x;
        mTargetPointOffset[0].y = y;
        mTargetPointOffset[0].z = z;
        break;
    }
    case 0x0355B70F:
    {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mTargetPointOffset[1].x = x;
        mTargetPointOffset[1].y = y;
        mTargetPointOffset[1].z = z;
        break;
    }

    default:
        if (!mEnergyBarClass.SetProperty(uProperty, pszValue))
        {
            const RedModel* pModel = mModel ? mModel->GetRedModel(0) : 0;
            if (!VehicleEngineClass::SetProperty(uProperty, pszValue) &&
                !PlayableEntitySoundsClass::SetProperty(uProperty, pszValue) &&
                !mAimerClass->SetProperty(uProperty, pszValue, pModel, 0) &&
                ((sCurrentCameraSplit && CameraManager::sInstance->m_iNumCam <= 1) ||
                 !(&mCameraSettings[0][0])[sCurrentCameraStance * 2 + sCurrentCameraZoomed].SetProperty(uProperty, pszValue)) &&
                !ControllableClass::SetProperty(uProperty, pszValue, pModel))
            {
                GameObjectClass::SetProperty(uProperty, pszValue);
            }
        }
        break;
    }
}

// FUNCTION: SWBF2 0x004fa880
int EntitySoldierClass::FindSoldierType(const char* pszName)
{
    switch (PblHash(pszName))
    {
    case 0xC322AD05:
        return 0;
    case 0xB0143BE7:
    case 0x66557E0C:
        return 1;
    case 0x3B3E18E8:
    case 0x29520BB4:
    case 0xD940D530:
        return 2;
    case 0x10066AE1:
        return 3;
    case 0xDF625F90:
        return 4;
    }
    return 5;
}

// FUNCTION: SWBF2 0x004fa900
EntityClass* EntitySoldierClass::Derive(unsigned int uHash)
{
    return new EntitySoldierClass(this, uHash);
}

// FUNCTION: SWBF2 0x004fa970
Entity* EntitySoldierClass::Build(const EntityDesc& desc)
{
    if (EntitySoldier::sMemoryPool.IsFull())
    {
        return 0;
    }
    EntitySoldier* pSoldier = new EntitySoldier(this, desc);
    if (pSoldier)
    {
        if (m_bIsAcklay)
        {
            pSoldier->AddRigidToCollisionManager();
        }
        else
        {
            pSoldier->AddSoftToCollisionManager();
        }
        pSoldier->ActivatePhysics();
        pSoldier->BuildEffects();
    }
    return pSoldier;
}

namespace FirstPerson
{
void ProneToStand(unsigned int uPlayer);
void CrouchToStand(unsigned int uPlayer);
void Land(unsigned int uPlayer);
}

// FUNCTION: SWBF2 0x004ddf00
const float UnitCircleNormalize(float fAngle)
{
    while (fAngle > PblMath::fPi)
    {
        fAngle -= PblMath::fTwoPi;
    }
    while (fAngle < -PblMath::fPi)
    {
        fAngle += PblMath::fTwoPi;
    }
    return fAngle;
}

// FUNCTION: SWBF2 0x004e0090
void EntitySoldier::TriggerBuffHealthEffect()
{
    if (mBuffHealthTimer > 0.0f && GetInvisibilityFactor() == 0.0f &&
        (IsThreadActive() || GetObjectData(0) == COLL_ATTACHED))
    {
        if (mClass->mBuffHealthEffect && mBuffHealthEffectHandle == 0)
        {
            if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mBuffHealthEffect))
            {
                if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                {
                    pEffect->AttachEffectToObject(this, true);
                    mBuffHealthEffectHandle = pEffect;
                }
            }
        }
    }
    else if (mBuffHealthEffectHandle)
    {
        mBuffHealthEffectHandle->StopAndFinish();
        mBuffHealthEffectHandle = NULL;
    }
}

// FUNCTION: SWBF2 0x004e01d0
void EntitySoldier::TriggerBuffOffenseEffect()
{
    if (mBuffOffenseTimer > 0.0f && GetInvisibilityFactor() == 0.0f &&
        (IsThreadActive() || GetObjectData(0) == COLL_ATTACHED))
    {
        if (mClass->mBuffOffenseEffect && mBuffOffenseEffectHandle == 0)
        {
            if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mBuffOffenseEffect))
            {
                if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                {
                    pEffect->AttachEffectToObject(this, true);
                    mBuffOffenseEffectHandle = pEffect;
                }
            }
        }
    }
    else if (mBuffOffenseEffectHandle)
    {
        mBuffOffenseEffectHandle->StopAndFinish();
        mBuffOffenseEffectHandle = NULL;
    }
}

// FUNCTION: SWBF2 0x004e0310
void EntitySoldier::TriggerBuffDefenseEffect()
{
    if (mBuffDefenseTimer > 0.0f && GetInvisibilityFactor() == 0.0f &&
        (IsThreadActive() || GetObjectData(0) == COLL_ATTACHED))
    {
        if (mClass->mBuffDefenseEffect && mBuffDefenseEffectHandle == 0)
        {
            if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mBuffDefenseEffect))
            {
                if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                {
                    pEffect->AttachEffectToObject(this, true);
                    mBuffDefenseEffectHandle = pEffect;
                }
            }
        }
    }
    else if (mBuffDefenseEffectHandle)
    {
        mBuffDefenseEffectHandle->StopAndFinish();
        mBuffDefenseEffectHandle = NULL;
    }
}

// FUNCTION: SWBF2 0x004e0450
void EntitySoldier::TriggerDebuffEffect()
{
    if (mDebuffDamageTimer > 0.0f && GetInvisibilityFactor() == 0.0f &&
        (IsThreadActive() || GetObjectData(0) == COLL_ATTACHED))
    {
        if (mClass->mDebuffEffect && mDebuffEffectHandle == 0)
        {
            if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mDebuffEffect))
            {
                if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
                {
                    pEffect->AttachEffectToObject(this, true);
                    mDebuffEffectHandle = pEffect;
                }
            }
        }
    }
    else if (mDebuffEffectHandle)
    {
        mDebuffEffectHandle->StopAndFinish();
        mDebuffEffectHandle = NULL;
    }
}

// FUNCTION: SWBF2 0x004e0670
void EntitySoldier::UpdateSmolderTimers(float fDeltaTime)
{
    if (!IsAlive())
    {
        return;
    }

    mSmolderTimer -= fDeltaTime;
    if (mSmolderTimer < 0.0f)
    {
        mSmolderTimer = 0.0f;
    }

    if (mSmolderTimer > 0.0f)
    {
        if (mSmolderDamageRate != 0.0f)
        {
            Damageable::ApplyDamage(DamageDesc(mSmolderDamageOwner, mSmolderDamageRate * fDeltaTime),
                                    0xffffffff);
        }
        for (int i = 0; i < 5; ++i)
        {
            TriggerSmolderEffect(i, mSmolderEffectBone[i], mSmolderEffect[i]);
        }
    }
    else
    {
        for (int i = 0; i < 5; ++i)
        {
            if (mSmolderEffectHandle[i])
            {
                mSmolderEffectHandle[i]->StopAndFinish();
                mSmolderEffectHandle[i] = NULL;
            }
        }
    }
}

// FUNCTION: SWBF2 0x004e0c50
void EntitySoldier::TurnOffInvincibleEffect()
{
    if (mInvincibleEffectHandle)
    {
        mInvincibleEffectHandle->DeactivateEffect();
        mInvincibleEffectHandle = NULL;
    }
}

// FUNCTION: SWBF2 0x004e1180
float EntitySoldier::GetClassMaxTurnSpeed() const
{
    float fSpeed = mClass->mMaxTurnSpeed;
    if (mPlayerId < 0)
    {
        fSpeed *= 4.0f;
    }
    return fSpeed;
}

// GLOBAL: SWBF2 0x01e51e10
static bool sNoAiMovements;

extern int gDebugCount;
extern int gSpawnInvicibility;

namespace ReserveManager
{
int GetOpenReservableSlots(const GameObject* pObj, int iTeam, ReserveType eType);
}

namespace AI
{
enum VOType
{
    VOT_HeadshotKill = 9,
    VOT_SC_Follow = 22,
    VOT_SC_StopFollow = 24,
    VOT_SC_VehicleWaitUp = 26,
    VOT_SC_VehicleWaitUpResponse = 27
};

namespace VOHelper
{
int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const GameObject* pOther, float fDelay);
}
}

// FUNCTION: SWBF2 0x004e8920
bool EntitySoldier::Update(float fDeltaTime)
{
    static int updateCount;
    static const int updateCheck = -1;
    static float s_fSplashTimeout;
    static float s_fWakeTimer;

    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }

    Controllable::Update(fDeltaTime);
    if (mState == SoldierState::PRONE)
    {
        Crouch();
    }

    if (!mAcklayData)
    {
        if (mStandingOnObj && (!(mFlags & flags_fallTimer) || mStateTimer < 0.5f))
        {

            PblMatrix mObject = mStandingOnObj->mCollisionModel->GetWorldMatrix(
                mStandingOnBody, mStandingOnObj->GetMatrix(), mStandingOnObj->GetPose());
            PblMatrix mLocal;
            D3DXMatrixMultiply(&mLocal, &mMatrix, &mLastVehicleMatrix);
            PblMatrix mNew;
            D3DXMatrixMultiply(&mNew, &mLocal, &mObject);
            const PblVector3& vUp = mMatrix.GetYAxis();
            const PblVector3& vFront = mNew.GetZAxis();
            if (fabs(vUp.y * vFront.y + vUp.x * vFront.x + vUp.z * vFront.z) < 0.99f)
            {
                SetMatrix(NormalMatrix(vFront, vUp, mNew.GetTranslation()));
                if (mSoldierAnimator)
                {
                    mSoldierAnimator->ModifyLegMatrixByVehicle(mLastVehicleMatrix, mObject);
                }
            }

            float fLastSpeed = sqrtf(mStandingOnObjLastVelocity.x * mStandingOnObjLastVelocity.x +
                                     mStandingOnObjLastVelocity.y * mStandingOnObjLastVelocity.y +
                                     mStandingOnObjLastVelocity.z * mStandingOnObjLastVelocity.z);
            const PblVector3& vObjectVelocity = mStandingOnObj->GetVelocity();
            float fObjectSpeed = sqrtf(vObjectVelocity.x * vObjectVelocity.x + vObjectVelocity.y * vObjectVelocity.y +
                                       vObjectVelocity.z * vObjectVelocity.z);
            float fSpeedLost;
            if (fLastSpeed > fObjectSpeed && (fSpeedLost = fLastSpeed - fObjectSpeed) > 8.2f)
            {
                PblVector3 vDir;
                D3DXVec3Normalize(&vDir, &mStandingOnObjLastVelocity);
                ApplyPush(PblVector3(vDir.x + 0.0f, vDir.y + 0.3f, vDir.z + 0.0f), fSpeedLost, false);
            }
            else if (mStandingOnObj->GetMatrix().m[1][1] < 0.55f)
            {
                SetState(SoldierState::TUMBLE);
            }

            if (mStandingOnObj)
            {
                mLastVehicleMatrix = mObject.GetInverseFast();
                mStandingOnObjLastVelocity = mStandingOnObj->GetVelocity();
            }
        }
        else
        {
            mStandingOnObj = 0;
        }
    }
    else if (mFlags & flags_isGrounded)
    {

        CollisionObject* pHit = 0;
        const GameObject* pIgnore = GetGameObject();
        PblVector3 vDown(-gVectorUnitY.x, -gVectorUnitY.y, -gVectorUnitY.z);
        PblVector3 vOffset(mMatrix.m[1][0] * 0.4f, mMatrix.m[1][1] * 0.4f, mMatrix.m[1][2] * 0.4f);
        const PblVector3& vPos = GetPosition();
        PblVector3 vStart(vPos.x + vOffset.x, vPos.y + vOffset.y, vPos.z + vOffset.z);
        PblVector3 vNormal;
        CollisionManager::RayHit(vStart, vDown, 2.0f, pHit, &vNormal, &pIgnore, 1, 0x98, false);

        PblVector3 vUp;
        bool bSlope = false;
        if (pHit)
        {
            PblVector3 vHitNormal;
            D3DXVec3Normalize(&vHitNormal, &vNormal);
            vNormal = vHitNormal;
            if (vHitNormal.y > 0.5f)
            {
                vUp = PblVector3(vHitNormal.x * 0.05f + mMatrix.m[1][0] * 0.95f,
                                 vHitNormal.y * 0.05f + mMatrix.m[1][1] * 0.95f,
                                 vHitNormal.z * 0.05f + mMatrix.m[1][2] * 0.95f);
                bSlope = true;
            }
        }
        if (!bSlope)
        {
            vUp = PblVector3(gVectorUnitY.x * 0.25f + mMatrix.m[1][0] * 0.75f,
                             gVectorUnitY.y * 0.25f + mMatrix.m[1][1] * 0.75f,
                             gVectorUnitY.z * 0.25f + mMatrix.m[1][2] * 0.75f);
        }

        PblVector3 vAxis;
        D3DXVec3Normalize(&vAxis, &vUp);
        mMatrix.m[1][0] = vAxis.x;
        mMatrix.m[1][1] = vAxis.y;
        mMatrix.m[1][2] = vAxis.z;
        mMatrix.m[1][3] = 0.0f;

        PblVector3 vCross(mMatrix.m[2][2] * mMatrix.m[1][1] - mMatrix.m[2][1] * mMatrix.m[1][2],
                          mMatrix.m[2][0] * mMatrix.m[1][2] - mMatrix.m[1][0] * mMatrix.m[2][2],
                          mMatrix.m[1][0] * mMatrix.m[2][1] - mMatrix.m[2][0] * mMatrix.m[1][1]);
        D3DXVec3Normalize(&vAxis, &vCross);
        mMatrix.m[0][0] = vAxis.x;
        mMatrix.m[0][1] = vAxis.y;
        mMatrix.m[0][2] = vAxis.z;
        mMatrix.m[0][3] = 0.0f;

        vCross = PblVector3(mMatrix.m[1][2] * mMatrix.m[0][1] - mMatrix.m[1][1] * mMatrix.m[0][2],
                            mMatrix.m[1][0] * mMatrix.m[0][2] - mMatrix.m[0][0] * mMatrix.m[1][2],
                            mMatrix.m[0][0] * mMatrix.m[1][1] - mMatrix.m[1][0] * mMatrix.m[0][1]);
        D3DXVec3Normalize(&vAxis, &vCross);
        mMatrix.m[2][0] = vAxis.x;
        mMatrix.m[2][1] = vAxis.y;
        mMatrix.m[2][2] = vAxis.z;
        mMatrix.m[2][3] = 0.0f;
    }

    if (mTentacles)
    {
        mTentacles->UpdateTimer(fDeltaTime);
    }
    UpdateBuffTimers(fDeltaTime);
    UpdateSmolderTimers(fDeltaTime);

    mCollisionState = COLLSTATE_NONE;
    mSafeMatrix = mMatrix;
    if (!(mFlags & flags_isGrounded))
    {
        mGroundNormal.x = 0.0f;
        mGroundNormal.y = 1.0f;
        mGroundNormal.z = 0.0f;
    }

    if (mState == SoldierState::SPRINT)
    {
        for (int i = 0; i < 2; ++i)
        {
            int iIndex = mWeaponIndex[i];
            if ((iIndex == -1 || (!mWeapon[iIndex]->IsMelee() && !mWeapon[iIndex]->IsMeleeThrow())) &&
                mControlFire[i].IsDown())
            {
                EndSprint();
            }
        }
    }

    int iJoystick = GetJoystickIndex();
    Weapon* pCurWeapon = GetCurWpn();
    bool bTargetHandled = false;
    if (iJoystick >= 0 &&
        (!NetGame::IsEnabled() || (NetGame::IsClient() && netIsLocalTurn) ||
         (NetGame::IsHost() && NetGame::IsOneWorld() && netIsLocalTurn)))
    {
        LockOnManager* pManager = LockOnManager::GetByPlayerIndex(iJoystick);
        if (!pCurWeapon)
        {
            TargetTrackingForceClear();
            pManager->RemoveTargetLock();
            mAllowedLockTypes = 0;
            bTargetHandled = true;
        }
        else
        {
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
            pManager->SetAutoLoseLocked(2.0f, true, 0.1f, fLoseDist);
            pManager->SetTarget(0.5f, 0.8f, fLockDist, true);
            pManager->SetAuto(true, false);
            pManager->SetNumAttackerTargets(0);
            pManager->SetNumNearestTargets(pManager->GetDisplayIconsEverything() ? 64 : 2);

            if (mControlLockTarget.IsPressed())
            {
                pManager->ForceNewTargetLock(0);
                bTargetHandled = true;
            }
            else if (mControlLockTarget.Held())
            {
                pManager->RemoveTargetLock();
                bTargetHandled = true;
            }
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

    if (mPlayerId < 0)
    {
        if (mSkip)
        {
            mSkipTime = fDeltaTime;
            return true;
        }
        fDeltaTime = mSkipTime + fDeltaTime;
        mSkipTime = 0.0f;
    }

    if (m_fWeaponAnimTimer >= 0.0f)
    {
        m_fWeaponAnimTimer -= fDeltaTime;
        if (m_fWeaponAnimTimer <= 0.0f)
        {
            m_uiWeaponAnimIndex = 0;
            m_fWeaponAnimTimer = 0.0f;
        }
    }

    if (mFlyingInFirstPerson && !IsNotFirstPersonState(mState) && mInputLockTime <= 0.0f)
    {
        if (iJoystick >= 0)
        {
            FirstPerson::Activate(iJoystick, true);
        }
        mFlyingInFirstPerson = 0;
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        if (mVelocity.y < -60.0f || mMatrix.m[3][1] < -1000.0f || fabsf(mMatrix.m[3][0]) > 2500.0f ||
            fabsf(mMatrix.m[3][2]) > 2500.0f || mMatrix.m[3][1] > 1000.0f)
        {
            Kill();
            return false;
        }
    }

    if (++updateCount == updateCheck)
    {
        ++gDebugCount;
    }

    int iPrevStance = mState == SoldierState::SPRINT ? 3 : mStanceIndex;

    if (mSoldierAnimator && mSoldierAnimator->GetOwner() != this)
    {
        mSoldierAnimator = 0;
    }

    mSoundState &= 0xff1f;
    if (mInputLockTime > 0.0f)
    {
        if (m_uiInputLockMask & INPUTLOCK_ACTION)
        {
            mControlCrouch.Clear();
            mControlJump.Clear();
            mControlSprint.Clear();
            mControlSpecialMove.Clear();
            mControlThrustFwd.Clear();
            mControlThrustBack.Clear();
            mControlStrafeLeft.Clear();
            mControlStrafeRight.Clear();
        }
        if (m_uiInputLockMask & INPUTLOCK_WEAPON)
        {
            mControlReload.Clear();
            for (int i = 0; i < 2; ++i)
            {
                SetSwitch(0.0f, i);
                mControlFire[i].Clear();
            }
        }
    }

    if (!mIsAlive)
    {
        int iPlayer = mInitialPlayerId;
        if (iPlayer >= 0 && !Character::GetPtr(iPlayer)->mUnit)
        {
            Trackable::TrackDeath(iPlayer, mClass->mHeroFlag ? 0 : mDamageOwner.mCharacter);
            if (mTracker && mTracker->IsFirstPersonView() && iJoystick >= 0)
            {
                FirstPerson::Activate(iJoystick, false);
            }
        }
    }

    if (!NetGame::IsEnabled() ||
        (!NetGame::IsDedicated() &&
         (NetGame::IsClient() ? netIsLocalTurn : NetGame::IsOneWorld() && !netIsLateTurn)))
    {
        UpdateFoleyFX(fDeltaTime);

        if (mAmbientSound.GetPlaying())
        {
            mAmbientSound.Update(&mClass->mAmbientSound, &mMatrix.GetTranslation(), &mVelocity);
        }
        else
        {
            mAmbientSound.Play(&mClass->mAmbientSound, &mMatrix.GetTranslation(), &mVelocity, true);
        }

        if (mIsAlive && mClass->mLowHealthThreshold * mMaxHealth >= mCurHealth)
        {
            if (!mLowHealthSound.GetPlaying())
            {
                mLowHealthSound.Play(&mClass->mLowHealthSound, &mMatrix.GetTranslation(), &mVelocity, true);
            }
        }
        else if (mLowHealthSound.GetPlaying())
        {
            mLowHealthSound.Stop(true);
        }

        if (mState == SoldierState::CHOKE)
        {
            if (!mChokeSound.GetPlaying())
            {
                mChokeSound.Play(&mClass->mChokeSound, &mMatrix.GetTranslation(), &mVelocity, true);
            }
        }
        else if (mChokeSound.GetPlaying())
        {
            mChokeSound.Stop(true);
        }

        if (mSoldierAnimator && mSoldierAnimator->GetOwner() == this && mFoliageCollisionTime > 0.2f)
        {
            FoliageCollision();
            mFoliageCollisionTime = 0.0f;
        }
        mFoliageCollisionTime += fDeltaTime;
    }

    if (mState != SoldierState::DEAD && mState != SoldierState::HERO_FLEE && mState != SoldierState::FLY &&
        mState != SoldierState::TUMBLE && mState != SoldierState::BOUNCE)
    {
        if (!mIsAlive)
        {

            EntitySoldierClass* pClass = mClass;
            float fChance = PblRandom::GetSim().GetFloat();
            if ((pClass->mChunkFrequency >= fChance || (mFlags & flags_forceChunkDeath)) && pClass &&
                pClass->mChunkDescriptors[0])
            {
                if (pClass->mExplosion && (!NetGame::IsEnabled() || netIsLocalTurn))
                {
                    DamageOwner owner(mCharacter, this, GetTeam(), pClass, 0, 1.0f);
                    mClass->mExplosion->CreateExplosion(GetCollisionCenter(), owner, false, COLL_UNKNOWN);
                }
                if (!(NetGame::IsEnabled() && NetGame::IsClient()))
                {
                    float fChunk = 1.0f;
                    if (mCurHealth > -50.0f)
                    {
                        fChunk = mCurHealth / -50.0f * 0.9f + 0.1f;
                    }
                    CreateChunkCluster(fChunk);
                    RemoveFromCollisionManager();
                    RemoveFromScene();
                }
            }
            Kill();
        }
        UpdateStatus(fDeltaTime);
        if (mIndirectControlFlag)
        {

            UnitControllerPtr(mCtrl);
            if (sNoAiMovements)
            {
                mControlMove = 0.0f;
                mControlStrafe = 0.0f;
            }
        }
    }

    if (mPlayerId >= 0 && GameLoop::_bFirstPersonEnabled && netIsLocalTurn && mControlView.IsPressed() &&
        !IsForcedThirdPerson() && !IsForcedFirstPerson() && mClass->mForceMode != 1 && mClass->mForceMode != 2 &&
        mTracker)
    {
        SwitchControlView(mPlayerId);
    }

    if (mState == SoldierState::DEAD || mState == SoldierState::FLY || mState == SoldierState::ROLL ||
        mState == SoldierState::SPRINT || mState == SoldierState::HERO_FLEE)
    {
        mTargetInfo.mIsAiming = false;
    }
    else
    {
        CheckForZoom(fDeltaTime);
    }

    Weapon* pWeapon = mActiveIndex < 0 ? 0 : mWeapon[mActiveIndex];
    bool bZoomLock = mTargetInfo.mIsAiming && pWeapon && pWeapon->mClass->mZoomRate != 0.0f;
    if (mInputLockTime > 0.0f && (m_uiInputLockMask & INPUTLOCK_MOVEMENT))
    {
        mControlTurn = 0.0f;
        mControlPitch = 0.0f;
        mControlStrafe = 0.0f;
        mControlMove = 0.0f;
    }
    else if (bZoomLock)
    {
        mControlStrafe = 0.0f;
        mControlMove = 0.0f;
    }

    float fThrust = mControlMove;
    float fStrafe = mControlStrafe;
    float fTurn = GetTurnAdjusted();
    float fPitch = GetPitchAdjusted();
    float fMoveSq = fStrafe * fStrafe + fThrust * fThrust;
    if (fMoveSq > 1.0f)
    {
        float fScale = 1.0f / (float)sqrt(fMoveSq);
        fStrafe = fScale * fStrafe;
        mControlStrafe = fStrafe;
        fThrust = fScale * fThrust;
        mControlMove = fThrust;
    }

    Character* pCharacter;
    if (NetGame::IsEnabled() && mPlayerId >= 0 && (pCharacter = mCharacter) != 0 &&
        (int)(NetComm::GetTime() - pCharacter->GetSpawnTime()) < gSpawnInvicibility)
    {
        if (mControlFire[0].IsDown())
        {
            mControlFire[0].Clear();
        }
        if (mControlFire[1].IsDown())
        {
            mControlFire[1].Clear();
        }
        if (m_bPrimaryWeaponIsMelee)
        {
            mControlReload.Clear();
        }
        TurnOnInvincibleEffect();
    }
    else
    {
        TurnOffInvincibleEffect();
    }

    if (mInputLockTime > 0.0f)
    {
        mInputLockTime -= fDeltaTime;
        if (mInputLockTime < 0.0f)
        {
            m_uiInputLockMask = 0;
            m_bSlide = 0;
            mInputLockTime = 0.0f;
        }
    }

    bool bMouse = false;
    if (mPlayerId >= 0)
    {
        bMouse = NetGame::IsMouseMode(mPlayerId);
    }

    PblVector3 vLocalVelocity;
    if (pWeapon)
    {
        vLocalVelocity.x = mVelocity.y * mMatrix.m[0][1] + mVelocity.x * mMatrix.m[0][0] + mVelocity.z * mMatrix.m[0][2];
        vLocalVelocity.y = mVelocity.y * mMatrix.m[1][1] + mVelocity.x * mMatrix.m[1][0] + mVelocity.z * mMatrix.m[1][2];
        vLocalVelocity.z = mVelocity.y * mMatrix.m[2][1] + mVelocity.x * mMatrix.m[2][0] + mVelocity.z * mMatrix.m[2][2];
        pWeapon->NotifySoldierState(mState, mStateTimer + fDeltaTime, mInputLockTime > 0.0f, mTargetInfo.mIsAiming,
                                    vLocalVelocity);
        if (pWeapon->OverrideSoldierControls())
        {
            fThrust = mControlMove;
            fStrafe = mControlStrafe;
            fTurn = GetTurnAdjusted();
            fPitch = GetPitchAdjusted();
        }
    }

    bool bBusy = false;
    int iChannel;
    for (iChannel = 0; iChannel < 2; ++iChannel)
    {
        int iIndex = mWeaponIndex[iChannel];
        if (iIndex >= 0 && mWeapon[iIndex] && mWeapon[iIndex]->IsBusy())
        {
            bBusy = true;
            break;
        }
    }
    if (!bBusy)
    {
        int iBest = -1;
        for (iChannel = 1; iChannel >= 0; --iChannel)
        {
            int iIndex = mWeaponIndex[iChannel];
            if (iIndex < 0 || !mWeapon[iIndex])
            {
                continue;
            }
            Weapon* pChannelWeapon = mWeapon[iIndex];
            if (pChannelWeapon->WillBecomeBusy())
            {
                bBusy = true;
                break;
            }
            if (iBest < 0)
            {
                if (pChannelWeapon->IsMelee())
                {
                    if (static_cast<WeaponMelee*>(pChannelWeapon)->GetWeaponMeleeAnimation() < 0xa4)
                    {
                        iBest = iChannel;
                    }
                }
                else if (pChannelWeapon->mState == Weapon::RELOAD)
                {
                    iBest = iChannel;
                }
            }
        }
        if (!bBusy)
        {
            iChannel = iBest < 0 ? 0 : iBest;
        }
    }

    if (mActiveIndex == mWeaponIndex[iChannel])
    {
        if (pWeapon && !pWeapon->IsMelee() && pWeapon->mState == Weapon::RELOAD)
        {
            SetAlert(false);
        }
    }
    else
    {

        if (pWeapon && pWeapon->IsMeleeThrow() && mWeapon[0] && mWeapon[0]->IsMelee())
        {

            static_cast<WeaponMelee*>(mWeapon[0])->m_pMeleeThrow = reinterpret_cast<WeaponMeleeThrow*>(pWeapon);
        }
        bool bOffhand = pWeapon->mClass->mIsOffhand || pWeapon->IsMeleeThrow();
        Weapon* pNext = mWeapon[mWeaponIndex[iChannel]];
        bool bNextOffhand = pNext->mClass->mIsOffhand || pNext->IsMeleeThrow();
        if (mActiveChannel == iChannel || iChannel == 1)
        {
            pWeapon->Deselect(iChannel, bNextOffhand);
        }
        mActiveChannel = iChannel;
        mActiveIndex = mWeaponIndex[mActiveChannel];
        pWeapon = mWeapon[mActiveIndex];
        if (mActiveChannel == iChannel || iChannel == 0)
        {
            pWeapon->Select(mActiveChannel, bOffhand);
        }
        pWeapon->NotifySoldierState(mState, mStateTimer + fDeltaTime, mInputLockTime > 0.0f, mTargetInfo.mIsAiming,
                                    vLocalVelocity);
        if (pWeapon->OverrideSoldierControls())
        {
            fThrust = mControlMove;
            fStrafe = mControlStrafe;
            fTurn = GetTurnAdjusted();
            fPitch = GetPitchAdjusted();
        }
        SetAlert(false);
    }

    if (bBusy)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (i == iChannel)
            {
                continue;
            }
            if (i == 1 && m_bPrimaryWeaponIsMelee)
            {
                if (mPlayerId >= 0)
                {
                    mControlReload.Clear();
                }
            }
            else
            {
                mControlFire[i].Clear();
            }
        }
    }

    float fPitchRate = bMouse ? GetClassMaxTurnSpeed() : mClass->mMaxPitchSpeed;
    if (mTargetInfo.mIsAiming && pWeapon)
    {
        fPitchRate /= pWeapon->mZoomTurnScale;
    }

    float fWaterHeight;
    if (RedWater::GetWaterHeight(GetPosition(), fWaterHeight) && fWaterHeight > GetPosition().y)
    {
        if (fWaterHeight > mAimer.mRootPos.y)
        {
            mControlFire[0].Clear();
            mControlFire[1].Clear();
        }
        if (mIsAlive && fWaterHeight >= mEyePoint.y)
        {
            mDrownTime -= fDeltaTime;
            if (mDrownTime < 0.0f)
            {
                mDrownTime += 1.0f;
                DamageDesc damage(DamageOwner(0, 0, 0, 0, 0, 1.0f), 30.0f);
                Damageable::ApplyDamage(damage, 0xffffffff);
            }
        }
        else
        {
            mDrownTime = mClass->mDrownTime;
        }

        s_fWakeTimer += fDeltaTime;
        const PblVector3& vVelocity = GetVelocity();
        float fWakeInterval =
            1.0f / ((vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z) * 0.3f + 4.0f);
        if (s_fWakeTimer > fWakeInterval)
        {
            RedWater::AddWaterWake(GetPosition(), vVelocity, 2.0f, 0.25f);
            FoleyFXCollider::CollisionCallback(COLL_WATER, 0, &GetPosition());
            s_fWakeTimer -= fWakeInterval;
            if (s_fWakeTimer > fWakeInterval)
            {
                s_fWakeTimer = fWakeInterval;
            }
        }

        s_fSplashTimeout += fDeltaTime;
        if (vVelocity.y < -5.0f && s_fSplashTimeout > 1.0f)
        {
            WaterEffect::AddWaterEffect(mClass->mWaterSplashEffect, GetPosition());
            s_fSplashTimeout = 0.0f;
        }
    }
    else
    {
        mDrownTime = mClass->mDrownTime;
    }

    float fKickScale = mKickScale;
    float fPitchVelocity = mPitchVelocity;
    mPitchValue = fPitchVelocity * (fKickScale < 1.0f ? fKickScale : 1.0f) * fDeltaTime + mPitchValue;
    mPitchOffset = fPitchVelocity * fDeltaTime + mPitchOffset;
    float fDamp = fDeltaTime * 20.0f;
    mPitchVelocity = fPitchVelocity - (fDamp < 1.0f ? fDamp : 1.0f) * fPitchVelocity;
    fDamp = fDeltaTime * 6.0f;
    mPitchOffset = mPitchOffset - (fDamp < 1.0f ? fDamp : 1.0f) * mPitchOffset;
    mKickScale = mKickBuild - fDeltaTime * 2.0f + fKickScale;
    if (mKickScale < 0.0f)
    {
        mKickScale = 0.0f;
    }
    mKickBuild = 0.0f;
    mPitchValue = fPitchRate * fPitch * fDeltaTime + mPitchValue;
    if (mState == SoldierState::PRONE)
    {
        if (mPitchValue < -0.4f)
        {
            mPitchValue = -0.4f;
        }
        else if (mPitchValue > 0.4f)
        {
            mPitchValue = 0.4f;
        }
    }
    else
    {
        if (mPitchValue < mClass->mMinPitch)
        {
            mPitchValue = mClass->mMinPitch;
        }
        else if (mPitchValue > mClass->mMaxPitch)
        {
            mPitchValue = mClass->mMaxPitch;
        }
    }

    if (mState != SoldierState::JET_HOVER && mState != SoldierState::JET_JUMP)
    {
        mJetFuel = mClass->mJetFuelRechargeRate * fDeltaTime + mJetFuel;
    }
    if (mJetFuel > 1.0f)
    {
        mJetFuel = 1.0f;
    }

    mStateTimer = fDeltaTime + mStateTimer;
    mLowHealthSound.Update(&mClass->mLowHealthSound, &mMatrix.GetTranslation(), &mVelocity);
    mChokeSound.Update(&mClass->mChokeSound, &mMatrix.GetTranslation(), &mVelocity);

    if (fDeltaTime > 0.0f && VehicleEngine::IsOn())
    {
        float fSpeed = sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z);
        bool bFirstPerson = mTracker && mTracker->IsFirstPersonView();
        bool bLocalPlayer = false;
        if (mPlayerId >= 0 && (NetGame::GetLocalPlayer(0) == mCharacter || NetGame::GetLocalPlayer(1) == mCharacter))
        {
            bLocalPlayer = true;
        }
        float fHealth = 0.0f;
        if (mMaxHealth != 0.0f)
        {
            fHealth = mCurHealth / mMaxHealth;
        }
        VehicleEngine::Update(mClass, fDeltaTime, mMatrix.GetTranslation(), mVelocity, fSpeed,
                              mSpeedPrev - fSpeed / fDeltaTime, fHealth, 0.0f, 0.0f, 0.0f, 0.0f, bLocalPlayer,
                              bFirstPerson, 0.0f);
        mSpeedPrev = fSpeed;
    }

    float fOverrideForward = 0.0f;
    float fOverrideStrafe = 0.0f;
    float fOverrideBlend = 0.0f;
    float fVelocityY = mVelocity.y;
    if (pWeapon)
    {
        pWeapon->OverrideSoldierVelocity(fOverrideForward, fOverrideStrafe, fOverrideBlend, fVelocityY);
        mVelocity.y = fVelocityY;
    }

    float fEnergyRestore = 0.0f;
    if (mState == SoldierState::SPRINT)
    {
        fEnergyRestore = -mClass->mEnergyDrainSprint;
    }
    else if (mState <= SoldierState::PRONE)
    {
        const PblVector3& vVelocity = GetVelocity();
        fEnergyRestore = vVelocity.x * vVelocity.x + vVelocity.z * vVelocity.z < 1.0f ? mClass->mEnergyRestoreIdle
                                                                                      : mClass->mEnergyRestore;
    }
    if (pWeapon)
    {
        pWeapon->OverrideSoldierEnergyRestore(fEnergyRestore);
    }
    EnergyBar* pEnergyBar = reinterpret_cast<EnergyBar*>(mEnergyBar);
    if (fEnergyRestore < 0.0f)
    {
        pEnergyBar->SpendEnergy(-(fEnergyRestore * fDeltaTime));
    }
    else
    {

        float fMultiplier = mCharacter && mCharacter->mMedalsMgr &&
                                    mCharacter->mMedalsMgr->IsAwardAvailable(MedalsMgr::ENDURANCE)
                                ? 1.5f
                                : 1.0f;
        bool bTeamBonus = BonusManager::Instance()->IsTeamBonusActive(GetTeam(), 0x85ad826d);
        float fRestore = fEnergyRestore * fMultiplier * fDeltaTime;
        if (bTeamBonus)
        {
            fRestore *= 3.0f;
        }
        pEnergyBar->AddEnergy(fRestore);
    }

    switch (mState)
    {
    case SoldierState::STAND:
    {
        mStanceIndex = 0;
        const EntitySoldierClass* pClass = mClass;
        float fForward =
            fThrust * (fThrust > 0.0f ? pClass->mMaxSpeed : pClass->mMaxStrafeSpeed) * pClass->mThrustFactor[0];
        float fSide = -(pClass->mMaxStrafeSpeed * fStrafe * pClass->mStrafeFactor[0]);
        if (fOverrideBlend >= 1.0f)
        {
            fForward = fOverrideForward;
            fSide = fOverrideStrafe;
        }
        else if (fOverrideBlend > 0.0f)
        {
            fForward = (fOverrideForward - fForward) * fOverrideBlend + fForward;
            fSide = (fOverrideStrafe - fSide) * fOverrideBlend + fSide;
        }
        float fOmega = GetOmega(mPlayerId >= 0 ? pClass->mTurnFactor[0] * fTurn : fTurn);
        float fAccelScale = 1.0f;
        if (mInputLockTime > 0.0f && (m_uiInputLockMask & INPUTLOCK_MOVEMENT))
        {
            fAccelScale = m_bSlide ? 0.05f : 1.7f;
        }
        Move(fDeltaTime, fForward, fSide, fOmega, mClass->mAcceleration * fAccelScale);

        if (mFlags & flags_isGrounded)
        {
            mFlags &= ~flags_fallTimer;
        }
        else
        {
            if (!(mFlags & flags_fallTimer))
            {
                mFlags |= flags_fallTimer;
                mStateTimer = 0.0f;
            }
            if (mStateTimer > 1.0f)
            {
                SetState(SoldierState::FALL);
            }
        }

        if (mPlayerId >= 0 && (!(mFlags & flags_fallTimer) || mStateTimer < 0.3f))
        {
            if (mControlSpecialMove.IsDown() && CheckMoveControlForRoll(fThrust, fStrafe))
            {
                RollUsingEnergy();
            }
            else if (mControlCrouch.IsPressed() || mControlCrouch.Held())
            {
                Crouch();
            }
            else if (mControlJump.IsPressed())
            {
                JumpUsingEnergy();
            }
            else if ((mControlThrustFwd.TappedAndPressed() ||
                      ((mControlSprint.Tapped() || mControlSprint.Held()) && CheckMoveControlForSprint(fThrust, fStrafe) &&
                       mVelocity.x * mMatrix.m[2][0] + mVelocity.y * mMatrix.m[2][1] + mVelocity.z * mMatrix.m[2][2] >
                           mClass->mMaxSpeed * 0.9f)) &&
                     SprintUsingEnergy())
            {
                ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_SoldierSprint, mPlayerId);
            }
        }
        break;
    }
    case SoldierState::CROUCH:
    {
        mStanceIndex = 1;
        const EntitySoldierClass* pClass = mClass;
        float fForward = pClass->mMaxSpeed * fThrust * pClass->mThrustFactor[1];
        float fSide = -(pClass->mMaxStrafeSpeed * fStrafe * pClass->mStrafeFactor[1]);
        if (fOverrideBlend >= 1.0f)
        {
            fForward = fOverrideForward;
            fSide = fOverrideStrafe;
        }
        else if (fOverrideBlend > 0.0f)
        {
            fForward = (fOverrideForward - fForward) * fOverrideBlend + fForward;
            fSide = (fOverrideStrafe - fSide) * fOverrideBlend + fSide;
        }
        float fOmega = GetOmega(mPlayerId >= 0 ? pClass->mTurnFactor[1] * fTurn : fTurn);
        float fAccelScale = 1.0f;
        if (mInputLockTime > 0.0f && (m_uiInputLockMask & INPUTLOCK_MOVEMENT))
        {
            fAccelScale = m_bSlide ? 0.05f : 1.7f;
        }
        Move(fDeltaTime, fForward, fSide, fOmega, mClass->mAcceleration * fAccelScale);

        if (mFlags & flags_isGrounded)
        {
            mFlags &= ~flags_fallTimer;
        }
        else
        {
            if (!(mFlags & flags_fallTimer))
            {
                mFlags |= flags_fallTimer;
                mStateTimer = 0.0f;
            }
            if (mStateTimer > 0.3f)
            {
                Stand();
                mStateTimer = 0.3f;
            }
        }

        if (mPlayerId >= 0)
        {
            if (mControlSpecialMove.IsDown() && CheckMoveControlForRoll(fThrust, fStrafe))
            {
                RollUsingEnergy();
            }
            else if ((mControlCrouch.Tapped() || mControlCrouch.HeldAndReleased()) && mStateTimer > 0.15f)
            {
                Stand();
            }
            else if (mControlJump.IsPressed() && mStateTimer > 0.5f)
            {
                Stand();
            }
        }
        break;
    }
    case SoldierState::PRONE:
    {
        mStanceIndex = 2;
        if (fStrafe * fStrafe + fThrust * fThrust > 0.01f)
        {
            if (mControlFire[0].IsDown())
            {
                mControlFire[0].Clear();
            }
            if (mControlFire[1].IsDown())
            {
                mControlFire[1].Clear();
            }
            if (m_bPrimaryWeaponIsMelee)
            {
                mControlReload.Clear();
            }
        }
        const EntitySoldierClass* pClass = mClass;
        fThrust = fThrust * (fThrust > 0.0f ? pClass->mMaxSpeed : pClass->mMaxStrafeSpeed) * pClass->mThrustFactor[2];
        fStrafe = -(pClass->mMaxStrafeSpeed * fStrafe * pClass->mStrafeFactor[2]);
        if (fOverrideBlend >= 1.0f)
        {
            fThrust = fOverrideForward;
            fStrafe = fOverrideStrafe;
        }
        else if (fOverrideBlend > 0.0f)
        {
            fThrust = (fOverrideForward - fThrust) * fOverrideBlend + fThrust;
            fStrafe = (fOverrideStrafe - fStrafe) * fOverrideBlend + fStrafe;
        }
        float fOmega = GetOmega(pClass->mTurnFactor[2] * fTurn);
        float fAccelScale = 1.0f;
        if (mInputLockTime > 0.0f && (m_uiInputLockMask & INPUTLOCK_MOVEMENT))
        {
            fAccelScale = m_bSlide ? 0.05f : 1.7f;
        }
        Move(fDeltaTime, fThrust, fStrafe, fOmega, mClass->mAcceleration * fAccelScale);

        if (mFlags & flags_isGrounded)
        {
            mFlags &= ~flags_fallTimer;
        }
        else
        {
            if (!(mFlags & flags_fallTimer))
            {
                mFlags |= flags_fallTimer;
                mStateTimer = 0.0f;
            }
            if (mStateTimer > 0.3f)
            {
                SetState(SoldierState::FALL);
            }
        }

        if (mPlayerId >= 0)
        {
            if (mControlCrouch.IsPressed())
            {
                Stand();
            }
            else if (mControlJump.IsPressed() && mStateTimer > 0.5f)
            {
                Stand();
            }
        }
        break;
    }
    case SoldierState::SPRINT:
    {
        mStanceIndex = 0;
        const EntitySoldierClass* pClass = mClass;

        float fTime = mStateTimer;
        float fSprintSpeed = pClass->mThrustFactor[3] * pClass->mMaxSpeed;
        if (pClass->mSprintAccelerateTime > fTime)
        {
            float fRatio = fTime / pClass->mSprintAccelerateTime;
            fSprintSpeed = (1.0f - fRatio) * pClass->mMaxSpeed + fRatio * fSprintSpeed;
        }
        float fForward = fThrust * fSprintSpeed;
        float fSide = -(pClass->mMaxStrafeSpeed * fStrafe * pClass->mStrafeFactor[3]);
        if (fOverrideBlend >= 1.0f)
        {
            fForward = fOverrideForward;
            fSide = fOverrideStrafe;
        }
        else if (fOverrideBlend > 0.0f)
        {
            fForward = (fOverrideForward - fForward) * fOverrideBlend + fForward;
            fSide = (fOverrideStrafe - fSide) * fOverrideBlend + fSide;
        }
        float fOmega = GetOmega(mPlayerId >= 0 ? pClass->mTurnFactor[3] * fTurn : fTurn);
        float fAccelScale = 1.0f;
        if (mInputLockTime > 0.0f && (m_uiInputLockMask & INPUTLOCK_MOVEMENT))
        {
            fAccelScale = m_bSlide ? 0.05f : 1.7f;
        }
        Move(fDeltaTime, fForward, fSide, fOmega, mClass->mAcceleration * fAccelScale);

        if (mFlags & flags_isGrounded)
        {
            mFlags &= ~flags_fallTimer;
        }
        else
        {
            if (!(mFlags & flags_fallTimer))
            {
                mFlags |= flags_fallTimer;
                mStateTimer = 0.0f;
            }
            if (mStateTimer > 1.0f)
            {
                SetState(SoldierState::FALL);
            }
        }

        if (mPlayerId >= 0 && (!(mFlags & flags_fallTimer) || mStateTimer < 0.3f))
        {
            if (mControlSpecialMove.IsDown() && CheckMoveControlForRoll(fThrust, fStrafe))
            {
                RollUsingEnergy();
            }
            else if (mControlJump.IsPressed())
            {
                JumpUsingEnergy();
            }
            else if (mControlSprint.Tapped() || mControlSprint.HeldAndReleased() || !CheckMoveControlForSprint(fThrust, fStrafe) ||
                     mVelocity.x * mMatrix.m[2][0] + mVelocity.y * mMatrix.m[2][1] + mVelocity.z * mMatrix.m[2][2] <= 0.2f ||
                     !SprintUsingEnergy())
            {
                EndSprint();
            }
        }
        break;
    }
    case SoldierState::JUMP:
    case SoldierState::FALL:
    case SoldierState::FLY:
    {
        mStanceIndex = 0;
        mStandingOnObj = 0;
        if (mControlFire[0].IsDown())
        {
            mControlFire[0].Clear();
        }
        if (mControlFire[1].IsDown())
        {
            mControlFire[1].Clear();
        }
        if (m_bPrimaryWeaponIsMelee)
        {
            mControlReload.Clear();
        }

        bool bCanJetJump = false;
        if (mIsAlive)
        {
            int iFactor = mState == SoldierState::FLY ? 7 : 4;
            bCanJetJump = mState != SoldierState::FLY;
            float fOmega = GetOmega(mClass->mTurnFactor[iFactor] * fTurn);
            float fForward;
            float fSide;
            if (fOverrideBlend > 0.0f)
            {
                fForward = fOverrideForward;
                fSide = fOverrideStrafe;
                if (fOverrideBlend < 1.0f)
                {
                    fForward = (fForward - 0.0f) * fOverrideBlend + 0.0f;
                    fSide = (fSide - 0.0f) * fOverrideBlend + 0.0f;
                }
            }
            else
            {
                float fAcceleration = mClass->mAcceleration;
                fForward = mClass->mThrustFactor[iFactor] * fThrust * fAcceleration;
                fSide = -(mClass->mStrafeFactor[iFactor] * fStrafe * fAcceleration);
            }
            MoveOffGround(fDeltaTime, fOmega, fForward, fSide);
        }
        else
        {
            MoveOffGround(fDeltaTime, 0.0f, 0.0f, 0.0f);
        }

        float fCeiling = AIUtil::gMaxPlayerFlyHeight - mMatrix.m[3][1];
        if (fCeiling < 5.0f)
        {
            mVelocity.y = (5.0f - fCeiling) * -10.0f * fDeltaTime + mVelocity.y;
        }

        if (mStateTimer > 0.3f && (mFlags & flags_isGrounded))
        {
            if (mState != SoldierState::FLY)
            {
                Stand(false);
                break;
            }

            float fSpeedSq = mVelocity.x * mVelocity.x + mVelocity.z * mVelocity.z;
            if (fSpeedSq > 36.0f || (fSpeedSq > 9.0f && PblRandom::GetSim().GetFloat() > 0.3f))
            {
                SetState(SoldierState::TUMBLE);
            }
            else
            {
                SetState(SoldierState::BOUNCE);
            }
            if (FoleyFXSoldier* pFoley = static_cast<FoleyFXSoldier*>(GetFoleyFX()))
            {
                pFoley->mBodyFall.Play(&mMatrix.GetTranslation(), &mVelocity, false, true);
            }
        }
        else if (bCanJetJump)
        {
            if (mControlJump.IsPressed())
            {
                JetJump();
                if (GetWeaponCount() > 0 && GetWeaponPtr(0)->IsMelee())
                {
                    ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_JediForceJump, mPlayerId);
                }
                else if (mClass->mJetFuelRechargeRate > 0.0f)
                {
                    ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_JetTrooper_Jet, mPlayerId);
                }
            }
        }
        else if (mIsAlive && mClass->m_bRecoverFromTumble && mStateTimer > 0.5f)
        {
            SetState(SoldierState::FLY_RECOVER);
        }
        break;
    }
    case SoldierState::ROLL:
    {
        mStanceIndex = 1;
        if (mControlFire[0].IsDown())
        {
            mControlFire[0].Clear();
        }
        if (mControlFire[1].IsDown())
        {
            mControlFire[1].Clear();
        }
        if (m_bPrimaryWeaponIsMelee)
        {
            mControlReload.Clear();
        }
        float fOmega = GetOmega(mClass->mTurnFactor[5] * fTurn);
        float fAcceleration = mClass->mAcceleration;
        MoveOffGround(fDeltaTime, fOmega, mClass->mThrustFactor[5] * fThrust * fAcceleration,
                      -(mClass->mStrafeFactor[5] * fStrafe * fAcceleration));
        const SoldierAnimation* pDive = SoldierAnimatorClass::Instance()->GetLowerBodyActionAnimation(
            SoldierAnimType::DIVE, SoldierAnimationBank::HUMAN_RIFLE);
        if (mStateTimer > (float)(pDive->GetAnimationNumFrames() - 1) * (1.0f / 30.0f) * 0.75f)
        {
            Stand(false);
        }
        break;
    }
    case SoldierState::JET_JUMP:
    case SoldierState::JET_HOVER:
    {
        mStanceIndex = 0;
        mJetFuel -= mClass->mJetFuelCost * fDeltaTime;
        bool bEndJet;
        if (mJetFuel < 0.0f)
        {
            mJetFuel = 0.0f;
            bEndJet = true;
        }
        else
        {
            bEndJet = mControlJump.IsPressed();
        }
        if (bEndJet)
        {
            EndJetJump();
            if (m_bPrimaryWeaponIsMelee)
            {
                JetJump();
            }
            break;
        }

        float fOmega = GetOmega(mPlayerId >= 0 ? mClass->mTurnFactor[6] * fTurn : fTurn);
        if (mState == SoldierState::JET_HOVER)
        {
            const EntitySoldierClass* pClass = mClass;
            MoveJetHover(fDeltaTime,
                         fThrust * (fThrust > 0.0f ? pClass->mMaxSpeed : pClass->mMaxStrafeSpeed) *
                             pClass->mThrustFactor[6],
                         -(pClass->mMaxStrafeSpeed * fStrafe * pClass->mStrafeFactor[6]), fOmega,
                         pClass->mJetAcceleration);
        }
        else
        {
            mVelocity.y = mClass->mJetPush * fDeltaTime + mVelocity.y;
            float fJetAcceleration = mClass->mJetAcceleration;
            MoveOffGround(fDeltaTime, fOmega, fThrust * fJetAcceleration, -(fStrafe * fJetAcceleration));
        }

        float fCeiling = AIUtil::gMaxPlayerFlyHeight - mMatrix.m[3][1];
        if (fCeiling < 5.0f)
        {
            mVelocity.y = (5.0f - fCeiling) * -10.0f * fDeltaTime + mVelocity.y;
        }

        if (mFlags & flags_isGrounded)
        {
            Stand(false);
        }
        break;
    }
    case SoldierState::TUMBLE:
    {

        mStanceIndex = 1;
        if (mControlFire[0].IsDown())
        {
            mControlFire[0].Clear();
        }
        if (mControlFire[1].IsDown())
        {
            mControlFire[1].Clear();
        }
        if (m_bPrimaryWeaponIsMelee)
        {
            mControlReload.Clear();
        }
        float fOmega = GetOmega(mClass->mTurnFactor[7] * fTurn);
        float fAcceleration = mClass->mAcceleration;
        float fForward = mClass->mThrustFactor[7] * fThrust * fAcceleration -
                         (mVelocity.y * mMatrix.m[2][1] * 2.5f + mVelocity.x * mMatrix.m[2][0] * 2.5f +
                          mVelocity.z * mMatrix.m[2][2] * 2.5f);
        float fSide = mVelocity.z * mMatrix.m[0][2] * -2.5f -
                      (mVelocity.y * mMatrix.m[0][1] * 2.5f + mVelocity.x * mMatrix.m[0][0] * 2.5f) -
                      mClass->mStrafeFactor[7] * fStrafe * fAcceleration;
        MoveOffGround(fDeltaTime, fOmega, fForward, fSide);
        if (mStateTimer > 0.5f &&
            (mVelocity.x * mVelocity.x + mVelocity.z * mVelocity.z < 16.0f || mStateTimer >= 0.75f))
        {
            SetState(SoldierState::BOUNCE);
            if (FoleyFXSoldier* pFoley = static_cast<FoleyFXSoldier*>(GetFoleyFX()))
            {
                pFoley->mBodyFall.Play(&mMatrix.GetTranslation(), &mVelocity, false, true);
            }
        }
        break;
    }
    case SoldierState::BOUNCE:
    {
        mStanceIndex = 1;
        if (mControlFire[0].IsDown())
        {
            mControlFire[0].Clear();
        }
        if (mControlFire[1].IsDown())
        {
            mControlFire[1].Clear();
        }
        if (m_bPrimaryWeaponIsMelee)
        {
            mControlReload.Clear();
        }
        float fOmega = GetOmega(mClass->mTurnFactor[7] * fTurn);
        float fAcceleration = mClass->mAcceleration;
        float fForward = mClass->mThrustFactor[7] * fThrust * fAcceleration -
                         (mVelocity.y * mMatrix.m[2][1] * 10.0f + mVelocity.x * mMatrix.m[2][0] * 10.0f +
                          mVelocity.z * mMatrix.m[2][2] * 10.0f);
        float fSide = mVelocity.z * mMatrix.m[0][2] * -10.0f -
                      (mVelocity.y * mMatrix.m[0][1] * 10.0f + mVelocity.x * mMatrix.m[0][0] * 10.0f) -
                      mClass->mStrafeFactor[7] * fStrafe * fAcceleration;
        MoveOffGround(fDeltaTime, fOmega, fForward, fSide);
        if ((mFlags & flags_isGrounded) &&
            (mVelocity.x * mVelocity.x + mVelocity.z * mVelocity.z < 0.36f || mStateTimer > 1.0f))
        {
            Stand(false);
        }
        break;
    }
    case SoldierState::FLY_RECOVER:
    {
        if (mControlFire[0].IsDown())
        {
            mControlFire[0].Clear();
        }
        if (mControlFire[1].IsDown())
        {
            mControlFire[1].Clear();
        }
        if (m_bPrimaryWeaponIsMelee)
        {
            mControlReload.Clear();
        }

        const PblVector3& vFront = GetMatrix().GetZAxis();
        float fForwardSpeed = mVelocity.y * vFront.y + mVelocity.x * vFront.x + mVelocity.z * vFront.z;
        const PblVector3& vRight = GetMatrix().GetXAxis();
        float fSideSpeed = mVelocity.y * vRight.y + mVelocity.x * vRight.x + mVelocity.z * vRight.z;
        float fSpeedSq = fSideSpeed * fSideSpeed + fForwardSpeed * fForwardSpeed;
        float fOmega = 0.0f;
        if (fSpeedSq > 0.01f)
        {
            static const float sMaxSoldierTurnRate = PblMath::fTwoPi * 3.0f;
            float fAngle = (float)atan2(fSideSpeed, fForwardSpeed);
            if (fAngle > sMaxSoldierTurnRate * fDeltaTime)
            {
                fOmega = sMaxSoldierTurnRate;
            }
            else if (-(sMaxSoldierTurnRate * fDeltaTime) > fAngle)
            {
                fOmega = -sMaxSoldierTurnRate;
            }
            else
            {
                fOmega = fAngle;
                if (fDeltaTime > 0.0f)
                {
                    fOmega = fAngle / fDeltaTime;
                }
            }
        }
        float fAcceleration = mClass->mAcceleration;
        MoveOffGround(fDeltaTime, fOmega, mClass->mThrustFactor[7] * fThrust * fAcceleration,
                      -(mClass->mStrafeFactor[7] * fStrafe * fAcceleration));
        if (mFlags & flags_isGrounded)
        {
            if (fSpeedSq > 64.0f)
            {
                SetState(SoldierState::TUMBLE);
            }
            else
            {
                Stand(false);
            }
            if (FoleyFXSoldier* pFoley = static_cast<FoleyFXSoldier*>(GetFoleyFX()))
            {
                pFoley->mBodyFall.Play(&mMatrix.GetTranslation(), &mVelocity, false, true);
            }
        }
        else if (mStateTimer > 0.5f)
        {
            SetState(SoldierState::FALL);
            mStateTimer = 2.0f;
        }
        break;
    }
    case SoldierState::TUMBLE_RECOVER:
    {
        mStanceIndex = 1;
        if (mControlFire[0].IsDown())
        {
            mControlFire[0].Clear();
        }
        if (mControlFire[1].IsDown())
        {
            mControlFire[1].Clear();
        }
        if (m_bPrimaryWeaponIsMelee)
        {
            mControlReload.Clear();
        }
        float fOmega = GetOmega(mClass->mTurnFactor[7] * fTurn);
        float fAcceleration = mClass->mAcceleration;
        MoveOffGround(fDeltaTime, fOmega, mClass->mThrustFactor[7] * fThrust * fAcceleration,
                      -(mClass->mStrafeFactor[7] * fStrafe * fAcceleration));
        if (mStateTimer > 0.5f)
        {
            Stand(false);
        }
        break;
    }
    case SoldierState::CHOKE:
    {

        mStanceIndex = 0;
        if (mControlFire[0].IsDown())
        {
            mControlFire[0].Clear();
        }
        if (mControlFire[1].IsDown())
        {
            mControlFire[1].Clear();
        }
        if (m_bPrimaryWeaponIsMelee)
        {
            mControlReload.Clear();
        }
        static const float CHOKE_MAX_ROTATE_RATE = PblMath::fPi;
        float fOmega = 0.0f;
        float fFront = m_vChokeDir.x * mMatrix.m[2][0] + m_vChokeDir.y * mMatrix.m[2][1] + m_vChokeDir.z * mMatrix.m[2][2];
        if (fFront < 0.7071f)
        {
            fOmega = m_vChokeDir.x * mMatrix.m[0][0] + m_vChokeDir.y * mMatrix.m[0][1] + m_vChokeDir.z * mMatrix.m[0][2] > 0.0f
                         ? CHOKE_MAX_ROTATE_RATE
                         : -CHOKE_MAX_ROTATE_RATE;
        }
        else if (fFront < 0.99f)
        {
            fOmega = (m_vChokeDir.x * mMatrix.m[0][0] + m_vChokeDir.y * mMatrix.m[0][1] + m_vChokeDir.z * mMatrix.m[0][2]) *
                     CHOKE_MAX_ROTATE_RATE * 1.4142f;
        }
        Move(fDeltaTime, 0.0f, 0.0f, fOmega, 36.0f);
        if (mStateTimer > 0.5f)
        {
            Stand();
        }
        break;
    }
    case SoldierState::DEAD:
    case SoldierState::HERO_FLEE:
    {
        if (mState == SoldierState::DEAD)
        {
            mStanceIndex = 2;
            if (mControlFire[0].IsDown())
            {
                mControlFire[0].Clear();
            }
            if (mControlFire[1].IsDown())
            {
                mControlFire[1].Clear();
            }
            if (m_bPrimaryWeaponIsMelee)
            {
                mControlReload.Clear();
            }

            float fTime = mStateTimer < 1.0f ? mStateTimer : 1.0f;
            if (IsInCollisionList())
            {
                Move(fDeltaTime, 0.0f, 0.0f, 0.0f, fTime * 100.0f * fTime);
            }
        }
        else
        {
            mStanceIndex = 1;
        }
        if (mStateTimer > 4.0f || mVanishing)
        {
            RemoveFromCollisionManager();
            if (!(NetGame::IsEnabled() && NetGame::IsClient()))
            {
                return false;
            }
        }
        break;
    }
    default:
    {
        const EntitySoldierClass* pClass = mClass;
        float fForwardSpeed = fThrust > 0.0f ? pClass->mMaxSpeed : pClass->mMaxStrafeSpeed;
        float fSide = -(pClass->mMaxStrafeSpeed * fStrafe);
        float fOmega = GetOmega(fTurn);
        Move(fDeltaTime, fThrust * fForwardSpeed, fSide, fOmega, mClass->mAcceleration);
        break;
    }
    }

    if (!mIsAlive && mStateTimer > 4.0f)
    {
        Kill();
    }

    if (pWeapon)
    {
        pWeapon->NotifySoldierState(mState, mStateTimer, mInputLockTime > 0.0f, mTargetInfo.mIsAiming, vLocalVelocity);
    }
    UpdateWeaponAndAimer(fDeltaTime);
    if (pWeapon)
    {
        pWeapon->OverrideAimer();
    }

    if (mState == SoldierState::DEAD)
    {
        SetCollisionModel(&mClass->mDeadCollision);
    }
    else
    {
        SetCollisionStance(mStanceIndex);
    }
    RecalculateAABB();

    int iStance = mState == SoldierState::SPRINT ? 3 : mStanceIndex;
    if (iStance != iPrevStance)
    {
        mCameraSettingPrev = iPrevStance;
        mCameraSettingTimer = 0.0f;
    }
    else
    {
        mCameraSettingTimer = fDeltaTime + mCameraSettingTimer;
    }

    if (mState != SoldierState::JET_HOVER && mState != SoldierState::JET_JUMP && VehicleEngine::IsOn())
    {
        VehicleEngine::TurnOff(mClass, mMatrix.GetTranslation(), mVelocity);
    }

    if (mIsAlive)
    {

        for (int iSwitch = 0; iSwitch < 2; ++iSwitch)
        {
            MedalsMgr* pMedals = 0;
            if (mCharacter)
            {
                pMedals = mCharacter->mMedalsMgr;
            }
            int iIndex = mWeaponIndex[iSwitch];
            if (iIndex < 0 || mControlSwitch[iSwitch] == 0.0f)
            {
                continue;
            }

            int iNew = iIndex;
            bool bAvailable;
            do
            {
                if (mControlSwitch[iSwitch] < 0.0f)
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
                bAvailable = true;
                if (mWeapon[iNew])
                {
                    const WeaponClass* pClass = mWeapon[iNew]->mClass;
                    if (pMedals)
                    {
                        bAvailable = pClass->mMedalsTypeToUnlock == -1 ||
                                     pMedals->IsAwardAvailable(pClass->mMedalsTypeToUnlock);
                        if (bAvailable && pClass->mMedalsTypeToLock != -1 &&
                            pMedals->IsAwardAvailable(pClass->mMedalsTypeToLock))
                        {
                            bAvailable = false;
                        }
                    }
                    else
                    {
                        bAvailable = pClass->mMedalsTypeToUnlock == -1;
                    }
                }
            } while (!((iNew == iIndex || mClass->mWeaponChannel[iNew] == iSwitch) && bAvailable));

            if (iNew == iIndex || !mWeapon[iNew])
            {
                if (iJoystick >= 0)
                {
                    WeaponClass::sWeaponChangeUnableSound.Play(&mMatrix.GetTranslation(), 0, false, true);
                }
            }
            else
            {
                Weapon* pOld = mWeapon[iIndex];
                if (pOld->mSelectedFlag)
                {
                    pOld->Deselect(iSwitch, false);
                    mWeaponIndex[iSwitch] = (char)iNew;
                    mWeapon[mWeaponIndex[iSwitch]]->Select(iSwitch, false);
                    SetAlert(false);
                }
                else
                {
                    mWeaponIndex[iSwitch] = (char)iNew;
                }
                if (iSwitch == mActiveChannel)
                {
                    mTargetInfo.mIsAiming = false;
                }
            }
        }

        if (UpdateBoardVsChangeClass())
        {
            mNetJustChangedState = 0;
            return true;
        }

        if (mControlSquadCommand.IsPressed())
        {
            const GameObject* pTarget = mReticuleTarget[0];
            UnitController* pUnit;
            if (pTarget && GetAffiliationToMe(pTarget) >= 0 && pTarget->GetControllable() &&
                (pUnit = UnitControllerPtr(pTarget->mControllableList.GetFirst()->_pObject)) != 0)
            {
                if (pTarget->IsRtti(EntityHover::rttiHashEntityHover) || pTarget->IsRtti(EntityWalker::rttiHashEntityWalker))
                {
                    if (mClass->mCanEnterVehicles)
                    {
                        AI::EventInfo waitInfo(AI::EVT_Vehicle_WaitForPickUp, 8.0f, 1);
                        pUnit->SendEvent(&waitInfo);
                        AI::VOHelper::Play(AI::VOT_SC_VehicleWaitUp, this, 0, pTarget, 0.0f);
                        GameObject* pDriver =
                            pTarget->mControllableList.GetFirst()->_pObject->mCharacter->mUnit->GetGameObject();
                        float fDelay = AIUtil::GetRandomFloat(0.5f, 1.2f);
                        AI::VOHelper::Play(AI::VOT_SC_VehicleWaitUpResponse, pDriver, this, this,
                                           fDelay + GameLoop::GetMissionTime());
                        AI::EventType eCommand = AI::EVT_Vehicle_WaitForPickUp;
                        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
                        {
                            EventManager::CharacterEvent::mIssueAICommand.Trigger(*mCharacter, eCommand);
                        }
                        const unsigned short* pText = RedLocalizeDB::Find(0xa0590ed7, false);
                        for (unsigned int i = 0; i < NetGame::GetNumLocalPlayers(); ++i)
                        {
                            if (NetGame::GetLocalPlayer(i) == mCharacter)
                            {
                                MessageDisplay::Instance(i, MessageDisplay::DISPLAY_SYSTEM_MSG)
                                    ->AddMessage(pText, RedColor::CYAN);
                            }
                        }
                    }
                }
                else if (mClass->mHeroFlag)
                {
                    AI::VOHelper::Play(AI::VOT_SC_Follow, this, 0, pTarget, 0.0f);
                }
                else
                {
                    AI::EventInfo queryInfo(AI::EVQ_SC_IsFollowingChr, this);
                    if (pUnit->SendEvent(&queryInfo))
                    {
                        AI::EventInfo stopInfo(AI::EVT_SC_StopFollowChr, this);
                        pUnit->SendEvent(&stopInfo);
                        AI::VOHelper::Play(AI::VOT_SC_StopFollow, this, 0, pTarget, 0.0f);
                        AI::EventType eCommand = AI::EVT_SC_StopFollowChr;
                        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
                        {
                            EventManager::CharacterEvent::mIssueAICommand.Trigger(*mCharacter, eCommand);
                        }
                        const unsigned short* pText = RedLocalizeDB::Find(0x0e0280e5, false);
                        for (unsigned int i = 0; i < NetGame::GetNumLocalPlayers(); ++i)
                        {
                            if (NetGame::GetLocalPlayer(i) == mCharacter)
                            {
                                MessageDisplay::Instance(i, MessageDisplay::DISPLAY_SYSTEM_MSG)
                                    ->AddMessage(pText, RedColor::CYAN);
                            }
                        }
                    }
                    else
                    {

                        if (ReserveManager::GetOpenReservableSlots(this, GetTeam(), RT_FORMATION) < 1)
                        {
                            AI::EventInfo stopInfo(AI::EVT_SC_StopFollowChr, this);
                            ControllerManager::BroadcastEvent(&stopInfo, 1);
                        }
                        AI::EventInfo followInfo(AI::EVT_SC_FollowChr, this);
                        pUnit->SendEvent(&followInfo);
                        AI::VOHelper::Play(AI::VOT_SC_Follow, this, 0, pTarget, 0.0f);
                        AI::EventType eCommand = AI::EVT_SC_FollowChr;
                        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
                        {
                            EventManager::CharacterEvent::mIssueAICommand.Trigger(*mCharacter, eCommand);
                        }
                        const unsigned short* pText = RedLocalizeDB::Find(0x731fd493, false);
                        for (unsigned int i = 0; i < NetGame::GetNumLocalPlayers(); ++i)
                        {
                            if (NetGame::GetLocalPlayer(i) == mCharacter)
                            {
                                MessageDisplay::Instance(i, MessageDisplay::DISPLAY_SYSTEM_MSG)
                                    ->AddMessage(pText, RedColor::CYAN);
                            }
                        }
                    }
                }
            }
        }

        if (mControlSquadCommand.IsDown())
        {
            mSquadCommandClearTimer += fDeltaTime;
            if (mSquadCommandClearTimer > 1.0f)
            {
                mSquadCommandClearTimer = -FLT_MAX;
                AI::EventInfo queryInfo(AI::EVQ_SC_IsFollowingChr, this);
                ControllerManager::BroadcastEvent(&queryInfo, 1000);
                if (queryInfo.mInt > 0)
                {
                    AI::EventInfo stopInfo(AI::EVT_SC_StopFollowChr, this);
                    ControllerManager::BroadcastEvent(&stopInfo, 1000);
                    AI::VOHelper::Play(AI::VOT_SC_StopFollow, this, 0, 0, 0.0f);
                    AI::EventType eCommand = AI::EVT_SC_StopFollowChr;
                    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
                    {
                        EventManager::CharacterEvent::mIssueAICommand.Trigger(*mCharacter, eCommand);
                    }
                    const unsigned short* pText = RedLocalizeDB::Find(0x0e0280e5, false);
                    for (unsigned int i = 0; i < NetGame::GetNumLocalPlayers(); ++i)
                    {
                        if (NetGame::GetLocalPlayer(i) == mCharacter)
                        {
                            MessageDisplay::Instance(i, MessageDisplay::DISPLAY_SYSTEM_MSG)
                                ->AddMessage(pText, RedColor::CYAN);
                        }
                    }
                }
            }
        }
        else
        {
            mSquadCommandClearTimer = 0.0f;
        }

        UpdateOrdnanceCollision(fDeltaTime);
    }

    mFlags &= ~flags_isGrounded;
    mLastRenderedLod = -1;
    if (mIsAlive)
    {
        int iFirstPerson = GetFirstPersonIndex();
        if (iFirstPerson >= 0)
        {
            FirstPerson::SetSoldierState(iFirstPerson, mState);
            FirstPerson::Update(iFirstPerson, TYPE_SOLDIER, mClass->mFirstPersonModelHandle, this, &mAimer);
        }
    }
    mNetJustChangedState = 0;
    return true;
}

// FUNCTION: SWBF2 0x004ed060
bool EntitySoldier::Stand(bool bArg)
{
    return Stand();
}

// FUNCTION: SWBF2 0x004ed080
bool EntitySoldier::Stand()
{
    if (mState == SoldierState::CROUCH || mState == SoldierState::PRONE)
    {
        const PblVector3& vPos = mMatrix.GetTranslation();
        float fHeight = sCollisionHeight[mState];
        PblVector3 vStart(vPos.x + 0.0f, vPos.y + fHeight, vPos.z + 0.0f);
        PblVector3 vDir(0.0f, 1.0f, 0.0f);
        CollisionObject* pHit;
        PblVector3 vNormal;

        if (CollisionManager::RayHit(vStart, vDir, sCollisionHeight[SoldierState::STAND] - fHeight, pHit,
                                     &vNormal, NULL, 0, COLL_RIGID | COLL_STATIC, true) <= 0.9999f)
        {
            if (mState == SoldierState::PRONE)
            {
                return Crouch();
            }
            return false;
        }
    }

    float fOldStateTimer = mStateTimer;
    if (!SetState(SoldierState::STAND))
    {
        return false;
    }

    int iFirstPerson = GetFirstPersonIndex();
    Weapon* pWeapon = mActiveIndex >= 0 ? mWeapon[mActiveIndex] : NULL;
    switch (mOldState)
    {
    case SoldierState::CROUCH:
    case SoldierState::PRONE:
    {
        if (mOldState == SoldierState::CROUCH)
        {
            if (iFirstPerson >= 0)
            {
                FirstPerson::CrouchToStand(iFirstPerson);
            }
        }
        else
        {
            const SoldierAnimation* pAnim = SoldierAnimatorClass::Instance()->GetLowerBodyActionAnimation(
                SoldierAnimType::PRONE_TO_STAND, SoldierAnimationBank::HUMAN_RIFLE);
            m_uiInputLockMask = INPUTLOCK_ALL;
            mInputLockTime = (float)(pAnim->GetAnimationNumFrames() - 1) * (1.0f / 30.0f);
            if (iFirstPerson >= 0)
            {
                FirstPerson::ProneToStand(iFirstPerson);
            }
        }
        if (FoleyFXSoldier* pFoley = static_cast<FoleyFXSoldier*>(GetFoleyFX()))
        {
            pFoley->mStand.Play(&mMatrix.GetTranslation(), &mVelocity, false, true);
        }
        if (pWeapon)
        {
            pWeapon->PlayFoleyFX(9);
        }
        break;
    }
    case SoldierState::JUMP:
    case SoldierState::JET_JUMP:
    case SoldierState::JET_HOVER:
    case SoldierState::FALL:
    case SoldierState::FLY_RECOVER:
    case SoldierState::CHOKE:
        if (mOldState == SoldierState::FLY_RECOVER || mOldState == SoldierState::CHOKE || m_bSlide)
        {
            m_uiInputLockMask = INPUTLOCK_ALL;
            mInputLockTime = 0.5f;
        }
        else if (mOldState == SoldierState::JET_HOVER || mOldState == SoldierState::JET_JUMP ||
                 fOldStateTimer <= 1.5f)
        {
            m_uiInputLockMask = INPUTLOCK_ACTION;
            mInputLockTime = 0.3f;
        }
        else
        {
            m_uiInputLockMask = INPUTLOCK_ALL;
            mInputLockTime = 0.5f;
        }
        if (iFirstPerson >= 0)
        {
            FirstPerson::Land(iFirstPerson);
        }
        if (FoleyFXSoldier* pFoley = static_cast<FoleyFXSoldier*>(GetFoleyFX()))
        {
            pFoley->mLand.Play(&mMatrix.GetTranslation(), &mVelocity, false, true);
        }
        if (pWeapon)
        {
            pWeapon->PlayFoleyFX(5);
        }
        mSoundState |= 0x20;
        break;
    case SoldierState::ROLL:
    case SoldierState::TUMBLE_RECOVER:
        m_uiInputLockMask = INPUTLOCK_ACTION;
        mInputLockTime = 0.3f;
        break;
    case SoldierState::FLY:
    case SoldierState::TUMBLE:
    case SoldierState::BOUNCE:
    {
        if (!IsAlive())
        {
            Kill();
            return false;
        }
        const SoldierAnimation* pAnim = SoldierAnimatorClass::Instance()->GetLowerBodyActionAnimation(
            SoldierAnimType::GETUP_FRONT, SoldierAnimationBank::HUMAN_RIFLE);
        m_uiInputLockMask = INPUTLOCK_ALL;
        mInputLockTime = (float)(pAnim->GetAnimationNumFrames() - 1) * (1.0f / 30.0f);
        break;
    }
    default:
        break;
    }

    if (mOldState == SoldierState::PRONE)
    {

        const PblVector3& vFront = mMatrix.GetZAxis();
        PblVector3 vUp(0.0f, 1.0f, 0.0f);
        PblVector3 vCross(vUp.y * vFront.z - vUp.z * vFront.y, vUp.z * vFront.x - vUp.x * vFront.z,
                          vUp.x * vFront.y - vUp.y * vFront.x);
        PblVector3 vRight;
        D3DXVec3Normalize(&vRight, &vCross);
        vCross = PblVector3(vRight.y * vUp.z - vRight.z * vUp.y, vRight.z * vUp.x - vRight.x * vUp.z,
                            vRight.x * vUp.y - vRight.y * vUp.x);
        PblVector3 vNewFront;
        D3DXVec3Normalize(&vNewFront, &vCross);
        const PblVector3& vOrigin = mMatrix.GetTranslation();
        PblMatrix mNew;
        mNew.m[0][0] = vRight.x;
        mNew.m[0][1] = vRight.y;
        mNew.m[0][2] = vRight.z;
        mNew.m[0][3] = 0.0f;
        mNew.m[1][0] = vUp.x;
        mNew.m[1][1] = vUp.y;
        mNew.m[1][2] = vUp.z;
        mNew.m[1][3] = 0.0f;
        mNew.m[2][0] = vNewFront.x;
        mNew.m[2][1] = vNewFront.y;
        mNew.m[2][2] = vNewFront.z;
        mNew.m[2][3] = 0.0f;
        mNew.m[3][0] = vOrigin.x;
        mNew.m[3][1] = vOrigin.y;
        mNew.m[3][2] = vOrigin.z;
        mNew.m[3][3] = 1.0f;
        SetMatrix(mNew);
    }
    else if (mMatrix.m[1][1] < 1.0f)
    {
        PblVector3 vDir(mMatrix.m[2][0], 0.0f, mMatrix.m[2][2]);
        SetMatrix(DirectionMatrix(vDir, gVectorUnitY, mMatrix.GetTranslation()));
    }
    return true;
}

// FUNCTION: SWBF2 0x004ed550
bool EntitySoldier::Crouch()
{
    if (!SetState(SoldierState::CROUCH))
    {
        return false;
    }
    if (FoleyFXSoldier* pFoley = static_cast<FoleyFXSoldier*>(GetFoleyFX()))
    {
        pFoley->mSquat.Play(&mMatrix.GetTranslation(), &mVelocity, false, true);
    }
    if (mActiveIndex >= 0 && mWeapon[mActiveIndex])
    {
        mWeapon[mActiveIndex]->PlayFoleyFX(8);
    }
    return true;
}

// FUNCTION: SWBF2 0x004ed5d0
bool EntitySoldier::Jump()
{
    if (m_bSlide)
    {
        return false;
    }
    if (mState != SoldierState::STAND && mState != SoldierState::SPRINT)
    {
        return false;
    }
    if (!SetState(SoldierState::JUMP))
    {
        return false;
    }

    const PblVector3* pAIDir = NULL;
    if (UnitControllerPtr(mCtrl))
    {
        const PblVector3& vAIDir = UnitControllerPtr(mCtrl)->GetLowLevel().mAIJetJumpDir;
        if (vAIDir.x * vAIDir.x + vAIDir.y * vAIDir.y + vAIDir.z * vAIDir.z > 0.1f)
        {
            pAIDir = &UnitControllerPtr(mCtrl)->GetLowLevel().mAIJetJumpDir;
        }
    }
    if (pAIDir)
    {
        PblVector3 vLifted(pAIDir->x, 0.01f, pAIDir->z);
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vLifted);
        float fSpeed = mClass->mMaxSpeed;
        float fFactor = mClass->mJumpForwardSpeedFactor;
        PblVector3 vPush(vDir.x * fSpeed * fFactor, vDir.y * fSpeed * fFactor, vDir.z * fSpeed * fFactor);
        mVelocity = PblVector3(0.0f + vPush.x, mClass->mJumpVerticalSpeed + vPush.y, 0.0f + vPush.z);
    }
    else
    {
        mVelocity.y += mClass->mJumpVerticalSpeed;
        const PblVector3& vFront = mMatrix.GetZAxis();
        const PblVector3& vRight = mMatrix.GetXAxis();
        float fForward = mVelocity.x * vFront.x + mVelocity.y * vFront.y + mVelocity.z * vFront.z;
        float fSide = mVelocity.x * vRight.x + mVelocity.y * vRight.y + mVelocity.z * vRight.z;
        float fScale;
        if (fForward > 0.0f)
        {
            fScale = (mClass->mJumpForwardSpeedFactor - 1.0f) * fForward;
        }
        else
        {
            fScale = (mClass->mJumpStrafeSpeedFactor - 1.0f) * fForward;
        }
        mVelocity.x += vFront.x * fScale;
        mVelocity.y += vFront.y * fScale;
        mVelocity.z += vFront.z * fScale;
        fScale = (mClass->mJumpStrafeSpeedFactor - 1.0f) * fSide;
        mVelocity.x += vRight.x * fScale;
        mVelocity.y += vRight.y * fScale;
        mVelocity.z += vRight.z * fScale;
    }

    if (FoleyFXSoldier* pFoley = static_cast<FoleyFXSoldier*>(GetFoleyFX()))
    {
        mCurFoleySound = pFoley->mJump;
        mLongFoleySound.Play(&mCurFoleySound, &mMatrix.GetTranslation(), &mVelocity, true);
    }
    if (mActiveIndex >= 0 && mWeapon[mActiveIndex])
    {
        mWeapon[mActiveIndex]->PlayFoleyFX(4);
    }
    mSoundState |= 0x40;
    if (mCtrl)
    {
        mCtrl->IncreaseTargetBubbleSize(AI::TBI_JUMP);
    }
    return true;
}

// FUNCTION: SWBF2 0x004eddb0
bool EntitySoldier::JetJump()
{
    if (mState != SoldierState::JUMP && mState != SoldierState::FALL)
    {
        return false;
    }
    if (mClass->mJetFuelMinBorder > mJetFuel)
    {
        return false;
    }
    if (mCharacter && mCharacter->mCarriedFlagPtr)
    {
        return false;
    }
    if (!reinterpret_cast<EnergyBar*>(mEnergyBar)->SpendEnergy(mClass->mJetEnergyDrain))
    {
        return false;
    }

    mJetFuel -= mClass->mJetFuelInitialCost;
    if (mJetFuel < 0.0f)
    {
        mJetFuel = 0.0f;
    }

    float fJet = mClass->mJetJump;
    float fForward = mControlMove > -0.1f ? mControlMove : -0.1f;
    float fStrafe = mControlStrafe * -0.1f;
    const PblVector3& vFront = mMatrix.GetZAxis();
    const PblVector3& vRight = mMatrix.GetXAxis();
    PblVector3 vPush(vRight.x * fStrafe + vFront.x * fForward + 0.0f,
                     vRight.y * fStrafe + vFront.y * fForward + 1.1f,
                     vRight.z * fStrafe + vFront.z * fForward + 0.0f);
    PblVector3 vDir;
    D3DXVec3Normalize(&vDir, &vPush);
    if (mPlayerId < 0)
    {
        const PblVector3& vAIDir = UnitControllerPtr(mCtrl)->GetLowLevel().mAIJetJumpDir;
        if (vAIDir.x * vAIDir.x + vAIDir.y * vAIDir.y + vAIDir.z * vAIDir.z > 0.0f)
        {
            vDir = vAIDir;
        }
    }
    mVelocity.x += vDir.x * fJet;
    mVelocity.y += vDir.y * fJet;
    mVelocity.z += vDir.z * fJet;

    float fOldStateTimer = mStateTimer;
    if (m_bPrimaryWeaponIsMelee && !mClass->mJetTypeHover)
    {
        SetState(SoldierState::JUMP);
        float fVertical = mClass->mJumpVerticalSpeed;
        if (fVertical * 0.5f > mVelocity.y)
        {
            fVertical += mVelocity.y;
        }
        else
        {
            fVertical *= 1.5f;
        }

        float fSpeed;
        if (mControlMove > 0.0f)
        {
            fSpeed = mClass->mMaxSpeed * mControlMove;
        }
        else
        {
            fSpeed = mClass->mMaxStrafeSpeed * mControlMove;
        }
        mVelocity = PblVector3(vFront.x * fSpeed, vFront.y * fSpeed, vFront.z * fSpeed);
        float fSide = -(mClass->mMaxStrafeSpeed * mControlStrafe);
        mVelocity.x += vRight.x * fSide;
        mVelocity.y += vRight.y * fSide;
        mVelocity.z += vRight.z * fSide;
        mVelocity.y = fVertical;

        if (!mSoldierAnimator ||
            mSoldierAnimator->RestartJumpAnimation(mControlMove, -mControlStrafe,
                                                   mState == SoldierState::JUMP && fOldStateTimer >= 0.5f))
        {
            mStateTimer = 0.0f;
        }
    }
    else
    {
        SetState(mClass->mJetTypeHover ? SoldierState::JET_HOVER : SoldierState::JET_JUMP);
    }

    if (!VehicleEngine::IsOn())
    {
        float fHealth = 0.0f;
        if (mMaxHealth != 0.0f)
        {
            fHealth = mCurHealth / mMaxHealth;
        }
        VehicleEngine::TurnOn(mClass, mMatrix.GetTranslation(), mVelocity, fHealth);
    }
    if (mCtrl)
    {
        mCtrl->IncreaseTargetBubbleSize(AI::TBI_JETJUMP);
    }
    return true;
}

// FUNCTION: SWBF2 0x004ee2c0
bool EntitySoldier::SetState(SoldierState::SoldierState eState)
{
    if (mState == eState || mState == SoldierState::DEAD || mState == SoldierState::HERO_FLEE)
    {
        return false;
    }

    if (mStandingOnObj)
    {
        switch (eState)
        {
        case SoldierState::JUMP:
        case SoldierState::JET_JUMP:
        case SoldierState::FLY:
        {
            const PblVector3& vVelocity = mStandingOnObj->GetVelocity();
            mVelocity.x += vVelocity.x;
            mVelocity.y += vVelocity.y;
            mVelocity.z += vVelocity.z;
            mStandingOnObj = NULL;
            break;
        }
        case SoldierState::DEAD:
        case SoldierState::FALL:
        case SoldierState::HERO_FLEE:
        case SoldierState::TUMBLE:
        case SoldierState::BOUNCE:
        case SoldierState::JET_HOVER:
            mStandingOnObj = NULL;
            break;
        default:
            break;
        }
    }

    if (mPlayerId >= 0 && IsNotFirstPersonState(eState) && mTracker && mTracker->IsFirstPersonView())
    {
        int iJoystick = NetGame::GetJoystickIndex(mPlayerId);
        if (iJoystick >= 0)
        {
            FirstPerson::Activate(iJoystick, false);
        }
        mFlyingInFirstPerson = 1;
    }

    if (mState != SoldierState::CHOKE && mChokeSound.GetPlaying())
    {
        mChokeSound.Stop(true);
    }
    if (mState != SoldierState::PILOT_ANIMATION)
    {
        m_ePilotAnimation = SoldierAnimationBank::INVALID_ANIMATION;
        mAnimationHash = 0;
    }
    mOldState = mState;
    mState = eState;
    mStateTimer = 0.0f;
    return true;
}

// FUNCTION: SWBF2 0x004ee4e0
float EntitySoldier::GetOmega(float fTurn) const
{
    bool bMouse = false;
    if (mPlayerId >= 0)
    {
        bMouse = NetGame::IsMouseMode(mPlayerId);
    }

    float fOmega;
    if (mPlayerId >= 0 && !bMouse)
    {
        float fMin = mClass->mMaxPitchSpeed;
        fOmega = (GetClassMaxTurnSpeed() - fMin) * mTurnBuildup + fMin;
    }
    else
    {
        fOmega = GetClassMaxTurnSpeed();
    }

    if (mTargetInfo.mIsAiming && mActiveIndex >= 0 && mWeapon[mActiveIndex] && mPlayerId >= 0)
    {
        fOmega /= mWeapon[mActiveIndex]->mZoomTurnScale;
    }
    return -(fOmega * fTurn);
}

// FUNCTION: SWBF2 0x004ee590
void EntitySoldier::Move(float fDeltaTime, float fForward, float fStrafe, float fTurn, float fAcceleration)
{
    if (mFlags & flags_isGrounded)
    {
        float fDot = mGroundNormal.x * mVelocity.x + mVelocity.y * mGroundNormal.y +
                     mVelocity.z * mGroundNormal.z;
        if (fDot < 0.0f)
        {
            mVelocity.x -= mGroundNormal.x * fDot;
            mVelocity.y -= mGroundNormal.y * fDot;
            mVelocity.z -= mGroundNormal.z * fDot;
        }
    }

    float fNormalY = mGroundNormal.y > 0.0f ? mGroundNormal.y : 0.0f;
    const PblVector3& vRight = mMatrix.GetXAxis();
    const PblVector3& vNormal = mGroundNormal;
    PblVector3 vFront(vRight.y * vNormal.z - vRight.z * vNormal.y, vRight.z * vNormal.x - vRight.x * vNormal.z,
                      vRight.x * vNormal.y - vRight.y * vNormal.x);
    PblVector3 vSide(vNormal.y * vFront.z - vNormal.z * vFront.y, vNormal.z * vFront.x - vNormal.x * vFront.z,
                     vNormal.x * vFront.y - vNormal.y * vFront.x);
    if (fNormalY < 0.99f)
    {
        PblVector3 vUnit;
        D3DXVec3Normalize(&vUnit, &vFront);
        vFront = vUnit;
        D3DXVec3Normalize(&vUnit, &vSide);
        vSide = vUnit;
    }

    float fFrontSpeed = vFront.x * mVelocity.x + vFront.y * mVelocity.y + vFront.z * mVelocity.z;
    float fSideSpeed = vSide.x * mVelocity.x + vSide.y * mVelocity.y + vSide.z * mVelocity.z;
    if ((mFlags & flags_isGrounded) && fNormalY < 0.7f)
    {
        fAcceleration *= 0.1f;
    }
    float fMaxChange = fAcceleration * fDeltaTime;

    float fSlopeFront = 0.0f;
    float fSlopeSide = 0.0f;
    if (mFlags & flags_isGrounded)
    {
        float fGravity = fDeltaTime * GRAVITY_FORCE;
        fSlopeFront = -(vFront.y * fGravity);
        fSlopeSide = -(vSide.y * fGravity);
        float fUphill = vFront.y * fForward + vSide.y * fStrafe;
        if (fUphill > 0.0f)
        {
            float fKeep = 1.0f - fUphill * fUphill / (fForward * fForward + fStrafe * fStrafe);
            fStrafe = fKeep * fStrafe;
            fForward = fKeep * fForward;
        }
    }

    float fChangeFront = fForward - fFrontSpeed;
    if (fChangeFront > fSlopeFront + fMaxChange)
    {
        fChangeFront = fSlopeFront + fMaxChange;
    }
    else if (fSlopeFront - fMaxChange > fChangeFront)
    {
        fChangeFront = fSlopeFront - fMaxChange;
    }
    float fChangeSide = fStrafe - fSideSpeed;
    if (fChangeSide > fSlopeSide + fMaxChange)
    {
        fChangeSide = fSlopeSide + fMaxChange;
    }
    else if (fSlopeSide - fMaxChange > fChangeSide)
    {
        fChangeSide = fSlopeSide - fMaxChange;
    }

    mVelocity.x += vFront.x * fChangeFront;
    mVelocity.z += vFront.z * fChangeFront;
    mVelocity.y += vFront.y * fChangeFront;
    mVelocity.x += vSide.x * fChangeSide;
    mVelocity.z += vSide.z * fChangeSide;
    mVelocity.y += vSide.y * fChangeSide;

    if (mHook)
    {
        mHook->MoveSoldier(this, fDeltaTime);
    }
    else if (!(mFlags & flags_isGrounded))
    {
        mVelocity.y += fDeltaTime * -GRAVITY_FORCE;
    }

    const PblVector3& vPos = mMatrix.GetTranslation();
    SetPosition(PblVector3(vPos.x + mVelocity.x * fDeltaTime, vPos.y + mVelocity.y * fDeltaTime,
                           vPos.z + mVelocity.z * fDeltaTime));

    float fAngle = fDeltaTime * fTurn;
    if (fAngle != 0.0f)
    {
        float fSin;
        float fCos;
        PblMath::SinCos(&fSin, &fCos, fAngle);
        PblMatrix mRotation;
        mRotation.m[0][0] = fCos;
        mRotation.m[0][1] = 0.0f;
        mRotation.m[0][2] = -fSin;
        mRotation.m[0][3] = 0.0f;
        mRotation.m[1][0] = 0.0f;
        mRotation.m[1][1] = 1.0f;
        mRotation.m[1][2] = 0.0f;
        mRotation.m[1][3] = 0.0f;
        mRotation.m[2][0] = fSin;
        mRotation.m[2][1] = 0.0f;
        mRotation.m[2][2] = fCos;
        mRotation.m[2][3] = 0.0f;
        mRotation.m[3][0] = 0.0f;
        mRotation.m[3][1] = 0.0f;
        mRotation.m[3][2] = 0.0f;
        mRotation.m[3][3] = 1.0f;
        PblMatrix mNew;
        D3DXMatrixMultiply(&mNew, &mRotation, &mMatrix);
        SetMatrix(mNew);
    }
    else
    {
        SetMatrix(mMatrix);
    }
}

// FUNCTION: SWBF2 0x004f3590
int EntitySoldier::GetFirstPersonIndex()
{
    if (mPlayerId >= 0 && mTracker && mTracker->IsFirstPersonView())
    {
        return GetJoystickIndex();
    }
    return -1;
}

// FUNCTION: SWBF2 0x004f3610
int EntitySoldier::GetJoystickIndex()
{
    if (mPlayerId < 0)
    {
        return -1;
    }
    if (NetGame::IsEnabled() && !NetGame::IsClient() &&
        !(NetGame::IsHost() && NetGame::IsOneWorld() && !NetGame::IsDedicated()))
    {
        return -1;
    }
    return NetGame::GetJoystickIndex(mPlayerId);
}

// FUNCTION: SWBF2 0x004f36e0
void EntitySoldier::SetFirstPersonView(int iView, bool bFirstPerson)
{
    if (bFirstPerson)
    {
        TurnOffJetEffect();
        TurnOffJetIdleEffect();
        if (NetGame::GetJoystickIndex(mPlayerId) >= 0 && mSoldierAnimator)
        {
            if (mSoldierAnimator->mOwner == this)
            {
                mSoldierAnimator->mOwner = 0;
            }
            mSoldierAnimator = 0;
        }
    }
    Trackable::SetFirstPersonView(iView, bFirstPerson);
}

// FUNCTION: SWBF2 0x004f3750
void EntitySoldier::BuffOffense(float fTimer, float fMult)
{
    if (mClass->mHeroFlag || mClass->m_bIsAcklay)
    {
        mBuffOffenseTimer = 0.0f;
    }
    else if (fTimer > mBuffOffenseTimer)
    {
        mBuffOffenseTimer = fTimer;
        mBuffOffenseMult = fMult;
    }
}

// FUNCTION: SWBF2 0x004f3790
void EntitySoldier::BuffDefense(float fTimer, float fMult)
{
    if (mClass->mHeroFlag || mClass->m_bIsAcklay)
    {
        mBuffDefenseTimer = 0.0f;
    }
    else if (fTimer > mBuffDefenseTimer)
    {
        mBuffDefenseTimer = fTimer;
        mBuffDefenseMult = fMult;
    }
}

// FUNCTION: SWBF2 0x004f37d0
void EntitySoldier::BuffHealth(float fTimer, float fRate)
{
    if (mClass->mHeroFlag || mClass->m_bIsAcklay)
    {
        mBuffHealthTimer = 0.0f;
    }
    else if (fTimer > mBuffHealthTimer)
    {
        mBuffHealthTimer = fTimer;
        mBuffHealthRate = fRate;
    }
}

// FUNCTION: SWBF2 0x004f38e0
void EntitySoldier::UpdateBuffTimers(float fDeltaTime)
{
    if (!IsAlive())
    {
        TurnOffEffects();
        mBuffOffenseTimer = 0.0f;
        mBuffDefenseTimer = 0.0f;
        mBuffHealthTimer = 0.0f;
        mDebuffDamageTimer = 0.0f;
        return;
    }

    if (mClass->mHeroFlag || mClass->m_bIsAcklay)
    {
        mBuffOffenseTimer = 0.0f;
        mBuffDefenseTimer = 0.0f;
        mBuffHealthTimer = 0.0f;
        mDebuffDamageTimer = 0.0f;
    }
    else if (mCharacter && mCharacter->mMedalsMgr)
    {
        MedalsMgr* pMedals = mCharacter->mMedalsMgr;
        if (pMedals->IsAwardAvailable(8))
        {
            mBuffOffenseTimer = 340.0f;
            mBuffOffenseMult = 1.5f;
        }
        if (pMedals->IsAwardAvailable(7))
        {
            mBuffDefenseTimer = 340.0f;
            mBuffDefenseMult = 0.5f;
        }
    }

    if (mBuffOffenseTimer <= 300.0f)
    {
        mBuffOffenseTimer -= fDeltaTime;
    }
    if (mBuffOffenseTimer < 0.0f)
    {
        mBuffOffenseTimer = 0.0f;
    }
    TriggerBuffOffenseEffect();

    if (mBuffDefenseTimer <= 300.0f)
    {
        mBuffDefenseTimer -= fDeltaTime;
    }
    if (mBuffDefenseTimer < 0.0f)
    {
        mBuffDefenseTimer = 0.0f;
    }
    TriggerBuffDefenseEffect();

    if (mBuffHealthTimer <= 300.0f)
    {
        mBuffHealthTimer -= fDeltaTime;
    }
    if (mBuffHealthTimer < 0.0f)
    {
        mBuffHealthTimer = 0.0f;
    }
    else if (mBuffHealthTimer > 0.0f)
    {
        float fHealth = mBuffHealthRate * fDeltaTime + mCurHealth;
        SetCurHealth(fHealth > mMaxHealth ? mMaxHealth : fHealth, true, NULL);
    }
    TriggerBuffHealthEffect();

    if (mDebuffDamageTimer <= 300.0f)
    {
        mDebuffDamageTimer -= fDeltaTime;
    }
    if (mDebuffDamageTimer < 0.0f)
    {
        mDebuffDamageTimer = 0.0f;
    }
    else if (mDebuffDamageTimer > 0.0f && mDebuffDamageRate != 0.0f)
    {
        Damageable::ApplyDamage(DamageDesc(mDebuffDamageOwner, mDebuffDamageRate * fDeltaTime), 0xffffffff);
    }
    TriggerDebuffEffect();
}

// FUNCTION: SWBF2 0x004f3bb0
void EntitySoldier::SetAlert(bool bAlert)
{
    mAlertTimer = bAlert ? 2.5f : 0.0f;
}

// FUNCTION: SWBF2 0x004f3be0
void EntitySoldier::SetAlert(float fTime)
{
    if (fTime > 0.0f)
    {
        mAlertTimer = fTime;
    }
    else
    {
        mAlertTimer = 0.0f;
    }
}

// FUNCTION: SWBF2 0x004f3c00
void EntitySoldier::SetCollisionStance(int iStance)
{
    if (mAcklayData)
    {
        return;
    }

    float fScale = mClass->mCollisionRootScale;
    PblMatrix mHead = _IdentityMatrix;
    PblMatrix mTorso = _IdentityMatrix;
    CollisionModel* pModel;
    switch (iStance)
    {
    case 1:
        mHead.m[3][0] = 0.0f;
        mHead.m[3][1] = fScale * 1.2f;
        mHead.m[3][2] = 0.25f;
        mHead.m[3][3] = 1.0f;
        mTorso.m[3][0] = 0.0f;
        mTorso.m[3][1] = fScale * 0.7f;
        mTorso.m[3][2] = 0.0f;
        mTorso.m[3][3] = 1.0f;
        pModel = &mClass->mCollisionModels[1];
        break;
    case 2:
        mHead.m[3][0] = 0.0f;
        mHead.m[3][1] = fScale * 0.28f;
        mHead.m[3][2] = fScale * 0.75f;
        mHead.m[3][3] = 1.0f;
        mTorso.m[3][0] = 0.0f;
        mTorso.m[3][1] = 0.0f;
        mTorso.m[3][2] = 0.0f;
        mTorso.m[3][3] = 1.0f;
        pModel = &mClass->mCollisionModels[2];
        break;
    default:
        mHead.m[3][0] = 0.0f;
        mHead.m[3][1] = fScale * 1.65f;
        mHead.m[3][2] = 0.16f;
        mHead.m[3][3] = 1.0f;
        mTorso.m[3][0] = 0.0f;
        mTorso.m[3][1] = fScale * 1.15f;
        mTorso.m[3][2] = 0.0f;
        mTorso.m[3][3] = 1.0f;
        pModel = &mClass->mCollisionModels[0];
        break;
    }
    mCollisionBodyHead.SetMatrix(mHead);
    mCollisionBodyTorso.SetMatrix(mTorso);
    for (int i = 2; i < mDynamicCollisionModel.GetNumBodies(); ++i)
    {
        mDynamicCollisionModel.SetBody(i, pModel->GetBody(i));
    }
    SetCollisionModel(&mDynamicCollisionModel);
}

// FUNCTION: SWBF2 0x004f3db0
void EntitySoldier::UpdateOrdnanceCollision(float fDeltaTime)
{
    Weapon* pWeapon = mActiveIndex >= 0 ? mWeapon[mActiveIndex] : NULL;
    Weapon* pAnimWeapon = pWeapon;
    m_eSoldierAnimationMap = SoldierAnimationBank::HUMAN_RIFLE;
    if (pWeapon)
    {
        bool bOffhand = false;
        if (pWeapon->mClass->mIsOffhand)
        {
            if (mActiveChannel == 1 && mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]])
            {
                pAnimWeapon = mWeapon[mWeaponIndex[0]];
                bOffhand = true;
            }
            else if (mActiveIndex != 0 && mWeapon[0])
            {
                pAnimWeapon = mWeapon[0];
                bOffhand = true;
            }
        }
        else if (pWeapon->IsMeleeThrow() && mActiveChannel == 1 && mWeaponIndex[0] >= 0 &&
                 mWeapon[mWeaponIndex[0]])
        {
            pAnimWeapon = mWeapon[mWeaponIndex[0]];
        }

        if (pAnimWeapon->mSoldierAnimationMap != SoldierAnimationBank::INVALID_MAP)
        {
            m_eSoldierAnimationMap = (SoldierAnimationBank::MAP)pAnimWeapon->mSoldierAnimationMap;
        }

        if (pAnimWeapon->IsMelee())
        {
            WeaponMelee* pMelee = static_cast<WeaponMelee*>(pAnimWeapon);
            if (pMelee->IsAnimationVelocityAligned())
            {
                SetAlert(true);
            }
            else
            {
                const SoldierAnimation* pAnim = SoldierAnimatorClass::Instance()->GetUpperBodyAnimation(
                    (SoldierAnimationBank::MAP)pAnimWeapon->mSoldierAnimationMap,
                    pMelee->GetWeaponMeleeAnimation());
                SetAlert(pAnim && pAnim->m_pAnimData->m_uiAimType == SoldierAnimationData::AIM_TORSO);
            }
        }
        else if (!SoldierAnimationBank::HasAlertState(
                     (SoldierAnimationBank::WEAPON)pAnimWeapon->GetSoldierAnimationWeapon()))
        {
            SetAlert(false);
        }
        else if (mTargetInfo.mIsAiming || pAnimWeapon->mTarget != NULL ||
                 (pWeapon->mState >= Weapon::FIRE && pWeapon->mState <= Weapon::FIRE2) ||
                 (!bOffhand && pWeapon->mState == Weapon::CHARGE))
        {
            SetAlert(true);
        }
        else if (mTracker && mTracker->IsFirstPersonView() && NetGame::GetJoystickIndex(mPlayerId) >= 0)
        {
            SetAlert(0.2f);
        }
    }

    if (mAlertTimer > 0.0f)
    {
        if (mAlertTimer > fDeltaTime)
        {
            mAlertTimer -= fDeltaTime;
        }
        else
        {
            mAlertTimer = 0.0f;
        }
    }

    const PblVector3& vPos = GetPosition();
    float fDX = vPos.x - m_vLastPosition.x;
    float fDY = vPos.y - m_vLastPosition.y;
    float fDZ = vPos.z - m_vLastPosition.z;
    float fMinMove = mClass->mSkeletonRootScale * fDeltaTime * (mMoveAnim == MOVEANIM_IDLE ? 0.3f : 0.2f);
    int iMove = 0;
    m_vLastPosition = GetPosition();
    float fDistSq = fDX * fDX + fDZ * fDZ;
    if (mState >= SoldierState::SPRINT || fMinMove * fMinMove > fDistSq)
    {
        mLegAngleForward = 1;
        mMoveAnim = MOVEANIM_IDLE;
        mLegAngle = 0.0f;
    }
    else
    {
        iMove = 1;
        const PblVector3& vRight = mMatrix.GetXAxis();
        const PblVector3& vFront = mMatrix.GetZAxis();
        float fAngle = (float)atan2(vRight.x * fDX + vRight.y * fDY + vRight.z * fDZ,
                                    vFront.x * fDX + vFront.y * fDY + vFront.z * fDZ);
        if (mAlertTimer > 0.0f)
        {
            if (!pAnimWeapon->IsMelee())
            {
                static const float sForwardMax = PblMath::fHalfPi * 0.75f;
                static const float sForwardMin = PblMath::fHalfPi * -1.15f;
                static const float sLegMax = PblMath::fHalfPi * 0.35f;
                static const float sLegMin = PblMath::fHalfPi * -0.5f;
                if (fAngle < sForwardMax && sForwardMin < fAngle)
                {
                    mLegAngleForward = 1;
                }
                else
                {
                    fAngle = UnitCircleNormalize(fAngle + PblMath::fPi);
                    mLegAngleForward = 0;
                    iMove = 2;
                }
                if (sLegMin > fAngle)
                {
                    fAngle = sLegMin;
                }
                else if (fAngle > sLegMax)
                {
                    fAngle = sLegMax;
                }
            }
            else if (mAimer.GetDirection().z * fDZ + fDX * mAimer.GetDirection().x < 0.0f)
            {
                fAngle = UnitCircleNormalize(fAngle + PblMath::fPi);
                mLegAngleForward = 0;
                iMove = 2;
            }
            else
            {
                mLegAngleForward = 1;
            }
            mLegAngle = fAngle;
        }
        else
        {
            static const float sLegMax = PblMath::fHalfPi * 0.8f;
            static const float sLegMin = -sLegMax;
            static const float sForwardLimit = PblMath::fHalfPi * 1.2f;
            static const float sBackwardLimit = PblMath::fHalfPi * 0.8f;
            float fLimit = mLegAngleForward ? sForwardLimit : sBackwardLimit;
            if (fAngle < fLimit && -fLimit < fAngle)
            {
                mLegAngleForward = 1;
            }
            else
            {
                fAngle += PblMath::fPi;
                while (fAngle > PblMath::fPi)
                {
                    fAngle -= PblMath::fTwoPi;
                }
                while (-PblMath::fPi > fAngle)
                {
                    fAngle += PblMath::fTwoPi;
                }
                mLegAngleForward = 0;
                iMove = 2;
            }
            if (sLegMin > fAngle)
            {
                mLegAngle = sLegMin;
            }
            else if (fAngle > sLegMax)
            {
                mLegAngle = sLegMax;
            }
            else
            {
                mLegAngle = fAngle;
            }
        }

        float fRun = mClass->mMaxSpeed * fDeltaTime * (mMoveAnim == MOVEANIM_RUN ? 0.71f : 0.78f);
        if (fDistSq > fRun * fRun)
        {
            if (mLegAngleForward)
            {
                iMove = 3;
            }
            mMoveAnim = MOVEANIM_RUN;
        }
        else
        {
            mMoveAnim = MOVEANIM_WALK;
        }
    }

    const SoldierOrdnanceCollisionTable& table =
        SoldierAnimatorClass::Instance()->m_aOrdCollTable[m_eSoldierAnimationMap];
    const PblVector3& vHeadOffset = mClass->m_vCollisionHeadOffset;
    const PblVector3& vTorsoOffset = mClass->m_vCollisionTorsoOffset;
    float fScale = mClass->mCollisionRootScale;
    PblMatrix mBody = _IdentityMatrix;
    int iCrouch = 0;
    switch (mState)
    {
    case SoldierState::CROUCH:
        iCrouch = 8;

    case SoldierState::STAND:
    case SoldierState::SPRINT:
        if (mAlertTimer > 0.0f && mAimer.GetDirection().y != 0.0f)
        {

            int i = iMove + 4 + iCrouch;
            const PblVector3& vAim = mAimer.GetDirection();
            const PblVector3& vFront = mMatrix.GetZAxis();
            float fPitch =
                (float)atan2(vAim.y, vAim.y * vFront.y + vAim.x * vFront.x + vAim.z * vFront.z);
            float fSin;
            float fCos;
            PblMath::SinCos(&fSin, &fCos, fPitch);
            PblMatrix mPitch;
            mPitch.m[0][0] = gVectorUnitX.x;
            mPitch.m[0][1] = gVectorUnitX.y;
            mPitch.m[0][2] = gVectorUnitX.z;
            mPitch.m[0][3] = 0.0f;
            mPitch.m[1][0] = 0.0f;
            mPitch.m[1][1] = fCos;
            mPitch.m[1][2] = -fSin;
            mPitch.m[1][3] = 0.0f;
            mPitch.m[2][0] = 0.0f;
            mPitch.m[2][1] = fSin;
            mPitch.m[2][2] = fCos;
            mPitch.m[2][3] = 0.0f;
            mPitch.m[3][0] = gVectorZero.x;
            mPitch.m[3][1] = gVectorZero.y;
            mPitch.m[3][2] = gVectorZero.z;
            mPitch.m[3][3] = 1.0f;
            PblVector3 vPivot((table.avPitchAxis[i].x + vTorsoOffset.x) * fScale,
                              (table.avPitchAxis[i].y + vTorsoOffset.y) * fScale,
                              (table.avPitchAxis[i].z + vTorsoOffset.z) * fScale);
            PblVector3 vTurned;
            D3DXVec3TransformNormal(&vTurned, &vPivot, &mPitch);
            mPitch.m[3][0] = vPivot.x - vTurned.x;
            mPitch.m[3][1] = vPivot.y - vTurned.y;
            mPitch.m[3][2] = vPivot.z - vTurned.z;
            mPitch.m[3][3] = 1.0f;

            PblVector3 vBody((vHeadOffset.x + table.avHeadPos[i].x) * fScale,
                             (vHeadOffset.y + table.avHeadPos[i].y) * fScale,
                             (vHeadOffset.z + table.avHeadPos[i].z) * fScale);
            PblVector3 vOut;
            D3DXVec3TransformCoord(&vOut, &vBody, &mPitch);
            mBody.m[3][0] = vOut.x;
            mBody.m[3][1] = vOut.y;
            mBody.m[3][2] = vOut.z;
            mBody.m[3][3] = 1.0f;
            mCollisionBodyHead.SetMatrix(mBody);
            vBody = PblVector3((vTorsoOffset.x + table.avTorsoPos[i].x) * fScale,
                               (vTorsoOffset.y + table.avTorsoPos[i].y) * fScale,
                               (vTorsoOffset.z + table.avTorsoPos[i].z) * fScale);
            D3DXVec3TransformCoord(&vOut, &vBody, &mPitch);
            mBody.m[3][0] = vOut.x;
            mBody.m[3][1] = vOut.y;
            mBody.m[3][2] = vOut.z;
            mBody.m[3][3] = 1.0f;
            mCollisionBodyTorso.SetMatrix(mBody);
        }
        else if (mLegAngle != 0.0f)
        {

            static const float sYawMin = PblMath::fHalfPi * -0.8f;
            static const float sYawMax = PblMath::fHalfPi * 0.8f;
            int i = iMove + iCrouch;
            float fYaw = mLegAngle;
            if (sYawMin > fYaw)
            {
                fYaw = sYawMin;
            }
            else if (fYaw > sYawMax)
            {
                fYaw = sYawMax;
            }
            float fSin;
            float fCos;
            PblMath::SinCos(&fSin, &fCos, fYaw);
            PblMatrix mYaw;
            mYaw.m[0][0] = fCos;
            mYaw.m[0][1] = 0.0f;
            mYaw.m[0][2] = -fSin;
            mYaw.m[0][3] = 0.0f;
            mYaw.m[1][0] = gVectorUnitY.x;
            mYaw.m[1][1] = gVectorUnitY.y;
            mYaw.m[1][2] = gVectorUnitY.z;
            mYaw.m[1][3] = 0.0f;
            mYaw.m[2][0] = fSin;
            mYaw.m[2][1] = 0.0f;
            mYaw.m[2][2] = fCos;
            mYaw.m[2][3] = 0.0f;
            mYaw.m[3][0] = gVectorZero.x;
            mYaw.m[3][1] = gVectorZero.y;
            mYaw.m[3][2] = gVectorZero.z;
            mYaw.m[3][3] = 1.0f;

            PblVector3 vBody((table.avHeadPos[i].x + vHeadOffset.x) * fScale,
                             (table.avHeadPos[i].y + vHeadOffset.y) * fScale,
                             (table.avHeadPos[i].z + vHeadOffset.z) * fScale);
            PblVector3 vOut;
            D3DXVec3TransformNormal(&vOut, &vBody, &mYaw);
            mBody.m[3][0] = vOut.x;
            mBody.m[3][1] = vOut.y;
            mBody.m[3][2] = vOut.z;
            mBody.m[3][3] = 1.0f;
            mCollisionBodyHead.SetMatrix(mBody);
            vBody = PblVector3((vTorsoOffset.x + table.avTorsoPos[i].x) * fScale,
                               (vTorsoOffset.y + table.avTorsoPos[i].y) * fScale,
                               (vTorsoOffset.z + table.avTorsoPos[i].z) * fScale);
            D3DXVec3TransformNormal(&vOut, &vBody, &mYaw);
            mBody.m[3][0] = vOut.x;
            mBody.m[3][1] = vOut.y;
            mBody.m[3][2] = vOut.z;
            mBody.m[3][3] = 1.0f;
            mCollisionBodyTorso.SetMatrix(mBody);
        }
        else
        {
            int i = iMove + iCrouch;
            mBody.m[3][0] = (vHeadOffset.x + table.avHeadPos[i].x) * fScale;
            mBody.m[3][1] = (vHeadOffset.y + table.avHeadPos[i].y) * fScale;
            mBody.m[3][2] = (vHeadOffset.z + table.avHeadPos[i].z) * fScale;
            mBody.m[3][3] = 1.0f;
            mCollisionBodyHead.SetMatrix(mBody);
            mBody.m[3][0] = (vTorsoOffset.x + table.avTorsoPos[i].x) * fScale;
            mBody.m[3][1] = (vTorsoOffset.y + table.avTorsoPos[i].y) * fScale;
            mBody.m[3][2] = (vTorsoOffset.z + table.avTorsoPos[i].z) * fScale;
            mBody.m[3][3] = 1.0f;
            mCollisionBodyTorso.SetMatrix(mBody);
        }
        break;

    case SoldierState::JUMP:
    case SoldierState::JET_JUMP:
    case SoldierState::JET_HOVER:
    case SoldierState::FALL:
    {

        const PblVector3& vOrigin = mMatrix.GetTranslation();
        SetCollisionCenter(PblVector3(vOrigin.x + table.avPitchAxis[16].x * fScale,
                                      vOrigin.y + table.avPitchAxis[16].y * fScale,
                                      vOrigin.z + table.avPitchAxis[16].z * fScale));
        mBody.m[3][0] = (vHeadOffset.x + table.avHeadPos[16].x) * fScale;
        mBody.m[3][1] = (vHeadOffset.y + table.avHeadPos[16].y) * fScale;
        mBody.m[3][2] = (vHeadOffset.z + table.avHeadPos[16].z) * fScale;
        mBody.m[3][3] = 1.0f;
        mCollisionBodyHead.SetMatrix(mBody);
        mBody.m[3][0] = (vTorsoOffset.x + table.avTorsoPos[16].x) * fScale;
        mBody.m[3][1] = (vTorsoOffset.y + table.avTorsoPos[16].y) * fScale;
        mBody.m[3][2] = (vTorsoOffset.z + table.avTorsoPos[16].z) * fScale;
        mBody.m[3][3] = 1.0f;
        mCollisionBodyTorso.SetMatrix(mBody);
        break;
    }

    case SoldierState::ROLL:
    case SoldierState::TUMBLE:
    case SoldierState::BOUNCE:
    case SoldierState::TUMBLE_RECOVER:
        mBody.m[3][0] = (table.avTorsoPos[17].x + vTorsoOffset.x) * fScale;
        mBody.m[3][1] = (table.avTorsoPos[17].y + vTorsoOffset.y) * fScale;
        mBody.m[3][2] = (table.avTorsoPos[17].z + vTorsoOffset.z) * fScale;
        mBody.m[3][3] = 1.0f;
        mCollisionBodyHead.SetMatrix(mBody);
        mCollisionBodyTorso.SetMatrix(mBody);
        break;

    case SoldierState::FLY:
    case SoldierState::FLY_RECOVER:
        mBody.m[3][0] = (vTorsoOffset.x + table.avTorsoPos[18].x) * fScale;
        mBody.m[3][1] = (vTorsoOffset.y + table.avTorsoPos[18].y) * fScale;
        mBody.m[3][2] = (vTorsoOffset.z + table.avTorsoPos[18].z) * fScale;
        mBody.m[3][3] = 1.0f;
        mCollisionBodyHead.SetMatrix(mBody);
        mCollisionBodyTorso.SetMatrix(mBody);
        break;

    case SoldierState::CHOKE:
    case SoldierState::PILOT_ANIMATION:
    case SoldierState::ARMED_PASSENGER:
        if (m_ePilotAnimation != SoldierAnimationBank::INVALID_ANIMATION)
        {
            int i = 19 + m_ePilotAnimation;
            mBody.m[3][0] = (vHeadOffset.x + table.avHeadPos[i].x) * fScale;
            mBody.m[3][1] = (vHeadOffset.y + table.avHeadPos[i].y) * fScale;
            mBody.m[3][2] = (vHeadOffset.z + table.avHeadPos[i].z) * fScale;
            mBody.m[3][3] = 1.0f;
            mCollisionBodyHead.SetMatrix(mBody);
            mBody.m[3][0] = (vTorsoOffset.x + table.avTorsoPos[i].x) * fScale;
            mBody.m[3][1] = (vTorsoOffset.y + table.avTorsoPos[i].y) * fScale;
            mBody.m[3][2] = (vTorsoOffset.z + table.avTorsoPos[i].z) * fScale;
            mBody.m[3][3] = 1.0f;
            mCollisionBodyTorso.SetMatrix(mBody);
        }
        break;

    default:
        break;
    }
}

// SYNTHETIC: SWBF2 0x004fab5c
// [thunk]:EntitySoldier::GetEntity`adjustor{12}'

// SYNTHETIC: SWBF2 0x004fab82
// [thunk]:EntitySoldier::GetGameObject`adjustor{600}'

// SYNTHETIC: SWBF2 0x004fab8d
// [thunk]:EntitySoldier::GetGameObject`adjustor{600}'

// SYNTHETIC: SWBF2 0x004fabbe
// [thunk]:EntitySoldier::GetControllable`adjustor{600}'

// SYNTHETIC: SWBF2 0x004fabc9
// [thunk]:EntitySoldier::GetControllable`adjustor{600}'

// SYNTHETIC: SWBF2 0x004fab98
// [thunk]:EntitySoldier::GetGameObject`adjustor{12}'

// SYNTHETIC: SWBF2 0x004faba8
// [thunk]:EntitySoldier::GetEntity`adjustor{576}'

// SYNTHETIC: SWBF2 0x004fabb3
// [thunk]:EntitySoldier::GetEntity`adjustor{576}'

// SYNTHETIC: SWBF2 0x004fac1e
// [thunk]:EntitySoldier::GetMatrix`adjustor{564}'

class PowerupItem
{
public:
    static MemoryPool sMemoryPool;
};

namespace AIDebug { void StartBulletTime(float f0, float f1, float f2); }

// SYNTHETIC: SWBF2 0x004de820
// EntitySoldier::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004fab51
// [thunk]:EntitySoldier::`scalar deleting destructor' adjustor{148}
// SYNTHETIC: SWBF2 0x004fabdf
// [thunk]:EntitySoldier::`scalar deleting destructor' adjustor{320}
// SYNTHETIC: SWBF2 0x004fabea
// [thunk]:EntitySoldier::`scalar deleting destructor' adjustor{972}
// SYNTHETIC: SWBF2 0x004fabf5
// [thunk]:EntitySoldier::`scalar deleting destructor' adjustor{600}
// SYNTHETIC: SWBF2 0x004fac00
// [thunk]:EntitySoldier::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x004fac08
// [thunk]:EntitySoldier::`scalar deleting destructor' adjustor{512}

// FUNCTION: SWBF2 0x004dfa20
EntitySoldier::~EntitySoldier()
{
    if ((!NetGame::IsEnabled() || NetGame::IsClient()) && mInitialPlayerId >= 0 && mTracker &&
        mTracker->IsFirstPersonView())
    {
        int iJoystick = NetGame::GetJoystickIndex(mInitialPlayerId);
        if (iJoystick >= 0)
        {
            FirstPerson::Activate(iJoystick, false);
            FirstPerson::Update(iJoystick, TYPE_SOLDIER, -2, 0, 0);
        }
    }

    NetGame::Destroyed(this);
    mControllableList.Remove(&this->Controllable::mNode);
    TargetManager::RemoveTarget(this);

    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        delete mWeapon[i];
        mWeapon[i] = 0;
    }

    TurnOffEffects();
    mLowHealthSound.Stop(true);
    mChokeSound.Stop(true);
    mAmbientSound.Stop(true);
    mLongFoleySound.Stop(true);

    for (unsigned int i = 0; i < mModel->mClothCount; ++i)
    {
        delete mClothPiece[i];
    }
    if (mModelLowRes)
    {
        for (unsigned int i = 0; i < mModelLowRes->mClothCount; ++i)
        {
            delete mClothPiece[mModel->mClothCount + i];
        }
    }

    for (int i = 0; i < mDynamicCollisionModel.GetNumBodies(); ++i)
    {
        mDynamicCollisionModel.SetBody(i, 0);
    }

    if (mSoldierAnimator && mSoldierAnimator->mOwner == this)
    {
        mSoldierAnimator->mOwner = 0;
    }
    if (mSoldierAnimatorLowRes)
    {
        mSoldierAnimatorLowRes->Release(*this);
    }

    delete mTentacles;
    delete mAcklayData;
    mSoldierMusic.Stop();
    mAISoldierMusic.Stop();

    reinterpret_cast<EnergyBar*>(mEnergyBar)->~EnergyBar();
}

// FUNCTION: SWBF2 0x004dfee0
void EntitySoldier::TriggerHealthGainEffect()
{
    if (mClass->mHealthGainEffect && mHealthGainEffectHandle == 0)
    {
        if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mHealthGainEffect))
        {
            if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
            {
                pEffect->AttachEffectToObject(this, true);
                mHealthGainEffectHandle = pEffect;
            }
        }
    }
}

// FUNCTION: SWBF2 0x004dff70
void EntitySoldier::TriggerAmmoGainEffect()
{
    if (mClass->mAmmoGainEffect && mAmmoGainEffectHandle == 0)
    {
        if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mAmmoGainEffect))
        {
            if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
            {
                pEffect->AttachEffectToObject(this, true);
                mAmmoGainEffectHandle = pEffect;
            }
        }
    }
}

// FUNCTION: SWBF2 0x004e0000
void EntitySoldier::TriggerEnergyGainEffect()
{
    if (mClass->mEnergyGainEffect && mEnergyGainEffectHandle == 0)
    {
        if (FLEffectClass* pEffectClass = FLEffect::Get(mClass->mEnergyGainEffect))
        {
            if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
            {
                pEffect->AttachEffectToObject(this, true);
                mEnergyGainEffectHandle = pEffect;
            }
        }
    }
}

// FUNCTION: SWBF2 0x004e0590
void EntitySoldier::ActivateSmolder(float fTimer, bool bVanishDeath, float fDamageRate, const DamageOwner& owner)
{
    if (fTimer > mSmolderTimer)
    {
        mSmolderTimer = fTimer;
    }
    mSmolderVanishDeath = bVanishDeath;
    for (int i = 0; i < 5; ++i)
    {
        if (mSmolderEffectHandle[i])
        {
            mSmolderEffectHandle[i]->StopAndFinish();
            mSmolderEffectHandle[i] = NULL;
        }
    }
    mSmolderDamageRate = fDamageRate;
    mSmolderDamageOwner = owner;
}

// FUNCTION: SWBF2 0x004e0650
void EntitySoldier::SetupSmolderEffect(int iIndex, unsigned int uiBone, unsigned int uiEffect)
{
    mSmolderEffectBone[iIndex] = uiBone;
    mSmolderEffect[iIndex] = uiEffect;
}

// FUNCTION: SWBF2 0x004e0780
void EntitySoldier::TriggerSmolderEffect(int iIndex, unsigned int uiBone, unsigned int uiEffect)
{
    if (uiEffect && mSmolderEffectHandle[iIndex] == 0)
    {
        if (FLEffectClass* pEffectClass = FLEffect::Get(uiEffect))
        {
            if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
            {
                pEffect->AttachEffectToObject(this, true);
                mSmolderEffectHandle[iIndex] = pEffect;
            }
        }
    }
}

// FUNCTION: SWBF2 0x004e0b10
void EntitySoldier::SetInvisibilityFactor(float fFactor)
{
    unsigned char uiOldAlpha = m_uiInvisibilityAlpha;
    m_uiInvisibilityAlpha = (unsigned char)(fFactor * 255.0f);
    UpdateInvisibility(uiOldAlpha);
}

namespace AI
{

enum AIAffil
{
    AFFIL_ENEMY = 1,
    AFFIL_NEUTRAL = 2,
    AFFIL_ALLY = 4,
    AFFIL_ALLYNEUTRAL = 6,
    AFFIL_ALL = 7
};
}

namespace ControllerManager
{

void BroadcastEvent(AI::EventInfo* pInfo, const PblVector3& vPos, float fRadius,
                    const UnitController* pSender, const Team* pTeam, AI::AIAffil eAffil);
}

// FUNCTION: SWBF2 0x004e0b40
void EntitySoldier::UpdateInvisibility(unsigned char uiOldAlpha)
{
    if (uiOldAlpha >= 245 && m_uiInvisibilityAlpha < 245)
    {
        AI::EventInfo info(AI::EVT_EndInvisibility, this);
        ControllerManager::BroadcastEvent(&info, GetPosition(), 20.0f, UnitControllerPtr(mCtrl),
                                          Team::sTeams[mTeam], AI::AFFIL_ENEMY);
    }
}

// FUNCTION: SWBF2 0x004e0800
void EntitySoldier::TurnOffEffects()
{
    TurnOffJetEffect();
    TurnOffJetIdleEffect();
    if (mHealthGainEffectHandle)
    {
        mHealthGainEffectHandle->StopAndFinish();
        mHealthGainEffectHandle = NULL;
    }
    if (mAmmoGainEffectHandle)
    {
        mAmmoGainEffectHandle->StopAndFinish();
        mAmmoGainEffectHandle = NULL;
    }
    if (mEnergyGainEffectHandle)
    {
        mEnergyGainEffectHandle->StopAndFinish();
        mEnergyGainEffectHandle = NULL;
    }
    if (mBuffHealthEffectHandle)
    {
        mBuffHealthEffectHandle->StopAndFinish();
        mBuffHealthEffectHandle = NULL;
    }
    if (mBuffOffenseEffectHandle)
    {
        mBuffOffenseEffectHandle->StopAndFinish();
        mBuffOffenseEffectHandle = NULL;
    }
    if (mBuffDefenseEffectHandle)
    {
        mBuffDefenseEffectHandle->StopAndFinish();
        mBuffDefenseEffectHandle = NULL;
    }
    if (mDebuffEffectHandle)
    {
        mDebuffEffectHandle->StopAndFinish();
        mDebuffEffectHandle = NULL;
    }
    if (mInvincibleEffectHandle)
    {
        mInvincibleEffectHandle->StopAndFinish();
        mInvincibleEffectHandle = NULL;
    }
    for (int i = 0; i < 5; ++i)
    {
        if (mSmolderEffectHandle[i])
        {
            mSmolderEffectHandle[i]->StopAndFinish();
            mSmolderEffectHandle[i] = NULL;
        }
    }
}

// FUNCTION: SWBF2 0x004e0cb0
void EntitySoldier::HealthChangeCallback(bool bArg, float fArg)
{
    if (fArg < 0.0f && mBuffHealthTimer <= 0.0f)
    {
        TriggerHealthGainEffect();
    }
    Damageable::HealthChangeCallback(bArg, fArg);
}

// FUNCTION: SWBF2 0x004e0cf0
void EntitySoldier::Kill()
{
    mIsAlive = 0;
    for (int i = 0; i < 2; ++i)
    {
        if (mWeaponIndex[i] >= 0)
        {
            Weapon* pWeapon = mWeapon[mWeaponIndex[i]];
            if (pWeapon->IsSelected())
            {
                pWeapon->Deselect(i, false);
            }
        }
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()) && !mVanishing && mClass->mDropItemCount > 0)
    {
        float fRandom = PblRandom::GetSim().GetFloat();
        for (int i = 0; i < mClass->mDropItemCount; ++i)
        {
            fRandom -= mClass->mDropItemProbability[i];
            if (fRandom < 0.0f && !PowerupItem::sMemoryPool.IsFull())
            {
                EntityDesc desc;
                desc.mMatrix = NormalMatrix(mMatrix.GetZAxis(), gVectorUnitY, GetCollisionCenter());
                Entity* pEntity = mClass->mDropItemClass[i]->Build(desc);
                if (pEntity && pEntity->GetGameObject())
                {
                    const PblVector3& vVelocity = GetVelocity();
                    if (vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z > 9.0f)
                    {
                        pEntity->GetGameObject()->SetVelocity(GetVelocity());
                    }
                    else
                    {
                        GameObject* pDrop = pEntity->GetGameObject();
                        const PblVector3& vSide = GetMatrix().GetXAxis();
                        pDrop->SetVelocity(PblVector3(vSide.x * 3.0f, vSide.y * 3.0f, vSide.z * 3.0f));
                    }
                }
                break;
            }
        }
    }

    if (mPlayerId >= 0)
    {
        FLInputManager::Instance()->StopIntermittentRumble(mPlayerId);
        int iJoystick = GetJoystickIndex();
        if (iJoystick >= 0)
        {
            MotionBlurEffect::Stop(0, iJoystick);
            FirstPerson::Activate(iJoystick, false);
            FirstPerson::Update(iJoystick, TYPE_SOLDIER, -2, 0, 0);
        }
        if (!mVanishing)
        {
            Trackable::TrackDeath(mPlayerId, mClass->mHeroFlag ? 0 : mDamageOwner.mCharacter);
        }
    }

    if (GetObjectData(0) == COLL_ATTACHED)
    {
        DirectionKill(gVectorZero);
    }
    else if (!IsThreadActive() && mCharacter)
    {
        if (mCharacter->mRemote)
        {
            ExitRemote();
        }
        if (Controllable* pVehicle = mCharacter->mVehicle)
        {
            ExitVehicle(pVehicle->GetGameObject()->mIsAlive != 0, false);
        }
    }

    if (mState != SoldierState::FLY)
    {
        if (mClass->mHeroFlag || mClass->mFleeLikeAHero)
        {
            switch (mState)
            {
            case SoldierState::JUMP:
            case SoldierState::FALL:
            case SoldierState::FLY_RECOVER:
            case SoldierState::JET_JUMP:
            case SoldierState::JET_HOVER:
                SetState(SoldierState::FLY);
                break;
            default:
                SetState(SoldierState::HERO_FLEE);
                break;
            }
        }
        else
        {
            SetState(SoldierState::DEAD);
        }
    }

    GameObject::Kill();
    Controllable::Kill();
    VehicleEngine::DeactivateEngine();
    mSoldierMusic.Stop();
    mAISoldierMusic.Stop();
}

// FUNCTION: SWBF2 0x004e1690
void EntitySoldier::ApplyPush(const PblVector3& vPush, float fArg, bool bArg)
{
    if (mAcklayData)
    {
        return;
    }

    if (bArg)
    {
        mVelocity = PblVector3(vPush.x * fArg, vPush.y * fArg, vPush.z * fArg);
    }
    else
    {
        float fDot = vPush.x * mVelocity.x + vPush.y * mVelocity.y + vPush.z * mVelocity.z;
        if (fDot < 0.0f)
        {
            fDot *= 0.5f;
            mVelocity.x -= vPush.x * fDot;
            mVelocity.y -= vPush.y * fDot;
            mVelocity.z -= vPush.z * fDot;
        }
        mVelocity.x += vPush.x * fArg;
        mVelocity.y += fArg * vPush.y;
        mVelocity.z += fArg * vPush.z;
    }

    SoldierState::SoldierState eState = mState;
    if (eState == SoldierState::FLY)
    {
        return;
    }

    if (eState == SoldierState::JET_HOVER)
    {
        if (fArg > 8.0f)
        {
            if (GameLoop::GetNumCameras() == 1 && mPlayerId >= 0)
            {
                mSoundState |= 0x10;
            }
            Fly();
        }
        else if (fArg > 2.0f)
        {
            m_uiInputLockMask = INPUTLOCK_ALL;
            m_bSlide = 1;
            mInputLockTime = (fArg - 2.0f) * 0.5f / 6.0f;
        }
        return;
    }

    if (eState == SoldierState::JET_JUMP || eState == SoldierState::JUMP || eState == SoldierState::FALL)
    {
        if (fArg > 5.0f)
        {
            if (GameLoop::GetNumCameras() == 1 && mPlayerId >= 0)
            {
                mSoundState |= 0x10;
            }
            Fly();
        }
        else if (fArg > 1.0f)
        {
            m_uiInputLockMask = INPUTLOCK_ALL;
            m_bSlide = 1;
            mInputLockTime = 1.0f;
        }
        return;
    }

    if (mVelocity.x * mGroundNormal.x + mVelocity.y * mGroundNormal.y + mVelocity.z * mGroundNormal.z > 3.0f)
    {
        if (GameLoop::GetNumCameras() == 1 && mPlayerId >= 0)
        {
            mSoundState |= 0x10;
        }
        Fly();
        return;
    }
    if (mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z > 64.0f)
    {
        SetState(SoldierState::TUMBLE);
        return;
    }
    if (fArg > 2.0f)
    {
        if (eState == SoldierState::SPRINT)
        {
            EndSprint();
        }
        m_uiInputLockMask = INPUTLOCK_ALL;
        m_bSlide = 1;
        mInputLockTime = 0.5f;
    }
}

// FUNCTION: SWBF2 0x004e19b0
void EntitySoldier::ApplyRadiusPush(const PblVector3& vPos, float fInnerRadius, float fOuterRadius, float fPush,
                                    bool bArg)
{
    switch (mState)
    {
    case SoldierState::PILOT_ANIMATION:
    case SoldierState::ARMED_PASSENGER:
    case SoldierState::FLY:
    case SoldierState::TUMBLE:
    case SoldierState::BOUNCE:
        return;
    default:
        break;
    }

    if (mClass && mClass->mChunkDescriptors[0])
    {
        if (mClass->mChunkFrequency >= PblRandom::GetSim().GetFloat())
        {
            mFlags |= flags_forceChunkDeath;
            return;
        }
        if (mCurHealth < -4.0f)
        {
            SetCurHealth(-4.0f, true, NULL);
        }
    }

    const PblVector3& vCenter = GetCollisionCenter();
    PblVector3 vDiff(vCenter.x - vPos.x, vCenter.y - vPos.y, vCenter.z - vPos.z);
    float fDist = sqrtf(vDiff.x * vDiff.x + vDiff.y * vDiff.y + vDiff.z * vDiff.z);
    if (fDist > 0.0001f)
    {
        float fEffect = CalcRadiusEffect(fDist, fInnerRadius, fOuterRadius);
        if (fEffect > 0.0f)
        {
            PblVector3 vDir;
            D3DXVec3Normalize(&vDir, &vDiff);
            ApplyPush(PblVector3(vDir.x + 0.0f, vDir.y + 0.6f, vDir.z + 0.0f), fEffect * fPush, bArg);
        }
    }
}

// FUNCTION: SWBF2 0x004e1de0
void EntitySoldier::ApplyRadiusDamage(const PblVector3& vPos, float fInnerRadius, float fOuterRadius,
                                      const DamageDesc& damage)
{
    Controllable* pVehicle = mCharacter ? mCharacter->mVehicle : 0;
    if (pVehicle && pVehicle->GetGameObject()->GetCollisionObject()->GetCollisionType() != COLL_STATIC)
    {
        return;
    }

    GameObject::ApplyRadiusDamage(vPos, fInnerRadius, fOuterRadius, damage);
    if (!mIsAlive)
    {
        const PblVector3& vPosition = mMatrix.GetTranslation();
        PblVector3 vAway(vPosition.x - vPos.x, vPosition.y - vPos.y, vPosition.z - vPos.z);
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vAway);
        DirectionKill(vDir);
    }
}

// FUNCTION: SWBF2 0x004e1ec0
void EntitySoldier::DirectionKill(const PblVector3& vDir)
{
    if (mState != SoldierState::PILOT_ANIMATION && mState != SoldierState::ARMED_PASSENGER)
    {
        mVelocity.x += vDir.x * 5.0f;
        mVelocity.y += vDir.y * 5.0f;
        mVelocity.z += vDir.z * 5.0f;
        if (mPlayerId >= 0 && mCurHealth < -100.0f)
        {
            AIDebug::StartBulletTime(0.15f, 0.08f, 0.4f);
        }
        return;
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mCharacter)
    {
        if (mCharacter->mRemote)
        {
            ExitRemote();
        }
        if (Controllable* pVehicle = mCharacter->mVehicle)
        {
            ExitVehicle(pVehicle->GetGameObject()->mIsAlive != 0, false);
            if (pVehicle->GetPilotDeathType() == PILOTDEATH_FLY)
            {
                PblVector3 vThrow = vDir;
                float fLift = PblRandom::GetSim().GetFloat() * 0.3f + 0.4f;
                mVelocity = PblVector3(vThrow.x * 9.0f, (fLift + vThrow.y) * 9.0f, vThrow.z * 9.0f);
                Fly();
            }
        }
    }
}

// FUNCTION: SWBF2 0x004e22a0
void EntitySoldier::TurnOffJetIdleEffect()
{
    if (mJetIdleEffectHandle)
    {
        mJetIdleEffectHandle->StopAndFinish();
        mJetIdleEffectHandle = NULL;
    }
}

// FUNCTION: SWBF2 0x004ed910
bool EntitySoldier::Roll()
{
    if (m_bSlide)
    {
        return false;
    }
    if (mClass->mRollSpeedFactor <= 0.0f)
    {
        return false;
    }
    if (mState != SoldierState::STAND && mState != SoldierState::CROUCH && mState != SoldierState::SPRINT)
    {
        return false;
    }
    if (!SetState(SoldierState::ROLL))
    {
        return false;
    }

    const PblVector3& vFront = mMatrix.GetZAxis();
    const PblVector3& vRight = mMatrix.GetXAxis();
    float fRollSpeed = mClass->mRollSpeedFactor * mClass->mMaxSpeed;
    float fForward = mVelocity.x * vFront.x + mVelocity.y * vFront.y + mVelocity.z * vFront.z;
    float fSide = mVelocity.x * vRight.x + mVelocity.y * vRight.y + mVelocity.z * vRight.z;
    float fSpeed2 = fSide * fSide + fForward * fForward;
    if (fSpeed2 > 0.01f && fRollSpeed * fRollSpeed > fSpeed2)
    {
        float fScale = fRollSpeed / (float)sqrt((double)fSpeed2) - 1.0f;
        if (fForward > 0.0f)
        {
            float fPush = fScale * fForward;
            mVelocity.x += fPush * vFront.x;
            mVelocity.y += vFront.y * fPush;
            mVelocity.z += vFront.z * fPush;
        }
        float fPush = fScale * fSide;
        mVelocity.x += vRight.x * fPush;
        mVelocity.y += vRight.y * fPush;
        mVelocity.z += vRight.z * fPush;
    }

    if (FoleyFXSoldier* pFoley = static_cast<FoleyFXSoldier*>(GetFoleyFX()))
    {
        mCurFoleySound = pFoley->mRoll;
        mLongFoleySound.Play(&mCurFoleySound, &mMatrix.GetTranslation(), &mVelocity, true);
    }
    if (mActiveIndex >= 0 && mWeapon[mActiveIndex])
    {
        mWeapon[mActiveIndex]->PlayFoleyFX(6);
    }
    mSoundState |= 0x80;

    int iFirstPerson = GetFirstPersonIndex();
    if (iFirstPerson >= 0)
    {
        float fMove = fabsf(mControlMove);
        if (mControlStrafe > fMove)
        {
            FirstPerson::RollRight(iFirstPerson);
        }
        else if (-fMove > mControlStrafe)
        {
            FirstPerson::RollLeft(iFirstPerson);
        }
    }
    if (mCtrl)
    {
        mCtrl->IncreaseTargetBubbleSize(AI::TBI_ROLL);
    }
    return true;
}

// FUNCTION: SWBF2 0x004ee1d0
bool EntitySoldier::EndJetJump()
{
    if (mState == SoldierState::JET_HOVER || mState == SoldierState::JET_JUMP)
    {
        return SetState(SoldierState::FALL);
    }
    return true;
}

// FUNCTION: SWBF2 0x004ee200
void EntitySoldier::Fly()
{
    SetState(SoldierState::FLY);
}

// FUNCTION: SWBF2 0x004eef40
void EntitySoldier::MoveJetHover(float fDeltaTime, float fForward, float fStrafe, float fTurn,
                                 float fAcceleration)
{
    if (mFlags & flags_isGrounded)
    {
        float fDot = mGroundNormal.x * mVelocity.x + mGroundNormal.y * mVelocity.y + mGroundNormal.z * mVelocity.z;
        if (fDot < 0.0f)
        {
            mVelocity.x -= mGroundNormal.x * fDot;
            mVelocity.y -= mGroundNormal.y * fDot;
            mVelocity.z -= mGroundNormal.z * fDot;
        }
    }

    float fDrag = (float)exp((double)(fDeltaTime * -2.0f));
    mVelocity.x *= fDrag;
    mVelocity.y *= fDrag;
    mVelocity.z *= fDrag;

    PblVector3 vLook(mEyeDir.x, (mEyeDir.y + 0.08f) * 2.0f, mEyeDir.z);
    PblVector3 vDir;
    D3DXVec3Normalize(&vDir, &vLook);

    const PblVector3& vRight = mMatrix.GetXAxis();
    float fSide = mVelocity.x * vRight.x + mVelocity.y * vRight.y + mVelocity.z * vRight.z;
    float fMax = fDeltaTime * fAcceleration;
    float fAlong = fForward - (mVelocity.x * vDir.x + mVelocity.y * vDir.y + mVelocity.z * vDir.z);
    if (fAlong > fMax)
    {
        fAlong = fMax;
    }
    else if (-fMax > fAlong)
    {
        fAlong = -fMax;
    }
    float fAcross = fStrafe - fSide;
    if (fAcross > fMax)
    {
        fAcross = fMax;
    }
    else if (-fMax > fAcross)
    {
        fAcross = -fMax;
    }

    mVelocity.x += vDir.x * fAlong;
    mVelocity.y += vDir.y * fAlong;
    mVelocity.z += vDir.z * fAlong;
    mVelocity.x += vRight.x * fAcross;
    mVelocity.y += vRight.y * fAcross;
    mVelocity.z += vRight.z * fAcross;

    const PblVector3& vPos = mMatrix.GetTranslation();
    SetPosition(PblVector3(vPos.x + mVelocity.x * fDeltaTime, vPos.y + mVelocity.y * fDeltaTime,
                           vPos.z + mVelocity.z * fDeltaTime));

    float fAngle = fDeltaTime * fTurn;
    if (fAngle != 0.0f)
    {
        float fSin;
        float fCos;
        PblMath::SinCos(&fSin, &fCos, fAngle);
        PblMatrix mRotation;
        mRotation.m[0][0] = fCos;
        mRotation.m[0][1] = 0.0f;
        mRotation.m[0][2] = -fSin;
        mRotation.m[0][3] = 0.0f;
        mRotation.m[1][0] = 0.0f;
        mRotation.m[1][1] = 1.0f;
        mRotation.m[1][2] = 0.0f;
        mRotation.m[1][3] = 0.0f;
        mRotation.m[2][0] = fSin;
        mRotation.m[2][1] = 0.0f;
        mRotation.m[2][2] = fCos;
        mRotation.m[2][3] = 0.0f;
        mRotation.m[3][0] = 0.0f;
        mRotation.m[3][1] = 0.0f;
        mRotation.m[3][2] = 0.0f;
        mRotation.m[3][3] = 1.0f;
        PblMatrix mNew;
        D3DXMatrixMultiply(&mNew, &mRotation, &mMatrix);
        SetMatrix(mNew);
    }
    else
    {
        SetMatrix(mMatrix);
    }
}

// FUNCTION: SWBF2 0x004f1dd0
bool EntitySoldier::EnterRemote(GameObject* pObject)
{
    Controllable* pRemote = pObject->GetControllable();
    if (!pRemote)
    {
        const PblList<Controllable>& rList = pObject->GetControllableList();
        if (rList._iCount < 1 || !(pRemote = rList._head._pNext->_pObject))
        {
            return false;
        }
    }

    if (mTracker && mTracker->IsFirstPersonView())
    {
        int iJoystick = NetGame::GetJoystickIndex(mInitialPlayerId);
        if (iJoystick >= 0)
        {
            FirstPerson::ClearFiringState(iJoystick);
        }
    }

    mCharacter->GetControllable()->ClearControllable();
    mCharacter->GetControllable()->ReleaseController();
    mPrevRemoteControllable = mCharacter->mRemote;
    mPrevRemoteGameObj = mPrevRemoteControllable ? mPrevRemoteControllable->GetGameObject() : 0;
    mCharacter->SetRemote(pRemote);
    mCharacter->mRemote->SetPilot(this);
    TurnOffEffects();
    return true;
}

// FUNCTION: SWBF2 0x004f1f00
bool EntitySoldier::ExitRemote()
{
    if (mCharacter->mRemote)
    {
        mCharacter->mRemote->SetPilot(0);
        mCharacter->SetRemote(mPrevRemoteGameObj ? mPrevRemoteControllable : 0);
        mPrevRemoteControllable = 0;
        mPrevRemoteGameObj = NULL;
    }

    mCharacter->GetControllable()->ClearControllable();
    mCharacter->GetControllable()->SetupController();

    if (mTracker && mTracker->IsFirstPersonView())
    {
        int iJoystick = NetGame::GetJoystickIndex(mInitialPlayerId);
        if (iJoystick >= 0)
        {
            FirstPerson::ClearFiringState(iJoystick);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x004f38a0
float EntitySoldier::GetDamageMultiplier(int iBodyId) const
{
    if (mBuffDefenseTimer > 0.0f)
    {
        return GameObject::GetDamageMultiplier(iBodyId) * mBuffDefenseMult;
    }
    return GameObject::GetDamageMultiplier(iBodyId);
}

// FUNCTION: SWBF2 0x004f3b90
float EntitySoldier::GetOffenseMultiplier() const
{
    if (mBuffOffenseTimer != 0.0f)
    {
        return mBuffOffenseMult;
    }
    return 1.0f;
}

// FUNCTION: SWBF2 0x004e1b40
bool EntitySoldier::ApplyDamage(const DamageDesc& damage, const PblVector3& vDir, const PblVector3& vPos,
                                int iBodyPart, unsigned int uFlags)
{
    if (!IsAlive())
    {
        return false;
    }

    const PblMatrix& mFront = GetMatrix();
    float fFront = mFront.m[2][0] * vDir.x + mFront.m[2][1] * vDir.y + mFront.m[2][2] * vDir.z;
    const PblMatrix& mRight = GetMatrix();
    float fRight = vDir.x * mRight.m[0][0] + mRight.m[0][1] * vDir.y + mRight.m[0][2] * vDir.z;
    if (fabsf(fFront) > fabsf(fRight))
    {
        m_uiLastDamageDir = fFront > 0.0f ? FORWARD : BACKWARD;
    }
    else
    {
        m_uiLastDamageDir = fRight > 0.0f ? LEFT : RIGHT;
    }
    if (mSoldierAnimator && mSoldierAnimator->GetOwner() == this)
    {
        mSoldierAnimator->SetDamageDir((DAMAGEDIR)m_uiLastDamageDir);
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

    if (iBodyPart == 0)
    {
        GameObject* pObject = GetGameObject();
        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            EventManager::ObjectEvent::mHeadshot.Trigger(*pObject, const_cast<DamageOwner&>(damage.mDamageOwner));
        }
    }

    if (Damageable::ApplyDamage(damage, vDir, vPos, iBodyPart, uFlags))
    {
        return true;
    }

    if (iBodyPart == 0 && mDamageOwner.mCharacter && mDamageOwner.mCharacter->IsPlayer() &&
        ResolveGameObjectHandle(mDamageOwner.mGameObject))
    {
        if (GetAffiliationToMe(ResolveGameObjectHandle(mDamageOwner.mGameObject)) < 0)
        {
            AI::VOHelper::Play(AI::VOT_HeadshotKill, 0, 0, this, 0.0f);
        }
    }
    DirectionKill(vDir);
    return NetGame::IsEnabled() && NetGame::IsClient();
}

// FUNCTION: SWBF2 0x004e2180
bool EntitySoldier::Deflect(const Ordnance* pOrdnance, const PblVector3& vPos, const PblVector3& vDir)
{
    if (mActiveIndex < 0)
    {
        return false;
    }
    Weapon* pWeapon = mWeapon[mActiveIndex];
    if (!pWeapon)
    {
        return false;
    }
    return pWeapon->Deflect(pOrdnance, vPos, vDir);
}

// FUNCTION: SWBF2 0x004e2250
void EntitySoldier::SetWeaponIndex(int iIndex, int iChannel)
{
    mWeaponIndex[iChannel] = (char)iIndex;
}

// FUNCTION: SWBF2 0x004edbd0
bool EntitySoldier::Sprint()
{
    if (m_bSlide)
    {
        return false;
    }
    if (mState != SoldierState::STAND && mState != SoldierState::SPRINT)
    {
        return false;
    }
    if (mClass->mThrustFactor[0] >= mClass->mThrustFactor[3])
    {
        return false;
    }
    if (mState == SoldierState::SPRINT)
    {
        return true;
    }
    bool bResult = SetState(SoldierState::SPRINT);
    if (bResult && mCtrl)
    {
        mCtrl->IncreaseTargetBubbleSize(AI::TBI_SPRINT);
    }
    return bResult;
}

// FUNCTION: SWBF2 0x004edc40
bool EntitySoldier::EndSprint()
{
    if (mState != SoldierState::SPRINT)
    {
        return false;
    }
    return SetState(SoldierState::STAND);
}

// FUNCTION: SWBF2 0x004f07d0
void EntitySoldier::DeactivateIndirect()
{
    mSkip = false;
    mSkipTime = 0.0f;
    mSkipIndirectTime = 0.0f;
    for (int i = 0; i < GetWeaponCount(); ++i)
    {
        Weapon* pWeapon = GetWeaponPtr(i);
        if (pWeapon)
        {
            pWeapon->ResetSkip();
        }
    }
    mAimer.ResetSkip();
}

// FUNCTION: SWBF2 0x004f0b40
bool EntitySoldier::EnterVehicle(GameObject* pObject)
{
    if (!pObject->IsAlive() || pObject->mDisableTime > 0.0f)
    {
        return false;
    }
    int iTeam = pObject->GetTeam();
    if (iTeam > 0)
    {
        if (mPlayerId < 0)
        {
            if (iTeam != GetTeam())
            {
                return false;
            }
        }
        else if (!IsMyFriend(pObject))
        {
            return false;
        }
    }

    Controllable* pSeat = 0;
    for (PblList<Controllable>::Itor it(pObject->GetControllableList()); it; ++it)
    {
        Controllable* pTest = it;
        if (pTest->mPilotType != PILOT_VEHICLE && pTest->mPilotType != PILOT_VEHICLESELF)
        {
            continue;
        }
        if (!pTest->GetPilot() &&
            (mPlayerId >= 0 || (pTest->mPilotType != PILOT_SELF && pTest->mPilotType != PILOT_VEHICLESELF)))
        {
            pSeat = pTest;
            break;
        }
        if (pTest->GetPilot() && pTest->GetPilot()->mPlayerId < 0 && mPlayerId >= 0 &&
            !pTest->GetPilot()->mCharacter->mRemote)
        {
            pSeat = pTest;
            break;
        }
    }
    if (!pSeat)
    {
        return false;
    }
    pObject->ReserveForPlayer(0.0f);
    return EnterControllable(pSeat);
}

// FUNCTION: SWBF2 0x004f0ca0
bool EntitySoldier::EnterControllable(Controllable* pControllable)
{
    Controllable* pPilot = pControllable->GetPilot();
    if (pPilot && !pPilot->CyclePosition(true))
    {
        if (pPilot->mCharacter && pPilot->mCharacter->mRemote)
        {
            pPilot->ExitRemote();
        }
        pPilot->ExitVehicle(false, true);
    }

    ClearControllable();
    if (pControllable->GetControllableClass()->mControlsUnit)
    {
        for (int iChannel = 0; iChannel < 2; ++iChannel)
        {
            int iIndex = mWeaponIndex[iChannel];
            if (iIndex >= 0 && mWeapon[iIndex])
            {
                mWeapon[iIndex]->Deselect(mActiveChannel, false);
            }
            mWeaponIndex[iChannel] = -1;
        }
        for (int i = 0; i < mClass->mWeaponCount; ++i)
        {
            if (!mWeapon[i])
            {
                continue;
            }
            if (mClass->mWeaponChannel[i] == 0)
            {
                if (mWeaponIndex[0] < 0)
                {
                    mWeaponIndex[0] = (char)i;
                }
            }
            else if (mWeaponIndex[1] < 0)
            {
                mWeaponIndex[1] = (char)i;
            }
            if (mWeaponIndex[0] >= 0 && mWeaponIndex[1] >= 0)
            {
                break;
            }
        }
        mActiveChannel = 0;
        mActiveIndex = mWeaponIndex[mActiveChannel];
        for (int iChannel = 0; iChannel < 2; ++iChannel)
        {
            int iIndex = mWeaponIndex[iChannel];
            if (iIndex >= 0 && mWeapon[iIndex])
            {
                mWeapon[iIndex]->Select(iChannel, false);
            }
        }
        SetAlert(false);
    }
    else if (mCtrl)
    {
        ReleaseController();
    }

    RemoveFromCollisionManager();
    DeactivatePhysics();
    RemoveFromScene();
    TargetManager::RemoveTarget(this);
    mTargetInfo.mIsAiming = false;
    mCharacter->SetVehicle(pControllable);
    mCharacter->mVehicle->SetPilot(this);
    TurnOffJetEffect();
    TurnOffJetIdleEffect();
    GameObject* pVehicle = pControllable->GetGameObject();
    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        EventManager::CharacterEvent::mEnterVehicle.Trigger(*mCharacter, *pVehicle);
    }
    if (NetGame::IsEnabled() && !NetGame::IsClient() && mPlayerId >= 0)
    {
        NetGame::SetEnterVehicleTurn(mPlayerId);
    }
    TurnOffEffects();
    NetGame::AddLateFence(mPlayerId);
    return true;
}

// FUNCTION: SWBF2 0x004f0f70
bool EntitySoldier::CyclePosition(bool bForward)
{
    if (mCharacter->mRemote)
    {
        return false;
    }

    Controllable* pCurrent = mCharacter->GetControllable();
    if (pCurrent)
    {
        for (int iChannel = 0; iChannel < 2; ++iChannel)
        {
            int iIndex = pCurrent->GetWeaponIndex(iChannel);
            if (iIndex < 0)
            {
                continue;
            }
            Weapon* pWeapon = pCurrent->GetWeaponPtr(iIndex);
            if (pWeapon && (pWeapon->mState == Weapon::FIRE || pWeapon->mState == Weapon::FIRE2))
            {
                return false;
            }
        }
    }

    Controllable* pVehicle = mCharacter->mVehicle;
    PblList<Controllable>::Itor it(&pVehicle->mNode);
    for (;;)
    {
        Controllable* pSeat = it;
        if (pSeat && (pSeat->mPilotType == PILOT_VEHICLE || pSeat->mPilotType == PILOT_VEHICLESELF) &&
            !(pVehicle && pVehicle->GetEntity()->IsRtti(PassengerSlot::rttiHashPassengerSlot) &&
              pSeat->GetEntity()->IsRtti(PassengerSlot::rttiHashPassengerSlot)))
        {
            if (!pSeat->GetPilot())
            {
                break;
            }
            if (pSeat->GetPilot()->mPlayerId < 0 && mPlayerId >= 0 && !pSeat->GetPilot()->mCharacter->mRemote)
            {
                break;
            }
        }
        if (bForward)
        {
            ++it;
        }
        else
        {
            --it;
        }
        if ((Controllable*)it == pVehicle)
        {
            return false;
        }
    }

    Controllable* pSeat = it;
    if (!pSeat)
    {
        return false;
    }
    Controllable* pOldPilot = pSeat->GetPilot();

    pVehicle->SetPilot(0);
    pSeat->SetPilot(0);
    mCharacter->SetVehicle(pSeat);
    pSeat->SetPilot(this);

    if (pSeat->GetControllableClass()->mControlsUnit)
    {
        if (!mCtrl)
        {
            SetupController();
        }
        for (int iChannel = 0; iChannel < 2; ++iChannel)
        {
            int iIndex = mWeaponIndex[iChannel];
            if (iIndex >= 0 && mWeapon[iIndex])
            {
                mWeapon[iIndex]->Deselect(mActiveChannel, false);
            }
            mWeaponIndex[iChannel] = -1;
        }
        for (int i = 0; i < mClass->mWeaponCount; ++i)
        {
            if (!mWeapon[i])
            {
                continue;
            }
            if (mClass->mWeaponChannel[i] == 0)
            {
                if (mWeaponIndex[0] < 0)
                {
                    mWeaponIndex[0] = (char)i;
                }
            }
            else if (mWeaponIndex[1] < 0)
            {
                mWeaponIndex[1] = (char)i;
            }
            if (mWeaponIndex[0] >= 0 && mWeaponIndex[1] >= 0)
            {
                break;
            }
        }
        mActiveChannel = 0;
        mActiveIndex = mWeaponIndex[mActiveChannel];
        for (int iChannel = 0; iChannel < 2; ++iChannel)
        {
            int iIndex = mWeaponIndex[iChannel];
            if (iIndex >= 0 && mWeapon[iIndex])
            {
                mWeapon[iIndex]->Select(iChannel, false);
            }
        }
        SetAlert(false);
    }
    else if (mCtrl)
    {
        ReleaseController();
    }
    mTargetInfo.mIsAiming = false;

    if (pOldPilot)
    {
        Controllable* pFree = 0;
        for (PblList<Controllable>::Itor itFree(pSeat->GetGameObject()->GetControllableList()); itFree; ++itFree)
        {
            Controllable* pTest = itFree;
            if ((pTest->mPilotType == PILOT_VEHICLE || pTest->mPilotType == PILOT_VEHICLESELF) &&
                !pTest->GetPilot())
            {
                pFree = pTest;
                break;
            }
        }
        if (pOldPilot->mCharacter)
        {
            pOldPilot->mCharacter->SetVehicle(pFree);
        }
        pFree->SetPilot(pOldPilot);
    }
    return true;
}

// FUNCTION: SWBF2 0x004f1380
bool EntitySoldier::ExitVehicle(bool bForce, bool bEject)
{
    Controllable* pVehicle = mCharacter->mVehicle;
    if (!pVehicle)
    {
        return false;
    }

    if (bEject)
    {
        GameObject* pObject = pVehicle->GetGameObject();
        const CollisionObject* pCollision = ((const GameObject*)pObject)->GetCollisionObject();
        PblVector3 vCenter = pCollision->GetCollisionCenter();

        float fRadius = pCollision->GetCollisionRadius() + 0.68400007f;
        float fDiag = fRadius * 0.7071f;
        PblVector3 avPoint[9] = {
            PblVector3(0.0f, 1.0f, -fRadius), PblVector3(-fRadius, 1.0f, 0.0f), PblVector3(fRadius, 1.0f, 0.0f),
            PblVector3(-fDiag, 1.0f, -fDiag), PblVector3(fDiag, 1.0f, -fDiag),  PblVector3(fDiag, 1.0f, fDiag),
            PblVector3(-fDiag, 1.0f, fDiag),  PblVector3(0.0f, 1.0f, fRadius),  PblVector3(0.0f, 1.0f, 0.0f)
        };
        PblMatrix mSeat = NormalMatrix(pVehicle->GetMatrix().GetZAxis(), pObject->GetMatrix().GetYAxis(),
                                     pObject->GetMatrix().GetTranslation());
        PblVector3 vExit = mSeat.GetTranslation();
        const GameObject* apIgnore[1] = { pObject };
        CollisionObject* pHit;
        for (int i = 0; i < 9; ++i)
        {
            PblVector3 vWorld;
            D3DXVec3TransformCoord(&vWorld, &avPoint[i], &mSeat);
            avPoint[i] = vWorld;
            if (i != 8 && CollisionManager::RayHit(vCenter, avPoint[i], pHit, 0, apIgnore, 1, 0x98, false) != 1.0f)
            {
                continue;
            }

            const PblVector3& v = avPoint[i];
            PblVector3 avFoot[5] = { v,
                                     PblVector3(v.x + 0.72f, v.y + 0.0f, v.z + 0.72f),
                                     PblVector3(v.x + 0.72f, v.y + 0.0f, v.z - 0.72f),
                                     PblVector3(v.x - 0.72f, v.y + 0.0f, v.z + 0.72f),
                                     PblVector3(v.x - 0.72f, v.y + 0.0f, v.z - 0.72f) };
            int iBest = -1;
            for (int j = 0; j < 5; ++j)
            {
                float fUp = CollisionManager::RayHit(avFoot[j], gVectorUnitY, 5.0f, pHit, 0, 0, 0, 0x98, true);
                avFoot[j].y += fUp * 5.0f - 0.1f;
                PblVector3 vDown(-gVectorUnitY.x, -gVectorUnitY.y, -gVectorUnitY.z);
                float fDown = CollisionManager::RayHit(avFoot[j], vDown, 15.0f, pHit, 0, 0, 0, 0x98, true);
                if (fDown < 1.0f)
                {
                    avFoot[j].y += fDown * -10.0f;
                    if (iBest < 0 || avFoot[j].y > avFoot[iBest].y)
                    {
                        iBest = j;
                    }
                }
            }
            if (iBest >= 0)
            {
                vExit = PblVector3(avFoot[0].x, avFoot[iBest].y, avFoot[0].z);
                break;
            }
        }
        SetMatrix(NormalMatrix(mSeat.GetZAxis(), gVectorUnitY, vExit));
    }

    mVelocity = PblVector3(0.0f, 0.0f, 0.0f);
    pVehicle->SetPilot(0);
    mCharacter->SetVehicle(0);
    if (mPlayerId >= 0 && pVehicle->GetGameObject())
    {
        pVehicle->GetGameObject()->ReserveForPlayer(30.0f);
    }

    if (bForce && pVehicle->mPilotType != PILOT_SELF && pVehicle->mPilotType != PILOT_VEHICLESELF)
    {
        Controllable* pPassenger = 0;
        for (PblList<Controllable>::Itor it(&pVehicle->mNode); it; ++it)
        {
            Controllable* pSeat = it;
            if ((pSeat->mPilotType == PILOT_VEHICLE || pSeat->mPilotType == PILOT_VEHICLESELF) &&
                pSeat->GetPilot() && pSeat->GetPilot()->mPlayerId < 0)
            {
                pPassenger = pSeat;
            }
        }
        if (pPassenger)
        {
            Controllable* pPilot = pPassenger->GetPilot();
            pPassenger->SetPilot(0);
            if (pPilot->mCharacter)
            {
                pPilot->mCharacter->SetVehicle(pVehicle);
            }
            pVehicle->SetPilot(pPilot);
        }
    }

    ClearControllable();
    Stand(false);

    static const float sFallingStartTimer = 0.9999f;
    mStateTimer = sFallingStartTimer;
    mFlags = (unsigned char)((mFlags & ~flags_isGrounded) | flags_fallTimer);

    if (pVehicle->GetGameObject())
    {
        PblVector3 vVelocity = pVehicle->GetGameObject()->GetVelocity();
        const PblVector3& vFrom = pVehicle->GetGameObject()->GetPosition();
        const PblVector3& vTo = GetPosition();
        PblVector3 vAway(vTo.x - vFrom.x, vTo.y - vFrom.y, vTo.z - vFrom.z);
        PblVector3 vNew(vVelocity.x * 0.85f, vVelocity.y * 0.85f, vVelocity.z * 0.85f);
        if (vAway.x * vAway.x + vAway.z * vAway.z > 0.01f)
        {
            vAway.y = 0.0f;
            D3DXVec3Normalize(&vVelocity, &vAway);
            vNew.x += vVelocity.x * 4.0f;
            vNew.y += vVelocity.y * 4.0f;
            vNew.z += vVelocity.z * 4.0f;
        }
        SetVelocity(vNew);
    }

    if (mSoldierAnimator)
    {
        if (mSoldierAnimator->mOwner == this)
        {
            mSoldierAnimator->mOwner = 0;
        }
        mSoldierAnimator = 0;
    }
    if (!mCtrl)
    {
        SetupController();
    }
    if (IsInCollisionList())
    {
        RemoveFromCollisionManager();
    }
    SetCollisionStance(mStanceIndex);
    AddSoftToCollisionManager();
    ActivatePhysics();
    if (!IsInScene())
    {
        AddToScene();
    }
    TargetManager::AddTarget(this, true);
    NetGame::AddLateFence(mPlayerId);
    return true;
}

// FUNCTION: SWBF2 0x004f2ec0
void EntitySoldier::SetPilotInfo(const PblMatrix& mMatrixIn, const PblVector3& vVelocity,
                                 SoldierAnimationBank::ANIMATION eAnimation, bool bAnimate, float fFrame1,
                                 float fFrame2, float fBlend, float fDeltaTime)
{
    if (!IsThreadActive())
    {
        mJetFuel += mClass->mJetFuelRechargeRate * fDeltaTime;
        if (mJetFuel > 1.0f)
        {
            mJetFuel = 1.0f;
        }
        UpdateBuffTimers(fDeltaTime);
        UpdateSmolderTimers(fDeltaTime);
    }
    SetMatrix(mMatrixIn);
    SetVelocity(vVelocity);

    if (!mPostCollision.IsThreadActive())
    {
        if (Smooth(fDeltaTime) && IsInScene())
        {
            const PblVector3& vCenter = GetCollisionCenter();
            const PblVector3& vPos = GetPosition();
            const PblMatrix& mSmoothed = GetSmoothedMatrix();
            SetRenderPos(PblVector3(mSmoothed.m[3][0] + vCenter.x - vPos.x, mSmoothed.m[3][1] + vCenter.y - vPos.y,
                                    mSmoothed.m[3][2] + vCenter.z - vPos.z));
        }
        if (!netFrameLock && NetGame::IsEnabled())
        {
            bool bSend = true;
            if (!NetGame::IsClient() || !netIsLocalTurn)
            {
                if (!NetGame::IsHost() || !NetGame::IsOneWorld() || netIsLateTurn)
                {
                    bSend = false;
                }
                else if (mNetUniqueId >= 0)
                {
                    mNetUniqueId = NetGame::GetHostWorldUniqueId(mNetUniqueId, this);
                }
            }
            if (bSend)
            {
                gameObjectInterpolator.SetNext(mNetUniqueId, &GetSmoothedMatrix());
                gameObjectInterpolator.SetNextVector(mNetUniqueId, &mAimer.GetDirection());
            }
        }
    }

    mLastPilotInfoFrame = NetGame::GetTurnNumber();
    if (eAnimation != SoldierAnimationBank::INVALID_ANIMATION && bAnimate)
    {
        Controllable* pVehicle = mCharacter ? mCharacter->mVehicle : 0;
        if (pVehicle)
        {
            GameObject* pObject = pVehicle->GetGameObject();
            if (pObject)
            {
                mLastVehicleMatrix = pObject->GetMatrix();
            }
        }
        if (!IsInScene())
        {
            AddToScene();
        }
        if (!IsInCollisionList())
        {
            CollisionManager::AddAttachedBody(this);
        }
        mForceFrameNum1 = fFrame1;
        mForceFrameNum2 = fFrame2;
        mForceFrameBlend = fBlend;
        m_ePilotAnimation = eAnimation;
        mAnimationHash = 0;
        mAnimationHash = SoldierAnimatorClass::sInstance->GetCustomAnimation(m_eSoldierAnimationMap, eAnimation);

        bool bArmed = false;
        Controllable* pSeat = mCharacter ? mCharacter->GetControllable() : 0;
        if (pSeat)
        {
            bArmed = pSeat->GetControllableClass()->mAnimatesUnitAsTurret ||
                     pSeat->GetControllableClass()->mControlsUnit;
        }
        mState = bArmed ? SoldierState::ARMED_PASSENGER : SoldierState::PILOT_ANIMATION;
        UpdateOrdnanceCollision(fDeltaTime);
        if (bArmed)
        {
            CheckForZoom(fDeltaTime);
            UpdateWeaponAndAimer(fDeltaTime);
        }
    }
    else
    {
        if (IsInCollisionList())
        {
            RemoveFromCollisionManager();
        }
        if (IsInScene())
        {
            RemoveFromScene();
        }
    }
}

// FUNCTION: SWBF2 0x004f3310
void EntitySoldier::DeactivatePhysics()
{
    for (int iChannel = 0; iChannel < 2; ++iChannel)
    {
        int iIndex = mWeaponIndex[iChannel];
        if (iIndex < 0)
        {
            continue;
        }
        Weapon* pWeapon = mWeapon[iIndex];
        if (pWeapon->mSelectedFlag)
        {
            pWeapon->Deselect(iChannel, false);
        }
    }
    mPostCollision.DeactivateThread();
    DeactivateThread();
}

// FUNCTION: SWBF2 0x004f3410
void EntitySoldier::Flee()
{
    mClass->mFleeSound.Play(&GetMatrix().GetTranslation(), 0, false, true);
}

// FUNCTION: SWBF2 0x004f3470
void EntitySoldier::HeardEnemy()
{
    mClass->mHeardEnemySound.Play(&GetMatrix().GetTranslation(), 0, false, true);
}

// FUNCTION: SWBF2 0x004f34a0
void EntitySoldier::Charge()
{
    mClass->mChargeSound.Play(&GetMatrix().GetTranslation(), 0, false, true);
}

// SYNTHETIC: SWBF2 0x004fab6c
// [thunk]:EntitySoldier::GetGameObject`adjustor{320}'

// SYNTHETIC: SWBF2 0x004fabd4
// [thunk]:EntitySoldier::Kill`adjustor{256}'

// SYNTHETIC: SWBF2 0x004fac13
// [thunk]:EntitySoldier::`scalar deleting destructor'`adjustor{576}'

// FUNCTION: SWBF2 0x004ee210
bool EntitySoldier::CheckMoveControlForSprint(float fThrust, float fStrafe)
{
    if (fThrust * fThrust + fStrafe * fStrafe < 0.81f || fThrust < 0.0f)
    {
        return false;
    }
    const float fAbsStrafe = fStrafe < 0.0f ? -fStrafe : fStrafe;
    return fThrust * (mState == 3 ? 1.732f : 0.57700002f) > fAbsStrafe;
}

// FUNCTION: SWBF2 0x004edc60
bool EntitySoldier::JumpUsingEnergy()
{
    const PblVector3& vForward = mMatrix.GetZAxis();
    const float fForward = mVelocity.x * vForward.x + mVelocity.y * vForward.y
                         + mVelocity.z * vForward.z;
    float fCost = mClass->mEnergyCostJump;
    if (fForward > mClass->mMaxSpeed + 0.0001f &&
        mClass->mEnergyCostJumpSprint > fCost)
    {
        fCost = mClass->mEnergyCostJumpSprint;
    }
    EnergyBar* pBar = reinterpret_cast<EnergyBar*>(mEnergyBar);
    if (!pBar->SpendEnergy(fCost))
    {
        return false;
    }
    if (!Jump())
    {
        pBar->AddEnergy(fCost);
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x004edd30
bool EntitySoldier::RollUsingEnergy()
{
    EnergyBar* pBar = reinterpret_cast<EnergyBar*>(mEnergyBar);
    const float fCost = mClass->mEnergyCostRoll;
    if (!pBar->SpendEnergy(fCost))
    {
        return false;
    }
    if (!Roll())
    {
        pBar->AddEnergy(fCost);
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x004edd90
bool EntitySoldier::SprintUsingEnergy()
{
    return !reinterpret_cast<EnergyBar*>(mEnergyBar)->IsOverheating() && Sprint();
}

// FUNCTION: SWBF2 0x004e0bc0
void EntitySoldier::TurnOnInvincibleEffect()
{
    if (!mClass->mInvincibleEffect)
    {
        return;
    }
    if (mInvincibleEffectHandle)
    {
        return;
    }
    FLEffectClass* pClass = FLEffect::Get(mClass->mInvincibleEffect);
    if (pClass)
    {
        FLEffectObject* pEffect = pClass->BuildEffect();
        if (pEffect)
        {
            pEffect->AttachEffectToObject(this, true);
            mInvincibleEffectHandle = pEffect;
        }
    }
}

// FUNCTION: SWBF2 0x004e8540
void EntitySoldier::UpdateAcklayFoleyFX(const RedPose& pose, const PblMatrix& mWorld)
{
    if (!(mFlags & flags_isGrounded) || (mTracker && mTracker->IsFirstPersonView()))
    {
        return;
    }

    static const unsigned int kFootBone[6] = {
        0xc08d2ca3u, 0xcdce0a7au, 0xc90f17cdu,
        0x730bf271u, 0x7e48d4a8u, 0x7a89c91fu
    };
    const PblMatrix* pBones[6];
    for (int i = 0; i < 6; ++i)
    {
        pBones[i] = pose.Find(kFootBone[i]);
    }
    if (!pBones[0])
    {
        return;
    }

    for (int i = 0; i < 6; ++i)
    {
        const PblMatrix* pBone = pBones[i];
        float* pElevation = i == 0 ? &mLeftFootElevation
                           : i == 1 ? &mRightFootElevation
                           : &mAcklayData->mFeetElevation[i - 2];
        PblMatrix mFoot;
        D3DXMatrixMultiply(&mFoot, pBone, &mWorld);
        const PblVector3& vFoot = mFoot.GetTranslation();
        const float fElevation = pBone->m[3][1] - 2.5f;

        PblVector3 vBone(pBone->m[0][0], pBone->m[0][1], pBone->m[0][2]);
        PblVector3 vWorldBone;
        D3DXVec3TransformNormal(&vWorldBone, &vBone, &mWorld);
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vWorldBone);
        PblVector3 vWaterTest(vFoot.x + vDir.x * 2.5f,
                              vFoot.y + vDir.y * 2.5f,
                              vFoot.z + vDir.z * 2.5f);
        float fWaterHeight = -FLT_MAX;
        const bool bWaterStep = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y
                              + mVelocity.z * mVelocity.z > 4.5f &&
                              RedWater::GetWaterHeight(vWaterTest, fWaterHeight) &&
                              fWaterHeight > vFoot.y - 2.5f && vFoot.y - 1.0f > fWaterHeight;

        const unsigned short uFootBit = static_cast<unsigned short>(0x100u << i);
        if (fElevation - *pElevation > 0.01f && fElevation > 0.1f)
        {
            if (!(mSoundState & uFootBit))
            {
                mSoundState |= uFootBit;
            }
        }
        else if ((mSoundState & uFootBit) && *pElevation - fElevation > 0.01f && fElevation < 0.0f)
        {
            mSoundState &= ~uFootBit;
            mAcklayData->mFootSound[i & 1].Play(&vFoot, &mVelocity, mSoundState != 0, true);
            if ((!NetGame::IsEnabled() || (NetGame::IsClient() && netIsLocalTurn)) && bWaterStep)
            {
                PblVector3 vHorizontal(mVelocity.x, 0.0f, mVelocity.z);
                PblVector3 vNormal;
                D3DXVec3Normalize(&vNormal, &vHorizontal);
                WaterEffect::AddDirectionalWaterEffect(mClass->mWakeWaterSplashEffect,
                                                       vWaterTest, vNormal);
            }
        }
        *pElevation = fElevation;
    }
}

// FUNCTION: SWBF2 0x004ee270
bool EntitySoldier::CheckMoveControlForRoll(float fThrust, float fStrafe)
{
    if (fThrust * fThrust + fStrafe * fStrafe < 0.81f)
    {
        return false;
    }
    const float fAbsStrafe = fStrafe < 0.0f ? -fStrafe : fStrafe;
    return fThrust > 0.0f || fAbsStrafe > fThrust * -0.83899999f;
}

// FUNCTION: SWBF2 0x004f3810
void EntitySoldier::DebuffDamage(float fTimer, float fRate, const DamageOwner& owner)
{
    if (mClass->mHeroFlag || mClass->m_bIsAcklay)
    {
        mDebuffDamageTimer = 0.0f;
    }
    else if (fTimer > mDebuffDamageTimer)
    {
        mDebuffDamageTimer = fTimer;
        mDebuffDamageRate = fRate;
        mDebuffDamageOwner = owner;
    }
}

// FUNCTION: SWBF2 0x004e1600
void EntitySoldier::GetPitchYawLimits(float& fPitchMin, float& fPitchMax, float& fYawMin, float& fYawMax) const
{
    fPitchMin = mClass->mMinPitch;
    fPitchMax = mClass->mMaxPitch;
    fYawMin = 1.5707964f * -0.35f;
    fYawMax = 1.5707964f * 0.35f;
}

// FUNCTION: SWBF2 0x004e21b0
void EntitySoldier::ApplyWeaponKick(float fPitch, float fYaw)
{
    mPitchVelocity += fPitch * 10.0f;
    mKickBuild += fYaw;
}

// FUNCTION: SWBF2 0x004e2240
float EntitySoldier::GetAimTurnRate() const
{
    return mClass->mMaxPitchSpeed;
}

// FUNCTION: SWBF2 0x004e2270
int EntitySoldier::GetWeaponChannelByWeaponIndex(int iIndex) const
{
    return mClass->mWeaponChannel[iIndex];
}

// FUNCTION: SWBF2 0x004ed5c0
bool EntitySoldier::Prone()
{
    return false;
}

// FUNCTION: SWBF2 0x004f2dc0
float EntitySoldier::CalcAimSpreadVal() const
{
    switch ((int)mState)
    {
    case 0:
    case 3:
        return 0.0f;
    case 2:
        return 0.5f;
    default:
        return 0.25f;
    }
}

// FUNCTION: SWBF2 0x004f36d0
float EntitySoldier::GetMaxStrafe() const
{
    return mClass->mMaxStrafeSpeed;
}

// FUNCTION: SWBF2 0x004f3380
void EntitySoldier::AcquiredTarget()
{
    mClass->mAcquiredTargetSound.Play(&GetMatrix().GetTranslation(), 0, false, true);
}

// FUNCTION: SWBF2 0x004f33b0
void EntitySoldier::Hiding()
{
    mClass->mHidingSound.Play(&GetMatrix().GetTranslation(), 0, false, true);
}

// FUNCTION: SWBF2 0x004f33e0
void EntitySoldier::ApproachingTarget()
{
    mClass->mApproachingTargetSound.Play(&GetMatrix().GetTranslation(), 0, false, true);
}

// FUNCTION: SWBF2 0x004f3440
void EntitySoldier::PreparingForDamage()
{
    mClass->mPreparingForDamageSound.Play(&GetMatrix().GetTranslation(), 0, false, true);
}

// FUNCTION: SWBF2 0x004f0940
CameraTrackSetting* EntitySoldier::GetCameraTrackSettings()
{
    const int iStance = mState == 3 ? 3 : mStanceIndex;
    return &((CameraTrackSetting*)mClass->mCameraSettings)[2 * iStance + (mTargetInfo.mIsAiming ? 1 : 0)];
}

// FUNCTION: SWBF2 0x004ee440
void EntitySoldier::SetGrapplingHook(OrdnanceGrapplingHook* pHook)
{
    if (mHook == pHook)
    {
        return;
    }

    CameraManager* pManager = CameraManager::sInstance;
    for (unsigned int i = 0; i < pManager->m_iNumCam; ++i)
    {
        Trackable* pTrackable = pManager->mCurGameCamera[i] == pManager->mDeathCamera[i]
            ? pManager->mDeathCamera[i]->mOwner : pManager->mChaseCamera[i]->mOwner;
        if (pTrackable && pTrackable->GetGameObject() == this)
        {
            pManager->mChaseCamera[i]->mTimer = 0.0f;
            break;
        }
    }
    mHook = pHook;
}

// FUNCTION: SWBF2 0x004f0980
bool EntitySoldier::CanEnterRemote(const GameObject* pObject) const
{
    return CanEnterVehicleInternal(pObject, true);
}

// FUNCTION: SWBF2 0x004e20a0
void EntitySoldier::ApplyChoke(const PblVector3& vDir, float, SoldierAnimationBank::ANIMATION eAnimation)
{
    if (mState == 14)
    {
        mStateTimer = 0.0f;
    }
    else if (mState == 0 || mState == 1)
    {
        SetState((SoldierState::SoldierState)14);
        m_ePilotAnimation = eAnimation;
        if (eAnimation == -1)
        {
            mAnimationHash = 0x144ebf32;
        }
        else if (SoldierAnimatorClass::sInstance)
        {
            mAnimationHash = SoldierAnimatorClass::sInstance->GetCustomAnimation(m_eSoldierAnimationMap, eAnimation);
        }
        mInputLockTime = 0.5f;
        m_uiInputLockMask = 7;
    }
    else if (mState == 2)
    {
        Stand();
        m_uiInputLockMask = 7;
    }
    m_vChokeDir = PblVector3(-vDir.x, -vDir.y, -vDir.z);
}

// GLOBAL: SWBF2 0x01e51dd7
bool gShowSoldierPolyCount;

// SYNTHETIC: SWBF2 0x00402d80
// `dynamic initializer for 'g_SoldierPolyCount''
RedCommandConsole g_SoldierPolyCount("SoldierPolyCount", MultiVar(&gShowSoldierPolyCount));
