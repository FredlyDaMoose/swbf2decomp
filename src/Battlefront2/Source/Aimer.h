#pragma once

#include <stddef.h>

#include "Thread.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class RedModel;

class NetPktGroup;

class AimerClass
{
public:

    struct BarrelInfo
    {
        unsigned int mHash;
        unsigned int mFireHash;
        unsigned int mParentHash;
        float mRecoil;
        PblVector3 mOffset;
        PblVector3 mFireOffset;
    };

    AimerClass();

    ~AimerClass();

    bool SetProperty(unsigned int uProperty, const char* pszValue, const RedModel* pModel, const char* pszUnused);

    BarrelInfo mBarrelInfo[4];
    int mNumBarrels;
    float mPitchMin;
    float mPitchMax;
    float mPitchRate;
    float mYawMin;
    float mYawMax;
    float mYawRate;
    bool mShiftYaw;
    PblVector3 mRestDir;
    PblVector3 mOffsetPos;
    PblQuaternion mOffsetRot;
    unsigned int mMountHash;
    unsigned int mParentHash;
    bool mIsInert;
    bool mFireOutsideLimits;
    int mHierarchyLevel;
    AimerClass* mNextAimerClass;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

class Controllable;
class RedPose;
class Weapon;

// VTABLE: SWBF2 0x00796bc8
class Aimer : public Thread
{
public:

    void Read(NetPktGroup& group);
    Aimer(AimerClass* pClass, const Controllable* pOwner, const PblMatrix* pParentMatrix,
          RedPose* pPose, Aimer* pNextAimer);
    virtual ~Aimer();
    virtual bool Update(float fDeltaTime);

    static void* operator new(size_t uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;

    void SetSoldierInfo(const PblVector3& vFirePos, const PblVector3& vDirection);
    void SetupPose(const PblMatrix& mParent, RedPose& pose, bool bRecurse) const;
    bool UpdateFire(Aimer*& pNextAimer, bool bFire);
    bool GetLeadPos(const PblVector3& vTarget, const PblVector3& vVelocity, PblVector3* pvLead) const;
    const PblMatrix GetMatrix() const;

    PblMatrix GetFireMatrix() const;
    void ActivatePhysics();
    void DeactivatePhysics();
    void SetWeapon(Weapon* pWeapon);

    Weapon* GetWeapon() const { return mWeapon; }

    const PblVector3& GetDirection() const { return mDirection; }
    void SetSkip(bool bSkip);
    void ResetSkip();
    const PblVector3& GetAngVel() const;
    void SetAngVel(const PblVector3& vAngVel);
    bool WeaponShouldFire() const;

private:
    bool ClampDirection(const PblMatrix& mMatrix);
    bool HasSibling() const;

public:
    Aimer* mNextAimer;
    AimerClass* mClass;
    const Controllable* mOwner;
    const PblMatrix* mParentMatrix;
    bool mWithinLimitsFlag;
    bool bDirect;
    PblVector3 mOffsetPos;
    float mPitch;
    float mOmegaPitch;
    float mYaw;
    float mOmegaYaw;
    PblVector3 mDirection;
    PblVector3 mMountPos;
    PblQuaternion mMountRot;
    PblVector3 mRootPos;
    PblVector3 mOldRootPos;
    PblVector3 mFirePos;
    PblVector3 mPrevFirePos;
    PblVector3 mAngVel;
    mutable PblMatrix mMountPoseMatrix;
    mutable PblMatrix mBarrelPoseMatrix[4];
    RedPose* mPose;
    float mRecoil[4];
    int mCurrentBarrel;
    Weapon* mWeapon;
    bool mSkip;
    float mSkipTime;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
