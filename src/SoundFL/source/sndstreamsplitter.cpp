#include "sndstreamsplitter.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074d100
void StreamSplitter::Setup(StreamPacket** const ppQueue,
                           unsigned short* const puRefCounts,
                           unsigned short uNumRefCounts,
                           unsigned short* const puRemaining,
                           unsigned short uNumRemaining)
{
    if (uNumRefCounts == 0 || uNumRemaining == 0)
    {
        Flush();

        mPackets = 0;
        mRemaining = 0;
        mRefCount = 0;
        mNumOutputs = 0;
        mNumPackets = 0;
        mReadPos = 0;
        return;
    }

    mPackets = ppQueue;
    mRefCount = puRefCounts;
    mRemaining = puRemaining;
    mNumOutputs = uNumRemaining;
    mNumPackets = uNumRefCounts;

    for (unsigned int i = 0; i < mNumPackets; i++)
    {
        mPackets[i] = 0;
        mRefCount[i] = 0;
    }

    for (unsigned int i = 0; i < mNumOutputs; i++)
    {
        mRemaining[i] = 0;
    }
}

// FUNCTION: SWBF2 0x0074cf70
void StreamSplitter::Flush()
{

    typedef StreamPacket* (__thiscall* GetPacketFn)(StreamSplitter* pThis,
                                                    unsigned int uiOutput);

    mFlushing = 1;

    for (unsigned int uiOutput = 0; uiOutput < mNumOutputs; uiOutput++)
    {
        for (;;)
        {
            StreamPacket* const pPacket =
                GetPacket(uiOutput);

            if (pPacket == 0)
            {
                break;
            }

            pPacket->Unlock();
        }
    }

    mReadPos = 0;
    mFlushing = 0;
}

}

namespace Snd
{

// FUNCTION: SWBF2 0x0074cf40
void* __fastcall sub_74CF40(void* pSplitter, int, unsigned int uiFlags)
{
    StreamSplitter* const pThis = (StreamSplitter*)pSplitter;

    pThis->StreamSplitter::~StreamSplitter();

    if ((uiFlags & 1) != 0)
    {
        ::operator delete(pThis);
    }

    return pThis;
}

// FUNCTION: SWBF2 0x0074cfe0
StreamPacket* StreamSplitter::GetPacket(unsigned int uiOutput)
{
    typedef StreamPacket* (__thiscall* GetPacketFn)(StreamNodeSource* pThis,
                                                    unsigned int uiOutput);

    if (((StreamNode*)mNode)->GetPlayState() == SourceTransport::playState_stopped &&
        !mFlushing)
    {
        return 0;
    }

    StreamPacket** const ppWrite = &mPackets[mReadPos];
    if (*ppWrite == 0 && !mFlushing)
    {
        StreamNodeSource* const pInput = mSink.mConnection.mInputNode;
        *ppWrite = pInput
            ? pInput->GetPacket(mSink.mConnection.mInputNodeOutputIndex)
            : 0;
        if (*ppWrite)
        {
            mRefCount[mReadPos] = 0;
            if (++mReadPos == mNumPackets)
            {
                mReadPos = 0;
            }
            for (unsigned int i = 0; i < mNumOutputs; i++)
            {
                ++mRemaining[i];
            }
        }
    }

    unsigned short* const puRemaining = &mRemaining[uiOutput];
    if (*puRemaining == 0)
    {
        return 0;
    }

    const unsigned int uiIndex = mReadPos < *puRemaining
        ? mReadPos + (mNumPackets - *puRemaining)
        : mReadPos - *puRemaining;
    StreamPacket** const ppRead = &mPackets[uiIndex];
    StreamPacket* const pPacket = *ppRead;
    if (pPacket == 0)
    {
        return 0;
    }

    unsigned short* const puRefCount = &mRefCount[uiIndex];
    --*puRemaining;
    pPacket->Lock();
    if (++*puRefCount == mNumOutputs)
    {
        pPacket->Unlock();
        *ppRead = 0;
        *puRefCount = 0;
    }
    return pPacket;
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0074cee0
StreamSplitter::StreamSplitter()
    : mPackets(0),
      mRemaining(0),
      mRefCount(0),
      mNumOutputs(0),
      mNumPackets(0),
      mReadPos(0),
      mFlushing(0)
{
    mNode[0] = 0;
    *(unsigned short*)&mNode[1] = (unsigned short)SourceTransport::flags_stopped;
    *(unsigned char*)&mNode[2] = 0;
    mSink.mConnection.mInputNode = 0;
    mSink.mConnection.mInputNodeOutputIndex = 0;

}
}

namespace Snd {

// FUNCTION: SWBF2 0x0074cf30
StreamSplitter::~StreamSplitter()
{

}
}
