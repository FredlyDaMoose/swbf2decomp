#include <string.h>

#include "NetPacket.h"
#include "NetComm.h"
#include "NetGame.h"

class NetDrag
{
public:
    static void Send(NetPacket* pPacket, int iDest);
    static void FreePackets();
};

extern NetPktGroup inQueue;
extern NetPktGroup inPktGroups[64];
extern NetDest netDests[MAX_DESTS];
extern NetPktGroup bandwidthQueue[3];
extern NetPktGroup lossQueue;
extern NetPktGroup latencyQueue;
extern NetPktGroup localQueue;

// GLOBAL: SWBF2 0x01e62e98
int freeGroupCycle;

// FUNCTION: SWBF2 0x005b2be0
void NetCommP::FreePackets()
{
    if (inQueue.pktCount)
    {
        unsigned int iCycle = (unsigned int)freeGroupCycle;
        inPktGroups[iCycle].Free();
        freeGroupCycle = iCycle < 64 ? iCycle : 0;
        for (int i = 0; i < 64; ++i)
        {
            NetPacket* pPacket = inQueue.PopFront();
            if (!pPacket)
            {
                break;
            }
            pPacket->Free();
        }
    }
    else
    {
        inQueue.Free();
        for (int i = 0; i < MAX_DESTS; ++i)
        {
            netDests[i].outQueue.Free();
        }
        for (int i = 0; i < 64; ++i)
        {
            inPktGroups[i].Free();
        }
    }
}

// FUNCTION: SWBF2 0x005b3b60
void NetDrag::FreePackets()
{
    for (int i = 0; i < 3; ++i)
    {
        bandwidthQueue[i].Free();
    }
    latencyQueue.Free();
    lossQueue.Free();
    localQueue.Free();
}

// FUNCTION: SWBF2 0x0053e0a0
void NetPktGroup::Write(bool bValue)
{
    WriteBits(bValue != 0, 1);
}

// FUNCTION: SWBF2 0x0053e0c0
void NetPktGroup::Read(bool& bValue)
{
    unsigned int uValue = 0;
    ReadBits(uValue, 1);
    bValue = uValue != 0;
}

// FUNCTION: SWBF2 0x0053e0f0
void NetPktGroup::Write(const float& fValue)
{
    WriteBits(*(const unsigned int*)&fValue, 32);
}

// FUNCTION: SWBF2 0x0053e110
void NetPktGroup::Read(float& fValue)
{
    ReadBits(*(unsigned int*)&fValue, 32);
}

// FUNCTION: SWBF2 0x0053e130
void NetPktGroup::Write(int iValue)
{
    WriteBits((unsigned int)iValue, 32);
}

// FUNCTION: SWBF2 0x0053e150
void NetPktGroup::Read(int& iValue)
{
    ReadBits(*(unsigned int*)&iValue, 32);
}

// GLOBAL: SWBF2 0x01e65160
NetBigPacket netBigPackets[MAX_BIG_PACKETS];
// GLOBAL: SWBF2 0x01ea5160
NetSmallPacket netSmallPackets[MAX_SMALL_PACKETS];

// GLOBAL: SWBF2 0x01ee7d58
NetPktGroup freeBigPackets;
// GLOBAL: SWBF2 0x01ee7d74
NetPktGroup freeSmallPackets;

// GLOBAL: SWBF2 0x01e65054
unsigned int NetPacket::sNextSeqNo;

// FUNCTION: SWBF2 0x005d8df0
NetPacket* NetPacket::Alloc(bool bSmall)
{
    NetPktGroup& freePackets = bSmall ? freeSmallPackets : freeBigPackets;
    NetPacket* pPacket = freePackets.PopFront();
    if (!pPacket)
    {
        NetDrag::FreePackets();
        NetGameP::FreePackets();
        NetCommP::FreePackets();
        pPacket = freePackets.PopFront();
        if (!pPacket)
        {
            for (int i = 0; i < MAX_BIG_PACKETS; ++i)
            {
                freeBigPackets.PushBack(&netBigPackets[i]);
            }
            for (int i = 0; i < MAX_SMALL_PACKETS; ++i)
            {
                freeSmallPackets.PushBack(&netSmallPackets[i]);
            }
            pPacket = freePackets.PopFront();
        }
    }
    pPacket->mSeqNo = (unsigned short)sNextSeqNo++;
    pPacket->mCurIndex = 2;
    return pPacket;
}

