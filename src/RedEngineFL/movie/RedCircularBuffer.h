#pragma once

class RedCircularBuffer
{
public:

    // FUNCTION: SWBF2 0x006f86c0
    RedCircularBuffer();

    ~RedCircularBuffer();

    void* GetBuffer() const;
    RedCircularBuffer& operator=(const RedCircularBuffer& other);
    unsigned int GetUsed() const;
    unsigned int GetFree() const;
    unsigned int GetFreeAligned(unsigned int uiAlign) const;
    unsigned int GetFreeBlock() const;
    unsigned int GetWriteOffset() const;
    void* GetWritePos() const;
    void UpdateWritePos(unsigned int uiSize);
    unsigned int ReadData(void* pDest, unsigned int uiSize, bool bUpdate);
    void SetBuffer(void* pBuffer, unsigned int uiSize);
    void UpdateReadPos(unsigned int uiSize);
    void Flush();

    unsigned char* mBuffer;
    unsigned int mPos;
    unsigned int mUsed;
    unsigned int mSize;
};

 
