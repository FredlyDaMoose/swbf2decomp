#include "UnitController.h"
#include "PebbleFL/Common/PblVector.h"

#include <float.h>
#include <new>

#include "AIGoal.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Character.h"
#include "CollisionObject.h"
#include "CommandPost.h"
#include "Controllable.h"
#include "ControllerManager.h"
#include "Damageable.h"
#include "Entity.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "CameraManager.h"
#include "SquadCommands.h"
#include "Team.h"
#include "Thread.h"
#include "RedEngineFL/Portals/Portal.h"
#include "UnitBoardAgent.h"
#include "UnitCTFOffenseAgent.h"
#include "UnitCombatAgent.h"
#include "UnitDeathmatchAgent.h"
#include "UnitDefendAgent.h"
#include "UnitDestroyAgent.h"
#include "UnitFollowAgent.h"
#include "UnitHoldAgent.h"
#include "UnitRandomAgent.h"
#include "UnitTrooperAgent.h"
#include "UnitUnBoardAgent.h"
#include "UnitWaitAgent.h"
#include "UnitWaitSecondaryAgent.h"
#include "EntityBuildingArmed.h"
#include "OrdnanceEmitter.h"
#include "WeaponDispenser.h"

class EntitySoldier { public: static PblHash rttiHashEntitySoldier; };

class EntityWalker { public: static PblHash rttiHashEntityWalker; };
class EntityHover { public: static PblHash rttiHashEntityHover; };
class EntityFlyer { public: static PblHash rttiHashEntityFlyer; };
class WeaponInvisibility { public: static PblHash rttiHashWeaponInvisibility; };

class AmmoCounter
{
public:
    AmmoCounterClass* m_pClass;
    float mDefaultMaxClips;
    float mMaxClips;
    float mNumClips;
    float mCurClip;
    unsigned int m_uiRefCount;
};
 

namespace AI
{

class UnitThreatManager
{
public:

    UnitThreatManager();
    const GameObject* GetVisibleObject() const;

    void Update(float dt, UnitController* pCtrl);
};
}

namespace AudibleManager
{
void UpdateAudience(Controller* pCtrl);
}
namespace VisionManager
{
void RemoveRequests(UnitController* pCtrl);
void UpdatePotentiallyVisible(UnitController* pCtrl);
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

int GetUnitsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                    int iMax, const Team* pTeam, AI::AIAffil eAffil);
}

namespace AIUtil { bool __fastcall ShouldStayInVehicle(Controllable* pMe); }

namespace AIUtil { bool __fastcall CanPotentiallySee(Controllable* pMe, const GameObject* pObject, float* pOut); }

namespace AIDebug { bool SkipCameraForLODs(); }

// FUNCTION: SWBF2 0x004386a0
bool AIDebug::SkipCameraForLODs()
{
    return false;
}

// FUNCTION: SWBF2 0x00663d00
void UnitController::ResetFriendlyList()
{
    mFriendlyCount = 0;
}

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

namespace
{
    const PblVector3& GetTrans(const PblMatrix& m)
    {
        return *reinterpret_cast<const PblVector3*>(&m.m[3][0]);
    }

    PblHandle<const GameObject> MakeHandle(const GameObject* pObject)
    {
        PblHandle<const GameObject> h;
        h.mObject = pObject;
    h.mSavedHandleId = pObject ? pObject->mHandleId : 0;
        return h;
    }

    const PblVector3& GetPosition(const GameObject* pObject)
    {
    return static_cast<const CollisionObject*>(pObject)->GetPosition();
    }

    GameObject* Resolve(PblHandle<GameObject>& h)
    {
        GameObject* pObject = h.mObject;
        if (pObject)
        {
        if (pObject->mHandleId == h.mSavedHandleId)
        {
                return pObject;
        }
            h.mObject = 0;
            h.mSavedHandleId = 0;
        }
        return 0;
    }

    AttackHelper* GetAttackHelper(UnitAgent* pAgent)
    {
        return reinterpret_cast<AttackHelper*>(pAgent->mAttackHelper);
    }
}

