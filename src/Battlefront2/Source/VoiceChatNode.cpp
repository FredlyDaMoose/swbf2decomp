#include <string.h>

#include "VoiceChatNode.h"
#include "VoiceChatTransport.h"
#include "VoiceChatUtil.h"
#include "VOHelper.h"
#include "NetPacket.h"
#include "GameVoiceChatUtil.h"
#include "GameVoiceChatDecoder.h"
#include <float.h>

#include "GameVoiceChat.h"

#include "GameVoiceChatNode.h"

// GLOBAL: SWBF2 0x01eafd3c
void*          VoiceChat::Node::sTransport;
// GLOBAL: SWBF2 0x01eafd40
unsigned short VoiceChat::Node::sSeqNumVariance;
// GLOBAL: SWBF2 0x01eafd42
unsigned char  VoiceChat::Node::sSkipDropped;
// GLOBAL: SWBF2 0x01eafd44
float          VoiceChat::Node::sBufferSizeMs;
// GLOBAL: SWBF2 0x01eafd48
unsigned short VoiceChat::Node::sRepeatMax;
// GLOBAL: SWBF2 0x01eafd4c
float          VoiceChat::Node::sReceiveBufferSize;
// GLOBAL: SWBF2 0x01eafd50
void*          VoiceChat::Node::sSendMetrics;
// GLOBAL: SWBF2 0x01eafd54
void*          VoiceChat::Node::sReceiveMetrics;
// GLOBAL: SWBF2 0x01eafd58
int            VoiceChat::Node::sTime;

// FUNCTION: SWBF2 0x00672770
VoiceChat::Node::Header::Header()
{
    mBitmap = 0;
    mSize = 0;
    mBits = 0;
}

// GLOBAL: SWBF2 0x01f9a26c
// SYNTHETIC: SWBF2 0x00406f80
// `dynamic initializer for 'VoiceChat::Node::sReadHeader''
VoiceChat::Node::Header VoiceChat::Node::sReadHeader;

// GLOBAL: SWBF2 0x01f9a730
// SYNTHETIC: SWBF2 0x00406f90
// `dynamic initializer for 'VoiceChat::Node::sWriteHeader''
VoiceChat::Node::Header VoiceChat::Node::sWriteHeader;

namespace VoiceChat { void* Alloc(unsigned int uiSize); }

namespace VoiceChat { void Free(void* pBlock); }

// GLOBAL: SWBF2 0x007ebee8
void* VoiceChat::Node::sList[4] = { VoiceChat::Node::sList, VoiceChat::Node::sList, VoiceChat::Node::sList, 0 };
// GLOBAL: SWBF2 0x007ebef8

int sRegisteredNodeCount;

// FUNCTION: SWBF2 0x00674580
void VoiceChat::Node::SetSkipOutOfSequence(float fRate)
{
    const float fPeriod = VoiceChat::Node::sBufferSizeMs / 1000.0f;

    VoiceChat::Node::sSkipDropped = 0;

    const int iCount = (int)(fRate / fPeriod);
    VoiceChat::Node::sRepeatMax = (iCount > 0xFFFF) ? (unsigned short)0xFFFF
                                     : (unsigned short)iCount;
}

// FUNCTION: SWBF2 0x00673d40
bool VoiceChat::Node::OpenStatic(float fSeconds, float fA, float fB)
{

    VoiceChat::Node::sTransport = &GameVoiceChat::gTransport;

    const unsigned int uiNodes = ((const VoiceChat::Transport*)(&GameVoiceChat::gTransport))->GetNumNodes();
    const unsigned int uiNodesLog2 = ((const VoiceChat::Transport*)(&GameVoiceChat::gTransport))->GetNumNodesLog2();

    const unsigned int uiBitmapBytes = ((uiNodesLog2 + 8 + uiNodes) & ~7u) >> 3;
    VoiceChat::Node::sReadHeader.mSize = uiBitmapBytes;
    VoiceChat::Node::sWriteHeader.mSize = uiBitmapBytes;

    const unsigned int uiSize = uiNodes * 68 + ((uiBitmapBytes + 3) >> 2) * 8;

    char* p = (char*)VoiceChat::Alloc(uiSize);
    VoiceChat::Node::sReadHeader.mBitmap = (unsigned char*)p;
    VoiceChat::Node::sReadHeader.mBits = 0;
    VoiceChat::Node::sWriteHeader.mBits = 0;

    if (!p)
    {
        VoiceChat::Node::sTransport = 0;
        VoiceChat::Node::sReadHeader.mSize = 0;
        return false;
    }

    char* pSecond = p + ((VoiceChat::Node::sReadHeader.mSize + 3) >> 2) * 4;
    VoiceChat::Node::sWriteHeader.mBitmap = (unsigned char*)pSecond;

    char* pPerNode = pSecond + ((VoiceChat::Node::sWriteHeader.mSize + 3) >> 2) * 4;
    VoiceChat::Node::sReceiveMetrics = pPerNode;
    VoiceChat::Node::sSendMetrics = pPerNode + uiNodes * 36;

    memset(VoiceChat::Node::sReadHeader.mBitmap, 0, VoiceChat::Node::sWriteHeader.mSize + uiNodes * 68 + VoiceChat::Node::sReadHeader.mSize);

    VoiceChat::Node::sTime = 0;
    VoiceChat::Node::sBufferSizeMs = (1.0f / fSeconds) * 2.0f * 1000.0f;
    VoiceChat::Node::sReceiveBufferSize = fA / 1000.0f;

    if (uiNodes != 0)
    {
        const float fPerNode = fB / 1000.0f;
        char* q = (char*)pPerNode;
        for (unsigned int i = uiNodes; i != 0; --i, q += 0x24)
        {
            *(float*)q = fPerNode;
        }
    }

    VoiceChat::Node::sSeqNumVariance = (unsigned short)(int)fSeconds;
    if (VoiceChat::Node::sSeqNumVariance > 0x3FF)
    {
        VoiceChat::Node::sSeqNumVariance = 0x3FF;
    }

    VoiceChat::Node::SetSkipOutOfSequence(0.2f);
    return true;
}

