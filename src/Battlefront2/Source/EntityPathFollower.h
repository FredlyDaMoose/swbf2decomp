#pragma once

#include <stddef.h>

#include "EntitySquadron.h"
#include "PebbleFL/Common/PblCatmullRom.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblListDouble.h"
#include "PebbleFL/Common/PblListDoubleSorted.h"
#include "RedEngineFL/Graphics/RedColor.h"

class Character;
class EntityPath;
class GameObject;
class NetObjState;
class NetPktGroup;
class PblMatrix;

class EntityPathFollowerClass : public EntitySquadronClass
{
public:
    EntityPathFollowerClass();

    bool SetProperty(unsigned int uProperty, const char* pszValue);

    static const ODFPropertySet::ODFProperty sPropertySet[15];

    struct
    {
        float mRollAcceleration;
        float mRollLookAhead;
        float mRollTolerance;
        float mRollSpeed;
        float mRollAccelResp;
        float mRollTurnFactor;
        float mRollSpeedFactor;
        float mRollDampening;
    } mPathRollValues;

    unsigned int mEntityClassID;
    unsigned int mEntityClassID_CTF;
    float mSpeed;
    float mTurnSpeed;
    float mAcceleration;
    unsigned int mBranchPaths;
    float mBranchRange;
    float mCosMaxAngle;
    float mMinBranchDist;
    bool mbFlyerVsFlyer;
};
 
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x0079c460
class EntityPathFollower : public EntitySquadron, public PblList<EntityPathFollower>::Node
{
public:

    class BranchParams
    {
    public:
        BranchParams(const EntityPathFollowerClass* pPathClass, PblMatrix& matrix, float fDiameter,
                     GameObject* const pGameObject, float fWorldSpeed, float fRange,
                     unsigned int uBranchPaths, float fCosMaxAngle, bool bOverrideBranching,
                     bool bFallbackToClosest);

        const EntityPathFollowerClass* mPathClass;
        PblMatrix& mMatrix;
        float mDiameter;
        GameObject* mGameObject;
        float mWorldSpeed;
        float mSquareRange;
        unsigned int mBranchPaths;
        float mCosMaxAngle;
        bool mOverrideBranching;
        bool mFallbackToClosest;
    };

    class PathAttributes
    {
    public:

        PathAttributes() {}
        static void* operator new(size_t, void* pMemory) { return pMemory; }
        static void operator delete(void*, void*) {}

        const EntityPath* mPath;
        int mPointIndex;
        float mSquareDist;
        PblVector3 mVectorDist;
    };

    class PathStart
    {
    public:

        static void* operator new(size_t, void* pMemory) { return pMemory; }
        static void operator delete(void*, void*) {}
        PathStart& operator=(const PathStart& other)
        {
            mNode.mSortValue = other.mNode.mSortValue;
            mAttrib = other.mAttrib;
            mDirection = other.mDirection;
            return *this;
        }

        PblListDoubleSorted::Node mNode;
        PathAttributes mAttrib;
        char mDirection;
    };

    EntityPathFollower(const EntityPathFollowerClass* const pClass);
    virtual ~EntityPathFollower();

    virtual unsigned int GetAllegiance();
    virtual bool CanJoinSquadron() const;

