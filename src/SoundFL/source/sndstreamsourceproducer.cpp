#include <new>

#include "sndstreamsourceproducer.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0073c740
StreamSourceNProducer::StreamSourceNProducer()
    : mProducers(0), mNumProducers(0)
{
}

// FUNCTION: SWBF2 0x0073c760
StreamSourceProducer::StreamSourceProducer()
{
    mProducer[0] = 0;
    mProducer[1] = 0;
    mProducer[2] = 0;
    mProducer[3] = 0;
    mProducer[4] = 0;
}

// FUNCTION: SWBF2 0x0073c790
StreamNodeSource::~StreamNodeSource()
{
}

// FUNCTION: SWBF2 0x0073c7a0
StreamSourceNProducer::~StreamSourceNProducer()
{
}

// FUNCTION: SWBF2 0x0073c7b0
StreamSourceProducer::~StreamSourceProducer()
{

    Flush();
}

// FUNCTION: SWBF2 0x0073cb10
void Snd::StreamSourceProducer::SetPackets(unsigned int uiPacketSize, StreamPacket* pPackets, unsigned int uiNumPackets, int iUnread)
{
    (void)iUnread;

    if (uiPacketSize == 0 || uiNumPackets == 0 || pPackets == 0)
    {
        mProducer[0] = 0;
        mProducer[1] = 0;
        mProducer[2] = 0;
        return;
    }

    mProducer[0] = (unsigned int)pPackets;
    mProducer[1] = uiNumPackets;
    mProducer[2] = uiPacketSize;

    for (unsigned int i = 0; i < mProducer[1]; ++i)
    {
        pPackets[i].mBufferUsed = 0;
    }
}

// FUNCTION: SWBF2 0x0073cb80
void StreamSourceProducer::SetPackets(unsigned int uiPacketSize,
                                      StreamPacket* pPackets,
                                      unsigned int uiNumPackets)
{
    SetPackets(uiPacketSize, pPackets, uiNumPackets, 1);
}

// FUNCTION: SWBF2 0x0073cba0
void StreamSourceNProducer::SetProducers(StreamSourceProducer* pProducers,
                                         unsigned int uiNumProducers)
{
    if (pProducers == 0 || uiNumProducers == 0)
    {
        mProducers = 0;
        mNumProducers = 0;
        return;
    }

    mProducers = pProducers;
    mNumProducers = uiNumProducers;
}

// FUNCTION: SWBF2 0x0073c8e0
void StreamSourceProducer::Flush()
{
    for (;;)
    {

        StreamPacket* const pPacket = GetPacket(0);

        if (pPacket == 0)
        {
            break;
        }

        pPacket->Unlock();
    }
}

// FUNCTION: SWBF2 0x0073c9a0
StreamPacket* StreamSourceProducer::GetPacket(unsigned int uiOutput)
{
    (void)uiOutput;

    StreamPacket* const pPackets = (StreamPacket*)mProducer[0];
    if (pPackets == 0)
    {
        return 0;
    }

    unsigned int& uiRead = mProducer[4];
    StreamPacket* const pPacket = &pPackets[uiRead];

    if (pPacket->mRefCount != 0)
    {
        return 0;
    }
    if (pPacket->mBufferUsed == 0)
    {
        return 0;
    }

    ++pPacket->mRefCount;

    ++uiRead;
    if (uiRead == mProducer[1])
    {
        uiRead = 0;
    }

    return pPacket;
}

// FUNCTION: SWBF2 0x0073c870
void* __fastcall sub_73C870(void* pProducer, int, unsigned int uiFlags)
{

    ((StreamSourceProducer*)pProducer)->StreamSourceProducer::~StreamSourceProducer();

    if ((uiFlags & 1) != 0)
    {
        ::operator delete(pProducer);
    }

    return pProducer;
}

// FUNCTION: SWBF2 0x0073c810
void* __fastcall sub_73C810(void* pNodeSource, int, unsigned int uiFlags)
{
    ((StreamNodeSource*)pNodeSource)->StreamNodeSource::~StreamNodeSource();
    if ((uiFlags & 1) != 0)
    {
        ::operator delete(pNodeSource);
    }
    return pNodeSource;
}

// FUNCTION: SWBF2 0x0073c840
void* __fastcall sub_73C840(void* pProducer, int, unsigned int uiFlags)
{
    ((StreamSourceNProducer*)pProducer)->StreamSourceNProducer::~StreamSourceNProducer();
    if ((uiFlags & 1) != 0)
    {
        ::operator delete(pProducer);
    }
    return pProducer;
}

