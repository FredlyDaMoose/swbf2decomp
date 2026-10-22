#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "MissionPlayList.h"
#include "GameLoop.h"
#include "LuaHelper.h"
#include "PlayerProfile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

using namespace MissionPlayList;

// GLOBAL: SWBF2 0x01f9d070
PlaylistEntry sPlaylist[256];
// GLOBAL: SWBF2 0x01e579c8
int sPlaylistCount;
// GLOBAL: SWBF2 0x01e579cc
int sPlaylistPosn;

// GLOBAL: SWBF2 0x01e579d0
int MissionPlayList::mFindIndex;
// GLOBAL: SWBF2 0x01e579c4
bool sbLoopForever;
// GLOBAL: SWBF2 0x01e579c5
bool sbRandomize;
// GLOBAL: SWBF2 0x01f9d06c
unsigned int sTeam1Hash;
// GLOBAL: SWBF2 0x01f9d068
unsigned int sTeam2Hash;
// GLOBAL: SWBF2 0x007e7f98
int sEras = -1;

namespace GameLoop { extern bool mbHistoricalRules; }

// FUNCTION: SWBF2 0x005a4410
int MissionPlayList::GetMaxSize()
{
    return sizeof(sPlaylist) / sizeof(sPlaylist[0]);
}

// FUNCTION: SWBF2 0x005a4420
void MissionPlayList::ClearEntries()
{
    memset(sPlaylist, 0, sizeof(sPlaylist));
    sPlaylistCount = 0;
    sPlaylistPosn = 0;
    sbRandomize = false;
    sEras = -1;
}

// FUNCTION: SWBF2 0x005a4460
void MissionPlayList::SetEntry(char const* pcLuaFile, unsigned char uSide, unsigned int uiTeam1Hash,
                               unsigned int uiTeam2Hash, int iTickets1, int iTickets2)
{
    strlcpy(sPlaylist[sPlaylistCount].mLuaFile, pcLuaFile, sizeof(sPlaylist[0].mLuaFile));
    PlaylistEntry& entry = sPlaylist[sPlaylistCount++];
    entry.mSide = uSide;
    entry.mTeam1Hash = uiTeam1Hash;
    entry.mTeam2Hash = uiTeam2Hash;
    entry.mTickets1 = iTickets1;
    entry.mTickets2 = iTickets2;
}

// FUNCTION: SWBF2 0x005a44c0
bool MissionPlayList::AddEntry(char const* pcLuaFile, unsigned char uSide, char const* pcTeam1,
                               char const* pcTeam2, int iTickets1, int iTickets2)
{
    if (sPlaylistCount >= 256)
    {
        return false;
    }

    unsigned int uiTeam1Hash = pcTeam1 ? (unsigned int)PblHash(pcTeam1) : 0;
    unsigned int uiTeam2Hash = pcTeam2 ? (unsigned int)PblHash(pcTeam2) : 0;
    SetEntry(pcLuaFile, uSide, uiTeam1Hash, uiTeam2Hash, iTickets1, iTickets2);

    int iEra = -1;
    unsigned int uLen = strlen(pcLuaFile) - 1;
    for (unsigned int i = 0; i < uLen; ++i)
    {
        if (isdigit(pcLuaFile[i]) && isalpha(pcLuaFile[i + 1]))
        {
            char c = pcLuaFile[i + 1] | 0x20;
            if (c == 'c')
            {
                iEra = 0;
                break;
            }
            if (c == 'g')
            {
                iEra = 1;
                break;
            }
        }
    }

    if (sEras == -1 || sEras == iEra)
    {
        sEras = iEra;
    }
    else
    {
        sEras = 2;
    }
    return true;
}

// FUNCTION: SWBF2 0x005a45b0
void MissionPlayList::SetLoopForever(bool bLoop)
{
    sbLoopForever = bLoop;
}

// FUNCTION: SWBF2 0x005a45c0
int MissionPlayList::GetCount()
{
    return sPlaylistCount;
}

