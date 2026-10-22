#pragma once

#include "SoundFL/source/sndenginebase.h"

extern bool gSoundEnabled;

namespace GameSoundEngine
{

extern int gSoundHeap;

extern Snd::EngineBase::SpeakerConfig gDefaultSpeakerConfig;

extern unsigned long gWindowsSpeakerConfig;

extern bool gSoundIsOpen;

void SetHeap(int iHeap);

bool Init(unsigned int uiFlags, bool bEnable);

void Create();

void Destroy();

void Term(bool);

}

void* __cdecl SndAlloc(unsigned int uiSize);
void __cdecl SndFree(void* pBlock);
void* __cdecl SndTempAlloc(unsigned int uiSize);
void __cdecl SndTempFree(void* pBlock);

void __cdecl PrintDebugString(char const* pcMessage);

namespace GameSoundEngine { bool __cdecl UseKmixer(); }

namespace GameSoundEngine
{
void StopAll();
void Update(float fDeltaTime);
}

struct BusGains
{
    BusGains()
        : mMusic(1.0f), mSFX(1.0f), mVoiceOver(1.0f), mBattleVO(1.0f),
          mAmbience(1.0f), mShellFX(1.0f), mMaster(1.0f)
    {
    }

    float mMusic;
    float mSFX;
    float mVoiceOver;
    float mBattleVO;
    float mAmbience;
    float mShellFX;
    float mMaster;
};
 

namespace GameSoundEngine
{
extern BusGains gBusGainsDefault;
extern BusGains gBusGainsUser;
extern bool gMixConfigChanged;
extern bool gDisableStartupMusic;

int GetHeap();
bool MixConfigChanged();
void DisableStartupMusic();
bool ChangeMixConfig(unsigned int uiFlags, bool bEnable);
void InitializeBusGains();
void UpdateBusGains(float fSFX, float fMusic, float fVoiceOver, float fBattleVO, float fMaster);
}
