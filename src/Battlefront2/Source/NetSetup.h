#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblList.h"

extern bool autoNetEnabled;
extern bool autoNetLan;
extern bool autoNetRandomize;
extern bool g_bNoRender;
extern unsigned int netCrashAddr;
extern char autoNetScript[32];
extern float g_fFixedRate;
extern unsigned int netPasswordHash;
extern int netDifficulty;
extern bool g_bNoWait;
extern bool g_bNoSound;

enum NetScript
{
    NS_HOST = 0,
    NS_CLIENT = 1,
    NS_DEDICATED = 2,
    NS_UNKNOWN = 3
};

struct stAutoNetMission
{
    char name[12];
    int tickets[2];
};

class PblRandom;

extern char autoNetSession[64];
extern stAutoNetMission autoNetMissions[256];
extern PblRandom netRandom;

class NetLaunchGame
{
public:
    // SYNTHETIC: SWBF2 0x005db120
    // NetLaunchGame::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x005db100
    virtual ~NetLaunchGame() {}
    virtual void Update() = 0;
    virtual bool IsDone() = 0;
};

enum FindType
{
    FT_NORMAL = 0,
    FT_QUICK = 1,
    FT_FIXED = 2,
    FT_PREVIOUS = 3,
    FT_CURRENT = 4
};

struct NetSessionId
{

    // FUNCTION: SWBF2 0x00591430
    NetSessionId() : id(0) {}

    unsigned __int64 id;

    // FUNCTION: SWBF2 0x00591440
    void SetNULL() { id = 0; }
};

struct FindGamePlayer;
struct FindGameRules;
struct FindGameData;
struct NetKey;
class NetPacket;

class NetHostGame
{
public:
    // SYNTHETIC: SWBF2 0x005db030
    // NetHostGame::`scalar deleting destructor'

    virtual ~NetHostGame();
    virtual void Cancel() = 0;
    virtual void Update() = 0;
    virtual int GetChangeCount() = 0;
    virtual int GetPlayerCount() = 0;
    virtual int GetPlayerIndex(int iIndex) = 0;
    virtual char const* GetPlayerName(int iPlayer) = 0;
    virtual int GetStartTeam(int iPlayer) = 0;
    virtual bool IsLaunchReady() = 0;
    virtual bool IsFailed() = 0;
    virtual bool IsCancelled() = 0;
    virtual void BootPlayer(int iPlayer) = 0;
    virtual void ForceWriteShellMove() = 0;
    virtual void ForceWriteShellUpdate() = 0;
};

class NetJoinGame
{
public:
    // SYNTHETIC: SWBF2 0x005db0d0
    // NetJoinGame::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x005db0b0
    virtual ~NetJoinGame() {}
    virtual void Update() = 0;
    virtual void Cancel() = 0;
    virtual bool IsLaunchReady() = 0;
    virtual bool IsHostPlaying() = 0;
    virtual void Launch() = 0;
    virtual int GetChangeCount() = 0;
    virtual int GetPlayerCount() = 0;
    virtual int GetPlayerIndex(int iIndex) = 0;
    virtual char const* GetPlayerName(int iPlayer) = 0;
    virtual int GetStartTeam(int iPlayer) = 0;
    virtual char const* GetEra() = 0;
    virtual bool IsFailed() = 0;
    virtual bool IsConnected() = 0;
    virtual bool IsCancelled() = 0;
    virtual void ForceWriteShellMove() = 0;
};

class NetFindGame
{
public:
    // SYNTHETIC: SWBF2 0x005db080
    // NetFindGame::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x005db060
    virtual ~NetFindGame() {}
    virtual bool Update() = 0;
    virtual void Cancel() = 0;
    virtual void Pause() = 0;
    virtual int GetGameCount() = 0;
    virtual void SelectFirst() = 0;
    virtual void Select(int iIndex) = 0;
    virtual void SelectSortPos(int iPos) = 0;
    virtual char* GetName() = 0;
    virtual int GetMaxPlayers() = 0;
    virtual int GetMaxBots() = 0;
    virtual bool GetInfo(char*& pcName, char*& pcMission, int& iNumPlayers, int& iMaxPlayers,
                         bool& bPassword, float& fPing, int& iArg7, int& iArg8, bool& bArg9,
                         bool& bArg10, char*& pcArg11, bool& bArg12) = 0;
    virtual bool GetFiltered() = 0;
    virtual void SetFiltered(bool bFiltered) = 0;
    virtual bool GetRules(FindGameRules& rules) = 0;
    virtual bool IsReady() = 0;
    virtual bool IsFinished() = 0;
    virtual void Refresh() = 0;
    virtual FindType GetFindType() = 0;
    virtual void SetSelectedSession(int iSession) = 0;
    virtual int GetSelectedSession() = 0;
    virtual bool GetPlayerData(int iPlayer, FindGamePlayer& player) = 0;
    virtual void SetDirty(bool bDirty) = 0;
    virtual bool GetDirty() = 0;
    virtual void SetSortPosition(unsigned short usPos) = 0;
    virtual bool GetFavorite() = 0;
    virtual void SetFavorite(bool bFavorite) = 0;
    virtual void SetNumVisible(int iNum) = 0;
    virtual int GetNumVisible() = 0;
    virtual int GetPercentDone() = 0;
};

