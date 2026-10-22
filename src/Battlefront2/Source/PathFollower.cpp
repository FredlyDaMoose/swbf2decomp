#include <math.h>
#include <string.h>

#include "PathFollower.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Entity.h"
#include "EntityFlyer.h"
#include "EntityCarrier.h"
#include "EntityDroideka.h"
#include "EntitySoldier.h"
#include "GameObject.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "PathFinder.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblHeap.h"
#include "PebbleFL/Common/PblMatrix.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

namespace AI
{
enum VOType
{
    VOT_PathFollowWait = 6,
    VOT_DefendIdleLookAround = 20
};

namespace VOHelper
{

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const GameObject* pOther, float fDelay);
}
}

// GLOBAL: SWBF2 0x01ef7da8
// SYNTHETIC: SWBF2 0x00405e10
// `dynamic initializer for 'PathFollower::sMemoryPool''
// SYNTHETIC: SWBF2 0x007692c0
// `dynamic atexit destructor for 'PathFollower::sMemoryPool''
MemoryPool PathFollower::sMemoryPool("PathFollower", sizeof(PathFollower));

namespace
{

    const PblHash kRttiHashEntityFlyer("EntityFlyer");
    const PblHash kRttiHashEntityHover("EntityHover");
    const PblHash kRttiHashEntityWalker("EntityWalker");
    const PblHash kRttiHashEntityDroideka("EntityDroideka");
    const PblHash kRttiHashEntitySoldier("EntitySoldier");

    const int kMaxPathNodes = 32;

    const PblMatrix& EntityGetMatrix(Entity* pEntity)
    {
        return static_cast<CollisionObject&>(static_cast<GameObject&>(*pEntity)).GetMatrix();
    }

    const PblVector3& EntityGetVelocity(Entity* pEntity)
    {
        return static_cast<GameObject*>(pEntity)->GetVelocity();
    }

    float EntityFlyerGetRoll(Entity* pFlyer) { return static_cast<EntityFlyer*>(pFlyer)->mGetSpeedSpeed; }

    PblVector3& EntitySoldierAiGetLastCollisionNormal(Entity* p) { return static_cast<EntitySoldier*>(p)->mAILastCollisionNormal; }
    char EntitySoldierAiGetLastCollisionDirection(Entity* p)
    {
        return static_cast<EntitySoldier*>(p)->mAILastCollisionDirection;
    }
    void EntitySoldierAiSetLastCollisionDirection(Entity* p, char cDir)
    {
        static_cast<EntitySoldier*>(p)->mAILastCollisionDirection = cDir;
    }

    PblVector3& EntityDroidekaAiGetLastCollisionNormal(Entity* p) { return static_cast<EntityDroideka*>(p)->mAILastCollisionNormal; }
    char EntityDroidekaAiGetLastCollisionDirection(Entity* p)
    {
        return static_cast<EntityDroideka*>(p)->mAILastCollisionDirection;
    }
    void EntityDroidekaAiSetLastCollisionDirection(Entity* p, char cDir)
    {
        static_cast<EntityDroideka*>(p)->mAILastCollisionDirection = cDir;
    }

    bool GameObjectIsRtti(GameObject* pObj, unsigned int uRtti)
    {
        return pObj->IsRtti(uRtti);
    }

    AILowLevel& LowLevel(UnitController* pCtrl)
    {
        return *reinterpret_cast<AILowLevel*>(pCtrl->mLowLevel);
    }

    const PblVector3& Trans(const PblMatrix& m) { return *(const PblVector3*)&m.m[3][0]; }
}

extern int WORLD_EXTENTS;
extern TreeGrid* sObstacles;

namespace PathFinder
{
// GLOBAL: SWBF2 0x01eaedb0
unsigned int _FindBarrierNameHash;
// GLOBAL: SWBF2 0x01eaedac
Obstacle* _FindBarrierRet;
}

namespace PathFinder { bool FindBarrierCallback(const TreeGridObject* pObject, TreeGridCallbackData*, float&); }

