#include "UnitFlyController.h"
#include "UnitFlyAgent.h"

// GLOBAL: SWBF2 0x01f993e4
// SYNTHETIC: SWBF2 0x00406cd0
// `dynamic initializer for 'UnitFlyController::rttiHashUnitFlyController''
PblHash UnitFlyController::rttiHashUnitFlyController("UnitFlyController");
char UnitFlyController::rttiNameUnitFlyController[] = "UnitFlyController";

// FUNCTION: SWBF2 0x00667d70
UnitFlyController::UnitFlyController(Controllable* pOwner)
    : UnitController(pOwner)
{
}

// FUNCTION: SWBF2 0x00667cf0
bool UnitFlyController::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashUnitFlyController || UnitController::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00667d20
unsigned int UnitFlyController::GetDerivedRtti() const
{
    return rttiHashUnitFlyController;
}

// FUNCTION: SWBF2 0x00667d30
const char* UnitFlyController::GetDerivedRttiName() const
{
    return rttiNameUnitFlyController;
}

// FUNCTION: SWBF2 0x00667da0
void UnitFlyController::ApplyNextState()
{
    const UnitCmdInfo* pCmd = GetCurCmd();
    switch (pCmd->GetCommand())
    {
    case CMD_COMBAT:
        SetNextState(STATE_COMBAT);
        break;
    case CMD_WAIT_SECONDARY:
        SetNextState(STATE_WAIT_SECONDARY);
        break;
    case CMD_CAPTURECP:
    case CMD_CTFOFFENSE:
    case CMD_DEATHMATCH:
    case CMD_DEFEND:
    case CMD_DESTROY:
        if (mCurState != STATE_FLY)
        {
            if (mJustEnteredVehicleGoToPickupAgent)
            {
                mJustEnteredVehicleGoToPickupAgent = false;
                PrependQueueCommands(*pCmd, UnitWaitSecondaryInfo(4.0f, false));
                ApplyNextCommand();
                mForceCommand = false;
                ApplyNextState();
            }
            else
            {
                SetNextState(STATE_FLY);
            }
        }
        break;
    case CMD_SC_HOLD:
        SetNextState(STATE_SC_HOLD);
        break;
    }
}

// FUNCTION: SWBF2 0x00667e80
void UnitFlyController::EnterState()
{
    if (mCurState == STATE_FLY)
    {
        mAgent = new UnitFlyAgent(this);
    }
    else
    {
        UnitController::EnterState();
    }
}

// FUNCTION: SWBF2 0x00667ec0
void UnitFlyController::ExitState()
{
    UnitController::ExitState();
}

// FUNCTION: SWBF2 0x00667ed0
void UnitFlyController::UpdateState(float dt)
{
    UnitController::UpdateState(dt);
}
