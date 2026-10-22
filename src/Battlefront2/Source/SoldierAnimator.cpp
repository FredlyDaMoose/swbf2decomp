#include <math.h>
#include "PebbleFL/Common/PblVector.h"

#include "SoldierAnimator.h"
#include "EntitySoldier.h"
#include "SoldierAnimation.h"
#include "SoldierAnimatorClass.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblRandom.h"
#include "Zephyr/Source/ZephyrSkeletonShared.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "Character.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "SoldierAnimationData.h"
#include "Tracker.h"

// FUNCTION: SWBF2 0x00643550
void SoldierAnimator::PlayIdleAnimation(SoldierAnimType::MovementAnimation eAnimation)
{
    if (eAnimation <= 2 && mAction == 39 && mMoveAnimation <= 2)
    {
        mMoveAnimation = eAnimation;
    }
}

extern bool netFrameLock;

static const float ANIM_FRAME_SPEED = 1.0f / 30.0f;
static const float FIRST_PERSON_ANIMATION_SCALE = 0.45f;
static const float MOVEANGLE_BLEND_RATE = 7.5f;
static const float OVERLAY_FADEOUT_RATE = 8.0f;

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationX(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationY(PblMatrix* pOut, float fAngle);

namespace GameLoop { extern float sClientDeltaTime; }

const float UnitCircleNormalize(float fAngle);

namespace
{

inline const EntitySoldierClass* SoldierClass(const EntitySoldier* pSoldier)
{
    return pSoldier->mClass;
}

const unsigned char NO_MELEE_ANIM = 0xa4;

inline PblQuaternion PblQuaternion_FromAxisAngle(const PblVector3& vAxis, float fAngle)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle * 0.5f);
    PblQuaternion q;
    q._fX = vAxis.x * fSin;
    q._fY = vAxis.y * fSin;
    q._fZ = vAxis.z * fSin;
    q._fS = fCos;
    return q;
}
}

// FUNCTION: SWBF2 0x0063e760
SoldierAnimator::SoldierAnimator()
{
    m_bRestartJumpForJedi = false;
    m_bUseDirectionalJumps = true;
    m_bUseDirectionalDeaths = true;
    mSoldierAction = SoldierState::STAND;
    m_bAlert = false;
    m_bRestartWeaponAnim = false;
    m_bAnimControlledMove = false;
    m_bAnimVelocityMatched = false;
    mAction = SoldierAnimType::MOVE;
    mPosture = SoldierAnimType::STAND;
    mWeaponAnimationMap = SoldierAnimationBank::INVALID_MAP;
    mWeaponState = 0;
    mWeaponAnim = SoldierAnimType::WEAPON_IDLE;
    mWeaponMeleeAnim = NO_MELEE_ANIM;
    mWeaponMeleeAnimNext = NO_MELEE_ANIM;
    mOverlayTime = 0.0f;
    mOverlayBlendFactor = 0.0f;
    mOverlayAnim = 0;
    mForceFrameNum1 = -1.0f;
    mForceFrameNum2 = -1.0f;
    mForceFrameBlend = -1.0f;
    m_fAlertTimer = 0.0f;
    m_fAlertBlendDuration = 0.5f;
    m_fSlideDistance = 0.0f;
    m_pAnimUpper = 0;
    m_pAnimLower = 0;
    mBlendFactorLower = 0.0f;
    mBlendFactorUpper = 0.0f;
    mBlendDurationRotate = 0.0f;
    mBlendDurationLower = 0.0f;
    mBlendDurationUpper = 0.0f;
    mAnimationTimerLower = 0.0f;
    mAnimationTimerUpper = 0.0f;
    mWeaponAnimationRate = 1.0f;
}

// FUNCTION: SWBF2 0x0063e910
SoldierAnimator::~SoldierAnimator()
{
    mZephyrPoseDynLower.Close();
    mZephyrPoseDynUpper.Close();
    mZephyrPoseStatic.Close();
    mZephyrSkeleton.Close();
}

// FUNCTION: SWBF2 0x0063e9c0
int SoldierAnimator::GetFramesSinceUsed() const
{
    return RedRenderer::GetFrameNumber() - mLastFrameUsed;
}

// FUNCTION: SWBF2 0x0063e9d0
void SoldierAnimator::Initialize(const ZephyrSkeletonShared* pSkeleton)
{
    mZephyrSkeleton.Open(*pSkeleton, mZephyrPoseStatic);
    mZephyrPoseStatic.Open(mZephyrSkeleton);
    mZephyrPoseDynUpper.Open(mZephyrSkeleton);
    mZephyrPoseDynLower.Open(mZephyrSkeleton);
    mOwner = 0;
    mOwnerId = 0;
}

// FUNCTION: SWBF2 0x0063ea30
void SoldierAnimator::SetNewOwner(unsigned int uiOwnerId, const EntitySoldier* pOwner,
                                  const ZephyrSkeletonShared* pSkeleton, const PblBitVector<32>& upperBodyMask,
                                  const PblBitVector<32>& lowerBodyMask, float fSkeletonScale)
{
    mOwnerId = uiOwnerId;
    m_bRestartJumpForJedi = false;
    m_bMoveAlert = false;
    m_bRestartWeaponAnim = false;
    m_bAnimControlledMove = false;
    m_bAnimVelocityMatched = false;
    mSkeletonScale = fSkeletonScale;
    mOwner = const_cast<EntitySoldier*>(pOwner);
    mMoveAnimation = SoldierAnimType::IDLE;
    mAction = SoldierAnimType::MOVE;
    mSoldierAction = SoldierState::STAND;
    mWeaponAnimationMap = SoldierAnimationBank::INVALID_MAP;
    mWeaponState = 0;
    mWeaponAnim = SoldierAnimType::WEAPON_IDLE;
    mWeaponMeleeAnim = NO_MELEE_ANIM;
    mWeaponMeleeAnimNext = NO_MELEE_ANIM;
    m_bUseDirectionalJumps = pOwner ? SoldierClass(pOwner)->m_bUseDirectionalJumps : true;
    m_bUseDirectionalDeaths = pOwner ? SoldierClass(pOwner)->m_bUseDirectionalDeaths : true;
    mZephyrSkeleton.SetSharedSkeleton(pSkeleton ? pSkeleton : SoldierAnimatorClass::Instance()->GetDefaultSkeleton());
    mUpperBodyAnimMask = upperBodyMask;
    mLowerBodyAnimMask = lowerBodyMask;
    m_bFirstTime = true;
    mNetTurn = 0;
    mExtraDt = 0.0f;
    mLegAngle = 0.0f;
    mAnimationHash = 0;
    mAimXAngle = 0.0f;
    mAimYAngle = 0.0f;
    m_uiAimState = 0;
    mOverlayTime = 0.0f;
    mLastFrameUsed = RedRenderer::GetFrameNumber() - 10;
    m_fAlertTimer = 0.0f;
    m_bAlert = false;
    m_bResetUpperAnim = false;
    m_bResetLowerAnim = false;
    m_fAlertBlendDuration = 0.5f;
    m_fSlideDistance = 0.0f;
    m_pAnimUpper = 0;
    m_pAnimLower = 0;
    mBlendFactorLower = 0.0f;
    mBlendFactorUpper = 0.0f;
    mBlendDurationRotate = 0.0f;
    mBlendDurationLower = 0.0f;
    mBlendDurationUpper = 0.0f;
    mAnimationTimerLower = 0.0f;
    mAnimationTimerUpper = 0.0f;
    mWeaponAnimationRate = 1.0f;
    mZephyrPoseDynLower.SetAnimation(0, 30.0f);
    mZephyrPoseDynUpper.SetAnimation(0, 30.0f);
    RotateLowerBody(0.0f);
}

// FUNCTION: SWBF2 0x0063ec60
bool SoldierAnimator::RestartJumpAnimation(float fForward, float fSide, bool bForce)
{
    SoldierAnimType::ActionAnimation ePrevAction = mAction;

    if (m_bUseDirectionalJumps && fForward * fForward + fSide * fSide >= 0.0625f)
    {
        float fAbsSide = fabsf(fSide);
        if (fForward > fAbsSide)
        {
            mAction = SoldierAnimType::JUMP_FORWARD;
        }
        else if (-fForward > fAbsSide)
        {
            mAction = SoldierAnimType::JUMP_BACKWARD;
        }
        else
        {
            mAction = fSide <= 0.0f ? SoldierAnimType::JUMP_RIGHT : SoldierAnimType::JUMP_LEFT;
        }
    }
    else
    {
        mAction = SoldierAnimType::JUMP_UP;
    }

    if (ePrevAction == mAction)
    {
        if (bForce || mAction == SoldierAnimType::JUMP_UP || mAction == SoldierAnimType::JUMP_FORWARD)
        {
            mActionTime = 0.0f;
            m_bRestartJumpForJedi = true;
            return true;
        }
        return false;
    }

    mActionTime = 0.0f;
    m_bFirstTime = false;
    return true;
}

