#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <windows.h>

#include <stdlib.h>

#include "Character.h"
#include "CommandLine.h"
#include "Controllable.h"
#include "GameLoop.h"
#include "LuaCallbacks_Options.h"
#include "MissionPlayList.h"
#include "pcNetSocket.h"
#include "NetGame.h"
#include "NetComm.h"
#include "NetPacket.h"
#include "NetSetup.h"
#include "NetAdmin.h"
#include "SpawnManager.h"
#include "Team.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblJournal.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

class NetAdmin
{
public:
    static void Create(bool bMission);
    static void RemovePlayer(int iPlayer);
    static void AddPlayer(int iPlayer);
    static void ReceiveConsole();
    static void __fastcall ChatMessage(int iPlayer, const unsigned short* pusMessage);

    static bool ParseLineRemote(int iPlayer, unsigned short* pusLine, bool bConsole, bool bPrefixed);
    static bool ParseLineLocal(unsigned short* pusLine, bool bConsole);
};

void __fastcall MD5Transform(unsigned int* pState, const unsigned char* pBlock);
void __fastcall MD5EncodeWords(void* pDest, const void* pSource, unsigned int uLength);
void __fastcall MD5DecodeWords(unsigned int* pWords, const unsigned char* pBlock);
void* __fastcall MD5CopyBytes(void* pDest, const void* pSource, unsigned int uLength);
void* __fastcall MD5ClearBytes(void* pDest, unsigned int uLength);
void __fastcall MD5Update(unsigned int* pState, const unsigned char* pData, unsigned int uLength);
void __fastcall MD5Print(const unsigned char* pDigest, char* pcHex);
extern char adminPlayerIdText[32];

// GLOBAL: SWBF2 0x007e88d0
unsigned char md5Padding[64] = { 0x80 };

// FUNCTION: SWBF2 0x005a2730
void __fastcall MD5Init(unsigned int* pState)
{
    pState[4] = pState[5] = 0;
    pState[0] = 0x67452301;
    pState[1] = 0xefcdab89;
    pState[2] = 0x98badcfe;
    pState[3] = 0x10325476;
}

// FUNCTION: SWBF2 0x005a2760
void __fastcall MD5Update(unsigned int* pState, const unsigned char* pData, unsigned int uLength)
{
    unsigned int uIndex = (pState[4] >> 3) & 63;
    unsigned int uBits = uLength << 3;
    pState[4] += uBits;
    if (pState[4] < uBits)
    {
        ++pState[5];
    }
    pState[5] += uLength >> 29;
    unsigned int uPart = 64 - uIndex;
    unsigned int i = 0;
    if (uLength >= uPart)
    {
        MD5CopyBytes((unsigned char*)(pState + 6) + uIndex, pData, uPart);
        MD5Transform(pState, (unsigned char*)(pState + 6));
        for (i = uPart; i + 63 < uLength; i += 64)
        {
            MD5Transform(pState, pData + i);
        }
        uIndex = 0;
    }
    MD5CopyBytes((unsigned char*)(pState + 6) + uIndex, pData + i, uLength - i);
}

// FUNCTION: SWBF2 0x005a2810
void __fastcall MD5Final(unsigned char* pDigest, unsigned int* pState)
{
    unsigned char aucBits[8];
    MD5EncodeWords(aucBits, pState + 4, 8);
    unsigned int uIndex = (pState[4] >> 3) & 63;
    MD5Update(pState, md5Padding, (uIndex < 56 ? 56 : 120) - uIndex);
    MD5Update(pState, aucBits, 8);
    MD5EncodeWords(pDigest, pState, 16);
    MD5ClearBytes(pState, 88);
}

#define MD5_F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | ~(z)))
#define MD5_ROL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define MD5_STEP(f, a, b, c, d, x, s, ac) { (a) += f((b), (c), (d)) + (x) + (unsigned int)(ac); (a) = MD5_ROL((a), (s)); (a) += (b); }

// FUNCTION: SWBF2 0x005a2880
void __fastcall MD5Transform(unsigned int* pState, const unsigned char* pBlock)
{
    unsigned int a = pState[0], b = pState[1], c = pState[2], d = pState[3];
    unsigned int x[16];
    MD5DecodeWords(x, pBlock);

    MD5_STEP(MD5_F, a, b, c, d, x[ 0],  7, 0xd76aa478); MD5_STEP(MD5_F, d, a, b, c, x[ 1], 12, 0xe8c7b756);
    MD5_STEP(MD5_F, c, d, a, b, x[ 2], 17, 0x242070db); MD5_STEP(MD5_F, b, c, d, a, x[ 3], 22, 0xc1bdceee);
    MD5_STEP(MD5_F, a, b, c, d, x[ 4],  7, 0xf57c0faf); MD5_STEP(MD5_F, d, a, b, c, x[ 5], 12, 0x4787c62a);
    MD5_STEP(MD5_F, c, d, a, b, x[ 6], 17, 0xa8304613); MD5_STEP(MD5_F, b, c, d, a, x[ 7], 22, 0xfd469501);
    MD5_STEP(MD5_F, a, b, c, d, x[ 8],  7, 0x698098d8); MD5_STEP(MD5_F, d, a, b, c, x[ 9], 12, 0x8b44f7af);
    MD5_STEP(MD5_F, c, d, a, b, x[10], 17, 0xffff5bb1); MD5_STEP(MD5_F, b, c, d, a, x[11], 22, 0x895cd7be);
    MD5_STEP(MD5_F, a, b, c, d, x[12],  7, 0x6b901122); MD5_STEP(MD5_F, d, a, b, c, x[13], 12, 0xfd987193);
    MD5_STEP(MD5_F, c, d, a, b, x[14], 17, 0xa679438e); MD5_STEP(MD5_F, b, c, d, a, x[15], 22, 0x49b40821);

    MD5_STEP(MD5_G, a, b, c, d, x[ 1],  5, 0xf61e2562); MD5_STEP(MD5_G, d, a, b, c, x[ 6],  9, 0xc040b340);
    MD5_STEP(MD5_G, c, d, a, b, x[11], 14, 0x265e5a51); MD5_STEP(MD5_G, b, c, d, a, x[ 0], 20, 0xe9b6c7aa);
    MD5_STEP(MD5_G, a, b, c, d, x[ 5],  5, 0xd62f105d); MD5_STEP(MD5_G, d, a, b, c, x[10],  9, 0x02441453);
    MD5_STEP(MD5_G, c, d, a, b, x[15], 14, 0xd8a1e681); MD5_STEP(MD5_G, b, c, d, a, x[ 4], 20, 0xe7d3fbc8);
    MD5_STEP(MD5_G, a, b, c, d, x[ 9],  5, 0x21e1cde6); MD5_STEP(MD5_G, d, a, b, c, x[14],  9, 0xc33707d6);
    MD5_STEP(MD5_G, c, d, a, b, x[ 3], 14, 0xf4d50d87); MD5_STEP(MD5_G, b, c, d, a, x[ 8], 20, 0x455a14ed);
    MD5_STEP(MD5_G, a, b, c, d, x[13],  5, 0xa9e3e905); MD5_STEP(MD5_G, d, a, b, c, x[ 2],  9, 0xfcefa3f8);
    MD5_STEP(MD5_G, c, d, a, b, x[ 7], 14, 0x676f02d9); MD5_STEP(MD5_G, b, c, d, a, x[12], 20, 0x8d2a4c8a);

    MD5_STEP(MD5_H, a, b, c, d, x[ 5],  4, 0xfffa3942); MD5_STEP(MD5_H, d, a, b, c, x[ 8], 11, 0x8771f681);
    MD5_STEP(MD5_H, c, d, a, b, x[11], 16, 0x6d9d6122); MD5_STEP(MD5_H, b, c, d, a, x[14], 23, 0xfde5380c);
    MD5_STEP(MD5_H, a, b, c, d, x[ 1],  4, 0xa4beea44); MD5_STEP(MD5_H, d, a, b, c, x[ 4], 11, 0x4bdecfa9);
    MD5_STEP(MD5_H, c, d, a, b, x[ 7], 16, 0xf6bb4b60); MD5_STEP(MD5_H, b, c, d, a, x[10], 23, 0xbebfbc70);
    MD5_STEP(MD5_H, a, b, c, d, x[13],  4, 0x289b7ec6); MD5_STEP(MD5_H, d, a, b, c, x[ 0], 11, 0xeaa127fa);
    MD5_STEP(MD5_H, c, d, a, b, x[ 3], 16, 0xd4ef3085); MD5_STEP(MD5_H, b, c, d, a, x[ 6], 23, 0x04881d05);
    MD5_STEP(MD5_H, a, b, c, d, x[ 9],  4, 0xd9d4d039); MD5_STEP(MD5_H, d, a, b, c, x[12], 11, 0xe6db99e5);
    MD5_STEP(MD5_H, c, d, a, b, x[15], 16, 0x1fa27cf8); MD5_STEP(MD5_H, b, c, d, a, x[ 2], 23, 0xc4ac5665);

    MD5_STEP(MD5_I, a, b, c, d, x[ 0],  6, 0xf4292244); MD5_STEP(MD5_I, d, a, b, c, x[ 7], 10, 0x432aff97);
    MD5_STEP(MD5_I, c, d, a, b, x[14], 15, 0xab9423a7); MD5_STEP(MD5_I, b, c, d, a, x[ 5], 21, 0xfc93a039);
    MD5_STEP(MD5_I, a, b, c, d, x[12],  6, 0x655b59c3); MD5_STEP(MD5_I, d, a, b, c, x[ 3], 10, 0x8f0ccc92);
    MD5_STEP(MD5_I, c, d, a, b, x[10], 15, 0xffeff47d); MD5_STEP(MD5_I, b, c, d, a, x[ 1], 21, 0x85845dd1);
    MD5_STEP(MD5_I, a, b, c, d, x[ 8],  6, 0x6fa87e4f); MD5_STEP(MD5_I, d, a, b, c, x[15], 10, 0xfe2ce6e0);
    MD5_STEP(MD5_I, c, d, a, b, x[ 6], 15, 0xa3014314); MD5_STEP(MD5_I, b, c, d, a, x[13], 21, 0x4e0811a1);
    MD5_STEP(MD5_I, a, b, c, d, x[ 4],  6, 0xf7537e82); MD5_STEP(MD5_I, d, a, b, c, x[11], 10, 0xbd3af235);
    MD5_STEP(MD5_I, c, d, a, b, x[ 2], 15, 0x2ad7d2bb); MD5_STEP(MD5_I, b, c, d, a, x[ 9], 21, 0xeb86d391);

    pState[0] += a;
    pState[1] += b;
    pState[2] += c;
    pState[3] += d;
}

