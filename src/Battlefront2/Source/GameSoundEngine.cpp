#include <new>
#include <stdlib.h>

#include <windows.h>

#include "GameSoundEngine.h"
#include "GameMemory.h"
#include "GameState.h"
#include "GameVoiceChat.h"
#include "LoadUtil.h"
#include "Music.h"
#include "NetSetup.h"
#include "PlayerProfile.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "SoundFL/source/sndbus.h"
#include "SoundFL/source/sndfreelist.h"
#include "SoundFL/source/sndi3dl2reverb.h"
#include "SoundFL/source/sndsoundlayered.h"
#include "SoundFL/source/sndglobals.h"
#include "SoundFL/source/sndsoundstream.h"
#include "SoundFL/source/win/sndengine.h"
#include "SoundFL/source/win/sndstream.h"
#include "SoundFL/source/sndsoundparameterized.h"
#include "EntitySound.h"
#include "FoleyFX.h"
#include "LuaCallbacks_Sound.h"
#include "VehicleEngine.h"
#include "TriggerSoundRegion.h"
#include "GameVoiceChatNode.h"
#include "GameVoiceChatDecoder.h"
#include "GameVoiceChatEncoder.h"
#include "VoiceChatTransport.h"
#include "GameVoiceChatTransport.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblTime.h"
#include "GameLoop.h"
#include "GameMusic.h"

// GLOBAL: SWBF2 0x01faa688

static unsigned __int64 sLastSoundTick;
// GLOBAL: SWBF2 0x01faa684

static float sAccumulatedSoundTime;

namespace FrameUpdate {
extern float fOneOverTicksPerSecond;
}
extern int g_InGameMoviePlayerState;
namespace Lua_Callbacks { extern bool sGameMusicRetrigger; }
namespace Lua_Callbacks { extern unsigned int sGameMusicCurDelayTime; }
namespace Lua_Callbacks { extern void* sGameMusic; }
namespace Lua_Callbacks { extern unsigned int sGameMusicDelayTime; }

extern HWND hGlobalWnd;

// GLOBAL: SWBF2 0x007e68e4
namespace GameSoundEngine { int gSamplerate = 48000; }
// GLOBAL: SWBF2 0x007e68e8
namespace GameSoundEngine { int gMaxVoices = 32; }
// GLOBAL: SWBF2 0x007e68ec
namespace GameSoundEngine { float gSoftMixerBufferSizeMs = 100.0f; }

// GLOBAL: SWBF2 0x007e68f0
bool gCentreSpeakerEnhancement = true;

namespace GameSoundEngine { extern bool sHwMixingSupported; }
namespace GameSoundEngine { extern bool sHwEffectsSupported; }

namespace GameVoiceChat { bool Open(float fSeconds, bool bSoundWorking); }

// GLOBAL: SWBF2 0x01e56282
bool gSoundEnabled;

namespace GameSoundEngine
{

// GLOBAL: SWBF2 0x01ec3238
// SYNTHETIC: SWBF2 0x00403df0
// `dynamic initializer for 'GameSoundEngine::gSoundHeap''
int gSoundHeap = GameMemory::RunTimeHeap;

bool gSoundIsOpen;
unsigned long gWindowsSpeakerConfig;

Snd::EngineBase::SpeakerConfig gDefaultSpeakerConfig;

// FUNCTION: SWBF2 0x005390e0
void SetHeap(int iHeap)
{
    gSoundHeap = iHeap;
}

}

// FUNCTION: SWBF2 0x005387a0
void* __cdecl SndAlloc(unsigned int uiSize)
{
    return RedAllocFromHeap(GameSoundEngine::gSoundHeap, uiSize, 0);
}

// FUNCTION: SWBF2 0x005387c0
void __cdecl SndFree(void* pBlock)
{
    RedFreeToHeap(GameSoundEngine::gSoundHeap, pBlock, 0);
}

// FUNCTION: SWBF2 0x005387e0
void* __cdecl SndTempAlloc(unsigned int uiSize)
{
    return RedAllocFromHeap(GameMemory::RunTimeHeap, uiSize, 0);
}

// FUNCTION: SWBF2 0x00538800
void __cdecl SndTempFree(void* pBlock)
{
    RedFreeToHeap(GameMemory::RunTimeHeap, pBlock, 0);
}

