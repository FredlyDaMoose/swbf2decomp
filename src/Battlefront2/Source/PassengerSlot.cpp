#include "PassengerSlot.h"
#include "PebbleFL/Common/PblVector.h"

#include "NetGame.h"
#include "PlayerController.h"
#include "UnitController.h"
#include "UnitStationaryController.h"
#include "AILowLevel.h"
#include "GameObject.h"
#include "Character.h"
#include "Tracker.h"
#include "FirstPerson.h"
#include "PowerupUtility.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);

// GLOBAL: SWBF2 0x01ef7ca0
// SYNTHETIC: SWBF2 0x00405da0
// `dynamic initializer for 'PassengerSlot::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769290
// `dynamic atexit destructor for 'PassengerSlot::sMemoryPool''
MemoryPool PassengerSlot::sMemoryPool("PassengerSlot", 0x1f0);

// GLOBAL: SWBF2 0x01ef7c90
// SYNTHETIC: SWBF2 0x00405dc0
// `dynamic initializer for 'PassengerSlot::rttiHashPassengerSlot''
PblHash PassengerSlot::rttiHashPassengerSlot("PassengerSlot");
char PassengerSlot::rttiNamePassengerSlot[] = "PassengerSlot";

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x0060fd00
PassengerSlotClass::PassengerSlotClass(unsigned int uHash)
    : EntityClass(uHash)
{
    mCameraSettings.mCameraMoveTensionL = 50.0f;
    mCameraSettings.mCameraMoveTensionR = 50.0f;
    mCameraSettings.mCameraMoveTensionU = 50.0f;
    mCameraSettings.mCameraMoveTensionD = 50.0f;
    mCameraSettings.mCameraMoveTensionF = 50.0f;
    mCameraSettings.mCameraMoveTensionB = 50.0f;
    mCameraSettings.mCameraAimTension = 50.0f;
    mAddHealth = 15.0f;
    mAddAmmo = 1.0f;
    mPowerupDelay = 1.0f;
    mMountMatrix = _IdentityMatrix;
}

// SYNTHETIC: SWBF2 0x0060fde0
// PassengerSlotClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0060fe10
EntityClass* PassengerSlotClass::Derive(unsigned int uHash)
{
    return 0;
}

// FUNCTION: SWBF2 0x0060fe20
Entity* PassengerSlotClass::Build(const EntityDesc& desc)
{
    return 0;
}

// FUNCTION: SWBF2 0x0060fe30
bool PassengerSlotClass::SetProperty(unsigned int uProperty, const char* pszValue, const RedModel* pModel)
{
    if (uProperty == 0xBFBBAB62)
    {
        if (pModel)
        {
            PblMatrix mat;
            if (pModel->GetParentBoneAndOffsetMatrix(PblTEMPHash(pszValue), mat))
            {
                mMountMatrix = mat;
            }
        }
    }
    else if (!mCameraSettings.SetProperty(uProperty, pszValue))
    {
        return ControllableClass::SetProperty(uProperty, pszValue, pModel);
    }
    return true;
}

float GetDefaultFOV();

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

// FUNCTION: SWBF2 0x0060fab0
void PassengerSlot::ActivatePhysics()
{
    ActivateThread(-20, PER_TURN);
}

// FUNCTION: SWBF2 0x0060fac0
void PassengerSlot::DeactivatePhysics()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x0060fad0
bool PassengerSlot::UpdateIndirect(float fDeltaTime)
{
    reinterpret_cast<AILowLevel*>(UnitControllerPtr(mCtrl)->mLowLevel)->UpdateIndirect(fDeltaTime);
    return true;
}

// FUNCTION: SWBF2 0x0060faf0
void PassengerSlot::SetupController()
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

// FUNCTION: SWBF2 0x0060fc20
CameraTrackSetting* PassengerSlot::GetCameraTrackSettings()
{
    return &mClass->mCameraSettings;
}

// FUNCTION: SWBF2 0x0060fc30
void PassengerSlot::SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime)
{
    PblVector3 vDir(-mEyeDir.x, -mEyeDir.y, -mEyeDir.z);
    rMatrix = DirectionMatrix(vDir, *(const PblVector3*)mAimMatrix.m[1], mEyePoint);
}

// FUNCTION: SWBF2 0x0060fcd0
void PassengerSlot::SetupCameraZoomFactor(float& rZoom)
{
    rZoom = 1.0f;
}

// FUNCTION: SWBF2 0x0060fce0
void PassengerSlot::SetupCameraFOV(float& rFOV)
{
    rFOV = GetDefaultFOV();
}

// FUNCTION: SWBF2 0x0060f560
PassengerSlot::PassengerSlot(GameObject* pParent, PassengerSlotClass* pClass, RedPose*)
    : EntityEx(0, pClass), Controllable(pClass ? static_cast<ControllableClass*>(pClass) : 0, -1),
      mPowerupTimer(0.0f), mParent(pParent), mClass(pClass)
{
    D3DXMatrixMultiply(&mAimMatrix, &mClass->mMountMatrix, &mParent->GetMatrix());
    mEyePoint = mClass->mCameraSettings.CalcEyePoint(mAimMatrix);
    mEyeDir = *reinterpret_cast<const PblVector3*>(mAimMatrix.m[2]);
}