// FUNCTION: SWBF2 0x00672b00
VoiceChat::Node::Node(Encoder* const encoder, Decoder* const decoder)
    : mEncoder(encoder),
      mDecoder(decoder)
{
    mIsOpen = false;
    mMaxReceivePackets = 0;
    mSendSeq = 0;
    mLastCaptureTime = 0.0f;
    mLastSendTime = 0.0f;
    mLastReceiveTime = 0.0f;
    mBufferUpdateTime = 0.0f;
    mReceiveStreams = 0;
    mAllocatedInputs = 0;
    mMaxReceiveSize = 0;
    mNodeID = 0xFF;
    mNumDecoderInputs = 0;
}

// FUNCTION: SWBF2 0x00673080
unsigned int VoiceChat::Node::UpdateReceiveStreamLatency(unsigned char ucIndex)
{
    const unsigned int uiIndex = ucIndex;

    const ReceiveMetrics& slot = ((const ReceiveMetrics*)sReceiveMetrics)[uiIndex];

    const int iSlot = slot.lastPacketSize;
    const int iDenom = iSlot ? iSlot : 1;

    const unsigned int uiBytes = mEncoder->GetMaxOutputPacketSizeBytes();

    const float fShare = (float)(int)uiBytes / (float)iDenom;
    const float fRate = slot.receiveLatency / mEncoderPacketSizeSec;

    unsigned int uiCount = (unsigned int)(int)(fShare * fRate);
    if (uiCount == 0)
    {
        uiCount = 1;
    }
    else
    {
        const unsigned int uiPackets = mMaxReceivePackets;
        if (uiCount > uiPackets)
        {
            uiCount = uiPackets;
        }
    }

    mReceiveStreams[uiIndex].numPackets = (unsigned short)uiCount;

    return (unsigned short)uiCount;
}

// FUNCTION: SWBF2 0x00672b70
bool VoiceChat::Node::Open()
{

    if (!mEncoder->Open())
    {
        return false;
    }

    if (!mDecoder->Open(mEncoder->GetMaxOutputPacketSizeMs(), 0x0C))
    {
        mEncoder->Close();
        return false;
    }

    const float fSeconds = mEncoder->GetMaxOutputPacketSizeMs() / 1000.0f;
    const unsigned int uiPackets =
        (unsigned short)(int)(VoiceChat::Node::sReceiveBufferSize / fSeconds);

    const unsigned int uiBytes = mEncoder->GetMaxOutputPacketSizeBytes();
    const unsigned int uiNodes = ((const VoiceChat::Transport*)(VoiceChat::Node::sTransport))->GetNumNodes();
    const unsigned int uiAlignedBytes = (uiBytes + 3u) & ~3u;

    char* const pBlock = (char*)VoiceChat::Alloc(
        ((uiAlignedBytes + 0x10u) * uiPackets + 0x1Cu) * uiNodes + 0x0Cu);
    mReceiveStreams = (ReceiveStream*)pBlock;

    if (!pBlock)
    {
        mDecoder->Close();
        mEncoder->Close();
        return false;
    }

    mIsOpen = true;
    mEncoderPacketSizeSec = fSeconds;
    mMaxReceiveSize = uiBytes;
    mMaxReceivePackets = (unsigned short)uiPackets;
    mLastCaptureTime = 0;
    mLastSendTime = 0;
    mLastReceiveTime = 0;
    mBufferUpdateTime = 0;
    mSendSeq = 0;
    mNumDecoderInputs = 0x0C;

    char* pEntryBuf = pBlock + uiNodes * 0x18;
    char* pData = (char*)((((unsigned long)(pEntryBuf) +
                            uiPackets * uiNodes * 0x10u) + 3u) & ~3u);

    for (unsigned int i = 0; i < uiNodes; ++i)
    {
        ReceiveStream& entry = mReceiveStreams[i];
        unsigned char* const pEntry = (unsigned char*)&entry;

        pEntry[0x17] &= 0xFC;
        *(unsigned int*)(pEntry + 0x12) = 0;
        entry.packet = (AudioPacket*)pEntryBuf;
        *(unsigned int*)(pEntry + 0x04) = 0;
        *(unsigned int*)(pEntry + 0x08) = 0;
        *(unsigned int*)(pEntry + 0x0C) = 0;
        pEntry[0x16] = (char)0xFF;

        UpdateReceiveStreamLatency((unsigned char)i);

        for (unsigned int p = uiPackets; p != 0; --p)
        {
            pEntryBuf[0x0C] &= 0xFE;
            ((AudioPacket*)pEntryBuf)->mData = pData;
            pData += uiAlignedBytes;
            *(unsigned int*)(pEntryBuf + 0x04) = 0;
            *(unsigned int*)(pEntryBuf + 0x08) = 0;
            pEntryBuf += 0x10;
        }
    }

    mAllocatedInputs = (unsigned char*)pData;
    for (unsigned int b = 0; b < mNumDecoderInputs; ++b)
    {
        mAllocatedInputs[b] = (unsigned char)0xFF;
    }

    return true;
}

// FUNCTION: SWBF2 0x00672d50
bool VoiceChat::Node::IsOpen() const
{
    return mIsOpen;
}

// FUNCTION: SWBF2 0x00672d60
void VoiceChat::Node::Close()
{
    Close(false, false);
}

// FUNCTION: SWBF2 0x00672d70
void VoiceChat::Node::Close(bool, bool)
{
    mEncoder->Stop();
    mEncoder->Close();
    mDecoder->Stop();
    mDecoder->Close();

    SetID(0xFF);
    VoiceChat::Free(mReceiveStreams);

    mIsOpen = false;

    mEncoderPacketSizeSec = 0;
    mReceiveStreams = 0;
    mMaxReceiveSize = 0;
    mMaxReceivePackets = 0;
    mSendSeq = 0;
    mNumDecoderInputs = 0;
    mAllocatedInputs = 0;
    mLastCaptureTime = 0;
    mLastSendTime = 0;
    mLastReceiveTime = 0;
    mBufferUpdateTime = 0;
}

