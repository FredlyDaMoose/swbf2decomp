#include "DefendHelper.h"

#include <float.h>

#include "AILowLevel.h"
#include "AIUtil.h"
#include "Character.h"
#include "Controllable.h"
#include "Entity.h"
#include "HintManager.h"
#include "PathFinder.h"
#include "PathManager.h"
#include "UnitAgent.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblRandom.h"

class EntitySoldier
{
public:
    static PblHash rttiHashEntitySoldier;
};

namespace
{

    const PblVector3& GetMatrixPos(const PblMatrix& m)
    {
        return *(const PblVector3*)&m.m[3][0];
    }

    float DistSq(const PblVector3& a, const PblVector3& b)
    {
        float fX = a.x - b.x;
        float fY = a.y - b.y;
        float fZ = a.z - b.z;
        return fX * fX + fY * fY + fZ * fZ;
    }
}

// FUNCTION: SWBF2 0x0048b6c0
DefendHelper::DefendHelper(UnitAgent* pOwner)
    : HelperFsm(STATE_START, STATE_START),
      mOwner(pOwner),
      mSnipeHelper(pOwner),
      mRepairHelper(pOwner),
      mPatrolPathHelper(pOwner),
      mIdleHelper(pOwner),
      mMineHelper(pOwner),
      mLastFortificationPos(0.0f, 0.0f, 0.0f),
      mFortificationTime(0.0f),
      mFortificationHint(0)
{
    mTarget.mType = AI::GTT_NONE;
}

DefendHelper::~DefendHelper()
{
}

// FUNCTION: SWBF2 0x0048b760
void DefendHelper::Kill()
{
    mMineHelper.Kill();
    mIdleHelper.Kill();
    mPatrolPathHelper.Kill();
    mSnipeHelper.Kill();
    Fsm::Kill();
}

void DefendHelper::Clean()
{
    ExitState();
}

// FUNCTION: SWBF2 0x0048b7b0
void DefendHelper::Init(const AI::GoalTarget& target)
{
    mTarget = target;
    SetNextState(STATE_Goto);
    FSMChangeState();
}

// FUNCTION: SWBF2 0x0048b810
float DefendHelper::GetUpdateRate() const
{
    if (mCurState == STATE_Idle || mCurState == STATE_GotoFortification || mCurState == STATE_Snipe)
    {
        return 2.0f;
    }
    return 1.5f;
}

UnitController* DefendHelper::GetCtrl()
{
    return mOwner->mCtrl;
}

Controllable* DefendHelper::GetMe()
{
    return mOwner->mCtrl->mOwner;
}

AILowLevel& DefendHelper::GetLowLevel()
{
    return *reinterpret_cast<AILowLevel*>(GetCtrl()->mLowLevel);
}

