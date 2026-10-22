#include <string.h>

#include "NavigatorPathFollower.h"
#include "AILowLevel.h"
#include "EventInfo.h"
#include "PathFollower.h"
#include "UnitController.h"

// GLOBAL: SWBF2 0x01ecb254
// SYNTHETIC: SWBF2 0x004043e0
// `dynamic initializer for 'NavigatorPathFollower::rttiHashNavigatorPathFollower''
PblHash NavigatorPathFollower::rttiHashNavigatorPathFollower("NavigatorPathFollower");
char NavigatorPathFollower::rttiNameNavigatorPathFollower[] = "NavigatorPathFollower";

// FUNCTION: SWBF2 0x005adad0
bool NavigatorPathFollower::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashNavigatorPathFollower || Navigator::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x005adb00
unsigned int NavigatorPathFollower::GetDerivedRtti() const
{
    return rttiHashNavigatorPathFollower;
}

// FUNCTION: SWBF2 0x005adb10
const char* NavigatorPathFollower::GetDerivedRttiName() const
{
    return rttiNameNavigatorPathFollower;
}

// SYNTHETIC: SWBF2 0x005adb20
// NavigatorPathFollower::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005adb50
NavigatorPathFollower::NavigatorPathFollower(AILowLevel* pOwner, bool bDoCallbacksOnPts,
                                             const Obstacle** ppIgnore, int iIgnoreCnt,
                                             bool bRollIntoDest)
    : Navigator(pOwner)
{
    mRollIntoDest = bRollIntoDest;
    mDoCallbacksOnPts = bDoCallbacksOnPts;
    mCurState = STATE_WAITFORPATH;
    mIgnoreCnt = iIgnoreCnt;
    memcpy(mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));
}

// FUNCTION: SWBF2 0x005adb90
NavigatorPathFollower::NavigatorPathFollower(AILowLevel* pOwner, bool bDoCallbacksOnPts,
                                             const Obstacle** ppIgnore, int iIgnoreCnt,
                                             bool bRollIntoDest, AIPath* pPath)
    : Navigator(pOwner)
{
    mRollIntoDest = bRollIntoDest;
    mDoCallbacksOnPts = bDoCallbacksOnPts;
    mCurState = STATE_START;
    mNextState = STATE_FOLLOWPATH;
    mIgnoreCnt = iIgnoreCnt;
    memcpy(mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));
    mPathFollower = new PathFollower(mOwner->mOwner, pPath, (const Obstacle**)mIgnore,
                                     mIgnoreCnt, mDoCallbacksOnPts);
}

// FUNCTION: SWBF2 0x005adc50
NavigatorPathFollower::~NavigatorPathFollower()
{
}

// FUNCTION: SWBF2 0x005adc60
void NavigatorPathFollower::EnterState()
{
}

// FUNCTION: SWBF2 0x005adc70
void NavigatorPathFollower::ExitState()
{
}

// FUNCTION: SWBF2 0x005adc80
void NavigatorPathFollower::UpdateState(float dt)
{
    if (mCurState == STATE_FOLLOWPATH)
    {
        mPathFollower->Update(dt);
        if (mPathFollower->mDone)
        {
            delete mPathFollower;
            mPathFollower = 0;
            SetNextState(100);

            AI::EventInfo info(AI::EVT_Nav_NavigatorFinished);
            mOwner->mOwner->SendEvent(&info);
        }
    }
}

// FUNCTION: SWBF2 0x005add00
void NavigatorPathFollower::ReceivePathResult(AIPath* pPath)
{
    if (!pPath)
    {
        mFailedPathSearch = true;
        SetNextState(100);
        return;
    }

    if (mRollIntoDest)
    {
        pPath->GetNodeList().GetLast()->_pObject->mFlags |= AIPathNode::AIPN_ROLLIN;
    }

    if (mCurState == STATE_WAITFORPATH)
    {
        mPathFollower = new PathFollower(mOwner->mOwner, pPath, (const Obstacle**)mIgnore,
                                         mIgnoreCnt, mDoCallbacksOnPts);
        SetNextState(STATE_FOLLOWPATH);
    }
    else if (mPathFollower)
    {
        mPathFollower->AppendPath(pPath, false);
    }
}