// FUNCTION: SWBF2 0x0063ed60
void SoldierAnimator::SetAction(SoldierState::SoldierState eState, const PblMatrix& legMatrix,
                                unsigned int uiAnimationHash)
{
    float fDt = GameLoop::sClientDeltaTime;
    if (fDt <= 0.0f && !m_bFirstTime)
    {
        return;
    }

    if (m_bFirstTime)
    {
        mMovement.z = 0.0f;
        mMovement.y = 0.0f;
        mMovement.x = 0.0f;
        mLegDirection = legMatrix.GetZAxis();
    }
    else
    {
        const PblVector3& newPos = legMatrix.GetTranslation();
        const PblVector3& oldPos = mLegMatrix.GetTranslation();
        mMovement.x = newPos.x - oldPos.x;
        mMovement.y = newPos.y - oldPos.y;
        mMovement.z = newPos.z - oldPos.z;
        if (mMovement.x * mMovement.x + mMovement.y * mMovement.y + mMovement.z * mMovement.z > 100.0f)
        {
            mMovement.z = 0.0f;
            mMovement.y = 0.0f;
            mMovement.x = 0.0f;
        }
    }
    mLegMatrix = legMatrix;

    bool bFirstTime = m_bFirstTime;
    if (!bFirstTime && mSoldierAction == eState)
    {
        if (mSoldierAction == SoldierState::PILOT_ANIMATION || mSoldierAction == SoldierState::CHOKE)
        {
            mAnimationHash = uiAnimationHash;
        }
        return;
    }

    if (m_bRestartJumpForJedi)
    {
        return;
    }

    mAnimationHash = 0;
    mMoveTurnRate = 0.0f;

    switch (eState)
    {
    case SoldierState::STAND:
    case SoldierState::CROUCH:
    case SoldierState::PRONE:
    case SoldierState::SPRINT:
    {
        SoldierAnimType::Posture ePosture =
            eState == SoldierState::SPRINT ? SoldierAnimType::STAND : (SoldierAnimType::Posture)eState;
        SoldierAnimType::ActionAnimation eAction = mAction;
        if (eAction != SoldierAnimType::MOVE && eAction != SoldierAnimType::SPRINT && !bFirstTime)
        {

            mPosture = ePosture;
            if (ePosture == SoldierAnimType::CROUCH)
            {
                m_fTakeKneeTimer = 0.0f;
            }
            SoldierState::SoldierState ePrev = mSoldierAction;
            if (ePrev == SoldierState::FLY_RECOVER || ePrev == SoldierState::CHOKE)
            {
                mAction = SoldierAnimType::LAND_HARD;
                mActionTime = 0.0f;
            }
            else if ((eAction >= SoldierAnimType::JUMP_UP && eAction <= SoldierAnimType::JUMP_RIGHT) ||
                     ePrev == SoldierState::FALL || eAction == SoldierAnimType::JUMP_FLY)
            {
                float fTime = mActionTime;
                mActionTime = 0.0f;
                mAction = fTime > 1.5f ? SoldierAnimType::LAND_HARD : SoldierAnimType::LAND_SOFT;
            }
            else if (ePrev == SoldierState::JET_HOVER || ePrev == SoldierState::JET_JUMP)
            {
                mAction = SoldierAnimType::LAND_SOFT;
                mActionTime = 0.0f;
            }
            else if (eAction == SoldierAnimType::BOUNCE_FRONT || eAction == SoldierAnimType::LAND_FRONT ||
                     ePrev == SoldierState::FLY || ePrev == SoldierState::TUMBLE)
            {
                mAction = SoldierAnimType::GETUP_FRONT;
                mActionTime = 0.0f;
            }
            else if (eAction == SoldierAnimType::BOUNCE_BACK || eAction == SoldierAnimType::LAND_BACK)
            {
                mAction = SoldierAnimType::GETUP_BACK;
                mActionTime = 0.0f;
            }
            else
            {
                mActionTime = 0.0f;
                mAction = eState == SoldierState::SPRINT ? SoldierAnimType::SPRINT : SoldierAnimType::MOVE;
            }
        }
        else if (bFirstTime)
        {
            mPosture = ePosture;
            if (eState == SoldierState::SPRINT)
            {
                mAction = SoldierAnimType::SPRINT;
            }
        }
        else
        {
            SoldierState::SoldierState ePrev = mSoldierAction;
            if (eState == ePrev)
            {
                break;
            }
            mActionTime = 0.0f;
            switch (eState)
            {
            case SoldierState::STAND:
                if (ePrev == SoldierState::CHOKE || ePrev == SoldierState::PRONE)
                {
                    mAction = SoldierAnimType::LAND_HARD;
                }
                else
                {
                    mAction = SoldierAnimType::MOVE;
                    if (ePrev == SoldierState::CROUCH && mMoveAnimation <= SoldierAnimType::IDLE_LOOK_AROUND)
                    {
                        mMoveAnimation = SoldierAnimType::IDLE;
                    }
                }
                break;
            case SoldierState::CROUCH:
                if (ePrev == SoldierState::CHOKE || ePrev == SoldierState::PRONE)
                {
                    mAction = SoldierAnimType::LAND_HARD;
                }
                else if (ePrev == SoldierState::STAND)
                {
                    mAction = SoldierAnimType::MOVE;
                    if (mMoveAnimation <= SoldierAnimType::IDLE_LOOK_AROUND)
                    {
                        mMoveAnimation = SoldierAnimType::IDLE;
                    }
                }
                break;
            case SoldierState::PRONE:
                mAction = SoldierAnimType::MOVE;
                mMoveAnimation = SoldierAnimType::IDLE;
                break;
            case SoldierState::SPRINT:
                if (ePrev == SoldierState::CHOKE || ePrev == SoldierState::PRONE)
                {
                    mAction = SoldierAnimType::LAND_HARD;
                }
                else if (ePrev == SoldierState::CROUCH)
                {
                    mAction = SoldierAnimType::MOVE;
                    if (mMoveAnimation <= SoldierAnimType::IDLE_LOOK_AROUND)
                    {
                        mMoveAnimation = SoldierAnimType::IDLE;
                    }
                }
                else
                {
                    mAction = SoldierAnimType::SPRINT;
                }
                break;
            }
            mPosture = ePosture;
        }
        break;
    }

    case SoldierState::JUMP:
    {
        mActionTime = 0.0f;
        float fForward;
        float fSide;
        if (fDt > 0.0f)
        {
            float fInvDt = 1.0f / fDt;
            const PblVector3& z = legMatrix.GetZAxis();
            const PblVector3& x = legMatrix.GetXAxis();
            fForward = (z.y * mMovement.y + z.x * mMovement.x + z.z * mMovement.z) * fInvDt;
            fSide = (mMovement.y * x.y + x.x * mMovement.x + mMovement.z * x.z) * fInvDt;
        }
        else
        {
            fSide = 0.0f;
            fForward = 0.0f;
        }
        if (m_bUseDirectionalJumps && fSide * fSide + fForward * fForward >= 4.0f)
        {
            float fAbsSide = fabsf(fSide);
            if (fForward > fAbsSide)
            {
                mAction = SoldierAnimType::JUMP_FORWARD;
            }
            else if (-fForward > fAbsSide)
            {
                mAction = SoldierAnimType::JUMP_BACKWARD;
            }
            else
            {
                mAction = fSide <= 0.0f ? SoldierAnimType::JUMP_RIGHT : SoldierAnimType::JUMP_LEFT;
            }
        }
        else
        {
            mAction = SoldierAnimType::JUMP_UP;
        }
        break;
    }

    case SoldierState::ROLL:
    case SoldierState::TUMBLE_RECOVER:
        mAction = SoldierAnimType::DIVE;
        mActionTime = 0.0f;
        break;

    case SoldierState::JET_JUMP:
    case SoldierState::JET_HOVER:
        mAction = SoldierAnimType::JET;
        mActionTime = 0.0f;
        break;

    case SoldierState::FALL:
        mAction = SoldierAnimType::JUMP_FLY;
        if (mSoldierAction == SoldierState::FLY_RECOVER)
        {
            mActionTime = 2.0f;
        }
        else
        {
            mActionTime = 0.0f;
        }
        break;

    case SoldierState::FLY:
    {
        mRotSpeed = PblRandom::GetGlobalFloat() * 20.0f - 10.0f;
        const PblVector3& z = legMatrix.GetZAxis();
        const PblVector3& x = legMatrix.GetXAxis();
        float fForward = z.y * mMovement.y + z.x * mMovement.x + z.z * mMovement.z;
        float fSide = mMovement.y * x.y + mMovement.x * x.x + mMovement.z * x.z;
        mActionTime = 0.0f;
        if (fabsf(fForward) > fabsf(fSide))
        {
            mAction = fForward < 0.0f ? SoldierAnimType::FLY_BACKWARD : SoldierAnimType::FLY_FORWARD;
        }
        else
        {
            mAction = fSide < 0.0f ? SoldierAnimType::FLY_RIGHT : SoldierAnimType::FLY_LEFT;
        }
        break;
    }

    case SoldierState::TUMBLE:
    {
        mActionTime = 0.0f;
        float fX = mMovement.y * mLegMatrix.m[0][1] + mMovement.x * mLegMatrix.m[0][0] +
                   mMovement.z * mLegMatrix.m[0][2];
        float fZ = mLegMatrix.m[2][1] * mMovement.y + mLegMatrix.m[2][0] * mMovement.x +
                   mLegMatrix.m[2][2] * mMovement.z;
        if (fabsf(fX) > 0.05f || fabsf(fZ) > 0.05f)
        {
            mLegAngle = atan2f(fX, fZ);
        }
        else
        {
            mLegAngle = 0.0f;
        }
        const PblVector3& z = legMatrix.GetZAxis();
        if (mMovement.y * z.y + mMovement.x * z.x + mMovement.z * z.z > 0.0f)
        {
            mAction = SoldierAnimType::TUMBLE_FORWARD;
        }
        else
        {
            mAction = SoldierAnimType::TUMBLE_BACKWARD;
            mLegAngle -= 3.1415927f;
        }
        mLegAngle = UnitCircleNormalize(mLegAngle);
        break;
    }

    case SoldierState::FLY_RECOVER:
        mActionTime = 0.0f;
        if (mLegMatrix.m[2][1] * mMovement.y + mLegMatrix.m[2][0] * mMovement.x +
                mLegMatrix.m[2][2] * mMovement.z > 0.0f)
        {
            mAction = SoldierAnimType::TUMBLE_FORWARD;
        }
        else
        {
            mAction = SoldierAnimType::TUMBLE_BACKWARD;
            mLegAngle -= 3.1415927f;
        }
        mLegAngle = UnitCircleNormalize(mLegAngle);
        break;

    case SoldierState::BOUNCE:
    {
        const SoldierAnimation* pAnim =
            SoldierAnimatorClass::Instance()->GetLowerBodyActionAnimation(mAction, mWeaponAnimationMap);
        if (pAnim)
        {
            if (mSoldierAction == SoldierState::TUMBLE)
            {
                float fCycles = mActionTime /
                                ((float)(pAnim->GetAnimationNumFrames() - 1) * (1.0f / 30.0f) * 0.5f);
                float fPhase = fCycles - (float)(int)fCycles;
                bool bMid = fPhase > 0.4f && fPhase < 0.9f;
                if (mAction == SoldierAnimType::TUMBLE_FORWARD)
                {
                    mAction = bMid ? SoldierAnimType::LAND_BACK : SoldierAnimType::LAND_FRONT;
                }
                else
                {
                    mAction = bMid ? SoldierAnimType::LAND_FRONT : SoldierAnimType::LAND_BACK;
                }
            }
            else if (mRotAngle > 3.1415927f)
            {
                mAction = mActionTime < 0.5f ? SoldierAnimType::LAND_BACK : SoldierAnimType::BOUNCE_BACK;
            }
            else
            {
                mAction = mActionTime < 0.5f ? SoldierAnimType::LAND_FRONT : SoldierAnimType::BOUNCE_FRONT;
            }
        }
        else
        {
            mAction = SoldierAnimType::BOUNCE_FRONT;
        }
        mActionTime = 0.0f;
        break;
    }

    case SoldierState::CHOKE:
    case SoldierState::PILOT_ANIMATION:
    case SoldierState::ARMED_PASSENGER:
        mAnimationHash = uiAnimationHash;
        break;

    case SoldierState::DEAD:
        if (!m_bUseDirectionalDeaths)
        {
            mAction = SoldierAnimType::DIE_BACKWARD;
        }
        else if (mAction == SoldierAnimType::BOUNCE_FRONT || mAction == SoldierAnimType::LAND_FRONT ||
                 mAction == SoldierAnimType::GETUP_FRONT ||
                 (mAction == SoldierAnimType::MOVE && mPosture == SoldierAnimType::PRONE))
        {
            mAction = SoldierAnimType::DEAD_BELLY;
        }
        else if (mAction == SoldierAnimType::BOUNCE_BACK || mAction == SoldierAnimType::GETUP_BACK ||
                 mAction == SoldierAnimType::LAND_BACK)
        {
            mAction = SoldierAnimType::DEAD_ASS;
        }
        else
        {
            mOverlayTime = 0.0f;
            switch (m_eLastHitDir)
            {
            case 1:
                mAction = SoldierAnimType::DIE_LEFT;
                break;
            case 2:
                mAction = SoldierAnimType::DIE_BACKWARD;
                break;
            case 3:
                mAction = SoldierAnimType::DIE_RIGHT;
                break;
            default:
                mAction = SoldierAnimType::DIE_FORWARD;
                break;
            }
        }
        mActionTime = 0.0f;
        break;

    case SoldierState::HERO_FLEE:
        mPosture = SoldierAnimType::CROUCH;
        mAction = SoldierAnimType::DEAD_HERO_STUN;
        mActionTime = 0.0f;
        break;

    case SoldierState::SLIDE:
        mAction = SoldierAnimType::LAND_HARD;
        mActionTime = 0.0f;
        break;

    default:
        mAction = SoldierAnimType::MOVE;
        mActionTime = 0.0f;
        break;
    }

    mSoldierAction = eState;
    if (mAction == SoldierAnimType::MOVE && mPosture == SoldierAnimType::CROUCH)
    {
        m_fTakeKneeTimer = 0.0f;
    }
}

