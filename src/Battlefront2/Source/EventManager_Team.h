#pragma once

#include "EventManager_Timer.h"
#include "Team.h"

namespace EventManager
{

template <>
bool LuaPushItem<int>(int& item);
template <>
bool LuaPushItem<float>(float& item);

template <>
bool LuaPushItem<Team>(Team& team);

template <class PARAM>
class TeamMatches
{
public:

    TeamMatches(int iTeam) : mTeam(iTeam) {}

    static const char* GetName();

    bool operator()(Team& team, PARAM&) const
    {
        return team.mTeamNumber == mTeam;
    }

    bool operator==(const TeamMatches& other) const
    {
        return mTeam == other.mTeam;
    }

    int mTeam;
};
 
 

namespace TeamEvent
{
extern Event<Team, int> mTicketCountChange;
extern Event<Team, float> mTicketBleedChange;
extern Event<Team, int> mPointsChange;
}

}
