#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "Zephyr/Source/ZephyrSkeleton.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"

class ZephyrAnim;

// VTABLE: SWBF2 0x00784284
class Red3DModelAnimator
{
public:
    Red3DModelAnimator();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char * GetDerivedRttiName() const;
    virtual ~Red3DModelAnimator();
    // SYNTHETIC: SWBF2 0x006f82f0
    // Red3DModelAnimator::`scalar deleting destructor'
    virtual void Open();
    virtual void Close();
    virtual RedPose * UpdatePose();
    virtual void Update(float fElapsed);

    void SetAnimation(unsigned int uAnimHash, float fTransitionTime)
    {
        Close();
        mAnimHash = uAnimHash;
        mQueuedAnim = 0;
        mAnimTransitionTimeTotal = fTransitionTime;
        mAnimTransitionTime = 0.0f;
        Open();
    }

    void SetAnimationBanks(const unsigned int* puBanks)
    {
        for (int i = 0; i < 5; ++i)
        {
            mAnimBankHash[i] = puBanks[i];
        }
    }
    void SetAnimationTime(float fTime) { mAnimationTime = fTime; }

    static PblHash rttiHashRed3DModelAnimator;
    static char rttiNameRed3DModelAnimator[];

protected:
    const ZephyrAnim* FindAnimation(unsigned int uAnimHash);

public:

    unsigned int mAnimBankHash[5];
    unsigned int mAnimHash;
    unsigned int mQueuedAnim;
    float mAnimTransitionTime;
    float mAnimTransitionTimeTotal;
    float mAnimationTime;
    float mAnimationTimeTotal;
    bool mLooping;
    ZephyrAnim* mZephyrAnimation;
    ZephyrSkeleton<32> mZephyrSkeleton;
    ZephyrPoseDyn<32> mZephyrPoseDyn;
    ZephyrPoseStatic<32> mZephyrPoseStatic;
    RedPose mPose;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
