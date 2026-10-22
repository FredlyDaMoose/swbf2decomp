#include <new>

#include "sndstreamrenderer.h"

// TEMPLATE: SWBF2 0x00749120
// Snd::Allocator<Snd::DSBufferRenderer>::operator delete

#include "SoundFL/source/sndglobals.h"
#include "sndsoftoutput.h"
#include "sndengine.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00748fe0
StreamRendererBase::StreamRendererBase()
    : Source3D(0)
{

    Source3D::mProps = static_cast<Source3D::Properties*>(this);
}

// FUNCTION: SWBF2 0x00748f30
StreamRenderer::StreamRenderer()
    : mFrequency(0),
      mNumChannels(0),
      mFormat(SampleInfo::format_Unknown),
      mChannelMask(0),
      mHardwareRenderer(0),
      mOutConnIdx(0xFFFFFFFF),
      mResamplerBuffer(0)
{

    ConnectInternal();
}

// FUNCTION: SWBF2 0x007491e0
void StreamRenderer::ConnectInternal()
{
    DisconnectInternal();

    const Engine::MixConfig eMixConfig = Engine::GetMixConfig();
    if ((unsigned int)(eMixConfig - 1) > 2)
    {
        return;
    }
    if (mHardwareRenderer != 0)
    {
        return;
    }

    Engine::GetThreadUpdate()->Resume();
    mHardwareRenderer = new (mHardwareRendererStorage, false) DSBufferRenderer;
    Engine::GetThreadUpdate()->Pause(false);

    if (mFormat != 0)
    {

        mHardwareRenderer->SetFormat(mFormat, mNumChannels, mFrequency,
                                     (Source2D3D::mFlags & 1) != 0, 200.0f,
                                     false, 0, mChannelMask, false);
    }

    mHardwareRenderer->StreamNodeSink::Connect(this, 0);
}

// FUNCTION: SWBF2 0x00749300
void StreamRenderer::DisconnectInternal()
{
    if (mHardwareRenderer != 0)
    {
        delete mHardwareRenderer;
        mHardwareRenderer = 0;
    }

    if (mOutConnIdx != 0xFFFFFFFF)
    {
        Engine::GetSoftOutput()->Connect(0, 0, 0);
    }

    if (mResamplerBuffer != 0)
    {
        mResampler.SetBuffer(0, 0);
        Free(mResamplerBuffer);
        mResamplerBuffer = 0;
    }
}

// FUNCTION: SWBF2 0x00749150
void* __fastcall sub_749150(void* pRenderer, int, unsigned int uiFlags)
{
    StreamRenderer* const pThis = (StreamRenderer*)pRenderer;

    pThis->StreamRenderer::~StreamRenderer();

    if ((uiFlags & 1) != 0)
    {
        ::operator delete(pThis);
    }

    return pThis;
}

// FUNCTION: SWBF2 0x00749370
void StreamRenderer::Flush()
{
    if (mHardwareRenderer != 0)
    {
        mHardwareRenderer->Flush();
    }
    else if (mOutConnIdx != 0xFFFFFFFF)
    {
        mResampler.Flush();
    }
}

// FUNCTION: SWBF2 0x007493a0
StreamPacket* StreamRenderer::GetPacket(unsigned int)
{
    StreamNodeSource* const pInputNode = mConnection.mInputNode;

    if (pInputNode == 0)
    {
        return 0;
    }
    return pInputNode->GetPacket(mConnection.mInputNodeOutputIndex);
}

// FUNCTION: SWBF2 0x007493c0
bool StreamRenderer::SetFormat(SampleInfo::Format eFormat,
                               unsigned int uiNumChannels,
                               unsigned int uiFrequency, bool b3D,
                               unsigned long ulFlags, float fBufferMs)
{

    mChannelMask = ulFlags;

    bool bOk = true;

    if (mHardwareRenderer != 0)
    {
        bOk = mHardwareRenderer->SetFormat(eFormat, uiNumChannels, uiFrequency,
                                           b3D, fBufferMs, false, 0, ulFlags,
                                           false);
        if (!bOk)
        {
            mFormat = SampleInfo::format_Unknown;
            mNumChannels = 0;
            mFrequency = 0;
            return false;
        }
    }

    mFormat = eFormat;
    mNumChannels = uiNumChannels;
    mFrequency = uiFrequency;

    Source2D3D::Set3DMode(b3D);

    return bOk;
}

// FUNCTION: SWBF2 0x00749480
void StreamRenderer::Update()
{
    switch (GetDesiredPlayState())
    {
    case playState_playing:
        if (mHardwareRenderer)
        {
            mHardwareRenderer->StreamNode::Play(true);
        }
        else if (mOutConnIdx != 0xFFFFFFFF)
        {
            mResampler.StreamNode::Play(true);
        }
        break;
    case playState_paused:
        if (mHardwareRenderer)
        {
            mHardwareRenderer->StreamNode::Pause();
        }
        else if (mOutConnIdx != 0xFFFFFFFF)
        {
            mResampler.StreamNode::Pause();
        }
        break;
    case playState_stopped:
        if (mHardwareRenderer)
        {
            mHardwareRenderer->StreamNode::Stop();
        }
        else if (mOutConnIdx != 0xFFFFFFFF)
        {
            mResampler.StreamNode::Stop();
        }
        break;
    }

    const bool b3D = (Source2D3D::mFlags & 1) != 0;
    if (b3D)
    {
        Source3D::mFlags |= Source3D::flags_updateListener;
        DetermineClosestListener();
    }

    if (mHardwareRenderer)
    {
        if (b3D != ((mHardwareRenderer->Source2D3D::mFlags & 1) != 0))
        {
            mHardwareRenderer->Set3DMode(b3D, mChannelMask);
        }
        mHardwareRenderer->Source3D::CopyParameters(*this);
        mHardwareRenderer->SetPan(mPan);
        mHardwareRenderer->SetGain(Source::mGain);
        mHardwareRenderer->SetDopplerMode(GetDopplerMode());
    }
    else if (GetPanChanged() || GetGainChanged() || b3D)
    {
        if (mOutConnIdx != 0xFFFFFFFF)
        {
            GainMatrix matrix;
            if (b3D)
            {
                Source3D::CalculateGain(Source::mGain * CalculateGainEstimate(),
                                        mReverbGain, matrix);
            }
            else
            {
                Source2D::CalculateGain(Source::mGain, mReverbGain, matrix);
            }
            Engine::GetSoftOutput()->SetGain(mOutConnIdx, matrix.frontLeft,
                                             matrix.frontRight, mReverbGain, false);
        }
    }

    if (mHardwareRenderer)
    {
        mHardwareRenderer->SetFrequency(mFrequency);
        mHardwareRenderer->SetPitch(mPitch);
    }
    else if (Engine::GetSoftOutput())
    {
        const float fRatio = (float)mFrequency
                             / (float)Engine::GetSoftOutput()->GetOutputFrequency();
        const float fPitch3D = b3D ? Source3D::CalculatePitch() : 1.0f;
        mResampler.mStepSize = (unsigned int)(mPitch * fRatio * fPitch3D * 65536.0f);
    }

    if (mHardwareRenderer)
    {
        SetPlayState(mHardwareRenderer->GetPlayState());
        mPlaybackPosition = mHardwareRenderer->mPlaybackPosition;
    }
    else
    {
        SetPlayState(mResampler.GetPlayState());
        mPlaybackPosition = mResampler.mPlaybackPosition;
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x007490a0
StreamRenderer::~StreamRenderer()
{
    DisconnectInternal();

}
}