// FUNCTION: SWBF2 0x00612510
bool PathFinder::FindBarrierCallback(const TreeGridObject* pObject, TreeGridCallbackData*, float&)
{
    const Obstacle* pObstacle = static_cast<const Obstacle*>(pObject);
    if (pObstacle->mNameHash == PathFinder::_FindBarrierNameHash)
    {
        PathFinder::_FindBarrierRet = const_cast<Obstacle*>(pObstacle);
        return false;
    }
    return true;
}

namespace PathFinder { Obstacle* FindBarrierByName(unsigned int uName); }

// FUNCTION: SWBF2 0x00612530
Obstacle* PathFinder::FindBarrierByName(unsigned int uName)
{
    PathFinder::_FindBarrierNameHash = uName;
    PathFinder::_FindBarrierRet = 0;
    TreeGridCallbackData data;
    sObstacles->CallbackOnRect(PblVector3((float)-WORLD_EXTENTS, 0.0f, (float)-WORLD_EXTENTS),
                               PblVector3((float)WORLD_EXTENTS, 0.0f, (float)WORLD_EXTENTS),
                               PathFinder::FindBarrierCallback, &data);
    return PathFinder::_FindBarrierRet;
}

// TEMPLATE: SWBF2 0x006125b0
// PblHeap<PathFinder::PathNode *,float>::Push
template void PblHeap<PathFinder::PathNode*, float>::Push(PathFinder::PathNode*&, float);
// TEMPLATE: SWBF2 0x00612610
// PblHeap<PathFinder::PathNode *,float>::ReplaceLowest
template void PblHeap<PathFinder::PathNode*, float>::ReplaceLowest(PathFinder::PathNode*&, float, int);
// TEMPLATE: SWBF2 0x00612670
// PblHeap<PathFinder::PathNode *,float>::Pop
template void PblHeap<PathFinder::PathNode*, float>::Pop();

// FUNCTION: SWBF2 0x00612710
PathFollower::PathFollower(UnitController* pCtrl, AIPath* pPath, const Obstacle** ppIgnore,
                           int iIgnoreCnt, bool bDoCallbacksOnPts)
{
    mPath = pPath;
    mCtrl = pCtrl;
    mItor = PblList<AIPathNode>::Itor(pPath->GetNodeList());
    mDoCallbacksOnPts = bDoCallbacksOnPts;
    mIgnoreCnt = iIgnoreCnt;
    mDone = false;
    mDoneReason = PF_NOTDONE;
    mPaused = false;
    memcpy(mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));
    ConstructorCommon();
}

// FUNCTION: SWBF2 0x00612770
PathFollower::PathFollower(UnitController* pCtrl, const PblVector3& vStart, const PblVector3& vEnd,
                           const Obstacle** ppIgnore, int iIgnoreCnt, bool bDoCallbacksOnPts)
{
    mCtrl = pCtrl;
    mItor = PblList<AIPathNode>::Itor();
    mIgnoreCnt = iIgnoreCnt;
    mDoCallbacksOnPts = bDoCallbacksOnPts;
    mDone = false;
    mDoneReason = PF_NOTDONE;
    mPaused = false;
    memcpy(mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));

    mPath = new AIPath;
    if (mPath)
    {
        mPath->Prepend(vEnd, 0);
        mPath->Prepend(vStart, 0);
        mItor = PblList<AIPathNode>::Itor(mPath->GetNodeList());
        ConstructorCommon();
        mPath->Release();
    }
}

// FUNCTION: SWBF2 0x00612830
void PathFollower::ConstructorCommon()
{
    mPath->AddRef();
    mStuckPos = PblVector3(0.0f, 0.0f, 0.0f);
    mIsStuck = 0;
    mStuckTime = 0.0f;
    mMinDistToGoal = 3.402823466e+38f;
    mMinDistToGoalTime = 0.0f;
    mWaitTime = 0.0f;
    mWaitState = WS_NONE;

    mPath->GetNodeList().GetLast()->_pObject->mFlags |= AIPathNode::AIPN_END_OF_SEGMENT;

    const PblMatrix& mMe = mCtrl->mOwner->GetMatrix();
    const PblVector3& vFirst = mPath->GetNodeList().GetFirst()->_pObject->mPos;
    float dx = mMe.m[3][0] - vFirst.x;
    float dy = mMe.m[3][1] - vFirst.y;
    float dz = mMe.m[3][2] - vFirst.z;
    if (dx * dx + dy * dy + dz * dz < 100.0f &&
        (mCtrl->mOwner->GetEntity()->IsRtti(kRttiHashEntityHover) ||
         mCtrl->mOwner->GetEntity()->IsRtti(kRttiHashEntityFlyer)))
    {
        SetLocalPoints(mItor->mPos, mItor);
    }
    else
    {
        SetLocalPoints(Trans(mCtrl->mOwner->GetMatrix()), mItor);
    }
}

