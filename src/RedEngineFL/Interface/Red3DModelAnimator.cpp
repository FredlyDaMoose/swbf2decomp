#include "Red3DModelAnimator.h"
#include "PebbleFL/Common/PblVector.h"

#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "Zephyr/Source/ZephyrAnimBank.h"

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x009c8500
// SYNTHETIC: SWBF2 0x004092a0
// `dynamic initializer for 'Red3DModelAnimator::rttiHashRed3DModelAnimator''
PblHash Red3DModelAnimator::rttiHashRed3DModelAnimator("Red3DModelAnimator");
char Red3DModelAnimator::rttiNameRed3DModelAnimator[] = "Red3DModelAnimator";

// FUNCTION: SWBF2 0x006f8190
Red3DModelAnimator::Red3DModelAnimator()
{
    mAnimHash = 0;
    mQueuedAnim = 0;
    mAnimTransitionTime = 0.0f;
    mAnimTransitionTimeTotal = 0.0f;
    mAnimationTime = -1.0f;
    mAnimationTimeTotal = 0.0f;
    mLooping = true;
    for (int i = 0; i < 5; ++i)
    {
        mAnimBankHash[i] = 0;
    }
    Open();
}

// FUNCTION: SWBF2 0x0056a110
bool Red3DModelAnimator::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashRed3DModelAnimator;
}

// FUNCTION: SWBF2 0x006f8270
Red3DModelAnimator::~Red3DModelAnimator()
{
    Close();
}

// FUNCTION: SWBF2 0x006f8320
void Red3DModelAnimator::Close()
{
    if (mAnimHash)
    {
        mZephyrPoseDyn.Close();
        mZephyrPoseStatic.Close();
        mZephyrSkeleton.Close();
        mAnimHash = 0;
    }
}

// FUNCTION: SWBF2 0x006f8350
const ZephyrAnim* Red3DModelAnimator::FindAnimation(unsigned int uAnimHash)
{
    for (int i = 0; i < 5; ++i)
    {
        const ZephyrAnimBank* pBank = RedAnimation::Find(mAnimBankHash[i])->_pZephyrAnimBank;
        if (pBank)
        {
            const ZephyrAnim* pAnim = pBank->Find(uAnimHash);
            if (pAnim)
            {
                return pAnim;
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006f83a0
unsigned int Red3DModelAnimator::GetDerivedRtti() const
{
    return rttiHashRed3DModelAnimator;
}

// FUNCTION: SWBF2 0x006f83b0
const char* Red3DModelAnimator::GetDerivedRttiName() const
{
    return rttiNameRed3DModelAnimator;
}

// FUNCTION: SWBF2 0x006f83c0
void Red3DModelAnimator::Open()
{
    if (mAnimHash)
    {
        mZephyrAnimation = const_cast<ZephyrAnim*>(FindAnimation(mAnimHash));
        mZephyrSkeleton.Open(*RedAnimation::Find(mAnimBankHash[0])->_pSkeletonShared, mZephyrPoseStatic);
        mZephyrPoseStatic.Open(mZephyrSkeleton);
        mZephyrPoseDyn.Open(mZephyrSkeleton);
        mZephyrPoseDyn.SetAnimation(mZephyrAnimation, 30.0f);
        if (mZephyrAnimation)
        {
            mAnimationTimeTotal = mZephyrAnimation->m_u16NumFrames / 30.0f;
        }
        else
        {
            mAnimationTimeTotal = 0.01f;
        }
        if (mAnimationTime >= 0.0f)
        {
            mZephyrPoseDyn.SetCurTime(mAnimationTime / mAnimationTimeTotal);
        }
        mZephyrPoseDyn.SetLoop(mLooping);
        mZephyrPoseDyn.Update(0.0f);
        if (mAnimTransitionTimeTotal <= 0.0f)
        {
            mZephyrPoseStatic.Set(mZephyrPoseDyn, true);
        }
        mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    }
}

// FUNCTION: SWBF2 0x006f8510
void Red3DModelAnimator::Update(float fElapsed)
{
    mAnimationTime += fElapsed;
    if (mAnimationTime >= mAnimationTimeTotal && mLooping)
    {
        mAnimationTime -= mAnimationTimeTotal;
        if (mQueuedAnim)
        {
            SetAnimation(mQueuedAnim, 0.3f);
            mQueuedAnim = 0;
        }
    }
    mZephyrPoseDyn.SetCurTime(mAnimationTime / mAnimationTimeTotal);
    mZephyrPoseDyn.Update(0.0f);
    if (mAnimHash && mAnimTransitionTimeTotal >= 0.0f)
    {
        mAnimTransitionTime += fElapsed;
    }
}

// FUNCTION: SWBF2 0x006f8620
RedPose* Red3DModelAnimator::UpdatePose()
{
    if (mAnimationTime < 0.0f)
    {
        int iNumFrames = mZephyrAnimation->m_u16NumFrames;
        mZephyrPoseDyn.SetFrameNum(RedRenderer::GetFrameNumber() % iNumFrames);
        mZephyrPoseDyn.Update(0.0f);
    }
    if (mAnimTransitionTimeTotal > 0.0f && mAnimTransitionTime < mAnimTransitionTimeTotal)
    {
        mZephyrPoseStatic.Blend(mZephyrPoseDyn, mAnimTransitionTime / mAnimTransitionTimeTotal);
    }
    else
    {
        if (mAnimTransitionTimeTotal > 0.0f)
        {
            mAnimTransitionTimeTotal = 0.0f;
        }
        mZephyrPoseStatic.Set(mZephyrPoseDyn, true);
    }
    mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
    return &mPose;
}