// FUNCTION: SWBF2 0x00538820
void __cdecl PrintDebugString(char const*)
{
}

namespace GameSoundEngine {

// FUNCTION: SWBF2 0x00538830
bool __cdecl UseKmixer()
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (!pProfile)
    {
        return false;
    }

    return (unsigned char)(pProfile->mSoundOptions.mPlaybackMode - 1) > 3;
}
}

namespace GameSoundEngine
{

// FUNCTION: SWBF2 0x00538850
bool Init(unsigned int uiFlags, bool bEnable)
{
    if (gSoundIsOpen)
    {
        return true;
    }

    Snd::Globals::smAlloc = SndAlloc;
    Snd::Globals::smFree = SndFree;
    Snd::Globals::smAllocPerm = malloc;
    Snd::Globals::smFreePerm = free;
    Snd::Globals::smTempAlloc = malloc;
    Snd::Globals::smTempFree = free;
    Snd::Globals::smWarningFunction = PrintDebugString;

    if (g_bNoSound)
    {

        gSoundIsOpen = Snd::Engine::Open(0, Snd::Engine::mixConfig_Disabled, true, 100.0f,
                                         true, 48000, 32, 0, 0);
    }
    else
    {
        bool bUseKmixer = GameSoundEngine::UseKmixer();

        unsigned long* pulWindowsSpeakerConfig =
            (gWindowsSpeakerConfig != 0) ? 0 : &gWindowsSpeakerConfig;

        Snd::Engine::MixConfig eMixConfig =
            (uiFlags != 0) ? (Snd::Engine::MixConfig)uiFlags : Snd::Engine::mixConfig_Software;

        gSoundIsOpen = Snd::Engine::Open(hGlobalWnd, eMixConfig, bEnable, GameSoundEngine::gSoftMixerBufferSizeMs,
                                         !bUseKmixer, (unsigned int)GameSoundEngine::gSamplerate,
                                         (unsigned int)GameSoundEngine::gMaxVoices,
                                         pulWindowsSpeakerConfig, 0);

        if (uiFlags == Snd::Engine::mixConfig_DirectSoundHardware &&
            Snd::Engine::GetMixConfig() != Snd::Engine::mixConfig_DirectSoundHardware)
        {
            GameSoundEngine::sHwMixingSupported = false;
            if (bEnable)
            {
                GameSoundEngine::sHwEffectsSupported = false;
            }
        }

        PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();

        if (gDefaultSpeakerConfig == Snd::EngineBase::speakerConfig_Invalid)
        {

            gDefaultSpeakerConfig = Snd::EngineBase::smSpeakerConfig;
            if (pProfile)
            {
                pProfile->mSoundOptions.mPlaybackMode =
                    (unsigned char)Snd::EngineBase::smSpeakerConfig;
            }
        }

        if (pProfile)
        {
            Snd::Engine::SetSpeakerConfig(
                (Snd::EngineBase::SpeakerConfig)pProfile->mSoundOptions.mPlaybackMode);

            pProfile->mSoundOptions.mPlaybackMode =
                (unsigned char)Snd::EngineBase::smSpeakerConfig;
        }
    }

    bool bSoundWorking = (Snd::Engine::GetMixConfig() != Snd::Engine::mixConfig_Disabled);

    if (gSoundIsOpen)
    {
        GameVoiceChat::Open(20.0f, bSoundWorking);
    }

    gSoundHeap = GameMemory::RunTimeHeap;
    return gSoundIsOpen;
}

}

// GLOBAL: SWBF2 0x007e3504
// SYNTHETIC: SWBF2 0x0040b7f0
// `dynamic initializer for 'Snd::SoundParameterized::Layer::smFreeList''
// SYNTHETIC: SWBF2 0x0076a250
// `dynamic atexit destructor for 'Snd::SoundParameterized::Layer::smFreeList''
Snd::FreeList Snd::SoundParameterized::Layer::smFreeList(0x100, 1, 0xa0, 1);

