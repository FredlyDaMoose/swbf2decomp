#include <new>
#include <stdio.h>
#include <string.h>

#include "EntityCarrier.h"
#include "CommandFlyer.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "GameModel.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"
#include "Character.h"
#include "CollisionObject.h"
#include "EntitySquadron.h"
#include "FirstPerson.h"
#include "Tracker.h"
#include "NetGame.h"
#include "WeaponCannonInstance.h"
#include "CameraManager.h"
#include "ChaseCamera.h"
#include "CollisionManager.h"
#include "GameLoop.h"
#include "MotionBlurEffect.h"
#include "Trackable.h"
#include <float.h>
float GetDefaultFOV();
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut,
    const PblVector3* pV, const PblMatrix* pM);

// FUNCTION: SWBF2 0x004b3dd0
CameraTrackSetting* EntityFlyer::GetCameraTrackSettings()
{
    return &mClass->mCameraSettings;
}

// FUNCTION: SWBF2 0x004b5f00
void EntityFlyer::SetupCameraZoomFactor(float& rZoom)
{
    rZoom = 1.0f;
    if (mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0 &&
        mWeapon[mWeaponIndex[0]])
    {
        rZoom = mWeapon[mWeaponIndex[0]]->mZoomTurnScale;
    }
}

// FUNCTION: SWBF2 0x004b5c10
void EntityFlyer::SetupCameraFOV(float& rFOV)
{
    if (!mIsAlive)
    {
        rFOV = GetDefaultFOV();
        return;
    }

    const bool bFirstPerson = mTargetInfo.mIsAiming ||
        (mTracker && mTracker->IsFirstPersonView());
    ControllableClass* pControlClass = GetControllableClass();
    float fDefaultFOV = bFirstPerson ? pControlClass->mFirstPersonFOV :
                                     pControlClass->mThirdPersonFOV;
    float fEffectFOV = bFirstPerson ? mClass->mFOVEffect1st :
                                    mClass->mFOVEffect3rd;
    float fBlend = 0.0f;
    if (mGetSpeedSpeed > 0.0f && fEffectFOV != 0.0f)
    {
        float fMaxSpeed = GetFlyerMaxSpeed();
        if (fMaxSpeed < mClass->mBoostFlyerSpeed)
        {
            fMaxSpeed = mClass->mBoostFlyerSpeed;
        }
        fBlend = mGetSpeedSpeed / fMaxSpeed;
        if (fBlend < 0.0f)
        {
            fBlend = 0.0f;
        }
        else if (fBlend > 1.0f)
        {
            fBlend = 1.0f;
        }
        rFOV = fDefaultFOV * (1.0f - fBlend) + fEffectFOV * fBlend;
    }
    else
    {
        rFOV = fDefaultFOV;
    }

    if (!bFirstPerson)
    {
        PblVector3 vOffset(
            mClass->mFOVEFFectMinCamOffset3rd.x * (1.0f - fBlend) +
                mClass->mFOVEFFectMaxCamOffset3rd.x * fBlend,
            mClass->mFOVEFFectMinCamOffset3rd.y * (1.0f - fBlend) +
                mClass->mFOVEFFectMaxCamOffset3rd.y * fBlend,
            mClass->mFOVEFFectMinCamOffset3rd.z * (1.0f - fBlend) +
                mClass->mFOVEFFectMaxCamOffset3rd.z * fBlend);
        if (vOffset.x != 0.0f || vOffset.y != 0.0f || vOffset.z != 0.0f)
        {
            mClass->mCameraSettings.mCameraTrackOffset = vOffset;
        }
    }
}

// SYNTHETIC: SWBF2 0x00496ea0
// EntityCarrier::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00497941
// [thunk]:EntityCarrier::`scalar deleting destructor'`adjustor{576}'
// SYNTHETIC: SWBF2 0x0049794c
// [thunk]:EntityCarrier::`scalar deleting destructor'`adjustor{148}'
// SYNTHETIC: SWBF2 0x00497962
// [thunk]:EntityCarrier::`scalar deleting destructor'`adjustor{320}'
// SYNTHETIC: SWBF2 0x0049796d
// [thunk]:EntityCarrier::`scalar deleting destructor'`adjustor{972}'
// SYNTHETIC: SWBF2 0x00497978
// [thunk]:EntityCarrier::`scalar deleting destructor'`adjustor{600}'
// SYNTHETIC: SWBF2 0x00497983
// [thunk]:EntityCarrier::`scalar deleting destructor'`adjustor{12}'
// SYNTHETIC: SWBF2 0x0049798b
// [thunk]:EntityCarrier::`scalar deleting destructor'`adjustor{512}'

