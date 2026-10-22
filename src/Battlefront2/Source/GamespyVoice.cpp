#include "SoundFL/source/sndstreamcodec.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0073d110
void* __fastcall sub_73D110(void* pCodec, int, unsigned int uiFlags)
{
    ((StreamCodec*)pCodec)->StreamCodec::~StreamCodec();
    if (uiFlags & 1)
    {
        operator delete(pCodec);
    }
    return pCodec;
}

// FUNCTION: SWBF2 0x0073d200
StreamPacket* StreamCodec::GetPacket(unsigned int uiOutput)
{
    if (StreamNode::GetCurrentPlayState() != playState_playing)
    {
        return 0;
    }

    StreamPacket* pPacket = StreamSourceProducer::GetPacket(uiOutput);
    if (pPacket == 0)
    {
        return 0;
    }

    if (pPacket->GetLast())
    {
        StreamNode::Stop();
    }

    return pPacket;
}

}