// SYNTHETIC: SWBF2 0x0060f530
// PassengerSlot::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x0060ff08
// [thunk]:PassengerSlot::`scalar deleting destructor' adjustor{36}
// SYNTHETIC: SWBF2 0x0060ff10
// [thunk]:PassengerSlot::`scalar deleting destructor' adjustor{12}
// FUNCTION: SWBF2 0x0060f6a0
PassengerSlot::~PassengerSlot()
{
}

// FUNCTION: SWBF2 0x0060f460
bool PassengerSlot::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashPassengerSlot || uRtti == EntityEx::rttiHashEntityEx ||
           uRtti == Entity::rttiHashEntity;
}

// FUNCTION: SWBF2 0x0060f490
unsigned int PassengerSlot::GetDerivedRtti() const { return rttiHashPassengerSlot; }

// FUNCTION: SWBF2 0x0060f4a0
const char* PassengerSlot::GetDerivedRttiName() const { return rttiNamePassengerSlot; }

// SYNTHETIC: SWBF2 0x0060fef8
// [thunk]:PassengerSlot::GetEntity adjustor{12}
// FUNCTION: SWBF2 0x0060f4b0
Entity* PassengerSlot::GetEntity() { return this; }

// SYNTHETIC: SWBF2 0x0060ff00
// [thunk]:PassengerSlot::GetEntity const adjustor{12}
// FUNCTION: SWBF2 0x0060f4c0
const Entity* PassengerSlot::GetEntity() const { return this; }

// SYNTHETIC: SWBF2 0x0060fee8
// [thunk]:PassengerSlot::GetGameObject adjustor{36}
// FUNCTION: SWBF2 0x0060f4d0
GameObject* PassengerSlot::GetGameObject() { return mParent; }

// SYNTHETIC: SWBF2 0x0060fef0
// [thunk]:PassengerSlot::GetGameObject const adjustor{36}
// FUNCTION: SWBF2 0x0060f4e0
const GameObject* PassengerSlot::GetGameObject() const { return mParent; }

// FUNCTION: SWBF2 0x0060f4f0
Trackable* PassengerSlot::GetTrackable() { return this; }

// FUNCTION: SWBF2 0x0060f500
const Controllable* PassengerSlot::GetControllable() const { return this; }

// FUNCTION: SWBF2 0x0060f510
Controllable* PassengerSlot::GetControllable() { return this; }

// FUNCTION: SWBF2 0x0060f520
const PblMatrix& PassengerSlot::GetMatrix() const { return mAimMatrix; }

// FUNCTION: SWBF2 0x0060f700
void PassengerSlot::Init() {}

// FUNCTION: SWBF2 0x0060f780
ControllableClass* PassengerSlot::GetControllableClass()
{
    return mClass ? static_cast<ControllableClass*>(mClass) : 0;
}

// FUNCTION: SWBF2 0x0060f760
const ControllableClass* PassengerSlot::GetControllableClass() const
{
    return mClass ? static_cast<const ControllableClass*>(mClass) : 0;
}

// FUNCTION: SWBF2 0x0060f710
void PassengerSlot::Kill()
{
    if (mTracker && mTracker->IsFirstPersonView())
    {
        int iJoystick = NetGame::GetJoystickIndex(mPlayerId);
        if (iJoystick >= 0)
        {
            FirstPerson::Activate(iJoystick, false);
            FirstPerson::Update(iJoystick, TYPE_COCKPIT, -2, 0, 0);
        }
    }
    Controllable::Kill();
}

// FUNCTION: SWBF2 0x0060f7a0
bool PassengerSlot::Update(float fDeltaTime)
{
    Controllable::Update(fDeltaTime);

    Controllable* pPilot = mPilot;
    if (mPilotType == PILOT_SELF || (mPilotType == PILOT_VEHICLESELF && pPilot == this))
    {
        pPilot = 0;
    }

    if (pPilot)
    {
        if (mControlView.IsPressed() && !IsForcedThirdPerson() && !IsForcedFirstPerson() &&
            mClass->mForceMode != 1 && mClass->mForceMode != 2 && mTracker)
        {
            SwitchControlView(mPlayerId);
        }

        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            if (mControlUse.IsPressed() || mParent->mDisableTime > 0.0f)
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
            else if (mParent->GetControllableList().GetCount() > 1 && mControlSwitch[0] != 0.0f)
            {
                pPilot->CyclePosition(mControlSwitch[0] > 0.0f);
            }
        }
    }

    D3DXMatrixMultiply(&mAimMatrix, &mClass->mMountMatrix, &mParent->GetMatrix());
    mEyePoint = mClass->mCameraSettings.CalcEyePoint(mAimMatrix);
    mEyeDir = *reinterpret_cast<const PblVector3*>(mAimMatrix.m[2]);
    mPowerupTimer += fDeltaTime;
    if (mPowerupTimer >= mClass->mPowerupDelay)
    {
        mPowerupTimer -= mClass->mPowerupDelay;
        if (pPilot)
        {
            GameObject* pPilotObject = mPilot->GetGameObject();
            if (pPilotObject)
            {
                PowerupUtility::ApplyHealth(pPilotObject, mClass->mAddHealth, 0, false);
                PowerupUtility::ApplyAmmo(pPilotObject, mClass->mAddAmmo, false);
            }
        }
    }
    if (pPilot)
    {
        mClass->UpdatePilotAnimation(pPilot, 0, mParent->GetMatrix(), mParent->GetVelocity(),
                                     fDeltaTime, -1.0f, -1.0f, -1.0f);
    }
    return true;
}
