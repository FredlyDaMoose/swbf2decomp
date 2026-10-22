#include <float.h>
#include "PebbleFL/Common/PblVector.h"
#include <stdio.h>
#include <stdlib.h>

#include "Controllable.h"
#include "ControllerManager.h"
#include "AILowLevel.h"
#include "Character.h"
#include "LockOnManager.h"
#include "NetGame.h"
#include "UnitController.h"
#include "WeaponCannonInstance.h"
#include "WeaponShield.h"
#include "CollisionObject.h"
#include "GameObject.h"
#include "MedalsMgr.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "Aimer.h"
#include "EntitySoldier.h"

class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x00484730
ControllableClass::ControllableClass()
{
    mCombatInterrupts = true;
    mIgnoreHintNodes = false;
    mShakeScale = 1.0f;
    mBobPath.numPoints = 0;
    mCockpitTension = 0.0f;
    mLocalNameID = 0;
    mForceMode = 0;
    mThirdPersonFOV = mFirstPersonFOV = GetDefaultFOV();
    mPilotParentNodeHash = 0;
    mPilotOffsetMatrix = _IdentityMatrix;
    mPilotAnimation = SoldierAnimationBank::INVALID_ANIMATION;
    mAnimatedPilotNodeFlag = false;
    mPilotDeathType = PILOTDEATH_FALL;
    mPilotType = PILOT_VEHICLE;
    mIsPilotExposed = true;
    mIsPilotAnimation9Pose = false;
    mCanCapturePosts = false;
    mCanEnterVehicles = false;
    mControlsUnit = false;
    mAnimatesUnitAsTurret = false;
}

UnitController* UnitControllerPtr(Controller* pController);

extern int netTurnsPerSecond;

namespace AIUtil
{
GameObject* GetAlivePtr(PblHandle<GameObject> const& hObj, bool bIncludeBuildings);
}

static inline unsigned int GameObjectHandleId(const GameObject* pObject)
{
    return pObject->mHandleId;
}

// FUNCTION: SWBF2 0x004833b0
float Controllable::GetSoundRadius() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x00483510
GameObject* LockOnManager::GetLockedTarget() const
{
    PblHandle<GameObject>& hLocked = const_cast<PblHandle<GameObject>&>(mLockedObj);
    if (hLocked.mObject && GameObjectHandleId(hLocked.mObject) != hLocked.mSavedHandleId)
    {
        hLocked.mObject = 0;
        hLocked.mSavedHandleId = 0;
    }
    return hLocked.mObject;
}

// FUNCTION: SWBF2 0x00483540
void LockOnManager::ForceNewTargetLock(GameObject* pObject)
{
    mForceNewLock = 5;
    mForceNewLockTarget.mObject = pObject;
    mForceNewLockTarget.mSavedHandleId = pObject ? GameObjectHandleId(pObject) : 0;
}

// FUNCTION: SWBF2 0x00483580
unsigned int LockOnManager::GetTargetLockedChangedTurn() const
{
    return mLockedObjChangedTurn;
}

// FUNCTION: SWBF2 0x00483590
LockOnManager* LockOnManager::GetByPlayerIndex(unsigned int uIndex)
{
    return sLockOnManagers[uIndex];
}

// FUNCTION: SWBF2 0x004835a0
Controllable::Controllable(ControllableClass* pClass, int iPlayerId)
    : mControlMove(0.0f),
      mControlStrafe(0.0f),
      mControlTurn(0.0f),
      mControlPitch(0.0f),
      mAIThread(this),
      mCtrl(0),
      mCharacter(0),
      mPilot((pClass->mPilotType == PILOT_SELF || pClass->mPilotType == PILOT_VEHICLESELF) ? this : 0),
      mPlayerId(iPlayerId),
      mInitialPlayerId(iPlayerId),
      mIndirectControlFlag(false),
      mEnemyLockedOnMeState(0.0f),
      mEnemyLockedOnMeTimestamp(0.0f),
      mEnemyLockedOnMeMissile(0),
      mEnemyLockedOnMeDistance(FLT_MAX),
      mPilotType(pClass->mPilotType)
{
    mCollisionObj.mObject = 0;
    mCollisionObj.mSavedHandleId = 0;
    mTargetLockedObj.mObject = 0;
    mTargetLockedObj.mSavedHandleId = 0;
    for (int i = 0; i < 2; ++i)
    {
        mReticuleTarget[i].mObject = 0;
        mReticuleTarget[i].mSavedHandleId = 0;
        mAIReticuleTarget[i].mObject = 0;
        mAIReticuleTarget[i].mSavedHandleId = 0;
    }
    ClearControllable();
    Initialize();
    NetGame::AddLateFence(mPlayerId);
}

