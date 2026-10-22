#include "DestroyGoal.h"
#include "Character.h"
#include "Controllable.h"
#include "Damageable.h"
#include "GameObject.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"

// GLOBAL: SWBF2 0x01eb980c
// SYNTHETIC: SWBF2 0x00402800
// `dynamic initializer for 'AI::DestroyGoal::rttiHashDestroyGoal''
PblHash AI::DestroyGoal::rttiHashDestroyGoal("DestroyGoal");
char AI::DestroyGoal::rttiNameDestroyGoal[] = "DestroyGoal";

// FUNCTION: SWBF2 0x0048c6b0
bool AI::DestroyGoal::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashDestroyGoal || AIGoal::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0048c6e0
unsigned int AI::DestroyGoal::GetDerivedRtti() const
{
    return rttiHashDestroyGoal;
}

// FUNCTION: SWBF2 0x0048c6f0
const char* AI::DestroyGoal::GetDerivedRttiName() const
{
    return rttiNameDestroyGoal;
}

// SYNTHETIC: SWBF2 0x0048c700
// AI::DestroyGoal::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0048c730
AI::DestroyGoal::DestroyGoal(unsigned char uTeam, float fWeight, const GoalTarget& target)
    : AIGoal(AIGOAL_DESTROY, uTeam, fWeight)
{
    mTarget = target;
}

// FUNCTION: SWBF2 0x0048c790
void AI::DestroyGoal::Assign(Character* pChr)
{
    if (!pChr)
    {
        return;
    }
    Controllable* pControllable = pChr->GetControllable();
    if (!pControllable)
    {
        return;
    }
    UnitController* pController = UnitControllerPtr(pControllable->mCtrl);
    if (pController)
    {
        pController->ForceCommand(UnitGoalAgentInfo(CMD_DESTROY, &mTarget));
    }
}

// FUNCTION: SWBF2 0x0048c810
void AI::DestroyGoal::Update()
{
    if (mEnabled)
    {
        if (mTarget.mType == GTT_GAMEOBJECT && mTarget.mGameObj && !mTarget.mGameObj->mIsAlive)
        {
            SetEnabled(false);
            return;
        }
        AIGoal::Update();
    }
    else if (mTarget.mType == GTT_GAMEOBJECT && mTarget.mGameObj && mTarget.mGameObj->mIsAlive)
    {
        SetEnabled(true);
    }
}
