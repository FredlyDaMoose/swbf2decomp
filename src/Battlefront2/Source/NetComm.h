#pragma once
#include <stddef.h>

#include "NetPacket.h"

enum PipeType
{
    PIPE_LAN = 0,
    PIPE_INTERNET = 1,
    PIPE_MAX = 2
};

enum NetArch
{
    NET_ARCH_PS2 = 0,
    NET_ARCH_PC = 1,
    NET_ARCH_XBOX = 2,
    NET_ARCH_ANY = 3,
    NET_ARCH_FAKE = 3
};

struct NetDest
{
    bool connected;
    NetPktGroup outQueue;

    char mAddressStorage[0x18];
    NetArch arch;
    float lastRecv;
    PipeType pipe;
    int ping;
    int PLKills;
    int PLDeaths;
    int PLCPs;
    int upBandwidth;
    int downBandwidth;
    int nextGroupNum;
};
 

const int MAX_DESTS = 67;

enum TicketHolder
{
    TKT_UPDATE = 0,
    TKT_QOS = 1,
    TKT_MAX = 2
};

extern bool netIsHost;

enum PktType
{
    PKT_FIND = 0,
    PKT_FOUND = 1,
    PKT_QUERY = 2,
    PKT_QUERY_RESPONSE = 3,
    PKT_JOIN = 4,
    PKT_REJECT = 5,
    PKT_SHELL_UPDATE = 6,
    PKT_SHELL_MOVE = 7,
    PKT_LAUNCH = 8,
    PKT_LOAD = 9,
    PKT_WAITING = 10,
    PKT_GAME_MOVE = 11,
    PKT_GAME_UPDATE = 12,
    PKT_GAME_RELAY_UPDATE = 13,
    PKT_GAME_RELAY_UPDATE_LOCAL = 14,
    PKT_GAME_REPLICA_UPDATE = 15,
    PKT_WAITING_ACK = 16,
    PKT_LAUNCH_ACK = 17,
    PKT_LOAD_ACK = 18,
    PKT_STATS_MOVE = 19,
    PKT_STATS_UPDATE = 20,
    PKT_QOS = 21,
    PKT_QOS_ACK = 22,
    PKT_QOS_LOW = 23,
    PKT_VOICE = 24,
    PKT_PLAYERCHATCONFIG = 25,
    PKT_NN_HOST = 26,
    PKT_NN_CLIENT = 27,
    PKT_CONSOLE = 28,
    PKT_CONSOLE_ACK = 29,
    PKT_NONE = 30,
    PKT_CDKEY = 59,
    PKT_QR1 = 92,
    PKT_NN = 253,
    PKT_QR2 = 254
};

enum ConnectType
{
    CT_LINK = 0,
    CT_LAN = 1,
    CT_WAN = 2
};

class NetComm
{
public:

    struct PktHeader
    {
        unsigned short group;
        unsigned char type;
        unsigned char src;
        unsigned char idx;
        unsigned char count;
        unsigned char compressed;
    };

    static PktHeader& Header(NetPacket* pPacket)
    {
        return *reinterpret_cast<PktHeader*>(pPacket->mData);
    }

    static NetPacket* Recv(PktType eType, PktType eType2);

    static NetPacket* RecvRange(int iFirst, int iLast);

    static int GetDirectPort();

    static void Init();

    static void Create(bool bMission);

    static int GetLocalIndex(int iLocal);

    static void GetBytes(unsigned int& uIn, unsigned int& uOut);
    static int GetPing(int iIndex);

    static int GetPLKills(int iDest);

    static void GetBandwidth(int iDest, int& iLimit, int& iOther);
    static void SetBandwidth(int iDest, int iLimit, int iOther);

    static void SetPLKills(int iIndex, int iKills);
    static void SetPLCommandPostPoints(int iIndex, int iPoints);
    static void SetPLDeaths(int iIndex, int iDeaths);

    static float GetTime();

    static NetArch GetMyArch();

    static bool IsLAN();
    static int SetBandwidthPercent();
    static void SendVoice(int iDest, NetPacket* pPacket, unsigned short uSequence);

    static NetArch GetArch(int iDest);

    // FUNCTION: SWBF2 0x00591420
    static bool IsHost() { return netIsHost; }

    static bool IsConnected(int iDest);

    static void CreateClient();
    static void Reset();
    static void BeginShell();
    static void EndShell();

    static void GetTicket(TicketHolder eHolder, PipeType ePipe);

    static bool IsTicketReady(TicketHolder eHolder, PipeType ePipe);
    static void UseTicket(TicketHolder eHolder, PipeType ePipe);
    static float GetLastRecv(int iDest);
    static PipeType GetPipe(int iDest);
    static bool IsPipeFull(PipeType ePipe, int iPacketBytes);

    static void SetHost(bool bHost);

    static void SetConnectType(ConnectType eType);

    static ConnectType GetConnectType();
    static int GetJoystickIndex(int iJoystick);
    static void SetPing(int iDest, int iPing);
    static void AdvanceTime(bool bArg);

    static void Update();
    static void Send(int iDest, PktType uType, NetPacket* pPacket);
    static void Send(int iDest, int uType, NetPktGroup* pGroup);
    static bool Recv(NetPktGroup* pGroup, int eType);
    static void Disconnect(int iDest);
};
 
 
 
 
 
 

class NetCommP
{
public:

    static void SetArch(int iDest, NetArch arch);

    static void __fastcall RecvPacket(NetPacket* pPacket, const void* pSource);

    static bool& GetConnected(int iDest);
    static int& GetLocalIndex(int iDest);
    static float& GetLastRecv(int iDest);

    static PipeType& GetPipe(int iDest);
    static bool SameAddr(int iDest, int iOther);

    static void FreePackets();
    static void __fastcall Connect(int iDest, const void* pAddress);
    static unsigned __int64* __fastcall GetAddr(int iDest);
    static int __fastcall FindJoinSource(const int* pAddress);
};