namespace GameSoundEngine
{

// FUNCTION: SWBF2 0x00538a20
void Create()
{

    gSoundHeap = GameMemory::RunTimeHeap;

    for (unsigned int i = 0; i < 4; ++i)
    {
        Snd::EngineBase::smListeners[i].mDopplerFactor = 1.0f;
    }

    Snd::SoundStream::Init();

    unsigned int uiBlocks = 0;
    for (const PblListSingle::Node* pNode = Snd::SoundParameterized::Layer::smFreeList.mBlockList.mHead.mNext;
         pNode != &Snd::SoundParameterized::Layer::smFreeList.mBlockList.mHead;
         pNode = pNode->mNext)
    {
        ++uiBlocks;
    }

    if (uiBlocks == 0)
    {

        Snd::SoundParameterized::Layer::smFreeList.AllocBlocks(1, 0);
        Snd::SoundParameterized::Layer::smFreeList.mFlags |= 2;
    }
}

}

extern int sVoiceChatHeap;
namespace GameVoiceChat { extern bool gPlayerChange; }
extern void* sVoiceChatEncoders[1];
namespace GameVoiceChat { extern void* gNodes[1]; }
extern void* sVoiceChatDecoders[1];
extern void* sEncoderStorage;
extern void* sDecoderStorage;
extern void* sNodeStorage;

namespace GameVoiceChat { unsigned int GetNumNodes(); }

extern int netVoiceMode;

namespace GameVoiceChat {

// FUNCTION: SWBF2 0x0053c040
bool Open(float fSeconds, bool bSoundWorking)
{

    sVoiceChatHeap = GameMemory::ApplicationHeap;
    int iOldHeap = RedSetCurrentHeap(GameMemory::ApplicationHeap);

    if (!GameVoiceChat::gTransport.Open())
    {
        RedSetCurrentHeap(iOldHeap);
        return false;
    }

    if (!VoiceChat::Node::OpenStatic(fSeconds, 1000.0f, 1000.0f))
    {
        GameVoiceChat::gTransport.Close();
        RedSetCurrentHeap(iOldHeap);
        return false;
    }

    unsigned int uiNodes = (unsigned int)GameVoiceChat::GetNumNodes();
    bool bAllOpen = true;

    for (unsigned int i = 0; i < uiNodes; ++i)
    {
        sVoiceChatEncoders[i] = new ((GameVoiceChat::Encoder*)sEncoderStorage + i) GameVoiceChat::Encoder;
        sVoiceChatDecoders[i] = new ((GameVoiceChat::Decoder*)sDecoderStorage + i) GameVoiceChat::Decoder;
        GameVoiceChat::gNodes[i] = new ((GameVoiceChat::Node*)sNodeStorage + i)
            GameVoiceChat::Node((GameVoiceChat::Encoder*)sVoiceChatEncoders[i],
                                (GameVoiceChat::Decoder*)sVoiceChatDecoders[i]);
    }

    GameVoiceChat::gPlayerChange = true;

    VoiceChat::Transport::Method cMethod(
        (VoiceChat::Transport::Method::Type)(netVoiceMode ? netVoiceMode : 2), 0x32);
    cMethod.SetNumRelayNodes(cMethod.CalculateOptimalRelayNodes(0x32));
    GameVoiceChat::gTransport.SetMethod(&cMethod, 1);

    if (bSoundWorking)
    {
        unsigned int i;

        for (i = 0; i < uiNodes && bAllOpen; ++i)
        {
            bAllOpen = (bAllOpen && ((VoiceChat::Node*)GameVoiceChat::gNodes[i])->Open());
        }

        if (!bAllOpen)
        {

            for (i = 0; i < uiNodes; ++i)
            {
                if (((VoiceChat::Node*)GameVoiceChat::gNodes[i])->IsOpen())
                {
                    ((VoiceChat::Node*)GameVoiceChat::gNodes[i])->Close();
                }
            }

            for (i = 0; i < uiNodes; ++i)
            {
                ((GameVoiceChat::Encoder*)sVoiceChatEncoders[i])->~Encoder();

                ((GameVoiceChat::Decoder*)sVoiceChatDecoders[i])->~Decoder();

                sVoiceChatEncoders[i] = 0;
                sVoiceChatDecoders[i] = 0;
                GameVoiceChat::gNodes[i] = 0;
            }

            VoiceChat::Node::CloseStatic();
            GameVoiceChat::gTransport.Close();
            RedSetCurrentHeap(iOldHeap);
            return false;
        }

        for (i = 0; i < uiNodes; ++i)
        {
            ((GameVoiceChat::Node*)GameVoiceChat::gNodes[i])->SetSendMask(1);
            ((GameVoiceChat::Node*)GameVoiceChat::gNodes[i])->SetReceiveMask(4);
        }
    }

    RedSetCurrentHeap(iOldHeap);
    return true;
}
}