// FUNCTION: SWBF2 0x00612980
PathFollower::~PathFollower()
{
    if (mPath)
    {
        mPath->Release();
    }
    mPath = 0;
}

// FUNCTION: SWBF2 0x006129c0
void PathFollower::AppendPath(const AIPath* pPath, bool b)
{
    if (mDone && mDoneReason == PF_PATHBLOCKED)
    {
        return;
    }

    int iCount = mPath->GetNodeList().GetCount() + pPath->GetNodeList().GetCount();
    while (iCount > kMaxPathNodes)
    {
        if (mItor->GetListNode() == mPath->GetNodeList().GetFirst())
        {
            break;
        }
        mPath->PopHead();
        --iCount;
    }

    mPath->AppendPath(pPath);
    mPath->GetNodeList().GetLast()->_pObject->mFlags |= AIPathNode::AIPN_END_OF_SEGMENT;
    mDone = false;
    mDoneReason = PF_NOTDONE;

    if (mCtrl->mLodState >= 3)
    {
        float fRadius = AIUtil::GetCollisionRadius(mCtrl->mOwner->GetGameObject());
        if (!GameObjectIsRtti(mCtrl->mOwner->GetGameObject(), kRttiHashEntityWalker) || fRadius > 3.0f)
        {
            int eType = AIUtil::GetAIType(mCtrl->mOwner->GetGameObject());
            mPath->SmoothCorners(fRadius * 10.0f, fRadius, eType);
            mPath->SmoothCorners(fRadius * 3.0f, fRadius, eType);
        }
    }

    SetLocalPoints(mPrevPoint, mItor);
}

// FUNCTION: SWBF2 0x00612b00
bool PathFollower::PrependPath(const AIPath* pPath)
{
    mPath->TrimBefore(mItor);
    if (pPath->GetNodeList().GetCount() + mPath->GetNodeList().GetCount() > kMaxPathNodes)
    {
        return false;
    }

    mPath->PrependPath(pPath);
    mItor = PblList<AIPathNode>::Itor(mPath->GetNodeList());
    SetLocalPoints(Trans(mCtrl->mOwner->GetMatrix()), mItor);
    return true;
}

// FUNCTION: SWBF2 0x00612b70
void PathFollower::SetLocalPoints(const PblVector3& vPrev, const PblList<AIPathNode>::Itor& itor)
{
    mPrevPoint = vPrev;
    mGotoPoint = itor->mPos;
    AIPathNode* pNext = itor->GetListNode()->_pNext->_pObject;
    mSafeHeight = SH_NONE;
    mDestSide = DS_EITHER;

    if (pNext)
    {
        mNextPoint = pNext->mPos;
        const PblMatrix& mMe = mCtrl->mOwner->GetMatrix();
        if (!(Trans(mMe) != vPrev))
        {
            PblVector3 vDelta(mNextPoint.x - mGotoPoint.x, mNextPoint.y - mGotoPoint.y,
                              mNextPoint.z - mGotoPoint.z);
            PblVector3 vDir;
            D3DXVec3Normalize(&vDir, &vDelta);
            mSafeHeight = SH_TAPER;
            mDir = vDir;
            mDestSide = DS_EITHER;
        }
        else if (mNextPoint != mGotoPoint)
        {
            PblVector3 vDelta(mNextPoint.x - mGotoPoint.x, mNextPoint.y - mGotoPoint.y,
                              mNextPoint.z - mGotoPoint.z);
            PblVector3 vDir;
            D3DXVec3Normalize(&vDir, &vDelta);
            mDir = vDir;
            mSafeHeight = SH_NONE;
            float fSide = (mNextPoint.x - mGotoPoint.x) * mDir.z +
                          (mNextPoint.y - mGotoPoint.y) * 0.0f +
                          (mNextPoint.z - mGotoPoint.z) * -mDir.x;
            mDestSide = fSide >= 0.0f ? DS_RIGHT : DS_LEFT;
        }
    }
    else
    {
        mNextPoint = mGotoPoint;
        if (mGotoPoint != mPrevPoint)
        {
            PblVector3 vDelta(mGotoPoint.x - mPrevPoint.x, mGotoPoint.y - mPrevPoint.y,
                              mGotoPoint.z - mPrevPoint.z);
            PblVector3 vDir;
            D3DXVec3Normalize(&vDir, &vDelta);
            mSafeHeight = SH_NONE;
            mDir = vDir;
            mDestSide = DS_EITHER;
        }
    }

    mFinalPoint = mPath->GetNodeList().GetLast()->_pObject->mPos;
    UpdateControllableMoving();
}

