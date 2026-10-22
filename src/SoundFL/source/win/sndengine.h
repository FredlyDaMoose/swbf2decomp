#pragma once

#if !defined(DIRECTSOUND_VERSION) || DIRECTSOUND_VERSION < 0x0800
#undef DIRECTSOUND_VERSION
#define DIRECTSOUND_VERSION 0x0800
#endif

#include <windows.h>
#include <dsound.h>

#include "SoundFL/source/sndenginebase.h"
#include "SoundFL/source/sndsampleinfo.h"

namespace Snd
{

class DSBuffer;
class I3DL2Reverb;
class SoftOutput;
class Source3D;
class SourceI3DL2;
class ThreadUpdate;

class Engine : public EngineBase
{
public:

    enum MixConfig
    {
        mixConfig_Invalid = 0,
        mixConfig_Software = 1,
        mixConfig_DirectSoundHardware = 2,
        mixConfig_DirectSoundSoftware = 3,
        mixConfig_Disabled = 4
    };

    static bool __cdecl Failed(long hr, const char* pcFile,
                               unsigned int uiLine);
    static bool __cdecl Succeeded(long hr, const char* pcFile,
                                  unsigned int uiLine);
    static bool __cdecl GetBassManagement();
    static const DSCAPS& __cdecl GetDSCaps();
    static unsigned int __cdecl GetEAXVersion();
    static float __cdecl GetMinStreamBufferSizeMs();
    static MixConfig __cdecl GetMixConfig();
    static unsigned int __cdecl GetOutputFrequency();
    static SoftOutput* __cdecl GetSoftOutput();
    static ThreadUpdate* __cdecl GetThreadUpdate();
    static void __cdecl PrintDSError(long hr, const char* pcFile,
                                     unsigned int uiLine);
    static void __cdecl SetBassManagement(bool bEnable);
    static void __cdecl SetGlobalReverb(const I3DL2Reverb* pReverb);

    static const I3DL2Reverb* GetGlobalReverb() { return smReverb; }
    static void __cdecl SetSpeakerConfig(EngineBase::SpeakerConfig eConfig);

    static bool __cdecl Open(HWND hwnd, MixConfig eMixConfig, bool bEnable,
                             float fMinStreamBufferSizeMs, bool bKmixer,
                             unsigned int uiOutputFrequency,
                             unsigned int uiNumVoices,
                             unsigned long* pulWindowsSpeakerConfig,
                             const GUID* pcGuidDevice);

    static void __cdecl Calculate3DParams(Source3D* const pSource,
                                          DS3DBUFFER* const pParams);

    static void __cdecl SetupDirectSoundEffects(SourceI3DL2* const pSource,
                                                DSBuffer* const pBuffer);
    static void __cdecl SetupEAXSource(void* const pA, void* const pB,
                                       SourceI3DL2* const pSource);

    static bool __cdecl InitializeEAX();

    static void __cdecl Close(unsigned long ulFlags, bool b);

    static void __cdecl Update(float fDeltaTime);

    static void __cdecl UpdateCooperativeLevel(IDirectSound* pDirectSound,
                                               bool bForce);

    static void __cdecl SetCentrePeakMode(bool bEnable);

    static bool __cdecl DSClose();

    static IDirectSound8* smDirectSound;

    static IDirectSound3DListener* smDirectSoundListener;
    static bool smUpdateListener;
    static bool smUpdateFocusWindow;
    static bool smHardwareEffectsAvailable;
    static bool smPrimaryWriteAccess;
    static MixConfig smMixConfig;
    static unsigned char* smSampleRAM;
    static float smSecBufSampleRateMin;
    static unsigned char smSampleRAMBitmap[0x8000];
    static DSCAPS smDirectSoundCaps;
    static float smSecBufSampleRateMax;
    static float smUpdateFocusWindowTimer;
    static SampleInfo::Format smSecBufFormat;
    static float smDeltaTime;
    static I3DL2Reverb* smReverb;
    static GUID* smEAXFXSlot;
    static bool smSoftwareEffectsEnabled;
    static bool smCentrePeakMode;
    static bool smBassManagement;
    static unsigned int smOutputFrequency;
    static unsigned long smCooperativeLevel;
    static GUID smOutputDevice;
    static SoftOutput* smSoftOutput;
    static HWND smApplicationWindow;

    static DSBuffer smDummySecBuffer;

    static bool smErrorsEnabled;
    static float smMinStreamBufferSize;
};
 

}
