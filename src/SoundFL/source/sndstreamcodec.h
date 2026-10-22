#pragma once

#include "sndstreamnode.h"
#include "sndstreamsourceproducer.h"

namespace Snd
{

class StreamCodec : public StreamSourceProducer,
                    public StreamNode,
                    public StreamNodeSink
{
public:

    virtual ~StreamCodec();

    virtual StreamPacket* GetPacket(unsigned int uiOutput);

protected:

    bool BufferPackets();
    bool Update();
};

 

}