// FUNCTION: SWBF2 0x0063f7b0
void SoldierAnimator::SetWeaponAnimationMap(SoldierAnimationBank::MAP eMap)
{
    if (mWeaponAnimationMap == eMap)
    {
        return;
    }

    mWeaponState = 0;
    mWeaponAnimationMap = eMap;
    bool bMelee = SoldierAnimationBank::GetWeaponRoot(SoldierAnimationBank::GetMapWeapon(eMap)) ==
                  SoldierAnimationBank::MELEE;
    mWeaponMeleeAnim = NO_MELEE_ANIM;
    mWeaponMeleeAnimNext = NO_MELEE_ANIM;
    m_fAlertTimer = 0.0f;
    m_fAlertBlendDuration = 0.5f;
    mWeaponAnim = bMelee ? SoldierAnimType::WEAPON_MELEE : SoldierAnimType::WEAPON_IDLE;
    m_bAlert = false;
    m_bRestartWeaponAnim = false;
    m_bAnimControlledMove = false;
    m_bAnimVelocityMatched = false;
}

// FUNCTION: SWBF2 0x0063f810
void SoldierAnimator::SetWeaponState(int eState, float fTime, bool bRestart, bool bAlternate)
{
    m_bAnimControlledMove = false;
    SoldierAnimType::WeaponAnimation eAnim = SoldierAnimatorClass::WeaponStateToAnimation(eState, bAlternate);

    bool bIdleOk = bRestart;
    if (!bRestart &&
        (mAction == SoldierAnimType::MOVE || mAction == SoldierAnimType::JET || mAction == SoldierAnimType::LAND_SOFT) &&
        mWeaponAnim >= SoldierAnimType::SHOOT)
    {
        bIdleOk = mWeaponAnim <= SoldierAnimType::SHOOT_SECONDARY2;
    }

    if (eAnim == SoldierAnimType::WEAPON_IDLE && bIdleOk)
    {
        m_bRestartWeaponAnim = false;
        mWeaponState = eState;
        return;
    }

    mWeaponAnimationRate = 1.0f;
    if (eAnim == SoldierAnimType::WEAPON_IDLE)
    {
        m_bAnimVelocityMatched = false;
    }
    else
    {
        mWeaponAnimationRate = 1.0f;
        m_bAnimVelocityMatched = eAnim != mWeaponAnim;
        if (fTime > 0.0f && eState == 4)
        {
            const SoldierAnimation* pAnim =
                SoldierAnimatorClass::Instance()->GetUpperBodyWeaponAnimation(mPosture, mWeaponAnimationMap, eAnim);
            if (pAnim && (float)(pAnim->GetAnimationNumFrames() - 1) * (1.0f / 30.0f) > 0.0f)
            {
                mWeaponAnimationRate = (float)(pAnim->GetAnimationNumFrames() - 1) * (1.0f / 30.0f) / fTime;
            }
        }
    }

    mWeaponAnim = eAnim;
    m_bRestartWeaponAnim = bRestart;
    mWeaponState = eState;
}

// FUNCTION: SWBF2 0x0063f990
void SoldierAnimator::SetWeaponComboState(int eState, bool bRestart, unsigned char uiAnim, unsigned char uiAnimNext,
                                          bool bAnimControlledMove)
{
    if (eState == 0 && bRestart)
    {
        mWeaponState = eState;
        m_bRestartWeaponAnim = false;
        m_bAnimControlledMove = bAnimControlledMove;
        return;
    }

    mWeaponMeleeAnimNext = NO_MELEE_ANIM;
    if (uiAnim < NO_MELEE_ANIM)
    {
        if (uiAnim != mWeaponMeleeAnim)
        {
            m_bAnimVelocityMatched = true;
        }
        mWeaponMeleeAnim = uiAnim;
        if (uiAnimNext < NO_MELEE_ANIM)
        {
            mWeaponMeleeAnimNext = uiAnimNext;
        }
    }
    else
    {
        mWeaponMeleeAnim = NO_MELEE_ANIM;
        m_bAnimVelocityMatched = false;
    }
    m_bRestartWeaponAnim = bRestart;
    mWeaponState = eState;
    mWeaponAnimationRate = 1.0f;
    m_bAnimControlledMove = bAnimControlledMove;
}

// FUNCTION: SWBF2 0x0063fa70
void SoldierAnimator::SetAimDirection(const PblVector3& aimDir)
{
    PblVector3 vDir;
    D3DXVec3Normalize(&vDir, &aimDir);
    mAimDir = vDir;
}

