#pragma once

#include <stddef.h>

#include "sndstreamnode.h"
#include "sndstreampacket.h"

namespace Snd
{

// VTABLE: SWBF2 0x00786a68
class StreamAGC : public StreamNodeSource,
                  public StreamNode,
                  public StreamNodeSink
{
public:
    StreamAGC();
    virtual ~StreamAGC();
    virtual StreamPacket* GetPacket(unsigned int uiOutput);
    void Update();
    void Flush();

    short mClipThreshold;
    short mGateThreshold;
    unsigned int mGainDest;
    unsigned int mGain;
    unsigned int mGainMax;
    unsigned int mShiftRateMask;
    unsigned char mShiftRate;
};
 
 
 
 
 

}
