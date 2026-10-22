#include "NetComm.h"
#include "NetGame.h"
#include "NetPacket.h"
#include "pcNetSocket.h"
#include <winsock2.h>
#include "PebbleFL/Common/PblJournal.h"
#include "PebbleFL/Common/PblRandom.h"

extern int dragLatency;
extern int dragJitter;
extern int gDebugCount;
void __stdcall InsertLatencyPacketBefore(NetPacket* pBefore, NetPacket* pNew);
NetPacket* PopLocal();
void __fastcall QueueLossPacket(NetPacket* pPacket);

// GLOBAL: SWBF2 0x01faa6a0
int gLatencyDebugPacketCount;

// GLOBAL: SWBF2 0x01faa6a4
int gLatencyDebugPacketLimit;

void AddToLatency(NetPacket* pPkt, float fLate);
void AddToDuplication(NetPacket* pPkt);

extern bool dragEnabled;
extern bool dragEnabledHost;
extern int dragBandwidth;

// GLOBAL: SWBF2 0x01faa72c
int netSocketErrorCount;
extern float thisDrag;

// GLOBAL: SWBF2 0x01fa9258
unsigned int netDirectSocket;

// GLOBAL: SWBF2 0x01fa925c
unsigned int netBroadcastSendSocket;

// GLOBAL: SWBF2 0x01fa9260
unsigned int netBroadcastRecvSocket;
extern int netOutBytes;

void AddToBandwidth(NetPacket* pPacket, int iDest);
void Deliver(NetPacket* pPacket, int iDest);

class NetDrag
{
public:
    static void Create(bool bMission);
    static void Reset();
    static void BeginShell();
    static void Update();
    static void EndShell();
    static void Send(NetPacket* pPacket, int iDest);
    static void FreePackets();
    static NetPacket* ReceiveLocal();
};

void CreateAll();

// GLOBAL: SWBF2 0x01ecda30
NetPktGroup bandwidthQueue[3];

// GLOBAL: SWBF2 0x01ecd9f8
NetPktGroup lossQueue;
// GLOBAL: SWBF2 0x01ecda14
NetPktGroup latencyQueue;
// GLOBAL: SWBF2 0x01ecda84
NetPktGroup localQueue;

// GLOBAL: SWBF2 0x01fa3d68
float lastDrag;
// GLOBAL: SWBF2 0x01fa3d70
float bandwidthAvailTime[3];

// FUNCTION: SWBF2 0x005b3ac0
void NetDrag::Create(bool  )
{
    CreateAll();
}

// FUNCTION: SWBF2 0x005b3af0
void NetDrag::EndShell()
{
}

// FUNCTION: SWBF2 0x005b3ae0
void NetDrag::BeginShell()
{
}

// FUNCTION: SWBF2 0x005b3b70
void CreateAll()
{
    lastDrag = NetComm::GetTime();
    bandwidthAvailTime[2] = 0.0f;
    bandwidthAvailTime[0] = bandwidthAvailTime[1] = 0.0f;

    for (int i = 0; i < 3; i++)
    {
        bandwidthQueue[i].Reset();
    }

    latencyQueue.Reset();
    lossQueue.Reset();
    localQueue.Reset();
}

// FUNCTION: SWBF2 0x005b3940
bool __fastcall DoDrag(int iDest)
{
    if (!dragEnabled)
    {
        return false;
    }
    return dragEnabledHost || !netIsHost ||
           (iDest != 64 && iDest != NetComm::GetLocalIndex(0));
}

// FUNCTION: SWBF2 0x005b3980
int __fastcall GetPipe(int iDest)
{
    if (netIsHost)
    {
        if (iDest == 64)
        {
            return 1;
        }
        if (iDest == NetComm::GetLocalIndex(0))
        {
            return 0;
        }
    }
    return 2;
}

