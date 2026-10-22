#include <new>
#include <string.h>

#include "sndstreamresampler.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074cb60
StreamResampler::StreamResampler()
    : mBufferSize(0),
      mStepSize(0x10000),
      mInputPacket(0),
      mInputPacketReadPos(0),
      mSampleOffset(0),
      mPrevSample(0)
{

    StreamNodeSink::Connect(0, 0);
}

// FUNCTION: SWBF2 0x0074ceb0
void StreamResampler::SetBuffer(void* pBuffer, unsigned int uiSize)
{
    mBufferSize = uiSize;
    mOutputPacket.mBuffer = pBuffer;
    mOutputPacket.mBufferUsed = 0;
}

// FUNCTION: SWBF2 0x0074cbe0
void* __fastcall sub_74CBE0(void* pResampler, int, unsigned int uiFlags)
{
    ((StreamResampler*)pResampler)->StreamResampler::~StreamResampler();
    if ((uiFlags & 1) != 0)
    {
        ::operator delete(pResampler);
    }
    return pResampler;
}

// FUNCTION: SWBF2 0x0074cc10
void StreamResampler::CompletePacket()
{
    bool bLast = false;
    bool bDiscontinuity = false;
    StreamPacket* const pInput = mInputPacket;
    unsigned int uiInputSamples = pInput->mBufferUsed >> 1;

    if (mOutputPacket.mBufferUsed != 0)
    {
        mOutputPacket.Lock();
    }
    mOutputPacket.mPos = pInput->mPos;

    if (mInputPacketReadPos >= uiInputSamples)
    {
        bLast = pInput->GetLast();
        bDiscontinuity = pInput->GetDiscontinuity();
        mInputPacketReadPos -= uiInputSamples;
        pInput->Unlock();
        mInputPacket = 0;
    }

    if (bLast)
    {
        mOutputPacket.mFlags |= 1;
    }
    else
    {
        mOutputPacket.mFlags &= ~1;
    }

    if (bDiscontinuity)
    {
        mOutputPacket.mFlags |= 2;
    }
    else
    {
        mOutputPacket.mFlags &= ~2;
    }
}

// FUNCTION: SWBF2 0x0074ccc0
void StreamResampler::Flush()
{
    mPrevSample = 0;
    mSampleOffset = 0;
    if (mInputPacket != 0)
    {
        if (mInputPacket->mRefCount > 0)
        {
            mInputPacket->Unlock();
        }
        mInputPacket = 0;
        mInputPacketReadPos = 0;
    }
}

// FUNCTION: SWBF2 0x0074cd30
StreamPacket* StreamResampler::GetPacket(unsigned int)
{
    if (mStepSize == 0x10000)
    {
        StreamPacket* const pInput = mInputPacket;
        if (pInput != 0)
        {
            if (mOutputPacket.mRefCount > 0)
            {
                return 0;
            }
            unsigned int uiCount = (pInput->mBufferUsed >> 1) - mInputPacketReadPos;
            if (uiCount > mBufferSize >> 1)
            {
                uiCount = mBufferSize >> 1;
            }
            memcpy(mOutputPacket.mBuffer, (short*)pInput->mBuffer + mInputPacketReadPos, uiCount * 2);
            mInputPacketReadPos += uiCount;
            mOutputPacket.mBufferUsed = uiCount;
            CompletePacket();
            if (mOutputPacket.mRefCount > 0)
            {
                return &mOutputPacket;
            }
        }
        return ConnectionGetPacket();
    }

    if (mStepSize == 0 || mOutputPacket.mRefCount > 0)
    {
        return 0;
    }

    StreamPacket* pInput = mInputPacket;
    if (pInput == 0)
    {
        pInput = ConnectionGetPacket();
        if (pInput == 0)
        {
            return 0;
        }
    }

    if (mInputPacketReadPos < pInput->mBufferUsed)
    {
        const short* pSrc = (const short*)pInput->mBuffer + mInputPacketReadPos;
        const short* const pSrcEnd = (const short*)((const char*)pInput->mBuffer + pInput->mBufferUsed);
        short* pDst = (short*)mOutputPacket.mBuffer;
        short* const pDstEnd = (short*)((char*)pDst + mBufferSize);
        while (pSrc < pSrcEnd && pDst < pDstEnd)
        {
            *pDst++ = (short)(((*pSrc * mSampleOffset) >> 16) + (((0xFFFF - mSampleOffset) * mPrevSample) >> 16));
            mSampleOffset += mStepSize;
            if (mSampleOffset & 0xFFFF0000)
            {
                int iAdvance = mSampleOffset >> 16;
                mPrevSample = *pSrc;
                mSampleOffset &= 0xFFFF;
                pSrc += iAdvance;
            }
        }
        mOutputPacket.mBufferUsed = (char*)pDst - (char*)mOutputPacket.mBuffer;
        mInputPacketReadPos = pSrc - (const short*)pInput->mBuffer;
    }
    else
    {
        mInputPacketReadPos -= pInput->mBufferUsed;
    }

    mInputPacket = pInput;
    CompletePacket();
    if (mOutputPacket.mRefCount > 0)
    {
        return &mOutputPacket;
    }
    return 0;
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0074ced0
void StreamResampler::Update()
{
    UpdateNode(false);
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0074cbd0
StreamResampler::~StreamResampler()
{

}
}