// FUNCTION: SWBF2 0x004837c0
void Controllable::ClearControllable()
{
    for (int i = 0; i < 2; ++i)
    {
        mReticuleTarget[i].mObject = 0;
        mReticuleTarget[i].mSavedHandleId = 0;
        mReticuleAffiliation[i] = 0;
    }

    mControlMove = 0.0f;
    mControlStrafe = 0.0f;
    mControlTurn = 0.0f;
    mControlPitch = 0.0f;
    for (int i = 0; i < 2; ++i)
    {
        mControlSwitch[i] = 0.0f;
        mTriggerControlSwitch[i].Clear();
        mControlFire[i].Clear();
    }
    mWpnChannel = 0;
    mControlReload.Clear();
    mControlJump.Clear();
    mControlCrouch.Clear();
    mControlSprint.Clear();
    mControlUse.Clear();
    mControlZoom.Clear();
    mControlView.Clear();
    mControlLockTarget.Clear();
    mControlSquadCommand.Clear();
    mControlSpecialMove.Clear();
    mControlThrustFwd.Clear();
    mControlThrustBack.Clear();
    mControlStrafeLeft.Clear();
    mControlStrafeRight.Clear();

    mWpnDistFactor = 1.0f;
    mAnimNameId = 0;
    mTurnBuildup = 0.0f;
    mPitchBuildup = 0.0f;
    mTargetLockedObj.mObject = 0;
    mTargetLockedObj.mSavedHandleId = 0;
    mAllowedLockTypes = 0;
    mTurnAdjusted = 2.0f;
    mPitchAdjusted = 2.0f;
    mTurnAuto = 0.0f;
    mPitchAuto = 0.0f;
    mLockBreakTimer = 0.0f;
    mUsingTurnAdjusted = false;
    mCollisionObj.mObject = 0;
    mCollisionObj.mSavedHandleId = 0;
    mEnemyLockedOnMeState = 0.0f;
    mEnemyLockedOnMeTimestamp = 0.0f;
    mEnemyLockedOnMeMissile = 0;
    mEnemyLockedOnMeDistance = FLT_MAX;

    if (UnitControllerPtr(mCtrl))
    {
        reinterpret_cast<AILowLevel*>(UnitControllerPtr(mCtrl)->mLowLevel)->ClearControllable();
    }
}

// FUNCTION: SWBF2 0x004839e0
void Controllable::SetSwitch(float fValue, int iIndex)
{
    if (mTriggerControlSwitch[iIndex].IsPressed())
    {
        mControlSwitch[iIndex] = fValue;
    }
    else
    {
        mControlSwitch[iIndex] = 0.0f;
    }
}

// FUNCTION: SWBF2 0x00483a10
Controllable* Controllable::GetEnteredRemoteOrVehicleIfNotMe()
{
    if (!mCharacter)
    {
        return 0;
    }
    Controllable* pEntered = mCharacter->mRemote;
    if (!pEntered)
    {
        pEntered = mCharacter->mVehicle;
        if (!pEntered)
        {
            return 0;
        }
    }
    return pEntered != this ? pEntered : 0;
}

// FUNCTION: SWBF2 0x00483a40
void Controllable::ResetControls()
{
    mControlMove = 0.0f;
    mControlStrafe = 0.0f;
    mControlTurn = 0.0f;
    mControlPitch = 0.0f;
    mAnimNameId = 0;
    mWpnDistFactor = 1.0f;
    if (UnitControllerPtr(mCtrl))
    {
        reinterpret_cast<AILowLevel*>(UnitControllerPtr(mCtrl)->mLowLevel)->ResetControls();
    }
}

// FUNCTION: SWBF2 0x00483ab0
void Controllable::Initialize()
{
    if (UnitControllerPtr(mCtrl))
    {
        reinterpret_cast<AILowLevel*>(UnitControllerPtr(mCtrl)->mLowLevel)->Initialize();
    }
    mPlayAnim = false;
}

// SYNTHETIC: SWBF2 0x004850ef
// [thunk]:Controllable::`scalar deleting destructor' adjustor{24}
// FUNCTION: SWBF2 0x00483af0
Controllable::~Controllable()
{
    ReleaseController();
}

