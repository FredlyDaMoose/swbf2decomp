#pragma once

#include <stddef.h>

#include "sndstreamnode.h"
#include "sndstreampacket.h"

namespace Snd
{

class StreamSplitter : public StreamNodeSource
{
public:
    StreamSplitter();

    virtual ~StreamSplitter();

    void Setup(StreamPacket** const ppQueue, unsigned short* const puRefCounts,
               unsigned short uNumRefCounts, unsigned short* const puRemaining,
               unsigned short uNumRemaining);

    void Flush();

    virtual StreamPacket* GetPacket(unsigned int uiOutput);

    unsigned int mNode[3];
    StreamNodeSink mSink;
    StreamPacket** mPackets;
    unsigned short* mRemaining;
    unsigned short* mRefCount;
    unsigned short mNumOutputs;
    unsigned short mNumPackets;
    unsigned short mReadPos;
    unsigned short mFlushing;
};

 
 
 
 
 
 

}
