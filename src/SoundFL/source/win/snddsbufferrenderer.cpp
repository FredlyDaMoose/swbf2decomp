#include "snddsbufferrenderer.h"

#include "sndengine.h"

void __cdecl PblTraceP(const char* Format, ...);

namespace Snd
{

// GLOBAL: SWBF2 0x007e37f8
// SYNTHETIC: SWBF2 0x0040be90
// `dynamic initializer for 'Snd::DSBufferRenderer::smRendererList''
// SYNTHETIC: SWBF2 0x0076a460
// `dynamic atexit destructor for 'Snd::DSBufferRenderer::smRendererList''
PblListSingle DSBufferRenderer::smRendererList;

// FUNCTION: SWBF2 0x0073d740
DSBufferRenderer::DSBufferRenderer()
    : Source3D(0),
      mFrequency(0),
      mNumChannels(0),
      mFormat(SampleInfo::format_Unknown),
      mBufferSize(0),
      mPrevWritePos(0),
      mPrevPlayPos(0),
      mNextLockPos(0),
      mBufferedData(0),
      mLeftToPlay(0),
      mPacketRead(0),
      mFrameSize(0),
      mBufferFillSize(0),
      mFlags(8)
{

    StreamNodeSink::Connect(0, 0);

    mGainTarget[0] = 0x7FFF;
    mGainTarget[1] = 0x7FFF;
    mGainCurrent[0] = 0x7FFF;
    mGainCurrent[1] = 0x7FFF;

    Engine::GetThreadUpdate()->AddObject(this);

    Source3D::mProps = static_cast<Source3D::Properties*>(this);

    ThreadUpdate::Object::GetThreadUpdate()->Pause(false);
    smRendererList.AddLast(&mNode);
    ThreadUpdate::Object::GetThreadUpdate()->Resume();
}

// FUNCTION: SWBF2 0x0073dcb0
float DSBufferRenderer::GetBufferSizeMs() const
{

    const unsigned int uiFrameBytes = SampleInfo::GetFrameSize(mFormat) * mNumChannels;
    const float fFrames = (float)(unsigned int)mBufferSize
                        / (float)(unsigned int)uiFrameBytes;
    const float fSamples = (float)(unsigned int)SampleInfo::GetSamplesPerFrame(mFormat)
                         * fFrames;
    const float fRate = (float)(unsigned int)mFrequency
                       * static_cast<const Source*>(static_cast<const Source2D3D*>(this))->mPitch;

    return fSamples / fRate * 1000.0f;
}

// FUNCTION: SWBF2 0x0073e200
void DSBufferRenderer::SetLowTideMark(float fMilliseconds)
{
    const float fBufferMs = GetBufferSizeMs();

    mBufferFillSize = (unsigned int)(fMilliseconds / fBufferMs
                                     * (float)(unsigned int)mBufferSize);
}

// FUNCTION: SWBF2 0x0073e2a0
void DSBufferRenderer::UpdateBufferParameters(IDirectSoundBuffer* const pBuffer,
                                              bool bUnusedArg4,
                                              bool bImmediate)
{
    (void)pBuffer;
    (void)bUnusedArg4;

    Source::mFlags &= (unsigned char)~(unsigned int)Source::flags_pitchChanged;

    DSBuffer::SetFrequency(
        (int)((float)(unsigned int)mFrequency * Source::mPitch));

    if ((Source2D3D::mFlags & 1) == 0)
    {
        UpdateGain(false, 1.0f);
        return;
    }

    if (Source3D::mClosestListener == 0)
    {
        Source3D::mFlags |=
            (unsigned char)(unsigned int)Source3D::flags_updateListener;

        Source3D::DetermineClosestListener();
    }

    if (Source3D::mClosestListener->mSpace != 0)
    {

        SourceI3DL2Interpolated::Update(
            Source3D::mClosestListener,
            Source3D::mPos3D,
            Source3D::mProps->mSpaceDistance,
            SourceTransport::GetCurrentPlayState()
                == SourceTransport::playState_playing);
    }

    if (DSBuffer::GetBuffer3D() != 0)
    {
        DS3DBUFFER params;
        Engine::Calculate3DParams(this, &params);
        if (!DSBuffer::Set3DParams(params, bImmediate))
        {
            PblTraceP("DSBufferRenderer : unable to set 3D parameters\n");

        }
    }

    float fGain;
    Source3D::CalculateGain(fGain);

    if (DSBuffer::GetPropertySet() != 0 && mNumChannels <= 1)
    {
        const unsigned short usFlags = SourceI3DL2::mFlags;
        const bool bChanged =
            (usFlags & SourceI3DL2::flags_changed) != 0;

        SourceI3DL2::mFlags = (unsigned short)(usFlags & 0xFE00u);

        if (bChanged)
        {
            Engine::SetupDirectSoundEffects(this, this);
        }
    }
    else
    {

        fGain *= SourceI3DL2::CalculateGainEstimate();
    }

    UpdateGain(true, fGain);

    DSBuffer::SetFrequency(
        (unsigned int)((float)(unsigned int)mFrequency
                       * (Source3D::CalculatePitch() * Source::mPitch)));

}

// FUNCTION: SWBF2 0x0073dd80
void DSBufferRenderer::QueuePackets()
{

    typedef StreamPacket* (__thiscall* GetPacketFn)(StreamNodeSource* pThis,
                                                    unsigned int uiOutput);

    while (!mPacketQueue.IsFull())
    {
        StreamNodeSource* const pSource =
            StreamNodeSink::mConnection.mInputNode;
        if (pSource == 0)
        {
            break;
        }

        StreamPacket* const pPacket =
            pSource->GetPacket(StreamNodeSink::mConnection.mInputNodeOutputIndex);

        if (pPacket == 0)
        {
            break;
        }

        mPacketQueue.Add(pPacket);
    }
}

// FUNCTION: SWBF2 0x0073ddf0
void DSBufferRenderer::ResetGain()
{

    if ((mFlags & 0x10) != 0)
    {
        DSBuffer::ResetGainInterpolation();
        return;
    }

    int* pCurrent = mGainCurrent;
    for (unsigned int i = 0; i < mNumChannels; ++i)
    {
        *pCurrent = pCurrent[-2];
        ++pCurrent;
    }
}

static const char kFile[] = "source\\win\\snddsbufferrenderer.cpp";

// FUNCTION: SWBF2 0x0073e250
void DSBufferRenderer::StopBuffer()
{

    if (DSBuffer::GetBuffer() == 0)
    {
        return;
    }

    if (!DSBuffer::IsPlaying())
    {
        return;
    }

    DSBuffer::Stop();

    if (!Engine::Succeeded(DSBuffer::GetBuffer()->SetCurrentPosition(0),
                           kFile, 578))
    {
        PblTraceP("DSBufferRenderer : unable to reset buffer play cursor\n");

    }
}

// FUNCTION: SWBF2 0x0073e490
void DSBufferRenderer::UpdateGain(bool bEnable, float fGain)
{
    (void)bEnable;

    const float fSourceGain = Source::mGain;

    if ((mFlags & 0x10) != 0)
    {

        if ((Source2D3D::mFlags & 1) != 0)
        {
            DSBuffer::SetGain(fSourceGain * fGain);
            return;
        }

        DSBuffer::SetGain(fSourceGain);
        DSBuffer::SetPan(Source2D::mPan);
        return;
    }

    if ((Source2D3D::mFlags & 1) != 0)
    {

        const int iGain =
            (int)(fSourceGain * fGain * 32767.0f) << 16;
        int* pTarget = mGainTarget;
        for (unsigned int i = 0; i < mNumChannels; ++i)
        {
            *pTarget++ = iGain;
        }
        return;
    }

    if (mNumChannels == 2)
    {

        GainMatrix gains;
        Source2D::CalculateGain(fSourceGain, 1.0f, gains);

        mGainTarget[0] = (int)(gains.frontLeft * 32767.0f) << 16;

        mGainTarget[1] = (int)(gains.frontRight * 32767.0f) << 16;

        return;
    }

    const int iGain = (int)(fSourceGain * 32767.0f) << 16;
    int* pTarget = mGainTarget;
    for (unsigned int i = 0; i < mNumChannels; ++i)
    {
        *pTarget++ = iGain;
    }

    DSBuffer::SetPan(Source2D::mPan);
}

// FUNCTION: SWBF2 0x0073eab0
void DSBufferRenderer::UpdateParameters()
{

    for (PblListSingle::Node* pNode = smRendererList.mHead.mNext;
         pNode != &smRendererList.mHead;
         pNode = pNode->mNext)
    {

        DSBufferRenderer* const pRenderer = reinterpret_cast<DSBufferRenderer*>(reinterpret_cast<char*>(pNode) - offsetof(DSBufferRenderer, mNode));

        IDirectSoundBuffer* const pBuffer =
            pRenderer->DSBuffer::GetBuffer();
        if (pBuffer == 0)
        {
            continue;
        }

        if (pRenderer->SourceTransport::GetPlayState()
            != SourceTransport::playState_playing)
        {
            continue;
        }

        pRenderer->UpdateBufferParameters(
            pBuffer, pRenderer->StreamNode::IsStarving(), true);
    }
}

// FUNCTION: SWBF2 0x0073d950
DSBufferRenderer::~DSBufferRenderer()
{

    ThreadUpdate::Object::mDestructing = 1;

    ThreadUpdate::Object::GetThreadUpdate()->Pause(false);

    mNode.Remove();

    StreamNode::SetPlayState(SourceTransport::playState_stopped);

    Flush();

    ThreadUpdate::Object::GetThreadUpdate()->Resume();
}

// FUNCTION: SWBF2 0x0073dad0
void DSBufferRenderer::Flush()
{
    IDirectSoundBuffer8* const pBuffer = DSBuffer::GetBuffer();
    if (pBuffer != 0)
    {
        if (DSBuffer::IsPlaying())
        {
            DSBuffer::Stop();

            if (!Engine::Succeeded(pBuffer->SetCurrentPosition(0), kFile, 578))
            {
                PblTraceP("DSBufferRenderer : unable to reset buffer"
                          " play cursor\n");
            }
        }

        void* pAudio1;
        unsigned long ulBytes1;
        void* pAudio2;
        unsigned long ulBytes2;
        if (Engine::Succeeded(pBuffer->Lock(0, mBufferSize,
                                            &pAudio1, &ulBytes1,
                                            &pAudio2, &ulBytes2, 0),
                              kFile, 85))
        {
            if (pAudio1 != 0)
            {
                memset(pAudio1, 0, ulBytes1);
            }
            if (pAudio2 != 0)
            {
                memset(pAudio2, 0, ulBytes2);
            }

            pBuffer->Unlock(pAudio1, ulBytes1, pAudio2, ulBytes2);

        }
    }

    while (!mPacketQueue.IsEmpty())
    {
        StreamPacket* pPacket;
        mPacketQueue.Get(pPacket);
        pPacket->Unlock();
    }

    mFlags = (unsigned char)(mFlags & 8u);

    mPrevWritePos = 0;
    mPrevPlayPos = 0;
    mNextLockPos = 0;
    mBufferedData = 0;
    mLeftToPlay = 0;
    mPacketRead = 0;

    switch (StreamNode::GetPlayState())
    {
    case SourceTransport::playState_playing:
        StreamNode::Play(true);
        break;

    case SourceTransport::playState_paused:
        StreamNode::Pause();
        break;

    default:
        break;
    }
}

const StreamPacket* DSBufferRenderer::PeekPacket() const
{
    if (mPacketQueue.IsEmpty())
    {
        return 0;
    }

    StreamPacket* const* const ppPacket = mPacketQueue.Peek(0);

    if (ppPacket == 0)
    {
        return 0;
    }

    return *ppPacket;
}

static int ClampSample(int iSample)
{
    if (iSample > 32767)
    {
        return 32767;
    }
    if (iSample < -32768)
    {
        return -32768;
    }
    return iSample;
}

// FUNCTION: SWBF2 0x0073eb00
unsigned int DSBufferRenderer::WriteData(void* pData, unsigned int uiSize,
                                         SourceTransport::PlayState ePlayState)
{
    unsigned int uiWritten = 0;

    for (;;)
    {
        const StreamPacket* pPacket = PeekPacket();
        if (pPacket == 0)
        {

            QueuePackets();
            pPacket = PeekPacket();
            if (pPacket == 0)
            {
                break;
            }
        }
        else
        {
            const unsigned char* const pSrc =
                (const unsigned char*)pPacket->mBuffer + mPacketRead;

            const unsigned int uiUsed = pPacket->mBufferUsed;

            unsigned int uiChunk = uiUsed - mPacketRead;
            if (uiChunk > uiSize)
            {
                uiChunk = uiSize;
            }

            StreamNode::SetStarved(false);

            if ((mFlags & 0x10) != 0)
            {

                memcpy((unsigned char*)pData + uiWritten, pSrc, uiChunk);
            }
            else
            {

                short* const pOut =
                    (short*)((unsigned char*)pData + uiWritten);
                const short* const pIn = (const short*)pSrc;
                const unsigned int uiSamples = uiChunk >> 1;

                if (mNumChannels == 1)
                {

                    mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;

                    for (unsigned int i = 0; i < uiSamples; ++i)
                    {
                        pOut[i] = (short)ClampSample(
                            (pIn[i] * (mGainCurrent[0] >> 16)) >> 16);
                        mGainCurrent[0] +=
                            (mGainTarget[0] - mGainCurrent[0]) >> 1;
                    }
                }
                else if (mNumChannels == 2)
                {

                    mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;
                    mGainCurrent[1] += (mGainTarget[1] - mGainCurrent[1]) >> 1;

                    for (unsigned int i = 0; i < uiSamples; i += 2)
                    {
                        pOut[i] = (short)ClampSample(
                            (pIn[i] * (mGainCurrent[0] >> 16)) >> 16);
                        mGainCurrent[0] +=
                            (mGainTarget[0] - mGainCurrent[0]) >> 1;

                        pOut[i + 1] = (short)ClampSample(
                            (pIn[i + 1] * (mGainCurrent[1] >> 16)) >> 16);
                        mGainCurrent[1] +=
                            (mGainTarget[1] - mGainCurrent[1]) >> 1;
                    }
                }
                else
                {

                    for (unsigned int ch = 0; ch < mNumChannels; ++ch)
                    {
                        for (unsigned int i = ch; i < uiSamples;
                             i += mNumChannels)
                        {
                            pOut[i] = (short)ClampSample(
                                (pIn[i] * (mGainCurrent[ch] >> 16)) >> 16);
                            mGainCurrent[ch] +=
                                (mGainTarget[ch] - mGainCurrent[ch]) >> 1;
                        }
                    }
                }
            }

            uiWritten += uiChunk;
            mPacketRead += uiChunk;
            uiSize -= uiChunk;
            mBufferedData += uiChunk;
            mLeftToPlay += uiChunk;

            if (mPacketRead == uiUsed)
            {

                const unsigned int uiPacketFlags = pPacket->mFlags;
                mFlags = (unsigned char)((mFlags & 0xFCu)
                                         | ((uiPacketFlags & 1u) << 1));
                mFlags = (unsigned char)(mFlags
                                         | ((uiPacketFlags >> 1) & 1u));

                StreamPacket* pDone;
                mPacketQueue.Get(pDone);

                SourceTransport::mPlaybackPosition = pDone->mPos;

                pDone->Unlock();
                mPacketRead = 0;
            }
        }

        if (uiSize == 0)
        {
            return uiWritten;
        }
    }

    const unsigned char ucLastPacketFlags = (unsigned char)(mFlags & 3u);

    if (ePlayState != SourceTransport::playState_playing)
    {
        return uiWritten;
    }

    if (mBufferedData != 0 && ucLastPacketFlags == 0)
    {
        return uiWritten;
    }

    if (mLeftToPlay == 0 && (mFlags & 8) != 0)
    {
        if (ucLastPacketFlags == 0)
        {

            StreamNode::SetStarved((mFlags & 4) == 0);
            return uiWritten;
        }

        mFlags = (unsigned char)(mFlags | 4u);

        if ((mFlags & 2) != 0)
        {
            StreamNode::Stop();
        }

        mFlags = (unsigned char)(mFlags & 0xFCu);
        return uiWritten;
    }

    memset((unsigned char*)pData + uiWritten, 0, uiSize);
    return uiWritten + uiSize;
}

// FUNCTION: SWBF2 0x0053c660
void DSBufferRenderer::Play(bool bRestart)
{
    StreamNode::Play(bRestart);
}

// FUNCTION: SWBF2 0x0073e630
void DSBufferRenderer::UpdateObject()
{

    IDirectSoundBuffer8* const pBuffer = DSBuffer::GetBuffer();

    const SourceTransport::PlayState eDesired =
        StreamNode::QueryDesiredPlayState();

    StreamNode::SourceTransport::mFlags =
        (unsigned short)(StreamNode::SourceTransport::mFlags
                         & ~(unsigned int)SourceTransport::flags_updateTransport);

    switch (eDesired)
    {
    case SourceTransport::playState_playing:

        if (DSBuffer::GetBuffer() != 0
            && (mBufferedData >= mBufferSize / 2 || (mFlags & 3) != 0))
        {

            const bool bWasStopped = !DSBuffer::IsPlaying();

            if ((Source2D3D::mFlags & 1) != 0)
            {

                SourceI3DL2::mFlags =
                    (unsigned short)(SourceI3DL2::mFlags
                                     | (unsigned int)
                                           SourceI3DL2::flags_forceUpdate);

                SourceI3DL2Interpolated::Update(Source3D::mClosestListener,
                                                Source3D::mPos3D,
                                                Source3D::mProps->mSpaceDistance,
                                                false);
            }

            Engine::SetupDirectSoundEffects(this, this);

            UpdateBufferParameters(pBuffer, false, false);

            ResetGain();
            DSBuffer::ResetFrequencyInterpolation();

            if (pBuffer != 0 && bWasStopped)
            {

                if (!DSBuffer::Play(true, 0xFFFFFFFFu, true, true))
                {
                    Play(true);
                }
            }

            StreamNode::SetPlayState(SourceTransport::playState_playing);

            mFlags = (unsigned char)(mFlags & 0xFBu);
        }
        else
        {

            StreamNode::Play(true);
        }
        break;

    case SourceTransport::playState_stopped:
        SourceTransport::mPlaybackPosition = 0;
        StreamNode::SetPlayState(SourceTransport::playState_stopped);

        Flush();

    case SourceTransport::playState_paused:
        if (DSBuffer::IsPlaying())
        {
            StopBuffer();
        }

        if (eDesired == SourceTransport::playState_stopped
            || (DSBuffer::GetBuffer() != 0
                && (mBufferedData >= mBufferSize || (mFlags & 3) != 0)))
        {
            StreamNode::SetStarved(false);
            StreamNode::SetPlayState(eDesired);
        }
        else
        {
            StreamNode::Pause();
            StreamNode::SetPlayState(SourceTransport::playState_pausing);

        }
        break;

    default:
        break;
    }

    const SourceTransport::PlayState eCurrent =
        StreamNode::GetCurrentPlayState();

    if (pBuffer == 0)
    {
        return;
    }

    if (eCurrent != SourceTransport::playState_playing
        && eCurrent != SourceTransport::playState_pausing
        && eDesired != SourceTransport::playState_playing)
    {
        return;
    }

    unsigned long ulPlayPos;
    unsigned long ulWritePos;
    if (!Engine::Succeeded(pBuffer->GetCurrentPosition(&ulPlayPos, &ulWritePos),
                           kFile, 268))
    {
        return;
    }

    const unsigned int uiWriteAdvance =
        (ulWritePos >= mPrevWritePos)
            ? (ulWritePos - mPrevWritePos)
            : (ulWritePos + (mBufferSize - mPrevWritePos));

    if (DSBuffer::IsPlaying() && uiWriteAdvance < mBufferFillSize)
    {
        return;
    }

    QueuePackets();

    const unsigned int uiBuffered = (mBufferedData > uiWriteAdvance)
                                        ? (mBufferedData - uiWriteAdvance)
                                        : 0u;
    mBufferedData = uiBuffered;
    mPrevWritePos = ulWritePos;

    const unsigned int uiPlayAdvance =
        (ulPlayPos >= mPrevPlayPos)
            ? (ulPlayPos - mPrevPlayPos)
            : (ulPlayPos + (mBufferSize - mPrevPlayPos));
    mLeftToPlay = (mLeftToPlay > uiPlayAdvance) ? (mLeftToPlay - uiPlayAdvance)
                                                : 0u;
    mPrevPlayPos = ulPlayPos;

    if (ulWritePos >= ulPlayPos)
    {
        if (mNextLockPos > ulPlayPos && mNextLockPos < ulWritePos)
        {
            mNextLockPos = ulWritePos;
        }
    }
    else
    {
        if (mNextLockPos >= ulPlayPos && mNextLockPos <= ulWritePos)
        {
            mNextLockPos = ulWritePos;
        }
    }

    unsigned int uiLockSize = mBufferSize - uiBuffered;
    const bool bLockBehindPlay =
        DSBuffer::IsPlaying() ? (ulPlayPos >= mNextLockPos)
                              : (ulPlayPos > mNextLockPos);
    const unsigned int uiToPlayCursor =
        bLockBehindPlay ? (ulPlayPos - mNextLockPos)
                        : (ulPlayPos + (mBufferSize - mNextLockPos));

    if (uiLockSize > uiToPlayCursor)
    {
        uiLockSize = uiToPlayCursor;
    }

    if (uiLockSize != 0)
    {

        void* pAudio1;
        unsigned long ulBytes1;
        void* pAudio2;
        unsigned long ulBytes2;
        unsigned int uiWritten = 0;
        if (!Engine::Succeeded(pBuffer->Lock(mNextLockPos, uiLockSize,
                                             &pAudio1, &ulBytes1,
                                             &pAudio2, &ulBytes2, 0),
                               kFile, 361))
        {
            return;
        }

        if (pAudio1 != 0)
        {
            uiWritten = WriteData(pAudio1, ulBytes1, eCurrent);
        }

        if (pAudio2 != 0)
        {
            uiWritten += WriteData(pAudio2, ulBytes2, eCurrent);
        }

        mNextLockPos += uiWritten;
        if (mNextLockPos >= mBufferSize)
        {
            mNextLockPos -= mBufferSize;
        }

        pBuffer->Unlock(pAudio1, ulBytes1, pAudio2, ulBytes2);

    }

    const bool bTerminated = DSBuffer::IsTerminated();

    if (DSBuffer::IsPlaying() && (mFlags & 8) != 0)
    {

        const bool bStarving = StreamNode::IsStarving();
        StreamNode::SetStarved(bStarving);

        if (!bStarving && (mFlags & 4) == 0)
        {
            return;
        }

        if (StreamNode::QueryDesiredPlayState()
            == SourceTransport::playState_stopped)
        {
            return;
        }
    }
    else if (!bTerminated)
    {
        return;
    }

    StreamNode::Stop();
    UpdateObject();
    Play(false);
}

// FUNCTION: SWBF2 0x0073de90
bool DSBufferRenderer::SetFormat(SampleInfo::Format eFormat,
                                 unsigned int uiNumChannels,
                                 unsigned int uiFrequency, bool b3D,
                                 float fBufferMs, bool bRestart,
                                 IDirectSound* pDirectSound,
                                 unsigned long ulFlags, bool bPrimary)
{

    const bool b3DMono = b3D && uiNumChannels == 1;

    float fMs = fBufferMs;

    if (Engine::GetDSCaps().dwMaxHwMixingAllBuffers == 0
        && 500.0f > fBufferMs)
    {
        fMs = 500.0f;
    }

    if (Engine::GetMinStreamBufferSizeMs() > fMs)
    {
        fMs = Engine::GetMinStreamBufferSizeMs();
    }

    Flush();
    DSBuffer::Destroy();

    mPrevWritePos = 0;
    mNextLockPos = 0;

    if (eFormat != SampleInfo::format_Unknown
        && uiNumChannels != 0
        && uiFrequency != 0)
    {

        const float fFrames =
            (float)(uiFrequency / SampleInfo::GetSamplesPerFrame(eFormat))
            * (fMs / 1000.0f);

        unsigned int uiLength =
            (unsigned int)((float)SampleInfo::GetFrameSize(eFormat) * fFrames
                           * (float)uiNumChannels);

        uiLength =
            (uiLength + SampleInfo::GetFrameSize(eFormat) * uiNumChannels - 1)
            / (SampleInfo::GetFrameSize(eFormat) * uiNumChannels)
            * (SampleInfo::GetFrameSize(eFormat) * uiNumChannels);

        const unsigned int uiLocation =
            (Engine::GetMixConfig() == Engine::mixConfig_DirectSoundHardware
             && ((!bPrimary && b3DMono) || ulFlags != 0))
                ? DSBCAPS_LOCHARDWARE
                : DSBCAPS_LOCSOFTWARE;

        const bool bProperties = Engine::smHardwareEffectsAvailable && b3DMono;

        const unsigned int uiBufferFlags =
            (unsigned int)(b3DMono ? DSBCAPS_CTRL3D : DSBCAPS_CTRLPAN)
            | uiLocation
            | (unsigned int)(DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME
                             | DSBCAPS_CTRLFREQUENCY);

        if (DSBuffer::Create(eFormat, uiNumChannels, uiFrequency, uiLength,
                             b3DMono, bProperties, uiBufferFlags, false, true,
                             pDirectSound, ulFlags, true))
        {

            if (DSBuffer::GetFormat(mFormat, mNumChannels,
                                    mFrequency))
            {

                DSBCAPS caps;
                caps.dwSize = sizeof(DSBCAPS);

                if (Engine::Succeeded(DSBuffer::GetBuffer()->GetCaps(&caps),
                                      kFile, 884))
                {
                    mBufferSize = caps.dwBufferBytes;
                    mFrameSize = SampleInfo::GetFrameSize(mFormat)
                                 * mNumChannels;

                    unsigned int uiFillSize =
                        (unsigned int)((float)uiFrequency * 0.005f
                                       * (float)mFrameSize);
                    if (uiFillSize > mBufferSize / 3)
                    {
                        uiFillSize = 0;
                    }
                    mBufferFillSize = uiFillSize;

                    Source2D3D::Set3DMode(b3D);

                    Flush();

                    if ((mFlags & 0x10) == 0)
                    {
                        DSBuffer::ResetGainInterpolation();
                        DSBuffer::SetGain(1.0f);
                    }

                    return true;
                }
            }
        }
    }

    mFrequency = 0;
    mNumChannels = 0;
    mFormat = SampleInfo::format_Unknown;
    mBufferSize = 0;
    return false;
}

// SYNTHETIC: SWBF2 0x0073da40
// Snd::DSBufferRenderer::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0073de40
void DSBufferRenderer::Set3DMode(bool b3D, unsigned long ulFlags)
{
    SetFormat(mFormat, mNumChannels, mFrequency, b3D, GetBufferSizeMs(),
              ulFlags != 0, 0, 0, false);
}

}