// FUNCTION: SWBF2 0x00612e00
bool PathFollower::CloseToDest() const
{
    if (!mCtrl || !mCtrl->mOwner)
    {
        return false;
    }

    const PblMatrix& mMe = mCtrl->mOwner->GetMatrix();
    float dx = mMe.m[3][0] - mGotoPoint.x;
    float dz = mMe.m[3][2] - mGotoPoint.z;

    float fStopDist;
    if ((AIPathNode*)mItor == mPath->GetNodeList().GetLast()->_pObject)
    {
        fStopDist = AIUtil::StopDist(mCtrl->mOwner);
    }
    else
    {
        fStopDist = AIUtil::StopDistIntermediate(mCtrl->mOwner);
    }
    if (LowLevel(mCtrl).mHeight == HEIGHT_WALK)
    {
        fStopDist *= 0.25f;
    }

    float fDistSq = dz * dz + dx * dx;
    if ((mItor->mFlags & AIPathNode::AIPN_ROLLIN) && fDistSq > 81.0f && fDistSq < 121.0f)
    {
        const PblMatrix& mNow = mCtrl->mOwner->GetMatrix();
        PblVector3 vDelta(mGotoPoint.x - mNow.m[3][0], mGotoPoint.y - mNow.m[3][1],
                          mGotoPoint.z - mNow.m[3][2]);
        vDelta.y = 0.0f;
        PblVector3 vRoll;
        D3DXVec3Normalize(&vRoll, &vDelta);
        LowLevel(mCtrl).SetRoll(vRoll);
        mItor->mFlags &= ~AIPathNode::AIPN_ROLLIN;
    }

    return fStopDist * fStopDist > fDistSq;
}

// FUNCTION: SWBF2 0x00612f80
bool PathFollower::GonePastDest() const
{
    float fSegZ = mGotoPoint.z - mPrevPoint.z;
    float fSegX = mGotoPoint.x - mPrevPoint.x;
    if (fSegZ * fSegZ + fSegX * fSegX < 0.0001f)
    {
        return true;
    }

    if (!mCtrl || !mCtrl->mOwner)
    {
        return false;
    }

    const PblMatrix& mMe = mCtrl->mOwner->GetMatrix();
    float dx = mMe.m[3][0] - mGotoPoint.x;
    float dz = mMe.m[3][2] - mGotoPoint.z;
    if (AIUtil::GetEntityType(mCtrl->mOwner->GetEntity()) == AIUtil::ENTITY_FLYER &&
        dx * dx + 0.0f + dz * dz > 100.0f)
    {
        return false;
    }

    return dx * fSegX + 0.0f + dz * fSegZ > 0.0f;
}