// FUNCTION: SWBF2 0x005a2f40
void __fastcall MD5EncodeWords(void* pDest, const void* pSource, unsigned int uLength)
{
    unsigned char* pOut = (unsigned char*)pDest;
    const unsigned char* pIn = (const unsigned char*)pSource;
    for (unsigned int i = 0; i < uLength; i += 4)
    {
        pOut[i] = pIn[i];
        pOut[i + 1] = pIn[i + 1];
        pOut[i + 2] = pIn[i + 2];
        pOut[i + 3] = pIn[i + 3];
    }
}

// FUNCTION: SWBF2 0x005a2f90
void __fastcall MD5DecodeWords(unsigned int* pWords, const unsigned char* pBlock)
{
    for (int i = 0; i < 16; ++i)
    {
        pWords[i] = (unsigned int)pBlock[i * 4] |
                    ((unsigned int)pBlock[i * 4 + 1] << 8) |
                    ((unsigned int)pBlock[i * 4 + 2] << 16) |
                    ((unsigned int)pBlock[i * 4 + 3] << 24);
    }
}

// FUNCTION: SWBF2 0x005a2fd0
void* __fastcall MD5CopyBytes(void* pDest, const void* pSource, unsigned int uLength)
{
    return memcpy(pDest, pSource, uLength);
}

// FUNCTION: SWBF2 0x005a2ff0
void* __fastcall MD5ClearBytes(void* pDest, unsigned int uLength)
{
    return memset(pDest, 0, uLength);
}

// FUNCTION: SWBF2 0x005a3010
void __fastcall MD5Print(const unsigned char* pDigest, char* pcHex)
{
    for (int i = 0; i < 16; ++i)
    {
        sprintf(pcHex, "%02x", pDigest[i]);
        pcHex += 2;
    }
}

// FUNCTION: SWBF2 0x005a3050
void MD5Digest(const char* pcText, unsigned int uLength, char* pcHex)
{
    unsigned int auiState[22];
    unsigned char aucDigest[20];
    MD5Init(auiState);
    MD5Update(auiState, (const unsigned char*)pcText, uLength);
    MD5Final(aucDigest, auiState);
    MD5Print(aucDigest, pcHex);
}

// GLOBAL: SWBF2 0x01ecb270
Admin adminDests[3];
// GLOBAL: SWBF2 0x01e57a04
int adminKeyCount;
// GLOBAL: SWBF2 0x01e579fc
float adminKeyNextTime;
// GLOBAL: SWBF2 0x007e8004
char adminKey[2] = { '4', 'z' };

// FUNCTION: SWBF2 0x005b1530
void __fastcall HeartBeat(const unsigned __int64* pPeer)
{
    const float fNow = NetComm::GetTime();
    if (adminKeyNextTime > fNow + 61.0f || fNow > adminKeyNextTime)
    {
        ++adminKeyCount;
        unsigned int seed = netRandom._uiSeed * 1664525u + 1013904223u;
        adminKeyNextTime = fNow + 60.0f;
        netRandom._uiSeed = seed * 1664525u + 1013904223u;
        adminKey[adminKeyCount & 1] = (char)(seed >> 16);
    }
    for (int i = 0; i < 3; ++i)
    {
        if (*(const unsigned __int64*)&adminDests[i].addr == *pPeer)
        {
            adminDests[i].lastRecvTime = fNow;
            return;
        }
    }
    float fOldest = 1000000.0f;
    int iSlot = 0;
    for (int i = 0; i < 3; ++i)
    {
        if (adminDests[i].lastRecvTime > fNow)
        {
            iSlot = i;
            break;
        }
        if (fOldest > adminDests[i].lastRecvTime)
        {
            fOldest = adminDests[i].lastRecvTime;
            iSlot = i;
        }
    }
    adminDests[iSlot].addr = (unsigned int)*pPeer;
    adminDests[iSlot].port = (unsigned int)(*pPeer >> 32);
    adminDests[iSlot].lastRecvTime = fNow;
    adminDests[iSlot].admin = false;
}

// FUNCTION: SWBF2 0x005b1630
int __fastcall FindAdminSource(int iDest)
{
    const unsigned __int64 peer = *NetCommP::GetAddr(iDest);
    for (int i = 0; i < 3; ++i)
    {
        if (*(const unsigned __int64*)&adminDests[i].addr == peer)
        {
            return i;
        }
    }
    return -1;
}

extern int joinSlots[0x1c0];
extern bool isLAN;

// FUNCTION: SWBF2 0x005b3800
int __fastcall NetCommP::FindJoinSource(const int* pAddress)
{
    const float fNow = NetComm::GetTime();
    for (int i = 0; i < 64; ++i)
    {
        int* pEntry = joinSlots + i * 7;
        bool bSame;
        if (isLAN)
        {
            int ip1, port1, ip2, port2;
            NetSocket::GetAddr(pAddress, ip1, port1);
            NetSocket::GetAddr(pEntry + 1, ip2, port2);
            bSame = ip1 == ip2 && port1 == port2;
        }
        else
        {
            bSame = pAddress[0] == pEntry[1] && pAddress[1] == pEntry[2];
        }
        if (bSame)
        {
            *(float*)pEntry = fNow;
            return i + 69;
        }
    }
    for (int i = 0; i < 64; ++i)
    {
        int* pEntry = joinSlots + i * 7;
        const float fLast = *(float*)pEntry;
        if (fNow - fLast > 1.0f || fLast == 0.0f)
        {
            pEntry[1] = pAddress[0];
            pEntry[2] = pAddress[1];
            memcpy(pEntry + 3, pAddress + 2, 16);
            *(float*)pEntry = fNow;
            return i + 69;
        }
    }
    return -1;
}

// GLOBAL: SWBF2 0x007e7fec
unsigned int lastTurn = 1000;
// GLOBAL: SWBF2 0x01e57a03
bool lastGameOver;
// GLOBAL: SWBF2 0x01e57a08
int lastScore;
// GLOBAL: SWBF2 0x01e579f8
int detectState;
// GLOBAL: SWBF2 0x01fa24c8
float nextDetect;
// GLOBAL: SWBF2 0x01fa24cc
unsigned int lastOutBytes;
// GLOBAL: SWBF2 0x01fa2498
unsigned int lastLostBytes;

extern int netLostBytes;
extern bool netBandwidthDetecting;
extern bool detectBandwidth;
extern int netCurMaxPlayers;
extern char remoteResponse[0x1800];
extern bool remoteLoggedIn;
extern int adminBandwidth;
void WriteString(NetPktGroup* pGroup, const char* pText, unsigned int uCapacity);

// FUNCTION: SWBF2 0x005b1920
void __fastcall SendAdminMessage(char* pcMessage)
{
    const float fNow = NetComm::GetTime();
    for (int i = 0; i < 3; ++i)
    {
        Admin& admin = adminDests[i];
        if (admin.lastRecvTime > fNow)
        {
            admin.addr = 0;
            admin.port = 0;
            admin.lastRecvTime = 0.0f;
        }
        else if ((admin.addr || admin.port) && admin.lastRecvTime >= fNow - 60.0f && admin.admin)
        {
            unsigned int address[2] = { admin.addr, admin.port };
            int iDest = NetCommP::FindJoinSource((const int*)address);
            NetPktGroup group;
            group.BeginWrite();
            group.WriteBits((char)adminKey[adminKeyCount & 1], 8);
            group.WriteBits(1, 8);
            WriteString(&group, pcMessage, 128);
            group.WriteBits(0, 8);
            group.EndWrite();
            NetComm::Send(iDest, PKT_CONSOLE_ACK, &group);
        }
    }
}

// FUNCTION: SWBF2 0x005b1a40
void __fastcall NetAdmin::ChatMessage(int iPlayer, const unsigned short* pusMessage)
{
    const char* pcName = iPlayer == 64 ? "ADMIN" : NetGame::GetPlayerName(iPlayer);
    char acBuffer[128];
    _snprintf(acBuffer, 127, "%s: ", pcName);
    acBuffer[127] = 0;
    unsigned int uLength = (unsigned int)strlen(acBuffer);
    PblUnicode::ustrncnv(acBuffer + uLength, pusMessage, 128 - uLength);
    acBuffer[127] = 0;
    SendAdminMessage(acBuffer);
}

// FUNCTION: SWBF2 0x005b1ac0
static void SendStateTransitions()
{
    unsigned int uTurn = NetGame::GetTurnNumber();
    if (uTurn < lastTurn)
    {
        char acMessage[64];
        _snprintf(acMessage, 63, "game started: %s", GameLoop::mMissionScript);
        acMessage[63] = 0;
        SendAdminMessage(acMessage);
        lastScore = 10000;
        lastGameOver = false;
    }
    lastTurn = NetGame::GetTurnNumber();
    if (!lastGameOver && GameLoop::IsGameOver())
    {
        SendAdminMessage((char*)"game over");
        lastGameOver = true;
    }

    unsigned int uiTeam1, uiTeam2;
    MissionPlayList::GetCurrentTeamNames(uiTeam1, uiTeam2);
    const unsigned short* pusTeam1 = RedLocalizeDB::Find(uiTeam1, true);
    const unsigned short* pusTeam2 = RedLocalizeDB::Find(uiTeam2, true);
    char acTeam1[32] = "Unknown";
    char acTeam2[32] = "Unknown";
    if (pusTeam1 && pusTeam2)
    {
        PblUnicode::ustrncnv(acTeam1, pusTeam1, 32);
        PblUnicode::ustrncnv(acTeam2, pusTeam2, 32);
    }
    int iScore1 = Team::sTeams[1]->mCurReinforcements;
    int iScore2 = Team::sTeams[2]->mCurReinforcements;
    if (iScore1 + iScore2 < lastScore - 25)
    {
        char acMessage[64];
        _snprintf(acMessage, 63, "score: %s(%d) to %s(%d)", acTeam1, iScore1, acTeam2,
                  Team::sTeams[2]->mCurReinforcements);
        acMessage[63] = 0;
        SendAdminMessage(acMessage);
        lastScore = iScore1 + iScore2;
    }
}

