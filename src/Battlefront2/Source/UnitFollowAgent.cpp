#include "UnitFollowAgent.h"
#include "GameObject.h"

#include "EventInfo.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"
#include "Controllable.h"
#include "PlayerController.h"
#include "AILowLevel.h"
#include "AttackHelper.h"
#include "Entity.h"
#include "EntitySoldier.h"
#include "Character.h"
#include "CollisionObject.h"
#include "AIUtil.h"
#include "GameLoop.h"
#include "PathFinder.h"
#include "PebbleFL/Common/PblRandom.h"

#include "UnitThreatManager.h"

enum FormationType
{
    FT_RANDOM = 0,
    FT_SINGLEFILE = 1,
    FT_DOUBLEFILE = 2,
    FT_DOUBLEFILE_RAND = 3,
    FT_CIRCLE = 4,
    FT_CONVOY = 5,
    FT_FLYINGV = 6
};

namespace ReserveManager
{

    PblVector3 ReserveOffset(const GameObject* pTarget, const GameObject* pSelf, FormationType eFormation);

    int GetReserveOrder(const GameObject* pTarget, const GameObject* pSelf, ReserveType eType);

    void UnreserveReserver(const GameObject* pReserver, ReserveType eType);
}

namespace SquadCommands
{
bool IsSquadCommand(int iCmd);
}

namespace AI
{
enum VOType
{
    VOT_SC_StopFollowResponse = 25
};

namespace VOHelper
{

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget, const GameObject* pOther, float fDelay);
}
}

namespace
{

    // FUNCTION: SWBF2 0x00667fc0
    FormationType GetFormationType(const UnitController* pCtrl)
    {
        return pCtrl->mOpenRadius > 10.0f ? FT_FLYINGV : FT_DOUBLEFILE;
    }

    template <typename TYPE>
    TYPE* ResolveHandle(PblHandle<TYPE>& handle)
    {
        return handle.GetValidObject();
    }

    GameObject* ResolveHandleAt(void* pHandleAddr)
    {
        return pHandleAddr ? ResolveHandle(*reinterpret_cast<PblHandle<GameObject>*>(pHandleAddr)) : 0;
    }

    PblHandle<GameObject> BuildHandle(GameObject* pObject)
    {
        PblHandle<GameObject> h;
        h.mObject = pObject;
        h.mSavedHandleId = pObject ? pObject->mHandleId : 0;
        return h;
    }

    const PblHash kRttiHashEntitySoldier("EntitySoldier");
    const PblHash kRttiHashEntityDroideka("EntityDroideka");

    Controllable* SeatOf(const GameObject* pTarget)
    {
        return pTarget->mControllableList.GetFirst()->_pObject;
    }

    const PblVector3* GetTargetPosition(const GameObject* pTarget)
    {
        return &static_cast<const CollisionObject*>(pTarget)->GetPosition();
    }
}

// GLOBAL: SWBF2 0x01f993e8
// SYNTHETIC: SWBF2 0x00406ce0
// `dynamic initializer for 'UnitFollowAgent::rttiHashUnitFollowAgent''
PblHash UnitFollowAgent::rttiHashUnitFollowAgent("UnitFollowAgent");
char UnitFollowAgent::rttiNameUnitFollowAgent[] = "UnitFollowAgent";

// FUNCTION: SWBF2 0x00667f00
bool UnitFollowAgent::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashUnitFollowAgent || UnitAgent::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00667f30
unsigned int UnitFollowAgent::GetDerivedRtti() const
{
    return rttiHashUnitFollowAgent;
}

// FUNCTION: SWBF2 0x00667f40
const char* UnitFollowAgent::GetDerivedRttiName() const
{
    return rttiNameUnitFollowAgent;
}

// SYNTHETIC: SWBF2 0x00667f50
// UnitFollowAgent::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00667fe0
UnitFollowAgent::UnitFollowAgent(UnitController* pCtrl, PblHandle<GameObject const> hTarget, bool bTightFollow)
    : UnitAgent(pCtrl)
    , mCoverHelper(this)
    , mCombatHelper(this)
{
    mTarget = hTarget;
    mTightFollowMode = bTightFollow;
    mCurState = 1;
    mNextState = bTightFollow ? 2 : 5;
}

// FUNCTION: SWBF2 0x00668070
void UnitFollowAgent::Kill()
{
    mCoverHelper.Kill();
    mCombatHelper.Kill();
    UnitAgent::Kill();
}

