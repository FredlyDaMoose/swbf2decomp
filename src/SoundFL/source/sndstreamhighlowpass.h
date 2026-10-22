#pragma once

#include "snddsp.h"
#include "sndstreampacketio.h"
#include "sndstreamsourceproducer.h"

namespace Snd
{

class StreamLowHighPass : public StreamSourceProducer
{
public:
    StreamLowHighPass();
    virtual ~StreamLowHighPass();

    void SetBuffer(void* pBuffer, unsigned int uiSize);

    void Flush();

protected:

    virtual StreamPacket* GetPacket(unsigned int uiOutput);

public:
    StreamNode mNode;
    StreamNodeSink mSink;
    DSP::HighLowPassFilter mFilter;
    unsigned int mBufferSize;
    StreamPacket mOutputPacket;
    StreamPacketIO mInputPacket;
};

 

}