// FUNCTION: SWBF2 0x005b1c10
static void DetectBandwidth()
{
    const float fNow = NetComm::GetTime();
    float fNext = nextDetect;
    if (fNext - 60.0f > fNow)
    {
        fNext = nextDetect = 0.0f;
    }

    const unsigned int uLostBytes = netLostBytes;
    int iState, iBandwidth;
    if (fNow > fNext)
    {
        nextDetect = fNow + 1.0f;
        unsigned int iIn;
        unsigned int iOut;
        NetComm::GetBytes(iIn, iOut);
        unsigned int uPrevOut = lastOutBytes;
        lastOutBytes = iOut;
        if ((unsigned int)iOut < uPrevOut)
        {
            uPrevOut = 0;
        }
        int iBytes = iOut - uPrevOut;
        iState = detectState;
        if (iState == 2)
        {
            iState = detectState = 3;
            iBytes = (iBytes + (int)lastLostBytes - (int)uLostBytes) / 16;
        }
        iBandwidth = adminBandwidth = 8 * iBytes / 1024;
    }
    else
    {
        iState = detectState;
        iBandwidth = adminBandwidth;
    }

    if (detectBandwidth)
    {
        iState = detectState = 1;
        detectBandwidth = false;
    }
    if (iState == 3)
    {
        netBandwidthDetecting = false;
        if (iBandwidth < 32)
        {
            iBandwidth = 32;
        }
        adminBandwidth = iBandwidth;
        int iUp, iDown;
        NetComm::GetBandwidth(64, iUp, iDown);
        NetComm::SetBandwidth(64, iBandwidth << 10, iDown);
        char acMessage[64];
        _snprintf(acMessage, 63, "detected and set bandwidth %d kbps", iBandwidth);
        acMessage[63] = 0;
        SendAdminMessage(acMessage);
        detectState = 0;
    }
    else if (iState == 1)
    {
        int iNeeded = 0;
        for (int i = 0; i < netCurMaxPlayers; ++i)
        {
            if (NetGame::IsPlaying(i, false))
            {
                iNeeded += 81920;
            }
        }
        int iUp, iDown;
        NetComm::GetBandwidth(64, iUp, iDown);
        if (iNeeded > iUp)
        {
            NetComm::SetBandwidth(64, iNeeded, iDown);
        }
        netBandwidthDetecting = true;
        unsigned int iIn, iOut;
        NetComm::GetBytes(iIn, iOut);

        lastOutBytes = iOut;
        lastLostBytes = uLostBytes;
        detectState = 2;
        nextDetect = fNow + 16.0f;
    }
}

void LoadServerBans();
void __fastcall LoadServerConfigSection(char const* pcSection);

extern char aSettings[512];
extern FILE* Stream;
extern bool adminStatusRequested;

// GLOBAL: SWBF2 0x01fa24d0
bool admins[64];

extern char Destination[16];

extern int netGameCount;

// FUNCTION: SWBF2 0x005aed50
void NetAdmin::Create(bool  )
{
    if (netGameCount != 1)
    {
        return;
    }

    JournalString(Destination, sizeof(Destination));
    memset(admins, 0, sizeof(admins));
    if (NetGame::GetLocalIndex(0) >= 0)
    {
        admins[0] = true;
    }
}

enum AdminCommand
{
    AC_WAITLATE = 0,
    AC_DROPMAP = 1,
    AC_SPLITUPDATE = 2,
    AC_PLAYERS = 3,
    AC_MAPS = 4,
    AC_BOOT = 6,
    AC_ADDMAP = 8,
    AC_NEXTMAP = 9,
    AC_LAGVERSION = 10,
    AC_ENDGAME = 11,
    AC_DROPLAGJUMPS = 12,
    AC_SETTINGS = 13,
    AC_SAY = 14,
    AC_SHUTDOWN = 16,
    AC_BANDWIDTH = 17,
    AC_DETECTBANDWIDTH = 18,
    AC_WINDOWSIZE = 19,
    AC_ADMINPW = 20
};

struct AdminCommandEntry
{
    AdminCommand cmd;
    char* token;
};

#define AC(n) ((AdminCommand)(n))

// GLOBAL: SWBF2 0x007e8910
AdminCommandEntry adminCommands[94] = {
    { AC_WAITLATE, "waitlate" },          { AC_DROPMAP, "dropmap" },
    { AC_ADDMAP, "addmap" },              { AC_NEXTMAP, "nextmap" },
    { AC_SPLITUPDATE, "splitupdate" },    { AC_PLAYERS, "players" },
    { AC_BOOT, "boot" },                  { AC(7), "kick" },
    { AC_ENDGAME, "endgame" },            { AC_LAGVERSION, "lagversion" },
    { AC_DROPLAGJUMPS, "droplagjumps" },  { AC_SETTINGS, "settings" },
    { AC_SAY, "say" },                    { AC(15), "adminsay" },
    { AC_MAPS, "maps" },                  { AC(5), "randomize" },
    { AC_SHUTDOWN, "shutdown" },          { AC_BANDWIDTH, "bandwidth" },
    { AC_DETECTBANDWIDTH, "detectbandwidth" }, { AC_WINDOWSIZE, "windowsize" },
    { AC_ADMINPW, "adminpw" },            { AC(21), "gamename" },
    { AC(22), "rconport" },               { AC(23), "playerlimit" },
    { AC(24), "playercount" },            { AC(25), "version" },
    { AC(26), "ip" },                     { AC(27), "gameport" },
    { AC(28), "tps" },                    { AC(29), "lan" },
    { AC(30), "password" },               { AC_ADMINPW, "adminpw" },
    { AC(31), "netregion" },              { AC(32), "videostd" },
    { AC(33), "voicemode" },              { AC(34), "heroes" },
    { AC(35), "hrunlock" },               { AC(36), "hrunlockvalue" },
    { AC(37), "hrplayer" },               { AC(38), "hrteam" },
    { AC(39), "hrrespawn" },              { AC(40), "contimelimit" },
    { AC(41), "ctftimelimit" },           { AC(42), "elitimelimit" },
    { AC(43), "huntimelimit" },           { AC(44), "ctfscorelimit" },
    { AC(45), "assscorelimit" },          { AC(46), "huntscorelimit" },
    { AC(47), "conreinforcements" },      { AC(48), "assreinforcements" },
    { AC(49), "teamdamage" },             { AC(50), "forgivetks" },
    { AC(51), "numtksbeforekick" },       { AC(52), "banteamkillers" },
    { AC(53), "autoassignteams" },        { AC(54), "shownames" },
    { AC(55), "aimassist" },              { AC(56), "difficulty" },
    { AC(57), "spawn" },                  { AC(58), "pregametime" },
    { AC(59), "kickvotethreshold" },      { AC(60), "teamvotethreshold" },
    { AC(61), "bots" },                   { AC(62), "conaiperteam" },
    { AC(63), "ctfaiperteam" },           { AC(64), "assaiperteam" },
    { AC(65), "eliaiperteam" },           { AC(66), "autokickscore" },
    { AC(67), "autokickscorevalue" },     { AC(68), "autokickping" },
    { AC(69), "autokickpingvalue" },      { AC(71), "highpingwarnings" },
    { AC(72), "autokickword" },           { AC(73), "autokickwordwarnings" },
    { AC(70), "autokickpingwarnings" },   { AC(74), "autoannounce" },
    { AC(75), "autoannounceperiod" },     { AC(76), "status" },
    { AC(77), "paused" },                 { AC(78), "whatmap" },
    { AC(79), "mapnext" },                { AC(80), "idlemap" },
    { AC(81), "setnextmap" },             { AC(82), "warn" },
    { AC(83), "swap" },                   { AC(84), "ban" },
    { AC(85), "bankey" },                 { AC(86), "unbankey" },
    { AC(87), "banip" },                  { AC(88), "unbanip" },
    { AC(89), "unbanall" },               { AC(90), "awards" },
    { AC(91), "restart" },                { AC(92), "changemap" },
};

#undef AC

// FUNCTION: SWBF2 0x005aee50
bool GetCommand(char* pcToken, AdminCommand& cmd, bool& bNo)
{
    if (*pcToken != '/')
    {
        return false;
    }

    char* pcName = pcToken + 1;
    bNo = false;
    if ((pcName[0] == 'n' || pcName[0] == 'N') && (pcName[1] == 'o' || pcName[1] == 'O'))
    {
        bNo = true;
        pcName += 2;
    }

    for (int i = 0; i < 94; ++i)
    {
        if (!_stricmp(adminCommands[i].token, pcName))
        {
            cmd = adminCommands[i].cmd;
            return true;
        }
    }
    return false;
}

void __fastcall SendAdminMessage(char* pcMessage);
namespace GameLoop { extern bool sQuitRequested; }
namespace GameLoop { extern unsigned int sQuitTurn; }

// FUNCTION: SWBF2 0x005aedb0
void NetAdmin::RemovePlayer(int iPlayer)
{
    admins[iPlayer] = false;
    if (GameLoop::sQuitRequested)
    {
        if (!NetGame::GetTurnNumber() || NetGame::GetTurnNumber() >= GameLoop::sQuitTurn)
        {
            return;
        }
    }

    char acBuffer[64];
    _snprintf(acBuffer, 63, "removed player %s\n", NetGame::GetPlayerName(iPlayer));
    acBuffer[63] = '\0';
    SendAdminMessage(acBuffer);
}

// FUNCTION: SWBF2 0x005aee10
void NetAdmin::AddPlayer(int iPlayer)
{
    char acBuffer[64];
    _snprintf(acBuffer, 63, "added player %s\n", NetGame::GetPlayerName(iPlayer));
    acBuffer[63] = '\0';
    SendAdminMessage(acBuffer);
}

// FUNCTION: SWBF2 0x005aeed0
const char* __fastcall GetMissionModeCode(const char* pcMission)
{
    if (strstr(pcMission, "_1flag"))
    {
        return "CTF1";
    }
    if (strstr(pcMission, "_ctf"))
    {
        return "CTF2";
    }
    if (strstr(pcMission, "_hunt"))
    {
        return "HUN";
    }
    if (strstr(pcMission, "_ass"))
    {
        return "ASS";
    }
    return strstr(pcMission, "_eli") ? "ASS" : "CON";
}

extern int netCurMaxPlayers;

// FUNCTION: SWBF2 0x005aef60
void __fastcall CountConnectedTeams(int* piTeam1, int* piTeam2, int* piWaiting)
{
    *piWaiting = 0;
    *piTeam2 = 0;
    *piTeam1 = 0;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (!NetComm::IsConnected(i))
        {
            continue;
        }
        if (NetGame::IsPlaying(i, false))
        {
            if (NetGame::GetStartTeam(i) == 1)
            {
                ++*piTeam1;
            }
            else
            {
                ++*piTeam2;
            }
        }
        else
        {
            ++*piWaiting;
        }
    }
}

// FUNCTION: SWBF2 0x005aefe0
void __fastcall GetAdminScorePair(int* pi1, int* pi2)
{
    if (!netInShell)
    {
        int i3;
        int i4;
        bool b1;
        bool b2;
        NetGame::GetScore(pi1, pi2, &i3, &i4, &b1, &b2);
        if (b1 || b2)
        {
            return;
        }
    }
    *pi1 = 0;
    *pi2 = 0;
}

// FUNCTION: SWBF2 0x005af030
void __fastcall GetAdminTicketPair(int* pi1, int* pi2)
{
    if (!netInShell)
    {
        int i3;
        int i4;
        bool b1;
        bool b2;
        NetGame::GetScore(pi1, pi2, &i3, &i4, &b1, &b2);
        if (!b1 && !b2)
        {
            return;
        }
    }
    *pi1 = 0;
    *pi2 = 0;
}

