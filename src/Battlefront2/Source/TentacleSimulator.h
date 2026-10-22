#pragma once

#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "PebbleFL/Common/PblVector.h"

class PblMatrix;
class RedPose;

class TentacleSimulator
{
public:

    TentacleSimulator(unsigned int uiNumTentacles, unsigned int uiBonesPerTentacle,
                      unsigned int uiCollType);

    static void Init();

    void UpdateTimer(float fDeltaTime);

    void DoTentacles(RedPose* pPose, const PblMatrix& mParent, const PblVector3& vVelocity,
                     PblMatrix* pOutMatrices, float fDeltaTime);

private:

    void UpdatePose(RedPose* pPose, const PblMatrix& mParent,
                    const PblMatrix* (*const pBones)[5], PblMatrix* pOutMatrices);

    void UpdatePositions(float fDeltaTime, const PblVector3& vVelocity, const RedPose* pPose,
                         const PblMatrix& mParent, const PblMatrix* (*const pBones)[5]);

    void EnforceCollisions(const RedPose* pPose, const PblMatrix& mParent);

    void EnforceSphereCollision(const PblMatrix& mMatrix, float fRadius);
    void EnforceCylinderCollision(const PblMatrix& mMatrix, float fA, float fB);
    void EnforceBoxCollision(const PblMatrix& mMatrix, float fX, float fY, float fZ);

public:
    PblVector3 tPos[4][6];
    PblVector3 oldPos[4][6];
    PblVector3 oldVelocity;
    float mInternalTimer;
    float mTimeSinceLastUpdate;
    float mTimerOffset;
    int mNumTentacles;
    int mBonesPerTentacle;
    int mCollType;
    bool mFirstUpdate;

    static MemoryPool sMemoryPool;

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }
};
 
