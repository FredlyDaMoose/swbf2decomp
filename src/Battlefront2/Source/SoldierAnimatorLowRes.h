#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblBitVector.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"
#include "Zephyr/Source/ZephyrSkeleton.h"
#include "SoldierAnimationBank.h"

class EntitySoldier;
class PblMatrix;
class ZephyrAnim;
class ZephyrSkeletonShared;
class SoldierAnimatorLowRes;
class SoldierAnimatorLowRes_Pool;
class SoldierAnimatorLowRes_UnsharedPool;
class SoldierAnimatorLowRes_HighResPool;

class SoldierAnimatorLowResClass
{
public:
    enum POSE
    {
        POSE_ANIMPOOL = -1,
        POSE_STAND = 0,
        POSE_CROUCH = 1,
        POSE_CROUCH_TAKEKNEE = 2,
        POSE_JET = 3,
        POSE_ROLL = 4,
        POSE_FLY = 5,
        NUM_BASE_POSES = 6,
        POSE_CUSTOM = 6,
        POSE_HIGHRES = 16
    };

    enum ANIMPOOL
    {
        ANIMPOOL_POSE = -1,
        ANIMPOOL_MOVE = 0,
        ANIMPOOL_DEATH = 1,
        NUM_BASE_ANIMPOOLS = 2,
        ANIMPOOL_HIGHRES = 2
    };

    struct PoseClass
    {
        const char* m_szName;
        int m_iFrame;
    };

    struct AnimClass
    {
        const char* m_szName;
        int m_iAnimGroup;
    };

    struct AnimPoolClass
    {
        AnimClass* m_aAnims;
        int m_iNumAnims;
        int m_iPoolSize;
        bool m_bLoop;
    };

    struct Pose
    {
        ZephyrAnim* m_pZephyrAnim;
        SoldierAnimatorLowRes* m_pAnimator;
    };

    struct HighResPose : public Pose
    {
        SoldierAnimationBank::MAP eMap;
        unsigned char uiAnimIndex;
    };

    struct AnimPool
    {
        ZephyrAnim** m_apZephyrAnim;
        SoldierAnimatorLowRes_Pool* m_apAnimator;
        int m_iLastUsed;
    };

    struct UnsharedAnimPool
    {
        ZephyrAnim** m_apZephyrAnim;
        SoldierAnimatorLowRes_UnsharedPool* m_apAnimator;
    };

    struct HighResAnimPool
    {
        SoldierAnimatorLowRes_HighResPool* m_apAnimator;
        int m_iPoolSize;
    };

    SoldierAnimatorLowResClass(const char* pcName, const SoldierAnimatorLowResClass* pParent);
    ~SoldierAnimatorLowResClass();

    static void Create();
    static void Destroy();
    static void AddBank(const char* pcName, const char* pcParent);
    static void PostLoad();
    static SoldierAnimatorLowRes* GetAnimator(const EntitySoldier& soldier);
    static ZephyrSkeletonShared* GetDefaultSkeleton();

private:
    SoldierAnimatorLowRes* GetAnimatorLocal(const EntitySoldier& soldier);
    static void SetupJointMask(const ZephyrSkeletonShared* pSkeleton, const ZephyrSkeletonShared* pSource,
                               PblBitVector<32>& kMask);
    void SetupJointTransformAnimationFlag(ZephyrSkeletonShared* pSkeleton);

public:
    SoldierAnimatorLowResClass* m_pNext;
    SoldierAnimatorLowResClass* m_pParent;
    char m_szName[32];
    unsigned int m_uiNameHash;
    Pose* m_aPose;
    AnimPool m_AnimPool;
    UnsharedAnimPool m_UnsharedAnimPool;
    HighResAnimPool* m_pHighResAnimPool;
    HighResPose* m_aHighResPose;
    int m_iNumHighResPoses;
    ZephyrSkeletonShared* m_pZephyrSkeletonShared;

    static PoseClass s_PoseClasses[7];
    static AnimClass s_AnimClasses[2];
    static AnimPoolClass s_AnimPoolClasses[2];
    static SoldierAnimatorLowResClass* s_pHead;
    static SoldierAnimatorLowResClass* s_pTail;
};
 
 
 

// VTABLE: SWBF2 0x007ae91c
class SoldierAnimatorLowRes
{
public:
    SoldierAnimatorLowRes();
    virtual ~SoldierAnimatorLowRes();
    virtual const RedPose* GetPose();
    virtual bool GetRootMatrixAnimation(const EntitySoldier& soldier, PblMatrix& kMatrix) const;
    virtual int GetFramesSinceUsed() const;