// FUNCTION: SWBF2 0x00497060
EntityCarrier::~EntityCarrier()
{
}

// SYNTHETIC: SWBF2 0x00478579
// [thunk]:EntityFlyer::GetGameObject`adjustor{320}'
// SYNTHETIC: SWBF2 0x004785ad
// [thunk]:EntityFlyer::GetGameObject`adjustor{12}'
// SYNTHETIC: SWBF2 0x004785bd
// [thunk]:EntityFlyer::GetEntity`adjustor{12}'
// SYNTHETIC: SWBF2 0x004785db
// [thunk]:EntityFlyer::GetControllable`adjustor{600}'
// SYNTHETIC: SWBF2 0x004785e6
// [thunk]:EntityFlyer::GetControllable`adjustor{600}'
// SYNTHETIC: SWBF2 0x004785fc
// [thunk]:EntityFlyer::`scalar deleting destructor'`adjustor{960}'

// FUNCTION: SWBF2 0x00496f60
EntityCarrier::EntityCarrier(EntityCarrierClass* pClass, const EntityDesc& desc)
    : EntityFlyer(pClass, desc)
{
    memset(mCargoArray, 0, sizeof(mCargoArray));
    mCargoCount = pClass->mCargoCount;
    static_cast<VehicleEngine*>(this)->TurnOn(
        mClass ? static_cast<VehicleEngineClass*>(mClass) : 0,
        mCollisionSphere.GetCenter(), mVelocity, 1.0f);
}

// SYNTHETIC: SWBF2 0x00497957
// [thunk]:EntityCarrier::Kill`adjustor{256}'

// FUNCTION: SWBF2 0x00497110
void EntityCarrier::Kill()
{
    if (mTracker && mTracker->IsFirstPersonView())
    {
        int iPlayer = NetGame::GetJoystickIndex(mPlayerId);
        if (iPlayer >= 0)
        {
            FirstPerson::Activate(iPlayer, false);
            FirstPerson::Update(iPlayer, TYPE_COCKPIT, -2, 0, 0);
        }
    }
    for (int i = 0; i < mCargoCount; ++i)
    {
        GameObject* pObject = mCargoArray[i].mObject;
        if (pObject)
        {
            Controllable* pControllable = pObject->GetControllable();
            if (pControllable)
            {
                pControllable->Kill();
            }
            DetachCargo(i);
        }
    }
    EntityFlyer::Kill();
}

// FUNCTION: SWBF2 0x004971d0
bool EntityCarrier::Update(float fDeltaTime)
{
    if (!EntityFlyer::Update(fDeltaTime))
    {
        return false;
    }

    for (int i = 0; i < mCargoCount; ++i)
    {
        GameObject* pObject = mCargoArray[i].mObject;
        if (!pObject)
        {
            continue;
        }

        PblMatrix matrix = mMatrix;
        PblVector3 position;
        D3DXVec3TransformCoord(&position, &mCargoArray[i].mOffset, &matrix);
        matrix.m[3][0] = position.x;
        matrix.m[3][1] = position.y;
        matrix.m[3][2] = position.z;
        matrix.m[3][3] = 1.0f;
        pObject->SetMatrix(matrix);
        pObject->SetVelocity(mVelocity);
    }
    return true;
}

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);

namespace
{

    void EntityFlyerUpdateCollisionSphere(EntityFlyer* pFlyer)
    {
        const PblMatrix& matrix = pFlyer->mMatrix;
        const GameModel* pModel = pFlyer->mModel;
        PblVector3 center;
        D3DXVec3TransformCoord(&center, &pModel->_mModel->_BoundingSphere.GetCenter(), &matrix);
        CollisionObject* pCollision = static_cast<CollisionObject*>(pFlyer);
        pCollision->SetCollisionCenter(center);
        pCollision->RecalculateAABB();
    }
}

// FUNCTION: SWBF2 0x00477720
void EntityFlyer::SetPosition(const PblVector3& vPos)
{
    PblMatrix& matrix = mMatrix;
    matrix.m[3][0] = vPos.x;
    matrix.m[3][1] = vPos.y;
    matrix.m[3][2] = vPos.z;
    matrix.m[3][3] = 1.0f;
    EntityFlyerUpdateCollisionSphere(this);
}

