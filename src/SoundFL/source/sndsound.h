#pragma once

#include "sndobjectweightlist.h"
#include "sndsoundproperties.h"
#include "sndsource2d3d.h"
#include "sndsource3d.h"
#include "sndsourcevariation.h"
#include "sndsourcevirtual.h"
#include "sndvoicevirtualbus.h"

namespace Snd
{

class Sound : public VoiceVirtualBus
{
public:
    class Properties;

    class FireForget : public VoiceVirtualBus
    {
    public:

        FireForget(unsigned int uiCustomFlags, VoiceVirtualManager* pManager,
                   bool bRestorePlaybackPos)
            : VoiceVirtualBus(uiCustomFlags, pManager, bRestorePlaybackPos),
              mCallback(0),
              mCallbackData(0)
        {
        }

        ~FireForget()
        {
            if (mCallback)
            {
                mCallback(this, mCallbackData);
                mCallback = 0;
                mCallbackData = 0;
            }
        }

        void SetStolenCallback(void (__cdecl* pfnCallback)(VoiceVirtual*, void*), void* pData)
        {
            mCallback = pfnCallback;
            mCallbackData = pData;
        }

        void (__cdecl* mCallback)(VoiceVirtual*, void*);
        void* mCallbackData;

    private:

        static void __cdecl InactiveCallback(const VoiceVirtual* const pVoice,
                                             void* const pData);
        friend class Sound;
    };

    static void Open();

    static void Close();

    static VoiceVirtual* __cdecl Play(Source3D* pSource3D, const Properties* pProps,
                                      void (__cdecl* pStolenCallback)(VoiceVirtual*, void*),
                                      void* pStolenCallbackData, bool bArg);
    static void __cdecl VoiceVirtualRelease(VoiceVirtual* pVoice);
    static VoiceVirtual* __cdecl VoiceVirtualHandleToVoiceVirtual(unsigned short uHandle);
    static unsigned short __cdecl VoiceVirtualToVoiceVirtualHandle(const VoiceVirtual* const pVoice);

    static void __cdecl UpdateAll(float fDeltaTime);

    static unsigned int __cdecl FindParameterIndexByHashID(unsigned int uiHashID);

    class ParameterID
    {
    public:
        ParameterID(void (__cdecl* pFunction)(Sound*, float), unsigned int uiHashID)
            : mFunction(pFunction), mHashID(uiHashID)
        {
        }
        unsigned int GetHashID() const { return mHashID; }

        void (__cdecl* mFunction)(Sound*, float);
        unsigned int mHashID;
    };

    static ParameterID smParameterFunctions[11];

    Sound();

    void SetProperties(const Properties* pProps);

protected:

    static void __cdecl ParamSetGain(Sound* pSound, float fValue);
    static void __cdecl ParamSetPitch(Sound* pSound, float fValue);
    static void __cdecl ParamSetReverbGain(Sound* pSound, float fValue);
    static void __cdecl ParamSetPan(Sound* pSound, float fValue);
    static void __cdecl ParamSetMinDistance(Sound* pSound, float fValue);
    static void __cdecl ParamSetMuteDistance(Sound* pSound, float fValue);
    static void __cdecl ParamSetMaxDistance(Sound* pSound, float fValue);
    static void __cdecl ParamSetRollOff(Sound* pSound, float fValue);
    static void __cdecl ParamSetTrigger(Sound* pSound, float fValue);
    static void __cdecl ParamSetRollIn(Sound* pSound, float fValue);
    static void __cdecl ParamSetRollInDistance(Sound* pSound, float fValue);

protected:

    static VoiceVirtual* __cdecl GetFreeVoice(VoiceVirtualManager* pManager);
    static VoiceVirtual* __cdecl StealVoice(VoiceVirtualManager* pManager,
                                            float fPriority);

private:

    static void __cdecl UpdateVoices(VoiceVirtualManager* pManager, float fDeltaTime);

public:

    static FireForget* smVoiceVirtuals;
    static unsigned char smVoiceVirtualStorage[12800];
    static Source3D smSource3Ds[64];

    static ::PblVector3 smSource3DPositions[64];
    static ::PblVector3 smSource3DVelocities[64];

    static PblListSingle smSoundList;

    static void __cdecl RemoveAll();

    static void __cdecl StopAll();

    void Play(bool bRestart);

    // FUNCTION: SWBF2 0x00735990
    ~Sound()
    {
        SourceBus::Stop(this);
        VoiceVirtual::Stop();
        mProperties = 0;
        if (mNode.mNext)
        {
            mNode.Remove();
        }
    }

    Properties* mProperties;
    PblListSingle::Node mNode;
};

 

class Sound::Properties : public SoundProperties,
                          public Source2D3D,
                          public SourceVirtual,
                          public SourceVariation
{
public:
    Properties();

    static void* operator new(unsigned int)
    {
        return smFreeList.Alloc();
    }
    static void __cdecl operator delete(void* p);

    ~Properties();
    static void __cdecl DestroyAll();

    static void __cdecl Load(PblFileChunk& chunk, bool bOnce);

    static const Properties* __cdecl FindByHashID(unsigned int uiHashID);
    void Read(PblFileChunk& chunk);

    void SetupVoiceVirtual(VoiceVirtualBus* pVoice, Source3D* pSource3D,
                           const Sample* pSample) const;

    static FreeList smFreeList;

    static PblListSingle smPropertyList;

    ObjectWeightList mObjectWeightList;
    ObjectWeightList* mSelObjectWeightList;
    PblListSingle::Node mNode;

private:

    static void const* __cdecl SampleFindByHashID(unsigned int uiHashID);
};

inline void Sound::SetProperties(const Properties* pProps)
{
    mProperties = const_cast<Properties*>(pProps);
    if (pProps)
    {
        pProps->SetupVoiceVirtual(this, this, 0);
    }
}

 

 

}
