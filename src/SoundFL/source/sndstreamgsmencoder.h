#pragma once

#include <stddef.h>

#include "sndgsm.h"
#include "sndstreamcodec.h"

namespace Snd
{
namespace DSP
{
class HighLowPassFilter;
}

class StreamGSMEncoder : public StreamCodec
{
public:
    StreamGSMEncoder();
    virtual ~StreamGSMEncoder();
    virtual StreamPacket* GetPacket(unsigned int uiOutput);
    void Update();
    void Flush();
    void SetPackets(unsigned int uiPacketSize, StreamPacket* pPackets,
                    unsigned int uiNumPackets);

    GSM mState;
    short mInputFrame[160];
    StreamPacket* mInputPacket;
    unsigned int mInputPacketOffset;
    unsigned int mInputPacketSize;
    unsigned int mInputFrameOffset;
    unsigned int mOutputPacketOffset;
    DSP::HighLowPassFilter* mFilter;
    bool mDecimate : 1;
};

 
 
 
 
 

}