// FUNCTION: SWBF2 0x005b39b0
float __fastcall GetBandwidth(int iDest)
{
    if (netIsHost && (iDest == 64 || iDest == NetComm::GetLocalIndex(0)))
    {
        return 1572864.0f;
    }
    return (float)dragBandwidth * 1024.0f;
}

// FUNCTION: SWBF2 0x005b3b20
void NetDrag::Send(NetPacket* pPacket, int iDest)
{
    if (DoDrag(iDest))
    {
        AddToBandwidth(pPacket, iDest);
    }
    else
    {
        Deliver(pPacket, iDest);
    }
}

// FUNCTION: SWBF2 0x005b3c10
void AddToBandwidth(NetPacket* pPacket, int iDest)
{
    int iQueue = GetPipe(iDest);
    float* pNextTime = &bandwidthAvailTime[iQueue];
    float fTime = *pNextTime;
    if (thisDrag > fTime)
    {
        fTime = thisDrag;
    }
    const int iBytes = (int)pPacket->mMaxIndex - (int)pPacket->mCommIndex;
    fTime += (float)(iBytes * 8) / GetBandwidth(iDest);
    NetComm::Header(pPacket).idx = (unsigned char)iDest;
    *(float*)pPacket->mData = fTime;
    bandwidthQueue[iQueue].PushBack(pPacket);
    *pNextTime = fTime;
}

// FUNCTION: SWBF2 0x005b3e50
void Deliver(NetPacket* pPacket, int iDest)
{
    if (iDest >= 64 && iDest - 64 == NetGameP::GetHostIndex())
    {
        localQueue.PushBack(pPacket);
        return;
    }
    for (int i = 0; i < 2; ++i)
    {
        if (iDest == NetComm::GetLocalIndex(i))
        {
            localQueue.PushBack(pPacket);
            return;
        }
    }

    const int iLen = pPacket->mMaxIndex - pPacket->mCommIndex;
    char* pData = pPacket->mData + pPacket->mCommIndex;
    if ((unsigned char)*pData == PKT_VOICE)
    {
        NetSocket::SendVoiceSocket(iDest, pData, iLen, 2);
    }
    else
    {
        NetGame::GetKey(iDest);
        pPacket = pPacket->Encrypt(iDest);
        pData = pPacket->mData + pPacket->mCommIndex;
        NetSocket::SendSocket(iDest, pData, iLen);
    }
    netOutBytes += iLen;
    pPacket->Free();
}

// FUNCTION: SWBF2 0x00533b10
template void PblJournal::Record<bool>(bool& value, PblJournalType type);

// GLOBAL: SWBF2 0x01fa3d6c
float thisDrag;
// GLOBAL: SWBF2 0x01fa3d64
float dragDt;

// GLOBAL: SWBF2 0x01e62e9c
int dragLoss;

static float GetTime(NetPacket* pPkt)
{
    return *(float*)pPkt->mData;
}

static int GetDst(NetPacket* pPkt)
{
    return (char)NetComm::Header(pPkt).idx;
}

static void UpdateTime();
static void UpdateBandwidth();
static void UpdateLatency();
static void UpdateLoss();

// FUNCTION: SWBF2 0x005b3b00
void NetDrag::Update()
{
    UpdateTime();
    UpdateBandwidth();
    UpdateLatency();
    UpdateLoss();
}

// FUNCTION: SWBF2 0x005b3bd0
static void UpdateTime()
{
    thisDrag = NetComm::GetTime();
    dragDt = thisDrag - lastDrag;
    lastDrag = thisDrag;
    if (dragDt > 0.5f)
    {
        dragDt = 0.5f;
    }
}

// FUNCTION: SWBF2 0x005b3c90
static void UpdateBandwidth()
{
    for (int i = 0; i < 3; i++)
    {
        NetPacket* pPkt;
        while ((pPkt = bandwidthQueue[i].GetFront()) != 0)
        {
            if (GetTime(pPkt) > thisDrag)
            {
                break;
            }
            AddToLatency(bandwidthQueue[i].PopFront(), thisDrag - GetTime(pPkt));
        }
    }
}

