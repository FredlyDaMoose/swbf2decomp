#pragma once
#include <stddef.h>

class NetPacket
{
public:
    static unsigned int sNextSeqNo;

    static NetPacket* Get(int iNum);

    int GetNum() const;

    static void SetHiRes(bool bHiRes);

    static void Reset();
    static void Create();

    void Free();

    NetPacket* Encrypt(int iDest);

    NetPacket* Decrypt(unsigned int key);

    bool Read(unsigned short& value);

    static NetPacket* Alloc(bool bSmall);

    void BeginWrite();

    void EndWrite();

    void BeginRead();

    bool Write(unsigned int const& uValue);
    bool Read(unsigned int& uValue);

    bool Read(unsigned char& uValue);

    bool Write(unsigned char uValue);
    bool Write(unsigned short uValue);

    int Write(char const* pData, int iLen);
    int Read(char* pData, int iLen);

    short mNextPkt;
    short mPrevPkt;
    short mCurIndex;
    short mMaxIndex;
    char mCommIndex;
    char mDataIndex;
    unsigned short mSeqNo;
    char mData[4];
};
 
 

class NetBigPacket : public NetPacket
{
public:
    char mData2[1008];
};
 

class NetSmallPacket : public NetPacket
{
public:
    char mData2[112];
};
 

const int MAX_BIG_PACKETS = 0x100;
const int MAX_SMALL_PACKETS = 0x100;

class NetPktGroupOffset
{
public:
    int pktNum;
    int byteNum;
    int bitNum;
};

class NetPktGroup
{
public:

    // FUNCTION: SWBF2 0x005aeca0
    NetPktGroup() : isSmall(false) {}

    void Reset();

    int GetBytes();

    void PushBack(NetPacket* pPkt);

    NetPacket* PopFront();

    void Free();

    void Remove(NetPacket* pPkt);

    bool IsEnd() const;
    void BeginWrite();
    void EndWrite();
    void PushFront(NetPacket* pPkt);
    NetPacket* PopBack();

    void WritePacked();
    void ReadPacked();
    void WriteBits(unsigned int uValue, int iBits);
    void ReadBits(unsigned int& uValue, int iBits);
    void Write(bool bValue);
    void Write(int iValue);
    void Write(const float& fValue);
    void Read(bool& bValue);
    void Read(float& fValue);
    void Read(int& iValue);
    void WriteFixed(float f, float fMin, float fMax, int iBits);
    void ReadFixed(float& f, float fMin, float fMax, int iBits);
    void WriteBytes(char const* pData, int iLen);
    void ReadBytes(char* pData, int iLen);

    NetPacket* GetFront();

    void BeginRead();
    void EndRead();
    void Seek(const NetPktGroupOffset& offset);

    void Set(NetPacket* pPkt);
    void Tell(NetPktGroupOffset& offset);

    static unsigned int sDebugBitCounter;

    bool isSmall;
    int firstPkt;
    int lastPkt;
    int pktCount;
    NetPacket* pktPtr;
    int curBit;
    unsigned int pktBits;
};
 

void NetPktGroupClampFixed(float& f, float fMin, float fMax, int iBits);

extern bool doHiRes;

extern NetBigPacket netBigPackets[MAX_BIG_PACKETS];
extern NetSmallPacket netSmallPackets[MAX_SMALL_PACKETS];
extern NetPktGroup freeBigPackets;
extern NetPktGroup freeSmallPackets;