// FUNCTION: SWBF2 0x00672de0
void VoiceChat::Node::SetID(unsigned char uChannel)
{
    const unsigned char uOld = mNodeID;
    if (uOld == uChannel)
    {
        return;
    }

    if (uChannel == 0xFF || uOld != 0xFF)
    {
        ((VoiceChat::Transport*)VoiceChat::Node::sTransport)->SetLocalNode(uOld, false, false, false);
        --sRegisteredNodeCount;

        PblList<Node>::Node* const pPrev = _pNext;
        PblList<Node>::Node* const pNext = _pPrev;
        _pList = 0;
        _pObject = 0;
        pPrev->_pPrev = pNext;
        _pPrev->_pNext = pPrev;

        const unsigned char uNodes = ((const VoiceChat::Transport*)(VoiceChat::Node::sTransport))->GetNumNodes();

        VoiceChat::AudioPacket packet;
        packet.mDiscontinuity = true;
        packet.mData = 0;
        packet.mSize = 0;
        packet.mAppData = 0;

        ReceiveStream* pStream = mReceiveStreams;
        for (unsigned char uLeft = uNodes; uLeft != 0; --uLeft)
        {
            Flush(*pStream, packet);
            ++pStream;
        }

        mEncoder->Stop();
        mDecoder->Stop();
    }

    if (uChannel != 0xFF)
    {
        mEncoder->Play();
        mDecoder->Play();

        const unsigned char uB = mEncoder->IsEnabled();
        const unsigned char uA = mDecoder->IsEnabled();

        ((VoiceChat::Transport*)VoiceChat::Node::sTransport)->SetLocalNode(uChannel, true, uA != 0, uB != 0);
        ++sRegisteredNodeCount;

        _pObject = this;
        _pList = (PblList<Node>*)VoiceChat::Node::sList;
        _pNext = (PblList<Node>::Node*)VoiceChat::Node::sList;
        _pPrev = (PblList<Node>::Node*)VoiceChat::Node::sList[2];
        VoiceChat::Node::sList[2] = this;
        _pPrev->_pNext = this;
    }

    mNodeID = uChannel;
}

// FUNCTION: SWBF2 0x00673ee0
void VoiceChat::Node::CloseStatic()
{

    VoiceChat::Free(VoiceChat::Node::sReadHeader.mBitmap);

    VoiceChat::Node::sTransport = 0;
    VoiceChat::Node::sReadHeader.mBitmap = 0;
    VoiceChat::Node::sReadHeader.mSize = 0;
    VoiceChat::Node::sReadHeader.mBits = 0;
    VoiceChat::Node::sWriteHeader.mBitmap = 0;
    VoiceChat::Node::sWriteHeader.mSize = 0;
    VoiceChat::Node::sWriteHeader.mBits = 0;
    VoiceChat::Node::sReceiveMetrics = 0;
    VoiceChat::Node::sBufferSizeMs = 0.0f;
    VoiceChat::Node::sTime = 0;

}

void sub_5D3BF0(void* p, int i);

// FUNCTION: SWBF2 0x00672f40
float VoiceChat::Node::GetLastSendTime() const
{
    return mLastSendTime;
}

// FUNCTION: SWBF2 0x00672f50
bool VoiceChat::Node::IsReceiving() const
{
    return (GetTime() - mLastReceiveTime) * 1000.0f < GetBufferSizeMs() * 2.0f;
}

// FUNCTION: SWBF2 0x00672f80
bool VoiceChat::Node::IsCapturing() const
{
    return (GetTime() - mLastCaptureTime) * 1000.0f < GetBufferSizeMs() * 2.0f;
}

// FUNCTION: SWBF2 0x00674340
float VoiceChat::Node::GetTime()
{
    return *(float*)&VoiceChat::Node::sTime;
}

// FUNCTION: SWBF2 0x00674350
float VoiceChat::Node::GetBufferSizeMs()
{
    return VoiceChat::Node::sBufferSizeMs;
}