// FUNCTION: SWBF2 0x00483ba0
void Controllable::Kill()
{
    NetGame::AddLateFence(mPlayerId);
    if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mPilot)
    {
        PilotType eType = mPilotType;
        if (eType == PILOT_VEHICLESELF)
        {
            eType = (mPilot != this) ? PILOT_VEHICLE : PILOT_SELF;
        }
        switch (eType)
        {
        case PILOT_SELF:
            if (mCharacter)
            {
                if (mCharacter->mVehicle)
                {
                    mCharacter->mVehicle->SetPilot(0);
                    mCharacter->SetVehicle(0);
                }
                if (mCharacter->mRemote)
                {
                    ExitRemote();
                }
                mCharacter->Died();
                SetCharacter(0);
            }
            break;
        case PILOT_VEHICLE:
        {
            bool bAttached = false;
            if (GetGameObject())
            {
                if (GetGameObject()->GetCollisionObject())
                {
                    bAttached = ((CollisionObject*)(mPilot->GetGameObject()->GetCollisionObject()))->GetCollisionType() == COLL_ATTACHED;
                }
                mPilot->GetGameObject()->InstantDeath(GetGameObject()->mDamageOwner, false);
            }
            mPilot->Kill();
            if (!bAttached && !(NetGame::IsEnabled() && NetGame::IsClient()))
            {
                delete mPilot;
            }
            break;
        }
        case PILOT_REMOTE:
            mPilot->ExitRemote();
            break;
        }
        mPilot = 0;
    }
    ClearControllable();
    ReleaseController();
}

// FUNCTION: SWBF2 0x00483d50
void Controllable::SetExitVehicle(const CommandPost* pPost)
{
    if (mPilot && mPilot != this)
    {
        mPilot->SetExitVehicle(pPost);
    }
    if (UnitControllerPtr(mCtrl))
    {
        reinterpret_cast<AILowLevel*>(UnitControllerPtr(mCtrl)->mLowLevel)->SetExitVehicle(pPost);
    }
}

// FUNCTION: SWBF2 0x00483da0
void Controllable::AddController(Controller* pController, bool bIndirect)
{
    if (mCtrl)
    {
        return;
    }
    mCtrl = pController;
    SetIndirectControl(bIndirect);
    if (NetGame::IsEnabled() && NetGame::IsClient() && mPlayerId >= 0)
    {
        int iCount = GetWeaponCount();
        for (int i = 0; i < iCount; ++i)
        {
            Weapon* pWeapon = GetWeaponPtr(i);
            if (pWeapon)
            {
                pWeapon->ActivatePhysics();
            }
        }
    }
    ControllerManager::AddController(mCtrl);
}

// FUNCTION: SWBF2 0x00483e30
void Controllable::ReleaseController()
{
    if (mCtrl)
    {
        ControllerManager::RemoveController(mCtrl);
        mCtrl->KillController();
        delete mCtrl;
        mCtrl = 0;
        SetIndirectControl(false);
    }
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        int iCount = GetWeaponCount();
        for (int i = 0; i < iCount; ++i)
        {
            Weapon* pWeapon = GetWeaponPtr(i);
            if (pWeapon && !pWeapon->IsRtti(WeaponShield::GetRtti()))
            {
                pWeapon->DeactivatePhysics();
            }
        }
    }
}

// FUNCTION: SWBF2 0x00483ef0
void Controllable::SetIndirectControl(bool bIndirect)
{
    Thread& rThread = mAIThread;
    if (!mIndirectControlFlag)
    {
        if (bIndirect)
        {
            rThread.ActivateThread(10, PER_TURN);
        }
    }
    else if (!bIndirect)
    {
        rThread.DeactivateThread();
    }
    mIndirectControlFlag = bIndirect;
}

// FUNCTION: SWBF2 0x00483f50
void Controllable::SetCurrentCollision(GameObject* pObject)
{
    mCollisionObj.mObject = pObject;
    mCollisionObj.mSavedHandleId = pObject ? GameObjectHandleId(pObject) : 0;
}

// FUNCTION: SWBF2 0x00483f80
GameObject* Controllable::GetCurrentCollision() const
{
    return AIUtil::GetAlivePtr(mCollisionObj, true);
}

// FUNCTION: SWBF2 0x00483f90
void Controllable::SetWeaponIndex(int iChannel, int iIndex)
{
}

// FUNCTION: SWBF2 0x00483fa0
void Controllable::SetCharacter(Character* pCharacter)
{
    mCharacter = pCharacter;
    mPlayerId = pCharacter ? pCharacter->mPlayerId : -1;
    if (pCharacter)
    {
        mInitialPlayerId = mPlayerId;
    }
}