    void Initialize(const ZephyrSkeletonShared* pSkeleton, const PblBitVector<32>& kMask,
                    const SoldierAnimatorLowResClass::PoseClass* pPoseClass,
                    const SoldierAnimatorLowResClass::Pose* pPose, SoldierAnimatorLowResClass::POSE ePose);

    void Release(const EntitySoldier& soldier)
    {
        GetOwnerAnimator(soldier, (SoldierAnimatorLowResClass::POSE)-1,
                         (SoldierAnimatorLowResClass::ANIMPOOL)-1);
    }

protected:
    friend class SoldierAnimatorLowResClass;
    virtual SoldierAnimatorLowRes* GetOwnerAnimator(const EntitySoldier& soldier,
                                                    SoldierAnimatorLowResClass::POSE ePose,
                                                    SoldierAnimatorLowResClass::ANIMPOOL eAnimPool);

public:
    SoldierAnimatorLowResClass::POSE m_ePose;
    SoldierAnimatorLowResClass::ANIMPOOL m_eAnimPool;
    ZephyrSkeleton<32> mZephyrSkeleton;
    RedPose mPose;
};
 
 
 
 
 

// VTABLE: SWBF2 0x007ae8c4
class SoldierAnimatorLowRes_Pool : public SoldierAnimatorLowRes
{
public:
    SoldierAnimatorLowRes_Pool();
    virtual ~SoldierAnimatorLowRes_Pool();
    virtual const RedPose* GetPose();
    virtual bool GetRootMatrixAnimation(const EntitySoldier& soldier, PblMatrix& kMatrix) const;

    void Initialize(const ZephyrSkeletonShared* pSkeleton, const PblBitVector<32>& kMask,
                    const SoldierAnimatorLowResClass::AnimPoolClass* pPoolClass,
                    const SoldierAnimatorLowResClass::AnimPool* pPool,
                    SoldierAnimatorLowResClass::ANIMPOOL eAnimPool);

protected:
    virtual SoldierAnimatorLowRes* GetOwnerAnimator(const EntitySoldier& soldier,
                                                    SoldierAnimatorLowResClass::POSE ePose,
                                                    SoldierAnimatorLowResClass::ANIMPOOL eAnimPool);

public:
    int m_iLastFrame;
    ZephyrPoseDyn<32> mZephyrPoseDyn;
    ZephyrPoseStatic<32> mZephyrPoseStatic;
    PblBitVector<32> mZephyrJointMask;
};
 
 
 

// VTABLE: SWBF2 0x007ae8f4
class SoldierAnimatorLowRes_UnsharedPool : public SoldierAnimatorLowRes_Pool
{
public:
    SoldierAnimatorLowRes_UnsharedPool();
    virtual ~SoldierAnimatorLowRes_UnsharedPool();
    virtual bool GetRootMatrixAnimation(const EntitySoldier& soldier, PblMatrix& kMatrix) const;
    virtual int GetFramesSinceUsed() const;

    void Initialize(const ZephyrSkeletonShared* pSkeleton, const PblBitVector<32>& kMask,
                    const SoldierAnimatorLowResClass::AnimPoolClass* pPoolClass,
                    const SoldierAnimatorLowResClass::UnsharedAnimPool* pPool,
                    SoldierAnimatorLowResClass::ANIMPOOL eAnimPool);

protected:
    friend class SoldierAnimatorLowResClass;
    virtual SoldierAnimatorLowRes* GetOwnerAnimator(const EntitySoldier& soldier,
                                                    SoldierAnimatorLowResClass::POSE ePose,
                                                    SoldierAnimatorLowResClass::ANIMPOOL eAnimPool);
    void SetAnim(const ZephyrAnim* pAnim, float fTime);

public:
    const EntitySoldier* m_pOwner;
    int mLastFrameUsed;
};
 
 

// VTABLE: SWBF2 0x007ae8dc
class SoldierAnimatorLowRes_HighResPool : public SoldierAnimatorLowRes_UnsharedPool
{
public:

    SoldierAnimatorLowRes_HighResPool();
    virtual ~SoldierAnimatorLowRes_HighResPool();

    void Initialize(const ZephyrSkeletonShared* pSkeleton, const PblBitVector<32>& kMask);

protected:
    friend class SoldierAnimatorLowResClass;
    void SetAnim(SoldierAnimationBank::MAP eMap, unsigned char uiAnimIndex);
    void ResetAnim(SoldierAnimationBank::MAP eMap, unsigned char uiAnimIndex);

public:
    SoldierAnimationBank::MAP m_eMap;
    unsigned char m_uiAnimIndex;
};
 
 
