#include "MountedTurret.h"

#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "AILowLevel.h"
#include "AIUtil.h"
#include "CameraManager.h"
#include "Character.h"
#include "CollisionManager.h"
#include "FirstPerson.h"
#include "GameModel.h"
#include "GameObject.h"
#include "LockOnManager.h"
#include "NetGame.h"
#include "NetPredict.h"
#include "PlayerController.h"
#include "SpaceDustEffect.h"
#include "Tracker.h"
#include "UnitController.h"
#include "UnitStationaryController.h"
#include "VehicleManager.h"
#include "WeaponCannonInstance.h"
#include "CollisionObject.h"
#include "EntitySoldier.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Util/RedSceneObject.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"
#include "SoundFL/source/sndvoicevirtualbus.h"

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

namespace
{
bool IsTurretCameraTargeting(const MountedTurret& turret)
{
    return turret.mParent->mIsAlive &&
           (turret.mTargetInfo.mIsAiming ||
            (turret.mTracker && turret.mTracker->IsFirstPersonView()));
}

PblMatrix TurretDirectionMatrix(const MountedTurret& turret)
{
    PblVector3 vDirection(-turret.mEyeDir.x, -turret.mEyeDir.y, -turret.mEyeDir.z);
    const PblVector3& vUp = *(const PblVector3*)&turret.mAimMatrix.m[1][0];
    return DirectionMatrix(vDirection, vUp, turret.mEyePoint);
}
}

// FUNCTION: SWBF2 0x005a8500
CameraTrackSetting* MountedTurret::GetCameraTrackSettings()
{
    return &mClass->mCameraSettings;
}

// FUNCTION: SWBF2 0x005a8520
void MountedTurret::SetupCameraTrackMatrixHUDTargeting(PblMatrix& rMatrix)
{
    if (IsTurretCameraTargeting(*this))
    {
        rMatrix = TurretDirectionMatrix(*this);
        return;
    }
    const PblMatrix& rTarget = mClass->mTrackUseRoot ? mParent->GetMatrix() : mAimMatrix;
    const PblVector3& vAimDir = *(const PblVector3*)&mAimMatrix.m[2][0];
    mClass->mCameraSettings.SetupCameraTrackMatrixHUDTargeting(rMatrix, rTarget, vAimDir);
}

// FUNCTION: SWBF2 0x005a8640
void MountedTurret::SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime)
{
    if (IsTurretCameraTargeting(*this))
    {
        rMatrix = TurretDirectionMatrix(*this);
        int iView = GetFirstPersonIndex();
        if (iView >= 0 && !mTargetInfo.mIsAiming)
        {
            FirstPerson::UpdateCamera(iView, rMatrix);
        }
        return;
    }
    const PblMatrix& rTarget = mClass->mTrackUseRoot ? mParent->GetMatrix() : mAimMatrix;
    const PblVector3& vAimDir = *(const PblVector3*)&mAimMatrix.m[2][0];
    mClass->mCameraSettings.SetupCameraTrackMatrix(this, rMatrix, fDeltaTime, rTarget,
                                                     vAimDir, 1.0f, 1.0f, mParent);
}

// FUNCTION: SWBF2 0x005a87b0
void MountedTurret::SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix)
{
    if (IsTurretCameraTargeting(*this))
    {
        int iView = GetFirstPersonIndex();
        if (iView >= 0 && !mTargetInfo.mIsAiming)
        {
            FirstPerson::UpdateAimer(iView, rMatrix);
        }
    }
}

// FUNCTION: SWBF2 0x005a8810
void MountedTurret::SetupCameraFOV(float& rFOV)
{
    ControllableClass* pClass = GetControllableClass();
    rFOV = (mTargetInfo.mIsAiming || (mTracker && mTracker->IsFirstPersonView()))
        ? pClass->mFirstPersonFOV : pClass->mThirdPersonFOV;
}

// FUNCTION: SWBF2 0x005a8870
void MountedTurret::SetupCameraZoomFactor(float& rZoom)
{
    int iWeapon = mWeaponIndex[0];
    if (mTargetInfo.mIsAiming && iWeapon >= 0 && mWeapon[iWeapon])
    {
        rZoom = mWeapon[iWeapon]->mZoom;
    }
    else
    {
        rZoom = 1.0f;
    }
}

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

extern PblVector3 gVectorZero;

extern int gDebugCount;

void RemoveChildren(const GameObject* pObject, RedPose& rPose, int iBone);

// FUNCTION: SWBF2 0x005a7c50
void RemoveChildren(const GameObject* pObject, RedPose& rPose, int iBone)
{
    const RedSkeleton* pSkeleton = pObject->GetModel()->GetRedModel(0)->GetSkeleton();
    for (;;)
    {
        const RedSkeleton::Connection& rConnection = pSkeleton->m_aConnections[iBone];
        int iChild = rConnection.iChild;
        int iSibling = rConnection.iSibling;
        if (iChild != -1)
        {
            PblMatrix& rMatrix = const_cast<PblMatrix&>(rPose._GetBoneMatrix(iChild));
            rMatrix.m[3][0] = 0.0f;
            rMatrix.m[3][1] = 0.0f;
            rMatrix.m[3][2] = -1000.0f;
            rMatrix.m[3][3] = 1.0f;
            RemoveChildren(pObject, rPose, iChild);
        }
        if (iSibling == -1)
        {
            return;
        }
        PblMatrix& rMatrix = const_cast<PblMatrix&>(rPose._GetBoneMatrix(iSibling));
        rMatrix.m[3][0] = 0.0f;
        rMatrix.m[3][1] = 0.0f;
        rMatrix.m[3][2] = -1000.0f;
        rMatrix.m[3][3] = 1.0f;
        iBone = iSibling;
    }
}

extern bool netFrameLock;
extern bool netIsLocalTurn;

class EntityFlyer
{
public:
    enum State
    {
        LANDED = 0,
        TAKEOFF = 1,
        FLYING = 2,
        LANDING = 3,
        CRASHING = 4,
        CRASHED = 5
    };

    char mFlyerBaseStorage[0x564];
    State mState;

    static PblHash rttiHashEntityFlyer;
};
 

// GLOBAL: SWBF2 0x01ecb150
// SYNTHETIC: SWBF2 0x00404340
// `dynamic initializer for 'MountedTurret::rttiHashMountedTurret''
PblHash MountedTurret::rttiHashMountedTurret("MountedTurret");
char MountedTurret::rttiNameMountedTurret[] = "MountedTurret";

// GLOBAL: SWBF2 0x01ecb154
// SYNTHETIC: SWBF2 0x00404330
// `dynamic initializer for 'MountedTurretClass::rttiHashMountedTurretClass''
PblHash MountedTurretClass::rttiHashMountedTurretClass("MountedTurretClass");
char MountedTurretClass::rttiNameMountedTurretClass[] = "MountedTurretClass";