// GLOBAL: SWBF2 0x01f99384
// SYNTHETIC: SWBF2 0x00406c50
// `dynamic initializer for 'UnitController::rttiHashUnitController''
PblHash UnitController::rttiHashUnitController("UnitController");
char UnitController::rttiNameUnitController[] = "UnitController";

// GLOBAL: SWBF2 0x01f9a1f8
// SYNTHETIC: SWBF2 0x00406c60
// `dynamic initializer for 'UnitController::sMemoryPool''
// SYNTHETIC: SWBF2 0x007696c0
// `dynamic atexit destructor for 'UnitController::sMemoryPool''
MemoryPool UnitController::sMemoryPool("UnitController", sizeof(UnitController));

// FUNCTION: SWBF2 0x00663050
bool UnitController::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashUnitController || Controller::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00663080
unsigned int UnitController::GetDerivedRtti() const
{
    return rttiHashUnitController;
}

// FUNCTION: SWBF2 0x00663090
const char* UnitController::GetDerivedRttiName() const
{
    return rttiNameUnitController;
}

// FUNCTION: SWBF2 0x006630a0
static int GetLODDistHigh()
{
    return AIUtil::IsUberMode() ? 5 : 25;
}

// FUNCTION: SWBF2 0x006630c0
static int GetLODDistLow()
{
    return AIUtil::IsUberMode() ? 20 : 100;
}

