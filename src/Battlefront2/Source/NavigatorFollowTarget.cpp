#include "NavigatorFollowTarget.h"
#include "AILowLevel.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "ConnectivityGraph.h"
#include "ConnectivityGraphFollower.h"
#include "EntitySoldier.h"
#include "GameLoop.h"
#include "PathFinder.h"
#include "PathFollower.h"
#include "PathManager.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);

namespace ReserveManager
{
    int GetReserveOrder(const GameObject* pObj, const GameObject* pReserver, ReserveType eType);
}

namespace
{
    unsigned int GameObjectHandleId(const GameObject* pObject)
    {
        return pObject->mHandleId;
    }

    const GameObject* HandleObject(PblHandle<const GameObject>& handle)
    {
        if (handle.mObject && GameObjectHandleId(handle.mObject) != handle.mSavedHandleId)
        {
            handle.mObject = 0;
            handle.mSavedHandleId = 0;
        }
        return handle.mObject;
    }

    const PblVector3& GameObjectPosition(const GameObject* pObject)
    {
        return static_cast<const CollisionObject&>(*pObject).GetPosition();
    }

    const PblMatrix& GameObjectMatrix(const GameObject* pObject)
    {
        return static_cast<const CollisionObject&>(*pObject).GetMatrix();
    }

    const PblVector3& GetPos(const PblMatrix& m)
    {
        return *(const PblVector3*)&m.m[3][0];
    }
}

// FUNCTION: SWBF2 0x005ac930
PblVector3 GetSineRandomOffsetXZ(const GameObject* pObj, const int i)
{
    const PblVector3& pos = GameObjectPosition(pObj);
    const float fPhase = pos.x * 0.2f + pos.y * 0.2f + pos.z * 0.2f;

    PblRandom rand(((unsigned int)pObj + i * 1009) ^ 0xC71C2830);

    const float fFreqX = rand.GetFloat() + 0.5f;
    const float fX = PblMath::Sin(fFreqX * fPhase + rand.GetFloat() * 6.2831855f);
    const float fFreqZ = rand.GetFloat() + 0.5f;
    const float fZ = PblMath::Cos(fFreqZ * fPhase + rand.GetFloat() * 6.2831855f);
    return PblVector3(fX, 0.0f, fZ * 0.5f);
}

// GLOBAL: SWBF2 0x01ecb1fc
// SYNTHETIC: SWBF2 0x004043c0
// `dynamic initializer for 'NavigatorFollowTarget::rttiHashNavigatorFollowTarget''
PblHash NavigatorFollowTarget::rttiHashNavigatorFollowTarget("NavigatorFollowTarget");
char NavigatorFollowTarget::rttiNameNavigatorFollowTarget[] = "NavigatorFollowTarget";

// FUNCTION: SWBF2 0x005ac510
bool NavigatorFollowTarget::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashNavigatorFollowTarget || NavigatorGraphFollower::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x005ac540
unsigned int NavigatorFollowTarget::GetDerivedRtti() const
{
    return rttiHashNavigatorFollowTarget;
}

// FUNCTION: SWBF2 0x005ac550
const char* NavigatorFollowTarget::GetDerivedRttiName() const
{
    return rttiNameNavigatorFollowTarget;
}

// SYNTHETIC: SWBF2 0x005ac560
// NavigatorFollowTarget::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005ac590
NavigatorFollowTarget* NavigatorFollowTargetPtr(Navigator* pNavigator)
{
    if (pNavigator && pNavigator->IsRtti(NavigatorFollowTarget::rttiHashNavigatorFollowTarget))
    {
        return (NavigatorFollowTarget*)pNavigator;
    }
    return 0;
}

// FUNCTION: SWBF2 0x005ac5b0
NavigatorFollowTarget::NavigatorFollowTarget(AILowLevel* pOwner, const GameObject* pTarget,
                                             const Obstacle** ppIgnore, int iIgnoreCnt,
                                             const PblVector3& followOffset)
    : NavigatorGraphFollower(pOwner, GameObjectPosition(pTarget), ppIgnore, iIgnoreCnt, false)
{
    mFollowTarget.mObject = pTarget;
    mFollowTarget.mSavedHandleId = GameObjectHandleId(pTarget);
    mFollowOffset = followOffset;
    mGraphFollower = 0;
    mCrumbHead = 0;
    mCrumbTail = 0;
    mLastShortcutCheck = 0;
}

// FUNCTION: SWBF2 0x005ac630
NavigatorFollowTarget::~NavigatorFollowTarget()
{
}

// FUNCTION: SWBF2 0x005ac640
bool NavigatorFollowTarget::IsEqual(AILowLevel* pOwner, const GameObject* pTarget,
                                    const PblVector3& followOffset)
{
    return mOwner == pOwner && HandleObject(mFollowTarget) == pTarget &&
           !(mFollowOffset != followOffset) && mCurState != 100;
}