// FUNCTION: SWBF2 0x00483fd0
void Controllable::SetPilot(Controllable* pPilot)
{
    if (mPilot == pPilot)
    {
        return;
    }
    GameObject* pObject = GetGameObject();
    if (mPilotType == PILOT_VEHICLESELF && mPilot == this)
    {
        mPilot = 0;
    }
    if (mPilot)
    {
        GameObject* pPilotObject = mPilot->GetGameObject();
        if (pObject &&
            (mPilotType == PILOT_VEHICLE || mPilotType == PILOT_VEHICLESELF || mPilotType == PILOT_REMOTE) &&
            pPilotObject)
        {
            pPilotObject->GetCollisionObject()->mParent = 0;
            if (pObject->GetControllable() == this &&
                ((pPilotObject->GetGameObjectClass()->GetSoldierType() == GameObjectClass::SOLDIER_PILOT &&
                  pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer) && pObject->mMaxHealth > 1400.0f) ||
                 (mPilot->mCharacter && mPilot->mCharacter->mMedalsMgr &&
                  mPilot->mCharacter->mMedalsMgr->IsAwardAvailable(3))))
            {
                pObject->AddPilotSkillCount(-1);
            }
        }
    }
    ClearControllable();
    ReleaseController();
    mPilot = pPilot;
    if (pPilot)
    {
        GameObject* pPilotObject = pPilot->GetGameObject();
        if (pObject && pPilotObject)
        {
            int iTeam = pPilotObject->GetTeam();
            pObject->SetTeam(iTeam);
            pObject->mPerceivedTeam = iTeam;
        }
        SetCharacter(mPilot->mCharacter);
        SetupController();
        if (pObject &&
            (mPilotType == PILOT_VEHICLE || mPilotType == PILOT_VEHICLESELF || mPilotType == PILOT_REMOTE) &&
            pPilotObject)
        {
            pPilotObject->GetCollisionObject()->mParent = pObject->GetCollisionObject();
            if (pObject->GetControllable() == this &&
                ((pPilotObject->GetGameObjectClass()->GetSoldierType() == GameObjectClass::SOLDIER_PILOT &&
                  pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer) && pObject->mMaxHealth > 1400.0f) ||
                 (mPilot->mCharacter && mPilot->mCharacter->mMedalsMgr &&
                  mPilot->mCharacter->mMedalsMgr->IsAwardAvailable(3))))
            {
                pObject->AddPilotSkillCount(1);
            }
        }
    }
    else
    {
        if (pObject && pObject->GetGameObjectClass() && pObject->GetGameObjectClass()->IsNeutralWhenEmpty())
        {

            bool bOccupied = false;
            for (PblList<Controllable>::Itor it(pObject->GetControllableList()); it; ++it)
            {
                Controllable* pControllable = it;
                if (pControllable->mPilotType == PILOT_SELF ||
                    (pControllable->mPilotType == PILOT_VEHICLESELF && pControllable->mPilot == pControllable))
                {
                    continue;
                }
                if (pControllable->mPilot)
                {
                    bOccupied = true;
                    break;
                }
            }
            if (!bOccupied)
            {
                pObject->SetTeam(0);
                pObject->mPerceivedTeam = 0;
            }
        }
        SetCharacter(0);
    }
    if (mPilotType == PILOT_VEHICLESELF && !mPilot)
    {
        mPilot = this;
        SetupController();
    }
}

// FUNCTION: SWBF2 0x00484290
bool Controllable::Update(float fDeltaTime)
{
    UpdateFirstThirdPerson(mPlayerId);
    return true;
}

// FUNCTION: SWBF2 0x004842b0
const Aimer* Controllable::GetCurAimer() const
{
    int iIndex = GetWeaponIndex(mWpnChannel);
    if (iIndex < 0)
    {
        return 0;
    }
    return GetAimer(iIndex);
}

// FUNCTION: SWBF2 0x004842e0
const Weapon* Controllable::GetCurWpn() const
{
    int iIndex = GetWeaponIndex(mWpnChannel);
    if (iIndex < 0)
    {
        return 0;
    }
    return GetWeaponPtr(iIndex);
}

// FUNCTION: SWBF2 0x00484310
Weapon* Controllable::GetCurWpn()
{
    int iIndex = GetWeaponIndex(mWpnChannel);
    if (iIndex < 0)
    {
        return 0;
    }
    return GetWeaponPtr(iIndex);
}

// FUNCTION: SWBF2 0x00484340
int Controllable::GetWpnChannel() const
{
    return mWpnChannel;
}

// FUNCTION: SWBF2 0x00484350
float Controllable::GetWpnDistFactor() const
{
    return mWpnDistFactor;
}

