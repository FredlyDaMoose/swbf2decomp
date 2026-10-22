#pragma once

#include <stddef.h>
#include "sndstreamnode.h"

namespace Snd
{

class StreamConsumer : public StreamNodeSink
{
public:
    StreamConsumer();
    ~StreamConsumer();

    unsigned char mReady;
};

 

 

}
