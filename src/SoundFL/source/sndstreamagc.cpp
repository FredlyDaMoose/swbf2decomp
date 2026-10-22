#include <new>
#include <stdlib.h>

#include "sndstreamagc.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00741da0
StreamAGC::StreamAGC()
    : mClipThreshold(0x7FFF),
      mGateThreshold(0),
      mGainDest(0x7FFF),
      mGain(0x7FFF),
      mGainMax(0x7FFF),
      mShiftRateMask(0xFFFFFFF8u),
      mShiftRate(4)
{
}

// FUNCTION: SWBF2 0x00741e00
StreamAGC::~StreamAGC()
{
}

// FUNCTION: SWBF2 0x00741e10
void* __fastcall sub_741E10(void* pAGC, int, unsigned int uiFlags)
{
    ((StreamAGC*)pAGC)->StreamAGC::~StreamAGC();
    if ((uiFlags & 1) != 0)
    {
        ::operator delete(pAGC);
    }
    return pAGC;
}

}

// FUNCTION: SWBF2 0x00741e40
void Snd::StreamAGC::Flush()
{
    void* pAGC = (void*)this;
    Snd::StreamAGC* const pThis = (Snd::StreamAGC*)pAGC;
    pThis->mGain = pThis->mGainDest;
}

namespace Snd
{

// FUNCTION: SWBF2 0x00741e50
StreamPacket* StreamAGC::GetPacket(unsigned int uiOutput)
{
    typedef StreamPacket* (__thiscall* GetPacketFn)(StreamNodeSource* pThis,
                                                    unsigned int uiOutput);
    (void)uiOutput;

    if (GetCurrentPlayState() != playState_playing)
    {
        return 0;
    }

    StreamNodeSource* const pNode = mConnection.mInputNode;
    if (pNode == 0)
    {
        return 0;
    }
    StreamPacket* const pPacket = pNode->GetPacket(mConnection.mInputNodeOutputIndex);
    if (pPacket == 0)
    {
        return 0;
    }

    short* const pSamples = (short*)pPacket->mBuffer;
    const unsigned int uiNumSamples = pPacket->mBufferUsed >> 1;
    mPlaybackPosition = pPacket->mPos;

    for (unsigned int i = 0; i < uiNumSamples; i++)
    {
        const unsigned short uLevel = (unsigned short)abs(pSamples[i]);
        unsigned __int64 uValue = mGain * uLevel;

        if (uLevel > mGateThreshold)
        {
            mGainDest = (mClipThreshold * 0x7FFFu) / uLevel;
        }
        else
        {
            mGainDest = 0;
        }
        if (mGainDest > mGainMax)
        {
            mGainDest = mGainMax;
        }

        uValue >>= 16;
        if (uValue > 0x7FFF)
        {
            uValue = 0x7FFF;
        }
        if (pSamples[i] < 0)
        {
            uValue = -(int)uValue;
        }
        pSamples[i] = (short)uValue;

        if (mGainDest < mGain)
        {
            const unsigned int uDelta = mGain - mGainDest;
            if ((uDelta & mShiftRateMask) != 0)
            {
                mGain = mGain - (uDelta >> mShiftRate) - 1;
            }
            else
            {
                mGain = mGainDest;
            }
        }
        else if (mGainDest > mGain)
        {
            const unsigned int uDelta = mGainDest - mGain;
            if ((uDelta & mShiftRateMask) != 0)
            {
                mGain = mGain + (uDelta >> mShiftRate) + 1;
            }
            else
            {
                mGain = mGainDest;
            }
        }
    }

    if ((pPacket->mFlags & 1) != 0)
    {
        Stop();
    }

    return pPacket;
}

// FUNCTION: SWBF2 0x00741fa0
void StreamAGC::Update()
{
    if (StreamNode::UpdateNode(false))
    {
        mGain = mGainDest;
    }
}

}
