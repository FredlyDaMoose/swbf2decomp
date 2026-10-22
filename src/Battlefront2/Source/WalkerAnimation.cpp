#include "WalkerAnimation.h"

#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "Zephyr/Source/ZephyrAnim.h"
#include "Zephyr/Source/ZephyrAnimBank.h"
#include "Zephyr/Source/ZephyrAnimJoint.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"

// GLOBAL: SWBF2 0x01f9a790
// SYNTHETIC: SWBF2 0x00406fa0
// `dynamic initializer for 'WalkerBlend::sMemoryPool''
// SYNTHETIC: SWBF2 0x007697a0
// `dynamic atexit destructor for 'WalkerBlend::sMemoryPool''
MemoryPool WalkerBlend::sMemoryPool("WalkerBlend", 76);

// GLOBAL: SWBF2 0x01f9a740
// SYNTHETIC: SWBF2 0x00406fc0
// `dynamic initializer for 'WalkerBlendUnit::sMemoryPool''
// SYNTHETIC: SWBF2 0x007697b0
// `dynamic atexit destructor for 'WalkerBlendUnit::sMemoryPool''
MemoryPool WalkerBlendUnit::sMemoryPool("WalkerBlendUnit", 4924);

// FUNCTION: SWBF2 0x00675990
WalkerBlend::WalkerBlend(int iTotal, ZephyrSkeleton<64>& rSkeleton)
    : m_bReversed(false),
      m_bFullWalkAnim(false),
      m_bBlendTransitions(false),
      mCount(0),
      mTotal(iTotal),
      mTime(0.0f),
      mSpeed(0.0f)
{
    for (int i = 0; i < 8; ++i)
    {
        mUnits[i] = 0;
    }

    for (int i = 0; i < mTotal; ++i)
    {
        mUnits[i] = new WalkerBlendUnit;
        mUnits[i]->mPose.Open(rSkeleton);
        if (m_bReversed)
        {
            mUnits[i]->mPose.PlayAnimBackward();
        }
        else
        {
            mUnits[i]->mPose.PlayAnimForward();
        }
        mUnits[i]->mWeight = 0.0f;
    }
    mUnits[0]->mWeight = 1.0f;

    mBlendDuration = 0.0f;
    mFrameBlendFactor = 1.0f;
    mBlendFactor = 1.0f;
}

// SYNTHETIC: SWBF2 0x00675ad0
// WalkerBlend::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00675b00
WalkerBlend::~WalkerBlend()
{
    for (int i = 0; i < mTotal; ++i)
    {
        delete mUnits[i];
    }
}

// FUNCTION: SWBF2 0x00675b50
void WalkerBlend::EnableTransitionBlending(bool bEnable)
{
    m_bBlendTransitions = bEnable;
    if (!bEnable)
    {
        mBlendDuration = 0.0f;
        mFrameBlendFactor = 1.0f;
        mBlendFactor = 1.0f;
    }
}

// FUNCTION: SWBF2 0x00675de0
void WalkerBlend::Reverse()
{
    m_bReversed = !m_bReversed;
    for (int i = 0; i < mCount; ++i)
    {
        mUnits[i]->mPose.m_bPlayingBackward = m_bReversed;
    }
}

// FUNCTION: SWBF2 0x00675b80
void WalkerBlend::Blend(ZephyrPoseStatic<64>& rPose)
{
    if (mFrameBlendFactor <= 0.0f)
    {
        return;
    }

    WalkerBlendUnit* pUnit = mUnits[0];
    if (mFrameBlendFactor < 1.0f)
    {
        rPose.Blend(pUnit->mPose, pUnit->mMask, mFrameBlendFactor);
    }
    else
    {
        rPose.Set(pUnit->mPose, pUnit->mMask, false);
    }

    for (int i = 1; i < mCount; ++i)
    {
        pUnit = mUnits[i];
        if (pUnit->mWeight > 0.0f)
        {
            rPose.Blend(pUnit->mPose, pUnit->mMask, pUnit->mWeight * mFrameBlendFactor);
        }
    }
}

// FUNCTION: SWBF2 0x00675c20
void WalkerBlend::Update(float fDt)
{
    if (m_bReversed)
    {
        mTime = mTime - fDt;
    }
    else
    {
        mTime = fDt + mTime;
    }

    for (int i = 0; i < mCount; ++i)
    {
        mUnits[i]->mPose.Update(fDt);
    }

    if (m_bBlendTransitions && mBlendDuration > 0.0f)
    {
        float fOld = mBlendFactor;
        float fNew = fDt / mBlendDuration + fOld;
        if (fNew >= 1.0f)
        {
            mBlendFactor = 1.0f;
            mFrameBlendFactor = 1.0f;
            mBlendDuration = 0.0f;
        }
        else
        {
            mBlendFactor = fNew;
            mFrameBlendFactor = (fNew - fOld) / (1.0f - fOld);
        }
    }
}

