#include <new>
#include <string.h>

#include "SoundFL/source/sndsampleinfo.h"
#include "SoundFL/source/sndstreamgsmdecoder.h"
#include "SoundFL/source/sndstreamnode.h"
#include "SoundFL/source/sndstreamsourceproducer.h"
#include "SoundFL/source/win/snddsbufferrenderer.h"
#include "SoundFL/source/win/sndsoftoutput.h"
#include "SoundFL/source/win/sndstreamauxrenderer.h"
#include "GameVoiceChatDecoder.h"
#include "VoiceChatNode.h"

#include "SoundFL/source/sndstreammixer.h"

// FUNCTION: SWBF2 0x0053d1d0
bool VoiceChat::Decoder::Open(float fPacketMs, int iSlots)
{
    return static_cast<GameVoiceChat::Decoder*>(this)->Open(fPacketMs, iSlots);
}

// FUNCTION: SWBF2 0x0053d200
void VoiceChat::Decoder::Close()
{
    static_cast<GameVoiceChat::Decoder*>(this)->Close();
}

// FUNCTION: SWBF2 0x0053d240
void VoiceChat::Decoder::Stop()
{
    static_cast<GameVoiceChat::Decoder*>(this)->Stop();
}

// FUNCTION: SWBF2 0x0053d1c0
VoiceChat::Decoder::Decoder()
    : mAppData(0)
{
}

// FUNCTION: SWBF2 0x0053c690
GameVoiceChat::Decoder::Decoder()
    : mProducers(0),
      mDecoders(0)
{
    mPlaying = false;
    mEnabled = false;
    mProducerPackets = 0;
    mDecoderBuffer = 0;
    mDecoderPackets = 0;
    mMixerAccBuffer = 0;
    mMixerOutBuffer = 0;
    mMixerPackets = 0;
    mMixerConnections = 0;
    mMixerPacketIO = 0;
    mMixerBufferOffsets = 0;
    mNumInputStreams = 0;
    mNumBuffers = 0;
    mRenderDeviceIndex = 0;
}

// FUNCTION: SWBF2 0x0053c7b0
GameVoiceChat::Decoder::~Decoder()
{
}

// FUNCTION: SWBF2 0x0053c780
void* __fastcall sub_53C780(void* pDecoder, void*  , unsigned int flags)
{
    ((GameVoiceChat::Decoder*)pDecoder)->GameVoiceChat::Decoder::~Decoder();
    if (flags & 1)
    {
        operator delete(pDecoder);
    }
    return pDecoder;
}

namespace VoiceChat { void* Alloc(unsigned int uiSize); }
namespace VoiceChat { void Free(void* pBlock); }
namespace GameVoiceChat { unsigned int CalculatePlayCaptureBufferSizeSamples(float fMilliseconds); }
namespace GameVoiceChat { unsigned int CalculateEncodedPacketSize(unsigned int uiSamples); }

namespace Snd
{

void __fastcall StreamAuxRenderer__Close(void* pAuxRenderer);

}

// FUNCTION: SWBF2 0x0053cc60
void GameVoiceChat::Decoder::FreeBuffers()
{
    VoiceChat::Free(mProducers);

    mProducers = 0;
    mDecoders = 0;
    mProducerPackets = 0;
    mDecoderBuffer = 0;
    mDecoderPackets = 0;
    mMixerAccBuffer = 0;
    mMixerOutBuffer = 0;
    mMixerPackets = 0;
    mMixerConnections = 0;
    mMixerPacketIO = 0;
    mMixerBufferOffsets = 0;
}

// FUNCTION: SWBF2 0x0053cfa0
char GameVoiceChat::Decoder::IsPlaying()
{
    return (char)mPlaying;
}

// FUNCTION: SWBF2 0x0053ce10
void GameVoiceChat::Decoder::Flush()
{
    if (mDecoders == 0)
    {
        return;
    }

    for (unsigned int i = 0; i < mNumInputStreams; ++i)
    {
        mDecoders[i].Flush();
    }

    mMixer.Flush();

    mAuxRenderer.Snd::DSBufferRenderer::Flush();
}

