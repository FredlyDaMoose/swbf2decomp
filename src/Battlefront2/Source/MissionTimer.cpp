#include "MissionTimer.h"
#include "HUDEvent.h"
#include "HUDGameEvents.h"
#include "NetGame.h"
#include "Team.h"

class Timer;

// GLOBAL: SWBF2 0x01fa2480
static Timer* mTimer;

// GLOBAL: SWBF2 0x01fa2470
static MissionTimer::Show mShow;

// GLOBAL: SWBF2 0x01fa2478
static int mTeam;

// GLOBAL: SWBF2 0x01fa2490
static Timer* mHostTimer;
// GLOBAL: SWBF2 0x01fa2474
static MissionTimer::Show mHostShow;
// GLOBAL: SWBF2 0x01fa247c
static int mHostTeam;
// GLOBAL: SWBF2 0x01fa2488
static Timer* mClientTimer;
// GLOBAL: SWBF2 0x01fa2484
static MissionTimer::Show mClientShow;
// GLOBAL: SWBF2 0x01fa248c
static int mClientTeam;

// FUNCTION: SWBF2 0x005a4cf0
void MissionTimer::Init()
{
    mTimer = 0;
    mShow = SHOW_NEUTRAL;
    mTeam = 0;
}

// FUNCTION: SWBF2 0x005a4d10
void MissionTimer::Set(Timer* pTimer, Show eShow, int iTeam)
{
    mShow = eShow;
    mTeam = iTeam;
    mTimer = pTimer;
    if ((!NetGame::IsEnabled() || NetGame::IsClient() || (NetGame::IsHost() && NetGame::IsOneWorld())) &&
        !pTimer)
    {
        HUD::Event(HUD::GameEvents::gObjectiveEvents.defeatTimerDisable, true).Send();
        HUD::Event(HUD::GameEvents::gObjectiveEvents.victoryTimerDisable, true).Send();
    }
}

// FUNCTION: SWBF2 0x005a4db0
Timer* MissionTimer::GetTimer()
{
    return mTimer;
}

// FUNCTION: SWBF2 0x005a4dc0
MissionTimer::Show MissionTimer::GetShow()
{
    return mShow;
}

// FUNCTION: SWBF2 0x005a4dd0
int MissionTimer::GetTeam()
{
    return mTeam;
}

// FUNCTION: SWBF2 0x005a4de0
MissionTimer::Show MissionTimer::GetTeamShow(int iTeam)
{
    if (mTeam <= 0)
    {
        return mShow;
    }
    return (Show)(mShow * Team::sTeams[iTeam]->mAffiliation[mTeam]);
}

// FUNCTION: SWBF2 0x005a4e10
void MissionTimer::EnterClient()
{
    mHostTimer = mTimer;
    mHostShow = mShow;
    mHostTeam = mTeam;
    mTimer = mClientTimer;
    mShow = mClientShow;
    mTeam = mClientTeam;
}

// FUNCTION: SWBF2 0x005a4e50
void MissionTimer::LeaveClient()
{
    mClientTimer = mTimer;
    mClientShow = mShow;
    mClientTeam = mTeam;
    mTimer = mHostTimer;
    mShow = mHostShow;
    mTeam = mHostTeam;
}
