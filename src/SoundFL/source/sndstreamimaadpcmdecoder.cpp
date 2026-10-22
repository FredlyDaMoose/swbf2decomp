#include "sndstreamimaadpcmdecoder.h"
#include <string.h>

namespace Snd
{

// FUNCTION: SWBF2 0x0074a3b0
StreamIMAADPCMDecoder::StreamIMAADPCMDecoder()
    : mNumChannels(0),
      mNextPos(0)
{

    StreamNodeSink::Connect(0, 0);
}

}

namespace Snd
{

// GLOBAL: SWBF2 0x007e3a50
char StreamIMAADPCMDecoder::smAsNextStep[16] =
{
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

// GLOBAL: SWBF2 0x007e3a60
short StreamIMAADPCMDecoder::smAsStep[89] =
{
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

bool StreamIMAADPCMDecoder::ValidateStepIndex(unsigned char uStepIndex)
{
    return uStepIndex < 89;
}

short StreamIMAADPCMDecoder::DecodeImaAdpcmSample(unsigned char uNibble, short sPredictor,
                                                  short sStepIndex)
{
    const int iStep = smAsStep[(char)sStepIndex];
    int iDiff = iStep >> 3;
    if (uNibble & 4)
    {
        iDiff += iStep;
    }
    if (uNibble & 2)
    {
        iDiff += iStep >> 1;
    }
    if (uNibble & 1)
    {
        iDiff += iStep >> 2;
    }
    if (uNibble & 8)
    {
        iDiff = -iDiff;
    }

    int iSample = sPredictor + iDiff;
    if (iSample < -32768)
    {
        iSample = -32768;
    }
    else if (iSample > 32767)
    {
        iSample = 32767;
    }
    return (short)iSample;
}

char StreamIMAADPCMDecoder::GetNextStepIndex(unsigned char uNibble, short sStepIndex)
{
    if (!ValidateStepIndex((unsigned char)sStepIndex))
    {
        return 0;
    }

    const short sNext = (short)(sStepIndex + smAsNextStep[uNibble]);
    if (sNext < 0)
    {
        return 0;
    }
    if (sNext >= 89)
    {
        return 88;
    }
    return (char)sNext;
}

// FUNCTION: SWBF2 0x0074a570
void StreamIMAADPCMDecoder::DecodeImaAdpcmFrame(unsigned char const* pSrc, short* pDst)
{
    const int iHeader = *(const int*)pSrc;
    short sPredictor = (short)iHeader;
    char cStepIndex = (char)((iHeader >> 16) & ~0x80);
    pSrc += 4;

    if (!ValidateStepIndex((unsigned char)cStepIndex))
    {
        memset(pDst, 0, 64 * sizeof(short));
        return;
    }

    *pDst++ = sPredictor;

    char cLeft = 63;
    for (;;)
    {
        const unsigned char uByte = *pSrc++;

        unsigned char uNibble = uByte & 0xf;
        sPredictor = DecodeImaAdpcmSample(uNibble, sPredictor, cStepIndex);
        cStepIndex = GetNextStepIndex(uNibble, cStepIndex);
        *pDst++ = sPredictor;
        if (--cLeft == 0)
        {
            break;
        }

        uNibble = uByte >> 4;
        sPredictor = DecodeImaAdpcmSample(uNibble, sPredictor, cStepIndex);
        cStepIndex = GetNextStepIndex(uNibble, cStepIndex);
        *pDst++ = sPredictor;
        if (--cLeft == 0)
        {
            break;
        }
    }
}

// FUNCTION: SWBF2 0x0074a740
void StreamIMAADPCMDecoder::DecodeImaAdpcmFrameInterleaved(unsigned char const* pSrc, short* pDst)
{
    if (mNumChannels == 1)
    {
        DecodeImaAdpcmFrame(pSrc, pDst);
        return;
    }

    for (unsigned char uChannel = 0; uChannel < mNumChannels; ++uChannel)
    {
        unsigned int auFrame[9];
        const unsigned int* puIn = (const unsigned int*)pSrc + uChannel;
        for (unsigned int i = 0; i < 9; ++i)
        {
            auFrame[i] = *puIn;
            puIn += mNumChannels;
        }

        short asSamples[64];
        DecodeImaAdpcmFrame((unsigned char const*)auFrame, asSamples);

        unsigned int uiOut = uChannel;
        for (unsigned int i = 0; i < 64; ++i)
        {
            pDst[uiOut] = asSamples[i];
            uiOut += mNumChannels;
        }
    }
}

// SYNTHETIC: SWBF2 0x0074a4e0
// Snd::StreamIMAADPCMDecoder::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0074aa80
void StreamIMAADPCMDecoder::Flush()
{
    StartFlush();
    mInputPacketIO.Flush();
    mInputBufferIO.Reset();
    mOutputPacketIO.Reset();
    StreamSourceProducer::Flush();
    mNextPos = 0;
    EndFlush();
}

// FUNCTION: SWBF2 0x0074aad0
StreamPacket* StreamIMAADPCMDecoder::GetPacket(unsigned int uiOutput)
{
    StreamPacket* pPacket = StreamCodec::GetPacket(uiOutput);
    if (pPacket == 0 && !IsFlushing())
    {
        Update();
        pPacket = StreamCodec::GetPacket(uiOutput);
    }
    return pPacket;
}

// FUNCTION: SWBF2 0x0074ab60
void StreamIMAADPCMDecoder::Update()
{
    if (StreamCodec::Update())
    {
        Flush();
    }

    if (!BufferPackets())
    {
        return;
    }

    StreamPacket* pOutput = GetReadPacket();
    StreamPacket* pInput = mInputPacketIO.GetPacket();
    while (pOutput != 0)
    {
        if (pInput == 0)
        {
            pInput = ConnectionGetPacket();
            if (pInput == 0)
            {
                return;
            }

            if (mNextPos != pInput->mPos && !pInput->GetDiscontinuity() &&
                !pInput->GetLast())
            {
                pInput->Unlock();
                Flush();
                return;
            }

            mNextPos = pInput->mPos + pInput->mBufferUsed;
            if (pInput->GetDiscontinuity() || pInput->GetLast())
            {
                mNextPos = 0;
            }
            mInputPacketIO.SetPacket(pInput, pInput->mBufferUsed);
        }

        while (pOutput != 0 && pInput != 0)
        {
            void* const pSamples = mOutputPacketIO.GetData(pOutput);
            const unsigned int uiFrameSize = mNumChannels * 36;
            mInputBufferIO.GetOffset();
            bool bLast = false;
            bool bDiscontinuity = false;

            mInputPacketIO.GetDataLeft(mInputPacketIO.GetPacketSize());
            const unsigned int uiCopy = mInputBufferIO.ClampToDataLeft(
                uiFrameSize, mInputPacketIO.GetDataLeft(mInputPacketIO.GetPacketSize()));
            memcpy(mInputBuffer + mInputBufferIO.GetOffset(),
                   mInputPacketIO.GetData(), uiCopy);
            mInputPacketIO.UpdateOffset(uiCopy);

            if (mInputBufferIO.UpdateOffset(uiFrameSize, uiCopy))
            {
                mInputBufferIO.Reset();

                const unsigned char* const pFrame = mInputBuffer;
                if (pFrame != 0)
                {
                    DecodeImaAdpcmFrameInterleaved(pFrame, (short*)pSamples);
                    mOutputPacketIO.UpdateOffset(mProducer[2], mNumChannels << 7);
                }
            }

            if (mInputPacketIO.GetDataLeft(mInputPacketIO.GetPacketSize()) == 0)
            {
                bLast = pInput->GetLast();
                bDiscontinuity = pInput->GetDiscontinuity();
                mInputPacketIO.Flush();
                pInput = 0;
                if (bLast || bDiscontinuity)
                {
                    mNextPos = 0;
                    mInputBufferIO.Reset();
                }
            }

            if (mOutputPacketIO.GetDataLeft(mProducer[2]) == 0 || bLast ||
                bDiscontinuity)
            {
                const unsigned int uiBytes = mOutputPacketIO.GetOffset();
                const unsigned int uiPos = mPlaybackPosition;
                const unsigned int uiSamples = uiBytes / (mNumChannels * 2);
                PacketComplete(pOutput, uiBytes, uiSamples, bLast,
                               bDiscontinuity, uiPos);
                mPlaybackPosition = uiPos + uiSamples;
                mOutputPacketIO.Reset();
                pOutput = 0;
            }
        }

        pOutput = GetReadPacket();
        pInput = mInputPacketIO.GetPacket();
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0074a460
StreamIMAADPCMDecoder::~StreamIMAADPCMDecoder()
{
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0074ab40
void StreamIMAADPCMDecoder::SetPackets(unsigned int uiPacketSize, StreamPacket* pPackets,
                                       unsigned int uiNumPackets, unsigned int uiNumChannels)
{
    mNumChannels = uiNumChannels;
    StreamSourceProducer::SetPackets(uiPacketSize, pPackets, uiNumPackets);
}
}
