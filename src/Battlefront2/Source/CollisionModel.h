#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblVector.h"
#include "CollisionObject.h"

class CollisionBody;
class CollisionObject;
class CollisionOptimizationInfo;
class Restrictor;

class CollisionMask
{
public:
    CollisionMask()
    {
        mMask[0] = 0xFFFFFFFFu;
        mMask[1] = 0xFFFFFFFFu;
    }

    // FUNCTION: SWBF2 0x004440a0
    CollisionMask(unsigned int uMask)
    {
        if (uMask == 0xFFFFFFFFu || uMask == 0)
        {
            mMask[1] = uMask;
            mMask[0] = uMask;
        }
        else
        {
            mMask[0] = uMask;
            mMask[1] = 0;
        }
    }

    // FUNCTION: SWBF2 0x00462100
    void SetId(int id)
    {
        if (id <= 31)
        {
            mMask[0] |= 1u << id;
        }
        else
        {
            mMask[1] |= 1u << (id - 32);
        }
    }

    // FUNCTION: SWBF2 0x00462170
    void ClearId(int id)
    {
        if (id <= 31)
        {
            mMask[0] &= ~(1u << id);
        }
        else
        {
            mMask[1] &= ~(1u << (id - 32));
        }
    }

    // FUNCTION: SWBF2 0x00462130
    bool IsSet(int id) const
    {
        if (id <= 31)
        {
            return (mMask[0] & (1u << id)) != 0;
        }
        return (mMask[1] & (1u << (id - 32))) != 0;
    }

    bool IsEmpty() const { return mMask[0] == 0 && mMask[1] == 0; }

    unsigned int mMask[2];
};
 

class PblMatrix;
class RedPose;

class CollisionResult
{
public:
    PblVector3 mCollisionPosition;
    PblVector3 mSeparationNormal;
    float mOverlap;
    unsigned char mColliderId;
    unsigned char mReceiverId;
};
 

class CollisionOptimizationInfo
{
public:

    CollisionOptimizationInfo() : bodyMatrices(NULL), ignoreMasks(NULL)
    {
        for (int i = 0; i < 64; ++i)
        {
            refreshMatrices[i] = true;
        }
    }

    PblMatrix* bodyMatrices;
    bool refreshMatrices[64];
    PblMatrix* otherModelMatrices;
    CollisionMask* ignoreMasks;
};
 

// VTABLE: SWBF2 0x00797450
class CollisionModel
{
public:
    CollisionModel();

    static void EnterClient();
    static void LeaveClient();

    virtual ~CollisionModel();

    virtual bool CheckForCollision(CollisionObject *, CollisionObject *, Restrictor *, CollisionOptimizationInfo *) const;
    virtual bool CheckForCollision_SSSC(CollisionObject *, CollisionObject *, Restrictor *, CollisionOptimizationInfo *) const;

    void ClearAll(bool bDeleteBodies);
    const CollisionResult* CollideTerrain(CollisionObject* pObject, const PblMatrix& matrix,
                                          const RedPose* pPose) const;
    int GetMaskId(unsigned int uiName) const;
    const float GetHeightFromMaskID(int id) const;
    unsigned int GetParentNameFromMaskId(int id) const;
    int GetTargetBodyID(const PblVector3& vPos, const PblVector3& vDir, const PblMatrix& matrix,
                        const RedPose* pPose) const;
    PblVector3 GetTargetPoint(const PblVector3& vPos, const PblVector3& vDir, const PblMatrix& matrix,
                              const RedPose* pPose) const;
    const PblMatrix& GetBodyMatrix(int id) const;
    PblMatrix GetWorldMatrix(int id, const PblMatrix& matrix, const RedPose* pPose) const;
    PblMatrix GetLocalMatrix(int id, const RedPose* pPose) const;

    int GetNumBodies() const;

    CollisionBody* GetBody(int id);

    CollisionBody* SetBody(int id, CollisionBody* pBody);

    bool AddBody(CollisionBody* pBody);
    bool PrepareCollision(CollisionObject*, const CollisionModel*, CollisionObject*,
                          const CollisionMask&, const CollisionMask&) const;
    static void RegisterCollision(int, int);
    const CollisionMask& GetCollisionMask(const CollisionObjectType&) const;

    void SetCollisionMask(const CollisionObjectType& eType, const CollisionMask& mask);
    float RayHit(const PblMatrix&, const RedPose*, const PblVector3&, const PblVector3&,
                 float, PblVector3*, int*, bool, float) const;
    bool RayTest(const PblMatrix&, const RedPose*, const PblVector3&, const PblVector3&, float) const;

    static CollisionResult sCollisionResult;
    static CollisionResult sHostCollisionResult;
    static CollisionResult sClientCollisionResult;
    static CollisionMask (*sCollisionMask)[64];
    static CollisionMask sHostCollisionMask[64];

    static CollisionMask sClientCollisionMask[64];

    bool mDenyFlyerLand;
    bool mDeathOnFlyerLand;
    bool mIsFlyerFlag;
    bool mUseVehicleCollisionAgainstFlyers;

    CollisionBody* mBody[64];
    int mNumBodies;
    CollisionMask mSoftMask;
    CollisionMask mRigidMask;
    CollisionMask mStaticMask;
    CollisionMask mTerrainMask;
    CollisionMask mOrdnanceMask;
    CollisionMask mTowCableMask;
    CollisionMask mTargetableMask;
    CollisionMask mFlyingOrdnanceMask;
};
 
 
 
 
 
 
 
 
 
 
 
 
