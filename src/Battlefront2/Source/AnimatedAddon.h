#pragma once

#include <stddef.h>

#include "RedEngineFL/Graphics/RedPose.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"
#include "Zephyr/Source/ZephyrSkeleton.h"

class ZephyrSkeletonShared;
class ZephyrAnim;
class GameModel;
class RedAnimation;

class AnimatedAddon
{
public:

    enum State
    {
        Idle = 0,
        WalkForward = 1,
        MaxAnimations = 2
    };

    AnimatedAddon(const ZephyrSkeletonShared& parentSkeleton);
    ~AnimatedAddon();

    void SetProperty(unsigned int uProperty, const char* pszValue);

    void PostReadSetup();

    void Render(class TentacleSimulator* pTentacles, const class PblVector3& vVelocity,
                const class RedPose* pPose, const class PblMatrix& mMatrix,
                const class RedColor& color, unsigned int uiFlags) const;

    unsigned int mAttachJoint;
    int mLastFrame;
    float mBlendAmount;
    State mState;
    ZephyrSkeleton<32> mZephyrSkeleton;
    ZephyrPoseStatic<32> mZephyrPoseStatic;
    ZephyrPoseDyn<32> mZephyrPoseDyn;
    GameModel* mModel;
    RedPose mPose;
    const ZephyrSkeletonShared& mParentSkeleton;
    RedAnimation* mAnimation;
    const ZephyrAnim* mAnimations[MaxAnimations];
};
 
 
 
 
 
 
 
 