// FUNCTION: SWBF2 0x0063faa0
void SoldierAnimator::SetupPose(RedPose& pose)
{
    float fDeltaTime = (NetGame::IsEnabled() || !GameLoop::IsGameOver()) ? GameLoop::sClientDeltaTime : 0.0f;

    int iFrame = RedRenderer::GetFrameNumber();
    if (mLastFrameUsed == iFrame)
    {
        pose.ConvertFromZephyrPose(mZephyrSkeleton);
        return;
    }
    if (mLastFrameUsed < iFrame - 5)
    {
        m_bFirstTime = true;
        mBlendFactorLower = 0.0f;
        mBlendFactorUpper = 0.0f;
    }
    unsigned int uiLastTurn = mNetTurn;
    mLastFrameUsed = iFrame;
    mNetTurn = NetGame::GetTurnNumber();

    if (fDeltaTime <= 0.0f || (mNetTurn == uiLastTurn && netFrameLock))
    {
        pose.ConvertFromZephyrPose(mZephyrSkeleton);
        mExtraDt += fDeltaTime;
        return;
    }

    if (fDeltaTime > 0.0f)
    {
        fDeltaTime += mExtraDt;
        mExtraDt = 0.0f;
        m_fAlertTimer += fDeltaTime;
    }

    const ZephyrAnim* pOverlayAnim = 0;
    float fOverlayBlend = 0.0f;
    if (mOverlayTime > 0.0f)
    {
        pOverlayAnim = mOverlayAnim;
        fOverlayBlend = mOverlayBlendFactor * (mOverlayTime < 1.0f ? mOverlayTime : 1.0f);
        mOverlayTime -= fDeltaTime * OVERLAY_FADEOUT_RATE;
        if (mOverlayTime < 0.0f)
        {
            mOverlayTime = 0.0f;
        }
    }

    if (mAnimationHash &&
        (mSoldierAction == SoldierState::PILOT_ANIMATION || mSoldierAction == SoldierState::CHOKE))
    {
        const ZephyrAnim* pAnim = SoldierAnimatorClass::Instance()->GetAnimation(mAnimationHash, 0);
        if (pAnim)
        {
            UpdateUpperBodyAnimation(fDeltaTime, pAnim, false, 0.0f, 0);
            UpdateLowerBodyAnimation(fDeltaTime, pAnim, false, 0.0f);
            if (mForceFrameNum1 >= 0.0f)
            {
                mZephyrPoseDynLower.SetLoop(false);
                mZephyrPoseDynLower.SetFrameNumFloat(mForceFrameNum1);
                mZephyrPoseStatic.Set(mZephyrPoseDynLower, true);
                mZephyrPoseDynLower.SetFrameNumFloat(mForceFrameNum2);
                mZephyrPoseStatic.Blend(mZephyrPoseDynLower, mForceFrameBlend);
            }
            else
            {
                mZephyrPoseStatic.Set(mZephyrPoseDynLower, true);
            }
            if (mSoldierAction != SoldierState::CHOKE && pOverlayAnim)
            {
                ZephyrPoseDyn<32> overlayPose;
                overlayPose.Open(mZephyrSkeleton);
                overlayPose.SetAnimation(pOverlayAnim, 30.0f);
                mZephyrPoseStatic.Blend(overlayPose, mUpperBodyAnimMask, fOverlayBlend);
            }
            mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
        }
        pose.ConvertFromZephyrPose(mZephyrSkeleton);
        return;
    }

    SoldierAnimatorClass* pClass = SoldierAnimatorClass::Instance();
    pClass->UpdateLRU(this);

    bool bRotate = false;
    switch (mAction)
    {
    case SoldierAnimType::SPRINT:
    {
        const SoldierAnimation* pAnim =
            pClass->GetLowerBodyActionAnimation(SoldierAnimType::SPRINT, mWeaponAnimationMap);
        float fAngle = 0.0f;
        float fSpeed = 1.0f;
        if (fDeltaTime > 0.0f)
        {
            bool bFirstPerson = false;
            if (mOwner)
            {
                Tracker* pTracker = mOwner->GetTracker();
                if (pTracker && pTracker->IsFirstPersonView() &&
                    NetGame::GetJoystickIndex(mOwner->GetPlayerId()) >= 0)
                {
                    bFirstPerson = true;
                }
            }
            const PblVector3& z = mLegMatrix.GetZAxis();
            const PblVector3& x = mLegMatrix.GetXAxis();
            float fForward = (z.y * mMovement.y + mMovement.x * z.x + z.z * mMovement.z) / fDeltaTime;
            float fSide = (mMovement.y * x.y + x.x * mMovement.x + mMovement.z * x.z) / fDeltaTime;
            fAngle = atan2f(fSide, fForward);
            fSpeed = sqrtf(fSide * fSide + fForward * fForward) / pAnim->GetVelocityZ();
            if (bFirstPerson)
            {
                fSpeed *= FIRST_PERSON_ANIMATION_SCALE;
            }
        }
        UpdateActionAnimation(mAction, fDeltaTime, fSpeed, fAngle, 0.0f, false);
        mMoveAnimation = SoldierAnimType::RUN_FORWARD;
        break;
    }

    case SoldierAnimType::JUMP_UP:
    case SoldierAnimType::JUMP_FORWARD:
    case SoldierAnimType::JUMP_BACKWARD:
    case SoldierAnimType::JUMP_LEFT:
    case SoldierAnimType::JUMP_RIGHT:
    {
        const SoldierAnimation* pAnim = pClass->GetLowerBodyActionAnimation(mAction, mWeaponAnimationMap);
        float fLength;
        if (pAnim && pAnim->GetAnimationNumFrames() > 1)
        {
            fLength = (float)(pAnim->GetAnimationNumFrames() - 1) * ANIM_FRAME_SPEED;
        }
        else
        {
            fLength = 0.3f;
        }
        if (mActionTime > fLength &&
            (mWeaponMeleeAnim == NO_MELEE_ANIM || mWeaponAnim != SoldierAnimType::WEAPON_MELEE))
        {
            mAction = SoldierAnimType::JUMP_FLY;
        }
        UpdateActionAnimation(mAction, fDeltaTime, 1.0f, 0.0f, 0.0f, false);
        break;
    }

    case SoldierAnimType::JUMP_FLY:
        if (mSoldierAction == SoldierState::FALL && mActionTime > 3.0f)
        {
            mAction = SoldierAnimType::FLY_FLAIL;
        }
        UpdateActionAnimation(mAction, fDeltaTime, 1.0f, 0.0f, 0.0f, false);
        break;

    case SoldierAnimType::FLY_FLAIL:
        if (mSoldierAction != SoldierState::FALL)
        {
            bRotate = true;
        }
        UpdateActionAnimation(SoldierAnimType::FLY_FLAIL, fDeltaTime, 1.0f, 0.0f, 0.0f, false);
        break;

    case SoldierAnimType::FLY_FORWARD:
    case SoldierAnimType::FLY_BACKWARD:
    case SoldierAnimType::FLY_LEFT:
    case SoldierAnimType::FLY_RIGHT:
        bRotate = true;
        if (mActionTime > 0.3f)
        {
            mAction = SoldierAnimType::FLY_FLAIL;
        }
        else
        {
            const PblVector3& z = mLegMatrix.GetZAxis();
            const PblVector3& x = mLegMatrix.GetXAxis();
            float fForward = z.y * mMovement.y + z.x * mMovement.x + z.z * mMovement.z;
            float fSide = mMovement.y * x.y + mMovement.x * x.x + mMovement.z * x.z;
            if (fabsf(fForward) > fabsf(fSide))
            {
                mAction = fForward < 0.0f ? SoldierAnimType::FLY_BACKWARD : SoldierAnimType::FLY_FORWARD;
            }
            else
            {
                mAction = fSide < 0.0f ? SoldierAnimType::FLY_RIGHT : SoldierAnimType::FLY_LEFT;
            }
        }
        UpdateActionAnimation(mAction, fDeltaTime, 1.0f, 0.0f, 0.0f, false);
        break;

    case SoldierAnimType::TUMBLE_FORWARD:
    case SoldierAnimType::TUMBLE_BACKWARD:
    {
        const PblVector3& z = mLegMatrix.GetZAxis();
        const PblVector3& x = mLegMatrix.GetXAxis();
        if (mSoldierAction == SoldierState::FLY_RECOVER)
        {
            float fZ = mMovement.y * z.y + mMovement.x * z.x + mMovement.z * z.z;
            float fX = mMovement.y * x.y + x.x * mMovement.x + mMovement.z * x.z;
            if (fX * fX + fZ * fZ > 0.01f)
            {
                mLegAngle = atan2f(fX, fZ);
                if (mAction == SoldierAnimType::TUMBLE_BACKWARD)
                {
                    mLegAngle -= 3.1415927f;
                }
                mLegAngle = UnitCircleNormalize(mLegAngle);
            }
            UpdateActionAnimation(mAction, fDeltaTime, 1.0f, mLegAngle, 0.0f, false);
        }
        else
        {
            float fZ = z.y * mMovement.y + z.x * mMovement.x + z.z * mMovement.z;
            float fX = mMovement.y * x.y + x.x * mMovement.x + mMovement.z * x.z;
            if (fabsf(fX) > 0.05f || fabsf(fZ) > 0.05f)
            {
                mLegAngle = atan2f(fX, fZ);
                if (mAction == SoldierAnimType::TUMBLE_BACKWARD)
                {
                    mLegAngle -= 3.1415927f;
                }
                mLegAngle = UnitCircleNormalize(mLegAngle);
            }
            float fSpeed;
            if (fDeltaTime > 0.0f)
            {
                fSpeed = sqrtf(mMovement.x * mMovement.x + mMovement.y * mMovement.y + mMovement.z * mMovement.z) /
                         (fDeltaTime * 10.0f);
            }
            else
            {
                fSpeed = 0.0f;
            }
            if (fDeltaTime > 0.0f)
            {
                UpdateActionAnimation(mAction, fDeltaTime, fSpeed, mLegAngle, 0.0f, false);
            }
        }
        break;
    }

    case SoldierAnimType::BOUNCE_FRONT:
    case SoldierAnimType::BOUNCE_BACK:
    case SoldierAnimType::LAND_FRONT:
    case SoldierAnimType::LAND_BACK:
        UpdateActionAnimation(mAction, fDeltaTime, 1.0f, mLegAngle, 0.0f, false);
        break;

    case SoldierAnimType::DEAD_BELLY:
    case SoldierAnimType::DEAD_ASS:
    case SoldierAnimType::DEAD_HERO_STUN:
    case SoldierAnimType::DIE_FORWARD:
    case SoldierAnimType::DIE_BACKWARD:
    case SoldierAnimType::DIE_LEFT:
    case SoldierAnimType::DIE_RIGHT:

        if (m_bFirstTime)
        {
            fDeltaTime = 2.0f;
        }
        UpdateActionAnimation(mAction, fDeltaTime, 1.0f, mLegAngle, 0.0f, false);
        break;

    case SoldierAnimType::DIVE:
    {
        const PblVector3& z = mLegMatrix.GetZAxis();
        const PblVector3& x = mLegMatrix.GetXAxis();
        float fAngle = atan2f(mMovement.y * x.y + mMovement.x * x.x + mMovement.z * x.z,
                              z.y * mMovement.y + z.x * mMovement.x + z.z * mMovement.z);
        UpdateActionAnimation(SoldierAnimType::DIVE, fDeltaTime, 1.0f, fAngle, 0.0f, false);
        break;
    }

    case SoldierAnimType::JET:
        UpdateActionAnimation(SoldierAnimType::JET, fDeltaTime, 1.0f, 0.0f, 0.0f, true);
        break;

    case SoldierAnimType::LAND_SOFT:
        if (UpdateActionAnimation(SoldierAnimType::LAND_SOFT, fDeltaTime, 1.0f, mLegAngle * 0.9f, 0.0f, true))
        {
            mAction = SoldierAnimType::MOVE;
        }
        else
        {
            Controllable* pControllable = mOwner->mCharacter ? mOwner->mCharacter->GetControllable() : 0;
            bool bMoving = pControllable &&
                           (pControllable->mControlMove != 0.0f || pControllable->mControlStrafe != 0.0f);
            if (fDeltaTime > 0.0f && mActionTime > 0.0f && (m_bAnimControlledMove || bMoving))
            {
                mAction = SoldierAnimType::MOVE;
            }
        }
        break;

    case SoldierAnimType::LAND_HARD:
        if (UpdateActionAnimation(SoldierAnimType::LAND_HARD, fDeltaTime, 1.0f, mLegAngle * 0.9f, 0.0f, false))
        {
            mAction = SoldierAnimType::MOVE;
        }
        else if (fDeltaTime > 0.0f && mActionTime > 0.5f &&
                 (m_bAnimControlledMove || mMovement.x * mMovement.x + mMovement.z * mMovement.z >
                                               fDeltaTime * 0.3f * 0.3f * fDeltaTime))
        {
            mAction = SoldierAnimType::MOVE;
        }
        break;

    case SoldierAnimType::GETUP_FRONT:
    case SoldierAnimType::GETUP_BACK:
    case SoldierAnimType::PRONE_TO_STAND:
    case SoldierAnimType::CROUCH_TO_PRONE:
    case SoldierAnimType::PRONE_TO_CROUCH:
        if (UpdateActionAnimation(mAction, fDeltaTime, 1.0f, mLegAngle * 0.9f, 0.0f, false) ||
            (fDeltaTime > 0.0f && mActionTime > 0.3f &&
             mMovement.x * mMovement.x + mMovement.z * mMovement.z > fDeltaTime * 0.3f * 0.3f * fDeltaTime))
        {
            mAction = SoldierAnimType::MOVE;
            if (mPosture == SoldierAnimType::CROUCH)
            {
                m_fTakeKneeTimer = 0.0f;
            }
        }
        break;

    case SoldierAnimType::MOVE:
        UpdateMovementAnimation(fDeltaTime, pOverlayAnim, fOverlayBlend);
        break;

    default:
        break;
    }

    if (mAction != SoldierAnimType::MOVE && mAction != SoldierAnimType::SPRINT)
    {
        mMoveAnimation = SoldierAnimType::IDLE;
    }

    ApplyProceduralAnimationAndBuildWorldMatrices(fDeltaTime);
    pose.ConvertFromZephyrPose(mZephyrSkeleton);

    m_bFirstTime = false;
    mActionTime += fDeltaTime;
    if (bRotate)
    {
        mRotAngle += mRotSpeed * fDeltaTime;
        if (mRotAngle > PblMath::fTwoPi)
        {
            mRotAngle -= PblMath::fTwoPi;
        }
        else if (mRotAngle < 0.0f)
        {
            mRotAngle += PblMath::fTwoPi;
        }
    }
    else
    {
        mRotAngle = 0.0f;
    }
}

namespace
{

inline PblMatrix& RotateJointWorldMatrix(ZephyrSkeleton<32>& kSkel, int iJoint, float fYaw, float fPitch,
                                         PblBitVector<32>& kDirty, const PblMatrix* pParent)
{
    PblMatrix mRotate;
    PblMatrix mTemp;
    D3DXMatrixRotationY(&mRotate, fYaw);
    D3DXMatrixMultiply(&mRotate, &mRotate, D3DXMatrixRotationX(&mTemp, fPitch));
    kSkel.BuildWorldMatrices(_IdentityMatrix, iJoint, kDirty);

    PblMatrix& mWorld = kSkel.m_pWorldMatrices[iJoint];
    PblVector3 vOrigin;
    if (pParent)
    {
        const ZephyrTransform& kBase = *(const ZephyrTransform*)kSkel.m_pShared->m_pJoints[iJoint].m_kBaseTransform;
        D3DXVec3TransformCoord(&vOrigin, &kBase.kTrans, pParent);
    }
    else
    {
        vOrigin = mWorld.GetTranslation();
    }
    D3DXMatrixMultiply(&mTemp, &mWorld, &mRotate);
    mWorld = mTemp;
    mWorld.m[3][0] = vOrigin.x;
    mWorld.m[3][1] = vOrigin.y;
    mWorld.m[3][2] = vOrigin.z;
    mWorld.m[3][3] = 1.0f;
    return mWorld;
}
}

