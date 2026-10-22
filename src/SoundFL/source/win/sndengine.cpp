#include <windows.h>
#include <objbase.h>

#include <d3dx9math.h>

#include <stdlib.h>
#include <string.h>

#include "sndengine.h"

#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "SoundFL/source/sndallocbitmap.h"
#include "SoundFL/source/sndenginebase.h"
#include "SoundFL/source/sndfile.h"
#include "SoundFL/source/sndglobals.h"
#include "SoundFL/source/sndi3dl2reverb.h"
#include "SoundFL/source/sndsourcetransport.h"
#include "SoundFL/source/sndstreamhighlowpass.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "SoundFL/source/sndvoicebase.h"
#include "snddsbuffer.h"
#include "sndsoftoutput.h"
#include "sndstream.h"
#include "sndvoice.h"

// FUNCTION: SWBF2 0x00731830
template <> void Snd::Allocator<Snd::SoftOutput>::operator delete(void* p, void*, bool)
{
    if (!p)
    {
        return;
    }
    switch (((Snd::SoftOutput*)p)->GetAllocType())
    {
    case Snd::Allocator<Snd::SoftOutput>::allocType_heap:
        Snd::Free(p);
        break;
    case Snd::Allocator<Snd::SoftOutput>::allocType_tempHeap:
        Snd::FreeTemp(p);
        break;
    default:
        break;
    }
}

void __cdecl PblTraceP(const char* Format, ...);

