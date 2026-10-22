#include <string.h>

#include "sndbufferio.h"

namespace Snd
{

// FUNCTION: SWBF2 0x007510e0
void* BufferIO::GetBuffer() const
{
    return mBuffer;
}

// FUNCTION: SWBF2 0x007510f0
void* BufferIO::GetData() const
{

    return (void*)(mOffset + (char*)mBuffer);
}

// FUNCTION: SWBF2 0x00751100
unsigned int BufferIO::GetDataLeft() const
{
    return mSize - mOffset;
}

// FUNCTION: SWBF2 0x00751110
unsigned int BufferIO::GetOffset() const
{
    return mOffset;
}

// FUNCTION: SWBF2 0x00751120
void BufferIO::Reset()
{
    mOffset = 0;
}

// FUNCTION: SWBF2 0x00751130
void BufferIO::SetBuffer(void* const pBuffer, unsigned int uiSize)
{
    mBuffer = pBuffer;
    mOffset = 0;
    mSize = uiSize;
}

// FUNCTION: SWBF2 0x00751150
bool BufferIO::UpdateOffset(unsigned int uiAmount)
{
    mOffset += uiAmount;
    return mOffset == mSize;
}

// FUNCTION: SWBF2 0x00751170
unsigned int BufferIO::Write(const void* pSrc, unsigned int uiSize)
{

    unsigned int uiCount = uiSize;
    const unsigned int uiLeft = mSize - mOffset;
    if (uiCount > uiLeft)
    {
        uiCount = uiLeft;
    }

    memcpy((char*)mBuffer + mOffset, pSrc, uiCount);

    mOffset += uiCount;
    return uiCount;
}

}

namespace Snd {

// FUNCTION: SWBF2 0x007510b0
BufferIO::BufferIO()
    : mBuffer(NULL),
      mSize(0),
      mOffset(0)
{
}
}

namespace Snd {

// FUNCTION: SWBF2 0x007510d0
BufferIO::~BufferIO()
{
}
}
