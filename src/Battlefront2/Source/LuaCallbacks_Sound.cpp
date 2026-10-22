#include "LuaCallbacks_Sound.h"
#include "GameMusic.h"
#include "LuaHelper.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "CommandPost.h"
#include "EntitySoldier.h"

#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "GameSound.h"
#include "GameSoundEngine.h"
#include "LoadUtil.h"
#include "Music.h"
#include "PauseMenu.h"
#include "PlayerProfile.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "SoundFL/source/sndbus.h"
#include "SoundFL/source/sndenginebase.h"
#include "SoundFL/source/sndsound.h"
#include "SoundFL/source/sndsoundstream.h"
#include "SoundFL/source/sndvoicevirtualbus.h"
#include "SoundFL/source/win/sndstream.h"
#include "SoundFL/source/win/sndengine.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "BirdEffect.h"
#include "HeroMessageDisplay.h"
#include "LeafEffect.h"
#include "LockOnManager.h"
#include "ScopeDisplay.h"
#include "TriggerSoundRegion.h"
#include "Weapon.h"
#include "VoiceOverManager.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"

#include "BoundaryManager.h"
#include "EndDisplay.h"
#include "PebbleFL/Common/PblUnicode.h"

// GLOBAL: SWBF2 0x007e69cb
namespace Lua_Callbacks { bool sSoundEnabled = true; }
// GLOBAL: SWBF2 0x01e57949
namespace Lua_Callbacks { bool sGameMusicRetrigger; }
// GLOBAL: SWBF2 0x01e57968
namespace Lua_Callbacks { unsigned int sShellMusicStoppedTime; }
// GLOBAL: SWBF2 0x01e5796c
namespace Lua_Callbacks { unsigned int sShellMusicID; }
// GLOBAL: SWBF2 0x01e57984
namespace Lua_Callbacks { unsigned int sGameMusicCurDelayTime; }
// GLOBAL: SWBF2 0x01e57988
namespace Lua_Callbacks { void* sGameMusic; }
// GLOBAL: SWBF2 0x01e5798c
namespace Lua_Callbacks { unsigned int sGameMusicDelayTime; }

namespace GameSoundEngine { extern bool sHwMixingSupported; }
namespace GameSoundEngine { extern bool sHwEffectsSupported; }

// FUNCTION: SWBF2 0x0059b470
void Snd::StreamBase::Play(bool bRestart)
{
    SourceBus::Play(this);

    SourceTransport::mFlags = (unsigned short)((SourceTransport::mFlags
        & ~(unsigned int)(flags_updateStop | flags_updatePause)) | (unsigned int)flags_updatePlay);
    if ((SourceTransport::mFlags & flags_updatePlaybackPosition) == 0 && bRestart)
    {
        mPlaybackPosition = 0;
        SourceTransport::mFlags = (unsigned short)(SourceTransport::mFlags
            | (unsigned int)flags_updatePlaybackPosition);
    }
}

// FUNCTION: SWBF2 0x0050b470
void LuaHelper::PushInt(int iStream)
{
    lua_pushnumber(LuaHelper::mState, (float)iStream);
}