namespace Snd
{

// GLOBAL: SWBF2 0x00787c5c

extern const GUID kEAX40ContextPropertySet =
    { 0x1D4870AD, 0x0DEF, 0x43C0, { 0xA4, 0x0C, 0x52, 0x36, 0x32, 0x29, 0x63, 0x42 } };
// GLOBAL: SWBF2 0x00787c6c

extern const GUID kEAX40FXSlot0PropertySet =
    { 0xC4D79F1E, 0xF1AC, 0x436B, { 0xA8, 0x1D, 0xA7, 0x38, 0xE7, 0x04, 0x54, 0x69 } };
// GLOBAL: SWBF2 0x00787cac

extern const GUID kEAX30BufferPropertySet =
    { 0x1B86B823, 0x22DF, 0x4EAE, { 0x8B, 0x3C, 0x12, 0x78, 0xCE, 0x54, 0x42, 0x27 } };
// GLOBAL: SWBF2 0x00787cbc

static const GUID kEAX20ListenerPropertySet =
    { 0x0CF95C8F, 0xA3CC, 0x4849, { 0xB0, 0xB6, 0x83, 0x2E, 0xCC, 0x18, 0x22, 0xDF } };

// GLOBAL: SWBF2 0x00787dbc

static const GUID kAlternateDirectSoundClassId =
    { 0xCA503B60, 0xB176, 0x11D4, { 0xA0, 0x94, 0xD0, 0xC0, 0xBF, 0x3A, 0x56, 0x0C } };

// GLOBAL: SWBF2 0x009cfda0
IDirectSound8* Engine::smDirectSound;
// GLOBAL: SWBF2 0x009cfda4
IDirectSound3DListener* Engine::smDirectSoundListener;
// GLOBAL: SWBF2 0x009cfda8
bool Engine::smUpdateListener;
// GLOBAL: SWBF2 0x009cfda9
bool Engine::smUpdateFocusWindow;
// GLOBAL: SWBF2 0x009cfdaa
bool Engine::smHardwareEffectsAvailable;
// GLOBAL: SWBF2 0x009cfdab
bool Engine::smPrimaryWriteAccess;
// GLOBAL: SWBF2 0x009cfdac
Engine::MixConfig Engine::smMixConfig;
// GLOBAL: SWBF2 0x009cfdb0
unsigned char* Engine::smSampleRAM;
// GLOBAL: SWBF2 0x009cfdb4
float Engine::smSecBufSampleRateMin;
// GLOBAL: SWBF2 0x009cfdb8
unsigned char Engine::smSampleRAMBitmap[0x8000];
// GLOBAL: SWBF2 0x009d7db8
DSCAPS Engine::smDirectSoundCaps;
// GLOBAL: SWBF2 0x009d7e18
float Engine::smSecBufSampleRateMax;
// GLOBAL: SWBF2 0x009d7e1c
float Engine::smUpdateFocusWindowTimer;
// GLOBAL: SWBF2 0x009d7e20
SampleInfo::Format Engine::smSecBufFormat;
// GLOBAL: SWBF2 0x009d7e24
float Engine::smDeltaTime;
// GLOBAL: SWBF2 0x009d7e28
I3DL2Reverb* Engine::smReverb;
// GLOBAL: SWBF2 0x009d7e2c
GUID* Engine::smEAXFXSlot;
// GLOBAL: SWBF2 0x009d7e30
bool Engine::smSoftwareEffectsEnabled;
// GLOBAL: SWBF2 0x009d7e31
bool Engine::smCentrePeakMode;
// GLOBAL: SWBF2 0x009d7e32
bool Engine::smBassManagement;
// GLOBAL: SWBF2 0x009d7e34
unsigned int Engine::smOutputFrequency;
// GLOBAL: SWBF2 0x009d7e38
unsigned long Engine::smCooperativeLevel;
// GLOBAL: SWBF2 0x009d7e3c
GUID Engine::smOutputDevice;
// GLOBAL: SWBF2 0x009d7e4c
SoftOutput* Engine::smSoftOutput;
// GLOBAL: SWBF2 0x009d7e50
HWND Engine::smApplicationWindow;

// GLOBAL: SWBF2 0x009d7e68
// SYNTHETIC: SWBF2 0x0040b500
// `dynamic initializer for 'Snd::Engine::smDummySecBuffer''
// SYNTHETIC: SWBF2 0x0076a160
// `dynamic atexit destructor for 'Snd::Engine::smDummySecBuffer''
DSBuffer Engine::smDummySecBuffer;

// GLOBAL: SWBF2 0x007e3310
bool Engine::smErrorsEnabled = true;
// GLOBAL: SWBF2 0x007e3314
float Engine::smMinStreamBufferSize = 100.0f;

// GLOBAL: SWBF2 0x009d7eac
// SYNTHETIC: SWBF2 0x0040b560
// `dynamic initializer for 'Snd::sndEngineThreadUpdate''
// SYNTHETIC: SWBF2 0x0076a1a0
// `dynamic atexit destructor for 'Snd::sndEngineThreadUpdate''
ThreadUpdate sndEngineThreadUpdate(0, 0x2000);

// GLOBAL: SWBF2 0x009cf4b0
unsigned char gSoftOutputStorage[2288];

// FUNCTION: SWBF2 0x00731c90
bool Engine::Failed(long hr, const char* pcFile, unsigned int uiLine)
{
    if (FAILED(hr))
    {
        PrintDSError(hr, pcFile, uiLine);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x007331b0
bool Engine::Succeeded(long hr, const char* pcFile, unsigned int uiLine)
{
    if (SUCCEEDED(hr))
    {
        return true;
    }

    PrintDSError(hr, pcFile, uiLine);
    return false;
}

// FUNCTION: SWBF2 0x007332a0
void Engine::UpdateCooperativeLevel(IDirectSound* pDirectSound, bool bForce)
{
    if (pDirectSound == 0)
    {
        return;
    }

    if (!bForce)
    {
        if (smApplicationWindow != 0)
        {
            return;
        }
        if (1.0f > smUpdateFocusWindowTimer)
        {
            return;
        }
    }

    DWORD dwLevel = DSSCL_EXCLUSIVE;
    if (pDirectSound == smDirectSound)
    {
        dwLevel = smCooperativeLevel;
    }

    if (smUpdateFocusWindow)
    {

        pDirectSound->SetCooperativeLevel(GetForegroundWindow(), dwLevel);
    }
    else if (smApplicationWindow != 0)
    {

        pDirectSound->SetCooperativeLevel(smApplicationWindow, dwLevel);
    }
}

// FUNCTION: SWBF2 0x007331e0
void Engine::Update(float fDeltaTime)
{
    smDeltaTime = fDeltaTime;

    sndEngineThreadUpdate.Pause(false);
    EngineBase::Update(fDeltaTime, true);
    if (smSoftOutput)
    {
        smSoftOutput->Update();
    }
    DSBufferRenderer::UpdateParameters();
    smUpdateListener = true;
    sndEngineThreadUpdate.Resume();

    if (smUpdateFocusWindowTimer > 1.0f)
    {
        smUpdateFocusWindowTimer = 0.0f;
    }
    smUpdateFocusWindowTimer += fDeltaTime;

    UpdateCooperativeLevel(smDirectSound, false);
}

// FUNCTION: SWBF2 0x00732860
void Engine::PrintDSError(long hr, const char* pcFile, unsigned int uiLine)
{
    if (!smErrorsEnabled)
    {
        return;
    }

    switch (hr)
    {
    case DSERR_UNSUPPORTED:

        return;

    case DSERR_ALLOCATED:
        PrintDebugMessage("DirectSound Error: The request failed because "
                          "resources, such as a priority level, were already "
                          "in use by another caller. \n", pcFile, uiLine);
        break;

    case DSERR_CONTROLUNAVAIL:
        PrintDebugMessage("DirectSound Error: The buffer control (volume, "
                          "pan, and so on) requested by the caller is not "
                          "available. \n", pcFile, uiLine);
        break;

    case DSERR_INVALIDPARAM:
        PrintDebugMessage("DirectSound Error: An invalid parameter was passed "
                          "to the returning function. \n", pcFile, uiLine);
        break;

    case DSERR_INVALIDCALL:
        PrintDebugMessage("DirectSound Error: This function is not valid for "
                          "the current state of this object. \n",
                          pcFile, uiLine);
        break;

    case DSERR_GENERIC:
        PrintDebugMessage("DirectSound Error: An undetermined error occurred "
                          "inside the DirectSound subsystem. \n",
                          pcFile, uiLine);
        break;

    case DSERR_PRIOLEVELNEEDED:
        PrintDebugMessage("DirectSound Error: The caller does not have the "
                          "priority level required for the function to "
                          "succeed. \n", pcFile, uiLine);
        break;

    case DSERR_OUTOFMEMORY:
        PrintDebugMessage("DirectSound Error: The DirectSound subsystem could "
                          "not allocate sufficient memory to complete the "
                          "caller's request. \n", pcFile, uiLine);
        break;

    case DSERR_BADFORMAT:
        PrintDebugMessage("DirectSound Error: The specified wave format is "
                          "not supported.\n", pcFile, uiLine);
        break;

    case DSERR_NODRIVER:
        PrintDebugMessage("DirectSound Error: No sound driver is available "
                          "for use, or the given GUID is not a valid "
                          "DirectSound device ID. \n", pcFile, uiLine);
        break;

    case DSERR_ALREADYINITIALIZED:
        PrintDebugMessage("DirectSound Error: The object is already "
                          "initialized. \n", pcFile, uiLine);
        break;

    case DSERR_NOAGGREGATION:
        PrintDebugMessage("DirectSound Error: The object does not support "
                          "aggregation. \n", pcFile, uiLine);
        break;

    case DSERR_BUFFERLOST:
        PrintDebugMessage("DirectSound Error: The buffer memory has been lost "
                          "and must be restored.\n", pcFile, uiLine);
        break;

    case DSERR_OTHERAPPHASPRIO:
        PrintDebugMessage("DirectSound Error: Another application has a "
                          "higher priority level, preventing this call from "
                          "succeeding. \n", pcFile, uiLine);
        break;

    case DSERR_UNINITIALIZED:
        PrintDebugMessage("DirectSound Error: The IDirectSound8::Initialize "
                          "method has not been called or has not been called "
                          "successfully before other methods were called. \n",
                          pcFile, uiLine);
        break;

    case DSERR_NOINTERFACE:
        PrintDebugMessage("DirectSound Error: The requested COM interface is "
                          "not available. \n", pcFile, uiLine);
        break;

    case DSERR_ACCESSDENIED:
        PrintDebugMessage("DirectSound Error: The request failed because "
                          "access was denied.\n", pcFile, uiLine);
        break;

    case DSERR_BUFFERTOOSMALL:
        PrintDebugMessage("DirectSound Error: The buffer size is not great "
                          "enough to enable effects processing. \n",
                          pcFile, uiLine);
        break;

    case DSERR_DS8_REQUIRED:
        PrintDebugMessage("DirectSound Error: A DirectSound object of class "
                          "CLSID_DirectSound8 or later is required for the "
                          "requested functionality. For more information, see "
                          "IDirectSound8. \n", pcFile, uiLine);

    case DSERR_FXUNAVAILABLE:
        PrintDebugMessage("DirectSound Error: The effects requested could not "
                          "be found on the system, or they are in the wrong "
                          "order or in the wrong location; for example, an "
                          "effect expected in hardware was found in "
                          "software. \n", pcFile, uiLine);
        break;

    case DSERR_SENDLOOP:
        PrintDebugMessage("DirectSound Error: A circular loop of send effects "
                          "was detected. \n", pcFile, uiLine);
        break;

    case DSERR_BADSENDBUFFERGUID:
        PrintDebugMessage("DirectSound Error: The GUID specified in an "
                          "audiopath file does not match a valid mix-in "
                          "buffer.\n", pcFile, uiLine);
        break;

    case DSERR_OBJECTNOTFOUND:
        PrintDebugMessage("DirectSound Error: The requested object was not "
                          "found. \n", pcFile, uiLine);
        break;

    default:
        PrintDebugMessage("DirectSound Error: Undocumented DirectSound error "
                          "has occurred. \n", pcFile, uiLine);
        break;
    }
}

// FUNCTION: SWBF2 0x00731cc0
bool Engine::GetBassManagement()
{
    return smBassManagement;
}

// FUNCTION: SWBF2 0x00731cd0
const DSCAPS& Engine::GetDSCaps()
{
    return smDirectSoundCaps;
}

// FUNCTION: SWBF2 0x00731ce0
unsigned int Engine::GetEAXVersion()
{
    unsigned int uiVersion = 0;

    if (smDummySecBuffer.GetBuffer() != 0 && smDummySecBuffer.GetPropertySet() != 0)
    {
        if (smDummySecBuffer.TestProperty(kEAX40ContextPropertySet, 1))
        {
            return 4;
        }
        if (smDummySecBuffer.TestProperty(kEAX30BufferPropertySet, 3))
        {
            return 3;
        }
        if (smDummySecBuffer.TestProperty(kEAX20ListenerPropertySet, 1))
        {
            uiVersion = 2;
        }
    }

    return uiVersion;
}

// FUNCTION: SWBF2 0x00731d50
float Engine::GetMinStreamBufferSizeMs()
{
    return smMinStreamBufferSize;
}

// FUNCTION: SWBF2 0x00731d60
Engine::MixConfig Engine::GetMixConfig()
{
    return smMixConfig;
}

// FUNCTION: SWBF2 0x00731d70
unsigned int Engine::GetOutputFrequency()
{
    return smOutputFrequency;
}

// FUNCTION: SWBF2 0x00731d80
SoftOutput* Engine::GetSoftOutput()
{
    return smSoftOutput;
}

// FUNCTION: SWBF2 0x00731d90
ThreadUpdate* Engine::GetThreadUpdate()
{
    return &sndEngineThreadUpdate;
}

// FUNCTION: SWBF2 0x00732b30
void Engine::SetBassManagement(bool bEnable)
{
    if (smMixConfig == mixConfig_DirectSoundHardware &&
        (EngineBase::smSpeakerConfig == EngineBase::speakerConfig_5point1 ||
         EngineBase::smSpeakerConfig == EngineBase::speakerConfig_7point1))
    {
        if (bEnable && smSoftOutput != 0)
        {
            sndEngineThreadUpdate.Pause(false);

            DSBufferRenderer& rRenderer = smSoftOutput->GetRenderer();

            const bool b3D = (rRenderer.Source2D3D::mFlags & 1) != 0;

            rRenderer.SetFormat(rRenderer.mFormat, rRenderer.mNumChannels,
                                rRenderer.mFrequency, b3D,
                                rRenderer.GetBufferSizeMs(), false, 0, 0x0C, true);

            sndEngineThreadUpdate.Resume();
        }

        smBassManagement = bEnable;
        return;
    }

    smBassManagement = false;
}

// FUNCTION: SWBF2 0x00732bf0
void Engine::SetCentrePeakMode(bool bEnable)
{

    if (smCentrePeakMode == bEnable)
    {
        return;
    }

    bool bWanted = false;

    if (bEnable && smMixConfig == mixConfig_DirectSoundHardware &&
        (EngineBase::smSpeakerConfig == EngineBase::speakerConfig_5point1 ||
         EngineBase::smSpeakerConfig == EngineBase::speakerConfig_7point1))
    {
        bWanted = true;
    }

    if (smCentrePeakMode == bWanted)
    {
        return;
    }

    for (unsigned int uiOffset = 0; uiOffset < 0xA800; uiOffset += 0x540)
    {

        Voice& rVoice =
            *(Voice*)((unsigned char*)EngineBase::smVoices + uiOffset);

        if (bWanted)
        {
            rVoice.Source3D::Properties::mFlags |=
                Source3D::Properties::flags_centrePeakMode;
        }
        else
        {
            rVoice.Source3D::Properties::mFlags &=
                (unsigned char)~Source3D::Properties::flags_centrePeakMode;
        }
    }

    smCentrePeakMode = bWanted;
}

struct EAXREVERBPROPERTIES
{
    unsigned long ulEnvironment;
    float flEnvironmentSize;
    float flEnvironmentDiffusion;
    long lRoom;
    long lRoomHF;
    long lRoomLF;
    float flDecayTime;
    float flDecayHFRatio;
    float flDecayLFRatio;
    long lReflections;
    float flReflectionsDelay;
    float vReflectionsPan[3];
    long lReverb;
    float flReverbDelay;
    float vReverbPan[3];
    float flEchoTime;
    float flEchoDepth;
    float flModulationTime;
    float flModulationDepth;
    float flAirAbsorptionHF;
    float flHFReference;
    float flLFReference;
    float flRoomRolloffFactor;
    unsigned long ulFlags;
};
 

static float ClampRange(float f, float fMin, float fMax)
{
    if (fMin > f)
    {
        return fMin;
    }
    if (f > fMax)
    {
        return fMax;
    }
    return f;
}

// FUNCTION: SWBF2 0x00732c60
void Engine::SetGlobalReverb(const I3DL2Reverb* pReverb)
{
    if (smMixConfig == mixConfig_DirectSoundHardware)
    {
        if (smDummySecBuffer.GetPropertySet() != 0 && smEAXFXSlot != 0)
        {
            EAXREVERBPROPERTIES props;

            props.ulEnvironment = 0;
            props.flEnvironmentSize = 7.5f;
            props.flEnvironmentDiffusion =
                ClampRange(pReverb->mDiffusion, 0.0f, 1.0f);
            props.lRoom = ConvertLinearToLog(pReverb->mRoomGain);
            props.lRoomHF = ConvertLinearToLog(pReverb->mRoomHFGain);
            props.lRoomLF = 0;
            props.flDecayTime =
                ClampRange(pReverb->mDecayTime, 0.1f, 20.0f);
            props.flDecayHFRatio =
                ClampRange(pReverb->mDecayHFRatio, 0.1f, 2.0f);
            props.flDecayLFRatio = 1.0f;
            props.lReflections = ConvertLinearToLog(pReverb->mERGain);
            props.flReflectionsDelay =
                ClampRange(pReverb->mERDelay, 0.0f, 0.3f);
            props.vReflectionsPan[0] = 0.0f;
            props.vReflectionsPan[1] = 0.0f;
            props.vReflectionsPan[2] = 0.0f;
            props.lReverb = ConvertLinearToLog(pReverb->mReverbGain);
            props.flReverbDelay =
                ClampRange(pReverb->mReverbDelay, 0.0f, 0.1f);
            props.vReverbPan[0] = 0.0f;
            props.vReverbPan[1] = 0.0f;
            props.vReverbPan[2] = 0.0f;
            props.flEchoTime = 0.25f;
            props.flEchoDepth = 0.0f;
            props.flModulationTime = 0.25f;
            props.flModulationDepth = 0.0f;
            props.flAirAbsorptionHF = 0.0f;
            props.flHFReference =
                ClampRange(pReverb->mHFReference, 1000.0f, 20000.0f);
            props.flLFReference = 250.0f;
            props.flRoomRolloffFactor =
                ClampRange(pReverb->mRoomRollOff, 0.0f, 10.0f);
            props.ulFlags = 0;

            smDummySecBuffer.SetProperty(*smEAXFXSlot, 1, &props,
                                         sizeof(props));
        }
    }
    else if (smMixConfig == mixConfig_Software)
    {
        smSoftOutput->SetReverb(pReverb);
    }

    smReverb = const_cast<I3DL2Reverb*>(pReverb);
}

// FUNCTION: SWBF2 0x00732ee0
void Engine::SetSpeakerConfig(EngineBase::SpeakerConfig eConfig)
{
    if (eConfig != EngineBase::smSpeakerConfig)
    {
        if (smMixConfig == mixConfig_Software)
        {
            unsigned long ulSpeakerConfig;

            if (eConfig > EngineBase::speakerConfig_AnalogSurround)
            {
                eConfig = EngineBase::speakerConfig_AnalogSurround;
                ulSpeakerConfig = DSSPEAKER_STEREO;
            }
            else if (eConfig == EngineBase::speakerConfig_Mono)
            {
                ulSpeakerConfig = DSSPEAKER_MONO;
            }
            else
            {
                ulSpeakerConfig = DSSPEAKER_STEREO;
            }

            smDirectSound->SetSpeakerConfig(ulSpeakerConfig);
            EngineBase::SetSpeakerConfig(eConfig);
        }
        else if (smMixConfig == mixConfig_DirectSoundHardware ||
                 smMixConfig == mixConfig_DirectSoundSoftware)
        {
            unsigned long ulSpeakerConfig;

            switch (eConfig)
            {
            case EngineBase::speakerConfig_Mono:
                ulSpeakerConfig = DSSPEAKER_MONO;
                break;
            case EngineBase::speakerConfig_Headphones:
                ulSpeakerConfig = DSSPEAKER_HEADPHONE;
                break;
            case EngineBase::speakerConfig_Stereo:
                ulSpeakerConfig = DSSPEAKER_STEREO;
                break;
            case EngineBase::speakerConfig_AnalogSurround:
                ulSpeakerConfig = DSSPEAKER_SURROUND;
                break;
            case EngineBase::speakerConfig_Quadraphonic:
                ulSpeakerConfig = DSSPEAKER_QUAD;
                break;
            case EngineBase::speakerConfig_5point1:
                ulSpeakerConfig = DSSPEAKER_5POINT1;
                break;
            case EngineBase::speakerConfig_7point1:
                ulSpeakerConfig = DSSPEAKER_7POINT1;
                break;
            default:
                ulSpeakerConfig = 0;
                break;
            }

            if (SUCCEEDED(smDirectSound->SetSpeakerConfig(ulSpeakerConfig)))
            {
                EngineBase::smSpeakerConfig = eConfig;
                smBassManagement = false;

                SetCentrePeakMode(
                    (ulSpeakerConfig == DSSPEAKER_5POINT1 ||
                     ulSpeakerConfig == DSSPEAKER_7POINT1) &&
                    smMixConfig == mixConfig_DirectSoundHardware);
            }
        }
    }

    SetBassManagement(smBassManagement);
}

}

class EngineUpdate : public Snd::ThreadUpdate::Object
{
public:
    EngineUpdate() {}
    virtual ~EngineUpdate() {}
    virtual void UpdateObject();
};

// SYNTHETIC: SWBF2 0x00731860
// EngineUpdate::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00733310
void EngineUpdate::UpdateObject()
{
    if (Snd::Engine::smUpdateListener && Snd::Engine::smDirectSoundListener &&
        Snd::Engine::smMixConfig != Snd::Engine::mixConfig_Software)
    {
        Snd::Engine::smDirectSoundListener->CommitDeferredSettings();
        Snd::Engine::smUpdateListener = false;
    }
}

// GLOBAL: SWBF2 0x009d7e54
// SYNTHETIC: SWBF2 0x0040b520
// `dynamic initializer for 'smEngineUpdater''
// SYNTHETIC: SWBF2 0x0076a170
// `dynamic atexit destructor for 'smEngineUpdater''
static EngineUpdate smEngineUpdater;

namespace Snd
{

}

// GLOBAL: SWBF2 0x009d7ed8
// SYNTHETIC: SWBF2 0x0040b540
// `dynamic initializer for '_smHighLowPass''
// SYNTHETIC: SWBF2 0x0076a190
// `dynamic atexit destructor for '_smHighLowPass''
Snd::StreamLowHighPass _smHighLowPass;

// GLOBAL: SWBF2 0x009d7f90
unsigned char _smHighLowPassBuffer[1024];

namespace Snd
{

// FUNCTION: SWBF2 0x00731ad0
bool Engine::DSClose()
{

    if (smMixConfig != mixConfig_Software && smMixConfig != mixConfig_Disabled)
    {
        sndEngineThreadUpdate.RemoveObject(&smEngineUpdater);
    }

    if (smSoftOutput)
    {
        sndEngineThreadUpdate.Pause(false);

        if (smMixConfig == mixConfig_DirectSoundHardware)
        {
            StreamLowHighPass* const pFilter = &_smHighLowPass;
            DSBufferRenderer& rRenderer = smSoftOutput->GetRenderer();

            rRenderer.mConnection.mInputNode =
                pFilter->mSink.mConnection.mInputNode;
            rRenderer.mConnection.mInputNodeOutputIndex = 0;

            pFilter->mSink.mConnection.mInputNode = 0;
            pFilter->mSink.mConnection.mInputNodeOutputIndex = 0;
            pFilter->Flush();
        }

        smSoftOutput->mFlags =
            (unsigned short)((smSoftOutput->mFlags &
                              ~(SourceTransport::flags_updatePlay |
                                SourceTransport::flags_updatePause)) |
                             SourceTransport::flags_updateStop);

        smSoftOutput->Update();
        sndEngineThreadUpdate.Resume();

        SoftOutput* const pSoftOutput = smSoftOutput;
        if (pSoftOutput)
        {

            pSoftOutput->~SoftOutput();

            switch (pSoftOutput->GetAllocType())
            {
            case Allocator<SoftOutput>::allocType_heap:
                Free(pSoftOutput);
                break;
            case Allocator<SoftOutput>::allocType_tempHeap:
                FreeTemp(pSoftOutput);
                break;
            default:
                break;
            }
        }

        smSoftOutput = 0;
    }

    sndEngineThreadUpdate.Pause(false);

    for (unsigned int uiStream = 0; uiStream < 6; uiStream++)
    {
        Stream* const pStream = &EngineBase::smStreams[uiStream];

        pStream->SourceBus::Stop(pStream);

        pStream->SourceTransport::mFlags =
            (unsigned short)((pStream->SourceTransport::mFlags &
                              ~(SourceTransport::flags_updatePlay |
                                SourceTransport::flags_updatePause)) |
                             SourceTransport::flags_updateStop);

        pStream->Update(0.0f);
    }

    DSBufferRenderer::UpdateParameters();
    sndEngineThreadUpdate.Resume();

    EngineBase::Close();
    smDummySecBuffer.Destroy();

    if (smDirectSoundListener)
    {
        smDirectSoundListener->Release();
        smDirectSoundListener = 0;
    }

    if (smDirectSound)
    {
        smDirectSound->Release();
        smDirectSound = 0;
    }

    if (smSampleRAM)
    {
        free(smSampleRAM);
        smSampleRAM = 0;
    }

    CoUninitialize();
    return false;
}

// FUNCTION: SWBF2 0x00731a70
void Engine::Close(unsigned long ulFlags, bool b)
{
    if (smDirectSound && ulFlags)
    {
        smDirectSound->SetSpeakerConfig(ulFlags);
    }

    DSClose();
    sndEngineThreadUpdate.Exit();

    if (b)
    {
        File::SystemClose();
    }

    smApplicationWindow = 0;
    smReverb = 0;
    smSoftwareEffectsEnabled = false;
    smCentrePeakMode = false;
}

// FUNCTION: SWBF2 0x00731da0
bool Engine::InitializeEAX()
{

    struct EAXCONTEXTPROPERTIES
    {
        GUID guidPrimaryFXSlotID;
        float flDistanceFactor;
        float flAirAbsorptionHF;
        float flHFReference;
    };

    if (!smDummySecBuffer.Create(SampleInfo::format_PCM16, 1, smOutputFrequency,
                                 4, true, true, 4, false, false, 0, 0, false))
    {
        smDummySecBuffer.Destroy();
        return false;
    }

    if (!smDummySecBuffer.GetPropertySet())
    {
        smDummySecBuffer.Destroy();
        return false;
    }

    smEAXFXSlot = const_cast<GUID*>(&kEAX40FXSlot0PropertySet);

    unsigned long ulActive = 1;
    if (!smDummySecBuffer.SetProperty(kEAX40FXSlot0PropertySet, 0x10005,
                                      &ulActive, 4))
    {
        smDummySecBuffer.Destroy();
        return false;
    }

    EAXCONTEXTPROPERTIES context;
    context.guidPrimaryFXSlotID = *smEAXFXSlot;
    context.flDistanceFactor = EngineBase::smListeners[0].mDistanceFactor;
    context.flAirAbsorptionHF = -5.0f;
    context.flHFReference = 5000.0f;

    if (!smDummySecBuffer.SetProperty(kEAX40ContextPropertySet, 1, &context,
                                      sizeof(context)))
    {
        smDummySecBuffer.Destroy();
        return false;
    }

    return true;
}

// FUNCTION: SWBF2 0x00731e90
bool Engine::Open(HWND hwnd, MixConfig eMixConfig, bool bEnable,
                  float fMinStreamBufferSizeMs, bool bKmixer,
                  unsigned int uiOutputFrequency, unsigned int uiNumVoices,
                  unsigned long* pulWindowsSpeakerConfig,
                  const GUID* pcGuidDevice)
{

    MixConfig eConfig = eMixConfig;

    smDeltaTime = 0.0f;
    smMinStreamBufferSize = fMinStreamBufferSizeMs;
    smOutputFrequency = uiOutputFrequency;
    smUpdateListener = false;

    if (eConfig != mixConfig_Disabled)
    {
        CoInitialize(0);

        bool bCreated = false;

        if (eConfig != mixConfig_Software)
        {
            if (SUCCEEDED(CoCreateInstance(kAlternateDirectSoundClassId, 0,
                                           CLSCTX_INPROC_SERVER,
                                           IID_IDirectSound8,
                                           (void**)&smDirectSound)))
            {

                smDirectSound->Initialize(pcGuidDevice);
                bCreated = true;
            }
        }

        if (!bCreated)
        {
            const HRESULT hr =
                DirectSoundCreate8(pcGuidDevice, &smDirectSound, 0);

            if (FAILED(hr))
            {
                PrintDSError(hr, "source\\win\\sndengine.cpp", 0xA0);

                eConfig = mixConfig_Disabled;
            }
            else
            {
                bCreated = true;
            }
        }

        if (bCreated)
        {
            if (pcGuidDevice != 0)
            {

                smOutputDevice = *pcGuidDevice;
            }
            else
            {

                GetDeviceID(&DSDEVID_DefaultPlayback, &smOutputDevice);
            }
        }
    }

    smApplicationWindow = hwnd;

    HWND hCooperativeWindow = hwnd;
    if (hwnd == 0)
    {
        smUpdateFocusWindow = true;

        smUpdateFocusWindowTimer = 0.0f;
        hCooperativeWindow = GetForegroundWindow();
    }
    else
    {
        smUpdateFocusWindow = false;
    }

    if (hCooperativeWindow != 0 && eConfig != mixConfig_Disabled)
    {
        smPrimaryWriteAccess = false;

        HRESULT hr = smDirectSound->SetCooperativeLevel(hCooperativeWindow,
                                                        DSSCL_EXCLUSIVE);
        if (FAILED(hr))
        {
            PrintDSError(hr, "source\\win\\sndengine.cpp", 0xC5);

            hr = smDirectSound->SetCooperativeLevel(hCooperativeWindow,
                                                    DSSCL_NORMAL);
            if (FAILED(hr))
            {
                PrintDSError(hr, "source\\win\\sndengine.cpp", 0xC8);
                PblTraceP("Unable to set DirectSound cooperative level, "
                           "hardware effects may not function correctly\n");
            }

            smCooperativeLevel = DSSCL_NORMAL;
        }
        else
        {
            smCooperativeLevel = DSSCL_EXCLUSIVE;
        }
    }

    smMixConfig = eConfig;
    smEAXFXSlot = 0;

    if (eConfig != mixConfig_Disabled)
    {

        memset(&smDirectSoundCaps, 0, sizeof(smDirectSoundCaps));
        smDirectSoundCaps.dwSize = sizeof(DSCAPS);

        HRESULT hr = smDirectSound->GetCaps(&smDirectSoundCaps);
        if (FAILED(hr))
        {
            PrintDSError(hr, "source\\win\\sndengine.cpp", 0xE7);

            return DSClose();
        }

        const unsigned long ulCaps = smDirectSoundCaps.dwFlags;

        if (ulCaps & DSCAPS_SECONDARY16BIT)
        {
            smSecBufFormat = SampleInfo::format_PCM16;
        }
        else if (ulCaps & DSCAPS_SECONDARY8BIT)
        {

            smSecBufFormat = SampleInfo::format_PCM8;
        }

        if (!((ulCaps & DSCAPS_SECONDARYSTEREO) &&
              (ulCaps & DSCAPS_SECONDARYMONO)))
        {
            if (smMixConfig == mixConfig_DirectSoundHardware)
            {
                smMixConfig = mixConfig_DirectSoundSoftware;
            }
        }

        smSecBufSampleRateMin =
            (float)smDirectSoundCaps.dwMinSecondarySampleRate;
        smSecBufSampleRateMax =
            (float)smDirectSoundCaps.dwMaxSecondarySampleRate;

        if (smMixConfig == mixConfig_DirectSoundHardware)
        {
            DSBuffer a3DBuffers[40];
            DSBuffer a2DBuffers[33];

            unsigned int uiCount2D = 0;
            unsigned int uiCount3D = 0;

            uiCount3D = DSBuffer::CreateBuffers(true, false, 4, a3DBuffers,
                                                uiNumVoices + 8, 0);

            smDirectSoundCaps.dwFreeHw3DStaticBuffers = uiCount2D;
            smDirectSoundCaps.dwFreeHw3DStreamingBuffers = uiCount3D;
            if (uiCount2D != 0)
            {
                smDirectSoundCaps.dwFreeHw3DStaticBuffers = uiCount2D - 1;
            }

            DSBuffer::DestroyBuffers(a3DBuffers, uiCount3D);

            DSBuffer::DestroyBuffers(a2DBuffers, uiCount2D);

            if (smDirectSoundCaps.dwFreeHw3DStreamingBuffers < 0x0E)
            {
                smMixConfig = mixConfig_DirectSoundSoftware;
            }
        }

        unsigned int uiNumChannels = 0;
        if (ulCaps & DSCAPS_PRIMARYSTEREO)
        {
            uiNumChannels = 2;
        }
        else if (ulCaps & DSCAPS_PRIMARYMONO)
        {
            uiNumChannels = 1;
        }

        if (smDirectSoundCaps.dwPrimaryBuffers > 1)
        {

            if (smMixConfig == mixConfig_DirectSoundHardware)
            {
                smMixConfig = mixConfig_DirectSoundSoftware;
            }

            smHardwareEffectsAvailable = false;
        }
        else
        {

            if (smMixConfig == mixConfig_DirectSoundHardware && bEnable)
            {
                smHardwareEffectsAvailable = InitializeEAX();
            }
            else
            {
                smHardwareEffectsAvailable = false;
            }
        }

        if (smMixConfig == mixConfig_DirectSoundSoftware && bKmixer)
        {
            smMixConfig = mixConfig_Software;
        }

        {
            DSBuffer primary;

            bool bOk = primary.Create(SampleInfo::format_PCM16, uiNumChannels,
                                      smOutputFrequency, 4, true, true, 0,
                                      true, true, 0, 0, false);
            if (!bOk)
            {
                primary.Destroy();

                bOk = primary.Create(SampleInfo::format_PCM16, uiNumChannels,
                                     smOutputFrequency, 4, true, true, 0,
                                     true, false, 0, 0, false);
            }

            if (!bOk)
            {

                return DSClose();
            }

            primary.Play(true, 0, true, false);

            HRESULT hrListener = primary.GetBuffer()->QueryInterface(
                IID_IDirectSound3DListener, (void**)&smDirectSoundListener);

            if (FAILED(hrListener))
            {
                PrintDSError(hrListener, "source\\win\\sndengine.cpp", 0x165);
                primary.Destroy();
                return DSClose();
            }

            primary.Destroy();

            smDirectSoundListener->SetDistanceFactor(1.0f, DS3D_IMMEDIATE);
            smDirectSoundListener->SetRolloffFactor(1.0f, DS3D_IMMEDIATE);
            smDirectSoundListener->SetDopplerFactor(0.0f, DS3D_IMMEDIATE);
        }

        smSampleRAM = (unsigned char*)malloc(0x2000000);
        if (smSampleRAM == 0)
        {
            return DSClose();
        }

        if (smMixConfig == mixConfig_Software ||
            smMixConfig == mixConfig_DirectSoundHardware)
        {

            smSoftOutput = new (gSoftOutputStorage, false) SoftOutput();

            const bool bEffects = (bEnable &&
                                   smMixConfig == mixConfig_Software);

            smSoftOutput->SetOutputBufferSize(fMinStreamBufferSizeMs, false,
                                              bEffects, smOutputFrequency);

            smSoftOutput->SetBusGain(1, 0.5f, 0.5f, false);

            smSoftOutput->mFlags = (unsigned short)(
                (smSoftOutput->mFlags & ~SourceTransport::flags_updateTransport) |
                SourceTransport::flags_updatePlay);
            if ((smSoftOutput->mFlags &
                 SourceTransport::flags_updatePlaybackPosition) == 0)
            {
                smSoftOutput->mPlaybackPosition = 0;
                smSoftOutput->mFlags = (unsigned short)(
                    smSoftOutput->mFlags |
                    SourceTransport::flags_updatePlaybackPosition);
            }

            if (bEnable && smMixConfig == mixConfig_Software)
            {
                smSoftwareEffectsEnabled = true;
            }
            else
            {
                smSoftwareEffectsEnabled = false;

                if (smMixConfig == mixConfig_DirectSoundHardware)
                {
                    DSBufferRenderer& rRenderer =
                        smSoftOutput->GetRenderer();

                    StreamLowHighPass& rFilter = _smHighLowPass;

                    rFilter.mSink.mConnection.mInputNode =
                        rRenderer.StreamNodeSink::mConnection.mInputNode;
                    rFilter.mSink.mConnection.mInputNodeOutputIndex = 0;
                    rRenderer.StreamNodeSink::mConnection.mInputNode =
                        (StreamNodeSource*)&_smHighLowPass;
                    rRenderer.StreamNodeSink::mConnection
                        .mInputNodeOutputIndex = 0;

                    rFilter.SetBuffer(_smHighLowPassBuffer, 0x400);
                    rFilter.mFilter.SetNumChannels(2);
                    rFilter.mFilter.SetMode(
                        DSP::HighLowPassFilter::mode_LowPass);
                    rFilter.mFilter.SetSampleRate(
                        (float)smSoftOutput->GetOutputFrequency());
                    rFilter.mFilter.SetCutOffFrequency(60.0f);
                }
            }
        }

        sndEngineThreadUpdate.SetSleepTimeMs(
            (unsigned int)(fMinStreamBufferSizeMs * 0.25f));
    }

    EngineBase::smSoundRAMAllocator.Initialize((unsigned int)smSampleRAM,
                                               0x2000000, 0x80,
                                               smSampleRAMBitmap);

    sndEngineThreadUpdate.Pause(false);
    EngineBase::Open();
    sndEngineThreadUpdate.Resume();

    unsigned int uiInitialised;
    if (smMixConfig == mixConfig_DirectSoundHardware)
    {

        uiInitialised = smDirectSoundCaps.dwFreeHw3DStreamingBuffers - 8;
    }
    else
    {
        uiInitialised = uiNumVoices;
    }

    for (unsigned int i = 0; i < uiInitialised; ++i)
    {

        ((Voice*)((unsigned char*)EngineBase::smVoices +
                  i * sizeof(Voice)))->Initialize();
    }

    for (unsigned int i = uiInitialised; i < 32; ++i)
    {
        Voice* const pVoice =
            (Voice*)((unsigned char*)EngineBase::smVoices + i * sizeof(Voice));

        if ((pVoice->VoiceBase::mFlags & 1) == 0)
        {
            pVoice->Lock(true);
        }
        pVoice->RemoveFromManager();
        pVoice->Initialize();
    }

    if (!File::SystemOpen(0x7FFFFFFF))
    {
        DSClose();
        sndEngineThreadUpdate.Exit();
        File::SystemClose();

        smApplicationWindow = 0;
        smReverb = 0;
        smSoftwareEffectsEnabled = false;
        smCentrePeakMode = false;

        return false;
    }

    I3DL2Reverb reverb;
    reverb.mFlags = (unsigned short)(reverb.mFlags | 1 | 2);
    reverb.mRoomGain = 0.0f;
    reverb.mRoomHFGain = 0.0f;
    SetGlobalReverb(&reverb);

    if (smDirectSound != 0)
    {
        unsigned long ulSpeakerConfig = 0;

        smDirectSound->GetSpeakerConfig(&ulSpeakerConfig);

        if (pulWindowsSpeakerConfig != 0)
        {
            *pulWindowsSpeakerConfig = ulSpeakerConfig;
        }

        EngineBase::SpeakerConfig eSpeakers = EngineBase::speakerConfig_Stereo;
        switch (ulSpeakerConfig)
        {
        case DSSPEAKER_HEADPHONE:
            eSpeakers = EngineBase::speakerConfig_Headphones;
            break;
        case DSSPEAKER_MONO:
            eSpeakers = EngineBase::speakerConfig_Mono;
            break;
        case DSSPEAKER_QUAD:
            eSpeakers = EngineBase::speakerConfig_Quadraphonic;
            break;
        case DSSPEAKER_SURROUND:
            eSpeakers = EngineBase::speakerConfig_AnalogSurround;
            break;
        case DSSPEAKER_5POINT1:
            eSpeakers = EngineBase::speakerConfig_5point1;
            break;
        case DSSPEAKER_7POINT1:
            eSpeakers = EngineBase::speakerConfig_7point1;
            break;
        default:
            break;
        }

        EngineBase::smSpeakerConfig = EngineBase::speakerConfig_Invalid;
        SetSpeakerConfig(eSpeakers);
    }

    if (smMixConfig != mixConfig_Software && smMixConfig != mixConfig_Disabled)
    {
        sndEngineThreadUpdate.AddObject(
            &smEngineUpdater);
    }

    return true;
}

// FUNCTION: SWBF2 0x00731890
void Engine::Calculate3DParams(Source3D* const pSource,
                               DS3DBUFFER* const pParams)
{
    const Listener* const pListener = pSource->mClosestListener;

    PblMatrix matListener;
    matListener.m[0][0] = pListener->mRight.x;
    matListener.m[0][1] = pListener->mRight.y;
    matListener.m[0][2] = pListener->mRight.z;
    matListener.m[0][3] = 0.0f;
    matListener.m[1][0] = pListener->mUp->x;
    matListener.m[1][1] = pListener->mUp->y;
    matListener.m[1][2] = pListener->mUp->z;
    matListener.m[1][3] = 0.0f;
    matListener.m[2][0] = -pListener->mFront->x;
    matListener.m[2][1] = -pListener->mFront->y;
    matListener.m[2][2] = -pListener->mFront->z;
    matListener.m[2][3] = 0.0f;
    matListener.m[3][0] = Globals::smNullVector.x;
    matListener.m[3][1] = Globals::smNullVector.y;
    matListener.m[3][2] = Globals::smNullVector.z;
    matListener.m[3][3] = 1.0f;

    matListener = matListener.GetInverseFast();

    PblVector3 vDelta;
    vDelta.x = pSource->mPos3D->x - pListener->mPos->x;
    vDelta.y = pSource->mPos3D->y - pListener->mPos->y;
    vDelta.z = pSource->mPos3D->z - pListener->mPos->z;

    D3DXVECTOR3 vLocal;
    D3DXVec3TransformNormal(&vLocal,
                            (const D3DXVECTOR3*)&vDelta,
                            (const D3DXMATRIX*)&matListener);

    pParams->dwSize = sizeof(DS3DBUFFER);
    pParams->vPosition.x = vLocal.x;
    pParams->vPosition.y = vLocal.y;
    pParams->vPosition.z = vLocal.z;
    pParams->vVelocity.x = 0.0f;
    pParams->vVelocity.y = 0.0f;
    pParams->vVelocity.z = 0.0f;
    pParams->dwInsideConeAngle = 360;
    pParams->dwOutsideConeAngle = 360;
    pParams->vConeOrientation.x = 0.0f;
    pParams->vConeOrientation.y = 0.0f;
    pParams->vConeOrientation.z = 1.0f;
    pParams->lConeOutsideVolume = 0;
    pParams->flMinDistance = pSource->mProps->mMaxDistance;
    pParams->flMaxDistance = pSource->mProps->mMaxDistance;
    pParams->dwMode = DS3DMODE_HEADRELATIVE;
}

// FUNCTION: SWBF2 0x00732ff0
void Engine::SetupDirectSoundEffects(SourceI3DL2* const pSource,
                                     DSBuffer* const pBuffer)
{
    if (smEAXFXSlot == 0)
    {
        return;
    }
    if (pBuffer->GetPropertySet() == 0)
    {
        return;
    }

    unsigned char abSend[24];
    unsigned char abBuffer[72];

    SetupEAXSource(abSend, abBuffer, pSource);

    pBuffer->SetProperty(kEAX30BufferPropertySet, 0x17, abSend, sizeof(abSend));

    pBuffer->SetProperty(kEAX30BufferPropertySet, 1, abBuffer, sizeof(abBuffer));

}

// FUNCTION: SWBF2 0x00733060
void Engine::SetupEAXSource(void* const pA, void* const pB,
                            SourceI3DL2* const pSource)
{
    long* const plSend = (long*)pA;
    long* const plBuffer = (long*)pB;
    float* const pfBuffer = (float*)pB;

    *(GUID*)pA = *smEAXFXSlot;
    plSend[4] = 0;
    plSend[5] = 0;

    plBuffer[0] = ConvertLinearToLog(pSource->mDirectGain);
    plBuffer[1] = ConvertLinearToLog(pSource->mDirectHFGain);
    plBuffer[2] = ConvertLinearToLog(pSource->mRoomGain);
    plBuffer[3] = ConvertLinearToLog(pSource->mRoomHFGain);
    plBuffer[4] = ConvertLinearToLog(pSource->mObstructionHFGain);

    float fRatio = pSource->mObstructionLFRatio;
    if (0.0f > fRatio)
    {
        fRatio = 0.0f;
    }
    else if (fRatio > 1.0f)
    {
        fRatio = 1.0f;
    }
    pfBuffer[5] = fRatio;

    plBuffer[6] = ConvertLinearToLog(pSource->mOcclusionHFGain);

    float fOcclusion = pSource->mOcclusionLFRatio;
    if (0.0f > fOcclusion)
    {
        fOcclusion = 0.0f;
    }
    else if (fOcclusion > 1.0f)
    {
        fOcclusion = 1.0f;
    }
    pfBuffer[7] = fOcclusion;

    pfBuffer[8] = 1.5f;
    pfBuffer[9] = 1.0f;
    plBuffer[10] = 0;
    pfBuffer[11] = 1.0f;
    plBuffer[12] = 0;
    plBuffer[13] = 0;
    plBuffer[14] = 0;

    float fRollOff = pSource->mRoomRollOffFactor;
    if (0.0f > fRollOff)
    {
        fRollOff = 0.0f;
    }
    else if (fRollOff > 10.0f)
    {
        fRollOff = 10.0f;
    }
    pfBuffer[15] = fRollOff;

    plBuffer[16] = 0;
    plBuffer[17] = 0;
}

}

// LIBRARY: SWBF2 0x00752ae6
// free

// LIBRARY: SWBF2 0x00752aec
// malloc

// LIBRARY: SWBF2 0x007535e0
// _alloca_probe

// LIBRARY: SWBF2 0x007548fc
// DirectSoundCreate8

// LIBRARY: SWBF2 0x00754902
// GetDeviceID