// FUNCTION: SWBF2 0x0053cf20
void GameVoiceChat::Decoder::Stop()
{
    Flush();

    for (unsigned int i = 0; i < mNumInputStreams; ++i)
    {
        static_cast<Snd::SourceTransport&>(mDecoders[i]).Stop();
    }

    ((Snd::SourceTransport*)mMixer.mNode)->Stop();
    static_cast<Snd::SourceTransport&>(mAuxRenderer).Stop();

    mPlaying = false;
}

// FUNCTION: SWBF2 0x0053cce0
void GameVoiceChat::Decoder::Close()
{

    for (unsigned int i = 0; i < mNumInputStreams; ++i)
    {
        mDecoders[i].mConnection.mInputNode = 0;
        mDecoders[i].mConnection.mInputNodeOutputIndex = 0;
        mMixer.mSink.Connect(0, 0, i);
    }

    mAuxRenderer.mConnection.mInputNode = 0;
    mAuxRenderer.mConnection.mInputNodeOutputIndex = 0;

    if (IsPlaying())
    {
        Stop();
    }

    Snd::StreamAuxRenderer__Close(&mAuxRenderer);

    mMixer.Snd::StreamSourceProducer::SetPackets(0, 0, 0);
    mMixer.SetMixBuffer(0);
    mMixer.mSink.SetConnectionArray(0, 0);
    mMixer.SetPacketArray(0, 0);

    SetProducers(0, 0);

    for (unsigned int i = 0; i < mNumInputStreams; ++i)
    {
        mProducers[i].~StreamSourceProducer();
        mDecoders[i].~StreamGSMDecoder();
        mMixerPacketIO[i].~StreamPacketIO();
    }

    mDecoders = 0;
    FreeBuffers();
    mPlaying = false;
    mEnabled = false;
    mNumBuffers = 0;
    mAppData = 0;
}

