#include "PatrolHelper.h"

#include "AIGoal.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "Character.h"
#include "Entity.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "HintManager.h"
#include "UnitAgent.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblRandom.h"

class EntitySoldier
{
public:
    static PblHash rttiHashEntitySoldier;
};

class EntityDroid
{
public:
    static PblHash rttiHashEntityDroid;
};

extern PblVector3 gVectorZero;

namespace
{

    AILowLevel& LowLevel(UnitController* pCtrl)
    {
        return *reinterpret_cast<AILowLevel*>(pCtrl->mLowLevel);
    }
}

// FUNCTION: SWBF2 0x00614e50
PatrolHelper::PatrolHelper(UnitAgent* pAgent)
    : HelperFsm(STATE_START, STATE_START), mPost(0)
{
    mAgent = pAgent;
    mHint = 0;
}

// FUNCTION: SWBF2 0x00614e80
void PatrolHelper::HELPERInit(const ConstCommandPostHandle& post)
{
    mPost = post;
    mOverrideDefaultFacePos = false;
    mHeight = HEIGHT_STAND;
    mHint = 0;
    mBeginTime = GameLoop::GetMissionTime();
    SetNextState(STATE_START);
    FSMChangeState();
}

// FUNCTION: SWBF2 0x006153b0
void PatrolHelper::ExitState()
{
    mOverrideDefaultFacePos = false;
    mHeight = HEIGHT_STAND;
}

// FUNCTION: SWBF2 0x00614ed0
void PatrolHelper::EnterState()
{
    const CommandPost* pPost = mPost;

    switch (mCurState)
    {
    case STATE_START:
    {
        Controllable* pVehicle = GetMe()->mCharacter->mVehicle;
        if (pVehicle)
        {
            if (pVehicle->GetGameObject()->GetMaxSpeed() < 12.5f)
            {
                const PblMatrix& mVehicle = pVehicle->GetMatrix();
                if (AIUtil::AreEnemiesInRange(pVehicle->GetGameObject()->GetTeam(),
                                              *(const PblVector3*)&mVehicle.m[3][0], 20.0f) == true)
                {
                    AI::GoalTarget target(AIUtil::GetCommandPostPos(pPost));
                    UnitGoalAgentInfo cmd(CMD_DEFEND, &target);
                    GetCtrl()->ForceCommand(cmd);
                    SetNextState(100);
                    return;
                }
            }
            LowLevel(GetCtrl()).SetExitVehicle(NULL);
            GetMe()->mCharacter->SetVehicleBlockTime(GameLoop::GetMissionTime() + 5.0f);
            SetNextState(100);
            return;
        }

        if (!GetMe()->GetGameObject()->IsMyFriend(pPost->GetGameObject()))
        {
            SetNextState(STATE_ACQUIRE);
            return;
        }

        bool bTryHint = AIUtil::GetRandomInt(1, 4) != 1;
        bool bSoldier = GetMe()->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier);
        if (bTryHint && bSoldier)
        {
            const BaseHint* hints[10];
            int iCount = HintManager::RequestPatrolHints(GetCtrl(), mPost, 10, hints);
            mHint = const_cast<PatrolHint*>(SelectHint(iCount, hints));
            if (mHint)
            {
                SetNextState(STATE_GOTO_HINT);
            }
            else
            {
                SetNextState(STATE_GOTO_BASE);
            }
        }
        else
        {
            SetNextState(STATE_GOTO_BASE);
        }
        break;
    }

    case STATE_ACQUIRE:
    {
        PblVector3 vPos = AIUtil::GetCommandPostPos(pPost);
        PblVector3 vOffset = AIUtil::GetCommandPostOffset(pPost);
        PblVector3 vDest(vOffset.x + vPos.x, vPos.y + vOffset.y, vPos.z + vOffset.z);
        LowLevel(GetCtrl()).SetNavigator_Goto(vDest, NULL, false, false);
        break;
    }

    case STATE_GOTO_HINT:
        mHint->Reserve(GetMe()->GetGameObject());
        LowLevel(GetCtrl()).SetNavigator_Goto(mHint->GetCollisionCenter(), NULL, false, false);
        mNextPatrolTime = AIUtil::GetRandomInt(30, 45) + GameLoop::GetMissionTime();
        break;

    case STATE_GOTO_BASE:
    {
        mNextPatrolTime = AIUtil::GetRandomInt(20, 30) + GameLoop::GetMissionTime();
        if (!GetMe()->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier) &&
            !GetMe()->GetEntity()->IsRtti(EntityDroid::rttiHashEntityDroid) &&
            AIUtil::IsNearPost(GetMe(), pPost, gVectorZero))
        {
            LowLevel(GetCtrl()).SetNavigator_Stationary();
            SetNextState(STATE_GUARD_BASE);
            return;
        }
        PblVector3 vPos = AIUtil::GetCommandPostPos(pPost);
        PblVector3 vDir = PblRandom::_Sim.GetVector3();
        vDir.y = 0.0f;
        float fDist = AIUtil::GetRandomFloat(8.0f, 14.0f);
        vPos.x += vDir.x * fDist;
        vPos.y += vDir.y * fDist;
        vPos.z += vDir.z * fDist;
        LowLevel(GetCtrl()).SetNavigator_Slide(vPos, NULL);
        break;
    }

    case STATE_GUARD_HINT:
    {
        mOverrideDefaultFacePos = true;
        const PblVector3& vForward = mHint->GetZAxis();
        PblVector3 vLook(vForward.x * 10.0f, vForward.y * 10.0f, vForward.z * 10.0f);
        PblVector3 vDest = LowLevel(GetCtrl()).GetGlobalDest();
        mDefaultFacePos = PblVector3(vDest.x + vLook.x, vDest.y + vLook.y, vDest.z + vLook.z);
        mHeight = mHint->GetRandomPrimaryStance();
        break;
    }

    case STATE_GUARD_BASE:
        mHeight = HEIGHT_STAND;
        break;
    }
}

