#include "LuaCallbacks_Stats.h"
#include "LuaHelper.h"
#include "PlayerProfile.h"
#include "PlayerStats.h"
#include "MedalsMgr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Character.h"
#include "Entity.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "Team.h"
#include "Weapon.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Interface/Red3DModelElementLite.h"
#include "RedEngineFL/Interface/RedInterfaceElement.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

using namespace PblUnicode;

extern int gActiveViewport;

// GLOBAL: SWBF2 0x01e57990
int gTeamStatsSortMode;
// GLOBAL: SWBF2 0x01e5794a
bool gTeamStatsSortInvert;

// FUNCTION: SWBF2 0x0059eac0
const char* GetSideIcon(int iSide)
{
    static const char* const sIcons[] =
    {
        0, "com_icon_alliance", "com_icon_imperial", "com_icon_republic", "com_icon_CIS", 0, 0
    };
    if (iSide == Team::side_Locals)
    {
        if (GameLoop::mMissionScriptHash == 0x2b6c2d0d || GameLoop::mMissionScriptHash == 0x41c2e16a ||
            GameLoop::mMissionScriptHash == 0xcdb830bd)
        {
            return "com_icon_republic";
        }
        return 0;
    }
    return sIcons[iSide];
}

// FUNCTION: SWBF2 0x005a3180
int MedalsMgr::GetMedalLevel(int iCount)
{
    if (iCount >= 64)
    {
        return 3;
    }
    if (iCount >= 32)
    {
        return 2;
    }
    return iCount >= 4;
}

// FUNCTION: SWBF2 0x005a34e0
int MedalsMgr::GetRankLevel(int iCount)
{
    if (iCount >= 300)
    {
        return 3;
    }
    if (iCount >= 100)
    {
        return 2;
    }
    return iCount >= 20;
}

// FUNCTION: SWBF2 0x005a3870
short MedalsMgr::GetMedalsThisGame(int iType) const
{
    if (!mOwner || NetGame::IsEnabled())
    {
        return 0;
    }
    PlayerProfile::tProfile* pProfile = mOwner->GetProfile();
    short sCurrent = GetMedals(iType);
    return pProfile ? sCurrent - pProfile->mGameOptions.mMedalsGathered[iType] : sCurrent;
}

namespace Lua_Callbacks
{

static void Stats_AddItem(const unsigned short* pusLabel, const unsigned short* pusContents);
static void Stats_AddItem(const unsigned short* pusLabel, int iValue);
static void CareerMedals_AddItem(const unsigned short* pusLabel, int iVal1, int iVal2);

// FUNCTION: SWBF2 0x005a07f0
int ScriptCB_GetRank(lua_State*)
{
    if (!lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }
    float fTeam = (float)luaL_checknumber(LuaHelper::mState, 1);
    if (fTeam < 1.0f || fTeam > 2.0f || !lua_isnumber(LuaHelper::mState, 2))
    {
        return 0;
    }
    float fPlayerID = (float)luaL_checknumber(LuaHelper::mState, 2);
    if (fPlayerID < 0.0f || fPlayerID > 999.0f)
    {
        return 0;
    }

    Team* pTeam = Team::sTeams[(int)fTeam];
    Character* apSorted[600];
    for (int i = 0; i < pTeam->mMemberCount; ++i)
    {
        apSorted[i] = pTeam->GetMemberPtr(i);
    }
    qsort(apSorted, pTeam->mMemberCount, sizeof(Character*), TeamStatSortCallback);
    int iRank = -1;
    for (int i = 0; i < pTeam->mMemberCount; ++i)
    {
        PlayerStats& stats = apSorted[i]->GetPlayerStats();
        stats.mRankOrder = i;
        if (apSorted[i]->mPlayerId == (int)fPlayerID && NetGame::GetJoystickIndex(apSorted[i]->mPlayerId) >= 0)
        {
            iRank = i;
        }
    }
    lua_pushnumber(LuaHelper::mState, (float)(iRank + 1));
    return 1;
}

// FUNCTION: SWBF2 0x005a0960
int ScriptCB_GetPlayerIDAtRank(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 2 || !lua_isnumber(LuaHelper::mState, 1)
        || !lua_isnumber(LuaHelper::mState, 2))
    {
        return 0;
    }
    if (NetGame::IsEnabled())
    {
        lua_pushnumber(LuaHelper::mState, -1.0f);
        return 1;
    }
    int iTeam = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iRank = (int)luaL_checknumber(LuaHelper::mState, 2);
    Team* pTeam = Team::sTeams[iTeam];
    if (iRank < 0 || pTeam->mMemberCount <= 0)
    {
        return 0;
    }
    Character* pCharacter = 0;
    for (int i = 0; i < pTeam->mMemberCount; ++i)
    {
        pCharacter = pTeam->GetMemberPtr(i);
        if (pCharacter->GetPlayerStats().mRankOrder == iRank - 1)
        {
            break;
        }
    }
    if (!pCharacter)
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, (float)pCharacter->mPlayerId);
    return 1;
}

// FUNCTION: SWBF2 0x0059f8b0
int ScriptCB_GetMedalStats(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 2 || !lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }
    float fTeam = (float)luaL_checknumber(LuaHelper::mState, 1);
    if (fTeam < 1.0f || fTeam > 2.0f || !lua_isnumber(LuaHelper::mState, 2))
    {
        return 0;
    }
    float fRank = (float)luaL_checknumber(LuaHelper::mState, 2);
    if (fRank < 0.0f || fRank > 999.0f)
    {
        return 0;
    }
    DoPeriodicLuaGC(true);
    if (NetGame::IsEnabled() && NetGame::IsClient() && !PlayerStats::mGotMPStats)
    {
        lua_pushnil(LuaHelper::mState);
        LuaHelper::SetGlobal("stats_listbox_contents");
        DoPeriodicLuaGC(false);
        lua_newtable(LuaHelper::mState);
        lua_newtable(LuaHelper::mState);
        unsigned short usText[30];
        ustrncnv(usText, "fix MP stats", 30);
        Stats_AddItem(usText, usText);
        lua_rawseti(LuaHelper::mState, -2, 1);
        LuaHelper::SetGlobal("stats_listbox_contents");
        DoPeriodicLuaGC(false);
        return 0;
    }
    lua_gettop(LuaHelper::mState);
    lua_pushnil(LuaHelper::mState);
    LuaHelper::SetGlobal("stats_listbox_contents");
    DoPeriodicLuaGC(false);
    lua_newtable(LuaHelper::mState);
    Team* pTeam = Team::sTeams[(int)fTeam];
    Character* pCharacter = 0;
    for (int i = 0; i < pTeam->mMemberCount; ++i)
    {
        pCharacter = pTeam->GetMemberPtr(i);
        if (pCharacter->GetPlayerStats().mRankOrder == (int)fRank - 1)
        {
            break;
        }
    }
    MedalsMgr* pMedals = pCharacter->mMedalsMgr;
    for (int i = 0; i < 9; ++i)
    {
        lua_newtable(LuaHelper::mState);
        LuaHelper::TableWriteVal("medaltype", (int)pMedals->GetMedalsThisGame(i));
        lua_rawseti(LuaHelper::mState, -2, i + 1);
    }
    lua_pushstring(LuaHelper::mState, "stats_listbox_contents");
    lua_insert(LuaHelper::mState, -2);
    lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);
    DoPeriodicLuaGC(false);
    return 0;
}

