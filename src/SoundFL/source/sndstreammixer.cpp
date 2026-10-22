#include <new>
#include <string.h>

#include "sndstreamsourceproducer.h"

#include "win/sndsoftoutput.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0073f760
StreamMixer::StreamMixer()
{
    char* const pThis = (char*)this;

    *(unsigned int*)(pThis + 0x018) = 0;
    *(unsigned short*)(pThis + 0x01C) = 0x40;
    pThis[0x020] = 0;

    *(unsigned int*)(pThis + 0x06C) = 0;
    *(unsigned int*)(pThis + 0x070) = 0x7FFF;
    *(unsigned int*)(pThis + 0x074) = 0;
    *(unsigned int*)(pThis + 0x078) = 0;
    *(unsigned int*)(pThis + 0x07C) = 0;
    *(unsigned int*)(pThis + 0x080) = 0;
    *(unsigned int*)(pThis + 0x084) = 0;
    *(unsigned int*)(pThis + 0x088) = 0x01010000u;
    pThis[0x08C] = 0x0A;

    memset(pThis + 0x02C, 0, 0x40);
}

// FUNCTION: SWBF2 0x0073f830
StreamMixer::~StreamMixer()
{
    Flush();
}

// FUNCTION: SWBF2 0x0073fb40
void StreamMixer::SetConstantOutputPacketSize(bool bConstant)
{
    char* const pThis = (char*)this;

    if (bConstant)
    {
        pThis[0x89] = (char)(pThis[0x89] | 0x04);
    }
    else
    {
        pThis[0x89] = (char)(pThis[0x89] & 0xFB);
    }
}

// FUNCTION: SWBF2 0x0073fc90
void StreamMixer::SetLastPacketInfo(unsigned char uc)
{
    char* const pThis = (char*)this;

    if (uc)
    {
        pThis[0x89] = (char)(pThis[0x89] | 0x20);
    }
    else
    {
        pThis[0x89] = (char)(pThis[0x89] & 0xDF);
    }
}

// FUNCTION: SWBF2 0x0073fcb0
void StreamMixer::SetMixBuffer(int* const pBuffer)
{
    const unsigned int uiSize = (mProducer[2] >> 1) << 2;

    memset(pBuffer, 0, uiSize);

    mMixBuffer = pBuffer;
    mMixBufferSize = uiSize;
    mMixBufferDataSize = 0;
}

// FUNCTION: SWBF2 0x0073fd30
void StreamMixer::SetPacketArray(StreamPacketIO* const pIO,
                                 unsigned int* const puOffsets)
{

    const unsigned int uiNumInputs = mSink.GetNumInputs();

    mInputPackets = pIO;
    mMixBufferOffsets = puOffsets;

    if (puOffsets == 0 || uiNumInputs == 0)
    {
        return;
    }

    for (unsigned int i = 0; i < uiNumInputs; ++i)
    {
        mMixBufferOffsets[i] = 0;
    }
}

// FUNCTION: SWBF2 0x0073fde0
void StreamMixer::SetSynchronizedMixing(bool b)
{
    char* const pThis = (char*)this;

    pThis[0x88] = (char)(b ? 1 : 0);
}

// FUNCTION: SWBF2 0x0073fb20
void StreamMixer::SetChannels(unsigned char ucIn, unsigned char ucOut)
{
    mInputChannels = ucIn;
    mOutputChannels = ucOut;
}

// FUNCTION: SWBF2 0x0073fbe0
void StreamMixer::SetGainArray(int* pGains)
{
    const unsigned int uiNumInputs = mSink.GetNumInputs();

    if (uiNumInputs == 0)
    {
        mGain = 0;
        return;
    }

    mGain = pGains;

    const unsigned int uiOutputs = mOutputChannels;

    if (pGains == 0)
    {
        return;
    }

    unsigned int uiSlotA = 0;
    unsigned int uiSlotB = uiOutputs * uiNumInputs;

    for (unsigned int i = 0; i < uiNumInputs; ++i)
    {
        for (unsigned int j = 0; j < uiOutputs; ++j)
        {
            mGain[uiSlotA + j] = 0x7FFF0000;
            mGain[uiSlotB + j] = 0x7FFF0000;
        }

        uiSlotA += uiOutputs;
        uiSlotB += uiOutputs;
    }
}