// FUNCTION: SWBF2 0x005a45d0
void MissionPlayList::RandomizeOrder()
{
    sbRandomize = true;
    unsigned int uCount = sPlaylistCount;
    if (sPlaylistCount < 2)
    {
        return;
    }

    for (int i = sPlaylistCount * 7; i > 0; --i)
    {
        unsigned int uA = (PblRandom::GetGlobalInt() >> 3) % uCount;
        unsigned int uB = PblRandom::GetGlobalInt() % uCount;
        PlaylistEntry temp = sPlaylist[uA];
        sPlaylist[uA] = sPlaylist[uB];
        sPlaylist[uB] = temp;
    }
}

// FUNCTION: SWBF2 0x005a4760
bool MissionPlayList::PeekNextEntry(char const*& pcLuaFile, unsigned int& uiTeam1Hash,
                                    unsigned int& uiTeam2Hash)
{
    if (sPlaylistPosn >= sPlaylistCount)
    {
        if (!sbLoopForever)
        {
            return false;
        }
        sPlaylistPosn = 0;
        if (sbRandomize)
        {
            RandomizeOrder();
        }
    }
    pcLuaFile = sPlaylist[sPlaylistPosn].mLuaFile;
    uiTeam1Hash = sPlaylist[sPlaylistPosn].mTeam1Hash;
    uiTeam2Hash = sPlaylist[sPlaylistPosn].mTeam2Hash;
    return true;
}

// FUNCTION: SWBF2 0x005a47d0
bool MissionPlayList::SelectNextEntry()
{
    if (sPlaylistPosn >= sPlaylistCount)
    {
        if (!sbLoopForever)
        {
            return false;
        }
        sPlaylistPosn = 0;
        if (sbRandomize)
        {
            RandomizeOrder();
        }
    }
    GameLoop::SetNextMission(sPlaylist[sPlaylistPosn].mLuaFile);

    const PlaylistEntry& entry = sPlaylist[sPlaylistPosn];
    if (GameLoop::mbHistoricalRules)
    {
        playerTeamNumbers[11] = entry.mSide;
    }
    playerTeamNumbers[0] = entry.mSide;
    sTeam1Hash = entry.mTeam1Hash;
    sTeam2Hash = entry.mTeam2Hash;
    PushToLua();
    ++sPlaylistPosn;
    return true;
}

// FUNCTION: SWBF2 0x005a4860
void MissionPlayList::GetCurrentTeamNames(unsigned int& uiTeam1Hash, unsigned int& uiTeam2Hash)
{
    uiTeam1Hash = sTeam1Hash;
    uiTeam2Hash = sTeam2Hash;
}

// FUNCTION: SWBF2 0x005a4870
void MissionPlayList::SetCurrentTeamNames(unsigned int uiTeam1Hash, unsigned int uiTeam2Hash)
{
    if (sTeam1Hash != uiTeam1Hash || sTeam2Hash != uiTeam2Hash)
    {
        sTeam1Hash = uiTeam1Hash;
        sTeam2Hash = uiTeam2Hash;
        PushToLua();
    }
}

// FUNCTION: SWBF2 0x005a48a0
void MissionPlayList::SetClientEntry(char const* pcLuaFile, unsigned int uiTeam1Hash,
                                     unsigned int uiTeam2Hash)
{
    ClearEntries();
    SetEntry(pcLuaFile, 1, uiTeam1Hash, uiTeam2Hash, 0, 0);
}

// FUNCTION: SWBF2 0x005a48d0
static void SetOneName(unsigned int uiHash, const char* pcKey)
{
    const unsigned short* pusName = RedLocalizeDB::Find(uiHash, true);
    if (pusName)
    {
        LuaHelper::TableWriteVal(pcKey, pusName);
        return;
    }
    char acHex[16];
    unsigned short ausHex[16];
    _snprintf(acHex, sizeof(acHex), "%08X", uiHash);
    PblUnicode::ustrncnv(ausHex, acHex, 16);
    LuaHelper::TableWriteVal(pcKey, ausHex);
}

// FUNCTION: SWBF2 0x005a4930
void MissionPlayList::PushToLua()
{
    lua_State* L = LuaHelper::mState;
    int iTop = lua_gettop(L);
    lua_getglobal(L, "ifs_missionselect");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "ifs_missionselect");
        lua_getglobal(L, "ifs_missionselect");
    }
    SetOneName(sTeam1Hash, "Team1Name");
    SetOneName(sTeam2Hash, "Team2Name");
    lua_pop(L, 1);
    (void)iTop;
    lua_gettop(L);
}