// GLOBAL: SWBF2 0x01fa3d24
int adminId;

// GLOBAL: SWBF2 0x01fa2510
bool isRemote;

// GLOBAL: SWBF2 0x01fa2518
char remoteResponse[0x1800];

// GLOBAL: SWBF2 0x01f9ae32
bool remoteLoggedIn;
// GLOBAL: SWBF2 0x01f9ae33
bool isDedicated;

// GLOBAL: SWBF2 0x01fa249c
int adminAutoKickScore;

// GLOBAL: SWBF2 0x01fa24a0
int adminBanTeamKillers;

// GLOBAL: SWBF2 0x01fa24a4
char adminServerIP[16];
// GLOBAL: SWBF2 0x01fa24b4
int adminBandwidth;

// GLOBAL: SWBF2 0x01fa24b8
int adminForgiveTeamKills;

// GLOBAL: SWBF2 0x01fa24bc
int adminAutoKickScoreValue;

// GLOBAL: SWBF2 0x01fa24c0
int adminAutoKickPing;

// GLOBAL: SWBF2 0x01fa24c4
int adminTeamKillsBeforeKick;

// GLOBAL: SWBF2 0x01fa2514
int adminAutoKickPingWarnings;

// GLOBAL: SWBF2 0x01fa3d18
int adminAutoKickPingValue;

// GLOBAL: SWBF2 0x01fa3d1c
int adminAutoKickWordWarnings;

// GLOBAL: SWBF2 0x01fa3d20
int adminHighPingWarnings;

// GLOBAL: SWBF2 0x01fa3d28
int adminAutoKickWord;

extern bool adminVersionRequested;
extern bool adminGameNameRequested;
extern bool adminRemoteReply;
extern bool detectBandwidth;
extern bool netAutoAim;
extern bool netDropLateUpdate;
extern bool netDropLagJumps;
extern bool netSplitUpdate;
extern bool gNoPregame;
extern bool netWaitLate;
extern bool netHeroesEnabled;
extern bool netAutoAssignTeamsOriginalSetting;
extern bool netAutoAssignTeams;
extern bool netShowNames;
extern bool netTeamDamage;
extern int netAutoAnnounce;
extern int netLagVersion;
extern int netVoiceMode;
extern int netThrottle;
extern int gSpawnInvicibility;

extern int netAutoAnnouncePeriod;
extern int netHeroRuleTeam;
extern int netHeroRuleUnlock;
extern int netHeroRuleUnlockVal;
extern int netHeroRulePlayer;
extern int netHeroRuleRespawn;
extern int netWindowSize;
extern int netPlayerLimit;
extern int netHostedPlayerLimit;
extern int netDedicatedPlayerLimit;
extern int hostVoteThreshold;
extern int autoNetPlayers;
extern int netFPS;
extern int hostshort;
extern char aNtsc[16];
extern char adminPlayerIdText[32];
extern char adminBanKeyIdText[32];
extern char adminBanIdText[32];

void NetWaitLate(bool bWait);
void __fastcall FormatRefusedAdminLine(int iPlayer, unsigned short* pusLine);
void FormatPlayerStatusLine(char* pcOut, int iNumber, const char* pcName, int iTeam, int iPoints,
                int iKills, int iDeaths, int iPing, const char* pcId, const char* pcKey);
void MD5Digest(const char* pcText, unsigned int uLength, char* pcHex);
void MissionPlayListStepBack();
void NetDisplayClientMessage();
bool GetNetAwardsEnabled();
void SetNetAwardsEnabled(int);
void AddAutoNetMission(char const* pcArg, int iA, int iB);

class PlayerStats
{
public:
    int GetPoints() const;
    int GetKills(int iTeam, int iStat) const;
    int GetDeaths(int iTeam, int iStat) const;
};

static bool IsMatch(char* pcMatch, unsigned short* pusLine)
{
    while (*pusLine == ' ')
    {
        ++pusLine;
    }
    if (*pusLine != '/')
    {
        return false;
    }
    ++pusLine;

    int c = *pcMatch;
    while (c)
    {
        if (c != *pusLine && c + 32 != *pusLine)
        {
            return false;
        }
        c = *++pcMatch;
        ++pusLine;
    }
    return c == ' ' || c == 0;
}

// FUNCTION: SWBF2 0x005b1300
static bool IsAdmin(unsigned short* pusLine)
{
    return IsMatch("ADMIN", pusLine);
}

// FUNCTION: SWBF2 0x005b1310
static bool IsOption(unsigned short* pusLine)
{
    return IsMatch("OPTION", pusLine);
}

// FUNCTION: SWBF2 0x005b1320
static bool GetNextArg(unsigned short*& pusLine, char* pcArg, int iMax)
{
    while (*pusLine == ' ')
    {
        ++pusLine;
    }
    if (*pusLine == 0)
    {
        return false;
    }

    bool bQuoted = *pusLine == '"';
    if (bQuoted)
    {
        ++pusLine;
    }

    for (;;)
    {
        --iMax;
        unsigned short c = *pusLine++;
        if (bQuoted && c == '"')
        {
            --pusLine;
            break;
        }
        *pcArg++ = (char)c;
        if (bQuoted ? *pusLine == '"' : *pusLine == ' ')
        {
            break;
        }
        if (*pusLine == 0 || iMax <= 0)
        {
            break;
        }
    }
    *pcArg = 0;
    if (bQuoted)
    {
        ++pusLine;
    }
    return true;
}

// FUNCTION: SWBF2 0x005b13b0
static void PutMsg(char* pcMsg)
{
    if (adminId < 0 && !isDedicated)
    {
        strlcat(remoteResponse, pcMsg, sizeof(remoteResponse));
        strlcat(remoteResponse, "\n", sizeof(remoteResponse));
        return;
    }

    unsigned short ausMsg[128];
    PblUnicode::ustrncnv(ausMsg, pcMsg, 128);

    int iDest = 64;
    unsigned __int64 uMask = ~(unsigned __int64)0;
    if (adminId != -1)
    {
        iDest = adminId;
        uMask = ~((unsigned __int64)1 << adminId);
    }

    if (isRemote)
    {
        NetGame::AddChatMessageToQue(true, iDest, uMask, -1, ausMsg, 0);
    }
    else
    {
        NetGame::AddChatMessageToQue(false, iDest, uMask, -1, ausMsg, 0);
        NetDisplayClientMessage();
    }
}

// FUNCTION: SWBF2 0x005b1480
bool IsNum(unsigned short c)
{
    return c >= '0' && c <= '9';
}

