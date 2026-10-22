#pragma once

#include <stddef.h>

#include "sndstreamnode.h"
#include "sndstreamsourceproducer.h"
#include "win/sndsoftoutput.h"

namespace Snd
{

class StreamSourceMemory : public StreamSourceProducer, public StreamNode
{
public:

    class SegmentProperties
    {
    public:
        unsigned int mOffset;
        unsigned int mLength;
    };

    StreamSourceMemory();
    virtual ~StreamSourceMemory();

    virtual StreamPacket* GetPacket(unsigned int uiOutput);

    void Update();

    void Setup(const StreamFormat& rFormat, void* const pData, unsigned int uiDataSize,
               unsigned int uiPacketSize, StreamPacket* pPackets, unsigned char uiNumPackets,
               bool bLooping);
    void SetLooping(bool bLooping);
    void Flush()
    {
        StartFlush();
        StreamSourceProducer::Flush();
        EndFlush();
    }

private:
    StreamPacket* GetPacket(unsigned int uiOutput, bool bUpdate);

public:

    SegmentProperties mSegment;
    SegmentProperties mNextSegment;
    void* mData;
    unsigned int mDataSize;
    unsigned int mAudioFrameSize;
    StreamFormat mFormat;
    unsigned char mLooping;
};

 
 
 
 
 

}
