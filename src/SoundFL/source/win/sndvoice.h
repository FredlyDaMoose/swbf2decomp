#pragma once

#include "SoundFL/source/sndstreampacket.h"
#include "SoundFL/source/sndstreamsourcememory.h"
#include "sndstreamrenderer.h"
#include "SoundFL/source/sndvoicehardware.h"

namespace Snd
{

class Sample;

class Voice : public VoiceHardware
{

    friend class VoiceHardware;

public:

    class SourceResampler
    {
    public:
        explicit SourceResampler(bool bAllocateBuffer);

        ~SourceResampler();

        void ClearAll();
        void Update()
        {
            mSource.Update();
            mResampler.Update();
        }
        void Flush();
        void Pause();
        void Stop();
        void Play();
        void Setup(const Sample* const pSample, bool bSetupPackets);

        void UnlockAll();

        StreamSourceMemory mSource;
        StreamPacket mPackets[1];
        StreamResampler mResampler;
        void* mResamplerBuffer;
    };

    Voice();

    ~Voice();

    void Initialize();

    void Update();

protected:

    static void __cdecl UpdatePlayState(Voice* pVoice, PlayState ePlayState);

    static void __cdecl UpdatePitch(Voice* pVoice, float fPitch);

    static void __cdecl UpdateGain(Voice* pVoice, GainMatrix& rMatrix,
                                   bool bPanChanged);

private:
    void Update3DMode();
    void UpdateSample();

public:

    StreamRenderer mRenderer;
    SourceResampler mSourceResampler;
    unsigned int mSoftConnIdx;
    Sample* mSample;
    bool mStarting;
    bool mSetupPackets;
    SourceResampler mSourceResamplerLFE;
};

 
 
 
 

}
