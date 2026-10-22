#include "IFaceSoldierAnimator.h"
#include "PebbleFL/Common/PblVector.h"
#include "SoldierAnimatorClass.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "Zephyr/Source/ZephyrAnimBank.h"

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x01ec601c
// SYNTHETIC: SWBF2 0x004040d0
// `dynamic initializer for 'IFaceSoldierAnimator::rttiHashIFaceSoldierAnimator''
PblHash IFaceSoldierAnimator::rttiHashIFaceSoldierAnimator("IFaceSoldierAnimator");
char IFaceSoldierAnimator::rttiNameIFaceSoldierAnimator[] = "IFaceSoldierAnimator";

// FUNCTION: SWBF2 0x0056a130
bool IFaceSoldierAnimator::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashIFaceSoldierAnimator || Red3DModelAnimator::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0056a160
unsigned int IFaceSoldierAnimator::GetDerivedRtti() const
{
    return rttiHashIFaceSoldierAnimator;
}

// FUNCTION: SWBF2 0x0056a170
const char* IFaceSoldierAnimator::GetDerivedRttiName() const
{
    return rttiNameIFaceSoldierAnimator;
}

// FUNCTION: SWBF2 0x0056a180
void IFaceSoldierAnimator::SetUpperLowerAnimations(unsigned int uUpperAnimHash, unsigned int uLowerAnimHash,
                                                   float fTransitionTime)
{
    Close();
    mAnimHash = uUpperAnimHash;
    mLowerAnimHash = uLowerAnimHash;
    mQueuedAnim = 0;
    mQueuedLowerAnim = 0;
    mAnimTransitionTimeTotal = fTransitionTime;
    mAnimTransitionTime = 0.0f;
    Open();
}

IFaceSoldierAnimator::~IFaceSoldierAnimator()
{
}

// FUNCTION: SWBF2 0x0056a240
IFaceSoldierAnimator::IFaceSoldierAnimator(const Red3DModelAnimator* pAnimator)
{
    mLowerAnimHash = 0;
    mQueuedLowerAnim = 0;
    mZephyrAnimationLower = 0;
    for (int i = 0; i < 5; ++i)
    {
        mAnimBankHash[i] = pAnimator->mAnimBankHash[i];
    }
    mLooping = pAnimator->mLooping;
}

// FUNCTION: SWBF2 0x0056a2e0
void IFaceSoldierAnimator::Open()
{
    if (mAnimHash)
    {
        Red3DModelAnimator::Open();

        ZephyrSkeletonShared* pSkelShared = RedAnimation::Find(mAnimBankHash[0])->_pSkeletonShared;
        SoldierAnimatorClass::SetupJointTransformAnimationFlag(pSkelShared);
        SoldierAnimatorClass::SetupBodyMasks(pSkelShared, mUpperBodyAnimMask, mLowerBodyAnimMask);

        mZephyrPoseDynLower.Open(mZephyrSkeleton);
        mZephyrAnimationLower = const_cast<ZephyrAnim*>(FindAnimation(mLowerAnimHash));
        mZephyrPoseDynLower.SetAnimation(mZephyrAnimationLower, 30.0f);
        if (mAnimationTime >= 0.0f)
        {
            mZephyrPoseDynLower.SetCurTime(mAnimationTime / mAnimationTimeTotal);
        }
        mZephyrPoseDynLower.SetLoop(mLooping);
        mZephyrPoseDynLower.Update(0.0f);

        if (mAnimTransitionTimeTotal <= 0.0f)
        {
            mZephyrPoseStatic.Set(mZephyrPoseDyn, mUpperBodyAnimMask, true);
            mZephyrPoseStatic.Set(mZephyrPoseDynLower, mLowerBodyAnimMask, true);
        }
        mPose.ConvertFromZephyrPose(mZephyrPoseStatic);
    }
}

// FUNCTION: SWBF2 0x0056a3f0
void IFaceSoldierAnimator::Close()
{
    if (mAnimHash)
    {
        Red3DModelAnimator::Close();
        mZephyrPoseDynLower.Close();
    }
}

// FUNCTION: SWBF2 0x0056a410
void IFaceSoldierAnimator::Update(float fElapsed)
{
    mAnimationTime += fElapsed;
    if (mAnimationTime > mAnimationTimeTotal)
    {
        mAnimationTime -= mAnimationTimeTotal;
        if (mQueuedAnim)
        {
            SetUpperLowerAnimations(mQueuedAnim, mQueuedLowerAnim, 0.3f);
            mQueuedLowerAnim = 0;
            mQueuedAnim = 0;
        }
    }
    mZephyrPoseDyn.SetCurTime(mAnimationTime / mAnimationTimeTotal);
    mZephyrPoseDyn.Update(0.0f);
    mZephyrPoseDynLower.SetCurTime(mAnimationTime / mAnimationTimeTotal);
    mZephyrPoseDynLower.Update(0.0f);
    if (mAnimHash && mAnimTransitionTimeTotal >= 0.0f)
    {
        mAnimTransitionTime += fElapsed;
    }
}

// FUNCTION: SWBF2 0x0056a590
RedPose* IFaceSoldierAnimator::UpdatePose()
{
    if (mAnimationTime < 0.0f)
    {
        int iNumFrames = mZephyrAnimation->m_u16NumFrames;
        mZephyrPoseDyn.SetFrameNum(RedRenderer::GetFrameNumber() % iNumFrames);
        mZephyrPoseDyn.Update(0.0f);
        iNumFrames = mZephyrAnimation->m_u16NumFrames;
        mZephyrPoseDynLower.SetFrameNum(RedRenderer::GetFrameNumber() % iNumFrames);
        mZephyrPoseDynLower.Update(0.0f);
    }
    if (mAnimTransitionTimeTotal > 0.0f && mAnimTransitionTime < mAnimTransitionTimeTotal)
    {
        mZephyrPoseStatic.Blend(mZephyrPoseDyn, mUpperBodyAnimMask, mAnimTransitionTime / mAnimTransitionTimeTotal);
        mZephyrPoseStatic.Blend(mZephyrPoseDynLower, mLowerBodyAnimMask,
                                mAnimTransitionTime / mAnimTransitionTimeTotal);
    }
    else
    {
        if (mAnimTransitionTimeTotal > 0.0f)
        {
            mAnimTransitionTimeTotal = 0.0f;
        }
        mZephyrPoseStatic.Set(mZephyrPoseDyn, mUpperBodyAnimMask, true);
        mZephyrPoseStatic.Set(mZephyrPoseDynLower, mLowerBodyAnimMask, true);
    }
    mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
    return &mPose;
}
