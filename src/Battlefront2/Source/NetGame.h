#pragma once
#include "TargetManager.h"

extern bool netInShell;
extern bool netEnabled;
extern bool netEnabledNext;

extern bool netIsHost;

extern int netHostIndex;

extern bool netIsDedicated;
extern bool netIsOneWorld;
extern bool netClientActive;

class Character;
class ChunkCluster;
class EntityGeometryClass;
class CommandPost;
class EntityClass;
class Team;
class WeaponClass;
class GameObject;
class NetPktGroup;
class OrdnanceClass;
class PblVector3;
class PblMatrix;
class ProcessedInputs;
struct OrdnanceDesc;
class Weapon;
namespace HUD { class Objective; }

struct NetEventVector
{
    float x, y, z;
};
struct NetGameObjectHandle
{
    GameObject* ptr;
    unsigned int id;
};
struct NetEvent
{
    int mType;
    union U
    {
        struct OrdnanceEvent
        {
            NetEventVector mPosition, mDirection;
            NetGameObjectHandle mOwner;
            float mStrength;
            Character* mCharacter;
            GameObject* mGameObject;
            unsigned int mGameObjectId;
            float mDamageMultiplier;
            int mOrdClassIndex, mWpnClassIndex, mEntClassIndex;
            GameObject* mTarget;
            unsigned int mTargetId;
            Weapon* mFireWeapon;
        } ordnance;
    } u;
};
extern NetEvent sServerNetEvents[128];

class NetGame
{
public:

    static int GetSpawnOrder(int iPlayer);
    static bool ErrorDismissed();
    static void Reboot();

    static void WritePreGame(NetPktGroup& group);

    static void Init();

    static void Reset();

    static void SetPreGame(bool bEnable);

    static bool IsPreGame();

    static int GetCurDst();

    static bool JustEndedPreGame();
    static int GetStartTeam(int iPlayer);

    static bool IsLocalTo(int iPlayer, int iOther);
    static int GetMyGuest(int iPlayer, int iSlot);
    static int GetGuestIndex(int iPlayer, int iSlot);

    static int GetMaxPreGameTime();
    static void SetMaxPreGameTime(int iSeconds);

    static int GetPreGameTime();
    static const char* GetPlayerName(int iPlayer);

    static void CreateEvent_CreateOrdnance(const OrdnanceClass* pClass, const OrdnanceDesc& desc);

    static void CreateEvent_DestroyTowCable(GameObject* pOwner, Weapon* pWeapon, unsigned __int64);

    static void CreateEvent_ConnectTowCable(GameObject* pOwner, Weapon* pWeapon, const PblVector3& vDir,
                                            const PblVector3& vPos, int iBodyId, GameObject* pWalker,
                                            unsigned __int64);

    static void AddLateFence(int iPlayer);

    static void Created(GameObject* pObject);

    static void Destroyed(GameObject* pObject);

    static void CreateEvent_PlayVoiceOver(unsigned int uHash, unsigned int uTeamMask);

    static void CreateEvent_ScriptMessage(unsigned int uLocalizeKey, unsigned int uTeamMask);

    static void SendChatMessageToServer(NetPktGroup& group);
    static void SendChatMessageToClient(NetPktGroup& group);

    static void CreateEvent_UpdateDoorState(unsigned int uDoorId, unsigned __int64 uMask);

    static void CreateEvent_ChangeTeam(CommandPost* pPost, unsigned __int64 uMask);

    static float GetLocalTime();

    static bool IsSkipped(const GameObject* pObject, float& fDeltaTime);

    static int GetHostWorldUniqueId(int iNetUniqueId, GameObject* pObject);

    static bool IsMouseMode(int iPlayer);
    static void AcceptHero(int iTeam, int iPlayer);
    static void DeclineHero(int iTeam, int iPlayer);

    static void HeroSelected(int iTeam);

    static bool AreHeroRulesRunning(int iTeam);
    static bool CanExitVehicle(unsigned int uPlayer);

    static void SetEnterVehicleTurn(unsigned int uPlayer);
    static void CreateEvent_UpdateInvincibility(Character* pCharacter);

