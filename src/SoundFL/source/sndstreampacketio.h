#pragma once

#include <stddef.h>
#include "sndstreampacket.h"

namespace Snd
{

class StreamPacketIOLite
{
public:
    StreamPacketIOLite();
    ~StreamPacketIOLite();

    void Reset();
    void* GetData(const StreamPacket* const pPacket) const;
    unsigned int GetDataLeft(unsigned int uiDataSize) const;
    bool UpdateOffset(unsigned int uiDataSize,
                      unsigned int uiAmount);

    unsigned int GetOffset() const;

    unsigned int ClampToDataLeft(unsigned int uiDataSize,
                                 unsigned int uiAmount);

    unsigned int mOffset;
};

class StreamPacketIO : public StreamPacketIOLite
{
public:

    StreamPacketIO();

    ~StreamPacketIO();
    void Flush();

    void* GetData() const;
    unsigned int GetDataLeft(unsigned int uiDataSize) const;
    unsigned int GetOffset() const;
    StreamPacket* GetPacket() const;
    void SetPacket(StreamPacket* const pPacket);
    bool UpdateOffset(unsigned int uiDataSize,
                      unsigned int uiAmount);

    StreamPacket* mPacket;
};

class StreamPacketIOHeavy : public StreamPacketIO
{
public:
    StreamPacketIOHeavy();
    ~StreamPacketIOHeavy();

    void Flush();

    void* GetData() const;
    unsigned int GetDataLeft(unsigned int uiDataSize) const;
    StreamPacket* GetPacket() const;
    unsigned int GetPacketSize() const;
    void SetPacket(StreamPacket* const pPacket,
                   unsigned int uiPacketSize);
    bool UpdateOffset(unsigned int uiAmount);

    unsigned int mPacketSize;
};

 
 

 
 

}
