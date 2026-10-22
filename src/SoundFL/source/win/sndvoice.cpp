#include "SoundFL/source/sndstreamnode.h"

#include "sndvoice.h"

#include "SoundFL/source/sndglobals.h"
#include "SoundFL/source/sndstreamformat.h"
#include "sndengine.h"
#include "sndsamplebank.h"
#include "sndsoftoutput.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00745760
Voice::SourceResampler::SourceResampler(bool bAllocateBuffer)
{

    mResamplerBuffer = 0;
    if (bAllocateBuffer)
    {
        mResamplerBuffer = AllocPerm(0x200);
        if (mResamplerBuffer)
        {
            mResampler.SetBuffer(mResamplerBuffer, 0x200);

            mResampler.mStepSize = 0x10000;

            mResampler.StreamNodeSink::Connect(&mSource, 0);
        }
    }
}

// FUNCTION: SWBF2 0x00745820
Voice::Voice()
    : mSourceResampler(true),
      mSoftConnIdx(0xFFFFFFFF),
      mSample(0),
      mStarting(false),
      mSetupPackets(true),

      mSourceResamplerLFE(Engine::GetMixConfig()
                          == Engine::mixConfig_DirectSoundHardware)
{

}

// FUNCTION: SWBF2 0x00745ae0
void Voice::Initialize()
{

    SoftOutput* const pOutput = Engine::GetSoftOutput();
    if (pOutput != 0)
    {
        mSoftConnIdx = pOutput->GetUnconnectedInput();
        if (mSoftConnIdx != 0xFFFFFFFF)
        {
            pOutput->Connect(&mSourceResampler.mResampler, 0, mSoftConnIdx);
        }
    }

    if ((VoiceBase::mFlags & 1) != 0)
    {
        return;
    }

    if (Engine::GetMixConfig() != Engine::mixConfig_DirectSoundSoftware
        && Engine::GetMixConfig() != Engine::mixConfig_DirectSoundHardware)
    {
        return;
    }

    if (mRenderer.SetFormat(SampleInfo::format_PCM16, 1,
                            Engine::GetOutputFrequency(), true, 0,
                            Engine::GetMinStreamBufferSizeMs()))
    {

        mRenderer.StreamNodeSink::Connect(&mSourceResampler.mResampler, 0);
        return;
    }

    if ((VoiceBase::mFlags & 1) == 0)
    {
        Lock(true);
    }
}

// FUNCTION: SWBF2 0x00745bd0
void Voice::SourceResampler::Play()
{
    mSource.SourceTransport::Play(true);
    mResampler.SourceTransport::Play(true);
    Update();
}

// FUNCTION: SWBF2 0x00745c30
void Voice::SourceResampler::Setup(const Sample* const pSample, bool bSetupPackets)
{
    if (pSample)
    {
        StreamFormat format((SampleInfo::Format)pSample->mFormat, pSample->mFrequency, 1);
        mSource.Setup(format, pSample->mAddress, pSample->mLength, 0x200, mPackets, 1,
                      bSetupPackets);
    }
    else
    {
        StreamFormat format;
        mSource.Setup(format, 0, 0, 0x200, mPackets, 1, bSetupPackets);
    }
}

// FUNCTION: SWBF2 0x00745d40
void Voice::Update3DMode()
{
    switch (Engine::GetMixConfig())
    {
    case Engine::mixConfig_DirectSoundHardware:
    {
        SoftOutput* pOutput = Engine::GetSoftOutput();
        if (Engine::GetBassManagement() && Get3DMode())
        {
            pOutput->Connect(&mSourceResamplerLFE.mResampler, 0, mSoftConnIdx);
        }
        else
        {
            pOutput->Connect(0, 0, mSoftConnIdx);
        }
    }

    case Engine::mixConfig_DirectSoundSoftware:
    {
        const bool b3D = Get3DMode();
        if (mRenderer.mFormat != SampleInfo::format_Unknown
            && ((mRenderer.Source2D3D::mFlags & 1) != 0) != b3D
            && mRenderer.mNumChannels == 1)
        {
            mRenderer.SetFormat(mRenderer.mFormat, 1, mRenderer.mFrequency, b3D,
                                mRenderer.mChannelMask, 200.0f);
        }
        break;
    }
    default:
        break;
    }
}

