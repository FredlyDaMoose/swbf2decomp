#include "sndstreamcapturesourcebase.h"
#include "sndstreamnode.h"
#include "sndstreampacket.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074bfe0
StreamCaptureSourceBase::StreamCaptureSourceBase()
    : mPacketSize(0),
      mPackets(0),
      mNumPackets(0),
      mReadPacketIndex(0),
      mLockPacketIndex(0),
      mFlags(0),
      mGain(1.0f)
{
}

// FUNCTION: SWBF2 0x0074c020
StreamCaptureSourceBase::~StreamCaptureSourceBase()
{
}

// FUNCTION: SWBF2 0x0074c030
void* __fastcall sub_74C030(void* pBase, int, unsigned int uiFlags)
{
    ((StreamCaptureSourceBase*)pBase)->StreamCaptureSourceBase::~StreamCaptureSourceBase();
    if (uiFlags & 1)
    {
        operator delete(pBase);
    }
    return pBase;
}

// FUNCTION: SWBF2 0x0074c060
void StreamCaptureSourceBase::Close(bool bFlush)
{
    if (bFlush)
    {
        Flush();
    }

    SetPlayState(SourceTransport::playState_stopped);

    mFlags = (unsigned char)(mFlags & ~flags_open);

    mNumPackets = 0;
    mReadPacketIndex = 0;

    mPacketSize = 0;
    mPackets = 0;
    mPlaybackPosition = 0;

    SetStarved(false);
}

// FUNCTION: SWBF2 0x0074c0b0
void StreamCaptureSourceBase::Flush()
{

    for (unsigned int uiIndex = 0;
         uiIndex < mNumPackets;
         ++uiIndex)
    {
        StreamPacket* const pPacket = mPackets + uiIndex;

        if (pPacket->mRefCount == 0 && pPacket->mBufferUsed != 0)
        {
            pPacket->mBufferUsed = 0;
        }
    }

    mLockPacketIndex = mReadPacketIndex;
}

// FUNCTION: SWBF2 0x0074c0f0
StreamPacket* StreamCaptureSourceBase::GetPacket(unsigned int  )
{
    if (GetCurrentPlayState() != SourceTransport::playState_playing)
    {
        return 0;
    }

    const unsigned char ucIndex = mLockPacketIndex;
    StreamPacket* const pPacket = mPackets + ucIndex;
    if (pPacket->mBufferUsed == 0 || pPacket->mRefCount != 0)
    {
        return 0;
    }

    mLockPacketIndex = (unsigned char)(ucIndex + 1);
    if ((unsigned char)(ucIndex + 1) == mNumPackets)
    {
        mLockPacketIndex = 0;
    }

    ++pPacket->mRefCount;
    if (pPacket->mFlags & 1)
    {
        SourceTransport::Stop();
    }
    return pPacket;
}

// FUNCTION: SWBF2 0x0074c160
bool StreamCaptureSourceBase::Open(unsigned char  ,
                                   StreamFormat const&  ,
                                   unsigned int uiPacketSize,
                                   unsigned char ucNumPackets,
                                   StreamPacket* pPackets)
{
    mPacketSize = uiPacketSize;

    mNumPackets = ucNumPackets;
    mPackets = pPackets;

    if (mNumPackets != 0)
    {
        unsigned int uiIndex = 0;
        do
        {

            mPackets[uiIndex].mBufferUsed = 0;
            mPackets[uiIndex].mPos = 0;
            ++uiIndex;
        }
        while (uiIndex < mNumPackets);
    }

    mFlags = (unsigned char)(mFlags | flags_open);

    return true;
}

// FUNCTION: SWBF2 0x0074c1b0
SourceTransport::PlayState StreamCaptureSourceBase::Update(bool bHaveDevice)
{
    if (bHaveDevice)
    {

        StreamPacket* const pPacket = mPackets + mReadPacketIndex;

        const SourceTransport::PlayState eDesired =
            QueryDesiredPlayState();

        SourceTransport::mFlags = (unsigned short)
            (SourceTransport::mFlags
             & ~(unsigned int)SourceTransport::flags_updateTransport);

        const SourceTransport::PlayState eCurrent =
            GetCurrentPlayState();

        SourceTransport::mFlags = (unsigned short)
            (SourceTransport::mFlags
             & ~(unsigned int)SourceTransport::flags_returnCurrentPlayState);

        if (eDesired == SourceTransport::playState_invalid)
        {
            return SourceTransport::playState_invalid;
        }

        if (eCurrent == eDesired)
        {
            return SourceTransport::playState_invalid;
        }

        switch (eDesired)
        {
        case SourceTransport::playState_playing:
            SetPlayState(SourceTransport::playState_playing);
            return SourceTransport::playState_playing;

        case SourceTransport::playState_paused:
            if (eCurrent != SourceTransport::playState_paused)
            {
                SetPlayState(SourceTransport::playState_pausing);

                if (pPacket->mRefCount != 0 || pPacket->mBufferUsed != 0)
                {
                    SetPlayState(
                        SourceTransport::playState_paused);
                    return SourceTransport::playState_paused;
                }

                Pause();
            }

            SetStarved(false);
            return SourceTransport::playState_invalid;

        case SourceTransport::playState_stopped:

            mPlaybackPosition = 0;
            SetPlayState(SourceTransport::playState_stopped);
            return SourceTransport::playState_stopped;

        default:
            return SourceTransport::playState_invalid;
        }
    }

    {
        SourceTransport::PlayState eState =
            QueryDesiredPlayState();

        SourceTransport::mFlags = (unsigned short)
            (SourceTransport::mFlags
             & ~(unsigned int)SourceTransport::flags_updateTransport);

        if (eState == SourceTransport::playState_invalid)
        {
            eState = GetPlayState();
        }

        SetDesiredPlayState(eState);
        SetPlayState(SourceTransport::playState_stopped);
        Flush();

        SourceTransport::mFlags = (unsigned short)
            (SourceTransport::mFlags
             | (unsigned int)SourceTransport::flags_returnCurrentPlayState);
    }

    return SourceTransport::playState_invalid;
}

}