// FUNCTION: SWBF2 0x00613070
void PathFollower::GotoNextNode()
{
    if (mDoCallbacksOnPts)
    {
        AI::EventInfo info(AI::EVT_Path_NextNode, mGotoPoint);
        mCtrl->SendEvent(&info);
    }

    PblList<AIPathNode>::Node* pLast = mPath->GetNodeList().GetLast();
    if (!mDone && (mItor->mFlags & AIPathNode::AIPN_END_OF_SEGMENT))
    {
        mItor->mFlags ^= AIPathNode::AIPN_END_OF_SEGMENT;
        PathManager::LowerRequestPriority(mCtrl);
    }

    if ((AIPathNode*)mItor == pLast->_pObject)
    {
        mDone = true;
        mDoneReason = PF_PATHDONE;
    }
    else
    {
        ++mItor;
        SetLocalPoints(mGotoPoint, mItor);
    }

    if (mItor._pNode->_pPrev->_pObject)
    {
        mPath->TrimBefore(mItor._pNode->_pPrev->_pObject);
    }

    const PblMatrix& mMe = mCtrl->mOwner->GetMatrix();
    float dx = mGotoPoint.x - mMe.m[3][0];
    float dy = mGotoPoint.y - mMe.m[3][1];
    float dz = mGotoPoint.z - mMe.m[3][2];
    mMinDistToGoalTime = 0.0f;
    mMinDistToGoal = dx * dx + dy * dy + dz * dz;
}

// FUNCTION: SWBF2 0x00613160
bool PathFollower::TestBarriersAhead() const
{
    const PblMatrix& mMe = mCtrl->mOwner->GetMatrix();
    PblVector3 vAhead(mGotoPoint.x - mMe.m[3][0], mGotoPoint.y - mMe.m[3][1],
                      mGotoPoint.z - mMe.m[3][2]);
    if (vAhead.x * vAhead.x + vAhead.z * vAhead.z > 25.0f)
    {
        PblVector3 vNormal = AIUtil::GetSafeNormalXZ(vAhead);
        vAhead = PblVector3(vNormal.x * 5.0f, vNormal.y * 5.0f, vNormal.z * 5.0f);
    }

    PblVector3 vTo(mMe.m[3][0] + vAhead.x, mMe.m[3][1] + vAhead.y, mMe.m[3][2] + vAhead.z);
    float fRadius = AIUtil::GetCollisionRadius(mCtrl->mOwner->GetGameObject());
    int eType = AIUtil::GetAIType(mCtrl->mOwner->GetGameObject());
    return PathFinder::TestTrivialStraightLineHit(Trans(mMe), vTo, fRadius, eType,
                                                  mIgnore, mIgnoreCnt, true);
}

// FUNCTION: SWBF2 0x00613290
void PathFollower::Wait(float fTime)
{
    mWaitTime = GameLoop::GetMissionTime() + fTime;
}