// FUNCTION: SWBF2 0x00745df0
void Voice::Update()
{
    const Engine::MixConfig eMixConfig = Engine::GetMixConfig();

    if (QueryDesiredPlayState() == playState_playing)
    {
        switch (Engine::GetMixConfig())
        {
        case Engine::mixConfig_DirectSoundHardware:
            mSourceResamplerLFE.Stop();

        case Engine::mixConfig_DirectSoundSoftware:
            if (!mStarting)
            {
                mRenderer.SourceTransport::Stop();
            }

        case Engine::mixConfig_Software:
            mSourceResampler.Stop();
            break;
        default:
            break;
        }
        SetPlayState(playState_stopped);
        mPlaybackPosition = 0;
    }

    mSourceResampler.mSource.SetLooping(GetLooping());
    mSourceResamplerLFE.mSource.SetLooping(GetLooping());

    if (Get3DModeChanged())
    {
        Update3DMode();
    }

    if (GetSampleChanged())
    {
        switch (Engine::GetMixConfig())
        {
        case Engine::mixConfig_DirectSoundHardware:
            mSourceResamplerLFE.Stop();

        case Engine::mixConfig_DirectSoundSoftware:
            if (!mStarting)
            {
                mRenderer.SourceTransport::Stop();
            }

        case Engine::mixConfig_Software:
            mSourceResampler.Stop();
            break;
        default:
            break;
        }
        SetPlayState(playState_stopped);
        mPlaybackPosition = 0;
        UpdateSample();
    }

    const bool bPositionChanged = GetPlaybackPositionChanged();
    if ((SourceTransport::mFlags & flags_updateTransport) == 0)
    {
        if (bPositionChanged)
        {
            switch (eMixConfig)
            {
            case Engine::mixConfig_DirectSoundHardware:
                mSourceResamplerLFE.mSource.SourceTransport::mFlags |= flags_updatePlaybackPosition;
                mSourceResamplerLFE.mSource.SourceTransport::mPlaybackPosition = mPlaybackPosition;

            case Engine::mixConfig_Software:
            case Engine::mixConfig_DirectSoundSoftware:
                mSourceResampler.mSource.SourceTransport::mFlags |= flags_updatePlaybackPosition;
                mSourceResampler.mSource.SourceTransport::mPlaybackPosition = mPlaybackPosition;
                break;
            default:
                break;
            }
        }
        else if (GetPlayState() == playState_playing)
        {
            if (eMixConfig > Engine::mixConfig_Invalid)
            {
                if (eMixConfig <= Engine::mixConfig_DirectSoundSoftware)
                {
                    if (GetSourceI3DL2Changed() && Get3DMode())
                    {
                        Engine::GetMixConfig();
                    }

                    PlayState ePlayState;
                    mPlaybackPosition = mSourceResampler.mSource.SourceTransport::mPlaybackPosition;
                    if (eMixConfig == Engine::mixConfig_Software)
                    {
                        ePlayState = mSourceResampler.mSource.GetPlayState();
                    }
                    else
                    {
                        ePlayState = mRenderer.GetPlayState();
                    }
                    SetPlayState(ePlayState);
                    if (ePlayState == playState_stopped)
                    {
                        mPlaybackPosition = 0;
                    }
                }
                else if (eMixConfig == Engine::mixConfig_Disabled)
                {
                    SourceTransport::Stop();
                }
            }
        }
        else
        {
            mPlaybackPosition = 0;
        }
    }

    mStarting = QueryDesiredPlayState() == playState_playing;

    if (GetPlayState() == playState_playing && Get3DMode()
        && GetPlayState() == playState_playing
        && (Engine::GetMixConfig() == Engine::mixConfig_DirectSoundHardware
            || Engine::GetMixConfig() == Engine::mixConfig_DirectSoundSoftware))
    {
        mRenderer.Source3D::CopyParameters(*this);
        mRenderer.SetDopplerMode(false);
    }

    VoiceHardware::Update(this);

    if (eMixConfig == Engine::mixConfig_DirectSoundHardware
        || eMixConfig == Engine::mixConfig_DirectSoundSoftware)
    {
        mRenderer.Update();
    }
}