// FUNCTION: SWBF2 0x006680a0
float UnitFollowAgent::GetUpdateRate() const
{
    if (mCurState == 4)
    {

        return 0.4f;
    }
    return UnitAgent::GetUpdateRate();
}

// FUNCTION: SWBF2 0x006680d0
void UnitFollowAgent::EnterState()
{
    switch (mCurState)
    {
    case 1:
    case 100:
        ReserveManager::UnreserveReserver(mCtrl->mOwner->GetGameObject(), RT_FORMATION);
        return;

    case 2:
        if (!ResolveHandle(mTarget))
        {
            SetNextState(100);
            return;
        }
        return;

    case 4:
        if (mCombatCmd.mWhat == 0)
        {
            UnitCombatInfo temp(UnitCombatInfo::ENGAGE);
            mCombatCmd = temp;
        }
        mCombatHelper.Init(reinterpret_cast<const UnitCombatInfo&>(mCombatCmd));
        return;

    case 5:
        if (!ResolveHandle(mTarget))
        {
            SetNextState(100);
            return;
        }
        break;

    default:
        return;
    }

    for (unsigned int uTry = 1; uTry <= 20; ++uTry)
    {

        const GameObject* pTargetA = ResolveHandle(mTarget);
        const GameObject* pTargetB = ResolveHandle(mTarget);

        PblVector3 vPush = PblRandom::GetGlobalVector3();

        vPush.x = (vPush.x * 20.0f) * 0.2f;
        vPush.y = (vPush.y * 20.0f) * 0.2f;
        vPush.z = (vPush.z * 20.0f) * 0.2f;

        const float* pRow = &static_cast<const CollisionObject*>(pTargetA)->GetMatrix().m[0][0];
        PblVector3 vRow((pRow[8] * 20.0f) * 0.5f, (pRow[9] * 20.0f) * 0.5f, (pRow[10] * 20.0f) * 0.5f);

        const PblVector3* pTargetPos = GetTargetPosition(pTargetB);
        PblVector3 vDest(pTargetPos->x - vRow.x + vPush.x,
                         pTargetPos->y - vRow.y + vPush.y,
                         pTargetPos->z - vRow.z + vPush.z);

        if (PathFinder::IsPointValid(vDest, 0, mCtrl->mOwner->GetGameObject()) && uTry < 20)
        {
            reinterpret_cast<AILowLevel*>(mCtrl->mLowLevel)->SetNavigator_Goto(vDest, 0, false, false);
            return;
        }
    }
    SetNextState(100);
}

// FUNCTION: SWBF2 0x006683d0
void UnitFollowAgent::ExitState()
{
    if (mCurState == 6)
    {
        mCoverHelper.HELPERClean();
    }
}

// FUNCTION: SWBF2 0x006683e0
bool UnitFollowAgent::CheckForForwardHintNodes()
{

    if (ReserveManager::GetReserveOrder(ResolveHandle(mTarget), mCtrl->mOwner->GetGameObject(), RT_FORMATION) > 2)
    {
        return false;
    }
    return mCoverHelper.CheckForHintAndInit(COVERTYPE_ForwardCover, ResolveHandle(mTarget));
}

// FUNCTION: SWBF2 0x00668480
bool UnitFollowAgent::IsInLooseRange()
{
    const GameObject* pTarget = ResolveHandle(mTarget);
    const PblMatrix& mine = mCtrl->mOwner->GetMatrix();
    const PblVector3* theirs = GetTargetPosition(pTarget);
    float dx = theirs->x - mine.m[3][0];
    float dz = theirs->z - mine.m[3][2];
    return (dx * dx + dz * dz) < 400.0f;
}