namespace Lua_Callbacks
{

static int OpenAudioStream(const char* pcName, unsigned int uiSegment);
static unsigned int GetStreamIndex(Snd::Stream* pStream);

// FUNCTION: SWBF2 0x0059d0c0
int ScriptCB_ToggleEffects(lua_State*)
{
    bool bEffects = Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable;
    GameSoundEngine::ChangeMixConfig(Snd::Engine::GetMixConfig(), !bEffects);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    if (pProfile)
    {
        PlayerProfile::tSoundOpt& opt = pProfile->mSoundOptions;
        opt.mEffectsEnabled = Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable;
        opt.mMixerMode = (unsigned char)Snd::Engine::GetMixConfig();
        opt.mPlaybackMode = (unsigned char)Snd::EngineBase::smSpeakerConfig;
        pProfile->UpdateProfCRC();
        opt.ApplySettings();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059d1c0
int ScriptCB_PreviousOutputMode(lua_State*)
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    int iMode = Snd::EngineBase::smSpeakerConfig - 1;
    if (iMode <= 4 && Snd::Engine::GetMixConfig() == Snd::Engine::mixConfig_DirectSoundSoftware)
    {
        GameSoundEngine::ChangeMixConfig(1, Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable);
    }
    if (iMode == 0)
    {
        iMode = 7;
        if (Snd::Engine::GetMixConfig() == Snd::Engine::mixConfig_Software)
        {
            if (pProfile)
            {
                pProfile->mSoundOptions.mPlaybackMode = 7;
            }
            if (!GameSoundEngine::ChangeMixConfig(2, false))
            {
                iMode = 4;
            }
        }
    }
    Snd::Engine::SetSpeakerConfig((Snd::EngineBase::SpeakerConfig)iMode);
    if (pProfile)
    {
        PlayerProfile::tSoundOpt& opt = pProfile->mSoundOptions;
        opt.mEffectsEnabled = Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable;
        opt.mMixerMode = (unsigned char)Snd::Engine::GetMixConfig();
        opt.mPlaybackMode = (unsigned char)Snd::EngineBase::smSpeakerConfig;
        pProfile->UpdateProfCRC();
        opt.ApplySettings();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059d2b0
int ScriptCB_NextOutputMode(lua_State*)
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    int iMode = Snd::EngineBase::smSpeakerConfig + 1;
    if (iMode > 7)
    {
        iMode = 1;
    }
    if (iMode <= 4)
    {
        if (Snd::Engine::GetMixConfig() == Snd::Engine::mixConfig_DirectSoundSoftware)
        {
            GameSoundEngine::ChangeMixConfig(1, Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable);
        }
    }
    else if (Snd::Engine::GetMixConfig() == Snd::Engine::mixConfig_Software)
    {
        if (pProfile)
        {
            pProfile->mSoundOptions.mPlaybackMode = (unsigned char)iMode;
        }
        if (!GameSoundEngine::ChangeMixConfig(2, false))
        {
            iMode = 1;
        }
    }
    Snd::Engine::SetSpeakerConfig((Snd::EngineBase::SpeakerConfig)iMode);
    if (pProfile)
    {
        PlayerProfile::tSoundOpt& opt = pProfile->mSoundOptions;
        opt.mEffectsEnabled = Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable;
        opt.mMixerMode = (unsigned char)Snd::Engine::GetMixConfig();
        opt.mPlaybackMode = (unsigned char)Snd::EngineBase::smSpeakerConfig;
        pProfile->UpdateProfCRC();
        opt.ApplySettings();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059d3a0
int ScriptCB_SetOutputMode(lua_State*)
{
    int iMode = (int)luaL_checknumber(LuaHelper::mState, 1);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    if (iMode > 7)
    {
        iMode = 1;
    }
    if (iMode <= 4)
    {
        if (Snd::Engine::GetMixConfig() == Snd::Engine::mixConfig_DirectSoundSoftware)
        {
            GameSoundEngine::ChangeMixConfig(1, Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable);
        }
    }
    else if (Snd::Engine::GetMixConfig() == Snd::Engine::mixConfig_Software)
    {
        if (pProfile)
        {
            pProfile->mSoundOptions.mPlaybackMode = (unsigned char)iMode;
        }
        if (!GameSoundEngine::ChangeMixConfig(2, false))
        {
            iMode = 1;
        }
    }
    Snd::Engine::SetSpeakerConfig((Snd::EngineBase::SpeakerConfig)iMode);
    if (pProfile)
    {
        PlayerProfile::tSoundOpt& opt = pProfile->mSoundOptions;
        opt.mEffectsEnabled = Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable;
        opt.mMixerMode = (unsigned char)Snd::Engine::GetMixConfig();
        opt.mPlaybackMode = (unsigned char)Snd::EngineBase::smSpeakerConfig;
        pProfile->UpdateProfCRC();
        opt.ApplySettings();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059d4e0
int ScriptCB_PreviousMixConfig(lua_State*)
{
    if (Lua_Callbacks::sSoundEnabled)
    {
        bool bEffects = Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable;
        int iMix = Snd::Engine::GetMixConfig();
        switch (iMix)
        {
        case 1: case 3: iMix = 2; break;
        case 2: case 4: iMix = Snd::EngineBase::smSpeakerConfig > 4 ? 3 : 1; break;
        }
        if (!GameSoundEngine::ChangeMixConfig(iMix, bEffects))
        {
            GameSoundEngine::ChangeMixConfig(iMix, !bEffects);
        }
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
        if (pProfile)
        {
            PlayerProfile::tSoundOpt& opt = pProfile->mSoundOptions;
            opt.mEffectsEnabled = Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable;
            opt.mMixerMode = (unsigned char)Snd::Engine::GetMixConfig();
            opt.mPlaybackMode = (unsigned char)Snd::EngineBase::smSpeakerConfig;
            pProfile->UpdateProfCRC();
            opt.ApplySettings();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059d5d0
int ScriptCB_NextMixConfig(lua_State*)
{
    if (Lua_Callbacks::sSoundEnabled)
    {
        bool bEffects = Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable;
        int iMix = Snd::Engine::GetMixConfig();
        switch (iMix)
        {
        case 1: case 3: iMix = 2; break;
        case 2: case 4: iMix = Snd::EngineBase::smSpeakerConfig > 4 ? 3 : 1; break;
        }
        if (!GameSoundEngine::ChangeMixConfig(iMix, bEffects))
        {
            GameSoundEngine::ChangeMixConfig(iMix, !bEffects);
        }
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
        if (pProfile)
        {
            PlayerProfile::tSoundOpt& opt = pProfile->mSoundOptions;
            opt.mEffectsEnabled = Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable;
            opt.mMixerMode = (unsigned char)Snd::Engine::GetMixConfig();
            opt.mPlaybackMode = (unsigned char)Snd::EngineBase::smSpeakerConfig;
            pProfile->UpdateProfCRC();
            opt.ApplySettings();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059d6c0
int ScriptCB_SetMixConfig(lua_State*)
{
    if (Lua_Callbacks::sSoundEnabled)
    {
        int iMix = (int)luaL_checknumber(LuaHelper::mState, 1);
        bool bEffects = Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable;
        switch (iMix)
        {
        case 1: iMix = Snd::EngineBase::smSpeakerConfig > 4 ? 3 : 1; break;
        case 2: iMix = 2; break;
        case 3: iMix = 4; break;
        }
        if (!GameSoundEngine::ChangeMixConfig(iMix, bEffects))
        {
            GameSoundEngine::ChangeMixConfig(iMix, !bEffects);
        }
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
        if (pProfile)
        {
            PlayerProfile::tSoundOpt& opt = pProfile->mSoundOptions;
            opt.mEffectsEnabled = Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable;
            opt.mMixerMode = (unsigned char)Snd::Engine::GetMixConfig();
            opt.mPlaybackMode = (unsigned char)Snd::EngineBase::smSpeakerConfig;
            pProfile->UpdateProfCRC();
            opt.ApplySettings();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059d850
int ScriptCB_SetBassManagement(lua_State*)
{
    Snd::Engine::SetBassManagement(lua_toboolean(LuaHelper::mState, 1) != 0);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    if (pProfile)
    {
        pProfile->mSoundOptions.mBassManagement = Snd::Engine::GetBassManagement();
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059cda0
int ScriptCB_TriggerSoundRegionEnable(lua_State*)
{
    const char* pcRegion = luaL_checklstring(LuaHelper::mState, 1, 0);
    bool bEnabled = lua_toboolean(LuaHelper::mState, 2) != 0;
    unsigned char* pRegion = (unsigned char*)TriggerSoundRegion::Group::FindByID(PblHash(pcRegion));
    if (pRegion)
    {
        pRegion[0x4c] = (pRegion[0x4c] & ~1) | bEnabled;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059bf80
int ScriptCB_CloseAudioStream(lua_State*)
{
    int iStream = (int)luaL_checknumber(LuaHelper::mState, 1);
    if (iStream)
    {
        Snd::Stream* pStream = &Snd::EngineBase::smStreams[iStream - 1];
        if (pStream->IsOpen())
        {
            pStream->Close();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059bab0
int ScriptCB_StopAudioStream(lua_State*)
{
    int iStream = (int)luaL_checknumber(LuaHelper::mState, 1);
    int bClose = lua_gettop(LuaHelper::mState) == 2
        ? (int)luaL_checknumber(LuaHelper::mState, 2) : 1;
    if (iStream && (unsigned)(iStream - 1) < 6)
    {
        Snd::Stream* pStream = &Snd::EngineBase::smStreams[iStream - 1];
        pStream->Stop();
        if (bClose)
        {
            pStream->Close();
        }
        pStream->SetBus(&Snd::Bus::smRootBus);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059bfd0
int ScriptCB_AudioStreamComplete(lua_State*)
{
    Snd::Stream* pStream = 0;
    if (lua_isnumber(LuaHelper::mState, 1))
    {
        int iStream = (int)luaL_checknumber(LuaHelper::mState, 1);
        if (!iStream)
        {
            return 0;
        }
        pStream = &Snd::EngineBase::smStreams[iStream - 1];
    }
    else if (lua_isstring(LuaHelper::mState, 1))
    {
        unsigned int uID = PblHash(luaL_checklstring(LuaHelper::mState, 1, 0));
        for (int i = 0; i < 6 && !pStream; ++i)
        {
            Snd::Stream* pCandidate = &Snd::EngineBase::smStreams[i];
            for (PblListSingle::Node* pLink = pCandidate->mInfoList.mHead.mNext;
                 pLink != &pCandidate->mInfoList.mHead; pLink = pLink->mNext)
            {
                Snd::ObjectList::Node* pNode = (Snd::ObjectList::Node*)pLink;
                Snd::StreamBase::Info* pInfo = (Snd::StreamBase::Info*)pNode->mObject;
                if (!pInfo)
                {
                    break;
                }
                if (pInfo->mHashID == uID) { pStream = pCandidate; break; }
            }
        }
        if (!pStream)
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, (float)(pStream->GetPlayState() == Snd::SourceTransport::playState_stopped));
    return 1;
}

// FUNCTION: SWBF2 0x0059b8f0
int ScriptCB_PlayAudioStream(lua_State*)
{
    const char* pcSegment = luaL_checklstring(LuaHelper::mState, 3, 0);
    float fGain = (float)luaL_checknumber(LuaHelper::mState, 4);
    const char* pcBus = luaL_checklstring(LuaHelper::mState, 5, 0);
    int iStream = lua_gettop(LuaHelper::mState) == 6
        ? (int)luaL_checknumber(LuaHelper::mState, 6) : 0;
    if (!Lua_Callbacks::sSoundEnabled)
    {
        return 0;
    }

    int iReturns;
    if (iStream)
    {
        lua_pushnumber(LuaHelper::mState, (float)iStream);
        iReturns = 1;
    }
    else
    {
        iReturns = ScriptCB_OpenAudioStream(LuaHelper::mState);
        if (!iReturns)
        {
            return 0;
        }
        iStream = (int)luaL_checknumber(LuaHelper::mState, lua_gettop(LuaHelper::mState));
    }

    Snd::Stream* pStream = &Snd::EngineBase::smStreams[iStream - 1];
    pStream->SetGain(fGain);
    pStream->Source2D::mFlags |= 1;
    pStream->Source2D::mPan = 0.0f;
    unsigned char& uMode = pStream->Source2D3D::mFlags;
    uMode = (unsigned char)((uMode & 0xFC) | ((uMode & 1) << 1) | 4);
    pStream->Source::mFlags |= 0x12;
    pStream->SetBus(Snd::Bus::FindByHashID(PblHash(pcBus)));
    pStream->SeekSegment(pStream->FindSegment(PblHash(pcSegment)));
    pStream->Play(true);
    return iReturns;
}

// FUNCTION: SWBF2 0x0059b7e0
int ScriptCB_PlayAudioStreamUsingProperties(lua_State*)
{
    if (!Lua_Callbacks::sSoundEnabled)
    {
        return 0;
    }
    const char* pcSound = luaL_checklstring(LuaHelper::mState, 2, 0);
    const Snd::SoundStream::Properties* pProps =
        Snd::SoundStream::Properties::FindByHashID(PblHash(pcSound));
    if (!pProps || !pProps->mStreamID)
    {
        return 0;
    }
    int iReturns = 0;
    if (lua_gettop(LuaHelper::mState) == 3 && (int)luaL_checknumber(LuaHelper::mState, 3))
    {
        if (!Snd::StreamBase::Info::Find(pProps->mStreamID))
        {
            return 0;
        }
    }
    else
    {
        const char* pcFile = luaL_checklstring(LuaHelper::mState, 1, 0);
        Snd::Stream* pStream = (Snd::Stream*)OpenAudioStream(pcFile, pProps->mStreamID);
        if (!pStream)
        {
            return 0;
        }
        LuaHelper::PushInt(GetStreamIndex(pStream) + 1);
        iReturns = 1;
    }
    GameSound sound;
    sound.SetID(pcSound, false, false);
    sound.Play(0, 0, false, true);
    return iReturns;
}

// FUNCTION: SWBF2 0x0059c0f0
int ScriptCB_SetPlanetaryBonusVoiceOver(lua_State*) { return 0; }

// FUNCTION: SWBF2 0x0059c100
int ScriptCB_SetReservesVO(lua_State*) { return 0; }

// FUNCTION: SWBF2 0x0059c190
int ScriptCB_SetBleedingRepeatTime(lua_State*)
{
    VoiceOverManager::sInstance->mBleedingRepeatTime = luaL_checknumber(LuaHelper::mState, 1);
    return 0;
}

// FUNCTION: SWBF2 0x0059c590
int ScriptCB_SetSpawnDisplayGain(lua_State*)
{
    SpawnDisplay::sInGameBusGain = luaL_optnumber(LuaHelper::mState, 1, SpawnDisplay::sInGameBusGain);
    SpawnDisplay::sInGameBusFadeTime = luaL_optnumber(LuaHelper::mState, 2, SpawnDisplay::sInGameBusFadeTime);
    return 0;
}

// FUNCTION: SWBF2 0x0059cd10
int ScriptCB_EnableCommandPostVO(lua_State*)
{
    CommandPost::sEnableVO = (int)luaL_checknumber(LuaHelper::mState, 1) == 1;
    return 0;
}

// FUNCTION: SWBF2 0x0059cd40
int ScriptCB_EnableHeroMusic(lua_State*)
{
    EntitySoldier::sEnableMusic = (int)luaL_checknumber(LuaHelper::mState, 1) == 1;
    return 0;
}

// FUNCTION: SWBF2 0x0059cd70
int ScriptCB_EnableHeroVO(lua_State*)
{
    EntitySoldier::sEnableAnnouncementVO = (int)luaL_checknumber(LuaHelper::mState, 1) == 1;
    return 0;
}

// FUNCTION: SWBF2 0x0059d8c0
int ScriptCB_PlayAuditionEffect(lua_State*) { return 0; }

// FUNCTION: SWBF2 0x0059cc70
int ScriptCB_PlayInGameMusic(lua_State*)
{
    GameMusic* pMusic = static_cast<GameMusic*>(Lua_Callbacks::sGameMusic);
    pMusic->Stop();
    pMusic->SetID(luaL_checklstring(LuaHelper::mState, 1, 0));
    pMusic->Play();
    float fTime = lua_gettop(LuaHelper::mState) < 2
        ? 15.0f : luaL_checknumber(LuaHelper::mState, 2);
    memcpy(&Lua_Callbacks::sGameMusicDelayTime, &fTime, sizeof(fTime));
    Lua_Callbacks::sGameMusicCurDelayTime = Lua_Callbacks::sGameMusicDelayTime;
    return 0;
}

// FUNCTION: SWBF2 0x0059cd00
int ScriptCB_StopInGameMusic(lua_State*)
{
    static_cast<GameMusic*>(Lua_Callbacks::sGameMusic)->Stop();
    return 0;
}

namespace
{

void ScaleParameter(Snd::Source2D3D& source, Snd::SourceVariation& variation, unsigned int uiParameterID,
                    float fScale)
{
    switch (uiParameterID)
    {
    case 0xBD324AB1u: source.SetPitch(source.mPitch * fScale); break;
    case 0x1B5426FEu: source.SetGain(source.mGain * fScale); break;
    case 0x6C408CFEu: source.SetReverbGain(source.mReverbGain * fScale); break;
    case 0x4F55EEECu: source.SetPan(source.mPan * fScale); break;
    case 0x1CE8C2E1u: source.SetRollIn(source.mRollInFactor * fScale); break;
    case 0x80A163FEu: source.SetRollInDistance(source.mRollInDistance * fScale); break;
    case 0xB835CF50u: source.SetMinDistance(source.mMinDistance * fScale); break;
    case 0xE4B12EDFu: source.SetMuteDistance(source.mMuteDistance * fScale); break;
    case 0xE6915CF6u: source.SetMaxDistance(source.mMaxDistance * fScale); break;
    case 0xAB59D4BBu: source.SetRollOff(source.mRollOffFactor * fScale); break;
    case 0xA7930B6Eu: variation.mPlayProbability *= fScale; break;
    case 0x010B9ECAu: variation.mPlayInterval *= fScale; break;
    case 0xEDCE1E71u: variation.mPlayIntervalDev *= fScale; break;
    case 0xF60B6465u: variation.mGainDev *= fScale; break;
    case 0x35119EB8u: variation.mPitchDev *= fScale; break;
    }
}

template <class TProperties>
void ScaleGroupParameter(unsigned int uiGroupID, unsigned int uiParameterID, float fScale)
{
    PblListSingle& list = TProperties::smPropertyList;
    unsigned int uiCount = 0;
    for (PblListSingle::Node* pNode = list.mHead.mNext; pNode != &list.mHead; pNode = pNode->mNext)
    {
        ++uiCount;
    }
    if (!uiCount)
    {
        return;
    }
    for (PblListSingle::Node* pNode = list.mHead.mNext; pNode != &list.mHead; pNode = pNode->mNext)
    {
        TProperties* pProps = (TProperties*)((char*)pNode - offsetof(TProperties, mNode));
        if (pProps->mGroupID == uiGroupID)
        {
            ScaleParameter(*pProps, *pProps, uiParameterID, fScale);
        }
    }
}
}

// FUNCTION: SWBF2 0x0059c780
int ScriptCB_ScaleSoundParameter(lua_State*)
{
    const char* pcGroup = luaL_checklstring(LuaHelper::mState, 1, 0);
    const char* pcParameter = luaL_checklstring(LuaHelper::mState, 2, 0);
    const unsigned int uiGroupID = PblHash(pcGroup);
    const unsigned int uiParameterID = PblHash(pcParameter);
    const float fScale = (float)luaL_checknumber(LuaHelper::mState, 3);
    ScaleGroupParameter<Snd::Sound::Properties>(uiGroupID, uiParameterID, fScale);
    ScaleGroupParameter<Snd::SoundStream::Properties>(uiGroupID, uiParameterID, fScale);
    return 0;
}

// GLOBAL: SWBF2 0x007e8390
static const CallbackEntryT sSoundCallbacks[53] =
{
    { "ScriptCB_SndBusFade", ScriptCB_SndBusFade },
    { "ScriptCB_SndPlaySound", ScriptCB_SndPlaySound },
    { "PlayAudioStream", ScriptCB_PlayAudioStream },
    { "PlayAudioStreamUsingProperties", ScriptCB_PlayAudioStreamUsingProperties },
    { "StopAudioStream", ScriptCB_StopAudioStream },
    { "OpenAudioStream", ScriptCB_OpenAudioStream },
    { "CloseAudioStream", ScriptCB_CloseAudioStream },
    { "AudioStreamComplete", ScriptCB_AudioStreamComplete },
    { "AudioStreamAppendSegments", ScriptCB_AudioStreamAppendSegments },
    { "SetPlanetaryBonusVoiceOver", ScriptCB_SetPlanetaryBonusVoiceOver },
    { "SetReservesVO", ScriptCB_SetReservesVO },
    { "SetBleedingVoiceOver", ScriptCB_SetBleedingVoiceOver },
    { "ScriptCB_SetBleedingRepeatTime", ScriptCB_SetBleedingRepeatTime },
    { "SetLowReinforcementsVoiceOver", ScriptCB_SetLowReinforcementsVoiceOver },
    { "SetSoundEffect", ScriptCB_SetSoundEffect },
    { "ScriptCB_SetSpawnDisplayGain", ScriptCB_SetSpawnDisplayGain },
    { "SetAmbientMusic", ScriptCB_SetAmbientMusic },
    { "SetAttackingTeam", ScriptCB_SetAttackingTeam },
    { "SetVictoryMusic", ScriptCB_SetVictoryMusic },
    { "SetDefeatMusic", ScriptCB_SetDefeatMusic },
    { "SetOutOfBoundsVoiceOver", ScriptCB_SetOutOfBoundsVoiceOver },
    { "ScaleSoundParameter", ScriptCB_ScaleSoundParameter },
    { "ScriptCB_GetVolumes", ScriptCB_GetVolumes },
    { "ScriptCB_SetVolumes", ScriptCB_SetVolumes },
    { "ScriptCB_EffectsEnabled", ScriptCB_EffectsEnabled },
    { "ScriptCB_ToggleEffects", ScriptCB_ToggleEffects },
    { "ScriptCB_HWSupport", ScriptCB_HWSupport },
    { "ScriptCB_GetOutputMode", ScriptCB_GetOutputMode },
    { "ScriptCB_PreviousOutputMode", ScriptCB_PreviousOutputMode },
    { "ScriptCB_NextOutputMode", ScriptCB_NextOutputMode },
    { "ScriptCB_SetOutputMode", ScriptCB_SetOutputMode },
    { "ScriptCB_GetMixConfig", ScriptCB_GetMixConfig },
    { "ScriptCB_SetMixConfig", ScriptCB_SetMixConfig },
    { "ScriptCB_PreviousMixConfig", ScriptCB_PreviousMixConfig },
    { "ScriptCB_NextMixConfig", ScriptCB_NextMixConfig },
    { "ScriptCB_GetEAXVersion", ScriptCB_GetEAXVersion },
    { "ScriptCB_GetMixConfigChanged", ScriptCB_GetMixConfigChanged },
    { "ScriptCB_SetBassManagement", ScriptCB_SetBassManagement },
    { "ScriptCB_GetBassManagement", ScriptCB_GetBassManagement },
    { "ScriptCB_PlayAuditionEffect", ScriptCB_PlayAuditionEffect },
    { "ScriptCB_SetShellMusic", ScriptCB_SetShellMusic },
    { "ScriptCB_SetShellMusicInterval", ScriptCB_SetShellMusicInterval },
    { "ScriptCB_IsShellMusicPlaying", ScriptCB_IsShellMusicPlaying },
    { "ScriptCB_ShellPlayDelayedStream", ScriptCB_ShellPlayDelayedStream },
    { "ScriptCB_SoundEnable", ScriptCB_SoundEnable },
    { "ScriptCB_SoundDisable", ScriptCB_SoundDisable },
    { "ScriptCB_SetDopplerFactor", ScriptCB_SetDopplerFactor },
    { "ScriptCB_PlayInGameMusic", ScriptCB_PlayInGameMusic },
    { "ScriptCB_StopInGameMusic", ScriptCB_StopInGameMusic },
    { "ScriptCB_EnableCommandPostVO", ScriptCB_EnableCommandPostVO },
    { "ScriptCB_EnableHeroMusic", ScriptCB_EnableHeroMusic },
    { "ScriptCB_EnableHeroVO", ScriptCB_EnableHeroVO },
    { "ScriptCB_TriggerSoundRegionEnable", ScriptCB_TriggerSoundRegionEnable },
};

// FUNCTION: SWBF2 0x0059de40
void InitSound()
{
    Lua_Callbacks::sShellMusicID = 0;
    Lua_Callbacks::sShellMusicStoppedTime = 0;
    Lua_Callbacks::sSoundEnabled = true;
    Lua_Callbacks::sGameMusicRetrigger = true;

    Lua_Callbacks::sGameMusic = new GameMusic();
    Lua_Callbacks::sGameMusicDelayTime = 0x41700000u;
    Lua_Callbacks::sGameMusicCurDelayTime = 0;

    for (int i = 0; i < 53; ++i)
    {
        LuaHelper::Register(sSoundCallbacks[i].pcName, sSoundCallbacks[i].pfnCallback, 0);
    }
}

}

// FUNCTION: SWBF2 0x0059bb60
static void _AudioStreamOpenIdleCallback(Snd::StreamBase*, void*)
{
    FileBlockCallback();
    Sleep(0);
}

namespace Lua_Callbacks
{

// GLOBAL: SWBF2 0x007e7f94
float sShellMusicInterval = 3.0f;

// GLOBAL: SWBF2 0x01ec91d0
// SYNTHETIC: SWBF2 0x00404230
// `dynamic initializer for 'Lua_Callbacks::sDelayedSounds''
// SYNTHETIC: SWBF2 0x00768cd0
// `dynamic atexit destructor for 'Lua_Callbacks::sDelayedSounds''
DelayedSound sDelayedSounds[2];

// FUNCTION: SWBF2 0x0059b570
DelayedSound::DelayedSound()
    : mQueue(true)
{
}

// FUNCTION: SWBF2 0x0059b590
DelayedSound::~DelayedSound()
{
}

void DelayedSound::Init(unsigned int uID, float fDelay, float fGain, bool bQueue)
{
    mGain = fGain;
    mDelay = fDelay;
    mElapsed = 0.0f;
    mQueue = bQueue;
    mSound.SetID(uID, false, false);
}

void DelayedSound::Update(float fDeltaTime)
{
    if (!mSound.GetProperties())
    {
        return;
    }

    mElapsed += fDeltaTime;
    if (mElapsed < mDelay)
    {
        return;
    }

    if (mSound.GetPropertiesType() == GameSound::type_soundStream && mGain != -1.0f)
    {
        ((Snd::SoundStream::Properties*)mSound.GetProperties())->SetGain(mGain);
    }

    mControllable.Play(&mSound, 0, 0, mQueue);

    Snd::VoiceVirtualBus* pVoice = (Snd::VoiceVirtualBus*)mControllable.GetVoiceVirtual();
    if (pVoice && mGain != -1.0f)
    {
        pVoice->SetGain(mGain);
    }

    Init(0, 0.0f, -1.0f, true);
}

// FUNCTION: SWBF2 0x0059b7c0
static unsigned int GetStreamIndex(Snd::Stream* pStream)
{
    return (unsigned int)(pStream - Snd::EngineBase::smStreams);
}

// FUNCTION: SWBF2 0x0059bb70
static int OpenAudioStream(const char* pcName, unsigned int uiSegment)
{
    PblFile file;
    char acSegment[256];
    char acFile[256];
    char acFullName[256];

    acSegment[0] = '\0';
    sscanf(pcName, "%s;%s", acFile, acSegment);

    const bool bDownloadable = _strnicmp(acFile, "dc:", 3) == 0;
    LoadUtil::MakeFullName(bDownloadable ? acFile + 3 : acFile, LoadUtil::FILE_TYPE_NONE,
                           acFullName, 256, bDownloadable);

    Snd::Stream* pStream = Snd::EngineBase::GetFreeStream();
    if (!pStream)
    {
        return 0;
    }

    PblFile::SetBlockCallback(FileBlockCallback);
    if (!file.Open(acFullName, true, false, true))
    {
        PblFile::SetBlockCallback(0);
        return 0;
    }

    PblFileChunk chunk(&file);
    PblFileChunk child = chunk;
    if (strlen(acSegment))
    {
        const unsigned int uID = PblHash(acSegment);
        while (chunk.HasMoreChildren())
        {
            child = chunk.ReadNextChild();
            if (child._iId == uID)
            {
                break;
            }
        }
    }

    const bool bOpened = pStream->Open(child, uiSegment, acFullName,
                                       _AudioStreamOpenIdleCallback, 0, 0);
    PblFile::SetBlockCallback(0);
    if (!bOpened)
    {
        file.Close();
        return 0;
    }
    return (int)pStream;
}

// FUNCTION: SWBF2 0x0059bd50
int ScriptCB_OpenAudioStream(lua_State*)
{
    const char* pcName = luaL_checkstring(LuaHelper::mState, 1);
    const char* pcSegment = luaL_checkstring(LuaHelper::mState, 2);
    unsigned int uiSegment = 0;
    if (strlen(pcSegment) > 1)
    {
        uiSegment = PblHash(pcSegment);
    }

    Snd::Stream* pStream = (Snd::Stream*)OpenAudioStream(pcName, uiSegment);
    if (!pStream)
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, (float)(GetStreamIndex(pStream) + 1));
    return 1;
}

// FUNCTION: SWBF2 0x0059b5a0
int ScriptCB_SndBusFade(lua_State* L)
{
    const int nArgs = lua_gettop(LuaHelper::mState);
    Snd::Bus* pBus = Snd::Bus::FindByHashID(PblHash(luaL_checklstring(LuaHelper::mState, 1, 0)));
    if (pBus)
    {
        pBus->Fade(luaL_checknumber(LuaHelper::mState, 2),
                   luaL_checknumber(LuaHelper::mState, 3),
                   nArgs == 3 ? -1.0f : luaL_checknumber(LuaHelper::mState, 4));
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059b660
int ScriptCB_SndPlaySound(lua_State* L)
{
    GameSound sound;
    GameSoundControllable controllable;

    const int nArgs = lua_gettop(LuaHelper::mState);
    if (nArgs && Lua_Callbacks::sSoundEnabled)
    {
        if (nArgs > 1)
        {
            luaL_checknumber(LuaHelper::mState, 2);
        }
        sound.SetID(luaL_checklstring(LuaHelper::mState, 1, 0), false, false);
        if (nArgs > 2)
        {
            controllable.Play(&sound, 0, 0, true);
        }
        else
        {
            sound.Play(0, 0, false, true);
        }

        Snd::VoiceVirtual* pVoice = controllable.GetVoiceVirtual();
        Snd::Bus* pBus = nArgs > 2
            ? Snd::Bus::FindByHashID(PblHash(luaL_checklstring(LuaHelper::mState, 3, 0)))
            : 0;
        if (pVoice && pBus)
        {
            Snd::Bus::UpdateAll(0.0f);
            ((Snd::VoiceVirtualBus*)pVoice)->SetGain(pBus->mGain);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059c2a0
int ScriptCB_SetSoundEffect(lua_State* L)
{
    const unsigned int uEffect = PblHash(luaL_checklstring(LuaHelper::mState, 1, 0));
    const char* pSound = luaL_checklstring(LuaHelper::mState, 2, 0);

    switch (uEffect)
    {
    case 0x80BD2D68u:
        if (SpawnDisplay* pDisplay = SpawnDisplay::Instance(0))
        {
            pDisplay->mUnitChange.SetID(pSound, false, false);
        }
        break;
    case 0x29F3F228u:
        if (SpawnDisplay* pDisplay = SpawnDisplay::Instance(0))
        {
            pDisplay->mUnitAccept.SetID(pSound, false, false);
        }
        break;
    case 0x7A1089D1u:
        if (SpawnDisplay* pDisplay = SpawnDisplay::Instance(0))
        {
            pDisplay->mSpawnPointChange.SetID(pSound, false, false);
        }
        break;
    case 0xAF57B9D5u:
        if (SpawnDisplay* pDisplay = SpawnDisplay::Instance(0))
        {
            pDisplay->mSpawnPointAccept.SetID(pSound, false, false);
        }
        break;
    case 0x7B76A4D5u:
        if (SpawnDisplay* pDisplay = SpawnDisplay::Instance(0))
        {
            pDisplay->mBack.SetID(pSound, false, false);
        }
        break;
    case 0x651E07ADu:
        ScopeDisplay::Instance(0)->mBinocularSound.SetAmbientID(pSound);
        break;
    case 0x4D9F349Bu:
        ScopeDisplay::Instance(0)->mBinocularSound.SetZoomInID(pSound);
        break;
    case 0x414B5348u:
        ScopeDisplay::Instance(0)->mBinocularSound.SetZoomOutID(pSound);
        break;
    case 0xD1454E62u:
        WeaponClass::sWeaponChangeUnableSound.SetID(pSound, false, false);
        break;
    case 0xF8258D3Bu:
        LockOnManager::sTargetLockSound.SetID(pSound, false, false);
        break;
    case 0x6DB518D0u:
    case 0x24422C9Fu:
        BirdParticleManager::Instance()->mScatterSound.SetID(pSound, false, false);
        break;
    case 0x977128B2u:
    case 0x3CD0D399u:
        BirdParticleManager::Instance()->mBirdSound.SetID(pSound, false, false);
        break;
    case 0x956177FFu:
        LeafParticleManager::Instance()->mLeafFallSound.SetID(pSound, false, false);
        break;
    case 0x4F3DFE5Cu:
        HeroMessageDisplay::mHeroesUnlocked->SetID(pSound, false, false);
        break;
    case 0x7D31DC25u:
    {
        const unsigned int uTeam = LuaHelper::GetInt(3);
        if (uTeam < 8)
        {
            HeroMessageDisplay::mHeroSpawned[uTeam]->SetID(pSound, false, false);
        }
        break;
    }
    case 0x56776EBBu:
    {
        const unsigned int uTeam = LuaHelper::GetInt(3);
        if (uTeam < 8)
        {
            HeroMessageDisplay::mHeroDefeated[uTeam]->SetID(pSound, false, false);
        }
        break;
    }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059ce40
int ScriptCB_GetVolumes(lua_State* L)
{
    float fMaster = 0.0f;
    float fMusic = 0.0f;
    float fSFX = 0.0f;
    float fSpeech = 0.0f;
    float fBattleSpeech = 0.0f;
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    if (pProfile)
    {
        const PlayerProfile::tSoundOpt& opt = pProfile->mSoundOptions;
        fMaster = (float)opt.mu8MasterVol;
        fMusic = (float)opt.mu8MusicVol;
        fSFX = (float)opt.mu8SFXVol;
        fSpeech = (float)opt.mu8SpeechVol;
        fBattleSpeech = (float)opt.mu8BattleSpeechVol;
    }
    lua_pushnumber(LuaHelper::mState, fMusic);
    lua_pushnumber(LuaHelper::mState, fSFX);
    lua_pushnumber(LuaHelper::mState, fSpeech);
    lua_pushnumber(LuaHelper::mState, fBattleSpeech);
    lua_pushnumber(LuaHelper::mState, 10.0f);
    lua_pushnumber(LuaHelper::mState, fMaster);
    return 6;
}

// FUNCTION: SWBF2 0x0059cf60
int ScriptCB_SetVolumes(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 5 &&
        lua_isnumber(LuaHelper::mState, 1) &&
        lua_isnumber(LuaHelper::mState, 2) &&
        lua_isnumber(LuaHelper::mState, 3))
    {
        float fMusic = (float)luaL_checknumber(LuaHelper::mState, 1);
        float fSFX = (float)luaL_checknumber(LuaHelper::mState, 2);
        float fSpeech = (float)luaL_checknumber(LuaHelper::mState, 3);
        float fBattleSpeech = (float)luaL_checknumber(LuaHelper::mState, 4);
        float fMaster = (float)luaL_checknumber(LuaHelper::mState, 5);
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
        if (pProfile)
        {
            PlayerProfile::tSoundOpt& opt = pProfile->mSoundOptions;
            opt.mu8SFXVol = (unsigned char)(int)fSFX;
            opt.mu8MasterVol = (unsigned char)(int)fMaster;
            opt.mu8MusicVol = (unsigned char)(int)fMusic;
            opt.mu8SpeechVol = (unsigned char)(int)fSpeech;
            opt.mu8BattleSpeechVol = (unsigned char)(int)fBattleSpeech;
            pProfile->UpdateProfCRC();
            opt.ApplySettings();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059d080
int ScriptCB_EffectsEnabled(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState,
        Snd::Engine::smSoftwareEffectsEnabled || Snd::Engine::smHardwareEffectsAvailable);
    return 1;
}

// FUNCTION: SWBF2 0x0059d160
int ScriptCB_HWSupport(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, GameSoundEngine::sHwEffectsSupported);
    lua_pushboolean(LuaHelper::mState, GameSoundEngine::sHwMixingSupported);
    return 2;
}

// FUNCTION: SWBF2 0x0059d190
int ScriptCB_GetOutputMode(lua_State* L)
{
    lua_pushnumber(LuaHelper::mState, (float)Snd::EngineBase::smSpeakerConfig);
    return 1;
}

// FUNCTION: SWBF2 0x0059d4b0
int ScriptCB_GetMixConfig(lua_State* L)
{
    lua_pushnumber(LuaHelper::mState, (float)Snd::Engine::GetMixConfig());
    return 1;
}

// FUNCTION: SWBF2 0x0059d7c0
int ScriptCB_GetEAXVersion(lua_State* L)
{
    if (!Lua_Callbacks::sSoundEnabled)
    {
        return 0;
    }
    if (Snd::Engine::smHardwareEffectsAvailable
        && Snd::Engine::GetMixConfig() == Snd::Engine::mixConfig_DirectSoundHardware)
    {
        lua_pushnumber(LuaHelper::mState, (float)(int)Snd::Engine::GetEAXVersion());
        return 1;
    }
    lua_pushnumber(LuaHelper::mState, 0.0f);
    return 1;
}

// FUNCTION: SWBF2 0x0059d830
int ScriptCB_GetMixConfigChanged(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, GameSoundEngine::MixConfigChanged());
    return 1;
}

// FUNCTION: SWBF2 0x0059d8a0
int ScriptCB_GetBassManagement(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, Snd::Engine::GetBassManagement());
    return 1;
}

// FUNCTION: SWBF2 0x0059d8d0
void PlayShellMusic(unsigned int uID)
{
    if (!Lua_Callbacks::sSoundEnabled || uID == Lua_Callbacks::sShellMusicID)
    {
        return;
    }

    const Snd::SoundStream::Properties* pCurProps =
        Snd::SoundStream::Properties::FindByHashID(Lua_Callbacks::sShellMusicID ? Lua_Callbacks::sShellMusicID : uID);
    const Snd::SoundStream::Properties* pNewProps =
        Snd::SoundStream::Properties::FindByHashID(uID);
    if (!pCurProps)
    {
        return;
    }
    Snd::Stream* pStream = Snd::EngineBase::GetOpenStream(pCurProps->mStreamID);
    if (!pStream)
    {
        return;
    }

    Snd::Engine::GetThreadUpdate()->Pause(false);
    if (Lua_Callbacks::sShellMusicID)
    {
        pStream->Stop();
        pStream->Update(0.0f);
        Lua_Callbacks::sShellMusicID = 0;
    }
    if (pNewProps)
    {
        pNewProps->SetupStream(pStream, 0);
        pStream->Play(true);
    }
    Snd::Engine::GetThreadUpdate()->Resume();
    Lua_Callbacks::sShellMusicID = uID;
    Lua_Callbacks::sShellMusicStoppedTime = 0;
}

// FUNCTION: SWBF2 0x0059d990
void UpdateShellMusic(float fDeltaTime)
{
    if (!Lua_Callbacks::sSoundEnabled || !Lua_Callbacks::sShellMusicID)
    {
        return;
    }

    const Snd::SoundStream::Properties* pProps =
        Snd::SoundStream::Properties::FindByHashID(Lua_Callbacks::sShellMusicID);
    if (!pProps)
    {
        return;
    }

    Snd::Stream* pStream = Snd::EngineBase::GetOpenStream(pProps->mStreamID);
    if (!pStream || pStream->GetPlayState() != Snd::SourceTransport::playState_stopped)
    {
        return;
    }

    float& fStoppedTime = *(float*)&Lua_Callbacks::sShellMusicStoppedTime;
    fStoppedTime += fDeltaTime;
    if (fStoppedTime > sShellMusicInterval)
    {
        fStoppedTime = 0.0f;
        Snd::SoundStream::Play(pProps, true);
    }
}

// FUNCTION: SWBF2 0x0059da20
void UpdateDelayedSounds(float fDeltaTime)
{
    for (int i = 0; i < 2; ++i)
    {
        sDelayedSounds[i].Update(fDeltaTime);
    }
}

// FUNCTION: SWBF2 0x0059db20
int ScriptCB_SetShellMusic(lua_State* L)
{
    if (Lua_Callbacks::sSoundEnabled)
    {
        if (lua_gettop(LuaHelper::mState))
        {
            PlayShellMusic(PblHash(LuaHelper::GetString(1)));
            return 0;
        }
        PlayShellMusic(0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059db80
int ScriptCB_SetShellMusicInterval(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1)
    {
        sShellMusicInterval = luaL_checknumber(LuaHelper::mState, 1);
    }
    else
    {
        sShellMusicInterval = 10.0f;
    }

    *(float*)&Lua_Callbacks::sShellMusicStoppedTime = sShellMusicInterval;
    Music::IgnoreInactiveTime(sShellMusicInterval == 0.0f);
    return 0;
}

// FUNCTION: SWBF2 0x0059dbf0
int ScriptCB_IsShellMusicPlaying(lua_State* L)
{
    const unsigned int uID = PblHash(LuaHelper::GetString(1));
    lua_pushnumber(LuaHelper::mState, (lua_Number)(uID == Lua_Callbacks::sShellMusicID));
    return 1;
}

// FUNCTION: SWBF2 0x0059dc40
int ScriptCB_ShellPlayDelayedStream(lua_State* L)
{
    if (Lua_Callbacks::sSoundEnabled)
    {
        const char* pcStream = LuaHelper::GetString(1);
        const int iSlot = (int)luaL_optnumber(LuaHelper::mState, 2, 0.0f);
        const float fDelay = luaL_optnumber(LuaHelper::mState, 3, 0.0f);
        const char* pcBus = luaL_optlstring(LuaHelper::mState, 4, 0, 0);
        const bool bQueue = (int)luaL_optnumber(LuaHelper::mState, 5, 1.0f) > 0;

        float fGain = -1.0f;
        if (pcBus)
        {
            Snd::Bus* pBus = Snd::Bus::FindByHashID(PblHash(pcBus));
            if (pBus)
            {
                fGain = pBus->mGain;
            }
        }

        sDelayedSounds[iSlot].Init(PblHash(pcStream), fDelay * 0.25f, fGain, bQueue);
        UpdateDelayedSounds(0.0f);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059dd80
int ScriptCB_SoundEnable(lua_State* L)
{
    Lua_Callbacks::sSoundEnabled = true;
    return 0;
}

// FUNCTION: SWBF2 0x0059dd90
int ScriptCB_SoundDisable(lua_State* L)
{
    GameSoundEngine::StopAll();
    GameSoundEngine::Update(0.1f);
    Lua_Callbacks::sSoundEnabled = false;
    return 0;
}

// FUNCTION: SWBF2 0x0059ddb0
int ScriptCB_SetDopplerFactor(lua_State* L)
{
    const float fFactor = luaL_checknumber(LuaHelper::mState, 1);
    for (int i = 0; i < 4; ++i)
    {
        Snd::EngineBase::smListeners[i].SetDopplerFactor(fFactor);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059de00
void InitDelayedSounds()
{
    for (int i = 0; i < 2; ++i)
    {
        sDelayedSounds[i].Init(0, 0.0f, -1.0f, true);
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0059b370
StreamBase::Info* StreamBase::Info::Find(unsigned int uiHashID, StreamBase* pStream, bool bAddRef)
{
    Info* pInfo = Find(uiHashID);
    if (!pInfo)
    {
        return 0;
    }

    if (pStream)
    {
        if (pStream->mNode.mNext)
        {
            pInfo->mOverrideManager = pStream->mManager;
        }
        else
        {
            pInfo->mStreamManager.mVoiceList.AddLast(&pStream->mNode);
            pStream->mManager = &pInfo->mStreamManager;
            ++pInfo->mStreamManager.mNumManagedVoices;
        }
    }
    if (bAddRef)
    {
        ++pInfo->mRefCount;
    }
    return pInfo;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0059b3e0
void StreamBase::SeekSegment(const Segment* pSegment)
{
    mCurrentSegment = const_cast<Segment*>(pSegment);
    if (SourceTransport::GetPlayState() == SourceTransport::playState_playing)
    {
        Play(true);
    }
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0059b4e0
StreamBase::Segment* StreamBase::FindSegment(unsigned int uiSegmentID) const
{
    for (const PblListSingle::Node* pLink = mInfoList.mHead.mNext; pLink != &mInfoList.mHead;
         pLink = pLink->mNext)
    {
        const Info* pInfo = (const Info*)((const ObjectList::Node*)pLink)->mObject;
        if (!pInfo)
        {
            break;
        }
        Segment* pSegment = (Segment*)bsearch(&uiSegmentID, pInfo->mSegments, pInfo->mNumSegments,
                                              sizeof(Segment), Segment::CompareSearch);
        if (pSegment)
        {
            return pSegment;
        }
    }
    return 0;
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0059ded0
void CleanupSound()
{
    if (Lua_Callbacks::sGameMusic)
    {
        delete (GameMusic*)Lua_Callbacks::sGameMusic;
        Lua_Callbacks::sGameMusic = 0;
    }
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0059bde0
int ScriptCB_AudioStreamAppendSegments(lua_State*)
{
    const char* pcFile = luaL_checkstring(LuaHelper::mState, 1);
    const char* pcSegment = luaL_checkstring(LuaHelper::mState, 2);
    unsigned int uiStream = lua_isnil(LuaHelper::mState, 3)
                                ? 0
                                : (int)luaL_checknumber(LuaHelper::mState, 3) - 1;
    unsigned int uiSegment = 0;
    if (lua_isnil(LuaHelper::mState, 3))
    {
        return 0;
    }
    if (strlen(pcSegment) > 1)
    {
        uiSegment = PblHash(pcSegment);
    }
    if (uiStream >= 6)
    {
        return 0;
    }

    char acFullName[256];
    strlcpy(acFullName, LoadUtil::GetDataDirectory(), 256);
    strlcat(acFullName, pcFile, 256);

    PblFile file;
    file.Open(acFullName, true, false, true);
    PblFileChunk chunk(&file);
    if (Snd::EngineBase::smStreams[uiStream].AppendSegments(chunk, uiSegment, pcFile, 0, 0))
    {
        lua_pushnumber(LuaHelper::mState, (float)(int)(uiStream + 1));
        return 1;
    }
    return 0;
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0059c110
int ScriptCB_SetBleedingVoiceOver(lua_State*)
{
    int iTeam = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iIndex = (int)luaL_checknumber(LuaHelper::mState, 2);
    VoiceOverManager::sInstance->SetBleedingVoiceOver(iTeam - 1, iIndex - 1,
                                                      luaL_checkstring(LuaHelper::mState, 3),
                                                      (int)luaL_checknumber(LuaHelper::mState, 4) != 0);
    return 0;
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0059c1c0
int ScriptCB_SetLowReinforcementsVoiceOver(lua_State*)
{
    const int nArgs = lua_gettop(LuaHelper::mState);
    int iTeam = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iIndex = (int)luaL_checknumber(LuaHelper::mState, 2);
    int iCount;
    if (nArgs == 5 && (int)luaL_checknumber(LuaHelper::mState, 5))
    {
        iCount = (int)(Team::sTeams[iTeam]->mCurReinforcements *
                       luaL_checknumber(LuaHelper::mState, 4));
    }
    else
    {
        iCount = (int)luaL_checknumber(LuaHelper::mState, 4);
    }
    VoiceOverManager::sInstance->SetLowReinforcementsVoiceOver(
        iTeam - 1, iIndex - 1, luaL_checkstring(LuaHelper::mState, 3), iCount);
    return 0;
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0059c5e0
int ScriptCB_SetAmbientMusic(lua_State*)
{
    const int nArgs = lua_gettop(LuaHelper::mState);
    int iTeam = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iCount;
    if (nArgs == 5 && (int)luaL_checknumber(LuaHelper::mState, 5))
    {
        iCount = (int)(Team::sTeams[iTeam]->mCurReinforcements *
                       luaL_checknumber(LuaHelper::mState, 2));
    }
    else
    {
        iCount = (int)luaL_checknumber(LuaHelper::mState, 2);
    }
    VoiceOverManager::sInstance->SetAmbientMusic(
        iTeam - 1, iCount, luaL_checkstring(LuaHelper::mState, 3),
        (VoiceOverManager::GameStage)(unsigned int)luaL_checknumber(LuaHelper::mState, 4));
    return 0;
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0059c6b0
int ScriptCB_SetAttackingTeam(lua_State*)
{
    VoiceOverManager::sInstance->SetAttackingTeam((int)luaL_checknumber(LuaHelper::mState, 1) - 1);
    return 0;
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0059c6e0
int ScriptCB_SetVictoryMusic(lua_State*)
{
    EndDisplay::sInstance->SetVictoryMusic((int)luaL_checknumber(LuaHelper::mState, 1) - 1,
                                           luaL_checkstring(LuaHelper::mState, 2));
    return 0;
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0059c730
int ScriptCB_SetDefeatMusic(lua_State*)
{
    EndDisplay::sInstance->SetDefeatMusic((int)luaL_checknumber(LuaHelper::mState, 1) - 1,
                                          luaL_checkstring(LuaHelper::mState, 2));
    return 0;
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0059ce00
int ScriptCB_SetOutOfBoundsVoiceOver(lua_State*)
{
    BoundaryManager::sInstance->SetOutOfBoundsVoiceOverID(
        (unsigned int)luaL_checknumber(LuaHelper::mState, 1), luaL_checkstring(LuaHelper::mState, 2));
    return 0;
}
}

// LIBRARY: SWBF2 0x00752ace
// qsort

// GLOBAL: SWBF2 0x01ec7d38
// SYNTHETIC: SWBF2 0x00404200
// `dynamic initializer for 'sAuditionEffects''
// SYNTHETIC: SWBF2 0x00768cb0
// `dynamic atexit destructor for 'sAuditionEffects''
GameSound sAuditionEffects[3];
