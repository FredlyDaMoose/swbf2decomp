#pragma once

namespace Snd
{

struct StreamPacket
{

    // FUNCTION: SWBF2 0x0053c610
    StreamPacket()
        : mPos(0),
          mBuffer(0),
          mBufferUsed(0),
          mRefCount(0),
          mFlags(0),
          mCallback(0),
          mCallbackData(0)
    {
    }

    // FUNCTION: SWBF2 0x0053c640
    ~StreamPacket() {}

    void Lock() { ++mRefCount; }

    // FUNCTION: SWBF2 0x0053d330
    void Unlock()
    {
        if (mRefCount != 0)
        {
            --mRefCount;
        }

        if (mRefCount == 0)
        {
            if (mCallback != 0)
            {
                mCallback(this, mCallbackData);
            }

            mBufferUsed = 0;
            mFlags = 0;
        }
    }

    bool GetLast() const { return (mFlags & 1) != 0; }

    bool GetDiscontinuity() const { return ((mFlags >> 1) & 1) != 0; }

    void SetDiscontinuity(bool bDiscontinuity);

    unsigned int mPos;
    void* mBuffer;
    unsigned int mBufferUsed;
    unsigned short mRefCount;
    unsigned short mFlags;
    void (__cdecl* mCallback)(StreamPacket* const,
                             void* const);
    void* mCallbackData;
};

 

}