// FUNCTION: SWBF2 0x00746310
void __cdecl Voice::UpdatePlayState(Voice* pVoice, PlayState ePlayState)
{
    switch (ePlayState)
    {
    case playState_playing:
    {
        const Sample* pSample = pVoice->VoiceBase::mSample;
        unsigned int uiPos = pVoice->mPlaybackPosition;
        const unsigned int uiLength = pSample->mLengthSamples;
        const unsigned int uiFrameSamples =
            SampleInfo::GetSamplesPerFrame((SampleInfo::Format)pSample->mFormat);
        if (pVoice->GetCurrentPlayState() != playState_stopped)
        {
            UpdatePlayState(pVoice, playState_stopped);
        }
        pVoice->SourceTransport::mFlags &= ~(unsigned int)flags_updatePlaybackPosition;

        if (uiPos > uiLength - uiFrameSamples && uiPos < uiLength)
        {
            uiPos -= uiFrameSamples;
        }
        else if (uiPos >= pSample->mLengthSamples)
        {
            uiPos = 0;
        }

        switch (Engine::GetMixConfig())
        {
        case Engine::mixConfig_DirectSoundHardware:
        case Engine::mixConfig_DirectSoundSoftware:
            if (pVoice->mRenderer.GetPlayState() == playState_stopped)
            {
                pVoice->mRenderer.SourceTransport::Play(true);
            }
            else
            {
                pVoice->mRenderer.Flush();
            }
            pVoice->mSourceResamplerLFE.Flush();

        case Engine::mixConfig_Software:
            pVoice->mSourceResampler.Flush();
            if (Engine::GetSoftOutput())
            {
                ((Snd::SoftOutput*)(Engine::GetSoftOutput()))->Discontinuity(pVoice->mSoftConnIdx);
            }
            pVoice->mSourceResampler.mSource.SourceTransport::mPlaybackPosition = uiPos;
            pVoice->mSourceResampler.mSource.SourceTransport::mFlags |= flags_updatePlaybackPosition;
            pVoice->mSourceResampler.Play();
            if (Engine::GetBassManagement() && pVoice->Get3DMode())
            {
                pVoice->mSourceResamplerLFE.mSource.SourceTransport::mPlaybackPosition = uiPos;
                pVoice->mSourceResamplerLFE.mSource.SourceTransport::mFlags |= flags_updatePlaybackPosition;
                pVoice->mSourceResamplerLFE.Play();
            }
            pVoice->SetPlayState(playState_playing);
            break;
        default:
            break;
        }
        break;
    }

    case playState_paused:
        switch (Engine::GetMixConfig())
        {
        case Engine::mixConfig_DirectSoundHardware:
            pVoice->mSourceResamplerLFE.Pause();

        case Engine::mixConfig_DirectSoundSoftware:
            pVoice->mRenderer.SourceTransport::Pause();

        case Engine::mixConfig_Software:
            pVoice->mSourceResampler.Pause();
            break;
        default:
            break;
        }
        pVoice->SetPlayState(playState_paused);
        break;

    case playState_stopped:
        switch (Engine::GetMixConfig())
        {
        case Engine::mixConfig_DirectSoundHardware:
            pVoice->mSourceResamplerLFE.Stop();

        case Engine::mixConfig_DirectSoundSoftware:
            if (!pVoice->mStarting)
            {
                pVoice->mRenderer.SourceTransport::Stop();
            }

        case Engine::mixConfig_Software:
            pVoice->mSourceResampler.Stop();
            break;
        default:
            break;
        }
        pVoice->SetPlayState(playState_stopped);
        pVoice->mPlaybackPosition = 0;
        break;

    default:
        break;
    }
}

// FUNCTION: SWBF2 0x007465d0
void Voice::UpdateSample()
{
    const Sample* pSample = VoiceBase::mSample;
    if (!pSample)
    {
        if ((unsigned int)(Engine::GetMixConfig() - 1) <= 2)
        {
            UpdatePlayState(this, playState_stopped);
        }
        SourceTransport::Stop();
        mSample = 0;
        return;
    }

    switch (Engine::GetMixConfig())
    {
    case Engine::mixConfig_DirectSoundHardware:
        if (Engine::GetBassManagement() && Get3DMode())
        {
            mSourceResamplerLFE.ClearAll();
            mSourceResamplerLFE.Setup(pSample, mSetupPackets);
            mSourceResamplerLFE.Stop();
            mSourceResamplerLFE.Flush();
        }

    case Engine::mixConfig_DirectSoundSoftware:
        mRenderer.Flush();

    case Engine::mixConfig_Software:
        mSourceResampler.ClearAll();
        mSourceResampler.Setup(pSample, mSetupPackets);
        mSourceResampler.Stop();
        mSourceResampler.Flush();
        mSetupPackets = false;
        break;
    default:
        break;
    }
}

// FUNCTION: SWBF2 0x00745cf0
void Voice::SourceResampler::UnlockAll()
{
    while (mPackets[0].mRefCount > 0)
    {
        mPackets[0].Unlock();
    }
}

}

