#include "sndstreamsourcememory.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074c770
StreamSourceMemory::StreamSourceMemory()
{

    mSegment.mOffset = 0;
    mSegment.mLength = 0;
    mNextSegment.mOffset = 0;
    mNextSegment.mLength = 0;
    mData = 0;
    mDataSize = 0;
    mAudioFrameSize = 1;

    mLooping = 0;
}

// FUNCTION: SWBF2 0x0074c820
void* __fastcall sub_74C820(void* pSource, int, unsigned int uiFlags)
{
    StreamSourceMemory* const pThis = (StreamSourceMemory*)pSource;

    pThis->StreamSourceMemory::~StreamSourceMemory();

    if ((uiFlags & 1) != 0)
    {
        ::operator delete(pThis);
    }

    return pThis;
}

// FUNCTION: SWBF2 0x0074c850
StreamPacket* StreamSourceMemory::GetPacket(unsigned int uiOutput, bool bUpdate)
{
    StreamPacket* const pPacket = StreamSourceProducer::GetPacket(uiOutput);
    if (pPacket == 0)
    {
        if (!bUpdate)
        {
            return 0;
        }
        Update();
        return GetPacket(uiOutput, false);
    }

    if (pPacket->mFlags & 1)
    {
        Stop();
    }

    return pPacket;
}

// FUNCTION: SWBF2 0x0074c8c0
StreamPacket* StreamSourceMemory::GetPacket(unsigned int uiOutput)
{
    return GetPacket(uiOutput, true);
}

// FUNCTION: SWBF2 0x0074c9d0
void StreamSourceMemory::Update()
{
    if (IsFlushing())
    {
        return;
    }

    const unsigned int uiPacketSize = mProducer[2];
    if (mDataSize == 0 || uiPacketSize == 0)
    {
        Stop();
        return;
    }

    const PlayState eDesired = GetDesiredPlayState();

    bool bStop;
    const PlayState eCurrent = GetCurrentPlayState();
    if (eCurrent == playState_playing || eCurrent == playState_pausing)
    {
        unsigned int uiPos = mPlaybackPosition * mAudioFrameSize;
        bStop = false;

        StreamPacket* pPacket;
        while ((pPacket = GetReadPacket()) != 0 && uiPos != mSegment.mLength && !bStop)
        {
            unsigned int uiSize = mSegment.mLength - uiPos;
            bool bLast = false;
            pPacket->mBuffer = (unsigned char*)mData + mSegment.mOffset + uiPos;
            if (uiSize > uiPacketSize)
            {
                uiSize = uiPacketSize;
            }
            uiPos += uiSize;

            if (uiPos == mSegment.mLength)
            {
                if (mLooping)
                {
                    uiPos = 0;
                }
                else if (mNextSegment.mLength != 0)
                {
                    uiPos = 0;
                    mSegment = mNextSegment;
                }
                else
                {
                    bLast = true;
                }
            }

            mPlaybackPosition = uiPos / mAudioFrameSize;
            if (eDesired != playState_invalid)
            {
                bStop = eDesired != playState_playing;
            }

            PacketComplete(pPacket, uiSize, 0, bLast, bStop, mPlaybackPosition);
        }
    }
    else
    {
        bStop = true;
    }

    switch (eDesired)
    {
    case playState_playing:
        SetPlayState(playState_playing);
        break;
    case playState_paused:
        SetPlayState(playState_pausing);
        break;
    case playState_stopped:
        if (bStop)
        {
            mPlaybackPosition = 0;
            SetPlayState(playState_stopped);
        }
        else
        {
            Stop();
        }
        break;
    default:
        break;
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0074c910
void StreamSourceMemory::SetLooping(bool bLooping)
{
    mLooping = bLooping;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0074c930
void StreamSourceMemory::Setup(const StreamFormat& rFormat, void* const pData,
                               unsigned int uiDataSize, unsigned int uiPacketSize,
                               StreamPacket* pPackets, unsigned char uiNumPackets,
                               bool bSetPackets)
{
    mFormat = rFormat;
    mData = pData;
    mDataSize = uiDataSize;
    if (bSetPackets)
    {
        SetPackets(uiPacketSize, pPackets, uiNumPackets, 0);
    }

    if (mData)
    {
        const unsigned int uiFrameSize = SampleInfo::GetFrameSize(mFormat.GetFormat());
        mSegment.mLength = mDataSize;
        mAudioFrameSize = uiFrameSize * mFormat.GetNumChannels();
    }
    else
    {
        mSegment.mLength = 0;
    }
    mNextSegment.mLength = 0;
    mNextSegment.mOffset = 0;
    mSegment.mOffset = 0;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0074c810
StreamSourceMemory::~StreamSourceMemory()
{

}
}
