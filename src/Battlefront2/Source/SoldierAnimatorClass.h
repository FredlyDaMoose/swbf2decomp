#pragma once

#include "PebbleFL/Common/PblBitVector.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "SoldierAnimType.h"
#include "SoldierAnimationBank.h"

class ZephyrSkeletonShared;
class SoldierAnimator;
class SoldierAnimation;
class RedAnimation;
class ZephyrAnim;
class PblFileChunk;
template <int N> class ZephyrSkeleton;
template <int N> class ZephyrPoseDyn;
template <int N> class ZephyrPoseStatic;

struct SoldierOrdnanceCollisionTable
{
    PblVector3 avHeadPos[29];
    PblVector3 avTorsoPos[29];
    PblVector3 avPitchAxis[17];
};

class SoldierAnimatorClass
{
public:

    static void SetupJointTransformAnimationFlag(ZephyrSkeletonShared* pSkel);
    static void SetupBodyMasks(const ZephyrSkeletonShared* pSkel, PblBitVector<32>& upperBodyMask,
                               PblBitVector<32>& lowerBodyMask);

    static SoldierAnimatorClass* Instance() { return sInstance; }
    ZephyrSkeletonShared* GetDefaultSkeleton() { return mZephyrSkeletonShared; }

    const PblMatrix& GetJetPackMatrix() const { return mJetPackMatrix; }

    const PblBitVector<32>& GetDefaultUpperBodyAnimMask() const { return mDefaultUpperBodyAnimMask; }
    const PblBitVector<32>& GetDefaultLowerBodyAnimMask() const { return mDefaultLowerBodyAnimMask; }

    const SoldierAnimation* GetLowerBodyActionAnimation(SoldierAnimType::ActionAnimation eAnim,
                                                        SoldierAnimationBank::MAP eMap) const;
    const SoldierAnimation* GetUpperBodyWeaponAnimation(SoldierAnimType::Posture ePosture,
                                                        SoldierAnimationBank::MAP eMap,
                                                        SoldierAnimType::WeaponAnimation eAnim) const;

    static SoldierAnimType::WeaponAnimation WeaponStateToAnimation(int eState, bool bAlternate);

    static void PostLoad();
    void _PostLoad();

    static void Create();

    static void Destroy();
    static void Read(PblFileChunk& chunk);
    SoldierAnimator* GetAnimator(float fTime);
    const SoldierAnimation* GetUpperBodyMovementAnimation(SoldierAnimType::MovementAnimation eAnim,
                                                          SoldierAnimType::Posture ePosture,
                                                          SoldierAnimationBank::MAP eMap, bool bAlert) const;
    const SoldierAnimation* GetLowerBodyMovementAnimation(SoldierAnimType::MovementAnimation eAnim,
                                                          SoldierAnimType::Posture ePosture,
                                                          SoldierAnimationBank::MAP eMap, bool bAlert) const;
    const SoldierAnimation* GetUpperBodyActionAnimation(SoldierAnimType::ActionAnimation eAnim,
                                                        SoldierAnimationBank::MAP eMap) const;
    const SoldierAnimation* GetLowerBodyWeaponAnimation(SoldierAnimType::Posture ePosture,
                                                        SoldierAnimationBank::MAP eMap,
                                                        SoldierAnimType::WeaponAnimation eAnim) const;
    const SoldierAnimation* GetUpperBodyAnimation(SoldierAnimationBank::MAP eMap, unsigned char uiAnimIndex) const;
    const SoldierAnimation* GetLowerBodyAnimation(SoldierAnimationBank::MAP eMap, unsigned char uiAnimIndex) const;
    unsigned int GetCustomAnimation(SoldierAnimationBank::MAP eMap, SoldierAnimationBank::ANIMATION eAnim) const;
    const ZephyrAnim* GetAnimation(unsigned int uiNameHash, const char* pcName) const;
    bool GetAnimationBoneMatrix(SoldierAnimationBank::MAP eMap, SoldierAnimType::Posture ePosture,
                                SoldierAnimType::MovementAnimation eAnim, unsigned int uiJointCrc, float fTime,
                                PblMatrix& matrix) const;
    bool GetAnimationBoneMatrix(SoldierAnimationBank::MAP eMap, SoldierAnimType::ActionAnimation eAnim,
                                unsigned int uiJointCrc, float fTime, PblMatrix& matrix) const;
    bool GetAnimationBoneMatrix(SoldierAnimationBank::MAP eMap, SoldierAnimType::Posture ePosture,
                                SoldierAnimType::WeaponAnimation eAnim, unsigned int uiJointCrc, float fTime,
                                PblMatrix& matrix) const;
    bool GetAnimationBoneMatrix(SoldierAnimationBank::MAP eMap, SoldierAnimationBank::ANIMATION eAnim,
                                unsigned int uiJointCrc, float fTime, PblMatrix& matrix) const;
    bool GetAnimationBoneMatrix(SoldierAnimationBank::MAP eMap, unsigned char uiAnimIndex, unsigned int uiJointCrc,
                                float fTime, PblMatrix& matrix) const;

