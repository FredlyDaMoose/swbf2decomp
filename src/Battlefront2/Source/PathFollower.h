#pragma once

#include "PathManager.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class Obstacle;
class UnitController;

enum SafeHeight
{
    SH_FULL = 0,
    SH_TAPER = 1,
    SH_NONE = 2,
    SH_TAPER_FACTOR = 3
};

enum ForceDestSide
{
    DS_LEFT = -1,
    DS_EITHER = 0,
    DS_RIGHT = 1
};

enum PathFollowerDoneReason
{
    PF_NOTDONE = 0,
    PF_PATHDONE = 1,
    PF_PATHBLOCKED = 2
};

class PathFollower
{
public:

    enum WaitState
    {
        WS_NONE = 0,
        WS_CROUCH = 1,
        WS_LOOKAROUND = 2
    };

    static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }
    static MemoryPool sMemoryPool;

    static void GetStuckParams(UnitController* pCtrl, float& fStuckDist, float& fStuckTime, float& fMinDistTime);

    PathFollower(UnitController* pCtrl, AIPath* pPath, const Obstacle** ppIgnore,
                 int iIgnoreCnt, bool bDoCallbacksOnPts);

    PathFollower(UnitController* pCtrl, const PblVector3& vStart, const PblVector3& vEnd,
                 const Obstacle** ppIgnore, int iIgnoreCnt, bool bDoCallbacksOnPts);
    ~PathFollower();

    void ConstructorCommon();

    void AppendPath(const AIPath* pPath, bool b);
    bool PrependPath(const AIPath* pPath);
    void UpdateControllable();
    void Update(float dt);

    unsigned char GetPathQueueDepth();
    void Wait(float fTime);

    void SetLocalPoints(const PblVector3& vPrev,
                        const PblList<AIPathNode>::Itor& itor);
    bool CloseToDest() const;
    bool GonePastDest() const;
    void GotoNextNode();
    bool TestBarriersAhead() const;

    void CheckStuck(float dt);
    void UpdateControllableStop();
    void UpdateControllableMoving();

    AIPath* mPath;
    UnitController* mCtrl;
    PblList<AIPathNode>::Itor mItor;
    PblVector3 mPrevPoint;
    PblVector3 mGotoPoint;
    PblVector3 mNextPoint;
    PblVector3 mFinalPoint;
    PblVector3 mDir;
    SafeHeight mSafeHeight;
    ForceDestSide mDestSide;
    int mIgnoreCnt;
    Obstacle* mIgnore[2];
    bool mDoCallbacksOnPts;
    bool mDone;
    PathFollowerDoneReason mDoneReason;
    int mIsStuck;
    PblVector3 mStuckPos;
    float mStuckTime;
    float mMinDistToGoal;
    float mMinDistToGoalTime;
    bool mPaused;
    float mWaitTime;
    WaitState mWaitState;
};

 