// FUNCTION: SWBF2 0x005ac6c0
void NavigatorFollowTarget::EnterState()
{
    if (mCurState == 100)
    {
        return;
    }

    if (!HandleObject(mFollowTarget))
    {
        SetNextState(100);
        return;
    }

    if (mCurState == STATE_PATHING)
    {
        mCrumbHead = 1;
        mCrumbTail = 0;
        const PblVector3& vTarget = GameObjectPosition(HandleObject(mFollowTarget));
        mCrumbBuffer[0] = vTarget;
        mCrumbBuffer[1] = vTarget;
        mGlobalDest = mCrumbBuffer[0];

        if (LookForCrumbs())
        {
            return;
        }

        const PblVector3& vMyPos = GameObjectPosition(GetGameObject());
        const float dx = vMyPos.x - mGlobalDest.x;
        const float dy = vMyPos.y - mGlobalDest.y;
        const float dz = vMyPos.z - mGlobalDest.z;
        if (!ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1) ||
            (dx * dx + dy * dy + dz * dz < 10000.0f &&
             !PathFinder::TestTrivialStraightLineHit(vMyPos, mGlobalDest, GetMyRadius(), GetMyProhibit(),
                                                     mIgnore, mIgnoreCnt, false)))
        {
            if (mGraphFollower)
            {
                delete mGraphFollower;
                mGraphFollower = 0;
            }
            delete mPathFollower;
            mPathFollower = new PathFollower(mOwner->mOwner, vMyPos, mGlobalDest,
                                             (const Obstacle**)mIgnore, mIgnoreCnt, false);
            return;
        }
    }

    if (mCurState < STATE_FOLLOWCRUMBS)
    {
        NavigatorGraphFollower::EnterState();
    }
}

// FUNCTION: SWBF2 0x005ac8f0
void NavigatorFollowTarget::ExitState()
{
    if (mCurState == STATE_PATHING)
    {
        PathManager::RemovePathRequest(mOwner->mOwner);
        if (mGraphFollower)
        {
            delete mGraphFollower;
        }
        mGraphFollower = 0;
    }
}

// FUNCTION: SWBF2 0x005ad160
void NavigatorFollowTarget::UpdateState(float dt)
{
    if (mCurState == 100)
    {
        return;
    }

    if (!HandleObject(mFollowTarget))
    {
        SetNextState(100);
        return;
    }

    UpdateTarget();

    switch (mCurState)
    {
    case STATE_PATHING:
        mOwner->SetHeight(HEIGHT_STAND);
        LookForCrumbs();
        break;
    case STATE_FOLLOWCRUMBS:
        UpdateFollowCrumbs();
        break;
    }

    if (mPathFollower)
    {
        mPathFollower->Update(dt);
        if (mPathFollower->mDone && mPathFollower->mDoneReason == PF_PATHBLOCKED)
        {
            SetNextState(100);
        }
    }
}

// FUNCTION: SWBF2 0x005acaf0
void NavigatorFollowTarget::UpdateTarget()
{
    if (!HandleObject(mFollowTarget))
    {
        SetNextState(100);
        return;
    }

    const int iPrev = (mCrumbHead > 0) ? mCrumbHead - 1 : NUM_CRUMBS - 1;
    const int iNext = (mCrumbHead < NUM_CRUMBS - 1) ? mCrumbHead + 1 : 0;

    const PblVector3 vWobble = GetSineRandomOffsetXZ(
        HandleObject(mFollowTarget),
        ReserveManager::GetReserveOrder(HandleObject(mFollowTarget),
                                        mOwner->mOwner->mOwner->GetGameObject(), RT_FORMATION));
    const PblVector3 vOffset(vWobble.x * 0.3f, vWobble.y * 0.3f, vWobble.z * 0.3f);

    PblVector3 vPos;
    D3DXVec3TransformCoord(&vPos, &mFollowOffset, &GameObjectMatrix(HandleObject(mFollowTarget)));
    PblVector3& vHead = mCrumbBuffer[mCrumbHead];
    vHead = PblVector3(vPos.x + vOffset.x, vPos.y + vOffset.y, vPos.z + vOffset.z);

    PblVector3 vNormal;
    PathFinder::PushValidPoint(GameObjectPosition(HandleObject(mFollowTarget)), vHead, vNormal,
                               GetMyRadius(), GetMyProhibit(), (const Obstacle**)mIgnore, mIgnoreCnt);

    const PblVector3& vPrev = mCrumbBuffer[iPrev];
    const float dx = vHead.x - vPrev.x;
    const float dy = vHead.y - vPrev.y;
    const float dz = vHead.z - vPrev.z;
    if (dy * dy + dx * dx + dz * dz < 4.0f)
    {
        return;
    }

    if (iNext == mCrumbTail)
    {
        SetNextState(STATE_PATHING);
        return;
    }
    mCrumbBuffer[iNext] = mCrumbBuffer[mCrumbHead];
    mCrumbHead = iNext;
}