    static ProcessedInputs* GetJoystick(int iPlayer);

    static void ConfirmBootPlayer(int iJoystick);
    static int GetBootPlayer();
    static float GetBootTime();
    static int GetBootVotes();
    static bool IsBootVoted(int iJoystick);
    static void SetBootVoted(int iJoystick, bool bVoted);
    static int GetBootReason();
    static bool GetBootCount(int& iA, int& iB, int& iC);
    static int GetJoystickIndex(int iPlayer);

    static void SetPlayerGSID(int iPlayer, unsigned __int64 uId);

    static void FrameLock();

    static void NominateBootPlayer(int iPlayer, int iReason, int iJoystick);

    // FUNCTION: SWBF2 0x00414920
    static bool IsEnabled() { return netInShell ? netEnabled : netEnabledNext; }

    // FUNCTION: SWBF2 0x00429280
    static bool IsHost() { return netIsHost || netHostIndex >= 0; }

    // FUNCTION: SWBF2 0x004292a0
    static bool IsDedicated() { return netIsDedicated; }

    // FUNCTION: SWBF2 0x00414940
    static bool InShell() { return netInShell; }
    // FUNCTION: SWBF2 0x004292d0
    static bool IsOneWorld() { return netIsOneWorld; }

    // FUNCTION: SWBF2 0x004292b0

    static bool IsNearLocalPlayer(const PblVector3& vPos);
    static bool IsClient() { return netEnabledNext ? netClientActive : false; }

    enum MessageType
    {
        MSG_SUICIDE = 0,
        MSG_KILLED_TEAMMATE = 1,
        MSG_KILLED = 2,
        MSG_DIED = 3,
        MSG_CAPTURED = 4,
        MSG_LOST = 5,
        MSG_DISPUTED = 6,
        MSG_SAVED = 7,
        MSG_JOINED = 8,
        MSG_LEFT = 9,
        MSG_BOOTED = 10,
        MSG_NOTBOOT = 11
    };

    static void CreateEvent_CreateChunkCluster(const ChunkCluster* pCluster, EntityGeometryClass* pClass,
                                               unsigned __int64 uMask);
    static void CreateEvent_CorrectChunkCluster(const ChunkCluster* pCluster, EntityGeometryClass* pClass,
                                                unsigned int uChunk, unsigned __int64 uMask);

    static void CreateEvent_Message(MessageType eType, const Character* pA, const Character* pB,
                                    EntityClass* pClass, WeaponClass* pWeapon, Team* pTeam,
                                    CommandPost* pPost);

    static void Create(bool bMission);

    static void CreateClient();

    static void CreateLast();

    static unsigned int GetNumLocalPlayers();
    static void SendHost();

    static void ReceiveClient();

    static void AddPlayer(int iPlayer);
    static void RemovePlayer(int iPlayer);
    static void FinishTurn();
    static GameObject* GetClientNDObject(GameObject* pObject);

    static int GetLocalIndex(int iLocal);

    static Character* GetLocalPlayer(unsigned int uLocal);

    static bool IsPlaying(int iPlayer, bool bNext);

    static void AddToSpawnQueue(int iPlayer);
    static void RemoveFromSpawnQueue(int iPlayer);

    static void GetPlayerQOS(int iPlayer, unsigned int& uUpdates,
                             unsigned int& uFills, int& iLoss);
    static float GetTurnRatio();
    static float GetTimeScale();
    static int GetSpectator(int iPlayer);
    static bool IsSpectator(int iPlayer);
    static void FreeAllChatMessageQue();

    static void ReadChatMessageFromClient(NetPktGroup& group);

    static void SetSpectator(int iPlayer, int iSpectateID);
    static void WriteSpectator(NetPktGroup& group);
    static void ReadSpectator(NetPktGroup& group);

    static void EnterClient();
    static void LeaveClient();