// FUNCTION: SWBF2 0x0073c930
unsigned int StreamSourceProducer::GetNumQueuedPackets() const
{
    const StreamPacket* const pPackets = (const StreamPacket*)mProducer[0];
    const unsigned int uiNumPackets = mProducer[1];
    unsigned int uiIndex = mProducer[3];
    unsigned int uiQueued = 0;

    for (unsigned int i = uiNumPackets; i != 0; --i)
    {
        const StreamPacket& packet = pPackets[uiIndex++];
        if (packet.mRefCount != 0 || packet.mBufferUsed != 0)
        {
            ++uiQueued;
        }
        if (uiIndex == uiNumPackets)
        {
            uiIndex = 0;
        }
    }
    return uiQueued;
}

// FUNCTION: SWBF2 0x0073c970
StreamPacket* StreamNodeSource::GetPacket(unsigned int uiOutput)
{
    (void)uiOutput;
    return 0;
}

// FUNCTION: SWBF2 0x0073c980
StreamPacket* StreamSourceNProducer::GetPacket(unsigned int uiOutput)
{
    StreamSourceProducer* const pOut = mProducers + uiOutput;
    return pOut->GetPacket(0);
}

// FUNCTION: SWBF2 0x0073ca10
StreamPacket* StreamSourceProducer::GetReadPacket()
{

    StreamPacket* const pPackets = (StreamPacket*)mProducer[0];
    StreamPacket* const pPacket = &pPackets[mProducer[3]];

    if (pPacket->mBufferUsed != 0)
    {
        return 0;
    }
    if (pPacket->mRefCount != 0)
    {
        return 0;
    }

    return pPacket;
}

// FUNCTION: SWBF2 0x0073ca90
void StreamSourceProducer::PacketComplete(StreamPacket* const pPacket,
                                          unsigned int uiBufferUsed,
                                          unsigned int uiUnused,
                                          bool bFlagBit0, bool bFlagBit1,
                                          unsigned int uiPos)
{
    (void)uiUnused;

    pPacket->mBufferUsed = uiBufferUsed;
    pPacket->mPos = uiPos;

    if (bFlagBit0)
    {
        pPacket->mFlags |= 1;
    }
    else
    {
        pPacket->mFlags &= 0xFFFE;
    }

    if (bFlagBit1)
    {
        pPacket->mFlags |= 2;
    }
    else
    {
        pPacket->mFlags &= 0xFFFD;
    }

    ++mProducer[3];
    if (mProducer[3] == mProducer[1])
    {
        mProducer[3] = 0;
    }
}

// FUNCTION: SWBF2 0x0073caf0
void StreamPacket::SetDiscontinuity(bool bDiscontinuity)
{
    if (bDiscontinuity)
    {
        mFlags |= 2;
    }
    else
    {
        mFlags &= 0xFFFD;
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0073c9e0
StreamPacket* StreamSourceNProducer::GetWritablePacket(unsigned int uiOutput)
{
    const StreamSourceProducer& producer = mProducers[uiOutput];
    StreamPacket* pPacket = (StreamPacket*)producer.mProducer[0] + producer.mProducer[3];
    return pPacket->mBufferUsed == 0 && pPacket->mRefCount == 0 ? pPacket : 0;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0073ca30
unsigned int StreamSourceNProducer::CompleteWrittenPacket(StreamPacket* pPacket,
                                   unsigned int uiBufferUsed, unsigned int uiUnused,
                                   bool bFlagBit0, bool bFlagBit1, unsigned int uiPos,
                                   unsigned int uiOutput)
{
    StreamSourceProducer& producer = mProducers[uiOutput];
    pPacket->mBufferUsed = uiBufferUsed;
    pPacket->mPos = uiPos;
    if (bFlagBit0)
    {
        pPacket->mFlags |= 1;
    }
    else
    {
        pPacket->mFlags &= ~1u;
    }
    if (bFlagBit1)
    {
        pPacket->mFlags |= 2;
    }
    else
    {
        pPacket->mFlags &= ~2u;
    }
    unsigned int& rIndex = producer.mProducer[3];
    unsigned int result = ++rIndex;
    if (result == producer.mProducer[1])
    {
        rIndex = 0;
    }
    return result;
}
}