// FUNCTION: SWBF2 0x0053c810
bool GameVoiceChat::Decoder::Open(float fPacketMs, int iSlots)
{
    const unsigned int uiSlots = (unsigned int)iSlots;

    const unsigned int uiSamples = GameVoiceChat::CalculatePlayCaptureBufferSizeSamples(fPacketMs);
    const unsigned int uiPacket = GameVoiceChat::CalculateEncodedPacketSize(uiSamples);

    const unsigned int uiSamples4 = (uiSamples * 4u + 3u) & 0xFFFFFFFCu;
    const unsigned int uiSamples2 = uiSamples * 2u;
    const unsigned int uiAligned2 = (uiSamples2 + 3u) & 0xFFFFFFFCu;

    const unsigned int uiSize = (uiAligned2 + 0x330u) * uiSlots + 0x3Cu
                              + uiSamples4 + uiAligned2 * 2u;

    char* const pBlock = (char*)VoiceChat::Alloc(uiSize);
    mProducers = (Snd::StreamSourceProducer*)pBlock;
    if (pBlock == 0)
    {
        return false;
    }

    const unsigned int uiSlots24 = uiSlots * 24u;
    char* p = pBlock + uiSlots24;
    mProducerPackets = (Snd::StreamPacket*)p;
    p += uiSlots24;
    mDecoders = (Snd::StreamGSMDecoder*)p;
    p = (char*)(((unsigned int)(p + uiSlots * 0x2D4u) + 3u) & 0xFFFFFFFCu);
    mDecoderBuffer = (unsigned char*)p;
    p += uiAligned2 * uiSlots;
    mDecoderPackets = (Snd::StreamPacket*)p;
    p += uiSlots24;
    mMixerConnections = (Snd::StreamNodeConnection*)p;
    p += uiSlots * 8u;
    mMixerPacketIO = (Snd::StreamPacketIO*)p;
    p += uiSlots * 8u;
    mMixerBufferOffsets = (unsigned int*)p;
    p = (char*)(((unsigned int)(p + uiSlots * 4u) + 3u) & 0xFFFFFFFCu);
    mMixerAccBuffer = (int*)p;
    p += uiSamples4;
    mMixerPackets = (Snd::StreamPacket*)p;
    p = (char*)(((unsigned int)(p + 0x33u)) & 0xFFFFFFFCu);
    mMixerOutBuffer = (short*)p;

    for (unsigned int i = 0; i < uiSlots; ++i)
    {
        ::new (&mProducers[i]) Snd::StreamSourceProducer();
        ::new (&mProducerPackets[i]) Snd::StreamPacket();
        ::new (&mDecoders[i]) Snd::StreamGSMDecoder();
        ::new (&mDecoderPackets[i]) Snd::StreamPacket();
        if (&mMixerConnections[i])
        {
            memset(&mMixerConnections[i], 0, 24);
        }
        ::new (&mMixerPacketIO[i]) Snd::StreamPacketIO;
    }

    for (unsigned int i = 0; i < 2; ++i)
    {
        ::new (&mMixerPackets[i]) Snd::StreamPacket();
    }

    for (unsigned int i = 0; i < uiSlots; ++i)
    {
        *(unsigned int*)&mProducerPackets[i].mBuffer = 1;
        mProducers[i].SetPackets(uiPacket, &mProducerPackets[i], 1);
        *(unsigned int*)&mProducerPackets[i].mBuffer = 0;

        mDecoderPackets[i].mBuffer = mDecoderBuffer + i * uiAligned2;
        mDecoders[i].SetPackets(uiSamples2, &mDecoderPackets[i], 1);

        mMixerBufferOffsets[i] = 0;
    }

    for (unsigned int i = 0; i < 2; ++i)
    {
        mMixerPackets[i].mBuffer = (char*)mMixerOutBuffer + i * uiAligned2;
    }

    SetProducers(mProducers, uiSlots);

    mMixer.Snd::StreamSourceProducer::SetPackets(uiSamples2, mMixerPackets, 2);
    mMixer.SetMixBuffer(mMixerAccBuffer);
    mMixer.mSink.SetConnectionArray(mMixerConnections, uiSlots);
    mMixer.SetPacketArray(mMixerPacketIO, mMixerBufferOffsets);
    mMixer.SetSynchronizedMixing(true);
    mMixer.SetConstantOutputPacketSize(true);
    mMixer.SetLastPacketInfo(0);

    Snd::StreamFormat cFormat(Snd::SampleInfo::format_PCM16, 0x1F40u, 1);

    if (!mAuxRenderer.Open(0xFF, cFormat, 200.0f))
    {
        FreeBuffers();
        return false;
    }

    for (unsigned int i = 0; i < uiSlots; ++i)
    {
        mDecoders[i].mConnection.mInputNode = this;
        mDecoders[i].mConnection.mInputNodeOutputIndex = i;
        mMixer.mSink.Connect(&mDecoders[i], 0, i);
    }

    mAuxRenderer.mConnection.mInputNodeOutputIndex = 0;
    mAuxRenderer.mConnection.mInputNode = &mMixer;

    mAuxRenderer.SetLowTideMark(1.0f);

    mNumInputStreams = uiSlots;
    mRenderDeviceIndex = 0xFF;
    mPlaying = false;
    mEnabled = true;
    mNumBuffers = 2;
    mAppData = this;

    return true;
}

// FUNCTION: SWBF2 0x0053ce70
void GameVoiceChat::Decoder::Play()
{
    for (unsigned int i = 0; i < mNumInputStreams; ++i)
    {
        static_cast<Snd::SourceTransport&>(mDecoders[i]).Play(true);
    }

    ((Snd::SourceTransport*)mMixer.mNode)->Play(true);
    static_cast<Snd::SourceTransport&>(mAuxRenderer).Play(true);

    mPlaying = true;
}

// FUNCTION: SWBF2 0x0053cfb0
bool GameVoiceChat::Decoder::IsEnabled()
{
    return mPlaying && IsEnabled(mRenderDeviceIndex);
}

// FUNCTION: SWBF2 0x0053cfd0
bool GameVoiceChat::Decoder::IsEnabled(unsigned int  )
{
    return static_cast<const Snd::SourceTransport&>(mAuxRenderer).GetPlayState()
        != Snd::SourceTransport::playState_stopped;
}

