#include <windows.h>
#include <new>
#include <string.h>

#include "PathManager.h"

#include "AILowLevel.h"
#include "EventInfo.h"
#include "PathFinder.h"
#include "UnitController.h"

// FUNCTION: SWBF2 0x0050b400
void PblRef::Kill()
{
    mAliveFlag = 0;
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// GLOBAL: SWBF2 0x01ef7df8
// SYNTHETIC: SWBF2 0x00405e30
// `dynamic initializer for 'AIPathNode::sMemoryPool''
// SYNTHETIC: SWBF2 0x007692d0
// `dynamic atexit destructor for 'AIPathNode::sMemoryPool''
MemoryPool AIPathNode::sMemoryPool("AIPathNode", sizeof(AIPathNode));
// GLOBAL: SWBF2 0x01ef7e98
// SYNTHETIC: SWBF2 0x00405e50
// `dynamic initializer for 'AIPath::sMemoryPool''
// SYNTHETIC: SWBF2 0x007692e0
// `dynamic atexit destructor for 'AIPath::sMemoryPool''
MemoryPool AIPath::sMemoryPool("AIPath", sizeof(AIPath));

namespace PathManager
{

// GLOBAL: SWBF2 0x01ef7e48
// SYNTHETIC: SWBF2 0x00405e70
// `dynamic initializer for 'PathManager::PathRequest::sMemoryPool''
// SYNTHETIC: SWBF2 0x007692f0
// `dynamic atexit destructor for 'PathManager::PathRequest::sMemoryPool''
MemoryPool PathRequest::sMemoryPool("PathRequest", sizeof(PathRequest));

// GLOBAL: SWBF2 0x01eaedc8
PblList<PathRequest>* sRequests;

// GLOBAL: SWBF2 0x01eaedcc
PathRequest* sCurRequest;

// GLOBAL: SWBF2 0x01eaedc4
int mCostConstraint;

// GLOBAL: SWBF2 0x01eaedc0
int sPathManagerUnknown0;
// GLOBAL: SWBF2 0x01eaedd0
int sPathManagerUnknown1;

// GLOBAL: SWBF2 0x007ebc38
// SYNTHETIC: SWBF2 0x00405e90
// `dynamic initializer for 'PathManager::mTimer''
// SYNTHETIC: SWBF2 0x00769300
// `dynamic atexit destructor for 'PathManager::mTimer''
PblPerformanceTimer mTimer;

}

// FUNCTION: SWBF2 0x00613fb0
AIPath::AIPath() : mNodes()
{
}

// FUNCTION: SWBF2 0x00614010
AIPath::~AIPath()
{
    PblList<AIPathNode>::Node* pHead = mNodes.End().GetNode();
    while (mNodes.GetFirst() != pHead)
    {
        PblList<AIPathNode>::Node* pNode = mNodes.GetFirst();
        AIPathNode* pObj = pNode->_pObject;
        mNodes.Remove(pNode);
        AIPathNode::sMemoryPool.Free(pObj);
    }
}

// FUNCTION: SWBF2 0x006140b0
void AIPath::Prepend(const PblVector3& pos, unsigned short uFlags)
{
    if (AIPathNode::sMemoryPool.mUsed < AIPathNode::sMemoryPool.mCount)
    {
        AIPathNode* pNode = (AIPathNode*)AIPathNode::sMemoryPool.Allocate(sizeof(AIPathNode));
        if (pNode)
        {
            new (pNode) AIPathNode(pos, uFlags);
        }
        else
        {
            pNode = 0;
        }
        mNodes.AddFirst(pNode->GetListNode(), pNode);
    }
}

// FUNCTION: SWBF2 0x00614120
void AIPath::Append(const PblVector3& pos, unsigned short uFlags)
{
    if (AIPathNode::sMemoryPool.mUsed < AIPathNode::sMemoryPool.mCount)
    {
        AIPathNode* pNode = (AIPathNode*)AIPathNode::sMemoryPool.Allocate(sizeof(AIPathNode));
        if (pNode)
        {
            new (pNode) AIPathNode(pos, uFlags);
        }
        else
        {
            pNode = 0;
        }
        mNodes.AddLast(pNode->GetListNode(), pNode);
    }
}

// FUNCTION: SWBF2 0x00614190
void AIPath::AppendPath(const AIPath* pOther)
{
    typedef PblList<AIPathNode>::Node Node;
    Node* pOtherHead = pOther->mNodes.End().GetNode();
    Node* pNode = pOther->mNodes.GetFirst();

    AIPathNode* pFirst = pNode->_pObject;
    AIPathNode* pMyLast = mNodes.GetLast()->_pObject;
    if (pMyLast->mPos.x == pFirst->mPos.x && pMyLast->mPos.y == pFirst->mPos.y
        && pMyLast->mPos.z == pFirst->mPos.z)
    {
        pNode = pNode->_pNext;
    }
    for (; pNode != pOtherHead; pNode = pNode->_pNext)
    {
        AIPathNode* pObj = pNode->_pObject;
        Append(pObj->mPos, pObj->mFlags);
    }
}

// FUNCTION: SWBF2 0x00614210
void AIPath::PrependPath(const AIPath* pOther)
{
    typedef PblList<AIPathNode>::Node Node;
    Node* pOtherHead = pOther->mNodes.End().GetNode();
    for (Node* pNode = pOther->mNodes.GetFirst(); pNode != pOtherHead; pNode = pNode->_pNext)
    {
        AIPathNode* pObj = pNode->_pObject;
        Prepend(pObj->mPos, pObj->mFlags);
    }
}

// FUNCTION: SWBF2 0x00614250
void AIPath::PopHead()
{
    PblList<AIPathNode>::Node* pNode = mNodes.GetFirst();
    AIPathNode* pObj = pNode->_pObject;
    mNodes.Remove(pNode);
    AIPathNode::sMemoryPool.Free(pObj);
}

// FUNCTION: SWBF2 0x006142a0
void AIPath::TrimBefore(AIPathNode* pStop)
{
    typedef PblList<AIPathNode>::Node Node;
    Node* pHead = mNodes.End().GetNode();
    Node* pNode = mNodes.GetFirst();
    while (pNode != pHead && pNode->_pObject != pStop)
    {
        Node* pNext = pNode->_pNext;
        AIPathNode* pObj = pNode->_pObject;
        mNodes.Remove(pNode);
        AIPathNode::sMemoryPool.Free(pObj);
        pNode = pNext;
    }
}

// FUNCTION: SWBF2 0x00614320
bool AIPath::SmoothCorners(float fCornerDist, float fRadius, int eProhibit)
{
    typedef PblList<AIPathNode>::Node Node;

    if (mNodes.GetCount() < 3)
    {
        return false;
    }

    bool bSmoothed = false;
    Node* pNode = mNodes.GetFirst();
    AIPathNode* pCur = pNode->_pObject;
    pNode = pNode->_pNext;
    AIPathNode* pNext = pNode->_pObject;
    for (pNode = pNode->_pNext; pNode->_pObject; pNode = pNode->_pNext)
    {
        AIPathNode* pPrev = pCur;
        pCur = pNext;
        pNext = pNode->_pObject;

        if ((int)(AIPathNode::sMemoryPool.mCount - AIPathNode::sMemoryPool.mUsed) < 64 || mNodes.GetCount() >= 30)
        {
            break;
        }

        PblVector3& vCur = pCur->mPos;
        const PblVector3& vPrev = pPrev->mPos;
        const PblVector3& vNext = pNext->mPos;
        float fMinDistSq = fCornerDist * fCornerDist * 4.0f;

        float fX = vPrev.x - vCur.x, fY = vPrev.y - vCur.y, fZ = vPrev.z - vCur.z;
        if (fMinDistSq > fX * fX + fY * fY + fZ * fZ)
        {
            continue;
        }

        fX = vCur.x - vNext.x, fY = vCur.y - vNext.y, fZ = vCur.z - vNext.z;
        if (fMinDistSq > fX * fX + fY * fY + fZ * fZ)
        {

            pNode = pNode->_pNext;
            if (!pNode->_pObject)
            {
                break;
            }
            continue;
        }

        PblVector3 vIn(vCur.x - vPrev.x, vCur.y - vPrev.y, vCur.z - vPrev.z);
        PblVector3 vDirIn;
        D3DXVec3Normalize(&vDirIn, &vIn);
        PblVector3 vOut(vNext.x - vCur.x, vNext.y - vCur.y, vNext.z - vCur.z);
        PblVector3 vDirOut;
        D3DXVec3Normalize(&vDirOut, &vOut);
        if (vDirOut.x * vDirIn.x + vDirIn.y * vDirOut.y + vDirIn.z * vDirOut.z > 0.984f)
        {
            continue;
        }

        PblVector3 vStart(vCur.x - vDirIn.x * fCornerDist, vCur.y - vDirIn.y * fCornerDist,
                          vCur.z - vDirIn.z * fCornerDist);
        PblVector3 vEnd(vCur.x + vDirOut.x * fCornerDist, vCur.y + vDirOut.y * fCornerDist,
                        vCur.z + vDirOut.z * fCornerDist);
        PblVector3 vMid((vCur.x + vStart.x + vEnd.x) * (1.0f / 3.0f),
                        (vCur.y + vStart.y + vEnd.y) * (1.0f / 3.0f),
                        (vCur.z + vStart.z + vEnd.z) * (1.0f / 3.0f));

        if (PathFinder::TestTrivialStraightLineHit(vStart, vMid, fRadius, eProhibit, 0, 0, false))
        {
            continue;
        }
        if (PathFinder::TestTrivialStraightLineHit(vMid, vEnd, fRadius, eProhibit, 0, 0, false))
        {
            continue;
        }

        AIPathNode* pStart = (AIPathNode*)AIPathNode::sMemoryPool.Allocate(sizeof(AIPathNode));
        if (pStart)
        {
            new (pStart) AIPathNode(vStart, 0);
        }
        else
        {
            pStart = 0;
        }
        mNodes.AddBefore(pCur->GetListNode(), pStart->GetListNode(), pStart);

        AIPathNode* pEnd = (AIPathNode*)AIPathNode::sMemoryPool.Allocate(sizeof(AIPathNode));
        if (pEnd)
        {
            new (pEnd) AIPathNode(vEnd, 0);
        }
        else
        {
            pEnd = 0;
        }
        mNodes.AddAfter(pCur->GetListNode(), pEnd->GetListNode(), pEnd);

        bSmoothed = true;
        vCur = vMid;

        pCur = pEnd;
    }
    return bSmoothed;
}

// SYNTHETIC: SWBF2 0x0050b420
// PblRef::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006148b0
bool PathManager::GetPathPriority(const PblVector3* a, const PblVector3* b)
{
    float dx = a->x - b->x;
    float dz = a->z - b->z;
    return dx * dx + dz * dz >= 2500.0f;
}

namespace PathManager
{

// FUNCTION: SWBF2 0x00614730
PathRequest::PathRequest(UnitController* pUnit, const PblVector3& vStart, const PblVector3& vGoal,
                         float fRadius, int eProhibit, unsigned char uPriority,
                         const ConnectivityArc*  ,
                         const Obstacle** ppIgnore, int iIgnoreCount)
    : mUnit(pUnit), mStart(vStart), mGoal(vGoal), mRadius(fRadius), mProhibit(eProhibit),
      mPriority(uPriority), mObstacleCount(iIgnoreCount)
{
    memcpy(mObstacles, ppIgnore, sizeof(const Obstacle*) * iIgnoreCount);
}

// FUNCTION: SWBF2 0x006147d0
void Initialize()
{
    PathRequest::sMemoryPool.Create(0x2ee, PathRequest::sMemoryPool.GetSize(),
                                    PathRequest::sMemoryPool.GetGrow());
    AIPath::sMemoryPool.Create(0x4b0, AIPath::sMemoryPool.GetSize(), AIPath::sMemoryPool.GetGrow());
    AIPathNode::sMemoryPool.Create(0x2580, AIPathNode::sMemoryPool.GetSize(),
                                   AIPathNode::sMemoryPool.GetGrow());
    sRequests = new PblList<PathRequest>();
    sCurRequest = 0;
    sPathManagerUnknown0 = 0;
    sPathManagerUnknown1 = 0;
}

// FUNCTION: SWBF2 0x00614870
void Reset()
{
    sRequests = 0;
    sCurRequest = 0;
    PathRequest::sMemoryPool.Reset();
    AIPath::sMemoryPool.Reset();
    AIPathNode::sMemoryPool.Reset();
}

// FUNCTION: SWBF2 0x006148e0
void RemovePathRequest(UnitController* pCtrl)
{
    if (sCurRequest && sCurRequest->mUnit == pCtrl)
    {
        PathFinder::CancelPathFind();
        PathRequest::sMemoryPool.Free(sCurRequest);
        sCurRequest = 0;
    }

    for (PblList<PathRequest>::Itor it(*sRequests); it != sRequests->End(); ++it)
    {
        PathRequest* pReq = it;
        if (pReq->mUnit == pCtrl)
        {
            sRequests->Remove(&pReq->mNode);
            PathRequest::sMemoryPool.Free(pReq);
            return;
        }
    }
}

// FUNCTION: SWBF2 0x006149b0
void LowerRequestPriority(UnitController* pCtrl)
{
    if (sCurRequest && sCurRequest->mUnit == pCtrl)
    {
        return;
    }

    for (PblList<PathRequest>::Itor it(*sRequests); it != sRequests->End(); ++it)
    {
        PathRequest* pReq = it;
        if (pReq->mUnit == pCtrl)
        {
            if (pReq->mPriority)
            {
                pReq->mPriority = reinterpret_cast<AILowLevel*>(pCtrl->mLowLevel)->GetPathQueueDepth();
            }
            return;
        }
    }
}

// FUNCTION: SWBF2 0x00614a60
bool RequestPath(UnitController* pUnit, const PblVector3& vStart, const PblVector3& vGoal,
                 float fRadius, int eProhibit, unsigned char uPriority,
                 const ConnectivityArc* pArc, const Obstacle** ppIgnore, int iIgnoreCount)
{
    AILowLevel* pLow = reinterpret_cast<AILowLevel*>(pUnit->mLowLevel);
    unsigned char uPriorityPlusDepth = (unsigned char)(pLow->GetPathQueueDepth() + uPriority);
    RemovePathRequest(pUnit);

    if (pLow->GetPathQueueDepth() <= 4
        && !PathFinder::TestTrivialStraightLineHit(vStart, vGoal, fRadius, eProhibit, ppIgnore,
                                                    iIgnoreCount, false))
    {
        AIPath* pPath = (AIPath*)AIPath::sMemoryPool.Allocate(sizeof(AIPath));
        if (pPath)
        {
            pPath = ::new (pPath) AIPath();
        }
        if (pPath)
        {
            pPath->Prepend(vGoal, 0);
            pPath->Prepend(vStart, 0);
            AI::EventInfo_PathResult event(pPath);
            pUnit->SendEvent(&event);
            pPath->Release();
            return true;
        }
        return false;
    }

    if ((unsigned int)PathRequest::sMemoryPool.mUsed >= PathRequest::sMemoryPool.mCount)
    {
        return false;
    }

    void* pMem = PathRequest::sMemoryPool.Allocate(sizeof(PathRequest));
    if (!pMem)
    {
        return false;
    }
    PathRequest* pReq = new (pMem) PathRequest(pUnit, vStart, vGoal, fRadius, eProhibit,
                                               uPriorityPlusDepth, pArc, ppIgnore, iIgnoreCount);
    sRequests->AddLast(&pReq->mNode, pReq);
    return true;
}

// FUNCTION: SWBF2 0x00614bb0
PathRequest* GetNextPathRequest()
{
    for (unsigned char uPriority = 0; uPriority < 5; ++uPriority)
    {
        PblList<PathRequest>::Itor it(*sRequests);
        while (it != sRequests->End())
        {
            PathRequest* pReq = it;
            ++it;
            if (pReq->mPriority == uPriority)
            {
                sRequests->Remove(&pReq->mNode);
                UnitController* pObj = pReq->mUnit.mObject;
                if (pObj && pObj->mHandleId == pReq->mUnit.mSavedHandleId)
                {
                    return pReq;
                }
                pReq->mUnit.mObject = 0;
                pReq->mUnit.mSavedHandleId = 0;
                PathRequest::sMemoryPool.Free(pReq);
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00614c40
bool IsDone()
{
    return PathFinder::IsCompleted() && sRequests->GetCount() == 0;
}

// FUNCTION: SWBF2 0x00614c60
void Update(float fDeltaTime)
{
    mCostConstraint = (int)(fDeltaTime * 10.0f * 5.0f / 5.0f);
    PathFinder::ResetCost();

    mTimer._iAccum = 0;
    QueryPerformanceCounter((LARGE_INTEGER*)&mTimer._iValue);

    if (IsWithinTimeConstraint())
    {
        for (;;)
        {
            if (PathFinder::IsCompleted())
            {
                sCurRequest = GetNextPathRequest();
                if (!sCurRequest)
                {
                    break;
                }
                PathFinder::NewPathFind(sCurRequest->mStart, sCurRequest->mGoal, sCurRequest->mRadius,
                                        sCurRequest->mProhibit, sCurRequest->mObstacles,
                                        sCurRequest->mObstacleCount);
            }
            if (!PathFinder::IsCompleted())
            {
                PathFinder::UpdatePathFind();
                if (PathFinder::GetPausedAtFinish())
                {
                    break;
                }
            }
            if (PathFinder::IsCompleted())
            {
                AIPath* pPath = PathFinder::GetPathResult();
                UnitController* pUnit = sCurRequest->mUnit.mObject;
                bool bValid = sCurRequest->mUnit == pUnit;
                if (!bValid)
                {
                    pUnit = 0;
                }
                if (pUnit)
                {
                    AI::EventInfo_PathResult event(pPath);
                    pUnit->SendEvent(&event);
                }
                if (pPath)
                {
                    pPath->Release();
                }
                if (sCurRequest)
                {
                    PathRequest::sMemoryPool.Free(sCurRequest);
                }
                sCurRequest = 0;
            }
            if (!IsWithinTimeConstraint())
            {
                break;
            }
        }
    }

    LARGE_INTEGER qEnd;
    QueryPerformanceCounter(&qEnd);
    mTimer._iAccum += qEnd.QuadPart - mTimer._iValue;
}

// FUNCTION: SWBF2 0x00614e00
bool IsWithinTimeConstraint()
{
    return PathFinder::GetCost() < mCostConstraint;
}

}