class NetLoadGame
{
public:
    // SYNTHETIC: SWBF2 0x005db170
    // NetLoadGame::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x005db150
    virtual ~NetLoadGame() {}
    virtual void Update() = 0;
    virtual bool IsDone() = 0;
    virtual bool HostKnowsILoaded() = 0;
};

class NetPostLoadHostGame
{
public:
    // SYNTHETIC: SWBF2 0x005db1c0
    // NetPostLoadHostGame::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x005db1a0
    virtual ~NetPostLoadHostGame() {}
    virtual void Cancel() = 0;
    virtual void Update() = 0;
    virtual void BootPlayer(int iPlayer) = 0;
};

class NetPostLoadJoinGame
{
public:
    // SYNTHETIC: SWBF2 0x005db210
    // NetPostLoadJoinGame::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x005db1f0
    virtual ~NetPostLoadJoinGame() {}
    virtual void Cancel() = 0;
    virtual void Update() = 0;
};

enum JoinRejectReason
{
    JRR_TOLKIEN = 0,
    JRR_FULL = 1,
    JRR_PASSWORD = 2,
    JRR_GAME = 3,
    JRR_VERSION = 4,
    JRR_BOOTED = 5,
    JRR_BUSY = 6,
    JRR_CONNECTION_LOST = 7,
    JRR_MISSING_DC = 8,
    JRR_BAD_CDKEY = 9,
    JRR_BAN = 10
};

struct IntraSessionPlayer
{
    unsigned int playerId;
    bool isTeam1;
};

struct NetAddr
{
    int pubAddr;
    int pubPort;
    bool hasPriv;
    int privAddr;
    int privPort;

    // FUNCTION: SWBF2 0x005d46a0
    NetAddr() : pubAddr(0), pubPort(0), hasPriv(false), privAddr(0), privPort(0) {}

    NetAddr(int addr, int port, int overridePort);

    NetAddr* CopyFieldsFrom(const NetAddr* other);

    char GetAddr(void* pOut, int pKey) const;
};
 
 
 
 

struct SendStateDst
{
    char fName[32];
    bool done;
    bool gotSome;
    int baseBlock;
    unsigned int hasBlocks;
    int fd;

    void Init();
    void Cancel();
};
 

struct SendStateSrc
{
    int fileCount;
    char* files;
    int fileNum;
    int baseBlock;
    unsigned int hasBlocks;
    char* curFile;
    int blockCount;
    int blockCycle;

    void Init(int iCount, char** ppcFiles);
};
 

struct ShellPlayer
{
    bool joinAck;
    unsigned int hostRequest;
    unsigned int moveNum;
    unsigned short name;
    int ping;
    int bw;
    unsigned int updateNum;
    unsigned int ackedUpdate;
    bool requestedQuit;
    unsigned int changedUpdate;
    unsigned int playerId;
    bool quitRequested;
    SendStateSrc patchFile;

    void Init();
};
 

struct ShellState
{
    bool canceled;
    ShellPlayer players[64];
};
 

struct ShellUpdate
{
    bool postLoad;
    bool hostLaunched;
    bool hostLoadDone;
    unsigned int turnNumber;
    unsigned __int64 connectMask;
    unsigned int moveNum;
    unsigned __int64 connectingMask;
    unsigned __int64 quitMask;
    unsigned __int64 bootMask;
    SendStateDst patchFile;

    ShellUpdate();
};
 

// VTABLE: SWBF2 0x007ac144
class CNetHostGame : public NetHostGame
{
public:
    CNetHostGame();

    // SYNTHETIC: SWBF2 0x005dc210
    // CNetHostGame::`scalar deleting destructor'

