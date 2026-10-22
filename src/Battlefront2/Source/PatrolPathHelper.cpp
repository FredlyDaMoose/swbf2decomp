#include "PatrolPathHelper.h"

#include <float.h>

#include "AILowLevel.h"
#include "AIUtil.h"
#include "Controllable.h"
#include "Controller.h"
#include "PathManager.h"
#include "PatrolPath.h"
#include "UnitAgent.h"
#include "UnitController.h"
#include "EventInfo.h"
#include "PebbleFL/Common/PblMatrix.h"

namespace
{

    AILowLevel& LowLevel(UnitController* pCtrl)
    {
        return *reinterpret_cast<AILowLevel*>(pCtrl->mLowLevel);
    }

    const PblVector3& Position(const PblMatrix& mat)
    {
        return *reinterpret_cast<const PblVector3*>(mat.m[3]);
    }
}

// FUNCTION: SWBF2 0x00615e90
PatrolPathHelper::PatrolPathHelper(UnitAgent* pAgent)
    : HelperFsm(STATE_START, STATE_START)
{
    mAgent = pAgent;
    mPatrolPath = NULL;
}

// FUNCTION: SWBF2 0x006160d0
void PatrolPathHelper::ExitState()
{
}

// FUNCTION: SWBF2 0x00616010
UnitController* PatrolPathHelper::GetCtrl()
{
    return mAgent->mCtrl;
}

// FUNCTION: SWBF2 0x00615ec0
bool PatrolPathHelper::CheckForPatrolPathsAndInit()
{
    AI::PatrolPath* paths[10];
    const PblVector3& vPos = Position(GetMe()->GetMatrix());
    int iCount = AI::PatrolPath::RequestPatrolPaths(Position(GetMe()->GetMatrix()), 50.0f, 10, paths);

    float fBestDistSq = FLT_MAX;
    for (int i = 0; i < iCount; ++i)
    {
        AI::PatrolPath* pPath = paths[i];
        if (pPath->GetReservingObject())
        {
            continue;
        }

        const PblVector3* pStart = pPath->GetPointPosition(0);
        float dx = pStart->x - vPos.x;
        float dy = pStart->y - vPos.y;
        float dz = pStart->z - vPos.z;
        float fDistSq = dx * dx + dy * dy + dz * dz;
        if (fDistSq < fBestDistSq)
        {
            paths[0] = pPath;
            fBestDistSq = fDistSq;
        }
    }

    if (fBestDistSq < FLT_MAX)
    {
        mPatrolPath = paths[0];
        mPatrolPath->SetReservingObject(GetMe()->GetGameObject());
        SetNextState(STATE_GotoStart);
        FSMChangeState();
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00615fe0
void PatrolPathHelper::HELPERClean()
{
    if (mPatrolPath)
    {
        mPatrolPath->SetReservingObject(0);
        mPatrolPath = 0;
    }
    ExitState();
}

// FUNCTION: SWBF2 0x00616000
Controllable* PatrolPathHelper::GetMe()
{
    return mAgent->mCtrl->mOwner;
}

// FUNCTION: SWBF2 0x00616020
void PatrolPathHelper::EnterState()
{
    switch (mCurState)
    {
    case STATE_GotoStart:
    {
        PblVector3 vStart = *mPatrolPath->GetPointPosition(0);
        LowLevel(GetCtrl()).SetNavigator_Goto(vStart, 0, false, false);
        break;
    }
    case STATE_RunPath:
    {
        AIPath* pPath = mPatrolPath->GetAIPath();
        LowLevel(GetCtrl()).SetNavigator_FollowPath(pPath, true, true, false);
        pPath->Release();
        break;
    }
    }
}

// FUNCTION: SWBF2 0x006160e0
void PatrolPathHelper::UpdateState(float dt)
{
    if (mCurState == 100)
    {
        return;
    }

    switch (mCurState)
    {
    case STATE_GotoStart:
        mPatrolPath->SetReservingObject(GetMe()->GetGameObject());
        LowLevel(GetCtrl()).SetHeadingDefault();
        LowLevel(GetCtrl()).SetHeight(HEIGHT_WALK);
        if (!LowLevel(GetCtrl()).IsNavigatorInProgress())
        {
            const PblVector3& vPos = Position(GetMe()->GetMatrix());
            const PblVector3* pStart = mPatrolPath->GetPointPosition(0);
            float dx = vPos.x - pStart->x;
            float dy = vPos.y - pStart->y;
            float dz = vPos.z - pStart->z;
            if (dx * dx + dy * dy + dz * dz < 1.0f)
            {
                SetNextState(STATE_RunPath);
            }
            else
            {
                SetNextState(100);
            }
        }
        break;

    case STATE_RunPath:
        mPatrolPath->SetReservingObject(GetMe()->GetGameObject());
        LowLevel(GetCtrl()).SetHeadingDefault();
        LowLevel(GetCtrl()).SetHeight(HEIGHT_WALK);
        if (!LowLevel(GetCtrl()).IsNavigatorInProgress())
        {
            SetNextState(100);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x00616210
void PatrolPathHelper::DoNodeActions(const PblVector3& vPos)
{
    if (!mPatrolPath)
    {
        return;
    }
    for (int i = 0; i < mPatrolPath->GetNumPoints(); ++i)
    {
        const PblVector3* pPoint = mPatrolPath->GetPointPosition(i);
        const float dx = vPos.x - pPoint->x;
        const float dy = vPos.y - pPoint->y;
        const float dz = vPos.z - pPoint->z;
        if (!(0.0001f > dx * dx + dy * dy + dz * dz))
        {
            continue;
        }
        const AI::PatrolPath::NodeProperties& props = mPatrolPath->GetNodeProperties(i);
        float fWait = AIUtil::GetRandomFloat(props.mWaitMin, props.mWaitMax);
        if (fWait > 0.0f)
        {
            LowLevel(GetCtrl()).Navigator_Wait(fWait);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x006162d0
int PatrolPathHelper::EventHandler(AI::EventInfo* pInfo)
{
    if (pInfo->mType != AI::EVT_Path_NextNode || mCurState != STATE_RunPath)
    {
        return AI::EVT_NotHandled;
    }
    DoNodeActions(pInfo->mPos);
    return AI::EVT_Handled;
}