    virtual void WriteObject(NetPktGroup& group, NetObjState* pState, float f1, float f2);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState, float f1, float f2);
    virtual BranchParams GetBranchParams() = 0;

    bool MoveTo(const PathStart& start, const PblVector3* const pPosition,
                const PblVector3* const pDirection, BranchParams& params, bool bUseCTFClass,
                bool b2, bool b3);
    void Reset(const EntityPathFollowerClass* const pClass);
    void ResetStateVars();
    void RemoveFromSquadron();

    unsigned int CreateSquadronForFlyby(BranchParams& params);
    unsigned int CreateSquadronFromClosestEntities(const PblVector3& vPosition, const PblVector3& vDirection,
                                                   float fSquareRange, float fCosMaxAngle);
    bool Branch(BranchParams& params, const PblVector3* const pPosition, bool bHavePosition);

    bool Update(BranchParams& params, float fElapsed, PblVector3& vVelocity, PblMatrix* const pMatrix, bool b,
                float fSpeedLimit, float fSpeedHold);
    bool CirclePoint(float fRadius);

    static void FindPath(const PblVector3* const pPosition, const PblVector3* const pDirection, float fRange,
                         unsigned int uBranchPaths, float fCosMaxAngle, float fMinBranchDist,
                         unsigned int uClassID, const PblVector3* const pExclude,
                         const EntityPath* const pExcludePath, const GameObject* const pIgnore,
                         bool bFallbackToClosest, PathStart& start);
    static bool RayTestSpline(const PblVector3& p0, const PblVector3& p1, const PblVector3& p2,
                              const PblVector3& p3, const GameObject* const pIgnore, int iFilter);

protected:
    void RemovePathRef(const EntityPathFollowerClass* const pClass);
    bool HeadTowards(PblVector3& vTarget, float fDistance, BranchParams& params);
    void SmoothPath(BranchParams& params, const PblVector3* pTarget, float fMinOffset,
                    float fMaxOffset, float fDistRange, float fForwardScale, float fCosAngle);
    void DelayPath(BranchParams& params, float fDistance);
    void EndOfSegment(BranchParams& params, const PblVector3* const pPosition, bool bForceBranch,
                      bool bSetupNext);

    void SetSquadronLeader(const EntityPathFollowerClass* const pClass, EntityPathFollower* pLeader);
    bool SetupNextSegment(const PblVector3* const pPosition, BranchParams& params, bool b1, bool b2);
    void SetPoint(const PblVector3& vPoint, unsigned char uIndex);
    const PblVector3& GetPoint(unsigned char uIndex) const;
    bool GetPointIsSet(unsigned char uIndex) const;
    static bool IsFlyerPlayerCentric(BranchParams& params);

    void UpdateSquadron(const EntityPathFollowerClass* const pClass, const PblMatrix& matrix, float fElapsed,
                        const PblVector3& vPosition, PblVector3& vPoint);
    bool ShotAtByEnemy(BranchParams& params);
    void PickupCTFFlag(BranchParams& params);

    const EntityPath* GetPath() const;

    void GetNextTowardsAttackerPoint(BranchParams& params, PblVector3& vPoint);

    float Rotate(const EntityPathFollowerClass* const pClass, PblMatrix& matrix, float fElapsed,
                 const PblVector3& vDirection, const PblVector3& vLookAhead, PblMatrix* const pMatrix);
    bool SavePoints(const PblVector3* const pPosition, BranchParams* const pParams, bool b);

    float Roll(const EntityPathFollowerClass* const pClass, float fYaw, float fElapsed, float fTurnRate);
    static bool CalculateDistance(const PblVector3& vPosition, const EntityPath* const pPath, unsigned int uIndex,
                                  float fMinDist, PathAttributes& attrib, float& fBestSquareDist);

public:
    PblListDouble::Node mNode;
    float mSpeed;
    float mOffPathSpeed;
    float mCurSpeed;
    float mAcceleration;
    float mYaw;
    float mSplineRoll;
    float mCurRoll;
    float mCurRollAcceleration;
    float mRollVelocity;
    float mStrafeTimer;
    float mT;
    PblCatmullRom mPathSpline;
    const EntityPath* mPath;
    int mIndex;
    char mDirection;
    unsigned char mSetPoints;
    Character* mAttacker;
    PblVector3 mPointToCircle;
    float mStaredownTimer;
    float mFireTimer;
    float mBoostTimer;
    bool mbTeleportNow;
    bool mbQueuePoint;
    bool mbGoMinSpeed;
    bool mbDelayIndexIncrement;
    bool mbCirclingPoint;
    bool mbLandNow;
    bool mbStraightLine;
    unsigned int mLastUsedID;

    static PblList<EntityPathFollower> sList;

    static void ClearList();
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
