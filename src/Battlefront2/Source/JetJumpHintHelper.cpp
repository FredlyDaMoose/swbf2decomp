#include "JetJumpHintHelper.h"

#include <float.h>
#include <stddef.h>

#include "AILowLevel.h"
#include "AIUtil.h"
#include "Controllable.h"
#include "GameLoop.h"
#include "HintManager.h"
#include "Trackable.h"
#include "UnitAgent.h"
#include "UnitAttackPatterns.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblMatrix.h"

// FUNCTION: SWBF2 0x0056afc0
JetJumpHintHelper::JetJumpHintHelper(AttackPatterns* pOwner)
    : HelperFsm(STATE_START, STATE_START)
{
    mOwner = pOwner;
    mHint = NULL;
}

// FUNCTION: SWBF2 0x0056aff0
void JetJumpHintHelper::HELPERInit(const JetJumpHint* pHint)
{
    mHint = const_cast<JetJumpHint*>(pHint);
    SetNextState(STATE_GOTO);
    FSMChangeState();
}

// FUNCTION: SWBF2 0x0056b140
void JetJumpHintHelper::ExitState()
{
}

// FUNCTION: SWBF2 0x0056b010
void JetJumpHintHelper::HELPERClean()
{
    ExitState();
}

// FUNCTION: SWBF2 0x0056b020
void JetJumpHintHelper::EnterState()
{
    switch (mCurState)
    {
    case STATE_GOTO:
        reinterpret_cast<AILowLevel*>(GetCtrl()->mLowLevel)
            ->SetNavigator_Goto(mHint->GetCollisionCenter(), NULL, false, false);
        break;

    case STATE_JUMPING:
    {
        const PblVector3& vDir = mHint->GetZAxis();
        PblVector3 vOffset(vDir.x * 80.0f, vDir.y * 80.0f, vDir.z * 80.0f);
        const PblMatrix& mMe = GetMe()->GetMatrix();
        PblVector3 vDest(mMe.m[3][0] + vOffset.x, mMe.m[3][1] + vOffset.y,
                         mMe.m[3][2] + vOffset.z);
        reinterpret_cast<AILowLevel*>(GetCtrl()->mLowLevel)
            ->SetNavigator_Goto(vDest, NULL, false, false);
        reinterpret_cast<AILowLevel*>(GetCtrl()->mLowLevel)->SetJetJump(vDir);
        mJumpTime = GameLoop::GetMissionTime();
        break;
    }
    }
}

// FUNCTION: SWBF2 0x0056b150
void JetJumpHintHelper::UpdateState(float dt)
{
    switch (mCurState)
    {
    case 100:
        break;

    case STATE_GOTO:
        if (reinterpret_cast<AILowLevel*>(GetCtrl()->mLowLevel)->IsAtDest())
        {
            SetNextState(STATE_JUMPING);
        }
        break;

    case STATE_JUMPING:
        if (GameLoop::GetMissionTime() > mJumpTime + 1.0f &&
            !reinterpret_cast<AILowLevel*>(GetCtrl()->mLowLevel)->mJetJumping)
        {
            SetNextState(100);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x0056b1d0
Controllable* JetJumpHintHelper::GetMe()
{
    return mOwner->mOwner->mCtrl->mOwner;
}

// FUNCTION: SWBF2 0x0056b1e0
UnitController* JetJumpHintHelper::GetCtrl()
{
    return mOwner->mOwner->mCtrl;
}

// FUNCTION: SWBF2 0x0056b1c0
const Controllable* JetJumpHintHelper::GetMe() const
{
    return mOwner->mOwner->mCtrl->mOwner;
}

// FUNCTION: SWBF2 0x0056b1f0
const UnitController* JetJumpHintHelper::GetCtrl() const
{
    return mOwner->mOwner->mCtrl;
}

// FUNCTION: SWBF2 0x0056b200
const JetJumpHint* JetJumpHintHelper::FindHint() const
{
    const BaseHint* pHints[30];
    int iCount = HintManager::RequestJetJumpHints(GetCtrl(), GetMe()->GetMatrix().GetTranslation(), 40, 30,
                                                  pHints);
    return SelectHint(iCount, pHints);
}

// FUNCTION: SWBF2 0x0056b250
const JetJumpHint* JetJumpHintHelper::HELPERFindHint() const
{
    return FindHint();
}

// FUNCTION: SWBF2 0x0056b260
const JetJumpHint* JetJumpHintHelper::SelectHint(int iCount, const BaseHint** ppHints) const
{
    static_cast<const Trackable*>(GetMe())->GetGameObject();

    const JetJumpHint* pBest = NULL;
    float fBest = FLT_MAX;
    while (iCount > 0)
    {
        --iCount;
        const int i = AIUtil::GetRandomInt(0, iCount);
        const JetJumpHint* pHint = static_cast<const JetJumpHint*>(ppHints[i]);
        const float fRating = IsHintSuitable(pHint);
        if (fRating < fBest)
        {
            pBest = pHint;
            fBest = fRating;
        }
        ppHints[i] = ppHints[iCount];
    }
    return pBest;
}

// FUNCTION: SWBF2 0x0056b2f0
float JetJumpHintHelper::IsHintSuitable(const JetJumpHint* pHint) const
{
    const PblVector3 vMe = GetMe()->GetMatrix().GetTranslation();
    const PblVector3 vHint = pHint->GetCollisionCenter();
    const float fX = vHint.x - vMe.x;
    const float fZ = vHint.z - vMe.z;
    return fX * fX * 2.25f + fZ * fZ * 2.25f;
}