// FUNCTION: SWBF2 0x006153c0
void PatrolHelper::UpdateState(float dt)
{
    if (mCurState == 100)
    {
        return;
    }

    const CommandPost* pPost = mPost;

    if (GameLoop::GetMissionTime() > mBeginTime + 5.0f)
    {
        mBeginTime = 1.0e8f;
        if (!pPost->mCaptureRegion || pPost->CanTeamSpawnHere(GetMe()->mCharacter->mTeamNumber))
        {
            SetNextState(100);
            return;
        }
    }

    switch (mCurState)
    {
    case STATE_ACQUIRE:
        if (GetMe()->GetGameObject()->IsMyFriend(pPost->GetGameObject()))
        {
            SetNextState(STATE_START);
        }
        break;

    case STATE_GOTO_HINT:
        if (LowLevel(GetCtrl()).IsAtDest())
        {
            SetNextState(STATE_GUARD_HINT);
            break;
        }
        mHint->Reserve(GetMe()->GetGameObject());
        if (GameLoop::GetMissionTime() > mNextPatrolTime)
        {
            SetNextState(STATE_START);
        }
        break;

    case STATE_GOTO_BASE:
        if (LowLevel(GetCtrl()).IsAtDest())
        {
            SetNextState(STATE_GUARD_BASE);
            break;
        }
        if (GameLoop::GetMissionTime() > mNextPatrolTime)
        {
            SetNextState(STATE_START);
        }
        break;

    case STATE_GUARD_HINT:
        if (GameLoop::GetMissionTime() > mNextPatrolTime)
        {
            SetNextState(STATE_START);
            break;
        }
        mHint->Reserve(GetMe()->GetGameObject());
        break;

    case STATE_GUARD_BASE:
        if (GameLoop::GetMissionTime() > mNextPatrolTime)
        {
            SetNextState(STATE_START);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x006155d0
const PatrolHint* PatrolHelper::SelectHint(int i, const BaseHint** ppHint)
{
    const GameObject* pMe = GetMe()->GetGameObject();
    while (i > 0)
    {
        --i;
        const BaseHint** ppPick = &ppHint[AIUtil::GetRandomInt(0, i)];
        const BaseHint* pHint = *ppPick;
        if (AIUtil::IsHintAvailable(pMe, pHint) && pHint != mHint)
        {
            return static_cast<const PatrolHint*>(pHint);
        }
        *ppPick = ppHint[i];
    }
    return NULL;
}

// FUNCTION: SWBF2 0x006155b0
Controllable* PatrolHelper::GetMe()
{
    return mAgent->mCtrl->mOwner;
}

const Controllable* PatrolHelper::GetMe() const
{
    return mAgent->mCtrl->mOwner;
}

// FUNCTION: SWBF2 0x006155c0
UnitController* PatrolHelper::GetCtrl()
{
    return mAgent->mCtrl;
}

const UnitController* PatrolHelper::GetCtrl() const
{
    return mAgent->mCtrl;
}

// FUNCTION: SWBF2 0x00615580
bool PatrolHelper::HELPERGetOverrideDefaultFacePos() const
{
    return mOverrideDefaultFacePos;
}

// FUNCTION: SWBF2 0x00615590
const PblVector3& PatrolHelper::HELPERGetDefaultFacePos() const
{
    return mDefaultFacePos;
}

// FUNCTION: SWBF2 0x006155a0
ControllableHeight PatrolHelper::HELPERGetHeight() const
{
    return mHeight;
}

// FUNCTION: SWBF2 0x00615650
bool PatrolHelper::HELPERGrenadeEventInterrupt() const
{
    return true;
}

// FUNCTION: SWBF2 0x00615660
bool PatrolHelper::HELPERDamagedInterrupt(bool b, const PblVector3& vPos) const
{
    return true;
}

// FUNCTION: SWBF2 0x00615670
bool PatrolHelper::HELPERSeeEnemyInterrupt(const GameObject* pEnemy) const
{
    return true;
}

// FUNCTION: SWBF2 0x00615680
bool PatrolHelper::HELPERHearEnemyInterrupt(const GameObject* pEnemy) const
{
    return true;
}

// FUNCTION: SWBF2 0x00615690
bool PatrolHelper::HELPERVehicleNearbyInterrupt(const GameObject* pVehicle) const
{
    return true;
}

// FUNCTION: SWBF2 0x00614ec0
void PatrolHelper::HELPERClean()
{
    ExitState();
}