// FUNCTION: SWBF2 0x005a00a0
int ScriptCB_GetCareerRank(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 1 || !lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }
    int iCharacter = (int)luaL_checknumber(LuaHelper::mState, 1);
    Character* pCharacter = Character::sCharacters && iCharacter >= 0 &&
        iCharacter < Character::sCharacterLimit ? &Character::sCharacters[iCharacter] : 0;
    lua_pushnumber(LuaHelper::mState, (float)pCharacter->mMedalsMgr->GetRank());
    return 1;
}

// FUNCTION: SWBF2 0x005a0630
int ScriptCB_GetCareerMedalLevel(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 2)
    {
        return 0;
    }
    int iCharacter = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iType = (int)luaL_checknumber(LuaHelper::mState, 2);
    int iLevel;
    if (iCharacter < 0)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(GetStatsPageJoystick());
        iLevel = MedalsMgr::GetMedalLevel(pProfile->mGameOptions.mMedalsGathered[iType]);
    }
    else
    {
        Character* pCharacter = Character::sCharacters && iCharacter < Character::sCharacterLimit
            ? &Character::sCharacters[iCharacter] : 0;
        iLevel = pCharacter->mMedalsMgr->GetLevel(iType);
    }
    lua_pushnumber(LuaHelper::mState, (float)iLevel);
    return 1;
}

// FUNCTION: SWBF2 0x0059fb20
int ScriptCB_GetCareerPersonalStats(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 2 || !lua_isnumber(LuaHelper::mState, 1) ||
        !lua_isnumber(LuaHelper::mState, 2))
    {
        return 0;
    }
    int iCharacter = (int)luaL_checknumber(LuaHelper::mState, 1);
    float fPage = (float)luaL_checknumber(LuaHelper::mState, 2);
    Character* pCharacter = 0;
    PlayerStats* pStats = 0;
    PlayerProfile::tProfile* pProfile;
    if (iCharacter >= 0)
    {
        pCharacter = Character::GetPtr(iCharacter);
        pStats = &pCharacter->GetPlayerStats();
        pProfile = pCharacter->GetProfile();
    }
    else
    {
        pProfile = PlayerProfile::GetProfile(GetStatsPageJoystick());
    }
    DoPeriodicLuaGC(true);

    if (NetGame::IsEnabled() && NetGame::IsClient() && !PlayerStats::mGotMPStats)
    {
        lua_pushnil(LuaHelper::mState);
        LuaHelper::SetGlobal("stats_listbox_contents");
        DoPeriodicLuaGC(false);
        lua_newtable(LuaHelper::mState);
        lua_newtable(LuaHelper::mState);
        unsigned short usText[30];
        ustrncnv(usText, "fix MP stats", 30);
        Stats_AddItem(usText, usText);
        lua_rawseti(LuaHelper::mState, -2, 1);
        LuaHelper::SetGlobal("stats_listbox_contents");
        DoPeriodicLuaGC(false);
        return 0;
    }

    lua_gettop(LuaHelper::mState);
    lua_pushnil(LuaHelper::mState);
    LuaHelper::SetGlobal("stats_listbox_contents");
    DoPeriodicLuaGC(false);
    lua_newtable(LuaHelper::mState);

    int iPage = (int)fPage;
    int iFirst = (iPage != 0) * 4;
    int iEnd = (iPage != 0) * 4 + 4;
    for (int i = iFirst; i < iEnd; ++i)
    {
        lua_newtable(LuaHelper::mState);
        switch (i)
        {
        case 0:
            Stats_AddItem(RedLocalizeDB::Find(0xc38cd05fu, false),
                          pProfile ? (int)pProfile->mAccumPoints.mTotalPoints : pStats->GetPoints());
            break;
        case 1:
        {
            unsigned short usBlank[5];
            ustrncnv(usBlank, "", 5);
            Stats_AddItem(usBlank, usBlank);
            break;
        }
        case 2:
            Stats_AddItem(RedLocalizeDB::Find(0x26adb709u, false),
                          pProfile ? (int)pProfile->mAccumPoints.mTotalKills
                                   : pStats->GetKills(-1, 1) + pStats->GetKills(-1, 0));
            break;
        case 3:
            Stats_AddItem(RedLocalizeDB::Find(0x39ef9b83u, false),
                          pProfile ? (int)pProfile->mAccumPoints.mTotalDeaths
                                   : pStats->GetDeaths(-1, 1) + pStats->GetDeaths(-1, 0));
            break;
        case 4:
        {
            int iRank;
            if (pCharacter)
            {
                iRank = pCharacter->mMedalsMgr->GetRank();
            }
            else
            {
                int iMedals = 0;
                for (int j = 0; j < 9; ++j)
                {
                    iMedals += pProfile->mGameOptions.mMedalsGathered[j];
                }
                iRank = MedalsMgr::GetRankLevel(iMedals);
            }
            unsigned int uRankName = 0;
            switch (iRank)
            {
            case 0: uRankName = 0x4191047cu; break;
            case 1: uRankName = 0xf0532ad2u; break;
            case 2: uRankName = 0x9937379bu; break;
            case 3: uRankName = 0xa03313bbu; break;
            }
            Stats_AddItem(RedLocalizeDB::Find(0x7d54045eu, false), RedLocalizeDB::Find(uRankName, false));
            break;
        }
        case 5:
        {
            unsigned short usBlank[5];
            ustrncnv(usBlank, "", 5);
            Stats_AddItem(usBlank, usBlank);
            break;
        }
        case 6:
        {
            int iKills, iDeaths;
            if (pProfile)
            {
                iKills = pProfile->mAccumPoints.mTotalKills;
                iDeaths = pProfile->mAccumPoints.mTotalDeaths;
            }
            else
            {
                iKills = pStats->GetKills(-1, 1) + pStats->GetKills(-1, 0);
                iDeaths = pStats->GetDeaths(-1, 1) + pStats->GetDeaths(-1, 0);
            }
            LuaHelper::TableWriteVal("labelustr", RedLocalizeDB::Find(0xfa323a81u, false));
            if (iDeaths)
            {
                char acRatio[8];
                _snprintf(acRatio, 8, "%.2f", (float)iKills / (float)iDeaths);
                LuaHelper::TableWriteVal("contentsstr", acRatio);
            }
            else
            {
                LuaHelper::TableWriteVal("contentsstr", "--");
            }
            break;
        }
        case 7:
        {
            int iPoints, iLives;
            if (pProfile)
            {
                iPoints = pProfile->mAccumPoints.mTotalPoints;
                iLives = pProfile->mAccumPoints.mTotalDeaths + 1;
            }
            else
            {
                iPoints = pStats->GetPoints();
                iLives = pStats->GetDeaths(-1, 1) + pStats->GetDeaths(-1, 0) + 1;
            }
            Stats_AddItem(RedLocalizeDB::Find(0xca14cb66u, false), iPoints / iLives);
            break;
        }
        }
        lua_rawseti(LuaHelper::mState, -2, i - iFirst + 1);
    }

    lua_pushstring(LuaHelper::mState, "stats_listbox_contents");
    lua_insert(LuaHelper::mState, -2);
    lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);
    DoPeriodicLuaGC(false);
    return 0;
}

