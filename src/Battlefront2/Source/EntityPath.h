#pragma once

#include <stddef.h>
#include "GamePath.h"
#include "PebbleFL/Common/PblListDouble.h"
#include "RedEngineFL/Util/RedRegion.h"

class EntityPath : public GamePath
{
public:

    // VTABLE: SWBF2 0x0079c440
    class BranchRegionFactory : public RedRegionFactory
    {
    public:

        BranchRegionFactory();

        // SYNTHETIC: SWBF2 0x004d0ed0
        // EntityPath::BranchRegionFactory::`scalar deleting destructor'

        virtual void* CreateRegion(RedRegionDesc& desc, const char* pcName);
    };

    class BranchRegion : public PblList<BranchRegion>::Node
    {
    public:

        BranchRegion(const PblVector3& vPosition, float fSize, unsigned int uHashID);
        ~BranchRegion();

        unsigned int GetHashID() const { return mHashID; }

        static const BranchRegion* FindByID(unsigned int uHashID);
        static void DestroyAll();

        PblVector3 mPosition;
        float mSize;
        unsigned int mHashID;

        static PblList<BranchRegion> sList;
    };

    class NodeProperties
    {
    public:
        union BranchInfo
        {
            float range;
            BranchRegion* branchRegion;
        };

        NodeProperties();

        const PblVector3& GetVariance() const { return mVariance; }
        float GetRange() const;
        const BranchRegion* GetBranchRegion() const;

        void CalculateNodePosition(const PblVector3* const pPosition,
                                   const PblVector3* const pVariance,
                                   const PblVector3* const pDirection,
                                   PblVector3& vPosition) const;

        static PblVector3 RandomVector(const PblVector3& v, float fX, float fY, float fZ);

        static bool ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                      RedPath::PathObject object,
                                      RedPath::PathObjectType type, void* pUserData);

        unsigned int mNumBranchPaths;
        float mBranchProbability;
        float mCosMaxAngle;
        float mSpeedFactor;
        PblVector3 mVariance;
        BranchInfo mBranchInfo;
        char mBranchInfoType;
        char mBranchSet;
        bool mLandOnArrival;
    };

    class SupportedClass
    {
    public:
        SupportedClass() : mClassID(0), mMaxCount(0), mCount(0) {}

        void SetID(unsigned int uClassID) { mClassID = uClassID; }
        unsigned int GetID() const { return mClassID; }
        void SetMaxCount(unsigned int uMaxCount) { mMaxCount = uMaxCount; }
        unsigned int GetMaxCount() const { return mMaxCount; }
        unsigned int GetCount() const { return mCount; }
        bool Add() const
        {
            if (mCount >= mMaxCount)
            {
                return false;
            }
            ++mCount;
            return true;
        }
        void Remove() const { --mCount; }

        unsigned int mClassID;
        unsigned int mMaxCount;
        mutable unsigned int mCount;
    };

    EntityPath();
    virtual ~EntityPath();

    // SYNTHETIC: SWBF2 0x004d1600
    // EntityPath::`scalar deleting destructor'

    virtual bool Read(PblConfig& config);

    const PblVector3* GetPointPosition(int i) const;
    void CalculateDirection(int i, char cDirection, PblVector3& vDirection) const;
    void CalculatePointPosition(int i, char cDirection, PblVector3& vPosition) const;
    float CalculateSuitability(const PblVector3* const pPosition, const PblVector3* const pDirection,
                               float fMinDot, float fRange, int i, char cDirection,
                               const PblVector3* pDelta, float fDistSq) const;
    bool ClassIsSupported(unsigned int uClassID, bool bCheckCount) const;
    const NodeProperties& GetNodeProperties(unsigned int i) const;
    bool Add(unsigned int uClassID, char cDirection, PblListDouble::Node* const pNode) const;
    void Remove(unsigned int uClassID, PblListDouble::Node* const pNode) const;
    bool IsEnabled();

    char GetDirection() const
    {
        if (mDirection)
        {
            return mDirection;
        }
        if (mSingleDirection)
        {
            return mDirectionEntity;
        }
        return 0;
    }

    static const PblList<EntityPath>& GetPathList();
    static void DestroyAll();

    static bool AllPathsIsClassSupported(unsigned int uClassID);

    static void EnablePathByName(unsigned int uName, bool bEnable);

    static float BuildMatrix(const PblVector3& vDirection, PblMatrix& matrix, PblMatrix* const pFlatMatrix);

protected:
    static bool ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                  RedPath::PathObject object,
                                  RedPath::PathObjectType type, void* pUserData);

private:
    void DupeHeadAndTail();

public:
    SupportedClass mClasses[5];
    unsigned int mNumClassIDs;
    NodeProperties* mNodeProps;
    float mSpeedFactor;
    PblVector3 mVariance;
    mutable unsigned int mCount;
    unsigned int mMaxCount;
    unsigned int mMaxSquadronSize;
    bool mPathRoll : 1;
    bool mDifferentPath : 1;
    bool mSingleDirection : 1;
    bool mEnabled : 1;
    char mDirection;
    float mFrequency;
    unsigned int mEnableObject;
    mutable char mDirectionEntity;
    mutable PblListDouble mEntityList;

    static PblList<EntityPath> sList;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x0079fba0
class EntityPathFactory : public GamePathFactory
{
public:
    EntityPathFactory() : GamePathFactory(0x87499D36) {}

    // SYNTHETIC: SWBF2 0x005392d0
    // EntityPathFactory::`scalar deleting destructor'

    virtual GamePath* CreatePath() const;
};
   
 