// FUNCTION: SWBF2 0x00674360
VoiceChat::Node* VoiceChat::Node::FindByID(unsigned char uNodeID)
{
    PblList<VoiceChat::Node>::Node* pNode =
        (PblList<VoiceChat::Node>::Node*)VoiceChat::Node::sList[1];
    while (pNode->_pObject)
    {
        if (pNode->_pObject->GetID() == uNodeID)
        {
            return pNode->_pObject;
        }
        pNode = pNode->_pNext;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00672fd0
void VoiceChat::Node::Send()
{
    if (mNodeID == 0xFF)
    {
        return;
    }

    while (AudioPacket* packet = mEncoder->GetData())
    {
        if (packet->mSize || packet->mDiscontinuity)
        {
            sub_5D3BF0(&packet->mSize, 0);
            mLastCaptureTime = GetTime();
            SendAudioPacket(packet);
        }
        mEncoder->FreeAudioPacket(packet);
    }

    const float fSinceSendMs = (GetTime() - GetLastSendTime()) * 1000.0f;
    ((Transport*)VoiceChat::Node::sTransport)->SetSending(GetID(), GetBufferSizeMs() * 2.0f > fSinceSendMs);
}

// FUNCTION: SWBF2 0x00673a50
void VoiceChat::Node::Flush(ReceiveStream& stream, AudioPacket& packet)
{
    while (stream.packet[stream.readIndex].mAppData)
    {
        AudioPacket& queued = stream.packet[stream.readIndex++];
        queued.mAppData = 0;
        queued.mSize = 0;
        if (stream.readIndex == mMaxReceivePackets)
        {
            stream.readIndex = 0;
        }
    }
    DecoderReceiveAudioPacket((unsigned char)(&stream - mReceiveStreams), &packet);
}

// FUNCTION: SWBF2 0x00673c10
bool VoiceChat::Node::DecoderReceiveAudioPacket(unsigned char stream,
                                                const AudioPacket* const packet)
{
    bool bReceived = false;
    ReceiveStream& s = mReceiveStreams[stream];

    if (packet->mSize)
    {
        if (s.decoderInput == 0xFF)
        {
            DecoderInputAlloc(stream);
        }
        if (s.decoderInput != 0xFF)
        {
            bReceived = mDecoder->ReceivePacket(s.decoderInput, packet);
        }
    }

    if (packet->mDiscontinuity && s.decoderInput != 0xFF &&
        ((bReceived && packet->mSize) ||
         (!packet->mSize && mDecoder->ReceivePacket(s.decoderInput, packet))))
    {
        DecoderInputFree(s.decoderInput, false);
        return true;
    }
    return bReceived;
}

// FUNCTION: SWBF2 0x006731a0
void VoiceChat::Node::Decode(float fDeltaTime)
{
    Transport* const pTransport = (Transport*)VoiceChat::Node::sTransport;
    const unsigned char numStreams = ((const VoiceChat::Transport*)(pTransport))->GetNumNodes();
    bool bStarving = false;

    for (unsigned char i = 0; i < numStreams; ++i)
    {
        ReceiveStream& stream = mReceiveStreams[i];
        if (stream.activated)
        {
            DecodeQueue(i, false);
            if (stream.bufferedData == 0.0f)
            {
                bStarving |= stream.decoderInput != 0xFF && pTransport->GetReceivePossible(mNodeID, i) &&
                             CalculateNumPackets(stream) == 0;
            }
        }
        if (stream.bufferedData != 0.0f)
        {
            stream.bufferedData -= fDeltaTime;
            if (stream.bufferedData < 0.0f)
            {
                stream.bufferedData = 0.0f;
            }
        }
    }

    const bool bWasReceiving = pTransport->IsConnectedReceive(mNodeID);
    const bool bDecoderEnabled = mDecoder->IsEnabled();
    pTransport->SetLocalNode(mNodeID, true, bDecoderEnabled, ((VoiceChat::Encoder*)mEncoder)->IsEnabled() != 0);
    if (bDecoderEnabled != bWasReceiving)
    {
        AudioPacket packet;
        packet.mDiscontinuity = true;
        packet.mData = 0;
        packet.mSize = 0;
        packet.mAppData = 0;
        for (unsigned char i = 0; i < numStreams; ++i)
        {
            Flush(mReceiveStreams[i], packet);
        }
    }

    if (bStarving || mDecoder->Update())
    {
        AudioPacket packet;
        packet.mDiscontinuity = true;
        packet.mData = 0;
        packet.mSize = 0;
        packet.mAppData = 0;
        for (unsigned char i = 0; i < numStreams; ++i)
        {
            ReceiveStream& stream = mReceiveStreams[i];
            if (DecoderIsStarving(i, bStarving) && stream.activated)
            {
                ReceiveMetrics& metrics = ((ReceiveMetrics*)VoiceChat::Node::sReceiveMetrics)[i];
                const float fTime = *(float*)&VoiceChat::Node::sTime;
                const float fSinceStarve = fTime - metrics.lastStarveTime;
                ++metrics.starvedCount;
                metrics.lastStarveTime = fTime;
                metrics.starvedFrequency = (fSinceStarve + metrics.starvedFrequency) * 0.5f;
                metrics.receiveLatency = VoiceChat::Node::sReceiveBufferSize;
                UpdateReceiveStreamLatency(i);
                if (!stream.activated && CalculateNumPackets(stream) >= stream.numPackets)
                {
                    stream.activated = true;
                    stream.activationTime = GetTime();
                }
                mBufferUpdateTime = 1.0f;
                DecoderReceiveAudioPacket(i, &packet);
            }
        }
        if (bStarving)
        {
            mDecoder->Update();
        }
    }

    if (mBufferUpdateTime <= 0.0f)
    {
        for (unsigned char i = 0; i < numStreams; ++i)
        {
            if (((ReceiveMetrics*)VoiceChat::Node::sReceiveMetrics)[i].activated)
            {
                UpdateReceiveStreamLatency(i);
            }
        }
        mBufferUpdateTime = 1.0f;
    }
    mBufferUpdateTime -= fDeltaTime;
}

// FUNCTION: SWBF2 0x00672fb0
const VoiceChat::Node::ReceiveStream& VoiceChat::Node::GetReceiveStream(unsigned char stream) const
{
    return mReceiveStreams[stream];
}

// FUNCTION: SWBF2 0x00673cf0
unsigned int VoiceChat::Node::CalculateNumPackets(const ReceiveStream& stream) const
{
    const unsigned int write = stream.writeIndex;
    const unsigned int read = stream.readIndex;
    if (write == read)
    {
        return stream.packet[write].mAppData ? mMaxReceivePackets : 0;
    }
    if (write > read)
    {
        return write - read;
    }
    return write + mMaxReceivePackets - read;
}

// FUNCTION: SWBF2 0x00674390
const VoiceChat::Node::SendMetrics& VoiceChat::Node::GetSendMetrics(unsigned char node)
{
    return ((const VoiceChat::Node::SendMetrics*)VoiceChat::Node::sSendMetrics)[node];
}

// FUNCTION: SWBF2 0x006743a0
const VoiceChat::Node::ReceiveMetrics& VoiceChat::Node::GetReceiveMetrics(unsigned char node)
{
    return ((const VoiceChat::Node::ReceiveMetrics*)VoiceChat::Node::sReceiveMetrics)[node];
}

// FUNCTION: SWBF2 0x00674320
float VoiceChat::Node::GetLastReceiveTime(unsigned char ucNode)
{
    return GetReceiveMetrics(ucNode).lastReceiveTime;
}

namespace VoiceChat
{

// FUNCTION: SWBF2 0x00672790
void Node::Header::SendSetup(const Transport& transport, unsigned char node, unsigned char receiver,
                             bool send, unsigned short sequence)
{
    Reset();
    WriteBits(receiver == 0xff, 1);
    WriteBits(send, 1);
    WriteBits(sequence, 10);
    const unsigned char nBits = transport.GetNumNodesLog2();
    WriteBits(node, nBits);
    if (receiver == 0xff)
    {
        WriteBits(0, transport.GetNumNodes());
    }
    else
    {
        WriteBits(receiver, nBits);
    }
}

// FUNCTION: SWBF2 0x00672830
void Node::Header::SetReceiveNode(const Transport& transport, unsigned char node, bool receive)
{
    const unsigned int bit = mBits + node - transport.GetNumNodes();
    VoiceChat::SetBit(mBitmap, bit, receive);
}

// FUNCTION: SWBF2 0x00672870
bool Node::Header::GetReceiveNode(const Transport& transport, unsigned char node)
{
    const Format format = GetFormat();
    const unsigned int nBits = transport.GetNumNodesLog2();
    if (format == format_direct)
    {
        return node == ReadBits(12 + nBits, (unsigned char)nBits);
    }
    if (format == format_relay)
    {
        return VoiceChat::GetBit(mBitmap, 12 + nBits + node);
    }
    return false;
}

// FUNCTION: SWBF2 0x006728f0
unsigned int Node::Header::Write(NetPacketAudio* const pPacket) const
{
    const unsigned int nBytes = (mBits + 7) >> 3;
    pPacket->Write(mBitmap, nBytes);
    return nBytes;
}

// FUNCTION: SWBF2 0x00672920
bool Node::Header::Read(const Transport& transport, NetPacketAudio* const pPacket)
{
    Reset();
    if (pPacket->Read(mBitmap, 1) != 1)
    {
        return false;
    }
    const Format format = GetFormat();
    const unsigned int nBits = CalculateSize(transport, format);
    const unsigned int nBytes = ((nBits + 7) >> 3) - 1;
    if (pPacket->Read((char*)mBitmap + 1, nBytes) != nBytes)
    {
        return false;
    }
    mBits = nBits;
    return true;
}

// FUNCTION: SWBF2 0x006729a0
unsigned char Node::Header::GetSendNodeID(const Transport& transport) const
{
    return (unsigned char)ReadBits(12, transport.GetNumNodesLog2());
}

// FUNCTION: SWBF2 0x006729c0
unsigned char Node::Header::GetNumReceiveNodes(const Transport& transport) const
{
    const Format format = GetFormat();
    if (format == format_direct)
    {
        return 1;
    }
    if (format != format_relay)
    {
        return 0;
    }
    unsigned char result = 0;
    const unsigned int first = transport.GetNumNodesLog2() + 1;
    const unsigned int count = transport.GetNumNodes();
    for (unsigned int i = 0; i < count; ++i)
    {
        result += ReadBits(first + i, 1) != 0;
    }
    return result;
}

// FUNCTION: SWBF2 0x00672a20
bool Node::Header::GetDiscontinuity() const
{
    return ReadBits(1, 1) != 0;
}

// FUNCTION: SWBF2 0x00672a30
unsigned short Node::Header::GetSeqNum() const
{
    return (unsigned short)ReadBits(2, 10);
}

// FUNCTION: SWBF2 0x00672a40
void Node::Header::Reset()
{
    memset(mBitmap, 0, mSize);
    mBits = 0;
}

// FUNCTION: SWBF2 0x00672a60
void Node::Header::WriteBits(unsigned int value, unsigned char count)
{
    VoiceChat::SetBits(mBitmap, mBits, value, count);
    mBits += count;
}

// FUNCTION: SWBF2 0x00672a90
unsigned int Node::Header::ReadBits(unsigned int bit, unsigned char count) const
{
    return VoiceChat::GetBits(mBitmap, bit, count);
}

// FUNCTION: SWBF2 0x00672ab0
Node::Header::Format Node::Header::GetFormat() const
{
    return (Format)(ReadBits(0, 1) + 1);
}

// FUNCTION: SWBF2 0x00672ac0
unsigned int Node::Header::CalculateSize(const Transport& transport, Format format) const
{
    const unsigned int nBits = transport.GetNumNodesLog2();
    if (format == format_direct)
    {
        return 12 + 2 * nBits;
    }
    if (format == format_relay)
    {
        return 12 + nBits + transport.GetNumNodes();
    }
    return 0;
}

}

// FUNCTION: SWBF2 0x00673480
void VoiceChat::Node::SendAudioPacket(const AudioPacket* const packet)
{
    Transport* transport = (Transport*)VoiceChat::Node::sTransport;
    Transport::Method& method = transport->mMethodIncoming;
    const unsigned char numNodes = transport->mNumNodes;
    switch (method.mType)
    {
    case Transport::Method::type_direct:
        for (unsigned char node = 0; node < numNodes; ++node)
        {
            if (transport->GetSendPossible(mNodeID, node))
            {
                VoiceChat::Node::sWriteHeader.SendSetup((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), mNodeID, node, packet->mDiscontinuity, mSendSeq);
                SendAudioPacket(packet, &sWriteHeader, node, &mLastSendTime);
            }
        }
        ++mSendSeq;
        break;
    case Transport::Method::type_relay:
    {
        unsigned char receivers = 0;
        for (unsigned char node = 0; node < numNodes; ++node)
        {
            if (transport->GetSendPossible(mNodeID, node))
            {
                ++receivers;
            }
        }
        const unsigned short sequence = mSendSeq++;
        SendAudioPacketPeerRelay(packet, mNodeID, numNodes, ((VoiceChat::Transport::Method*)(&method))->CalculateOptimalRelayNodes(receivers),
                   sequence, false, &mLastSendTime);
        break;
    }
    case Transport::Method::type_serverRelay:
    {
        const unsigned short sequence = mSendSeq++;
        VoiceChat::Node::sWriteHeader.SendSetup((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), mNodeID, 0xff, packet->mDiscontinuity, sequence);

        const float sendTime = GetTime();
        for (unsigned char node = 0; node < numNodes; ++node)
        {
            const bool receive = transport->GetSendPossible(mNodeID, node);
            sWriteHeader.SetReceiveNode((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), node, receive);
            SendMetrics& metrics = ((SendMetrics*)VoiceChat::Node::sSendMetrics)[node];
            ++metrics.sendCount;
            metrics.lastSendTime = sendTime;
        }
        SendAudioPacket(packet, &sWriteHeader, 0xff, &mLastSendTime);
        break;
    }
    default:
        break;
    }
    mSendSeq &= 0x3ff;
}

// FUNCTION: SWBF2 0x00673670
void VoiceChat::Node::SendAudioPacketPeerRelay(const AudioPacket* const packet, unsigned char first,
                           unsigned char numNodes, unsigned char maxRelayNodes,
                           unsigned short sequence, bool incoming, float* sendTime)
{
    VoiceChat::Transport* transport = (VoiceChat::Transport*)VoiceChat::Node::sTransport;
    unsigned char node = first;
    unsigned char selected = 0xff;
    unsigned char grouped = 0;
    for (unsigned int index = 0; index < numNodes; ++index)
    {
        const bool last = index + 1 == numNodes;
        const bool receive = incoming ? VoiceChat::Node::sReadHeader.GetReceiveNode((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), node)
                                      : transport->GetSendPossible(first, node);
        const bool firstInGroup = receive && selected == 0xff;
        if (firstInGroup)
        {
            VoiceChat::Node::sWriteHeader.SendSetup((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), first, 0xff, packet->mDiscontinuity, sequence);
            for (unsigned char i = 0; i < numNodes; ++i)
            {
                VoiceChat::Node::sWriteHeader.SetReceiveNode((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), i, i == node);
            }
            selected = node;
            grouped = 0;
        }
        if (selected != 0xff && ((!firstInGroup && receive) || last))
        {
            VoiceChat::Node::sWriteHeader.SetReceiveNode((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), node, receive);
            if (++grouped == maxRelayNodes || last)
            {
                SendAudioPacket(packet, &VoiceChat::Node::sWriteHeader, selected, sendTime);
                selected = 0xff;
            }
        }
        if (++node == numNodes)
        {
            node = 0;
        }
    }
}

// FUNCTION: SWBF2 0x006743b0
bool VoiceChat::Node::SendAudioPacket(const AudioPacket* const audio, const Header* const  ,
                                       unsigned char destination, float* const sendTime)
{
    VoiceChat::Node::SendMetrics* metrics = destination == 0xff ? 0
        : &((VoiceChat::Node::SendMetrics*)VoiceChat::Node::sSendMetrics)[destination];
    NetPacket* packet = (NetPacket*)VoiceChat::NetPacketAudio::Alloc();
    if (!packet)
    {
        if (metrics)
        {
            ++metrics->droppedCount;
        }
        return false;
    }
    NetPacket* wrapper = packet;
    const unsigned short headerBytes = (unsigned short)VoiceChat::Node::sWriteHeader.Write((VoiceChat::NetPacketAudio*)&wrapper);
    const unsigned int bytes = audio->mSize ? ((VoiceChat::NetPacketAudio*)&wrapper)->Write(
        audio->mData, audio->mSize) : 0;
    VoiceChat::NetPacketAudio::Send(&wrapper, destination, headerBytes);
    if (sendTime)
    {
        *sendTime = VoiceChat::Node::GetTime();
    }
    if (metrics)
    {
        if (sendTime)
        {
            const float previous = metrics->lastSendTimeDirect;
            if (*sendTime > previous)
            {
                metrics->lastSendInterval = *sendTime - previous;
                metrics->bandwidth = (float)bytes * 0.0078125f / metrics->lastSendInterval;
            }
            else
            {
                metrics->bandwidth = (float)bytes * 0.0078125f / metrics->lastSendInterval + metrics->bandwidth;
            }
            metrics->lastSendTime = *sendTime;
            metrics->lastSendTimeDirect = *sendTime;
        }
        ++metrics->sendCountDirect;
        ++metrics->sendCount;
    }
    return true;
}

// FUNCTION: SWBF2 0x006744d0
void VoiceChat::Node::RelayAudioPacket(const AudioPacket* const packet, unsigned char node,
                                        unsigned short sequence)
{
    VoiceChat::Transport* transport = (VoiceChat::Transport*)VoiceChat::Node::sTransport;
    VoiceChat::Transport::Method& method = transport->mMethodIncoming;
    if (method.mType == VoiceChat::Transport::Method::type_relay)
    {
        SendAudioPacketPeerRelay(packet, node, transport->mNumNodes, method.GetNumRelayNodes(),
                   sequence, true, 0);
    }
    else if (method.mType == VoiceChat::Transport::Method::type_serverRelay)
    {
        for (unsigned char i = 0; i < transport->mNumNodes; ++i)
        {
            if (VoiceChat::Node::sReadHeader.GetReceiveNode((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), i))
            {
                VoiceChat::Node::sWriteHeader.SendSetup((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), node, i, packet->mDiscontinuity, sequence);
                SendAudioPacket(packet, &VoiceChat::Node::sWriteHeader, i, 0);
            }
        }
    }
}

namespace VoiceChat
{

// FUNCTION: SWBF2 0x00673110
void Node::RepeatLastPacket(unsigned char stream)
{
    ReceiveStream& receive = mReceiveStreams[stream];
    if (!receive.lastPacketSize)
    {
        return;
    }
    const unsigned short previous = receive.readIndex ? receive.readIndex - 1 : mMaxReceivePackets - 1;
    AudioPacket& packet = receive.packet[previous];
    if (packet.mAppData)
    {
        return;
    }
    const bool different = &receive.packet[receive.writeIndex] != &packet;
    packet.mSize = receive.lastPacketSize;
    ReceiveAudioPacket(stream, &packet);
    if (different)
    {
        packet.mSize = 0;
    }
    DecodeQueue(stream, true);
}

// FUNCTION: SWBF2 0x006737a0
void Node::DecodeQueue(unsigned char stream, bool force)
{
    ReceiveStream& receive = mReceiveStreams[stream];
    if (!receive.activated && CalculateNumPackets(receive) < receive.numPackets && !force)
    {
        return;
    }

    AudioPacket* packet = &receive.packet[receive.readIndex];
    const float timePerByte = mEncoderPacketSizeSec / (float)((VoiceChat::Encoder*)mEncoder)->GetMaxOutputPacketSizeBytes();
    const bool connected = ((Transport*)VoiceChat::Node::sTransport)->GetReceivePossible(mNodeID, stream);
    if (!connected && receive.activated)
    {
        AudioPacket discontinuity = {};
        discontinuity.mDiscontinuity = true;
        DecoderReceiveAudioPacket(stream, &discontinuity);
    }
    while (packet->mAppData)
    {
        if (connected && !DecoderReceiveAudioPacket(stream, packet))
        {
            return;
        }
        if (force)
        {
            receive.activated = true;
            receive.starving = connected;
        }
        else if (connected)
        {
            receive.activated = !packet->mDiscontinuity;
            receive.starving = false;
            receive.lastPacketSize = packet->mSize;
            receive.bufferedData += (float)packet->mSize * timePerByte;
        }
        packet->mAppData = 0;
        packet->mSize = 0;
        if (++receive.readIndex == mMaxReceivePackets)
        {
            receive.readIndex = 0;
        }
        packet = &receive.packet[receive.readIndex];
    }
}

// FUNCTION: SWBF2 0x00673900
bool Node::ReceiveAudioPacket(unsigned char stream, const AudioPacket* packet)
{
    unsigned int copied = 0;
    if (packet->mSize)
    {
        mLastReceiveTime = Node::GetTime();
    }
    Node::ReceiveStream& receive = mReceiveStreams[stream];
    while (true)
    {
        AudioPacket& slot = receive.packet[receive.writeIndex];
        DecodeQueue(stream, false);
        if (slot.mAppData)
        {
            AudioPacket discontinuity = {};
            discontinuity.mDiscontinuity = true;
            Flush(receive, discontinuity);
            return false;
        }
        unsigned int size = packet->mSize - copied;
        const unsigned int remaining = mMaxReceiveSize - slot.mSize;
        if (size > remaining)
        {
            size = remaining;
        }
        if (size)
        {
            void* dst = (unsigned char*)slot.mData + slot.mSize;
            const void* src = (const unsigned char*)packet->mData + copied;
            if (dst != src)
            {
                memcpy(dst, src, size);
            }
        }
        slot.mSize += size;
        copied += size;
        const bool more = copied < packet->mSize;
        slot.mDiscontinuity = !more && packet->mDiscontinuity;
        if (slot.mSize == mMaxReceiveSize || slot.mDiscontinuity)
        {
            slot.mAppData = this;
            if (++receive.writeIndex == mMaxReceivePackets)
            {
                receive.writeIndex = 0;
            }
            if (slot.mDiscontinuity)
            {
                receive.activated = true;
            }
            DecodeQueue(stream, false);
        }
        if (!more)
        {
            return true;
        }
    }
}

// FUNCTION: SWBF2 0x00673ad0
unsigned char Node::DecoderInputAlloc(unsigned char stream)
{
    ReceiveStream& receive = mReceiveStreams[stream];
    const unsigned char count = ((const VoiceChat::Transport*)(VoiceChat::Node::sTransport))->GetNumNodes();
    if (mNumDecoderInputs == count)
    {
        mAllocatedInputs[stream] = stream;
        receive.decoderInput = stream;
        return stream;
    }

    unsigned char oldest = 0xff;
    float oldestTime = FLT_MAX;
    for (unsigned char input = 0; input < mNumDecoderInputs; ++input)
    {
        const unsigned char owner = mAllocatedInputs[input];
        if (owner == 0xff)
        {
            mAllocatedInputs[input] = stream;
            receive.decoderInput = input;
            return input;
        }
        if (oldestTime > mReceiveStreams[owner].activationTime)
        {
            oldestTime = mReceiveStreams[owner].activationTime;
            oldest = input;
        }
    }
    DecoderInputFree(oldest, true);
    mAllocatedInputs[oldest] = stream;
    receive.decoderInput = oldest;
    return oldest;
}

// FUNCTION: SWBF2 0x00673ba0
void Node::DecoderInputFree(unsigned char input, bool flush)
{
    if (mNumDecoderInputs == ((const VoiceChat::Transport*)(VoiceChat::Node::sTransport))->GetNumNodes())
    {
        return;
    }
    const unsigned char stream = mAllocatedInputs[input];
    ReceiveStream& receive = mReceiveStreams[stream];
    if (flush)
    {
        AudioPacket packet = {};
        packet.mDiscontinuity = true;
        Flush(receive, packet);
    }
    else
    {
        receive.decoderInput = 0xff;
        mAllocatedInputs[input] = 0xff;
    }
}

// FUNCTION: SWBF2 0x00673cc0
bool Node::DecoderIsStarving(unsigned char stream, bool force)
{
    const unsigned char input = mReceiveStreams[stream].decoderInput;
    if (input == 0xff)
    {
        return false;
    }
    return mDecoder->IsStarving(input, force);
}
}

// FUNCTION: SWBF2 0x00672f10
unsigned char VoiceChat::Node::GetID() const
{
    return mNodeID;
}

// FUNCTION: SWBF2 0x00672f20
VoiceChat::Encoder& VoiceChat::Node::GetEncoder() const
{
    return *mEncoder;
}

// FUNCTION: SWBF2 0x00672f30
VoiceChat::Decoder& VoiceChat::Node::GetDecoder() const
{
    return *mDecoder;
}

// FUNCTION: SWBF2 0x00673ed0
bool VoiceChat::Node::IsOpenStatic()
{
    return VoiceChat::Node::sTransport != 0;
}

// FUNCTION: SWBF2 0x00674020
void VoiceChat::Node::Update(float fDeltaTime)
{
    float& sTime = *(float*)&VoiceChat::Node::sTime;

    void* pAppPacket;
    while ((pAppPacket = NetPacketAudio::Receive()) != 0)
    {
        NetPacketAudio kAudio(pAppPacket);
        if (VoiceChat::Node::sReadHeader.Read(*(const VoiceChat::Transport*)VoiceChat::Node::sTransport, &kAudio))
        {
            const unsigned short uSeq = VoiceChat::Node::sReadHeader.GetSeqNum();
            const unsigned char uID = VoiceChat::Node::sReadHeader.GetSendNodeID(*(const VoiceChat::Transport*)VoiceChat::Node::sTransport);
            ReceiveMetrics& rm = ((ReceiveMetrics*)VoiceChat::Node::sReceiveMetrics)[uID];
            const bool bDiscontinuity = VoiceChat::Node::sReadHeader.GetDiscontinuity();

            if (!(uSeq < rm.receiveSeq
                  && uSeq - rm.receiveSeq + 1023 >= (int)VoiceChat::Node::sSeqNumVariance
                  && rm.activated))
            {
                unsigned short uDropped = rm.activated
                    ? (unsigned short)(uSeq - rm.receiveSeq - 1) : 0;

                AudioPacket kPacket;
                unsigned int uiSize = 0xFFFFFFFF;
                kPacket.mData = kAudio.Read(uiSize);
                kPacket.mSize = uiSize;
                kPacket.mAppData = 0;
                kPacket.mDiscontinuity = bDiscontinuity;

                if (uDropped)
                {
                    ++rm.outOfOrderCount;
                    if (uDropped > VoiceChat::Node::sRepeatMax)
                    {
                        uDropped = VoiceChat::Node::sRepeatMax;
                    }
                }

                if (ReceiveAudioPacketAllLocalNodes(uID, &kPacket, bDiscontinuity, uDropped))
                {
                    RelayAudioPacket(&kPacket, uID, uSeq);
                }

                if (rm.activated)
                {
                    const float fInterval = sTime - rm.lastReceiveTime;
                    if (fInterval != 0.0f)
                    {
                        if (fInterval > rm.receiveLatency)
                        {
                            rm.receiveLatency = rm.receiveLatency * 0.5f + fInterval * 0.5f;
                        }
                        else
                        {
                            rm.receiveLatency = rm.receiveLatency * 0.99f + fInterval * 0.01f;
                        }
                    }
                }

                rm.lastPacketSize = uiSize;
                rm.activated = !bDiscontinuity;
            }

            rm.receiveSeq = uSeq;
            rm.lastReceiveTime = sTime;
        }
        NetPacketAudio::Free(pAppPacket);
    }

    for (void** pLink = (void**)VoiceChat::Node::sList[1]; pLink[3]; pLink = (void**)pLink[1])
    {
        Node* pNode = (Node*)pLink[3];
        ((SendMetrics*)VoiceChat::Node::sSendMetrics)[pNode->GetID()].bandwidth = 0.0f;
        if (((VoiceChat::Node*)pLink[3])->IsOpen())
        {
            pNode->Send();
            pNode->Decode(fDeltaTime);
        }
    }

    const unsigned char uNodes = ((const VoiceChat::Transport*)(VoiceChat::Node::sTransport))->GetNumNodes();
    for (unsigned char i = 0; i < uNodes; ++i)
    {
        ReceiveMetrics& rm = ((ReceiveMetrics*)VoiceChat::Node::sReceiveMetrics)[i];
        if (rm.activated && sTime - rm.lastReceiveTime > VoiceChat::Node::sReceiveBufferSize)
        {
            rm.activated = false;

            AudioPacket kSilence;
            kSilence.mDiscontinuity = true;
            kSilence.mData = 0;
            kSilence.mSize = 0;
            kSilence.mAppData = 0;
            for (void** pLink = (void**)VoiceChat::Node::sList[1]; pLink[3]; pLink = (void**)pLink[1])
            {
                ((Node*)pLink[3])->ReceiveAudioPacket(i, &kSilence);
            }
        }

        SendMetrics& sm = ((SendMetrics*)VoiceChat::Node::sSendMetrics)[i];
        sm.bandwidthAverage = sm.bandwidth * (1.0f - 0.9f) + sm.bandwidthAverage * 0.9f;
    }

    sTime += fDeltaTime;
}

// FUNCTION: SWBF2 0x00673f50
bool __fastcall VoiceChat::Node::ReceiveAudioPacketAllLocalNodes(unsigned char uID,
                           const VoiceChat::AudioPacket* pPacket,
                           bool  , unsigned short uDropped)
{
    unsigned char uRemaining = VoiceChat::Node::sReadHeader.GetNumReceiveNodes((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport));
    PblList<VoiceChat::Node>* pList = (PblList<VoiceChat::Node>*)VoiceChat::Node::sList;
    for (PblList<VoiceChat::Node>::Node* pLink = pList->GetFirst();
         pLink->_pObject; pLink = pLink->_pNext)
    {
        VoiceChat::Node* pNode = pLink->_pObject;
        unsigned char uNodeID = pNode->mNodeID;
        if (!VoiceChat::Node::sReadHeader.GetReceiveNode((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), uNodeID))
        {
            continue;
        }
        VoiceChat::Transport* pTransport = (VoiceChat::Transport*)VoiceChat::Node::sTransport;
        if (pTransport->GetReceivePossible(uNodeID, uID))
        {
            if (!VoiceChat::Node::sSkipDropped)
            {
                for (unsigned int i = 0; i < uDropped; ++i)
                {
                    pNode->RepeatLastPacket(uID);
                }
            }
            pNode->ReceiveAudioPacket(uID, pPacket);
        }
        if (VoiceChat::Node::sReadHeader.GetFormat() == VoiceChat::Node::Header::format_relay)
        {
            VoiceChat::Node::sReadHeader.SetReceiveNode((*(const VoiceChat::Transport*)VoiceChat::Node::sTransport), uNodeID, false);
        }
        --uRemaining;
    }
    return uRemaining != 0;
}

// LIBRARY: SWBF2 0x0075490e
// DirectSoundCaptureCreate8
