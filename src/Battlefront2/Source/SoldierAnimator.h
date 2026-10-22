#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblBitVector.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"
#include "Zephyr/Source/ZephyrSkeleton.h"
#include "SoldierAnimType.h"
#include "SoldierAnimationBank.h"

#include "EntitySoldier.h"

class EntitySoldier;
class SoldierAnimation;
class SoldierAnimationData;
class ZephyrAnim;
class ZephyrSkeletonShared;

class SoldierAnimator
{
public:
    SoldierAnimator();
    ~SoldierAnimator();

    void Initialize(const ZephyrSkeletonShared* pSkeleton);
    int GetFramesSinceUsed() const;
    void SetNewOwner(unsigned int uiOwnerId, const EntitySoldier* pOwner, const ZephyrSkeletonShared* pSkeleton,
                     const PblBitVector<32>& upperBodyMask, const PblBitVector<32>& lowerBodyMask,
                     float fSkeletonScale);
    bool RestartJumpAnimation(float fForward, float fSide, bool bForce);
    void SetAction(SoldierState::SoldierState eState, const PblMatrix& legMatrix, unsigned int uiAnimationHash);

    const EntitySoldier* GetOwner() const
    {
        return mOwner ? mOwner : (const EntitySoldier*)mOwnerId;
    }
    void SetWeaponAnimationMap(SoldierAnimationBank::MAP eMap);

    void SetDamageDir(EntitySoldier::DAMAGEDIR eDir);

    void SetWeaponState(int eState, float fTime, bool bRestart, bool bAlternate);
    void SetWeaponComboState(int eState, bool bRestart, unsigned char uiAnim, unsigned char uiAnimNext,
                             bool bAnimControlledMove);
    void SetAimDirection(const PblVector3& aimDir);

    void SetAlert(bool bAlert);

    float GetAlertFactor() const;
    void SetupPose(class RedPose& pose);

    void PlayIdleAnimation(SoldierAnimType::MovementAnimation eAnimation);

    void ModifyLegMatrixByVehicle(const PblMatrix& lastVehicleMatrix, const PblMatrix& vehicleMatrix);

private:

    void RotateLowerBody(float fAngle);

    void UpdateMovementAnimation(float fDeltaTime, const ZephyrAnim* pAnim, float fBlend);

    bool UpdateActionAnimation(SoldierAnimType::ActionAnimation eAction, float fDeltaTime, float fSpeed,
                               float fAngle, float, bool);

    SoldierAnimType::MovementAnimation GetMovementAnimation(float fSide, float fForward, float& fAngle,
                                                            float& fRate) const;
public:

    bool UpdateUpperBodyAnimation(float fDeltaTime, const SoldierAnimation* pAnim);

    bool UpdateLowerBodyAnimation(float fDeltaTime, const SoldierAnimation* pAnim);
private:

    float GetUpperBodyBlendFactor();

    static float CalculateBlendFactorDelta(float fPrevFactor, float fFactor);
    float GetLowerBodyBlendFactor();

    void ApplyProceduralAnimationAndBuildWorldMatrices(float fDeltaTime);
public:

    bool UpdateUpperBodyAnimation(float fDeltaTime, const ZephyrAnim* pAnim, bool, float, unsigned char);
    bool UpdateLowerBodyAnimation(float fDeltaTime, const ZephyrAnim* pAnim, bool, float);

public:

    PblList<SoldierAnimator>::Node& GetLRUNode() { return mLRUNode; }

    PblMatrix mLegMatrix;
    PblList<SoldierAnimator>::Node mLRUNode;
    EntitySoldier* mOwner;
    unsigned int mOwnerId;
    PblBitVector<32> mUpperBodyAnimMask;
    PblBitVector<32> mLowerBodyAnimMask;
    float mSkeletonScale;
    PblVector3 mAimDir;
    SoldierState::SoldierState mSoldierAction;
    SoldierAnimationBank::MAP mWeaponAnimationMap;
    int mWeaponState;
    float m_fAlertTimer;
    float m_fAlertBlendDuration;
    float m_fTakeKneeTimer;
    float m_fSlideDistance;
    int m_eLastHitDir;
    ZephyrAnim* mOverlayAnim;
    float mOverlayTime;
    float mOverlayBlendFactor;
    unsigned int mAnimationHash;
    float mForceFrameNum1;
    float mForceFrameNum2;
    float mForceFrameBlend;
    PblVector3 mMovement;
    PblVector3 mLegDirection;
    int mLastFrameUsed;
    unsigned int mNetTurn;
    float mExtraDt;
    ZephyrSkeleton<32> mZephyrSkeleton;
    ZephyrPoseStatic<32> mZephyrPoseStatic;
    ZephyrPoseDyn<32> mZephyrPoseDynUpper;
    ZephyrPoseDyn<32> mZephyrPoseDynLower;
    SoldierAnimation* m_pAnimLower;
    SoldierAnimation* m_pAnimUpper;
    float mAnimationTimerLower;
    float mAnimationTimerUpper;
    float mBlendDurationLower;
    float mBlendDurationUpper;
    float mBlendDurationRotate;
    float mBlendFactorLower;
    float mBlendFactorUpper;
    SoldierAnimType::Posture mPosture;
    SoldierAnimType::ActionAnimation mAction;
    float mActionTime;
    SoldierAnimType::MovementAnimation mMoveAnimation;
    float mMoveTurnRate;
    float mWeaponAnimationRate;
    float mLegAngle;
    float mAimXAngle;
    float mAimYAngle;
    float mRotAngle;
    float mRotSpeed;
    SoldierAnimType::WeaponAnimation mWeaponAnim;
    unsigned char mWeaponMeleeAnim;
    unsigned char mWeaponMeleeAnimNext;

    unsigned short m_uiAimState : 8;
    unsigned short m_bAlert : 1;
    unsigned short m_bMoveAlert : 1;
    unsigned short m_bRestartWeaponAnim : 1;
    unsigned short m_bFirstTime : 1;
    unsigned short m_bResetUpperAnim : 1;
    unsigned short m_bResetLowerAnim : 1;
    unsigned short m_bAnimControlledMove : 1;
    unsigned short m_bAnimVelocityMatched : 1;

    unsigned short m_bUseDirectionalJumps : 1;
    unsigned short m_bUseDirectionalDeaths : 1;
    unsigned short m_bRestartJumpForJedi : 1;
    float m_fLastNonZeroDT;
    SoldierAnimation* m_aAnimPrev;
    SoldierAnimationData* m_aZephyrAnimData;
};
 
 
 
 
 
 
 
 
 
