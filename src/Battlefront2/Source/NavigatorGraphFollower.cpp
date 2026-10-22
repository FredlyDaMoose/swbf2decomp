#include <string.h>

#include "NavigatorGraphFollower.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "ConnectivityGraph.h"
#include "ConnectivityGraphFollower.h"
#include "Controllable.h"
#include "EventInfo.h"
#include "PathFinder.h"
#include "PathFollower.h"
#include "PathManager.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMatrix.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

namespace
{

    const PblVector3& GetPos(const PblMatrix& m)
    {
        return *(const PblVector3*)&m.m[3][0];
    }
}

// GLOBAL: SWBF2 0x01ecb250
// SYNTHETIC: SWBF2 0x004043d0
// `dynamic initializer for 'NavigatorGraphFollower::rttiHashNavigatorGraphFollower''
PblHash NavigatorGraphFollower::rttiHashNavigatorGraphFollower("NavigatorGraphFollower");
char NavigatorGraphFollower::rttiNameNavigatorGraphFollower[] = "NavigatorGraphFollower";

// FUNCTION: SWBF2 0x005ac4e0
bool NavigatorGraphFollower::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashNavigatorGraphFollower || Navigator::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x005ad230
unsigned int NavigatorGraphFollower::GetDerivedRtti() const
{
    return rttiHashNavigatorGraphFollower;
}

// FUNCTION: SWBF2 0x005ad240
const char* NavigatorGraphFollower::GetDerivedRttiName() const
{
    return rttiNameNavigatorGraphFollower;
}

// SYNTHETIC: SWBF2 0x005ad250
// NavigatorGraphFollower::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005ad310
NavigatorGraphFollower::NavigatorGraphFollower(AILowLevel* pOwner, const PblVector3& globalDest,
                                               const Obstacle** ppIgnore, int iIgnoreCnt,
                                               bool bRollIntoDest)
    : Navigator(pOwner)
{
    mGlobalDest = globalDest;
    mRollIntoDest = bRollIntoDest;
    mGraphFollower = 0;
    memcpy(mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));
    mIgnoreCnt = iIgnoreCnt;
    mCurState = STATE_START;
    mNextState = STATE_PATHING;
}

// FUNCTION: SWBF2 0x005ad370
NavigatorGraphFollower::~NavigatorGraphFollower()
{
    if (mGraphFollower)
    {
        delete mGraphFollower;
        mGraphFollower = 0;
    }
}

// FUNCTION: SWBF2 0x005ad730
void NavigatorGraphFollower::ExitState()
{
}

// FUNCTION: SWBF2 0x005ad740
void NavigatorGraphFollower::UpdateState(float dt)
{
    if (mCurState == STATE_PATHING)
    {
        mPathFollower->Update(dt);
        if (mPathFollower->mDone && mPathFollower->mDoneReason == PF_PATHBLOCKED)
        {
            SetNextState(STATE_PATHING);
        }
        else if (mPathFollower->mDone && !mGraphFollower)
        {
            delete mPathFollower;
            mPathFollower = 0;
            SetNextState(100);

            AI::EventInfo info(AI::EVT_Nav_NavigatorFinished);
            mOwner->mOwner->SendEvent(&info);
        }
    }
}

// FUNCTION: SWBF2 0x005adac0
const PblVector3& NavigatorGraphFollower::GetGlobalDest() const
{
    return mGlobalDest;
}