    virtual ~CNetHostGame();
    virtual void Cancel();
    virtual void Update();
    virtual int GetChangeCount();
    virtual int GetPlayerCount();
    virtual int GetPlayerIndex(int iIndex);
    virtual char const* GetPlayerName(int iPlayer);
    virtual int GetStartTeam(int iPlayer);
    virtual bool IsLaunchReady();
    virtual bool IsFailed();
    virtual bool IsCancelled();
    virtual void BootPlayer(int iPlayer);
    virtual void ForceWriteShellMove();
    virtual void ForceWriteShellUpdate();

    void DropPlayer(int iPlayer);
    void SendUpdates();

    float timer;
    int changeCount;
    int playerCount;
    float launchTimer;
    bool launchReady;
    ShellState shellState;
    bool hostFailed;
    unsigned __int64 bootMask;
    float bootTime[64];
    unsigned __int64 needSendMask;
    ShellUpdate shellUpdate;
};
 

struct ShellMove
{

    ShellMove() : moveNum(0), hostRequest(0), quitRequested(false) {}

    bool isTeamOne;
    bool launchGuestHost;
    unsigned int moveNum;
    unsigned int hostRequest;
    char gameName[64];
    char missionName[64];
    bool quitRequested;
};
 

// VTABLE: SWBF2 0x007ac2b4
class CNetJoinGame : public NetJoinGame
{
public:
    explicit CNetJoinGame(NetFindGame* pFind);
    virtual ~CNetJoinGame();
    virtual void Update();
    virtual void Cancel();
    virtual bool IsLaunchReady();
    virtual bool IsHostPlaying();
    virtual void Launch();
    virtual int GetChangeCount();
    virtual int GetPlayerCount();
    virtual int GetPlayerIndex(int iIndex);
    virtual char const* GetPlayerName(int iPlayer);
    virtual int GetStartTeam(int iPlayer);
    virtual char const* GetEra();
    virtual bool IsFailed();
    virtual bool IsConnected();
    virtual bool IsCancelled();
    virtual void ForceWriteShellMove();

    CNetJoinGame* NextHost(int);

    float timer;
    int changeCount;
    int tryCount;
    bool hostLaunched;
    bool clientLaunched;
    void* cFind;
    char gameName[64];
    char gameEra[64];
    unsigned int hostRequest;
    unsigned int moveNum;
    unsigned int ackedMoveNum;
    bool connecting;
    bool connected;
    NetAddr hostAddr;
    ShellMove shellMove;
    ShellUpdate shellUpdate;
    bool joinFailed;
    int rejectReason;
    float cancelTimer;
};
 

// VTABLE: SWBF2 0x007ac180
class CNetLaunchGame : public NetLaunchGame
{
public:
    CNetLaunchGame(bool host);
    virtual ~CNetLaunchGame();
    virtual void Update();
    virtual bool IsDone();

    float timer;
    bool launchHost;
    bool launchAcks[64];
    bool launchAcked;
    float ackedTimer;
    bool launchDone;
    float startTime;
    float sendTimes[64];
    int sendIndex;
};
 

// VTABLE: SWBF2 0x007ac30c
class CNetLoadGame : public NetLoadGame
{
public:
    CNetLoadGame(bool host);
    virtual ~CNetLoadGame();
    virtual void Update();
    virtual bool IsDone();
    virtual bool HostKnowsILoaded();

    float timer;
    bool loadHost;
    bool loadDone;
    unsigned __int64 ackedMask;
    bool loadAcked;
    bool hostKnowsILoaded;
    float ackedTimer;
    float sendTimes[64];
};
 

class NetGameDesc;

// VTABLE: SWBF2 0x007ac340
class CNetFindGame : public NetFindGame
{
public:

