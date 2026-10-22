#pragma once

#include "LuaCallbacks.h"

namespace Lua_Callbacks
{

int ScriptCB_ClientGotStats(lua_State* L);
int ScriptCB_SetTeamStatsSortMode(lua_State* L);
int ScriptCB_GetTeamstats(lua_State* L);
int ScriptCB_TeamStatsGetTeam1(lua_State* L);
int ScriptCB_TeamStatsGetTeamColor(lua_State* L);
int ScriptCB_TeamStatsSetTitles(lua_State* L);
int ScriptCB_PersonalStatsSetTitles(lua_State* L);
int ScriptCB_TeamStatsValidatePos(lua_State* L);
int ScriptCB_GetPersonalStats(lua_State* L);
int ScriptCB_GetAwardStats(lua_State* L);
int ScriptCB_GetMedalStats(lua_State* L);
int ScriptCB_GetCareerPersonalStats(lua_State* L);
int ScriptCB_GetCareerRank(lua_State* L);
int ScriptCB_GetCareerMedals(lua_State* L);
int ScriptCB_GetCareerMedalLevel(lua_State* L);
int ScriptCB_SetPlayerStatsPoints(lua_State* L);
int ScriptCB_GetRank(lua_State* L);
int ScriptCB_GetPlayerIDAtRank(lua_State* L);

void InitStats();

int GetStatsPageJoystick();

}

const char* GetSideIcon(int iSide);
int __cdecl TeamStatSortCallback(const void* pA, const void* pB);
extern int gTeamStatsSortMode;
extern bool gTeamStatsSortInvert;
