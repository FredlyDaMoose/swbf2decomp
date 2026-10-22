#include "sndstreamcodec.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0073d100
StreamCodec::~StreamCodec()
{

}

// FUNCTION: SWBF2 0x0074bc50
bool StreamCodec::BufferPackets()
{
    switch (StreamNode::GetCurrentPlayState())
    {
    case SourceTransport::playState_playing:
        return true;

    case SourceTransport::playState_pausing:
        if (GetReadPacket())
        {
            return true;
        }
        StreamNode::SetPlayState(SourceTransport::playState_paused);
        return false;
    }
    return false;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0074bc90
bool StreamCodec::Update()
{
    return StreamNode::UpdateNode(true);
}
}
