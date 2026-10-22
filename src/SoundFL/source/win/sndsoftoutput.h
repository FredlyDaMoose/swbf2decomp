#pragma once

#include "SoundFL/source/sndallocator.h"
#include "SoundFL/source/sndglobals.h"
#include "SoundFL/source/sndsampleinfo.h"
#include "SoundFL/source/sndsourcetransport.h"
#include "SoundFL/source/sndstreampacket.h"
#include "SoundFL/source/sndstreamnode.h"
#include "SoundFL/source/sndstreampacketio.h"

#include "SoundFL/source/sndstreamformat.h"

#include "SoundFL/source/sndstreammixer.h"
#include "SoundFL/source/sndstreamsourceproducer.h"

#include "SoundFL/source/sndstreamsplitter.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "snddsbufferrenderer.h"

#include "sndstreamdsdmo.h"

#include "sndengine.h"

struct IDirectSound;
struct _GUID;

namespace Snd
{

class SoftOutput;

class SoftOutput : public SourceTransport, public Allocator<SoftOutput>
{
public:

    class BusMixerInfo
    {
    public:
        BusMixerInfo();
        ~BusMixerInfo();

        StreamPacket mMixersPackets[2];
        int mMixerGains[128];
        StreamNodeConnection mMixerConns[32];
        StreamPacketIO mPacketIO[32];
        unsigned int mOffsets[32];
    };

    class MainMixerInfo
    {
    public:
        ~MainMixerInfo();

        StreamPacket mMixersPackets[2];
        int mMixerGains[8];
        StreamNodeConnection mMixerConns[2];
        StreamPacketIO mPacketIO[2];
        unsigned int mOffsets[2];
    };

    class SplitterQueueInfo
    {
    public:
        StreamPacket* mPacketQueue[2];
        unsigned short mRefCounts[2];
        unsigned short mRemaining[2];
    };

    // VTABLE: SWBF2 0x00786b50
    class PostOutputMixFlusher : public StreamNodeSource
    {
    public:
        PostOutputMixFlusher()
        {

            mSink.Connect(0, 0);
            mSoftOutput = 0;
        }

        void SetOutput(SoftOutput* const pOutput) { mSoftOutput = pOutput; }

        virtual StreamPacket* GetPacket(unsigned int uiOutput);

        StreamNodeSink mSink;
        SoftOutput* mSoftOutput;
    };

    SoftOutput();

    void Connect(StreamNodeSource* const pSource, unsigned int uiOutput,
                 unsigned int uiInput);
    unsigned int GetUnconnectedInput() const;

    void Discontinuity(unsigned int uiInput);
    void SetBusGain(unsigned int uiBus, float fLeftGain, float fRightGain,
                    bool bImmediate);

    void SetGain(unsigned int uiInput, float fLeftGain, float fRightGain,
                 float fReverbGain, bool bImmediate);
    bool SetOutputBufferSize(float fBufferMs, bool bRestart, bool bEffects,
                             unsigned int uiFrequency);

    DSBufferRenderer& GetRenderer();
    unsigned int GetOutputFrequency() const;

    bool SetReverb(const I3DL2Reverb* const pReverb);

    ~SoftOutput();
    void Update();

private:
    void Connect();
    void Disconnect();
    void FreeMixBuffer();

public:
    StreamMixer mMixer;
    BusMixerInfo mMixerInfo;
    StreamSplitter mSplitter;
    SplitterQueueInfo mSplitterInfo;
    StreamDSDMO mDMO;
    StreamPacket mDMOPackets[2];
    PostOutputMixFlusher mPostOutputMixFlusher;
    StreamMixer mMixerOutput;
    MainMixerInfo mMixerOutputInfo;
    DSBufferRenderer mRenderer;
    void* mMixBuffer;
    bool mEffectsEnabled;
};

 
 
 
 

}