    static const int NUM_ACTION_ANIMATIONS_TOTAL = 76;
    static const int NUM_MOVEMENT_ANIMATIONS_TOTAL = 156;
    static const int NUM_WEAPON_ANIMATIONS_TOTAL = 36;
    static const int NUM_WEAPON_MELEE_ANIMATIONS_TOTAL = 60;
    static const int NUM_SOLDIER_ANIMATIONS_TOTAL = 292;
    static const int ANIMATION_MAP_SIZE = 302;
    static const int MAX_ANIMATION_BANK_FILES = 18;

    struct AnimationMap
    {
        union
        {
            SoldierAnimation* mAnimation[NUM_SOLDIER_ANIMATIONS_TOTAL];
            struct
            {
                SoldierAnimation* mActionAnimation[SoldierAnimType::NUM_ACTION_ANIMATIONS][2];
                SoldierAnimation* mMovementAnimation[SoldierAnimType::NUM_POSTURES]
                                                    [SoldierAnimType::NUM_MOVEMENT_ANIMATIONS][2];
                union
                {
                    SoldierAnimation* mWeaponAnimation[SoldierAnimType::NUM_STANDARD_POSTURES]
                                                      [SoldierAnimType::NUM_WEAPON_ANIMATIONS][2];
                    SoldierAnimation* mWeaponMeleeAnimation[30][2];
                };
            };
        };
        unsigned int mCustomAnimation[10];
    };

private:
    SoldierAnimatorClass();
    ~SoldierAnimatorClass();

public:

    void UpdateLRU(SoldierAnimator* pAnimator);
    bool _AddBank(const RedAnimation* pBank);
    bool _AddBank(const char* pcName);
    static bool _GetAnimationBoneMatrix(const ZephyrSkeletonShared* pSkel, const ZephyrAnim* pUpperAnim,
                                        const ZephyrAnim* pLowerAnim, float fUpperTime, float fLowerTime,
                                        unsigned int uiJointCrc, PblMatrix& matrix);
    static const ZephyrSkeletonShared* _GetAnimationMapSkeleton(SoldierAnimationBank::MAP eMap);

    const PblMatrix* _GetBoneMatrix(unsigned int uiJointCrc, const SoldierAnimation* pAnim,
                                    ZephyrSkeleton<32>& skeleton, ZephyrPoseDyn<32>& poseDyn,
                                    ZephyrPoseStatic<32>& poseStatic) const;

public:

    static SoldierAnimatorClass* sInstance;

    SoldierAnimator* mSoldierAnimators;
    PblList<SoldierAnimator> mLRU_List;
    ZephyrSkeletonShared* mZephyrSkeletonShared;
    PblBitVector<32> mDefaultUpperBodyAnimMask;
    PblBitVector<32> mDefaultLowerBodyAnimMask;

    AnimationMap mMap[SoldierAnimationBank::MAX_MAPS];
    SoldierOrdnanceCollisionTable m_aOrdCollTable[SoldierAnimationBank::MAX_MAPS];
    RedAnimation* mAnimBank[MAX_ANIMATION_BANK_FILES];
    int mAnimBankCount;
    PblMatrix mJetPackMatrix;
};
 
 
 
