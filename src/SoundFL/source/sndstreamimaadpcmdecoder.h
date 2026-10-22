#pragma once

#include "sndstreamcodec.h"
#include "sndstreampacketio.h"

namespace Snd
{

class StreamIMAADPCMDecoder : public StreamCodec
{
public:
    StreamIMAADPCMDecoder();

    virtual ~StreamIMAADPCMDecoder();

    virtual StreamPacket* GetPacket(unsigned int uiOutput);

    void Flush();
    void Update();

    void SetPackets(unsigned int uiPacketSize, StreamPacket* pPackets,
                    unsigned int uiNumPackets, unsigned int uiNumChannels);

protected:

    void DecodeImaAdpcmFrame(unsigned char const* pSrc, short* pDst);
    void DecodeImaAdpcmFrameInterleaved(unsigned char const* pSrc, short* pDst);
    short DecodeImaAdpcmSample(unsigned char uNibble, short sPredictor, short sStepIndex);
    char GetNextStepIndex(unsigned char uNibble, short sStepIndex);
    static bool ValidateStepIndex(unsigned char uStepIndex);

    static char smAsNextStep[16];
    static short smAsStep[89];

public:
    StreamPacketIOHeavy mInputPacketIO;
    StreamPacketIOLite mOutputPacketIO;
    unsigned char mInputBuffer[72];
    StreamPacketIOLite mInputBufferIO;
    unsigned int mNumChannels;
    unsigned int mNextPos;
};

 

}