// FUNCTION: SWBF2 0x00642860
void SoldierAnimator::ApplyProceduralAnimationAndBuildWorldMatrices(float fDeltaTime)
{
    if (!(m_uiAimState & 0x33))
    {
        mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
        return;
    }

    float fBlend;
    if (mAnimationTimerUpper < mBlendDurationRotate)
    {
        fBlend = mAnimationTimerUpper / mBlendDurationRotate;
    }
    else
    {
        m_uiAimState &= ~0x30;
        if (!(m_uiAimState & 0x33))
        {
            mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
            return;
        }
        fBlend = 1.0f;
    }

    unsigned int uiAimState = m_uiAimState;
    float fHeadWeight = 0.0f;
    float fTorsoWeight = 0.0f;
    if (uiAimState & 1)
    {
        if ((uiAimState & 0x30) == 0x30)
        {
            fHeadWeight = fBlend;
            fTorsoWeight = 1.0f - fBlend;
        }
        else
        {
            fHeadWeight = (uiAimState & 0x10) ? fBlend : 1.0f;
        }
    }
    else if (uiAimState & 2)
    {
        if ((uiAimState & 0x30) == 0x30)
        {
            fTorsoWeight = fBlend;
            fHeadWeight = 1.0f - fBlend;
        }
        else
        {
            fTorsoWeight = (uiAimState & 0x10) ? fBlend : 1.0f;
        }
    }
    else if (uiAimState & 0x10)
    {
        fHeadWeight = 1.0f - fBlend;
    }
    else
    {
        fTorsoWeight = 1.0f - fBlend;
    }

    float fPitchMin = PblMath::fHalfPi * -0.7f;
    float fPitchMax = PblMath::fHalfPi * 0.7f;
    float fYawMin = PblMath::fHalfPi * -0.4f;
    float fYawMax = PblMath::fHalfPi * 0.4f;
    if (mOwner && mOwner->mCharacter)
    {
        if (const Controllable* pControllable = mOwner->mCharacter->GetControllable())
        {
            if (pControllable->GetControllableClass()->mControlsUnit)
            {
                pControllable->GetPitchYawLimits(fPitchMin, fPitchMax, fYawMin, fYawMax);
            }
        }
    }

    float fYaw = atan2f(mLegMatrix.GetXAxis().x * mAimDir.x + mLegMatrix.GetXAxis().y * mAimDir.y +
                            mLegMatrix.GetXAxis().z * mAimDir.z,
                        mLegMatrix.GetZAxis().x * mAimDir.x + mLegMatrix.GetZAxis().y * mAimDir.y +
                            mLegMatrix.GetZAxis().z * mAimDir.z);
    float fPitch = -asinf(mLegMatrix.GetYAxis().x * mAimDir.x + mLegMatrix.GetYAxis().y * mAimDir.y +
                          mLegMatrix.GetYAxis().z * mAimDir.z);

    float fLegAngle = mLegAngle;
    float fYawOffset = fYaw - fLegAngle;
    float fClampedYaw = (fYawOffset < fYawMin) ? fYawMin : (fYawOffset > fYawMax) ? fYawMax : fYawOffset;
    float fClampedPitch = (fPitch < fPitchMin) ? fPitchMin : (fPitch > fPitchMax) ? fPitchMax : fPitch;
    float fRate;
    if (mSoldierAction == SoldierState::ARMED_PASSENGER || (fRate = fDeltaTime * 4.5f) >= 1.0f)
    {
        mAimYAngle = fClampedYaw + fLegAngle;
        mAimXAngle = fClampedPitch;
    }
    else
    {
        mAimYAngle += (fClampedYaw + fLegAngle - mAimYAngle) * fRate;
        mAimXAngle += (fClampedPitch - mAimXAngle) * fRate;
    }
    if (mPosture == SoldierAnimType::PRONE && mAimXAngle < 0.0f)
    {
        mAimXAngle = 0.0f;
    }

    float fAimYaw = mAimYAngle - fLegAngle;
    float fAimPitch = mAimXAngle;
    PblBitVector<32> kDirty;
    kDirty.SetAll();
    const ZephyrSkeletonShared* pShared;

    if (fTorsoWeight > 0.0f)
    {
        float fTorsoYaw = fAimYaw * fTorsoWeight;
        float fTorsoPitch = fAimPitch * fTorsoWeight;
        const PblMatrix* pSpine;
        pShared = mZephyrSkeleton.m_pShared;
        int iJoint = pShared->GetJointIdxByCrc(0x5de52389);
        if (iJoint >= 0)
        {
            const PblMatrix& mLower =
                RotateJointWorldMatrix(mZephyrSkeleton, iJoint, fTorsoYaw * 0.125f, fTorsoPitch * 0.125f, kDirty, 0);
            pSpine = &RotateJointWorldMatrix(mZephyrSkeleton, mZephyrSkeleton.m_pShared->GetJointIdxByCrc(0x8c1c9bba),
                                             fTorsoYaw * 0.375f, fTorsoPitch * 0.375f, kDirty, &mLower);
        }
        else
        {
            pSpine = &RotateJointWorldMatrix(mZephyrSkeleton, pShared->GetJointIdxByCrc(0x93e93819),
                                             fTorsoYaw * 0.5f, fTorsoPitch * 0.5f, kDirty, 0);
        }
        RotateJointWorldMatrix(mZephyrSkeleton, mZephyrSkeleton.m_pShared->GetJointIdxByCrc(0xe5b51072),
                               fTorsoYaw * 0.5f, fTorsoPitch * 0.5f, kDirty, pSpine);
    }

    if (fHeadWeight > 0.0f)
    {
        pShared = mZephyrSkeleton.m_pShared;
        int iNeck = pShared->GetJointIdxByCrc(0xd2afb28a);
        if (iNeck >= 0)
        {
            float fHeadYaw = fAimYaw * fHeadWeight;
            float fHeadPitch = fAimPitch * fHeadWeight;
            const PblMatrix& mNeck =
                RotateJointWorldMatrix(mZephyrSkeleton, iNeck, fHeadYaw * 0.25f, fHeadPitch * 0.25f, kDirty, 0);
            RotateJointWorldMatrix(mZephyrSkeleton, mZephyrSkeleton.m_pShared->GetJointIdxByCrc(0x8afdf5e4),
                                   fHeadYaw * 0.375f, fHeadPitch * 0.375f, kDirty, &mNeck);
        }
        else
        {
            RotateJointWorldMatrix(mZephyrSkeleton, pShared->GetJointIdxByCrc(0x8afdf5e4),
                                   fAimYaw * fHeadWeight * 0.625f, fAimPitch * fHeadWeight * 0.625f, kDirty, 0);
        }
    }

    mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix, kDirty);
}

// FUNCTION: SWBF2 0x00640860
bool SoldierAnimator::UpdateActionAnimation(SoldierAnimType::ActionAnimation eAction, float fDeltaTime,
                                            float fSpeed, float fAngle, float, bool bWeaponUpper)
{
    const SoldierAnimation* pUpper = 0;
    const SoldierAnimation* pLower = 0;
    bool bRestartWeaponAnim = m_bRestartWeaponAnim;
    if (m_bRestartJumpForJedi)
    {
        m_bResetUpperAnim = true;
        m_bResetLowerAnim = true;
    }
    m_bRestartWeaponAnim = false;
    m_bRestartJumpForJedi = false;

    if (mWeaponAnim == SoldierAnimType::WEAPON_MELEE && mWeaponMeleeAnim != NO_MELEE_ANIM)
    {
        pUpper = SoldierAnimatorClass::Instance()->GetUpperBodyAnimation(mWeaponAnimationMap, mWeaponMeleeAnim);
        if (m_bAnimControlledMove)
        {
            pLower = SoldierAnimatorClass::Instance()->GetLowerBodyAnimation(mWeaponAnimationMap, mWeaponMeleeAnim);
        }
    }

    float fLegAngle = mLegAngle;
    if (pLower)
    {
        if (bRestartWeaponAnim && pLower->GetAnimation() == mZephyrPoseDynLower.m_kAnim.m_pkAnim)
        {
            m_bResetLowerAnim = true;
        }
        UpdateLowerBodyAnimation(mWeaponAnimationRate * fDeltaTime, pLower);
    }
    else
    {
        pLower = SoldierAnimatorClass::Instance()->GetLowerBodyActionAnimation(eAction, mWeaponAnimationMap);
        if (!pLower)
        {
            return true;
        }
        UpdateLowerBodyAnimation(fDeltaTime * fSpeed, pLower);
    }

    bool bFinished = true;
    const int iLastFrame = mZephyrPoseDynLower.m_kAnim.m_pkAnim->GetNumFrames() - 1;
    if ((m_pAnimLower && m_pAnimLower->m_pAnimData->m_uiLoop != SoldierAnimationData::LOOP_END) ||
        (int)((float)iLastFrame * mZephyrPoseDynLower.m_fCurT) < iLastFrame)
    {
        bFinished = false;
    }

    if (bWeaponUpper && !pUpper)
    {
        if (mWeaponAnim == SoldierAnimType::WEAPON_MELEE)
        {
            if (mWeaponMeleeAnim != NO_MELEE_ANIM)
            {
                pUpper = SoldierAnimatorClass::Instance()->GetUpperBodyAnimation(mWeaponAnimationMap, mWeaponMeleeAnim);
            }
        }
        else if (mWeaponAnim != SoldierAnimType::WEAPON_IDLE)
        {
            pUpper = SoldierAnimatorClass::Instance()->GetUpperBodyWeaponAnimation(SoldierAnimType::STAND,
                                                                                   mWeaponAnimationMap, mWeaponAnim);
        }
    }
    if (pUpper)
    {
        if (bRestartWeaponAnim && pUpper->GetAnimation() == mZephyrPoseDynUpper.m_kAnim.m_pkAnim)
        {
            m_bResetUpperAnim = true;
        }
        UpdateUpperBodyAnimation(mWeaponAnimationRate * fDeltaTime, pUpper);
        if (pUpper->m_pAnimData->m_uiLoop == SoldierAnimationData::LOOP_END && mZephyrPoseDynUpper.m_bAnimFinished)
        {
            if (mWeaponAnim == SoldierAnimType::WEAPON_MELEE)
            {
                mWeaponMeleeAnim = mWeaponMeleeAnimNext;
                mWeaponMeleeAnimNext = NO_MELEE_ANIM;
            }
            else
            {
                mWeaponAnim = SoldierAnimType::WEAPON_IDLE;
            }
        }
    }
    else
    {
        UpdateUpperBodyAnimation(fDeltaTime * fSpeed,
                                 SoldierAnimatorClass::Instance()->GetUpperBodyActionAnimation(eAction,
                                                                                               mWeaponAnimationMap));
    }

    float fUpperBlend = GetUpperBodyBlendFactor();
    if (m_bFirstTime || fUpperBlend >= 1.0f)
    {
        mZephyrPoseStatic.Set(mZephyrPoseDynUpper, mUpperBodyAnimMask, true);
    }
    else if (fUpperBlend > 0.0f)
    {
        mZephyrPoseStatic.Blend(mZephyrPoseDynUpper, mUpperBodyAnimMask, fUpperBlend);
    }

    float fLowerBlend = GetLowerBodyBlendFactor();
    if (m_bFirstTime || fLowerBlend >= 1.0f)
    {
        mZephyrPoseStatic.Set(mZephyrPoseDynLower, mLowerBodyAnimMask, true);
    }
    else if (fLowerBlend > 0.0f)
    {
        mZephyrPoseStatic.Blend(mZephyrPoseDynLower, mLowerBodyAnimMask, fLowerBlend);
    }

    float fTarget = (m_uiAimState & 4) ? 0.0f : fAngle;
    float fFactor = fDeltaTime * MOVEANGLE_BLEND_RATE;
    if (fFactor > 1.0f)
    {
        fFactor = 1.0f;
    }
    if (fFactor > 0.0f)
    {
        fLegAngle += (fTarget - fLegAngle) * fFactor;
    }
    mLegDirection = mLegMatrix.GetZAxis();
    mLegAngle = fLegAngle;
    RotateLowerBody(fLegAngle);

    if (mRotAngle > 0.0f)
    {
        const PblVector3& z = mLegMatrix.GetZAxis();
        const PblVector3& x = mLegMatrix.GetXAxis();
        float fForward = mMovement.y * z.y + mMovement.x * z.x + mMovement.z * z.z;
        float fSide = x.x * mMovement.x + mMovement.y * x.y + mMovement.z * x.z;
        unsigned short uiRoot = (unsigned short)mZephyrSkeleton.m_pShared->GetJointIdxByCrc(0x16e27226);
        PblVector3 vTravel(fabsf(fForward), 0.0f, fabsf(fSide));
        PblVector3 vAxis;
        if (vTravel.x * vTravel.x + vTravel.y * vTravel.y + vTravel.z * vTravel.z > 0.0001f)
        {
            D3DXVec3Normalize(&vAxis, &vTravel);
        }
        else
        {
            vAxis = PblVector3(1.0f, 0.0f, 0.0f);
        }
        mZephyrPoseStatic.m_pJoints[uiRoot].kQuat = PblQuaternion_FromAxisAngle(vAxis, mRotAngle);
    }
    return bFinished;
}

