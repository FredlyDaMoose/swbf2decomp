#include <float.h>
#include <math.h>
#include <string.h>

#include "PathFinder.h"
#include "AIUtil.h"
#include "ListPool.h"
#include "Entity.h"
#include "NetGame.h"
#include "PathManager.h"
#include "PebbleFL/Common/PblHeap.h"
#include "PebbleFL/Common/PblMatrix.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

extern int WORLD_EXTENTS;

static const float EPSILON = 0.0001f;
static const float HEAP_MAX_VALUE = 100000.0f;
static const float EXTRA_OBSTACLE_PUSHOUT_OFFSET = 0.5f;

// GLOBAL: SWBF2 0x01ef7cf0
// SYNTHETIC: SWBF2 0x00405dd0
// `dynamic initializer for 'Obstacle::sMemoryPool''
// SYNTHETIC: SWBF2 0x007692a0
// `dynamic atexit destructor for 'Obstacle::sMemoryPool''
MemoryPool Obstacle::sMemoryPool("Obstacle", sizeof(Obstacle));

// GLOBAL: SWBF2 0x01ef7d40
// SYNTHETIC: SWBF2 0x00405df0
// `dynamic initializer for 'PathFinder::PathNode::sMemoryPool''
// SYNTHETIC: SWBF2 0x007692b0
// `dynamic atexit destructor for 'PathFinder::PathNode::sMemoryPool''
MemoryPool PathFinder::PathNode::sMemoryPool("PathNode", sizeof(PathFinder::PathNode));

// GLOBAL: SWBF2 0x007e8ee8
bool PathFinder::mRespectEnabled = true;

// GLOBAL: SWBF2 0x007e8ee9
bool sComplete = true;

// GLOBAL: SWBF2 0x01eaed9c
TreeGrid* sObstacles;

// GLOBAL: SWBF2 0x01eaeda0
ListPool<PblVector3, int>* sSearchedPos;

// GLOBAL: SWBF2 0x01eaeda4
PblList<PathFinder::PathNode>* sClosedList;

// GLOBAL: SWBF2 0x01eaeda8
int sCost;

// GLOBAL: SWBF2 0x01eaedb4
PblHeap<PathFinder::PathNode*, float>* sOpenList;

// GLOBAL: SWBF2 0x01eaedb8
int sPausedAtFinish;

// GLOBAL: SWBF2 0x01eaedbc
bool sFoundDest;

// GLOBAL: SWBF2 0x01ef7c94
PblVector3 sPerpVec;

// GLOBAL: SWBF2 0x01ef7d90
PblVector3 sSrcPos;

// GLOBAL: SWBF2 0x01ef7d9c
PblVector3 sDestPos;

// GLOBAL: SWBF2 0x01fa8c38
Obstacle* sIgnore[2];

// GLOBAL: SWBF2 0x01fa8c40
float sPerpLength;

// GLOBAL: SWBF2 0x01fa8c44
int sProhibit;

// GLOBAL: SWBF2 0x01fa8c48
int sIgnoreCnt;

// GLOBAL: SWBF2 0x01fa8c4c
float sRadiusVehicle;

// GLOBAL: SWBF2 0x007ec548

int gPathResultFailureCount = 1;

// FUNCTION: SWBF2 0x0060ff20
Obstacle::Obstacle(const PblMatrix& rMatrix, const PblVector4& rSize, int eProhibit,
                   unsigned int uName)
    : TreeGridObject(-1)
{
    mNameHash = uName;
    mProhibitFlags = eProhibit;
    mIsEnabled = 1;
    mForwardX = rMatrix.m[2][0];
    mForwardZ = rMatrix.m[2][2];
    mPosX = rMatrix.m[3][0];
    mPosZ = rMatrix.m[3][2];
    mSizeX = rSize.x;
    mSizeZ = rSize.z;
    mSizeRad = (float)sqrt(mSizeX * mSizeX + mSizeZ * mSizeZ);
}

// FUNCTION: SWBF2 0x0060ffc0
void Obstacle::UpdatePosition(const PblMatrix& rMatrix, float fOffsetX, float fOffsetZ,
                              const PblVector3& rVelocity)
{
    if (rVelocity.x * rVelocity.x + rVelocity.y * rVelocity.y + rVelocity.z * rVelocity.z < 36.0f)
    {
        SetEnabled(true);
        PblVector3 forward = AIUtil::GetSafeNormalXZ(*(const PblVector3*)rMatrix.m[2]);
        float fPosX = rMatrix.m[3][0] + forward.x * fOffsetZ + -forward.z * fOffsetX;
        float fPosZ = rMatrix.m[3][2] + forward.z * fOffsetZ + forward.x * fOffsetX;
        float fDX = fPosX - mPosX;
        float fDZ = fPosZ - mPosZ;
        PblVector3 zAxis = GetZAxis();
        if (fDX * fDX + fDZ * fDZ > 0.25f ||
            zAxis.x * forward.x + zAxis.y * forward.y + zAxis.z * forward.z < 0.99f)
        {
            mForwardX = forward.x;
            mForwardZ = forward.z;
            mPosX = fPosX;
            mPosZ = fPosZ;
            PathFinder::MoveBarrier(this);
        }
    }
    else
    {
        SetEnabled(false);
    }
}