// FUNCTION: SWBF2 0x006132c0
void PathFollower::Update(float dt)
{
    if (mWaitTime != 0.0f)
    {
        if (mWaitState == WS_CROUCH)
        {
            LowLevel(mCtrl).SetHeight(HEIGHT_CROUCH);
        }
        else if (mWaitState == WS_LOOKAROUND)
        {
            LowLevel(mCtrl).mDoIdle = (SoldierAnimType::MovementAnimation)2;
        }

        if (GameLoop::GetMissionTime() > mWaitTime)
        {
            mWaitTime = 0.0f;
            if (mWaitState == WS_CROUCH)
            {
                LowLevel(mCtrl).SetHeight(HEIGHT_STAND);
            }
        }
        UpdateControllableStop();
        return;
    }

    bool bFlyer = mCtrl->mOwner->GetEntity()->IsRtti(kRttiHashEntityFlyer);
    if (!mDone && !bFlyer)
    {
        CheckStuck(dt);
    }

    if (!mPaused)
    {
        if (CloseToDest() || GonePastDest())
        {
            AIPathNode* pNode = mItor;
            unsigned short uFlags = pNode->mFlags;

            if (uFlags & (AIPathNode::AIPN_WAITLEFT | AIPathNode::AIPN_WAITRIGHT))
            {
                if ((uFlags & AIPathNode::AIPN_WAITLEFT) && !(uFlags & AIPathNode::AIPN_WAITRIGHT))
                {
                    LowLevel(mCtrl).HeadLook_Glance(true, 0.2f, 1.2f, 0.2f);
                }
                if ((mItor->mFlags & AIPathNode::AIPN_WAITRIGHT) &&
                    !(mItor->mFlags & AIPathNode::AIPN_WAITLEFT))
                {
                    LowLevel(mCtrl).HeadLook_Glance(false, 0.2f, 1.2f, 0.2f);
                }

                float fWait = 2.0f;
                if (mItor->mFlags & AIPathNode::AIPN_WAITLONG)
                {
                    fWait = (float)AIUtil::GetRandomInt(2, 4) + fWait;
                }
                mItor->mFlags &= ~(AIPathNode::AIPN_WAITLEFT | AIPathNode::AIPN_WAITRIGHT |
                                   AIPathNode::AIPN_WAITLONG);
                Wait(fWait);
                LowLevel(mCtrl).SetAlert();

                AIPathNode* pNext = mItor._pNode->_pNext->_pObject;
                if (pNext)
                {
                    PblVector3 vLook = pNext->mPos;
                    vLook.y = mCtrl->mOwner->GetMatrix().m[3][1] + 1.6f;
                    LowLevel(mCtrl).SetHeading(vLook, 0.0f);
                }

                mWaitState = WS_CROUCH;
                AI::VOHelper::Play(AI::VOT_PathFollowWait, mCtrl->mOwner->GetGameObject(), 0,
                                   mCtrl->mOwner->GetGameObject(), 0.0f);
                UpdateControllable();
                return;
            }

            if (uFlags & AIPathNode::AIPN_WAITLOOKAROUND)
            {
                pNode->mFlags = uFlags & ~AIPathNode::AIPN_WAITLOOKAROUND;
                Wait(9.0f);
                mWaitState = WS_LOOKAROUND;
                AI::VOHelper::Play(AI::VOT_DefendIdleLookAround, mCtrl->mOwner->GetGameObject(),
                                   0, 0, 0.0f);
                UpdateControllable();
                return;
            }

            if (uFlags & (AIPathNode::AIPN_JUMP | AIPathNode::AIPN_JETJUMP))
            {
                PblVector3 vJump(0.0f, 1.0f, 0.0f);
                AIPathNode* pNext = mItor._pNode->_pNext->_pObject;
                if (pNext)
                {
                    const PblMatrix& mMe = mCtrl->mOwner->GetMatrix();
                    vJump = PblVector3(pNext->mPos.x - mMe.m[3][0], 0.0f, pNext->mPos.z - mMe.m[3][2]);
                    PblVector3 vDir;
                    D3DXVec3Normalize(&vDir, &vJump);
                    vJump = PblVector3(vDir.x * 0.70710677f, 0.70710677f, vDir.z * 0.70710677f);
                }

                AILowLevel& ll = LowLevel(mCtrl);
                ll.mAIJetJumpDir = vJump;
                if (mItor->mFlags & AIPathNode::AIPN_JUMP)
                {
                    ll.mDoJump = true;
                }
                else
                {
                    ll.mDoJetJump = true;
                }
                GotoNextNode();
                UpdateControllable();
                return;
            }

            if (uFlags & AIPathNode::AIPN_ROLLOUT)
            {
                PblVector3 vRoll(0.0f, 1.0f, 0.0f);
                AIPathNode* pNext = mItor._pNode->_pNext->_pObject;
                if (pNext)
                {
                    const PblMatrix& mMe = mCtrl->mOwner->GetMatrix();
                    PblVector3 vDelta(pNext->mPos.x - mMe.m[3][0], 0.0f, pNext->mPos.z - mMe.m[3][2]);
                    D3DXVec3Normalize(&vRoll, &vDelta);
                }
                LowLevel(mCtrl).SetRoll(vRoll);
            }

            GotoNextNode();
        }
        else if (TestBarriersAhead() == true)
        {
            mDone = true;
            mDoneReason = PF_PATHBLOCKED;
        }
    }

    UpdateControllable();
}

// FUNCTION: SWBF2 0x00613780
void PathFollower::UpdateControllable()
{
    if (mDone || mWaitTime != 0.0f)
    {
        UpdateControllableStop();
    }
    else
    {
        UpdateControllableMoving();
    }
}

// FUNCTION: SWBF2 0x006137b0
void PathFollower::UpdateControllableStop()
{
    LowLevel(mCtrl).SetDestCurrent();
}