// FUNCTION: SWBF2 0x005a0140
int ScriptCB_GetCareerMedals(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 2 || !lua_isnumber(LuaHelper::mState, 1) ||
        !lua_isnumber(LuaHelper::mState, 2))
    {
        return 0;
    }
    int iCharacter = (int)luaL_checknumber(LuaHelper::mState, 1);
    float fPage = (float)luaL_checknumber(LuaHelper::mState, 2);
    const char* pcTable = (int)fPage ? "stats_medalbox_right_contents" : "stats_medalbox_left_contents";
    DoPeriodicLuaGC(true);

    if (NetGame::IsEnabled() && NetGame::IsClient() && !PlayerStats::mGotMPStats)
    {
        lua_pushnil(LuaHelper::mState);
        LuaHelper::SetGlobal(pcTable);
        DoPeriodicLuaGC(false);
        lua_newtable(LuaHelper::mState);
        lua_newtable(LuaHelper::mState);
        unsigned short usText[30];
        ustrncnv(usText, "fix MP stats", 30);
        Stats_AddItem(usText, usText);
        lua_rawseti(LuaHelper::mState, -2, 1);
        LuaHelper::SetGlobal(pcTable);
        DoPeriodicLuaGC(false);
        return 0;
    }

    lua_gettop(LuaHelper::mState);
    lua_pushnil(LuaHelper::mState);
    LuaHelper::SetGlobal(pcTable);
    DoPeriodicLuaGC(false);
    lua_newtable(LuaHelper::mState);

    const short* psGathered = 0;
    MedalsMgr* pMedals = 0;
    if (iCharacter < 0)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(GetStatsPageJoystick());
        if (pProfile)
        {
            psGathered = pProfile->mGameOptions.mMedalsGathered;
        }
    }
    else
    {
        Character* pCharacter = Character::GetPtr(iCharacter);
        if (pCharacter)
        {
            pMedals = pCharacter->mMedalsMgr;
        }
    }

    int iPage = (int)fPage;
    int iFirst = (iPage != 0) * 5;
    int iEnd = (iPage != 0) * 4 + 5;
    for (int i = iFirst; i < iEnd; ++i)
    {
        lua_newtable(LuaHelper::mState);
        switch (i)
        {
        case 0:
            CareerMedals_AddItem(RedLocalizeDB::Find(0xcafcdffeu, false),
                                 pMedals ? pMedals->GetMedals(0) : psGathered[0],
                                 pMedals ? pMedals->GetLevel(0) : MedalsMgr::GetMedalLevel(psGathered[0]));
            break;
        case 1:
            CareerMedals_AddItem(RedLocalizeDB::Find(0x4c112ebcu, false),
                                 pMedals ? pMedals->GetMedals(2) : psGathered[2],
                                 pMedals ? pMedals->GetLevel(2) : MedalsMgr::GetMedalLevel(psGathered[2]));
            break;
        case 2:
            CareerMedals_AddItem(RedLocalizeDB::Find(0x21c98eadu, false),
                                 pMedals ? pMedals->GetMedals(5) : psGathered[5],
                                 pMedals ? pMedals->GetLevel(5) : MedalsMgr::GetMedalLevel(psGathered[5]));
            break;
        case 3:
            CareerMedals_AddItem(RedLocalizeDB::Find(0xe4035095u, false),
                                 pMedals ? pMedals->GetMedals(7) : psGathered[7],
                                 pMedals ? pMedals->GetLevel(7) : MedalsMgr::GetMedalLevel(psGathered[7]));
            break;
        case 4:
            CareerMedals_AddItem(RedLocalizeDB::Find(0xea62b23cu, false),
                                 pMedals ? pMedals->GetMedals(1) : psGathered[1],
                                 pMedals ? pMedals->GetLevel(1) : MedalsMgr::GetMedalLevel(psGathered[1]));
            break;
        case 5:
            CareerMedals_AddItem(RedLocalizeDB::Find(0xed854facu, false),
                                 pMedals ? pMedals->GetMedals(3) : psGathered[3],
                                 pMedals ? pMedals->GetLevel(3) : MedalsMgr::GetMedalLevel(psGathered[3]));
            break;
        case 6:
            CareerMedals_AddItem(RedLocalizeDB::Find(0xd7a03b48u, false),
                                 pMedals ? pMedals->GetMedals(4) : psGathered[4],
                                 pMedals ? pMedals->GetLevel(4) : MedalsMgr::GetMedalLevel(psGathered[4]));
            break;
        case 7:
            CareerMedals_AddItem(RedLocalizeDB::Find(0x4e47ef5bu, false),
                                 pMedals ? pMedals->GetMedals(6) : psGathered[6],
                                 pMedals ? pMedals->GetLevel(6) : MedalsMgr::GetMedalLevel(psGathered[6]));
            break;
        case 8:
            CareerMedals_AddItem(RedLocalizeDB::Find(0xfd7404ccu, false),
                                 pMedals ? pMedals->GetMedals(8) : psGathered[8],
                                 pMedals ? pMedals->GetLevel(8) : MedalsMgr::GetMedalLevel(psGathered[8]));
            break;
        }
        lua_rawseti(LuaHelper::mState, -2, i - iFirst + 1);
    }

    lua_pushstring(LuaHelper::mState, pcTable);
    lua_insert(LuaHelper::mState, -2);
    lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);
    DoPeriodicLuaGC(false);
    return 0;
}

