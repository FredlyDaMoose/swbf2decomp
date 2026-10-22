#include <math.h>
#include <stddef.h>
#include <string.h>

#include "snddsbuffer.h"

#include "SoundFL/source/sndenginebase.h"
#include "SoundFL/source/sndglobals.h"
#include "sndengine.h"

namespace Snd
{

static const char kFile[] = "source\\win\\snddsbuffer.cpp";

static const float kGainEpsilon = 0.001f;

static const GUID kSubFormatPcm =
    { 0x00000001, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };

extern const GUID kEAX40ContextPropertySet;
extern const GUID kEAX40FXSlot0PropertySet;
extern const GUID kEAX30BufferPropertySet;

// GLOBAL: SWBF2 0x007e3870
// SYNTHETIC: SWBF2 0x0040c140
// `dynamic initializer for 'Snd::DSBuffer::smList''
// SYNTHETIC: SWBF2 0x0076a490
// `dynamic atexit destructor for 'Snd::DSBuffer::smList''
PblListDouble DSBuffer::smList;

// FUNCTION: SWBF2 0x00744020
unsigned int __cdecl DSBuffer::CreateBuffers(bool b3D, bool bProperties,
                                             unsigned int uiFlags,
                                             DSBuffer* pBuffers,
                                             unsigned int uiCount,
                                             IDirectSound* pDirectSound)
{
    Engine::smErrorsEnabled = false;
    unsigned int uiCreated = 0;
    const unsigned int uiBaseFlags = uiFlags | (b3D ? DSBCAPS_CTRL3D : DSBCAPS_CTRLPAN)
                                           | 0x100A0;
    IDirectSound* pDevice = pDirectSound ? pDirectSound : (IDirectSound*)Engine::smDirectSound;

    do
    {
        DSBuffer& rBuffer = pBuffers[uiCreated];
        const bool bSetupEffects = b3D && pDevice == (IDirectSound*)Engine::smDirectSound
                                   && Engine::smHardwareEffectsAvailable;
        const unsigned int uiChannels = b3D ? 1 : (bProperties ? 2 : 1);
        WaveFormat format(SampleInfo::format_PCM16, uiChannels, 44100, !b3D, 0);

        unsigned int uiBufferFlags = uiBaseFlags;
        if ((uiBufferFlags & (DSBCAPS_LOCHARDWARE | DSBCAPS_LOCSOFTWARE)) == 0)
        {
            uiBufferFlags |= DSBCAPS_LOCDEFER;
        }
        rBuffer.mDirectSound = (IDirectSound*)Engine::smDirectSound;
        rBuffer.mResourcesAcquired = (unsigned char)((uiBufferFlags & DSBCAPS_LOCDEFER) == 0);

        DSBUFFERDESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc);
        desc.dwFlags = uiBufferFlags;
        desc.dwBufferBytes = 4;
        desc.lpwfxFormat = &format.Format;
        desc.guid3DAlgorithm = DS3DALG_DEFAULT;
        HRESULT hr = rBuffer.mDirectSound->CreateSoundBuffer(
            &desc, (IDirectSoundBuffer**)&rBuffer.mBuffer, 0);
        if (!Engine::Succeeded(hr, kFile, 222))
        {
            rBuffer.mResourcesAcquired = 0;
            rBuffer.Destroy();
            break;
        }

        if (b3D)
        {
            hr = rBuffer.mBuffer->QueryInterface(IID_IDirectSound3DBuffer,
                                                  (void**)&rBuffer.mBuffer3D);
            if (!Engine::Succeeded(hr, kFile, 244))
            {
                rBuffer.Destroy();
                break;
            }
        }
        if (bSetupEffects && rBuffer.mBuffer3D)
        {
            rBuffer.SetupPropertySet(0);
        }

        rBuffer.mFlags = uiBufferFlags;
        rBuffer.mLength = 4;
        if (Engine::GetMixConfig() == Engine::mixConfig_DirectSoundSoftware)
        {
            rBuffer.mInterpolation = 0x0f;
        }
        if (rBuffer.mBuffer && (rBuffer.mFlags & DSBCAPS_CTRLPAN))
        {
            rBuffer.mBuffer->SetPan(0);
        }
        rBuffer.ResetGainInterpolation();
        rBuffer.SetGain(0.0f);
        rBuffer.ResetFrequencyInterpolation();
        rBuffer.mSampleRate = 44100;
        rBuffer.SetFrequency(44100);
        smList.AddLast(&rBuffer.mNode);
        rBuffer.ResetGainInterpolation();
        rBuffer.ResetFrequencyInterpolation();
        ++uiCreated;
    } while (uiCreated < uiCount);

