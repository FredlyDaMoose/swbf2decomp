#pragma once

#if !defined(DIRECTSOUND_VERSION) || DIRECTSOUND_VERSION < 0x0800
#undef DIRECTSOUND_VERSION
#define DIRECTSOUND_VERSION 0x0800
#endif

#include <windows.h>
#include <mmreg.h>
#include <dsound.h>

#include "PebbleFL/Common/PblListDouble.h"
#include "SoundFL/source/sndsampleinfo.h"

namespace Snd
{

class DSBuffer
{
public:

    class WaveFormat : public WAVEFORMATEXTENSIBLE
    {
    public:
        WaveFormat(SampleInfo::Format eFormat, unsigned int uiNumChannels,
                   unsigned int uiSampleRate, bool bExtensible,
                   unsigned long ulChannelMask);
    };

    DSBuffer();
    ~DSBuffer();

    bool AcquireResources();

    bool Create(SampleInfo::Format eFormat, unsigned int uiNumChannels,
                unsigned int uiSampleRate, unsigned int uiLength, bool b3D,
                bool bProperties, unsigned int uiFlags, bool bPrimary,
                bool bSetFormat, IDirectSound* pDirectSound,
                unsigned long ulChannelMask, bool bRetryInSoftware);

    void Destroy();

    bool GetFormat(SampleInfo::Format& reFormat, unsigned int& ruiNumChannels,
                   unsigned int& ruiSampleRate) const;

    IDirectSoundBuffer8* GetBuffer() const { return mBuffer; }
    IDirectSound3DBuffer* GetBuffer3D() const { return mBuffer3D; }
    IKsPropertySet* GetPropertySet() const { return mProperties; }

    bool IsPlaying();
    bool IsTerminated();

    bool Play(bool bLooping, unsigned int uiPriority, bool bImmediate,
              bool bRewind) const;

    static void __cdecl ResetBuffers();

    void ResetFrequencyInterpolation();
    void ResetGainInterpolation();

    void SetFrequency(unsigned int uiFrequency);
    void SetGain(float fGain);
    void SetPan(float fPan) const;

    bool SetProperty(const GUID& rguidPropSet, unsigned long ulId,
                     void* pData, unsigned long ulLength) const;

    void Stop();

    bool TestProperty(const GUID& rguidPropSet,
                      unsigned long ulId) const;

    static unsigned int __cdecl CreateBuffers(bool b3D, bool bProperties,
                                              unsigned int uiFlags,
                                              DSBuffer* pBuffers,
                                              unsigned int uiCount,
                                              IDirectSound* pDirectSound);

    static void __cdecl DestroyBuffers(DSBuffer* pBuffers,
                                       unsigned int& ruiCount);

    bool Set3DParams(DS3DBUFFER& rParams, bool bImmediate) const;

private:

    bool SetupPropertySet(bool* pbSupported);

    bool SetFormat(WaveFormat& rFormat) const;

    unsigned long GetStatus();

    IDirectSoundBuffer8* mBuffer;

    IDirectSound3DBuffer* mBuffer3D;
    IKsPropertySet* mProperties;

    IDirectSound* mDirectSound;
    unsigned int mFlags;

    unsigned int mSampleRate;

    __int64 mFrequency;

    __int64 mTargetFrequency;
    float mGain;
    float mTargetGain;
    unsigned int mLength;
    PblListDouble::Node mNode;

    unsigned char mResourcesAcquired;

    unsigned char mInterpolation;

    static PblListDouble smList;
};
 
 

}