// FUNCTION: SWBF2 0x0059def0
static void Stats_AddItem(const unsigned short* pusLabel, const char* pcContents)
{
    LuaHelper::TableWriteVal("labelustr", pusLabel);
    LuaHelper::TableWriteVal("contentsstr", pcContents);
}

// FUNCTION: SWBF2 0x0059df10
static void Stats_AddItem(const unsigned short* pusLabel, const unsigned short* pusContents)
{
    LuaHelper::TableWriteVal("labelustr", pusLabel);
    LuaHelper::TableWriteVal("contentsustr", pusContents);
}

// FUNCTION: SWBF2 0x0059df30
static void Stats_AddItem(const unsigned short* pusLabel, int iValue)
{
    char acBuf[32];
    _snprintf(acBuf, 32, "%d", iValue);
    Stats_AddItem(pusLabel, acBuf);
}

// FUNCTION: SWBF2 0x0059df60
static void Stats_AddLabelNameTime(const unsigned short* pusLabel, const unsigned short* pusName, int iSeconds)
{
    unsigned short usText[64];
    memset(usText, 0, sizeof(usText));
    if (pusName)
    {
        ustrlcpy(usText, pusName, 64);
        for (unsigned short* p = ustrchr(usText, '\n'); p; p = ustrchr(usText, '\n'))
        {
            *p = ' ';
        }
        char acTime[64];
        _snprintf(acTime, 64, " %d:%02d", iSeconds / 60, iSeconds % 60);
        unsigned short usTime[32];
        ustrncnv(usTime, acTime, 32);
        ustrlcat(usText, usTime, 64);
    }
    Stats_AddItem(pusLabel, usText);
}

// FUNCTION: SWBF2 0x0059e040
static void Stats_AddPlayerAward(const unsigned short* pusLabel, const Character* pCharacter, int iValue)
{
    unsigned short usText[64];
    if (pCharacter && iValue)
    {
        ustrlcpy(usText, pCharacter->GetName(), 64);
        char acBuf[16];
        _snprintf(acBuf, 16, " (%d)", iValue);
        unsigned short usBuf[16];
        ustrncnv(usBuf, acBuf, 16);
        ustrlcat(usText, usBuf, 64);
        Stats_AddItem(pusLabel, usText);
    }
    else
    {
        ustrncnv(usText, "", 64);
        Stats_AddItem(pusLabel, usText);
    }
}

// FUNCTION: SWBF2 0x0059e0e0
static void Teamstats_AddItem(const unsigned short* pusLabel, int iVal1, int iVal2, int iVal3, int iVal4,
                              RedColor color)
{
    char acBuf[32];
    LuaHelper::TableWriteVal("labelustr", pusLabel);
    _snprintf(acBuf, 32, "%d", iVal1);
    LuaHelper::TableWriteVal("val1str", acBuf);
    _snprintf(acBuf, 32, "%d", iVal2);
    LuaHelper::TableWriteVal("val2str", acBuf);
    _snprintf(acBuf, 32, "%d", iVal3);
    LuaHelper::TableWriteVal("val3str", acBuf);
    _snprintf(acBuf, 32, "%d", iVal4);
    LuaHelper::TableWriteVal("val4str", acBuf);
    LuaHelper::TableWriteVal("ColorR", (float)color.r);
    LuaHelper::TableWriteVal("ColorG", (float)color.g);
    LuaHelper::TableWriteVal("ColorB", (float)color.b);
}

}

