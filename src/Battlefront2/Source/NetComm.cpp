#include <string.h>
#include <winsock2.h>
#include <windows.h>

#include "NetComm.h"
#include "NetGame.h"
#include "GameLoop.h"
#include "pcNetSocket.h"
#include "PebbleFL/Common/PblTimeBase.h"
#include "PebbleFL/Common/PblJournal.h"

#include "NetSetup.h"
#include "Steam/galaxy_api.h"

bool __fastcall RoutesToHostDestination(unsigned int uType);
extern int netCurMaxHosts, netCurMaxPlayers;

extern int gDebugCount;
bool __fastcall IsAdminPacketAuthorized(const unsigned char* pPacket);

class NetDrag
{
public:
    static void Send(NetPacket* pPacket, int iDest);
    static void FreePackets();
    static void Update();
    static NetPacket* ReceiveLocal();
};

int netLocalIndex[2];

// GLOBAL: SWBF2 0x01ecc0c0
// SYNTHETIC: SWBF2 0x00404400
// `dynamic initializer for 'netDests''

NetDest netDests[MAX_DESTS];

float netTime;

// GLOBAL: SWBF2 0x01e57a0d
bool netIsHost;

// GLOBAL: SWBF2 0x01ecd9e0
// SYNTHETIC: SWBF2 0x00404460
// `dynamic initializer for 'netTimeBase''
PblTimeBase netTimeBase;

// GLOBAL: SWBF2 0x007ec248
// SYNTHETIC: SWBF2 0x00404470
// `dynamic initializer for 'netTimeStamp''
PblTimeBase::Stamp netTimeStamp(&netTimeBase);

// GLOBAL: SWBF2 0x01ecb2a0
// SYNTHETIC: SWBF2 0x00404490
// `dynamic initializer for 'inPktGroups''
NetPktGroup inPktGroups[64];

// GLOBAL: SWBF2 0x01ecb9a0
NetPktGroup inQueue;

// GLOBAL: SWBF2 0x01ecb9c0
// SYNTHETIC: SWBF2 0x00404430
// `dynamic initializer for 'joinSlots''

int joinSlots[0x1C0];

// GLOBAL: SWBF2 0x01fa3d48
unsigned __int64 lastTicks;

// GLOBAL: SWBF2 0x01fa3d50
int ticketTail[2];

// GLOBAL: SWBF2 0x01fa3d30
TicketHolder ticketQueue[PIPE_MAX][3];

// GLOBAL: SWBF2 0x007e8008
int bwPercent = 100;

extern int TEMP_bandwith;

// GLOBAL: SWBF2 0x01ecd914
int gInternetBandwidthScale;

extern int bps[PIPE_MAX];
extern float sendBits[PIPE_MAX];

// GLOBAL: SWBF2 0x01fa3d2c
int inBytes;
// GLOBAL: SWBF2 0x01e62e8c
int netOutBytes;

// FUNCTION: SWBF2 0x005b23d0
void NetComm::Init()
{
    netLocalIndex[0] = -1;
    netLocalIndex[1] = -1;

    for (int i = 0; i < MAX_DESTS; i++)
    {
        netDests[i].pipe = PIPE_INTERNET;
        netDests[i].ping = 250;
        netDests[i].PLKills = 0;
        netDests[i].PLDeaths = 0;
        netDests[i].upBandwidth = 0x10000;
        netDests[i].downBandwidth = 0x10000;
        netDests[i].nextGroupNum = 0;
    }

    NetSocket::StartSockets();
}

// FUNCTION: SWBF2 0x005b2430
void NetComm::Create(bool bMission)
{
    NetPacket::Create();

    LARGE_INTEGER liTicks;
    QueryPerformanceCounter(&liTicks);
    lastTicks = liTicks.QuadPart;
    netTimeBase.AdvanceTicks(lastTicks);
    netTimeStamp.Mark();
    netTime = netTimeStamp.GetElapsed();

    if (!bMission)
    {
        netLocalIndex[0] = -1;
        netLocalIndex[1] = -1;
    }

    inBytes = 0;
    netOutBytes = 0;

    const float fTime = GetTime();
    for (int i = 0; i < MAX_DESTS; i++)
    {
        NetDest& dest = netDests[i];
        dest.nextGroupNum = 0;
        if (!bMission)
        {
            dest.connected = false;
            dest.lastRecv = 0.0f;
            dest.ping = 250;
            dest.PLDeaths = 0;
            dest.PLKills = 0;
            dest.upBandwidth = 0x10000;
            dest.downBandwidth = 0x10000;
            dest.pipe = PIPE_INTERNET;
            dest.arch = GetMyArch();
        }
        dest.outQueue.Reset();
        dest.lastRecv = fTime;
    }

    inQueue.Reset();
    for (int i = 0; i < 64; i++)
    {
        inPktGroups[i].Reset();
    }

    ticketTail[0] = 0;
    ticketTail[1] = 0;
    memset(joinSlots, 0, sizeof(joinSlots));

    if (!NetGame::IsEnabled())
    {
        for (unsigned int i = 0; i < 2; i++)
        {
            if (!bMission || i < GameLoop::GetNumCameras())
            {
                netLocalIndex[i] = i;
            }
            else
            {
                netLocalIndex[i] = -1;
            }
        }
        netIsHost = true;
    }
}