// FUNCTION: SWBF2 0x00668500
void UnitFollowAgent::UpdateStatePostVision(float dt)
{
    if (mCurState == 100)
    {
        return;
    }

    reinterpret_cast<AttackHelper*>(mAttackHelper)->Update();

    const GameObject* pTarget = ResolveHandle(mTarget);
    if (!pTarget)
    {
        SetNextState(100);
        return;
    }

    if (pTarget->GetControllable())
    {
        Character* pInner = static_cast<const Controllable*>(pTarget->GetControllable())->mCharacter;
        if (pInner)
        {
            Controllable* pDeep = pInner->mVehicle;
            if (pDeep)
            {
                Entity* pOwnerEntity = mCtrl->mOwner->GetEntity();
                if (!pOwnerEntity->IsRtti(kRttiHashEntityDroideka))
                {

                    GameObject* pDeepGameObject =
                        static_cast<Trackable*>(pDeep)->GetGameObject();
                    UnitCmdInfo cmdBoard = UnitBoardInfo(BuildHandle(pDeepGameObject), -1);
                    mCtrl->PrependQueueCommand(cmdBoard);
                    SetNextState(100);
                    return;
                }
                SetNextState(100);
                return;
            }
        }
    }

    switch (mCurState)
    {
    case 2:
        if (mTightFollowMode)
        {
            bool bIsSeatedSoldier = false;
            GameObject* pSelfObj = mCtrl->mOwner->GetGameObject();
            if (pSelfObj->IsRtti(kRttiHashEntitySoldier) &&
                static_cast<EntitySoldier*>(mCtrl->mOwner->GetGameObject())->mState == SoldierState::JET_HOVER)
            {
                bIsSeatedSoldier = true;
            }
            else
            {

                const PblMatrix& mine = mCtrl->mOwner->GetMatrix();
                const PblVector3* theirPos = GetTargetPosition(ResolveHandle(mTarget));
                float dx = theirPos->x - mine.m[3][0];
                float dy = theirPos->y - mine.m[3][1];
                float dz = theirPos->z - mine.m[3][2];
                if (dx * dx + dy * dy + dz * dz < 100.0f && CheckForForwardHintNodes())
                {
                    SetNextState(6);
                    break;
                }
            }

            float fScale = 2.0f;
            const GameObject* pTargetNow = ResolveHandle(mTarget);
            const PblVector3& vTargetVel = pTargetNow->GetVelocity();
            if (vTargetVel.x * vTargetVel.x + vTargetVel.y * vTargetVel.y + vTargetVel.z * vTargetVel.z < 16.0f)
            {
                fScale = 2.2f;
            }
            if (bIsSeatedSoldier)
            {
                fScale = 1.7f;
            }

            FormationType eFormation = GetFormationType(mCtrl);
            GameObject* pOwnerObj = mCtrl->mOwner->GetGameObject();
            pTargetNow = ResolveHandle(mTarget);
            PblVector3 vOffset = ReserveManager::ReserveOffset(pTargetNow, pOwnerObj, eFormation);
            vOffset.x *= fScale;
            vOffset.y *= fScale;
            vOffset.z *= fScale;
            pTargetNow = ResolveHandle(mTarget);
            reinterpret_cast<AILowLevel*>(mCtrl->mLowLevel)->SetNavigator_FollowTarget(pTargetNow, vOffset);

            if (!reinterpret_cast<AttackHelper*>(mAttackHelper)->HasTarget())
            {

                if (SeatOf(ResolveHandle(mTarget)))
                {
                    if (ResolveHandleAt(&SeatOf(ResolveHandle(mTarget))->mReticuleTarget[0]))
                    {
                        GameObject* pOccupant = ResolveHandleAt(&SeatOf(ResolveHandle(mTarget))->mReticuleTarget[0]);

                        const GameObject* pOwnerGameObject =
                            static_cast<const Trackable*>(mCtrl->mOwner)->GetGameObject();
                        if (pOwnerGameObject->GetAffiliationToMe(pOccupant) < 0)
                        {

                            GameObject* pOccupant2 =
                                const_cast<GameObject*>(ResolveHandle(const_cast<PblHandle<const GameObject>&>(
                                    ResolveHandle(mTarget)->GetControllable()->mReticuleTarget[0])));
                            reinterpret_cast<AI::UnitThreatManager*>(mCtrl->mThreatManager)->AddThreat(pOccupant2);
                        }
                    }
                }
            }
            FaceTarget();
        }
        else
        {
            SetNextState(5);
        }
        break;
    case 3:
        if (mTightFollowMode)
        {
            SetNextState(2);
        }
        else if (!IsInLooseRange())
        {
            SetNextState(5);
        }
        else if (CheckForForwardHintNodes())
        {
            SetNextState(6);
        }
        else
        {
            reinterpret_cast<AILowLevel*>(mCtrl->mLowLevel)->SetNavigator_Stationary();
        }
        break;
    case 4:
        if (!IsInLooseRange())
        {
            SetNextState(5);
        }
        else if (CheckForForwardHintNodes())
        {
            SetNextState(6);
        }
        break;
    case 5:
        if (reinterpret_cast<AILowLevel*>(mCtrl->mLowLevel)->IsNavigatorInProgress())
        {
            if (CheckForForwardHintNodes())
            {
                SetNextState(6);
            }
            else
            {
                FaceTarget();
            }
        }
        else
        {

            SetNextState(IsInLooseRange() ? 3 : 5);
        }
        break;
    case 6:
        FaceTarget(HEIGHT_STAND);
        mCoverHelper.FSMUpdate(dt, false);
        if (mCoverHelper.IsDone())
        {
            SetNextState(mTightFollowMode ? 2 : 5);
        }
        else
        {

            const PblMatrix& mine = mCtrl->mOwner->GetMatrix();
            const PblVector3* theirs = GetTargetPosition(ResolveHandle(mTarget));
            float dx = theirs->x - mine.m[3][0];
            float dy = theirs->y - mine.m[3][1];
            float dz = theirs->z - mine.m[3][2];
            if (dx * dx + dy * dy + dz * dz > 400.0f)
            {
                SetNextState(mTightFollowMode ? 2 : 5);
            }
        }
        break;
    default:
        break;
    }
}

