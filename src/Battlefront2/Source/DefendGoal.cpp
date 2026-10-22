#include "DefendGoal.h"
#include "Character.h"
#include "Controllable.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"

// GLOBAL: SWBF2 0x01eb9808
// SYNTHETIC: SWBF2 0x004027f0
// `dynamic initializer for 'AI::DefendGoal::rttiHashDefendGoal''
PblHash AI::DefendGoal::rttiHashDefendGoal("DefendGoal");
char AI::DefendGoal::rttiNameDefendGoal[] = "DefendGoal";

// FUNCTION: SWBF2 0x0048b4f0
bool AI::DefendGoal::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashDefendGoal || AIGoal::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0048b520
unsigned int AI::DefendGoal::GetDerivedRtti() const
{
    return rttiHashDefendGoal;
}

// FUNCTION: SWBF2 0x0048b530
const char* AI::DefendGoal::GetDerivedRttiName() const
{
    return rttiNameDefendGoal;
}

// SYNTHETIC: SWBF2 0x0048b540
// AI::DefendGoal::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0048b570
AI::DefendGoal::DefendGoal(unsigned char uTeam, float fWeight, const GoalTarget& target)
    : AIGoal(AIGOAL_DEFEND, uTeam, fWeight)
{
    mTarget = target;
}

// FUNCTION: SWBF2 0x0048b5d0
void AI::DefendGoal::Assign(Character* pChr)
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
        pController->ForceCommand(UnitGoalAgentInfo(CMD_DEFEND, &mTarget));
    }
}
