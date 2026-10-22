#include "sndstreampacketio.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0073ce70
unsigned int StreamPacketIO::GetOffset() const
{
    return mOffset;
}

// FUNCTION: SWBF2 0x0073cc00
StreamPacketIOLite::StreamPacketIOLite()
    : mOffset(0)
{
}

// FUNCTION: SWBF2 0x0073cd30
StreamPacketIOLite::~StreamPacketIOLite()
{
    mOffset = 0;
}

// FUNCTION: SWBF2 0x0073cec0
void StreamPacketIOLite::Reset()
{
    mOffset = 0;
}

// FUNCTION: SWBF2 0x0073ce80
unsigned int StreamPacketIOLite::GetOffset() const
{
    return mOffset;
}

// FUNCTION: SWBF2 0x0073ce30
void* StreamPacketIOLite::GetData(const StreamPacket* const pPacket) const
{
    return (char*)pPacket->mBuffer + mOffset;
}

// FUNCTION: SWBF2 0x0073ce60
unsigned int StreamPacketIOLite::GetDataLeft(unsigned int uiDataSize) const
{
    return uiDataSize - mOffset;
}

// FUNCTION: SWBF2 0x0073d020
bool StreamPacketIOLite::UpdateOffset(unsigned int uiDataSize,
                                      unsigned int uiAmount)
{
    mOffset += uiAmount;
    return mOffset == uiDataSize;
}

// FUNCTION: SWBF2 0x0073cbe0
StreamPacketIOHeavy::StreamPacketIOHeavy()
    : mPacketSize(0)
{
}

// FUNCTION: SWBF2 0x0073cca0
StreamPacketIOHeavy::~StreamPacketIOHeavy()
{
}

// FUNCTION: SWBF2 0x0073cbd0
StreamPacketIO::StreamPacketIO()
    : mPacket(0)
{
}

// FUNCTION: SWBF2 0x0073cc10
StreamPacketIO::~StreamPacketIO()
{
    if (mPacket != 0)
    {
        mPacket->Unlock();
        mPacket = 0;
        mOffset = 0;
    }
}

// FUNCTION: SWBF2 0x0073cd60
void StreamPacketIO::Flush()
{
    if (mPacket == 0)
    {
        return;
    }

    mPacket->Unlock();
    mPacket = 0;
    mOffset = 0;
}

// FUNCTION: SWBF2 0x0073ce10
void* StreamPacketIO::GetData() const
{
    return (char*)mPacket->mBuffer + mOffset;
}

// FUNCTION: SWBF2 0x0073ce40
unsigned int StreamPacketIO::GetDataLeft(unsigned int uiDataSize) const
{
    return uiDataSize - mOffset;
}

// FUNCTION: SWBF2 0x0073ce90
StreamPacket* StreamPacketIO::GetPacket() const
{
    return mPacket;
}

// FUNCTION: SWBF2 0x0073ced0
void StreamPacketIO::SetPacket(StreamPacket* const pPacket)
{
    Flush();
    mPacket = pPacket;
}

// FUNCTION: SWBF2 0x0073cfe0
bool StreamPacketIO::UpdateOffset(unsigned int uiDataSize,
                                  unsigned int uiAmount)
{
    mOffset += uiAmount;
    return mOffset == uiDataSize;
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0073cd40
unsigned int StreamPacketIOLite::ClampToDataLeft(unsigned int uiDataSize, unsigned int uiAmount)
{
    unsigned int uiDataLeft = uiDataSize - mOffset;
    if (uiAmount > uiDataLeft)
    {
        uiAmount = uiDataLeft;
    }
    return uiAmount;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0073cdb0
void StreamPacketIOHeavy::Flush()
{
    StreamPacketIO::Flush();
    mPacketSize = 0;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0073ce20
void* StreamPacketIOHeavy::GetData() const
{
    return (char*)mPacket->mBuffer + mOffset;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0073ce50
unsigned int StreamPacketIOHeavy::GetDataLeft(unsigned int uiDataSize) const
{
    return mPacketSize - mOffset;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0073cea0
StreamPacket* StreamPacketIOHeavy::GetPacket() const
{
    return mPacket;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0073ceb0
unsigned int StreamPacketIOHeavy::GetPacketSize() const
{
    return mPacketSize;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0073cf30
void StreamPacketIOHeavy::SetPacket(StreamPacket* const pPacket, unsigned int uiPacketSize)
{
    Flush();
    StreamPacketIO::SetPacket(pPacket);
    mPacketSize = uiPacketSize;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0073d000
bool StreamPacketIOHeavy::UpdateOffset(unsigned int uiAmount)
{
    mOffset += uiAmount;
    return mOffset == mPacketSize;
}
}