// FUNCTION: SWBF2 0x0059e1c0
int __cdecl TeamStatSortCallback(const void* pA, const void* pB)
{
    Character* pCharacterA = *(Character* const*)pA;
    Character* pCharacterB = *(Character* const*)pB;
    PlayerStats& statsA = pCharacterA->GetPlayerStats();
    PlayerStats& statsB = pCharacterB->GetPlayerStats();
    int aiSortOrder[5][5] =
    {
        { 1, 2, 3, 4, 5 },
        { 2, 1, 3, 4, 5 },
        { 3, 1, 2, 4, 5 },
        { 4, 1, 2, 3, 5 },
        { 5, 1, 2, 3, 4 },
    };

    for (int i = 0; i < 4; ++i)
    {
        int iDir = 1;
        int iKey = aiSortOrder[gTeamStatsSortMode][i];
        if (iKey == gTeamStatsSortMode + 1 && gTeamStatsSortInvert)
        {
            iDir = -1;
        }

        switch (iKey)
        {
        case 1:
            if (statsA.GetPoints(true) + statsA.GetPoints(false) < statsB.GetPoints(true) + statsB.GetPoints(false))
            {
                return iDir;
            }
            if (statsB.GetPoints(true) + statsB.GetPoints(false) < statsA.GetPoints(true) + statsA.GetPoints(false))
            {
                return -iDir;
            }
            break;
        case 2:
            if (statsA.GetKills(-1, 1) + statsA.GetKills(-1, 0) < statsB.GetKills(-1, 1) + statsB.GetKills(-1, 0))
            {
                return iDir;
            }
            if (statsB.GetKills(-1, 1) + statsB.GetKills(-1, 0) < statsA.GetKills(-1, 1) + statsA.GetKills(-1, 0))
            {
                return -iDir;
            }
            break;
        case 3:
            if (statsA.GetDeaths(-1, 1) + statsA.GetDeaths(-1, 0) > statsB.GetDeaths(-1, 1) + statsB.GetDeaths(-1, 0))
            {
                return iDir;
            }
            if (statsB.GetDeaths(-1, 1) + statsB.GetDeaths(-1, 0) > statsA.GetDeaths(-1, 1) + statsA.GetDeaths(-1, 0))
            {
                return -iDir;
            }
            break;
        case 4:
            if (statsA.GetPoints(false) < statsB.GetPoints(false))
            {
                return iDir;
            }
            if (statsB.GetPoints(false) < statsA.GetPoints(false))
            {
                return -iDir;
            }
            break;
        case 5:
        {
            int iCmp = -ustrcmp(pCharacterA->GetName(), pCharacterB->GetName());
            if (iCmp)
            {
                return iDir * iCmp;
            }
            break;
        }
        }
    }
    return 0;
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x0059e4a0
static void CareerMedals_AddItem(const unsigned short* pusLabel, int iVal1, int iVal2)
{
    char acBuf[32];
    LuaHelper::TableWriteVal("labelustr", pusLabel);
    _snprintf(acBuf, 32, "%d", iVal1);
    LuaHelper::TableWriteVal("val1str", acBuf);
    _snprintf(acBuf, 32, "%d", iVal2);
    LuaHelper::TableWriteVal("val2str", acBuf);
}

// FUNCTION: SWBF2 0x0059e500
int GetStatsPageJoystick()
{
    return gActiveViewport;
}

// FUNCTION: SWBF2 0x0059e570
int ScriptCB_SetTeamStatsSortMode(lua_State* L)
{
    int iMode = (int)luaL_checknumber(LuaHelper::mState, 1);
    if (iMode == gTeamStatsSortMode && !gTeamStatsSortInvert)
    {
        gTeamStatsSortInvert = true;
    }
    else
    {
        gTeamStatsSortInvert = false;
    }
    gTeamStatsSortMode = iMode;
    return 0;
}

// FUNCTION: SWBF2 0x0059e5c0
int ScriptCB_GetTeamstats(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) != 1 || !lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }
    float fSide = (float)luaL_checknumber(LuaHelper::mState, 1);
    if (fSide < 0.0f || fSide > 1.0f)
    {
        return 0;
    }

    const char* pcTable = "teamstats_listbox_contentsL";
    if (fSide != 0.0f)
    {
        pcTable = "teamstats_listbox_contentsR";
    }
    DoPeriodicLuaGC(true);

    if (NetGame::IsEnabled() && NetGame::IsClient() && !PlayerStats::mGotMPStats)
    {
        lua_pushnil(LuaHelper::mState);
        LuaHelper::SetGlobal(pcTable);
        DoPeriodicLuaGC(false);
        lua_newtable(LuaHelper::mState);
        lua_newtable(LuaHelper::mState);
        unsigned short usText[30];
        ustrncnv(usText, "fix MP stats", 30);
        Teamstats_AddItem(usText, 0, 0, 0, 0, RedColor::WHITE);
        lua_rawseti(LuaHelper::mState, -2, 1);
        LuaHelper::SetGlobal(pcTable);
        DoPeriodicLuaGC(false);
        return 0;
    }

    lua_gettop(LuaHelper::mState);
    lua_pushnil(LuaHelper::mState);
    LuaHelper::SetGlobal(pcTable);
    DoPeriodicLuaGC(false);
    lua_newtable(LuaHelper::mState);

    Team* pTeam = Team::sTeams[(int)fSide + 1];
    Character* apSorted[600];
    for (int i = 0; i < pTeam->mMemberCount; ++i)
    {
        apSorted[i] = pTeam->GetMemberPtr(i);
    }
    qsort(apSorted, pTeam->mMemberCount, sizeof(Character*), TeamStatSortCallback);

    int iLocalRank = -1;
    for (int i = 0; i < pTeam->mMemberCount; ++i)
    {
        PlayerStats& stats = apSorted[i]->GetPlayerStats();
        stats.mRankOrder = i;
        RedColor color = RedColor::MDGREY;
        if (apSorted[i]->mPlayerId >= 0)
        {
            int iLocal = NetGame::GetJoystickIndex(apSorted[i]->mPlayerId);
            if (iLocal < 0 || playerJoystickNumbers[iLocal * 11] < 0)
            {
                color = RedColor::YELLOW;
            }
            else
            {
                color = RedColor::WHITE;
                if (playerJoystickNumbers[iLocal * 11] == GetStatsPageJoystick())
                {
                    iLocalRank = i;
                }
            }
        }
        lua_newtable(LuaHelper::mState);
        Teamstats_AddItem(apSorted[i]->GetName(),
                          stats.GetPoints(true) + stats.GetPoints(false),
                          stats.GetKills((int)fSide, 1) + stats.GetKills((int)fSide, 0),
                          stats.GetDeaths((int)fSide, 1) + stats.GetDeaths((int)fSide, 0),
                          stats.GetFlagCaptures((int)fSide, 1) + stats.GetFlagCaptures((int)fSide, 0),
                          color);
        lua_rawseti(LuaHelper::mState, -2, i + 1);
    }

    lua_pushstring(LuaHelper::mState, pcTable);
    lua_insert(LuaHelper::mState, -2);
    lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);
    DoPeriodicLuaGC(false);
    lua_pushnumber(LuaHelper::mState, (float)iLocalRank);
    return 1;
}