// FUNCTION: SWBF2 0x005b14a0
static bool IsNextArgInt(unsigned short* pusLine)
{
    while (*pusLine == ' ')
    {
        ++pusLine;
    }
    if (!IsNum(*pusLine))
    {
        return false;
    }

    for (int i = 0; i < 8; ++i)
    {
        unsigned short c = *++pusLine;
        if (c == ' ' || c == 0)
        {
            return true;
        }
        if (!IsNum(c))
        {
            return false;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x005b1500
static bool IsNextArgString(unsigned short* pusLine)
{
    while (*pusLine == ' ')
    {
        ++pusLine;
    }
    return *pusLine != '/' && *pusLine != 0;
}

// FUNCTION: SWBF2 0x005aecf0
void __fastcall PutAdminNumber(int iValue)
{
    char acBuffer[16];
    _snprintf(acBuffer, 15, "%d", iValue);
    PutMsg(acBuffer);
}

// FUNCTION: SWBF2 0x005aed20
int __fastcall PassHeroUnlockValue(int iValue)
{
    return iValue;
}

// FUNCTION: SWBF2 0x005aed30
int __fastcall PassHeroPlayerValue(int iValue)
{
    return iValue;
}

// FUNCTION: SWBF2 0x005aed40
int __fastcall PassHeroTeamValue(int iValue)
{
    return iValue;
}

// FUNCTION: SWBF2 0x005af090
bool NetAdmin::ParseLineRemote(int iPlayer, unsigned short* pusLine, bool bConsole, bool bPrefixed)
{
    if (bPrefixed && !IsAdmin(pusLine))
    {
        FormatRefusedAdminLine(iPlayer, pusLine);
        return false;
    }

    adminId = iPlayer;
    isRemote = true;
    isDedicated = bConsole;
    remoteResponse[0] = 0;

    char acToken[32 + 1];
    char acArg[64 + 1];
    char acMsg[512];

    if (bPrefixed)
    {
        GetNextArg(pusLine, acToken, 32);
    }

    while (GetNextArg(pusLine, acToken, 32))
    {
        if (!_stricmp(acToken, "/login"))
        {
            if (!GetNextArg(pusLine, acToken, 32))
            {
                return true;
            }
            if (Destination[0] && !_stricmp(acToken, Destination))
            {
                if (iPlayer >= 0)
                {
                    admins[iPlayer] = true;
                }
                else
                {
                    remoteLoggedIn = true;
                }
                _snprintf(acMsg, 0x1FF, "logged in");
                acMsg[511] = 0;
                PutMsg(acMsg);
            }
            continue;
        }

        if ((iPlayer < 0 || !admins[iPlayer]) && !remoteLoggedIn && !bConsole)
        {
            PutMsg("not logged in");
            continue;
        }

        AdminCommand cmd;
        bool bNo;
        if (!GetCommand(acToken, cmd, bNo))
        {
            PutMsg("command not found");
            continue;
        }

        switch (cmd)
        {
        case AC_WAITLATE:
            if (bNo)
            {
                NetWaitLate(false);
                _snprintf(acMsg, 0x1FF, "host will not wait for late moves");
            }
            else
            {
                NetWaitLate(true);
                _snprintf(acMsg, 0x1FF, "host will wait for late moves");
            }
            acMsg[511] = 0;
            PutMsg(acMsg);
            break;

        case AC_DROPMAP:
        {
            char* pcMap = 0;
            if (IsNextArgString(pusLine))
            {
                GetNextArg(pusLine, acArg, 64);
                pcMap = acArg;
            }
            if (MissionPlayList::DropEntry(pcMap))
            {
                PutMsg("map dropped");
            }
            else
            {
                PutMsg("map not dropped");
            }
            break;
        }

        case AC_SPLITUPDATE:
            if (bNo)
            {
                netSplitUpdate = false;
                PutMsg("host will not split update");
            }
            else
            {
                netSplitUpdate = true;
                PutMsg("host will split update");
            }
            break;

        case AC_PLAYERS:
            if (adminRemoteReply)
            {
                for (int i = 0; i < netCurMaxPlayers; ++i)
                {
                    if (!NetComm::IsConnected(i))
                    {
                        continue;
                    }

                    const char* pcName = NetGame::GetPlayerName(i);
                    Character* pCharacter = 0;
                    int iTeam = 0;
                    if (NetGame::IsPlaying(i, false))
                    {
                        iTeam = NetGame::GetStartTeam(i);
                        pCharacter = Character::FindPlayer(i);
                    }

                    int iPoints = 0;
                    int iKills = 0;
                    int iDeaths;
                    if (pCharacter)
                    {
                        PlayerStats& stats = pCharacter->GetPlayerStats();
                        iPoints = stats.GetPoints();
                        iKills = stats.GetKills(-1, -1);
                        iDeaths = stats.GetDeaths(-1, -1);
                    }
                    else
                    {
                        iDeaths = 0;
                    }
                    int iPing = NetComm::GetPing(i);

                    char acKey[33] = "36b7ae01b85eef9ecea666b441d5acd3";
                    _snprintf(adminPlayerIdText, 32, "%ull", *NetCommP::GetAddr(i));
                    MD5Digest(adminPlayerIdText, strlen(adminPlayerIdText), acKey);
                    acKey[32] = 0;

                    FormatPlayerStatusLine(acMsg, i + 1, pcName, iTeam, iPoints, iKills, iDeaths, iPing,
                               adminPlayerIdText, acKey);
                    PutMsg(acMsg);
                }
            }
            else
            {
                for (int i = 0; i < netCurMaxPlayers; ++i)
                {
                    if (NetGame::IsPlaying(i, false))
                    {
                        _snprintf(acMsg, 0x1FF, "%d - '%s'", i + 1, NetGame::GetPlayerName(i));
                        acMsg[511] = 0;
                        PutMsg(acMsg);
                    }
                }
            }
            break;

        case AC_MAPS:
        {
            int iCount = MissionPlayList::GetEntryCount();
            int iEntry = MissionPlayList::GetCurrentEntry();
            int iOnLine = 0;
            acMsg[0] = 0;
            for (int i = 0; i < iCount; ++i)
            {
                if (iEntry >= iCount)
                {
                    iEntry = 0;
                }

                const char* pcName;
                int iTickets1;
                int iTickets2;
                MissionPlayList::GetEntry(iEntry, pcName, iTickets1, iTickets2);
                if (iOnLine == 0)
                {
                    strncat(acMsg, pcName, 512);
                    iOnLine = 1;
                }
                else if (iOnLine == 8)
                {
                    strncat(acMsg, ", ", 512);
                    strncat(acMsg, pcName, 512);
                    PutMsg(acMsg);
                    acMsg[0] = 0;
                    iOnLine = 0;
                }
                else
                {
                    strncat(acMsg, ", ", 512);
                    strncat(acMsg, pcName, 512);
                    ++iOnLine;
                }
                ++iEntry;
            }
            if (acMsg[0])
            {
                PutMsg(acMsg);
            }
            break;
        }

        case 5:
            if (GetNextArg(pusLine, acToken, 32))
            {
                autoNetRandomize = atoi(acToken) != 0;
            }
            else
            {
                PutAdminNumber(autoNetRandomize != 0);
            }
            break;

        case AC_BOOT:
        case 7:
        {
            if (!GetNextArg(pusLine, acToken, 32))
            {
                break;
            }
            int iTarget = atoi(acToken);
            GetNextArg(pusLine, acToken, 32);
            --iTarget;
            if (iTarget >= 0 && iTarget < netCurMaxPlayers && NetGame::IsPlaying(iTarget, false))
            {
                NetGame::ForceBootPlayer(iTarget);
                _snprintf(acMsg, 0x1FF, "booted player %d (%s)", iTarget + 1,
                          NetGame::GetPlayerName(iTarget));
            }
            else
            {
                _snprintf(acMsg, 0x1FF, "can't boot player %d", iTarget + 1);
            }
            acMsg[511] = 0;
            PutMsg(acMsg);
            break;
        }

        case AC_ADDMAP:
        {
            if (!GetNextArg(pusLine, acToken, 32))
            {
                break;
            }
            if (netInShell)
            {
                AddAutoNetMission(acToken, 0, 0);
                break;
            }

            PblHash hMap(acToken);
            int iTickets1 = 0;
            int iTickets2 = 0;
            if (IsNextArgInt(pusLine))
            {
                GetNextArg(pusLine, acArg, 64);
                iTickets1 = atoi(acArg);
                if (IsNextArgInt(pusLine))
                {
                    GetNextArg(pusLine, acArg, 64);
                    iTickets2 = atoi(acArg);
                }
            }
            if (iTickets1 < 0 || iTickets2 < 0)
            {
                _snprintf(acMsg, 0x1FF, "map '%s' not added", acToken);
                acMsg[511] = 0;
                PutMsg(acMsg);
            }
            else
            {
                MissionPlayList::AddEntry(acToken, iTickets1, iTickets2);
                PutMsg("map added");
            }
            break;
        }

        case AC_NEXTMAP:
            if (GetNextArg(pusLine, acToken, 32) && !netInShell)
            {
                PblHash hMap(acToken);
                if (MissionPlayList::NextEntry(acToken))
                {
                    PutMsg("next map set");
                }
                else
                {
                    PutMsg("map not found");
                }
            }
            break;

        case AC_LAGVERSION:
            if (!GetNextArg(pusLine, acToken, 32))
            {
                break;
            }
            netLagVersion = atoi(acToken);
            _snprintf(acMsg, 0x1FF, "host will use lag version %d", netLagVersion);
            acMsg[511] = 0;
            PutMsg(acMsg);
            break;

        case AC_ENDGAME:

            Team::sWinningTeams = 6;
            PutMsg("ended");
            break;

        case AC_DROPLAGJUMPS:
            if (bNo)
            {
                netDropLagJumps = false;
                _snprintf(acMsg, 0x1FF, "host will not drop lag jumps");
            }
            else
            {
                netDropLagJumps = true;
                _snprintf(acMsg, 0x1FF, "host will drop lag jumps");
            }
            acMsg[511] = 0;
            PutMsg(acMsg);
            break;

        case AC_SETTINGS:
            _snprintf(acMsg, 0x1FF, "waitlate(%d) lagversion(%d) splitupdate(%d)",
                      netWaitLate != 0, netLagVersion, netSplitUpdate != 0);
            acMsg[511] = 0;
            PutMsg(acMsg);
            break;

        case AC_SAY:
        case 15:
            while (*pusLine == ' ')
            {
                ++pusLine;
            }
            NetGame::AddChatMessageToQue(true, 64, 0, -1, pusLine, 0);
            pusLine += PblUnicode::ustrlen(pusLine);
            PutMsg("done");
            break;

        case AC_SHUTDOWN:
            PutMsg("quitting");
            NetSetup::RequestQuit(0);
            break;

        case AC_BANDWIDTH:
            if (netInShell)
            {
                if (GetNextArg(pusLine, acToken, 32))
                {
                    netThrottle = atoi(acToken);
                }
                else
                {
                    PutAdminNumber(netThrottle);
                }
            }
            else if (adminRemoteReply)
            {
                if (GetNextArg(pusLine, acToken, 32))
                {
                    int iKbps = atoi(acToken);
                    netThrottle = iKbps;
                    int iLimit;
                    int iOther;
                    NetComm::GetBandwidth(64, iLimit, iOther);
                    NetComm::SetBandwidth(64, iKbps * 1024, iOther);
                }
                else
                {
                    PutAdminNumber(netThrottle);
                }
            }
            else
            {
                int iLimit;
                int iOther;
                NetComm::GetBandwidth(64, iLimit, iOther);
                if (IsNextArgString(pusLine))
                {
                    GetNextArg(pusLine, acArg, 64);
                    iLimit = atoi(acArg);
                    if (iLimit < 33 || iLimit > 3071)
                    {
                        PutMsg("bandwidth not set");
                    }
                    else
                    {
                        _snprintf(acMsg, 0x1FF, "setting bandwidth to %d kbps", iLimit);
                        acMsg[511] = 0;
                        PutMsg(acMsg);
                        NetComm::SetBandwidth(64, iLimit * 1024, iOther);
                    }
                }
                else
                {
                    _snprintf(acMsg, 0x1FF, "bandwidth: used(%d) limit(%d)", adminBandwidth,
                              iLimit / 1024);
                    acMsg[511] = 0;
                    PutMsg(acMsg);
                }
            }
            break;

        case AC_DETECTBANDWIDTH:
            detectBandwidth = true;
            break;

        case AC_WINDOWSIZE:
            if (IsNextArgString(pusLine))
            {
                GetNextArg(pusLine, acArg, 64);
                int iSize = atoi(acArg);
                if (iSize < 1 || iSize > 5)
                {
                    PutMsg("window size not set");
                }
                else
                {
                    _snprintf(acMsg, 0x1FF, "setting window size to %d", iSize);
                    acMsg[511] = 0;
                    PutMsg(acMsg);
                    netWindowSize = iSize;
                }
            }
            else
            {
                _snprintf(acMsg, 0x1FF, "window size %d", netWindowSize);
                acMsg[511] = 0;
                PutMsg(acMsg);
            }
            break;

        case AC_ADMINPW:
            if (GetNextArg(pusLine, acToken, 32))
            {
                strncpy(Destination, acToken, 16);
                Destination[15] = 0;
            }
            else
            {
                PutMsg(Destination);
            }
            break;

        case 21:
            if (GetNextArg(pusLine, acToken, 32))
            {
                strlcpy(autoNetSession, acToken, 64);
            }
            else
            {
                adminGameNameRequested = true;
                PutMsg(autoNetSession);
            }
            break;

        case 22:
        case 26:
        case 87:
        case 88:
        case 89:
            break;

        case 23:
            if (GetNextArg(pusLine, acToken, 32))
            {
                if (netInShell)
                {
                    int iLimit = atoi(acToken);
                    if (iLimit > 64)
                    {
                        iLimit = 64;
                    }
                    netPlayerLimit = iLimit;
                    netHostedPlayerLimit = iLimit;
                    netDedicatedPlayerLimit = iLimit;
                }
            }
            else
            {
                PutAdminNumber(netPlayerLimit);
            }
            break;

        case 24:
            if (GetNextArg(pusLine, acToken, 32))
            {
                if (netInShell)
                {
                    autoNetPlayers = atoi(acToken);
                }
            }
            else
            {
                PutAdminNumber(autoNetPlayers);
            }
            break;

        case 25:
            adminVersionRequested = true;
            PutMsg("1.1");
            break;

        case 27:
            if (GetNextArg(pusLine, acToken, 32))
            {
                hostshort = atoi(acToken);
            }
            else
            {
                PutAdminNumber(hostshort);
            }
            break;

        case 28:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netFPS = atoi(acToken);
            }
            else
            {
                PutAdminNumber(netFPS);
            }
            break;

        case 29:
            if (GetNextArg(pusLine, acToken, 32))
            {
                autoNetLan = atoi(acToken) != 0;
            }
            else
            {
                PutAdminNumber(autoNetLan != 0);
            }
            break;

        case 30:
            if (GetNextArg(pusLine, acToken, 32))
            {
                strlcpy(gGamespyAutoPassStr, acToken, sizeof(gGamespyAutoPassStr));
                if (gGamespyAutoPassStr[0])
                {
                    gGamespyAutoPass = NetSetup::CalcPassword(gGamespyAutoPassStr);
                    netPasswordHash = gGamespyAutoPass;
                }
            }
            else
            {
                PutMsg(gGamespyAutoPassStr);
            }
            break;

        case 31:
            if (!GetNextArg(pusLine, acToken, 32))
            {
                PutMsg("NA");
            }
            break;

        case 32:
            if (GetNextArg(pusLine, acToken, 32))
            {
                strlcpy(aNtsc, acToken, 16);
            }
            else
            {
                PutMsg(aNtsc);
            }
            break;

        case 33:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netVoiceMode = atoi(acToken);
            }
            else
            {
                PutAdminNumber(netVoiceMode);
            }
            break;

        case 34:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netHeroesEnabled = atoi(acToken) != 0;
                Lua_Callbacks::gHeroesEnabled[0] = netHeroesEnabled;
                Lua_Callbacks::gHeroesEnabled[1] = netHeroesEnabled;
            }
            else
            {
                PutAdminNumber(netHeroesEnabled != 0);
            }
            break;

        case 35:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netHeroRuleUnlock = PassHeroUnlockValue(atoi(acToken));
                Lua_Callbacks::gHeroRuleUnlock[0] = netHeroRuleUnlock;
                Lua_Callbacks::gHeroRuleUnlock[1] = netHeroRuleUnlock;
            }
            else
            {
                PutAdminNumber(netHeroRuleUnlock);
            }
            break;

        case 36:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netHeroRuleUnlockVal = atoi(acToken);
                Lua_Callbacks::gHeroRuleUnlockVal[0] = netHeroRuleUnlockVal;
                Lua_Callbacks::gHeroRuleUnlockVal[1] = netHeroRuleUnlockVal;
            }
            else
            {
                PutAdminNumber(netHeroRuleUnlockVal);
            }
            break;

        case 37:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netHeroRulePlayer = PassHeroPlayerValue(atoi(acToken));
                Lua_Callbacks::gHeroRulePlayer[0] = netHeroRulePlayer;
                Lua_Callbacks::gHeroRulePlayer[1] = netHeroRulePlayer;
            }
            else
            {
                PutAdminNumber(netHeroRulePlayer);
            }
            break;

        case 38:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netHeroRuleTeam = PassHeroTeamValue(atoi(acToken));
                Lua_Callbacks::gHeroRuleTeam[0] = netHeroRuleTeam;
                Lua_Callbacks::gHeroRuleTeam[1] = netHeroRuleTeam;
            }
            else
            {
                PutAdminNumber(netHeroRuleTeam);
            }
            break;

        case 39:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netHeroRuleRespawn = atoi(acToken);
                Lua_Callbacks::gHeroRuleRespawn[0] = netHeroRuleRespawn;
                Lua_Callbacks::gHeroRuleRespawn[1] = netHeroRuleRespawn;
            }
            else
            {
                PutAdminNumber(netHeroRuleRespawn);
            }
            break;

        case 40:
            if (GetNextArg(pusLine, acToken, 32))
            {
                Lua_Callbacks::gCONMaxTimeLimit[0] = atoi(acToken);
                Lua_Callbacks::gCONMaxTimeLimit[1] = Lua_Callbacks::gCONMaxTimeLimit[0];
            }
            else
            {
                PutAdminNumber(Lua_Callbacks::gCONMaxTimeLimit[1]);
            }
            break;

        case 41:
            if (GetNextArg(pusLine, acToken, 32))
            {
                Lua_Callbacks::gCTFMaxTimeLimit[0] = atoi(acToken);
                Lua_Callbacks::gCTFMaxTimeLimit[1] = Lua_Callbacks::gCTFMaxTimeLimit[0];
            }
            else
            {
                PutAdminNumber(Lua_Callbacks::gCTFMaxTimeLimit[1]);
            }
            break;

        case 42:
        case 65:
            if (!GetNextArg(pusLine, acToken, 32))
            {
                PutAdminNumber(0);
            }
            break;

        case 43:
            if (GetNextArg(pusLine, acToken, 32))
            {
                Lua_Callbacks::gHuntMaxTimeLimit[0] = atoi(acToken);
                Lua_Callbacks::gHuntMaxTimeLimit[1] = Lua_Callbacks::gHuntMaxTimeLimit[0];
            }
            else
            {
                PutAdminNumber(Lua_Callbacks::gHuntMaxTimeLimit[1]);
            }
            break;

        case 44:
            if (GetNextArg(pusLine, acToken, 32))
            {
                int iLimit = atoi(acToken);
                if (!iLimit)
                {
                    iLimit = 5;
                }
                Lua_Callbacks::gCTFScore[0] = iLimit;
                Lua_Callbacks::gCTFScore[1] = iLimit;
            }
            else
            {
                PutAdminNumber(Lua_Callbacks::gCTFScore[1]);
            }
            break;

        case 45:
            if (GetNextArg(pusLine, acToken, 32))
            {
                int iLimit = atoi(acToken);
                if (!iLimit)
                {
                    iLimit = 180;
                }
                Lua_Callbacks::gSpaceAssaultScoreLimit[0] = iLimit;
                Lua_Callbacks::gSpaceAssaultScoreLimit[1] = iLimit;
            }
            else
            {
                PutAdminNumber(Lua_Callbacks::gSpaceAssaultScoreLimit[1]);
            }
            break;

        case 46:
            if (GetNextArg(pusLine, acToken, 32))
            {
                int iLimit = atoi(acToken);
                if (!iLimit)
                {
                    iLimit = 50;
                }
                Lua_Callbacks::gHuntScoreLimit[0] = iLimit;
                Lua_Callbacks::gHuntScoreLimit[1] = iLimit;
            }
            else
            {
                PutAdminNumber(Lua_Callbacks::gHuntScoreLimit[1]);
            }
            break;

        case 47:
            if (GetNextArg(pusLine, acToken, 32))
            {
                Lua_Callbacks::gCONReinforcementMult[0] = atoi(acToken);
                Lua_Callbacks::gCONReinforcementMult[1] = Lua_Callbacks::gCONReinforcementMult[0];
            }
            else
            {
                PutAdminNumber(Lua_Callbacks::gCONReinforcementMult[1]);
            }
            break;

        case 48:
            if (GetNextArg(pusLine, acToken, 32))
            {
                Lua_Callbacks::gDMReinforcementMult[0] = atoi(acToken);
                Lua_Callbacks::gDMReinforcementMult[1] = Lua_Callbacks::gDMReinforcementMult[0];
            }
            else
            {
                PutAdminNumber(Lua_Callbacks::gDMReinforcementMult[1]);
            }
            break;

        case 49:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netTeamDamage = atoi(acToken) != 0;
            }
            else
            {
                PutAdminNumber(netTeamDamage != 0);
            }
            break;

        case 50:
            if (GetNextArg(pusLine, acToken, 32))
            {
                adminForgiveTeamKills = atoi(acToken);
            }
            else
            {
                PutAdminNumber(adminForgiveTeamKills);
            }
            break;

        case 51:
            if (GetNextArg(pusLine, acToken, 32))
            {
                adminTeamKillsBeforeKick = atoi(acToken);
            }
            else
            {
                PutAdminNumber(adminTeamKillsBeforeKick);
            }
            break;

        case 52:
            if (GetNextArg(pusLine, acToken, 32))
            {
                adminBanTeamKillers = atoi(acToken);
            }
            else
            {
                PutAdminNumber(adminBanTeamKillers);
            }
            break;

        case 53:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netAutoAssignTeams = atoi(acToken) != 0;
                netAutoAssignTeamsOriginalSetting = netAutoAssignTeams;
            }
            else
            {
                PutAdminNumber(netAutoAssignTeams != 0);
            }
            break;

        case 54:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netShowNames = atoi(acToken) != 0;
            }
            else
            {
                PutAdminNumber(netShowNames != 0);
            }
            break;

        case 55:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netAutoAim = atoi(acToken) != 0;
            }
            else
            {
                PutAdminNumber(netAutoAim != 0);
            }
            break;

        case 56:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netDifficulty = atoi(acToken);
            }
            else
            {
                PutAdminNumber(netDifficulty);
            }
            break;

        case 57:
            if (GetNextArg(pusLine, acToken, 32))
            {
                gSpawnInvicibility = atoi(acToken);
            }
            else
            {
                PutAdminNumber(gSpawnInvicibility);
            }
            break;

        case 58:
            if (GetNextArg(pusLine, acToken, 32))
            {
                int iSeconds = atoi(acToken);
                NetGame::SetMaxPreGameTime(iSeconds);
                if (iSeconds <= 0)
                {
                    gNoPregame = true;
                }
            }
            else
            {
                PutAdminNumber(NetGame::GetMaxPreGameTime());
            }
            break;

        case 59:
        case 60:
            if (GetNextArg(pusLine, acToken, 32))
            {
                hostVoteThreshold = atoi(acToken);
            }
            else
            {
                PutAdminNumber(hostVoteThreshold);
            }
            break;

        case 61:
            if (!GetNextArg(pusLine, acToken, 32))
            {
                PutAdminNumber(Lua_Callbacks::gCONNumBots[1]);
            }
            break;

        case 62:
            if (GetNextArg(pusLine, acToken, 32))
            {
                Lua_Callbacks::gCONNumBots[0] = atoi(acToken);
                Lua_Callbacks::gCONNumBots[1] = Lua_Callbacks::gCONNumBots[0];
                Lua_Callbacks::gCONNumBots[2] = Lua_Callbacks::gCONNumBots[0];
                Lua_Callbacks::gCONNumBots[3] = Lua_Callbacks::gCONNumBots[0];
            }
            else
            {
                PutAdminNumber(Lua_Callbacks::gCONNumBots[1]);
            }
            break;

        case 63:
            if (GetNextArg(pusLine, acToken, 32))
            {
                Lua_Callbacks::gCTFNumBots[0] = atoi(acToken);
                Lua_Callbacks::gCTFNumBots[1] = Lua_Callbacks::gCTFNumBots[0];
                Lua_Callbacks::gCTFNumBots[2] = Lua_Callbacks::gCTFNumBots[0];
                Lua_Callbacks::gCTFNumBots[3] = Lua_Callbacks::gCTFNumBots[0];
            }
            else
            {
                PutAdminNumber(Lua_Callbacks::gCTFNumBots[1]);
            }
            break;

        case 64:
            if (GetNextArg(pusLine, acToken, 32))
            {
                Lua_Callbacks::gASSNumBots[0] = atoi(acToken);
                Lua_Callbacks::gASSNumBots[1] = Lua_Callbacks::gASSNumBots[0];
                Lua_Callbacks::gASSNumBots[2] = Lua_Callbacks::gASSNumBots[0];
                Lua_Callbacks::gASSNumBots[3] = Lua_Callbacks::gASSNumBots[0];
            }
            else
            {
                PutAdminNumber(Lua_Callbacks::gASSNumBots[1]);
            }
            break;

        case 66:
            if (GetNextArg(pusLine, acToken, 32))
            {
                adminAutoKickScore = atoi(acToken);
            }
            else
            {
                PutAdminNumber(adminAutoKickScore);
            }
            break;

        case 67:
            if (GetNextArg(pusLine, acToken, 32))
            {
                adminAutoKickScoreValue = atoi(acToken);
            }
            else
            {
                PutAdminNumber(adminAutoKickScoreValue);
            }
            break;

        case 68:
            if (GetNextArg(pusLine, acToken, 32))
            {
                adminAutoKickPing = atoi(acToken);
            }
            else
            {
                PutAdminNumber(adminAutoKickPing);
            }
            break;

        case 69:
            if (GetNextArg(pusLine, acToken, 32))
            {
                adminAutoKickPingValue = atoi(acToken);
            }
            else
            {
                PutAdminNumber(adminAutoKickPingValue);
            }
            break;

        case 70:
            if (GetNextArg(pusLine, acToken, 32))
            {
                adminAutoKickPingWarnings = atoi(acToken);
            }
            else
            {
                PutAdminNumber(adminAutoKickPingWarnings);
            }
            break;

        case 71:
            if (GetNextArg(pusLine, acToken, 32))
            {
                adminHighPingWarnings = atoi(acToken);
            }
            else
            {
                PutAdminNumber(adminHighPingWarnings);
            }
            break;

        case 72:
            if (GetNextArg(pusLine, acToken, 32))
            {
                adminAutoKickWord = atoi(acToken);
            }
            else
            {
                PutAdminNumber(adminAutoKickWord);
            }
            break;

        case 73:
            if (GetNextArg(pusLine, acToken, 32))
            {
                adminAutoKickWordWarnings = atoi(acToken);
            }
            else
            {
                PutAdminNumber(adminAutoKickWordWarnings);
            }
            break;

        case 74:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netAutoAnnounce = atoi(acToken);
            }
            else
            {
                PutAdminNumber(netAutoAnnounce);
            }
            break;

        case 75:
            if (GetNextArg(pusLine, acToken, 32))
            {
                netAutoAnnouncePeriod = atoi(acToken);
            }
            else
            {
                PutAdminNumber(netAutoAnnouncePeriod);
            }
            break;

        case 76:
        {
            adminStatusRequested = true;
            _snprintf(acMsg, 0x200, "Server name : %s", autoNetSession);
            PutMsg(acMsg);
            _snprintf(acMsg, 0x200, "Server IP   : %s", adminServerIP);
            PutMsg(acMsg);
            PutMsg("Version     : 1.00");
            _snprintf(acMsg, 0x200, "Max players : %d", netPlayerLimit);
            PutMsg(acMsg);
            _snprintf(acMsg, 0x200, "Password    : %s", gGamespyAutoPassStr);
            PutMsg(acMsg);
            _snprintf(acMsg, 0x200, "Current map : %s", GameLoop::mMissionScript);
            PutMsg(acMsg);

            const char* pcNext = "";
            unsigned int uiTeam1Hash;
            unsigned int uiTeam2Hash;
            MissionPlayList::PeekNextEntry(pcNext, uiTeam1Hash, uiTeam2Hash);
            _snprintf(acMsg, 0x200, "Next map    : %s", pcNext);
            PutMsg(acMsg);
            _snprintf(acMsg, 0x200, "Game mode   : %s", GetMissionModeCode(GameLoop::mMissionScript));
            PutMsg(acMsg);

            int iTeam1;
            int iTeam2;
            int iWaiting;
            CountConnectedTeams(&iTeam1, &iTeam2, &iWaiting);
            _snprintf(acMsg, 0x200, "Players     : %d/%d/%d", iTeam1, iTeam2,
                      iTeam1 + iTeam2 + iWaiting);
            PutMsg(acMsg);

            int iScore1;
            int iScore2;
            GetAdminScorePair(&iScore1, &iScore2);
            _snprintf(acMsg, 0x200, "Scores      : %d/%d/%d", iScore1, iScore2, iScore1 + iScore2);
            PutMsg(acMsg);

            int iTickets1;
            int iTickets2;
            GetAdminTicketPair(&iTickets1, &iTickets2);
            _snprintf(acMsg, 0x200, "Tickets     : %d/%d", iTickets1, iTickets2);
            PutMsg(acMsg);

            _snprintf(acMsg, 0x200, "FF enabled  : %s", netTeamDamage ? "yes" : "no");
            PutMsg(acMsg);
            _snprintf(acMsg, 0x200, "Heroes      : %s", netHeroesEnabled ? "yes" : "no");
            PutMsg(acMsg);
            break;
        }

        case 77:
            PutAdminNumber(0);
            break;

        case 78:
            PutMsg(GameLoop::mMissionScript);
            break;

        case 79:
        {
            const char* pcNext = "";
            unsigned int uiTeam1Hash;
            unsigned int uiTeam2Hash;
            MissionPlayList::PeekNextEntry(pcNext, uiTeam1Hash, uiTeam2Hash);
            PutMsg((char*)pcNext);
            break;
        }

        case 81:
            GetNextArg(pusLine, acToken, 32);
            break;

        case 82:
        {
            if (!GetNextArg(pusLine, acToken, 32))
            {
                PutMsg("no player id");
                break;
            }
            while (*pusLine == ' ')
            {
                ++pusLine;
            }
            int iTarget = atoi(acToken) - 1;
            if (iTarget < 0 || iTarget >= netCurMaxPlayers || !NetComm::IsConnected(iTarget))
            {
                PutMsg("invalid player");
                break;
            }
            NetGame::AddChatMessageToQue(true, 64, ~((unsigned __int64)1 << iTarget), -1, pusLine, 0);
            pusLine += PblUnicode::ustrlen(pusLine);
            PutMsg("done");
            break;
        }

        case 83:
        {
            if (!GetNextArg(pusLine, acToken, 32))
            {
                PutMsg("no player id");
                break;
            }
            int iTarget = atoi(acToken) - 1;
            if (iTarget < 0 || iTarget >= netCurMaxPlayers || !NetComm::IsConnected(iTarget))
            {
                PutMsg("invalid player");
                break;
            }
            NetGame::SetStartTeam(iTarget, 3 - NetGame::GetStartTeam(iTarget));
            NetSetup::PlayerDataChanged(iTarget);

            Character* pCharacter = Character::FindPlayer(iTarget);
            if (pCharacter && pCharacter->mUnit)
            {
                if (pCharacter->mRemote)
                {
                    pCharacter->mUnit->ExitRemote();
                }
                if (pCharacter->mVehicle)
                {
                    pCharacter->mUnit->ExitVehicle(true, true);
                }

                Trackable* pTrackable = pCharacter->mUnit;
                unsigned int* puFlags =
                    (unsigned int*)((char*)pTrackable->GetGameObject() + 0x1fc);
                *puFlags = (*puFlags & ~8u) | 0x10;
            }
            break;
        }

        case 84:
        {
            if (!GetNextArg(pusLine, acToken, 32))
            {
                break;
            }
            int iTarget = atoi(acToken) - 1;
            if (iTarget >= 0 && iTarget < netCurMaxPlayers && NetComm::IsConnected(iTarget))
            {
                char acKey[33] = "36b7ae01b85eef9ecea666b441d5acd3";
                _snprintf(adminBanIdText, 32, "%ull", *NetCommP::GetAddr(iTarget));
                MD5Digest(adminBanIdText, strlen(adminBanIdText), acKey);
                acKey[32] = 0;
                NetGame::ForceBootPlayer(iTarget);
                if (GetNextArg(pusLine, acArg, 64))
                {
                    GetNextArg(pusLine, acArg, 64);
                }
            }
            break;
        }

        case 85:
        {
            if (!GetNextArg(pusLine, acArg, 64))
            {
                break;
            }
            for (int i = 0; i < netCurMaxPlayers; ++i)
            {
                if (!NetComm::IsConnected(i))
                {
                    continue;
                }

                char acKey[33] = "36b7ae01b85eef9ecea666b441d5acd3";
                _snprintf(adminBanKeyIdText, 32, "%ull", *NetCommP::GetAddr(iPlayer));
                MD5Digest(adminBanKeyIdText, strlen(adminBanKeyIdText), acKey);
                acKey[32] = 0;
                if (!strcmp(acKey, acArg))
                {
                    NetGame::ForceBootPlayer(i);
                    break;
                }
            }
            break;
        }

        case 86:
            GetNextArg(pusLine, acArg, 64);
            break;

        case 90:
            if (GetNextArg(pusLine, acToken, 32))
            {
                SetNetAwardsEnabled(atoi(acToken) != 0);
            }
            else
            {
                PutAdminNumber(GetNetAwardsEnabled() != 0);
            }
            break;

        case 91:
            MissionPlayListStepBack();
            Team::sWinningTeams = 6;
            break;

        case 92:
            if (GetNextArg(pusLine, acToken, 32) && !netInShell)
            {
                PblHash hMap(acToken);
                if (MissionPlayList::NextEntry(acToken))
                {
                    PutMsg("next map set");
                }
                else
                {
                    MissionPlayList::AddEntry(acToken, 0, 0);
                    PutMsg("map added");
                }
                Team::sWinningTeams = 6;
            }
            break;

        default:
            PutMsg("command not handled");
            break;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x005b11c0