// FUNCTION: SWBF2 0x00484360
void Controllable::SetWpnDistFactor(float fFactor)
{
    mWpnDistFactor = fFactor;
}

// FUNCTION: SWBF2 0x00484370
float Controllable::GetTurnAdjusted()
{
    if (mTurnAdjusted < 1.5f)
    {
        return mTurnAdjusted;
    }
    return mControlTurn;
}

// FUNCTION: SWBF2 0x00484390
float Controllable::GetPitchAdjusted()
{
    if (mPitchAdjusted < 1.5f)
    {
        return mPitchAdjusted;
    }
    return mControlPitch;
}

// FUNCTION: SWBF2 0x004843b0
GameObject* Controllable::GetTargetLockedObj()
{
    if (mPlayerId >= 0 && !GetEnteredRemoteOrVehicleIfNotMe())
    {
        if (!NetGame::IsEnabled() || NetGame::IsClient() ||
            (NetGame::IsOneWorld() && NetGame::IsHost()))
        {
            int iLocal = NetGame::GetJoystickIndex(mPlayerId);
            if (iLocal >= 0)
            {
                LockOnManager* pManager = LockOnManager::GetByPlayerIndex(iLocal);
                if (pManager)
                {
                    GameObject* pLocked = pManager->GetLockedTarget();
                    mTargetLockedObj.mObject = pLocked;
                    mTargetLockedObj.mSavedHandleId = pLocked ? GameObjectHandleId(pLocked) : 0;
                }
            }
        }
    }

    if (mTargetLockedObj.mObject &&
        GameObjectHandleId(mTargetLockedObj.mObject) != mTargetLockedObj.mSavedHandleId)
    {
        mTargetLockedObj.mObject = 0;
        mTargetLockedObj.mSavedHandleId = 0;
    }
    return mTargetLockedObj.mObject;
}

// FUNCTION: SWBF2 0x00484460
void Controllable::TargetTrackingForceClear()
{
    mTurnAdjusted = 2.0f;
    mPitchAdjusted = 2.0f;
    mTurnAuto = 0.0f;
    mPitchAuto = 0.0f;
    SetTargetLockedObj(0);
}

// FUNCTION: SWBF2 0x004844a0
void Controllable::SetTargetLockedObj(GameObject* pObject)
{
    if (mPlayerId < 0 || GetEnteredRemoteOrVehicleIfNotMe())
    {
        mTargetLockedObj.mObject = pObject;
        mTargetLockedObj.mSavedHandleId = pObject ? GameObjectHandleId(pObject) : 0;
        return;
    }

    if (mTargetLockedObj.mObject &&
        GameObjectHandleId(mTargetLockedObj.mObject) != mTargetLockedObj.mSavedHandleId)
    {
        mTargetLockedObj.mObject = 0;
        mTargetLockedObj.mSavedHandleId = 0;
    }
    if (mTargetLockedObj.mObject != pObject)
    {
        mTargetLockedObj.mObject = pObject;
        mTargetLockedObj.mSavedHandleId = pObject ? GameObjectHandleId(pObject) : 0;
    }

    if (!NetGame::IsEnabled() || NetGame::IsClient() ||
        (NetGame::IsOneWorld() && NetGame::IsHost()))
    {
        int iLocal = NetGame::GetJoystickIndex(mPlayerId);
        if (iLocal < 0)
        {
            return;
        }
        LockOnManager* pManager = LockOnManager::GetByPlayerIndex(iLocal);
        if (!pManager)
        {
            return;
        }
        GameObject* pLocked = pManager->GetLockedTarget();
        if (pLocked == pObject)
        {
            return;
        }

        if (NetGame::IsEnabled() && !(NetGame::IsOneWorld() && NetGame::IsHost()))
        {
            unsigned int uChangedTurn = pManager->mLockedObjChangedTurn;
            int iGrace = netTurnsPerSecond / 2;
            unsigned int uTurn = NetGame::GetClientAckedTurnNumber();
            if (uChangedTurn + (iGrace > 5 ? iGrace : 5) >= uTurn)
            {
                mTargetLockedObj.mObject = pLocked;
                mTargetLockedObj.mSavedHandleId = pLocked ? GameObjectHandleId(pLocked) : 0;
                return;
            }
        }

        if (pObject)
        {
            pManager->ForceNewTargetLock(pObject);
        }
        else
        {
            pManager->RemoveTargetLock();
        }
    }
}

