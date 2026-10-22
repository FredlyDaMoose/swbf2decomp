#include <windows.h>

#include "sndstreamsource.h"
#include "sndglobals.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00749850
StreamSource::StreamSource()
    : mFlags((unsigned char)flags_MediaSecBase2),
      mPacketReadPos(0),
      mPacketLockPos(0),
      mNumPackets(0),
      mPacketSize(0),
      mPackets(0),
      mSeekPos(0),
      mReadSize(0),
      mMediaSectorSize(2048)
{

    mFile.SetFileOpCompleteFunc(&StreamSource::FileOpComplete, this);
}

// FUNCTION: SWBF2 0x00749b30
void __cdecl StreamSource::FileOpComplete(File* pFile, void* pCallbackData,
                                          bool bSuccess)
{
    StreamSource* const pThis = (StreamSource*)pCallbackData;

    pThis->mLock.Wait();

    StreamPacket* const pPacket = &pThis->mPackets[pThis->mPacketReadPos];

    if (bSuccess && (pThis->mFlags & flags_Reading) != 0)
    {

        pPacket->mBufferUsed =
            pThis->mReadSize - (unsigned int)(unsigned long)pPacket->mCallbackData;
        pPacket->mCallback = 0;
        pPacket->mCallbackData = 0;

        if ((pThis->mFlags & flags_SeekPending) != 0)
        {
            pThis->mFlags &= (unsigned char)~flags_SeekPending;
            pThis->mFlags |= (unsigned char)flags_SeekReady;

            pPacket->mFlags |= 2;
        }

        ++pThis->mPacketReadPos;
        if (pThis->mPacketReadPos == pThis->mNumPackets)
        {
            pThis->mPacketReadPos = 0;
        }
    }

    pThis->mFlags &= (unsigned char)~flags_Reading;

    if (!bSuccess)
    {
        pThis->mFile.Seek(pThis->mSegment.mOffset + pPacket->mPos);
    }

    pThis->mLock.Signal();
}

// SYNTHETIC: SWBF2 0x007499e0
// Snd::StreamSource::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00749a80
void StreamSource::Close()
{
    SetPlayState(playState_stopped);
    Flush();
    mFile.Close();
    mLock.Destroy();

    mNumPackets = 0;
    mPacketSize = 0;
    mPackets = 0;
    mPacketReadPos = 0;
    mPacketLockPos = 0;
    mFlags = 0;
    mReadSize = 0;

    mSegment.mLength = 0;
    mSegment.mOffset = 0;
    mSegment.mPadding = 0;
    mSeekSegment.mLength = 0;
    mSeekSegment.mOffset = 0;
    mSeekSegment.mPadding = 0;
    mNextSegment.mLength = 0;
    mNextSegment.mOffset = 0;
    mNextSegment.mPadding = 0;

    mSeekPos = 0;
}

// FUNCTION: SWBF2 0x00749c10
void StreamSource::Flush()
{
    typedef StreamPacket* (__thiscall* GetPacketFn)(StreamSource* pThis,
                                                    unsigned int uiOutput);

    mLock.Wait();
    while ((mFlags & flags_Reading) != 0)
    {
        mLock.Signal();
        Sleep(0);
        mLock.Wait();
    }
    mLock.Signal();

    StartFlush();

    StreamPacket* pPacket;
    while ((pPacket = GetPacket(0)) != 0)
    {
        pPacket->Unlock();
    }

    if ((mFlags & flags_LastPacket) != 0
        && GetCurrentPlayState() != playState_stopped)
    {
        mFlags &= (unsigned char)~flags_LastPacket;
        Stop();
        Update();
    }

    if ((mFlags & flags_SeekPending) != 0)
    {
        mFlags &= (unsigned char)~flags_SeekPending;
        mFlags |= (unsigned char)flags_SeekReady;
    }

    EndFlush();
}

