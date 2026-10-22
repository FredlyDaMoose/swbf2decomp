#include "GameVoiceChatNode.h"
#include <string.h>
#include <io.h>
#include <stdio.h>
#include <winsock2.h>

#include "NetSetup.h"
#include "pcNetSocket.h"
#include "NetMatch.h"
#include "GameLoop.h"
#include "ShellLoop.h"
#include "LuaCallbacks_Options.h"
#include "PebbleFL/Common/PblJournal.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblTime.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "FLGuiManager.h"
#include "FLInputManager.h"
#include "GameState.h"
#include "LuaHelper.h"
#include "PebbleFL/Common/PblValues.h"
#include "MissionPlayList.h"
#include "NetComm.h"
#include "NetGame.h"
#include "NetPacket.h"
#include "Character.h"
#include "GameVoiceChat.h"
#include "GameVoiceChatTransport.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "PlayerProfile.h"
#include "LuaCallbacks_Options.h"
#include "Steam/utf8.h"
#include "Steam/galaxy_api.h"
#include "Team.h"
#include "DownloadableContent.h"
#include <math.h>

extern void sub_42E960();

extern unsigned __int64 netCreatedLobbyId;
extern int netLoginFailureReason;
const char* GetNextMissionForLobby();
const char* GetEncodedLobbyPassword();
void* GetGalaxyMatchmaking();

namespace gf { namespace utils {
const char* ToString(bool b);
const char* ToString(int i);
const char* ToString(unsigned int u);
} }

static void SetLobbyData(const char* pKey, const char* pValue)
{
    void* pMatchmaking = GetGalaxyMatchmaking();
    ((galaxy::api::IMatchmaking*)pMatchmaking)->SetLobbyData(galaxy::api::GalaxyID(netCreatedLobbyId), pKey, pValue);
}

// LIBRARY: SWBF2 0x00411c30
// strchr

bool autoNetEnabled;
bool autoNetLan;
bool autoNetRandomize;
bool g_bNoRender;
unsigned int netCrashAddr;
char autoNetScript[32];
float g_fFixedRate;
unsigned int netPasswordHash;

// GLOBAL: SWBF2 0x01ead474
int NetSetup::mPasswordHash;

// GLOBAL: SWBF2 0x01ead4b8
unsigned char netPasswordSha512[64];

// GLOBAL: SWBF2 0x01ead4f8
unsigned char netEncodedPassword[512];
int netDifficulty;
bool g_bNoWait;
bool g_bNoSound;

 

// GLOBAL: SWBF2 0x007e8e90
int autoNetPlayers = 2;
// GLOBAL: SWBF2 0x007e8e94
int netFPS = 20;

// GLOBAL: SWBF2 0x007e8e98
char autoNetSession[64] = "LY";

// GLOBAL: SWBF2 0x007e8ed8
bool useProfile = true;

// GLOBAL: SWBF2 0x01fa88fc
NetScript netScript;

// GLOBAL: SWBF2 0x01ead924
unsigned int autoNetMissionCount;
// GLOBAL: SWBF2 0x01ead930
stAutoNetMission autoNetMissions[256];

// GLOBAL: SWBF2 0x01ef24b0
// SYNTHETIC: SWBF2 0x004059a0
// `dynamic initializer for 'netRandom''
PblRandom netRandom;

extern float gGameSlice;

// FUNCTION: SWBF2 0x005db5d0
void NetSetup::Create(bool bMission)
{
    unsigned int uiSeed = (unsigned int)PblTime::GetTickCount();
    PblJournal::Record(uiSeed, PJT_FULL);
    netRandom.SetSeed(uiSeed);

    PblJournal::Record(netFPS, PJT_FULL);
    PblJournal::Record(gGameSlice, PJT_FULL);
    PblJournal::Record(autoNetEnabled, PJT_FULL);

    if (autoNetEnabled)
    {
        PblJournal::Record(autoNetLan, PJT_FULL);
        JournalString(autoNetScript, sizeof(autoNetScript));
        JournalString(autoNetSession, sizeof(autoNetSession));
        PblJournal::Record(autoNetPlayers, PJT_FULL);
        useProfile = false;
    }

    if (autoNetEnabled)
    {
        if (strcmp(autoNetScript, "host") == 0)
        {
            netScript = NS_HOST;
        }
        else if (strcmp(autoNetScript, "client") == 0)
        {
            netScript = NS_CLIENT;
        }
        else if (strcmp(autoNetScript, "dedicated") == 0)
        {
            netScript = NS_DEDICATED;
        }
        else
        {
            netScript = NS_UNKNOWN;
        }

        if (netScript == NS_HOST || netScript == NS_DEDICATED)
        {
            PblJournal::Record(autoNetMissionCount, PJT_FULL);
            for (unsigned int i = 0; i < autoNetMissionCount; i++)
            {
                JournalString(autoNetMissions[i].name, sizeof(autoNetMissions[i].name));
            }
        }

        if (bMission)
        {
            autoNetEnabled = false;
            autoNetScript[0] = '\0';
        }
    }
}

class NetPacket;

// GLOBAL: SWBF2 0x007e8ed9
bool useMatchService = true;
// GLOBAL: SWBF2 0x007e8eda
bool gCableConnected = true;

// GLOBAL: SWBF2 0x007e8ee0
int netHostSetupReady = 1;

// GLOBAL: SWBF2 0x01ead4ae
bool netGameError;
// GLOBAL: SWBF2 0x01ead4af
bool netLoginError;
// GLOBAL: SWBF2 0x01ead4b5
bool netWasDedicated;
// GLOBAL: SWBF2 0x01ead4b7
bool netInMultiplayer;
// GLOBAL: SWBF2 0x01ead904
char netLoginNames[1][32];

// GLOBAL: SWBF2 0x01ead8f8
unsigned int intraSessionPlayerId0;

// GLOBAL: SWBF2 0x01ead8fc
unsigned int intraSessionPlayerIdRest[1];
// GLOBAL: SWBF2 0x01ead92a
bool gOverrideCable;
// GLOBAL: SWBF2 0x01ead92b
bool netWasHost;
// GLOBAL: SWBF2 0x01ead92c
bool isInSession;
// GLOBAL: SWBF2 0x01ead92d
bool netWasClient;
// GLOBAL: SWBF2 0x01ead92f
bool netInGame;

// FUNCTION: SWBF2 0x005e7210
unsigned int NetSetup::CalcPassword(char const* password)
{
    return password ? CalcCRC(password, strlen(password), 0) : 0;
}

// FUNCTION: SWBF2 0x005e7270
void NetSetup::SetPassword(char const* password)
{
    if (netInGame && netWasClient)
    {
        return;
    }
    NetSetup::mPasswordHash = CalcPassword(password);
    memset(netPasswordSha512, 0, sizeof(netPasswordSha512));
    memset(netEncodedPassword, 0, sizeof(netEncodedPassword));
    if (password)
    {
        Sha512Hash(reinterpret_cast<const unsigned char*>(password), strlen(password), netPasswordSha512, 0);
        unsigned int length = 0;
        Base64Encode(netEncodedPassword, sizeof(netEncodedPassword), &length, netPasswordSha512, sizeof(netPasswordSha512));
    }
    if (netPasswordHash)
    {
        NetSetup::mPasswordHash = netPasswordHash;
    }
}

// GLOBAL: SWBF2 0x01ead470
int gNetSetup_errState;

// GLOBAL: SWBF2 0x01eaed34
NetLoadGame* loadGame;
// GLOBAL: SWBF2 0x01eaed38
NetJoinGame* joinGame;
// GLOBAL: SWBF2 0x01eaed3c
NetHostGame* hostGame;
// GLOBAL: SWBF2 0x01eaed40
NetFindGame* findGame;
// GLOBAL: SWBF2 0x01eaed44
NetPostLoadHostGame* postLoadHostGame;
// GLOBAL: SWBF2 0x01eaed48
NetPostLoadJoinGame* postLoadJoinGame;

// GLOBAL: SWBF2 0x01ead6f8
IntraSessionPlayer intraSessionPlayers[64];

// GLOBAL: SWBF2 0x01ef1fa0
// SYNTHETIC: SWBF2 0x00405990
// `dynamic initializer for 'netPrevSessionId''
NetSessionId netPrevSessionId;
// GLOBAL: SWBF2 0x01ef1fa8
// SYNTHETIC: SWBF2 0x00405940
// `dynamic initializer for 'clientAddrs''
NetAddr clientAddrs[64];

// GLOBAL: SWBF2 0x01ef24a8
// SYNTHETIC: SWBF2 0x00405980
// `dynamic initializer for 'netSessionId''
NetSessionId netSessionId;

// GLOBAL: SWBF2 0x01ef2510
// SYNTHETIC: SWBF2 0x00405960
// `dynamic initializer for 'hostAddrs''
NetAddr hostAddrs[3];
// GLOBAL: SWBF2 0x01ef2558
// SYNTHETIC: SWBF2 0x00405930
// `dynamic initializer for 'myAddr''
NetAddr myAddr;

// GLOBAL: SWBF2 0x01fa88f0
int hostPlayer;

// GLOBAL: SWBF2 0x01fa88f4
bool isLoginError;
// GLOBAL: SWBF2 0x01fa8900
unsigned __int64 playerChangedMask[64];
// GLOBAL: SWBF2 0x01fa8b00
unsigned int playerChangeTurn[64];
// GLOBAL: SWBF2 0x01fa8c08
unsigned __int64 playerChangedSendMask;

extern int netCurMaxPlayers;
void ReadDestMask(NetPktGroup& group, unsigned __int64& mask);
void ReadString(NetPktGroup& group, char* text, unsigned int capacity);

// FUNCTION: SWBF2 0x005d5b00
void __fastcall ReadAddr(NetPktGroup* pGroup, NetAddr* pAddr)
{
    pGroup->ReadBytes((char*)pAddr, 8);
    unsigned int uValue;
    pGroup->ReadBits(uValue, 1);
    pAddr->hasPriv = uValue != 0;
    if (pAddr->hasPriv)
    {
        pGroup->ReadBits(uValue, 32);
        pAddr->privAddr = uValue;
        pGroup->ReadBits(uValue, 16);
        pAddr->privPort = uValue;
    }
}

// FUNCTION: SWBF2 0x005d5e30
void NetMatch::ChangeRegisterGame()
{
}

// FUNCTION: SWBF2 0x005e39b0
void NetSetup::RemoveGamePlayer(int iPlayer)
{
    if (!NetGame::IsHost() || NetGame::IsClient())
    {
        return;
    }
    const unsigned __int64 mask = 1ULL << iPlayer;
    playerChangedSendMask &= ~mask;
    playerChangeTurn[iPlayer] = 0;
    playerChangedMask[iPlayer] = 0;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        playerChangedMask[i] &= ~mask;
    }
    NetMatch::ChangeRegisterGame();
}

// FUNCTION: SWBF2 0x005e37f0
void NetSetup::PlayerDataChanged(int iChangedPlayer)
{
    if (!NetGame::IsHost() || NetGame::IsClient())
    {
        return;
    }
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (NetComm::IsConnected(i) && NetGame::GetJoystickIndex(i) < 0)
        {
            playerChangedSendMask |= 1ULL << i;
            playerChangeTurn[i] = NetGame::GetTurnNumber();
            playerChangedMask[i] |= 1ULL << iChangedPlayer;
        }
    }
    NetMatch::ChangeRegisterGame();
}

// FUNCTION: SWBF2 0x005e38d0
void NetSetup::AddGamePlayer(int iChangedPlayer)
{
    if (!NetGame::IsHost() || NetGame::IsClient())
    {
        return;
    }
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (NetComm::IsConnected(i) && NetGame::GetJoystickIndex(i) < 0)
        {
            playerChangedSendMask |= 1ULL << i;
            playerChangeTurn[i] = NetGame::GetTurnNumber();
            playerChangedMask[i] |= 1ULL << iChangedPlayer;
        }
    }
    NetMatch::ChangeRegisterGame();
}

// FUNCTION: SWBF2 0x005e3cd0
void NetSetup::ReadGameUpdate(NetPktGroup* pGroup)
{
    if (NetGame::IsHost())
    {
        return;
    }
    NetGame::ReadPreGame(*pGroup);
    bool bHasPlayers = false;
    pGroup->Read(bHasPlayers);
    if (!bHasPlayers)
    {
        return;
    }

    unsigned __int64 mask = 0;
    ReadDestMask(*pGroup, mask);
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (!(mask & (1ULL << i)))
        {
            continue;
        }
        bool bTeam1 = false;
        pGroup->Read(bTeam1);
        NetGame::SetStartTeam(i, bTeam1 ? 1 : 2);
        Character* pCharacter = Character::FindPlayer(i);
        if (pCharacter)
        {
            pCharacter->SelectTeam(NetGame::GetStartTeam(i));
        }

        const int iJoystick = NetGame::GetJoystickIndex(i);
        if (iJoystick >= 0)
        {
            SpawnDisplay* pDisplay = SpawnDisplay::Instance(iJoystick);
            if (pDisplay->mNetWaitingForTeam)
            {
                pDisplay->SelectTeam(NetGame::GetStartTeam(i));
                SpawnDisplay::Instance(iJoystick)->mNetWaitingForTeam = false;
            }
        }
        if (i == NetComm::GetLocalIndex(0))
        {
            continue;
        }

        char acName[32];
        ReadString(*pGroup, acName, sizeof(acName));
        NetGameP::SetPlayerName(i, acName);
        unsigned __int64 uGSID = 0;
        pGroup->ReadBytes((char*)&uGSID, sizeof(uGSID));
        bool bGuest = false;
        pGroup->Read(bGuest);
        NetAddr addr;
        if (bGuest)
        {
            unsigned int uHost = 0;
            pGroup->ReadBits(uHost, 6);
            NetGameP::SetGuestOf(i, uHost);
        }
        else
        {
            ReadAddr(pGroup, &addr);
        }

        if (NetGame::GetPlayerGSID(i) != uGSID)
        {
            NetGame::SetPlayerGSID(i, uGSID);
            NetGame::ClearPlayerCDKey(i);
        }
        if (bGuest)
        {
            const int iHost = NetGame::GetGuestOf(i);
            NetCommP::SetArch(i, NetComm::GetArch(iHost));
            clientAddrs[i] = clientAddrs[iHost];
        }
        else
        {
            unsigned int uArch = 0;
            pGroup->ReadBits(uArch, 2);
            NetCommP::SetArch(i, (NetArch)uArch);
            clientAddrs[i] = addr;
        }
    }
}
// GLOBAL: SWBF2 0x01fa8c10
char netGameType[32];

extern char netGameName[64];

namespace GameLoop { extern bool sQuitRequested; }
namespace GameLoop { extern unsigned int sQuitTurn; }
extern int netCurMaxPlayers;

// FUNCTION: SWBF2 0x005db330
char const* NetSetup::GetAutoNetScript()
{
    return autoNetScript;
}

// FUNCTION: SWBF2 0x005db340
bool NetSetup::IsAutoNetLan()
{
    return autoNetLan;
}

// FUNCTION: SWBF2 0x005db350
char const* NetSetup::GetAutoNetName()
{
    return autoNetSession;
}

// FUNCTION: SWBF2 0x005db360
int NetSetup::GetAutoNetPlayerCount()
{
    return autoNetPlayers;
}

// FUNCTION: SWBF2 0x005db370
static char* GetFactionName(char faction, int team)
{
    static struct
    {
        char faction;
        char* name;
        char enemy;
    } factions[4] = {
        { 'a', "common.sides.all.name", 'i' },
        { 'i', "common.sides.imp.name", 'a' },
        { 'r', "common.sides.rep.name", 'c' },
        { 'c', "common.sides.cis.name", 'r' },
    };

    char side = faction;
    if (team != 1)
    {
        unsigned int i;
        for (i = 0; i < 4; i++)
        {
            if (factions[i].faction == faction)
            {
                side = factions[i].enemy;
                break;
            }
        }
        if (i == 4)
        {
            return 0;
        }
    }

    for (unsigned int i = 0; i < 4; i++)
    {
        if (factions[i].faction == side)
        {
            return factions[i].name;
        }
    }
    return 0;
}

char* NetSetup_GetFactionName(char faction, int team)
{
    return GetFactionName(faction, team);
}

// FUNCTION: SWBF2 0x005db410
void NetSetup::SetupAutoNetMissions()
{
    MissionPlayList::ClearEntries();
    for (unsigned int i = 0; i < autoNetMissionCount; i++)
    {
        char faction = autoNetMissions[i].name[4];
        char* pcTeam1 = GetFactionName(faction, 1);
        char* pcTeam2 = GetFactionName(faction, 2);
        MissionPlayList::AddEntry(autoNetMissions[i].name, 1, pcTeam1, pcTeam2,
                                  autoNetMissions[i].tickets[0], autoNetMissions[i].tickets[1]);
    }
    MissionPlayList::SetLoopForever(true);
    if (autoNetRandomize)
    {
        MissionPlayList::RandomizeOrder();
    }
    MissionPlayList::SelectNextEntry();
}

// FUNCTION: SWBF2 0x005db4e0
void JRR_to_Errorbox(JoinRejectReason eReason)
{
    char const* pcError = 0;
    int iSeverity = 6;

    switch (eReason)
    {
    case JRR_FULL: pcError = "ifs.mp.joinerrors.full"; break;
    case JRR_PASSWORD: pcError = "ifs.mp.joinerrors.password"; break;
    case JRR_GAME: pcError = "ifs.mp.joinerrors.game"; break;
    case JRR_VERSION: pcError = "ifs.mp.joinerrors.version"; break;
    case JRR_BOOTED: pcError = "ifs.mp.joinerrors.booted"; break;
    case JRR_CONNECTION_LOST: pcError = "ifs.mp.joinerrors.connectlost"; break;
    case JRR_MISSING_DC:
        pcError = "ifs.mp.joinerrors.missingdc";
        iSeverity = 12;
        break;
    case JRR_BAD_CDKEY: pcError = "ifs.mp.joinerrors.cdkey"; break;
    case JRR_BAN: pcError = "ifs.mp.joinerrors.banned"; break;
    case 11: pcError = "ifs.mp.joinerrors.doesnotexist"; break;
    case 12: pcError = "ifs.mp.joinerrors.cantconnect"; break;
    default: break;
    }

    if (pcError)
    {
        NetMatch::SetGameError(pcError, iSeverity);
    }
}

// FUNCTION: SWBF2 0x005db850
void NetSetup::BeginShell()
{
}

void NetSetup::EndShell()
{
}

// FUNCTION: SWBF2 0x005db860
void NetSetup::SetAsHost()
{
    netEnabled = true;
    NetComm::SetHost(true);
}

// FUNCTION: SWBF2 0x005db880
void NetSetup::SetAsClient()
{
    netEnabled = true;
    NetComm::SetHost(false);
}

// FUNCTION: SWBF2 0x005db8a0
void NetSetup::SetAsDedicated(bool bDedicated)
{
    netIsDedicated = bDedicated;
}

// FUNCTION: SWBF2 0x005db8b0
void NetSetup::SetGameName(char const* pcName)
{
    strlcpy(netGameName, pcName, 64);
}

// FUNCTION: SWBF2 0x005db8d0
void NetSetup::SetLoginName(int iViewport, char const* pcName)
{
    char const* pcPlatformName = NetMatch::GetLoginOverrideName(iViewport);
    memset(netLoginNames[iViewport], 0, sizeof(netLoginNames[iViewport]));
    if (pcPlatformName)
    {
        strlcpy(netLoginNames[iViewport], pcPlatformName, 32);
    }
    else
    {
        strlcpy(netLoginNames[iViewport], pcName, 32);
    }
}

// FUNCTION: SWBF2 0x005db950
bool NetSetup::InNetGame()
{
    return netInGame;
}

// FUNCTION: SWBF2 0x005db960
bool NetSetup::InMultiplayer()
{
    return netInMultiplayer;
}

// FUNCTION: SWBF2 0x005db970
bool NetSetup::SetInNetGame(bool bInGame)
{
    if (!bInGame)
    {
        netInMultiplayer = false;
    }
    netInGame = bInGame;
    return netInGame;
}

// FUNCTION: SWBF2 0x005db9a0
bool NetSetup::WasHost()
{
    return netWasHost;
}

// FUNCTION: SWBF2 0x005db9b0
bool NetSetup::WasDedicated()
{
    return netWasDedicated;
}

// FUNCTION: SWBF2 0x005db9c0
bool NetSetup::WasClient()
{
    return netWasClient;
}

// FUNCTION: SWBF2 0x005db9d0
bool NetSetup::IsInSession()
{
    return isInSession;
}

// FUNCTION: SWBF2 0x005db9e0
void NetSetup::RequestQuit(unsigned int uiReason)
{
    FLInputManager::Instance()->StopAllRumble();
    if (!GameLoop::sQuitRequested)
    {
        GameLoop::sQuitTurn = uiReason;
        GameLoop::sQuitRequested = true;
    }
    if (NetGame::IsEnabled())
    {
        netInGame = false;
        netWasHost = false;
        netWasClient = false;
    }
}

// FUNCTION: SWBF2 0x005dba30
bool NetSetup::IsLoginError()
{
    return netLoginError;
}

// FUNCTION: SWBF2 0x005dba40
bool NetSetup::IsGameError()
{
    return netGameError;
}

// FUNCTION: SWBF2 0x005dba50
bool NetSetup::IsNetCableConnected()
{
    if (gOverrideCable)
    {
        return gCableConnected;
    }
    return true;
}

// FUNCTION: SWBF2 0x005dba70
void NetSetup::GetLoginError(char const*& pcError, int& iSeverity)
{
    NetMatch::GetLoginError(pcError, iSeverity);
}

// FUNCTION: SWBF2 0x005dba80
void NetSetup::GetGameError(char const*& pcError, int& iSeverity)
{
    NetMatch::GetGameError(pcError, iSeverity);
}

// FUNCTION: SWBF2 0x005dba90
void NetSetup::SetErrorMode(bool bMode)
{
    isLoginError = bMode;
}

// FUNCTION: SWBF2 0x005dbaa0
bool NetSetup::IsErrorDismissed()
{
    char const* pcError;
    int iSeverity;
    GetLoginError(pcError, iSeverity);
    if (!pcError)
    {
        GetGameError(pcError, iSeverity);
    }

    GuiManager* pGui = GuiManager::Instance();
    bool bDismissed = false;
    switch (gNetSetup_errState)
    {
    case 0:
        pGui->OpenError((GuiManager::eErrorSeverity)iSeverity,
                        RedLocalizeDB::Find(PblHash(pcError), false), 0, 0);
        gNetSetup_errState = 1;
        break;
    case 2:
        bDismissed = true;
        break;
    }
    return bDismissed;
}

// FUNCTION: SWBF2 0x005dbb40
void NetSetup::HandleError(bool bArg)
{
    if (isLoginError)
    {
        NetMatch::HandleLoginError(bArg);
        return;
    }

    unsigned int uiState = GameState::GetState();
    if (uiState != 0x11E1FC01)
    {
        if (postLoadHostGame)
        {
            postLoadHostGame->Cancel();
        }
        else if (postLoadJoinGame)
        {
            postLoadJoinGame->Cancel();
        }
        GameState::SetState(0x11E1FC01);
    }
    NetMatch::ClearGameError();
}

// FUNCTION: SWBF2 0x005dbbc0
void NetSetup::EnableOnlineService(bool bEnable)
{
    useMatchService = bEnable;
}

// FUNCTION: SWBF2 0x005dbbd0
char const* NetSetup::GetOnlineService()
{
    return "Galaxy";
}

// FUNCTION: SWBF2 0x005dbbe0
char* NetSetup::GetIPAddr()
{
    return NetMatch::GetIPAddr();
}

// FUNCTION: SWBF2 0x005dbbf0
void NetSetup::GetPrevSessionId(NetSessionId* pId)
{
    if (pId)
    {
        *pId = netPrevSessionId;
    }
}

// FUNCTION: SWBF2 0x005dbc10
void NetSetup::StartLogin(int iViewport, bool bArg2, char const* pcArg3, char const* pcArg4,
                          char const* pcArg5, bool bArg6)
{
    NetMatch::StartLogin(iViewport, bArg2, pcArg3, pcArg4, pcArg5, bArg6);
}

// FUNCTION: SWBF2 0x005dbc40
bool NetSetup::IsLoginDone(int iViewport)
{
    return NetMatch::UpdateLogin(iViewport);
}

// FUNCTION: SWBF2 0x005dbc50
bool NetSetup::IsLoginSuccessful(int iViewport, bool bArg)
{
    return NetMatch::IsLoginSuccessful(iViewport, bArg);
}

// FUNCTION: SWBF2 0x005dbc60
void NetSetup::CancelLogin(int iViewport, bool bArg)
{
    NetMatch::CancelLogin(iViewport, bArg);
}

