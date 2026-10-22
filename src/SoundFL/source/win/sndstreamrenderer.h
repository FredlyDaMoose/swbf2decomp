#pragma once

#include <stddef.h>

#include "SoundFL/source/sndsampleinfo.h"
#include "SoundFL/source/sndstreamnode.h"
#include "SoundFL/source/sndstreamrendererbase.h"
#include "SoundFL/source/sndstreamresampler.h"
#include "snddsbufferrenderer.h"

namespace Snd
{

class StreamRenderer : public StreamNodeSource, public StreamRendererBase
{
public:
    StreamRenderer();

    virtual ~StreamRenderer();

    bool SetFormat(SampleInfo::Format eFormat, unsigned int uiNumChannels,
                   unsigned int uiFrequency, bool b3D, unsigned long ulFlags,
                   float fBufferMs);

    void Flush();

    void Update();

private:
    void ConnectInternal();
    void DisconnectInternal();

protected:

    virtual StreamPacket* GetPacket(unsigned int uiOutput);

public:
    unsigned int mFrequency;
    unsigned int mNumChannels;
    SampleInfo::Format mFormat;
    unsigned long mChannelMask;
    DSBufferRenderer* mHardwareRenderer;
    unsigned char mHardwareRendererStorage[416];
    unsigned int mOutConnIdx;
    StreamResampler mResampler;
    void* mResamplerBuffer;
};

 
 
 
 
 
 

}