// FUNCTION: SWBF2 0x00749d20
StreamPacket* StreamSource::GetPacket(unsigned int uiOutput)
{
    if (!mFile.IsOpen())
    {
        return 0;
    }

    mLock.Wait();

    if (GetPlayState() == playState_stopped && !IsFlushing())
    {
        mLock.Signal();
        return 0;
    }

    StreamPacket* const pPacket = &mPackets[mPacketLockPos];

    if (pPacket->mBufferUsed == 0 || pPacket->mRefCount > 0)
    {
        SetStarved(true);
        mLock.Signal();
        if (!IsFlushing())
        {
            Update();
        }
        return 0;
    }

    SetStarved(false);
    if (pPacket->GetLast())
    {
        Stop();
    }

    if (++mPacketLockPos == mNumPackets)
    {
        mPacketLockPos = 0;
    }

    pPacket->Lock();
    mLock.Signal();
    return pPacket;
}

// FUNCTION: SWBF2 0x00749f10
void StreamSource::SeekSegment(unsigned int uiPos)
{
    if (uiPos > mSeekSegment.mLength)
    {
        uiPos = mSeekSegment.mLength;
    }

    unsigned int uiSeekPos;
    if ((mFlags & flags_MediaSecBase2) != 0)
    {
        uiSeekPos = (uiPos + mMediaSectorSize - 1) & ~(mMediaSectorSize - 1);
    }
    else
    {
        uiSeekPos = (uiPos + mMediaSectorSize - 1) / mMediaSectorSize
                    * mMediaSectorSize;
    }

    mLock.Wait();
    if ((mFlags & flags_SegmentSetup) != 0
        && GetCurrentPlayState() != playState_stopped)
    {
        mFlags |= (unsigned char)flags_SeekPending;
    }
    else
    {
        mFlags |= (unsigned char)flags_SeekReady;
    }
    mSeekPos = uiSeekPos;
    mLock.Signal();
}

// FUNCTION: SWBF2 0x00749ff0
void StreamSource::SetSegmentOffsetLength(unsigned int uiOffset,
                                          unsigned int uiLength,
                                          unsigned int uiPadding)
{
    mLock.Wait();
    mSeekSegment.mLength = uiLength;
    mSeekSegment.mOffset = uiOffset;
    mSeekSegment.mPadding = uiPadding;
    mLock.Signal();
    SeekSegment(0);
}