// FUNCTION: SWBF2 0x005b25d0
void NetComm::CreateClient()
{
    if (NetGame::IsEnabled())
    {
        int iLocal = GetLocalIndex(0);
        if (iLocal >= 0)
        {
            netDests[iLocal].pipe = PIPE_LAN;
        }
    }
}

// GLOBAL: SWBF2 0x01e57a0e
bool isLink;

extern bool isLAN;

// FUNCTION: SWBF2 0x005b2650
void NetComm::EndShell()
{
}

// FUNCTION: SWBF2 0x005b2620
void NetComm::BeginShell()
{
    const float fTime = GetTime();
    for (int i = 0; i < MAX_DESTS; ++i)
    {
        netDests[i].lastRecv = fTime;
    }
}

// FUNCTION: SWBF2 0x005b36f0
void NetComm::GetBytes(unsigned int& uIn, unsigned int& uOut)
{
    uIn = inBytes;
    uOut = netOutBytes;
}

// FUNCTION: SWBF2 0x005b2720
bool NetComm::IsConnected(int iDest)
{
    return netDests[iDest].connected;
}

// FUNCTION: SWBF2 0x005b35d0
void NetComm::SetPLCommandPostPoints(int iIndex, int iPoints)
{
    netDests[iIndex].PLCPs = iPoints;
}

// FUNCTION: SWBF2 0x005b35e0
void NetComm::SetPLDeaths(int iIndex, int iDeaths)
{
    netDests[iIndex].PLDeaths = iDeaths;
}

// FUNCTION: SWBF2 0x005b2670
void NetComm::SetConnectType(ConnectType eType)
{
    switch (eType)
    {
    case CT_LINK:
        isLink = true;
        isLAN = true;
        break;
    case CT_LAN:
        isLink = false;
        isLAN = true;
        break;
    case CT_WAN:
        isLAN = false;
        isLink = false;
        break;
    }
}

// FUNCTION: SWBF2 0x005b2660
void NetComm::SetHost(bool bHost)
{
    netIsHost = bHost;
}

// FUNCTION: SWBF2 0x005b26b0
ConnectType NetComm::GetConnectType()
{
    if (isLAN)
    {
        return isLink ? CT_LINK : CT_LAN;
    }
    return CT_WAN;
}

