#include "UnitMobileController.h"
#include "PebbleFL/Common/PblHashTable.h"

// GLOBAL: SWBF2 0x01f99414
// SYNTHETIC: SWBF2 0x00406d00
// `dynamic initializer for 'UnitMobileController::rttiHashUnitMobileController''
PblHash UnitMobileController::rttiHashUnitMobileController("UnitMobileController");
char UnitMobileController::rttiNameUnitMobileController[] = "UnitMobileController";

// FUNCTION: SWBF2 0x00669180
bool UnitMobileController::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashUnitMobileController || UnitController::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00669200
UnitMobileController::UnitMobileController(Controllable* pOwner)
    : UnitController(pOwner)
{
}

// SYNTHETIC: SWBF2 0x006691d0
// UnitMobileController::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00669230
void UnitMobileController::ApplyNextState()
{
    switch (GetCurCmd()->GetCommand())
    {
    case CMD_WAIT:
        SetNextState(STATE_WAIT);
        break;
    case CMD_COMBAT:
        SetNextState(STATE_COMBAT);
        break;
    case CMD_WAIT_SECONDARY:
        SetNextState(STATE_WAIT_SECONDARY);
        break;
    case CMD_BOARD:
        SetNextState(STATE_BOARD);
        break;
    case CMD_UNBOARD:
        SetNextState(STATE_UNBOARD);
        break;
    case CMD_RANDOM:
        SetNextState(STATE_RANDOM);
        break;
    case CMD_CAPTURECP:
        SetNextState(STATE_CAPTURECP);
        break;
    case CMD_CTFOFFENSE:
        SetNextState(STATE_CTFOFFENSE);
        break;
    case CMD_DEATHMATCH:
        SetNextState(STATE_DEATHMATCH);
        break;
    case CMD_DEFEND:
        SetNextState(STATE_DEFEND);
        break;
    case CMD_DESTROY:
        SetNextState(STATE_DESTROY);
        break;
    case CMD_SC_FOLLOW:
        SetNextState(STATE_SC_FOLLOW);
        break;
    case CMD_SC_HOLD:
        SetNextState(STATE_SC_HOLD);
        break;
    }
}