bool NetAdmin::ParseLineLocal(unsigned short* pusLine, bool bConsole)
{
    if (!IsOption(pusLine))
    {
        return false;
    }

    NetGame::EnterClient();
    adminId = NetGame::GetLocalIndex(0);
    isRemote = false;
    isDedicated = bConsole;

    char acToken[32 + 1];
    GetNextArg(pusLine, acToken, 32);
    while (GetNextArg(pusLine, acToken, 32))
    {
        if (!_stricmp(acToken, "/droplateupdate"))
        {
            netDropLateUpdate = true;
            PutMsg("client will drop late updates");
        }
        else if (!_stricmp(acToken, "/nodroplateupdate"))
        {
            netDropLateUpdate = false;
            PutMsg("client will keep late updates");
        }
    }
    NetGame::LeaveClient();
    return true;
}

// FUNCTION: SWBF2 0x005b21d0
void __fastcall ParseAutoNetCommandLine(char* pCmdLine)
{

    if (!strstr(pCmdLine, "/autonet "))
    {
        return;
    }
    if (strstr(pCmdLine, "/gamename "))
    {
        return;
    }

    char* pcOption = strstr(pCmdLine, "/configdir ");
    if (pcOption)
    {
        char* pcValue = strchr(pcOption, ' ');
        char cTerminator = ' ';

        while (*pcValue == ' ')
        {
            ++pcValue;
        }

        if (*pcValue == '"')
        {
            ++pcValue;
            cTerminator = '"';
        }

        char* pcEnd = pcValue;
        while (*pcEnd != '\0' && *pcEnd != cTerminator)
        {
            ++pcEnd;
        }

        unsigned int uiLength = (unsigned int)(pcEnd - pcValue);
        if (uiLength > 0x1FF)
        {
            uiLength = 0x1FF;
        }

        strncpy(aSettings, pcValue, uiLength);
        aSettings[uiLength] = '\0';
    }

    LoadServerConfigSection("ServerSettings");
    LoadServerConfigSection("ServerRotation");
    LoadServerBans();

    DWORD dwStart = GetTickCount();
    if (Stream)
    {
        fprintf(Stream, "%d - start\n", dwStart);
    }

    while (GetTickCount() - dwStart < 0x4E20)
    {
        if (adminStatusRequested)
        {
            break;
        }
        Sleep(100);
    }

    if (Stream)
    {
        fclose(Stream);
        Stream = 0;
    }
}