// FUNCTION: SWBF2 0x0059e970
int ScriptCB_TeamStatsGetTeam1(lua_State* L)
{
    Team* pTeam = Team::sTeams[1];
    if (pTeam)
    {
        lua_pushnumber(LuaHelper::mState, (float)pTeam->GetSide());
    }
    else
    {
        lua_pushnumber(LuaHelper::mState, 0.0f);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0059e9c0
int ScriptCB_TeamStatsGetTeamColor(lua_State* L)
{
    float fTeam = (float)luaL_checknumber(LuaHelper::mState, 1);
    float fSlot = (float)luaL_checknumber(LuaHelper::mState, 2);
    Team* pTeam = Team::sTeams[(int)fTeam];
    if (pTeam)
    {
        RedColor color = pTeam->mColor[(int)fSlot];
        lua_pushnumber(LuaHelper::mState, (float)color.r);
        lua_pushnumber(LuaHelper::mState, (float)color.g);
        lua_pushnumber(LuaHelper::mState, (float)color.b);
    }
    else
    {
        lua_pushnumber(LuaHelper::mState, 0.0f);
        lua_pushnumber(LuaHelper::mState, 0.0f);
        lua_pushnumber(LuaHelper::mState, 0.0f);
    }
    return 3;
}

// FUNCTION: SWBF2 0x0059eb00
int ScriptCB_TeamStatsSetTitles(lua_State* L)
{
    float fSlot = (float)luaL_checknumber(LuaHelper::mState, 1);
    Team* pTeam1 = Team::sTeams[1];
    Team* pTeam2 = Team::sTeams[2];
    int iCount1, iCount2, i3, i4;
    bool b5, bShowCount;
    NetGame::GetScore(&iCount1, &iCount2, &i3, &i4, &b5, &bShowCount);

    unsigned short usTitle[100];
    unsigned short usBuf[100];
    char acBuf[100];

    ustrlcpy(usTitle, pTeam1->mName, 100);
    if (pTeam1->mCurReinforcements > 1000000 && !bShowCount)
    {
        _snprintf(acBuf, 100, " : --");
    }
    else
    {
        _snprintf(acBuf, 100, " : %d", iCount1);
    }
    ustrncnv(usBuf, acBuf, 100);
    ustrlcat(usTitle, usBuf, 100);
    ((RedTextElement*)RedInterfaceElement::Find(0x964e1d4bu))->SetText(usTitle, true);

    ustrlcpy(usTitle, pTeam2->mName, 100);
    if (pTeam2->mCurReinforcements > 1000000 && !bShowCount)
    {
        _snprintf(acBuf, 100, " : --");
    }
    else
    {
        _snprintf(acBuf, 100, " : %d", iCount2);
    }
    ustrncnv(usBuf, acBuf, 100);
    ustrlcat(usTitle, usBuf, 100);
    ((RedTextElement*)RedInterfaceElement::Find(0x0a25d764u))->SetText(usTitle, true);

    if (GetSideIcon(pTeam1->GetSide()) && GetSideIcon(pTeam2->GetSide()))
    {
        Red3DModelElementLite* pIcon = (Red3DModelElementLite*)RedInterfaceElement::Find(0xb0fcda7cu);
        pIcon->SetModel(GetSideIcon(pTeam1->GetSide()));
        pIcon->SetColor(pTeam1->mColor[(int)fSlot]);
        pIcon = (Red3DModelElementLite*)RedInterfaceElement::Find(0x14bdc17du);
        pIcon->SetModel(GetSideIcon(pTeam2->GetSide()));
        pIcon->SetColor(pTeam2->mColor[(int)fSlot]);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059ed40
int ScriptCB_PersonalStatsSetTitles(lua_State* L)
{
    int iSlot = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iTeam = (int)luaL_checknumber(LuaHelper::mState, 2);
    if (GetSideIcon(Team::sTeams[1]->GetSide()) && GetSideIcon(Team::sTeams[2]->GetSide()))
    {
        Team* pTeam = Team::sTeams[iTeam];
        Red3DModelElementLite* pIcon = (Red3DModelElementLite*)RedInterfaceElement::Find(0x262d6743u);
        pIcon->SetModel(GetSideIcon(pTeam->GetSide()));
        pIcon->SetColor(pTeam->mColor[iSlot]);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059edf0
int ScriptCB_TeamStatsValidatePos(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) != 2 || !lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }
    float fSide = (float)luaL_checknumber(LuaHelper::mState, 1);
    if (fSide < 0.0f || fSide > 1.0f || !lua_isnumber(LuaHelper::mState, 2))
    {
        return 0;
    }
    float fPos = (float)luaL_checknumber(LuaHelper::mState, 2);
    float fMax = (float)Team::sTeams[(int)fSide + 1]->mMemberCount;
    lua_pushnumber(LuaHelper::mState, (fPos > fMax) ? fMax : fPos);
    return 1;
}

// FUNCTION: SWBF2 0x0059eec0
int ScriptCB_GetPersonalStats(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) != 2 || !lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }
    float fTeam = (float)luaL_checknumber(LuaHelper::mState, 1);
    if (fTeam < 1.0f || fTeam > 2.0f || !lua_isnumber(LuaHelper::mState, 2))
    {
        return 0;
    }
    float fRank = (float)luaL_checknumber(LuaHelper::mState, 2);
    DoPeriodicLuaGC(true);

    if (NetGame::IsEnabled() && NetGame::IsClient() && !PlayerStats::mGotMPStats)
    {
        lua_pushnil(LuaHelper::mState);
        LuaHelper::SetGlobal("stats_listbox_contents");
        DoPeriodicLuaGC(false);
        lua_newtable(LuaHelper::mState);
        lua_newtable(LuaHelper::mState);
        unsigned short usText[30];
        ustrncnv(usText, "fix MP stats", 30);
        Stats_AddItem(usText, usText);
        lua_rawseti(LuaHelper::mState, -2, 1);
        LuaHelper::SetGlobal("stats_listbox_contents");
        DoPeriodicLuaGC(false);
        return 0;
    }

    lua_gettop(LuaHelper::mState);
    lua_pushnil(LuaHelper::mState);
    LuaHelper::SetGlobal("stats_listbox_contents");
    DoPeriodicLuaGC(false);
    lua_newtable(LuaHelper::mState);

    Team* pTeam = Team::sTeams[(int)fTeam];
    PlayerStats* pStats = 0;
    if (fRank >= 0.0f)
    {
        int iRank = (int)fRank - 1;
        for (int i = 0; i < pTeam->mMemberCount; ++i)
        {
            pStats = &pTeam->GetMemberPtr(i)->GetPlayerStats();
            if (pStats->mRankOrder == iRank)
            {
                break;
            }
        }
    }
    else
    {
        pStats = &NetGame::GetLocalPlayer(GetStatsPageJoystick())->GetPlayerStats();
    }

    for (int i = 0; i < 10; )
    {
        lua_newtable(LuaHelper::mState);
        switch (i)
        {
        case 0:
            Stats_AddItem(RedLocalizeDB::Find(0xc38cd05fu, false), pStats->GetPoints(true));
            break;
        case 1:
            Stats_AddItem(RedLocalizeDB::Find(0xa63ee31eu, false), pStats->GetPoints(false));
            break;
        case 2:
            Stats_AddItem(RedLocalizeDB::Find(0x26adb709u, false),
                          pStats->GetKills(-1, 1) + pStats->GetKills(-1, 0));
            break;
        case 3:
            Stats_AddItem(RedLocalizeDB::Find(0x39ef9b83u, false),
                          pStats->GetDeaths(-1, 1) + pStats->GetDeaths(-1, 0));
            break;
        case 4:
        {
            unsigned short usBlank[5];
            ustrncnv(usBlank, "", 5);
            Stats_AddItem(usBlank, usBlank);
            break;
        }
        case 5:
        {
            const EntityClass* pClass = pStats->GetFavoriteSoldierClass(-1);
            Stats_AddLabelNameTime(RedLocalizeDB::Find(0x766b2b87u, false), pClass ? pClass->mLabel : 0,
                                   (int)pStats->GetFavoriteSoldierTime(-1));
            break;
        }
        case 6:
        {
            const WeaponClass* pWeapon = pStats->GetFavoriteWeaponClass(-1);
            Stats_AddLabelNameTime(RedLocalizeDB::Find(0xe5a23889u, false), pWeapon ? pWeapon->mLabel : 0,
                                   (int)pStats->GetFavoriteWeaponTime(-1));
            break;
        }
        case 7:
            Stats_AddItem(RedLocalizeDB::Find(0x1821d4fbu, false),
                          pStats->GetCriticalShots(-1, 1) + pStats->GetCriticalShots(-1, 0));
            break;
        case 8:
        {
            int iTeam = (pStats->GetNemesisVal(-1, 0) > pStats->GetNemesisVal(-1, 1)) ? 0 : 1;
            Stats_AddPlayerAward(RedLocalizeDB::Find(0xa39860dcu, false), pStats->GetNemesis(-1, iTeam),
                                 pStats->GetNemesisVal(-1, iTeam));
            break;
        }
        case 9:
        {
            int iTeam = (pStats->GetBaitVal(-1, 0) > pStats->GetBaitVal(-1, 1)) ? 0 : 1;
            Stats_AddPlayerAward(RedLocalizeDB::Find(0x1e7c34f0u, false), pStats->GetBait(-1, iTeam),
                                 pStats->GetBaitVal(-1, iTeam));
            break;
        }
        }
        ++i;
        lua_rawseti(LuaHelper::mState, -2, i);
    }

    lua_pushstring(LuaHelper::mState, "stats_listbox_contents");
    lua_insert(LuaHelper::mState, -2);
    lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);
    DoPeriodicLuaGC(false);
    return 0;
}

// FUNCTION: SWBF2 0x0059f350
int ScriptCB_GetAwardStats(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) != 1 || !lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }
    int iSide = (int)luaL_checknumber(LuaHelper::mState, 1);
    const char* pcTable = iSide ? "teamstats_awardsListbox_contentsR" : "teamstats_awardsListbox_contentsL";

    if (NetGame::IsEnabled() && NetGame::IsClient() && !PlayerStats::mGotMPStats)
    {
        lua_pushnil(LuaHelper::mState);
        LuaHelper::SetGlobal(pcTable);
        DoPeriodicLuaGC(false);
        lua_newtable(LuaHelper::mState);
        lua_newtable(LuaHelper::mState);
        unsigned short usText[30];
        ustrncnv(usText, "fix MP stats", 30);
        Stats_AddItem(usText, usText);
        lua_rawseti(LuaHelper::mState, -2, 1);
        LuaHelper::SetGlobal(pcTable);
        DoPeriodicLuaGC(false);
        return 0;
    }

    float fTeam = (float)luaL_checknumber(LuaHelper::mState, 1);
    Team* pTeam = Team::sTeams[(int)fTeam];
    bool bIcons = GetSideIcon(Team::sTeams[1]->GetSide()) && GetSideIcon(Team::sTeams[2]->GetSide());

    lua_gettop(LuaHelper::mState);
    lua_pushnil(LuaHelper::mState);
    LuaHelper::SetGlobal(pcTable);
    DoPeriodicLuaGC(false);
    lua_newtable(LuaHelper::mState);

    int iFirst = (iSide != 0) * 4 + 2;
    int iEnd = (iSide != 0) * 4 + 6;
    for (int i = iFirst; i < iEnd; ++i)
    {
        lua_newtable(LuaHelper::mState);
        Character* pCharacter = 0;
        int iValue = 0;
        switch (i)
        {
        case 0:
        {
            unsigned short usBlank[5];
            ustrncnv(usBlank, "", 5);
            Stats_AddItem(usBlank, usBlank);
            break;
        }
        case 1:
        {
            unsigned short usBlank[5];
            ustrncnv(usBlank, "", 5);
            Stats_AddItem(usBlank, usBlank);
            break;
        }
        case 2:
            pCharacter = PlayerStats::GetAwardChr(AWARDSTAT_TANK_BUSTER);
            iValue = PlayerStats::GetAwardVal(AWARDSTAT_TANK_BUSTER);
            Stats_AddPlayerAward(RedLocalizeDB::Find(0x2f6a1ed9u, false), pCharacter, iValue);
            break;
        case 3:
            pCharacter = PlayerStats::GetAwardChr(AWARDSTAT_KILLING_SPREE);
            iValue = PlayerStats::GetAwardVal(AWARDSTAT_KILLING_SPREE);
            Stats_AddPlayerAward(RedLocalizeDB::Find(0xd0ad7735u, false), pCharacter, iValue);
            break;
        case 4:
        {
            pCharacter = PlayerStats::GetAwardChr(AWARDSTAT_CAMPER);
            iValue = PlayerStats::GetAwardVal(AWARDSTAT_CAMPER);
            unsigned short usText[64];
            if (pCharacter && iValue)
            {
                ustrlcpy(usText, pCharacter->GetName(), 64);
                char acBuf[32];
                _snprintf(acBuf, 32, " (%d:%02d)", iValue / 60, iValue % 60);
                unsigned short usTime[16];
                ustrncnv(usTime, acBuf, 16);
                ustrlcat(usText, usTime, 64);
            }
            else
            {
                ustrncnv(usText, "", 64);
            }
            Stats_AddItem(RedLocalizeDB::Find(0x4b56af4eu, false), usText);
            break;
        }
        case 5:
            pCharacter = PlayerStats::GetAwardChr(AWARDSTAT_SURVIVALIST);
            iValue = PlayerStats::GetAwardVal(AWARDSTAT_SURVIVALIST);
            Stats_AddPlayerAward(RedLocalizeDB::Find(0x1c47f24au, false), pCharacter, iValue);
            break;
        case 6:
            pCharacter = PlayerStats::GetAwardChr(AWARDSTAT_TRAITOR);
            iValue = PlayerStats::GetAwardVal(AWARDSTAT_TRAITOR);
            Stats_AddPlayerAward(RedLocalizeDB::Find(0x5c23c469u, false), pCharacter, iValue);
            break;
        case 7:
            pCharacter = PlayerStats::GetAwardChr(AWARDSTAT_PUBLIC_ENEMY);
            iValue = PlayerStats::GetAwardVal(AWARDSTAT_PUBLIC_ENEMY);
            Stats_AddPlayerAward(RedLocalizeDB::Find(0x5d21ecafu, false), pCharacter, iValue);
            break;
        case 8:
            pCharacter = PlayerStats::GetAwardChr(AWARDSTAT_MEAT);
            iValue = PlayerStats::GetAwardVal(AWARDSTAT_MEAT);
            Stats_AddPlayerAward(RedLocalizeDB::Find(0x1f913713u, false), pCharacter, iValue);
            break;
        case 9:
            pCharacter = PlayerStats::GetAwardChr(AWARDSTAT_DEADEYE);
            iValue = PlayerStats::GetAwardVal(AWARDSTAT_DEADEYE);
            Stats_AddPlayerAward(RedLocalizeDB::Find(0x7d3440e7u, false), pCharacter, iValue);
            break;
        }

        char acName[100];
        sprintf(acName, "ifs_awardstats.IconModels.%d.model", i - 1);
        Red3DModelElementLite* pIcon = (Red3DModelElementLite*)RedInterfaceElement::Find(acName);
        if (pIcon)
        {
            if (bIcons && pCharacter && iValue)
            {
                Team* pAwardTeam = pCharacter->mTeamPtr;
                pIcon->SetModel(GetSideIcon(pAwardTeam->GetSide()));

                RedColor color = pAwardTeam->mColor[(int)fTeam];
                color.a = 0;
                pIcon->SetColor(color);
                LuaHelper::CallGlobalProc("ifs_awardstats_seticonstartside",
                                          (pAwardTeam->GetSide() == pTeam->GetSide()) * 100.0f + (float)(i - 1));
            }
            else
            {
                pIcon->SetModel((const char*)0);
            }
        }
        lua_rawseti(LuaHelper::mState, -2, i - iFirst + 1);
    }

    lua_pushstring(LuaHelper::mState, pcTable);
    lua_insert(LuaHelper::mState, -2);
    lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);
    DoPeriodicLuaGC(false);
    return 0;
}