// FUNCTION: SWBF2 0x005ad3e0
void NavigatorGraphFollower::EnterState()
{
    if (mCurState != STATE_PATHING)
    {
        return;
    }

    Controllable* pMe = mOwner->mOwner->mOwner;
    PblVector3 vFrom = GetPos(pMe->GetMatrix());
    PblVector3 vTo = mGlobalDest;
    const GameObject* pObj = pMe->GetGameObject();

    delete mPathFollower;
    mPathFollower = new PathFollower(mOwner->mOwner, vFrom, vFrom, (const Obstacle**)mIgnore,
                                     mIgnoreCnt, false);

    delete mGraphFollower;
    mGraphFollower = new ConnectivityGraphFollower(mOwner->mOwner);
    if (!mGraphFollower)
    {
        return;
    }

    mGraphFollower->SetDestination(
        ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1), vFrom, vTo,
        (ConnectivityGraph::ProhibitFlag)AIUtil::GetAIType(pObj));

    if (!PathFinder::IsPointValid(mGraphFollower->GetCurrentDestination(),
                                  AIUtil::GetCollisionRadius(pObj), AIUtil::GetAIType(pObj), 0,
                                  (const Obstacle**)mIgnore, mIgnoreCnt))
    {
        mGraphFollower->GetNewValidRandomPoint(AIUtil::GetCollisionRadius(pObj));
    }

    const PblVector3& vDest = mGraphFollower->GetCurrentDestination();
    PathFinder::GetClosestValidPoint(vFrom, vFrom, AIUtil::GetCollisionRadius(pObj),
                                     AIUtil::GetAIType(pObj), (const Obstacle**)mIgnore, mIgnoreCnt);

    unsigned int uArcFlags = mGraphFollower->GetCurrentArcFlags();
    if (uArcFlags)
    {
        unsigned short uFlags = (uArcFlags & 2) ? AIPathNode::AIPN_JUMP : 0;
        if (uArcFlags & 4)
        {
            uFlags |= AIPathNode::AIPN_JETJUMP;
        }

        AIPath* pPath = new AIPath();
        if (pPath)
        {
            pPath->Prepend(vDest, 0);
            PblVector3 vDelta(vDest.x - vFrom.x, vDest.y - vFrom.y, vDest.z - vFrom.z);
            PblVector3 vDir;
            D3DXVec3Normalize(&vDir, &vDelta);
            PblVector3 vJump(vDir.x * 0.1f + vFrom.x, vDir.y * 0.1f + vFrom.y, vDir.z * 0.1f + vFrom.z);
            pPath->Prepend(vJump, uFlags);
            pPath->Prepend(vFrom, 0);

            AI::EventInfo_PathResult info(pPath);
            mOwner->mOwner->SendEvent(&info);
            pPath->Release();
        }
    }
    else
    {
        PathManager::RequestPath(mOwner->mOwner, vFrom, vDest, AIUtil::GetCollisionRadius(pObj),
                                 AIUtil::GetAIType(pObj), 1, 0, (const Obstacle**)mIgnore,
                                 mIgnoreCnt);
    }
}

// FUNCTION: SWBF2 0x005ad7e0
void NavigatorGraphFollower::ReceivePathResult(AIPath* pPath)
{
    if (pPath)
    {
        if (mRollIntoDest)
        {
            pPath->GetNodeList().GetLast()->_pObject->mFlags |= AIPathNode::AIPN_ROLLIN;
        }
        if (mPathFollower)
        {
            mPathFollower->AppendPath(pPath, false);
        }
    }
    else
    {
        mFailedPathSearch = true;
    }

    if (!mGraphFollower)
    {
        return;
    }

    if (!(mGraphFollower->GetCurrentDestination() != mGlobalDest))
    {
        delete mGraphFollower;
        mGraphFollower = 0;
        return;
    }

    const GameObject* pObj = mOwner->mOwner->mOwner->GetGameObject();
    PblVector3 vFrom = mGraphFollower->GetCurrentDestination();
    if (!mGraphFollower->GotoNextLeg())
    {
        return;
    }

    PblVector3 vTo = mGraphFollower->GetCurrentDestination();
    if (!PathFinder::IsPointValid(vTo, AIUtil::GetCollisionRadius(pObj), AIUtil::GetAIType(pObj), 0, 0, 0))
    {
        if (mGraphFollower->GetNewValidRandomPoint(AIUtil::GetCollisionRadius(pObj)) != true)
        {
            delete mGraphFollower;
            mGraphFollower = 0;
            return;
        }
        vTo = mGraphFollower->GetCurrentDestination();
    }

    unsigned int uArcFlags = mGraphFollower->GetCurrentArcFlags();
    if (uArcFlags)
    {
        unsigned short uFlags = (uArcFlags & 2) ? AIPathNode::AIPN_JUMP : 0;
        if (uArcFlags & 4)
        {
            uFlags |= AIPathNode::AIPN_JETJUMP;
        }

        AIPath* pLegPath = new AIPath();
        if (pLegPath)
        {
            pLegPath->Prepend(vTo, 0);
            PblVector3 vDelta(vTo.x - vFrom.x, vTo.y - vFrom.y, vTo.z - vFrom.z);
            PblVector3 vDir;
            D3DXVec3Normalize(&vDir, &vDelta);
            PblVector3 vJump(vDir.x * 0.1f + vFrom.x, vDir.y * 0.1f + vFrom.y, vDir.z * 0.1f + vFrom.z);
            pLegPath->Prepend(vJump, uFlags);
            pLegPath->Prepend(vFrom, 0);

            AI::EventInfo_PathResult info(pLegPath);
            mOwner->mOwner->SendEvent(&info);
            pLegPath->Release();
        }
    }
    else
    {
        PathManager::RequestPath(mOwner->mOwner, vFrom, vTo, AIUtil::GetCollisionRadius(pObj),
                                 AIUtil::GetAIType(pObj), 1, 0, (const Obstacle**)mIgnore,
                                 mIgnoreCnt);
    }
}