class SoundSpaceRegion
{
public:
    static void DestroyAll();
};

namespace GameSoundEngine
{

// GLOBAL: SWBF2 0x007ec1dc
BusGains gBusGainsDefault;
// GLOBAL: SWBF2 0x007ec1f8
BusGains gBusGainsUser;

// GLOBAL: SWBF2 0x01e5639c
bool gMixConfigChanged;

// GLOBAL: SWBF2 0x01e5639d
bool gDisableStartupMusic;

// FUNCTION: SWBF2 0x005390f0
int GetHeap()
{
    return gSoundHeap;
}

// FUNCTION: SWBF2 0x00539100
void DisableStartupMusic()
{
    gDisableStartupMusic = true;
}

// FUNCTION: SWBF2 0x00538ba0
bool MixConfigChanged()
{
    bool bChanged = gMixConfigChanged;
    gMixConfigChanged = false;
    return bChanged;
}

// FUNCTION: SWBF2 0x005389e0
void Term(bool bFlag)
{
    if (gSoundIsOpen)
    {
        if (GameVoiceChat::IsOpen())
        {
            GameVoiceChat::Close();
        }
        Destroy();
        Snd::Engine::Close(gWindowsSpeakerConfig, bFlag);
        gSoundIsOpen = false;
        gMixConfigChanged = false;
    }
}

// FUNCTION: SWBF2 0x00538a60
void Destroy()
{
    VehicleEngineClass::Boost::DestroyAll();
    EntitySound::DestroyAll();
    Music::DestroyAll();
    VehicleEngine::DeactivateAll();
    StopAll();
    FoleyFXGroup::DestroyAll();
    FoleyFX::DestroyAll();

    for (unsigned int i = 0; i < 6; i++)
    {
        Snd::Stream* pStream = &Snd::EngineBase::smStreams[i];
        pStream->SetBus(0);
        if (!pStream->mInfoList.IsEmpty())
        {
            pStream->Close();
        }
        if (pStream->GetLocked())
        {
            pStream->Lock(false);
        }
    }

    Snd::Sound::RemoveAll();
    Snd::SoundLayered::RemoveAll();
    Snd::SoundParameterized::RemoveAll();
    Snd::SoundParameterized::Free();
    SoundSpaceRegion::DestroyAll();
    TriggerSoundRegion::DestroyAll();

    for (int i = 0; i < 4; i++)
    {
        Snd::EngineBase::smListeners[i].SetSpace(0);
    }
    Snd::Engine::Update(0.1f);
    for (int i = 0; i < 4; i++)
    {
        Snd::EngineBase::smListeners[i].Enable(false);
    }

    Snd::EngineBase::DestroyAll();
    Lua_Callbacks::InitDelayedSounds();
    Snd::Stream::EnableStreaming(true);
}

// FUNCTION: SWBF2 0x00538bb0
bool ChangeMixConfig(unsigned int uiFlags, bool bEnable)
{
    if (GameState::GetState() != 0x11E1FC01)
    {
        return false;
    }

    if (uiFlags == 2)
    {
        if (!GameSoundEngine::sHwMixingSupported && !GameSoundEngine::UseKmixer())
        {
            return false;
        }
        if (bEnable && !GameSoundEngine::sHwEffectsSupported)
        {
            return false;
        }
    }

    Term(false);
    if (!Init(uiFlags, bEnable))
    {
        return false;
    }

    unsigned int auiChunks[2] = { 0x5E262256, 0x5F646E73 };
    Snd::EngineBase::smListeners[0].Enable(true);
    LoadUtil::ReadDataFileOnHeap("core.lvl", -1, auiChunks, 2, 0, 0, 0);
    gMixConfigChanged = true;
    return true;
}

// FUNCTION: SWBF2 0x00538c60
void StopAll()
{
    for (unsigned int i = 0; i < 6; i++)
    {
        Snd::Stream* pStream = Snd::EngineBase::GetStream(i);
        if (pStream->GetPlayState() == Snd::SourceTransport::playState_playing)
        {
            pStream->SourceBus::Stop(pStream);
            pStream->SourceTransport::Stop();
        }
    }

    Snd::Sound::StopAll();
    Snd::SoundLayered::FireForgetLayer::StopAll();
    Music::StopAll();
    Snd::Engine::Update(0.00001f);

    Snd::I3DL2Reverb reverb;
    reverb.mFlags |= Snd::I3DL2Reverb::flags_roomGainChanged;
    reverb.mRoomGain = 0.0f;
    Snd::Engine::SetGlobalReverb(&reverb);
}

// FUNCTION: SWBF2 0x00538e50
void UpdateBusGains(float fSFX, float fMusic, float fVoiceOver, float fBattleVO, float fMaster)
{
    gBusGainsUser.mSFX = fSFX;
    gBusGainsUser.mMusic = fMusic;
    gBusGainsUser.mVoiceOver = fVoiceOver;
    gBusGainsUser.mBattleVO = fBattleVO;
    gBusGainsUser.mMaster = fMaster;

    Snd::Bus* pBus;
    if ((pBus = Snd::Bus::FindByHashID(0x3b13aa4b)) != 0)
    {
        pBus->SetGain(gBusGainsDefault.mVoiceOver * gBusGainsUser.mVoiceOver, false);
    }
    if ((pBus = Snd::Bus::FindByHashID(0x74d771fc)) != 0)
    {
        pBus->SetGain(gBusGainsDefault.mBattleVO * gBusGainsUser.mBattleVO, false);
    }
    if ((pBus = Snd::Bus::FindByHashID(0xeceb308a)) != 0)
    {
        pBus->SetGain(gBusGainsDefault.mSFX * gBusGainsUser.mSFX, false);
    }
    if ((pBus = Snd::Bus::FindByHashID(0x9f9c4fd4)) != 0)
    {
        pBus->SetGain(gBusGainsDefault.mMusic * gBusGainsUser.mMusic, false);
    }
    if ((pBus = Snd::Bus::FindByHashID(0xaebbad4d)) != 0)
    {
        pBus->SetGain(gBusGainsDefault.mAmbience * gBusGainsUser.mSFX, false);
    }
    if ((pBus = Snd::Bus::FindByHashID(0x82c7f467)) != 0)
    {
        pBus->SetGain(gBusGainsDefault.mShellFX * gBusGainsUser.mSFX, false);
    }
    Snd::Bus::smRootBus.SetGain(gBusGainsUser.mMaster, false);
}

// FUNCTION: SWBF2 0x00538fc0
void InitializeBusGains()
{
    Snd::Bus* pBus;
    if ((pBus = Snd::Bus::FindByHashID(0x3b13aa4b)) != 0)
    {
        gBusGainsDefault.mVoiceOver = pBus->mGain;
    }
    if ((pBus = Snd::Bus::FindByHashID(0x74d771fc)) != 0)
    {
        gBusGainsDefault.mBattleVO = pBus->mGain;
    }
    if ((pBus = Snd::Bus::FindByHashID(0xeceb308a)) != 0)
    {
        gBusGainsDefault.mSFX = pBus->mGain;
    }
    if ((pBus = Snd::Bus::FindByHashID(0x9f9c4fd4)) != 0)
    {
        gBusGainsDefault.mMusic = pBus->mGain;
    }
    if ((pBus = Snd::Bus::FindByHashID(0xaebbad4d)) != 0)
    {
        gBusGainsDefault.mAmbience = pBus->mGain;
    }
    if ((pBus = Snd::Bus::FindByHashID(0x82c7f467)) != 0)
    {
        gBusGainsDefault.mShellFX = pBus->mGain;
    }
    gBusGainsDefault.mMaster = Snd::Bus::smRootBus.mGain;

    if (gDisableStartupMusic)
    {
        gBusGainsDefault.mMusic = 0.0f;
        gBusGainsUser.mMusic = 0.0f;
        gDisableStartupMusic = false;
    }

    UpdateBusGains(gBusGainsUser.mSFX, gBusGainsUser.mMusic, gBusGainsUser.mVoiceOver,
                   gBusGainsUser.mBattleVO, gBusGainsUser.mMaster);
}

}

