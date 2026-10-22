#include "sndstreamhighlowpass.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00747d40
StreamLowHighPass::StreamLowHighPass()
    : mBufferSize(0)
{
    mSink.mConnection.mInputNode = 0;
    mSink.mConnection.mInputNodeOutputIndex = 0;
}

// FUNCTION: SWBF2 0x00747e00
StreamLowHighPass::~StreamLowHighPass()
{

}

// SYNTHETIC: SWBF2 0x00747e60
// Snd::StreamLowHighPass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00747ee0
void StreamLowHighPass::Flush()
{
    mInputPacket.Flush();
}

// FUNCTION: SWBF2 0x00747ef0
StreamPacket* StreamLowHighPass::GetPacket(unsigned int uiOutput)
{
    typedef StreamPacket* (__thiscall* GetPacketFn)(StreamNodeSource* pThis,
                                                    unsigned int uiOutput);
    (void)uiOutput;

    if (mInputPacket.GetPacket() == 0)
    {

        StreamNodeSource* const pNode = mSink.mConnection.mInputNode;
        mInputPacket.SetPacket(
            pNode != 0
                ? pNode->GetPacket(mSink.mConnection.mInputNodeOutputIndex)
                : 0);
    }

    StreamPacket* const pInput = mInputPacket.GetPacket();
    if (pInput == 0 || mOutputPacket.mRefCount > 0)
    {
        return 0;
    }

    short* const pOut = (short*)mOutputPacket.mBuffer;
    unsigned int uiSize = mInputPacket.GetDataLeft(pInput->mBufferUsed);
    if (uiSize > mBufferSize)
    {
        uiSize = mBufferSize;
    }

    mFilter.Process16((const short*)mInputPacket.GetData(), uiSize >> 1, pOut);

    mOutputPacket.mBufferUsed = uiSize;
    mOutputPacket.mPos = pInput->mPos + (mInputPacket.GetOffset() >> 1);

    if (mInputPacket.UpdateOffset(pInput->mBufferUsed, uiSize))
    {
        if ((pInput->mFlags & 1) != 0)
        {
            mOutputPacket.mFlags |= 1;
        }
        else
        {
            mOutputPacket.mFlags &= ~1;
        }

        if ((pInput->mFlags & 2) != 0)
        {
            mOutputPacket.mFlags |= 2;
        }
        else
        {
            mOutputPacket.mFlags &= ~2;
        }

        mInputPacket.Flush();
    }
    else
    {
        mOutputPacket.mFlags &= ~1;
        mOutputPacket.mFlags &= ~2;
    }

    ++mOutputPacket.mRefCount;
    return &mOutputPacket;
}

// FUNCTION: SWBF2 0x00748050
void StreamLowHighPass::SetBuffer(void* pBuffer, unsigned int uiSize)
{
    mBufferSize = uiSize;
    mOutputPacket.mBuffer = pBuffer;
    mOutputPacket.mBufferUsed = 0;
}

}