// FUNCTION: SWBF2 0x0073fd80
void StreamMixer::SetStopOnLast(unsigned char uc)
{
    if (uc)
    {
        mFlags = (unsigned char)(mFlags | 0x40);
    }
    else
    {
        mFlags = (unsigned char)(mFlags & 0xBF);
    }
}

// FUNCTION: SWBF2 0x0073fda0
void StreamMixer::SetSynchronizedActivation(bool b)
{
    if (b)
    {
        mFlags = (unsigned char)(mFlags | 0x03);
    }
    else
    {
        mFlags = (unsigned char)(mFlags & 0xFC);
    }
}

// FUNCTION: SWBF2 0x0073fdc0
void StreamMixer::SetSynchronizedDeactivation(bool b)
{
    if (b)
    {
        mFlags = (unsigned char)(mFlags | 0x08);
    }
    else
    {
        mFlags = (unsigned char)(mFlags & 0xF7);
    }
}

// FUNCTION: SWBF2 0x0073f8a0
void* __fastcall sub_73F8A0(void* pMixer, int, unsigned int uiFlags)
{
    ((StreamMixer*)pMixer)->StreamMixer::~StreamMixer();
    if ((uiFlags & 1) != 0)
    {
        ::operator delete(pMixer);
    }
    return pMixer;
}

// FUNCTION: SWBF2 0x0073f920
void StreamMixer::Discontinuity(unsigned int uiInput, bool bFlush)
{
    if (bFlush)
    {
        FlushInput(uiInput);
    }
    mActive[uiInput >> 3] &= ~(1 << (uiInput & 7));
}

// FUNCTION: SWBF2 0x0073f950
void StreamMixer::Flush()
{
    memset(mActive, 0, sizeof(mActive));
    memset(mMixedPacket, 0, sizeof(mMixedPacket));

    const unsigned int uiNumInputs = mSink.GetNumInputs();
    if (mInputPackets)
    {
        for (unsigned int i = 0; i < uiNumInputs; ++i)
        {
            mInputPackets[i].Flush();
        }
    }
    if (mMixBufferOffsets)
    {
        for (unsigned int i = 0; i < uiNumInputs; ++i)
        {
            mMixBufferOffsets[i] = 0;
        }
    }
    StreamSourceProducer::Flush();
}

StreamPacket* StreamMixer::ConnectionGetPacket(unsigned int uiInput)
{
    if (mInputPackets)
    {
        StreamPacketIO* const pIO = &mInputPackets[uiInput];
        StreamPacket* const pPacket = pIO->GetPacket();
        if (pPacket)
        {
            if (pIO->GetDataLeft(pPacket->mBufferUsed))
            {
                return pIO->GetPacket();
            }
            pIO->Flush();
        }
        pIO->SetPacket(mSink.ConnectionGetPacket(uiInput));
        return pIO->GetPacket();
    }
    return mSink.ConnectionGetPacket(uiInput);
}

// FUNCTION: SWBF2 0x0073fe00
void StreamMixer::UnlockPacket(unsigned int uiInput, StreamPacket* const pPacket)
{
    if (mInputPackets)
    {
        mInputPackets[uiInput].Flush();
    }
    else
    {
        pPacket->Unlock();
    }
}

// FUNCTION: SWBF2 0x0073f9c0
void StreamMixer::FlushInput(unsigned int uiInput)
{
    StreamPacket* pPacket;
    while ((pPacket = ConnectionGetPacket(uiInput)) != 0)
    {
        UnlockPacket(uiInput, pPacket);
    }
}

// FUNCTION: SWBF2 0x0073fa80
StreamPacket* StreamMixer::GetPacket(unsigned int uiOutput)
{
    StreamPacket* const pPacket = StreamSourceProducer::GetPacket(uiOutput);
    if (pPacket)
    {
        return pPacket;
    }
    Update();
    return StreamSourceProducer::GetPacket(uiOutput);
}