    static unsigned int GetClientAckedTurnNumber();
    static unsigned int GetClientTurnNumber();
    static bool IsCockpitCam(int iPlayer);
    static void SetForceHumansToTeam1(bool bForce);
    static bool GetForceHumansToTeam1();
    static bool DoesHostKnowILoaded();
    static bool IsReadyToPlay();
    static bool IsGameOver();
    static bool CanQuit();
    static void BeginShell(int iStage);
    static void EndShell(int iStage);
    static void DestroyClient();
    static void CreateVeryLast();

    static void Update();

    static bool AutoBootLagger();

protected:
    static int GetRandomHeroTeam();

    static void HeroRulesUpdate(int iTeam);

public:

    static bool IsLateTurn();
    static void LeaveTurn();
    static void EnterTurn();
    static unsigned int GetKey(int index);
    static void SendClient();
    static void ReceiveHost();
    static void SubmitMove(int iTurns);
    static int GetMaxSimTurns();
    static bool IsTurnReady();
    static void SaveLateState();
    static void ApplyLateMoves();
    static float GetTurnStep();
    static void SetTurnRatio(float fTime);
    static bool FinishedTimeScale();
    static float GetTimeAdjust();
    static bool Predict();
    static CommandPost* ConvertCP(CommandPost* pPost);
    static void CreateEvent_ChangeCommandPostTeams(unsigned __int64 sendMask);
    static void CreateEvent_UpdateObjective(HUD::Objective* objective, int maskLow, int maskHigh);

    static void CreateEvent_AddObjective(HUD::Objective* objective);
    static void EnableJournal();
    static void EnablePlayback();
    static bool IsPlayingNext(int player);
    static unsigned int GetAckedUpdateTurn(int iPlayer);
    static unsigned __int64 GetPlayerGSID(int player);
    static int FindPlayerByGSID(int low, int high);
    static void ClearPlayerCDKey(int);
    static void ClearAllCDKeyInfo();
    static int GetGuestOf(unsigned int iPlayer);
    static int FindSpectateTarget(int player);
    static int FindSpectateTarget(int player, int team);
    static bool ChangeSpectateTrack(int player, int spectator);
    static void SetPlayerToSpectator(int player);
    static void HostChangeSpectator();
    static void ClientUpdateSpectator();
    static void SetNextSpectator(int iPlayer, int iSpectator, ProcessedInputs* pState);
    static void UpdatePreGame();
    static void ReadPreGame(NetPktGroup& group);
    static void HostChangePregame();
    static void VanishAllPlayer();
    static void SetSpawning(int player, bool spawning);
    static bool IsSpawning(int player);
    static void SetStartTeam(int player, int team);
    static bool DetectError();
    static bool IsJournal();
    static bool IsFullState();
    static unsigned int GetTurnNumber();
    static void Unlink(GameObject* pObject);
    static void HeroSpawned(int team);
    static void HeroKilled(int iTeam);
    static void ClearLastHero(int team);
    static int GetObjectsInRange(const PblVector3& position, float radius, GameObject** const found, int capacity, const Team* excluded, int team);
    static void SelectSpawn(int player, int team, int classIndex, CommandPost* post);
    static void CancelSpawn(int player);
    static void SpectateSpawn(int player, bool spectate);
    static void AutoAssignSpawn(int player, int autoassign);
    static void RequestSuicide(int index);
    static void ForceBootPlayer(int player);
    static void AddChatMessageToQue(bool local, int player, unsigned __int64 mask, int code, const unsigned short* text, int flag);
    static void ReadChatMessageFromServer(NetPktGroup* group);
    static void GetScore(int* count1, int* count2, int* limit1, int* limit2, bool* limits, bool* points);
};

class NetGameP
{
public:
    static void SetGuestOf(int iPlayer, int iHost);
    static void SetPlayerName(int iPlayer, char const* pcName);
    static int& GetHostIndex();

    static int* GetHostIndex(int iPlayer);
    static void SetTurnRate(int iRate, int iArg);
    static void SetPlaying(unsigned __int64 uMask, bool bPlaying);

    static void SetClientTurnNumber(unsigned int uTurn);

public:

