#include "DeathmatchGoal.h"
#include "Character.h"
#include "Controllable.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"

// GLOBAL: SWBF2 0x01eb97b4
// SYNTHETIC: SWBF2 0x004027c0
// `dynamic initializer for 'AI::DeathmatchGoal::rttiHashDeathmatchGoal''
PblHash AI::DeathmatchGoal::rttiHashDeathmatchGoal("DeathmatchGoal");
char AI::DeathmatchGoal::rttiNameDeathmatchGoal[] = "DeathmatchGoal";

// FUNCTION: SWBF2 0x0048a980
bool AI::DeathmatchGoal::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashDeathmatchGoal || AIGoal::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0048a9b0
unsigned int AI::DeathmatchGoal::GetDerivedRtti() const
{
    return rttiHashDeathmatchGoal;
}

// FUNCTION: SWBF2 0x0048a9c0
const char* AI::DeathmatchGoal::GetDerivedRttiName() const
{
    return rttiNameDeathmatchGoal;
}

// SYNTHETIC: SWBF2 0x0048a9d0
// AI::DeathmatchGoal::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0048aa00
AI::DeathmatchGoal::DeathmatchGoal(unsigned char uTeam, float fWeight)
    : AIGoal(AIGOAL_DEATHMATCH, uTeam, fWeight)
{
}

// FUNCTION: SWBF2 0x0048aa20
void AI::DeathmatchGoal::Assign(Character* pChr)
{
    if (!pChr || !pChr->GetControllable())
    {
        return;
    }

    UnitController* pCtrl = UnitControllerPtr(pChr->GetControllable()->mCtrl);
    if (pCtrl)
    {
        CmdInfo cmd = UnitCmdInfo(CMD_DEATHMATCH, PblHandle<GameObject>(),
                                  PblVector3(0.0f, 0.0f, 0.0f), PblHandle<GameObject>());
        pCtrl->ForceCommand(cmd);
    }
}