// FUNCTION: SWBF2 0x004777c0
void EntityFlyer::SetMatrix(const PblMatrix& mMatrix)
{
    this->mMatrix = mMatrix;
    EntityFlyerUpdateCollisionSphere(this);
}

// FUNCTION: SWBF2 0x00477870
bool EntityFlyer::GetFollowingPath() const
{
    return mFollowingPath && mState == FLYING && mIsAlive &&
           mAIEng.ai.mFreeFlyTime <= 0.0f && !mInRoll && !mInFlip;
}

// FUNCTION: SWBF2 0x004778d0
Trackable* EntityFlyer::GetTrackable()
{
    return this ? static_cast<Trackable*>(static_cast<Controllable*>(this)) : 0;
}

// FUNCTION: SWBF2 0x004779a0
unsigned int EntityFlyer::GetAllegiance()
{
    const Character* pCharacter = mCharacter;
    return pCharacter ? pCharacter->mTeamNumber : 0;
}

// FUNCTION: SWBF2 0x004779c0
bool EntityFlyer::CanJoinSquadron() const
{
    return !EntitySquadron::InSquadron() && GetFollowingPath();
}

// FUNCTION: SWBF2 0x00477b00
int EntityFlyerClass::GetVehicleType()
{
    return mVehicleType;
}

// FUNCTION: SWBF2 0x00477b10
int EntityFlyerClass::GetWeaponCount()
{
    return mWeaponCount;
}

// FUNCTION: SWBF2 0x00477b20
const WeaponClass* EntityFlyerClass::GetWeaponClass(int iIndex)
{
    return mWeaponClass[iIndex];
}

// FUNCTION: SWBF2 0x00477b40
float EntityFlyerClass::GetTimeRequiredToEject()
{
    return mTimeRequiredToEject;
}

// FUNCTION: SWBF2 0x00477b50
float EntityFlyerClass::GetReboardTime()
{
    return mTimeTilReboard;
}

// FUNCTION: SWBF2 0x00477b60
const void* EntityFlyerClass::GetControllableClass()
{
    return this ? static_cast<const ControllableClass*>(this) : 0;
}

// GLOBAL: SWBF2 0x01ebb8ec
// SYNTHETIC: SWBF2 0x00402960
// `dynamic initializer for 'EntityCarrier::rttiHashEntityCarrier''
PblHash EntityCarrier::rttiHashEntityCarrier("EntityCarrier");
char EntityCarrier::rttiNameEntityCarrier[] = "EntityCarrier";

// GLOBAL: SWBF2 0x01ebb8f0
// SYNTHETIC: SWBF2 0x00402940
// `dynamic initializer for 'EntityCarrier::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f10
// `dynamic atexit destructor for 'EntityCarrier::sMemoryPool''
MemoryPool EntityCarrier::sMemoryPool("EntityCarrier", 0x1df0);

// FUNCTION: SWBF2 0x00496e20
bool EntityCarrier::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityCarrier || EntityFlyer::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00496e70
unsigned int EntityCarrier::GetDerivedRtti() const
{
    return rttiHashEntityCarrier;
}

// FUNCTION: SWBF2 0x00496e80
const char* EntityCarrier::GetDerivedRttiName() const
{
    return rttiNameEntityCarrier;
}

// GLOBAL: SWBF2 0x01ebb8e8
// SYNTHETIC: SWBF2 0x00402970
// `dynamic initializer for 'EntityCarrierClass::rttiHashEntityCarrierClass''
PblHash EntityCarrierClass::rttiHashEntityCarrierClass("EntityCarrierClass");
char EntityCarrierClass::rttiNameEntityCarrierClass[] = "EntityCarrierClass";

// FUNCTION: SWBF2 0x00496f00
bool EntityCarrierClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityCarrierClass || EntityFlyerClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00496f40
unsigned int EntityCarrierClass::GetDerivedRtti()
{
    return rttiHashEntityCarrierClass;
}

// FUNCTION: SWBF2 0x00496f50
const char* EntityCarrierClass::GetDerivedRttiName()
{
    return rttiNameEntityCarrierClass;
}

