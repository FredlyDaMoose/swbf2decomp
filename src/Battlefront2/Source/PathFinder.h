#pragma once

#include "CollisionObject.h"
#include "TreeGrid.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class GameObject;
class PblMatrix;
class AIPath;

class ObstacleTest
{
public:
    ObstacleTest() {}

    ObstacleTest(const PblVector3& rSrc, const PblVector3& rDest);

    PblVector3 mSrc;
    PblVector3 mDest;
    PblVector3 mBoundCenter;
    float mBoundRadius;
};
 

class Obstacle : public TreeGridObject
{
public:

    Obstacle(const PblMatrix& rMatrix, const PblVector4& rSize, int eProhibit, unsigned int uName);

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* p) { sMemoryPool.Free(p); }

    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow)
    {
        sMemoryPool.Setup(uCount, uSize, uGrow);
    }

    void UpdatePosition(const PblMatrix& rMatrix, float fOffsetX, float fOffsetZ,
                        const PblVector3& rVelocity);
    bool IsPointInside(const PblVector3& rPos, float fRadius) const;
    PblVector3 GetVertexXZ(int iVertex, float fRadius, float fPushOut) const;
    PblVector3 GetPushedOutVertexXZ(int iVertex, float fRadius) const;
    float GetDistFromSideXZ(const PblVector3& rPos, float fRadius, int iSide,
                            PblVector3& rClosest) const;
    bool IntersectionNewer(const ObstacleTest& rTest, float fRadius, float* pDistSq,
                           PblVector3* pNormal, PblVector3* pVertex0, PblVector3* pVertex1) const;
    void FindCorners(const PblVector3& rSrc, const PblVector3& rDest, float fRadius,
                     PblVector3* pLeft, PblVector3* pRight) const;
    bool Prohibits(int eProhibit) const;
    void ToggleEnabled();
    void SetEnabled(bool bEnabled);

    PblVector3 GetBoundingCircleCenter() const { return PblVector3(mPosX, 0.0f, mPosZ); }
    float GetBoundingCircleRadius() const { return mSizeRad; }

    PblVector3 GetXAxis() const { return PblVector3(-mForwardZ, 0.0f, mForwardX); }
    PblVector3 GetZAxis() const { return PblVector3(mForwardX, 0.0f, mForwardZ); }
    PblVector3 GetTranslation() const { return PblVector3(mPosX, 0.0f, mPosZ); }

    float mForwardX;
    float mForwardZ;
    float mPosX;
    float mPosZ;
    float mSizeX;
    float mSizeZ;
    float mSizeRad;
    unsigned int mNameHash;
    unsigned char mProhibitFlags : 5;
    unsigned char mIsEnabled : 1;

    static MemoryPool sMemoryPool;
};
 

namespace PathFinder
{

    class PathNode
    {
    public:

        PathNode(const PblVector3& rPos, float fCostSoFar, const PathNode* pParent)
            : mPos(rPos), mCostSoFar(fCostSoFar), mParentNode(const_cast<PathNode*>(pParent))
        {
        }
        void Set(const PblVector3& rPos, float fCostSoFar, const PathNode* pParent)
        {
            mPos = rPos;
            mCostSoFar = fCostSoFar;
            mParentNode = const_cast<PathNode*>(pParent);
        }

        static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
        static void operator delete(void* p) { sMemoryPool.Free(p); }

        const PblVector3& GetPos() const { return mPos; }
        float GetCostSoFar() const { return mCostSoFar; }

        PblVector3 mPos;
        float mCostSoFar;
        PathNode* mParentNode;
        PblList<PathNode>::Node mNode;

        static MemoryPool sMemoryPool;

