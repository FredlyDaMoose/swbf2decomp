#pragma once

#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblRef.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "PerformanceTimer.h"

class MemoryPool;
class UnitController;
class ConnectivityArc;
class Obstacle;

class AIPathNode
{
public:

    enum
    {
        AIPN_END_OF_SEGMENT = 1,
        AIPN_WAITLEFT = 2,
        AIPN_WAITRIGHT = 4,
        AIPN_WAITLOOKAROUND = 8,
        AIPN_WAITLONG = 16,
        AIPN_JUMP = 32,
        AIPN_JETJUMP = 64,
        AIPN_ROLLOUT = 128,
        AIPN_ROLLIN = 256
    };

    // FUNCTION: SWBF2 0x00613f60
    AIPathNode(const PblVector3& pos, unsigned short flags) : mPos(pos), mFlags(flags) {}

    AIPathNode() : mFlags(0) {}

    // FUNCTION: SWBF2 0x00613fa0
    PblList<AIPathNode>::Node* GetListNode() { return &mNode; }

    PblList<AIPathNode>::Node mNode;
    PblVector3 mPos;
    unsigned short mFlags;

    static MemoryPool sMemoryPool;
};

// VTABLE: SWBF2 0x007ad610
class AIPath : public PblRef
{
public:

    // FUNCTION: SWBF2 0x00614310
    const PblList<AIPathNode>& GetNodeList() const { return mNodes; }

    static void* operator new(unsigned int uiSize);

    AIPath();

    virtual ~AIPath();

    // SYNTHETIC: SWBF2 0x00613fe0
    // AIPath::`scalar deleting destructor' -- compiler-generated from the

    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    void Append(const PblVector3& pos, unsigned short uFlags);
    void Prepend(const PblVector3& pos, unsigned short uFlags);
    void AppendPath(const AIPath* pOther);
    void PrependPath(const AIPath* pOther);

    void PopHead();

    void TrimBefore(AIPathNode* pNode);

    bool SmoothCorners(float fCornerDist, float fRadius, int eProhibit);

    static MemoryPool sMemoryPool;

    PblList<AIPathNode> mNodes;
};

 
 

namespace PathManager
{

class PathRequest
{
public:

    PathRequest(UnitController* pUnit, const PblVector3& vStart, const PblVector3& vGoal,
                float fRadius, int eProhibit, unsigned char uPriority,
                const ConnectivityArc* pArc, const Obstacle** ppIgnore, int iIgnoreCount);

    PblList<PathRequest>::Node mNode;
    PblHandle<UnitController> mUnit;
    PblVector3 mStart;
    PblVector3 mGoal;
    float mRadius;
    int mProhibit;
    unsigned char mPriority;
    const Obstacle* mObstacles[2];
    int mObstacleCount;

    static MemoryPool sMemoryPool;
};
 

extern int mCostConstraint;

void Initialize();
void Reset();

void RemovePathRequest(UnitController* pCtrl);
void LowerRequestPriority(UnitController* pCtrl);
bool RequestPath(UnitController* pUnit, const PblVector3& vStart, const PblVector3& vGoal,
                 float fRadius, int eProhibit, unsigned char uPriority,
                 const ConnectivityArc* pArc, const Obstacle** ppIgnore, int iIgnoreCount);
PathRequest* GetNextPathRequest();
bool IsDone();

void Update(float fDeltaTime);
bool IsWithinTimeConstraint();

}

namespace PathManager { bool GetPathPriority(const PblVector3* a, const PblVector3* b); }
