#pragma once

#include <stddef.h>

#include "SoundFL/source/sndbufferio.h"
#include "SoundFL/source/sndstreampacketio.h"
#include "SoundFL/source/sndstreamnode.h"
#include "SoundFL/source/sndstreamsourceproducer.h"
#include "snddsdmo.h"

namespace Snd
{

class StreamFormat;

class StreamDSDMO : public StreamSourceProducer
{
public:
    StreamDSDMO();
    virtual ~StreamDSDMO();

    virtual StreamPacket* GetPacket(unsigned int uiOutput);
    bool CreateDMO(const _GUID& rCLSID, const StreamFormat& rIn,
                   const StreamFormat& rOut);
    unsigned int GetInputAligment() const;
    unsigned int GetMinInputSize() const;
    void Flush();

    void SetInputBuffer(void* pBuffer, unsigned int uiSize);

    void Update();

    StreamNode mNode;
    StreamNodeSink mSink;
    DSDMO mDMO;

    StreamPacketIO mInputPacket;
    BufferIO mBuffer;
    StreamPacketIOLite mOutputPacket;
    unsigned int mAlignment;
    unsigned int mMinSize;
    unsigned int mAudioFrameSize;
    unsigned int mInputFrameSize;
    unsigned char mFlags;
};

 
 
 
 
 
 
 
 

}
