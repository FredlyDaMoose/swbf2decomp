#pragma once

#include <string.h>

namespace Snd
{

// VTABLE: SWBF2 0x00785cc0
class FilePlatformBase
{
public:

    class Buffer
    {
    public:

        Buffer()
        {
            mBuffer = 0;
            mSize = 0;
            mUsed = 0;
            mOffset = 0;
        }

        void Init(void* const pBuffer, unsigned int uiSize)
        {
            mSize = uiSize;
            mUsed = 0;
            mOffset = 0;
            mBuffer = pBuffer;
        }

        void Flush()
        {
            mUsed = 0;
            mOffset = 0;
        }

        void* GetBuffer() const { return mBuffer; }
        unsigned int GetSize() const { return mSize; }
        unsigned int GetUsed() const { return mUsed; }
        unsigned int GetReadOffset() const { return mOffset; }

        void* GetWritePos() const { return (char*)mBuffer + mUsed; }

        void UpdateWritePos(unsigned int uiSize) { mUsed += uiSize; }

        bool IsFull() const { return mUsed == mSize; }

        bool IsEmpty() const { return mUsed == 0 || mOffset == mUsed; }

        unsigned int GetReadLeft() const { return mUsed - mOffset; }
        unsigned int GetWriteLeft() const { return mSize - mUsed; }

        unsigned int Read(void* const pDest, unsigned int uiSize)
        {
            unsigned int uiLeft = GetReadLeft();
            if (uiSize > uiLeft)
            {
                uiSize = uiLeft;
            }
            if (pDest)
            {
                memcpy(pDest, (char*)mBuffer + mOffset, uiSize);
            }
            mOffset += uiSize;
            return uiSize;
        }

        unsigned int Write(void* const pSrc, unsigned int uiSize)
        {
            unsigned int uiLeft = GetWriteLeft();
            if (uiSize > uiLeft)
            {
                uiSize = uiLeft;
            }
            memcpy((char*)mBuffer + mUsed, pSrc, uiSize);
            mUsed += uiSize;
            return uiSize;
        }

        void* GetReadPos() const;
        void SetReadPos(unsigned int uiPos);
        void UpdateReadPos(unsigned int uiSize);
        unsigned int GetWriteOffset() const;

        void* mBuffer;
        unsigned int mSize;
        unsigned int mUsed;
        unsigned int mOffset;
    };

    FilePlatformBase();
    virtual ~FilePlatformBase();

    // SYNTHETIC: SWBF2 0x0072ff40
    // Snd::FilePlatformBase::`scalar deleting destructor'

    FilePlatformBase& operator=(FilePlatformBase& rOther);

    bool Open(char const* pcFileName, unsigned int uiAccessFlags, unsigned int uiBufferSize,
              unsigned int uiSectorSize, unsigned int uiAlignment);
    void Close();
    void Flush();
    unsigned int GetPos() const;
    bool IsDataBuffered(unsigned int uiPos, unsigned int uiSize);
    bool IsOpen() const;
    unsigned int Read(void* pBuffer, unsigned int uiSize);
    unsigned int Write(void* pBuffer, unsigned int uiSize);
    void Seek(unsigned int uiPos);

    virtual unsigned int GetSize() const = 0;

protected:
    virtual void RealSeek(unsigned int uiPos) = 0;
    virtual unsigned int RealRead(void* pBuffer, unsigned int uiSize) = 0;
    virtual unsigned int RealWrite(void* pBuffer, unsigned int uiSize) = 0;

    unsigned int RealGetPos() const { return mPos; }

    void Seek(unsigned int uiPos, bool bFlush)
    {
        if (uiPos >= mPos + mBuffer.GetUsed() || uiPos < mPos || (mFlags & 4))
        {
            if (bFlush)
            {
                Flush();
            }
            mPos = uiPos & ~(mSectorSize - 1);
            mFlags |= 2;
        }
        mBuffer.mOffset = uiPos - mPos;
    }

    void CommitSeek(bool bClear)
    {
        if (mFlags & 2)
        {
            RealSeek(mPos);
            if (bClear)
            {
                mFlags &= ~2;
            }
        }
    }

    unsigned int SeekRead(void* pBuffer, unsigned int uiSize)
    {
        CommitSeek(true);
        return RealRead(pBuffer, uiSize);
    }

    unsigned int SeekWrite(void* pBuffer, unsigned int uiSize)
    {
        CommitSeek(true);
        return RealWrite(pBuffer, uiSize);
    }

private:
    unsigned int BufferedReadWrite(void* pBuffer, unsigned int uiSize, bool bRead);

public:
    unsigned char mAccess;
    unsigned char mFlags;
    void* mBufferAlloc;
    Buffer mBuffer;
    unsigned int mSectorSize;
    unsigned int mAlignment;
    unsigned int mPos;
};
 

}
