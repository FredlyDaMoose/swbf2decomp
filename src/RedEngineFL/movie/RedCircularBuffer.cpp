#include "RedCircularBuffer.h"
#include <string.h>

// FUNCTION: SWBF2 0x006f86c0
RedCircularBuffer::RedCircularBuffer()
{
    mBuffer = 0;
    mPos = 0;
    mUsed = 0;
    mSize = 0;
}

// FUNCTION: SWBF2 0x006f8730
void* RedCircularBuffer::GetBuffer() const
{
    return mBuffer;
}

// FUNCTION: SWBF2 0x006f87d0
unsigned int RedCircularBuffer::GetUsed() const
{
    return mUsed;
}

// FUNCTION: SWBF2 0x006f88e0
void RedCircularBuffer::UpdateReadPos(unsigned int uiSize)
{
    mUsed -= uiSize;
    mPos += uiSize;
    if (mPos >= mSize)
    {
        mPos -= mSize;
    }
}

// FUNCTION: SWBF2 0x006f8800
unsigned int RedCircularBuffer::ReadData(void* pDest, unsigned int uiSize, bool bUpdate)
{
    unsigned int remaining = uiSize < GetUsed() ? uiSize : GetUsed();
    unsigned int first = mSize - mPos;
    if (first > remaining)
    {
        first = remaining;
    }
    unsigned int pos = mPos + first;
    if (pos == mSize)
    {
        pos = 0;
    }
    if (first)
    {
        memcpy(pDest, mBuffer + mPos, first);
        if (bUpdate)
        {
            UpdateReadPos(first);
        }
    }
    remaining -= first;
    unsigned int second = mSize - pos;
    if (second > remaining)
    {
        second = remaining;
    }
    if (second)
    {
        memcpy((unsigned char*)pDest + first, mBuffer + pos, second);
        if (bUpdate)
        {
            UpdateReadPos(second);
        }
    }
    return first + second;
}

// FUNCTION: SWBF2 0x006f86e0
RedCircularBuffer::~RedCircularBuffer()
{
}

// FUNCTION: SWBF2 0x006f8720
void RedCircularBuffer::Flush()
{
    mPos = 0;
    mUsed = 0;
}

// FUNCTION: SWBF2 0x006f88c0
void RedCircularBuffer::SetBuffer(void* pBuffer, unsigned int uiSize)
{
    mBuffer = (unsigned char*)pBuffer;
    mSize = uiSize;
    mUsed = 0;
    mPos = 0;
}