// FUNCTION: SWBF2 0x0048b840
void DefendHelper::EnterState()
{
    switch (mCurState)
    {
    case STATE_Goto:
    {
        mFortificationTime = FLT_MAX;
        PblVector3 vPos;
        if (!mTarget.GetLocation(vPos))
        {
            if (mTarget.mType == AI::GTT_GAMEOBJECT)
            {
                SetNextState(100);
            }
            else if (mTarget.mType == AI::GTT_CHARACTER)
            {
                SetNextState(STATE_WaitForSpawn);
            }
            else
            {
                SetNextState(100);
            }
            return;
        }

        PathFinder::GetClosestValidPoint(vPos, vPos, GetMe()->GetGameObject());
        mLastSeekPos = vPos;

        PblVector3 vDest = PblRandom::GetGlobalVector3();
        float fDist = AIUtil::GetRandomFloat(2.0f, 5.0f);
        PblVector3 vDir = AIUtil::GetSafeNormalXZ(vDest);
        vDest.x = vPos.x + fDist * vDir.x;
        vDest.y = vPos.y + vDir.y * fDist;
        vDest.z = vPos.z + vDir.z * fDist;

        float fRadius = AIUtil::GetCollisionRadius(GetMe()->GetGameObject());
        int eProhibit = AIUtil::GetAIType(GetMe()->GetGameObject());
        const Obstacle* pIgnore = AIUtil::GetGameObjectBarrier(GetMe()->GetGameObject());
        PblVector3 vNormal;
        PathFinder::PushValidPoint(vPos, vDest, vNormal, fRadius, eProhibit, &pIgnore, 1);

        GetLowLevel().SetNavigator_Goto(vDest, 0, true, false);
        break;
    }

    case STATE_Idle:
        mIdleHelper.Init(mFortificationHint ? IH_Stationary : IH_ALL, mFortificationHint);
        break;

    case STATE_GotoFortification:
    {
        if (!mFortificationHint)
        {
            SetNextState(STATE_Goto);
            return;
        }

        PblVector3 vPos = mFortificationHint->GetCollisionCenter();

        const PblVector3& vMyPos = GetMatrixPos(GetMe()->GetMatrix());
        (void)vMyPos;
        PathFinder::GetClosestValidPoint(vPos, vPos, GetMe()->GetGameObject());
        GetLowLevel().SetNavigator_Goto(vPos, 0, true, false);
        GetLowLevel().SetHeight(HEIGHT_WALK);
        break;
    }

    case STATE_DoPatrol:
    {

        const Obstacle* pHit = 0;
        const PblVector3& vMyPos = GetMatrixPos(GetMe()->GetMatrix());
        GameObject* pObj = GetMe()->GetGameObject();
        PblVector3 vOutPos;

        bool bFound = false;
        if ((PblRandom::GetGlobalInt() & 0x10) &&
            !PathFinder::IsPointValid(vMyPos, 5.0f, AIUtil::GetAIType(pObj), &pHit, 0, 0))
        {
            int iStart = PblRandom::GetGlobalInt() & 3;
            for (int i = 0; i < 4; ++i)
            {
                vOutPos = pHit->GetPushedOutVertexXZ((iStart + i) & 3,
                                                     AIUtil::GetCollisionRadius(pObj) * 1.1f);
                if (!PathFinder::TestTrivialStraightLineHit(vMyPos, vOutPos, pObj))
                {
                    bFound = true;
                    break;
                }
            }
        }
        if (!bFound)
        {
            PblVector3 vOffset = PblRandom::GetGlobalVector3();
            vOffset.x *= 15.0f;
            vOffset.y *= 15.0f;
            vOffset.z *= 15.0f;
            vOffset.y = 0.0f;
            vOutPos.x = vMyPos.x + vOffset.x;
            vOutPos.y = vMyPos.y + vOffset.y;
            vOutPos.z = vMyPos.z + vOffset.z;
            PathFinder::PushValidPoint(vMyPos, vOutPos, pObj);
        }

        PblVector3 vDest = vMyPos;
        const BaseHint* hints[10];
        int iCount = HintManager::RequestFortificationHints(GetCtrl(), vOutPos, 20, 10, hints);
        for (int i = 0; i < iCount; ++i)
        {
            if (AIUtil::IsHintAvailable(pObj, hints[i]) &&
                !PathFinder::TestTrivialStraightLineHit(vOutPos, hints[i]->GetCollisionCenter(), pObj))
            {
                vDest = hints[i]->GetCollisionCenter();
                mFortificationHint = const_cast<BaseHint*>(hints[i]);
                hints[i]->Reserve(pObj);
                break;
            }
        }

        AIPath* pPath = new AIPath();
        if (pPath)
        {
            pPath->Append(vMyPos, 0);
            pPath->Append(vOutPos, (PblRandom::GetGlobalInt() & 0x10)
                                       ? AIPathNode::AIPN_WAITLOOKAROUND
                                       : AIPathNode::AIPN_WAITLEFT | AIPathNode::AIPN_WAITRIGHT |
                                             AIPathNode::AIPN_WAITLONG);
            pPath->Append(vDest, 0);
            GetLowLevel().SetNavigator_FollowPath(pPath, false, true, false);
            pPath->Release();
        }
        GetLowLevel().SetHeight(HEIGHT_WALK);
        break;
    }
    }
}

