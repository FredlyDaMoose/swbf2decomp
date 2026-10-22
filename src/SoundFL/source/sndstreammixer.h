#pragma once

#include <stddef.h>

#include "sndstreampacketio.h"
#include "sndstreamsourceproducer.h"

namespace Snd
{

class StreamMixer : public StreamSourceProducer
{
public:
    StreamMixer();
    virtual ~StreamMixer();

    enum Flags
    {
        flags_Enable = 1,
        flags_Sync = 2,
        flags_CPS = 4,
        flags_SyncDisc = 8,
        flags_Discon = 16,
        flags_LastPck = 32,
        flags_StopOnLast = 64
    };

    void Discontinuity(unsigned int uiInput, bool bFlush);
    void SetChannels(unsigned char ucIn, unsigned char ucOut);
    void SetConstantOutputPacketSize(bool bConstant);
    void SetGain(unsigned int uiInput, unsigned int uiChannel,
                 float fGain, bool bImmediate);
    void SetGainArray(int* pGains);
    void SetMixBuffer(int* const pBuffer);
    void SetPacketArray(StreamPacketIO* const pIO,
                        unsigned int* const puOffsets);
    void SetLastPacketInfo(unsigned char uc);
    void SetStopOnLast(unsigned char uc);
    void SetSynchronizedActivation(bool b);
    void SetSynchronizedDeactivation(bool b);
    void SetSynchronizedMixing(bool b);

    void Flush();

    void SetOutputGain(float fGain);
    void FlushInput(unsigned int uiInput);

    virtual StreamPacket* GetPacket(unsigned int uiOutput);
    void Update();

    bool IsActive() const;

    bool HasMixedPacket(unsigned int uiInput) const;
private:

    StreamPacket* ConnectionGetPacket(unsigned int uiInput);
    void UnlockPacket(unsigned int uiInput, StreamPacket* const pPacket);
public:

    unsigned int mNode[3];
    StreamNodeSinkNInput mSink;

    unsigned char mActive[32];
    unsigned char mMixedPacket[32];
    int* mGain;
    int mOutputGain;
    StreamPacketIO* mInputPackets;
    unsigned int* mMixBufferOffsets;
    int* mMixBuffer;
    unsigned int mMixBufferSize;
    unsigned int mMixBufferDataSize;
    unsigned char mSynchronizedMixing;
    unsigned char mFlags;
    unsigned char mInputChannels;
    unsigned char mOutputChannels;
    unsigned char mInterpolationRate;
};

 
 
 
 
 
 

}