// FUNCTION: SWBF2 0x005b3d50
static void UpdateLatency()
{
    NetPacket* pPkt;
    while ((pPkt = latencyQueue.GetFront()) != 0)
    {
        if (GetTime(pPkt) > thisDrag)
        {
            break;
        }
        AddToDuplication(latencyQueue.PopFront());
    }
}

// FUNCTION: SWBF2 0x005b3db0
static void UpdateLoss()
{
    NetPacket* pPkt;
    while ((pPkt = lossQueue.PopFront()) != 0)
    {
        if ((float)dragLoss > PblRandom::_Global.GetFloat() * 100.0f)
        {
            pPkt->Free();
        }
        else
        {
            Deliver(pPkt, GetDst(pPkt));
        }
    }
}

// FUNCTION: SWBF2 0x005b3f10
static void FreeAllPackets()
{
    for (int i = 0; i < 3; i++)
    {
        bandwidthQueue[i].Free();
    }
    latencyQueue.Free();
    lossQueue.Free();
    localQueue.Free();
}

// FUNCTION: SWBF2 0x005b3ad0
void NetDrag::Reset()
{
    FreeAllPackets();
    thisDrag = 0.0f;
    lastDrag = 0.0f;
    dragDt = 0.0f;
    bandwidthAvailTime[0] = bandwidthAvailTime[1] = 0.0f;
    bandwidthAvailTime[2] = 0.0f;
}

// FUNCTION: SWBF2 0x005b39f0
float GetLatency(int iDest)
{
    if (netIsHost && (iDest == 64 || iDest == NetComm::GetLocalIndex(0)))
    {
        return 2.5f * 0.001f;
    }
    return (float)dragLatency * 0.001f;
}

// FUNCTION: SWBF2 0x005b3a30
float GetJitter()
{
    const float fWidth = (float)dragJitter * (float)dragLatency / 100.0f;
    return ((PblRandom::_Global.GetFloat() * fWidth * 2.0f) - fWidth) * 0.001f;
}

// FUNCTION: SWBF2 0x005b3b50
NetPacket* NetDrag::ReceiveLocal()
{
    return PopLocal();
}

// FUNCTION: SWBF2 0x005b3cf0
void AddToLatency(NetPacket* pPacket, float fElapsed)
{
    const int iDest = (signed char)NetComm::Header(pPacket).idx;
    const float fLatency = GetLatency(iDest);
    const float fJitter = GetJitter();
    *(float*)pPacket->mData = ((thisDrag - fElapsed) + fLatency) + fJitter;
    if (++gLatencyDebugPacketCount == gLatencyDebugPacketLimit)
    {
        ++gDebugCount;
    }

    NetPacket* pBefore = latencyQueue.GetFront();
    while (pBefore && *(float*)pBefore->mData <= *(float*)pPacket->mData)
    {
        pBefore = NetPacket::Get(pBefore->mNextPkt);
    }
    if (pBefore)
    {
        InsertLatencyPacketBefore(pBefore, pPacket);
    }
    else
    {
        latencyQueue.PushBack(pPacket);
    }
}

// FUNCTION: SWBF2 0x005b3d90
void AddToDuplication(NetPacket* pPacket)
{
    QueueLossPacket(pPacket);
}

// FUNCTION: SWBF2 0x005b3f00
NetPacket* PopLocal()
{
    return localQueue.PopFront();
}

// FUNCTION: SWBF2 0x005b3da0
void __fastcall QueueLossPacket(NetPacket* pPacket)
{
    lossQueue.PushBack(pPacket);
}

// GLOBAL: SWBF2 0x01e57a0f
bool dragEnabled = false;
// GLOBAL: SWBF2 0x01e62ea0
int dragLatency = 0;
// GLOBAL: SWBF2 0x01e62ea4
int dragJitter = 0;
// GLOBAL: SWBF2 0x007e800c
int dragBandwidth = 8000;
