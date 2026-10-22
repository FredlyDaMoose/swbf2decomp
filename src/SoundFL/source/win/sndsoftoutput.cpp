#include <windows.h>
#include <dsound.h>

#include "SoundFL/source/sndi3dl2reverb.h"
#include "sndsoftoutput.h"

namespace Snd
{

// FUNCTION: SWBF2 0x007467b0
SoftOutput::SoftOutput()
{
    mPlaybackPosition = 0;
    mFlags = flags_stopped;
    mMixBuffer = 0;
    mEffectsEnabled = false;

    mSplitter.Setup(mSplitterInfo.mPacketQueue, mSplitterInfo.mRefCounts, 2,
                    mSplitterInfo.mRemaining, 2);

    mPostOutputMixFlusher.SetOutput(this);
}

// FUNCTION: SWBF2 0x00746940
SoftOutput::BusMixerInfo::~BusMixerInfo()
{
}

// FUNCTION: SWBF2 0x007469d0
SoftOutput::MainMixerInfo::~MainMixerInfo()
{
}

// FUNCTION: SWBF2 0x00746c70
void SoftOutput::Connect(StreamNodeSource* const pSource, unsigned int uiOutput,
                         unsigned int uiInput)
{
    mMixer.mSink.Connect(pSource, uiOutput, uiInput);
}

// FUNCTION: SWBF2 0x00746be0
void SoftOutput::Connect()
{
    StreamNodeSource* pSource = &mMixer;

    if (mEffectsEnabled)
    {
        mSplitter.mSink.Connect(&mMixer, 0);
        mMixerOutput.mSink.Connect(&mSplitter, 0, 0);
        mDMO.mSink.Connect(&mSplitter, 1);
        mMixerOutput.mSink.Connect(&mDMO, 0, 1);
        mPostOutputMixFlusher.mSink.Connect(&mMixerOutput, 0);
        pSource = &mPostOutputMixFlusher;
    }

    mRenderer.StreamNodeSink::Connect(pSource, 0);
}

// FUNCTION: SWBF2 0x00746c80
void SoftOutput::Disconnect()
{
    unsigned int uiNumInputs = mMixer.mSink.GetNumInputs();
    for (unsigned int i = 0; i < uiNumInputs; i++)
    {
        mMixer.mSink.Connect(0, 0, i);
    }

    mSplitter.mSink.Connect(0, 0);
    mDMO.mSink.Connect(0, 0);
    mRenderer.StreamNodeSink::Connect(0, 0);

    uiNumInputs = mMixerOutput.mSink.GetNumInputs();
    for (unsigned int i = 0; i < uiNumInputs; i++)
    {
        mMixerOutput.mSink.Connect(0, 0, i);
    }
}

// FUNCTION: SWBF2 0x00746d40
void SoftOutput::FreeMixBuffer()
{
    if (mMixBuffer)
    {
        FreePerm(mMixBuffer);
    }

    mMixer.SetPackets(0, 0, 0);
    mMixer.SetChannels(1, 1);
    mMixer.mSink.SetConnectionArray(0, 0);
    mMixer.SetGainArray(0);
    mMixer.SetMixBuffer(0);
    mMixer.SetPacketArray(0, 0);

    mDMO.SetInputBuffer(0, 0);
    mDMO.SetPackets(0, 0, 0);

    mMixerOutput.SetPackets(0, 0, 0);
    mMixerOutput.SetChannels(0, 0);
    mMixerOutput.mSink.SetConnectionArray(0, 0);
    mMixerOutput.SetGainArray(0);
    mMixerOutput.SetMixBuffer(0);
    mMixerOutput.SetPacketArray(0, 0);
}

// FUNCTION: SWBF2 0x00746e20
unsigned int SoftOutput::GetOutputFrequency() const
{
    return mRenderer.mFrequency;
}

// FUNCTION: SWBF2 0x00746f20
DSBufferRenderer& SoftOutput::GetRenderer()
{
    return mRenderer;
}

// FUNCTION: SWBF2 0x00746f30
unsigned int SoftOutput::GetUnconnectedInput() const
{
    for (unsigned int i = 0; i < 32; i++)
    {
        if (!mMixer.mSink.ConnectionGetNode(i))
        {
            return i;
        }
    }

    return 0xffffffff;
}

// FUNCTION: SWBF2 0x00746f60
void SoftOutput::SetBusGain(unsigned int uiBus, float fLeftGain, float fRightGain,
                            bool bImmediate)
{
    mMixerOutput.SetGain(uiBus, 0, fLeftGain, bImmediate);
    mMixerOutput.SetGain(uiBus, 1, fRightGain, bImmediate);
}

// FUNCTION: SWBF2 0x00746ff0
bool SoftOutput::SetOutputBufferSize(float fBufferMs, bool bRestart, bool bEffects,
                                     unsigned int uiFrequency)
{
    StreamFormat format(SampleInfo::format_PCM16, uiFrequency, 2);

    Engine::GetThreadUpdate()->Pause(false);
    Disconnect();

    mEffectsEnabled = bEffects;

    mRenderer.SetFormat(format.GetFormat(), format.GetNumChannels(), format.GetFrequency(),
                        false, fBufferMs, bRestart, 0, 0, false);

    Engine::GetThreadUpdate()->Resume();

    if (mRenderer.mNumChannels != format.GetNumChannels()
        || mRenderer.mFormat != format.GetFormat())
    {
        if (!bRestart)
        {
            return false;
        }
        if (!SetOutputBufferSize(fBufferMs, false, true, 48000))
        {
            return false;
        }
    }

    format.SetFrequency(mRenderer.mFrequency);

    if (mEffectsEnabled
        && !mDMO.CreateDMO(GUID_DSFX_STANDARD_I3DL2REVERB, format, format))
    {
        mEffectsEnabled = false;
    }

    const unsigned int uiInputAlignment = mDMO.GetInputAligment();
    const unsigned int uiMinInputSize = mDMO.GetMinInputSize();
    const unsigned int uiDMOSum =
        uiMinInputSize + ((uiInputAlignment == 1) ? 0 : uiInputAlignment);
    const unsigned int uiDMOBytes = uiDMOSum ? uiDMOSum : 4;
    const unsigned int uiAlignment = uiInputAlignment ? uiInputAlignment : 1;

    const unsigned int uiGranularity = uiDMOBytes / 4;
    const unsigned int uiPacketBytes =
        (unsigned int)((float)mRenderer.mFrequency * 20.0f
                       * (float)mRenderer.mNumChannels * 2.0f / 1000.0f);
    const unsigned int uiPacketSize =
        ((uiPacketBytes - 1 + uiGranularity) / uiGranularity) * uiGranularity;
    const unsigned int uiHalfPacket = uiPacketSize / 2;
    const unsigned int uiMixBytes = uiHalfPacket * 4;

    FreeMixBuffer();

    mMixBuffer = AllocPerm(uiHalfPacket * 4
                           + (uiHalfPacket + uiPacketSize * 2) * 4
                           + uiDMOBytes);
    if (!mMixBuffer)
    {
        return false;
    }

    unsigned char* pBuffer = (unsigned char*)mMixBuffer;
    mMixerInfo.mMixersPackets[0].mBuffer = pBuffer;
    mMixerInfo.mMixersPackets[1].mBuffer = pBuffer + uiPacketSize;
    unsigned char* pNext = pBuffer + uiPacketSize * 2;

    mMixer.SetPackets(uiPacketSize, mMixerInfo.mMixersPackets, 2);
    mMixer.SetChannels(1, 2);
    mMixer.mSink.SetConnectionArray(mMixerInfo.mMixerConns, 32);
    mMixer.SetGainArray(mMixerInfo.mMixerGains);
    mMixer.SetMixBuffer((int*)pNext);
    mMixer.SetPacketArray(mMixerInfo.mPacketIO, mMixerInfo.mOffsets);
    mMixer.SetSynchronizedMixing(true);
    mMixer.SetSynchronizedActivation(false);
    mMixer.SetConstantOutputPacketSize(true);
    mMixer.SetLastPacketInfo(mEffectsEnabled);
    mMixer.SetStopOnLast(mEffectsEnabled);

    unsigned char* pSlice =
        (unsigned char*)(((unsigned int)(pNext + uiMixBytes) + uiAlignment - 1)
                         & ~(uiAlignment - 1));
    const unsigned int uiDMOInput = (uiDMOBytes / uiAlignment) * uiAlignment;

    if (mEffectsEnabled)
    {
        mDMO.SetInputBuffer(pSlice, uiDMOInput);
    }
    pSlice += uiDMOInput;

    if (mEffectsEnabled)
    {
        mDMOPackets[0].mBuffer = pSlice;
        pSlice += uiPacketSize;
        mDMOPackets[1].mBuffer = pSlice;
        pSlice += uiPacketSize;
        mDMO.SetPackets(uiPacketSize, mDMOPackets, 2);
    }

    mMixerOutputInfo.mMixersPackets[0].mBuffer = pSlice;
    pSlice += uiPacketSize;
    mMixerOutputInfo.mMixersPackets[1].mBuffer = pSlice;
    pSlice += uiPacketSize;

    mMixerOutput.SetPackets(uiPacketSize, mMixerOutputInfo.mMixersPackets, 2);
    mMixerOutput.mSink.SetConnectionArray(mMixerOutputInfo.mMixerConns, 2);
    mMixerOutput.SetMixBuffer((int*)pSlice);
    mMixerOutput.SetPacketArray(mMixerOutputInfo.mPacketIO, mMixerOutputInfo.mOffsets);
    mMixerOutput.SetChannels(2, 2);
    mMixerOutput.SetGainArray(mMixerOutputInfo.mMixerGains);
    mMixerOutput.SetSynchronizedMixing(true);
    mMixerOutput.SetSynchronizedActivation(false);
    if (mEffectsEnabled)
    {
        mMixerOutput.SetSynchronizedDeactivation(true);
    }
    mMixerOutput.SetConstantOutputPacketSize(true);

    Connect();
    return true;
}

// FUNCTION: SWBF2 0x007473c0
bool SoftOutput::SetReverb(const I3DL2Reverb* const pReverb)
{

    if (!mEffectsEnabled)
    {
        return false;
    }

    IDirectSoundFXI3DL2Reverb8* pFX = 0;
    if (Engine::Failed(mDMO.mDMO.GetMediaObject()->QueryInterface(
                           IID_IDirectSoundFXI3DL2Reverb8, (void**)&pFX),
                       "source\\win\\sndsoftoutput.cpp", 0x188))
    {
        return false;
    }

    DSFXI3DL2Reverb reverb;

    reverb.flDecayHFRatio =
        (DSFX_I3DL2REVERB_DECAYHFRATIO_MIN > pReverb->mDecayHFRatio)
            ? DSFX_I3DL2REVERB_DECAYHFRATIO_MIN
            : (pReverb->mDecayHFRatio > DSFX_I3DL2REVERB_DECAYHFRATIO_MAX
                   ? DSFX_I3DL2REVERB_DECAYHFRATIO_MAX
                   : pReverb->mDecayHFRatio);

    reverb.flDecayTime =
        (DSFX_I3DL2REVERB_DECAYTIME_MIN > pReverb->mDecayTime)
            ? DSFX_I3DL2REVERB_DECAYTIME_MIN
            : (pReverb->mDecayTime > DSFX_I3DL2REVERB_DECAYTIME_MAX
                   ? DSFX_I3DL2REVERB_DECAYTIME_MAX
                   : pReverb->mDecayTime);

    reverb.flDensity =
        (DSFX_I3DL2REVERB_DENSITY_MIN > pReverb->mDensity)
            ? DSFX_I3DL2REVERB_DENSITY_MIN
            : (pReverb->mDensity > DSFX_I3DL2REVERB_DENSITY_MAX
                   ? DSFX_I3DL2REVERB_DENSITY_MAX
                   : pReverb->mDensity);

    reverb.flDiffusion =
        (DSFX_I3DL2REVERB_DIFFUSION_MIN > pReverb->mDiffusion)
            ? DSFX_I3DL2REVERB_DIFFUSION_MIN
            : (pReverb->mDiffusion > DSFX_I3DL2REVERB_DIFFUSION_MAX
                   ? DSFX_I3DL2REVERB_DIFFUSION_MAX
                   : pReverb->mDiffusion);

    reverb.flHFReference =
        (DSFX_I3DL2REVERB_HFREFERENCE_MIN > pReverb->mHFReference)
            ? DSFX_I3DL2REVERB_HFREFERENCE_MIN
            : (pReverb->mHFReference > DSFX_I3DL2REVERB_HFREFERENCE_MAX
                   ? DSFX_I3DL2REVERB_HFREFERENCE_MAX
                   : pReverb->mHFReference);

    reverb.flReflectionsDelay =
        (DSFX_I3DL2REVERB_REFLECTIONSDELAY_MIN > pReverb->mERDelay)
            ? DSFX_I3DL2REVERB_REFLECTIONSDELAY_MIN
            : (pReverb->mERDelay > 0.1f ? 0.1f : pReverb->mERDelay);

    reverb.flReverbDelay =
        (DSFX_I3DL2REVERB_REVERBDELAY_MIN > pReverb->mReverbDelay)
            ? DSFX_I3DL2REVERB_REVERBDELAY_MIN
            : (pReverb->mReverbDelay > DSFX_I3DL2REVERB_REVERBDELAY_MAX
                   ? DSFX_I3DL2REVERB_REVERBDELAY_MAX
                   : pReverb->mReverbDelay);

    reverb.flRoomRolloffFactor =
        (DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_MIN > pReverb->mRoomRollOff)
            ? DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_MIN
            : (pReverb->mRoomRollOff > DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_MAX
                   ? DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_MAX
                   : pReverb->mRoomRollOff);

    const int iReflections = ConvertLinearToLog(pReverb->mERGain);
    reverb.lReflections =
        (iReflections < DSFX_I3DL2REVERB_REFLECTIONS_MIN)
            ? DSFX_I3DL2REVERB_REFLECTIONS_MIN
            : (iReflections > DSFX_I3DL2REVERB_REFLECTIONS_MAX
                   ? DSFX_I3DL2REVERB_REFLECTIONS_MAX
                   : iReflections);

    const int iReverb = ConvertLinearToLog(pReverb->mReverbGain);
    reverb.lReverb = (iReverb < DSFX_I3DL2REVERB_REVERB_MIN)
                         ? DSFX_I3DL2REVERB_REVERB_MIN
                         : (iReverb > DSFX_I3DL2REVERB_REVERB_MAX
                                ? DSFX_I3DL2REVERB_REVERB_MAX
                                : iReverb);

    const int iRoom = ConvertLinearToLog(pReverb->mRoomGain);
    reverb.lRoom = (iRoom < DSFX_I3DL2REVERB_ROOM_MIN)
                       ? DSFX_I3DL2REVERB_ROOM_MIN
                       : (iRoom > DSFX_I3DL2REVERB_ROOM_MAX
                              ? DSFX_I3DL2REVERB_ROOM_MAX
                              : iRoom);

    const int iRoomHF = ConvertLinearToLog(pReverb->mRoomHFGain);
    reverb.lRoomHF = (iRoomHF < DSFX_I3DL2REVERB_ROOMHF_MIN)
                         ? DSFX_I3DL2REVERB_ROOMHF_MIN
                         : (iRoomHF > DSFX_I3DL2REVERB_ROOMHF_MAX
                                ? DSFX_I3DL2REVERB_ROOMHF_MAX
                                : iRoomHF);

    const bool bOK = Engine::Succeeded(pFX->SetAllParameters(&reverb),
                                       "source\\win\\sndsoftoutput.cpp", 0x1b2);
    pFX->Release();
    return bOK;
}

// FUNCTION: SWBF2 0x00746bb0
void* __fastcall sub_746BB0(void* pFlusher, int, unsigned int uiFlags)
{
    ((StreamNodeSource*)pFlusher)->StreamNodeSource::~StreamNodeSource();
    if (uiFlags & 1)
    {
        operator delete(pFlusher);
    }
    return pFlusher;
}

// FUNCTION: SWBF2 0x00746d20
void Snd::SoftOutput::Discontinuity(unsigned int uiInput)
{
    SoftOutput* pOutput = (SoftOutput*)this;
    pOutput->mMixer.Discontinuity(uiInput, true);
}

// FUNCTION: SWBF2 0x00746e30
StreamPacket* SoftOutput::PostOutputMixFlusher::GetPacket(unsigned int  )
{
    typedef StreamPacket* (__thiscall* GetPacketFn)(StreamNodeSource* pThis,
                                                    unsigned int uiOutput);

    StreamNodeSource* const pSource = mSink.mConnection.mInputNode;
    if (pSource == 0)
    {
        return 0;
    }

    StreamPacket* const pPacket = pSource->GetPacket(mSink.mConnection.mInputNodeOutputIndex);
    if (pPacket != 0 && (pPacket->GetDiscontinuity() || pPacket->GetLast()))
    {
        mSoftOutput->mDMO.mNode.Play(true);
        ((StreamNode*)mSoftOutput->mSplitter.mNode)->Play(true);
        ((StreamNode*)mSoftOutput->mMixer.mNode)->Play(true);
        pPacket->mFlags = (unsigned short)((pPacket->mFlags & ~1u) | 2u);
    }
    return pPacket;
}

// FUNCTION: SWBF2 0x00747670
void SoftOutput::Update()
{
    const PlayState eDesired = QueryDesiredPlayState();
    mFlags = (unsigned short)(mFlags & ~(unsigned int)flags_updateTransport);

    switch (eDesired)
    {
    case playState_playing:
        SetPlayState(playState_playing);
        ((StreamNode*)mMixer.mNode)->Play(true);
        ((StreamNode*)mSplitter.mNode)->Play(true);
        mDMO.mNode.Play(true);
        ((StreamNode*)mMixerOutput.mNode)->Play(true);
        mRenderer.StreamNode::Play(true);
        break;

    case playState_paused:
        ((StreamNode*)mMixer.mNode)->Pause();
        ((StreamNode*)mSplitter.mNode)->Pause();
        mDMO.mNode.Pause();
        ((StreamNode*)mMixerOutput.mNode)->Pause();
        mRenderer.StreamNode::Pause();
        SetPlayState(playState_paused);
        break;

    case playState_stopped:
        ((StreamNode*)mMixer.mNode)->Stop();
        ((StreamNode*)mSplitter.mNode)->Stop();
        mDMO.mNode.Stop();
        ((StreamNode*)mMixerOutput.mNode)->Stop();
        mRenderer.StreamNode::Stop();
        mRenderer.Flush();
        mMixerOutput.Flush();
        mDMO.Flush();
        mSplitter.Flush();
        mMixer.Flush();
        SetPlayState(playState_stopped);
        break;
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x00746720
SoftOutput::BusMixerInfo::BusMixerInfo()
{
    for (int i = 0; i < 32; ++i)
    {
        mMixerConns[i].mInputNode = 0;
        mMixerConns[i].mInputNodeOutputIndex = 0;
    }
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00746fb0
void SoftOutput::SetGain(unsigned int uiInput, float fLeftGain, float fRightGain,
                         float fReverbGain, bool bImmediate)
{
    (void)fReverbGain;
    mMixer.SetGain(uiInput, 0, fLeftGain, bImmediate);
    mMixer.SetGain(uiInput, 1, fRightGain, bImmediate);
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00746a60
SoftOutput::~SoftOutput()
{
    FreeMixBuffer();
}
}