// FUNCTION: SWBF2 0x00610100
bool Obstacle::IsPointInside(const PblVector3& rPos, float fRadius) const
{
    if (!mIsEnabled && PathFinder::mRespectEnabled)
    {
        return false;
    }

    PblVector3 pos = GetTranslation();
    PblVector3 d(pos.x - rPos.x, pos.y - rPos.y, pos.z - rPos.z);
    PblVector3 xAxis = GetXAxis();
    if ((float)fabs(xAxis.x * d.x + xAxis.y * d.y + xAxis.z * d.z) < mSizeX + fRadius)
    {
        PblVector3 zAxis = GetZAxis();
        if ((float)fabs(zAxis.x * d.x + zAxis.y * d.y + zAxis.z * d.z) < mSizeZ + fRadius)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x006101d0
PblVector3 Obstacle::GetVertexXZ(int iVertex, float fRadius, float fPushOut) const
{
    float fX = mSizeX + fRadius + fPushOut;
    float fZ = mSizeZ + fRadius + fPushOut;
    PblVector3 xAxis = GetXAxis();
    PblVector3 zAxis = GetZAxis();
    PblVector3 x(xAxis.x * fX, xAxis.y * fX, xAxis.z * fX);
    PblVector3 z(zAxis.x * fZ, zAxis.y * fZ, zAxis.z * fZ);
    PblVector3 pos = GetTranslation();

    switch (iVertex)
    {
    case 0:
        return PblVector3(pos.x + x.x + z.x, pos.y + x.y + z.y, pos.z + x.z + z.z);
    case 1:
        return PblVector3(pos.x + x.x - z.x, pos.y + x.y - z.y, pos.z + x.z - z.z);
    case 2:
        return PblVector3(pos.x - x.x - z.x, pos.y - x.y - z.y, pos.z - x.z - z.z);
    default:
        return PblVector3(pos.x - x.x + z.x, pos.y - x.y + z.y, pos.z - x.z + z.z);
    }
}

// FUNCTION: SWBF2 0x006102f0
PblVector3 Obstacle::GetPushedOutVertexXZ(int iVertex, float fRadius) const
{
    return GetVertexXZ(iVertex, fRadius, EXTRA_OBSTACLE_PUSHOUT_OFFSET);
}

// FUNCTION: SWBF2 0x00610310
float GetDistanceSquaredFromLineSegment(const PblVector3& rEnd, const PblVector3& rStart,
                                        const PblVector3& rPos, PblVector3& rClosest)
{
    PblVector3 dir(rEnd.x - rStart.x, rEnd.y - rStart.y, rEnd.z - rStart.z);
    float fT = (rPos.x - rStart.x) * dir.x + (rPos.y - rStart.y) * dir.y + (rPos.z - rStart.z) * dir.z;
    if (fT <= 0.0f)
    {
        rClosest = rStart;
    }
    else
    {
        float fLengthSq = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
        if (fT >= fLengthSq)
        {
            rClosest = rEnd;
        }
        else
        {
            fT /= fLengthSq;
            rClosest = PblVector3(rStart.x + dir.x * fT, rStart.y + dir.y * fT, rStart.z + dir.z * fT);
        }
    }
    float fDX = rPos.x - rClosest.x;
    float fDY = rPos.y - rClosest.y;
    float fDZ = rPos.z - rClosest.z;
    return fDX * fDX + fDY * fDY + fDZ * fDZ;
}

// FUNCTION: SWBF2 0x00610420
float Obstacle::GetDistFromSideXZ(const PblVector3& rPos, float fRadius, int iSide,
                                  PblVector3& rClosest) const
{
    PblVector3 pos(rPos.x, 0.0f, rPos.z);
    int iNext = iSide < 3 ? iSide + 1 : 0;
    PblVector3 v0 = GetVertexXZ(iSide, fRadius, 0.0f);
    PblVector3 v1 = GetVertexXZ(iNext, fRadius, 0.0f);

    PblVector3 closest;
    float fDistSq = GetDistanceSquaredFromLineSegment(v0, v1, pos, closest);

    PblVector3 normal(v1.z - v0.z, 0.0f, -(v1.x - v0.x));
    PblVector3 n;
    D3DXVec3Normalize(&n, &normal);
    rClosest = PblVector3(closest.x + n.x * 0.1f, closest.y + n.y * 0.1f, closest.z + n.z * 0.1f);
    return fDistSq;
}

// FUNCTION: SWBF2 0x00610540
bool Obstacle::IntersectionNewer(const ObstacleTest& rTest, float fRadius, float* pDistSq,
                                 PblVector3* pNormal, PblVector3* pVertex0,
                                 PblVector3* pVertex1) const
{
    if (!mIsEnabled && PathFinder::mRespectEnabled)
    {
        return false;
    }

    float fTestRadius = rTest.mBoundRadius + fRadius;
    PblVector3 center = GetBoundingCircleCenter();
    float fDZ = rTest.mBoundCenter.z - center.z;
    float fDX = rTest.mBoundCenter.x - center.x;
    float fReach = GetBoundingCircleRadius() + fTestRadius;
    if (fDZ * fDZ + fDX * fDX >= fReach * fReach)
    {
        return false;
    }
    if (!IsPointInside(rTest.mBoundCenter, fTestRadius))
    {
        return false;
    }

    PathFinder::AddCost();

    PblVector3 vertex[4];
    for (int i = 0; i < 4; i++)
    {
        vertex[i] = GetVertexXZ(i, fRadius, 0.0f);
    }

    float fBest = 1e10f;
    PblVector3 src = rTest.mSrc;
    PblVector3 dir(rTest.mDest.x - src.x, 0.0f, rTest.mDest.z - src.z);
    for (int i = 0; i < 4; i++)
    {
        PblVector3 a = vertex[i];
        int j = (i + 1) & 3;
        float fEdgeX = vertex[j].x - a.x;
        float fEdgeZ = vertex[j].z - a.z;
        float fDet = fEdgeZ * dir.x - dir.z * fEdgeX;
        if ((float)fabs(fDet) > EPSILON)
        {
            float fInvDet = 1.0f / fDet;
            float fWZ = a.z - src.z;
            float fWX = a.x - src.x;
            float fT = (fEdgeZ * fWX - fWZ * fEdgeX) * fInvDet;
            float fU = (dir.z * fWX - fWZ * dir.x) * fInvDet;
            if (fT >= 0.0f && fT <= 1.0f && fU >= 0.0f && fU <= 1.0f && fT < fBest)
            {
                fBest = fT;
                *pNormal = PblVector3(-fEdgeZ, 0.0f, fEdgeX);
                if (pVertex0)
                {
                    *pVertex0 = vertex[i];
                }
                if (pVertex1)
                {
                    *pVertex1 = vertex[j];
                }
            }
        }
    }

    if (fBest >= 1e10f)
    {
        return false;
    }

    *pDistSq = (dir.x * dir.x + dir.y * dir.y + dir.z * dir.z) * fBest * fBest;
    PblVector3 n;
    D3DXVec3Normalize(&n, pNormal);
    *pNormal = n;
    return true;
}

// FUNCTION: SWBF2 0x00610800
void Obstacle::FindCorners(const PblVector3& rSrc, const PblVector3& rDest, float fRadius,
                           PblVector3* pLeft, PblVector3* pRight) const
{
    PblVector3 dir(rDest.x - rSrc.x, 0.0f, rDest.z - rSrc.z);
    if (dir.x * dir.x + dir.y * dir.y + dir.z * dir.z < EPSILON * EPSILON)
    {
        dir = PblVector3(1.0f, 0.0f, 0.0f);
    }
    PblVector3 dirN;
    D3DXVec3Normalize(&dirN, &dir);

    float fBestLeft = 1.0f;
    float fBestRight = 1.0f;
    for (int i = 0; i < 4; i++)
    {
        PblVector3 vertex = GetPushedOutVertexXZ(i, fRadius);
        PblVector3 d(vertex.x - rSrc.x, 0.0f, vertex.z - rSrc.z);
        if (d.x * d.x + d.y * d.y + d.z * d.z > EPSILON * EPSILON)
        {
            PblVector3 n;
            D3DXVec3Normalize(&n, &d);
            float fDot = dirN.x * n.x + dirN.y * n.y + dirN.z * n.z;
            float fCross = dirN.z * n.x - dirN.x * n.z;
            if (fCross < 0.0f && fDot < fBestLeft)
            {
                *pLeft = vertex;
                fBestLeft = fDot;
            }
            else if (fCross > 0.0f && fDot < fBestRight)
            {
                *pRight = vertex;
                fBestRight = fDot;
            }
        }
    }
}

// FUNCTION: SWBF2 0x006109b0
bool Obstacle::Prohibits(int eProhibit) const
{
    if (!mIsEnabled && PathFinder::mRespectEnabled)
    {
        return false;
    }
    return (mProhibitFlags & eProhibit) != 0;
}

// FUNCTION: SWBF2 0x006109e0
void Obstacle::ToggleEnabled()
{
    mIsEnabled = !mIsEnabled;
}

// FUNCTION: SWBF2 0x00610a00
void Obstacle::SetEnabled(bool bEnabled)
{
    mIsEnabled = bEnabled;
}

// FUNCTION: SWBF2 0x00610a20
void PathFinder::ResetCost()
{
    sCost = 0;
}

// FUNCTION: SWBF2 0x00610a30
int PathFinder::GetCost()
{
    return sCost;
}

// FUNCTION: SWBF2 0x00610a40
void PathFinder::AddCost()
{
    ++sCost;
}

// FUNCTION: SWBF2 0x00610a50
void PathFinder::Initialize()
{
    PathNode::SetupMemoryPool(1024, PathNode::sMemoryPool.GetSize(), PathNode::sMemoryPool.GetGrow());
    Obstacle::SetupMemoryPool(512, Obstacle::sMemoryPool.GetSize(), Obstacle::sMemoryPool.GetGrow());

    sOpenList = new PblHeap<PathNode*, float>(512, HEAP_MAX_VALUE, 0);
    sSearchedPos = new ListPool<PblVector3, int>(100);
    sObstacles = new TreeGrid(PblVector3((float)-WORLD_EXTENTS, 0.0f, (float)-WORLD_EXTENTS),
                              PblVector3((float)WORLD_EXTENTS, 0.0f, (float)WORLD_EXTENTS), 16.0f);
    sClosedList = new PblList<PathNode>;
    sFoundDest = false;
    sComplete = true;
}

// FUNCTION: SWBF2 0x00610c30
void PathFinder::Reset()
{
    sOpenList = 0;
    sSearchedPos = 0;
    sClosedList = 0;
    sObstacles = 0;
    PathNode::sMemoryPool.Reset();
    Obstacle::sMemoryPool.Reset();
}

// FUNCTION: SWBF2 0x00610c70
void PathFinder::AddObstacle(const PblMatrix& rMatrix, const PblVector4& rSize, int eProhibit,
                             unsigned int uName)
{
    Obstacle* pObstacle = new Obstacle(rMatrix, rSize, eProhibit, uName);
    const PblVector3& rPos = *(const PblVector3*)rMatrix.m[3];
    pObstacle->SetCollisionCenter(rPos);
    pObstacle->SetCollisionRadius(rSize.w);
    pObstacle->Set2DCollisionRadius(rSize.w);
    sObstacles->Add(pObstacle, rPos, rSize.w * 2.0f);
}

class EntityGeometry : public EntityEx, public CollisionObject
{
};
class GameObject : public EntityGeometry
{
};
 

// FUNCTION: SWBF2 0x00610d20
void PathFinder::CreateGameObjectBarrier(Obstacle*& rpObstacle, GameObject* pObject,
                                         float fSizeX, float fSizeZ,
                                         float fOffsetX, float fOffsetZ, unsigned int uName)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    if (rpObstacle)
    {
        delete rpObstacle;
    }
    CollisionObject* pCollision = pObject;
    if (fSizeX <= 0.0f)
    {
        fSizeX = pCollision->m2dRadius;
    }
    if (fSizeZ <= 0.0f)
    {
        fSizeZ = pCollision->m2dRadius;
    }
    PblVector4 size;
    size.x = fSizeX;
    size.y = 0.0f;
    size.z = fSizeZ;
    size.w = 0.0f;
    PblMatrix mat = pCollision->GetMatrix();
    PblVector3 xAxis = AIUtil::GetSafeNormalXZ(*(const PblVector3*)mat.m[0]);
    PblVector3 zAxis = AIUtil::GetSafeNormalXZ(*(const PblVector3*)mat.m[2]);
    mat.m[3][0] += zAxis.x * fOffsetZ + xAxis.x * fOffsetX;
    mat.m[3][1] += zAxis.y * fOffsetZ + xAxis.y * fOffsetX;
    mat.m[3][2] += zAxis.z * fOffsetZ + xAxis.z * fOffsetX;
    rpObstacle = new Obstacle(mat, size, ~32, uName);
    sObstacles->Add(rpObstacle, rpObstacle->GetBoundingCircleCenter(),
                    rpObstacle->GetBoundingCircleRadius() * 2.0f);
}

// FUNCTION: SWBF2 0x00610f20
void PathFinder::DeleteGameObjectBarrier(Obstacle*& rpObstacle)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    if (rpObstacle)
    {
        sObstacles->Remove(rpObstacle);
        delete rpObstacle;
        rpObstacle = 0;
    }
}

