#pragma once

#include <stddef.h>

namespace Snd
{

class BufferIO
{
public:

    BufferIO();
    ~BufferIO();

    void* GetBuffer() const;

    void* GetData() const;

    unsigned int GetDataLeft() const;
    unsigned int GetOffset() const;
    void Reset();
    void SetBuffer(void* const pBuffer, unsigned int uiSize);
    bool UpdateOffset(unsigned int uiAmount);
    unsigned int Write(const void* pSrc, unsigned int uiSize);

    void* mBuffer;
    unsigned int mSize;
    unsigned int mOffset;
};

 

 
 
 

}