    static void SetNotPlaying(int player);
    static void SetKey(int iPlayer, int iKey);
    static int* GetClientOfIndex(int iPlayer);
    static int* GetHostMode(int iHost);
    static void SetPlayerCanHost(int iPlayer, char bReplica, char bRelay);
    static bool HasHostCycled();
    static void FreePackets();
};

void UpdateCurMaxPlayers(unsigned __int64 playingMask, bool includeLast);

class NetQOS
{
public:
    static void Init();
    static void Create(bool bMission);
    static void Reset();

    static void BeginShell();
    static void EndShell();

public:

    static bool Update(bool);

    static void CancelGame();
    static bool IsDone();
};

void CloseJournal();

#include <stddef.h>

class MemoryPool;

class NetObj
{
public:

    static void* operator new(size_t uSize);

    GameObject* obj;
    unsigned int uniqId;
    unsigned int usedTurn;

    static MemoryPool sMemoryPool;
};
 

#include <stddef.h>
#include <string.h>

#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"

enum HostMode
{
    HM_HOST = 0,
    HM_RELAY = 1,
    HM_REPLICA = 2,
    HM_NONE = 3
};

enum SwitchResponse
{
    SR_NONE = 0,
    SR_CAN = 1,
    SR_CANT = 2
};

SwitchResponse ReadSwitchResponse(NetPktGroup& group, unsigned int turn, unsigned int otherTurn);

class StickAxisConvert {};
class MouseAxisConvert {};
template <class Convert> void ReadFixed(Convert& convert, NetPktGroup& group, float& value);
template <> void ReadFixed<StickAxisConvert>(StickAxisConvert& convert, NetPktGroup& group, float& value);
template <> void ReadFixed<MouseAxisConvert>(MouseAxisConvert& convert, NetPktGroup& group, float& value);
EntityClass* ReadEntityClass(NetPktGroup& group);
WeaponClass* ReadWeaponClass(NetPktGroup& group);
bool AcceptLocalPlayerMessage(const Character* character, int value);
bool ReadMove(NetPktGroup& group, const ProcessedInputs& previous, ProcessedInputs& current);
unsigned int ReadMoves(NetPktGroup& group, int player, unsigned int turn);
void ReadSpawnDisplay(NetPktGroup& group);

struct SwitchResponseRelayInfo
{
    SwitchResponse switchResponse;
    unsigned int switchTurn;
};

class NG_ChatMsg
{
public:
    unsigned int player_id;
    unsigned int flag;
    unsigned __int64 mask;
    int code;
    unsigned short msg[64];
    PblList<NG_ChatMsg>::Node mNode;
};
 

class NetObjState;

class NetObjStateMap
{
public:

    NetObjStateMap()
    {
        turn = (unsigned int)-1;
        next = 0;
        memset(stateMap, 0, sizeof(stateMap));
    }

    static NetObjStateMap* Alloc();
    void Free();

    NetObjState* stateMap[64];
    NetObjStateMap* next;
    unsigned int turn;

    static NetObjStateMap* topFree;
};
 

struct NetHost
{
    void Init(bool bMission);

    HostMode hostMode;
    bool isDedicated;
    unsigned int updateTurn;
    unsigned int hostAckedUpdateTurn;
    unsigned int clientMask;
    unsigned int key;
};
 

struct NetPlayer
{
    NetPlayer();
    void Init(bool bMission);
    void Clean();

    NetObjStateMap* lastObjState;
    NetObjStateMap* nextObjState;
    SwitchResponseRelayInfo objSwitchRelay;
    unsigned int reliableEventHead;
    unsigned int reliableEventTail;
    unsigned int reliableBatchNumber;
    unsigned int reliableSendTurn;
    unsigned int nextUpdateTurn;
    unsigned int ackedUpdateTurn;
    unsigned int clientTurn;

    unsigned int mLastReceivedTurn;
    unsigned int nextSendPingIndex;
    unsigned int lastEnterVehicleTurn;
    int netEventTail;
    unsigned int lagSample;
    int lagSamples[20];
    unsigned int lagTurns[20];
    float avgLag;
    float errLag;
    char name[32];
    char prevName[32];
    int hostIndex;
    int clientOfIndex;
    bool canReplica;
    bool canRelay;
    unsigned int updateCount;
    unsigned int filledCount;
    unsigned int dropCount;
    unsigned int packetCount;
    bool mBooted;
    float mBootTime;
    bool mDeclinedHero;
    bool requestedQuit;
    int myGuest[1];
    int guestOf;
    bool isTeam1;
    bool mIsSpawning;
    int mSpawnOrder;
    int mSpectateID;

