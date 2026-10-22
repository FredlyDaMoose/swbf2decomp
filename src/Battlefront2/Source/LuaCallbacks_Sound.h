#pragma once

#include "LuaCallbacks.h"
#include "GameSound.h"

namespace Lua_Callbacks
{

int ScriptCB_SndBusFade(lua_State* L);
int ScriptCB_SndPlaySound(lua_State* L);
int ScriptCB_OpenAudioStream(lua_State* L);
int ScriptCB_SetBleedingRepeatTime(lua_State* L);
int ScriptCB_SetSpawnDisplayGain(lua_State* L);
int ScriptCB_PlayInGameMusic(lua_State* L);
int ScriptCB_StopInGameMusic(lua_State* L);
int ScriptCB_EnableCommandPostVO(lua_State* L);
int ScriptCB_EnableHeroMusic(lua_State* L);
int ScriptCB_EnableHeroVO(lua_State* L);
int ScriptCB_TriggerSoundRegionEnable(lua_State* L);
int ScriptCB_GetVolumes(lua_State* L);
int ScriptCB_SetVolumes(lua_State* L);
int ScriptCB_EffectsEnabled(lua_State* L);
int ScriptCB_ToggleEffects(lua_State* L);
int ScriptCB_HWSupport(lua_State* L);
int ScriptCB_GetOutputMode(lua_State* L);
int ScriptCB_PreviousOutputMode(lua_State* L);
int ScriptCB_NextOutputMode(lua_State* L);
int ScriptCB_SetOutputMode(lua_State* L);
int ScriptCB_GetMixConfig(lua_State* L);
int ScriptCB_PreviousMixConfig(lua_State* L);
int ScriptCB_NextMixConfig(lua_State* L);
int ScriptCB_GetEAXVersion(lua_State* L);
int ScriptCB_GetMixConfigChanged(lua_State* L);
int ScriptCB_SetBassManagement(lua_State* L);
int ScriptCB_GetBassManagement(lua_State* L);
int ScriptCB_PlayAuditionEffect(lua_State* L);
int ScriptCB_SetShellMusic(lua_State* L);
int ScriptCB_SetShellMusicInterval(lua_State* L);
int ScriptCB_IsShellMusicPlaying(lua_State* L);
int ScriptCB_ShellPlayDelayedStream(lua_State* L);
int ScriptCB_SoundEnable(lua_State* L);
int ScriptCB_SoundDisable(lua_State* L);
int ScriptCB_SetDopplerFactor(lua_State* L);

int ScriptCB_SetSoundEffect(lua_State* L);

int ScriptCB_PlayAudioStreamUsingProperties(lua_State* L);
int ScriptCB_PlayAudioStream(lua_State* L);
int ScriptCB_StopAudioStream(lua_State* L);
int ScriptCB_AudioStreamAppendSegments(lua_State* L);
int ScriptCB_CloseAudioStream(lua_State* L);
int ScriptCB_AudioStreamComplete(lua_State* L);
int ScriptCB_SetPlanetaryBonusVoiceOver(lua_State* L);
int ScriptCB_SetReservesVO(lua_State* L);
int ScriptCB_SetBleedingVoiceOver(lua_State* L);
int ScriptCB_SetLowReinforcementsVoiceOver(lua_State* L);
int ScriptCB_SetAmbientMusic(lua_State* L);
int ScriptCB_SetAttackingTeam(lua_State* L);
int ScriptCB_SetVictoryMusic(lua_State* L);
int ScriptCB_SetDefeatMusic(lua_State* L);
int ScriptCB_ScaleSoundParameter(lua_State* L);
int ScriptCB_SetOutOfBoundsVoiceOver(lua_State* L);
int ScriptCB_SetMixConfig(lua_State* L);

void InitSound();

void PlayShellMusic(unsigned int uID);
void UpdateShellMusic(float fDeltaTime);
void UpdateDelayedSounds(float fDeltaTime);
void InitDelayedSounds();

class DelayedSound
{
public:
    DelayedSound();
    ~DelayedSound();
    void Init(unsigned int uID, float fDelay, float fGain, bool bQueue);
    void Update(float fDeltaTime);

    float mDelay;
    float mElapsed;
    GameSound mSound;
    GameSoundControllable mControllable;
    float mGain;
    bool mQueue;
};
 

extern DelayedSound sDelayedSounds[2];

}