// FUNCTION: SWBF2 0x00497300
bool EntityCarrier::AttachCargo(int, GameObject* pGameObject)
{

    if (mCargoArray[0].mObject.mObject)
    {
        if (mCargoArray[0].mObject.mObject->mHandleId != mCargoArray[0].mObject.mSavedHandleId)
        {
            mCargoArray[0].mObject.mObject = 0;
            mCargoArray[0].mObject.mSavedHandleId = 0;
        }
        else
        {
            return false;
        }
    }

    EntityCarrierClass* pClass = GetClass();
    const PblVector3* pOffset = &pGameObject->GetAttachOffset();

    mCargoArray[0].mOffset.x = pClass->mCargoInfo[0].mOffset.x - pOffset->x;
    mCargoArray[0].mOffset.y = pClass->mCargoInfo[0].mOffset.y - pOffset->y;
    mCargoArray[0].mOffset.z = pClass->mCargoInfo[0].mOffset.z - pOffset->z;
    mCargoArray[0].mObject.mObject = (GameObject*)pGameObject;
    mCargoArray[0].mObject.mSavedHandleId = ((GameObject*)pGameObject)->mHandleId;

    static_cast<CollisionObject*>(pGameObject)->mParent = static_cast<CollisionObject*>(this);

    UpdateLandedHeight();

    const PblVector3& vVelocity = GetVelocity();
    GetClass()->mSoundCargoPickup.Play(&mMatrix.GetTranslation(), &vVelocity, true, true);
    return true;
}

// FUNCTION: SWBF2 0x00497410
bool EntityCarrier::DetachCargo(int iSlot)
{

    EntityCarrier::CargoSlot* pSlot = &mCargoArray[iSlot];
    if (!pSlot->mObject.mObject)
    {
        return false;
    }

    if (pSlot->mObject.mObject->mHandleId != pSlot->mObject.mSavedHandleId)
    {
        pSlot->mObject.mObject = 0;
        pSlot->mObject.mSavedHandleId = 0;
        return false;
    }

    static_cast<CollisionObject*>(pSlot->mObject.mObject)->mParent = 0;
    pSlot->mObject.mObject = 0;
    pSlot->mObject.mSavedHandleId = 0;

    UpdateLandedHeight();

    const PblVector3& vVelocity = GetVelocity();
    GetClass()->mSoundCargoDropoff.Play(&mMatrix.GetTranslation(), &vVelocity, true, true);
    return true;
}

// FUNCTION: SWBF2 0x004974b0
void EntityCarrier::UpdateLandedHeight()
{
    float& fLandedHeight = mLandedHeight;
    fLandedHeight = GetClass()->mLandedHeight;

    for (int i = 0; i < mCargoCount; ++i)
    {
        CargoSlot* pSlot = &mCargoArray[i];
        if (!pSlot->mObject.mObject)
        {
            continue;
        }

        if (pSlot->mObject.mObject->mHandleId != pSlot->mObject.mSavedHandleId)
        {
            pSlot->mObject.mObject = 0;
            pSlot->mObject.mSavedHandleId = 0;
            continue;
        }

        const GameModel* pModel = pSlot->mObject.mObject->GetModel();
        if (!pModel)
        {
            continue;
        }

        float fCandidate = -pSlot->mOffset.y - pModel->_mModel->_TotalBoundingBox._vMin.y;
        fLandedHeight = fLandedHeight > fCandidate ? fLandedHeight : fCandidate;
    }
}

// FUNCTION: SWBF2 0x00497560
void EntityCarrier::ActivatePhysics()
{
    static_cast<Controllable*>(this)->ActivateThread(-1, PER_TURN);
}

// FUNCTION: SWBF2 0x00497580
EntityCarrierClass::EntityCarrierClass(unsigned int uHash)
    : EntityFlyerClass(uHash)
{
    mPilotType = PILOT_NONE;

    for (int i = 0; i < 4; ++i)
    {
        mCargoInfo[i].mHash = 0;
        mCargoInfo[i].mOffset.x = 0.0f;
        mCargoInfo[i].mOffset.y = 0.0f;
        mCargoInfo[i].mOffset.z = 0.0f;
    }

    mCargoCount = 0;
}

// SYNTHETIC: SWBF2 0x004975e0
// EntityCarrierClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004976a0
EntityCarrierClass::~EntityCarrierClass()
{
}

// SYNTHETIC: SWBF2 0x004780b0
// CommandFlyerClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00478160
CommandFlyerClass::~CommandFlyerClass()
{
}

// SYNTHETIC: SWBF2 0x004785f1
// [thunk]:CommandFlyerClass::`scalar deleting destructor'`adjustor{4256}'