// FUNCTION: SWBF2 0x005dbc70
static void FindAddress()
{
    myAddr = NetAddr();
    NetMatch::StartFindAddress();
    while (!NetMatch::UpdateFindAddress())
        ;
    NetMatch::EndFindAddress();
}

// FUNCTION: SWBF2 0x005dbcb0
NetHostGame* NetSetup::HostGame()
{
    return new CNetHostGame;
}

// FUNCTION: SWBF2 0x005db240
ShellUpdate::ShellUpdate()
{
    postLoad = false;
    hostLaunched = false;
    connectMask = 0;
    moveNum = 0;
    turnNumber = 0;
    connectingMask = 0;
    quitMask = 0;
    hostLoadDone = false;
    patchFile.Init();
}

// FUNCTION: SWBF2 0x005db2c0
void ShellPlayer::Init()
{
    joinAck = false;
    hostRequest = 0;
    moveNum = 0;
    updateNum = 1;
    ackedUpdate = 0;
    changedUpdate = 0;
    playerId = 0;
    quitRequested = false;
    patchFile.Init(0, 0);
}

// FUNCTION: SWBF2 0x005dbd20
CNetHostGame::CNetHostGame()
{
    FindAddress();

    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        PblValues::Record<bool>(pProfile->mGameOptions.mCockpitCam, (PblValuesType)0);
    }

    netLoginError = false;
    netGameError = false;
    NetComm::AdvanceTime(true);
    netHostSetupReady = 0;

    changeCount = 0;
    timer = 0.0f;
    playerCount = 0;
    launchReady = false;
    launchTimer = 0.0f;
    hostFailed = false;
    bootMask = 0;
    needSendMask = 0;

    NetCommP::GetConnected(64) = true;
    NetCommP::GetLastRecv(64) = NetComm::GetTime();
    NetGameP::GetHostIndex() = 0;
    *NetGameP::GetHostMode(0) = 0;

    if (NetGame::IsDedicated())
    {
        hostPlayer = -1;
        for (unsigned int i = 0; i < 2; i++)
        {
            NetCommP::GetLocalIndex(i) = -1;
        }
        NetComm::SetPing(64, 33);
    }
    else
    {
        FLInputManager* pInput = FLInputManager::Instance();
        int aiJoystick[1];
        unsigned int uiCount = 0;
        unsigned int j;

        if (autoNetEnabled)
        {
            aiJoystick[uiCount++] = 0;
        }
        else
        {
            for (j = 0; j < 1; j++)
            {
                bool bActive = ((unsigned char*)pInput)[0x2e6c + j] != 0;
                if (bActive)
                {
                    aiJoystick[uiCount++] = j;
                }
            }
        }
        for (j = uiCount; j < 1; j++)
        {
            aiJoystick[j] = -1;
        }

        for (j = 0; j < 2; j++)
        {
            int iJoystick = -1;
            if (j < 1)
            {
                iJoystick = aiJoystick[j];
            }

            if (iJoystick >= 0)
            {
                hostPlayer = 0;
                NetCommP::GetConnected(j) = true;
                NetCommP::GetLastRecv(j) = NetComm::GetTime();
                NetCommP::GetLocalIndex(j) = j;
                NetCommP::GetPipe(j) = PIPE_LAN;

                char acName[64];
                PblUnicode::ustrncnv(acName, PlayerProfile::GetProfile(iJoystick)->GetName(), 64);
                NetGameP::SetPlayerName(j, acName);
                if (j == 0)
                {
                    *NetGameP::GetHostIndex(0) = 0;
                }
                *NetGameP::GetClientOfIndex(j) = 0;
                if (j == 0)
                {
                    NetGameP::SetGuestOf(j, -1);
                }
                else
                {
                    NetGameP::SetGuestOf(j, 0);
                }
                if (j > 0)
                {
                    UpdateCurMaxPlayers((unsigned __int64)1 << j, true);
                }
            }
            else
            {
                NetCommP::GetLocalIndex(j) = -1;
            }
        }
        changeCount++;
    }

    shellState.canceled = false;
    for (unsigned int j = 0; j < 64; j++)
    {
        shellState.players[j].Init();
        clientAddrs[j] = NetAddr();
        bootTime[j] = 0.0f;
    }

    strlcpy(netGameType, "duel", sizeof(netGameType));
    NetMatch::StartRegisterGame(useMatchService);
    isInSession = true;
    if (!NetSetup::InNetGame())
    {
        netPrevSessionId.SetNULL();
    }
    NetMatch::GetSessionId(&netSessionId);

    for (unsigned int j = 0; j < 64; j++)
    {
        playerChangeTurn[j] = 0;
        playerChangedMask[j] = 0;
    }
    playerChangedSendMask = 0;

    int iTurnRate = netFPS;
    if (useProfile)
    {
        PlayerProfile::tProfile* pCur = PlayerProfile::GetCurProfile();
        if (pCur)
        {
            iTurnRate = pCur->mOnlineOptions.mu8TurnsPerSecond;
        }
    }
    if (iTurnRate < 10)
    {
        iTurnRate = netFPS;
    }
    NetGameP::SetTurnRate(iTurnRate, 1);

    shellUpdate.patchFile.Init();
    shellUpdate.patchFile.done = true;
}

// FUNCTION: SWBF2 0x005dc240
CNetHostGame::~CNetHostGame()
{
    if (shellState.canceled)
    {
        netInGame = false;
        isInSession = false;
        for (int i = 0; i < netCurMaxPlayers; i++)
        {
            if (NetComm::IsConnected(i))
            {
                NetComm::Disconnect(i);
            }
        }
        if (NetComm::IsConnected(64))
        {
            NetComm::Disconnect(64);
        }
        NetMatch::EndRegisterGame();
        for (int j = 0; j < 64; j++)
        {
            NetGame::SetPlayerGSID(j, 0);
            NetGame::ClearPlayerCDKey(j);
            NetGameP::SetPlayerName(j, "");
        }
        NetGameP::GetHostIndex() = -1;
        netIsDedicated = false;
        NetQOS::CancelGame();
    }
    else
    {
        for (int k = 0; k < netCurMaxPlayers; k++)
        {
            if (NetComm::IsConnected(k))
            {
                intraSessionPlayers[k].playerId = shellState.players[k].playerId;
            }
        }
    }
}

// FUNCTION: SWBF2 0x005dc3b0
void CNetHostGame::Cancel()
{
    if (shellState.canceled)
    {
        return;
    }

    shellState.canceled = true;
    unsigned __int64 uMask = 0;
    for (int i = 0; i < netCurMaxPlayers; i++)
    {
        if (NetComm::IsConnected(i) && shellState.players[i].joinAck)
        {
            uMask |= (unsigned __int64)1 << i;
        }
    }
    needSendMask = uMask;
}

// FUNCTION: SWBF2 0x005dc450
bool CNetHostGame::IsCancelled()
{
    if (!shellState.canceled)
    {
        return false;
    }

    bool bCancelled = true;
    for (int i = 0; i < netCurMaxPlayers; i++)
    {
        if (NetComm::IsConnected(i) && NetGame::GetJoystickIndex(i) < 0 &&
            !shellState.players[i].quitRequested)
        {
            bCancelled = false;
            break;
        }
    }
    return bCancelled;
}

// FUNCTION: SWBF2 0x005dc4d0
bool BanIP(int iPlayer, NetPacket* pPacket)
{
    return false;
}

// FUNCTION: SWBF2 0x005e75f0
void SendStateDst::Init()
{
    done = false;
    gotSome = false;
    baseBlock = 0;
    hasBlocks = 0;
    fd = -1;
}

// FUNCTION: SWBF2 0x005e7460
void SendStateSrc::Init(int iCount, char** ppcFiles)
{
    fileCount = iCount;
    fileNum = 0;
    if (!iCount)
    {
        files = 0;
        return;
    }

    int iSize = iCount * 4;
    for (int i = 0; i < iCount; i++)
    {
        iSize += strlen(ppcFiles[i]) + 1;
    }

    files = (char*)RedAllocFromHeap(__RedCurrHeap, iSize, 0);
    if (!files)
    {
        return;
    }

    char** ppcTable = (char**)files;
    char* pcName = (char*)(ppcTable + iCount);
    for (int i = 0; i < iCount; i++)
    {
        ppcTable[i] = pcName;
        strlcpy(ppcTable[i], ppcFiles[i], iSize - (pcName - files));
        pcName += strlen(pcName) + 1;
    }
    baseBlock = 0;
    hasBlocks = 0;
    curFile = 0;
    blockCycle = 0;
}

// GLOBAL: SWBF2 0x007e8edc
int autoNetLast = 1;

// GLOBAL: SWBF2 0x01ead4b0
int autoNetCounter;

// GLOBAL: SWBF2 0x01ead4b6
bool autoNetFinishedShell;

// GLOBAL: SWBF2 0x01ead928
bool autoNetTimeInit;
// GLOBAL: SWBF2 0x01ead92e
bool NetSetup::mNoticeNoCable;

// GLOBAL: SWBF2 0x01fa88f8
int autoNetRepeat;

extern PblTimeBase::Stamp autoNetTimeStamp;
extern int netMinPlayersToAutoLaunchDedicated;
extern int netNumBots;

namespace StatsManager { void Update(); }

void AddAutoNetMission(char const* pcArg, int iA, int iB);

// FUNCTION: SWBF2 0x005e4030
void AddAutoNetMission(char const* pcArg, int iA, int iB)
{
    if (autoNetMissionCount < 256)
    {
        stAutoNetMission& mission = autoNetMissions[autoNetMissionCount++];
        strlcpy(mission.name, pcArg, sizeof(mission.name));
        mission.tickets[0] = iA;
        mission.tickets[1] = iB;
    }
}

// FUNCTION: SWBF2 0x005e3fb0
void NetSetupP::Update()
{
    char cMark1 = 'Z';
    PblJournal::Record(cMark1, PJT_FULL);
    NetMatch::UpdateLogin(-1);
    char cMark2 = 'Z';
    PblJournal::Record(cMark2, PJT_FULL);
    NetMatch::UpdatePresence(false);
    char cMark3 = 'Z';
    PblJournal::Record(cMark3, PJT_FULL);
    NetMatch::UpdateRegisterGame();
    char cMark4 = 'Z';
    PblJournal::Record(cMark4, PJT_FULL);
    StatsManager::Update();
    char cMark5 = 'Z';
    PblJournal::Record(cMark5, PJT_FULL);
}

static void AutoNetQueueMissions()
{
    MissionPlayList::ClearEntries();
    for (unsigned int i = 0; i < autoNetMissionCount; i++)
    {
        MissionPlayList::AddEntry(autoNetMissions[i].name, autoNetMissions[i].name[4], 0, 0,
                                  autoNetMissions[i].tickets[0], autoNetMissions[i].tickets[1]);
    }
    MissionPlayList::SelectNextEntry();
    autoNetRepeat = -1;
}

// FUNCTION: SWBF2 0x005e41e0
void NetSetupP::UpdateScreen()
{
    netRandom.GetInt();

    bool bInShell = NetGame::InShell();
    if (bInShell)
    {
        NetComm::AdvanceTime(true);
        Update();
    }

    if (NetSetup::mNoticeNoCable && !NetSetup::IsNetCableConnected())
    {
        if (bInShell)
        {
            NetMatch::SetNoCableError();
        }
        else
        {
            NetSetup::RequestQuit(NetGame::GetTurnNumber());
            ShellLoop::SetNetCableErrorOnEnter(GuiManager::ERROR_LOGOFF_CABLE, NetComm::IsLAN() ? 0 : "common.mp.errors.nocable");
        }
    }

    if (!autoNetEnabled)
    {
        return;
    }

    if (!autoNetFinishedShell && !bInShell)
    {
        autoNetFinishedShell = true;
        autoNetTimeInit = false;
    }

    char const* pcScreen = GuiManager::Instance()->GetCurrentScreen();
    if (strcmp(pcScreen, "ifs_mp_autonet") == 0)
    {
        netMinPlayersToAutoLaunchDedicated = autoNetPlayers;
        AutoNetQueueMissions();
    }

    if (autoNetFinishedShell)
    {
        return;
    }

    if (!autoNetTimeInit)
    {
        autoNetTimeInit = true;
        autoNetTimeStamp.Mark(GameState::GetSystemTimeBase());
    }
    if (autoNetTimeStamp.GetElapsed() < 0.1f)
    {
        return;
    }
    autoNetTimeStamp.Mark(GameState::GetSystemTimeBase());

    if (GuiManager::Instance()->IsPopupOpen())
    {
        char const* pcPopup = GuiManager::Instance()->GetCurrentFocus();
        if (strcmp(pcPopup, "Popup_LobbyOpts") == 0)
        {
            if (++autoNetCounter == 10)
            {
                LuaHelper::CallTableProcWThis(pcPopup, "Input_Accept");
                autoNetCounter = 0;
            }
        }
        else if (strcmp(pcScreen, "ifs_mpgs_login") == 0)
        {
            if (++autoNetCounter == 10)
            {
                LuaHelper::CallTableProcWThis(pcPopup, "Input_GeneralRight");
            }
            else if (autoNetCounter == 20)
            {
                LuaHelper::CallTableProcWThis(pcPopup, "Input_Accept");
                autoNetCounter = 0;
            }
        }
        return;
    }

    autoNetCounter++;
    if (strcmp(pcScreen, "ifs_legal") == 0)
    {
        autoNetCounter = 0;
    }
    else if (strcmp(pcScreen, "ifs_start") == 0)
    {
        if (autoNetCounter > 10)
        {
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Start");
            autoNetCounter = 0;
        }
    }
    else if (strcmp(pcScreen, "ifs_login") == 0)
    {
        autoNetLast = 1;
        if (autoNetCounter == 30)
        {
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Accept");
            autoNetCounter = 0;
        }
    }
    else if (strcmp(pcScreen, "ifs_main") == 0)
    {
        if (autoNetCounter == 10)
        {
            LuaHelper::CallTableProcWThis(pcScreen, "Input_GeneralDown");
        }
        else if (autoNetCounter == 20)
        {
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Accept");
            autoNetCounter = 0;
        }
    }
    else if (strcmp(pcScreen, "ifs_mpxl_login") == 0)
    {
        if (autoNetCounter == 10)
        {
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Accept");
            autoNetCounter = 0;
        }
    }
    else if (strcmp(pcScreen, "ifs_mp") == 0)
    {
        if (autoNetCounter == 10)
        {
            if (!autoNetLan)
            {
                LuaHelper::CallTableProcWThis(pcScreen, "Input_GeneralDown");
            }
            if (netScript == NS_DEDICATED)
            {
                netMinPlayersToAutoLaunchDedicated = autoNetPlayers;
            }
        }
        else if (autoNetCounter == 20)
        {
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Accept");
            autoNetCounter = 0;
        }
    }
    else if (strcmp(pcScreen, "ifs_mp_main") == 0)
    {
        autoNetLast = 2;
        if (autoNetCounter == 10)
        {
            if (netScript == NS_DEDICATED || netScript == NS_HOST)
            {
                LuaHelper::CallTableProcWThis(pcScreen, "Input_GeneralDown");
            }
        }
        else if (autoNetCounter == 20)
        {
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Accept");
            autoNetCounter = 0;
        }
    }
    else if (strcmp(pcScreen, "ifs_vkeyboard") == 0)
    {
        if (autoNetCounter == 20)
        {
            unsigned short usName[64];
            PblUnicode::ustrncnv(usName, "<NULL>", 64);
            if (autoNetLast == 1)
            {
                PblUnicode::ustrncnv(usName, "AUTONET", 64);
            }
            else if (autoNetLast == 2)
            {
                PblUnicode::ustrncnv(usName, "LY", 64);
            }
            LuaHelper::CallTableProcWThis(pcScreen, "AcceptString", usName);
        }
        else if (autoNetCounter == 40)
        {
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Accept");
            autoNetCounter = 0;
        }
    }
    else if (strcmp(pcScreen, "ifs_mp_createopts") == 0 || strcmp(pcScreen, "ifs_mp_createoptsds") == 0)
    {
        if (autoNetCounter == 100)
        {
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Accept");
            autoNetCounter = 0;
        }
        else if (autoNetCounter == 10)
        {
            netNumBots = 32;
        }
    }
    else if (strcmp(pcScreen, "ifs_missionselect") == 0)
    {
        switch (autoNetCounter)
        {
        case 10:
        case 20:
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Accept");
            break;
        case 30:
            LuaHelper::CallTableProcWThis(pcScreen, "Input_GeneralLeft");
            break;
        case 40:
            LuaHelper::CallTableProcWThis(pcScreen, "Input_GeneralUp");
            break;
        case 50:
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Accept");
            autoNetCounter = 0;
            break;
        }
    }
    else if (strcmp(pcScreen, "ifs_mp_maptype") == 0)
    {
        if (autoNetCounter == 10)
        {
            LuaHelper::CallTableProcWThis(pcScreen, "Input_GeneralDown");
        }
        else if (autoNetCounter == 20)
        {
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Accept");
            autoNetCounter = 0;
        }
    }
    else if (strcmp(pcScreen, "ifs_mp_gameopts") == 0)
    {
        switch (autoNetCounter)
        {
        case 10:
            if (!autoNetMissionCount)
            {
                AddAutoNetMission("hot1i", 250, 250);
            }
            AutoNetQueueMissions();
            break;
        case 20:
            if (netScript == NS_DEDICATED)
            {
                LuaHelper::CallTableProcWThis(pcScreen, "Input_GeneralDown");
                if (autoNetRepeat == -1)
                {
                    autoNetRepeat = 7;
                }
                if (autoNetRepeat-- > 0)
                {
                    autoNetCounter = 11;
                }
            }
            break;
        case 30:
            if (netScript == NS_DEDICATED)
            {
                LuaHelper::CallTableProcWThis(pcScreen, "Input_GeneralRight");
            }
            break;
        case 50:
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Accept");
            autoNetCounter = 0;
            break;
        }
    }
    else if (strcmp(pcScreen, "ifs_mp_lobby") == 0 || strcmp(pcScreen, "ifs_lobby") == 0)
    {
        if (netScript == NS_HOST && autoNetCounter > 10 && hostGame &&
            hostGame->GetPlayerCount() >= autoNetPlayers && NetQOS::IsDone())
        {
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Accept");
            autoNetCounter = 0;
        }
    }
    else if (strcmp(pcScreen, "ifs_mp_lobbyds") == 0)
    {
        if (autoNetCounter == 10)
        {
            netMinPlayersToAutoLaunchDedicated = autoNetPlayers;
        }
    }
    else if (strcmp(pcScreen, "ifs_mp_sessionlist") == 0)
    {
        if (autoNetCounter > 500)
        {
            autoNetCounter = 90;
            LuaHelper::CallTableProcWThis(pcScreen, "Input_Back");
        }
        if (autoNetCounter % 10 == 0)
        {

            int iTop = lua_gettop(LuaHelper::mState);
            (void)iTop;

            LuaHelper::GetGlobal("mpsessionlist_listbox_layout");
            lua_pushstring(LuaHelper::mState, "SelectedIdx");
            lua_rawget(LuaHelper::mState, -2);
            int iSelected = (int)luaL_checknumber(LuaHelper::mState, -1);
            lua_pop(LuaHelper::mState, 2);

            LuaHelper::GetGlobal("mpsessionlist_listbox_contents");
            int iCount = luaL_getn(LuaHelper::mState, -1);
            lua_pop(LuaHelper::mState, 1);

            bool bFound = false;
            if (iSelected <= iCount)
            {
                LuaHelper::GetGlobal("mpsessionlist_listbox_contents");
                lua_rawgeti(LuaHelper::mState, -1, iSelected);
                lua_pushstring(LuaHelper::mState, "namestr");
                lua_rawget(LuaHelper::mState, -2);
                char const* pcName = luaL_checkstring(LuaHelper::mState, -1);
                bFound = _stricmp(pcName, "LY") == 0;
                lua_pop(LuaHelper::mState, 3);
            }

            if (bFound)
            {
                LuaHelper::CallTableProcWThis(pcScreen, "Input_Accept");
                autoNetCounter = 0;
            }
            else if (iSelected == iCount)
            {
                for (; iSelected > 1; iSelected--)
                {
                    LuaHelper::CallTableProcWThis(pcScreen, "Input_GeneralUp");
                }
            }
            else
            {
                LuaHelper::CallTableProcWThis(pcScreen, "Input_GeneralDown");
            }
        }
    }
}

// FUNCTION: SWBF2 0x005d5e40
void __fastcall NetMatch::ReceiveRegisterPacket(const char*  , int  , const void*  )
{
}

// FUNCTION: SWBF2 0x005e3f70
void NetSetupP::Recv(NetPacket* pPacket, const void* pSource)
{
    NetMatch::ReceiveRegisterPacket(pPacket->mData + pPacket->mCommIndex, pPacket->mMaxIndex - pPacket->mCommIndex, pSource);
}

// FUNCTION: SWBF2 0x0052e130
bool __fastcall IsGalaxyIdValid(const unsigned __int64* pThis)
{
    return *pThis != 0;
}

// FUNCTION: SWBF2 0x0052e470
unsigned int* __fastcall SetGalaxyIdWords(unsigned int* pThis, void*, unsigned int lo, unsigned int hi)
{
    pThis[0] = lo;
    pThis[1] = hi;
    return pThis;
}

// FUNCTION: SWBF2 0x0052e490
unsigned int* __fastcall CopyGalaxyIdWords(unsigned int* pThis, void*, const unsigned int* pOther)
{
    pThis[0] = pOther[0];
    pThis[1] = pOther[1];
    return pThis;
}

// FUNCTION: SWBF2 0x0052e4b0
bool __fastcall EqualGalaxyIdWords(const unsigned int* pThis, void*, const unsigned int* pOther)
{
    return pThis[0] == pOther[0] && pThis[1] == pOther[1];
}

extern unsigned __int64 netEnteredLobbyId;
extern unsigned __int64 netCreatedLobbyId;

// FUNCTION: SWBF2 0x005d5dc0
bool HasEnteredLobby()
{
    return netEnteredLobbyId != 0;
}

// FUNCTION: SWBF2 0x005d7760
bool NetMatch::UpdateFindAddress()
{
    return true;
}

// FUNCTION: SWBF2 0x005d7820
int __fastcall NetMatch::SetAddr(unsigned int* pOut, const unsigned int* pIn)
{
    pOut[0] = pIn[0];
    pOut[1] = pIn[1];
    return pIn[1];
}

// FUNCTION: SWBF2 0x005daf40
bool NetQOS::IsDone()
{
    return true;
}

// FUNCTION: SWBF2 0x005e7430
bool SelectNextHostMode(int)
{
    return false;
}

// FUNCTION: SWBF2 0x005e7440
bool SelectNextHost(int)
{
    return false;
}

// FUNCTION: SWBF2 0x005dd170
int CNetHostGame::GetChangeCount()
{
    return changeCount;
}

// FUNCTION: SWBF2 0x005dd190
int CNetHostGame::GetPlayerCount()
{
    return playerCount;
}

// FUNCTION: SWBF2 0x005dd1b0
void CNetHostGame::ForceWriteShellMove()
{
    timer = NetComm::GetTime() - 0.001f;
}

// FUNCTION: SWBF2 0x005dd1d0
void CNetHostGame::ForceWriteShellUpdate()
{
    unsigned __int64 mask = 0;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (NetComm::IsConnected(i))
        {
            mask |= 1ULL << i;
        }
    }
    needSendMask = mask;
}

// FUNCTION: SWBF2 0x005dd250
int CNetHostGame::GetPlayerIndex(int iIndex)
{
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (NetComm::IsConnected(i) && shellState.players[i].joinAck && iIndex-- == 0)
        {
            return i;
        }
    }
    return -1;
}

// GLOBAL: SWBF2 0x01faab58
char hostPlayerNameBuffer[64];

// FUNCTION: SWBF2 0x005dd2c0
const char* CNetHostGame::GetPlayerName(int iPlayer)
{
    if (iPlayer < 0)
    {
        return 0;
    }
    strlcpy(hostPlayerNameBuffer, NetGame::GetPlayerName(iPlayer), sizeof(hostPlayerNameBuffer));
    return hostPlayerNameBuffer;
}

// FUNCTION: SWBF2 0x005dd300
int CNetHostGame::GetStartTeam(int iPlayer)
{
    return iPlayer >= 0 ? NetGame::GetStartTeam(iPlayer) : 0;
}

// FUNCTION: SWBF2 0x005dd500
bool CNetHostGame::IsFailed()
{
    return hostFailed;
}

extern NetDest netDests[MAX_DESTS];
extern int netPlayerLimit;
extern int netLobbyState;
extern char const* netLoginErrorMessage;
extern int netLoginErrorLevel;
extern int netNoCableCount;
extern char const* gameErrorMsg;
extern int gameErrorLvl;

// FUNCTION: SWBF2 0x005d65c0
void NetMatch::HandleLoginError(bool)
{
    netLoginErrorMessage = 0;
    netLoginErrorLevel = 0;
    netNoCableCount = 0;
}