// FUNCTION: SWBF2 0x006630e0
UnitController* UnitControllerPtr(Controller* pController)
{
    if (pController && pController->IsRtti(UnitController::rttiHashUnitController))
    {
        return (UnitController*)pController;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00663100
const UnitController* UnitControllerPtr(const Controller* pController)
{
    if (pController && pController->IsRtti(UnitController::rttiHashUnitController))
    {
        return (const UnitController*)pController;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00663120
UnitController* UnitControllerPtr(Controllable* pControllable)
{
    if (pControllable && pControllable->mCtrl &&
        pControllable->mCtrl->IsRtti(UnitController::rttiHashUnitController))
    {
        return (UnitController*)pControllable->mCtrl;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00663150
const UnitController* UnitControllerPtr(const Controllable* pControllable)
{
    if (pControllable && pControllable->mCtrl &&
        pControllable->mCtrl->IsRtti(UnitController::rttiHashUnitController))
    {
        return (const UnitController*)pControllable->mCtrl;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00663180
UnitController::UnitController(Controllable* pOwner)
    : Controller(pOwner)
{
    new (mManagerNode) PblOrderedList<UnitController, float>::Node();
    new (mThreatManager) AI::UnitThreatManager();
    new (mLowLevel) AILowLevel(this);
    new (mLastSquadCommand) SquadCommands::SquadCommandOrder();

    for (int i = 0; i < 8; i++)
    {
        new (mFriendly + i * sizeof(PblHandle<const GameObject>)) PblHandle<const GameObject>();
    }

    mOwner = pOwner;
    mAgent = 0;
    mLodHumanPlayer = 0;
    mCrowdCheckTime = -100.0f;
    mJustEnteredVehicleGoToPickupAgent =
        mOwner->GetEntity()->IsRtti(EntityWalker::rttiHashEntityWalker) ||
        mOwner->GetEntity()->IsRtti(EntityHover::rttiHashEntityHover) ||
        mOwner->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer);
    mLookAheadOnly = false;
    SetRandomSeed(AIUtil::GetRandomInt(0, 100));
    const PblMatrix& rOwnerMatrix = mOwner->GetMatrix();
    mOriginalAheadDir = *reinterpret_cast<const PblVector3*>(&rOwnerMatrix.m[2][0]);
    mSkip = false;
    mFriendlyCount = 0;
    mUpdateRate = 0.5f;
    mLodState = 3;
    mLowLevelUpdateSkips = 0;
    mLowLevelUpdateSkipDt = 0.0f;
    mVisionSkip = 0;
    mLastUpdateTime = GameLoop::GetMissionTime();
    mNextUpdateTime = GameLoop::GetMissionTime();
}

// FUNCTION: SWBF2 0x006633d0
UnitController::~UnitController()
{
    DeleteAgent();
    for (int i = 7; i >= 0; --i)
    {
        reinterpret_cast<PblHandle<const GameObject>*>(mFriendly)[i].~PblHandle();
    }
    GetLowLevel().~AILowLevel();
    reinterpret_cast<AI::UnitThreatManager*>(mThreatManager)->~UnitThreatManager();
}

// SYNTHETIC: SWBF2 0x006633a0
// UnitController::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x006645cb
// [thunk]:UnitController::`scalar deleting destructor' adjustor{120}

// SYNTHETIC: SWBF2 0x006645d3
// [thunk]:UnitController::`scalar deleting destructor' adjustor{452}

// FUNCTION: SWBF2 0x006634a0
void UnitController::KillController()
{
    if (mAgent)
    {
        mAgent->Kill();
    }
    Fsm::Kill();
}

// FUNCTION: SWBF2 0x006634c0
void UnitController::DeleteAgent()
{
    if (mAgent)
    {
        delete mAgent;
        mAgent = 0;
    }
}

// FUNCTION: SWBF2 0x006634e0
float UnitController::GetUpdateRate() const
{
    float fAgentRate = 1.0f;
    if (mAgent)
    {
        fAgentRate = mAgent->GetUpdateRate();
    }
    float fIntervals[5] = { 4.0f, 3.0f, 2.0f, 1.0f, 0.25f };
    return fIntervals[mLodState] * fAgentRate;
}

// FUNCTION: SWBF2 0x00663540
void UnitController::EnterCombat()
{
    mAgent = new UnitCombatAgent(this, *static_cast<const UnitCombatInfo*>(GetCurCmd()));
}

// FUNCTION: SWBF2 0x00663030
PblHandle<GameObject> UnitFollowInfo::GetWho() const
{
    return mWho;
}

// FUNCTION: SWBF2 0x00663580
void UnitController::EnterState()
{
    switch (mCurState)
    {
    case STATE_WAIT:
        mAgent = new UnitWaitAgent(this);
        break;
    case STATE_COMBAT:
        EnterCombat();
        return;
    case STATE_WAIT_SECONDARY:
    {

        mAgent = new UnitWaitSecondaryAgent(this, GetCurCmd()->GetParam() / 1000.0f,
                                            (GetCurCmd()->GetParam() & 1) != 0);
        break;
    }
    case STATE_BOARD:
    {
        const UnitBoardInfo* pInfo = static_cast<const UnitBoardInfo*>(GetCurCmd());
        GameObject* pVehicle = pInfo->GetVehicle();
        mAgent = new UnitBoardAgent(this, pVehicle, pInfo->GetVOResponseHandle());
        break;
    }
    case STATE_UNBOARD:
        mAgent = new UnitUnBoardAgent(this);
        break;
    case STATE_RANDOM:
        mAgent = new UnitRandomAgent(this);
        break;
    case STATE_CAPTURECP:
    {
        const CommandPost* pPost =
            static_cast<const UnitCaptureCPInfo*>(GetCurCmd())->GetCommandPost();
        if (!pPost)
        {
            return;
        }
        mAgent = new UnitTrooperAgent(this, pPost);
        break;
    }
    case STATE_CTFOFFENSE:
    {
        const UnitGoalAgentInfo* pInfo = static_cast<const UnitGoalAgentInfo*>(GetCurCmd());
        mAgent = new UnitCTFOffenseAgent(this, pInfo->GetFlag(), pInfo->GetPos());
        break;
    }
    case STATE_DEATHMATCH:
        mAgent = new UnitDeathmatchAgent(this);
        break;
    case STATE_DEFEND:
    {
        AI::GoalTarget target;
        static_cast<const UnitGoalAgentInfo*>(GetCurCmd())->GetGoalTarget(target);
        mAgent = new UnitDefendAgent(this, target);
        break;
    }
    case STATE_DESTROY:
    {
        AI::GoalTarget target;
        static_cast<const UnitGoalAgentInfo*>(GetCurCmd())->GetGoalTarget(target);
        mAgent = new UnitDestroyAgent(this, target);
        break;
    }
    case STATE_SC_FOLLOW:
    {
        const UnitFollowInfo* pInfo = static_cast<const UnitFollowInfo*>(GetCurCmd());
        PblHandle<GameObject> who = pInfo->GetWho();
        mAgent = new UnitFollowAgent(this, MakeHandle(Resolve(who)), pInfo->GetParam() != 0);
        break;
    }
    case STATE_SC_HOLD:
        mAgent = new UnitHoldAgent(this);
        break;
    }
}

// FUNCTION: SWBF2 0x00663920
void UnitController::ExitState()
{
    if (mAgent)
    {
        mAgent->Kill();
    }
    DeleteAgent();
}

// FUNCTION: SWBF2 0x00663940
void UnitController::UpdateState(float dt)
{
    if (mAgent)
    {
        mCommandDone = mAgent->Update(dt, false);
    }
}

// FUNCTION: SWBF2 0x00663970
void UnitController::UpdateHighLevel()
{
    float fTime = GameLoop::GetMissionTime();
    float dt = fTime - mLastUpdateTime;
    mLastUpdateTime = fTime;

    static_cast<Trackable*>(mOwner)->GetGameObject();
    const Damageable* pHealth = static_cast<const Damageable*>(
        static_cast<Trackable*>(mOwner)->GetGameObject());
    if (!pHealth->mIsAlive)
    {
        mNextUpdateTime = fTime + 100000.0f;
        return;
    }

    mOwner->ResetControls();

    if (mJustEnteredVehicleGoToPickupAgent)
    {
        AI::EventInfo info(AI::EVT_Vehicle_WaitForPickUp, 4.0f, 0);
        if (SendEvent(&info))
        {
            mJustEnteredVehicleGoToPickupAgent = false;
        }
    }

    UpdateLodState();
    UpdateFreeSpace();
    AudibleManager::UpdateAudience(this);

    if (Character* pCharacter = mOwner->mCharacter)
    {
        pCharacter->mAdrenaline.Update(dt, static_cast<Trackable*>(mOwner)->GetGameObject());
    }

    reinterpret_cast<AI::UnitThreatManager*>(mThreatManager)->Update(dt, this);
    CommandFsm::FSMUpdate(dt, false);
    UpdateInvisibility();
    UseBuffs();
    UseAutoTurrets();
    mOwner->SetCurrentCollision(0);

    if (static_cast<Trackable*>(mOwner)->GetGameObject() && AIUtil::IsEndGame(0) &&
        !AIUtil::ShouldStayInVehicle(mOwner))
    {
        reinterpret_cast<AILowLevel*>(mLowLevel)->SetExitVehicle(0);
    }

    VisionManager::RemoveRequests(this);
    VisionManager::UpdatePotentiallyVisible(this);
    mNextUpdateTime = GetUpdateRate() + fTime;
}

// FUNCTION: SWBF2 0x00663af0
void UnitController::UpdateLowLevel(float dt)
{
    AILowLevel* pLowLevel = reinterpret_cast<AILowLevel*>(mLowLevel);
    mLowLevelUpdateSkipDt += dt;
    if (--mLowLevelUpdateSkips >= 0)
    {
        pLowLevel->UpdateSkip();
        return;
    }

    switch (mLodState)
    {
    case 0:
    case 1:
        mLowLevelUpdateSkips = 4;
        break;
    case 2:
        mLowLevelUpdateSkips = 2;
        break;
    default:
        mLowLevelUpdateSkips = 0;
        break;
    }

    static_cast<Trackable*>(mOwner)->GetGameObject();
    UpdateTargetBubbleSize(mLowLevelUpdateSkipDt);
    UpdateTotalVisFactor();
    pLowLevel->Update(mLowLevelUpdateSkipDt);
    mLowLevelUpdateSkipDt = 0.0f;
}

// FUNCTION: SWBF2 0x00663b90
bool UnitController::IsInCombat() const
{
    return mCurState == STATE_COMBAT;
}

// FUNCTION: SWBF2 0x00663ba0
bool UnitController::IsAgentDone()
{
    return mAgent && mAgent->IsDone();
}

// FUNCTION: SWBF2 0x00663bc0
CmdInfo UnitController::CopyCurCmd() const
{
    CmdInfo cmd;
    if ((unsigned int)GetCurCmd()->GetCommand() <= CMD_SC_END)
    {
        cmd = *GetCurCmd();
    }
    return cmd;
}

// FUNCTION: SWBF2 0x00663c00
const UnitCmdInfo* UnitController::GetCurCmd() const
{
    return static_cast<const UnitCmdInfo*>(&mCurCmd);
}

// FUNCTION: SWBF2 0x00663c10
const SquadCommands::SquadCommandOrder& UnitController::GetLastSquadCommand()
{
    return *reinterpret_cast<const SquadCommands::SquadCommandOrder*>(mLastSquadCommand);
}

// FUNCTION: SWBF2 0x00663c20
const GameObject* UnitController::GetAttackHelperTarget() const
{
    if (!mAgent)
    {
        return 0;
    }
    return AIUtil::GetAlivePtr(MakeHandle(GetAttackHelper(mAgent)->GetTarget()), false);
}

// FUNCTION: SWBF2 0x00663c70
const GameObject* UnitController::GetAttackHelperVisibleTarget() const
{
    if (!mAgent)
    {
        return 0;
    }
    return AIUtil::GetAlivePtr(MakeHandle(GetAttackHelper(mAgent)->GetVisibleTarget()), false);
}

// FUNCTION: SWBF2 0x00663cc0
const PblVector3& UnitController::GetAttackHelperTargetLastSeenPosition() const
{
    if (mAgent)
    {
        return GetAttackHelper(mAgent)->GetLastSeenTargetPos();
    }
    return GetTrans(_IdentityMatrix);
}

// FUNCTION: SWBF2 0x00663ce0
void UnitController::ForceSetAttackHelperTarget(const GameObject* pTarget)
{
    if (mAgent)
    {
        GetAttackHelper(mAgent)->SetVisibleTarget(pTarget);
    }
}

// FUNCTION: SWBF2 0x00663dd0
void UnitController::UpdateLodState()
{
    if (mLodStateUpdateTime + 0.5f > GameLoop::GetMissionTime())
    {
        return;
    }
    mLodStateUpdateTime = GameLoop::GetMissionTime();

    int iLod = 0;
    const PblVector3& vMyPos = GetTrans(mOwner->GetMatrix());
    float fClosestDistSq = FLT_MAX;
    mLodHumanPlayer = 0;

    for (int i = 0; i < Character::sCharacterLimit; i++)
    {
        Character* pCharacter = Character::GetPtr(i);
        if (pCharacter->mPlayerId < 0 || !pCharacter->GetControllable())
        {
            continue;
        }

        int iCharLod = 3;
        const PblVector3& vPos = GetTrans(pCharacter->GetControllable()->GetMatrix());
        float dx = vPos.x - vMyPos.x;
        float dy = vPos.y - vMyPos.y;
        float dz = vPos.z - vMyPos.z;
        float fDistSq = dx * dx + dy * dy + dz * dz;
        int iHigh = GetLODDistHigh();
        if ((float)(iHigh * iHigh) > fDistSq)
        {
            iCharLod = 4;
        }
        else
        {
            int iLow = GetLODDistLow();
            if (fDistSq > (float)(iLow * iLow))
            {
                iCharLod = 2;
            }
        }

        const Controllable* pOther = pCharacter->GetControllable();
        if (!AIUtil::CanPotentiallySee(mOwner, static_cast<const Trackable*>(pOther)->GetGameObject(), 0))
        {
            iCharLod--;
        }

        pOther = pCharacter->GetControllable();
        const RedSceneObject& scene = *static_cast<const Trackable*>(pOther)->GetGameObject();
        if (scene.PortalActive() && !scene.IsActive(pvs::GetCurrentFrame_()))
        {
            iCharLod--;
        }

        if (iLod > iCharLod)
        {
            iCharLod = iLod;
        }
        iLod = iCharLod;
        if (fClosestDistSq > fDistSq)
        {
            fClosestDistSq = fDistSq;
            mLodHumanPlayer = pCharacter;
        }
        if (iLod == 4)
        {
            break;
        }
    }

    if (!AIDebug::SkipCameraForLODs() && iLod != 4)
    {
        float fRadius = AIUtil::GetCollisionRadius(static_cast<Trackable*>(mOwner)->GetGameObject()) * 3.0f;
        if (CameraManager::sInstance->IsInsideAnyCamera(vMyPos, fRadius, (float)GetLODDistLow()))
        {
            iLod = 3;
            if (CameraManager::sInstance->IsInsideAnyCamera(vMyPos, fRadius, (float)GetLODDistHigh()))
            {
                iLod = 4;
            }
        }
    }

    mLodState = iLod;
}

// FUNCTION: SWBF2 0x006640b0
int UnitController::GetRandomSeed() const
{
    return mSeed;
}

// FUNCTION: SWBF2 0x006640c0
void UnitController::SetRandomSeed(int iSeed)
{
    mSeed = iSeed;
}

// FUNCTION: SWBF2 0x006640d0
int UnitController::SendEvent(AI::EventInfo* pInfo)
{
    if (!mAgent)
    {
        return 0;
    }

    int iResult = mAgent->EventHandler(pInfo);
    if (pInfo->mNextCmd.GetCmdType() != CMD_NONE)
    {
        if (mAgent->ShouldQueueCurCmd(pInfo->mNextCmd))
        {
            mAgent->mCtrl->PrependQueueCommands(mAgent->mCtrl->CopyCurCmd(), pInfo->mNextCmd);
        }
        else
        {
            mAgent->mCtrl->PrependQueueCommand(pInfo->mNextCmd);
        }

        if (mLodState >= 3)
        {
            ControllerManager::RequestControllerUpdateASAP(this);
        }
    }
    return iResult;
}

// FUNCTION: SWBF2 0x00664170
int UnitController::SendEvent(AI::EventInfo* pInfo) const
{
    if (mAgent)
    {
        return mAgent->EventHandler(pInfo);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00664190
void UnitController::SetLastShotDist(float fDistSq)
{
    mLastShotDistSq = fDistSq;
    mLastShotDistPos = mOwner ? GetTrans(mOwner->GetMatrix()) : GetTrans(_IdentityMatrix);
    mLastShotDistTime = GameLoop::GetMissionTime();
}

// FUNCTION: SWBF2 0x006641e0
bool UnitController::IsTargetLOSPossiblyBlocked()
{
    if (GameLoop::GetMissionTime() - 2.0f > mLastShotDistTime)
    {
        return false;
    }

    const GameObject* pTarget = GetAttackHelperTarget();
    if (!pTarget || !mOwner)
    {
        return false;
    }

    const PblVector3& vMyPos = GetTrans(mOwner->GetMatrix());
    float dx = vMyPos.x - mLastShotDistPos.x;
    float dy = vMyPos.y - mLastShotDistPos.y;
    float dz = vMyPos.z - mLastShotDistPos.z;
    if (dx * dx + dy * dy + dz * dz > 1.0f)
    {
        return false;
    }

    const PblVector3& vTargetPos = GetPosition(pTarget);
    dx = vTargetPos.x - vMyPos.x;
    dy = vTargetPos.y - vMyPos.y;
    dz = vTargetPos.z - vMyPos.z;
    float fDistSq = dx * dx + dy * dy + dz * dz;
    if (fDistSq < 4.0f)
    {
        return false;
    }
    return fDistSq > mLastShotDistSq * 4.0f;
}

// FUNCTION: SWBF2 0x006642d0
void UnitController::UpdateInvisibility()
{
    if (!mOwner || !mOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return;
    }
    if (reinterpret_cast<AI::UnitThreatManager*>(mThreatManager)->GetVisibleObject())
    {
        return;
    }

    AILowLevel* pLowLevel = reinterpret_cast<AILowLevel*>(mLowLevel);
    for (int i = 0; i < mOwner->GetWeaponCount(); ++i)
    {
        if (mOwner->GetWeaponPtr(i)->IsRtti(WeaponInvisibility::rttiHashWeaponInvisibility))
        {
            pLowLevel->SetNextWeapon(i);
            pLowLevel->SetFireRequest(true, false);
        }
    }
}

// FUNCTION: SWBF2 0x00664360
void UnitController::UseBuffs()
{
    if (!mOwner || !mOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return;
    }
    if (!reinterpret_cast<AI::UnitThreatManager*>(mThreatManager)->GetVisibleObject())
    {
        return;
    }

    for (int i = 0; i < mOwner->GetWeaponCount(); ++i)
    {
        Weapon* pWeapon = mOwner->GetWeaponPtr(i);
        const WeaponClass* pClass = pWeapon->mClass;
        if (pWeapon->CheckOverheat())
        {
            continue;
        }
        if (pClass->mOrdnanceClass &&
            pClass->mOrdnanceClass->IsRtti(OrdnanceEmitterClass::rttiHashOrdnanceEmitterClass) &&
            (pClass->mTargetSides & 4) && (pClass->mAITargetTypes & 1))
        {
            GameObject* apUnits[3];
            const Team* pTeam = Team::GetPtr(static_cast<Trackable*>(mOwner)->GetGameObject()->GetTeam());
            if (ControllerManager::GetUnitsInRange(GetTrans(mOwner->GetMatrix()), pClass->mMaxRange, apUnits, 3,
                                                   pTeam, AI::AFFIL_ALLYNEUTRAL) < 3)
            {
                return;
            }
            AILowLevel* pLowLevel = reinterpret_cast<AILowLevel*>(mLowLevel);
            pLowLevel->SetNextWeapon(i);
            pLowLevel->SetFireRequest(true, false);
        }
    }
}

// FUNCTION: SWBF2 0x00664490
void UnitController::UseAutoTurrets()
{
    if (!mOwner || !mOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return;
    }

    const GameObject* pTarget = reinterpret_cast<AI::UnitThreatManager*>(mThreatManager)->GetVisibleObject();
    const Controllable* pTargetCtl = pTarget ? pTarget->GetControllable() : 0;
    Controller* pTargetCtrl = pTargetCtl ? pTargetCtl->mCtrl : 0;
    if (!pTargetCtrl || !pTargetCtrl->IsInCrowd())
    {
        return;
    }
    if (!pTarget->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return;
    }

    for (int i = 0; i < mOwner->GetWeaponCount(); ++i)
    {
        Weapon* pWeapon = mOwner->GetWeaponPtr(i);
        WeaponClass* pClass = pWeapon->mClass;
        if (!pClass || pWeapon->CheckOverheat())
        {
            continue;
        }
        const AmmoCounter* pAmmo = pWeapon->m_pAmmoCounter;
        float fAmmoPerRound = pAmmo->m_pClass->mAmmoPerRound;
        if (fAmmoPerRound > pAmmo->mCurClip && fAmmoPerRound > pAmmo->mNumClips)
        {
            continue;
        }
        if (pClass->IsRtti(WeaponDispenserClass::rttiHashWeaponDispenserClass))
        {
            EntityClass* pDispense = static_cast<WeaponDispenserClass*>(pClass)->mDispenseClass;
            if (pDispense && pDispense->IsRtti(EntityBuildingArmedClass::rttiHashEntityBuildingArmedClass) &&
                (pClass->mAITargetTypes & 1))
            {
                AILowLevel* pLowLevel = reinterpret_cast<AILowLevel*>(mLowLevel);
                pLowLevel->SetNextWeapon(i);
                pLowLevel->SetFireRequest(true, false);
            }
        }
    }
}

// FUNCTION: SWBF2 0x00663d60
int UnitController::GetFriendlyFireCount() const
{
    return mFriendlyCount;
}

// FUNCTION: SWBF2 0x00663d10
void UnitController::AddFriendlyFire(const GameObject* pObject)
{
    if (mFriendlyCount < 8)
    {
        ((PblHandle<const GameObject>*)mFriendly)[mFriendlyCount++] = pObject;
    }
}

// FUNCTION: SWBF2 0x00663d70
const GameObject* UnitController::GetFriendlyFire(int iIndex)
{
    if (iIndex >= mFriendlyCount)
    {
        return 0;
    }
    return ((PblHandle<const GameObject>*)mFriendly)[iIndex];
}

// FUNCTION: SWBF2 0x00664090
bool UnitController::IsLookAheadOnly() const
{
    return mLookAheadOnly;
}

// FUNCTION: SWBF2 0x006640a0
const PblVector3& UnitController::GetOriginalAheadDir() const
{
    return mOriginalAheadDir;
}