// FUNCTION: SWBF2 0x00497610
EntityCarrierClass::EntityCarrierClass(EntityCarrierClass* pSrc, unsigned int uHash)
    : EntityFlyerClass(pSrc, uHash)
{
    memcpy(mCargoInfo, pSrc->mCargoInfo, sizeof(mCargoInfo));
    mCargoCount = pSrc->mCargoCount;
}

// FUNCTION: SWBF2 0x004976b0
void EntityCarrierClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty > 0x8897DB28u)
    {
        if (uProperty != 0x910A89FCu)
        {
            EntityFlyerClass::SetProperty(uProperty, pszValue);
            return;
        }

        float x, y, z;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mCargoInfo[mCargoCount].mHash = 0;
        mCargoInfo[mCargoCount].mOffset.x = x;
        mCargoInfo[mCargoCount].mOffset.y = y;
        mCargoInfo[mCargoCount].mOffset.z = z;
        ++mCargoCount;
    }
    else if (uProperty == 0x8897DB28u)
    {
        mSoundCargoDropoff.SetID(pszValue, true, false);
    }
    else if (uProperty == 0x3E2C4DA4u)
    {
        mCargoInfo[mCargoCount].mHash = PblTEMPHash(pszValue);

        if (mModel)
        {
            const RedSkeleton* pSkeleton = mModel->GetRedModel(0)->GetSkeleton();
            if (pSkeleton)
            {
                for (unsigned int i = 0; i < 128; ++i)
                {
                    if (pSkeleton->GetBoneNameHash(i) == mCargoInfo[mCargoCount].mHash)
                    {

                        const PblMatrix& mBone = pSkeleton->GetDefaultPoseMatrix(i);
                        mCargoInfo[mCargoCount].mOffset = *(const PblVector3*)&mBone.m[3][0];
                        ++mCargoCount;
                        break;
                    }
                }
            }
        }
    }
    else if (uProperty == 0x759C8F54u)
    {
        mSoundCargoPickup.SetID(pszValue, true, false);
    }
    else
    {
        EntityFlyerClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x00497830
EntityClass* EntityCarrierClass::Derive(unsigned int uHash)
{
    return (EntityClass*)new EntityCarrierClass(this, uHash);
}

extern unsigned int gCarrierBuildLimit;
extern unsigned int gCarrierBuildCount;

// FUNCTION: SWBF2 0x004978a0
Entity* EntityCarrierClass::Build(const EntityDesc& desc)
{
    if (gCarrierBuildCount >= gCarrierBuildLimit)
    {
        return 0;
    }

    EntityCarrier* pCarrier = new EntityCarrier(this, desc);
    if (pCarrier)
    {
        CollisionManager::AddRigidBody(static_cast<CollisionObject*>(pCarrier));
        pCarrier->ActivatePhysics();
        static_cast<EntityGeometry*>(pCarrier)->BuildEffects();
    }
    return (Entity*)pCarrier;
}

void* EntityCarrier::operator new(unsigned int uSize)
{
    return sMemoryPool.Allocate(uSize);
}

extern "C" PblMatrix* __stdcall D3DXMatrixRotationX(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationZ(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

namespace FLRenderer
{
unsigned int GetViewportCount();
}

// FUNCTION: SWBF2 0x004b5010
void EntityFlyer::SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime)
{
    if (mClass->IsGuidedMissile() &&
        (!mPilot || mEnergyBar.GetEnergy() <= 0.0f || mCrashTimer != 0.0f))
    {
        int iJoystick = GetJoystickIndex();
        if (iJoystick >= 0)
        {
            ChaseCamera* pCamera = CameraManager::sInstance->mChaseCamera[iJoystick];
            if (pCamera)
            {
                pCamera->ClearShake();
            }
        }
        PblVector3 vCamera = rMatrix.GetTranslation();
        const PblMatrix& mBody = static_cast<CollisionObject*>(this)->GetMatrix();
        PblVector3 vBack(-mBody.m[2][0], -mBody.m[2][1], -mBody.m[2][2]);
        const PblVector3& vCenter = GetCollisionCenter();
        PblVector3 vOffset(vCamera.x - vCenter.x, vCamera.y - vCenter.y, vCamera.z - vCenter.z);
        if (vOffset.y * vBack.y + vOffset.x * vBack.x + vOffset.z * vBack.z -
                mClass->mGuidedMissileDeathCamMinDistance < 0.0f)
        {
            float fPullback = mClass->mGuidedMissileDeathCamPullbackSpeed * fDeltaTime;
            const PblMatrix& mNow = static_cast<CollisionObject*>(this)->GetMatrix();
            PblVector3 vDir(-mNow.m[2][0], -mNow.m[2][1], -mNow.m[2][2]);
            const GameObject* pIgnore = GetGameObject();
            CollisionObject* pHit;
            if (CollisionManager::RayHit(vCamera, vDir, fPullback, pHit, 0, &pIgnore, 1, 0x90, false) >= 1.0f)
            {
                rMatrix.m[3][0] = vDir.x * fPullback + vCamera.x;
                rMatrix.m[3][1] = vDir.y * fPullback + vCamera.y;
                rMatrix.m[3][2] = vDir.z * fPullback + vCamera.z;
                rMatrix.m[3][3] = 1.0f;
            }
        }
        return;
    }

    const PblMatrix& mTarget = GetSmoothedMatrix();
    if (IsAlive() && (mTargetInfo.mIsAiming || (mTracker && mTracker->IsFirstPersonView())))
    {
        mTrickCameraInterpDir = 0.0f;
        mTrickCameraInterpDetach = 0.0f;
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
        PblMatrix mUnbank;
        PblMatrix mUnpitch;
        D3DXMatrixRotationZ(&mUnbank, -mBankRollAngle);
        D3DXMatrixMultiply(&mUnbank, &mUnbank, D3DXMatrixRotationX(&mUnpitch, -mBankPitchAngle));
        PblMatrix mBody;
        D3DXMatrixMultiply(&mBody, &mUnbank, &mTarget);

        if (mTowCableWalker != 0)
        {
            CameraTrackSetting& rSettings = mClass->mCameraSettings;
            const PblVector3 vSavedOffset = rSettings.mCameraTrackOffset;
            PblVector3 vPos = mBody.GetTranslation();
            const PblVector3& vWalker = mTowCableWalker->GetCollisionObject()->GetPosition();
            PblVector3 vToCamera(vPos.x - vWalker.x, vPos.y - vWalker.y, vPos.z - vWalker.z);
            D3DXVec3Normalize(&vPos, &vToCamera);
            const float fSide = vPos.x * mTarget.m[0][0] + vPos.y * mTarget.m[0][1] + vPos.z * mTarget.m[0][2];
            rSettings.mCameraTrackOffset.y = 0.5f;
            rSettings.mCameraTrackOffset.z = 8.0f;
            rSettings.mCameraTrackOffset.x = fSide > 0.0f ? 2.0f : -2.0f;
            const float fRadius = GetCollisionRadius();
            rSettings.mCameraTrackOffset.x *= fRadius;
            rSettings.mCameraTrackOffset.y = fRadius * rSettings.mCameraTrackOffset.y;
            rSettings.mCameraTrackOffset.z = fRadius * rSettings.mCameraTrackOffset.z;
            rSettings.SetupCameraTrackMatrix(this, rMatrix, fDeltaTime, mBody, mEyeDir, 1.0f, 1.0f, this);
            rSettings.mCameraTrackOffset = vSavedOffset;
            mTrickCameraInterpDir = 0.0f;
            mTrickCameraInterpDetach = 0.0f;
        }
        else
        {
            CameraTrackSetting settings = mClass->mCameraSettings;
            if (mInFlip)
            {
                mTrickCameraInterpDir = 1.0f;
                mTrickCameraDetach = 0.2f;
                mTrickCameraInterpDetach = 1.0f;
            }
            else if (mSideRoll.x != 0.0f || mSideRoll.y != 0.0f)
            {
                mTrickCameraDetach = 0.48f;
                mTrickCameraInterpDetach = 1.0f;
            }
            else if (mState == CRASHING || mState == CRASHED)
            {
                mTrickCameraDetach = 0.25f;
                mTrickCameraInterpDetach = 1.0f;
            }

            if (mTrickCameraInterpDir > 0.0f)
            {
                settings.mCameraAimTension = (1.0f - mTrickCameraInterpDir) * 50.0f + mTrickCameraInterpDir * 20.0f;
            }
            else
            {
                settings.mCameraAimTension = FLT_MAX;
            }
            settings.mCameraTiltValue = 0.087665f;
            if (FLRenderer::GetViewportCount() == 2)
            {
                settings.mCameraMoveTensionU *= 1.5f;
                settings.mCameraMoveTensionD *= 1.5f;
                settings.mCameraTrackCenter.y *= 0.6666667f;
                settings.mCameraTrackOffset.y *= 0.6666667f;
            }

            const float fDetach = mTrickCameraDetach * mTrickCameraInterpDetach + (1.0f - mTrickCameraInterpDetach);
            if (fDetach < 0.99f)
            {
                int iJoystick = GetJoystickIndex();
                ChaseCamera* pCamera = iJoystick >= 0 ? CameraManager::sInstance->mChaseCamera[iJoystick] : 0;
                if (pCamera)
                {
                    pCamera->mTimer = fDetach;
                }
                if (NetGame::IsEnabled())
                {
                    for (unsigned int i = 0; i < GameLoop::GetNumCameras(); ++i)
                    {
                        if (NetGame::GetSpectator(i) == mPlayerId)
                        {
                            ChaseCamera* pSpectator = CameraManager::sInstance->mChaseCamera[i];
                            if (pSpectator)
                            {
                                pSpectator->mTimer = fDetach;
                            }
                        }
                    }
                }
            }

            const PblVector3 vAhead(0.0f, 0.0f, 5.0f);
            PblVector3 vAim;
            if (mTrickCameraInterpDir > 0.0f)
            {
                PblVector3 vPoint;
                D3DXVec3TransformCoord(&vPoint, &vAhead, &mBody);
                PblVector3 vToPoint(vPoint.x - rMatrix.m[3][0], vPoint.y - rMatrix.m[3][1],
                                    vPoint.z - rMatrix.m[3][2]);
                PblVector3 vNormal;
                D3DXVec3Normalize(&vNormal, &vToPoint);
                vAim = vNormal;
            }
            else
            {
                vAim = mEyeDir;
            }

            if (mGetSpeedSpeed > 0.0001f)
            {
                settings.SetupAmbientShift(mBody, mAmbientShift, mAmbientShiftTarget, mAmbientShiftSpeed,
                                           mAmbientShiftSpeedMax, fDeltaTime);
            }
            else
            {
                mAmbientShift.x = 0.0f;
                mAmbientShift.y = 0.0f;
                mAmbientShift.z = 0.0f;
            }

            const float fInterp = mTrickCameraInterpDir;
            const PblVector3 vBlendAim(vAim.x * fInterp + mEyeDir.x * (1.0f - fInterp),
                                       vAim.y * fInterp + mEyeDir.y * (1.0f - fInterp),
                                       vAim.z * fInterp + mEyeDir.z * (1.0f - fInterp));
            settings.SetupCameraTrackMatrix(this, rMatrix, fDeltaTime, mBody, vBlendAim, 1.0f, 1.0f, this);

            const float fDecay = fDeltaTime * 0.9f;
            mTrickCameraInterpDir -= fDecay;
            if (mTrickCameraInterpDir < 0.0f)
            {
                mTrickCameraInterpDir = 0.0f;
            }
            mTrickCameraInterpDetach -= fDecay;
            if (mTrickCameraInterpDetach < 0.0f)
            {
                mTrickCameraInterpDetach = 0.0f;
            }
        }
    }

    if (!NetGame::IsEnabled())
    {
        const float fBlurStart = mClass->mBlurStartV;
        const float fBlurEnd = mClass->mMaxFlyerSpeed > mClass->mBoostFlyerSpeed ? mClass->mMaxFlyerSpeed
                                                                                 : mClass->mBoostFlyerSpeed;
        const float fSpeed = mGetSpeedSpeed;
        int iJoystick = GetJoystickIndex();
        if (iJoystick >= 0 && fSpeed > fBlurStart)
        {
            float fBlend = (fSpeed - fBlurStart) / (fBlurEnd - fBlurStart);
            if (fBlend < 0.0f)
            {
                fBlend = 0.0f;
            }
            else if (fBlend > 1.0f)
            {
                fBlend = 1.0f;
            }
            if (mClass->mBlurEffect * fBlend > 0.0f)
            {
                MotionBlurEffect::Start(0, mClass->mBlurEffect * fBlend, iJoystick);
            }
        }
    }
}

// GLOBAL: SWBF2 0x01ebb924
unsigned int gCarrierBuildLimit = 0u;

// GLOBAL: SWBF2 0x01ebb92c
unsigned int gCarrierBuildCount = 0u;