    int mPlayerIdAlignmentStorage;

    unsigned __int64 mGSPlayerID;

    unsigned char skippedSends[64];
    int updateWindowSize;
    unsigned int sentUpdateTurn[2];
    float sentUpdateTime[2];
    float averageRtt;
    float deviationRtt;
    unsigned int key;
    unsigned int spawnChangeCount;
    bool spawnRequested;
    bool selectedSide;
    int spawnPost;
    int spawnClass;
    float nextStats;
    bool hasStats;
    unsigned int lateFenceTurn;

    unsigned int mLastStatusTurn;
    PblVector3 spawnCenter;
    PblVector3 spawnFront;
};
 

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

enum AtleastOnceType
{
    ALO_NONE = 0,
    ALO_TRACKABLE = 1,
    ALO_REINFORCEMENTS = 2,
    ALO_HERO_DATA = 3,
    ALO_MAPDISPLAY_MARKERS = 4,
    ALO_STATS = 5,
    ALO_MINI_STATS = 6,
    ALO_BOOT_VOTE = 7,
    ALO_NEXT_GAME_INFO = 8,
    ALO_MISSION_TIMER = 9
};

class AtleastOnce
{
public:
    static void ResetAll();
    static void WriteAll(NetPktGroup& group);

    AtleastOnceType sig;

public:

    static void ReadAll(NetPktGroup& group);
    static void DropAll(int player);
};

class AtleastOnceSame : public AtleastOnce
{
public:
    AtleastOnceSame() : deltaSends(true), changeTurn((unsigned int)-1) {}
    bool IsConfirmed();
    bool Send(NetPktGroup& group);

    bool deltaSends;
    unsigned int changeTurn;
};
 

class AtleastOnceUniq : public AtleastOnce
{
public:
    AtleastOnceUniq()
    {
        deltaSends = true;
        for (int i = 0; i < 64; i++)
        {
            changeTurn[i] = (unsigned int)-1;
        }
    }

    bool Send(NetPktGroup& group);
    bool CanChange();

    bool deltaSends;
    unsigned int changeTurn[64];
};
 

class AloReinforcements : public AtleastOnceSame
{
public:
    AloReinforcements()
    {
        sig = ALO_REINFORCEMENTS;
        deltaSends = false;
        Reset();
    }
    void Reset();
    void Update(NetPktGroup& group);
    void Read(NetPktGroup& group);
    void Set(int iTeam, int iCount, int iActive, float fBleed, int iPoints);
    void Write(NetPktGroup& group);

    int teamCount[2];
    int teamActive[2];
    float teamBleed[2];
    int teamPoints[2];
};
 

class AloHeroData : public AtleastOnceSame
{
public:
    AloHeroData(int iTeamNum) : mTeamNum(iTeamNum)
    {
        sig = ALO_HERO_DATA;
        deltaSends = false;
        Reset();
    }
    void Reset();
    void Update(NetPktGroup& group);
    void Read(NetPktGroup& group);
    void Set();
    void Write(NetPktGroup& group);

    int mTeamNum;
    int heroPlayerID;
    bool heroUnlocked;
    int nextHeroPlayerID;
    int lastHeroPlayerID;
    int heroKillerID;
    unsigned int heroSelectedTurn;
    unsigned int heroSpawnedTurn;
    unsigned int heroDiedTurn;
};
 

class AloMissionTimer : public AtleastOnceSame
{
public:
    void Update(NetPktGroup& group);
    AloMissionTimer()
    {
        sig = ALO_MISSION_TIMER;
        deltaSends = false;
        Reset();
    }
    void Reset();
    void Read(NetPktGroup& group);
    void Set(bool bValid, float fTime, float fRate,
                    bool bActive, int iShow, int iTeam);
    void Write(NetPktGroup& group);