// FUNCTION: SWBF2 0x00484800
ControllableClass::ControllableClass(ControllableClass* pBase)
{
    mPilotParentNodeHash = pBase->mPilotParentNodeHash;
    mPilotOffsetMatrix = pBase->mPilotOffsetMatrix;
    mPilotAnimation = pBase->mPilotAnimation;
    mIsPilotAnimation9Pose = pBase->mIsPilotAnimation9Pose;
    mAnimatedPilotNodeFlag = pBase->mAnimatedPilotNodeFlag;
    mPilotType = pBase->mPilotType;
    mIsPilotExposed = pBase->mIsPilotExposed;
    mCanCapturePosts = pBase->mCanCapturePosts;
    mCanEnterVehicles = pBase->mCanEnterVehicles;
    mCombatInterrupts = pBase->mCombatInterrupts;
    mIgnoreHintNodes = pBase->mIgnoreHintNodes;
    mShakeScale = pBase->mShakeScale;
    mBobPath = pBase->mBobPath;
    mCockpitTension = pBase->mCockpitTension;
    mControlsUnit = pBase->mControlsUnit;
    mAnimatesUnitAsTurret = pBase->mAnimatesUnitAsTurret;
    mLocalNameID = pBase->mLocalNameID;
    mForceMode = pBase->mForceMode;
    mThirdPersonFOV = pBase->mThirdPersonFOV;
    mFirstPersonFOV = pBase->mFirstPersonFOV;
}