// FUNCTION: SWBF2 0x00640db0
void SoldierAnimator::UpdateMovementAnimation(float fDeltaTime, const ZephyrAnim* pAnim, float fBlend)
{
    bool bFirstPerson = false;
    float fRate = fDeltaTime;
    float fAnimDeltaTime = fDeltaTime;
    if (mOwner)
    {
        Tracker* pTracker = mOwner->GetTracker();
        if (pTracker && pTracker->IsFirstPersonView() && NetGame::GetJoystickIndex(mOwner->GetPlayerId()) >= 0)
        {
            bFirstPerson = true;
        }
    }

    float fLegAngle = mLegAngle;
    bool bRestartWeaponAnim = m_bRestartWeaponAnim;
    m_bRestartWeaponAnim = false;
    bool bAlert = m_bAlert;

    SoldierAnimType::MovementAnimation eMove = SoldierAnimType::IDLE;
    SoldierAnimType::MovementAnimation eNewMove = SoldierAnimType::IDLE;
    float fMoveAngle = 0.0f;
    float fForward = 0.0f;
    float fSide = 0.0f;
    float fTurn = 0.0f;

    if (fDeltaTime > 0.0f)
    {
        if (mSoldierAction == SoldierState::ARMED_PASSENGER)
        {
            if (mMoveAnimation <= SoldierAnimType::IDLE_LOOK_AROUND)
            {
                eMove = mMoveAnimation;
            }
            fLegAngle = 0.0f;
            mLegDirection = mLegMatrix.GetZAxis();
        }
        else
        {
            const PblVector3& z = mLegMatrix.GetZAxis();
            const PblVector3& x = mLegMatrix.GetXAxis();
            fForward = (z.y * mMovement.y + z.x * mMovement.x + z.z * mMovement.z) / fDeltaTime;
            fSide = (mMovement.y * x.y + x.x * mMovement.x + mMovement.z * x.z) / fDeltaTime;
            eNewMove = GetMovementAnimation(fSide, fForward, fMoveAngle, fRate);
            if (bFirstPerson && eNewMove > SoldierAnimType::TURN_RIGHT)
            {
                fAnimDeltaTime = fRate * FIRST_PERSON_ANIMATION_SCALE;
            }
            else
            {
                fAnimDeltaTime = fRate;
            }

            eMove = eNewMove;
            if (eNewMove == SoldierAnimType::SLIDE)
            {
                eMove = SoldierAnimType::IDLE;
            }
            else if (eNewMove == SoldierAnimType::IDLE)
            {
                if (mMoveAnimation <= SoldierAnimType::TURN_RIGHT)
                {
                    eMove = mMoveAnimation;
                }
                const PblVector3& dir = mLegDirection;
                fLegAngle += atan2f(dir.y * x.y + dir.x * x.x + dir.z * x.z, z.y * dir.y + z.x * dir.x + z.z * dir.z);
                if (eMove <= SoldierAnimType::IDLE_LOOK_AROUND)
                {

                    static const float sTurnLeftLimit[SoldierAnimType::NUM_STANDARD_POSTURES] = {
                        PblMath::fHalfPi * 0.25f, PblMath::fHalfPi * 0.25f, PblMath::fHalfPi * 0.3f };
                    static const float sTurnRightLimit[SoldierAnimType::NUM_STANDARD_POSTURES] = {
                        PblMath::fHalfPi * -0.45f, PblMath::fHalfPi * -0.45f, PblMath::fHalfPi * -0.3f };
                    if (fLegAngle > sTurnLeftLimit[mPosture])
                    {
                        eMove = SoldierAnimType::TURN_RIGHT;
                        fTurn = -fLegAngle - 0.2f;
                    }
                    else if (fLegAngle < sTurnRightLimit[mPosture])
                    {
                        eMove = SoldierAnimType::TURN_LEFT;
                        fTurn = 0.2f - fLegAngle;
                    }
                }
            }
            mLegDirection = mLegMatrix.GetZAxis();
        }
    }
    else
    {
        eMove = mMoveAnimation;
        if (eMove > SoldierAnimType::TURN_RIGHT)
        {
            eNewMove = eMove;
        }
    }

    bool bTakeKnee = mPosture == SoldierAnimType::CROUCH && eMove == SoldierAnimType::IDLE_TAKEKNEE;
    bool bWeaponUpper = false;
    bool bAnimControlledMove = false;
    SoldierAnimatorClass* pClass = SoldierAnimatorClass::Instance();
    const SoldierAnimation* pUpper = 0;
    if (mWeaponAnim == SoldierAnimType::WEAPON_MELEE)
    {
        if (mWeaponMeleeAnim != NO_MELEE_ANIM)
        {
            pUpper = pClass->GetUpperBodyAnimation(mWeaponAnimationMap, mWeaponMeleeAnim);
            bAlert = false;
        }
    }
    else if (mWeaponAnim != SoldierAnimType::WEAPON_IDLE)
    {
        pUpper = pClass->GetUpperBodyWeaponAnimation(mPosture, mWeaponAnimationMap, mWeaponAnim);
    }

    if (pUpper)
    {
        bWeaponUpper = true;
        if (eMove <= SoldierAnimType::IDLE_LOOK_AROUND && !bTakeKnee)
        {
            eMove = SoldierAnimType::IDLE;
        }
        bAnimControlledMove = m_bAnimControlledMove;
    }
    else if (eNewMove == SoldierAnimType::SLIDE)
    {
        pUpper = pClass->GetUpperBodyActionAnimation(SoldierAnimType::LAND_HARD, mWeaponAnimationMap);
    }
    else
    {
        pUpper = pClass->GetUpperBodyMovementAnimation(eMove, mPosture, mWeaponAnimationMap, bAlert);
    }

    const SoldierAnimation* pLower = 0;
    bool bWeaponLower = false;
    if (mSoldierAction != SoldierState::ARMED_PASSENGER)
    {
        if (bWeaponUpper && m_bAnimVelocityMatched)
        {
            if (mWeaponAnim == SoldierAnimType::WEAPON_MELEE)
            {
                if (mWeaponMeleeAnim != NO_MELEE_ANIM)
                {
                    pLower = pClass->GetLowerBodyAnimation(mWeaponAnimationMap, mWeaponMeleeAnim);
                    if (pLower && !bAnimControlledMove)
                    {
                        float fDX = fSide - pLower->GetVelocityX();
                        float fDZ = fForward - pLower->GetVelocityZ();
                        if (fDX * fDX + fDZ * fDZ > 0.09f)
                        {
                            pLower = 0;
                        }
                    }
                }
            }
            else if (mWeaponAnim != SoldierAnimType::WEAPON_IDLE &&
                     (bAnimControlledMove || (!bTakeKnee && eNewMove == SoldierAnimType::IDLE)))
            {
                pLower = pClass->GetLowerBodyWeaponAnimation(mPosture, mWeaponAnimationMap, mWeaponAnim);
            }

            if (!pLower)
            {
                m_bAnimVelocityMatched = false;
            }
            if (pLower)
            {
                bWeaponLower = true;
                if (bAnimControlledMove)
                {
                    eMove = SoldierAnimType::IDLE;
                    eNewMove = SoldierAnimType::IDLE;
                }
            }
        }

        if (!bWeaponLower)
        {
            if (eNewMove == SoldierAnimType::SLIDE)
            {
                eMove = SoldierAnimType::IDLE;
                pLower = pClass->GetLowerBodyActionAnimation(SoldierAnimType::LAND_HARD, mWeaponAnimationMap);
                UpdateLowerBodyAnimation(fDeltaTime, pLower);
            }
            else
            {
                pLower = pClass->GetLowerBodyMovementAnimation(eMove, mPosture, mWeaponAnimationMap, bAlert);

                if (UpdateLowerBodyAnimation(fAnimDeltaTime, pLower) && m_bFirstTime &&
                    mMoveAnimation <= SoldierAnimType::IDLE_LOOK_AROUND)
                {
                    UpdateLowerBodyAnimation((float)(pLower->GetAnimationNumFrames() - 1) * ANIM_FRAME_SPEED *
                                   PblRandom::GetGlobalFloat(),
                               pLower);
                }
            }
        }
    }

    if (bWeaponUpper)
    {
        if (bRestartWeaponAnim && pUpper->GetAnimation() == mZephyrPoseDynUpper.m_kAnim.m_pkAnim)
        {
            m_bResetUpperAnim = true;
        }
        UpdateUpperBodyAnimation(mWeaponAnimationRate * fDeltaTime, pUpper);
        if (pUpper->m_pAnimData->m_uiLoop == SoldierAnimationData::LOOP_END && mZephyrPoseDynUpper.m_bAnimFinished)
        {
            if (mWeaponAnim == SoldierAnimType::WEAPON_MELEE)
            {
                mWeaponMeleeAnim = mWeaponMeleeAnimNext;
                mWeaponMeleeAnimNext = NO_MELEE_ANIM;
                if (mWeaponMeleeAnim != NO_MELEE_ANIM)
                {
                    m_bAnimVelocityMatched = true;
                }
            }
            else
            {
                mWeaponAnim = SoldierAnimType::WEAPON_IDLE;
            }
        }
    }
    else
    {
        UpdateUpperBodyAnimation(fAnimDeltaTime, pUpper);
    }

    if (mSoldierAction == SoldierState::ARMED_PASSENGER)
    {
        UpdateLowerBodyAnimation(0.0f, pClass->GetAnimation(mAnimationHash, 0), false, 0.0f);
        mZephyrPoseStatic.Set(mZephyrPoseDynLower, mLowerBodyAnimMask, true);
    }
    else
    {
        if (bWeaponLower)
        {
            if (bRestartWeaponAnim && pLower->GetAnimation() == mZephyrPoseDynLower.m_kAnim.m_pkAnim)
            {
                m_bResetLowerAnim = true;
            }
            UpdateLowerBodyAnimation(mWeaponAnimationRate * fDeltaTime, pLower);
        }
        float fLowerBlend = GetLowerBodyBlendFactor();
        if (m_bFirstTime || fLowerBlend >= 1.0f)
        {
            mZephyrPoseStatic.Set(mZephyrPoseDynLower, mLowerBodyAnimMask, true);
        }
        else if (fLowerBlend > 0.0f)
        {
            mZephyrPoseStatic.Blend(mZephyrPoseDynLower, mLowerBodyAnimMask, fLowerBlend);
        }

        if (pAnim && (!bWeaponLower || !bAnimControlledMove) && eMove <= SoldierAnimType::IDLE_LOOK_AROUND &&
            mPosture != SoldierAnimType::PRONE)
        {
            ZephyrPoseDyn<32> overlayPose;
            overlayPose.Open(mZephyrSkeleton);
            overlayPose.SetAnimation(pAnim, 30.0f);
            mZephyrPoseStatic.Blend(overlayPose, mLowerBodyAnimMask, fBlend);
        }
    }

    float fUpperBlend = GetUpperBodyBlendFactor();
    if (m_bFirstTime || fUpperBlend >= 1.0f)
    {
        mZephyrPoseStatic.Set(mZephyrPoseDynUpper, mUpperBodyAnimMask, true);
    }
    else if (fUpperBlend > 0.0f)
    {
        mZephyrPoseStatic.Blend(mZephyrPoseDynUpper, mUpperBodyAnimMask, fUpperBlend);
    }

    if (pAnim && mPosture != SoldierAnimType::PRONE)
    {
        ZephyrPoseDyn<32> overlayPose;
        overlayPose.Open(mZephyrSkeleton);
        overlayPose.SetAnimation(pAnim, 30.0f);
        mZephyrPoseStatic.Blend(overlayPose, mUpperBodyAnimMask, fBlend);
    }

    if (mSoldierAction != SoldierState::ARMED_PASSENGER)
    {
        if (eMove == SoldierAnimType::TURN_LEFT || eMove == SoldierAnimType::TURN_RIGHT)
        {
            if (bWeaponLower)
            {
                float fTarget = (m_uiAimState & 4) ? 0.0f : fMoveAngle;
                float fFactor = fDeltaTime * MOVEANGLE_BLEND_RATE;
                if (fFactor > 1.0f)
                {
                    fFactor = 1.0f;
                }
                if (fFactor > 0.0f)
                {
                    fLegAngle += (fTarget - fLegAngle) * fFactor;
                }
                mMoveTurnRate = 0.0f;
            }
            else if (fTurn != 0.0f)
            {
                mMoveTurnRate = fTurn / ((float)(pLower->GetAnimationNumFrames() - 1) * ANIM_FRAME_SPEED);
            }
            else
            {
                fLegAngle += mMoveTurnRate * fDeltaTime;
            }
            if (mZephyrPoseDynLower.m_bAnimFinished)
            {
                eMove = SoldierAnimType::IDLE;
                m_bResetLowerAnim = true;
            }
            m_fTakeKneeTimer = 0.0f;
        }
        else if ((bWeaponLower && bAnimControlledMove) || (!bWeaponLower && eNewMove != SoldierAnimType::IDLE))
        {
            float fTarget = (m_uiAimState & 4) ? 0.0f : fMoveAngle;
            float fFactor = fDeltaTime * MOVEANGLE_BLEND_RATE;
            if (fFactor > 1.0f)
            {
                fFactor = 1.0f;
            }
            if (fFactor > 0.0f)
            {
                fLegAngle += (fTarget - fLegAngle) * fFactor;
            }
            mMoveTurnRate = 0.0f;
            m_fTakeKneeTimer = 0.0f;
        }
        else
        {
            mMoveTurnRate = 0.0f;
            if (mZephyrPoseDynLower.m_bAnimFinished)
            {
                eMove = SoldierAnimType::IDLE;
            }

            if (mWeaponAnim != SoldierAnimType::WEAPON_MELEE && mPosture == SoldierAnimType::CROUCH && !bWeaponLower)
            {
                if (eMove == SoldierAnimType::IDLE)
                {
                    m_fTakeKneeTimer += fDeltaTime;
                    if (m_fTakeKneeTimer > 1.0f)
                    {
                        eMove = SoldierAnimType::IDLE_TAKEKNEE;
                        m_fTakeKneeTimer = 1.0f;
                    }
                }
                else if (eMove != SoldierAnimType::IDLE_TAKEKNEE)
                {
                    m_fTakeKneeTimer = 0.0f;
                }
            }
            else
            {
                m_fTakeKneeTimer = 0.0f;
            }
        }
        RotateLowerBody(fLegAngle);
    }

    if (eNewMove == SoldierAnimType::SLIDE)
    {
        eMove = SoldierAnimType::SLIDE;
    }
    mMoveAnimation = eMove;
    m_bMoveAlert = bAlert;
    mLegAngle = fLegAngle;
}