// FUNCTION: SWBF2 0x0059e510
int ScriptCB_ClientGotStats(lua_State* L)
{
    bool bGotStats = true;
    if (NetGame::IsEnabled() && !NetGame::IsHost())
    {
        bGotStats = PlayerStats::mGotMPStats;
    }
    lua_pushboolean(LuaHelper::mState, bGotStats);
    return 1;
}

// GLOBAL: SWBF2 0x007e8540
static const CallbackEntryT sStatsCallbacks[18] =
{
    { "ScriptCB_ClientGotStats", ScriptCB_ClientGotStats },
    { "ScriptCB_GetTeamstats", ScriptCB_GetTeamstats },
    { "ScriptCB_SetTeamStatsSortMode", ScriptCB_SetTeamStatsSortMode },
    { "ScriptCB_GetPersonalStats", ScriptCB_GetPersonalStats },
    { "ScriptCB_GetAwardStats", ScriptCB_GetAwardStats },
    { "ScriptCB_GetMedalStats", ScriptCB_GetMedalStats },
    { "ScriptCB_GetCareerPersonalStats", ScriptCB_GetCareerPersonalStats },
    { "ScriptCB_GetCareerRank", ScriptCB_GetCareerRank },
    { "ScriptCB_GetCareerMedals", ScriptCB_GetCareerMedals },
    { "ScriptCB_GetCareerMedalLevel", ScriptCB_GetCareerMedalLevel },
    { "ScriptCB_TeamStatsValidatePos", ScriptCB_TeamStatsValidatePos },
    { "ScriptCB_TeamStatsGetTeam1", ScriptCB_TeamStatsGetTeam1 },
    { "ScriptCB_TeamStatsGetTeamColor", ScriptCB_TeamStatsGetTeamColor },
    { "ScriptCB_TeamStatsSetTitles", ScriptCB_TeamStatsSetTitles },
    { "ScriptCB_PersonalStatsSetTitles", ScriptCB_PersonalStatsSetTitles },
    { "ScriptCB_SetPlayerStatsPoints", ScriptCB_SetPlayerStatsPoints },
    { "ScriptCB_GetRank", ScriptCB_GetRank },
    { "ScriptCB_GetPlayerIDAtRank", ScriptCB_GetPlayerIDAtRank },
};

// FUNCTION: SWBF2 0x005a06f0
void InitStats()
{
    for (int i = 0; i < 18; ++i)
    {
        LuaHelper::Register(sStatsCallbacks[i].pcName, sStatsCallbacks[i].pfnCallback, 0);
    }
}

// FUNCTION: SWBF2 0x005a0730
int ScriptCB_SetPlayerStatsPoints(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_type(LuaHelper::mState, 1) == LUA_TTABLE)
    {
        lua_gettop(LuaHelper::mState);
        int iCount = luaL_getn(LuaHelper::mState, 1);
        for (int i = 1; i <= iCount; ++i)
        {
            lua_rawgeti(LuaHelper::mState, 1, i);
            int iPoints = 0;
            LuaHelper::TableReadVal("point_gain", iPoints, false);
            PlayerStats::SetPointsTable(i - 1, iPoints);
            lua_settop(LuaHelper::mState, -2);
        }
        lua_gettop(LuaHelper::mState);
    }
    return 0;
}

}