// FUNCTION: SWBF2 0x005b16b0
void NetAdmin::ReceiveConsole()
{
    for (NetPacket* pPacket = NetComm::Recv(PKT_CONSOLE, PKT_NONE);
         pPacket; pPacket = NetComm::Recv(PKT_CONSOLE, PKT_NONE))
    {
        int iDest = (unsigned char)pPacket->mData[3];
        pPacket->BeginRead();
        unsigned char uCommand, uKey;
        pPacket->Read(uCommand);
        pPacket->Read(uKey);

        if (uCommand == 0x29)
        {
            HeartBeat(NetCommP::GetAddr(iDest));
            unsigned char uUnused;
            pPacket->Read(uUnused);
            pPacket->Free();

            NetPktGroup reply;
            reply.BeginWrite();
            reply.WriteBits((char)adminKey[adminKeyCount & 1], 8);
            reply.WriteBits(0, 8);
            reply.EndWrite();
            NetComm::Send(iDest, PKT_CONSOLE_ACK, &reply);
        }
        else if (uCommand == 's')
        {
            pPacket->Free();
            NetPktGroup reply;
            reply.BeginWrite();
            reply.WriteBits((char)adminKey[adminKeyCount & 1], 8);
            reply.WriteBits(1, 8);
            WriteString(&reply, "hello", 128);
            reply.WriteBits(0, 8);
            reply.EndWrite();
            NetComm::Send(iDest, PKT_CONSOLE_ACK, &reply);
        }
        else if (uCommand == 'J')
        {
            unsigned char uUnused, uLength;
            pPacket->Read(uUnused);
            pPacket->Read(uLength);
            unsigned short ausLine[64];
            const char* pcData = pPacket->mData + pPacket->mCurIndex;
            for (unsigned int i = 0; i < uLength; ++i)
            {
                ausLine[i] = (short)pcData[i];
            }
            ausLine[uLength] = 0;
            pPacket->Free();

            int iAdmin = FindAdminSource(iDest);
            if (iAdmin < 0)
            {
                continue;
            }
            bool bAdmin = adminDests[iAdmin].admin;
            remoteLoggedIn = bAdmin;
            NetAdmin::ParseLineRemote(-1, ausLine, false, true);
            adminDests[iAdmin].admin = remoteLoggedIn;

            NetPktGroup reply;
            reply.BeginWrite();
            reply.WriteBits((char)adminKey[adminKeyCount & 1], 8);
            for (char* pcLine = remoteResponse; ; )
            {
                char* pcEnd = strchr(pcLine, '\n');
                if (!pcEnd)
                {
                    break;
                }
                *pcEnd = 0;
                reply.WriteBits(1, 8);
                WriteString(&reply, pcLine, 128);
                pcLine = pcEnd + 1;
            }
            reply.WriteBits(0, 8);
            reply.EndWrite();
            NetComm::Send(iDest, PKT_CONSOLE_ACK, &reply);
        }
        else
        {
            pPacket->Free();
        }
    }
    SendStateTransitions();
    DetectBandwidth();
}

