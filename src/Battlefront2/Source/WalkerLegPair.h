#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblBitVector.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblQuaternion.h"

class CollisionResult;
class EntityWalker;
class MemoryPool;
class WalkerBlend;
class NetPktGroup;

struct WalkerLegPairInfo
{

    WalkerLegPairInfo();

    int mFootIndices[2];
    int mFootCollisionBody[2];
    int mLegJoints[2];
    PblBitVector<64> mLegMasks[2];
    PblBitVector<64> mLegTopMasks[2];
};
 

class WalkerLegPair
{
public:
    enum eFoot
    {
        Left = 0,
        Right = 1,
        FootCount = 2
    };

    WalkerLegPair(EntityWalker* pParent, const WalkerLegPairInfo& info);

    ~WalkerLegPair();
    void InitDone();
    bool FootTerrainCollision(const CollisionResult* pResult);
    void SetTerrainHeight(int iIndex, float fHeight);
    void SetupGroundCollision(const PblMatrix& mMatrix, float fPitch, const PblVector3* const pFeet);

    float GetStateLimit();
    void SetFrameState(float fState);
    void ResetTurnState();

    void SetInterpolate(bool bInterpolate);

    void BuildBlendLookupTable();
    static void* operator new(unsigned int uSize);
    static void operator delete(void* pBlock);

    void Update(float fDt);
    void ApplyBlend();
    void CorrectJoints();
    void RestoreJoints();
    void UpdateDone();
    void StateChanged(int iState);

    WalkerBlend* GetBlend() { return mBlend; }
    float GetTurnState();

private:
    void AdjustLeg(eFoot eWhich, const PblQuaternion& kAdjust);
    void AdjustBlending();

public:

    float mLocalZOffset;
    float mFootIntersection[2];
    float mFootHeight[2];
    EntityWalker* mParent;
    WalkerBlend* mBlend;
    PblBitVector<64> mMainMask;
    PblBitVector<64> mTopMask;
    bool mFootContact[2];
    eFoot mIdleBlendFoot;
    float mLastIdleBlend;
    int mFootIndices[2];
    int mFootCollisionBody[2];
    float mFootTerrainHeight[2];
    float mBlendLookupTable[9];
    int mLegJoints[2];
    PblMatrix mLegTransform[2];
    PblQuaternion mLegAdjustments[2];
    float mBlendFactor;
    float mPushValue;
    float mFootHeightDifference;

    static MemoryPool sMemoryPool;

public:

    void Write(NetPktGroup& group);
    void Read(NetPktGroup& group);
};
 
 
 
 
 
 