// FUNCTION: SWBF2 0x005d7620
NetAddr* NetAddr::CopyFieldsFrom(const NetAddr* pOther)
{
    this->pubAddr = pOther->pubAddr;
    this->pubPort = pOther->pubPort;
    this->hasPriv = pOther->hasPriv;
    this->privAddr = pOther->privAddr;
    this->privPort = pOther->privPort;
    return this;
}

// FUNCTION: SWBF2 0x005e7200
int NetSetup::GetMaxPlayers()
{
    return netPlayerLimit;
}

extern int netLocalIndex[2];
extern int netDirectPort;

// FUNCTION: SWBF2 0x005e71b0
int NetSetup::GetNumPlayers()
{
    int count = 0;
    for (int iDest = 0; iDest < 64; ++iDest)
    {
        if (NetComm::IsConnected(iDest))
        {
            ++count;
        }
    }
    return count;
}

// FUNCTION: SWBF2 0x005d5ba0
void __fastcall WriteSessionId(NetPktGroup* pGroup, const unsigned int* pWords)
{
    unsigned int words[2] = { pWords[0], pWords[1] };
    pGroup->WriteBytes((const char*)words, sizeof(words));
}

// FUNCTION: SWBF2 0x005e6fb0
void WriteJoinReject(NetPktGroup& group, JoinRejectReason eReason)
{
    group.WriteBits(eReason, 4);
}

// FUNCTION: SWBF2 0x00591400
unsigned int* __fastcall CopyNetSessionIdWords(unsigned int* pThis, void*, const unsigned int* pOther)
{
    pThis[0] = pOther[0];
    pThis[1] = pOther[1];
    return pThis;
}

// FUNCTION: SWBF2 0x005e71a0
char* NetSetup::GetGameType()
{
    return netGameType;
}

// FUNCTION: SWBF2 0x005e7360
void NetSetup::GetSpecialJoinAddress(NetSessionId* pOut)
{
    if (pOut)
    {
        *pOut = mSpecialJoinAddress;
    }
}

// FUNCTION: SWBF2 0x005e7380
void NetSetup::SetSpecialJoinValid(bool bValid)
{
    mSpecialJoinValid = bValid;
}

// FUNCTION: SWBF2 0x005e73a0
NetAddr& NetSetupP::GetMyAddr()
{
    return myAddr;
}

// FUNCTION: SWBF2 0x005e7450
int NetGetHostPlayer()
{
    return hostPlayer;
}

extern unsigned char sStatsMatchData[0x40];

namespace StatsManager { void BeginMatch(); }

// FUNCTION: SWBF2 0x00530860
void StatsManager::BeginMatch()
{
    strlcpy((char*)sStatsMatchData, GameLoop::mMissionScript, sizeof(sStatsMatchData));
}

// FUNCTION: SWBF2 0x005e6fd0
void ReadJoinReject(NetPktGroup& group, JoinRejectReason& eReason)
{
    unsigned int reason;
    group.ReadBits(reason, 4);
    eReason = (JoinRejectReason)reason;
    JRR_to_Errorbox(eReason);
}

// FUNCTION: SWBF2 0x005d5aa0
void __fastcall WriteAddr(NetPktGroup* pGroup, const NetAddr* pAddr)
{
    unsigned int publicWords[2] = { (unsigned int)pAddr->pubAddr, (unsigned int)pAddr->pubPort };
    pGroup->WriteBytes((const char*)publicWords, sizeof(publicWords));
    pGroup->WriteBits(pAddr->hasPriv ? 1 : 0, 1);
    if (pAddr->hasPriv)
    {
        pGroup->WriteBits((unsigned int)pAddr->privAddr, 32);
        pGroup->WriteBits((unsigned int)pAddr->privPort, 16);
    }
}

// FUNCTION: SWBF2 0x005d4ab0
hostent* getlocalhost()
{
    char name[256];
    memset(name, 0, sizeof(name));
    gethostname(name, sizeof(name));
    return gethostbyname(name);
}

extern unsigned int netDirectSocket;

// FUNCTION: SWBF2 0x005e7630
void SendStateDst::Cancel()
{
    if (fd >= 0)
    {
        _close(fd);
        fd = -1;
        remove("sendfile.dat");
    }
}

extern int savedHeroRuleUnlock, savedHeroRuleUnlockVal, savedHeroRuleTeam;
extern int savedHeroRulePlayer, savedHeroRuleRespawn, savedHeroRuleRespawnVal, savedPlayerLimit;
extern int savedDedicatedAutoLaunchMinPlayers, savedHostVoteThreshold, savedPreGameTimeMax;
extern bool savedTeamDamage, savedAutoAim, savedShowNames, savedAutoAssignTeams;
extern bool savedAutoAssignTeamsOriginal, savedAwardsEnabled;
extern int netHeroRuleUnlock, netHeroRuleUnlockVal, netHeroRuleTeam;
extern int netHeroRulePlayer, netHeroRuleRespawn, netHeroRuleRespawnVal, hostVoteThreshold;
extern bool netTeamDamage, netAutoAim, netShowNames, netAutoAssignTeams;
extern bool netAutoAssignTeamsOriginalSetting, netAwardsEnabled;
extern int netMinPlayersToAutoLaunchDedicated, netPreGameTimeMax;

// FUNCTION: SWBF2 0x005d32c0
char SaveSessionRuleDefaults()
{
    savedHeroRuleUnlock = netHeroRuleUnlock;
    savedHeroRuleUnlockVal = netHeroRuleUnlockVal;
    savedHeroRuleTeam = netHeroRuleTeam;
    savedHeroRulePlayer = netHeroRulePlayer;
    savedHeroRuleRespawn = netHeroRuleRespawn;
    savedHeroRuleRespawnVal = netHeroRuleRespawnVal;
    savedPlayerLimit = netPlayerLimit;
    savedTeamDamage = netTeamDamage;
    savedAutoAim = netAutoAim;
    savedShowNames = netShowNames;
    savedAutoAssignTeams = netAutoAssignTeams;
    savedAutoAssignTeamsOriginal = netAutoAssignTeamsOriginalSetting;
    savedDedicatedAutoLaunchMinPlayers = netMinPlayersToAutoLaunchDedicated;
    savedHostVoteThreshold = hostVoteThreshold;
    savedPreGameTimeMax = netPreGameTimeMax;
    savedAwardsEnabled = netAwardsEnabled;
    return savedAwardsEnabled;
}

extern unsigned __int64 netLocalGSID;

namespace gf { bool isInitialized(); }

// FUNCTION: SWBF2 0x00437590
void LuaHelper::PushString(const char* pString)
{
    lua_pushstring(LuaHelper::mState, pString);
}

// FUNCTION: SWBF2 0x0057dc20
int LuaHelper::GetTop()
{
    return lua_gettop(LuaHelper::mState);
}

// FUNCTION: SWBF2 0x0057dc50
int LuaHelper::TableGetN(int iStack)
{
    return luaL_getn(LuaHelper::mState, iStack);
}

// FUNCTION: SWBF2 0x0057dc60
void LuaHelper::TableGetByPosition(int iStack, int iIndex)
{
    lua_rawgeti(LuaHelper::mState, iStack, iIndex);
}

// FUNCTION: SWBF2 0x005808a0
void LuaHelper::RawGetFromTable(int iStack)
{
    lua_rawget(LuaHelper::mState, iStack);
}

// FUNCTION: SWBF2 0x005e7bc0
void WriteFilesAck(NetPktGroup& group, SendStateDst& state)
{
    group.Write(state.done);
    if (state.done)
    {
        return;
    }
    group.WriteBits(state.baseBlock, 24);
    group.WriteBits(state.hasBlocks, 32);
    unsigned int mask = state.hasBlocks;
    int bitCount = 0;
    while (mask)
    {
        mask >>= 1;
        ++bitCount;
    }
}

// FUNCTION: SWBF2 0x005e7c30
void ReadFilesAck(NetPktGroup& group, SendStateSrc& state)
{
    bool done;
    group.Read(done);
    if (done)
    {
        return;
    }
    unsigned int position, mask;
    group.ReadBits(position, 24);
    group.ReadBits(mask, 32);
    while (state.baseBlock < (int)position)
    {
        state.hasBlocks >>= 1;
        ++state.baseBlock;
        if (--state.blockCycle < 0)
        {
            state.blockCycle = 0;
        }
    }
    int iBit = 0;
    while (mask)
    {
        if (mask & 1)
        {
            unsigned int shift = (iBit + position - state.baseBlock) & 31;
            state.hasBlocks |= 1u << shift;
            while (state.hasBlocks & 1)
            {
                state.hasBlocks >>= 1;
                ++state.baseBlock;
                if (--state.blockCycle < 0)
                {
                    state.blockCycle = 0;
                }
            }
        }
        mask >>= 1;
        ++iBit;
    }
}

void WriteDestMask(NetPktGroup& group, unsigned __int64 mask);
namespace Lua_Callbacks { extern int gHuntShowScoreLimit; }

// FUNCTION: SWBF2 0x005e7000
void WriteLoad(NetPktGroup& group, int, unsigned __int64 firstMask,
               unsigned __int64 secondMask)
{
    group.WriteBits(netCurMaxPlayers, 7);
    WriteDestMask(group, firstMask);
    WriteDestMask(group, secondMask);
    group.WriteBits(NetGame::GetTurnNumber(), 32);
    group.Write(Lua_Callbacks::gHuntScoreLimit[Lua_Callbacks::gCurRules]);
    group.Write(Lua_Callbacks::gHuntShowScoreLimit);
    group.Write(Lua_Callbacks::gSpaceAssaultScoreLimit[Lua_Callbacks::gCurRules]);
}

// FUNCTION: SWBF2 0x005e70a0
void ReadLoad(NetPktGroup& group, bool& bReady)
{
    unsigned int maxPlayers = 0;
    group.ReadBits(maxPlayers, 7);
    netCurMaxPlayers = maxPlayers;
    unsigned __int64 firstMask = 0, secondMask = 0;
    ReadDestMask(group, firstMask);
    ReadDestMask(group, secondMask);
    if (!(firstMask & ~secondMask))
    {
        bReady = true;
    }
    unsigned int turn = 0;
    group.ReadBits(turn, 32);
    group.Read(Lua_Callbacks::gHuntScoreLimit[Lua_Callbacks::gCurRules]);
    group.Read(Lua_Callbacks::gHuntShowScoreLimit);
    group.Read(Lua_Callbacks::gSpaceAssaultScoreLimit[Lua_Callbacks::gCurRules]);
    if (bReady)
    {
        NetGameP::SetClientTurnNumber(turn + 4);
    }
}

// FUNCTION: SWBF2 0x005b2ca0
void __fastcall NetCommP::Connect(int iDest, const void* pAddress)
{
    NetDest& dest = netDests[iDest];
    memcpy(dest.mAddressStorage, pAddress, sizeof(dest.mAddressStorage));
    dest.connected = true;
    dest.lastRecv = NetComm::GetTime();
}

extern NetPktGroup inQueue;

// FUNCTION: SWBF2 0x005d5f60
void NetMatch::PauseFindGames() {}

// FUNCTION: SWBF2 0x005d7750
void NetMatch::EndFindAddress() {}

// FUNCTION: SWBF2 0x005d7810
void NetMatch::ClientGetAddr() {}

// FUNCTION: SWBF2 0x005daf20
void NetQOS::CancelGame() {}

// FUNCTION: SWBF2 0x005e54a0
void SetupSleep() {}

// FUNCTION: SWBF2 0x005e09f0
int CNetJoinGame::GetChangeCount()
{
    return changeCount;
}

// FUNCTION: SWBF2 0x005e0a10
int CNetJoinGame::GetPlayerCount()
{
    int count = 0;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (shellUpdate.connectMask & (1ULL << i))
        {
            ++count;
        }
    }
    return count;
}