// FUNCTION: SWBF2 0x00675e40
const PblVector3 WalkerBlend::GetMovement() const
{
    return mUnits[0]->mPose.GetRootTransDelta();
}

// FUNCTION: SWBF2 0x00675e60
const ZephyrAnim* GetWalkerAnimation(const RedAnimation* const* ppAnims, int iNumAnims, unsigned int uiHash)
{
    const ZephyrAnim* pAnim = 0;
    for (int i = 0; i < iNumAnims && !pAnim; ++i)
    {
        ZephyrAnimBank* pBank = ppAnims[i]->_pZephyrAnimBank;
        pAnim = pBank ? pBank->Find(uiHash) : 0;
    }
    return pAnim;
}

// SYNTHETIC: SWBF2 0x00676010
// WalkerAnimation::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00676040
WalkerAnimation::WalkerAnimation(const RedAnimation* const* ppAnims, int iNumAnims, unsigned int uiHash)
    : m_bHasTiltAnims(false), m_bFullWalkAnim(false), mSpeed(0.0f), mTimeTotal(0.0f)
{
    mAnimations = new ZephyrAnim*[1];
    mAnimations[0] = const_cast<ZephyrAnim*>(GetWalkerAnimation(ppAnims, iNumAnims, uiHash));
    mSpeed = 0.0f;
    mTimeTotal = 0.0f;
    if (mAnimations[0])
    {
        int iNumFrames = mAnimations[0]->m_u16NumFrames;
        if (iNumFrames > 1)
        {

            mTimeTotal = (float)(iNumFrames - 1) * (1.0f / 30.0f);
            PblVector3 kTrans;
            mAnimations[0]->m_pkJoints->GetTranslationUncached(kTrans, (unsigned short)(iNumFrames - 1));
            mSpeed = kTrans.z / mTimeTotal;
        }
    }
}

const ZephyrAnim* GetWalkerAnimation(const RedAnimation* const* ppAnims, int iNumAnims, const char* pszName);

// FUNCTION: SWBF2 0x00675ef0
WalkerAnimation::WalkerAnimation(const RedAnimation* const* ppAnims, int iNumAnims, const char* pszNames, bool bFullWalkAnim)
    : m_bHasTiltAnims(false), m_bFullWalkAnim(bFullWalkAnim), mSpeed(0.0f), mTimeTotal(0.0f)
{
    const ZephyrAnim* pAnims[3] = { 0, 0, 0 };
    char acName[52];
    char* pcName = acName;
    int iCount = 0;
    do
    {
        if (*pszNames == '\0' || *pszNames == ' ')
        {
            *pcName = '\0';
            pcName = acName;
            pAnims[iCount++] = GetWalkerAnimation(ppAnims, iNumAnims, acName);
            if (*pszNames == '\0')
            {
                break;
            }
        }
        *pcName++ = *pszNames++;
    } while (iCount < 3);

    if (pAnims[0] && pAnims[1] && pAnims[2])
    {
        mAnimations = new ZephyrAnim*[3];
        mAnimations[0] = const_cast<ZephyrAnim*>(pAnims[0]);
        mAnimations[1] = const_cast<ZephyrAnim*>(pAnims[1]);
        mAnimations[2] = const_cast<ZephyrAnim*>(pAnims[2]);
        m_bHasTiltAnims = true;
    }
    else
    {
        mAnimations = new ZephyrAnim*[1];
        mAnimations[0] = const_cast<ZephyrAnim*>(pAnims[0]);
    }

    mSpeed = 0.0f;
    mTimeTotal = 0.0f;
    if (mAnimations[0])
    {
        int iNumFrames = mAnimations[0]->m_u16NumFrames;
        if (iNumFrames > 1)
        {
            mTimeTotal = (float)(iNumFrames - 1) * (1.0f / 30.0f);
            PblVector3 kTrans;
            mAnimations[0]->m_pkJoints->GetTranslationUncached(kTrans, (unsigned short)(iNumFrames - 1));
            mSpeed = kTrans.z / mTimeTotal;
        }
    }
}

// FUNCTION: SWBF2 0x006760f0
WalkerAnimation::~WalkerAnimation()
{
    delete[] mAnimations;
}

