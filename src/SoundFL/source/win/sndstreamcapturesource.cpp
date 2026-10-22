#if !defined(DIRECTSOUND_VERSION) || DIRECTSOUND_VERSION < 0x0800
#undef DIRECTSOUND_VERSION
#define DIRECTSOUND_VERSION 0x0800
#endif

#include <windows.h>
#include <dsound.h>

#include <new>
#include <stdlib.h>
#include <string.h>

#include "SoundFL/source/sndglobals.h"
#include "SoundFL/source/sndsampleinfo.h"
#include "SoundFL/source/sndsourcetransport.h"
#include "SoundFL/source/sndstreamformat.h"
#include "SoundFL/source/sndstreampacket.h"
#include "SoundFL/source/sndstreampacketio.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "snddsbuffer.h"
#include "snddsdeviceenumerator.h"
#include "sndengine.h"
#include "sndsemaphore.h"
#include "sndstreamcapturesource.h"

void __cdecl PblTraceP(const char* Format, ...);

namespace Snd
{

static const char kFile[] = "source\\win\\sndstreamcapturesource.cpp";

static const GUID kDSDevIdDefaultCapture =
    { 0xDEF00001, 0x9C6D, 0x47ED, { 0xAA, 0xF1, 0x4D, 0xDA, 0x8F, 0x2B, 0x5C, 0x03 } };
static const GUID kDSDevIdDefaultVoiceCapture =
    { 0xDEF00003, 0x9C6D, 0x47ED, { 0xAA, 0xF1, 0x4D, 0xDA, 0x8F, 0x2B, 0x5C, 0x03 } };

// FUNCTION: SWBF2 0x00740e70
StreamCaptureSource::StreamCaptureSource()
    : mCaptureDevice(0),
      mCaptureBuffer(0),
      mCaptureBufferSize(0),
      mNextLockPos(0),
      mFrameSize(0)
{

    if (!mLock.Create(1, 1))
    {
        PrintDebugMessage("Unable to create semaphore : out of system "
                          "resources!\n");
        exit(0);
    }

    Engine::GetThreadUpdate()->AddObject(this);
}

// FUNCTION: SWBF2 0x00740f60
StreamCaptureSource::~StreamCaptureSource()
{

    if (mCaptureBuffer != 0)
    {
        Close();
    }

    mDestructing = 1;

    if (mNode.mNext != 0)
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x00741020
void* __fastcall sub_741020(void* pCaptureSource, int, unsigned int uiFlags)
{
    char* const pThis = (char*)pCaptureSource;

    ((StreamCaptureSource*)pThis)
        ->StreamCaptureSource::~StreamCaptureSource();

    if ((uiFlags & 1) != 0)
    {
        ::operator delete(pThis);
    }

    return pThis;
}

// FUNCTION: SWBF2 0x0074100e
void* __fastcall sub_74100E(void* pObject, int, unsigned int uiFlags)
{
    StreamCaptureSource* const pSource = static_cast<StreamCaptureSource*>(
        static_cast<ThreadUpdate::Object*>(pObject));
    return sub_741020(pSource, 0, uiFlags);
}

// FUNCTION: SWBF2 0x00741050
void StreamCaptureSource::Close()
{
    mLock.Wait();
    StreamCaptureSourceBase::Flush();

    IDirectSoundCaptureBuffer8* const pBuffer =
        mCaptureBuffer;
    if (pBuffer != 0)
    {
        pBuffer->Stop();

        void* pvAudio1 = 0;
        DWORD dwAudio1 = 0;
        void* pvAudio2 = 0;
        DWORD dwAudio2 = 0;
        pBuffer->Lock(0, mPacketSize,
                      &pvAudio1, &dwAudio1, &pvAudio2, &dwAudio2, 0);

        if (dwAudio1 != 0)
        {
            memset(pvAudio1, 0, dwAudio1);
        }

        if (dwAudio2 != 0)
        {
            memset(pvAudio1, 0, dwAudio2);
        }

        pBuffer->Unlock(pvAudio1, dwAudio1,
                        pvAudio2, dwAudio2);

        mNextLockPos = 0;
        mOutputPacket.Reset();
    }

    {
        IDirectSoundCaptureBuffer8* const pRelease =
            mCaptureBuffer;

        pRelease->Release();

        IDirectSoundCapture* const pDevice =
            mCaptureDevice;

        mCaptureBuffer = 0;
        mCaptureBufferSize = 0;

        pDevice->Release();

        mCaptureDevice = 0;
        mFrameSize = 0;
    }

    StreamCaptureSourceBase::Close(true);
    mLock.Signal();
}

// FUNCTION: SWBF2 0x00741130
void StreamCaptureSource::Flush(bool bAlreadyLocked)
{
    if (!bAlreadyLocked)
    {
        mLock.Wait();
    }

    StreamCaptureSourceBase::Flush();

    IDirectSoundCaptureBuffer8* const pBuffer =
        mCaptureBuffer;
    if (pBuffer != 0)
    {
        pBuffer->Stop();

        void* pvAudio1 = 0;
        DWORD dwAudio1 = 0;
        void* pvAudio2 = 0;
        DWORD dwAudio2 = 0;
        pBuffer->Lock(0, mPacketSize,
                      &pvAudio1, &dwAudio1, &pvAudio2, &dwAudio2, 0);

        if (dwAudio1 != 0)
        {
            memset(pvAudio1, 0, dwAudio1);
        }

        if (dwAudio2 != 0)
        {
            memset(pvAudio1, 0, dwAudio2);
        }

        pBuffer->Unlock(pvAudio1, dwAudio1, pvAudio2, dwAudio2);

        mNextLockPos = 0;
        mOutputPacket.Reset();
    }

    if (!bAlreadyLocked)
    {
        mLock.Signal();
    }
}

// FUNCTION: SWBF2 0x007411f0
bool StreamCaptureSource::Open(unsigned char ucDeviceIndex,
                               StreamFormat const& cFormat,
                               unsigned int uiPacketSize,
                               unsigned char ucNumPackets,
                               StreamPacket* pPackets)
{
    mLock.Wait();

    const unsigned int uiIndex = ucDeviceIndex;

    GUID cGuidDevice;
    if (uiIndex == 0xFE)
    {
        cGuidDevice = kDSDevIdDefaultCapture;
    }
    else if (uiIndex == 0xFF)
    {
        cGuidDevice = kDSDevIdDefaultVoiceCapture;
    }
    else
    {

        DSDeviceEnumerator cEnumerator;

        if (!cEnumerator.Enumerate(DSDeviceEnumerator::type_Capture, false))
        {
            mLock.Signal();
            return false;
        }

        cGuidDevice = cEnumerator.GetDeviceIDByIndex(uiIndex);
    }

    if (Engine::Failed(DirectSoundCaptureCreate8(
                           &cGuidDevice,
                           &mCaptureDevice, 0),
                       kFile, 112))
    {
        mLock.Signal();
        return false;
    }

    mCaptureBufferSize = uiPacketSize * 2;

    DSBuffer::WaveFormat cWaveFormat(cFormat.GetFormat(),
                                     cFormat.GetNumChannels(),
                                     cFormat.GetFrequency(), false, 0);

    DSCBUFFERDESC cDesc;
    memset(&cDesc, 0, sizeof(cDesc));
    cDesc.dwSize = sizeof(DSCBUFFERDESC);
    cDesc.dwFlags = DSCBCAPS_WAVEMAPPED;
    cDesc.dwBufferBytes = mCaptureBufferSize;
    cDesc.lpwfxFormat = &cWaveFormat.Format;
    cDesc.dwFXCount = 0;
    cDesc.lpDSCFXDesc = 0;

    IDirectSoundCapture8* const pDevice =
        mCaptureDevice;

    if (Engine::Failed(pDevice->CreateCaptureBuffer(
                           &cDesc,
                           (IDirectSoundCaptureBuffer**)&mCaptureBuffer, 0),
                       kFile, 133))
    {

        pDevice->Release();
        mLock.Signal();
        return false;
    }

    mFrameSize =
        SampleInfo::GetFrameSize(cFormat.GetFormat())
        * cFormat.GetNumChannels();

    StreamCaptureSourceBase::Open((unsigned char)uiIndex, cFormat,
                                  uiPacketSize, ucNumPackets, pPackets);

    Flush(true);
    mLock.Signal();
    return true;
}

// FUNCTION: SWBF2 0x00741470
void StreamCaptureSource::UpdateObject()
{

    mLock.Wait();

    const SourceTransport::PlayState eAction =
        Update(mCaptureBuffer != 0);

    switch (eAction)
    {
    case SourceTransport::playState_stopped:

        Flush(true);
        break;

    case SourceTransport::playState_paused:
    case SourceTransport::playState_playing:
        {
            IDirectSoundCaptureBuffer8* const pBuffer =
                mCaptureBuffer;
            if (pBuffer != 0)
            {
                if (eAction == SourceTransport::playState_paused)
                {
                    pBuffer->Stop();
                }
                else
                {
                    pBuffer->Start(DSCBSTART_LOOPING);
                }
            }
            else
            {

                SourceTransport::Stop();
            }
        }
        break;

    default:

        break;
    }

    IDirectSoundCaptureBuffer8* const pBuffer =
        mCaptureBuffer;
    if (pBuffer != 0)
    {
        const SourceTransport::PlayState eState =
            GetCurrentPlayState();

        if (eState == SourceTransport::playState_playing
            || eState == SourceTransport::playState_paused
            || eState == SourceTransport::playState_pausing)
        {
            const unsigned int uiPacketSize = mPacketSize;
            StreamPacketIOLite* const pIO = &mOutputPacket;

            bool bContinue = true;

            for (;;)
            {

                StreamPacket* const pPacket =
                    mPackets
                    + mReadPacketIndex;

                if (pPacket->mRefCount != 0)
                {
                    break;
                }
                if (pPacket->mBufferUsed != 0)
                {
                    break;
                }

                DWORD dwCapturePos = 0;
                DWORD dwReadPos = 0;
                pBuffer->GetCurrentPosition(&dwCapturePos,
                                            &dwReadPos);

                {
                    const unsigned int uiNext =
                        mNextLockPos;
                    bool bSafe;
                    if (dwCapturePos >= dwReadPos)
                    {
                        bSafe = (uiNext <= dwReadPos)
                                || (uiNext >= dwCapturePos);
                    }
                    else
                    {
                        bSafe = (uiNext < dwReadPos)
                                || (uiNext > dwCapturePos);
                    }

                    if (!bSafe)
                    {

                        PblTraceP("StreamCaptureSource : NLP %8d RP %8d "
                                  "CP %8d - buffer underrun!\n",
                                  uiNext, dwReadPos, dwCapturePos);
                        mNextLockPos =
                            dwCapturePos;
                    }
                }

                unsigned int uiAvailable;
                {
                    const unsigned int uiNext =
                        mNextLockPos;
                    if (dwReadPos >= uiNext)
                    {
                        uiAvailable = dwReadPos - uiNext;
                    }
                    else
                    {
                        uiAvailable = dwReadPos
                                      - (mCaptureBufferSize
                                         - uiNext);
                    }
                }

                if (uiAvailable == 0)
                {
                    bContinue = false;
                }

                unsigned int uiChunk = uiAvailable;
                {
                    const unsigned int uiDataLeft =
                        pIO->GetDataLeft(uiPacketSize);
                    if (uiChunk > uiDataLeft)
                    {
                        uiChunk = uiDataLeft;
                    }
                }

                if (uiChunk != 0)
                {
                    void* pvAudio1 = 0;
                    DWORD dwAudio1 = 0;
                    void* pvAudio2 = 0;
                    DWORD dwAudio2 = 0;
                    pBuffer->Lock(mNextLockPos,
                                  uiChunk,
                                  &pvAudio1, &dwAudio1,
                                  &pvAudio2, &dwAudio2, 0);

                    mNextLockPos += uiChunk;
                    if (mNextLockPos
                        >= mCaptureBufferSize)
                    {
                        mNextLockPos -=
                            mCaptureBufferSize;
                    }

                    if (dwAudio1 != 0)
                    {
                        memcpy(pIO->GetData(pPacket), pvAudio1, dwAudio1);
                        pIO->UpdateOffset(uiPacketSize, dwAudio1);
                    }

                    if (dwAudio2 != 0)
                    {
                        memcpy(pIO->GetData(pPacket), pvAudio2, dwAudio2);
                        pIO->UpdateOffset(uiPacketSize, dwAudio2);
                    }

                    pBuffer->Unlock(pvAudio1, dwAudio1,
                                    pvAudio2, dwAudio2);

                    if (pIO->GetDataLeft(uiPacketSize) == 0)
                    {
                        const unsigned int uiPos =
                            mPlaybackPosition;

                        pPacket->mBufferUsed = uiPacketSize;
                        pPacket->mFlags = (unsigned short)
                            (pPacket->mFlags & 0xFFFCu);
                        pPacket->mPos = uiPos;
                        pIO->Reset();

                        mPlaybackPosition =
                            uiPos
                            + uiPacketSize / mFrameSize;

                        ++mReadPacketIndex;
                        if (mReadPacketIndex
                            == mNumPackets)
                        {
                            mReadPacketIndex = 0;
                        }
                    }
                }

                if (!bContinue)
                {
                    break;
                }
            }
        }
    }

    mLock.Signal();
}

}