    void RemoveGameByAddress(const NetAddr* address);
    CNetFindGame(FindType eType, const char* pcName, NetSessionId* pId);
    virtual ~CNetFindGame();
    virtual bool Update();
    virtual void Cancel();
    virtual void Pause();
    virtual int GetGameCount();
    virtual void SelectFirst();
    virtual void Select(int iIndex);
    virtual void SelectSortPos(int iPos);
    virtual char* GetName();
    virtual int GetMaxPlayers();
    virtual int GetMaxBots();
    virtual bool GetInfo(char*& pcName, char*& pcMission, int& iNumPlayers, int& iMaxPlayers,
                         bool& bPassword, float& fPing, int& iArg7, int& iArg8, bool& bArg9,
                         bool& bArg10, char*& pcArg11, bool& bArg12);
    virtual bool GetFiltered();
    virtual void SetFiltered(bool bFiltered);
    virtual bool GetRules(FindGameRules& rules);
    virtual bool IsReady();
    virtual bool IsFinished();
    virtual void Refresh();
    virtual FindType GetFindType();
    virtual void SetSelectedSession(int iSession);
    virtual int GetSelectedSession();
    virtual bool GetPlayerData(int iPlayer, FindGamePlayer& player);
    virtual void SetDirty(bool bDirty);
    virtual bool GetDirty();
    virtual void SetSortPosition(unsigned short usPos);
    virtual bool GetFavorite();
    virtual void SetFavorite(bool bFavorite);
    virtual void SetNumVisible(int iNum);
    virtual int GetNumVisible();

    void RemoveOld(float age);
    virtual int GetPercentDone();

    virtual char* GetDescriptorExtension();
    virtual bool IsMetaGame();
    virtual const char* GetEra();

    void AddGame(const NetAddr* address);
    void UpdateGame(const NetAddr* address, const void* session, const void* key,
                    const char* version, const void* playerData, const void* extra);
    void GetHostAddr(NetAddr* address, NetKey* key) const;

    bool mDirty;
    int mNumVisible;

    // TEMPLATE: SWBF2 0x005e7d70
    // PblList<NetGameDesc>::PblList<NetGameDesc>

    // TEMPLATE: SWBF2 0x005e7df0
    // PblList<NetGameDesc>::Remove
    PblList<NetGameDesc> gameList;
    PblList<NetGameDesc>::Itor curGame;
    float findStartTime;
    float prevFindStartTime;
    bool canceled;
    FindType findType;

    unsigned char mDirectJoinState[0x28];

    bool mWaitingForQueries;
    unsigned char mQueryWaitPadding[3];
    float mQueryFinishTime;
    int mSelectedSession;
    int mPercentDone;
};
 
 
 

struct NetKey
{
    unsigned int id;
    unsigned int key;
};

struct FindGameData
{
    int mNumPlayers;
    int mMaxPlayers;
    char mName[64];
    char mMapName[32];
    char mMapType[32];
    bool mDidServerPing;
    float mPingTime;
    bool mLocked;
    int mServerType;
    unsigned short mSortOrder;
    bool mbVerMatch;
    bool mbFilteredOut;
    float mQuerySentTime;
    float mResponseRecvTime;
    unsigned int mQueryId;
    int mQuerySeqNo;
    bool mConnecting;
    bool mConnected;
    float mConnectTimeout;
    bool mbIsFavorite;

    char mGalaxyLobbyPasswordData[64];
};
 

struct FindGameRules
{
    int mNumReinforcements1;
    int mNumReinforcements2;
    int mNumAIUnits;
    int mMinPlayers;
    bool mAutoTeams;
    bool mTeamDamage;
    int mAIDifficulty;
    bool mAimAssist;
    bool mShowPlayerNames;
    int mSpawnInvincibilityTime;
    bool mHeroes;
    int mHeroUnlockRule;
    int mHeroUnlockRuleVal;
    int mHeroTeamRule;
    int mHeroPlayerRule;
    int mHeroRespawnRule;
    int mHeroRespawnRuleVal;
};
 

class NetGameDesc
{
public:

    NetGameDesc(const NetAddr& addr);

    bool operator==(const NetGameDesc& other);
    void Update(const NetAddr* address, const NetKey* key, const NetSessionId* session,
                const char* version, const FindGameData* data, const FindGameRules* rules);

    PblList<NetGameDesc>::Node mNode;
    bool mValid;
    bool mRulesAreValid;
    bool mHasSessionData;
    NetAddr mAddr;
    NetKey mKey;
    NetSessionId mPrevId;
    float mStamp;
    FindGameData mFindData;
    FindGameRules mFindRules;
    char mGameVer[16];
};
 

// VTABLE: SWBF2 0x007ac2f8
class CNetPostLoadHostGame : public NetPostLoadHostGame
{
public:
    CNetPostLoadHostGame();
    virtual ~CNetPostLoadHostGame();
    virtual void Cancel();
    virtual void Update();
    virtual void BootPlayer(int iPlayer);

    void SendUpdates();

    float timer;
    ShellState shellState;
    unsigned __int64 needSendMask;
    unsigned __int64 loadingMask;
    bool canceled;
};
 