// FUNCTION: SWBF2 0x005acd60
bool NavigatorFollowTarget::LookForCrumbs()
{
    bool bCheckLOS = false;
    const PblVector3& vMyPos = GetPos(GetMatrix());
    const float fTime = GameLoop::GetMissionTime();
    if (fTime > mLastShortcutCheck + 2.0f)
    {
        bCheckLOS = true;
        mLastShortcutCheck = fTime;
    }

    int i = mCrumbHead;
    for (;;)
    {
        const float dx = mCrumbBuffer[i].x - vMyPos.x;
        const float dz = mCrumbBuffer[i].z - vMyPos.z;
        const float fDistSq = dx * dx + dz * dz;
        if (fDistSq < 4.0f)
        {
            break;
        }
        if (bCheckLOS && fDistSq < 900.0f &&
            !PathFinder::TestTrivialStraightLineHit(vMyPos, mCrumbBuffer[i], GetMyRadius(), GetMyProhibit(),
                                                    mIgnore, mIgnoreCnt, false))
        {
            break;
        }

        i = (i > 0) ? i - 1 : NUM_CRUMBS - 1;
        if (i == mCrumbTail)
        {
            return false;
        }
    }

    if (i == mCrumbHead)
    {
        i = (i > 0) ? i - 1 : NUM_CRUMBS - 1;
    }
    mCrumbTail = i;
    if (mCurState != STATE_FOLLOWCRUMBS)
    {
        SetNextState(STATE_FOLLOWCRUMBS);
    }
    return true;
}

// FUNCTION: SWBF2 0x005ace90
void NavigatorFollowTarget::UpdateFollowCrumbs()
{
    LookForCrumbs();

    const PblVector3& vMyPos = GetPos(GetMatrix());
    const PblVector3& vHead = mCrumbBuffer[mCrumbHead];
    const float dx = vHead.x - vMyPos.x;
    const float dz = vHead.z - vMyPos.z;
    mOwner->SetHeight(dz * dz + dx * dx > 1.0f ? HEIGHT_SPRINT : HEIGHT_STAND);

    const int iNext = (mCrumbTail < NUM_CRUMBS - 1) ? mCrumbTail + 1 : 0;
    const PblVector3& vCrumb = mCrumbBuffer[iNext];

    EntitySoldier* pMe = mOwner->mOwner->mOwner->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier)
                             ? (EntitySoldier*)mOwner->mOwner->mOwner->GetGameObject()
                             : 0;
    const EntitySoldier* pHim = HandleObject(mFollowTarget)->IsRtti(EntitySoldier::rttiHashEntitySoldier)
                                    ? (const EntitySoldier*)HandleObject(mFollowTarget)
                                    : 0;

    if (iNext == mCrumbHead && pMe && pHim && pMe->mClass->mJetFuelRechargeRate > 0.0f)
    {
        if (pHim->mState == SoldierState::JET_HOVER && pMe->mClass->mJetTypeHover)
        {
            mOwner->mDoJetJump = true;
            mOwner->mAIJetJumpDir = PblVector3(0.0f, 0.0f, 0.0f);
        }
        if (pHim->mState == SoldierState::JET_JUMP && !pMe->mClass->mJetTypeHover)
        {
            mOwner->mDoJetJump = true;
            mOwner->mAIJetJumpDir = PblVector3(0.0f, 0.0f, 0.0f);
        }
    }

    if (pMe && pHim && pMe->mState == SoldierState::JET_HOVER)
    {
        mOwner->mDesiredHoverHeight = GameObjectPosition(pHim).y;
        if (pHim->mState == SoldierState::STAND || pHim->mState == SoldierState::CROUCH)
        {
            mOwner->mJetJumpStop = true;
        }
    }

    if (mPathFollower)
    {
        const PblVector3& vEnd = mPathFollower->mFinalPoint;
        const float ex = vEnd.x - vCrumb.x;
        const float ez = vEnd.z - vCrumb.z;
        if (ez * ez + ex * ex < 0.0001f)
        {
            return;
        }
    }

    delete mPathFollower;
    mPathFollower = 0;
    mPathFollower = new PathFollower(mOwner->mOwner, GetPos(GetMatrix()), vCrumb,
                                     (const Obstacle**)mIgnore, mIgnoreCnt, false);
}

// FUNCTION: SWBF2 0x005ad1f0
const PblVector3& NavigatorFollowTarget::GetGlobalDest() const
{
    return mCrumbBuffer[mCrumbHead];
}

// FUNCTION: SWBF2 0x005ad200
void NavigatorFollowTarget::ReceivePathResult(AIPath* pPath)
{
    if (mCurState == STATE_PATHING)
    {
        if (pPath)
        {
            NavigatorGraphFollower::ReceivePathResult(pPath);
        }
        else
        {
            mFailedPathSearch = true;
            SetNextState(100);
        }
    }
}