    bool timerValid;
    float timerTime;
    float timerRate;
    bool timerActive;
    int timerShow;
    int timerTeam;
};
 

class Trackable;

class AloTrackable : public AtleastOnceUniq
{
public:
    AloTrackable();
    void Update(NetPktGroup& group);
    void Write(NetPktGroup& group);
    void Reset();
    void Drop(int iPlayer);
    void Read(NetPktGroup& group);
    void Set(Trackable* pTrackable, Trackable* const* ppOther);

    Trackable* mTrackable[64][2];
};
 

class AloMiniStats : public AtleastOnceUniq
{
public:
    AloMiniStats();
    void Update(NetPktGroup& group);
    void Reset();
    void Drop(int iPlayer);
    void Read(NetPktGroup& group);
    void Set(unsigned int uEnemyKills, unsigned int uTeamKills,
                    unsigned int uDeaths, unsigned int uCps, short iPoints);
    void Write(NetPktGroup& group);

    unsigned int mEnemyKills[64];
    unsigned int mTeamKills[64];
    unsigned int mDeaths[64];
    unsigned int mCps[64];
    short mPoints[64];
};
 

class AloStats : public AtleastOnceSame
{
public:
    void Update(NetPktGroup& group);
    bool Sent();
    AloStats()
    {
        sig = ALO_STATS;
        lastGameOver = false;
        gotGameOver = false;
        changeTurn = (unsigned int)-1;
    }

    bool lastGameOver;
    bool gotGameOver;
};
 

class AloNextGameInfo : public AtleastOnceSame
{
public:
    void Update(NetPktGroup& group);
    void Read(NetPktGroup& group);
    void Write(NetPktGroup& group);
    bool Sent();
    AloNextGameInfo()
    {
        sig = ALO_NEXT_GAME_INFO;
        lastGameOver = false;
        gotGameOver = false;
        changeTurn = (unsigned int)-1;
    }

    bool lastGameOver;
    bool gotGameOver;
};
 

void WriteObjectPtr(NetPktGroup& group, const GameObject* pObject);
GameObject* ReadObjectPtr(NetPktGroup& group);
void MarkWriteAlign(NetPktGroup& group);
void CheckReadAlign(NetPktGroup& group);
struct ReliableNetEvent;
void WriteEvent_Message(NetPktGroup& group, const ReliableNetEvent& event);
void WriteEvent_ChangeTeam(NetPktGroup& group, const ReliableNetEvent& event);
void WriteMoves(NetPktGroup& group, int iPlayer, unsigned int uTurn);
int FrontCount(GameObject* const* ppObjects, int iCount,
               int iCurrent, const PblMatrix& camera);
float GetNetEventScore(NetEvent& event);
void SendReliableNetEvents(NetPktGroup& group);
void WriteObject(NetPktGroup& group, GameObject* pObject);
void WriteSpawnDisplay(NetPktGroup& group);
void WritePlayerMoves(NetPktGroup& group);
void WriteSpawnCamera(NetPktGroup& group);
void AddSendLate();
int ComputeNetLOD(int iUnused, int iPlayer);
unsigned int __fastcall sub_5D3720(const void* pThis);

class AloBootVote
{
public:
    void Read(NetPktGroup& group);
    void Update(NetPktGroup& group);
};
 

class AloMapDisplayMarkers : public AtleastOnceSame
{
public:
    AloMapDisplayMarkers();
    void Update(NetPktGroup& group);
    void Read(NetPktGroup& group);
    void Set(int iMarker, const TargetManager::Marker* pMarker);
    void Write(NetPktGroup& group);

    TargetManager::Marker markers[32];
};
 

struct RecentObjectHandle
{

    // FUNCTION: SWBF2 0x0042e950
    RecentObjectHandle() : mObject(0), mHandleId(0) {}
    ~RecentObjectHandle() {}

    void* mObject;
    unsigned int mHandleId;
};

struct RecentGuestObjectRecord
{
    RecentObjectHandle mObjectHandle;
    float mRecordedTime;
};