// VTABLE: SWBF2 0x007ac320
class CNetPostLoadJoinGame : public NetPostLoadJoinGame
{
public:

    CNetPostLoadJoinGame()
        : connectingMask(0), connectFailedMask(0), canceled(false), nextStats(0.0f)
    {
    }
    virtual ~CNetPostLoadJoinGame();
    virtual void Cancel();
    virtual void Update();

    unsigned __int64 connectingMask;
    unsigned __int64 connectFailedMask;
    bool canceled;
    float nextStats;
};
 

class NetPktGroup;

class NetSetup
{
public:

    static char* GetGameType();
    static int GetNumPlayers();
    static int GetMaxPlayers();

    static void GetSpecialJoinAddress(NetSessionId* pOut);
    static void SetSpecialJoinValid(bool bValid);
    static void WriteGameUpdate(NetPktGroup& group);

    static void PlayerDataChanged(int iChangedPlayer);
    static void AddGamePlayer(int iChangedPlayer);
    static void RemoveGamePlayer(int iPlayer);
    static void ReadGameUpdate(NetPktGroup* pGroup);
    static unsigned int CalcPassword(char const* password);
    static void SetPassword(char const* password);

    static unsigned int GetPassword();

    static void Create(bool bMission);

    static char* GetGameVersion();

    static const char* GetHostName();

    static NetLaunchGame* LaunchGame(bool bHost);

    static void SetLoginName(int iViewport, char const* pcName);
    static bool InNetGame();
    static bool SetInNetGame(bool bInGame);
    static bool IsLoginError();
    static bool IsGameError();
    static void GetLoginError(char const*& pcError, int& iSeverity);
    static void GetGameError(char const*& pcError, int& iSeverity);
    static void SetErrorMode(bool bMode);
    static void GetPrevSessionId(NetSessionId* pId);
    static void StartLogin(int iViewport, bool bArg2, char const* pcArg3, char const* pcArg4,
                           char const* pcArg5, bool bArg6);
    static bool IsLoginDone(int iViewport);
    static bool IsLoginSuccessful(int iViewport, bool bArg);
    static void CancelLogin(int iViewport, bool bArg);
    static NetFindGame* FindGame(FindType eType, char const* pcName, NetSessionId* pId);
    static NetJoinGame* JoinGame(NetFindGame* pFind);

    static char const* GetAutoNetScript();
    static bool IsAutoNetLan();
    static char const* GetAutoNetName();
    static int GetAutoNetPlayerCount();
    static void SetupAutoNetMissions();
    static void BeginShell();
    static void EndShell();
    static void SetAsHost();
    static void SetAsClient();
    static void SetAsDedicated(bool bDedicated);
    static void SetGameName(char const* pcName);
    static bool InMultiplayer();
    static bool WasHost();
    static bool WasDedicated();
    static bool WasClient();
    static bool IsInSession();
    static void RequestQuit(unsigned int uiReason);
    static bool IsNetCableConnected();
    static bool IsErrorDismissed();
    static void HandleError(bool bArg);
    static void EnableOnlineService(bool bEnable);
    static char const* GetOnlineService();
    static char* GetIPAddr();
    static NetHostGame* HostGame();

    static NetPostLoadHostGame* PostLoadHostGame();

    static NetLoadGame* LoadGame(bool bHost);
    static NetPostLoadJoinGame* PostLoadJoinGame();

    static NetSessionId mSpecialJoinAddress;

    static bool GetSpecialJoinValid();
    static bool mSpecialJoinValid;

    static bool mNoticeNoCable;
public:
    static bool mLoginErrorsAreFatal;
    static bool mPrivateGame;
    static int mPasswordHash;
};

int NetGetHostPlayer();

class NetSetupP
{
public:
    static void Update();
    static void UpdateScreen();
    static NetAddr& GetMyAddr();

    static void Recv(NetPacket* pPacket, const void* pSource);
    static void RecvLan(NetPacket* packet, const void* source);
    static void NextHost(int direction);
    static void AddGame(const NetAddr* address);
    static void UpdateGame(const NetAddr* address, const void* session, const void* key,
                           const char* version, const void* playerData, const void* extra);
};

extern NetHostGame* hostGame;
extern NetJoinGame* joinGame;
extern NetFindGame* findGame;

extern NetLoadGame* loadGame;
extern NetPostLoadHostGame* postLoadHostGame;
extern NetPostLoadJoinGame* postLoadJoinGame;
extern NetSessionId netPrevSessionId;