// GLOBAL: SWBF2 0x01ecb158
// SYNTHETIC: SWBF2 0x00404310
// `dynamic initializer for 'MountedTurret::sMemoryPool''
// SYNTHETIC: SWBF2 0x00768d20
// `dynamic atexit destructor for 'MountedTurret::sMemoryPool''
MemoryPool MountedTurret::sMemoryPool("MountedTurret", 0x2c0);

// FUNCTION: SWBF2 0x005a5160
bool MountedTurret::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashMountedTurret || EntityEx::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x005a5190
unsigned int MountedTurret::GetDerivedRtti() const
{
    return rttiHashMountedTurret;
}

// FUNCTION: SWBF2 0x005a51a0
const char* MountedTurret::GetDerivedRttiName() const
{
    return rttiNameMountedTurret;
}

// FUNCTION: SWBF2 0x005a51b0
const PblMatrix& MountedTurret::GetMatrix() const
{
    return mAimMatrix;
}

// FUNCTION: SWBF2 0x005a51c0
Entity* MountedTurret::GetEntity()
{
    return this;
}

// FUNCTION: SWBF2 0x005a51d0
const Entity* MountedTurret::GetEntity() const
{
    return this;
}

// FUNCTION: SWBF2 0x005a51e0
GameObject* MountedTurret::GetGameObject()
{
    return mParent;
}

// FUNCTION: SWBF2 0x005a51f0
const GameObject* MountedTurret::GetGameObject() const
{
    return mParent;
}

// FUNCTION: SWBF2 0x005a5200
Trackable* MountedTurret::GetTrackable()
{
    return this;
}

// FUNCTION: SWBF2 0x005a5210
const Controllable* MountedTurret::GetControllable() const
{
    return this;
}

// FUNCTION: SWBF2 0x005a5220
Controllable* MountedTurret::GetControllable()
{
    return this;
}

// FUNCTION: SWBF2 0x005a5230
int MountedTurret::GetWeaponIndex(int iChannel) const
{
    return mWeaponIndex[iChannel];
}

// FUNCTION: SWBF2 0x005a5250
Weapon* MountedTurret::GetWeaponPtr(int iIndex) const
{
    return mWeapon[iIndex];
}

// FUNCTION: SWBF2 0x005a5270
const Aimer* MountedTurret::GetAimer(int iIndex) const
{
    return mAimer[iIndex];
}

// FUNCTION: SWBF2 0x005a5290
bool MountedTurret::PostCollision::Update(float fDeltaTime)
{
    return mOwner->PostCollisionUpdate(fDeltaTime);
}

// SYNTHETIC: SWBF2 0x005a52b0
// MountedTurret::PostCollision::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005a52e0
// MountedTurret::PostCollision::~PostCollision

// FUNCTION: SWBF2 0x005a52f0
bool MountedTurretClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashMountedTurretClass || EntityClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x005a5320
unsigned int MountedTurretClass::GetDerivedRtti()
{
    return rttiHashMountedTurretClass;
}

// FUNCTION: SWBF2 0x005a5330
const char* MountedTurretClass::GetDerivedRttiName()
{
    return rttiNameMountedTurretClass;
}

// SYNTHETIC: SWBF2 0x005a5340
// MountedTurret::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005a5370
MountedTurret::MountedTurret(GameObject* pParent, MountedTurretClass* pClass, RedPose* pPose)
    : EntityEx(0, pClass),
      Controllable(pClass, -1),
      mClass(pClass),
      mPose(pPose),
      mPostCollision(this)
{
    mParent = pParent;

    mYaw = 0.0f;
    mPitch = 0.0f;
    if (mClass->mMinYaw > 0.0f || mClass->mMaxYaw < 0.0f)
    {
        mYaw = (mClass->mMaxYaw + mClass->mMinYaw) * 0.5f;
    }
    if (mClass->mMinPitch > 0.0f || mClass->mMaxPitch < 0.0f)
    {
        mPitch = (mClass->mMaxPitch + mClass->mMinPitch) * 0.5f;
    }
    mOmegaPitch = 0.0f;
    mOmegaYaw = 0.0f;

    for (int i = 0; i < 4; ++i)
    {
        mAimer[i] = 0;
        mWeapon[i] = 0;
    }

    mRootMatrix = mParent->GetMatrix();
    mAimMatrix = mParent->GetMatrix();
    mEyeDir = mAimMatrix.GetZAxis();
    mEyePoint = mClass->mCameraSettings.CalcEyePoint(mAimMatrix);

    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (!mClass->mWeaponClass[i])
        {
            continue;
        }

        mAimer[i] = new Aimer(&mClass->mAimerClass[i], this,
                              mClass->mMountHash ? &mAimMatrix : &mParent->GetMatrix(),
                              mPose, 0);

        WeaponDesc desc;
        desc.mOwner = this;
        desc.mAimer = mAimer[i];
        desc.mTrigger = &mControlFire[mClass->mWeaponChannel[i]];
        desc.mReload = &mControlReload;
        desc.mNumClips = 0;
        desc.m_pAmmoCounter = 0;
        desc.m_pEnergyBar = 0;

        const WeaponAmmoDesc& ammo = mClass->mWeaponAmmo[i];
        if (ammo.bFlagShareAmmo)
        {
            desc.m_pAmmoCounter = mWeapon[ammo.uiAmmo]->m_pAmmoCounter;
        }
        else if (ammo.bFlagAmmoInfinite)
        {
            desc.mNumClips = 0x7fffffff;
        }
        else
        {
            desc.mNumClips = ammo.uiAmmo;
        }
        if (ammo.bFlagShareHeat)
        {
            desc.m_pEnergyBar = mWeapon[ammo.uiHeat]->m_pEnergyBar;
        }

        mWeapon[i] = mClass->mWeaponClass[i]->Build(desc);
        if (mWeapon[i])
        {
            mWeapon[i]->Select(0, false);
        }
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

    if (!mClass->mIsVirtual && mPose && mClass->mMountHash && !mPose->Find(mClass->mMountHash))
    {
        mPose->Store(mClass->mMountHash, &mRootMatrix);
    }

    if (mClass->mIsControllable)
    {
        VehicleManager::Add(mParent);
    }

    mPrevPilot = 0;
    mNetActiveTimer = 0.0f;
    mInterpIndex = -1;
}

// FUNCTION: SWBF2 0x005a5820
MountedTurret::~MountedTurret()
{
    if (mClass->mIsControllable)
    {
        VehicleManager::Remove(mParent);
    }

    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        delete mWeapon[i];
        delete mAimer[i];
    }

    mTurretPitchSound.Stop(true);
    mTurretYawSound.Stop(true);
}

// FUNCTION: SWBF2 0x005a5940
void MountedTurret::Init()
{
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mWeapon[i])
        {
            mWeapon[i]->Init();
        }
    }
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mWeapon[i])
        {
            mWeapon[i]->Select(0, false);
        }
    }
}

