#include "sndstreamconsumer.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074deb0
StreamConsumer::StreamConsumer()
    : mReady(0)
{
    StreamNodeSink::Connect(0, 0);
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0074ded0
StreamConsumer::~StreamConsumer()
{
}
}