// FUNCTION: SWBF2 0x005b1e20
void FormatPlayerStatusLine(char* pcOut, int iNumber, const char* pcName, int iTeam, int iPoints,
                int iKills, int iDeaths, int iPing, const char* pcId, const char* pcKey)
{
    strcpy(pcOut, "1  \"b\"               Non 112 15  11  120  1.2.3.4         36b7ae01b85eef9ecea666b441d5acd3");
    _snprintf(pcOut, 512, "%d", iNumber);
    pcOut[strlen(pcOut)] = ' ';
    _snprintf(pcOut + 3, 17, "\"%s\"", pcName);
    unsigned int uLength = strlen(pcOut + 3);
    if (uLength >= 17)
    {
        pcOut[19] = '"';
    }
    else
    {
        pcOut[uLength + 3] = ' ';
    }
    Team* pTeam = iTeam > 0 ? Team::sTeams[iTeam] : 0;
    if (pTeam && pTeam->mName)
    {
        char acTeam[128];
        PblUnicode::ustrncnv(acTeam, pTeam->mName, 128);
        strncpy(pcOut + 21, acTeam, 3);
        pcOut[strlen(pcOut)] = ' ';
    }
    _snprintf(pcOut + 25, 16, "%-3d %-3d %-3d %-4d", iPoints, iKills, iDeaths, iPing);
    pcOut[strlen(pcOut)] = ' ';
    strcpy(pcOut + 41, pcId);
    pcOut[strlen(pcOut)] = ' ';
    strcpy(pcOut + 57, pcKey);
}

// FUNCTION: SWBF2 0x005b1f70
void __fastcall FormatRefusedAdminLine(int iPlayer, unsigned short* pusLine)
{
    const char* pcPlayer = "admin";
    if (iPlayer != 64)
    {
        pcPlayer = NetGame::GetPlayerName(iPlayer);
        if (!pcPlayer)
        {
            pcPlayer = "None";
        }
    }
    char acLine[512];
    char acMessage[512];
    PblUnicode::ustrncnv(acLine, pusLine, 512);
    _snprintf(acMessage, 512, "\t%s\t%s", pcPlayer, acLine);
}

// FUNCTION: SWBF2 0x005b1660
bool __fastcall IsAdminPacketAuthorized(const unsigned char* pRawPacket)
{
    const NetPacket* pPacket = (const NetPacket*)pRawPacket;
    int iRead = (signed char)pPacket->mCommIndex;
    int iLength = pPacket->mMaxIndex;
    if (iLength - iRead < 7)
    {
        return false;
    }

    unsigned char uCommand = pPacket->mData[iRead + 5];
    if (uCommand == ')' || uCommand == 'J')
    {
        if (pPacket->mData[iRead + 6])
        {
            return false;
        }
        unsigned char uKey = pPacket->mData[iRead + 7];
        return uKey == (unsigned char)adminKey[0] ||
               uKey == (unsigned char)adminKey[1];
    }
    return uCommand == 's' && pPacket->mData[iRead + 6] == 0;
}

// FUNCTION: SWBF2 0x005b1fe0
void LoadServerBans()
{
    char line[1024];
    char path[512];
    char destination[36];
    sprintf(path, "%s\\%s.cfg", aSettings, "ServerBans");
    FILE* stream = fopen(path, "rt");
    if (!stream)
    {
        return;
    }

    while (fgets(line, sizeof(line), stream))
    {
        char* pFirst = strchr(line, ',');
        if (!pFirst)
        {
            continue;
        }
        char* pSecond = strchr(pFirst + 1, ',');
        if (!pSecond || !strchr(pSecond + 1, ','))
        {
            continue;
        }
        int iLength = (int)(pSecond - pFirst - 1);
        if (iLength <= 32)
        {
            strncpy(destination, pFirst + 1, iLength);
            destination[iLength] = 0;
        }
    }
    fclose(stream);
}

// FUNCTION: SWBF2 0x005b20e0
void __fastcall LoadServerConfigSection(const char* pcSection)
{
    char line[1024];
    char path[512];
    unsigned short wideLine[1024];
    sprintf(path, "%s\\%s.cfg", aSettings, pcSection);
    FILE* stream = fopen(path, "rt");
    if (!stream)
    {
        return;
    }

    while (fgets(line, sizeof(line), stream))
    {
        char* pEnd;
        while ((pEnd = strchr(line, '\r')) != 0 ||
               (pEnd = strchr(line, '\n')) != 0)
        {
            *pEnd = 0;
        }
        PblUnicode::ustrncnv(wideLine, line, 1024);
        remoteLoggedIn = true;
        NetAdmin::ParseLineRemote(-1, wideLine, false, false);
        remoteLoggedIn = false;
    }
    fclose(stream);
}

// GLOBAL: SWBF2 0x01e579c7
bool detectBandwidth = false;

// GLOBAL: SWBF2 0x01e57a00
bool adminStatusRequested = false;

// GLOBAL: SWBF2 0x01e57a01
bool adminVersionRequested = false;

// GLOBAL: SWBF2 0x01e57a02
bool adminGameNameRequested = false;

// GLOBAL: SWBF2 0x01e57a0c
bool adminRemoteReply = false;

// GLOBAL: SWBF2 0x01e62f13
bool netBandwidthDetecting = false;

// GLOBAL: SWBF2 0x01e62f17
bool netDropLateUpdate = false;

// GLOBAL: SWBF2 0x01e62f4c
int netLostBytes = 0;

// GLOBAL: SWBF2 0x01faaad0
char adminPlayerIdText[32] = { 0 };

// GLOBAL: SWBF2 0x01faaaf0
char adminBanKeyIdText[32] = { 0 };

// GLOBAL: SWBF2 0x01faab10
char adminBanIdText[32] = { 0 };

// GLOBAL: SWBF2 0x01e62e84
int netAutoAnnounce = 0;

// GLOBAL: SWBF2 0x007e7fe8
int netAutoAnnouncePeriod = 60;

// GLOBAL: SWBF2 0x007e8258
int netWindowSize = 2;

// GLOBAL: SWBF2 0x007e8538
int netHostedPlayerLimit = 64;
// GLOBAL: SWBF2 0x01e579f4
FILE* Stream = 0;
// GLOBAL: SWBF2 0x007e8c00
char aNtsc[16] = "NTSC";
// GLOBAL: SWBF2 0x007e8c10
char aSettings[512] = "settings";
// GLOBAL: SWBF2 0x007e8ef4
int hostshort = 3658;