// FUNCTION: SWBF2 0x005a9d10
void Music::UpdateAll(float fDeltaTime)
{
    int i = 0;
    PblListDouble::Node* pLink = sMusicActive.mTerminator.mNext;
    while (pLink != &sMusicActive.mTerminator)
    {
        PblListDoubleSorted::Node* pNode =
            (PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode));
        if (pNode == sMusicActive.GetTerminator())
        {
            break;
        }
        pLink = pLink->mNext;

        Music* pMusic = (Music*)((char*)pNode - offsetof(Music, mNode));
        if (i == 0)
        {
            if (!sMusicPlaying && (sInactiveTime >= sMinInactiveTime || sIgnoreInactiveTime)
                && pMusic->mPlayStateCurrent != playState_starting
                && pMusic->mPlayStateCurrent != playState_playing)
            {
                sInactiveTime = 0.0f;
                pMusic->mPlayStateDesired = playState_playing;
            }
        }
        else if (pMusic->mPlayStateCurrent != playState_stopping
                 && pMusic->mPlayStateCurrent != playState_stopped
                 && pMusic->mPlayStateCurrent != playState_pausing
                 && pMusic->mPlayStateCurrent != playState_paused)
        {
            pMusic->Stop();
        }
        pMusic->Update(fDeltaTime);
        ++i;
    }

    pLink = sMusicInactive.mTerminator.mNext;
    while (pLink != &sMusicInactive.mTerminator)
    {
        PblListDoubleSorted::Node* pNode =
            (PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode));
        if (pNode == sMusicInactive.GetTerminator())
        {
            break;
        }
        pLink = pLink->mNext;
        ((Music*)((char*)pNode - offsetof(Music, mNode)))->Update(fDeltaTime);
    }

    if (!sMusicPlaying)
    {
        sInactiveTime += fDeltaTime;
    }
}