// FUNCTION: SWBF2 0x00484930
bool ControllableClass::SetProperty(unsigned int uProperty, const char* pszValue, const RedModel* pModel)
{
    float fX = 0.0f;
    float fY = 0.0f;
    float fZ = 0.0f;
    int iIndex = 0;
    char szBuffer[128];

    switch (uProperty)
    {
    case 0x11CE2025:
        mAnimatedPilotNodeFlag = true;
        mPilotParentNodeHash = PblTEMPHash(pszValue);
        return true;
    case 0x104CD1B8:
        sscanf(pszValue, "%f", &mCockpitTension);
        return true;
    case 0x20728C12:
        sscanf(pszValue, "%f", &mThirdPersonFOV);
        mThirdPersonFOV *= 0.017453292f;
        return true;
    case 0x4785D6B7:
        sscanf(pszValue, "%f", &mFirstPersonFOV);
        mFirstPersonFOV *= 0.017453292f;
        return true;
    case 0x325F7C20:
        mCanCapturePosts = atoi(pszValue) != 0;
        return true;
    case 0x2B60E217:
        mIsPilotExposed = atoi(pszValue) != 0;
        return true;
    case 0x44C77C95:
        mControlsUnit = atoi(pszValue) != 0;
        return true;
    case 0x83192029:
        mAnimatesUnitAsTurret = atoi(pszValue) != 0;
        return true;
    case 0x848819E7:
        mIgnoreHintNodes = atoi(pszValue) != 0;
        return true;
    case 0x939B9985:
        mCanEnterVehicles = atoi(pszValue) == 0;
        return true;
    case 0x72CCBDE1:
        sscanf(pszValue, "%f", &mShakeScale);
        return true;
    case 0x9D1F5707:
        sscanf(pszValue, "%d", &mForceMode);
        return true;
    case 0x8A301535:
        sscanf(pszValue, "%d", &mBobPath.numPoints);
        return true;
    case 0xEF20A0A0:
        sscanf(pszValue, "%i %f %f %f", &iIndex, &fX, &fY, &fZ);
        mBobPath.Vec[iIndex].x = fX;
        mBobPath.Vec[iIndex].y = fY;
        mBobPath.Vec[iIndex].z = fZ;
        return true;
    case 0x49824403:
        sscanf(pszValue, "%i %f %f %f", &iIndex, &fX, &fY, &fZ);
        mBobPath.Slope[iIndex].x = fX;
        mBobPath.Slope[iIndex].y = fY;
        mBobPath.Slope[iIndex].z = fZ;
        return true;
    case 0x35341815:
        sscanf(pszValue, "%i %f %f %f", &iIndex, &fX, &fY, &fZ);
        mBobPath.Ore[iIndex].x = fX;
        mBobPath.Ore[iIndex].y = fY;
        mBobPath.Ore[iIndex].z = fZ;
        return true;
    case 0xD67D9BA2:
        sscanf(pszValue, "%i %f %f %f", &iIndex, &fX, &fY, &fZ);
        mBobPath.RSlope[iIndex].x = fX;
        mBobPath.RSlope[iIndex].y = fY;
        mBobPath.RSlope[iIndex].z = fZ;
        return true;
    case 0xBADF3672:
        sscanf(pszValue, "%i %f", &iIndex, &fX);
        mBobPath.Time[iIndex] = fX;
        return true;
    case 0xA976D065:
        mIsPilotAnimation9Pose = true;

    case 0x6E4FC069:
        mPilotAnimation = SoldierAnimationBank::AddAnimation(pszValue);
        return true;
    case 0x51CA39A6:
    {

        unsigned int uBone = PblTEMPHash(pszValue);
        if (pModel->GetHierarchicalBoneMatrix(uBone, mPilotOffsetMatrix))
        {
            mPilotParentNodeHash = pModel->GetParentBone(uBone);
        }
        return true;
    }
    case 0xDA1B8C0C:
        sscanf(pszValue, "%s", szBuffer);
        mLocalNameID = PblHash(szBuffer);
        return true;
    case 0x91EE6929:
        switch (PblHash(pszValue))
        {
        case 0xADA7AFDB: mPilotType = PILOT_NONE; break;
        case 0x645BA277: mPilotType = PILOT_SELF; break;
        case 0x71CD882F: mPilotType = PILOT_VEHICLE; break;
        case 0xC78D7953: mPilotType = PILOT_REMOTE; break;
        case 0x1FE70361: mPilotType = PILOT_VEHICLESELF; break;
        }
        return true;
    case 0x81BCE0F9:
        switch (PblHash(pszValue))
        {
        case 0xBFF162E2: mPilotDeathType = PILOTDEATH_FLY; break;
        case 0xF85A0760: mPilotDeathType = PILOTDEATH_FALL; break;
        }
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00484ed0
void ControllableClass::UpdatePilotAnimation(Controllable* pPilot, const RedPose* pPose,
                                             const PblMatrix& mMatrix, const PblVector3& vOffset,
                                             float fDeltaTime, float f1, float f2, float f3)
{
    if (!pPilot)
    {
        return;
    }

    if (mPilotAnimation == SoldierAnimationBank::INVALID_ANIMATION)
    {
        pPilot->SetPilotInfo(mMatrix, vOffset, SoldierAnimationBank::INVALID_ANIMATION, false,
                             f1, f2, f3, fDeltaTime);
    }
    else
    {
        const PblMatrix* pNode = 0;
        if (pPose && mPilotParentNodeHash)
        {
            pNode = pPose->Find(mPilotParentNodeHash);
        }
        if (mAnimatedPilotNodeFlag)
        {
            PblMatrix mPilot;
            D3DXMatrixMultiply(&mPilot, pNode, &mMatrix);
            pPilot->SetPilotInfo(mPilot, vOffset, mPilotAnimation, mIsPilotExposed,
                                 f1, f2, f3, fDeltaTime);
        }
        else
        {
            PblMatrix mPilot;
            if (pNode)
            {
                PblMatrix mSeat;
                D3DXMatrixMultiply(&mSeat, &mPilotOffsetMatrix, pNode);
                D3DXMatrixMultiply(&mPilot, &mSeat, &mMatrix);
            }
            else
            {
                D3DXMatrixMultiply(&mPilot, &mPilotOffsetMatrix, &mMatrix);
            }
            pPilot->SetPilotInfo(mPilot, vOffset, mPilotAnimation, mIsPilotExposed,
                                 f1, f2, f3, fDeltaTime);
        }
    }

    if (mControlsUnit || mAnimatesUnitAsTurret)
    {
        Aimer* pAimer = const_cast<Aimer*>(pPilot->GetAimer(0));
        if (pAimer)
        {
            pAimer->Update(fDeltaTime);
        }
    }
}

// FUNCTION: SWBF2 0x004774e0
float Controllable::GetTargetBubbleStanceModifier() const
{
    return 1.0f;
}

// FUNCTION: SWBF2 0x004774f0
SoldierAnimationBank::BANK Controllable::GetSoldierAnimationBank() const
{
    return SoldierAnimationBank::INVALID_BANK;
}

// FUNCTION: SWBF2 0x00477570
void Controllable::SetPilotInfo(const PblMatrix& mMatrix, const PblVector3& vOffset,
                                SoldierAnimationBank::ANIMATION eAnimation, bool bFlag,
                                float f0, float f1, float f2, float f3)
{
}

// FUNCTION: SWBF2 0x00477590
void Controllable::ApplyWeaponKick(float fPitch, float fYaw)
{
}

// FUNCTION: SWBF2 0x00477610
bool Controllable::EndSprint()
{
    return false;
}

// FUNCTION: SWBF2 0x00477640
float Controllable::GetOffenseMultiplier() const
{
    return 1.0f;
}

// FUNCTION: SWBF2 0x00477660
void Controllable::DeactivateIndirect()
{
}

// FUNCTION: SWBF2 0x00483400
EnergyBar* Controllable::GetEnergyBar()
{
    return 0;
}

// FUNCTION: SWBF2 0x00485080
GameObject* Controllable::GetVehicleIfAttached(GameObject* gameObject)
{
    if (gameObject)
    {
        Controllable* controllable = gameObject->GetControllable();
        if (controllable)
        {
            Character* character = controllable->mCharacter;
            if (character)
            {
                Controllable* vehControllable = character->GetVehicle();
                if (vehControllable)
                {
                    return vehControllable->GetGameObject();
                }
            }
        }
    }
    return gameObject;
}

// FUNCTION: SWBF2 0x004774d0
CollisionModel* Controllable::GetDefaultPilotCollision()
{
    return 0;

}

// FUNCTION: SWBF2 0x00477500
bool Controllable::CanEnterRemote(const GameObject*) const
{
    return false;

}

// FUNCTION: SWBF2 0x00477510
bool Controllable::CanEnterVehicle(const GameObject*) const
{
    return false;

}

// FUNCTION: SWBF2 0x00477520
bool Controllable::EnterVehicle(GameObject*)
{
    return false;

}

// FUNCTION: SWBF2 0x00477530
bool Controllable::CyclePosition(bool)
{
    return false;

}

// FUNCTION: SWBF2 0x00477540
bool Controllable::ExitVehicle(bool, bool)
{
    return false;

}

// FUNCTION: SWBF2 0x00477580
PilotDeathType Controllable::GetPilotDeathType()
{
    return PILOTDEATH_FALL;

}

// FUNCTION: SWBF2 0x004775a0
void Controllable::GetPitchYawLimits(float&, float&, float&, float&) const
{

}

// FUNCTION: SWBF2 0x004775b0
bool Controllable::Stand()
{
    return false;

}

// FUNCTION: SWBF2 0x004775c0
bool Controllable::Crouch()
{
    return false;

}

// FUNCTION: SWBF2 0x004775d0
bool Controllable::Prone()
{
    return false;

}

// FUNCTION: SWBF2 0x004775e0
bool Controllable::Jump()
{
    return false;

}

// FUNCTION: SWBF2 0x004775f0
bool Controllable::Roll()
{
    return false;

}

// FUNCTION: SWBF2 0x00477600
bool Controllable::Sprint()
{
    return false;

}

// FUNCTION: SWBF2 0x00477620
bool Controllable::JetJump()
{
    return false;

}

// FUNCTION: SWBF2 0x00477630
bool Controllable::EndJetJump()
{
    return false;

}

// FUNCTION: SWBF2 0x00477650
int Controllable::GetActiveWeaponChannel()
{
    return mWpnChannel;

}

// FUNCTION: SWBF2 0x00478a40
EnergyMode Controllable::GetEnergyMode() const
{
    return static_cast<EnergyMode>(0);

}

// FUNCTION: SWBF2 0x004833c0
float Controllable::GetAimTurnRate() const
{
    return 0.0f;

}

// FUNCTION: SWBF2 0x004833d0
int Controllable::GetWeaponCount() const
{
    return 0;

}

// FUNCTION: SWBF2 0x004833e0
int Controllable::GetWeaponIndex(int) const
{
    return 0;

}

// FUNCTION: SWBF2 0x004833f0
Weapon* Controllable::GetWeaponPtr(int) const
{
    return 0;

}

// FUNCTION: SWBF2 0x00483410
const Aimer* Controllable::GetAimer(int) const
{
    return 0;

}

// FUNCTION: SWBF2 0x00484660
void Controllable::GetPilotMatrix(const PblMatrix& mIn, PblMatrix& mOut)
{
    if (mPilot)
    {
        const PblMatrix& mPilotMatrix = mPilot->GetMatrix();
        const PblMatrix mInverse = static_cast<EntitySoldier*>(mPilot)->mLastVehicleMatrix.GetInverseFast();
        PblMatrix mRelative;
        D3DXMatrixMultiply(&mRelative, &mPilotMatrix, &mInverse);
        PblMatrix mResult;
        D3DXMatrixMultiply(&mResult, &mRelative, &mIn);
        mOut = mResult;
    }
}

// FUNCTION: SWBF2 0x0043aad0
Controllable* GetControllablePilot(Controllable* pControllable)
{
    if (pControllable->mPilotType == PILOT_SELF ||
        (pControllable->mPilotType == PILOT_VEHICLESELF && pControllable->mPilot == pControllable))
    {
        return 0;
    }
    return pControllable->mPilot;
}