// FUNCTION: SWBF2 0x005d8f60
void NetPacket::BeginWrite()
{
    mData[0] = mData[1] = mData[2] = mData[3] = 0;
    *(int*)((char*)this + 16) = 0;
    mData[0] = mData[1] = (char)0xff;
    mDataIndex = 14;
    mMaxIndex = (short)(GetNum() >= MAX_BIG_PACKETS ? 116 : 1012);
    mCurIndex = 14;
}

// FUNCTION: SWBF2 0x005d8d80
void NetPacket::Create()
{
    freeBigPackets.Reset();
    freeSmallPackets.Reset();

    for (int i = 0; i < MAX_BIG_PACKETS; i++)
    {
        freeBigPackets.PushBack(NetPacket::Get(i));
    }
    for (int i = MAX_BIG_PACKETS; i < MAX_BIG_PACKETS + MAX_SMALL_PACKETS; i++)
    {
        freeSmallPackets.PushBack(NetPacket::Get(i));
    }
}

// FUNCTION: SWBF2 0x005d8ed0
int NetPacket::GetNum() const
{
    const int iSmall = (int)((NetSmallPacket const*)this - netSmallPackets);
    if ((unsigned int)iSmall < (unsigned int)MAX_SMALL_PACKETS)
    {
        return iSmall + MAX_BIG_PACKETS;
    }
    return (int)((NetBigPacket const*)this - netBigPackets);
}