// FUNCTION: SWBF2 0x00610f80
void PathFinder::MoveBarrier(Obstacle* pObstacle)
{
    sObstacles->Update(pObstacle, pObstacle->GetBoundingCircleCenter(),
                       pObstacle->GetBoundingCircleRadius() * 2.0f);
}

// FUNCTION: SWBF2 0x00610fd0
bool PathFinder::PointValidCallback(const TreeGridObject* pObj, TreeGridCallbackData* pData, float&)
{
    PointValidData* pValid = (PointValidData*)pData;
    const Obstacle* pObstacle = (const Obstacle*)pObj;
    for (int i = 0; i < pValid->mIgnoreCnt; i++)
    {
        if (pObstacle == pValid->mIgnore[i])
        {
            return true;
        }
    }
    if (pObstacle->Prohibits(pValid->mProhibit) && pObstacle->IsPointInside(pValid->mPos, pValid->mRadius))
    {
        pValid->hit = true;
        pValid->hitObs = (Obstacle*)pObstacle;
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x00611030
bool PathFinder::IsPointValid(const PblVector3& rPos, float fRadius, int eProhibit,
                              const Obstacle** ppHit, const Obstacle** ppIgnore, int iIgnoreCnt)
{
    PointValidData data;
    data.mPos = rPos;
    data.mRadius = fRadius;
    data.mProhibit = eProhibit;
    PblVector3 min(rPos.x - fRadius, -FLT_MAX, rPos.z - fRadius);
    PblVector3 max(rPos.x + fRadius, FLT_MAX, rPos.z + fRadius);
    memcpy(data.mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));
    data.mIgnoreCnt = iIgnoreCnt;
    sObstacles->CallbackOnRect(min, max, PointValidCallback, &data);
    if (ppHit)
    {
        *ppHit = data.hitObs;
    }
    return !data.hit;
}

// FUNCTION: SWBF2 0x00611100
bool PathFinder::IsPointValid(const PblVector3& rPos, const Obstacle** ppHit, const GameObject* pObj)
{
    float fRadius = AIUtil::GetCollisionRadius(pObj);
    int eProhibit = AIUtil::GetAIType(pObj);
    const Obstacle* pIgnore = AIUtil::GetGameObjectBarrier(pObj);
    return IsPointValid(rPos, fRadius, eProhibit, ppHit, &pIgnore, 1);
}

// FUNCTION: SWBF2 0x00611150
bool PathFinder::PushValidPointCallback(const TreeGridObject* pObj, TreeGridCallbackData* pData, float&)
{
    PushValidPointData* pPush = (PushValidPointData*)pData;
    const Obstacle* pObstacle = (const Obstacle*)pObj;
    for (int i = 0; i < pPush->mIgnoreCnt; i++)
    {
        if (pPush->mIgnore[i] == pObstacle)
        {
            return true;
        }
    }
    if (pObstacle->Prohibits(pPush->mProhibit))
    {
        float fDistSq = FLT_MAX;
        if (pObstacle->IntersectionNewer(pPush->mObsTest, pPush->mRadius, &fDistSq, &pPush->mNormal, 0, 0))
        {
            float fDist = (float)sqrt(fDistSq);
            const PblVector3& rSrc = pPush->mObsTest.mSrc;
            ObstacleTest test;
            test.mSrc = rSrc;
            test.mDest = PblVector3(rSrc.x + pPush->mRayDir.x * fDist,
                                    rSrc.y + pPush->mRayDir.y * fDist,
                                    rSrc.z + pPush->mRayDir.z * fDist);
            test.mBoundCenter = PblVector3((test.mSrc.x + test.mDest.x) * 0.5f,
                                           (test.mSrc.y + test.mDest.y) * 0.5f,
                                           (test.mSrc.z + test.mDest.z) * 0.5f);
            float fDX = test.mBoundCenter.x - test.mSrc.x;
            float fDY = test.mBoundCenter.y - test.mSrc.y;
            float fDZ = test.mBoundCenter.z - test.mSrc.z;
            test.mBoundRadius = sqrtf(fDX * fDX + fDY * fDY + fDZ * fDZ);
            pPush->mTruncated = true;
            pPush->mObsTest = test;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x006112f0
bool PathFinder::PushValidPoint(const PblVector3& rSrc, PblVector3& rDest, PblVector3& rNormal,
                                float fRadius, int eProhibit, const Obstacle** ppIgnore,
                                int iIgnoreCnt)
{
    PushValidPointData data;
    data.mObsTest.mSrc = rSrc;
    data.mObsTest.mDest = rDest;
    data.mObsTest.mBoundCenter = PblVector3((rDest.x + rSrc.x) * 0.5f,
                                            (rDest.y + rSrc.y) * 0.5f,
                                            (rDest.z + rSrc.z) * 0.5f);
    float fDX = data.mObsTest.mBoundCenter.x - rSrc.x;
    float fDY = data.mObsTest.mBoundCenter.y - rSrc.y;
    float fDZ = data.mObsTest.mBoundCenter.z - rSrc.z;
    data.mObsTest.mBoundRadius = sqrtf(fDX * fDX + fDY * fDY + fDZ * fDZ);

    PblVector3 dir(rDest.x - rSrc.x, rDest.y - rSrc.y, rDest.z - rSrc.z);
    PblVector3 dirN;
    D3DXVec3Normalize(&dirN, &dir);
    data.mRayDir = dirN;
    data.mRadius = fRadius;
    data.mProhibit = eProhibit;
    memcpy(data.mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));
    data.mIgnoreCnt = iIgnoreCnt;

    sObstacles->CallbackOnFatRay(rSrc, rDest, fRadius * 2.0f, PushValidPointCallback, &data);

    rDest = data.mObsTest.mDest;
    rNormal = data.mNormal;
    return data.mTruncated;
}

// FUNCTION: SWBF2 0x00611500
bool PathFinder::PushValidPoint(const PblVector3& rSrc, PblVector3& rDest, const GameObject* pObj)
{
    PblVector3 normal;
    float fRadius = AIUtil::GetCollisionRadius(pObj);
    int eProhibit = AIUtil::GetAIType(pObj);
    const Obstacle* pIgnore = AIUtil::GetGameObjectBarrier(pObj);
    return PushValidPoint(rSrc, rDest, normal, fRadius, eProhibit, &pIgnore, 1);
}

// FUNCTION: SWBF2 0x00611560
bool PathFinder::GetClosestValidPoint(const PblVector3& rPos, PblVector3& rOut, const GameObject* pObj)
{
    float fRadius = AIUtil::GetCollisionRadius(pObj);
    int eProhibit = AIUtil::GetAIType(pObj);
    const Obstacle* pIgnore = AIUtil::GetGameObjectBarrier(pObj);
    return GetClosestValidPoint(rPos, rOut, fRadius, eProhibit, &pIgnore, 1);
}

// FUNCTION: SWBF2 0x006115b0
bool PathFinder::GetClosestValidPoint(const PblVector3& rPos, PblVector3& rOut, float fRadius,
                                      int eProhibit, const Obstacle** ppIgnore, int iIgnoreCnt)
{
    const Obstacle* pHit = 0;
    if (IsPointValid(rPos, fRadius, eProhibit, &pHit, ppIgnore, iIgnoreCnt))
    {
        return true;
    }

    const Obstacle* pObstacle = pHit;
    PblVector3 side[4];
    float fDistSq[4];
    for (int i = 0; i < 4; i++)
    {
        fDistSq[i] = pObstacle->GetDistFromSideXZ(rPos, fRadius, i, side[i]);
    }

    for (int iTry = 0; iTry < 4; iTry++)
    {
        int iBest = 0;
        float fBest = FLT_MAX;
        PblVector3 best;
        for (int i = 0; i < 4; i++)
        {
            if (fDistSq[i] < fBest)
            {
                fBest = fDistSq[i];
                iBest = i;
                best = side[i];
            }
        }
        if (IsPointValid(best, fRadius, eProhibit, &pHit, ppIgnore, iIgnoreCnt))
        {
            rOut.x = best.x;
            rOut.z = best.z;
            return true;
        }
        fDistSq[iBest] = FLT_MAX;
    }
    return false;
}

// FUNCTION: SWBF2 0x00611750
float PathFinder::sInvertedTotalCost(const PathNode* pNode)
{
    const PblVector3& rPos = pNode->GetPos();
    float fDX = sDestPos.x - rPos.x;
    float fDZ = sDestPos.z - rPos.z;
    float fCost = sqrtf(fDX * fDX + fDZ * fDZ) + pNode->GetCostSoFar() + 0.01f;
    if (fCost < HEAP_MAX_VALUE)
    {
        return HEAP_MAX_VALUE - fCost;
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x006117c0
bool PathFinder::GenerateObstacleListCallback(const TreeGridObject* pObj, TreeGridCallbackData* pData,
                                              float&)
{
    ObstacleData* pList = (ObstacleData*)pData;
    if (pList->mLength < 300)
    {
        pList->mArray[pList->mLength] = (Obstacle*)pObj;
        pList->mLength++;
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x006117f0
bool PathFinder::FindClosestObstacleIntersection(const PblVector3& rSrc, const PblVector3& rDest,
                                                 float fRadius, int eProhibit, const Obstacle** ppHit,
                                                 float* pDistSq, bool, const Obstacle** ppIgnore,
                                                 int iIgnoreCnt)
{
    bool bOldRespect = mRespectEnabled;
    mRespectEnabled = true;
    bool bHit = false;

    PblVector3 min((rSrc.x < rDest.x ? rSrc.x : rDest.x) - fRadius, -FLT_MAX,
                   (rSrc.z < rDest.z ? rSrc.z : rDest.z) - fRadius);
    PblVector3 max((rSrc.x > rDest.x ? rSrc.x : rDest.x) + fRadius, FLT_MAX,
                   (rSrc.z > rDest.z ? rSrc.z : rDest.z) + fRadius);
    static ObstacleData data;
    data.mLength = 0;
    sObstacles->CallbackOnRect(min, max, GenerateObstacleListCallback, &data);

    ObstacleTest test;
    test.mSrc = rSrc;
    test.mDest = rDest;
    test.mBoundCenter = PblVector3((rDest.x + rSrc.x) * 0.5f, (rDest.y + rSrc.y) * 0.5f,
                                   (rDest.z + rSrc.z) * 0.5f);
    float fDX = test.mBoundCenter.x - rSrc.x;
    float fDY = test.mBoundCenter.y - rSrc.y;
    float fDZ = test.mBoundCenter.z - rSrc.z;
    test.mBoundRadius = sqrtf(fDX * fDX + fDY * fDY + fDZ * fDZ);

    *pDistSq = FLT_MAX;
    for (int i = 0; i < data.mLength; i++)
    {
        const Obstacle* pObstacle = data.mArray[i];
        int j;
        for (j = 0; j < iIgnoreCnt; j++)
        {
            if (ppIgnore[j] == pObstacle)
            {
                break;
            }
        }
        if (j < iIgnoreCnt)
        {
            continue;
        }
        if (pObstacle->Prohibits(eProhibit))
        {
            float fDistSq = *pDistSq;
            PblVector3 normal;
            if (pObstacle->IntersectionNewer(test, fRadius, &fDistSq, &normal, 0, 0))
            {
                bHit = true;
                if (fDistSq < *pDistSq)
                {
                    *pDistSq = fDistSq;
                    *ppHit = pObstacle;
                }
            }
        }
    }
    mRespectEnabled = bOldRespect;
    return bHit;
}

// FUNCTION: SWBF2 0x00611a30
bool PathFinder::TrivialIntersectCallback(const TreeGridObject* pObj, TreeGridCallbackData* pData, float&)
{
    TrivialIntersectData* pTrivial = (TrivialIntersectData*)pData;
    const Obstacle* pObstacle = (const Obstacle*)pObj;
    if (pTrivial->mOnlyDynamic && pObstacle->mNameHash != 0)
    {
        return true;
    }
    for (int i = 0; i < pTrivial->mIgnoreCnt; i++)
    {
        if (pTrivial->mIgnore[i] == pObstacle)
        {
            return true;
        }
    }
    if (pObstacle->Prohibits(pTrivial->mProhibit))
    {
        float fDistSq = FLT_MAX;
        PblVector3 normal;
        if (pObstacle->IntersectionNewer(pTrivial->mObsTest, pTrivial->mRadiusVehicle, &fDistSq, &normal, 0, 0))
        {
            pTrivial->hit = true;
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00611ab0
bool PathFinder::TestTrivialStraightLineHit(const PblVector3& from, const PblVector3& to,
                                            const GameObject* pObj)
{
    float fRadius = AIUtil::GetCollisionRadius(pObj);
    int eProhibit = AIUtil::GetAIType(pObj);
    const Obstacle* pIgnore = AIUtil::GetGameObjectBarrier(pObj);
    return TestTrivialStraightLineHit(from, to, fRadius, eProhibit, &pIgnore, 1, false);
}

// FUNCTION: SWBF2 0x00611b10
bool PathFinder::TestTrivialStraightLineHit(const PblVector3& from, const PblVector3& to, float fRadius,
                                            int eProhibit, const Obstacle* const* ppIgnore, int iIgnoreCnt,
                                            bool bOnlyDynamic)
{
    float fDX = from.x - to.x;
    float fDY = from.y - to.y;
    float fDZ = from.z - to.z;
    if (fDX * fDX + fDY * fDY + fDZ * fDZ < EPSILON)
    {
        return false;
    }

    PblVector3 min((to.x < from.x ? to.x : from.x) - fRadius, -FLT_MAX,
                   (to.z < from.z ? to.z : from.z) - fRadius);
    PblVector3 max((to.x > from.x ? to.x : from.x) + fRadius, FLT_MAX,
                   (to.z > from.z ? to.z : from.z) + fRadius);
    static TrivialIntersectData data;

    ObstacleTest test;
    test.mSrc = from;
    test.mDest = to;
    test.mBoundCenter = PblVector3((to.x + from.x) * 0.5f, (to.y + from.y) * 0.5f,
                                   (to.z + from.z) * 0.5f);
    float fCX = test.mBoundCenter.x - from.x;
    float fCY = test.mBoundCenter.y - from.y;
    float fCZ = test.mBoundCenter.z - from.z;
    test.mBoundRadius = sqrtf(fCX * fCX + fCY * fCY + fCZ * fCZ);

    data.mObsTest = test;
    data.mProhibit = eProhibit;
    data.mRadiusVehicle = fRadius;
    memcpy(data.mIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));
    data.mOnlyDynamic = bOnlyDynamic;
    data.mIgnoreCnt = iIgnoreCnt;
    data.hit = false;
    sObstacles->CallbackOnRect(min, max, TrivialIntersectCallback, &data);
    return data.hit;
}

// FUNCTION: SWBF2 0x00611d20
bool PathFinder::sSamePosXZ(const PblVector3& a, const PblVector3& b)
{
    float fDX = a.x - b.x;
    float fDZ = a.z - b.z;
    return fDX * fDX + fDZ * fDZ < EPSILON * EPSILON;
}

// FUNCTION: SWBF2 0x00611d50
void PathFinder::sExpandNode(PathFinder::PathNode* pParent, const PblVector3& rPos, int iDepth)
{
    using namespace PathFinder;

    if (iDepth >= 32)
    {
        return;
    }

    float fPerp = (rPos.x - sDestPos.x) * sPerpVec.x + (rPos.y - sDestPos.y) * sPerpVec.y +
                  (rPos.z - sDestPos.z) * sPerpVec.z;
    if ((float)fabs(fPerp) > sPerpLength)
    {
        return;
    }

    for (int i = 0; i < sSearchedPos->GetLength(); i++)
    {
        if (sSamePosXZ(rPos, (*sSearchedPos)[i]))
        {
            return;
        }
    }
    sSearchedPos->Append(rPos);

    const Obstacle* pHit = 0;
    float fDistSq = -1.0f;

    if (FindClosestObstacleIntersection(pParent->GetPos(), rPos, sRadiusVehicle, sProhibit, &pHit,
                                        &fDistSq, false, (const Obstacle**)sIgnore, sIgnoreCnt))
    {
        PblVector3 vLeft;
        PblVector3 vRight;
        pHit->FindCorners(pParent->GetPos(), rPos, sRadiusVehicle, &vLeft, &vRight);
        PathFinder::sExpandNode(pParent, vLeft, iDepth + 1);
        PathFinder::sExpandNode(pParent, vRight, iDepth + 1);
        return;
    }

    float fDX = pParent->GetPos().x - rPos.x;
    float fDZ = pParent->GetPos().z - rPos.z;
    float fDist = sqrtf(fDZ * fDZ + fDX * fDX);
    float fCost = pParent->GetCostSoFar() + fDist;

    for (int i = 0; i < sOpenList->GetCount(); i++)
    {
        PathNode* pNode = sOpenList->Get(i);
        if (sSamePosXZ(rPos, pNode->GetPos()))
        {
            if (pNode->GetCostSoFar() > fCost)
            {
                pNode->Set(rPos, pParent->GetCostSoFar() + fDist, pParent);
                sOpenList->ReplaceLowest(pNode, sInvertedTotalCost(pNode), i);
            }
            return;
        }
    }

    for (PblList<PathNode>::Itor it(*sClosedList); (PathNode*)it; ++it)
    {
        PathNode* pNode = it;
        if (sSamePosXZ(rPos, pNode->GetPos()))
        {
            if (pNode->GetCostSoFar() > fCost)
            {
                pNode->Set(rPos, pParent->GetCostSoFar() + fDist, pParent);
            }
            return;
        }
    }

    float fNewDX = rPos.x - pParent->GetPos().x;
    float fNewDZ = rPos.z - pParent->GetPos().z;
    PathNode* pNode = new PathNode(rPos, pParent->GetCostSoFar() + sqrtf(fNewDZ * fNewDZ + fNewDX * fNewDX),
                                   pParent);
    if (pNode)
    {
        sOpenList->Push(pNode, sInvertedTotalCost(pNode));
    }
}

// FUNCTION: SWBF2 0x006120a0
void PathFinder::NewPathFind(const PblVector3& vStart, const PblVector3& vGoal, float fRadius,
                             int eProhibit, const Obstacle** ppIgnore, int iIgnoreCnt)
{
    if (!IsPointValid(vGoal, fRadius, eProhibit, 0, ppIgnore, iIgnoreCnt))
    {
        sComplete = true;
        sFoundDest = false;
        return;
    }

    while (!sOpenList->IsEmpty())
    {
        PathNode* pNode = sOpenList->Get(0);
        sOpenList->Pop();
        delete pNode;
    }
    for (PblList<PathNode>::Itor it(*sClosedList); (PathNode*)it;)
    {
        PathNode* pNode = it;
        ++it;
        sClosedList->Remove(&pNode->mNode);
        delete pNode;
    }

    sSrcPos = vStart;
    sDestPos = vGoal;
    sProhibit = eProhibit;
    sIgnoreCnt = iIgnoreCnt;
    sRadiusVehicle = fRadius;
    memcpy(sIgnore, ppIgnore, iIgnoreCnt * sizeof(Obstacle*));

    PblVector3 vDir(vGoal.x - vStart.x, 0.0f, vGoal.z - vStart.z);
    sComplete = false;
    sFoundDest = false;
    sPausedAtFinish = 0;
    sPerpLength = sqrtf(vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z);
    float fInv = 1.0f / sPerpLength;
    sPerpVec = PblVector3(vDir.z * fInv, 0.0f, -(vDir.x * fInv));
    if (sPerpLength <= 25.0f)
    {
        sPerpLength = 25.0f;
    }

    if (!AIPath::sMemoryPool.IsFull() && AIPathNode::sMemoryPool.IsSufficientSpace(32) &&
        !PathNode::sMemoryPool.IsFull())
    {
        PathNode* pNode = new PathNode(vStart, 0.0f, 0);
        if (pNode)
        {
            sOpenList->Push(pNode, sInvertedTotalCost(pNode));
        }
    }
}

// FUNCTION: SWBF2 0x00612320
bool PathFinder::IsCompleted()
{
    return sComplete;
}

// FUNCTION: SWBF2 0x00612330
bool PathFinder::GetPausedAtFinish()
{
    return sPausedAtFinish > 0;
}

// FUNCTION: SWBF2 0x00612340
void PathFinder::UpdatePathFind()
{
    if (sPausedAtFinish > 0)
    {
        if (--sPausedAtFinish <= 0)
        {
            sComplete = true;
        }
        return;
    }

    while (!sOpenList->IsEmpty())
    {
        if (!PathManager::IsWithinTimeConstraint())
        {
            if (!sOpenList->IsEmpty())
            {
                return;
            }
            break;
        }

        sSearchedPos->Clear();
        PathNode* pNode = sOpenList->Get(0);
        if (sSamePosXZ(pNode->GetPos(), sDestPos))
        {
            sFoundDest = true;
            break;
        }

        sOpenList->Pop();
        sClosedList->Add(&pNode->mNode, pNode);
        int iDepth = 0;
        for (const PathNode* p = pNode; p; p = p->mParentNode)
        {
            iDepth++;
        }
        PathFinder::sExpandNode(pNode, sDestPos, iDepth);
    }

    if (AIDebug::PauseAfterPathFind())
    {
        sPausedAtFinish = 2;
        return;
    }
    sComplete = true;
}

// FUNCTION: SWBF2 0x00612430
void PathFinder::CancelPathFind()
{
    sComplete = true;
}

// FUNCTION: SWBF2 0x00612440
AIPath* PathFinder::GetPathResult()
{
    if (!sFoundDest)
    {
        ++gPathResultFailureCount;
        return 0;
    }

    AIPath* pPath = new AIPath;
    if (pPath)
    {
        float fLastY = 0.0f;
        for (const PathNode* pNode = sOpenList->Get(0); pNode; pNode = pNode->mParentNode)
        {
            PblVector3 vPos = pNode->GetPos();
            if (fabs(vPos.y) < EPSILON)
            {
                vPos.y = fLastY;
            }
            else
            {
                fLastY = vPos.y;
            }
            pPath->Prepend(vPos, 0);
        }
    }
    return pPath;
}

// FUNCTION: SWBF2 0x00438690
bool AIDebug::PauseAfterPathFind()
{
    return false;
}