    Engine::smErrorsEnabled = true;
    return uiCreated;
}

// FUNCTION: SWBF2 0x00743bb0
DSBuffer::WaveFormat::WaveFormat(SampleInfo::Format eFormat,
                                 unsigned int uiNumChannels,
                                 unsigned int uiSampleRate, bool bExtensible,
                                 unsigned long ulChannelMask)
{
    const unsigned short usBitsPerSample =
        (unsigned short)SampleInfo::GetBitDepth(eFormat);

    Format.nChannels = (WORD)uiNumChannels;
    Format.nSamplesPerSec = uiSampleRate;
    Format.wBitsPerSample = usBitsPerSample;
    Format.nBlockAlign = (WORD)((usBitsPerSample >> 3) * uiNumChannels);
    Format.nAvgBytesPerSec = Format.nBlockAlign * uiSampleRate;

    if (!bExtensible && ulChannelMask == 0)
    {
        Format.wFormatTag = WAVE_FORMAT_PCM;

        Format.cbSize = 14;
        return;
    }

    Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    Samples.wValidBitsPerSample = usBitsPerSample;

    Format.cbSize = 40;
    SubFormat = kSubFormatPcm;

    if (ulChannelMask != 0)
    {
        dwChannelMask = ulChannelMask;
        return;
    }

    dwChannelMask = 0;
    if (uiNumChannels == 1)
    {

        if (EngineBase::smSpeakerConfig < EngineBase::speakerConfig_5point1)
        {
            return;
        }
        dwChannelMask = SPEAKER_FRONT_CENTER;
    }
    else if (uiNumChannels > 4)
    {
        dwChannelMask = SPEAKER_FRONT_CENTER;
    }

    if (uiNumChannels >= 2)
    {
        dwChannelMask |= SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
    }
    if (uiNumChannels >= 4)
    {
        dwChannelMask |= SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
    }
    if (uiNumChannels >= 6)
    {
        dwChannelMask |= SPEAKER_LOW_FREQUENCY;
    }
    if (uiNumChannels >= 8)
    {
        dwChannelMask |= SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
    }
}

// FUNCTION: SWBF2 0x00743b30
DSBuffer::DSBuffer()
{

     

    mBuffer = 0;
    mBuffer3D = 0;
    mProperties = 0;
    mDirectSound = 0;
    mFlags = 0;
    mSampleRate = 48000;
    mFrequency = 0;
    mTargetFrequency = 0;
    mGain = 1.0f;
    mTargetGain = 1.0f;
    mLength = 0;
    mNode.mNext = 0;
    mNode.mPrev = 0;
    mResourcesAcquired = 0;
    mInterpolation = 0;
}

