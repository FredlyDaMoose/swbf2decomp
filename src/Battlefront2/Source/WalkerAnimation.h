#pragma once

#include <stddef.h>

class RedAnimation;
class WalkerBlend;
class NetPktGroup;
class ZephyrAnim;

const ZephyrAnim* GetWalkerAnimation(const RedAnimation* const* ppAnims, int iNumAnims, unsigned int uiHash);

// VTABLE: SWBF2 0x007b00fc
class WalkerAnimation
{
public:
    WalkerAnimation(const RedAnimation* const* ppAnims, int iNumAnims, unsigned int uiHash);
    WalkerAnimation(const RedAnimation* const* ppAnims, int iNumAnims, const char* pszNames, bool bFullWalkAnim);
    virtual ~WalkerAnimation();
    virtual void Setup(WalkerBlend& rBlend, float fTime, float fBlendDuration, bool bLoop, bool bReversed);

protected:
    float ComputeTime(float fTime);

public:

    bool m_bHasTiltAnims;
    bool m_bFullWalkAnim;
    ZephyrAnim** mAnimations;
    float mSpeed;
    float mTimeTotal;
};
 
 
 
 
 
 

// VTABLE: SWBF2 0x0079dd10
class WalkerAnimationIdle : public WalkerAnimation
{
public:

    WalkerAnimationIdle(const RedAnimation* const* ppAnims, int iNumAnims, const char* pszNames, bool bFullWalkAnim)
        : WalkerAnimation(ppAnims, iNumAnims, pszNames, bFullWalkAnim)
    {
    }
    virtual ~WalkerAnimationIdle();
    virtual void Setup(WalkerBlend& rBlend, float fTime, float fBlendDuration, bool bLoop, bool bReversed);
};
 

#include "PebbleFL/Common/PblBitVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"

template <int N> class ZephyrSkeleton;
template <int N> class ZephyrPoseStatic;

class WalkerBlendUnit
{
public:

    static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    WalkerBlendUnit() { mMask.SetAll(); }

    ZephyrPoseDyn<64> mPose;
    float mWeight;
    PblBitVector<64> mMask;

    static MemoryPool sMemoryPool;
};
 

// VTABLE: SWBF2 0x007b00ec
class WalkerBlend
{
public:

    WalkerBlend(int iTotal, ZephyrSkeleton<64>& rSkeleton);

    virtual ~WalkerBlend();
    virtual void Blend(ZephyrPoseStatic<64>& rPose);
    virtual void Update(float fDt);

    void EnableTransitionBlending(bool bEnable);

    float GetStateLimit();
    void SetFrameState(float fState);
    void ResetTurnState();

    const PblVector3 GetMovement() const;

    void Reverse();
    float GetTurnState();
    bool GetReversed() const { return m_bReversed; }
    bool IsFullWalkAnim() const { return m_bFullWalkAnim; }
    int GetCount() const { return mCount; }
    float GetTime() const { return mTime; }
    float GetSpeed() const { return mSpeed; }
    float GetTimeRemaining() const { return m_bReversed ? mTime : mTimeTotal - mTime; }

    void SetInterpolate(bool bInterpolate)
    {
        for (int i = 0; i < mTotal; ++i)
        {
            mUnits[i]->mPose.SetInterpolate(bInterpolate);
        }
    }

    static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    void SetJointMask(int iUnit, PblBitVector<64>& rMask) { mUnits[iUnit]->mMask = rMask; }

    bool m_bReversed;
    bool m_bFullWalkAnim;
    bool m_bBlendTransitions;
    int mCount;
    int mTotal;
    float mTime;
    float mSpeed;
    float mTimeTotal;
    float mTurnState;
    float mBlendDuration;
    float mBlendFactor;
    float mFrameBlendFactor;
    WalkerBlendUnit* mUnits[8];

    static MemoryPool sMemoryPool;
public:

    void Write(NetPktGroup& group);
    void Read(NetPktGroup& group);
    void Skip(NetPktGroup& group);
};
 
 