// FUNCTION: SWBF2 0x006137c0
void PathFollower::UpdateControllableMoving()
{
    PblVector3 vSteer = mGotoPoint;

    if (mCtrl->mOwner->GetEntity()->IsRtti(kRttiHashEntityFlyer))
    {
        Entity* pFlyer = mCtrl->mOwner->GetEntity();
        const EntityFlyerClass* pClass = static_cast<const EntityFlyerClass*>(pFlyer->GetEntityClass());
        float fTurn = fabsf(EntityFlyerGetRoll(pFlyer)) / pClass->GetTurnRate() * 2.0f;
        if (fTurn < 5.0f)
        {
            fTurn = 5.0f;
        }

        const PblMatrix& mFlyer = EntityGetMatrix(pFlyer);
        PblVector3 vForward(mFlyer.m[2][0] + 0.0001f, 0.0f, mFlyer.m[2][2]);
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vForward);
        vForward = vDir;
        AIUtil::CalculateFlyerSteerPoint(mGotoPoint, mDir, Trans(EntityGetMatrix(pFlyer)), vForward,
                                         fTurn, vSteer, mDestSide);
    }

    if (mCtrl->mOwner->GetEntity()->IsRtti(kRttiHashEntityHover))
    {
        PblVector3 vPos = Trans(mCtrl->mOwner->GetMatrix());
        const PblVector3& vVel = EntityGetVelocity(mCtrl->mOwner->GetEntity());
        float fSpeed = sqrtf(vVel.y * vVel.y + vVel.x * vVel.x + vVel.z * vVel.z);
        AIUtil::CalculateSoldierSteerPoint(mPrevPoint, mGotoPoint, vPos, fSpeed, vSteer);
    }

    if (mCtrl->mOwner->GetEntity()->IsRtti(kRttiHashEntityWalker))
    {
        PblVector3 vPos = Trans(mCtrl->mOwner->GetMatrix());
        AIUtil::CalculateSoldierSteerPoint(mPrevPoint, mGotoPoint, vPos, 20.0f, vSteer);
    }

    bool bDroideka = mCtrl->mOwner->GetEntity()->IsRtti(kRttiHashEntityDroideka);
    bool bSoldier = mCtrl->mOwner->GetEntity()->IsRtti(kRttiHashEntitySoldier);
    if (bSoldier || bDroideka)
    {
        PblVector3 vNormal;
        char cDir;
        if (bSoldier)
        {
            Entity* pSoldier = mCtrl->mOwner->GetEntity();
            vNormal = EntitySoldierAiGetLastCollisionNormal(pSoldier);
            cDir = EntitySoldierAiGetLastCollisionDirection(pSoldier);
        }
        else
        {
            Entity* pDroideka = mCtrl->mOwner->GetEntity();
            vNormal = EntityDroidekaAiGetLastCollisionNormal(pDroideka);
            cDir = EntityDroidekaAiGetLastCollisionDirection(pDroideka);
        }

        if (vNormal.x * vNormal.x + vNormal.y * vNormal.y + vNormal.z * vNormal.z > 0.0f)
        {
            PblVector3 vN;
            D3DXVec3Normalize(&vN, &vNormal);
            PblVector3 vAlong(vN.z, 0.0f, -vN.x);
            PblVector3 vPos = Trans(mCtrl->mOwner->GetMatrix());

            if (cDir == 0)
            {
                PblVector3 vAway(vPos.x - mGotoPoint.x, vPos.y - mGotoPoint.y, vPos.z - mGotoPoint.z);
                PblVector3 vAwayN;
                D3DXVec3Normalize(&vAwayN, &vAway);
                float fDot = vAwayN.x * vAlong.x + vAwayN.y * 0.0f + vAwayN.z * vAlong.z;
                cDir = fDot <= 0.0f ? 1 : -1;
            }

            float fStep = (float)cDir * 5.0f;
            vSteer = PblVector3(vAlong.x * fStep + vPos.x, fStep * 0.0f + vPos.y,
                                vPos.z + vAlong.z * fStep);

            if (bSoldier)
            {
                Entity* pSoldier = mCtrl->mOwner->GetEntity();
                EntitySoldierAiGetLastCollisionNormal(pSoldier) = PblVector3(0.0f, 0.0f, 0.0f);
                EntitySoldierAiSetLastCollisionDirection(mCtrl->mOwner->GetEntity(), cDir);
            }
            else
            {
                Entity* pDroideka = mCtrl->mOwner->GetEntity();
                EntityDroidekaAiGetLastCollisionNormal(pDroideka) = PblVector3(0.0f, 0.0f, 0.0f);
                EntityDroidekaAiSetLastCollisionDirection(mCtrl->mOwner->GetEntity(), cDir);
            }
        }
        else
        {
            if (bSoldier)
            {
                EntitySoldierAiSetLastCollisionDirection(mCtrl->mOwner->GetEntity(), 0);
            }
            else
            {
                EntityDroidekaAiSetLastCollisionDirection(mCtrl->mOwner->GetEntity(), 0);
            }

            PblVector3 vPos = Trans(mCtrl->mOwner->GetMatrix());
            const PblVector3& vVel = EntityGetVelocity(mCtrl->mOwner->GetEntity());
            float fSpeed = sqrtf(vVel.x * vVel.x + vVel.y * vVel.y + vVel.z * vVel.z);
            AIUtil::CalculateSoldierSteerPoint(mPrevPoint, mGotoPoint, vPos, fSpeed, vSteer);
        }
    }

    LowLevel(mCtrl).SetDest(vSteer, mGotoPoint, mDir, mSafeHeight, -1.0f, 1.0f);
}