// FUNCTION: SWBF2 0x0074a030
void StreamSource::Update()
{
    for (;;)
    {
        const PlayState eDesired = QueryDesiredPlayState();
        SourceTransport::mFlags = (unsigned short)
            (SourceTransport::mFlags & ~(unsigned int)flags_updateTransport);
        const PlayState eCurrent = GetCurrentPlayState();

        const bool bSeek =
            (SourceTransport::mFlags & flags_updatePlaybackPosition) != 0;
        SourceTransport::mFlags = (unsigned short)
            (SourceTransport::mFlags & ~(unsigned int)flags_updatePlaybackPosition);
        if (bSeek)
        {
            SeekSegment(mPlaybackPosition);
        }

        mLock.Wait();

        if (eDesired != playState_invalid && eDesired != eCurrent)
        {
            switch (eDesired)
            {
            case playState_playing:
                if ((mFlags & (flags_SegmentSetup | flags_SeekPending
                               | flags_SeekReady)) != 0)
                {
                    SetPlayState(playState_playing);
                }
                break;

            case playState_paused:
                if ((mFlags & (flags_SegmentSetup | flags_SeekPending
                               | flags_SeekReady)) != 0)
                {
                    SetStarved(false);
                    if ((mFlags & flags_Reading) != 0
                        || mPackets[mPacketReadPos].mBufferUsed != 0)
                    {
                        SetPlayState(playState_paused);
                    }
                    else
                    {
                        Pause();
                        SetPlayState(playState_pausing);
                    }
                }
                break;

            case playState_stopped:
                mFlags &= (unsigned char)~flags_LastPacket;
                SetStarved(false);
                SetPlayState(playState_stopped);
                if (!IsFlushing())
                {
                    mLock.Signal();
                    Flush();
                    mLock.Wait();
                }
                break;
            }
        }

        switch (GetPlayState())
        {
        case playState_playing:
        case playState_paused:
            break;
        case playState_stopped:
            mLock.Signal();
            return;
        default:
            return;
        }

        StreamPacket* const pPacket = &mPackets[mPacketReadPos];
        if ((mFlags & flags_Reading) != 0 || pPacket->mBufferUsed != 0
            || pPacket->mRefCount > 0)
        {
            mLock.Signal();
            return;
        }

        unsigned int uiReadSize = mPacketSize;

        if ((mFlags & (flags_LastPacket | flags_SeekReady)) != 0)
        {
            if ((mFlags & flags_SeekReady) != 0)
            {
                mSegment = mSeekSegment;
                mFlags &= (unsigned char)~flags_SeekReady;
                mFile.Seek(mSegment.mOffset + mSeekPos);
                mFlags |= (unsigned char)flags_SegmentSetup;
            }
            else
            {
                if ((mFlags & flags_Looping) == 0)
                {
                    mLock.Signal();
                    return;
                }
                if ((mFlags & flags_SegmentQueued) != 0)
                {
                    mFlags &= (unsigned char)~flags_SegmentQueued;
                    SetSegmentOffsetLength(mNextSegment.mOffset,
                                           mNextSegment.mLength,
                                           mNextSegment.mPadding);
                    mLock.Signal();
                    SeekSegment(0);
                    continue;
                }
                mFile.Seek(mSegment.mOffset);
                mFlags &= (unsigned char)~flags_LastPacket;
            }
        }

        pPacket->mFlags &= (unsigned short)~1;

        if (mFile.GetPos() + uiReadSize >= mSegment.mOffset + mSegment.mLength)
        {
            if (mFile.GetPos() >= mSegment.mOffset + mSegment.mLength)
            {
                uiReadSize = 0;
            }
            else
            {
                pPacket->SetDiscontinuity(!GetLooping());
                pPacket->mFlags |= 2;
                if ((mFlags & (flags_Looping | flags_SegmentQueued)) == 0)
                {
                    pPacket->mFlags |= 1;
                }
                mFlags |= (unsigned char)flags_LastPacket;
                pPacket->mCallback = 0;
                pPacket->mCallbackData = (void*)(unsigned long)mSegment.mPadding;
                uiReadSize = mSegment.mOffset + mSegment.mLength - mFile.GetPos();
            }
        }

        unsigned int uiRequest = 0;
        if (uiReadSize != 0)
        {
            const unsigned int uiPos = mFile.GetPos() - mSegment.mOffset;
            pPacket->mPos = uiPos;
            mPlaybackPosition = uiPos;
            mReadSize = uiReadSize;
            mFlags |= (unsigned char)flags_Reading;

            if ((mFlags & flags_MediaSecBase2) != 0)
            {
                uiRequest = (uiReadSize + mMediaSectorSize - 1)
                            & ~(mMediaSectorSize - 1);
            }
            else
            {
                uiRequest = (uiReadSize + mMediaSectorSize - 1)
                            / mMediaSectorSize * mMediaSectorSize;
            }
        }

        mLock.Signal();
        if (uiRequest != 0)
        {
            mFile.Read(pPacket->mBuffer, uiRequest);
        }
        return;
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x00749950
StreamSource::~StreamSource()
{
    if (mFile.IsOpen())
    {
        Flush();
        Close();
    }

}
}

namespace Snd {

// FUNCTION: SWBF2 0x00749e20
bool StreamSource::Open(File& rFile, unsigned int uiPacketSize, unsigned char uiNumPackets,
                        StreamPacket* pPackets, unsigned int uiMediaSectorSize)
{
    mMediaSectorSize = uiMediaSectorSize;
    if (IsBase2(uiMediaSectorSize))
    {
        mFlags |= flags_MediaSecBase2;
    }
    else
    {
        mFlags &= ~flags_MediaSecBase2;
    }

    for (unsigned int i = 0; i < uiNumPackets; ++i)
    {
        while (pPackets[i].mRefCount > 0)
        {
            pPackets[i].Unlock();
        }
    }

    if (!mLock.Create(1, 1))
    {
        return false;
    }

    mFile = rFile;
    mFile.SetFileOpCompleteFunc(FileOpComplete, this);
    mNumPackets = uiNumPackets;
    mPacketSize = uiPacketSize;
    mPackets = pPackets;
    return true;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00749fb0
void StreamSource::SetLooping(bool bLooping)
{
    mLock.Wait();
    if (bLooping)
    {
        mFlags |= flags_Looping;
    }
    else
    {
        mFlags &= (unsigned char)~(unsigned int)flags_Looping;
    }
    mLock.Signal();
}
}