namespace Snd
{

// FUNCTION: SWBF2 0x00745a90
void Voice::SourceResampler::ClearAll()
{
    if (mPackets[0].mRefCount == 0)
    {
        mPackets[0].mBuffer = 0;
        mPackets[0].mBufferUsed = 0;
    }
}

// FUNCTION: SWBF2 0x00745ab0
void Voice::SourceResampler::Flush()
{
    mSource.Flush();
    mResampler.Flush();
}

// FUNCTION: SWBF2 0x00745b90
void Voice::SourceResampler::Pause()
{
    mSource.SourceTransport::Pause();
    mResampler.SourceTransport::Pause();
    Update();
}

// FUNCTION: SWBF2 0x00745cb0
void Voice::SourceResampler::Stop()
{
    mSource.SourceTransport::Stop();
    mResampler.SourceTransport::Stop();
    Update();
}
}

namespace Snd {

// FUNCTION: SWBF2 0x007458f0
Voice::SourceResampler::~SourceResampler()
{
    if (mResamplerBuffer)
    {
        FreePerm(mResamplerBuffer);
        mResamplerBuffer = 0;
    }
    mResampler.SetBuffer(0, 0);
    mResampler.StreamNodeSink::Connect(0, 0);

}
}

namespace Snd {

// FUNCTION: SWBF2 0x007459a0
Voice::~Voice()
{
    mSourceResampler.UnlockAll();
    mSourceResamplerLFE.UnlockAll();
    if (GetLocked())
    {
        Lock(false);
    }
    if (mSoftConnIdx != (unsigned int)-1)
    {
        if (Engine::GetSoftOutput())
        {
            Engine::GetSoftOutput()->Connect(0, 0, mSoftConnIdx);
        }
        mSoftConnIdx = (unsigned int)-1;
    }
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00746120
void __cdecl Voice::UpdateGain(Voice* pVoice, GainMatrix& rMatrix, bool bPanChanged)
{
    switch (Engine::GetMixConfig())
    {
    case Engine::mixConfig_Software:
        if (pVoice->Get3DMode())
        {
            const float fGain = pVoice->mGain * pVoice->CalculateGainEstimate();
            pVoice->Source3D::CalculateGain(fGain, pVoice->mReverbGain, rMatrix);
        }
        else
        {
            pVoice->Source2D::CalculateGain(pVoice->mGain, pVoice->mReverbGain,
                                            rMatrix);
        }
        Engine::GetSoftOutput()->SetGain(pVoice->mSoftConnIdx, rMatrix.frontLeft,
                                         rMatrix.frontRight, rMatrix.reverb,
                                         pVoice->mStarting);
        break;

    case Engine::mixConfig_DirectSoundHardware:
        Engine::GetSoftOutput()->SetGain(pVoice->mSoftConnIdx, 0.0f,
                                         rMatrix.frontLeft, 0.0f,
                                         pVoice->mStarting);

    case Engine::mixConfig_DirectSoundSoftware:
        pVoice->mRenderer.SetGain(pVoice->mGain);
        pVoice->mRenderer.SetPan(pVoice->mPan);
        break;

    default:
        break;
    }
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00746250
void __cdecl Voice::UpdatePitch(Voice* pVoice, float fPitch)
{
    switch (Engine::GetMixConfig())
    {
    case Engine::mixConfig_Software:
    case Engine::mixConfig_DirectSoundHardware:
    case Engine::mixConfig_DirectSoundSoftware:
    {
        const unsigned int uiOutputFrequency = Engine::GetSoftOutput()
            ? Engine::GetSoftOutput()->GetOutputFrequency()
            : Engine::GetOutputFrequency();

        float fRatio = (float)pVoice->VoiceBase::mSample->mFrequency
                     / (float)uiOutputFrequency * fPitch;
        if (fRatio < 0.01f)
        {
            fRatio = 0.01f;
        }
        else if (fRatio > 4.0f)
        {
            fRatio = 4.0f;
        }

        const unsigned int uiStepSize = (unsigned int)(fRatio * 65536.0f);
        pVoice->mSourceResampler.mResampler.mStepSize = uiStepSize;
        if (Engine::GetMixConfig() == Engine::mixConfig_DirectSoundHardware)
        {
            pVoice->mSourceResamplerLFE.mResampler.mStepSize = uiStepSize;
        }
        break;
    }
    default:
        break;
    }
}
}

// FUNCTION: SWBF2 0x00746a50
void __fastcall DestroyStreamNodeSourceAlias(Snd::StreamNodeSource* pSource)
{
    pSource->Snd::StreamNodeSource::~StreamNodeSource();
}
