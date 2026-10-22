#pragma once

#include "sndstreamnode.h"
#include "sndstreampacket.h"

namespace Snd
{

class SubstreamSplitter : public StreamNodeSource,
                          public StreamNode,
                          public StreamNodeSink
{
public:
    SubstreamSplitter();

    // FUNCTION: SWBF2 0x0074ae90
    virtual ~SubstreamSplitter() {}

    virtual StreamPacket* GetPacket(unsigned int uiSubstream);
    void Flush();
    void Update();

    void Setup(unsigned int uiNumSubstreams, unsigned int uiSubstreamInterleave,
               StreamPacket* pPackets, unsigned short* pReadPos,
               unsigned short* pLockPos, unsigned int uiNumPackets);

protected:
    bool IsQueueFull() const;

private:

    static void __cdecl PacketCompleteCallback(StreamPacket* const pPacket,
                                               void* const pData);

public:

    unsigned int mNumPackets;
    unsigned int mPacketsPerSubstream;
    StreamPacket* mPackets;
    unsigned int mNumSubstreams;
    unsigned int mSubstreamInterleave;
    unsigned int mSubstreamBlockSize;
    unsigned short* mReadPos;
    unsigned short* mLockPos;
};

 

}
