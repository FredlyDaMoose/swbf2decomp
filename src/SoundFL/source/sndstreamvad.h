#pragma once

#include <stddef.h>

#include "sndqueue.h"
#include "sndstreamnode.h"
#include "sndstreampacket.h"

namespace Snd
{

// VTABLE: SWBF2 0x00786a80
class StreamVAD : public StreamNodeSource,
                  public StreamNode,
                  public StreamNodeSink
{
public:
    StreamVAD();
    virtual ~StreamVAD();
    virtual StreamPacket* GetPacket(unsigned int uiOutput);
    void Update();

    Queue<StreamPacket*, 4> mPacketQueue;
    unsigned int mSampleRate;
    unsigned int mReleaseTimeBytes;
    unsigned int mReleaseTimeBytesRemaining;
    bool mActive;
};
 
 
 
 
 

}
