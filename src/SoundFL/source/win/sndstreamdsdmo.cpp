#include <windows.h>
#include <string.h>
#include <new>

#include "sndstreamdsdmo.h"

#include "SoundFL/source/sndsampleinfo.h"
#include "SoundFL/source/sndstreampacketio.h"
#include "sndengine.h"
#include "sndsoftoutput.h"

namespace Snd {
}

namespace Snd
{

static const char kFile[] = "source\\win\\sndstreamdsdmo.cpp";

static const float kLatencySeconds = 0.16f;

static const GUID* const kStandardEffects[] = {
    &GUID_DSFX_STANDARD_CHORUS,
    &GUID_DSFX_STANDARD_COMPRESSOR,
    &GUID_DSFX_STANDARD_DISTORTION,
    &GUID_DSFX_STANDARD_ECHO,
    &GUID_DSFX_STANDARD_FLANGER,
    &GUID_DSFX_STANDARD_GARGLE,
    &GUID_DSFX_STANDARD_I3DL2REVERB,
    &GUID_DSFX_STANDARD_PARAMEQ,
    &GUID_DSFX_WAVES_REVERB,
};

// FUNCTION: SWBF2 0x0074d8c0
bool StreamDSDMO::CreateDMO(const _GUID& rCLSID, const StreamFormat& rIn,
                            const StreamFormat& rOut)
{
    if (mDMO.IsValid())
    {
        mDMO.Destroy();
    }

    if (mDMO.Create(rCLSID, &rIn, &rOut))
    {
        unsigned long ulSize = 0;
        unsigned long ulMaxLookahead = 0;
        unsigned long ulAlignment = 0;

        if (Engine::Succeeded(mDMO.GetMediaObject()->GetInputSizeInfo(
                                  0, &ulSize, &ulMaxLookahead, &ulAlignment),
                              kFile, 58))
        {
            const unsigned int uiInputFrameSize =
                SampleInfo::GetFrameSize(rIn.GetFormat()) * rIn.GetNumChannels();
            mInputFrameSize = uiInputFrameSize;

            unsigned int uiMinSize = ulSize;
            for (int i = 0; i < 9; ++i)
            {
                if (rCLSID.Data1 != kStandardEffects[i]->Data1)
                {
                    continue;
                }

                const unsigned int uiLatencyBytes =
                    (unsigned int)((float)(rIn.GetFrequency() * uiInputFrameSize)
                                   * kLatencySeconds);
                const unsigned int uiRounded =
                    ((uiLatencyBytes - 1 + uiInputFrameSize) / uiInputFrameSize)
                    * uiInputFrameSize;
                if (uiMinSize < uiRounded)
                {
                    uiMinSize = uiRounded;
                }
                break;
            }

            mMinSize = uiMinSize;
            mAlignment = ulAlignment;
            mAudioFrameSize =
                SampleInfo::GetFrameSize(rOut.GetFormat()) * rOut.GetNumChannels();
            return true;
        }

        mDMO.Destroy();
    }

    mMinSize = 0;
    mAlignment = 0;
    mAudioFrameSize = 0;
    return false;
}

// FUNCTION: SWBF2 0x0074da50
unsigned int StreamDSDMO::GetInputAligment() const
{
    return mAlignment;
}

// FUNCTION: SWBF2 0x0074da60
unsigned int StreamDSDMO::GetMinInputSize() const
{
    return mMinSize;
}

struct StreamNodeUpdateAccess : public StreamNode
{
    using StreamNode::UpdateNode;
};

 

// FUNCTION: SWBF2 0x0074db40
void StreamDSDMO::Update()
{

    StreamPacketIO* const pInputIO = &mInputPacket;
    StreamPacketIOLite* const pOutputIO = &mOutputPacket;
    StreamNodeUpdateAccess* const pNode = (StreamNodeUpdateAccess*)&mNode;

    if (pNode->UpdateNode(true))
    {
        pInputIO->Flush();
        pOutputIO->Reset();
        mBuffer.Reset();
        StreamSourceProducer::Flush();
        mFlags = 0;
    }

    if (pNode->GetCurrentPlayState() == SourceTransport::playState_stopped)
    {
        return;
    }

    StreamPacket* pOutPacket = GetReadPacket();
    if (pOutPacket == 0)
    {
        return;
    }

    for (;;)
    {

        void* pDeliver = 0;
        unsigned int uiDeliverSize = 0;

        StreamPacket* pInPacket = pInputIO->GetPacket();
        void* const pBuffer = mBuffer.GetBuffer();
        bool bProduced = false;
        bool bMarkerDelivered = false;

        if (pInPacket == 0)
        {

            StreamNodeSource* const pInputNode = mSink.mConnection.mInputNode;
            if (pInputNode != 0)
            {
                typedef StreamPacket* (__thiscall* GetPacketFn)(
                    StreamNodeSource* pThis, unsigned int uiOutput);
                pInPacket = pInputNode->GetPacket(mSink.mConnection.mInputNodeOutputIndex);
            }
            else
            {
                pInPacket = 0;
            }

            pInputIO->SetPacket(pInPacket);
        }

        if (pInPacket == 0)
        {

            if ((mFlags & 8) != 0)
            {

                const unsigned int uiOffset = mBuffer.GetOffset();
                const unsigned int uiMin = mMinSize;
                if (uiOffset < uiMin)
                {
                    const unsigned int uiPad = uiMin - uiOffset;

                    if (uiPad != 0)
                    {

                        memset(mBuffer.GetData(), 0, uiPad);
                        mBuffer.UpdateOffset(uiPad);
                    }
                }

                mFlags = (unsigned char)((mFlags & ~8u) | 2u);
            }
        }
        else
        {

            const bool bLastPacket = (pInPacket->mFlags & 1) != 0;
            const bool bDiscontinuity = ((pInPacket->mFlags >> 1) & 1) != 0;

            if (pBuffer == 0)
            {

                pDeliver = pInPacket->mBuffer;
                uiDeliverSize = pInPacket->mBufferUsed;
            }
            else
            {
                if (mBuffer.GetDataLeft() != 0)
                {

                    const unsigned int uiUsed = pInPacket->mBufferUsed;
                    const unsigned int uiLeft =
                        pInputIO->GetDataLeft(uiUsed);
                    void* const pSrc = pInputIO->GetData();
                    const unsigned int uiWritten =
                        mBuffer.Write(pSrc, uiLeft);

                    if (pInputIO->UpdateOffset(uiUsed, uiWritten))
                    {

                        if (bLastPacket || bDiscontinuity)
                        {
                            const unsigned int uiOffset =
                                mBuffer.GetOffset();
                            const unsigned int uiMin = mMinSize;
                            if (uiOffset < uiMin)
                            {
                                const unsigned int uiPad = uiMin - uiOffset;
                                if (uiPad != 0)
                                {
                                    memset(mBuffer.GetData(), 0,
                                           uiPad);
                                    mBuffer.UpdateOffset(uiPad);
                                }
                            }
                        }

                        pInputIO->Flush();
                    }
                }

                const unsigned int uiOffset = mBuffer.GetOffset();
                if (uiOffset >= mMinSize)
                {
                    pDeliver = pBuffer;
                    uiDeliverSize = uiOffset;
                }
            }

            if (bLastPacket)
            {
                mFlags = (unsigned char)(mFlags | 1u);
            }
            if (bDiscontinuity)
            {
                mFlags = (unsigned char)(mFlags | 2u);
            }
        }

        if (mDMO.IsInputDataComplete()
            && pDeliver != 0)
        {

            mDMO.DeliverInputData(pDeliver, uiDeliverSize);

            if ((mFlags & 3) != 0)
            {
                mFlags = (unsigned char)(mFlags | 4u);
            }
        }

        const unsigned int uiPacketSize = mProducer[2];
        const unsigned int uiOutLeft =
            pOutputIO->GetDataLeft(uiPacketSize);
        void* const pOutData = pOutputIO->GetData(pOutPacket);

        const unsigned int uiProduced = mDMO.GetOutputData(pOutData, uiOutLeft);

        if (uiProduced != 0)
        {
            pOutputIO->UpdateOffset(uiPacketSize, uiProduced);
            bProduced = true;

            if (mDMO.IsInputDataComplete() && (mFlags & 4) != 0)
            {

                bMarkerDelivered = ((mFlags >> 2) & 1) != 0;

                if (pOutputIO->GetOffset() == 0)
                {
                    memset(pOutputIO->GetData(pOutPacket), 0,
                           mAudioFrameSize);
                    pOutputIO->UpdateOffset(uiPacketSize,
                                            mAudioFrameSize);
                }
            }
        }

        if (mDMO.IsInputDataComplete())
        {

            if (pBuffer != 0 && mBuffer.GetDataLeft() == 0)
            {
                mBuffer.Reset();
            }

            else if (pInputIO->GetPacket() != 0)
            {
                pInputIO->Flush();
            }
        }

        if (bProduced)
        {

            const unsigned int uiPos = pNode->mPlaybackPosition;
            const unsigned int uiBytes = pOutputIO->GetOffset();

            const unsigned int uiFrames = uiBytes / mAudioFrameSize;

            bool bFlagBit0 = false;
            bool bFlagBit1 = false;
            if (bMarkerDelivered)
            {

                const unsigned char ucFlags = mFlags;
                mFlags = 0;
                bFlagBit0 = (ucFlags & 1) != 0;
                bFlagBit1 = ((ucFlags >> 1) & 1) != 0;
            }

            PacketComplete(pOutPacket, uiBytes, uiFrames, bFlagBit0, bFlagBit1,
                           uiPos);

            pNode->mPlaybackPosition = uiPos + uiFrames;
            pOutputIO->Reset();
        }

        else if (pInPacket == 0)
        {
            break;
        }

        pOutPacket = GetReadPacket();
        if (pOutPacket == 0)
        {
            break;
        }
    }
}

// FUNCTION: SWBF2 0x0074d820
void* __fastcall sub_74D820(void* pDSDMO, int, unsigned int uiFlags)
{
    StreamDSDMO* const pThis = (StreamDSDMO*)pDSDMO;

    pThis->StreamDSDMO::~StreamDSDMO();

    if ((uiFlags & 1) != 0)
    {
        ::operator delete(pThis);
    }

    return pThis;
}

// FUNCTION: SWBF2 0x0074da70
StreamPacket* StreamDSDMO::GetPacket(unsigned int uiOutput)
{

    StreamPacket* pPacket = StreamSourceProducer::GetPacket(uiOutput);

    if (pPacket == 0)
    {
        Update();
        pPacket = StreamSourceProducer::GetPacket(uiOutput);
        if (pPacket == 0)
        {
            return 0;
        }
    }

    if ((pPacket->mFlags & 0x01) != 0)
    {

        mNode.Stop();
    }

    return pPacket;
}

// FUNCTION: SWBF2 0x0074da20
void StreamDSDMO::Flush()
{
    mInputPacket.Flush();
    mOutputPacket.Reset();
    mBuffer.Reset();
    StreamSourceProducer::Flush();
    mFlags = 0;
}

// FUNCTION: SWBF2 0x0074dac0
void StreamDSDMO::SetInputBuffer(void* pBuffer, unsigned int uiSize)
{

    mInputPacket.Flush();
    mOutputPacket.Reset();

    mBuffer.Reset();
    StreamSourceProducer::Flush();

    const unsigned int uiRequested = uiSize;

    mFlags = 0;

    if (pBuffer != 0 && uiSize != 0)
    {

        unsigned int uiAlignment = mAlignment;
        if (uiAlignment == 0)
        {
            uiAlignment = 1;
        }
        mAlignment = uiAlignment;
    }

    const unsigned int uiFrameSize = mInputFrameSize;
    if (uiFrameSize != 0)
    {

        mBuffer.SetBuffer(pBuffer,
                          (uiRequested / uiFrameSize) * uiFrameSize);
    }
    else
    {

        mBuffer.SetBuffer(pBuffer, 0);
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0074d6e0
StreamDSDMO::StreamDSDMO()
    : mAlignment(0),
      mMinSize(0),
      mAudioFrameSize(0),
      mInputFrameSize(0),
      mFlags(0)
{
    mSink.mConnection.mInputNode = 0;
    mSink.mConnection.mInputNodeOutputIndex = 0;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0074d7a0
StreamDSDMO::~StreamDSDMO()
{
}
}