// FUNCTION: SWBF2 0x0048bdc0
void DefendHelper::ExitState()
{
    switch (mCurState)
    {
    case STATE_Idle:
        mIdleHelper.Clean();
        break;
    case STATE_DoPatrolPath:
        mPatrolPathHelper.HELPERClean();
        break;
    case STATE_Snipe:
        mSnipeHelper.HELPERClean();
        break;
    case STATE_Mine:
        mMineHelper.HELPERClean();
        break;
    }
}

// FUNCTION: SWBF2 0x0048be20
bool DefendHelper::CheckTargetPosition(const PblVector3& vPos)
{
    const PblVector3& vMyPos = GetMatrixPos(GetMe()->GetMatrix());
    if (DistSq(vMyPos, vPos) > 2500.0f)
    {
        float fRange = mCurState == STATE_Goto ? 25.0f : 5.0f;
        if (DistSq(vPos, mLastSeekPos) > fRange * fRange)
        {
            SetNextState(STATE_Goto);
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0048bef0
bool DefendHelper::ChooseHint(const BaseHint** ppHints, int iCount, const PblVector3& vPos)
{
    GameObject* pObj = GetMe()->GetGameObject();
    const BaseHint* pBest = 0;
    float fBestDistSq = FLT_MAX;
    for (int i = 0; i < iCount; ++i)
    {
        const BaseHint* pHint = ppHints[i];
        if (AIUtil::IsHintAvailable(pObj, pHint))
        {
            float fDistSq = DistSq(vPos, pHint->GetCollisionCenter());
            if (fBestDistSq > fDistSq)
            {
                pBest = pHint;
                fBestDistSq = fDistSq;
            }
        }
    }

    if (!pBest)
    {
        return false;
    }

    mFortificationHint = const_cast<BaseHint*>(pBest);
    pBest->Reserve(pObj);
    SetNextState(STATE_GotoFortification);
    return true;
}

// FUNCTION: SWBF2 0x0048bfe0
bool DefendHelper::CheckEnterFortification(float dt, const PblVector3& vPos)
{
    if (!GetMe()->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return false;
    }

    if (DistSq(vPos, mLastFortificationPos) < 625.0f)
    {
        mFortificationTime += dt;
    }
    else
    {
        mFortificationTime = 0.0f;
        mLastFortificationPos = vPos;
    }

    if (mFortificationTime > 10.0f)
    {
        mFortificationTime = 8.0f;
        const BaseHint* hints[30];
        int iCount = HintManager::RequestFortificationHints(GetCtrl(), vPos, 50, 30, hints);
        if (ChooseHint(hints, iCount, vPos))
        {
            return true;
        }
        iCount = HintManager::RequestCoverHints(GetCtrl(), vPos, 50, 30, hints);
        if (ChooseHint(hints, iCount, vPos))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0048c130
bool DefendHelper::CheckExitFortification(const PblVector3& vPos)
{
    if (DistSq(vPos, mLastFortificationPos) > 625.0f)
    {
        mFortificationTime = 0.0f;
        mLastFortificationPos = vPos;
        SetNextState(STATE_Goto);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0048c1b0
void DefendHelper::UpdateState(float dt)
{
    if (mCurState == 100)
    {
        return;
    }

    PblVector3 vPos(0.0f, 0.0f, 0.0f);
    if (mCurState != STATE_WaitForSpawn && !mTarget.GetLocation(vPos))
    {
        if (mTarget.mType == AI::GTT_CHARACTER)
        {
            SetNextState(STATE_WaitForSpawn);
        }
        else
        {
            SetNextState(100);
        }
        return;
    }

    PathFinder::GetClosestValidPoint(vPos, vPos, GetMe()->GetGameObject());
    GetLowLevel().SetHeight(HEIGHT_STAND);

    switch (mCurState)
    {
    case STATE_Goto:
        if (CheckTargetPosition(vPos))
        {
            break;
        }
        if (!GetLowLevel().IsNavigatorInProgress())
        {
            SetNextState(STATE_Idle);
        }
        else if (mMineHelper.CheckForHintsAndInit())
        {
            SetNextState(STATE_Mine);
        }
        else
        {
            CheckEnterFortification(dt, vPos);
            mOwner->FaceTarget(HEIGHT_STAND);
        }
        break;

    case STATE_Idle:
        if (CheckTargetPosition(vPos))
        {
            break;
        }
        if (mFortificationHint)
        {
            mFortificationHint->Reserve(GetMe()->GetGameObject());
            if (CheckExitFortification(vPos))
            {
                break;
            }
            if (AIUtil::GetRandomInt(0, 100) > 90)
            {
                SetNextState(STATE_DoPatrol);
                break;
            }
        }
        else
        {
            CheckEnterFortification(dt, vPos);
        }

        if (mSnipeHelper.LookForSnipeHintAndInit() == true)
        {
            SetNextState(STATE_Snipe);
        }
        else if (mRepairHelper.CheckForRecipientAndInit() == true)
        {
            SetNextState(STATE_Repair);
        }
        else if (mPatrolPathHelper.CheckForPatrolPathsAndInit())
        {
            SetNextState(STATE_DoPatrolPath);
        }
        else if (mMineHelper.CheckForHintsAndInit())
        {
            SetNextState(STATE_Mine);
        }
        else
        {
            mIdleHelper.FSMUpdate(dt, false);
            if (mIdleHelper.IsDone())
            {
                SetNextState(STATE_Goto);
            }
        }
        break;

    case STATE_WaitForSpawn:
        if (mMineHelper.CheckForHintsAndInit())
        {
            SetNextState(STATE_Mine);
        }
        else if (mTarget.mChr->GetControllable())
        {
            SetNextState(STATE_Goto);
        }
        break;

    case STATE_GotoFortification:
    case STATE_DoPatrol:
        GetLowLevel().SetHeight(HEIGHT_WALK);
        GetLowLevel().SetHeadingDefault();
        GetLowLevel().mHeading.mDestWander = 0.4f;
        mFortificationHint->Reserve(GetMe()->GetGameObject());
        if (CheckExitFortification(vPos))
        {
            break;
        }

        if (mSnipeHelper.LookForSnipeHintAndInit() == true)
        {
            SetNextState(STATE_Snipe);
        }
        else if (mRepairHelper.CheckForRecipientAndInit())
        {
            SetNextState(STATE_Repair);
        }
        else if (mMineHelper.CheckForHintsAndInit())
        {
            SetNextState(STATE_Mine);
        }
        else if (!GetLowLevel().IsNavigatorInProgress())
        {
            vPos = mFortificationHint->GetCollisionCenter();
            if (DistSq(vPos, GetMatrixPos(GetMe()->GetMatrix())) > 25.0f)
            {
                SetNextState(STATE_Goto);
            }
            else
            {
                SetNextState(STATE_Idle);
            }
        }
        break;

    case STATE_DoPatrolPath:
        mPatrolPathHelper.FSMUpdate(dt, false);
        if (mPatrolPathHelper.IsDone())
        {
            SetNextState(STATE_Goto);
        }
        break;

    case STATE_Snipe:
        mSnipeHelper.FSMUpdate(dt, false);
        if (mSnipeHelper.IsDone())
        {
            SetNextState(STATE_Goto);
        }
        break;

    case STATE_Repair:
        if (!mRepairHelper.Update(dt))
        {
            SetNextState(STATE_Goto);
        }
        break;

    case STATE_Mine:
        mMineHelper.FSMUpdate(dt, false);
        if (mMineHelper.IsDone())
        {
            SetNextState(STATE_Goto);
        }
        else
        {
            GetLowLevel().SetHeadingDefault();
            GetLowLevel().SetHeight(HEIGHT_STAND);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x0048c680
AI::EVTResult DefendHelper::EventHandler(AI::EventInfo* pInfo)
{
    if (pInfo->mType == AI::EVT_Path_NextNode && mCurState == STATE_DoPatrolPath)
    {
        return (AI::EVTResult)mPatrolPathHelper.EventHandler(pInfo);
    }
    return AI::EVT_NotHandled;
}