// FUNCTION: SWBF2 0x00668a60
bool UnitFollowAgent::ShouldQueueCurCmd(const UnitCmdInfo& cmd) const
{
    return !SquadCommands::IsSquadCommand(cmd.GetCommand());
}

// FUNCTION: SWBF2 0x00668a80
int UnitFollowAgent::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_EmptyVehicle:
    case AI::EVT_SC_FollowChrLoose:
    case AI::EVT_EndInvisibility:
        return 0;

    case AI::EVT_Grenade:
        if (mCurState != 3 || UnitAgent::EventHandler(pInfo) != 1)
        {
            return 0;
        }
        mCombatCmd = pInfo->mNextCmd;
        SetNextState(4);
        pInfo->mNextCmd = UnitCmdInfo(CMD_NONE, PblHandle<GameObject>(), PblVector3(0, 0, 0), PblHandle<GameObject>());
        return 1;

    case AI::EVT_Damaged:
        if (mCurState == 3)
        {
            if (UnitAgent::HandleDamagedEvent(pInfo) != 1)
            {
                return 0;
            }
            mCombatCmd = pInfo->mNextCmd;
            SetNextState(4);
            pInfo->mNextCmd =
                UnitCmdInfo(CMD_NONE, PblHandle<GameObject>(), PblVector3(0, 0, 0), PblHandle<GameObject>());
            return 1;
        }
        UnitAgent::DamagedEventPlayerCentricness(pInfo);
        UnitAgent::PlayDamageEventVO(pInfo);
        return 0;

    case AI::EVT_Vision_CanSeeEnemy:
    case AI::EVT_Audible_CanHearEnemy:
        if (mCurState != 3 || UnitAgent::EventHandler(pInfo) != 1)
        {
            return 0;
        }
        mCombatCmd = pInfo->mNextCmd;
        SetNextState(4);
        pInfo->mNextCmd = UnitCmdInfo(CMD_NONE, PblHandle<GameObject>(), PblVector3(0, 0, 0), PblHandle<GameObject>());
        return 1;

    case AI::EVQ_SC_IsFollowingChr:
        if (ResolveHandle(mTarget) != ResolveHandle(pInfo->mGameObject))
        {
            return 0;
        }
        ++pInfo->mInt;
        return 1;

    case AI::EVT_SC_FollowChr:
        if (mTightFollowMode)
        {
            return 0;
        }
        mTightFollowMode = true;
        return 1;

    case AI::EVT_SC_StopFollowChr:
        if (ResolveHandle(mTarget) != ResolveHandle(pInfo->mGameObject))
        {
            return -1;
        }

        AI::VOHelper::Play(AI::VOT_SC_StopFollowResponse, mCtrl->mOwner->GetGameObject(), 0, 0,
                           AIUtil::GetRandomFloat(0.5f, 1.2f) + GameLoop::GetMissionTime());
        SetNextState(100);
        return 1;

    case AI::EVQ_GetSkillModifier:

        if (ResolveHandle(mTarget) &&
            PlayerControllerPtr(reinterpret_cast<Controllable*>(SeatOf(ResolveHandle(mTarget)))))
        {
            pInfo->mInt = 6;
            return 1;
        }
        pInfo->mInt = 0;
        return 0;

    default:
        return UnitAgent::EventHandler(pInfo);
    }
}

// FUNCTION: SWBF2 0x00668d40
const char* UnitFollowAgent::NFINAL_GetStateString(int iState) const
{
    return 0;
}