        static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow)
        {
            sMemoryPool.Setup(uCount, uSize, uGrow);
        }
    };
     

    struct ObstacleData : public TreeGridCallbackData
    {
        Obstacle* mArray[300];
        int mLength;
    };

    struct PointValidData : public TreeGridCallbackData
    {
        PointValidData() : hit(false), hitObs(0) {}

        PblVector3 mPos;
        float mRadius;
        int mProhibit;
        bool hit;
        Obstacle* hitObs;
        Obstacle* mIgnore[2];
        int mIgnoreCnt;
    };

    struct PushValidPointData : public TreeGridCallbackData
    {
        PushValidPointData() : mTruncated(false), mIgnoreCnt(0) {}

        ObstacleTest mObsTest;
        PblVector3 mRayDir;
        float mRadius;
        int mProhibit;
        PblVector3 mNormal;
        bool mTruncated;
        Obstacle* mIgnore[2];
        int mIgnoreCnt;
    };

    struct TrivialIntersectData : public TreeGridCallbackData
    {
        ObstacleTest mObsTest;
        float mRadiusVehicle;
        int mProhibit;
        Obstacle* mIgnore[2];
        int mIgnoreCnt;
        bool hit;
        bool mOnlyDynamic;
    };

     
     
     
     

    extern bool mRespectEnabled;

    void Initialize();
    void Reset();
    void ResetCost();
    int GetCost();
    void AddCost();

    void AddObstacle(const PblMatrix& rMatrix, const PblVector4& rSize, int eProhibit,
                     unsigned int uName);
    void CreateGameObjectBarrier(Obstacle*& rpObstacle, GameObject* pObject, float fSizeX,
                                 float fSizeZ, float fOffsetX, float fOffsetZ, unsigned int uName);
    void DeleteGameObjectBarrier(Obstacle*& rpObstacle);
    void MoveBarrier(Obstacle* pObstacle);

    bool IsPointValid(const PblVector3& rPos, float fRadius, int eProhibit,
                      const Obstacle** ppHit, const Obstacle** ppIgnore, int iIgnoreCnt);
    bool IsPointValid(const PblVector3& rPos, const Obstacle** ppHit, const GameObject* pObj);
    bool PushValidPoint(const PblVector3& rSrc, PblVector3& rDest, PblVector3& rNormal,
                        float fRadius, int eProhibit, const Obstacle** ppIgnore, int iIgnoreCnt);
    bool PushValidPoint(const PblVector3& rSrc, PblVector3& rDest, const GameObject* pObj);
    bool GetClosestValidPoint(const PblVector3& rPos, PblVector3& rOut, float fRadius,
                              int eProhibit, const Obstacle** ppIgnore, int iIgnoreCnt);
    bool GetClosestValidPoint(const PblVector3& rPos, PblVector3& rOut, const GameObject* pObj);

    bool PointValidCallback(const TreeGridObject* pObj, TreeGridCallbackData* pData, float& fDist);
    bool PushValidPointCallback(const TreeGridObject* pObj, TreeGridCallbackData* pData, float& fDist);
    bool GenerateObstacleListCallback(const TreeGridObject* pObj, TreeGridCallbackData* pData,
                                      float& fDist);
    float sInvertedTotalCost(const PathNode* pNode);
    bool TrivialIntersectCallback(const TreeGridObject* pObj, TreeGridCallbackData* pData, float& fDist);
    bool sSamePosXZ(const PblVector3& a, const PblVector3& b);

    bool FindClosestObstacleIntersection(const PblVector3& rSrc, const PblVector3& rDest, float fRadius,
                                         int eProhibit, const Obstacle** ppHit, float* pDistSq,
                                         bool bParam, const Obstacle** ppIgnore, int iIgnoreCnt);

    bool TestTrivialStraightLineHit(const PblVector3& from, const PblVector3& to,
                                    const GameObject* pObj);

    bool TestTrivialStraightLineHit(const PblVector3& from, const PblVector3& to, float fRadius,
                                    int eProhibit, const Obstacle* const* ppIgnore, int iIgnoreCnt,
                                    bool bOnlyDynamic);

    void NewPathFind(const PblVector3& vStart, const PblVector3& vGoal, float fRadius,
                     int eProhibit, const Obstacle** ppIgnore, int iIgnoreCnt);
    bool IsCompleted();
    bool GetPausedAtFinish();
    void UpdatePathFind();
    AIPath* GetPathResult();
    void CancelPathFind();
}

template <class T, class KEY> class PblHeap;
template <class TYPE, class INDEX> class ListPool;
extern int sCost;
extern PblVector3 sDestPos;

extern ListPool<PblVector3, int>* sSearchedPos;
extern PblList<PathFinder::PathNode>* sClosedList;
extern PblHeap<PathFinder::PathNode*, float>* sOpenList;
extern int sPausedAtFinish;
extern bool sFoundDest;
extern bool sComplete;
extern PblVector3 sPerpVec;
extern PblVector3 sSrcPos;
extern Obstacle* sIgnore[2];
extern float sPerpLength;
extern int sProhibit;
extern int sIgnoreCnt;
extern float sRadiusVehicle;

namespace PathFinder { void sExpandNode(PathFinder::PathNode* pParent, const PblVector3& rPos, int iDepth); }
namespace AIDebug { bool PauseAfterPathFind(); }

float GetDistanceSquaredFromLineSegment(const PblVector3& rEnd, const PblVector3& rStart,
                                        const PblVector3& rPos, PblVector3& rClosest);