// FUNCTION: SWBF2 0x005b26f0
int NetComm::GetJoystickIndex(int iJoystick)
{
    if (iJoystick < 0)
    {
        return -1;
    }
    if (netInShell && iJoystick < 2)
    {
        return iJoystick;
    }
    for (int i = 0; i < 2; ++i)
    {
        if (netLocalIndex[i] == iJoystick)
        {
            return i;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x005b26e0
int NetComm::GetLocalIndex(int iLocal)
{
    return netLocalIndex[iLocal];
}

// FUNCTION: SWBF2 0x005b28a0
float NetComm::GetTime()
{
    return netTime;
}

bool IsSame(const void* pLeft, const void* pRight);

// FUNCTION: SWBF2 0x005b2ce0
bool NetCommP::SameAddr(int iDest, int iOther)
{
    if (iDest == iOther)
    {
        return true;
    }
    const int iLocal = NetComm::GetLocalIndex(0);
    if (iDest == iLocal || iOther == iLocal)
    {
        return false;
    }
    return IsSame(NetCommP::GetAddr(iDest), NetCommP::GetAddr(iOther));
}

// FUNCTION: SWBF2 0x005b2d80
NetArch NetComm::GetMyArch()
{
    return NET_ARCH_PC;
}

// FUNCTION: SWBF2 0x005b2d90
NetArch NetComm::GetArch(int iDest)
{
    return netDests[iDest].arch;
}

// FUNCTION: SWBF2 0x005b2da0
void NetCommP::SetArch(int iDest, NetArch arch)
{
    netDests[iDest].arch = arch;
}

// FUNCTION: SWBF2 0x005b34e0
void NetComm::GetTicket(TicketHolder eHolder, PipeType ePipe)
{
    ticketQueue[ePipe][ticketTail[ePipe]++] = eHolder;
}

// FUNCTION: SWBF2 0x005b3500
bool NetComm::IsTicketReady(TicketHolder eHolder, PipeType ePipe)
{
    return ticketQueue[ePipe][0] == eHolder;
}

// FUNCTION: SWBF2 0x005b3510
void NetComm::UseTicket(TicketHolder, PipeType ePipe)
{
    int iCount = ticketTail[ePipe];
    for (int i = 0; i < iCount; ++i)
    {
        ticketQueue[ePipe][i] = ticketQueue[ePipe][i + 1];
    }
    ticketTail[ePipe] = iCount - 1;
}

// FUNCTION: SWBF2 0x005b2bd0
float NetComm::GetLastRecv(int iDest)
{
    return netDests[iDest].lastRecv;
}

// FUNCTION: SWBF2 0x005b3470
PipeType NetComm::GetPipe(int iDest)
{
    return netDests[iDest].pipe;
}

// FUNCTION: SWBF2 0x005b3490
bool NetComm::IsPipeFull(PipeType ePipe, int iPacketBytes)
{
    float fSent = sendBits[ePipe];
    if (1024.0f > fSent)
    {
        return false;
    }
    return (float)(iPacketBytes * 8) * 0.75f > (float)bps[ePipe] / 20.0f - fSent;
}

// FUNCTION: SWBF2 0x005b3640
void NetComm::SetBandwidth(int iDest, int iUp, int iDown)
{
    netDests[iDest].upBandwidth = iUp;
    netDests[iDest].downBandwidth = iDown;
    if (netIsHost && iDest == 64)
    {
        bool bLAN = NetComm::IsLAN();
        int iPipe = bLAN ? PIPE_LAN : PIPE_INTERNET;
        int iPercent = bLAN ? 100 : bwPercent;
        sendBits[iPipe] = 0.0f;
        bps[iPipe] = iPercent * iUp / 100;
    }
}

extern unsigned __int64 netEnteredLobbyId;

// FUNCTION: SWBF2 0x005b2d50
unsigned __int64* __fastcall NetCommP::GetAddr(int iDest)
{
    if ((unsigned int)iDest < MAX_DESTS)
    {
        return (unsigned __int64*)((char*)&netDests[iDest] + 0x20);
    }
    if (iDest == MAX_DESTS)
    {
        return (unsigned __int64*)&inPktGroups[63].lastPkt;
    }
    if (iDest == MAX_DESTS + 1)
    {
        return (unsigned __int64*)&inQueue.lastPkt;
    }

    return (unsigned __int64*)&joinSlots[(iDest - 69) * 7 + 1];
}

// FUNCTION: SWBF2 0x005b35a0
void NetComm::SetPing(int iDest, int iPing)
{
    netDests[iDest].ping = iPing;
}

// FUNCTION: SWBF2 0x005b35b0
int NetComm::GetPLKills(int iPlayer)
{
    return netDests[iPlayer].PLKills;
}

// FUNCTION: SWBF2 0x005b35c0
void NetComm::SetPLKills(int iDest, int iKills)
{
    netDests[iDest].PLKills = iKills;
}

// FUNCTION: SWBF2 0x005d5de0
unsigned __int64* __fastcall CopyEnteredLobbyId(unsigned __int64* pID)
{
    *pID = netEnteredLobbyId;
    return pID;
}

// FUNCTION: SWBF2 0x00618560
unsigned short __fastcall NetSocket::SetAddr(void* pDest, unsigned int uIP, unsigned short uPort)
{
    NetSocketPeerRecord* pPeer = (NetSocketPeerRecord*)pDest;
    sockaddr_in* pAddr = (sockaddr_in*)pPeer->mSocketAddress;
    pAddr->sin_family = AF_INET;
    pAddr->sin_addr.s_addr = uIP;
    pAddr->sin_port = htons(uPort);
    return pAddr->sin_port;
}

// FUNCTION: SWBF2 0x005b2300
void* GetGalaxyMatchmakingForNetComm()
{
    if (!galaxy::api::GalaxyFactory::GetInstance())
    {
        return 0;
    }
    galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::GetInstance();
    return pGalaxy->Matchmaking();
}

// FUNCTION: SWBF2 0x005b2320
void* GetGalaxyNetworkingForNetComm()
{
    if (!galaxy::api::GalaxyFactory::GetInstance())
    {
        return 0;
    }
    galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::GetInstance();
    return pGalaxy->Networking();
}

// FUNCTION: SWBF2 0x005b3550
int NetComm::GetPing(int iDest)
{
    NetDest& dest = netDests[iDest];
    const unsigned __int64 peer = *(unsigned __int64*)dest.mAddressStorage;
    if (!peer)
    {
        return dest.ping;
    }
    void* pNetworking = GetGalaxyNetworkingForNetComm();
    int iPing = ((galaxy::api::INetworking*)pNetworking)->GetPingWith(galaxy::api::GalaxyID(peer));
    return iPing < 0 ? dest.ping : iPing;
}

// FUNCTION: SWBF2 0x005b35f0
void NetComm::GetBandwidth(int iDest, int& iUp, int& iDown)
{
    iUp = netDests[iDest].upBandwidth;
    iDown = netDests[iDest].downBandwidth;
    if (netIsHost && iDest == 64)
    {
        iUp = bps[NetComm::IsLAN() ? PIPE_LAN : PIPE_INTERNET];
    }
}

void DoSends();
void DoRecvs();
void __fastcall SendLow(NetPacket* pPacket);
bool __fastcall RecvGroup(NetPktGroup* pGroup, NetPacket* pPacket);
void __fastcall HandleCommPacketOut(NetPacket* pPacket);
namespace gf { bool checkError(); }

// FUNCTION: SWBF2 0x005b2740
void NetComm::Update()
{
    DoSends();
    DoRecvs();
}

// FUNCTION: SWBF2 0x005b28b0
void NetComm::Send(int iDest, PktType uType, NetPacket* pPacket)
{
    Header(pPacket).src = (unsigned char)iDest;
    Header(pPacket).type = uType;
    if (iDest < MAX_DESTS)
    {
        netDests[iDest].outQueue.PushBack(pPacket);
    }
    else
    {
        SendLow(pPacket);
    }
    DoSends();
    DoRecvs();
}

// FUNCTION: SWBF2 0x005b2900
void NetComm::Send(int iDest, int uType, NetPktGroup* pGroup)
{
    unsigned short uSequence = 0;
    if (iDest < MAX_DESTS)
    {
        uSequence = (unsigned short)netDests[iDest].nextGroupNum++;
    }
    const int iCount = pGroup->pktCount;
    for (int i = 0; i < iCount; ++i)
    {
        NetPacket* pPacket = pGroup->PopFront();
        Header(pPacket).count = (unsigned char)iCount;
        Header(pPacket).group = uSequence;
        Header(pPacket).idx = (unsigned char)i;
        NetComm::Send(iDest, (PktType)uType, pPacket);
    }
}

// FUNCTION: SWBF2 0x005b2a60
bool NetComm::Recv(NetPktGroup* pGroup, int eType)
{
    for (NetPacket* pPacket = NetComm::Recv((PktType)eType, PKT_NONE);
         pPacket; pPacket = NetComm::Recv((PktType)eType, PKT_NONE))
    {
        if (RecvGroup(pGroup, pPacket))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x005b2aa0
void __fastcall ClearGroup(NetPktGroup* pGroup, int iDest)
{
    for (NetPacket* pPacket = pGroup->GetFront(); pPacket; )
    {
        NetPacket* pNext = NetPacket::Get(pPacket->mNextPkt);
        if (NetComm::Header(pPacket).src == iDest)
        {
            pGroup->Remove(pPacket);
        }
        pPacket = pNext;
    }
}

// FUNCTION: SWBF2 0x005b2af0
void NetComm::Disconnect(int iDest)
{
    ClearGroup(&inQueue, iDest);
    for (int i = 0; i < 64; ++i)
    {
        ClearGroup(&inPktGroups[i], iDest);
    }
    NetDest& dest = netDests[iDest];
    dest.connected = false;
    dest.lastRecv = 0.0f;
    dest.ping = 250;
    dest.PLDeaths = 0;
    dest.PLKills = 0;
    dest.upBandwidth = 0x10000;
    dest.downBandwidth = 0x10000;
    dest.pipe = PIPE_INTERNET;
    dest.nextGroupNum = 0;
    unsigned __int64* pPeer = (unsigned __int64*)dest.mAddressStorage;
    if (iDest == 64 && *pPeer)
    {
        void* pInterface = GetGalaxyMatchmakingForNetComm();
        ((galaxy::api::IMatchmaking*)pInterface)->LeaveLobby(galaxy::api::GalaxyID(*pPeer));
        gf::checkError();
    }
    *pPeer = 0;
    NetSocket::SetAddr(pPeer, 0, 0);
}

// FUNCTION: SWBF2 0x005b2e30
void __fastcall SendLow(NetPacket* pPacket)
{
    const unsigned int iDest = NetComm::Header(pPacket).src;
    HandleCommPacketOut(pPacket);
    NetDrag::Send(pPacket, iDest);
    if (iDest < MAX_DESTS && iDest != 64)
    {
        const int iBytes = pPacket->mMaxIndex - pPacket->mCommIndex;
        sendBits[netDests[iDest].pipe] += (float)(iBytes * 8 + 224);
    }
}

void __fastcall PackHeader(NetPacket* pPacket);

// FUNCTION: SWBF2 0x005b2e90
void __fastcall HandleCommPacketOut(NetPacket* pPacket)
{
    PackHeader(pPacket);
}

// FUNCTION: SWBF2 0x005b2eb0
void __fastcall PackHeader(NetPacket* pPacket)
{
    pPacket->mCommIndex = pPacket->mDataIndex - 5;
    pPacket->mCurIndex = (signed char)pPacket->mCommIndex;
    const unsigned int a = *(unsigned int*)&NetComm::Header(pPacket).group;
    const unsigned int b = *(unsigned int*)&NetComm::Header(pPacket).idx;
    pPacket->Write((unsigned char)(a >> 16));
    pPacket->Write((unsigned short)a);
    pPacket->Write((unsigned char)b);
    pPacket->Write((unsigned char)((b >> 8) | (((b >> 16) & 0xff) ? 0x80 : 0)));
}

// FUNCTION: SWBF2 0x005b3130
void __fastcall CleanGroups(int iDest, int iSequence)
{
    int iRecent = iSequence - 16;
    int iOld = iSequence - 272;
    if (iRecent < 0)
    {
        iRecent += 0x10000;
    }
    if (iOld < 0)
    {
        iOld += 0x10000;
    }
    for (int i = 0; i < 64; ++i)
    {
        NetPktGroup& group = inPktGroups[i];
        if (!group.pktCount)
        {
            continue;
        }
        NetPacket* pFirst = NetPacket::Get(group.firstPkt);
        if (NetComm::Header(pFirst).src != iDest)
        {
            continue;
        }
        const int iGroupSequence = NetComm::Header(pFirst).group;
        const bool bExpired = iOld < iRecent
            ? iGroupSequence >= iOld && iGroupSequence <= iRecent
            : iGroupSequence <= iRecent || iGroupSequence >= iOld;
        if (bExpired)
        {
            group.Free();
        }
    }
}

// FUNCTION: SWBF2 0x005b2f90
bool __fastcall RecvGroup(NetPktGroup* pComplete, NetPacket* pPacket)
{
    const NetComm::PktHeader& header = NetComm::Header(pPacket);
    const unsigned short uSequence = header.group;
    const unsigned char uType = header.type;
    const unsigned char uDest = header.src;
    const unsigned char uFragment = header.idx;
    const unsigned char uCount = header.count;
    CleanGroups(uDest, uSequence);
    if (uCount == 1 && uFragment == 0)
    {
        pPacket->mNextPkt = pPacket->mPrevPkt = -1;
        pComplete->Set(pPacket);
        return true;
    }

    NetPktGroup* pPartial = 0;
    for (int i = 0; i < 64; ++i)
    {
        NetPktGroup& group = inPktGroups[i];
        if (!group.pktCount)
        {
            continue;
        }
        NetPacket* pFirst = NetPacket::Get(group.firstPkt);
        if (NetComm::Header(pFirst).group == uSequence &&
            NetComm::Header(pFirst).type == uType &&
            NetComm::Header(pFirst).src == uDest)
        {
            pPartial = &group;
            break;
        }
    }
    if (!pPartial)
    {
        for (int i = 0; i < 64; ++i)
        {
            if (!inPktGroups[i].pktCount)
            {
                pPartial = &inPktGroups[i];
                break;
            }
        }
        if (!pPartial)
        {
            pPacket->Free();
            return false;
        }
        pPartial->PushBack(pPacket);
        return false;
    }
    else
    {
        NetPacket* pCur = NetPacket::Get(pPartial->firstPkt);
        const int iPackets = pPartial->pktCount;
        for (int i = 0; i < iPackets; ++i)
        {
            NetPacket* pNext = NetPacket::Get(pCur->mNextPkt);
            if (NetComm::Header(pCur).idx == uFragment)
            {
                pPartial->Remove(pCur);
            }
            pCur = pNext;
        }
    }
    pPartial->PushBack(pPacket);
    if (pPartial->pktCount != uCount)
    {
        return false;
    }

    pComplete->Reset();
    for (int i = 0; i < uCount; ++i)
    {
        NetPacket* pNext = pPartial->PopFront();
        while (NetComm::Header(pNext).idx != i)
        {
            pPartial->PushBack(pNext);
            pNext = pPartial->PopFront();
        }
        pComplete->PushBack(pNext);
    }
    return true;
}

// FUNCTION: SWBF2 0x005b2c90
float& NetCommP::GetLastRecv(int iIndex)
{
    return netDests[iIndex].lastRecv;
}

// FUNCTION: SWBF2 0x005b3480
PipeType& NetCommP::GetPipe(int iDest)
{
    return netDests[iDest].pipe;
}

// FUNCTION: SWBF2 0x005b2c70
bool& NetCommP::GetConnected(int iDest)
{
    return netDests[iDest].connected;
}

// FUNCTION: SWBF2 0x005b2c80
int& NetCommP::GetLocalIndex(int iDest)
{
    return netLocalIndex[iDest];
}

extern int netDirectPort;

// FUNCTION: SWBF2 0x005b2730
int NetComm::GetDirectPort()
{
    return netDirectPort;
}

// FUNCTION: SWBF2 0x005b2370
bool IsSame(const void* pLeft, const void* pRight)
{
    if (NetComm::IsLAN())
    {
        int leftIP, leftPort, rightIP, rightPort;
        NetSocket::GetAddr(pLeft, leftIP, leftPort);
        NetSocket::GetAddr(pRight, rightIP, rightPort);
        return leftIP == rightIP && leftPort == rightPort;
    }
    const unsigned int* pA = (const unsigned int*)pLeft;
    const unsigned int* pB = (const unsigned int*)pRight;
    return pA[0] == pB[0] && pA[1] == pB[1];
}

// FUNCTION: SWBF2 0x005b2a10
NetPacket* NetComm::RecvRange(int iFirstType, int iLastType)
{
    for (NetPacket* pPkt = NetPacket::Get(inQueue.firstPkt); pPkt;
         pPkt = NetPacket::Get(pPkt->mNextPkt))
    {
        int iType = (unsigned char)pPkt->mData[2];
        if (iType >= iFirstType && iType <= iLastType)
        {
            inQueue.Remove(pPkt);
            return pPkt;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005b2610
void NetComm::Reset()
{
    NetPacket::Reset();
}

// FUNCTION: SWBF2 0x005b2750
void NetComm::AdvanceTime(bool bClamp)
{
    LARGE_INTEGER liTicks;
    QueryPerformanceCounter(&liTicks);
    unsigned __int64 iDelta = liTicks.QuadPart - lastTicks;
    lastTicks = liTicks.QuadPart;
    netTimeBase.AdvanceTicks(iDelta);

    float fOld = netTime;
    float fElapsed = netTimeStamp.GetElapsed();
    if (fElapsed < 0.01f)
    {
        return;
    }

    netTimeStamp.Mark();
    netTime = fOld + fElapsed;
    if (bClamp && netTime > fOld + 1.0f)
    {
        netTime = fOld + 0.05f;
    }

    float fDt = netTime - fOld;
    sendBits[0] -= (float)bps[0] * fDt;
    if (sendBits[0] < 0.0f)
    {
        sendBits[0] = 0.0f;
    }
    sendBits[1] -= (float)bps[1] * fDt;
    if (sendBits[1] < 0.0f)
    {
        sendBits[1] = 0.0f;
    }

    NetDrag::Update();
}

// FUNCTION: SWBF2 0x005b29c0
NetPacket* NetComm::Recv(PktType eType, PktType eType2)
{
    for (NetPacket* pPkt = NetPacket::Get(inQueue.firstPkt); pPkt; pPkt = NetPacket::Get(pPkt->mNextPkt))
    {
        const int iType = (unsigned char)pPkt->mData[2];
        if (iType == eType || iType == eType2)
        {
            inQueue.Remove(pPkt);
            return pPkt;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005b3930
template void PblJournal::Record<unsigned __int64>(unsigned __int64& value, PblJournalType type);

// FUNCTION: SWBF2 0x005b26d0
bool NetComm::IsLAN()
{
    return isLAN;
}

// FUNCTION: SWBF2 0x005b36b0
int NetComm::SetBandwidthPercent()
{
    bwPercent = TEMP_bandwith;
    sendBits[PIPE_INTERNET] = 0.0f;
    bps[PIPE_INTERNET] = TEMP_bandwith * gInternetBandwidthScale / 100;
    return bps[PIPE_INTERNET];
}

// FUNCTION: SWBF2 0x005b3700
int __fastcall FindSource(const void* pSource, unsigned int uType)
{
    const unsigned int* pRawSource = (const unsigned int*)pSource;
    unsigned int uIP = pRawSource[0];
    unsigned int uPort = pRawSource[1];
    unsigned int iDest = RoutesToHostDestination(uType) ? 64 : 0;

    if (iDest == 64 && !NetComm::IsLAN())
    {
        unsigned __int64 peerID;
        CopyEnteredLobbyId(&peerID);
        void* pInterface = GetGalaxyMatchmakingForNetComm();
        galaxy::api::GalaxyID mappedID =
            ((galaxy::api::IMatchmaking*)pInterface)->GetLobbyOwner(galaxy::api::GalaxyID(peerID));
        if (mappedID.value == *(const unsigned __int64*)pSource)
        {
            uIP = (unsigned int)peerID;
            uPort = (unsigned int)(peerID >> 32);
        }
    }

    for (; iDest < MAX_DESTS; ++iDest)
    {
        const unsigned char* pAddress = (const unsigned char*)&netDests[iDest] + 0x20;
        if (NetComm::IsLAN())
        {
            int sourceIP, sourcePort, destIP, destPort;
            NetSocket::GetAddr(pSource, sourceIP, sourcePort);
            NetSocket::GetAddr(pAddress, destIP, destPort);
            if (sourceIP == destIP && sourcePort == destPort)
            {
                return iDest;
            }
        }
        else
        {
            const unsigned int* pDest = (const unsigned int*)pAddress;
            if (uIP == pDest[0] && uPort == pDest[1])
            {
                return iDest;
            }
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x005b2970
void NetComm::SendVoice(int iDest, NetPacket* pPacket, unsigned short uSequence)
{
    Header(pPacket).src = (unsigned char)iDest;
    Header(pPacket).type = PKT_VOICE;
    Header(pPacket).group = uSequence;
    Header(pPacket).idx = 0;
    Header(pPacket).count = 1;
    if ((unsigned int)iDest < MAX_DESTS)
    {
        netDests[iDest].outQueue.PushBack(pPacket);
    }
    else
    {
        SendLow(pPacket);
    }
    DoSends();
    DoRecvs();
}

// FUNCTION: SWBF2 0x005b2db0
void DoSends()
{
    for (int iDest = 0; iDest < 64 + netCurMaxHosts; ++iDest)
    {
        if (iDest < 64 && iDest >= netCurMaxPlayers)
        {
            iDest = 64;
        }
        if (NetComm::IsConnected(iDest))
        {
            NetPktGroup& queue = netDests[iDest].outQueue;
            while (queue.pktCount > 0)
            {
                SendLow(queue.PopFront());
            }
        }
    }
}

unsigned int __fastcall UnpackHeader(NetPacket* pPacket);

// FUNCTION: SWBF2 0x005b2ea0
unsigned int __fastcall HandleCommPacketIn(NetPacket* pPacket)
{
    return UnpackHeader(pPacket);
}

// FUNCTION: SWBF2 0x005b2f10
unsigned int __fastcall UnpackHeader(NetPacket* pPacket)
{
    pPacket->mCurIndex = (signed char)pPacket->mCommIndex;
    unsigned char uByte;
    unsigned short uWord;
    unsigned int uFirst, uSecond;
    unsigned char* pFirst = (unsigned char*)&uFirst;
    unsigned char* pSecond = (unsigned char*)&uSecond;
    pPacket->Read(uByte);
    pFirst[2] = uByte;
        pPacket->Read(uWord);
    pFirst[0] = (unsigned char)uWord;
    pFirst[1] = (unsigned char)(uWord >> 8);
    pPacket->Read(uByte);
    pSecond[0] = uByte;
    pPacket->Read(uByte);
    pSecond[1] = uByte & 0x7f;
    pSecond[2] = uByte >> 7;
    pPacket->mDataIndex = (char)pPacket->mCurIndex;
    *(unsigned int*)&NetComm::Header(pPacket).group = uFirst;
    *(unsigned int*)&NetComm::Header(pPacket).idx = uSecond;
    return uSecond;
}

// FUNCTION: SWBF2 0x005b31c0
void DoRecvs()
{
    if (NetComm::IsLAN())
    {
        NetSocket::RecvSockets();
    }
    else
    {
        NetSocket::RecvGalaxySockets();
    }

    for (NetPacket* pPacket = NetDrag::ReceiveLocal(); pPacket; pPacket = NetDrag::ReceiveLocal())
    {
        const unsigned char uType = (unsigned char)pPacket->mData[(signed char)pPacket->mCommIndex];
        int iDest = RoutesToHostDestination(uType) ? 64 : NetComm::GetLocalIndex(0);
        HandleCommPacketIn(pPacket);
        NetComm::Header(pPacket).src = (unsigned char)iDest;
        if ((unsigned int)iDest < MAX_DESTS)
        {
            netDests[iDest].lastRecv = NetComm::GetTime();
            if (iDest < 64)
            {
                const int iGuest = NetGame::GetMyGuest(iDest, 0);
                if (iGuest >= 0)
                {
                    netDests[NetGame::GetMyGuest(iDest, 0)].lastRecv = NetComm::GetTime();
                }
            }
        }
        inQueue.PushBack(pPacket);
    }
}

// FUNCTION: SWBF2 0x005b3290
void __fastcall NetCommP::RecvPacket(NetPacket* pPacket, const void* pSource)
{
    const int iRead = (signed char)pPacket->mCommIndex;
    inBytes += pPacket->mMaxIndex - iRead;
    const unsigned char uType = (unsigned char)pPacket->mData[iRead];
    if (uType == 0xfe || uType == 92 || uType == 0xfd || uType == 59)
    {
        NetSetupP::Recv(pPacket, pSource);
        pPacket->Free();
        return;
    }
    if (uType >= PKT_NONE)
    {
        pPacket->Free();
        return;
    }
    if (uType == PKT_FIND || uType == PKT_FOUND)
    {
        HandleCommPacketIn(pPacket);
        NetSetupP::RecvLan(pPacket, pSource);
        return;
    }

    int iDest;
    if (uType == PKT_JOIN)
    {
        ++gDebugCount;
        iDest = FindSource(pSource, uType);
    }
    else if (uType == PKT_CONSOLE)
    {
        if (IsAdminPacketAuthorized((const unsigned char*)pPacket))
        {
            iDest = NetCommP::FindJoinSource((const int*)pSource);
        }
        else
        {
            iDest = -1;
        }
    }
    else
    {
        iDest = FindSource(pSource, uType);
    }

    if (iDest == -1)
    {
        if (uType == PKT_JOIN || (NetComm::IsLAN() && (uType == PKT_QUERY || uType == PKT_QUERY_RESPONSE)))
        {
            iDest = NetCommP::FindJoinSource((const int*)pSource);
        }
        if (iDest == -1)
        {
            pPacket->Free();
            return;
        }
    }

    if (iDest < MAX_DESTS)
    {
    pPacket = pPacket->Decrypt(NetGame::GetKey(iDest));
    }
    if (!pPacket)
    {
        return;
    }

    HandleCommPacketIn(pPacket);
    NetComm::Header(pPacket).src = (unsigned char)iDest;
    if ((unsigned int)iDest < MAX_DESTS)
    {
        netDests[iDest].lastRecv = NetComm::GetTime();
        if (iDest < 64)
        {
            const int iGuest = NetGame::GetMyGuest(iDest, 0);
            if (iGuest >= 0)
            {
                netDests[NetGame::GetMyGuest(iDest, 0)].lastRecv = NetComm::GetTime();
            }
        }
    }
    inQueue.PushBack(pPacket);
}

// GLOBAL: SWBF2 0x01fa2511
bool isLAN;

// FUNCTION: SWBF2 0x005b3420
bool __fastcall RoutesToHostDestination(unsigned int uType)
{
    switch (uType)
    {
    case 0: case 2: case 4: case 7: case 11:
    case 16: case 17: case 18: case 19:
    case 22: case 24: case 25: case 26: case 28:
        return false;
    default:
        return true;
    }
}

// GLOBAL: SWBF2 0x007e8024
int bps[2] = { 0x00a00000, 0x0004b000 };

// GLOBAL: SWBF2 0x01e62e90
float sendBits[2];

// GLOBAL: SWBF2 0x007e8e10
char netGameName[64] = "SWBF2";