// FUNCTION: SWBF2 0x0053d0d0
bool GameVoiceChat::Decoder::Update()
{
    const bool bEnabled = IsEnabled(mRenderDeviceIndex);
    if (mEnabled != bEnabled)
    {
        mEnabled = bEnabled;
        return true;
    }

    mMixer.Update();
    return static_cast<const Snd::StreamNode&>(mAuxRenderer).IsStarving();
}

// FUNCTION: SWBF2 0x0053d220
void VoiceChat::Decoder::Play()
{
    static_cast<GameVoiceChat::Decoder*>(this)->Play();
}

// FUNCTION: SWBF2 0x0053d260
bool VoiceChat::Decoder::IsEnabled()
{
    return static_cast<GameVoiceChat::Decoder*>(this)->IsEnabled();
}

// FUNCTION: SWBF2 0x0053d2a0
bool VoiceChat::Decoder::Update()
{
    return static_cast<GameVoiceChat::Decoder*>(this)->Update();
}

namespace GameVoiceChat {

// FUNCTION: SWBF2 0x0053cff0
bool Decoder::ReceivePacket(unsigned int uiInput, const VoiceChat::AudioPacket* pPacket)
{
    Snd::StreamGSMDecoder* pDecoder = &mDecoders[uiInput];
    Snd::StreamMixer* pMixer = &mMixer;

    if (pPacket->mSize)
    {
        if (pDecoder->GetNumQueuedPackets() >= 1)
        {
            return false;
        }
        Snd::StreamPacket* pOutput = static_cast<Snd::StreamSourceNProducer*>(this)->GetWritablePacket(uiInput);
        if (!pOutput)
        {
            return false;
        }
        pOutput->mBuffer = pPacket->mData;
        static_cast<Snd::StreamSourceNProducer*>(this)->CompleteWrittenPacket(pOutput, pPacket->mSize,
                        160 * (pPacket->mSize / 33u), false,
                        pPacket->mDiscontinuity, 0, uiInput);
        pDecoder->Update();
        pMixer->Update();
        return true;
    }

    if (pPacket->mDiscontinuity)
    {
        pDecoder->Flush();
        pMixer->Discontinuity(uiInput, true);
        return true;
    }
    return false;
}
}

namespace GameVoiceChat {

// FUNCTION: SWBF2 0x0053d110
bool Decoder::IsStarving(unsigned int uiInput, bool bForce)
{
    const Snd::StreamNode& cNode = mAuxRenderer;
    return (bForce || (cNode.GetPlayState() == 1 && cNode.IsStarving())) &&
           !mMixer.HasMixedPacket(uiInput);
}
}

// FUNCTION: SWBF2 0x0053d280
bool VoiceChat::Decoder::ReceivePacket(unsigned int input, const AudioPacket* packet)
{
    return static_cast<GameVoiceChat::Decoder*>(this)->ReceivePacket(input, packet);
}

// FUNCTION: SWBF2 0x0053d2c0
bool VoiceChat::Decoder::IsStarving(unsigned int input, bool force)
{
    return static_cast<GameVoiceChat::Decoder*>(this)->IsStarving(input, force);
}

// FUNCTION: SWBF2 0x0053d160
void GameVoiceChat::Decoder::SetGain(float fGain, unsigned int uiOutput)
{
    if (uiOutput == 0)
    {
        mMixer.SetOutputGain(fGain);
    }

    mRenderGain[uiOutput] = fGain;
}

// FUNCTION: SWBF2 0x0053d1b0
unsigned int GameVoiceChat::Decoder::GetNumOutputs() const
{
    return 1;
}

// FUNCTION: SWBF2 0x0053d2e0
void VoiceChat::Decoder::SetGain(float fGain, unsigned int uiOutput)
{
    static_cast<GameVoiceChat::Decoder*>(this)->SetGain(fGain, uiOutput);
}

// FUNCTION: SWBF2 0x0053d300
float VoiceChat::Decoder::GetGain(unsigned int uiOutput) const
{
    return static_cast<const GameVoiceChat::Decoder*>(this)->mRenderGain[uiOutput];
}

// FUNCTION: SWBF2 0x0053d320
unsigned int VoiceChat::Decoder::GetNumOutputs() const
{
    return static_cast<const GameVoiceChat::Decoder*>(this)->GetNumOutputs();
}
