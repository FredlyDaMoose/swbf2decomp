#pragma once

#include <stddef.h>

#include "sndstreamnode.h"
#include "sndstreampacket.h"

namespace Snd
{

class StreamResampler : public StreamNodeSource,
                        public StreamNode,
                        public StreamNodeSink
{
public:
    StreamResampler();
    virtual ~StreamResampler();

    void SetBuffer(void* pBuffer, unsigned int uiSize);
    void Flush();

    void Update();
    virtual StreamPacket* GetPacket(unsigned int uiOutput);

private:
    void CompletePacket();

public:

    unsigned int mBufferSize;
    unsigned int mStepSize;
    StreamPacket* mInputPacket;
    unsigned int mInputPacketReadPos;
    StreamPacket mOutputPacket;
    int mSampleOffset;
    int mPrevSample;
};

 
 
 
 

}
