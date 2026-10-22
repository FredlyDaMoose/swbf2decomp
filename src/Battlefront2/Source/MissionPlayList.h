#pragma once

namespace MissionPlayList
{

struct PlaylistEntry
{
    char mLuaFile[64];
    unsigned char mSide;
    unsigned int mTeam1Hash;
    unsigned int mTeam2Hash;
    int mTickets1;
    int mTickets2;
};

void ClearEntries();
void SetEntry(char const* pcLuaFile, unsigned char uSide, unsigned int uiTeam1Hash,
              unsigned int uiTeam2Hash, int iTickets1, int iTickets2);
bool AddEntry(char const* pcLuaFile, unsigned char uSide, char const* pcTeam1,
              char const* pcTeam2, int iTickets1, int iTickets2);
void SetLoopForever(bool bLoop);
int GetCount();
void RandomizeOrder();

int GetMaxSize();
bool PeekNextEntry(char const*& pcLuaFile, unsigned int& uiTeam1Hash,
                   unsigned int& uiTeam2Hash);
bool SelectNextEntry();
void GetCurrentTeamNames(unsigned int& uiTeam1Hash, unsigned int& uiTeam2Hash);
void SetCurrentTeamNames(unsigned int uiTeam1Hash, unsigned int uiTeam2Hash);
void SetClientEntry(char const* pcLuaFile, unsigned int uiTeam1Hash,
                    unsigned int uiTeam2Hash);
void PushToLua();

int GetEntryCount();
int GetCurrentEntry();
void GetEntry(int iEntry, char const*& pcLuaFile, int& iTickets1, int& iTickets2);
void AddEntry(char const* pcLuaFile, int iTickets1, int iTickets2);
bool DropEntry(char const* pcLuaFile);
bool NextEntry(char const* pcLuaFile);
unsigned int GetFirstMission();
unsigned int GetNextMission();
extern int mFindIndex;

}