inline unsigned char SoldierAnimation::GetAnimIndex() const
{
    return m_pAnimData->GetAnimIndex();
}

inline unsigned char SoldierAnimation::GetSyncType() const
{
    return (unsigned char)m_pAnimData->m_uiSyncType;
}

inline float SoldierAnimation::GetBlendInTime() const
{
    return m_pAnimData->m_uiBlendInTime * SoldierAnimationData::BLEND_TIME_SCALE_INV;
}

inline float SoldierAnimation::GetBlendTime(const SoldierAnimation* pNext) const
{
    return m_pAnimData->GetBlendTime(pNext->GetBlendInTime(), pNext->GetAnimIndex());
}

// FUNCTION: SWBF2 0x00641960
static float _GetMovementAnimSpeed(const SoldierAnimation* pAnim, SoldierAnimType::MovementAnimation eAnim)
{
    switch (eAnim)
    {
    case SoldierAnimType::WALK_FORWARD:
    case SoldierAnimType::RUN_FORWARD:
        return pAnim->GetVelocityZ();
    case SoldierAnimType::WALK_BACKWARD:
    case SoldierAnimType::RUN_BACKWARD:
        return -pAnim->GetVelocityZ();
    case SoldierAnimType::WALK_LEFT:
    case SoldierAnimType::RUN_LEFT:
        return -pAnim->GetVelocityX();
    case SoldierAnimType::WALK_RIGHT:
    case SoldierAnimType::RUN_RIGHT:
        return pAnim->GetVelocityX();
    default:
        return 0.0f;
    }
}

// FUNCTION: SWBF2 0x00641a00
SoldierAnimType::MovementAnimation SoldierAnimator::GetMovementAnimation(float fSide, float fForward, float& fAngle,
                                                                         float& fRate) const
{
    float fMinSlideSpeed = 10.0f;
    if (mSkeletonScale != 1.0f)
    {
        float fInvScale = 1.0f / mSkeletonScale;
        fSide *= fInvScale;
        fForward *= fInvScale;
        if (fInvScale > 1.0f)
        {
            fMinSlideSpeed *= fInvScale;
        }
    }

    float fSpeedSq = fSide * fSide + fForward * fForward;
    if (mMoveAnimation != SoldierAnimType::IDLE)
    {
        if (fSpeedSq < 0.2f * 0.2f)
        {
            return SoldierAnimType::IDLE;
        }
    }
    else if (fSpeedSq < 0.3f * 0.3f)
    {
        return SoldierAnimType::IDLE;
    }

    bool bAlert = m_bAlert;
    const SoldierAnimatorClass* pClass = SoldierAnimatorClass::Instance();
    const SoldierAnimation* apAnim[SoldierAnimType::NUM_MOVEMENT_ANIMATIONS] = { 0 };
    for (int i = SoldierAnimType::WALK_FORWARD; i < SoldierAnimType::NUM_MOVEMENT_ANIMATIONS; ++i)
    {
        apAnim[i] = pClass->GetLowerBodyMovementAnimation((SoldierAnimType::MovementAnimation)i, mPosture,
                                                          mWeaponAnimationMap, bAlert);
    }

    int eAnim = SoldierAnimType::WALK_FORWARD;
    if ((apAnim[SoldierAnimType::WALK_RIGHT] || apAnim[SoldierAnimType::RUN_RIGHT]) &&
        (apAnim[SoldierAnimType::WALK_LEFT] || apAnim[SoldierAnimType::RUN_LEFT]))
    {
        fAngle = atan2f(fSide, fForward);
        if (fabsf(fSide) > fabsf(fForward))
        {
            if (fSide > 0.0f)
            {
                eAnim = SoldierAnimType::WALK_RIGHT;
                fAngle = UnitCircleNormalize(fAngle - PblMath::fHalfPi);
            }
            else
            {
                eAnim = SoldierAnimType::WALK_LEFT;
                fAngle = UnitCircleNormalize(fAngle + PblMath::fHalfPi);
            }
        }
        else if (fForward <= 0.0f)
        {
            eAnim = SoldierAnimType::WALK_BACKWARD;
            fAngle = UnitCircleNormalize(fAngle + PblMath::fPi);
        }
    }
    else if (mOwner)
    {
        fAngle = mOwner->mLegAngle;
        if (!mOwner->mLegAngleForward)
        {
            eAnim = SoldierAnimType::WALK_BACKWARD;
        }
    }
    else
    {
        fAngle = atan2f(fSide, fForward);
        if (fForward <= 0.0f)
        {
            eAnim = SoldierAnimType::WALK_BACKWARD;
            fAngle = UnitCircleNormalize(fAngle + PblMath::fPi);
        }
    }

    float fSpeed = sqrtf(fSpeedSq);
    if (!apAnim[eAnim])
    {
        ++eAnim;
    }
    else if (apAnim[eAnim + 1] && mOwner && mOwner->mMoveAnim == EntitySoldier::MOVEANIM_RUN)
    {
        ++eAnim;
    }

    float fAnimSpeed = _GetMovementAnimSpeed(apAnim[eAnim], (SoldierAnimType::MovementAnimation)eAnim);

    float& fSlideDistance = const_cast<SoldierAnimator*>(this)->m_fSlideDistance;
    if (fSpeed > fMinSlideSpeed && fSpeed > fAnimSpeed * 1.5f && mMoveAnimation >= SoldierAnimType::WALK_FORWARD)
    {
        fSlideDistance += fRate * fSpeed;
        if (fSlideDistance > 5.0f)
        {
            return SoldierAnimType::SLIDE;
        }
    }
    else
    {
        fSlideDistance = 0.0f;
    }

    float fRatio = fSpeed / fAnimSpeed;
    fRate *= (1.0f - fRatio) * 0.05f + fRatio;
    return (SoldierAnimType::MovementAnimation)eAnim;
}

// FUNCTION: SWBF2 0x00641e10
bool SoldierAnimator::UpdateUpperBodyAnimation(float fDeltaTime, const SoldierAnimation* pAnim)
{
    const ZephyrAnim* pCurAnim = mZephyrPoseDynUpper.m_kAnim.m_pkAnim;
    if (pCurAnim == pAnim->GetAnimation())
    {
        if (!m_bResetUpperAnim)
        {
            mZephyrPoseDynUpper.Update(fDeltaTime);
            mAnimationTimerUpper += fDeltaTime;
            return false;
        }
    }
    else
    {
        m_bResetUpperAnim = false;
    }

    unsigned char uiSyncType = 0;
    float fBlendTime;
    if (m_pAnimUpper)
    {
        uiSyncType = m_pAnimUpper->GetSyncType();
        fBlendTime = m_pAnimUpper->GetBlendTime(pAnim);
    }
    else
    {
        fBlendTime = pAnim->GetBlendInTime();
    }

    float fTime;
    if (m_bResetUpperAnim)
    {
        fTime = 0.0f;
        m_bResetUpperAnim = false;
    }
    else if (pAnim->GetAnimation() == mZephyrPoseDynLower.m_kAnim.m_pkAnim ||
             (m_pAnimLower && pAnim->GetAnimIndex() == m_pAnimLower->GetAnimIndex()))
    {
        fTime = mZephyrPoseDynLower.m_kAnim.m_pkAnim ? mZephyrPoseDynLower.m_fCurT : 0.0f;
        fDeltaTime = 0.0f;
    }
    else if (uiSyncType && uiSyncType == pAnim->GetSyncType() && pCurAnim)
    {
        fTime = mZephyrPoseDynUpper.m_fCurT;
        if (fTime > 0.0f && SoldierAnimationData::IsSyncTypeByTime(uiSyncType) && pAnim->GetAnimationNumFrames() > 1)
        {
            fTime = (float)(pCurAnim->GetNumFrames() - 1) / (float)(pAnim->GetAnimationNumFrames() - 1) * fTime;
        }
    }
    else
    {
        fTime = 0.0f;
    }

    mZephyrPoseDynUpper.SetAnimation(pAnim->GetAnimation(), 30.0f);
    mZephyrPoseDynUpper.SetLoop(pAnim->m_pAnimData->IsLooping());
    mZephyrPoseDynUpper.SetCurTime(fTime);
    mZephyrPoseDynUpper.Update(fDeltaTime);
    mAnimationTimerUpper = fDeltaTime;
    mBlendFactorUpper = 0.0f;
    mBlendDurationUpper = fBlendTime;
    m_pAnimUpper = const_cast<SoldierAnimation*>(pAnim);

    unsigned int uiPrevAim = m_uiAimState & 3;
    m_uiAimState = pAnim->m_pAnimData->m_uiAimType;
    if (m_uiAimState == SoldierAnimationData::AIM_FULLBODY)
    {
        m_uiAimState = 4;
    }
    unsigned int uiAim = m_uiAimState & 3;
    if (pCurAnim && uiAim != uiPrevAim)
    {
        mBlendDurationRotate = pAnim->GetBlendInTime();
        m_uiAimState |= (uiAim | uiPrevAim) << 4;
    }
    return true;
}