// FUNCTION: SWBF2 0x00743ca0
DSBuffer::~DSBuffer()
{
    Destroy();

    if (mNode.IsAttached())
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x00743d10
bool DSBuffer::AcquireResources()
{
    DWORD dwFlags;
    if ((mFlags & DSBCAPS_CTRL3D) != 0
        && mDirectSound == (IDirectSound*)Engine::smDirectSound
        && Engine::GetMixConfig() == Engine::mixConfig_DirectSoundHardware)
    {
        dwFlags = DSBPLAY_LOCHARDWARE;
    }
    else
    {
        dwFlags = DSBPLAY_LOCSOFTWARE;
    }

    GetStatus();

    if (mResourcesAcquired)
    {
        return true;
    }

    if (mProperties)
    {
        mProperties->Release();
    }

    mResourcesAcquired =
        (unsigned char)(mBuffer->AcquireResources(dwFlags, 0, 0) >= 0);

    if (mProperties)
    {
        mProperties = 0;
        SetupPropertySet(0);
    }

    return mResourcesAcquired != 0;
}

// FUNCTION: SWBF2 0x00743da0
bool DSBuffer::Create(SampleInfo::Format eFormat, unsigned int uiNumChannels,
                      unsigned int uiSampleRate, unsigned int uiLength,
                      bool b3D, bool bProperties, unsigned int uiFlags,
                      bool bPrimary, bool bSetFormat,
                      IDirectSound* pDirectSound, unsigned long ulChannelMask,
                      bool bRetryInSoftware)
{
    WaveFormat format(eFormat, uiNumChannels, uiSampleRate, !b3D, ulChannelMask);

    mDirectSound = pDirectSound ? pDirectSound
                                : (IDirectSound*)Engine::smDirectSound;

    unsigned int uiBufferFlags;
    unsigned int uiBufferBytes;
    WAVEFORMATEX* pwfx;

    if (bPrimary)
    {

        uiBufferFlags = (b3D ? DSBCAPS_CTRL3D : 0)
                      | (uiFlags & ~(DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN
                                     | DSBCAPS_CTRLVOLUME))
                      | DSBCAPS_PRIMARYBUFFER;
        uiBufferBytes = 0;
        pwfx = 0;
    }
    else
    {
        uiBufferFlags = uiFlags
                      | (b3D ? DSBCAPS_CTRL3D : DSBCAPS_CTRLPAN)
                      | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME
                      | DSBCAPS_CTRLFREQUENCY;
        if ((uiBufferFlags & (DSBCAPS_LOCHARDWARE | DSBCAPS_LOCSOFTWARE)) == 0)
        {
            uiBufferFlags |= DSBCAPS_LOCDEFER;
        }
        uiBufferBytes = uiLength;

        mResourcesAcquired =
            (unsigned char)((uiBufferFlags & DSBCAPS_LOCDEFER) == 0);
        pwfx = &format.Format;
    }

    DSBUFFERDESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(DSBUFFERDESC);
    desc.dwFlags = uiBufferFlags;
    desc.dwBufferBytes = uiBufferBytes;
    desc.lpwfxFormat = pwfx;

    desc.guid3DAlgorithm = DS3DALG_DEFAULT;

    HRESULT hr = mDirectSound->CreateSoundBuffer(
        &desc, (IDirectSoundBuffer**)&mBuffer, 0);

    if (!Engine::Succeeded(hr, kFile, 222))
    {
        mResourcesAcquired = 0;
        Destroy();

        if (bRetryInSoftware)
        {
            uiBufferFlags &= ~DSBCAPS_LOCHARDWARE;
            uiBufferFlags |= DSBCAPS_LOCSOFTWARE;
            return Create(eFormat, uiNumChannels, uiSampleRate, uiBufferBytes,
                          b3D, bProperties, uiBufferFlags, bPrimary, bSetFormat,
                          pDirectSound, ulChannelMask, false);
        }
        return false;
    }

    if (b3D && !bPrimary)
    {
        hr = mBuffer->QueryInterface(IID_IDirectSound3DBuffer,
                                     (void**)&mBuffer3D);
        if (!Engine::Succeeded(hr, kFile, 244))
        {
            Destroy();
            return false;
        }
    }

    if (bProperties && mBuffer3D)
    {
        SetupPropertySet(0);
    }

    mFlags = uiBufferFlags;
    mLength = uiBufferBytes;

    if (bSetFormat && bPrimary)
    {
        if (!SetFormat(format))
        {
            PrintDebugMessage("Unable to set primary buffer format\n");
        }
    }

    if (Engine::GetMixConfig() == Engine::mixConfig_DirectSoundSoftware)
    {
        mInterpolation = 0x0F;
    }

    SetPan(0.0f);

    ResetGainInterpolation();
    SetGain(0.0f);

    ResetFrequencyInterpolation();
    mSampleRate = uiSampleRate;
    SetFrequency(uiSampleRate);

    smList.AddLast(&mNode);

    ResetGainInterpolation();
    ResetFrequencyInterpolation();

    return true;
}

// FUNCTION: SWBF2 0x00744320
void DSBuffer::Destroy()
{
    if (mBuffer)
    {
        if ((GetStatus() & DSBSTATUS_PLAYING) != 0)
        {
            mBuffer->Stop();
        }

        if (mBuffer)
        {
            mBuffer->Release();
        }
    }

    if (mBuffer3D)
    {
        mBuffer3D->Release();
    }

    if (mProperties)
    {
        mProperties->Release();
    }

    mBuffer = 0;
    mBuffer3D = 0;
    mProperties = 0;
    mFlags = 0;
    mLength = 0;
    mResourcesAcquired = 0;
    mInterpolation = 0;

    if (mNode.IsAttached())
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x007443c0
void DSBuffer::DestroyBuffers(DSBuffer* pBuffers, unsigned int& ruiCount)
{
    while (ruiCount != 0)
    {
        --ruiCount;
        pBuffers[ruiCount].Destroy();
    }

    ruiCount = 0;
}

// FUNCTION: SWBF2 0x00744490
bool DSBuffer::GetFormat(SampleInfo::Format& reFormat,
                         unsigned int& ruiNumChannels,
                         unsigned int& ruiSampleRate) const
{
    bool bResult = false;

    DWORD dwSize;
    HRESULT hr = mBuffer->GetFormat(0, 0, &dwSize);
    if (!Engine::Succeeded(hr, kFile, 758))
    {
        return bResult;
    }

    WAVEFORMATEX* pwfx = (WAVEFORMATEX*)AllocTemp(dwSize);
    if (pwfx)
    {
        hr = mBuffer->GetFormat(pwfx, dwSize, 0);
        if (Engine::Succeeded(hr, kFile, 763))
        {
            bResult = true;
            reFormat = (pwfx->wBitsPerSample == 8) ? SampleInfo::format_PCM8
                                                   : SampleInfo::format_PCM16;
            ruiNumChannels = pwfx->nChannels;
            ruiSampleRate = pwfx->nSamplesPerSec;
        }
        FreeTemp(pwfx);
    }

    return bResult;
}

// FUNCTION: SWBF2 0x00744540
unsigned long DSBuffer::GetStatus()
{
    DWORD dwStatus;
    mBuffer->GetStatus(&dwStatus);

    if ((dwStatus & DSBSTATUS_BUFFERLOST) != 0)
    {
        mResourcesAcquired = 0;

        if (mBuffer->Restore() >= 0)
        {
            DSBCAPS caps;
            memset(&caps, 0, sizeof(caps));
            caps.dwSize = sizeof(DSBCAPS);
            mBuffer->GetCaps(&caps);

            void* pAudio1 = 0;
            void* pAudio2 = 0;
            DWORD dwBytes1 = 0;
            DWORD dwBytes2 = 0;
            HRESULT hr = mBuffer->Lock(0, caps.dwBufferBytes, &pAudio1,
                                       &dwBytes1, &pAudio2, &dwBytes2, 0);
            if (Engine::Succeeded(hr, kFile, 810))
            {
                if (dwBytes1)
                {
                    memset(pAudio1, 0, dwBytes1);
                }
                if (dwBytes2)
                {
                    memset(pAudio2, 0, dwBytes2);
                }
                mBuffer->Unlock(pAudio1, dwBytes1, pAudio2, dwBytes2);
            }
        }
    }

    if ((dwStatus & DSBSTATUS_TERMINATED) != 0)
    {
        mResourcesAcquired = 0;
    }

    return dwStatus
         & ~(DSBSTATUS_LOOPING | DSBSTATUS_LOCHARDWARE | DSBSTATUS_LOCSOFTWARE);
}

// FUNCTION: SWBF2 0x00744640
bool DSBuffer::IsPlaying()
{
    if (!mBuffer)
    {
        return false;
    }

    return (GetStatus() & DSBSTATUS_PLAYING) != 0;
}

// FUNCTION: SWBF2 0x00744660
bool DSBuffer::IsTerminated()
{
    return (GetStatus() & DSBSTATUS_TERMINATED) != 0;
}

// FUNCTION: SWBF2 0x00744670
bool DSBuffer::Play(bool bLooping, unsigned int uiPriority, bool bImmediate,
                    bool bRewind) const
{
    DWORD dwFlags;
    if ((mFlags & DSBCAPS_CTRL3D) != 0
        && mDirectSound == (IDirectSound*)Engine::smDirectSound
        && Engine::GetMixConfig() == Engine::mixConfig_DirectSoundHardware)
    {
        dwFlags = DSBPLAY_LOCHARDWARE | DSBPLAY_TERMINATEBY_TIME
                | DSBPLAY_TERMINATEBY_PRIORITY;
    }
    else
    {
        dwFlags = DSBPLAY_LOCSOFTWARE;
    }

    if (bRewind)
    {
        mBuffer->SetCurrentPosition(0);
    }

    if (bImmediate)
    {
        dwFlags = 0;
    }

    if (bLooping)
    {
        dwFlags |= DSBPLAY_LOOPING;
    }

    return mBuffer->Play(0, bImmediate ? 0 : uiPriority, dwFlags) >= 0;
}

// FUNCTION: SWBF2 0x007446f0
void DSBuffer::ResetBuffers()
{
    for (PblListDouble::Node* pNode = smList.mTerminator.mNext; pNode != &smList.mTerminator;
         pNode = pNode->mNext)
    {
        DSBuffer* pBuffer = (DSBuffer*)((char*)pNode - offsetof(DSBuffer, mNode));
        if (pBuffer->mBuffer)
        {
            pBuffer->ResetFrequencyInterpolation();
            pBuffer->ResetGainInterpolation();
        }
    }
}

// FUNCTION: SWBF2 0x00744730
void DSBuffer::ResetFrequencyInterpolation()
{
    mFrequency = mTargetFrequency;
    mInterpolation |= 0x02;
}

// FUNCTION: SWBF2 0x00744750
void DSBuffer::ResetGainInterpolation()
{
    mGain = mTargetGain;
    mInterpolation |= 0x08;
}

// FUNCTION: SWBF2 0x00744920
bool DSBuffer::SetFormat(WaveFormat& rFormat) const
{
    HRESULT hr;
    for (;;)
    {
        hr = mBuffer->SetFormat(&rFormat.Format);
        rFormat.Format.wBitsPerSample -= 8;

        if (rFormat.Format.wBitsPerSample == 0)
        {
            break;
        }
        if (Engine::Succeeded(hr, kFile, 849))
        {
            break;
        }
    }

    if (!Engine::Succeeded(hr, kFile, 852))
    {
        return false;
    }

    SampleInfo::Format eFormat;
    unsigned int uiNumChannels;
    unsigned int uiSampleRate;
    GetFormat(eFormat, uiNumChannels, uiSampleRate);

    rFormat = WaveFormat(eFormat, uiNumChannels, uiSampleRate, false, 0);
    return true;
}

// FUNCTION: SWBF2 0x007449d0
void DSBuffer::SetFrequency(unsigned int uiFrequency)
{
    if (!mBuffer)
    {
        return;
    }
    if ((mFlags & DSBCAPS_CTRLFREQUENCY) == 0)
    {
        return;
    }

    const unsigned char ucInterpolation = mInterpolation;
    const int iMax = (int)Engine::smSecBufSampleRateMax;

    if ((ucInterpolation & 0x01) != 0)
    {
        mTargetFrequency = (__int64)uiFrequency << 32;

        if ((ucInterpolation & 0x02) != 0)
        {
            mFrequency = mTargetFrequency;
        }

        if (mFrequency == mTargetFrequency && (ucInterpolation & 0x02) == 0)
        {
            return;
        }

        mFrequency += (mTargetFrequency - mFrequency) >> 8;

        int iHz = (int)(mFrequency >> 32);
        if (iHz < 100)
        {
            iHz = 100;
        }
        else if (iHz > iMax)
        {
            iHz = iMax;
        }

        mBuffer->SetFrequency(iHz);
        mInterpolation &= ~0x02;
        return;
    }

    if (mFrequency == ((__int64)uiFrequency << 32)
        && (ucInterpolation & 0x02) == 0)
    {
        return;
    }

    mInterpolation = (unsigned char)(ucInterpolation & ~0x02);

    int iHz = (int)uiFrequency;
    if (iHz < 100)
    {
        iHz = 100;
    }
    else if (iHz > iMax)
    {
        iHz = iMax;
    }

    mBuffer->SetFrequency(iHz);

    mFrequency = (__int64)uiFrequency << 32;
    mTargetFrequency = mFrequency;
}

// FUNCTION: SWBF2 0x00744ae0
void DSBuffer::SetGain(float fGain)
{
    if (!mBuffer)
    {
        return;
    }
    if ((mFlags & DSBCAPS_CTRLVOLUME) == 0)
    {
        return;
    }

    const unsigned char ucInterpolation = mInterpolation;

    if ((ucInterpolation & 0x04) != 0)
    {
        mTargetGain = fGain;

        if ((ucInterpolation & 0x08) != 0)
        {
            mGain = fGain;
        }

        if ((float)fabs(mGain - fGain) <= kGainEpsilon
            && (ucInterpolation & 0x08) == 0)
        {
            return;
        }

        mGain += (fGain - mGain) * 0.5f;

        long lVolume = ConvertLinearToLog(mGain);
        if (lVolume < DSBVOLUME_MIN)
        {
            lVolume = DSBVOLUME_MIN;
        }
        else if (lVolume > DSBVOLUME_MAX)
        {
            lVolume = DSBVOLUME_MAX;
        }

        mBuffer->SetVolume(lVolume);
        mInterpolation &= ~0x08;
        return;
    }

    if ((float)fabs(fGain - mGain) <= kGainEpsilon
        && (ucInterpolation & 0x08) == 0)
    {
        return;
    }

    mInterpolation = (unsigned char)(ucInterpolation & ~0x08);
    mGain = fGain;
    mTargetGain = fGain;

    long lVolume = ConvertLinearToLog(fGain);
    if (lVolume < DSBVOLUME_MIN)
    {
        lVolume = DSBVOLUME_MIN;
    }
    else if (lVolume > DSBVOLUME_MAX)
    {
        lVolume = DSBVOLUME_MAX;
    }

    mBuffer->SetVolume(lVolume);
}

// FUNCTION: SWBF2 0x00744c20
void DSBuffer::SetPan(float fPan) const
{
    if (!mBuffer)
    {
        return;
    }
    if ((mFlags & DSBCAPS_CTRLPAN) == 0)
    {
        return;
    }

    if (fPan == 0.0f)
    {
        mBuffer->SetPan(0);
        return;
    }

    if (fPan < 0.0f)
    {
        mBuffer->SetPan(ConvertLinearToLog(fPan + 1.0f));
    }
    else
    {
        mBuffer->SetPan(-ConvertLinearToLog(1.0f - fPan));
    }
}

// FUNCTION: SWBF2 0x00744ca0
bool DSBuffer::SetProperty(const GUID& rguidPropSet, unsigned long ulId,
                           void* pData, unsigned long ulLength) const
{
    return mProperties->Set(rguidPropSet, ulId, 0, 0, pData, ulLength) >= 0;
}

// FUNCTION: SWBF2 0x00744cd0
bool DSBuffer::SetupPropertySet(bool* pbSupported)
{
    if (AcquireResources())
    {
        if (mProperties)
        {
            mProperties->Release();
        }

        if (mBuffer3D->QueryInterface(IID_IKsPropertySet,
                                      (void**)&mProperties) >= 0)
        {

            bool bSupported = TestProperty(kEAX40ContextPropertySet, 1);
            bSupported |= TestProperty(kEAX40FXSlot0PropertySet, 0x10001);
            bSupported |= TestProperty(kEAX30BufferPropertySet, 1);

            if (pbSupported)
            {
                *pbSupported = bSupported;
            }

            if (bSupported)
            {
                return true;
            }

            mProperties->Release();
        }
    }

    mProperties = 0;
    return false;
}

// FUNCTION: SWBF2 0x00744dc0
void DSBuffer::Stop()
{
    mBuffer->Stop();
}

// FUNCTION: SWBF2 0x00744dd0
bool DSBuffer::TestProperty(const GUID& rguidPropSet, unsigned long ulId) const
{
    ULONG ulSupport;
    if (mProperties->QuerySupport(rguidPropSet, ulId, &ulSupport) < 0)
    {
        return false;
    }

    return (ulSupport & 3) == 3;
}

// FUNCTION: SWBF2 0x00744760
bool DSBuffer::Set3DParams(DS3DBUFFER& rParams, bool bDeferred) const
{
    DWORD dwApply = bDeferred ? DS3D_DEFERRED : DS3D_IMMEDIATE;

    if (mBuffer3D->SetAllParameters(&rParams, dwApply) >= 0)
    {
        return true;
    }

    bool bResult = Engine::Succeeded(
        mBuffer3D->SetMinDistance(rParams.flMinDistance, dwApply), kFile, 544);

    bResult &= Engine::Succeeded(
        mBuffer3D->SetMaxDistance(rParams.flMaxDistance, dwApply), kFile, 546);

    bResult &= Engine::Succeeded(
        mBuffer3D->SetPosition(rParams.vPosition.x, rParams.vPosition.y,
                               rParams.vPosition.z, dwApply), kFile, 550);

    bResult &= Engine::Succeeded(
        mBuffer3D->SetVelocity(rParams.vVelocity.x, rParams.vVelocity.y,
                               rParams.vVelocity.z, dwApply), kFile, 554);

    bResult &= Engine::Succeeded(
        mBuffer3D->SetConeAngles(rParams.dwInsideConeAngle,
                                 rParams.dwOutsideConeAngle, dwApply),
        kFile, 558);

    bResult &= Engine::Succeeded(
        mBuffer3D->SetConeOrientation(rParams.vConeOrientation.x,
                                      rParams.vConeOrientation.y,
                                      rParams.vConeOrientation.z, dwApply),
        kFile, 563);

    bResult &= Engine::Succeeded(
        mBuffer3D->SetConeOutsideVolume(rParams.lConeOutsideVolume, dwApply),
        kFile, 566);

    return bResult;
}

}
