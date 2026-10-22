#pragma once

#include <stddef.h>

#include "sndgsm.h"
#include "sndstreamcodec.h"

namespace Snd
{

class StreamGSMDecoder : public StreamCodec
{
public:
    StreamGSMDecoder();

    virtual ~StreamGSMDecoder() {}
    virtual StreamPacket* GetPacket(unsigned int uiOutput);
    void Flush();
    void Update();
    void SetPackets(unsigned int uiPacketSize, StreamPacket* pPackets,
                    unsigned int uiNumPackets);

    GSM mState;
    StreamPacket* mInputPacket;
    unsigned int mInputPacketOffset;
    unsigned int mInputPacketSize;
    unsigned int mOutputPacketOffset;
    short (*mFrame)[160];
    unsigned int mFrameOffset;
    bool mDecimate : 1;
};

 
 
 
 
 

}