// FUNCTION: SWBF2 0x005e0a80
int CNetJoinGame::GetPlayerIndex(int iIndex)
{
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if ((shellUpdate.connectMask & (1ULL << i)) && !iIndex--)
        {
            return i;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x005e0b40
char const* CNetJoinGame::GetEra()
{
    return gameEra;
}

// FUNCTION: SWBF2 0x005e0b60
bool CNetJoinGame::IsFailed()
{
    return joinFailed;
}

// FUNCTION: SWBF2 0x005e0940
bool CNetJoinGame::IsLaunchReady()
{
    return hostLaunched && shellUpdate.patchFile.done;
}

// FUNCTION: SWBF2 0x005e0980
bool CNetJoinGame::IsHostPlaying()
{
    return shellUpdate.hostLoadDone || shellUpdate.turnNumber > 0;
}

// FUNCTION: SWBF2 0x005e09c0
void CNetJoinGame::Launch()
{
    if (!clientLaunched)
    {
        clientLaunched = true;
        timer = NetComm::GetTime();
    }
}

// FUNCTION: SWBF2 0x005e0b80
bool CNetJoinGame::IsConnected()
{
    return connected && NetComm::GetLocalIndex(0) >= 0;
}

// FUNCTION: SWBF2 0x005e0920
void CNetJoinGame::ForceWriteShellMove()
{
    timer = NetComm::GetTime() - 0.001f;
}

// FUNCTION: SWBF2 0x005e0df0
bool CNetLaunchGame::IsDone()
{
    return launchDone;
}

// FUNCTION: SWBF2 0x005e1d30
bool CNetLoadGame::HostKnowsILoaded()
{
    return hostKnowsILoaded;
}

// FUNCTION: SWBF2 0x005e22d0
bool CNetLoadGame::IsDone()
{
    return loadDone;
}

// FUNCTION: SWBF2 0x005ddc40
int CNetFindGame::GetSelectedSession()
{
    return mSelectedSession;
}

// FUNCTION: SWBF2 0x005ddc60
void CNetFindGame::Cancel()
{
    canceled = true;
}

// FUNCTION: SWBF2 0x005def30
FindType CNetFindGame::GetFindType()
{
    return findType;
}

// FUNCTION: SWBF2 0x005df440
void CNetFindGame::SetDirty(bool bDirty)
{
    mDirty = bDirty;
}

// FUNCTION: SWBF2 0x005df460
bool CNetFindGame::GetDirty()
{
    return mDirty;
}

// FUNCTION: SWBF2 0x005df6c0
void CNetFindGame::SetFavorite(bool) {}

// FUNCTION: SWBF2 0x005df4b0
void CNetFindGame::SetNumVisible(int iNum)
{
    mNumVisible = iNum;
}

// FUNCTION: SWBF2 0x005df4d0
int CNetFindGame::GetNumVisible()
{
    return mNumVisible;
}

// FUNCTION: SWBF2 0x005df4f0
int CNetFindGame::GetPercentDone()
{
    return mPercentDone;
}

// FUNCTION: SWBF2 0x005df080
int CNetFindGame::GetGameCount()
{
    int count = 0;
    for (PblList<NetGameDesc>::Node* node = gameList._head._pNext;
         node->_pObject; node = node->_pNext)
    {
        if (node->_pObject->mValid)
        {
            ++count;
        }
    }
    return count;
}

// FUNCTION: SWBF2 0x005df0e0
void CNetFindGame::SelectFirst()
{
    if (gameList._iCount <= 0)
    {
        curGame._pNode = &gameList._head;
        return;
    }
    curGame._pNode = gameList._head._pNext;
    while (curGame._pNode->_pObject && !curGame._pNode->_pObject->mValid)
    {
        curGame._pNode = curGame._pNode->_pNext;
    }
}

// FUNCTION: SWBF2 0x005df190
void CNetFindGame::Select(int iIndex)
{
    if (gameList._iCount <= 0)
    {
        curGame._pNode = &gameList._head;
        return;
    }
    while (iIndex)
    {
        if (iIndex < 0)
        {
            do
            {
                curGame._pNode = curGame._pNode->_pPrev;
                if (!curGame._pNode->_pObject)
                {
                    curGame._pNode = gameList._head._pPrev;
                }
            } while (!curGame._pNode->_pObject->mValid);
            ++iIndex;
        }
        else
        {
            do
            {
                curGame._pNode = curGame._pNode->_pNext;
                if (!curGame._pNode->_pObject)
                {
                    curGame._pNode = gameList._head._pNext;
                }
            } while (!curGame._pNode->_pObject->mValid);
            --iIndex;
        }
    }
}

// FUNCTION: SWBF2 0x005df2c0
void CNetFindGame::SelectSortPos(int iPos)
{
    if (gameList._iCount >= 1)
    {
        do
        {
            NetGameDesc* game = curGame._pNode->_pObject;
            if (game->mValid && game->mFindData.mSortOrder == iPos)
            {
                break;
            }
            curGame._pNode = curGame._pNode->_pNext;
        } while (curGame._pNode->_pObject);
    }
}

// FUNCTION: SWBF2 0x005df420
char* CNetFindGame::GetName()
{
    return curGame._pNode->_pObject->mFindData.mName;
}

// FUNCTION: SWBF2 0x005df480
void CNetFindGame::SetSortPosition(unsigned short usPos)
{
    curGame._pNode->_pObject->mFindData.mSortOrder = usPos;
}

// FUNCTION: SWBF2 0x005df550
int CNetFindGame::GetMaxPlayers()
{
    return curGame._pNode->_pObject->mFindData.mMaxPlayers;
}

// FUNCTION: SWBF2 0x005df570
int CNetFindGame::GetMaxBots()
{
    return curGame._pNode->_pObject->mFindRules.mNumAIUnits;
}

// FUNCTION: SWBF2 0x005df680
bool CNetFindGame::GetFavorite()
{
    NetGameDesc* game = curGame._pNode->_pObject;
    return game ? game->mFindData.mbIsFavorite : false;
}

// FUNCTION: SWBF2 0x005df6d0
bool CNetFindGame::GetFiltered()
{
    NetGameDesc* game = curGame._pNode->_pObject;
    return game ? game->mFindData.mbFilteredOut : true;
}

// FUNCTION: SWBF2 0x005df710
void CNetFindGame::SetFiltered(bool bFiltered)
{
    NetGameDesc* game = curGame._pNode->_pObject;
    if (game)
    {
        game->mFindData.mbFilteredOut = bFiltered;
    }
}

// FUNCTION: SWBF2 0x005df750
bool CNetFindGame::GetRules(FindGameRules& rules)
{
    NetGameDesc* game = curGame._pNode->_pObject;
    rules = game->mFindRules;
    return game->mRulesAreValid;
}

// FUNCTION: SWBF2 0x005df510
char* CNetFindGame::GetDescriptorExtension()
{
    NetGameDesc* game = curGame._pNode->_pObject;
    return game ? (char*)game + 0xf1 : 0;
}

// FUNCTION: SWBF2 0x005df340
bool CNetFindGame::IsMetaGame()
{
    return _strnicmp(curGame._pNode->_pObject->mFindData.mMapType,
                     "metagame", 8) == 0;
}

// FUNCTION: SWBF2 0x005e07d0
void CNetJoinGame::Cancel()
{
    if (!shellMove.quitRequested)
    {
        cancelTimer = NetComm::GetTime() + 5.0f;
        shellMove.quitRequested = true;
        timer = 0.0f;
        for (int i = 0; i < netCurMaxPlayers; ++i)
        {
            if (NetComm::IsConnected(i))
            {
                NetComm::Disconnect(i);
            }
        }
    }
}

// FUNCTION: SWBF2 0x005e0860
bool CNetJoinGame::IsCancelled()
{
    if (connecting)
    {
        return true;
    }
    int iLocal = NetComm::GetLocalIndex(0);
    if (iLocal < 0)
    {
        if (NetComm::IsConnected(64))
        {
            NetComm::Disconnect(64);
        }
        return true;
    }
    return (shellUpdate.quitMask & (1ULL << iLocal)) != 0 ||
           NetComm::GetTime() > cancelTimer;
}

// FUNCTION: SWBF2 0x005e0b00
char const* CNetJoinGame::GetPlayerName(int iPlayer)
{
    return NetGame::GetPlayerName(iPlayer);
}

// FUNCTION: SWBF2 0x005e0b20
int CNetJoinGame::GetStartTeam(int iPlayer)
{
    return NetGame::GetStartTeam(iPlayer);
}

// FUNCTION: SWBF2 0x005e0bc0
CNetJoinGame* CNetJoinGame::NextHost(int)
{
    ++hostRequest;
    return this;
}

// FUNCTION: SWBF2 0x005e2620
void CNetPostLoadHostGame::Cancel()
{
    canceled = true;
}

// FUNCTION: SWBF2 0x005e3280
void CNetPostLoadHostGame::BootPlayer(int) {}

// FUNCTION: SWBF2 0x005e3700
void CNetPostLoadJoinGame::Cancel()
{
    canceled = true;
}

// FUNCTION: SWBF2 0x005df390
const char* CNetFindGame::GetEra()
{
    const char* era = curGame._pNode->_pObject->mFindData.mMapType;
    return _strnicmp(era, "metagame", 8) == 0 ? era + 8 : "";
}

void ReadString(NetPktGroup& group, char* text, unsigned int capacity);

// FUNCTION: SWBF2 0x005e7a00
void ReadFiles(NetPktGroup& group, SendStateDst& state)
{
    bool done = false;
    group.Read(done);
    if (done)
    {
        if (!state.done && state.gotSome)
        {
            _close(state.fd);
            state.fd = -1;
            char fileName[48];
            _snprintf(fileName, sizeof(fileName), "dl_%s", state.fName);
            rename("sendfile.dat", fileName);
        }
        state.done = true;
        return;
    }

    state.gotSome = true;
    if (state.fd < 0)
    {
        state.fd = _open("sendfile.dat", 0x8301, 0x1ff);
    }
    unsigned int baseBlock = 0;
    group.ReadBits(baseBlock, 24);
    for (;;)
    {
        unsigned int code = 0;
        group.ReadBits(code, 8);
        if (!code)
        {
            break;
        }
        unsigned int block = baseBlock + code - 1;
        if (!block)
        {
            ReadString(group, state.fName, sizeof(state.fName));
        }
        state.hasBlocks |= 1u << (block - state.baseBlock);
        while (state.hasBlocks & 1)
        {
            ++state.baseBlock;
            state.hasBlocks >>= 1;
        }
        char data[2048];
        group.ReadBytes(data, sizeof(data));
        _lseek(state.fd, block << 11, SEEK_SET);
        _write(state.fd, data, sizeof(data));
    }
}

void WriteString(NetPktGroup* group, const char* text, unsigned int capacity);

// FUNCTION: SWBF2 0x005e7670
void WriteFiles(NetPktGroup& group, SendStateSrc& state)
{
    if (!state.files)
    {
        group.Write(true);
        return;
    }
    if (state.curFile && state.baseBlock >= state.blockCount)
    {
        RedFreeToHeap(__RedCurrHeap, state.curFile, 0);
        state.curFile = 0;
        ++state.fileNum;
        if (state.fileNum >= state.fileCount)
        {
            RedFreeToHeap(__RedCurrHeap, state.files, 0);
            state.files = 0;
            group.Write(false);
            return;
        }
    }
    group.Write(false);
    if (!state.curFile)
    {
        PblFile file;
        const char* name = ((char**)state.files)[state.fileNum];
        if (!file.Open(name, false, false, true))
        {
            RedFreeToHeap(__RedCurrHeap, state.files, 0);
            state.files = 0;
            return;
        }
        int remaining = file.GetSize();
        state.blockCount = (remaining + 2047) / 2048;
        state.curFile = (char*)RedAllocFromHeap(__RedCurrHeap, state.blockCount << 11, 0);
        if (state.curFile)
        {
            for (int i = 0; i < state.blockCount; ++i)
            {
                int size = remaining > 2048 ? 2048 : remaining;
                file.ReadData(state.curFile + (i << 11), size);
                remaining -= size;
            }
        }
        file.Close();
    }
    group.WriteBits(state.baseBlock, 24);
    for (int i = 0; i < 8; ++i)
    {
        for (;;)
        {
            if (state.baseBlock + state.blockCycle >= state.blockCount)
            {
                state.blockCycle = 0;
            }
            if (!(state.hasBlocks & (1u << state.blockCycle)))
            {
                break;
            }
            if (++state.blockCycle >= 32)
            {
                state.blockCycle = 0;
            }
        }
        int block = state.baseBlock + state.blockCycle;
        group.WriteBits(state.blockCycle + 1, 8);
        if (!block)
        {
            WriteString(&group, ((char**)state.files)[state.fileNum], 32);
        }
        group.WriteBytes(state.curFile + (block << 11), 2048);
        if (++state.blockCycle >= 32)
        {
            state.blockCycle = 0;
        }
    }
    group.WriteBits(0, 8);
}

// FUNCTION: SWBF2 0x005b1a30
unsigned int* __fastcall ClearGalaxyIdWords(unsigned int* pWords)
{
    pWords[0] = 0;
    pWords[1] = 0;
    return pWords;
}

// FUNCTION: SWBF2 0x005df5a0
bool CNetFindGame::GetInfo(char*& pcName, char*& pcMission, int& iNumPlayers,
                           int& iMaxPlayers, bool& bPassword, float& fPing,
                           int& iArg7, int& iArg8, bool& bArg9, bool& bArg10,
                           char*& pcArg11, bool& bArg12)
{
    NetGameDesc* game = curGame._pNode->_pObject;
    FindGameData& data = game->mFindData;
    pcName = data.mName;
    pcMission = data.mMapName;
    iNumPlayers = data.mNumPlayers;
    iMaxPlayers = data.mMaxPlayers;
    pcArg11 = game->mGameVer;
    bPassword = IsMetaGame();
    fPing = data.mPingTime;
    iArg7 = data.mServerType;
    iArg8 = data.mSortOrder;
    bArg9 = data.mLocked;
    bArg10 = data.mbVerMatch;
    bArg12 = data.mbIsFavorite;
    return data.mServerType > 0;
}

// FUNCTION: SWBF2 0x005dedf0
bool CNetFindGame::IsReady()
{
    switch (findType)
    {
    case FT_QUICK:
    {
        int count = GetGameCount();
        if (count <= 0)
        {
            return false;
        }
        SelectFirst();
        for (int i = 0; i < count; ++i)
        {
            char *name, *mission, *version;
            int numPlayers, maxPlayers, arg7, arg8;
            bool password, arg9, arg10, arg12;
            float ping;
            findGame->GetInfo(name, mission, numPlayers, maxPlayers, password,
                              ping, arg7, arg8, arg9, arg10, version, arg12);
            if (numPlayers < maxPlayers)
            {
                return true;
            }
            findGame->Select(1);
        }
        return false;
    }
    case FT_PREVIOUS:
        if (NetComm::GetTime() - findStartTime < 2.0f)
        {
            return false;
        }

    case FT_FIXED:
    case FT_CURRENT:
        if (GetGameCount() > 0)
        {
            SelectFirst();
            return true;
        }
        return false;
    default:
        return true;
    }
}

// SYNTHETIC: SWBF2 0x005e1ce0
// CNetLoadGame::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005e1d10
CNetLoadGame::~CNetLoadGame() {}

// SYNTHETIC: SWBF2 0x005e0dc0
// CNetLaunchGame::`scalar deleting destructor'
CNetLaunchGame::~CNetLaunchGame() {}

extern int updatePercent;

// FUNCTION: SWBF2 0x005d6730
NetAddr::NetAddr(int addr, int port, int overridePort)
{
    pubAddr = 0;
    pubPort = 0;
    if (!IsPrivateIP((unsigned long)addr))
    {
        __debugbreak();
        return;
    }
    privAddr = addr;
    privPort = overridePort ? overridePort : port;
    hasPriv = true;
}

// FUNCTION: SWBF2 0x005dd470
bool CNetHostGame::IsLaunchReady()
{
    if (!launchReady || !NetQOS::IsDone())
    {
        return false;
    }
    bool ready = true;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (NetComm::IsConnected(i) && !shellState.players[i].joinAck)
        {
            ready = false;
        }
    }
    return ready;
}

// FUNCTION: SWBF2 0x005ddc80
void CNetFindGame::Pause()
{
    NetMatch::PauseFindGames();
}

// FUNCTION: SWBF2 0x005df3f0
bool CNetFindGame::GetPlayerData(int, FindGamePlayer&)
{
    return NetMatch::GetPlayerDataByAddress();
}

// GLOBAL: SWBF2 0x01e6503c
int netMatchOperation;

extern int netLanRegisterState;
extern bool isLAN;
extern unsigned int netBroadcastSendSocket;
extern unsigned int netBroadcastRecvSocket;

extern bool netLobbyRequestPending;
extern bool netLeavingLobby;
extern unsigned __int64 netCreatedLobbyId;
extern bool gGalaxyInitialized;
void* GetGalaxyMatchmaking();
void ResetLanRegistration();
namespace gf { bool checkError(); }

// FUNCTION: SWBF2 0x005d6770
void ResetLanRegistration()
{
    netLobbyState = 0;
    netLobbyRequestPending = false;
    netLeavingLobby = false;
    netCreatedLobbyId = 0;
    netEnteredLobbyId = 0;
    NetSocket::OpenBroadcastSockets();
}

// GLOBAL: SWBF2 0x01faab3c
char netLocalIPText[18];

// FUNCTION: SWBF2 0x005dd520
NetGameDesc::NetGameDesc(const NetAddr& addr)
{
    mNode._pList = 0;
    mNode._pNext = 0;
    mNode._pPrev = 0;
    mNode._pObject = 0;
    mAddr.pubAddr = 0;
    mAddr.pubPort = 0;
    mAddr.hasPriv = false;
    mAddr.privAddr = 0;
    mAddr.privPort = 0;
    mPrevId.id = 0;
    mValid = false;
    mRulesAreValid = false;
    mHasSessionData = false;
    mAddr.CopyFieldsFrom(&addr);
    mStamp = NetComm::GetTime();
    mFindData.mDidServerPing = false;
    mFindData.mQuerySentTime = -1.0f;
    mFindData.mResponseRecvTime = -1.0f;
    const unsigned int pingBits = 0x411ffbe7u;
    memcpy(&mFindData.mPingTime, &pingBits, sizeof(pingBits));
    mFindData.mConnecting = false;
    mFindData.mConnected = false;
    mFindData.mConnectTimeout = 0.0f;
    mFindData.mSortOrder = 0xffff;
    mFindData.mServerType = 0;
    mFindData.mLocked = false;
    mFindRules.mAutoTeams = true;
    mFindRules.mAimAssist = true;
    mFindRules.mHeroes = false;
    mFindRules.mNumAIUnits = 16;
    mFindRules.mNumReinforcements1 = 200;
    mFindRules.mNumReinforcements2 = 200;
    mFindData.mQueryId = netRandom.GetInt();
    mFindData.mQuerySeqNo = 0;
    mFindData.mbVerMatch = true;
    mFindData.mbIsFavorite = false;
}

// FUNCTION: SWBF2 0x005dd6b0
bool NetGameDesc::operator==(const NetGameDesc& rOther)
{
    const NetGameDesc* game = this;
    const NetGameDesc* other = &rOther;
    if (game->mAddr.hasPriv &&
        (game->mAddr.privAddr != other->mAddr.privAddr ||
         game->mAddr.privPort != other->mAddr.privPort))
    {
        return false;
    }
    return EqualGalaxyIdWords((const unsigned int*)&other->mAddr, 0,
                        (const unsigned int*)&game->mAddr);
}

// GLOBAL: SWBF2 0x01ee7d48
unsigned __int64 netFindSessionId;

// GLOBAL: SWBF2 0x01fa88d4
float netNextLanQueryTime;

// GLOBAL: SWBF2 0x01fa88d8
int netLanQueryCount;

// GLOBAL: SWBF2 0x01fa88dc
unsigned int netLanQuerySequence;

// FUNCTION: SWBF2 0x005d5e70
char NetMatch::StartFindGames(int, const NetSessionId* id)
{
    if (id)
    {
        netFindSessionId = id->id;
    }
    netMatchOperation = 2;
    if (NetComm::IsLAN())
    {
        if (!netDirectPort)
        {
            netDirectPort = NetSocket::OpenDirectSocket(false);
        }
        NetSocket::OpenBroadcastSockets();
        netNextLanQueryTime = 0.0f;
        netLanQueryCount = 0;
        netLanQuerySequence = netRandom.GetInt();
        return (char)netLanQuerySequence;
    }
    if (gf::isInitialized())
    {
        netLobbyState = 1;
        void* matchmaking = GetGalaxyMatchmaking();
        ((galaxy::api::IMatchmaking*)matchmaking)->AddRequestLobbyListStringFilter("galaxy_lobby_cancelled", "1", 1);
        matchmaking = GetGalaxyMatchmaking();
        return ((galaxy::api::IMatchmaking*)matchmaking)->RequestLobbyList(false);
    }
    return 0;
}

extern int netConnectState;

// FUNCTION: SWBF2 0x005d7700
void NetMatch::StartFindAddress()
{
    NetAddr* addr = &NetSetupP::GetMyAddr();
    if (NetComm::IsLAN())
    {
        unsigned long ip = GetPrivateIP();
        addr->privAddr = ip;
        if (ip)
        {
            addr->pubAddr = 0;
            addr->pubPort = 0;
            addr->hasPriv = true;
        }
    }
    else
    {
        addr->hasPriv = false;
        addr->privAddr = 0;
        addr->privPort = 0;
    }
}

// FUNCTION: SWBF2 0x005def50
bool CNetFindGame::IsFinished()
{
    bool& waiting = mWaitingForQueries;
    float& finishTime = mQueryFinishTime;
    if (waiting && NetComm::GetTime() > finishTime)
    {
        return true;
    }
    if (!waiting && NetMatch::IsDoneFindGames())
    {
        bool allFinished = true;
        for (PblList<NetGameDesc>::Node* node = gameList._head._pNext;
             node->_pObject; node = node->_pNext)
        {
            NetGameDesc* game = node->_pObject;
            if (game->mValid && !game->mHasSessionData &&
                game->mFindData.mQuerySeqNo < 8)
            {
                allFinished = false;
                break;
            }
        }
        if (allFinished)
        {
            waiting = true;
            finishTime = NetComm::GetTime() + 3.0f;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x005df030
void CNetFindGame::Refresh()
{
    mWaitingForQueries = false;
    mQueryFinishTime = 0.0f;
    NetMatch::EndFindGames(false);
    NetMatch::StartFindGames(0, (const NetSessionId*)(mDirectJoinState + 32));
}

// FUNCTION: SWBF2 0x005dec90
void CNetFindGame::RemoveOld(float age)
{
    float now = NetComm::GetTime();
    for (PblList<NetGameDesc>::Node* node = gameList._head._pNext;
         node->_pObject; )
    {
        NetGameDesc* game = node->_pObject;
        node = node->_pNext;
        if (now - age > game->mStamp)
        {
            if (game->mValid && (game->mFindData.mConnecting || game->mFindData.mConnected))
            {
                NetMatch::EndConnect();
            }
            gameList.Remove(&game->mNode);
            mDirty = true;
            ::operator delete(game);
        }
    }
}

// SYNTHETIC: SWBF2 0x005dda90
// CNetFindGame::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005ddac0
CNetFindGame::~CNetFindGame()
{
    RemoveOld(-1.0f);
    NetMatch::EndFindGames(canceled);
    findGame = 0;
    netInGame = false;
}

// GLOBAL: SWBF2 0x01eaed30
float gSelectionTimer;

// FUNCTION: SWBF2 0x005ddb40
void CNetFindGame::SetSelectedSession(int session)
{
    if (session > gameList._iCount - 1)
    {
        return;
    }
    if (session != mSelectedSession)
    {
        mSelectedSession = session;
        gSelectionTimer = NetComm::GetTime();
    }
    int validIndex = 0;
    for (PblList<NetGameDesc>::Node* node = gameList._head._pNext;
         node->_pObject; node = node->_pNext)
    {
        NetGameDesc* game = node->_pObject;
        if (game->mValid)
        {
            if (validIndex == mSelectedSession && !game->mFindData.mDidServerPing)
            {
                game->mFindData.mDidServerPing = NetMatch::PingServerByAddress();
            }
            ++validIndex;
        }
    }
    char journalCode = 'c';
    PblJournal::Record(journalCode, PJT_FULL);
}

void __fastcall HandleFindPacket(NetPacket* packet);
void __fastcall HandleFoundPacket(NetPacket* packet, const void* source);

// FUNCTION: SWBF2 0x005e3fa0
void NetSetupP::RecvLan(NetPacket* packet, const void* source)
{
    NetMatch::ReceiveLanRegisterPacket(packet, source);
}

class NetAdmin { public: static void RemovePlayer(int iPlayer); };

// SYNTHETIC: SWBF2 0x005e24a0
// CNetPostLoadHostGame::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005e24d0
CNetPostLoadHostGame::~CNetPostLoadHostGame()
{
    if (canceled)
    {
        for (int i = 0; i < netCurMaxPlayers; ++i)
        {
            if (NetComm::IsConnected(i))
            {
                NetComm::Disconnect(i);
            }
        }
        if (NetComm::IsConnected(64))
        {
            NetComm::Disconnect(64);
        }
        netCurMaxPlayers = 1;
        netPrevSessionId = netSessionId;
        NetMatch::EndRegisterGame();
        isInSession = false;
        GameVoiceChat::Reset();
        for (int i = 0; i < 64; ++i)
        {
            NetGame::SetPlayerGSID(i, 0);
            NetAdmin::RemovePlayer(i);
            NetGame::ClearPlayerCDKey(i);
            NetGameP::SetPlayerName(i, "");
        }
        NetGameP::GetHostIndex() = -1;
        netIsDedicated = false;
    }
    else
    {
        netEnabled = true;
    }
}

// SYNTHETIC: SWBF2 0x005dfb10
// CNetJoinGame::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005dfb40
CNetJoinGame::~CNetJoinGame()
{
    if (shellMove.quitRequested)
    {
        if (cFind)
        {
            delete (NetFindGame*)cFind;
        }
        netInGame = false;
        isInSession = false;
        NetMatch::EndJoinGame();
        for (int i = 0; i < 64; ++i)
        {
            NetGame::SetPlayerGSID(i, 0);
            NetGame::ClearPlayerCDKey(i);
            NetGameP::SetPlayerName(i, "");
        }
        NetQOS::CancelGame();
        shellUpdate.patchFile.Cancel();
    }
}

// FUNCTION: SWBF2 0x005dceb0
void CNetHostGame::DropPlayer(int iPlayer)
{
    NetGameP::SetPlayerName(iPlayer, "");
    NetGame::SetPlayerGSID(iPlayer, 0);
    NetGame::ClearPlayerCDKey(iPlayer);
    if (NetGame::IsHost())
    {
        GameVoiceChat::GetTransport()->DisconnectNode((unsigned char)iPlayer);
    }
    int hostIndex = *NetGameP::GetHostIndex(iPlayer);
    if (hostIndex >= 0)
    {
        *NetGameP::GetHostMode(hostIndex) = 3;
    }
    NetComm::Disconnect(iPlayer);
    bootMask &= ~(1ULL << iPlayer);
    shellState.players[iPlayer].Init();
    NetGameP::SetKey(iPlayer, 0);
}

// GLOBAL: SWBF2 0x01faa720
bool netHandlingFindPacket;

extern int archFilter;
void WriteString(NetPktGroup* pGroup, const char* text, unsigned int capacity);

// FUNCTION: SWBF2 0x005e73b0
void NetSetupP::AddGame(const NetAddr* address)
{
    ((CNetFindGame*)((CNetFindGame*)findGame))->AddGame(address);
}

// FUNCTION: SWBF2 0x005e73f0
void NetSetupP::UpdateGame(const NetAddr* address, const void* session, const void* key, const char* version, const void* playerData, const void* extra)
{
    if (findGame)
    {
        ((CNetFindGame*)findGame)->UpdateGame(address, session, key, version, playerData, extra);
    }
}

// FUNCTION: SWBF2 0x005e0bf0
NetLaunchGame* NetSetup::LaunchGame(bool host)
{
    return new CNetLaunchGame(host);
}

// FUNCTION: SWBF2 0x005e1ba0
NetLoadGame* NetSetup::LoadGame(bool host)
{
    return new CNetLoadGame(host);
}

// FUNCTION: SWBF2 0x005e0c60
CNetLaunchGame::CNetLaunchGame(bool host)
{
    launchHost = host;
    launchDone = false;
    launchAcked = false;
    NetComm::AdvanceTime(true);
    const float now = NetComm::GetTime();
    for (unsigned int i = 0; i < 64; ++i)
    {
        launchAcks[i] = false;
        sendTimes[i] = now - 1.0f;
    }
    startTime = now;
    timer = 0.0f;
    if (NetComm::GetLocalIndex(0) >= 0)
    {
        NetPktGroup group;
        group.BeginWrite();
        group.WriteBits(0, 1);
        group.EndWrite();
        NetComm::Send(64, PKT_LAUNCH_ACK, group.pktPtr);
    }
    netInMultiplayer = true;
    netInGame = true;
    netWasHost = NetComm::IsHost();
    netWasClient = !netWasHost;
    netWasDedicated = NetGame::IsDedicated();
}

// FUNCTION: SWBF2 0x005e1c10
CNetLoadGame::CNetLoadGame(bool host)
{
    loadHost = host;
    loadDone = false;
    hostKnowsILoaded = false;
    timer = 0.0f;
    ackedMask = 0;
    loadAcked = false;
    NetComm::AdvanceTime(false);
    for (unsigned int i = 0; i < 64; ++i)
    {
        sendTimes[i] = 0.0f;
    }
}

// FUNCTION: SWBF2 0x005e22f0
NetPostLoadHostGame* NetSetup::PostLoadHostGame()
{
    return new CNetPostLoadHostGame;
}

// FUNCTION: SWBF2 0x005e3770
NetPostLoadJoinGame* NetSetup::PostLoadJoinGame()
{
    return new CNetPostLoadJoinGame;
}

// FUNCTION: SWBF2 0x005e2360
CNetPostLoadHostGame::CNetPostLoadHostGame()
{
    canceled = false;
    netHostSetupReady = 1;
    needSendMask = 0;
    loadingMask = 0;
    StatsManager::BeginMatch();
    timer = 0.0f;
    shellState.canceled = false;
    for (int i = 0; i < 64; ++i)
    {
        shellState.players[i].Init();
        if (NetGame::IsPlaying(i, false))
        {
            shellState.players[i].joinAck = true;
        }
        else if (NetComm::IsConnected(i))
        {
            loadingMask |= 1ULL << i;
        }
    }
}

// FUNCTION: SWBF2 0x005dd950
CNetFindGame::CNetFindGame(FindType type, const char*, NetSessionId* id)
{
    curGame._pNode = 0;
    memset(mDirectJoinState + 8, 0, sizeof(NetAddr));
    memset(mDirectJoinState + 32, 0, sizeof(NetSessionId));
    mDirty = true;
    mPercentDone = 0;
    mSelectedSession = -1;
    findType = type;
    if (NetComm::IsLAN())
    {
        FindAddress();
    }
    canceled = false;
    netLoginError = false;
    netGameError = false;
    SetupSleep();
    NetComm::AdvanceTime(true);
    prevFindStartTime = 0.0f;
    findStartTime = NetComm::GetTime();
    if (id)
    {
        memcpy(mDirectJoinState + 32, id, sizeof(*id));
    }
    NetMatch::StartFindGames(0, (const NetSessionId*)(mDirectJoinState + 32));
    mWaitingForQueries = false;
    mQueryFinishTime = 0.0f;
}

// FUNCTION: SWBF2 0x005dd8d0
NetFindGame* NetSetup::FindGame(FindType type, const char* name, NetSessionId* id)
{
    return new CNetFindGame(type, name, id);
}

// GLOBAL: SWBF2 0x01fa8c00
NetKey hostKey;

extern int netPlayerLimit;

// FUNCTION: SWBF2 0x005df890
CNetJoinGame::CNetJoinGame(NetFindGame* find)
{
    SaveSessionRuleDefaults();
    NetGame::ClearAllCDKeyInfo();
    cFind = find;
    if (((CNetFindGame*)find)->gameList._iCount < 1)
    {
        return;
    }

    ((CNetFindGame*)((CNetFindGame*)find))->GetHostAddr(&hostAddr, &hostKey);
    connecting = true;
    hostPlayer = -1;
    NetMatch::StartJoinGame((const NetSessionId*)&hostAddr);
    connected = NetMatch::StartConnect();
    strlcpy(gameName, find->GetName(), sizeof(gameName));
    NetSetup::SetGameName(gameName);
    GameLoop::SetMetagameRulesOn(((CNetFindGame*)find)->IsMetaGame());
    strlcpy(gameEra, ((CNetFindGame*)find)->GetEra(), sizeof(gameEra));
    hostLaunched = false;
    tryCount = 0;
    clientLaunched = false;
    changeCount = 0;
    timer = 0.0f;
    hostRequest = 0;
    moveNum = 0;
    ackedMoveNum = 0;
    joinFailed = false;
    rejectReason = 0;
    cancelTimer = 0.0f;
    for (int i = 0; i < 2; ++i)
    {
        NetCommP::GetLocalIndex(i) = -1;
    }
    isInSession = true;
    netPlayerLimit = find->GetMaxPlayers();
    netNumBots = find->GetMaxBots();
}

// FUNCTION: SWBF2 0x005df820
NetJoinGame* NetSetup::JoinGame(NetFindGame* find)
{
    return new CNetJoinGame(find);
}

// FUNCTION: SWBF2 0x005ded60
void CNetFindGame::RemoveGameByAddress(const NetAddr* address)
{
    NetGameDesc probeDesc(*address);
    NetGameDesc* probe = &probeDesc;

    PblList<NetGameDesc>::Node* node = gameList._head._pNext;
    while (node->_pObject)
    {
        NetGameDesc* game = node->_pObject;
        if (*game == *probe)
        {
            gameList.Remove(&game->mNode);
            mDirty = true;
            ::operator delete(game);
        }
    }
}

bool PumpGalaxyMatchmaking();

// FUNCTION: SWBF2 0x005dd330
void CNetHostGame::BootPlayer(int player)
{
    char name[64];
    strlcpy(name, GetPlayerName(player), sizeof(name));
    char* suffix = strchr(name, '|');
    if (suffix)
    {
        *suffix = 0;
    }
    int index = 0;
    for (; index < netCurMaxPlayers; ++index)
    {
        if (NetComm::IsConnected(index) &&
            strcmp(name, NetGame::GetPlayerName(index)) == 0)
        {
            break;
        }
    }
    if (index != netCurMaxPlayers)
    {
        bootMask |= 1ULL << index;
        bootTime[index] = NetComm::GetTime() + 5.0f;
    }
}

// SYNTHETIC: SWBF2 0x005e3740
// CNetPostLoadJoinGame::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005e3470
CNetPostLoadJoinGame::~CNetPostLoadJoinGame()
{
    if (canceled)
    {
        netPrevSessionId = netSessionId;
        NetMatch::EndJoinGame();
        isInSession = false;
        GameVoiceChat::Reset();
        for (int i = 0; i < netCurMaxPlayers; ++i)
        {
            if (NetComm::IsConnected(i))
            {
                NetComm::Disconnect(i);
            }
        }
        netCurMaxPlayers = 1;
        if (NetComm::IsConnected(64))
        {
            NetComm::Disconnect(64);
        }
        for (int i = 0; i < 64; ++i)
        {
            NetGame::SetPlayerGSID(i, 0);
            NetGame::ClearPlayerCDKey(i);
            NetGameP::SetPlayerName(i, "");
        }
        return;
    }

    netEnabled = true;
    NetComm::AdvanceTime(true);
    float now = NetComm::GetTime();
    float finishTime = now + 0.5f;
    for (;;)
    {
        SetupSleep();
        NetComm::AdvanceTime(true);
        now = NetComm::GetTime();
        if (now > nextStats)
        {
            nextStats = now + 0.2f;
            NetPktGroup outgoing;
            outgoing.BeginWrite();
            outgoing.Write(false);
            outgoing.EndWrite();
            NetComm::Send(64, 19, &outgoing);
        }
        if (now > finishTime)
        {
            break;
        }
        NetComm::Update();
        for (NetPacket* packet = NetComm::Recv(PKT_GAME_UPDATE, PKT_NONE);
             packet; packet = NetComm::Recv(PKT_GAME_UPDATE, PKT_NONE))
        {
            packet->Free();
            finishTime = now + 3.0f;
        }
        NetPktGroup incoming;
        while (NetComm::Recv(&incoming, 20))
        {
            incoming.BeginRead();
            bool accepted = false;
            incoming.Read(accepted);
            if (accepted)
            {
                finishTime = now + 3.0f;
            }
            incoming.Free();
        }
    }
}

// FUNCTION: SWBF2 0x005e3a90
void NetSetup::WriteGameUpdate(NetPktGroup& group)
{
    int player = NetGame::GetCurDst();
    if (NetGame::GetJoystickIndex(player) >= 0)
    {
        return;
    }
    NetGame::WritePreGame(group);
    unsigned __int64 playerBit = 1ULL << player;
    if (!(playerChangedSendMask & playerBit))
    {
        group.Write(false);
        return;
    }
    if (NetGame::GetAckedUpdateTurn(player) > playerChangeTurn[player])
    {
        playerChangedSendMask &= ~playerBit;
        playerChangedMask[player] = 0;
        group.Write(false);
        return;
    }
    group.Write(true);
    WriteDestMask(group, playerChangedMask[player]);
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (!(playerChangedMask[player] & (1ULL << i)))
        {
            continue;
        }
        group.Write(NetGame::GetStartTeam(i) == 1);
        if (i == player)
        {
            continue;
        }
        WriteString(&group, NetGame::GetPlayerName(i), 32);
        unsigned __int64 id = NetGame::GetPlayerGSID(i);
        group.WriteBytes((const char*)&id, sizeof(id));
        int owner = NetGame::GetGuestOf(i);
        group.Write(owner >= 0);
        if (owner >= 0)
        {
            group.WriteBits((unsigned int)owner, 6);
        }
        else
        {
            WriteAddr(&group, &clientAddrs[i]);
            group.WriteBits((unsigned int)NetComm::GetArch(i), 2);
        }
    }
}

// FUNCTION: SWBF2 0x005e40b0
void NetSetupP::NextHost(int direction)
{
    LuaHelper::GetTop();
    LuaHelper::GetGlobal("lobby_listbox_layout");
    LuaHelper::PushString("SelectedIdx");
    LuaHelper::RawGetFromTable(-2);
    if (LuaHelper::IsNumber(-1))
    {
        int selected = LuaHelper::GetInt(-1);
        LuaHelper::Pop(2);
        if (NetComm::IsHost())
        {
            int player = 0;
            for (; player < netCurMaxPlayers; ++player)
            {
                if (NetComm::IsConnected(player))
                {
                    if (selected == 1)
                    {
                        break;
                    }
                    --selected;
                }
            }
            bool changed = direction > 0 ? SelectNextHostMode(player) :
                           direction < 0 ? SelectNextHost(player) : false;
            if (changed && hostGame)
            {
                ++((CNetHostGame*)hostGame)->changeCount;
            }
        }
        else
        {
            ((CNetJoinGame*)joinGame)->NextHost(direction);
        }
    }
    else
    {
        LuaHelper::Pop(2);
    }
}

extern int gDebugCount;

// FUNCTION: SWBF2 0x005e5bb0
void WriteShellMove(NetPktGroup& group, ShellUpdate& update, ShellMove& move)
{
    const unsigned char* input = (const unsigned char*)FLInputManager::Instance();
    int viewport = ((FLInputManager*)input)->GetPrimaryPort();
    WriteString(&group, (const char*)netLoginNames + 32 * viewport, 32);
    if (move.launchGuestHost)
    {
        ++gDebugCount;
    }
    group.Write(move.launchGuestHost);
    group.Write(move.isTeamOne);
    group.Write(move.quitRequested);
    char mission[64];
    strlcpy(mission, GameLoop::GetNextMission(), sizeof(mission));
    WriteString(&group, netGameName, 64);
    WriteString(&group, mission, 64);
    group.WriteBits(move.hostRequest, 16);
    group.WriteBits(move.moveNum, 24);
    int local = 0;
    while (local < 2 && NetComm::GetLocalIndex(local) >= 0)
    {
        group.Write(true);
        group.WriteBits(NetComm::GetLocalIndex(local), 6);
        if (local == 0)
        {
            unsigned __int64 id = NetMatch::GetLocalGSID();
            group.WriteBytes((const char*)&id, sizeof(id));
        }
        ++local;
    }
    group.Write(false);
    WriteFilesAck(group, update.patchFile);
}

// FUNCTION: SWBF2 0x005e5d10
bool ReadShellMove(NetPktGroup& group, int player, ShellMove& move, ShellState& state)
{
    bool changed = false;
    char name[32];
    ReadString(group, name, sizeof(name));
    if (player != NetComm::GetLocalIndex(0) && strcmp(name, NetGame::GetPlayerName(player)))
    {
        changed = true;
        NetGameP::SetPlayerName(player, name);
    }

    bool guestHost, teamOne;
    group.Read(guestHost);
    group.Read(teamOne);
    if (player != NetComm::GetLocalIndex(0) &&
        (NetGame::GetStartTeam(player) == 1) != teamOne)
    {
        changed = true;
        NetGame::SetStartTeam(player, teamOne ? 1 : 2);
    }

    bool oldQuit = state.players[player].quitRequested;
    group.Read(move.quitRequested);
    if (move.quitRequested && !oldQuit)
    {
        changed = true;
        state.players[player].quitRequested = true;
    }

    char gameName[64], missionName[64];
    ReadString(group, gameName, sizeof(gameName));
    ReadString(group, missionName, sizeof(missionName));
    group.ReadBits(move.hostRequest, 16);
    group.ReadBits(move.moveNum, 24);

    for (int local = 0;; ++local)
    {
        bool present;
        group.Read(present);
        if (!present)
        {
            break;
        }
        unsigned int index;
        group.ReadBits(index, 6);
        if (index == (unsigned int)player || NetGame::GetGuestOf(index) == player)
        {
            ShellPlayer& shellPlayer = state.players[index];
            if (!shellPlayer.joinAck)
            {
                shellPlayer.joinAck = true;
                changed = true;
            }
        }
        if (local == 0)
        {
            unsigned __int64 id;
            group.ReadBytes((char*)&id, sizeof(id));
            if (player != NetComm::GetLocalIndex(0) && id != NetGame::GetPlayerGSID(player))
            {
                changed = true;
                NetGame::SetPlayerGSID(player, id);
                NetGame::ClearPlayerCDKey(player);
            }
        }
    }
    ReadFilesAck(group, state.players[player].patchFile);
    return changed;
}

// GLOBAL: SWBF2 0x007e8e52
bool netReplicaOk;
// GLOBAL: SWBF2 0x007e8e53
bool netRelayOk;
extern bool g_SpectatorEnabled[1];

// FUNCTION: SWBF2 0x005e54b0
void WriteJoin(NetPktGroup& group, unsigned int guestCount)
{
    group.WriteBits(guestCount, 3);
    unsigned int connectionType = netScript == NS_CLIENT && g_bNoRender ? 3 : 1;
    group.WriteBits(connectionType, 2);

    const unsigned char* input = (const unsigned char*)FLInputManager::Instance();
    int playerIndexes[2];
    int count = 0;
    for (int i = 0; i < 1; ++i)
    {
        if (input[0x2e6c + i])
        {
            playerIndexes[count++] = i;
        }
    }
    for (int i = count; i < 1; ++i)
    {
        playerIndexes[i] = -1;
    }
    WriteString(&group, (const char*)netLoginNames + 32 * playerIndexes[0], 32);

    unsigned __int64 id = NetMatch::GetLocalGSID();
    group.WriteBytes((const char*)&id, sizeof(id));
    for (unsigned int guest = 0; guest < guestCount; ++guest)
    {
        PlayerProfile::tProfile* profile = PlayerProfile::GetProfile((unsigned int)playerIndexes[guest + 1]);
        char converted[68];
        PblUnicode::ustrncnv(converted, profile->GetName(), 66);
        WriteString(&group, converted, 66);
    }
    group.WriteBits(intraSessionPlayerId0, 32);
    for (unsigned int guest = 0; guest < guestCount; ++guest)
    {
        group.WriteBits(intraSessionPlayerIdRest[guest], 32);
    }
    group.Write(netCrashAddr != 0);
    if (netCrashAddr)
    {
        group.WriteBits(netCrashAddr, 32);
    }
    group.Write(netReplicaOk);
    group.Write(netRelayOk);
    WriteAddr(&group, &myAddr);
    WriteAddr(&group, &hostAddrs[0]);
    group.Write(g_SpectatorEnabled[0]);
}

bool PumpGalaxyMatchmaking();

// FUNCTION: SWBF2 0x005d6090
void NetMatch::UpdateJoinGame()
{
    if (!NetComm::IsLAN())
    {
        PumpGalaxyMatchmaking();
    }
}

// FUNCTION: SWBF2 0x005e3720
void CNetPostLoadJoinGame::Update()
{
    NetMatch::UpdateLogin(-1);
    NetMatch::UpdatePresence(false);
    NetMatch::UpdateJoinGame();
}

// FUNCTION: SWBF2 0x005df790
void CNetFindGame::GetHostAddr(NetAddr* address, NetKey* key) const
{
    const CNetFindGame* find = (const CNetFindGame*)this;
    NetGameDesc* game = find->curGame._pNode->_pObject;
    address->CopyFieldsFrom(&game->mAddr);
    *key = game->mKey;
    game->mFindData.mQuerySeqNo = 69;
    if (game->mFindData.mConnecting || game->mFindData.mConnected)
    {
        game->mFindData.mConnecting = false;
        game->mFindData.mConnected = false;
    }
}

// FUNCTION: SWBF2 0x005dd730
void NetGameDesc::Update(const NetAddr* address, const NetKey* key, const NetSessionId* session, const char* version, const FindGameData* data, const FindGameRules* rules)
{
    NetGameDesc* game = (NetGameDesc*)this;
    game->mValid = true;
    game->mAddr.CopyFieldsFrom(address);
    game->mFindData.mNumPlayers = data->mNumPlayers;
    game->mFindData.mMaxPlayers = data->mMaxPlayers;
    strlcpy(game->mFindData.mName, data->mName, sizeof(game->mFindData.mName));
    strlcpy(game->mFindData.mMapType, data->mMapType, sizeof(game->mFindData.mMapType));
    if (data->mMapName[0])
    {
        strlcpy(game->mFindData.mMapName, data->mMapName, sizeof(game->mFindData.mMapName));
        game->mFindData.mLocked = data->mLocked;
        game->mFindData.mServerType = data->mServerType;
        game->mFindData.mbVerMatch = data->mbVerMatch;
        game->mFindData.mPingTime = data->mPingTime;
    }
    game->mFindData.mbIsFavorite = false;
    memcpy(game->mFindData.mGalaxyLobbyPasswordData, data->mGalaxyLobbyPasswordData, sizeof(game->mFindData.mGalaxyLobbyPasswordData));
    if (key)
    {
        game->mKey = *key;
    }
    if (session)
    {
        game->mPrevId = *session;
    }
    if (version)
    {
        strlcpy(game->mGameVer, version, sizeof(game->mGameVer));
        game->mGameVer[15] = 0;
    }
    game->mStamp = NetComm::GetTime();
    if (rules)
    {
        game->mRulesAreValid = true;
        game->mFindRules = *rules;
    }
}

// FUNCTION: SWBF2 0x005dea00
void CNetFindGame::AddGame(const NetAddr* address)
{
    CNetFindGame* find = (CNetFindGame*)this;
    if (find->gameList._iCount >= 2000)
    {
        return;
    }
    NetGameDesc probeDesc(*address);
    NetGameDesc* probe = &probeDesc;
    for (PblList<NetGameDesc>::Node* node = find->gameList._head._pNext;
         node->_pObject; node = node->_pNext)
    {
        if (*node->_pObject == *probe)
        {
            return;
        }
    }
    NetGameDesc* game = new NetGameDesc(*address);
    find->gameList.AddLast(&game->mNode, game);
    find->mDirty = true;
}

// FUNCTION: SWBF2 0x005deb60
void CNetFindGame::UpdateGame(const NetAddr* address, const void* session, const void* key, const char* version, const void* playerData, const void* extra)
{
    NetFindGame* base = (NetFindGame*)this;
    CNetFindGame* find = (CNetFindGame*)base;
    char journalCode = 'u';
    PblJournal::Record(journalCode, PJT_FULL);
    NetGameDesc probeDesc(*address);
    NetGameDesc* probe = &probeDesc;
    for (PblList<NetGameDesc>::Node* node = find->gameList._head._pNext;
         node->_pObject; node = node->_pNext)
    {
        NetGameDesc* game = node->_pObject;
        if (!(*game == *probe))
        {
            continue;
        }
        find->mDirty = true;
        ((NetGameDesc*)game)->Update(address, (const NetKey*)session, (const NetSessionId*)key, version, (const FindGameData*)playerData, (const FindGameRules*)extra);
        if (find->findType == 3 || find->findType == 2)
        {
            game->mHasSessionData = true;
        }
        if (find->GetGameCount() == 1)
        {
            find->curGame._pNode = node;
            unsigned char endpoint[24] = {};
            game->mAddr.GetAddr(endpoint, (int)&game->mKey);
            int ip = 0, port = 0;
            NetSocket::GetAddr(endpoint, ip, port);
        }
        return;
    }
}

extern bool netAutoAssignTeams;
extern bool netAutoAssignTeamsOriginalSetting;
extern bool netHeroesEnabled;
extern int netHeroRuleUnlock, netHeroRuleUnlockVal, netHeroRulePlayer, netHeroRuleRespawn, netHeroRuleRespawnVal;
extern int gSpawnInvicibility;
extern bool netAutoAim, netShowNames, netTeamDamage;
extern int netCurMaxHosts;
extern bool dragEnabled;
extern int dragLatency, dragBandwidth, dragLoss, dragJitter;
extern bool netIsDedicated;
extern bool netIsHostDedicated;
extern NetKey hostKey;

// GLOBAL: SWBF2 0x01ead4b4
bool netJoinUpdatePending;

void WriteDestMask(NetPktGroup& group, unsigned __int64 mask);
void UpdateCurMaxHosts(unsigned int uMask, bool bIncludeCurrent);
int GetPacketSourceHost(NetPacket* packet);
namespace GameVoiceChat { void* GetNode(); }
void WriteFiles(NetPktGroup& group, SendStateSrc& state);
void ReadFiles(NetPktGroup& group, SendStateDst& state);

// FUNCTION: SWBF2 0x005e0e10
void WriteLaunch(NetPktGroup& group, bool bAcked, int player, bool bPlaying)
{
    group.Write(netAutoAssignTeams);
    group.Write(netAutoAssignTeamsOriginalSetting);
    group.Write(bAcked);

    unsigned __int64 playerMask = 0;
    int i;
    for (i = 0; i < netCurMaxPlayers; ++i)
    {
        bool present;
        if (bPlaying)
        {
            present = NetGame::IsPlaying(i, false);
        }
        else
        {
            present = NetComm::IsConnected(i);
        }
        if (present)
        {
            playerMask |= 1ULL << i;
        }
    }
    for (int slot = 0; slot < 2; ++slot)
    {
        int guest = NetGame::GetGuestIndex(player, slot);
        if (guest >= 0)
        {
            playerMask |= 1ULL << guest;
        }
    }
    WriteDestMask(group, playerMask);

    char mission[64];
    strlcpy(mission, GameLoop::GetNextMission(), sizeof(mission));
    mission[63] = 0;
    WriteString(&group, mission, 64);
    WriteSessionId(&group, (const unsigned int*)&netSessionId.id);

    group.Write(netHeroesEnabled);
    if (netHeroesEnabled)
    {
        group.WriteBits(netHeroRuleUnlock, 3);
        group.WriteBits(netHeroRuleUnlockVal, 9);
        group.WriteBits(netHeroRulePlayer, 2);
        group.WriteBits(netHeroRuleRespawn, 2);
        group.WriteBits(netHeroRuleRespawnVal, 9);
    }
    group.WriteBits(hostVoteThreshold, 8);
    group.Write(netTeamDamage);
    group.Write(netAutoAim);
    group.WriteBits(gSpawnInvicibility, 8);
    group.Write(netShowNames);

    int hostIndex = *NetGameP::GetHostIndex(player);
    if (hostIndex < 0)
    {
        group.Write(false);
        int clientOf = *NetGameP::GetClientOfIndex(player);
        group.WriteBits(clientOf, 2);
        if (clientOf > 0)
        {
            WriteAddr(&group, &hostAddrs[clientOf]);
        }
    }
    else if (*NetGameP::GetHostMode(hostIndex))
    {
        group.Write(true);
        group.WriteBits(hostIndex, 2);
        NetCommP::Connect(hostIndex + 64, NetCommP::GetAddr(player));

        unsigned __int64 clientMask = 0;
        for (i = 0; i < netCurMaxPlayers; ++i)
        {
            if (*NetGameP::GetClientOfIndex(i) == hostIndex)
            {
                clientMask |= 1ULL << i;
            }
        }
        WriteDestMask(group, clientMask);
        for (i = 0; i < netCurMaxPlayers; ++i)
        {
            unsigned __int64 bit = clientMask & (1ULL << i);
            if (bit && i != player)
            {
                WriteAddr(&group, &clientAddrs[i]);
            }
        }

        int hostType = *NetGameP::GetHostMode(player);
        bool dedicated = hostType == 2;
        group.Write(dedicated);
        unsigned int hostMask = 0;
        for (i = 0; i < netCurMaxHosts; ++i)
        {
            if (*NetGameP::GetHostMode(i) == hostType)
            {
                hostMask |= 1 << i;
            }
        }
        group.WriteBits(hostMask, 3);
        for (i = 0; i < netCurMaxHosts; ++i)
        {
            if ((hostMask & (1 << i)) && i != hostIndex)
            {
                WriteAddr(&group, &hostAddrs[i]);
            }
        }
    }

    for (i = 0; i < 2; ++i)
    {
        unsigned int maxReinforcements = Team::sTeamMaxReinforcementsOverride[i];
        group.WriteBits(maxReinforcements, 32);
    }
    NetGame::WriteSpectator(group);
}

// FUNCTION: SWBF2 0x005e1290
void ReadLaunch(NetPktGroup& group, bool& bAcked)
{
    bool flagA, flagB;
    group.Read(flagA);
    group.Read(flagB);
    if (!NetGame::IsHost())
    {
        netAutoAssignTeams = flagA;
        netAutoAssignTeamsOriginalSetting = flagB;
    }
    group.Read(bAcked);

    unsigned __int64 playerMask;
    ReadDestMask(group, playerMask);
    UpdateCurMaxPlayers(playerMask, false);
    NetGameP::SetPlaying(playerMask, 0);

    char mission[64];
    ReadString(group, mission, 64);
    NetSessionId session;
    ReadSessionId(&group, (unsigned int*)&session.id);
    if (!NetComm::IsHost())
    {
        GameLoop::SetNextMission(mission);
        CopyNetSessionIdWords((unsigned int*)&netSessionId.id, 0, (const unsigned int*)&session.id);
    }

    group.Read(netHeroesEnabled);
    if (netHeroesEnabled)
    {
        group.ReadBits((unsigned int&)netHeroRuleUnlock, 3);
        group.ReadBits((unsigned int&)netHeroRuleUnlockVal, 9);
        group.ReadBits((unsigned int&)netHeroRulePlayer, 2);
        group.ReadBits((unsigned int&)netHeroRuleRespawn, 2);
        group.ReadBits((unsigned int&)netHeroRuleRespawnVal, 9);
    }
    group.ReadBits((unsigned int&)hostVoteThreshold, 8);
    group.Read(netTeamDamage);
    group.Read(netAutoAim);
    group.ReadBits((unsigned int&)gSpawnInvicibility, 8);
    group.Read(netShowNames);

    unsigned int value;
    int i;
    if (!NetComm::IsHost())
    {
        bool isHostClient;
        group.Read(isHostClient);
        if (isHostClient)
        {
            group.ReadBits(value, 2);
            int hostIndex = value;
            NetGameP::GetHostIndex() = hostIndex;

            unsigned int address[6];
            ClearGalaxyIdWords(address);
            NetSocket::SetAddr(address, 0, 0);
            unsigned int noPeer[2];
            SetGalaxyIdWords(noPeer, 0, 0, 0);
            CopyNetSessionIdWords(address, 0, noPeer);
            NetCommP::Connect(hostIndex + 64, address);

            unsigned __int64 clientMask;
            ReadDestMask(group, clientMask);
            for (i = 0; i < netCurMaxPlayers; ++i)
            {
                if (clientMask & (1ULL << i))
                {
                    *NetGameP::GetClientOfIndex(i) = hostIndex;
                    if (i == NetComm::GetLocalIndex(0))
                    {
                        NetSocket::SetAddr(address, 0, 0);
                        unsigned int self[2];
                        SetGalaxyIdWords(self, 0, 0, 0);
                        CopyNetSessionIdWords(address, 0, self);
                    }
                    else
                    {
                        NetAddr peer;
                        ReadAddr(&group, &peer);
                        peer.GetAddr(address, 0);
                    }
                    NetCommP::Connect(i, address);
                }
                else
                {
                    *NetGameP::GetClientOfIndex(i) = -1;
                }
            }

            bool dedicated;
            group.Read(dedicated);
            int hostType = dedicated ? 2 : 1;
            *NetGameP::GetHostMode(hostIndex) = hostType;
            unsigned int hostMask;
            group.ReadBits(hostMask, 3);
            UpdateCurMaxHosts(hostMask, false);
            for (i = 0; i < netCurMaxHosts; ++i)
            {
                if (hostMask & (1 << i))
                {
                    *NetGameP::GetHostMode(i) = hostType;
                    if (i == hostIndex)
                    {
                        NetSocket::SetAddr(address, 0, 0);
                        unsigned int self[2];
                        SetGalaxyIdWords(self, 0, 0, 0);
                        CopyNetSessionIdWords(address, 0, self);
                    }
                    else
                    {
                        NetAddr peer;
                        ReadAddr(&group, &peer);
                        peer.GetAddr(address, 0);
                    }
                    NetCommP::Connect(i + 64, address);
                }
                else
                {
                    *NetGameP::GetHostMode(i) = 3;
                }
            }
        }
        else
        {
            group.ReadBits(value, 2);
            int clientOf = value;
            *NetGameP::GetClientOfIndex(NetComm::GetLocalIndex(0)) = clientOf;
            if (clientOf > 0)
            {
                UpdateCurMaxHosts(1 << clientOf, true);
                NetAddr host;
                ReadAddr(&group, &host);
                unsigned int hostAddress[6];
                ClearGalaxyIdWords(hostAddress);
                host.GetAddr(hostAddress, 0);
                NetCommP::Connect(clientOf + 64, hostAddress);
            }
        }
    }

    for (i = 0; i < 2; ++i)
    {
        group.ReadBits(value, 32);
        int maxReinforcements = value;
        Team::sTeamMaxReinforcementsOverride[i] = maxReinforcements;
    }
    NetGame::ReadSpectator(group);
}

// FUNCTION: SWBF2 0x005e1770
void CNetLaunchGame::Update()
{
    SetupSleep();
    NetComm::AdvanceTime(true);
    float now = NetComm::GetTime();
    NetComm::Update();

    while (true)
    {
        NetPacket* stale = NetComm::RecvRange(PKT_FIND, PKT_SHELL_UPDATE);
        if (!stale)
        {
            break;
        }
        stale->Free();
    }
    if (launchHost)
    {
        while (true)
        {
            NetPacket* loadAck = NetComm::Recv(PKT_LOAD_ACK, PKT_NONE);
            if (!loadAck)
            {
                break;
            }
            loadAck->Free();
        }
    }

    if (launchHost)
    {
        for (int send = 0; send < 8; ++send)
        {
            float oldest = now + 1.0f;
            int player = -1;
            for (int i = 0; i < netCurMaxPlayers; ++i)
            {
                if (!NetComm::IsConnected(i))
                {
                    continue;
                }
                if (NetGame::GetGuestOf(i) >= 0)
                {
                    continue;
                }
                if (sendTimes[i] > now - 0.2f)
                {
                    continue;
                }
                if (oldest > sendTimes[i])
                {
                    oldest = sendTimes[i];
                    player = i;
                }
            }
            if (player == -1)
            {
                break;
            }
            NetPktGroup group;
            group.BeginWrite();
            WriteLaunch(group, launchAcks[player], player, false);
            group.EndWrite();
            NetComm::Send(player, PKT_LAUNCH, group.pktPtr);
            sendTimes[player] = now;
        }
    }

    bool done;
    while (true)
    {
        NetPacket* launch = NetComm::Recv(PKT_LAUNCH, PKT_NONE);
        if (!launch)
        {
            break;
        }
        NetPktGroup group;
        group.Set(launch);
        group.BeginRead();
        if (!launchHost && !launchDone)
        {
            ReadLaunch(group, done);
            if (done)
            {
                launchDone = true;
            }
        }
        group.EndRead();
        launch->Free();
        group.BeginWrite();
        group.WriteBits(0, 1);
        group.EndWrite();
        NetComm::Send(64, PKT_LAUNCH_ACK, group.pktPtr);
    }

    if (launchHost)
    {
        while (true)
        {
            NetPacket* ack = NetComm::Recv(PKT_LAUNCH_ACK, PKT_NONE);
            if (!ack)
            {
                break;
            }
            int player = GetPacketSourceHost(ack);
            if (!launchAcks[player])
            {
                launchAcks[player] = true;
                for (unsigned int slot = 0; slot < 1; ++slot)
                {
                    int guest = NetGame::GetMyGuest(player, slot);
                    if (guest >= 0)
                    {
                        launchAcks[guest] = true;
                    }
                }
            }
            ack->Free();
        }
        if (!launchAcked)
        {
            bool allAcked = true;
            for (int i = 0; i < netCurMaxPlayers; ++i)
            {
                if (NetComm::IsConnected(i) && !launchAcks[i])
                {
                    allAcked = false;
                    break;
                }
            }
            if (allAcked || now > startTime + 15.0f)
            {
                launchAcked = true;
                ackedTimer = now + 0.5f;
            }
        }
        else if (now > ackedTimer)
        {
            launchDone = true;
        }
    }
    else
    {
        if (now > startTime + 5.0f)
        {
            launchDone = true;
        }
        while (true)
        {
            NetPacket* update = NetComm::Recv(PKT_GAME_UPDATE, PKT_NONE);
            if (!update)
            {
                break;
            }
            launchDone = true;
            update->Free();
        }
    }
}

// FUNCTION: SWBF2 0x005e5700
void ReadJoin(NetPktGroup& group, int player, unsigned int guestCount, ShellState& state)
{
    unsigned int value;
    group.ReadBits(value, 2);
    int arch = value;
    NetCommP::SetArch(player, (NetArch)arch);

    char name[32];
    ReadString(group, name, sizeof(name));
    NetGameP::SetPlayerName(player, name);
    unsigned __int64 id;
    group.ReadBytes((char*)&id, sizeof(id));
    NetGame::SetPlayerGSID(player, id);
    NetGame::ClearPlayerCDKey(player);

    unsigned int i;
    for (i = 0; i < guestCount; ++i)
    {
        char guestName[32];
        ReadString(group, guestName, sizeof(guestName));
        NetGameP::SetPlayerName(NetGame::GetMyGuest(player, i), guestName);
    }

    unsigned int playerId;
    int j;
    group.ReadBits(playerId, 32);
    state.players[player].playerId = netRandom.GetInt();
    bool newSession = !IsGalaxyIdValid(&netPrevSessionId.id);
    if (!newSession && playerId)
    {
        for (j = 0; j < 64; ++j)
        {
            if (intraSessionPlayers[j].playerId == playerId)
            {
                state.players[player].playerId = playerId;
                break;
            }
        }
    }
    NetGameP::SetKey(player, state.players[player].playerId);
    for (i = 0; i < guestCount; ++i)
    {
        int guest = NetGame::GetMyGuest(player, i);
        group.ReadBits(playerId, 32);
        state.players[guest].playerId = netRandom.GetInt();
        bool guestNewSession = !IsGalaxyIdValid(&netPrevSessionId.id);
        if (!guestNewSession && playerId)
        {
            for (j = 0; j < netCurMaxPlayers; ++j)
            {
                if (intraSessionPlayers[j].playerId == playerId)
                {
                    state.players[guest].playerId = playerId;
                    break;
                }
            }
        }
    }

    bool crashed;
    group.Read(crashed);
    if (crashed)
    {
        unsigned int crashAddr;
        group.ReadBits(crashAddr, 32);
        FILE* file = fopen("crashes.txt", "ta");
        if (file)
        {
            fprintf(file, "player %s had crash at 0x%x\n", NetGame::GetPlayerName(player), crashAddr);
            fclose(file);
        }
    }

    bool replica, relay;
    group.Read(replica);
    group.Read(relay);
    NetAddr clientAddr;
    ReadAddr(&group, &clientAddr);
    NetAddr seenHostAddr;
    ReadAddr(&group, &seenHostAddr);
    bool spectator = false;
    group.Read(spectator);
    if (spectator)
    {
        int none = 0;
        NetGame::SetSpectator(player, none);
        int team = NetGame::GetStartTeam(none);
        NetGame::SetStartTeam(player, team);
    }
    NetMatch::HostGetAddr(&seenHostAddr, &clientAddr, player);
    if (NetGame::GetJoystickIndex(player) < 0)
    {
        clientAddrs[player].CopyFieldsFrom(&clientAddr);
        if (IsSameNet((const unsigned int*)&clientAddrs[player], (const unsigned int*)&seenHostAddr))
        {
            NetCommP::GetPipe(player) = PIPE_LAN;
        }
        else
        {
            NetCommP::GetPipe(player) = PIPE_INTERNET;
        }
    }

    if (NetGame::InShell())
    {
        NetGameP::SetPlayerCanHost(player, replica, relay);
        *NetGameP::GetClientOfIndex(player) = 0;
        if (!replica && !relay)
        {
            *NetGameP::GetClientOfIndex(player) = 0;
        }
    }
    else
    {
        NetGameP::SetPlayerCanHost(player, 0, 0);
        *NetGameP::GetClientOfIndex(player) = 0;
    }
    for (unsigned int k = 0; k < 1; ++k)
    {
        if (NetGame::GetMyGuest(player, k) >= 0)
        {
            int guest = NetGame::GetMyGuest(player, k);
            NetGameP::SetPlayerCanHost(guest, 0, 0);
            *NetGameP::GetClientOfIndex(guest) = *NetGameP::GetClientOfIndex(player);
            NetCommP::GetPipe(guest) = NetComm::GetPipe(player);
        }
    }
}

// FUNCTION: SWBF2 0x005e5fc0
void WriteShellUpdate(NetPktGroup& group, int player, ShellUpdate& update, ShellState& state)
{
    group.WriteBits(player, 6);
    unsigned int guestCount = 0;
    unsigned int i;
    for (i = 0; i < 1; ++i)
    {
        if (NetGame::GetMyGuest(player, i) >= 0)
        {
            ++guestCount;
        }
    }
    group.WriteBits(guestCount, 3);
    for (i = 0; i < guestCount; ++i)
    {
        group.WriteBits(NetGame::GetMyGuest(player, i), 6);
    }
    WriteAddr(&group, &clientAddrs[player]);

    group.Write(dragEnabled);
    if (dragEnabled)
    {
        group.WriteBits(dragLatency, 12);
        group.WriteBits(dragBandwidth, 12);
        group.WriteBits(dragLoss, 7);
        group.WriteBits(dragJitter, 7);
    }
    group.Write(netIsDedicated);
    group.WriteBits(NetComm::GetMyArch(), 2);
    int localIndex = NetComm::GetLocalIndex(0);
    bool hasLocal = localIndex >= 0;
    group.Write(hasLocal);
    if (localIndex >= 0)
    {
        group.WriteBits(localIndex, 6);
    }

    ShellPlayer& self = state.players[player];
    bool booted = (update.bootMask & (1ULL << player)) != 0;
    group.Write(booted);
    bool quitKnown = state.canceled || state.players[player].quitRequested;
    group.Write(quitKnown);
    if (quitKnown)
    {
        bool quit = state.players[player].quitRequested;
        group.Write(quit);
    }

    int ping = NetComm::GetPing(64);
    group.WriteBits(ping, 10);
    int up, down;
    NetComm::GetBandwidth(64, up, down);
    down = up;
    group.WriteBits(up / 1024, 12);
    group.WriteBits(down / 1024, 12);

    int j;
    for (j = 0; j < 64; ++j)
    {
        if (!NetComm::IsConnected(j))
        {
            group.Write(false);
            continue;
        }
        group.Write(true);
        ShellPlayer& other = state.players[j];
        bool guest = NetGame::GetGuestOf(j) >= 0;
        group.Write(guest);
        if (guest)
        {
            group.WriteBits(NetGame::GetGuestOf(j), 6);
        }
        else
        {
            ping = NetComm::GetPing(j);
            group.WriteBits(ping, 10);
            NetComm::GetBandwidth(j, up, down);
            group.WriteBits(up / 1024, 12);
            group.WriteBits(down / 1024, 12);
        }
        if (self.ackedUpdate <= other.changedUpdate)
        {
            group.Write(true);
            char name[32];
            strlcpy(name, NetGame::GetPlayerName(j), sizeof(name));
            WriteString(&group, name, 32);
            group.Write(netAutoAssignTeams);
            group.Write(netAutoAssignTeamsOriginalSetting);
            bool teamOne = NetGame::GetStartTeam(j) == 1;
            group.Write(teamOne);
            unsigned __int64 id = NetGame::GetPlayerGSID(j);
            group.WriteBytes((const char*)&id, sizeof(id));
            if (!guest)
            {
                WriteAddr(&group, &clientAddrs[j]);
            }
        }
        else
        {
            group.Write(false);
        }
    }

    bool hostPostLoad = update.postLoad && !loadGame;
    group.Write(hostPostLoad);
    unsigned int turn = 0;
    if (update.postLoad)
    {
        turn = NetGame::GetTurnNumber();
    }
    group.WriteBits(turn, 21);
    group.WriteBits(state.players[player].playerId, 32);
    for (i = 0; i < guestCount; ++i)
    {
        group.WriteBits(state.players[NetGame::GetMyGuest(player, i)].playerId, 32);
    }
    if (player != NetGame::GetLocalIndex(0))
    {
        NetGameP::SetKey(64, intraSessionPlayerId0);
    }

    unsigned __int64 playingMask = 0;
    if (update.postLoad)
    {
        for (j = 0; j < netCurMaxPlayers; ++j)
        {
            if (NetGame::IsPlaying(j, false))
            {
                playingMask |= 1ULL << j;
            }
        }
    }
    WriteDestMask(group, playingMask);
    WriteString(&group, GameLoop::GetNextMission(), 64);

    unsigned int team1, team2;
    MissionPlayList::GetCurrentTeamNames(team1, team2);
    group.WriteBits(team1, 32);
    group.WriteBits(team2, 32);
    unsigned int mission;
    for (mission = MissionPlayList::GetFirstMission(); mission; mission = MissionPlayList::GetNextMission())
    {
        if (DownloadableContent::IsMissionDownloaded(mission))
        {
            group.WriteBits(mission, 32);
        }
    }
    group.WriteBits(mission, 32);
    group.WriteBits(update.moveNum, 32);

    int turnRate = 20;
    unsigned int turnScale = 1;
    turnRate = netFPS;
    if (useProfile)
    {
        PlayerProfile::tProfile* profile = PlayerProfile::GetCurProfile();
        if (profile)
        {
            turnRate = profile->mOnlineOptions.mu8TurnsPerSecond;
        }
    }
    if (turnRate < 10)
    {
        turnRate = netFPS;
    }
    group.WriteBits(turnRate, 10);
    group.WriteBits(turnScale, 3);
    WriteFiles(group, state.players[player].patchFile);
}

// FUNCTION: SWBF2 0x005e6690
bool ReadShellUpdate(NetPktGroup& group, ShellUpdate& update, ShellMove& move)
{
    bool changed = false;
    unsigned int value;
    group.ReadBits(value, 6);
    int host = value;
    unsigned int guestCount;
    group.ReadBits(guestCount, 3);
    int guests[1];
    unsigned int i;
    for (i = 0; i < 1; ++i)
    {
        if (i < guestCount)
        {
            group.ReadBits(value, 6);
            guests[i] = value;
        }
        else
        {
            guests[i] = -1;
        }
    }

    bool isLocal = host == NetComm::GetLocalIndex(0);
    if (!isLocal)
    {
        NetCommP::GetLocalIndex(0) = host;
        UpdateCurMaxPlayers(1ULL << host, true);
        for (i = 0; i < guestCount; ++i)
        {
            NetCommP::GetLocalIndex(i + 1) = guests[i];
            NetGameP::SetGuestOf(guests[i], host);
            UpdateCurMaxPlayers(1ULL << guests[i], true);
        }
        changed = true;
    }

    NetAddr address;
    ReadAddr(&group, &address);
    NetMatch::ClientGetAddr();

    bool hasLimits;
    group.Read(hasLimits);
    if (hasLimits)
    {
        unsigned int limitA, limitB, limitC, limitD;
        group.ReadBits(limitA, 12);
        group.ReadBits(limitB, 12);
        group.ReadBits(limitC, 7);
        group.ReadBits(limitD, 7);
        if (!NetComm::IsHost() && !dragEnabled)
        {
            dragEnabled = hasLimits;
            dragLatency = limitA;
            dragBandwidth = limitB;
            dragLoss = limitC;
            dragJitter = limitD;
        }
    }
    bool flag;
    group.Read(flag);
    if (!isLocal)
    {
        netIsHostDedicated = flag;
    }
    group.ReadBits(value, 2);
    if (!isLocal)
    {
        NetCommP::SetArch(64, (NetArch)value);
    }

    bool hasHostPlayer;
    group.Read(hasHostPlayer);
    if (hasHostPlayer)
    {
        group.ReadBits(value, 6);
        hostPlayer = value;
        if (!NetComm::IsConnected(hostPlayer))
        {
            NetCommP::Connect(hostPlayer, NetCommP::GetAddr(64));
        }
    }

    bool booted;
    group.Read(booted);
    update.bootMask = 0;
    if (booted)
    {
        update.bootMask = 1ULL << host;
    }
    update.quitMask = 0;
    bool quitKnown;
    group.Read(quitKnown);
    if (quitKnown)
    {
        bool quit;
        group.Read(quit);
        if (quit)
        {
            update.quitMask = 1ULL << host;
        }
        else
        {
            move.quitRequested = true;
        }
    }

    group.ReadBits(value, 10);
    int ping = value;
    group.ReadBits(value, 12);
    int up = value << 10;
    group.ReadBits(value, 12);
    int down = value << 10;
    if (!NetGame::IsHost())
    {
        int oldUp, oldDown;
        NetComm::GetBandwidth(64, oldUp, oldDown);
        if (up != oldUp || down != oldDown)
        {
            changed = true;
            NetComm::SetBandwidth(64, up, down);
        }
        NetComm::SetPing(64, ping);
    }

    unsigned __int64* connectMask = &update.connectMask;
    *connectMask = 0;
    for (int j = 0; j < 64; ++j)
    {
        bool present;
        group.Read(present);
        if (present)
        {
            *connectMask |= 1ULL << j;
            if (!NetComm::IsConnected(j))
            {
                changed = true;
            }
            bool guest;
            group.Read(guest);
            if (guest)
            {
                unsigned int owner;
                group.ReadBits(owner, 6);
                NetGameP::SetGuestOf(j, owner);
            }
            else
            {
                group.ReadBits(value, 10);
                ping = value;
                group.ReadBits(value, 12);
                up = value << 10;
                group.ReadBits(value, 12);
                down = value << 10;
                if (!NetGame::IsHost())
                {
                    int curUp, curDown;
                    NetComm::GetBandwidth(j, curUp, curDown);
                    if (up != curUp || down != curDown)
                    {
                        changed = true;
                        NetComm::SetBandwidth(j, up, down);
                    }
                    NetComm::SetPing(j, ping);
                }
            }

            bool hasInfo;
            group.Read(hasInfo);
            if (hasInfo)
            {
                char name[32];
                ReadString(group, name, sizeof(name));
                bool flagA, flagB, teamOne;
                group.Read(flagA);
                group.Read(flagB);
                group.Read(teamOne);
                unsigned __int64 id;
                group.ReadBytes((char*)&id, sizeof(id));
                NetAddr peer;
                if (!guest)
                {
                    ReadAddr(&group, &peer);
                }
                if (!NetGame::IsHost())
                {
                    if (strcmp(NetGame::GetPlayerName(j), name))
                    {
                        changed = true;
                        NetGameP::SetPlayerName(j, name);
                    }
                    netAutoAssignTeams = flagA;
                    netAutoAssignTeamsOriginalSetting = flagB;
                    bool wasTeamOne = NetGame::GetStartTeam(j) == 1;
                    if (wasTeamOne != teamOne)
                    {
                        changed = true;
                        NetGame::SetStartTeam(j, teamOne ? 1 : 2);
                    }
                    if (NetGame::GetPlayerGSID(j) != id)
                    {
                        changed = true;
                        NetGame::SetPlayerGSID(j, id);
                        NetGame::ClearPlayerCDKey(j);
                    }
                    if (!guest)
                    {
                        clientAddrs[j].CopyFieldsFrom(&peer);
                    }
                }
            }
        }
        else if (!NetGame::IsHost() && NetComm::IsConnected(j))
        {
            changed = true;
            NetGameP::SetPlayerName(j, "");
            NetGame::SetPlayerGSID(j, 0);
            NetGame::ClearPlayerCDKey(j);
        }
    }
    NetGameP::SetPlaying(*connectMask, 0);
    UpdateCurMaxPlayers(*connectMask, true);

    bool hostLoadDone;
    group.Read(hostLoadDone);
    update.hostLoadDone = hostLoadDone;
    unsigned int turn;
    group.ReadBits(turn, 21);
    update.turnNumber = turn;
    group.ReadBits(intraSessionPlayerId0, 32);
    for (i = 0; i < guestCount; ++i)
    {
        group.ReadBits(intraSessionPlayerIdRest[i], 32);
    }
    if (!isLocal)
    {
        NetGameP::SetKey(64, intraSessionPlayerId0);
    }

    unsigned __int64 playingMask;
    ReadDestMask(group, playingMask);
    char mission[64];
    ReadString(group, mission, sizeof(mission));
    unsigned int team1, team2;
    group.ReadBits(team1, 32);
    group.ReadBits(team2, 32);
    if (!isLocal)
    {
        MissionPlayList::SetCurrentTeamNames(team1, team2);
    }
    unsigned int missionHash;
    group.ReadBits(missionHash, 32);
    while (missionHash)
    {
        if (!DownloadableContent::IsMissionDownloaded(missionHash))
        {
            netJoinUpdatePending = true;
        }
        group.ReadBits(missionHash, 32);
    }
    unsigned int moveNum;
    group.ReadBits(moveNum, 32);
    unsigned int turnRate, turnScale;
    group.ReadBits(turnRate, 10);
    group.ReadBits(turnScale, 3);
    if (!isLocal)
    {
        NetGameP::SetTurnRate(turnRate, turnScale);
    }
    ReadFiles(group, update.patchFile);
    return changed;
}

// FUNCTION: SWBF2 0x005dfc50
void CNetJoinGame::Update()
{
    int entryMark = 733;
    PblJournal::Record(entryMark, PJT_FULL);
    NetComm::AdvanceTime(true);
    float now = NetComm::GetTime();
    PblJournal::Record(now, PJT_FULL);
    NetComm::Update();

    if (connecting)
    {
        if (connected)
        {
            connecting = false;
            hostAddrs[0].CopyFieldsFrom(&hostAddr);
            int connectMark = 736;
            PblJournal::Record(connectMark, PJT_FULL);
            unsigned int address[6];
            ClearGalaxyIdWords(address);
            hostAddr.GetAddr(address, 0);
            NetCommP::Connect(64, address);
            for (unsigned int i = 0; i < 2; ++i)
            {
                NetCommP::GetLocalIndex(i) = -1;
            }
            delete (NetFindGame*)cFind;
            cFind = 0;
        }
        else
        {
            int reason;
            if (NetMatch::UpdateConnect(&hostAddr, &hostKey, 64, &reason))
            {
                if (reason == 13)
                {
                    connected = true;
                }
                else if (NetComm::IsLAN())
                {
                    connecting = false;
                    joinFailed = true;
                    rejectReason = JRR_CONNECTION_LOST;
                    JRR_to_Errorbox((JoinRejectReason)rejectReason);
                }
            }
            else
            {
                connecting = false;
                joinFailed = true;
                rejectReason = reason;
                JRR_to_Errorbox((JoinRejectReason)rejectReason);
            }
        }
        NetComm::Update();
    }
    else if (!joinFailed)
    {
        int localIndex = NetComm::GetLocalIndex(0);
        PblJournal::Record(localIndex, PJT_FULL);
        PblJournal::Record(timer, PJT_FULL);
        if (NetComm::GetLocalIndex(0) == -1)
        {
            if (now > timer && !shellMove.quitRequested)
            {
                ++tryCount;
                if (tryCount > 12)
                {
                    timer = now + 3.0f;
                }
                else if (tryCount > 6)
                {
                    timer = now + 1.0f;
                }
                else
                {
                    timer = now + 0.5f;
                }
                NetPktGroup group;
                group.BeginWrite();
                group.WriteBits(1, 12);
                unsigned int sequence = NetSetup::GetPassword();
                group.WriteBits(sequence, 32);
                unsigned int guestCount = 0;
                guestCount = GameLoop::GetNumCameras() - 1;
                WriteJoin(group, guestCount);
                group.EndWrite();
                NetComm::Send(64, PKT_JOIN, group.pktPtr);
                int joinMark = 735;
                PblJournal::Record(joinMark, PJT_FULL);
            }
        }
        else if (now > timer)
        {
            bool sendLaunchAck = true;
            if (sendLaunchAck && (hostLaunched || (IsHostPlaying() && shellUpdate.patchFile.done)))
            {
                timer = now + 0.2f;
                NetPktGroup group;
                group.BeginWrite();
                group.WriteBits(0, 1);
                group.EndWrite();
                NetComm::Send(64, PKT_LAUNCH_ACK, group.pktPtr);
            }
            else
            {
                ++tryCount;
                if (shellMove.quitRequested)
                {
                    timer = now + 0.2f;
                }
                else if (!shellUpdate.patchFile.done)
                {
                    timer = now + 0.05f;
                }
                else if (tryCount > 12)
                {
                    timer = now + 3.0f;
                }
                else if (tryCount > 6)
                {
                    timer = now + 1.0f;
                }
                else
                {
                    timer = now + 0.2f;
                }
                NetPktGroup group;
                group.BeginWrite();
                shellMove.moveNum = moveNum++;
                shellMove.launchGuestHost = false;
                shellMove.isTeamOne = NetGame::GetStartTeam(NetComm::GetLocalIndex(0)) == 1;
                shellMove.hostRequest = hostRequest;
                WriteShellMove(group, shellUpdate, shellMove);
                group.EndWrite();
                NetComm::Send(64, PKT_SHELL_MOVE, group.pktPtr);
            }
        }

        while (true)
        {
            NetPacket* reject = NetComm::Recv(PKT_REJECT, PKT_NONE);
            if (!reject)
            {
                break;
            }
            NetPktGroup group;
            group.Set(reject);
            group.BeginRead();
            if (!joinFailed)
            {
                ReadJoinReject(group, *(JoinRejectReason*)&rejectReason);
                if (rejectReason == JRR_BUSY)
                {
                    rejectReason = 0;
                }
                else
                {
                    joinFailed = true;
                }
            }
            group.EndRead();
            group.Free();
        }

        netJoinUpdatePending = false;
        while (true)
        {
            NetPktGroup group;
            if (!NetComm::Recv(&group, PKT_SHELL_UPDATE))
            {
                break;
            }
            int oldLocal = NetComm::GetLocalIndex(0);
            group.BeginRead();
            if (ReadShellUpdate(group, shellUpdate, shellMove))
            {
                ++changeCount;
            }
            hostLaunched = shellUpdate.hostLaunched;
            if (shellUpdate.moveNum > ackedMoveNum)
            {
                ackedMoveNum = shellUpdate.moveNum;
            }
            group.EndRead();
            group.Free();
            UpdateCurMaxPlayers(shellUpdate.connectMask, false);

            int newLocal = NetComm::GetLocalIndex(0);
            if (newLocal != oldLocal)
            {
                timer = 0.0f;
                tryCount = 0;
                int joystick = FLInputManager::Instance()->GetHWPort(0);
                void* node = joystick < 0 ? 0 : GameVoiceChat::GetNode();
                void* voiceNode = node;
                if (voiceNode)
                {
                    unsigned char channel = (unsigned char)newLocal;
                    ((GameVoiceChat::Node*)voiceNode)->SetID(channel);
                    GameVoiceChat::Transport* transport = GameVoiceChat::GetTransport();
                    transport->SetSendEnable(channel, channel, false, false);
                    transport->SetReceiveEnable(channel, channel, false, false);
                }
            }
            if (shellUpdate.bootMask)
            {
                joinFailed = true;
                rejectReason = JRR_BOOTED;
                JRR_to_Errorbox((JoinRejectReason)rejectReason);
            }
        }

        if (NetGame::GetLocalIndex(0) >= 0)
        {
            while (true)
            {
                NetPacket* launch = NetComm::Recv(PKT_LAUNCH, PKT_NONE);
                if (!launch)
                {
                    break;
                }
                if (!hostLaunched)
                {
                    NetPktGroup group;
                    group.Set(launch);
                    group.BeginRead();
                    bool acked;
                    ReadLaunch(group, acked);
                    group.EndRead();
                    hostLaunched = true;
                }
                if (launch)
                {
                    launch->Free();
                }
            }
        }

        while (true)
        {
            NetPacket* packet = NetComm::Recv(PKT_GAME_UPDATE, PKT_NONE);
            if (!packet)
            {
                packet = NetComm::Recv(PKT_LOAD, PKT_NONE);
            }
            if (!packet)
            {
                break;
            }
            packet->Free();
            NetComm::Disconnect(64);
        }

        float timeout = NetComm::GetLocalIndex(0) < 0 ? 60.0f : 15.0f;
        if (NetComm::IsConnected(64) && now - NetComm::GetLastRecv(64) >= timeout)
        {
            NetComm::Disconnect(64);
            ++changeCount;
            shellUpdate.connectMask = 0;
            if (NetComm::GetLocalIndex(0) >= 0)
            {
                shellUpdate.connectMask |= 1ULL << NetComm::GetLocalIndex(0);
            }
            int* hostIndex = &NetGameP::GetHostIndex();
            if (*hostIndex >= 0)
            {
                *NetGameP::GetHostMode(*hostIndex) = 3;
                *hostIndex = -1;
            }
            joinFailed = true;
            rejectReason = JRR_CONNECTION_LOST;
            JRR_to_Errorbox((JoinRejectReason)rejectReason);
        }
        if (netJoinUpdatePending)
        {
            NetComm::Disconnect(64);
            ++changeCount;
            shellUpdate.connectMask = 0;
            if (NetComm::GetLocalIndex(0) >= 0)
            {
                shellUpdate.connectMask |= 1ULL << NetComm::GetLocalIndex(0);
            }
            int* hostIndex = &NetGameP::GetHostIndex();
            if (*hostIndex >= 0)
            {
                *NetGameP::GetHostMode(*hostIndex) = 3;
                *hostIndex = -1;
            }
            joinFailed = true;
            rejectReason = JRR_MISSING_DC;
            JRR_to_Errorbox((JoinRejectReason)rejectReason);
        }
        if (!HasEnteredLobby() && !NetComm::IsLAN())
        {
            joinFailed = true;
        }
    }

    if (NetQOS::Update(true))
    {
        ++changeCount;
    }
    NetMatch::UpdateLogin(-1);
    NetMatch::UpdatePresence(false);
    int exitMark = 734;
    PblJournal::Record(exitMark, PJT_FULL);
}

// GLOBAL: SWBF2 0x007e8ee4
char* netGameVersion = "1.1";

void* GetGalaxyNetworkingInterface();
int Min(int a, int b);

// GLOBAL: SWBF2 0x01faa724
int netHostUpdateCount;

// GLOBAL: SWBF2 0x01ead900
int netFindQueryCount;

// FUNCTION: SWBF2 0x005dcf90
void CNetHostGame::SendUpdates()
{
    for (int tries = 0; needSendMask && tries < 4; ++tries)
    {
        const int start = (unsigned char)netRandom.GetInt();
        for (int j = 0; j < 64; ++j)
        {
            const int player = (start + j) % 64;
            if (!(needSendMask & (1ULL << player)))
            {
                continue;
            }
            if (NetComm::IsConnected(player) && NetGame::GetGuestOf(player) < 0)
            {
                ShellPlayer& shellPlayer = shellState.players[player];
                NetPktGroup group;
                group.BeginWrite();
                ShellUpdate update;
                update.postLoad = false;
                update.moveNum = shellPlayer.moveNum;
                update.bootMask = bootMask;
                if (shellPlayer.quitRequested)
                {
                    NetGameP::SetPlayerName(player, "");
                    update.quitMask = 1ULL << player;
                }
                WriteShellUpdate(group, player, update, shellState);
                group.EndWrite();
                NetComm::Send(player, PKT_SHELL_UPDATE, &group);
            }
            needSendMask &= ~(1ULL << player);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x005dc4e0
void CNetHostGame::Update()
{
    ++netHostUpdateCount;
    PblJournal::Record(netHostUpdateCount, PJT_FULL);
    NetComm::AdvanceTime(true);
    float now = NetComm::GetTime();
    NetComm::Update();
    int oldChangeCount = changeCount;

    while (true)
    {
        NetPacket* join = NetComm::Recv(PKT_JOIN, PKT_NONE);
        if (!join)
        {
            break;
        }
        int from = GetPacketSourceHost(join);
        if (from < 64)
        {
            join->Free();
            join = 0;
            if (shellState.players[from].quitRequested)
            {
                DropPlayer(from);
                break;
            }
            needSendMask |= 1ULL << from;
        }
        else if (!BanIP(from, join))
        {
            int player = -1;
            int i;
            for (i = 0; i < netCurMaxPlayers; ++i)
            {
                if (NetComm::IsConnected(i) && NetCommP::SameAddr(i, from))
                {
                    player = i;
                    break;
                }
            }
            if (player == -1)
            {
                unsigned int j;
                int guests[1];
                for (j = 0; j < 1; ++j)
                {
                    guests[j] = -1;
                }
                int slots = Min(netCurMaxPlayers + 2, netPlayerLimit);
                for (i = 0; i < slots; ++i)
                {
                    if (NetComm::IsConnected(i))
                    {
                        continue;
                    }
                    if (player == -1)
                    {
                        player = i;
                    }
                    else
                    {
                        for (j = 0; j < 1; ++j)
                        {
                            if (guests[j] == -1)
                            {
                                guests[j] = i;
                                break;
                            }
                        }
                    }
                }

                NetPktGroup group;
                group.Set(join);
                group.BeginRead();
                unsigned int version;
                group.ReadBits(version, 12);
                unsigned int password;
                group.ReadBits(password, 32);
                unsigned int guestCount;
                group.ReadBits(guestCount, 3);
                bool hasRoom = player != -1;
                for (j = 0; j < guestCount; ++j)
                {
                    if (guests[j] == -1)
                    {
                        hasRoom = false;
                    }
                }
                if (!hasRoom)
                {
                    group.EndRead();
                    join->Free();
                    join = 0;
                    group.BeginWrite();
                    WriteJoinReject(group, JRR_FULL);
                    group.EndWrite();
                    NetComm::Send(from, PKT_REJECT, &group);
                }
                else if (password != (unsigned int)NetSetup::GetPassword())
                {
                    group.EndRead();
                    join->Free();
                    join = 0;
                    group.BeginWrite();
                    WriteJoinReject(group, JRR_PASSWORD);
                    group.EndWrite();
                    NetComm::Send(from, PKT_REJECT, &group);
                }
                else if (version != 1)
                {
                    group.EndRead();
                    join->Free();
                    join = 0;
                    group.BeginWrite();
                    WriteJoinReject(group, JRR_VERSION);
                    group.EndWrite();
                    NetComm::Send(from, PKT_REJECT, &group);
                }
                else
                {
                    unsigned __int64* address = NetCommP::GetAddr(from);
                    NetCommP::Connect(player, address);
                    UpdateCurMaxPlayers(1ULL << player, true);
                    for (unsigned int k = 0; k < guestCount; ++k)
                    {
                        NetCommP::Connect(guests[k], address);
                        UpdateCurMaxPlayers(1ULL << guests[k], true);
                        NetGameP::SetGuestOf(guests[k], player);
                    }
                    ReadJoin(group, player, guestCount, shellState);
                    group.EndRead();
                    join->Free();
                    join = 0;
                    ++changeCount;
                    needSendMask |= 1ULL << from;
                    if (NetGame::IsDedicated())
                    {
                        launchTimer = NetComm::GetTime();
                    }
                }
            }
        }
        if (join)
        {
            join->Free();
        }
    }

    while (true)
    {
        NetPacket* packet = NetComm::Recv(PKT_SHELL_MOVE, PKT_NONE);
        if (!packet)
        {
            break;
        }
        int player = GetPacketSourceHost(packet);
        if (player)
        {
            ++gDebugCount;
        }
        NetPktGroup group;
        group.Set(packet);
        group.BeginRead();
        ShellMove move;
        if (ReadShellMove(group, player, move, shellState))
        {
            ++changeCount;
        }
        group.EndRead();
        packet->Free();
        ShellPlayer& shellPlayer = shellState.players[player];
        if (move.hostRequest != shellPlayer.hostRequest)
        {
            shellPlayer.hostRequest = move.hostRequest;
            if (SelectNextHost(player))
            {
                ++changeCount;
            }
        }
        shellPlayer.moveNum = move.moveNum;
        needSendMask |= 1ULL << player;
    }

    if (!launchReady)
    {
        bool newSession = !IsGalaxyIdValid(&netPrevSessionId.id);
        float delay = newSession ? 0.1f : 5.0f;
        if (now - launchTimer > delay)
        {
            launchReady = true;
        }
    }

    while (true)
    {
        NetPktGroup group;
        if (!NetComm::Recv(&group, PKT_SHELL_UPDATE))
        {
            break;
        }
        group.Free();
    }

    if (now > timer)
    {
        timer = now + 0.2f;
        if (NetComm::GetLocalIndex(0) >= 0)
        {
            NetPktGroup group;
            group.BeginWrite();
            ShellMove move;
            move.launchGuestHost = false;
            move.isTeamOne = NetGame::GetStartTeam(NetComm::GetLocalIndex(0)) == 1;
            move.quitRequested = false;
            WriteShellMove(group, shellUpdate, move);
            group.EndWrite();
            NetComm::Send(64, PKT_SHELL_MOVE, group.pktPtr);
        }
    }

    int oldPlayerCount = playerCount;
    playerCount = 0;
    unsigned __int64 joinedMask = 0;
    int i;
    for (i = 0; i < netCurMaxPlayers; ++i)
    {
        if (NetComm::IsConnected(i) && shellState.players[i].joinAck)
        {
            ++playerCount;
            joinedMask |= 1ULL << i;
        }
    }
    if (playerCount != oldPlayerCount)
    {
        ++changeCount;
    }
    if (NetQOS::Update(true))
    {
        ++changeCount;
    }

    for (i = 0; i < netCurMaxPlayers; ++i)
    {
        if (!NetComm::IsConnected(i))
        {
            continue;
        }
        float elapsed = NetComm::GetTime() - NetComm::GetLastRecv(i);
        float timeout = 15.0f;
        if (shellState.players[i].quitRequested)
        {
            timeout = 5.0f;
        }
        if (elapsed > timeout)
        {
            DropPlayer(i);
            ++changeCount;
        }
        else if ((bootMask & (1ULL << i)) && now > bootTime[i])
        {
            DropPlayer(i);
            ++changeCount;
        }
    }
    if (changeCount != oldChangeCount)
    {
        needSendMask = joinedMask;
    }
    SendUpdates();
}

// FUNCTION: SWBF2 0x005ddc90
bool CNetFindGame::Update()
{
    int entryMark = 533;
    PblJournal::Record(entryMark, PJT_FULL);
    SetupSleep();
    NetComm::AdvanceTime(true);
    NetComm::Update();
    NetMatch::UpdateFindGames();
    int pumpMark = 536;
    PblJournal::Record(pumpMark, PJT_FULL);
    float now = NetComm::GetTime();
    bool updated = false;
    bool doQueries = true;

    if (doQueries && !joinGame)
    {
        int gameCount = gameList._iCount;
        PblJournal::Record(gameCount, PJT_FULL);
        int queryBudget = 4;
        PblList<NetGameDesc>::Itor itor(gameList);
        int maxConnecting = 64;
        int connectingCount = 0;
        while (NetGameDesc* game = itor)
        {
            ++itor;
            if (!game->mValid)
            {
                continue;
            }
            if (game->mFindData.mConnected || game->mFindData.mConnecting)
            {
                ++connectingCount;
            }
        }

        if (NetComm::IsLAN())
        {
            itor = PblList<NetGameDesc>::Itor(gameList);
            while (itor && queryBudget > 0)
            {
                NetGameDesc* game = itor;
                ++itor;
                if (!game->mValid)
                {
                    continue;
                }
                FindGameData& data = game->mFindData;
                if (!data.mConnected && !data.mConnecting && gSelectionTimer + 1.0f > now)
                {
                    continue;
                }
                float connectTime = 5.0f;
                if (data.mQuerySeqNo >= 8)
                {
                    if (data.mConnected)
                    {
                        NetMatch::EndConnect();
                        data.mConnected = false;
                        data.mConnecting = false;
                    }
                    continue;
                }
                if (data.mResponseRecvTime > 0.0f && data.mQuerySeqNo >= 5)
                {
                    if (data.mConnected)
                    {
                        NetMatch::EndConnect();
                        data.mConnected = false;
                        data.mConnecting = false;
                    }
                    data.mQuerySeqNo = 8;
                    continue;
                }
                if (data.mConnecting && !data.mConnected && now > data.mConnectTimeout)
                {
                    NetMatch::EndConnect();
                    data.mConnected = false;
                    data.mConnecting = false;
                    data.mQuerySeqNo = 8;
                }
                PblJournal::Record(data.mQuerySentTime, PJT_FULL);
                int seqNo = data.mQuerySeqNo;
                if (now <= (float)(seqNo * seqNo) * 0.2f + data.mQuerySentTime)
                {
                    continue;
                }
                --queryBudget;
                PblJournal::Record(data.mConnected, PJT_FULL);
                if (!data.mConnected)
                {
                    PblJournal::Record(data.mConnecting, PJT_FULL);
                }
                if (data.mConnected)
                {
                    unsigned int address[6];
                    ClearGalaxyIdWords(address);
                    game->mAddr.GetAddr(address, (int)&game->mKey);
                    int dest = NetCommP::FindJoinSource((const int*)address);
                    if (dest >= 0)
                    {
                        if (data.mQuerySeqNo == 0)
                        {
                            data.mQuerySentTime = NetComm::GetTime();
                        }
                        NetPktGroup group;
                        group.BeginWrite();
                        group.Write(false);
                        group.WriteBits(data.mQueryId, 32);
                        group.WriteBits(data.mQuerySeqNo, 4);
                        group.EndWrite();
                        NetComm::Send(dest, PKT_QUERY, group.pktPtr);
                        ++netFindQueryCount;
                        ++data.mQuerySeqNo;
                    }
                }
                else if (data.mConnecting)
                {
                    int reason;
                    if (NetMatch::UpdateConnect(&game->mAddr, &game->mKey, 64, &reason))
                    {
                        if (reason == 13)
                        {
                            data.mConnected = true;
                            data.mConnecting = false;
                        }
                        else
                        {
                            NetMatch::EndConnect();
                            data.mConnecting = false;
                            data.mQuerySeqNo = 8;
                        }
                    }
                }
                else if (connectingCount < maxConnecting)
                {
                    data.mConnecting = true;
                    data.mConnectTimeout = now + connectTime;
                    if (NetMatch::StartConnect())
                    {
                        data.mConnected = true;
                        data.mConnecting = false;
                    }
                    ++connectingCount;
                }
            }
        }

        int recvMark = 535;
        PblJournal::Record(recvMark, PJT_FULL);
        if (NetComm::IsLAN())
        {
            while (true)
            {
                NetPacket* response = NetComm::Recv(PKT_QUERY_RESPONSE, PKT_NONE);
                if (!response)
                {
                    break;
                }
                NetPktGroup group;
                group.Set(response);
                group.BeginRead();
                unsigned int queryId;
                group.ReadBits(queryId, 32);
                unsigned int seqNo;
                group.ReadBits(seqNo, 4);
                bool found = false;
                NetAddr address;
                FindGameData data;
                FindGameRules rules;
                if (findType == 2)
                {
                    mDirectJoinState[4] = true;
                    NetMatch::SetAddr((unsigned int*)&address, (const unsigned int*)(mDirectJoinState + 8));
                    AddGame(&address);
                }
                else
                {
                    for (itor = PblList<NetGameDesc>::Itor(gameList); itor; ++itor)
                    {
                        NetGameDesc* game = itor;
                        if (!game->mValid)
                        {
                            continue;
                        }
                        if (game->mFindData.mQueryId == queryId)
                        {
                            address.CopyFieldsFrom(&game->mAddr);
                            data = game->mFindData;
                            found = true;
                            break;
                        }
                    }
                }

                unsigned int tps = 0;
                PblJournal::Record(found, PJT_FULL);
                ReadString(group, data.mName, sizeof(data.mName));
                ReadString(group, data.mMapType, sizeof(data.mMapType));
                ReadString(group, data.mMapName, sizeof(data.mMapName));
                group.Read(rules.mAimAssist);
                group.Read(rules.mAutoTeams);
                group.Read(rules.mTeamDamage);
                group.Read(data.mLocked);
                unsigned int value;
                group.ReadBits(value, 8);
                rules.mNumAIUnits = value;
                group.ReadBits(value, 11);
                rules.mNumReinforcements1 = value;
                group.ReadBits(value, 11);
                rules.mNumReinforcements2 = value;
                group.ReadBits(value, 7);
                data.mNumPlayers = value;
                group.ReadBits(value, 7);
                data.mMaxPlayers = value;
                group.ReadBits(value, 3);
                data.mServerType = value;
                group.ReadBits(tps, 8);
                group.ReadBits(value, 7);
                rules.mMinPlayers = value;
                group.ReadBits(value, 2);
                rules.mAIDifficulty = value;
                group.Read(rules.mShowPlayerNames);
                group.ReadBits(value, 6);
                rules.mSpawnInvincibilityTime = value;
                group.Read(rules.mHeroes);
                if (rules.mHeroes)
                {
                    group.ReadBits(value, 4);
                    rules.mHeroUnlockRule = value;
                    group.ReadBits(value, 8);
                    rules.mHeroUnlockRuleVal = value;
                    group.ReadBits(value, 3);
                    rules.mHeroTeamRule = value;
                    group.ReadBits(value, 4);
                    rules.mHeroPlayerRule = value;
                    group.ReadBits(value, 2);
                    rules.mHeroRespawnRule = value;
                    group.ReadBits(value, 10);
                    rules.mHeroRespawnRuleVal = value;
                }
                group.EndRead();
                response->Free();

                if (findType == 2)
                {
                    UpdateGame(&address, 0, 0, NetSetup::GetGameVersion(), &data, &rules);
                }
                else if (found)
                {
                    NetGameDesc* game = itor;
                    float ping = now - ((float)(seqNo * seqNo) * 0.2f + game->mFindData.mQuerySentTime);
                    if (data.mPingTime > ping)
                    {
                        data.mPingTime = ping;
                    }
                    game->mStamp = NetComm::GetTime();
                    PlayerProfile::tProfile* profile = PlayerProfile::GetCurProfile();
                    int myTps = profile ? profile->mOnlineOptions.mu8TurnsPerSecond : 30;
                    unsigned char tpsLimit = (unsigned char)myTps;
                    bool tpsOk = tps <= tpsLimit;
                    bool versionOk = true;
                    game->mHasSessionData = tpsOk && versionOk;
                    game->mRulesAreValid = true;
                    game->mFindRules = rules;
                    game->mFindData.mPingTime = data.mPingTime;
                    game->mFindData.mLocked = data.mLocked;
                    game->mFindData.mServerType = data.mServerType;
                    strlcpy(game->mFindData.mName, data.mName, sizeof(game->mFindData.mName));
                    strlcpy(game->mFindData.mMapType, data.mMapType, sizeof(game->mFindData.mMapType));
                    strlcpy(game->mFindData.mMapName, data.mMapName, sizeof(game->mFindData.mMapName));
                    game->mFindData.mNumPlayers = data.mNumPlayers;
                    game->mFindData.mMaxPlayers = data.mMaxPlayers;
                    game->mFindData.mbIsFavorite = false;
                    memset((unsigned char*)game + 241, 0, 64);
                    if (game->mHasSessionData && GetGameCount() == 1)
                    {
                        curGame = itor;
                        unsigned int endpoint[6];
                        ClearGalaxyIdWords(endpoint);
                        NetGameDesc* selected = curGame;
                        selected->mAddr.GetAddr(endpoint, (int)&selected->mKey);
                        int ip, port;
                        NetSocket::GetAddr(endpoint, ip, port);
                    }
                }
                updated = true;
            }
        }
        else
        {
            for (itor = PblList<NetGameDesc>::Itor(gameList); itor; ++itor)
            {
                NetGameDesc* game = itor;
                if (!game->mValid)
                {
                    continue;
                }
                void* networking = GetGalaxyNetworkingInterface();
                unsigned __int64 peer;
                CopyGalaxyIdWords((unsigned int*)&peer, 0, (const unsigned int*)&game->mAddr);
                int ping = ((galaxy::api::INetworking*)networking)->GetPingWith(galaxy::api::GalaxyID(peer));
                game->mFindData.mPingTime = fmaxf(0.0f, (float)ping * 0.001f);
            }
        }
        int doneMark = 538;
        PblJournal::Record(doneMark, PJT_FULL);
    }

    RemoveOld(now - prevFindStartTime);
    NetMatch::UpdateLogin(-1);
    mPercentDone = NetMatch::GetPercentDone();
    int pumpDoneMark = 535;
    PblJournal::Record(pumpDoneMark, PJT_FULL);
    NetMatch::UpdatePresence(false);
    int exitMark = 534;
    PblJournal::Record(exitMark, PJT_FULL);
    if (netLoginError)
    {
        netInGame = false;
    }
    return updated;
}

// FUNCTION: SWBF2 0x005e1d50
void CNetLoadGame::Update()
{
    float now = NetComm::GetTime();
    int i;
    if (loadHost)
    {
        for (i = 0; i < 8; ++i)
        {
            int player = -1;
            float oldest = now;
            int j;
            for (j = 0; j < netCurMaxPlayers; ++j)
            {
                if (!NetGame::IsPlaying(j, false))
                {
                    continue;
                }
                if (NetGame::GetGuestOf(j) >= 0)
                {
                    continue;
                }
                if (sendTimes[j] > now - 0.2f)
                {
                    continue;
                }
                if (oldest > sendTimes[j])
                {
                    player = j;
                    oldest = sendTimes[j];
                }
            }
            if (player != -1)
            {
                NetPktGroup group;
                group.BeginWrite();
                unsigned __int64 playingMask = 0;
                for (j = 0; j < netCurMaxPlayers; ++j)
                {
                    if (NetGame::IsPlaying(j, false))
                    {
                        playingMask |= 1ULL << j;
                    }
                }
                WriteLoad(group, player, playingMask, ackedMask);
                group.EndWrite();
                NetComm::Send(player, PKT_LOAD, group.pktPtr);
                sendTimes[player] = now;
            }
        }
    }

    if (now > timer)
    {
        timer = now + 0.1f;
        if (NetComm::GetLocalIndex(0) >= 0)
        {
            NetPktGroup group;
            group.BeginWrite();
            group.WriteBits(0, 1);
            group.EndWrite();
            NetComm::Send(64, PKT_LOAD_ACK, group.pktPtr);
        }
    }

    if (loadHost)
    {
        while (true)
        {
            NetPacket* ack = NetComm::Recv(PKT_LOAD_ACK, PKT_NONE);
            if (!ack)
            {
                break;
            }
            unsigned int player = GetPacketSourceHost(ack);
            if (!(ackedMask & (1ULL << player)))
            {
                ackedMask |= 1ULL << player;
                for (int slot = 0; slot < 1; ++slot)
                {
                    int guest = NetGame::GetMyGuest(player, slot);
                    if (guest >= 0)
                    {
                        ackedMask |= 1ULL << guest;
                    }
                }
            }
            ack->Free();
        }
        while (true)
        {
            NetPacket* load = NetComm::Recv(PKT_LOAD, PKT_NONE);
            if (!load)
            {
                break;
            }
            load->Free();
        }
        if (!loadAcked)
        {
            if (now > 60.0f)
            {
                for (i = 0; i < netCurMaxPlayers; ++i)
                {
                    if (NetGame::IsPlaying(i, false) && !(ackedMask & (1ULL << i)))
                    {
                        NetGameP::SetNotPlaying(i);
                        NetComm::Disconnect(i);
                        GameVoiceChat::GetTransport()->DisconnectNode((unsigned char)i);
                    }
                }
                loadDone = true;
            }
            unsigned __int64 playingMask = 0;
            for (i = 0; i < netCurMaxPlayers; ++i)
            {
                if (NetGame::IsPlaying(i, false))
                {
                    playingMask |= 1ULL << i;
                }
            }
            if (!(playingMask & ~ackedMask))
            {
                loadAcked = true;
                ackedTimer = now + 0.3f;
            }
        }
        else if (now > ackedTimer)
        {
            loadDone = true;
        }
    }
    else
    {
        while (true)
        {
            NetPacket* launch = NetComm::Recv(PKT_LAUNCH, PKT_NONE);
            if (!launch)
            {
                break;
            }
            launch->Free();
        }
        while (true)
        {
            NetPacket* load = NetComm::Recv(PKT_LOAD, PKT_NONE);
            if (!load)
            {
                break;
            }
            NetPktGroup group;
            group.Set(load);
            if (!loadDone)
            {
                hostKnowsILoaded = true;
                group.BeginRead();
                ReadLoad(group, loadDone);
                group.EndRead();
            }
            group.Free();
        }
        if (!loadDone && now > 60.0f)
        {
            loadDone = true;
            NetComm::Disconnect(64);
        }
    }
}

// FUNCTION: SWBF2 0x005e3290
void CNetPostLoadHostGame::SendUpdates()
{
    for (int tries = 0; needSendMask && tries < 4; ++tries)
    {
        const int start = (unsigned char)netRandom.GetInt();
        for (int j = 0; j < 64; ++j)
        {
            const int player = (start + j) % 64;
            if (!(needSendMask & (1ULL << player)))
            {
                continue;
            }
            if (NetComm::IsConnected(player) && NetGame::GetGuestOf(player) < 0)
            {
                ShellPlayer& shellPlayer = shellState.players[player];
                NetPktGroup group;
                group.BeginWrite();
                ShellUpdate update;
                update.postLoad = true;
                update.moveNum = shellPlayer.moveNum;
                update.bootMask = 0;
                if (shellPlayer.quitRequested)
                {
                    NetGameP::SetPlayerName(player, "");
                    update.quitMask = 1ULL << player;
                }
                WriteShellUpdate(group, player, update, shellState);
                group.EndWrite();
                NetComm::Send(player, PKT_SHELL_UPDATE, &group);
            }
            needSendMask &= ~(1ULL << player);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x005e2640
void CNetPostLoadHostGame::Update()
{
    NetComm::AdvanceTime(true);
    float now = NetComm::GetTime();
    if (timer > now)
    {
        return;
    }
    timer = now + 0.2f;

    while (true)
    {
        NetPacket* join = NetComm::Recv(PKT_JOIN, PKT_NONE);
        if (!join)
        {
            break;
        }
        int from = GetPacketSourceHost(join);
        if (from < 64)
        {
            if (NetGame::IsPlaying(from, false))
            {
                join->Free();
                join = 0;
                NetGameP::SetNotPlaying(from);
                break;
            }
            join->Free();
            join = 0;
            needSendMask |= 1ULL << from;
        }
        else if (loadGame || GameLoop::IsGameOver())
        {
            NetPktGroup group;
            group.Set(join);
            group.BeginRead();
            unsigned int value;
            group.ReadBits(value, 12);
            if (value != 1)
            {
                group.EndRead();
                join->Free();
                join = 0;
                group.BeginWrite();
                WriteJoinReject(group, JRR_VERSION);
                group.EndWrite();
                NetComm::Send(from, PKT_REJECT, &group);
            }
            else
            {
                group.ReadBits(value, 32);
                if (value != (unsigned int)NetSetup::GetPassword())
                {
                    group.EndRead();
                    join->Free();
                    join = 0;
                    group.BeginWrite();
                    WriteJoinReject(group, JRR_PASSWORD);
                }
                else
                {
                    unsigned int guestCount;
                    group.ReadBits(guestCount, 3);
                    group.ReadBits(value, 2);
                    char name[32];
                    ReadString(group, name, sizeof(name));
                    group.EndRead();
                    join->Free();
                    join = 0;
                    group.BeginWrite();
                    WriteJoinReject(group, JRR_BUSY);
                }
                group.EndWrite();
                NetComm::Send(from, PKT_REJECT, &group);
            }
        }
        else if (!BanIP(from, join))
        {
            int player = -1;
            int i;
            for (i = 0; i < netCurMaxPlayers; ++i)
            {
                if (NetComm::IsConnected(i) && NetCommP::SameAddr(i, from))
                {
                    player = i;
                    break;
                }
            }
            if (player == -1)
            {
                unsigned int j;
                int guests[1];
                for (j = 0; j < 1; ++j)
                {
                    guests[j] = -1;
                }
                int slots = Min(netCurMaxPlayers + 2, netPlayerLimit);
                for (i = 0; i < slots; ++i)
                {
                    if (NetComm::IsConnected(i) || NetGame::IsPlaying(i, false))
                    {
                        continue;
                    }
                    if (player == -1)
                    {
                        player = i;
                    }
                    else
                    {
                        for (j = 0; j < 1; ++j)
                        {
                            if (guests[j] == -1)
                            {
                                guests[j] = i;
                                break;
                            }
                        }
                    }
                }

                NetPktGroup group;
                group.Set(join);
                group.BeginRead();
                unsigned int version;
                group.ReadBits(version, 12);
                unsigned int password;
                group.ReadBits(password, 32);
                unsigned int guestCount;
                group.ReadBits(guestCount, 3);
                bool hasRoom = player != -1;
                for (j = 0; j < guestCount; ++j)
                {
                    if (guests[j] == -1)
                    {
                        hasRoom = false;
                    }
                }
                if (!hasRoom)
                {
                    group.EndRead();
                    join->Free();
                    join = 0;
                    group.BeginWrite();
                    WriteJoinReject(group, JRR_FULL);
                    group.EndWrite();
                    NetComm::Send(from, PKT_REJECT, &group);
                }
                else if (password != (unsigned int)NetSetup::GetPassword())
                {
                    group.EndRead();
                    join->Free();
                    join = 0;
                    group.BeginWrite();
                    WriteJoinReject(group, JRR_PASSWORD);
                    group.EndWrite();
                    NetComm::Send(from, PKT_REJECT, &group);
                }
                else if (version != 1)
                {
                    group.EndRead();
                    join->Free();
                    join = 0;
                    group.BeginWrite();
                    WriteJoinReject(group, JRR_VERSION);
                    group.EndWrite();
                    NetComm::Send(from, PKT_REJECT, &group);
                }
                else
                {
                    unsigned __int64* address = NetCommP::GetAddr(GetPacketSourceHost(join));
                    NetCommP::Connect(player, address);
                    UpdateCurMaxPlayers(1ULL << player, true);
                    for (unsigned int k = 0; k < guestCount; ++k)
                    {
                        NetCommP::Connect(guests[k], address);
                        UpdateCurMaxPlayers(1ULL << guests[k], true);
                        NetGameP::SetGuestOf(guests[k], player);
                    }
                    ReadJoin(group, player, guestCount, shellState);
                    group.EndRead();
                    join->Free();
                    join = 0;
                    needSendMask |= 1ULL << player;
                }
            }
        }
        if (join)
        {
            join->Free();
        }
    }

    while (true)
    {
        NetPacket* packet = NetComm::Recv(PKT_SHELL_MOVE, PKT_NONE);
        if (!packet)
        {
            break;
        }
        int player = GetPacketSourceHost(packet);
        NetPktGroup group;
        group.Set(packet);
        group.BeginRead();
        ShellMove move;
        ReadShellMove(group, player, move, shellState);
        group.EndRead();
        packet->Free();
        ShellPlayer& shellPlayer = shellState.players[player];
        group.BeginWrite();
        ShellUpdate update;
        update.postLoad = true;
        update.moveNum = shellPlayer.moveNum;
        update.bootMask = 0;
        if (shellPlayer.quitRequested)
        {
            NetGameP::SetPlayerName(player, "");
            update.quitMask = 1ULL << player;
        }
        WriteShellUpdate(group, player, update, shellState);
        group.EndWrite();
        NetComm::Send(player, PKT_SHELL_UPDATE, &group);
    }

    while (true)
    {
        NetPacket* ack = NetComm::Recv(PKT_LAUNCH_ACK, PKT_NONE);
        if (!ack)
        {
            break;
        }
        int player = GetPacketSourceHost(ack);
        ack->Free();
        NetPktGroup group;
        group.BeginWrite();
        WriteLaunch(group, true, player, true);
        group.EndWrite();
        NetComm::Send(player, PKT_LAUNCH, group.pktPtr);
        loadingMask |= 1ULL << player;
    }

    int i;
    while (true)
    {
        NetPacket* ack = NetComm::Recv(PKT_LOAD_ACK, PKT_NONE);
        if (!ack)
        {
            break;
        }
        int player = GetPacketSourceHost(ack);
        ack->Free();
        loadingMask &= ~(1ULL << player);
        unsigned __int64 mask = 0;
        for (i = 0; i < netCurMaxPlayers; ++i)
        {
            if (NetGame::IsPlayingNext(i))
            {
                mask |= 1ULL << i;
            }
        }
        mask |= 1ULL << player;
        for (int slot = 0; slot < 1; ++slot)
        {
            int guest = NetGame::GetMyGuest(player, slot);
            if (guest >= 0)
            {
                mask |= 1ULL << guest;
            }
        }
        NetGameP::SetPlaying(mask, true);
        NetPktGroup group;
        group.BeginWrite();
        unsigned __int64 ackedMask = mask;
        WriteLoad(group, player, mask, ackedMask);
        group.EndWrite();
        NetComm::Send(player, PKT_LOAD, group.pktPtr);
    }

    for (i = 0; i < netCurMaxPlayers; ++i)
    {
        if (!NetComm::IsConnected(i) || NetGame::IsPlaying(i, false))
        {
            continue;
        }
        float elapsed = NetComm::GetTime() - NetComm::GetLastRecv(i);
        float timeout = 15.0f;
        if (shellState.players[i].quitRequested)
        {
            timeout = 5.0f;
        }
        else if (loadingMask & (1ULL << i))
        {
            timeout = 60.0f;
        }
        if (elapsed > timeout)
        {
            NetGameP::SetPlayerName(i, "");
            NetGame::SetPlayerGSID(i, 0);
            NetGame::ClearPlayerCDKey(i);
            int hostIndex = *NetGameP::GetHostIndex(i);
            if (hostIndex >= 0)
            {
                *NetGameP::GetHostMode(hostIndex) = 3;
            }
            NetComm::Disconnect(i);
            shellState.players[i].Init();
        }
    }

    SendUpdates();
    NetMatch::UpdateLogin(-1);
    NetMatch::UpdateRegisterGame();
    char exitMark = 'c';
    PblJournal::Record(exitMark, PJT_FULL);
}

// FUNCTION: SWBF2 0x005db010
NetHostGame::~NetHostGame()
{
}

// FUNCTION: SWBF2 0x005e7170
char* NetSetup::GetGameVersion()
{
    return netGameVersion;
}

// FUNCTION: SWBF2 0x005e7390
bool NetSetup::GetSpecialJoinValid()
{
    return mSpecialJoinValid;
}

// GLOBAL: SWBF2 0x01ead929
bool NetSetup::mSpecialJoinValid;

// FUNCTION: SWBF2 0x005d6360
int GetLastSignInError()
{
    return netLoginFailureReason;
}

// GLOBAL: SWBF2 0x01ee7d50
unsigned __int64 netCreatedLobbyId;

// GLOBAL: SWBF2 0x007ec514
unsigned int netLobbyPublishPumps = 5;

// GLOBAL: SWBF2 0x01e65048
int netLanRegisterState;

// GLOBAL: SWBF2 0x01fa88d0
char const* netLoginErrorMessage;

// GLOBAL: SWBF2 0x01e64f04
int netLoginErrorLevel;

// FUNCTION: SWBF2 0x005d6880
void PublishNextMissionToLobby()
{
    if (NetComm::IsLAN())
    {
        NetPacket* pPkt;
        while ((pPkt = NetComm::Recv(PKT_QUERY, PKT_NONE)) != 0)
        {
            if (!NetGame::IsHost())
            {
                pPkt->Free();
                continue;
            }

            int iSrc = (unsigned char)pPkt->mData[3];
            NetPktGroup group;
            group.Set(pPkt);
            group.BeginRead();
            unsigned int uWord = 0;
            unsigned int uNibble = 0;
            unsigned int uNone;
            group.ReadBits(uNone, 1);
            if (!uNone)
            {
                group.ReadBits(uWord, 32);
                group.ReadBits(uNibble, 4);
            }
            pPkt->Free();

            int iReinf0 = Team::GetPtr(1) ? Team::GetPtr(1)->GetCurReinforcements() : 300;
            int iReinf1 = Team::GetPtr(2) ? Team::GetPtr(2)->GetCurReinforcements() : 300;
            int iPlaying = 0;
            for (int i = 0; i < 64; i++)
            {
                bool bPlaying = NetGame::IsPlaying(i, false);
                if (!NetGame::IsClient())
                {
                    bPlaying |= NetGame::IsPlayingNext(i);
                }
                if (bPlaying)
                {
                    iPlaying++;
                }
            }
            unsigned char u8Turns = PlayerProfile::GetCurProfile()->mOnlineOptions.mu8TurnsPerSecond;

            group.BeginWrite();
            group.WriteBits(uWord, 32);
            group.WriteBits(uNibble, 4);
            WriteString(&group, NetSetup::GetHostName(), 64);
            WriteString(&group, NetSetup::GetGameType(), 64);
            WriteString(&group, GetNextMissionForLobby(), 64);
            group.WriteBits(netAutoAim != 0, 1);
            group.WriteBits(netAutoAssignTeams != 0, 1);
            group.WriteBits(netTeamDamage != 0, 1);
            group.WriteBits(NetSetup::mPasswordHash != 0, 1);
            group.WriteBits(netNumBots, 8);
            group.WriteBits(iReinf0, 11);
            group.WriteBits(iReinf1, 11);
            group.WriteBits(iPlaying, 7);
            group.WriteBits(netPlayerLimit, 7);
            group.WriteBits(1, 3);
            group.WriteBits(u8Turns, 8);
            group.WriteBits(netMinPlayersToAutoLaunchDedicated, 7);
            group.WriteBits(PlayerPrefs::mAllUserPrefs[0].mDifficulty, 2);
            group.WriteBits(netShowNames != 0, 1);
            group.WriteBits(gSpawnInvicibility, 6);
            group.WriteBits(netHeroesEnabled != 0, 1);
            if (netHeroesEnabled)
            {
                group.WriteBits(netHeroRuleUnlock, 4);
                group.WriteBits(netHeroRuleUnlockVal, 8);
                group.WriteBits(netHeroRuleTeam, 3);
                group.WriteBits(netHeroRulePlayer, 4);
                group.WriteBits(netHeroRuleRespawn, 2);
                group.WriteBits(netHeroRuleRespawnVal, 10);
            }
            group.EndWrite();
            NetComm::Send(iSrc, PKT_QUERY_RESPONSE, group.pktPtr);
        }
        return;
    }

    if (!NetGame::IsHost() || netCreatedLobbyId == 0)
    {
        return;
    }

    if (netLobbyPublishPumps >= 5)
    {
        netLobbyPublishPumps = 0;
        SetLobbyData("fgd_str_host_name", NetSetup::GetHostName());
        SetLobbyData("fgd_str_game_type", NetSetup::GetGameType());
        SetLobbyData("fgd_str_map_name", GetNextMissionForLobby());
        SetLobbyData("fgr_bool_netAutoAim", gf::utils::ToString(netAutoAim));
        SetLobbyData("fgr_bool_netAutoAssignTeams", gf::utils::ToString(netAutoAssignTeams));
        SetLobbyData("fgr_bool_netTeamDamage", gf::utils::ToString(netTeamDamage));
        SetLobbyData("fgd_bool_has_password", gf::utils::ToString(NetSetup::mPasswordHash != 0));
        if (NetSetup::mPasswordHash)
        {
            SetLobbyData("galaxy_lobby_password", GetEncodedLobbyPassword());
        }

        int iReinf0 = Team::GetPtr(1) ? Team::GetPtr(1)->GetCurReinforcements() : 300;
        int iReinf1 = Team::GetPtr(2) ? Team::GetPtr(2)->GetCurReinforcements() : 300;
        int iPlaying = 0;
        for (int i = 0; i < 64; i++)
        {
            bool bPlaying = NetGame::IsPlaying(i, false);
            if (!NetGame::IsClient())
            {
                bPlaying |= NetGame::IsPlayingNext(i);
            }
            if (bPlaying)
            {
                iPlaying++;
            }
        }
        SetLobbyData("fgr_int_netNumBots", gf::utils::ToString(netNumBots));
        SetLobbyData("fgr_int_r0", gf::utils::ToString(iReinf0));
        SetLobbyData("fgr_int_r1", gf::utils::ToString(iReinf1));
        SetLobbyData("fgd_int_numPlayers", gf::utils::ToString(iPlaying));
        SetLobbyData("fgd_int_netPlayerLimit", gf::utils::ToString(netPlayerLimit));
        unsigned char u8Turns = PlayerProfile::GetCurProfile()->mOnlineOptions.mu8TurnsPerSecond;
        SetLobbyData("fgd_int_serverType", gf::utils::ToString(1));
        SetLobbyData("fgd_int_turns_per_second", gf::utils::ToString((int)u8Turns));
        SetLobbyData("fgr_int_netMinPlayersToAutoLaunchDedicated", gf::utils::ToString(netMinPlayersToAutoLaunchDedicated));
        SetLobbyData("fgr_int_mDifficulty", gf::utils::ToString(PlayerPrefs::mAllUserPrefs[0].mDifficulty));
        SetLobbyData("fgr_int_netShowNames", gf::utils::ToString(netShowNames));
        SetLobbyData("fgr_int_gSpawnInvicibility", gf::utils::ToString(gSpawnInvicibility));
        SetLobbyData("fgr_int_netHeroesEnabled", gf::utils::ToString(netHeroesEnabled));
        if (netHeroesEnabled)
        {
            SetLobbyData("fgr_uint_netHeroRuleUnlock", gf::utils::ToString((unsigned int)netHeroRuleUnlock));
            SetLobbyData("fgr_int_netHeroRuleUnlockVal", gf::utils::ToString(netHeroRuleUnlockVal));
            SetLobbyData("fgr_uint_netHeroRuleTeam", gf::utils::ToString((unsigned int)netHeroRuleTeam));
            SetLobbyData("fgr_uint_netHeroRulePlayer", gf::utils::ToString((unsigned int)netHeroRulePlayer));
            SetLobbyData("fgr_uint_netHeroRuleRespawn", gf::utils::ToString((unsigned int)netHeroRuleRespawn));
            SetLobbyData("fgr_uint_netHeroRuleRespawnVal", gf::utils::ToString((unsigned int)netHeroRuleRespawnVal));
        }
        SetLobbyData("galaxy_revision", gf::utils::ToString(1u));
        SetLobbyData("galaxy_lobby_cancelled", gf::utils::ToString(false));

        void* pMatchmaking = GetGalaxyMatchmaking();
        ((galaxy::api::IMatchmaking*)pMatchmaking)->SetLobbyJoinable(galaxy::api::GalaxyID(netCreatedLobbyId), true);
    }
    netLobbyPublishPumps++;
}

// GLOBAL: SWBF2 0x01e64f2c
int netNoCableCount;

// GLOBAL: SWBF2 0x01e65040
int netLobbyState;

// GLOBAL: SWBF2 0x01ee7d40
unsigned __int64 netEnteredLobbyId;

// GLOBAL: SWBF2 0x01e65034
int netConnectState;

// GLOBAL: SWBF2 0x01e65038
int netLoginFailureReason;

// GLOBAL: SWBF2 0x01e65031
bool netLobbyRequestPending;

// GLOBAL: SWBF2 0x01e65032
bool netLeavingLobby;

// FUNCTION: SWBF2 0x0052fbc0
void* __fastcall ReturnUnorderedMapThis(void* pThis)
{
    return pThis;
}

// FUNCTION: SWBF2 0x0052fc60
void __stdcall NoOpListCleanup(int, int)
{
}

// FUNCTION: SWBF2 0x0052fc70
void* __fastcall ReturnListThis(void* pThis)
{
    return pThis;
}

// FUNCTION: SWBF2 0x0052fc80
void* __fastcall ReturnListEnd(void* pThis)
{
    return pThis;
}

// FUNCTION: SWBF2 0x005e7350
const char* GetEncodedLobbyPassword()
{
    return (const char*)netEncodedPassword;
}

// LIBRARY: SWBF2 0x005d4700
// std::error_category::default_error_condition

// LIBRARY: SWBF2 0x005d4720
// std::error_category::equivalent(int, std::error_condition const&)

// LIBRARY: SWBF2 0x005d4760
// std::error_category::equivalent(std::error_code const&, int)

// LIBRARY: SWBF2 0x005d4780
// std::_Generic_error_category::name

// LIBRARY: SWBF2 0x005d4790
// std::_Generic_error_category::message

// LIBRARY: SWBF2 0x005d4840
// std::_Iostream_error_category::name

// LIBRARY: SWBF2 0x005d4850
// std::_Iostream_error_category::message

// LIBRARY: SWBF2 0x005d48d0
// std::_System_error_category::name

// LIBRARY: SWBF2 0x005d48e0
// std::_System_error_category::message

// LIBRARY: SWBF2 0x005d4960
// std::_System_error_category::default_error_condition

// TEMPLATE: SWBF2 0x005d8d40
// utf8::internal::increase_safely<std::_String_iterator<std::_String_val<std::_Simple_types<char> > > >

// TEMPLATE: SWBF2 0x005d8bd0
// utf8::internal::get_sequence_1<std::_String_iterator<std::_String_val<std::_Simple_types<char> > > >

// TEMPLATE: SWBF2 0x005d8bf0
// utf8::internal::get_sequence_2<std::_String_iterator<std::_String_val<std::_Simple_types<char> > > >

// TEMPLATE: SWBF2 0x005d8c40
// utf8::internal::get_sequence_3<std::_String_iterator<std::_String_val<std::_Simple_types<char> > > >

// TEMPLATE: SWBF2 0x005d8cb0
// utf8::internal::get_sequence_4<std::_String_iterator<std::_String_val<std::_Simple_types<char> > > >

// TEMPLATE: SWBF2 0x005d8aa0
// ??$validate_next@V?$_String_iterator@V?$_String_val@U?$_Simple_types@D@std@@@std@@@std@@@internal@utf8@@YA?AW4utf_error@01@AAV?$_String_iterator@V?$_String_val@U?$_Simple_types@D@std@@@std@@@std@@V34@AAI@Z

// TEMPLATE: SWBF2 0x005d84e0
// utf8::append<std::back_insert_iterator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >

// TEMPLATE: SWBF2 0x005d8430
// utf8::next<std::_String_iterator<std::_String_val<std::_Simple_types<char> > > >

// TEMPLATE: SWBF2 0x005d82e0
// utf8::replace_invalid<std::_String_iterator<std::_String_val<std::_Simple_types<char> > >,std::back_insert_iterator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >

// FUNCTION: SWBF2 0x005d49d0
const char* utf8::invalid_code_point::what() const throw()
{
    return "Invalid code point";
}

// FUNCTION: SWBF2 0x005d4a50
const char* utf8::invalid_utf8::what() const throw()
{
    return "Invalid UTF-8";
}

// FUNCTION: SWBF2 0x005d4aa0
const char* utf8::not_enough_room::what() const throw()
{
    return "Not enough space";
}

// FUNCTION: SWBF2 0x005d85f0
utf8::not_enough_room::not_enough_room(const not_enough_room& rhs) : exception(rhs) {}

// FUNCTION: SWBF2 0x005d8610
utf8::exception::exception(const exception& rhs) : std::exception(rhs) {}

// FUNCTION: SWBF2 0x005d8630
utf8::invalid_utf8::invalid_utf8(const invalid_utf8& rhs) : exception(rhs), byte(rhs.byte) {}

// FUNCTION: SWBF2 0x005d8660
utf8::invalid_code_point::invalid_code_point(const invalid_code_point& rhs) : exception(rhs), code(rhs.code) {}

// FUNCTION: SWBF2 0x005d3460
void __fastcall CallEmptySessionElementHook(void*)
{
    sub_42E960();
}

// FUNCTION: SWBF2 0x005e7190
const char* GetNextMissionForLobby()
{
    return GameLoop::GetNextMission();
}

// LIBRARY: SWBF2 0x005d7d30
// std::basic_string<char>::insert(size_type, size_type, char)

// LIBRARY: SWBF2 0x005d8a40
// std::back_insert_iterator<std::string>::operator=(const char&)

// FUNCTION: SWBF2 0x005e73d0
void RemoveFoundGameByAddress(const NetAddr* address)
{
    ((CNetFindGame*)findGame)->RemoveGameByAddress(address);
}

// SYNTHETIC: SWBF2 0x005d49e0
utf8::invalid_code_point::~invalid_code_point() {}

// SYNTHETIC: SWBF2 0x005d4a20
utf8::exception::~exception() {}

// SYNTHETIC: SWBF2 0x005d4a60
utf8::invalid_utf8::~invalid_utf8() {}

// SYNTHETIC: SWBF2 0x005d8a10
utf8::not_enough_room::~not_enough_room() {}

// SYNTHETIC: SWBF2 0x004056c0
// `dynamic initializer for 'PblMath::fTwoPi''

// SYNTHETIC: SWBF2 0x004056e0
// `dynamic initializer for 'PblMath::fHalfPi''

// SYNTHETIC: SWBF2 0x00405700
// `dynamic initializer for 'PblMath::fOneOverPi''

// SYNTHETIC: SWBF2 0x00405720
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// SYNTHETIC: SWBF2 0x00405740
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00405760
// `dynamic initializer for '_IdentityMatrix''

// GLOBAL: SWBF2 0x01ef1f90
// SYNTHETIC: SWBF2 0x004059b0
// `dynamic initializer for 'autoNetTimeStamp''

PblTimeBase::Stamp autoNetTimeStamp;

// GLOBAL: SWBF2 0x01ef254c
// SYNTHETIC: SWBF2 0x004059c0
// `dynamic initializer for 'NetSetup::mSpecialJoinAddress''

NetSessionId NetSetup::mSpecialJoinAddress;

// SYNTHETIC: SWBF2 0x00405870
// `dynamic initializer for 'TRIGGER_TIMER_SECONDS_PER_NETTICK''

// SYNTHETIC: SWBF2 0x00405890
// `dynamic initializer for 'TRIGGER_TIMER_TICKS_PER_SECOND''

// SYNTHETIC: SWBF2 0x004058b0
// `dynamic initializer for 'TRIGGER_TIMER_SECONDS_PER_TICK''

// SYNTHETIC: SWBF2 0x004058d0
// `dynamic initializer for 'MAX_TAP_DURATION_TICKS''

// SYNTHETIC: SWBF2 0x00405900
// `dynamic initializer for 'MAX_DOUBLETAP_INTERVAL_TICKS''

// GLOBAL: SWBF2 0x01e6505b
bool NetSetup::mPrivateGame = false;