// FUNCTION: SWBF2 0x00613d50
void PathFollower::GetStuckParams(UnitController* pCtrl, float& fStuckDist, float& fStuckTime, float& fMinDistTime)
{
    bool bDroideka = AIUtil::GetEntityType(pCtrl->mOwner->GetEntity()) == AIUtil::ENTITY_DROIDEKA;
    fStuckDist = 2.0f;
    if (bDroideka)
    {
        fStuckTime = 6.0f;
        fMinDistTime = 6.0f;
    }
    else
    {
        fStuckTime = 3.0f;
        fMinDistTime = 5.0f;
    }
}

// FUNCTION: SWBF2 0x00613da0
void PathFollower::CheckStuck(float dt)
{
    float fStuckDist;
    float fStuckTime;
    float fMinDistTime;
    PathFollower::GetStuckParams(mCtrl, fStuckDist, fStuckTime, fMinDistTime);
    mIsStuck = 0;

    PblVector3 vPos = Trans(mCtrl->mOwner->GetMatrix());
    float fGoalDX = vPos.x - mGotoPoint.x;
    float fGoalDZ = vPos.z - mGotoPoint.z;
    if (fGoalDZ * fGoalDZ + fGoalDX * fGoalDX < 0.04f)
    {
        return;
    }

    float fDX = mStuckPos.x - vPos.x;
    float fDY = mStuckPos.y - vPos.y;
    float fDZ = mStuckPos.z - vPos.z;
    if (fDY * fDY + fDX * fDX + fDZ * fDZ > fStuckDist * fStuckDist)
    {
        mStuckPos = vPos;
        mStuckTime = 0.0f;
    }
    else
    {
        mStuckTime += dt;
    }
    if (mStuckTime > fStuckTime)
    {
        mIsStuck = 2;
    }

    fDX = mGotoPoint.x - vPos.x;
    fDY = mGotoPoint.y - vPos.y;
    fDZ = mGotoPoint.z - vPos.z;
    float fDistSq = fDY * fDY + fDX * fDX + fDZ * fDZ;
    if (fDistSq < mMinDistToGoal)
    {
        mMinDistToGoal = fDistSq;
        mMinDistToGoalTime = 0.0f;
    }
    else
    {
        mMinDistToGoalTime += dt;
        if (mMinDistToGoalTime > fMinDistTime && mIsStuck == 0)
        {
            mIsStuck = 1;
        }
    }

    if (mIsStuck)
    {
        AI::EventInfo info(AI::EVT_Path_PathFollowerStuck, 0.0f, mIsStuck);
        mCtrl->SendEvent(&info);
    }
}

// FUNCTION: SWBF2 0x00613f30
unsigned char PathFollower::GetPathQueueDepth()
{
    unsigned char depth = 0;
    for (PblList<AIPathNode>::Itor it(mPath->GetNodeList()); (AIPathNode*)it; ++it)
    {
        if (it->mFlags & AIPathNode::AIPN_END_OF_SEGMENT)
        {
            ++depth;
        }
    }
    return depth;
}
