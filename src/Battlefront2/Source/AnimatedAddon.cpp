#include "AnimatedAddon.h"

#include "GameModel.h"
#include "TentacleSimulator.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Util/RedAnimation.h"

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);

// FUNCTION: SWBF2 0x004439f0
AnimatedAddon::AnimatedAddon(const ZephyrSkeletonShared& parentSkeleton)
    : mAttachJoint((unsigned int)-1),
      mLastFrame(-1),
      mBlendAmount(0.0f),
      mState(MaxAnimations),
      mModel(0),
      mParentSkeleton(parentSkeleton),
      mAnimation(0)
{
    mAnimations[Idle] = 0;
    mAnimations[WalkForward] = 0;
}

// FUNCTION: SWBF2 0x00443ac0
AnimatedAddon::~AnimatedAddon()
{
    mZephyrPoseDyn.Close();
    mZephyrPoseStatic.Close();
    mZephyrSkeleton.Close();
}

// FUNCTION: SWBF2 0x00443b50
void AnimatedAddon::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x94272331:
        mModel = const_cast<GameModel*>(GameModel::Find(PblHash(pszValue)));
        break;
    case 0xA423E008:
        mAttachJoint = PblTEMPHash(pszValue);
        break;
    case 0xDDDCDE45:
        mAnimation = RedAnimation::Find(PblHash(pszValue));
        break;
    }
}

// FUNCTION: SWBF2 0x00443be0
void AnimatedAddon::PostReadSetup()
{
    if (mAnimation)
    {
        mAnimations[Idle] = mAnimation->FindAnimation("idle");
        mAnimations[WalkForward] = mAnimation->FindAnimation("runforward");
        mZephyrPoseDyn.Open(mZephyrSkeleton);
        mZephyrSkeleton.Open(*mAnimation->GetZephyrSkeletonShared());
        mPose.ConvertFromZephyrPose(mZephyrSkeleton);
    }
    mModel->GetRedModel(0)->SetupPose(mPose);
}

// FUNCTION: SWBF2 0x00443ca0
void AnimatedAddon::Render(TentacleSimulator* pTentacles, const PblVector3& vVelocity, const RedPose* pPose,
                           const PblMatrix& mMatrix, const RedColor& color, unsigned int uiFlags) const
{
    const PblMatrix* pJoint = pPose->Find(mAttachJoint);
    if (!pJoint)
    {
        return;
    }

    PblMatrix mWorld;
    D3DXMatrixMultiply(&mWorld, pJoint, &mMatrix);
    RedPose pose;
    mModel->GetRedModel(0)->SetupPose(pose);

    const PblMatrix* pRoot = pPose->Find(0xE5B51072);
    if (pRoot)
    {
        PblMatrix mRelative;
        D3DXMatrixMultiply(&mRelative, pRoot, &pJoint->GetInverseFast());
        PblMatrix mRoot = mRelative;
        pose.Store(0xE5B51072, &mRoot);
    }

    PblMatrix aTentacleMatrices[20];
    if (pTentacles)
    {
        pTentacles->DoTentacles(&pose, mWorld, vVelocity, aTentacleMatrices,
                                (uiFlags & 0x10000000) || (uiFlags & 0x200000) ? 0.0f : -1.0f);
    }

    mModel->GetRedModel(0)->Render(&mWorld, &pose, color, uiFlags, 0);
}
