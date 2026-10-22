#include "UnitUnBoardAgent.h"
#include "AILowLevel.h"
#include "AttackHelper.h"

// GLOBAL: SWBF2 0x01f9a254
// SYNTHETIC: SWBF2 0x00406d40
// `dynamic initializer for 'UnitUnBoardAgent::rttiHashUnitUnBoardAgent''
PblHash UnitUnBoardAgent::rttiHashUnitUnBoardAgent("UnitUnBoardAgent");
char UnitUnBoardAgent::rttiNameUnitUnBoardAgent[] = "UnitUnBoardAgent";

// FUNCTION: SWBF2 0x0066b1b0
bool UnitUnBoardAgent::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashUnitUnBoardAgent || UnitAgent::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0066b1e0
unsigned int UnitUnBoardAgent::GetDerivedRtti() const
{
    return rttiHashUnitUnBoardAgent;
}

// FUNCTION: SWBF2 0x0066b1f0
const char* UnitUnBoardAgent::GetDerivedRttiName() const
{
    return rttiNameUnitUnBoardAgent;
}

// FUNCTION: SWBF2 0x0066b200
bool UnitUnBoardAgent::ShouldQueueCurCmd(const UnitCmdInfo& cmd) const
{
    return false;
}

// FUNCTION: SWBF2 0x0066b240
UnitUnBoardAgent::UnitUnBoardAgent(UnitController* pCtrl)
    : UnitAgent(pCtrl)
{
    mCurState = 1;
    mNextState = 1;
}

// FUNCTION: SWBF2 0x0066b270
void UnitUnBoardAgent::EnterState()
{
    if (mCurState == 1)
    {
        GetLowLevel().SetExitVehicle(0);
    }
}

// FUNCTION: SWBF2 0x0066b290
void UnitUnBoardAgent::ExitState()
{
}

// FUNCTION: SWBF2 0x0066b2a0
void UnitUnBoardAgent::UpdateStatePostVision(float dt)
{
    if (mCurState != 100)
    {
        reinterpret_cast<AttackHelper*>(mAttackHelper)->Update();
    }
}

// FUNCTION: SWBF2 0x0066b2c0
int UnitUnBoardAgent::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_Damaged:
        PlayDamageEventVO(pInfo);

    case AI::EVT_EmptyVehicle:
    case AI::EVT_Grenade:
    case AI::EVT_Audible_CanHearEnemy:
    case AI::EVT_EndInvisibility:
        return 0;

    default:
        return UnitAgent::EventHandler(pInfo);
    }
}

// FUNCTION: SWBF2 0x0066b320
const char* UnitUnBoardAgent::NFINAL_GetStateString(int iState) const
{
    return 0;
}

// SYNTHETIC: SWBF2 0x0066b210
// UnitUnBoardAgent::`scalar deleting destructor'