// FUNCTION: SWBF2 0x00676100
void WalkerAnimation::Setup(WalkerBlend& rBlend, float fTime, float fBlendDuration, bool bLoop, bool bReversed)
{
    rBlend.mTime = fTime;
    rBlend.mSpeed = mSpeed;
    rBlend.m_bReversed = bReversed;
    rBlend.m_bFullWalkAnim = m_bFullWalkAnim;
    if (bReversed)
    {
        rBlend.mSpeed = mSpeed * -1.0f;
    }
    rBlend.mBlendDuration = fBlendDuration;
    if (fBlendDuration > 0.0f)
    {
        rBlend.mFrameBlendFactor = 0.0f;
        rBlend.mBlendFactor = 0.0f;
    }
    else
    {
        rBlend.mFrameBlendFactor = 1.0f;
        rBlend.mBlendFactor = 1.0f;
    }

    ZephyrAnim* pAnim = mAnimations[0];
    if (!pAnim)
    {
        rBlend.mCount = 0;
        rBlend.mTimeTotal = 0.0f;
        return;
    }

    rBlend.mTimeTotal = mTimeTotal;
    if (bReversed)
    {
        fTime = mTimeTotal - fTime;
        rBlend.mTime = fTime;
    }
    rBlend.mCount = 1;

    ZephyrPoseDyn<64>& rPose = rBlend.mUnits[0]->mPose;
    rPose.m_bLoop = bLoop;
    rPose.m_bPlayingBackward = bReversed;
    rPose.SetAnimation(pAnim, 30.0f);
    rPose.SetCurTime(ComputeTime(fTime));
    rPose.Update(0.0f);
}

// FUNCTION: SWBF2 0x00676240
float WalkerAnimation::ComputeTime(float fTime)
{
    return 30.0f / (float)(mAnimations[0]->m_u16NumFrames - 1) * fTime;
}

// FUNCTION: SWBF2 0x00676270
void WalkerAnimationIdle::Setup(WalkerBlend& rBlend, float fTime, float fBlendDuration, bool bLoop, bool bReversed)
{
    if (!mAnimations[1] || !mAnimations[2])
    {
        WalkerAnimation::Setup(rBlend, fTime, fBlendDuration, bLoop, bReversed);
        return;
    }

    rBlend.mTime = fTime;
    rBlend.mSpeed = mSpeed;
    rBlend.m_bReversed = bReversed;
    rBlend.m_bFullWalkAnim = m_bFullWalkAnim;
    if (bReversed)
    {
        rBlend.mSpeed = mSpeed * -1.0f;
    }
    rBlend.mBlendDuration = fBlendDuration;
    if (fBlendDuration > 0.0f)
    {
        rBlend.mFrameBlendFactor = 0.0f;
        rBlend.mBlendFactor = 0.0f;
    }
    else
    {
        rBlend.mFrameBlendFactor = 1.0f;
        rBlend.mBlendFactor = 1.0f;
    }

    if (!mAnimations[0])
    {
        rBlend.mCount = 0;
        rBlend.mTimeTotal = 0.0f;
        return;
    }

    rBlend.mTimeTotal = mTimeTotal;
    if (bReversed)
    {
        fTime = mTimeTotal - fTime;
        rBlend.mTime = fTime;
    }
    rBlend.mCount = m_bHasTiltAnims ? 3 : 1;

    for (int i = 0; i < rBlend.mCount; ++i)
    {
        ZephyrPoseDyn<64>& rPose = rBlend.mUnits[i]->mPose;
        rPose.m_bLoop = bLoop;
        rPose.m_bPlayingBackward = bReversed;
        rPose.SetAnimation(mAnimations[i], 30.0f);
        rPose.SetCurTime(ComputeTime(fTime));
        rPose.Update(0.0f);
    }
}

// FUNCTION: SWBF2 0x00675ce0
float WalkerBlend::GetTurnState()
{
    mTurnState = (float)(mUnits[0]->mPose.m_kAnim.m_pkAnim->m_u16NumFrames - 1) * mUnits[0]->mPose.m_fCurT;
    return mTurnState;
}

// FUNCTION: SWBF2 0x00675d10
void WalkerBlend::SetFrameState(float fState)
{
    if (m_bReversed ? mTurnState <= fState : fState <= mTurnState)
    {
        for (int i = 0; i < mCount; ++i)
        {
            mUnits[i]->mPose.SetFrameNumFloat(fState);
            mUnits[i]->mPose.Update(0.0f);
        }
    }
}

// FUNCTION: SWBF2 0x00675d80
void WalkerBlend::ResetTurnState()
{
    for (int i = 0; i < mCount; ++i)
    {
        mUnits[i]->mPose.SetFrameNumFloat(mTurnState);
        mUnits[i]->mPose.Update(0.0f);
    }
}

// FUNCTION: SWBF2 0x00675dc0
float WalkerBlend::GetStateLimit()
{
    return (float)(mUnits[0]->mPose.m_kAnim.m_pkAnim->m_u16NumFrames - 1);
}

// FUNCTION: SWBF2 0x00675ea0
const ZephyrAnim* GetWalkerAnimation(const RedAnimation* const* ppAnims, int iNumAnims, const char* pszName)
{
    return GetWalkerAnimation(ppAnims, iNumAnims, PblTEMPHash(pszName));
}