// FUNCTION: SWBF2 0x005a59c0
void MountedTurret::Kill()
{
    int iIndex = GetFirstPersonIndex();
    if (iIndex >= 0)
    {
        FirstPerson::Activate(iIndex, false);
        FirstPerson::Update(iIndex, TYPE_COCKPIT, -2, 0, 0);
    }
    mTurretPitchSound.Stop(true);
    mTurretYawSound.Stop(true);
    Controllable::Kill();
}

// FUNCTION: SWBF2 0x005a7bf0
void MountedTurret::DeactivatePhysics()
{
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

// FUNCTION: SWBF2 0x005a5a20
const ControllableClass* MountedTurret::GetControllableClass() const
{
    return mClass;
}

// FUNCTION: SWBF2 0x005a5a40
ControllableClass* MountedTurret::GetControllableClass()
{
    return mClass;
}

// FUNCTION: SWBF2 0x005a5a60
float MountedTurret::GetAimTurnRate() const
{
    return mClass->mPitchRate;
}

// FUNCTION: SWBF2 0x005a5a70
void MountedTurret::SetWeaponIndex(int iIndex, int iChannel)
{
    mWeaponIndex[iChannel] = iIndex;
}

// FUNCTION: SWBF2 0x005a5a90
int MountedTurret::GetWeaponCount() const
{
    return mClass->mWeaponCount;
}

// FUNCTION: SWBF2 0x005a5aa0
void MountedTurret::GetPitchYawLimits(float& fPitchMin, float& fPitchMax,
                                      float& fYawMin, float& fYawMax) const
{
    fPitchMin = mClass->mMinPitch;
    fPitchMax = mClass->mMaxPitch;
    fYawMin = mClass->mMinYaw;
    fYawMax = mClass->mMaxYaw;
}

// FUNCTION: SWBF2 0x005a5af0
bool MountedTurret::PostCollisionUpdate(float)
{
    if (mPilot && mPilot->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        EntitySoldier* pSoldier = (EntitySoldier*)mPilot->GetEntity();
        pSoldier->SetRenderPos(pSoldier->GetMatrix().GetTranslation());

        if (!netFrameLock && NetGame::IsEnabled() &&
            (NetGame::IsClient() || (NetGame::IsHost() && NetGame::IsOneWorld())) &&
            netIsLocalTurn)
        {
            if (netInterpolator.GetIndex(mInterpIndex, 1, pSoldier->mHandleId))
            {
                netInterpolator.SetNext(mInterpIndex, &mRootMatrix);
                netInterpolator.Prev(mInterpIndex);
                if (mAimer[0])
                {
                    netInterpolator.SetNext(mInterpIndex, &mAimer[0]->mDirection);
                }
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x005a5bf0
void MountedTurret::GetPilotMatrix(const PblMatrix& mIn, PblMatrix& mOut)
{
    RedPose* pPose = mPose;
    if (!mPilot)
    {
        return;
    }

    const ControllableClass* pClass = GetControllableClass();
    if (pClass->mPilotAnimation == SoldierAnimationBank::INVALID_ANIMATION)
    {
        mOut = mIn;
        return;
    }

    PblMatrix mMatrix;
    PblMatrix mOldRoot;
    mOldRoot.m[3][0] = mOldRoot.m[3][1] = mOldRoot.m[3][2] = mOldRoot.m[3][3] = 0.0f;
    const PblMatrix* pBone = 0;

    if (pPose && pClass->mPilotParentNodeHash)
    {
        PblMatrix mInverse = mIn.GetInverseFast();

        if (!mClass->mIsVirtual && pPose->Find(mClass->mMountHash))
        {
            mMatrix = mRootMatrix;
            if (!netFrameLock && NetGame::IsEnabled())
            {
                int iIndex = mInterpIndex;
                if (netInterpolator.GetIndex(iIndex, 1, static_cast<EntitySoldier*>(mPilot)->mHandleId) &&
                    iIndex == mInterpIndex)
                {
                    netInterpolator.GetMat(mInterpIndex, mMatrix);
                    mOldRoot = mRootMatrix;
                    mRootMatrix = mMatrix;
                }
            }
        }

        if (mAimer[0])
        {
            PblVector3 vOldDirection = mAimer[0]->mDirection;
            PblVector3 vDirection = vOldDirection;
            netInterpolator.GetVec(mInterpIndex, vDirection);
            mAimer[0]->mDirection = vDirection;
            mAimer[0]->SetupPose(mInverse, *pPose, true);
            mAimer[0]->mDirection = vOldDirection;
        }

        pBone = pPose->Find(pClass->mPilotParentNodeHash);
    }

    if (pClass->mAnimatedPilotNodeFlag)
    {
        D3DXMatrixMultiply(&mMatrix, pBone, &mIn);
    }
    else if (pBone)
    {
        PblMatrix mOffset;
        D3DXMatrixMultiply(&mOffset, &pClass->mPilotOffsetMatrix, pBone);
        D3DXMatrixMultiply(&mMatrix, &mOffset, &mIn);
    }
    else
    {
        D3DXMatrixMultiply(&mMatrix, &pClass->mPilotOffsetMatrix, &mIn);
    }

    if (mOldRoot.m[3][3] != 0.0f)
    {
        mRootMatrix = mOldRoot;
    }

    mOut = mMatrix;
}

// FUNCTION: SWBF2 0x005a5f50
bool MountedTurret::Update(float aDt)
{
    if (NetGame::IsSkipped(GetGameObject(), aDt))
    {
        return true;
    }

    Controllable::Update(aDt);

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
            pManager->SetAutoLoseLocked(2.0f, true, -1.0f, fLoseDist);
            pManager->SetTarget(0.5f, 0.8f, fLockDist, true);
            pManager->SetAuto(true, false);
            pManager->SetNumAttackerTargets(0);
            pManager->SetNumNearestTargets(64);

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

    const PblMatrix& parentMatrix = mParent->GetMatrix();
    const PblVector3& parentVelocity = mParent->GetVelocity();
    if (!CameraManager::sInstance->IsChaseMode(iJoystick))
    {
        SpaceDustEffect::Stop(iJoystick);
    }
    const PblVector3& vFront = parentMatrix.GetZAxis();
    if (vFront.x * parentVelocity.x + vFront.y * parentVelocity.y + vFront.z * parentVelocity.z > 20.0f)
    {
        SpaceDustEffect::Start(iJoystick);
    }

    if (mControlFire[0].IsDown())
    {
        ++gDebugCount;
    }

    if (mPilotType != PILOT_SELF && !(mPilotType == PILOT_VEHICLESELF && mPilot == this) && mPilot)
    {
        Controllable* pPilot = mPilot;
        if (mControlView.IsPressed() && netIsLocalTurn && !IsForcedThirdPerson() && !IsForcedFirstPerson() &&
            mClass->mForceMode != 1 && mClass->mForceMode != 2 && mTracker)
        {
            SwitchControlView(mPlayerId);
        }

        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            if (mControlUse.IsPressed() || mParent->IsDisabled() ||
                mParent->GetTeam() != pPilot->GetGameObject()->GetTeam())
            {
                SpaceDustEffect::Stop(iJoystick);
                if (mPilotType == PILOT_REMOTE)
                {
                    pPilot->ExitRemote();
                }
                else if (mPilotType == PILOT_VEHICLE || mPilotType == PILOT_VEHICLESELF)
                {

                    if (!(GetGameObject() && GetGameObject()->IsRtti(EntityFlyer::rttiHashEntityFlyer) &&
                          ((EntityFlyer*)GetGameObject())->mState != EntityFlyer::LANDED))
                    {
                        if (pPilot->mCharacter && pPilot->mCharacter->mRemote)
                        {
                            pPilot->ExitRemote();
                        }
                        pPilot->ExitVehicle(true, true);
                    }
                }
            }
            else if (mParent->GetControllableList().GetCount() > 1 && mControlSwitch[0] != 0.0f)
            {
                pPilot->CyclePosition(mControlSwitch[0] > 0.0f);
            }
        }
    }

    if (mParent->GetControllableList().GetCount() == 1)
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
            else if (mPlayerId >= 0 && NetGame::GetJoystickIndex(mCharacter->mPlayerId >= 0))
            {
                WeaponClass::sWeaponChangeUnableSound.Play(&mAimMatrix.GetTranslation(), 0, false, true);
            }
        }
    }

    float turn = 1.0f;
    if (mParent->IsAlive())
    {
        if (mPlayerId >= 0)
        {
            int iIndex = mWeaponIndex[0];
            if (iIndex >= 0 && mWeapon[iIndex] && mWeapon[iIndex]->mClass->mZoomMax > 1.0f)
            {
                if (mControlZoom.IsPressed() && netIsLocalTurn)
                {
                    mTargetInfo.mIsAiming = mWeapon[iIndex]->CycleZoom(mTargetInfo.mIsAiming);
                }
                if (mWeapon[mWeaponIndex[0]]->mClass->mZoomRate != 0.0f)
                {
                    mWeapon[mWeaponIndex[0]]->UpdateZoom(aDt, mControlMove);
                }
            }

            int iFirstPerson = GetFirstPersonIndex();
            if (iFirstPerson >= 0)
            {
                FirstPerson::Update(iFirstPerson, TYPE_COCKPIT, mClass->mFirstPersonModelHandle, this, 0);
            }
        }

        if (mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]])
        {
            turn = 1.0f / mWeapon[mWeaponIndex[0]]->mZoomTurnScale;
        }

        const PblVector3* pos = &mParent->GetMatrix().GetTranslation();
        const PblVector3* vel = &mParent->GetVelocity();
        int prevPitch = (int)(mPitch * 1000.0f);
        int prevYaw = (int)(mYaw * 1000.0f);

        float pitchRate;
        float turnRate;
        if (mPlayerId >= 0 && NetGame::IsMouseMode(mPlayerId))
        {
            pitchRate = mClass->mTurnRate;
            turnRate = mClass->mTurnRate;
        }
        else
        {
            pitchRate = mClass->mPitchRate;
            turnRate = mClass->mTurnRate;
            if (mPlayerId >= 0)
            {
                turnRate = (turnRate - pitchRate) * mTurnBuildup + pitchRate;
            }
        }
        float pitch = pitchRate * turn;
        turn *= turnRate;

        float fWant = -(GetPitchAdjusted() * pitch);
        float fStep = mClass->mPitchFilter * aDt;
        if (fWant > mOmegaPitch)
        {
            mOmegaPitch += fStep;
            if (mOmegaPitch > fWant)
            {
                mOmegaPitch = fWant;
            }
        }
        else
        {
            mOmegaPitch -= fStep;
            if (fWant > mOmegaPitch)
            {
                mOmegaPitch = fWant;
            }
        }

        fWant = -(GetTurnAdjusted() * turn);
        fStep = mClass->mTurnFilter * aDt;
        if (fWant > mOmegaYaw)
        {
            mOmegaYaw += fStep;
            if (mOmegaYaw > fWant)
            {
                mOmegaYaw = fWant;
            }
        }
        else
        {
            mOmegaYaw -= fStep;
            if (fWant > mOmegaYaw)
            {
                mOmegaYaw = fWant;
            }
        }

        if (mPlayerId >= 0)
        {
            PblMatrix mInverse = parentMatrix.GetInverseFast();
            PblVector3 aimLocal;
            D3DXVec3TransformNormal(&aimLocal, &mAimMatrix.GetZAxis(), &mInverse);
            float aimYaw = atan2(aimLocal.x, aimLocal.z);
            if (mClass->mShiftYaw && aimYaw < 0.0f)
            {
                aimYaw += 6.2831855f;
            }
            mYaw = aimYaw;
            mPitch = -asin(aimLocal.y);
            if (mClass->mShiftYaw && mYaw < 0.0f)
            {
                mYaw += 6.2831855f;
            }
        }

        float fAbsOmegaPitch = fabs(mOmegaPitch);
        mPitch += mOmegaPitch * aDt;
        float fAbsOmegaYaw = fabs(mOmegaYaw);
        mYaw += mOmegaYaw * aDt;

        float minYaw = mClass->mMinYaw;
        float maxYaw = mClass->mMaxYaw;
        float minPitch = mClass->mMinPitch;
        float maxPitch = mClass->mMaxPitch;
        if (minPitch > -3.1416926f && maxPitch < 3.1414928f)
        {
            if (minPitch > mPitch)
            {
                mPitch = minPitch;
            }
            else if (mPitch > maxPitch)
            {
                mPitch = maxPitch;
            }
        }
        else
        {
            while (mPitch > 3.1415927f)
            {
                mPitch -= 6.2831855f;
            }
            while (mPitch < -3.1415927f)
            {
                mPitch += 6.2831855f;
            }
        }
        if (mClass->mShiftYaw || (minYaw > -3.1416926f && maxYaw < 3.1414928f))
        {
            if (minYaw > mYaw)
            {
                mYaw = minYaw;
            }
            else if (mYaw > maxYaw)
            {
                mYaw = maxYaw;
            }
        }
        else
        {
            while (mYaw > 3.1415927f)
            {
                mYaw -= 6.2831855f;
            }
            while (mYaw < -3.1415927f)
            {
                mYaw += 6.2831855f;
            }
        }

        UpdateSound(prevPitch, (int)(mPitch * 1000.0f), fAbsOmegaPitch, pitch, mClass->mTurretPitchSoundPitch,
                    pos, vel, mClass->mTurretPitchSound, mTurretPitchSound, mTurretPitchSoundPitch,
                    mTurretPitchSoundGain);
        UpdateSound(prevYaw, (int)(mYaw * 1000.0f), fAbsOmegaYaw, turn, mClass->mTurretYawSoundPitch,
                    pos, vel, mClass->mTurretYawSound, mTurretYawSound, mTurretYawSoundPitch,
                    mTurretYawSoundGain);
    }

    PblVector3 mountPos = mClass->mMountPos;
    PblVector3 mountUp = mClass->mMountUp;
    if (mPose && mClass->mParentHash)
    {
        const PblMatrix* pParentBone = mPose->Find(mClass->mParentHash);
        if (pParentBone)
        {
            D3DXVec3TransformCoord(&mountPos, &mClass->mMountPos, pParentBone);
            D3DXVec3TransformNormal(&mountUp, &mClass->mMountUp, pParentBone);
        }
    }

    if (!mClass->mIsVirtual)
    {
        float fYaw = mClass->mYawTurnFactor * mYaw;
        float fCosYaw = cos(fYaw);
        float fSinYaw = sin(fYaw);
        float fPitch = mClass->mPitchTurnFactor * mPitch;
        float fCosPitch = cos(fPitch);
        PblVector3 rootDir(fCosPitch * fSinYaw, -sin(fPitch), fCosPitch * fCosYaw);
        mRootMatrix = DirectionMatrix(rootDir, mountUp, mountPos);

        if (mPose && mClass->mMountHash)
        {
            const PblMatrix* pOld = mPose->Find(mClass->mMountHash);
            if (pOld != &mRootMatrix)
            {
                if (pOld)
                {
                    mPose->Remove(mClass->mMountHash);
                }
                mPose->Store(mClass->mMountHash, &mRootMatrix);
            }
        }
    }

    if (mPilotType != PILOT_REMOTE)
    {
        if (mClass->mIsPilotAnimation9Pose)
        {

            float minPitch;
            float maxPitch;
            float minYaw;
            float maxYaw;
            GetPitchYawLimits(minPitch, maxPitch, minYaw, maxYaw);

            float fPitchBlend = 0.5f;
            if (maxPitch > minPitch)
            {
                fPitchBlend = ((mPitch - minPitch) / (maxPitch - minPitch) - 0.5f) * 2.0f;
                if (-1.0f > fPitchBlend)
                {
                    fPitchBlend = -1.0f;
                }
                else if (fPitchBlend > 1.0f)
                {
                    fPitchBlend = 1.0f;
                }
            }

            float fYawBlend;
            if (maxYaw > minYaw && maxYaw - minYaw < 4.712389f)
            {
                fYawBlend = ((mYaw - minYaw) / (maxYaw - minYaw) - 0.5f) * 2.0f;
                if (-1.0f > fYawBlend)
                {
                    fYawBlend = -1.0f;
                }
                else if (fYawBlend > 1.0f)
                {
                    fYawBlend = 1.0f;
                }
            }
            else
            {
                fYawBlend = mOmegaYaw;
                if (turn > 0.0f)
                {
                    fYawBlend = mOmegaYaw / turn;
                }
            }

            mClass->UpdatePilotAnimation(mPilot, mPose, parentMatrix, parentVelocity, aDt, fYawBlend + 4.0f,
                                         fPitchBlend > 0.0f ? fYawBlend + 7.0f : fYawBlend + 1.0f,
                                         fabs(fPitchBlend));
        }
        else
        {
            mClass->UpdatePilotAnimation(mPilot, mPose, parentMatrix, parentVelocity, aDt, -1.0f, -1.0f, -1.0f);
        }
    }

    float fCosYaw = cos(mYaw);
    float fSinYaw = sin(mYaw);
    float fCosPitch = cos(mPitch);
    PblVector3 aimDir(fCosPitch * fSinYaw, -sin(mPitch), fCosPitch * fCosYaw);
    if (mClass->mForceLockAimerToParentAimer && mParent->GetControllable())
    {
        const PblVector3& vParentDir = mParent->GetControllable()->mEyeDir;
        PblMatrix mInverse = parentMatrix.GetInverseFast();
        D3DXVec3TransformNormal(&aimDir, &vParentDir, &mInverse);
    }
    PblMatrix mDirection = DirectionMatrix(aimDir, mountUp, mountPos);
    PblMatrix aimMat;
    D3DXMatrixMultiply(&aimMat, &mDirection, &parentMatrix);
    mAimMatrix = aimMat;

    mEyePoint = mClass->mCameraSettings.CalcEyePoint(mAimMatrix);
    mEyeDir = mAimMatrix.GetZAxis();

    if (mPlayerId >= 0 && mTracker)
    {
        if (mTargetInfo.mIsAiming || mTracker->IsFirstPersonView())
        {
            mTargetInfo.mAimStart = mEyePoint;
        }
        else
        {

            PblVector3 vBack(-mEyeDir.x, -mEyeDir.y, -mEyeDir.z);
            PblMatrix mBack = DirectionMatrix(vBack, mAimMatrix.GetYAxis(), gVectorZero);
            PblQuaternion targetRot = PblQuaternion(mBack);
            float fSin;
            float fCos;
            PblMath::SinCos(&fSin, &fCos, -mClass->mCameraSettings.GetCameraTiltValue() * 0.5f);
            PblQuaternion tiltRot;
            tiltRot._fX = fSin * gVectorUnitX.x;
            tiltRot._fY = fSin * gVectorUnitX.y;
            tiltRot._fZ = fSin * gVectorUnitX.z;
            tiltRot._fS = fCos;
            PblQuaternion qRot = targetRot * tiltRot;
            PblQuaternion qUnit;
            D3DXMatrixRotationQuaternion(&aimMat, D3DXQuaternionNormalize(&qUnit, &qRot));
            aimMat.m[3][0] = gVectorZero.x;
            aimMat.m[3][1] = gVectorZero.y;
            aimMat.m[3][2] = gVectorZero.z;
            aimMat.m[3][3] = 1.0f;

            PblVector3& vStart = mTargetInfo.mAimStart;
            vStart = mClass->mCameraSettings.CalcCameraPoint(
                mClass->mTrackUseRoot ? mParent->GetMatrix() : mAimMatrix, aimMat);
            float t = mEyeDir.y * (mEyePoint.y - vStart.y) + mEyeDir.x * (mEyePoint.x - vStart.x) +
                      mEyeDir.z * (mEyePoint.z - vStart.z);
            vStart.x += mEyeDir.x * t;
            vStart.y += mEyeDir.y * t;
            vStart.z += mEyeDir.z * t;
        }

        CollisionObject* hitObj;
        const GameObject* ignoreObj[1] = { mParent };
        float dist = CollisionManager::RayHit(mTargetInfo.mAimStart, mEyeDir, 512.0f, hitObj, 0, ignoreObj, 1,
                                              154, true) * 512.0f;
        mTargetInfo.mAimPoint.x = mTargetInfo.mAimStart.x + mEyeDir.x * dist;
        mTargetInfo.mAimPoint.y = mTargetInfo.mAimStart.y + mEyeDir.y * dist;
        mTargetInfo.mAimPoint.z = mTargetInfo.mAimStart.z + mEyeDir.z * dist;
    }
    else if (mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]])
    {
        mTargetInfo.mAimStart = mEyePoint;
        if (!mClass->mIsControllable && mWeapon[mWeaponIndex[0]]->mClass->mLockOnRange > 0.0f)
        {

            float fRange = mWeapon[mWeaponIndex[0]]->mClass->mLockOnRange;
            CollisionObject* hitObj = 0;
            const GameObject* ignore[1] = { mParent };
            float t = CollisionManager::RayHit(mEyePoint, mEyeDir, fRange, hitObj, 0, ignore, 1, 0, false);
            mWeapon[mWeaponIndex[0]]->mTarget = hitObj ? hitObj->GetGameObject() : 0;
            mWeapon[mWeaponIndex[0]]->mTargetBodyID = -1;
            float dist = mWeapon[mWeaponIndex[0]]->mClass->mLockOnRange * t;
            mTargetInfo.mAimPoint.x = mEyePoint.x + mEyeDir.x * dist;
            mTargetInfo.mAimPoint.y = mEyePoint.y + mEyeDir.y * dist;
            mTargetInfo.mAimPoint.z = mEyePoint.z + mEyeDir.z * dist;
        }
        else
        {
            Weapon* pWeapon = mWeapon[mWeaponIndex[0]];
            float dist = 512.0f;
            if (pWeapon->mTarget)
            {
                const GameObject* pTarget = pWeapon->mTarget;
                PblVector3 targetPos = pTarget->GetTargetPoint(mEyePoint, mEyeDir, -1);
                float dx = targetPos.x - mEyePoint.x;
                float dy = targetPos.y - mEyePoint.y;
                float dz = targetPos.z - mEyePoint.z;
                dist = sqrtf(dy * dy + dx * dx + dz * dz);
            }
            mTargetInfo.mAimPoint.x = mEyePoint.x + mEyeDir.x * dist;
            mTargetInfo.mAimPoint.y = mEyePoint.y + mEyeDir.y * dist;
            mTargetInfo.mAimPoint.z = mEyePoint.z + mEyeDir.z * dist;
        }
    }

    if (mParent->IsAlive())
    {
        int iIndex = mWeaponIndex[0];
        if (iIndex >= 0 && mWeapon[iIndex] && mClass->mIsAutoFire[iIndex])
        {
            mControlFire[iIndex].Update(aDt, true);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x005a77c0
void MountedTurret::UpdateSound(int prevAngle, int angle, float fAngle, float fAngleMax, float minSoundPitch,
                                const PblVector3* const pos, const PblVector3* const vel, GameSound& sound,
                                GameSoundControllable& controllable, float& soundPitch, float& soundGain)
{
    if (prevAngle != angle)
    {
        if (fAngleMax <= 0.0f)
        {
            return;
        }
        float t = fAngle / fAngleMax;
        if (sound.GetGain() > soundGain)
        {
            soundGain = sound.GetGain() * 0.1f + soundGain;
        }
        if (!controllable.GetPlaying())
        {
            controllable.Play(&sound, pos, vel, true);

            controllable.GetVoiceVirtual();
            soundPitch = sound.GetPitch();
            soundGain = sound.GetGain();
        }
        if (Snd::VoiceVirtual* pVoice = controllable.GetVoiceVirtual())
        {
            float fPitch = ((1.0f - t) * minSoundPitch + t) * soundPitch;
            if (minSoundPitch > fPitch)
            {
                fPitch = minSoundPitch;
            }
            else if (fPitch > soundPitch)
            {
                fPitch = soundPitch;
            }
            pVoice->SetPitch(fPitch);

            float fGain = soundGain * t;
            if (0.0f > fGain)
            {
                fGain = 0.0f;
            }
            else if (fGain > 1.0f)
            {
                fGain = 1.0f;
            }
            static_cast<Snd::VoiceVirtualBus*>(pVoice)->SetGain(fGain);
        }
    }
    else if (controllable.GetPlaying())
    {
        Snd::VoiceVirtualBus* pVoice = static_cast<Snd::VoiceVirtualBus*>(controllable.GetVoiceVirtual());
        if (pVoice && pVoice->GetGain() > 0.0001f)
        {
            float fGain = pVoice->GetGain() - pVoice->GetGain() * 0.4f;
            if (0.0f > fGain)
            {
                fGain = 0.0f;
            }
            else if (fGain > 1.0f)
            {
                fGain = 1.0f;
            }
            pVoice->SetGain(fGain);
        }
        else
        {
            controllable.Stop(true);
        }
    }

    if (mPrevPilot != mPilot)
    {
        (mPilot ? mClass->mTurretActivateSound : mClass->mTurretDeactivateSound).Play(pos, vel, false, true);
        mPrevPilot = mPilot;
    }
    if (mPilot)
    {
        if (mTurretAmbience.GetPlaying())
        {
            mTurretAmbience.Update(&mClass->mTurretAmbientSound, pos, vel);
        }
        else
        {
            mTurretAmbience.Play(&mClass->mTurretAmbientSound, pos, vel, true);
        }
    }
    else
    {
        mTurretAmbience.Stop(true);
    }
}

// FUNCTION: SWBF2 0x005a7a20
bool MountedTurret::RenderTrackable(unsigned int uView) const
{
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

// FUNCTION: SWBF2 0x005a7a80
int MountedTurret::GetFirstPersonIndex()
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

// FUNCTION: SWBF2 0x005a7b00
int MountedTurret::GetJoystickIndex()
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

// GLOBAL: SWBF2 0x01e579e0
static int sWeaponSection;

// FUNCTION: SWBF2 0x005a7b70
void MountedTurret::ActivatePhysics()
{
    ActivateThread(-20 - mClass->mHierarchyLevel, PER_TURN);
    mPostCollision.ActivateThread(-15, PER_TURN);
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mAimer[i])
        {
            mAimer[i]->ActivatePhysics();
        }
    }
}

// FUNCTION: SWBF2 0x005a7d00
void MountedTurret::Render(const PblMatrix& aRootMatrix, const PblMatrix& aInvRoot, RedPose& aPose,
                           unsigned int uiRenderFlags, PblMatrix& aSaveMatrix) const
{

    MountedTurret* pThis = const_cast<MountedTurret*>(this);

    aSaveMatrix.m[3][0] = 0.0f;
    aSaveMatrix.m[3][1] = 0.0f;
    aSaveMatrix.m[3][2] = 0.0f;
    aSaveMatrix.m[3][3] = 0.0f;

    if (!RenderTrackable(mPlayerId))
    {
        if (mClass->mMountHash)
        {
            const PblMatrix* pMount = aPose.Find(mClass->mMountHash);
            if (pMount)
            {
                if (pMount != &mRootMatrix)
                {
                    pThis->mRootMatrix = *pMount;
                    aPose.Remove(mClass->mMountHash);
                    aPose.Store(mClass->mMountHash, &mRootMatrix);
                }
                aSaveMatrix = mRootMatrix;
                pThis->mRootMatrix.m[3][0] = 0.0f;
                pThis->mRootMatrix.m[3][1] = 0.0f;
                pThis->mRootMatrix.m[3][2] = -1000.0f;
                pThis->mRootMatrix.m[3][3] = 1.0f;
                RemoveChildren(GetGameObject(), aPose,
                               GetGameObject()->GetModel()->GetRedModel(0)->GetBoneIndex(mClass->mMountHash));
            }
        }
        return;
    }

    const PblMatrix* pMount = 0;
    if (!mClass->mIsVirtual)
    {
        pMount = aPose.Find(mClass->mMountHash);
        if (pMount)
        {
            aSaveMatrix = mRootMatrix;
            PblMatrix smoothedMat = mRootMatrix;
            if (!netFrameLock && NetGame::IsEnabled() && mPilot &&
                mPilot->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
            {
                int idx = mInterpIndex;
                if (netInterpolator.GetIndex(idx, 1, ((EntitySoldier*)mPilot->GetEntity())->mHandleId) &&
                    idx == mInterpIndex)
                {
                    netInterpolator.GetMat(mInterpIndex, smoothedMat);
                }
            }
            pThis->mRootMatrix = smoothedMat;
            if (pMount != &mRootMatrix)
            {
                aPose.Remove(mClass->mMountHash);
                aPose.Store(mClass->mMountHash, &mRootMatrix);
                pMount = &mRootMatrix;
            }
        }
    }

    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (!mAimer[i])
        {
            continue;
        }

        bool bSmoothed = false;
        PblVector3 vOldDir;
        if (i == 0 && !netFrameLock && NetGame::IsEnabled() && mPilot &&
            mPilot->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            int idx = mInterpIndex;
            if (netInterpolator.GetIndex(idx, 1, ((EntitySoldier*)mPilot->GetEntity())->mHandleId) &&
                idx == mInterpIndex)
            {
                bSmoothed = true;
                vOldDir = mAimer[0]->mDirection;
                PblVector3 newDir = vOldDir;
                netInterpolator.GetVec(mInterpIndex, newDir);
                mAimer[0]->mDirection = newDir;
            }
        }
        mAimer[i]->SetupPose(aInvRoot, aPose, true);
        if (bSmoothed)
        {
            mAimer[i]->mDirection = vOldDir;
        }
    }

    if (pMount)
    {
        const PblVector3& vVelocity = mParent->GetVelocity();
        D3DXVec3TransformCoord(&pThis->mTurretWorldTranslation, &pMount->GetTranslation(), &mParent->GetMatrix());
        pThis->mTurretPitchSound.Update(&mClass->mTurretPitchSound, &mTurretWorldTranslation, &vVelocity);
        pThis->mTurretYawSound.Update(&mClass->mTurretYawSound, &mTurretWorldTranslation, &vVelocity);
    }
}

// FUNCTION: SWBF2 0x005a8280
void MountedTurret::PostRender(const PblMatrix& aSaveMatrix)
{
    if (aSaveMatrix.m[3][3] != 0.0f)
    {
        mRootMatrix = aSaveMatrix;
    }
}

// FUNCTION: SWBF2 0x005a82d0
bool MountedTurret::UpdateIndirect(float fDeltaTime)
{
    AILowLevel& lowLevel = UnitControllerPtr(mCtrl)->GetLowLevel();
    if (lowLevel.UpdateIndirect(fDeltaTime) && mCtrl)
    {
        ControllableHeading heading = lowLevel.mHeading;
        lowLevel.ProcessFire(fDeltaTime, &heading);
        float fPitch = GetPitchAdjusted();
        float fTurn = GetTurnAdjusted();
        AIUtil::CalculateHeadingControls(this, fDeltaTime, lowLevel.mDest, heading, mClass->mTurnRate, &fTurn,
                                         mClass->mPitchRate, &fPitch, false);
        mControlPitch = fPitch;
        mControlTurn = fTurn;
    }
    return true;
}

// FUNCTION: SWBF2 0x005a83d0
void MountedTurret::SetupController()
{
    Controller* pController = 0;
    if (mPlayerId >= 0)
    {
        pController = new PlayerController(this);
        Track(mPilot->mPlayerId);
    }
    else if (mPilot && !(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        pController = new UnitStationaryController(this);
    }
    if (pController)
    {
        AddController(pController, mPilot->mPlayerId < 0);
    }
    ClearControllable();
}

// FUNCTION: SWBF2 0x005a8510
PilotDeathType MountedTurret::GetPilotDeathType()
{
    return static_cast<ControllableClass*>(mClass)->mPilotDeathType;
}

// FUNCTION: SWBF2 0x005a88b0
MountedTurretClass::MountedTurretClass(unsigned int uHash)
    : EntityClass(uHash)
{
    mCameraSettings.mCameraMoveTensionL = FLT_MAX;
    mCameraSettings.mCameraMoveTensionR = FLT_MAX;
    mCameraSettings.mCameraMoveTensionU = FLT_MAX;
    mCameraSettings.mCameraMoveTensionD = FLT_MAX;
    mCameraSettings.mCameraMoveTensionF = FLT_MAX;
    mCameraSettings.mCameraMoveTensionB = FLT_MAX;
    mCameraSettings.mCameraAimTension = FLT_MAX;

    mMinYaw = -10.0f;
    mMaxYaw = 10.0f;
    mMinPitch = -10.0f;
    mMaxPitch = 10.0f;

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
    sWeaponSection = 0;
    mWeaponCount = 0;

    mTurnRate = 2.0f;
    mTurnFilter = 10.0f;
    mPitchRate = 0.5f;
    mPitchFilter = 10.0f;
    mMountHash = 0;
    mParentHash = 0;
    mHierarchyLevel = 0;
    mMountPos.x = 0.0f;
    mMountPos.y = 0.0f;
    mMountPos.z = 0.0f;
    mMountUp.x = 0.0f;
    mMountUp.y = 1.0f;
    mMountUp.z = 0.0f;
    mYawTurnFactor = 1.0f;
    mPitchTurnFactor = 1.0f;
    mShiftYaw = false;
    mIsControllable = true;
    for (int i = 0; i < 4; ++i)
    {
        mIsAutoFire[i] = false;
    }
    mIsVirtual = false;
    mTrackUseRoot = false;
    mForceLockAimerToParentAimer = false;
    mTurretYawSoundPitch = 0.8f;
    mTurretPitchSoundPitch = 0.8f;
    mFirstPersonModelHandle = -1;
}

MountedTurretClass::~MountedTurretClass()
{
}

// SYNTHETIC: SWBF2 0x005a8b40
// MountedTurretClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005a8bb0
EntityClass* MountedTurretClass::Derive(unsigned int)
{
    return 0;
}

// FUNCTION: SWBF2 0x005a8bc0
Entity* MountedTurretClass::Build(const EntityDesc&)
{
    return 0;
}

// FUNCTION: SWBF2 0x005a8bd0
bool MountedTurretClass::SetProperty(unsigned int uHash, const char* pcValue, const RedModel* pModel)
{
    switch (uHash)
    {
    case 0x0F77C4E8:
    case 0x5433F589:
        sscanf(pcValue, "%f", &mTurnRate);
        break;
    case 0xDE721CB4:
        sscanf(pcValue, "%f", &mTurnFilter);
        break;
    case 0xB9B8ABC3:
    case 0xB5534BE0:
        sscanf(pcValue, "%f", &mPitchRate);
        break;
    case 0x9B0CA8C3:
        sscanf(pcValue, "%f", &mPitchFilter);
        break;
    case 0x0EBB87C8:
        sscanf(pcValue, "%f", &mYawTurnFactor);
        break;
    case 0x0B2FC2F9:
        sscanf(pcValue, "%f", &mPitchTurnFactor);
        break;
    case 0x2C3E8078:
        sscanf(pcValue, "%f %f", &mMinYaw, &mMaxYaw);
        mMinYaw *= 0.017453292f;
        mMaxYaw *= 0.017453292f;

        mShiftYaw = mMinYaw > 0.0f && mMinYaw <= 3.1415927f &&
                    mMaxYaw >= 3.1415927f && mMaxYaw < 6.2831855f;
        break;
    case 0x3403B139:
        sscanf(pcValue, "%f %f", &mMinPitch, &mMaxPitch);
        mMinPitch *= 0.017453292f;
        mMaxPitch *= 0.017453292f;
        break;
    case 0xF2AA4A90:
    {
        float x, y, z;
        sscanf(pcValue, "%f %f %f", &x, &y, &z);
        mMountPos.x = x;
        mMountPos.y = y;
        mMountPos.z = z;
        break;
    }
    case 0xB69A7CB4:
        mMountHash = PblTEMPHash(pcValue);
        if (pModel)
        {
            PblMatrix mat;
            mParentHash = pModel->GetParentBoneAndOffsetMatrix(mMountHash, mat);
            if (mParentHash)
            {
                mMountPos = *(const PblVector3*)&mat.m[3][0];
                mMountUp = *(const PblVector3*)&mat.m[1][0];
            }
            for (unsigned int uBone = mMountHash; uBone; uBone = pModel->GetParentBone(uBone))
            {
                ++mHierarchyLevel;
            }
        }
        break;
    case 0xB371FC9A:
        mIsControllable = atoi(pcValue) != 0;
        break;
    case 0x9D39F68A:
        mIsAutoFire[sWeaponSection] = atoi(pcValue) != 0;
        break;
    case 0x5D967EBC:
        mIsVirtual = atoi(pcValue) != 0;
        break;
    case 0xD6C12C3B:
        mTrackUseRoot = atoi(pcValue) != 0;
        break;
    case 0x865061FC:
        mForceLockAimerToParentAimer = atoi(pcValue) != 0;
        break;
    case 0x1FA43B99:
        mTurretYawSound.SetID(pcValue, true, false);
        break;
    case 0x1503F425:
        sscanf(pcValue, "%f", &mTurretYawSoundPitch);
        break;
    case 0x28F016D2:
        mTurretPitchSound.SetID(pcValue, true, false);
        break;
    case 0x4ED7A4D0:
        sscanf(pcValue, "%f", &mTurretPitchSoundPitch);
        break;
    case 0x4C39154D:
        mTurretYawSound.SetID(pcValue, true, false);
        mTurretPitchSound.SetID(pcValue, true, false);
        break;
    case 0xE9D0FB69:
        mTurretActivateSound.SetID(pcValue, true, false);
        break;
    case 0x8E5DA6E8:
        mTurretAmbientSound.SetID(pcValue, true, false);
        break;
    case 0xF84AA204:
        mTurretDeactivateSound.SetID(pcValue, true, false);
        break;
    case 0x8822CB3C:
        mTurretStart.SetID(pcValue, true, false);
        break;
    case 0xC13068B6:
        mTurretStop.SetID(pcValue, true, false);
        break;
    case 0x6363D774:
        mFirstPersonModelHandle = FirstPerson::AddGeometry(pcValue);
        break;
    case 0xD0329E80:
        sWeaponSection = atoi(pcValue) - 1;
        break;
    case 0x6F332041:
    case 0xFBF47DBA:
    {
        mWeaponClass[sWeaponSection] = WeaponClass::Find(PblHash(pcValue));
        int iCount = sWeaponSection + 1;
        if (mWeaponCount > iCount)
        {
            iCount = mWeaponCount;
        }
        mWeaponCount = iCount;
        break;
    }
    case 0x16472328:
        sscanf(pcValue, "%d", &mWeaponChannel[sWeaponSection]);
        break;
    case 0xDC19BD13:
    {
        int iAmmo = 0;
        sscanf(pcValue, "%d", &iAmmo);
        WeaponAmmoDesc& ammo = mWeaponAmmo[sWeaponSection];
        ammo.bFlagShareAmmo = 0;
        ammo.bFlagAmmoInfinite = iAmmo <= 0;
        if (!ammo.bFlagAmmoInfinite)
        {
            ammo.uiAmmo = iAmmo;
        }
        break;
    }
    case 0x8585BB8A:
    {
        int iWeapon = 0;
        sscanf(pcValue, "%d", &iWeapon);
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
    case 0x9455A446:
    {
        int iWeapon = 0;
        sscanf(pcValue, "%d", &iWeapon);
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
    default:
        if (mCameraSettings.SetProperty(uHash, pcValue))
        {
            break;
        }

        if (mAimerClass[sWeaponSection].SetProperty(uHash, pcValue, pModel, 0))
        {
            for (AimerClass* pAimer = &mAimerClass[sWeaponSection]; pAimer; pAimer = pAimer->mNextAimerClass)
            {
                if (pAimer->mHierarchyLevel <= mHierarchyLevel)
                {
                    pAimer->mHierarchyLevel = mHierarchyLevel + 1;
                }
            }
            break;
        }
        return ControllableClass::SetProperty(uHash, pcValue, pModel);
    }
    return true;
}

// SYNTHETIC: SWBF2 0x005a93f4
// [thunk]:MountedTurret::GetGameObject`adjustor{36}'

// SYNTHETIC: SWBF2 0x005a9404
// [thunk]:MountedTurret::GetEntity`adjustor{12}'

// SYNTHETIC: SWBF2 0x005a940c
// [thunk]:MountedTurret::`vector deleting destructor' adjustor{36}

// SYNTHETIC: SWBF2 0x005a9414
// [thunk]:MountedTurret::`vector deleting destructor' adjustor{12}

// SYNTHETIC: SWBF2 0x005a93ec
// [thunk]:MountedTurret::GetGameObject`adjustor{36}'