// FUNCTION: SWBF2 0x005a4a90
int MissionPlayList::GetEntryCount()
{
    return sPlaylistCount;
}

// FUNCTION: SWBF2 0x005a4aa0
int MissionPlayList::GetCurrentEntry()
{
    int iEntry = sPlaylistPosn - 1;
    if (iEntry < 0)
    {
        iEntry = 0;
    }
    return iEntry;
}

// FUNCTION: SWBF2 0x005a4ab0
void MissionPlayList::GetEntry(int iEntry, char const*& pcLuaFile, int& iTickets1, int& iTickets2)
{
    PlaylistEntry& entry = sPlaylist[iEntry];
    pcLuaFile = entry.mLuaFile;
    iTickets1 = entry.mTickets1;
    iTickets2 = entry.mTickets2;
}

// FUNCTION: SWBF2 0x005a4ae0
void MissionPlayList::AddEntry(char const* pcLuaFile, int iTickets1, int iTickets2)
{
    int iCount = sPlaylistCount;
    if (iCount >= 256)
    {
        return;
    }
    for (int i = iCount - 1; i >= sPlaylistPosn; --i)
    {
        sPlaylist[i + 1] = sPlaylist[i];
    }
    sPlaylistCount = sPlaylistPosn;
    AddEntry(pcLuaFile, 1, 0, 0, iTickets1, iTickets2);
    sPlaylistCount = iCount + 1;
}

// FUNCTION: SWBF2 0x005a4ba0
bool MissionPlayList::DropEntry(char const* pcLuaFile)
{
    if (sPlaylistCount <= 1)
    {
        return false;
    }

    int iDrop = GetCurrentEntry();
    if (pcLuaFile)
    {
        int iEntry = iDrop;
        for (int i = 0; i < sPlaylistCount; ++i, ++iEntry)
        {
            if (iEntry >= sPlaylistCount)
            {
                iEntry = 0;
            }
            char const* pcName;
            int iTickets1, iTickets2;
            GetEntry(iEntry, pcName, iTickets1, iTickets2);
            if (!_stricmp(pcLuaFile, pcName))
            {
                iDrop = iEntry;
                break;
            }
        }
    }

    int iCount = sPlaylistCount - 1;
    if (iDrop < iCount)
    {
        memcpy(&sPlaylist[iDrop], &sPlaylist[iDrop + 1], (iCount - iDrop) * sizeof(PlaylistEntry));
    }
    sPlaylistCount = iCount;
    if (sPlaylistPosn > iCount)
    {
        --sPlaylistPosn;
    }
    return true;
}

// FUNCTION: SWBF2 0x005a4c60
bool MissionPlayList::NextEntry(char const* pcLuaFile)
{
    if (!pcLuaFile)
    {
        return false;
    }

    int iFound = -1;
    int iEntry = GetCurrentEntry();
    for (int i = 0; i < sPlaylistCount; ++i, ++iEntry)
    {
        if (iEntry >= sPlaylistCount)
        {
            iEntry = 0;
        }
        char const* pcName;
        int iTickets1, iTickets2;
        GetEntry(iEntry, pcName, iTickets1, iTickets2);
        if (!_stricmp(pcLuaFile, pcName))
        {
            iFound = iEntry;
            break;
        }
    }
    if (iFound == -1)
    {
        return false;
    }
    sPlaylistPosn = iFound;
    return true;
}

// FUNCTION: SWBF2 0x005a4ce0
void MissionPlayListStepBack()
{
    if (sPlaylistPosn > 0)
    {
        --sPlaylistPosn;
    }
}

// FUNCTION: SWBF2 0x005a4a20
unsigned int MissionPlayList::GetFirstMission()
{
    mFindIndex = 0;
    if (mFindIndex > sPlaylistCount)
    {
        return 0;
    }
    return PblHash(sPlaylist[mFindIndex].mLuaFile);
}

// FUNCTION: SWBF2 0x005a4a50
unsigned int MissionPlayList::GetNextMission()
{
    ++mFindIndex;
    if (mFindIndex > sPlaylistCount)
    {
        return 0;
    }
    return PblHash(sPlaylist[mFindIndex].mLuaFile);
}