// FUNCTION: SWBF2 0x006420d0
bool SoldierAnimator::UpdateLowerBodyAnimation(float fDeltaTime, const SoldierAnimation* pAnim)
{
    if (mZephyrPoseDynLower.m_kAnim.m_pkAnim == pAnim->GetAnimation())
    {
        if (!m_bResetLowerAnim)
        {
            mZephyrPoseDynLower.Update(fDeltaTime);
            mAnimationTimerLower += fDeltaTime;
            return false;
        }
    }
    else
    {
        m_bResetLowerAnim = false;
    }

    unsigned char uiSyncType = 0;
    float fBlendTime;
    if (m_pAnimLower)
    {
        uiSyncType = m_pAnimLower->GetSyncType();
        fBlendTime = m_pAnimLower->GetBlendTime(pAnim);
    }
    else
    {
        fBlendTime = pAnim->GetBlendInTime();
    }

    float fTime;
    if (m_bResetLowerAnim)
    {
        fTime = 0.0f;
        m_bResetLowerAnim = false;
    }
    else if (pAnim->GetAnimation() == mZephyrPoseDynUpper.m_kAnim.m_pkAnim ||
             (m_pAnimUpper && pAnim->GetAnimIndex() == m_pAnimUpper->GetAnimIndex()))
    {
        fTime = mZephyrPoseDynUpper.m_kAnim.m_pkAnim ? mZephyrPoseDynUpper.m_fCurT : 0.0f;
        fDeltaTime = 0.0f;
    }
    else if (uiSyncType && uiSyncType == pAnim->GetSyncType() && mZephyrPoseDynLower.m_kAnim.m_pkAnim)
    {
        fTime = mZephyrPoseDynLower.m_fCurT;
        if (fTime > 0.0f && SoldierAnimationData::IsSyncTypeByTime(uiSyncType) && pAnim->GetAnimationNumFrames() > 1)
        {
            fTime = (float)(mZephyrPoseDynUpper.m_kAnim.m_pkAnim->GetNumFrames() - 1) /
                    (float)(pAnim->GetAnimationNumFrames() - 1) * fTime;
        }
    }
    else
    {
        fTime = 0.0f;
    }

    mZephyrPoseDynLower.SetAnimation(pAnim->GetAnimation(), 30.0f);
    mZephyrPoseDynLower.SetLoop(pAnim->m_pAnimData->IsLooping());
    mZephyrPoseDynLower.SetCurTime(fTime);
    mZephyrPoseDynLower.Update(fDeltaTime);
    mAnimationTimerLower = fDeltaTime;
    m_pAnimLower = const_cast<SoldierAnimation*>(pAnim);
    mBlendFactorLower = 0.0f;
    mBlendDurationLower = fBlendTime;
    return true;
}

// FUNCTION: SWBF2 0x00642330
bool SoldierAnimator::UpdateUpperBodyAnimation(float fDeltaTime, const ZephyrAnim* pAnim, bool, float fBlendTime, unsigned char)
{
    const ZephyrAnim* pPrevAnim = mZephyrPoseDynUpper.m_kAnim.m_pkAnim;
    if (pPrevAnim == pAnim)
    {
        if (!m_bResetUpperAnim)
        {
            mZephyrPoseDynUpper.Update(fDeltaTime);
            mAnimationTimerUpper += fDeltaTime;
            return false;
        }
    }
    else
    {
        m_bResetUpperAnim = false;
    }

    const float fBlend = m_pAnimUpper ? m_pAnimUpper->m_pAnimData->GetBlendTime(fBlendTime) : fBlendTime;

    float fTime;
    if (m_bResetUpperAnim)
    {
        m_bResetUpperAnim = false;
        fTime = 0.0f;
    }
    else if (pAnim == mZephyrPoseDynLower.m_kAnim.m_pkAnim)
    {
        fTime = mZephyrPoseDynLower.m_fCurT;
        fDeltaTime = 0.0f;
    }
    else
    {
        fTime = 0.0f;
    }

    mZephyrPoseDynUpper.SetAnimation(pAnim, 30.0f);
    mZephyrPoseDynUpper.SetLoop(true);
    mZephyrPoseDynUpper.SetCurTime(fTime);
    mZephyrPoseDynUpper.Update(fDeltaTime);
    mAnimationTimerUpper = fDeltaTime;

    const unsigned int uOldAim = m_uiAimState & 3;
    mBlendFactorUpper = 0.0f;
    mBlendDurationUpper = fBlend;
    m_pAnimUpper = 0;
    m_uiAimState = 0;
    if (m_uiAimState == 3)
    {
        m_uiAimState = 4;
    }
    const unsigned int uNewAim = m_uiAimState & 3;
    if (pPrevAnim && uNewAim != uOldAim)
    {
        mBlendDurationRotate = fBlendTime;
        m_uiAimState = (unsigned char)(m_uiAimState | ((uNewAim | uOldAim) << 4));
    }
    return true;
}

// FUNCTION: SWBF2 0x00642510
bool SoldierAnimator::UpdateLowerBodyAnimation(float fDeltaTime, const ZephyrAnim* pAnim, bool, float fBlendTime)
{
    if (mZephyrPoseDynLower.m_kAnim.m_pkAnim == pAnim)
    {
        if (!m_bResetLowerAnim)
        {
            mZephyrPoseDynLower.Update(fDeltaTime);
            mAnimationTimerLower += fDeltaTime;
            return false;
        }
    }
    else
    {
        m_bResetLowerAnim = false;
    }

    const float fBlend = m_pAnimLower ? m_pAnimLower->m_pAnimData->GetBlendTime(fBlendTime) : fBlendTime;

    float fTime;
    if (m_bResetLowerAnim)
    {
        m_bResetLowerAnim = false;
        fTime = 0.0f;
    }
    else if (pAnim == mZephyrPoseDynUpper.m_kAnim.m_pkAnim)
    {
        fTime = mZephyrPoseDynUpper.m_fCurT;
        fDeltaTime = 0.0f;
    }
    else
    {
        fTime = 0.0f;
    }

    mZephyrPoseDynLower.SetAnimation(pAnim, 30.0f);
    mZephyrPoseDynLower.SetLoop(true);
    mZephyrPoseDynLower.SetCurTime(fTime);
    mZephyrPoseDynLower.Update(fDeltaTime);
    mAnimationTimerLower = fDeltaTime;
    mBlendFactorLower = 0.0f;
    mBlendDurationLower = fBlend;
    m_pAnimLower = 0;
    return true;
}

// FUNCTION: SWBF2 0x006426a0
float SoldierAnimator::CalculateBlendFactorDelta(float fPrevFactor, float fFactor)
{
    if (fFactor >= 1.0f || fPrevFactor >= 1.0f)
    {
        return 1.0f;
    }
    if (fFactor <= 0.0f || fPrevFactor >= fFactor)
    {
        return 0.0f;
    }
    if (fPrevFactor > 0.0f)
    {
        return (fFactor - fPrevFactor) / (1.0f - fPrevFactor);
    }
    return fFactor;
}

// FUNCTION: SWBF2 0x006426e0
float SoldierAnimator::GetUpperBodyBlendFactor()
{
    float fFactor = (mBlendDurationUpper > mAnimationTimerUpper) ? mAnimationTimerUpper / mBlendDurationUpper : 1.0f;
    float fBlend = SoldierAnimator::CalculateBlendFactorDelta(mBlendFactorUpper, fFactor);
    mBlendFactorUpper = fFactor;
    return fBlend;
}

// FUNCTION: SWBF2 0x00642720
float SoldierAnimator::GetLowerBodyBlendFactor()
{
    float fFactor = (mBlendDurationLower > mAnimationTimerLower) ? mAnimationTimerLower / mBlendDurationLower : 1.0f;
    float fBlend = SoldierAnimator::CalculateBlendFactorDelta(mBlendFactorLower, fFactor);
    mBlendFactorLower = fFactor;
    return fBlend;
}

// FUNCTION: SWBF2 0x00642760
void SoldierAnimator::RotateLowerBody(float fAngle)
{
    static unsigned short uiRoot = (unsigned short)mZephyrSkeleton.m_pShared->GetJointIdxByCrc(0x4446e9b8);
    mZephyrPoseStatic.m_pJoints[uiRoot].kQuat = PblQuaternion_FromAxisAngle(PblVector3(0.0f, 1.0f, 0.0f), fAngle);
}

// FUNCTION: SWBF2 0x00643470
void SoldierAnimator::SetDamageDir(EntitySoldier::DAMAGEDIR eDir)
{
    m_eLastHitDir = eDir;

    int iDir = eDir;
    float fRand = PblRandom::GetGlobalFloat();
    if (fRand > 0.9f)
    {
        iDir = (iDir + 2) % 4;
    }
    else if (fRand > 0.7f)
    {
        iDir = (iDir + 1) % 4;
    }
    else if (fRand > 0.5f)
    {
        iDir = (iDir + 3) % 4;
    }

    SoldierAnimType::ActionAnimation eAnim;
    switch (iDir)
    {
    case EntitySoldier::LEFT:
        eAnim = SoldierAnimType::HIT_LEFT;
        break;
    case EntitySoldier::BACKWARD:
        eAnim = SoldierAnimType::HIT_BACK;
        break;
    case EntitySoldier::RIGHT:
        eAnim = SoldierAnimType::HIT_RIGHT;
        break;
    default:
        eAnim = SoldierAnimType::HIT_FRONT;
        break;
    }

    mOverlayAnim = (ZephyrAnim*)SoldierAnimatorClass::Instance()
                       ->GetUpperBodyActionAnimation(eAnim, mWeaponAnimationMap)
                       ->GetAnimation();
    mOverlayTime = 1.0f;
    mOverlayBlendFactor = 0.5f;
}

// FUNCTION: SWBF2 0x00643580
float SoldierAnimator::GetAlertFactor() const
{
    if (m_bAlert)
    {
        if (m_fAlertBlendDuration > m_fAlertTimer)
        {
            return m_fAlertTimer / m_fAlertBlendDuration;
        }
        return 1.0f;
    }
    if (m_fAlertBlendDuration > m_fAlertTimer)
    {
        return 1.0f - m_fAlertTimer / m_fAlertBlendDuration;
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x006435d0
void SoldierAnimator::SetAlert(bool bAlert)
{
    if (!SoldierAnimationBank::HasAlertState(SoldierAnimationBank::GetMapWeapon(mWeaponAnimationMap)))
    {
        return;
    }

    float fFactor = GetAlertFactor();
    m_fAlertBlendDuration = 0.5f;
    if (bAlert)
    {
        m_bAlert = true;
        m_fAlertTimer = fFactor * 0.5f;
    }
    else
    {
        m_bAlert = false;
        m_fAlertTimer = (1.0f - fFactor) * 0.5f;
    }
}

// FUNCTION: SWBF2 0x00643650
void SoldierAnimation::SetVelocity(float fVelocityX, float fVelocityZ)
{
    float fX = fVelocityX * 100.0f + 16383.0f + 0.5f;
    if (fX < 0.0f)
    {
        fX = 0.0f;
    }
    else if (fX > 32767.0f)
    {
        fX = 32767.0f;
    }
    m_iVelocityX = (int)fX - 16383;

    float fZ = fVelocityZ * 100.0f + 16383.0f + 0.5f;
    if (fZ < 0.0f)
    {
        fZ = 0.0f;
    }
    else if (fZ > 32767.0f)
    {
        fZ = 32767.0f;
    }
    m_iVelocityZ = (int)fZ - 16383;
}