// FUNCTION: SWBF2 0x00538d00
void GameSoundEngine::Update(float fDeltaTime)
{
    (void)fDeltaTime;
    if (NetGame::IsEnabled() && NetGame::IsDedicated())
    {
        return;
    }
    if (!Snd::EngineBase::smOpened)
    {
        return;
    }

    const unsigned __int64 uiNow = PblTime::GetTickCount();
    const int iTicks = (int)((unsigned int)(uiNow - sLastSoundTick) & 0x7fffffff);
    sLastSoundTick = uiNow;
    const float fElapsed = (float)iTicks * FrameUpdate::fOneOverTicksPerSecond;

    sAccumulatedSoundTime += fElapsed;
    if (!(sAccumulatedSoundTime > 0.02f))
    {
        return;
    }
    sAccumulatedSoundTime = 0.0f;

    Snd::Engine::Update(fElapsed);

    if (g_InGameMoviePlayerState)
    {
        return;
    }
    if (GameState::GetState() == 0x8ff60339 && GameLoop::IsGameOver())
    {
        return;
    }

    Music::UpdateAll(fElapsed);
    Lua_Callbacks::UpdateDelayedSounds(fElapsed);

    if (Lua_Callbacks::sGameMusicRetrigger)
    {
        float& fCountdown = *(float*)&Lua_Callbacks::sGameMusicCurDelayTime;
        if (fCountdown > 0.0f)
        {
            fCountdown -= fElapsed;
        }
        else if (Lua_Callbacks::sGameMusic)
        {
            GameMusic* pMusic = (GameMusic*)Lua_Callbacks::sGameMusic;
            if (pMusic->GetCurrentPlayState() == GameMusic::playState_stopped)
            {
                fCountdown = *(float*)&Lua_Callbacks::sGameMusicDelayTime;
                pMusic->Play();
            }
        }
    }
}

// GLOBAL: SWBF2 0x007e2e20

float sDefaultSoundDamping = 0.95f;

// GLOBAL: SWBF2 0x01ec323c

// SYNTHETIC: SWBF2 0x00403e00
// `dynamic initializer for 'sSoundDamping''
float sSoundDamping = sDefaultSoundDamping;

// GLOBAL: SWBF2 0x007e666e
namespace GameSoundEngine { bool sHwMixingSupported = true; }

// GLOBAL: SWBF2 0x007e666f
namespace GameSoundEngine { bool sHwEffectsSupported = true; }
