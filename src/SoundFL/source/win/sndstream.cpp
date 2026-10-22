#include "sndstream.h"
#include "sndengine.h"
#include "SoundFL/source/sndobjectlist.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "SoundFL/source/sndsampleinfo.h"

#include <windows.h>
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

namespace Snd
{

// FUNCTION: SWBF2 0x007346d0
Stream::Stream()
    : mOpening(0)
{

    for (unsigned int i = 0; i < 8; ++i)
    {
        mPackets[i].mBuffer = mStreamBuf[i];
        mADPCMPackets[0][i].mBuffer = mADPCMPacketBuf[0][i];
        mADPCMPackets[1][i].mBuffer = mADPCMPacketBuf[1][i];
        mADPCMPackets[2][i].mBuffer = mADPCMPacketBuf[2][i];
    }

    StreamBase::mUpdateI3DL2Source = 0;
}

// FUNCTION: SWBF2 0x00734b40
void Stream::Flush(bool bThreadPaused, bool bFlushRenderers)
{
    if (!bThreadPaused)
    {
        Engine::GetThreadUpdate()->Pause(false);
    }

    const PlayState ePlayState = GetPlayState();
    mSource.Stop();
    mSource.Update();

    const Info* pInfo = 0;
    if (!mInfoList.IsEmpty())
    {
        pInfo = (const Info*)((ObjectList::Node*)PblListSingle::Itor(mInfoList).GetNode())->mObject;
    }

    for (unsigned int i = 0; i < pInfo->mNumSubstreams; ++i)
    {
        if (bFlushRenderers)
        {
            mRenderer[i].Flush();
        }
        mADPCMDecoder[i].Flush();
    }

    mSplitter.Flush();
    mSource.Flush();

    if (ePlayState == playState_playing)
    {
        mSource.Play(false);
    }
    else if (ePlayState == playState_pausing || ePlayState == playState_paused)
    {
        mSource.Pause();
    }

    if (!bThreadPaused)
    {
        Engine::GetThreadUpdate()->Resume();
    }
}

// FUNCTION: SWBF2 0x00734a00
void Stream::Close()
{
    Engine::GetThreadUpdate()->Pause(false);
    if (GetPlayState() != playState_stopped)
    {
        SourceBus::Stop(this);
        SourceTransport::Stop();
        Update(0.0f);
    }
    Flush(true, true);
    mSource.Close();
    StreamBase::Close();
    Engine::GetThreadUpdate()->Resume();
}

// FUNCTION: SWBF2 0x00734b10
void Stream::EnableStreaming(bool bEnable)
{
    smEnableStreaming = bEnable;
}

// GLOBAL: SWBF2 0x01e2978c
bool Stream::smFileOpening;

// GLOBAL: SWBF2 0x007e34b0
bool Stream::smEnableStreaming = true;

// FUNCTION: SWBF2 0x00734b20
void __cdecl Stream::FileOpenCallback(File* pFile, void* pData, bool bSuccess)
{
    *(bool*)pData = bSuccess;
    smFileOpening = false;
}

// FUNCTION: SWBF2 0x00735020
bool Stream::OpenFile(File& rFile, char const* pcFileName,
                      void (__cdecl* pfnIdle)(StreamBase*, void*), void* pIdleData)
{
    bool bSuccess;
    rFile.SetFileOpCompleteFunc(FileOpenCallback, &bSuccess);
    smFileOpening = true;
    rFile.Open(pcFileName, 1, 0x800, 0x800, 1);
    while (smFileOpening)
    {
        if (pfnIdle)
        {
            pfnIdle(this, pIdleData);
        }
        Sleep(0);
    }
    rFile.SetFileOpCompleteFunc(0, 0);
    return bSuccess;
}

// FUNCTION: SWBF2 0x007350b0
void Stream::Update(float fDeltaTime)
{
    if (!IsOpen() || mOpening || !smEnableStreaming)
    {
        return;
    }

    mSource.SetLooping(GetLooping());
    StreamBase::Update(fDeltaTime);

    const Info* pInfo = mInfoList.IsEmpty()
        ? 0 : (const Info*)((ObjectList::Node*)mInfoList.mHead.mNext)->mObject;
    const unsigned int uiNumSubstreams = pInfo->mNumSubstreams;

    if (GetGainChanged())
    {
        for (unsigned int i = 0; i < uiNumSubstreams; ++i)
        {
            mRenderer[i].SetGain(Source::mGain);
        }
    }
    if (GetReverbGainChanged())
    {
        for (unsigned int i = 0; i < uiNumSubstreams; ++i)
        {
            mRenderer[i].SetReverbGain(mReverbGain);
        }
    }
    if (GetPanChanged())
    {
        for (unsigned int i = 0; i < uiNumSubstreams; ++i)
        {
            mRenderer[i].SetPan(mPan);
        }
    }

    for (unsigned int i = 0; i < uiNumSubstreams; ++i)
    {
        if (mCurrentSegment)
        {
            mRenderer[i].mFrequency = mCurrentSegment->mFrequency;
        }
        mRenderer[i].SetPitch(mPitch);
    }

    for (unsigned int i = 0; i < uiNumSubstreams; ++i)
    {
        StreamRenderer& rRenderer = mRenderer[i];
        const bool b3D = (Source2D3D::mFlags & 1) != 0;
        if (rRenderer.mFormat != SampleInfo::format_Unknown
            && ((rRenderer.Source2D3D::mFlags & 1) != 0) != b3D
            && rRenderer.mNumChannels == 1)
        {
            rRenderer.SetFormat(rRenderer.mFormat, rRenderer.mNumChannels,
                                rRenderer.mFrequency, b3D, rRenderer.mChannelMask, 200.0f);
        }
        if (Source2D3D::mFlags & 1)
        {
            rRenderer.Source3D::CopyParameters(*this);
            rRenderer.Set3DMode(Get3DMode());
        }
    }

    if (GetPlaybackPositionChanged())
    {
        const Segment* pSegment = mCurrentSegment;
        mSource.SetSegmentOffsetLength(
            (unsigned int)pSegment->mAddress,
            uiNumSubstreams > 1 ? pSegment->mPadding + pSegment->mLength : pSegment->mLength,
            uiNumSubstreams > 1 ? pSegment->mPadding : 0);
        if (pInfo->mNumSubstreams == 1)
        {
            const unsigned int uiFrameSize =
                SampleInfo::GetFrameSize(pInfo->mFormat) * pInfo->mNumChannels;
            mSource.SourceTransport::mPlaybackPosition =
                (mPlaybackPosition / uiFrameSize) * uiFrameSize;
        }
        else
        {
            mSource.SourceTransport::mPlaybackPosition = 0;
        }
        mSource.SourceTransport::mFlags |= flags_updatePlaybackPosition;
        Flush(true, true);
    }

    switch (GetDesiredPlayState())
    {
    case playState_playing:
        mSource.SourceTransport::mFlags = (unsigned short)((mSource.SourceTransport::mFlags
            & ~(unsigned int)(flags_updateStop | flags_updatePause)) | (unsigned int)flags_updatePlay);
        mSplitter.SourceTransport::Play(true);
        for (unsigned int i = 0; i < uiNumSubstreams; ++i)
        {
            mRenderer[i].SourceTransport::Stop();
            mRenderer[i].Update();
            mADPCMDecoder[i].SourceTransport::Play(true);
            mRenderer[i].SourceTransport::Play(true);
        }
        break;

    case playState_paused:
        mSource.SourceTransport::Pause();
        mSplitter.SourceTransport::Pause();
        for (unsigned int i = 0; i < uiNumSubstreams; ++i)
        {
            mADPCMDecoder[i].SourceTransport::Pause();
            mRenderer[i].SourceTransport::Pause();
        }
        break;

    case playState_stopped:
        Flush(true, true);
        mSource.SourceTransport::Stop();
        mSplitter.SourceTransport::Stop();
        for (unsigned int i = 0; i < uiNumSubstreams; ++i)
        {
            mADPCMDecoder[i].SourceTransport::Stop();
            mRenderer[i].SourceTransport::Stop();
        }
        break;
    }

    for (unsigned int i = 0; i < uiNumSubstreams; ++i)
    {
        mRenderer[i].Update();
    }

    mPlaybackPosition = mRenderer[0].mPlaybackPosition;
    const PlayState eRendererState = mRenderer[0].GetPlayState();
    const PlayState eCurrentState = GetCurrentPlayState();
    if (eRendererState != eCurrentState)
    {
        if (eCurrentState == playState_playing && eRendererState == playState_stopped)
        {
            SourceBus::Stop(this);
        }
        SetPlayState(eRendererState);
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x00734920
bool Stream::AppendSegments(PblFileChunk& chunk, unsigned int uiHashID, const char* pcFileName,
                            void (__cdecl* pfnIdle)(StreamBase*, void*), void* pIdleData)
{
    const bool bWasClosed = !IsOpen();
    bool bOk = false;

    Engine::GetThreadUpdate()->Pause(false);
    if (StreamBase::AppendSegments(chunk, uiHashID, pcFileName, 0, 0, false))
    {
        chunk._pFile->Close();
        if (bWasClosed)
        {
            File file;
            bOk = OpenFile(file, pcFileName, pfnIdle, pIdleData);
            if (bOk)
            {
                Open(file);
            }
        }
        else
        {
            bOk = true;
        }
    }
    Engine::GetThreadUpdate()->Resume();
    return bOk;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00734a90
void Stream::Connect(StreamNodeSink** ppSinks)
{
    const Info* pInfo = mInfoList.IsEmpty()
        ? 0 : (const Info*)((ObjectList::Node*)mInfoList.mHead.mNext)->mObject;
    const unsigned int uiNumSubstreams = pInfo->mNumSubstreams;

    if (uiNumSubstreams == 1)
    {
        ppSinks[0]->Connect(&mSource, 0);
        return;
    }

    mSplitter.StreamNodeSink::Connect(&mSource, 0);
    for (unsigned int i = 0; i < uiNumSubstreams; ++i)
    {
        ppSinks[i]->Connect(&mSplitter, i);
    }
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00734c80
void Stream::Open(File& rFile)
{
    const Info* pInfo = mInfoList.IsEmpty()
        ? 0 : (const Info*)((ObjectList::Node*)mInfoList.mHead.mNext)->mObject;
    const Segment* pSegment = mCurrentSegment;
    const unsigned int uiNumSubstreams = pInfo->mNumSubstreams;
    SampleInfo::Format eFormat = pInfo->mFormat;

    mSource.Open(rFile,
                 uiNumSubstreams > 1 ? pInfo->mSubstreamInterleave * uiNumSubstreams : 0x8000,
                 8, mPackets, 0x800);
    mSource.SetSegmentOffsetLength((unsigned int)pSegment->mAddress, pSegment->mLength,
                                   uiNumSubstreams > 1 ? pSegment->mPadding : 0);

    StreamNodeSink* apSinks[3];
    if (eFormat >= SampleInfo::format_PCM16)
    {
        if (eFormat <= SampleInfo::format_PCM8)
        {
            for (unsigned int i = 0; i < uiNumSubstreams; ++i)
            {
                mADPCMDecoder[i].StreamNodeSink::Connect(0, 0);
                mRenderer[i].StreamConsumer::Connect(0, 0);
                apSinks[i] = &mRenderer[i];
            }
        }
        else if (eFormat == SampleInfo::format_IMAADPCM)
        {
            for (unsigned int i = 0; i < uiNumSubstreams; ++i)
            {
                mADPCMDecoder[i].StreamNodeSink::Connect(0, 0);
                mRenderer[i].StreamConsumer::Connect(&mADPCMDecoder[i], 0);
                apSinks[i] = &mADPCMDecoder[i];
            }
            eFormat = SampleInfo::format_PCM16;
        }
    }
    Connect(apSinks);

    for (unsigned int i = 0; i < 24; ++i)
    {
        StreamPacket& rPacket = mSplitterPackets[i];
        if (rPacket.mRefCount > 0)
        {
            rPacket.Unlock();
        }
        rPacket.mBuffer = 0;
    }
    mSplitter.Setup(uiNumSubstreams, pInfo->mSubstreamInterleave, mSplitterPackets,
                    mSplitterReadPos, mSplitterLockPos, (24 / uiNumSubstreams) * uiNumSubstreams);

    const unsigned int uiNumChannels = pInfo->mNumChannels;
    unsigned long aulChannelMask[3] = { 0x3, 0x30, 0xC };
    for (unsigned int i = 0; i < uiNumSubstreams; ++i)
    {
        mADPCMDecoder[i].SetPackets(0x1B000, mADPCMPackets[i], 8, uiNumChannels);
        const bool b3D = (Source2D3D::mFlags & 1) != 0;
        if (uiNumChannels == 1 || uiNumSubstreams == 1)
        {
            aulChannelMask[i] = 0;
        }
        mRenderer[i].SetFormat(eFormat, uiNumChannels, pSegment->mFrequency, b3D,
                               aulChannelMask[i], 200.0f);
    }
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00734f40
bool Stream::Open(PblFileChunk& chunk, unsigned int uiSegment, char const* pcFileName,
                  void (__cdecl* pfnIdle)(StreamBase*, void*), void* pIdleData,
                  unsigned int uiArg)
{
    Engine::GetThreadUpdate()->Pause(false);
    mOpening = true;
    Engine::GetThreadUpdate()->Resume();

    bool bOk = StreamBase::Open(chunk, uiSegment, pcFileName, 0, 0, false);
    chunk._pFile->Close();
    if (bOk)
    {
        File file;
        bOk = OpenFile(file, pcFileName, pfnIdle, pIdleData);
        if (bOk)
        {
            Open(file);
        }
    }
    if (!bOk)
    {
        StreamBase::Close();
    }

    mOpening = false;
    return bOk;
}
}