// FUNCTION: SWBF2 0x005d8f00
NetPacket* NetPacket::Get(int iNum)
{
    if (iNum >= 0)
    {
        if (iNum < MAX_BIG_PACKETS)
        {
            return &netBigPackets[iNum];
        }
        if (iNum < MAX_BIG_PACKETS + MAX_SMALL_PACKETS)
        {
            return &netSmallPackets[iNum - MAX_BIG_PACKETS];
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005d91e0
void NetPktGroup::Reset()
{
    pktCount = 0;
    lastPkt = -1;
    firstPkt = -1;
}

// FUNCTION: SWBF2 0x005d93c0
void NetPktGroup::PushBack(NetPacket* pPkt)
{
    const int iNum = pPkt->GetNum();
    pPkt->mPrevPkt = (short)lastPkt;
    if (pktCount == 0)
    {
        firstPkt = iNum;
    }
    else
    {
        NetPacket::Get(lastPkt)->mNextPkt = (short)iNum;
    }
    lastPkt = iNum;
    pPkt->mNextPkt = -1;
    pktCount++;
}

// GLOBAL: SWBF2 0x01e65058
bool doHiRes;

// GLOBAL: SWBF2 0x01e6505c
unsigned int NetPktGroup::sDebugBitCounter;

// FUNCTION: SWBF2 0x005d8d70
void NetPacket::SetHiRes(bool bHiRes)
{
    doHiRes = bHiRes;
}

// FUNCTION: SWBF2 0x005d8fc0
void NetPacket::EndWrite()
{
    mMaxIndex = mCurIndex;
}

// FUNCTION: SWBF2 0x005d8fd0
void NetPacket::BeginRead()
{
    mCurIndex = mDataIndex;
}

// FUNCTION: SWBF2 0x005d90c0
bool NetPacket::Write(unsigned int const& uValue)
{
    if (mCurIndex + 4 > mMaxIndex)
    {
        return false;
    }
    char* pDst = &mData[mCurIndex];
    const char* pSrc = (const char*)&uValue;
    pDst[0] = pSrc[0];
    pDst[1] = pSrc[1];
    pDst[2] = pSrc[2];
    pDst[3] = pSrc[3];
    mCurIndex += 4;
    return true;
}

// FUNCTION: SWBF2 0x005d9110
bool NetPacket::Read(unsigned int& uValue)
{
    if (mCurIndex + 4 > mMaxIndex)
    {
        return false;
    }
    const char* pSrc = &mData[mCurIndex];
    char* pDst = (char*)&uValue;
    pDst[0] = pSrc[0];
    pDst[1] = pSrc[1];
    pDst[2] = pSrc[2];
    pDst[3] = pSrc[3];
    mCurIndex += 4;
    return true;
}

// FUNCTION: SWBF2 0x005d9160
int NetPacket::Write(char const* pData, int iLen)
{
    int iEnd = mCurIndex + iLen;
    if (iEnd > mMaxIndex)
    {
        iEnd = mMaxIndex;
    }
    const int iCount = iEnd - mCurIndex;
    if (iCount > 0)
    {
        memcpy(&mData[mCurIndex], pData, iCount);
    }
    mCurIndex += (short)iCount;
    return iCount;
}

// FUNCTION: SWBF2 0x005d91a0
int NetPacket::Read(char* pData, int iLen)
{
    int iEnd = mCurIndex + iLen;
    if (iEnd > mMaxIndex)
    {
        iEnd = mMaxIndex;
    }
    const int iCount = iEnd - mCurIndex;
    if (iCount > 0)
    {
        memcpy(pData, &mData[mCurIndex], iCount);
    }
    mCurIndex += (short)iCount;
    return iCount;
}

// FUNCTION: SWBF2 0x005d95d0
void NetPktGroup::WritePacked()
{
    if (!pktPtr)
    {
        pktPtr = NetPacket::Alloc(isSmall);
        PushBack(pktPtr);
        pktPtr->BeginWrite();
    }
    if (!pktPtr->Write(pktBits))
    {
        pktPtr->EndWrite();
        pktPtr = NetPacket::Alloc(isSmall);
        PushBack(pktPtr);
        pktPtr->BeginWrite();
        pktPtr->Write(pktBits);
    }
}

// FUNCTION: SWBF2 0x005d9640
void NetPktGroup::ReadPacked()
{
    if (pktPtr && pktPtr->Read(pktBits))
    {
        return;
    }
    pktPtr = pktPtr ? NetPacket::Get(pktPtr->mNextPkt) : 0;
    if (pktPtr)
    {
        pktPtr->BeginRead();
        pktPtr->Read(pktBits);
    }
    else
    {
        pktBits = 0;
    }
}

// FUNCTION: SWBF2 0x005d9690
void NetPktGroup::WriteBits(unsigned int uValue, int iBits)
{
    sDebugBitCounter += iBits;
    for (;;)
    {
        int iCount = 32 - curBit;
        if (iCount > iBits)
        {
            iCount = iBits;
        }
        const unsigned int uPart = (iCount == 32) ? uValue : uValue & ((1 << iCount) - 1);
        pktBits |= uPart << curBit;
        curBit += iCount;
        if (curBit == 32)
        {
            WritePacked();
            curBit = 0;
            pktBits = 0;
        }
        iBits -= iCount;
        if (iBits == 0)
        {
            break;
        }
        uValue >>= iCount;
    }
}

// FUNCTION: SWBF2 0x005d9710
void NetPktGroup::ReadBits(unsigned int& uValue, int iBits)
{
    uValue = 0;
    int iShift = 0;
    for (;;)
    {
        if (curBit == 32)
        {
            ReadPacked();
            curBit = 0;
        }
        int iCount = 32 - curBit;
        if (iCount > iBits)
        {
            iCount = iBits;
        }
        const unsigned int uMask = (iCount == 32) ? 0xffffffff : (1 << iCount) - 1;
        const unsigned int uPart = pktBits >> curBit;
        curBit += iCount;
        uValue |= (uPart & uMask) << iShift;
        iBits -= iCount;
        if (iBits == 0)
        {
            break;
        }
        iShift += iCount;
    }
}

// FUNCTION: SWBF2 0x005d9790
void NetPktGroup::WriteFixed(float f, float fMin, float fMax, int iBits)
{
    if (doHiRes || iBits >= 32)
    {
        WriteBits(*(unsigned int*)&f, 32);
        return;
    }
    if (f <= fMin)
    {
        WriteBits(0, iBits);
    }
    else if (f >= fMax)
    {
        WriteBits(0xffffffff, iBits);
    }
    else
    {
        WriteBits((unsigned int)((float)(unsigned int)((1 << iBits) - 2) * ((f - fMin) / (fMax - fMin)) + 0.5f), iBits);
    }
}

// FUNCTION: SWBF2 0x005d9860
void NetPktGroup::ReadFixed(float& f, float fMin, float fMax, int iBits)
{
    if (doHiRes || iBits >= 32)
    {
        ReadBits(*(unsigned int*)&f, 32);
        return;
    }
    unsigned int uValue;
    ReadBits(uValue, iBits);
    const unsigned int uMax = (1 << iBits) - 1;
    if (uValue == 0)
    {
        f = fMin;
    }
    else if (uValue == uMax)
    {
        f = fMax;
    }
    else
    {
        f = (float)uValue / (float)(uMax - 1) * (fMax - fMin) + fMin;
    }
}

// FUNCTION: SWBF2 0x005d9920
void NetPktGroupClampFixed(float& f, float fMin, float fMax, int iBits)
{
    if (iBits >= 32)
    {
        return;
    }
    unsigned int uValue;
    if (f <= fMin)
    {
        uValue = 0;
    }
    else if (f >= fMax)
    {
        uValue = 0xffffffff;
    }
    else
    {
        uValue = (unsigned int)((f - fMin) / (fMax - fMin) * (float)(unsigned int)((1 << iBits) - 2) + 0.5f);
    }
    const unsigned int uMax = (1 << iBits) - 1;
    uValue &= uMax;
    if (uValue == 0)
    {
        f = fMin;
    }
    else if (uValue == uMax)
    {
        f = fMax;
    }
    else
    {
        f = (float)uValue / (float)(uMax - 1) * (fMax - fMin) + fMin;
    }
}

// FUNCTION: SWBF2 0x005d9a30
void NetPktGroup::WriteBytes(char const* pData, int iLen)
{
    if (!pktPtr)
    {
        pktPtr = NetPacket::Alloc(isSmall);
        PushBack(pktPtr);
        pktPtr->BeginWrite();
    }
    if (curBit > 0)
    {
        WriteBits(0, 32 - curBit);
    }
    int iCount = pktPtr->Write(pData, iLen);
    sDebugBitCounter += iCount * 8;
    while (iCount != iLen)
    {
        pData += iCount;
        iLen -= iCount;
        pktPtr->EndWrite();
        pktPtr = NetPacket::Alloc(isSmall);
        PushBack(pktPtr);
        pktPtr->BeginWrite();
        iCount = pktPtr->Write(pData, iLen);
        sDebugBitCounter += iCount * 8;
    }
}

// FUNCTION: SWBF2 0x005d9ae0
void NetPktGroup::ReadBytes(char* pData, int iLen)
{
    if (curBit > 0)
    {
        curBit = 32;
        pktBits = 0;
    }
    while (pktPtr)
    {
        const int iCount = pktPtr->Read(pData, iLen);
        if (iCount == iLen)
        {
            return;
        }
        pData += iCount;
        iLen -= iCount;
        pktPtr = NetPacket::Get(pktPtr->mNextPkt);
        if (!pktPtr)
        {
            return;
        }
        pktPtr->BeginRead();
    }
}

// FUNCTION: SWBF2 0x005d8e90
void NetPacket::Free()
{
    mCurIndex = 0;
    if (GetNum() >= MAX_BIG_PACKETS)
    {
        freeSmallPackets.PushFront(this);
    }
    else
    {
        freeBigPackets.PushFront(this);
    }
}

// FUNCTION: SWBF2 0x005d8f40
NetPacket* NetPacket::Encrypt(int) { return this; }

// FUNCTION: SWBF2 0x005d8fe0
bool NetPacket::Write(unsigned char value)
{
    if (mCurIndex + 1 > mMaxIndex)
    {
        return false;
    }
    mData[mCurIndex++] = value;
    return true;
}

// FUNCTION: SWBF2 0x005d9010
bool NetPacket::Read(unsigned char& value)
{
    if (mCurIndex + 1 > mMaxIndex)
    {
        return false;
    }
    value = mData[mCurIndex++];
    return true;
}

// FUNCTION: SWBF2 0x005d9040
bool NetPacket::Write(unsigned short value)
{
    if (mCurIndex + 2 > mMaxIndex)
    {
        return false;
    }
    mData[mCurIndex] = (char)value;
    mData[mCurIndex + 1] = (char)(value >> 8);
    mCurIndex += 2;
    return true;
}

// FUNCTION: SWBF2 0x005d9200
void NetPktGroup::Set(NetPacket* pPkt)
{
    firstPkt = pPkt->GetNum();
    pktCount = 1;
    lastPkt = firstPkt;
    while (pPkt->mNextPkt != -1)
    {
        lastPkt = pPkt->mNextPkt;
        pPkt = NetPacket::Get(lastPkt);
        ++pktCount;
    }
}

// FUNCTION: SWBF2 0x005d92a0
bool NetPktGroup::IsEnd() const
{
    if (!pktPtr)
    {
        return true;
    }
    return pktPtr->GetNum() == lastPkt && pktPtr->mCurIndex >= pktPtr->mMaxIndex;
}

// FUNCTION: SWBF2 0x005d92d0
void NetPktGroup::BeginWrite()
{
    pktCount = 0;
    lastPkt = -1;
    firstPkt = -1;
    pktPtr = 0;
    curBit = 0;
    pktBits = 0;
}

// FUNCTION: SWBF2 0x005d9300
void NetPktGroup::EndWrite()
{
    WritePacked();
    if (pktPtr)
    {
        pktPtr->EndWrite();
    }
}

// FUNCTION: SWBF2 0x005d9320
void NetPktGroup::BeginRead()
{
    pktPtr = NetPacket::Get(firstPkt);
    pktPtr->BeginRead();
    pktBits = 0;
    curBit = 32;
}

// FUNCTION: SWBF2 0x005d9350
void NetPktGroup::EndRead() {}

// FUNCTION: SWBF2 0x005d9360
void NetPktGroup::PushFront(NetPacket* pPkt)
{
    int num = pPkt->GetNum();
    pPkt->mNextPkt = (short)firstPkt;
    if (pktCount)
    {
        NetPacket::Get(firstPkt)->mPrevPkt = (short)num;
    }
    else
    {
        lastPkt = num;
    }
    firstPkt = num;
    pPkt->mPrevPkt = -1;
    ++pktCount;
}

// FUNCTION: SWBF2 0x005d9480
NetPacket* NetPktGroup::PopBack()
{
    if (!pktCount)
    {
        return 0;
    }
    NetPacket* pPkt = NetPacket::Get(lastPkt);
    lastPkt = pPkt->mPrevPkt;
    --pktCount;
    if (!pktCount)
    {
        firstPkt = -1;
    }
    else
    {
        NetPacket::Get(lastPkt)->mNextPkt = -1;
    }
    pPkt->mNextPkt = -1;
    pPkt->mPrevPkt = -1;
    return pPkt;
}

// FUNCTION: SWBF2 0x005d9500
void NetPktGroup::Remove(NetPacket* pPkt)
{
    if (pPkt->mPrevPkt == -1)
    {
        PopFront();
        return;
    }
    if (pPkt->mNextPkt == -1)
    {
        PopBack();
        return;
    }
    NetPacket::Get(pPkt->mNextPkt)->mPrevPkt = pPkt->mPrevPkt;
    NetPacket::Get(pPkt->mPrevPkt)->mNextPkt = pPkt->mNextPkt;
    --pktCount;
    pPkt->mNextPkt = -1;
    pPkt->mPrevPkt = -1;
}

// FUNCTION: SWBF2 0x005d9b40
void NetPktGroup::Tell(NetPktGroupOffset& offset)
{
    offset.pktNum = 0;
    offset.bitNum = curBit;
    if (!pktPtr)
    {
        offset.byteNum = 0;
        return;
    }
    for (NetPacket* pPkt = NetPacket::Get(pktPtr->mPrevPkt); pPkt;
         pPkt = NetPacket::Get(pPkt->mPrevPkt))
    {
        ++offset.pktNum;
    }
    offset.byteNum = pktPtr->mCurIndex - pktPtr->mDataIndex;
}

// FUNCTION: SWBF2 0x005d9280
int NetPktGroup::GetBytes()
{
    return pktPtr ? pktPtr->mCurIndex + 998 * pktCount - 1012 : 0;
}

// FUNCTION: SWBF2 0x005d8de0
void NetPacket::Reset()
{
    doHiRes = false;
}

// FUNCTION: SWBF2 0x005d9250
void NetPktGroup::Free()
{
    NetPacket* pPkt;
    while ((pPkt = PopFront()) != 0)
    {
        pPkt->Free();
    }
}

// FUNCTION: SWBF2 0x005d9420
NetPacket* NetPktGroup::PopFront()
{
    if (pktCount == 0)
    {
        return 0;
    }

    NetPacket* pPkt = NetPacket::Get(firstPkt);
    firstPkt = pPkt->mNextPkt;
    if (--pktCount == 0)
    {
        lastPkt = -1;
    }
    else
    {
        NetPacket::Get(firstPkt)->mPrevPkt = -1;
    }
    pPkt->mPrevPkt = -1;
    pPkt->mNextPkt = -1;
    return pPkt;
}

// FUNCTION: SWBF2 0x005d94e0
NetPacket* NetPktGroup::GetFront()
{
    if (pktCount == 0)
    {
        return 0;
    }
    return NetPacket::Get(firstPkt);
}

// FUNCTION: SWBF2 0x005d9ba0
void NetPktGroup::Seek(const NetPktGroupOffset& offset)
{
    pktPtr = NetPacket::Get(firstPkt);
    for (int i = 0; i < offset.pktNum; ++i)
    {
        pktPtr = NetPacket::Get(pktPtr->mNextPkt);
    }
    pktPtr->BeginRead();
    pktPtr->mCurIndex += (short)offset.byteNum;
    ReadPacked();
    curBit = offset.bitNum;
}

// FUNCTION: SWBF2 0x005d8f50
NetPacket* NetPacket::Decrypt(unsigned int)
{
    return this;
}

// FUNCTION: SWBF2 0x005d9080
bool NetPacket::Read(unsigned short& value)
{
    int iByte = mCurIndex;
    if (iByte + 2 > mMaxIndex)
    {
        return false;
    }
    unsigned char* pDest = (unsigned char*)&value;
    pDest[0] = (unsigned char)mData[iByte];
    pDest[1] = (unsigned char)mData[iByte + 1];
    mCurIndex += 2;
    return true;
}

// FUNCTION: SWBF2 0x005d9570
void __stdcall InsertLatencyPacketBefore(NetPacket* pBefore, NetPacket* pNew)
{
    pNew->mPrevPkt = pBefore->mPrevPkt;
    int iBefore = pBefore->GetNum();
    pNew->mNextPkt = (short)iBefore;
    int iNew = pNew->GetNum();
    pBefore->mPrevPkt = (short)iNew;
    ++latencyQueue.pktCount;
    if (latencyQueue.firstPkt == iBefore)
    {
        latencyQueue.firstPkt = iNew;
    }
    else
    {
        NetPacket::Get(pNew->mPrevPkt)->mNextPkt = (short)iNew;
    }
}