namespace
{
struct MixerNode : public StreamNode
{
    using StreamNode::UpdateNode;
    using SourceTransport::SetPlayState;
};
}

// FUNCTION: SWBF2 0x0073fab0
bool StreamMixer::IsActive() const
{
    bool bActive = false;
    for (unsigned int i = 0; i < sizeof(mActive); ++i)
    {
        bActive |= (mActive[i] != 0);
    }
    return bActive;
}

// FUNCTION: SWBF2 0x0073fe60
void StreamMixer::Update()
{
    if (mMixBuffer == 0)
    {
        return;
    }

    MixerNode* const pNode = (MixerNode*)mNode;
    if (pNode->UpdateNode(true))
    {
        Flush();
    }

    switch (pNode->GetCurrentPlayState())
    {
    case SourceTransport::playState_playing:
        break;
    case SourceTransport::playState_pausing:
        if (GetReadPacket() != 0)
        {
            break;
        }
        pNode->SetPlayState(SourceTransport::playState_paused);
        return;
    default:
        return;
    }

    unsigned int uiNumPackets;
    do
    {
        unsigned int uiNumMixed = 0;
        StreamPacket* const pOutPacket = GetReadPacket();
        unsigned int uiNumActive = 0;
        const unsigned int uiNumInputs = mSink.GetNumInputs();
        if (pOutPacket == 0)
        {
            return;
        }

        uiNumPackets = 0;
        const bool bActivate = (mFlags & flags_SyncDisc) == 0 || (mFlags & flags_Discon) == 0;

        unsigned char ucMask = 1;
        for (unsigned int uiInput = 0; uiInput < uiNumInputs;
             ++uiInput, ucMask = (unsigned char)((ucMask << 1) | (ucMask >> 7)))
        {
            StreamPacket* pPacket;
            if (!mSynchronizedMixing && bActivate)
            {
                pPacket = ConnectionGetPacket(uiInput);
            }
            else if (mActive[uiInput >> 3] & ucMask)
            {
                ++uiNumActive;
                if (mMixedPacket[uiInput >> 3] & ucMask)
                {
                    ++uiNumMixed;
                    continue;
                }
                pPacket = ConnectionGetPacket(uiInput);
            }
            else
            {
                if (!bActivate)
                {
                    continue;
                }
                pPacket = ConnectionGetPacket(uiInput);
                if (pPacket == 0)
                {
                    continue;
                }
                if (pPacket->mBufferUsed == 0)
                {
                    UnlockPacket(uiInput, pPacket);
                    continue;
                }
                mActive[uiInput >> 3] |= ucMask;
                ++uiNumActive;
            }

            if (pPacket == 0)
            {
                continue;
            }

            const unsigned int uiBufferUsed = pPacket->mBufferUsed;
            if (uiBufferUsed < sizeof(short))
            {
                pPacket->Unlock();
                continue;
            }
            ++uiNumPackets;

            const unsigned int uiInFrameSize = mInputChannels * sizeof(short);
            const unsigned int uiOutFrameSize = mOutputChannels * sizeof(int);
            unsigned int uiFrames;
            unsigned int uiMixEnd;
            const short* pData;
            int* pMix;
            if (mInputPackets)
            {
                StreamPacketIO* const pIO = &mInputPackets[uiInput];
                uiFrames = pIO->GetDataLeft(uiBufferUsed) / uiInFrameSize;
                pData = (const short*)pIO->GetData();
                const unsigned int uiMaxFrames = mMixBufferDataSize
                    ? mMixBufferDataSize - mMixBufferOffsets[uiInput] / uiOutFrameSize
                    : mMixBufferSize / uiOutFrameSize;
                if (uiFrames > uiMaxFrames)
                {
                    uiFrames = uiMaxFrames;
                }
                pMix = (int*)((char*)mMixBuffer + mMixBufferOffsets[uiInput]);
                mMixBufferOffsets[uiInput] += uiFrames * uiOutFrameSize;
                pIO->UpdateOffset(uiBufferUsed, uiFrames * uiInFrameSize);
                uiMixEnd = mMixBufferOffsets[uiInput] / uiOutFrameSize;
            }
            else
            {
                uiFrames = uiBufferUsed / uiInFrameSize;
                pData = (const short*)pPacket->mBuffer;
                pMix = mMixBuffer;
                uiMixEnd = uiFrames;
            }

            if (uiFrames == 0)
            {
                ++uiNumMixed;
                continue;
            }

            if (mFlags & flags_CPS)
            {
                mMixBufferDataSize = mMixBufferSize / uiOutFrameSize;
            }
            else if (uiMixEnd > mMixBufferDataSize)
            {
                for (unsigned int i = 0; i < (uiNumInputs >> 3); ++i)
                {
                    mMixedPacket[i] = 0;
                }
                uiNumMixed = 0;
                mMixBufferDataSize = uiMixEnd;
            }

            const bool bDone = mInputPackets
                ? mInputPackets[uiInput].GetDataLeft(uiBufferUsed) == 0
                : true;
            const bool bEnd = bDone && (pPacket->GetDiscontinuity() || pPacket->GetLast());

            if (mGain)
            {
                const unsigned char ucRate = mInterpolationRate;
                const unsigned int uiTargets = mOutputChannels * uiNumInputs;
                const unsigned int uiGainBase = mOutputChannels * uiInput;

                bool bFadeOut = false;
                int aiTargets[6];
                if (bEnd && !pPacket->GetLast())
                {
                    bFadeOut = true;
                    for (unsigned char c = 0; c < mOutputChannels; ++c)
                    {
                        int* const pTarget = &mGain[uiTargets + c + uiGainBase];
                        aiTargets[c] = *pTarget;
                        *pTarget = 0;
                    }
                    mInterpolationRate = 1;
                }

                if (mInputChannels < mOutputChannels)
                {
                    if (mInputChannels == 1 && mOutputChannels == 2)
                    {
                        int* const pGain = &mGain[uiGainBase];
                        int iGain0 = pGain[0] >> 16;
                        const int iTarget0 = mGain[uiGainBase + uiTargets];
                        int iGain1 = pGain[1] >> 16;
                        const int iTarget1 = mGain[uiGainBase + uiTargets + 1];
                        pGain[0] += (iTarget0 - pGain[0]) >> mInterpolationRate;
                        pGain[1] += (iTarget1 - pGain[1]) >> mInterpolationRate;
                        for (unsigned int f = 0; f < uiFrames; ++f)
                        {
                            pMix[0] += (pData[f] * iGain0) >> 15;
                            iGain0 = pGain[0] >> 16;
                            pGain[0] += (iTarget0 - pGain[0]) >> mInterpolationRate;
                            pMix[1] += (pData[f] * iGain1) >> 15;
                            iGain1 = pGain[1] >> 16;
                            pGain[1] += (iTarget1 - pGain[1]) >> mInterpolationRate;
                            pMix += 2;
                        }
                    }
                    else
                    {
                        for (unsigned char c = 0; c < mOutputChannels; ++c)
                        {
                            int* const pGain = &mGain[uiGainBase + c];
                            const int iTarget = mGain[uiTargets + c + uiGainBase];
                            int iGain = *pGain >> 16;
                            *pGain += (iTarget - *pGain) >> mInterpolationRate;
                            unsigned int uiOut = c;
                            for (unsigned int f = 0; f < uiFrames; ++f)
                            {
                                pMix[uiOut] += (pData[f] * iGain) >> 15;
                                iGain = *pGain >> 16;
                                *pGain += (iTarget - *pGain) >> mInterpolationRate;
                                uiOut += mOutputChannels;
                            }
                        }
                    }
                }
                else if (mInputChannels > mOutputChannels)
                {
                    for (unsigned char c = 0; c < mInputChannels; ++c)
                    {
                        int* const pGain = &mGain[uiGainBase + c];
                        const int iTarget = mGain[uiTargets + c + uiGainBase];
                        unsigned int uiIn = c;
                        for (unsigned int f = 0; f < uiFrames; ++f)
                        {
                            const int iGain = *pGain >> 16;
                            *pGain += (iTarget - *pGain) >> mInterpolationRate;
                            pMix[f] += (pData[uiIn] * iGain) >> 16;
                            uiIn += mInputChannels;
                        }
                    }
                }
                else if (mInputChannels == 2)
                {
                    int* const pGain = &mGain[uiGainBase];
                    int iGain0 = pGain[0] >> 16;
                    const int iTarget0 = mGain[uiGainBase + uiTargets];
                    int iGain1 = pGain[1] >> 16;
                    const int iTarget1 = mGain[uiGainBase + uiTargets + 1];
                    pGain[0] += (iTarget0 - pGain[0]) >> mInterpolationRate;
                    pGain[1] += (iTarget1 - pGain[1]) >> mInterpolationRate;
                    for (unsigned int f = 0; f < uiFrames; ++f)
                    {
                        pMix[0] += (pData[0] * iGain0) >> 15;
                        iGain0 = pGain[0] >> 16;
                        pGain[0] += (iTarget0 - pGain[0]) >> mInterpolationRate;
                        pMix[1] += (pData[1] * iGain1) >> 15;
                        iGain1 = pGain[1] >> 16;
                        pGain[1] += (iTarget1 - pGain[1]) >> mInterpolationRate;
                        pMix += 2;
                        pData += 2;
                    }
                }
                else
                {
                    for (unsigned char c = 0; c < mInputChannels; ++c)
                    {
                        int* const pGain = &mGain[uiGainBase + c];
                        const int iTarget = mGain[uiTargets + c + uiGainBase];
                        unsigned int uiSample = c;
                        for (unsigned int f = 0; f < uiFrames; ++f)
                        {
                            const int iGain = *pGain >> 16;
                            *pGain += (iTarget - *pGain) >> mInterpolationRate;
                            pMix[uiSample] += (pData[uiSample] * iGain) >> 16;
                            uiSample += mInputChannels;
                        }
                    }
                }

                if (bFadeOut)
                {
                    mInterpolationRate = ucRate;
                    for (unsigned char c = 0; c < mOutputChannels; ++c)
                    {
                        mGain[uiTargets + c + uiGainBase] = aiTargets[c];
                    }
                }
            }
            else if (mInputChannels < mOutputChannels)
            {
                unsigned int uiOut = 0;
                for (unsigned char c = 0; c < mOutputChannels; ++c)
                {
                    for (unsigned int s = c; s < uiFrames; s += mOutputChannels)
                    {
                        pMix[uiOut++] += pData[s];
                    }
                }
            }
            else if (mInputChannels > mOutputChannels)
            {
                unsigned int uiIn = 0;
                for (unsigned char c = 0; c < mInputChannels; ++c)
                {
                    for (unsigned int s = c; s < uiFrames; s += mInputChannels)
                    {
                        pMix[s] += pData[uiIn++];
                    }
                }
            }
            else
            {
                const unsigned int uiSamples = mInputChannels * uiFrames;
                for (unsigned int s = 0; s < uiSamples; ++s)
                {
                    pMix[s] += pData[s];
                }
            }

            const unsigned int uiEndOffset = mMixBufferDataSize * uiOutFrameSize;
            if (bDone)
            {
                if (bEnd)
                {
                    mActive[uiInput >> 3] &= (unsigned char)~ucMask;
                    mFlags |= flags_Discon;
                }
                if (mMixBufferOffsets == 0)
                {
                    mMixedPacket[uiInput >> 3] |= ucMask;
                    ++uiNumMixed;
                }
                else if (mMixBufferOffsets[uiInput] == uiEndOffset || bEnd)
                {
                    mMixBufferOffsets[uiInput] = uiEndOffset;
                    mMixedPacket[uiInput >> 3] |= ucMask;
                    ++uiNumMixed;
                }
                UnlockPacket(uiInput, pPacket);
            }
            else if (mMixBufferOffsets && mMixBufferOffsets[uiInput] == uiEndOffset)
            {
                mMixedPacket[uiInput >> 3] |= ucMask;
                ++uiNumMixed;
            }
        }

        if (uiNumActive == 0 && (mFlags & flags_Enable))
        {
            mFlags |= flags_Sync;
        }
        else if (mFlags & flags_Sync)
        {
            if (uiNumActive < uiNumInputs)
            {
                uiNumMixed = 0;
            }
            else
            {
                mFlags &= (unsigned char)~flags_Sync;
            }
        }

        if (mSynchronizedMixing ? (uiNumActive != 0 && uiNumActive == uiNumMixed)
                                : (uiNumMixed != 0))
        {
            const unsigned int uiPos = pNode->mPlaybackPosition;
            int* pSum = mMixBuffer;
            short* pOut = (short*)pOutPacket->mBuffer;
            const unsigned int uiSamples = mMixBufferDataSize * mOutputChannels;
            if (mOutputGain == 0x7FFF)
            {
                for (unsigned int s = 0; s < uiSamples; ++s)
                {
                    int iSample = *pSum;
                    if (iSample > 0x7FFF)
                    {
                        iSample = 0x7FFF;
                    }
                    else if (iSample < -0x8000)
                    {
                        iSample = -0x8000;
                    }
                    *pOut++ = (short)iSample;
                    *pSum++ = 0;
                }
            }
            else
            {
                for (unsigned int s = 0; s < uiSamples; ++s)
                {
                    int iSample = (*pSum * mOutputGain) >> 16;
                    if (iSample > 0x7FFF)
                    {
                        iSample = 0x7FFF;
                    }
                    else if (iSample < -0x8000)
                    {
                        iSample = -0x8000;
                    }
                    *pOut++ = (short)iSample;
                    *pSum++ = 0;
                }
            }

            if (mMixBufferOffsets)
            {
                for (unsigned int i = 0; i < uiNumInputs; ++i)
                {
                    mMixBufferOffsets[i] = 0;
                }
            }
            memset(mMixedPacket, 0, sizeof(mMixedPacket));

            const bool bInactive = !IsActive();
            PacketComplete(pOutPacket, uiSamples * sizeof(short), mMixBufferDataSize,
                           (mFlags & flags_LastPck) != 0 && bInactive, bInactive, uiPos);
            pNode->mPlaybackPosition = uiPos + mMixBufferDataSize;
            mMixBufferDataSize = 0;

            if (bInactive)
            {
                mFlags &= (unsigned char)~flags_Discon;
                if (mFlags & flags_StopOnLast)
                {
                    pNode->Stop();
                    return;
                }
            }
        }
    } while (uiNumPackets != 0);
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0073fb60
void StreamMixer::SetGain(unsigned int uiInput, unsigned int uiChannel,
                          float fGain, bool bImmediate)
{
    const unsigned int uiTarget =
        mSink.GetNumInputs() * mOutputChannels + uiInput * mOutputChannels + uiChannel;
    int iGain;
    if (fGain > 0.0f)
    {
        iGain = (int)(fGain * 32767.0f) << 16;
    }
    else
    {
        iGain = (int)(fGain * 32768.0f) << 16;
    }
    mGain[uiTarget] = iGain;
    if (bImmediate)
    {
        mGain[uiInput * mOutputChannels + uiChannel] = iGain;
    }
}
}

// FUNCTION: SWBF2 0x0073fcf0
void Snd::StreamMixer::SetOutputGain(float fGain)
{
    if (fGain > 0.0f)
    {
        mOutputGain = (int)(fGain * 32767.0f);
    }
    else
    {
        mOutputGain = (int)(fGain * -32768.0f);
    }
}

// FUNCTION: SWBF2 0x0073faf0
bool Snd::StreamMixer::HasMixedPacket(unsigned int uiInput) const
{
    return (mMixedPacket[uiInput >> 3] & (1u << (uiInput & 7))) != 0;
}
