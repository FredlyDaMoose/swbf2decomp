#include "RedEngineFL/Util/RedCommandConsole.h"

#include <stdio.h>
#include <string.h>

#include "PauseMenu.h"

#include "BootDisplay.h"
#include "CameraManager.h"
#include "CaptureDisplay.h"
#include "FLGuiManager.h"
#include "FLInputManager.h"
#include "FLJoystick.h"
#include "GameLoop.h"
#include "PlayerProfile.h"
#include "PlayerStats.h"
#include "GameSoundEngine.h"
#include "HeroMessageDisplay.h"
#include "HUDGameEvents.h"
#include "HUDElementMap.h"
#include "HUDEvent.h"
#include "HUDManager.h"
#include "MessageDisplay.h"
#include "NetGame.h"
#include "ReticuleDisplay.h"
#include "SpectatorDisplay.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/input/pc/pcRedInputManager.h"
#include "SoundFL/source/sndbus.h"
#include "SoundFL/source/sndenginebase.h"
#include "SoundFL/source/win/sndengine.h"
#include "Trigger.h"
#include "PlayerController.h"

// GLOBAL: SWBF2 0x01e5600b
namespace GameLoop { bool sFreeMode; }

namespace GameLoop { void SetFreeMode(bool bFreeCamera); }

// FUNCTION: SWBF2 0x00533600
void GameLoop::SetFreeMode(bool bFreeCamera)
{
    if (sFreeMode == bFreeCamera)
    {
        return;
    }
    sFreeMode = bFreeCamera;
    if (bFreeCamera)
    {
        CameraManager::sInstance->FreeLook(0);
    }
    else
    {
        CameraManager::sInstance->ChaseMode(0);
    }
}

class StatusDisplay
{
public:
    static void EnableViewport(unsigned int uiViewport, bool bEnable);
};

namespace GameLoop { void SetFreeMode(bool bFreeCamera); }
namespace PopupText { void DisablePopup(); }

extern bool gAttractMode;
extern bool sPauseMenuSoundEnabled;
extern int gActiveViewport;
extern bool gPauseReverbEnabled;
extern int gSkipToPlayerListState;
extern float gSkipToPlayerListTime;

// GLOBAL: SWBF2 0x007e8ef0
int PauseMenu::sPausingViewport = -1;
// GLOBAL: SWBF2 0x01ec0aac

int gActiveViewport;

// GLOBAL: SWBF2 0x01eaedbd
bool PauseMenu::sSkipToPlayerList;
// GLOBAL: SWBF2 0x01eaedd5
bool PauseMenu::sPauseDedicated;

// GLOBAL: SWBF2 0x01ef7ee8
// SYNTHETIC: SWBF2 0x00405ed0
// `dynamic initializer for '_sPauseMenuInstance''
// SYNTHETIC: SWBF2 0x00769310
// `dynamic atexit destructor for '_sPauseMenuInstance''
PauseMenu _sPauseMenuInstance[1];

// GLOBAL: SWBF2 0x007e8eec
PauseMenu* PauseMenu::sPauseMenuInstance = _sPauseMenuInstance;

// GLOBAL: SWBF2 0x007e8eea
bool PauseMenu::sEnablePause = true;

namespace
{

void EnableDisplays(int iViewport, bool bEnable)
{
    StatusDisplay::EnableViewport(iViewport, bEnable);
    ReticuleDisplay::EnableViewport(iViewport, bEnable);
    CaptureDisplay::EnableViewport(iViewport, bEnable);
    MessageDisplay::EnableViewport(iViewport, bEnable);
    BootDisplay::EnableViewport(iViewport, bEnable);
    HeroMessageDisplay::EnableViewport(iViewport, bEnable);
    SpectatorDisplay::EnableViewport(iViewport, bEnable);
}

void EnableScreenGroups(bool bEnable)
{
    for (int i = 0; i < 5; ++i)
    {
        HUD::gScreenGroup[i].Enable(bEnable);
    }
}

}

// FUNCTION: SWBF2 0x00616300
PauseMenu::PauseMenu()
{
    mVisible = false;
    mFreecamView = false;
    mEntryStatusVisible = true;
    mShowStats = false;
    mStatsDone = false;
    mDebounceTime = 0.0f;
    mInGameReverb = 0;
    mPauseReverb.SetRoomGain(0.0f);
    mPauseReverb.SetRoomHFGain(0.0f);
    mPauseReverb.SetERGain(0.0f);
    mPauseReverb.SetReverbGain(0.0f);
    PauseMenu::sSkipToPlayerList = false;
}

// FUNCTION: SWBF2 0x006163e0
PauseMenu::~PauseMenu()
{
}

// FUNCTION: SWBF2 0x006163f0
bool PauseMenu::GetPaused(int iViewport)
{
    return sPauseMenuInstance[iViewport].mVisible;
}

// FUNCTION: SWBF2 0x00616400
void PauseMenu::SetPaused(int iViewport, bool bPaused)
{
    sPauseMenuInstance[iViewport]._SetPaused(bPaused);
}

// FUNCTION: SWBF2 0x00616420
void PauseMenu::SetFreeCamera(int iViewport, bool bFreeCamera)
{
    sPauseMenuInstance[0]._SetFreeCamera(bFreeCamera);
}

// FUNCTION: SWBF2 0x00616430
int PauseMenu::GetPausingViewport()
{
    if (PauseMenu::sPausingViewport >= 0)
    {
        return PauseMenu::sPausingViewport;
    }

    return (gActiveViewport >= 0) ? gActiveViewport : 0;
}

// FUNCTION: SWBF2 0x00616450
int PauseMenu::GetPausingJoystick()
{
    int iViewport = PauseMenu::sPausingViewport;
    if (iViewport >= 0)
    {
        int iJoystick = FLInputManager::s_instance.GetHWPort(iViewport);
        return (iJoystick >= 0) ? iJoystick : iViewport;
    }
    return (gActiveViewport >= 0) ? gActiveViewport : 0;
}

// FUNCTION: SWBF2 0x00616480
void PauseMenu::SetStatVisible()
{
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        sPauseMenuInstance[i]._SetStatsVisible();
    }
}

// FUNCTION: SWBF2 0x006164b0
bool PauseMenu::DoUpdate(float fDeltaTime)
{
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        sPauseMenuInstance[i].Update(fDeltaTime);
    }
    return true;
}

// FUNCTION: SWBF2 0x00616500
void PauseMenu::SetStatsDone()
{
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        if (sPauseMenuInstance[i].mShowStats)
        {
            sPauseMenuInstance[i].mStatsDone = true;
        }
    }
}

// FUNCTION: SWBF2 0x00616530
void PauseMenu::_Init(int iViewport)
{
    mViewport = iViewport;
    ActivateThread(0, PER_LOCAL_TURN);
    mFreecamSetting = false;
    mShowStats = false;
    mStatsDone = false;
    PauseMenu::sSkipToPlayerList = false;
    PauseMenu::sPauseDedicated = false;
}

// FUNCTION: SWBF2 0x00616570
void PauseMenu::Init()
{
    for (unsigned int i = 0; i < GameLoop::mNumCameras; ++i)
    {
        sPauseMenuInstance[i]._Init(i);
    }
}

// FUNCTION: SWBF2 0x006165a0
bool PauseMenu::IsPauseAllowed()
{
    return !NetGame::IsEnabled();
}

// FUNCTION: SWBF2 0x006165c0
void PauseMenu::_SetPauseAudio(bool bPause)
{
    if (bPause && GameLoop::IsGameOver())
    {
        GameSoundEngine::StopAll();
    }

    if (NetGame::IsEnabled() && GameLoop::GetNumCameras() > 1)
    {
        return;
    }
    if (GameLoop::IsGameOver())
    {
        return;
    }

    Snd::Bus* pBus = Snd::Bus::FindByHashID(0xFA930D1C);
    if (bPause)
    {
        if (pBus)
        {
            pBus->Fade(1.0f, 0.0f, -1.0f);
        }

        for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
        {
            Snd::EngineBase::smListeners[i].mFlags &= ~Snd::ListenerBase::flags_reverbUpdate;
        }

        if (gPauseReverbEnabled)
        {
            mInGameReverb = const_cast<Snd::I3DL2Reverb*>(Snd::Engine::GetGlobalReverb());
            Snd::Engine::SetGlobalReverb(&mPauseReverb);
        }
    }
    else
    {
        if (pBus)
        {
            pBus->Fade(1.0f, 1.0f, -1.0f);
        }

        for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
        {
            Snd::Listener& listener = Snd::EngineBase::smListeners[i];
            Snd::Space* pSpace = listener.mSpace;
            listener.mFlags |= Snd::ListenerBase::flags_reverbUpdate;
            listener.mSpace = 0;
            listener.SetSpace(pSpace);
        }

        if (gPauseReverbEnabled)
        {
            if (mInGameReverb)
            {
                Snd::Engine::SetGlobalReverb(mInGameReverb);
            }
            mInGameReverb = 0;
        }
    }
}

// FUNCTION: SWBF2 0x00616710
void PauseMenu::_SetPaused(bool bPaused)
{
    if (bPaused == mVisible || mDebounceTime > 0.0f)
    {
        return;
    }

    int iViewport = mViewport;
    mVisible = bPaused;
    mDebounceTime = 0.25f;
    if (GameLoop::IsGameOver())
    {
        iViewport = 0;
    }

    RedInputManager::GetMouse(0)->SetCursorPos(RedInterfaceScreen::GetScreenWidth() >> 1,
                                               RedInterfaceScreen::GetScreenHeight() >> 1);

    GuiManager* pGui = GuiManager::Instance(iViewport);
    if (mVisible)
    {
        if (iViewport >= 1)
        {
            char screen[256];
            _snprintf(screen, 256, "ifs_pausemenu%d", iViewport + 1);
            pGui->PushScreen(screen, iViewport, 0);
        }
        else
        {
            pGui->PushScreen("ifs_pausemenu", iViewport, 0);
        }
        GuiManager::sStartupDelay = 3;
        _SetPauseAudio(true);

        if (PauseMenu::IsPauseAllowed())
        {
            GameLoop::Pause();
        }
        else if (!NetGame::IsEnabled())
        {
            PlayerController::sEnabled = 0;
        }
    }
    else
    {
        pGui->PopScreen(0);

        if (PauseMenu::IsPauseAllowed())
        {
            GameLoop::Resume();
        }
        else
        {
            PlayerController::sEnabled = NetGame::IsEnabled() ? PlayerController::sEnabled : -1;
        }

        PopupText::DisablePopup();
        FLInputManager::s_instance
            .GetPlayerJoystick(GuiManager::GetJoystickFromViewport(GuiManager::mCurViewport))
            ->ClearInputsToOff();
        _SetPauseAudio(false);
    }

    if (sPauseMenuSoundEnabled)
    {
        EnableDisplays(iViewport, !mVisible);
        for (int i = 0; i < 5; ++i)
        {
            HUD::gScreenGroup[i].Enable(!mVisible);
            HUD::gScreenGroupEdit[i].Enable(HUD::gScreenGroupEditEnabled && !mVisible);
        }

        MessageDisplay* pMessages = MessageDisplay::Instance(iViewport, MessageDisplay::DISPLAY_SYSTEM_MSG);
        if (pMessages)
        {
            pMessages->Update(0.01f);
        }
        pMessages = MessageDisplay::Instance(iViewport, MessageDisplay::DISPLAY_SYSTEM_MSG);
        if (pMessages)
        {
            pMessages->Update(0.01f);
        }

        PauseMenu::sPausingViewport = mVisible ? PauseMenu::sPausingViewport : -1;
    }
}

// FUNCTION: SWBF2 0x006169a0
void PauseMenu::_SetFreeCamera(bool bFreeCamera)
{
    bool bFree = !mFreecamSetting;
    mFreecamSetting = bFree;
    if (mFreecamView == bFree)
    {
        return;
    }

    GuiManager* pGui = GuiManager::Instance(mViewport);
    if (mVisible)
    {
        StatusDisplay::EnableViewport(mViewport, true);
        pGui->PopScreen(0);
        mVisible = false;
        GameLoop::SetFreeMode(bFree);
    }

    if (!bFree)
    {
        if (PauseMenu::IsPauseAllowed())
        {
            GameLoop::Resume();
        }
        EnableDisplays(mViewport, !mVisible);
        _SetPauseAudio(false);
    }

    mFreecamView = bFree;
}

// FUNCTION: SWBF2 0x00616a90
void PauseMenu::_SetStatsVisible()
{
    mShowStats = true;
    GuiManager* pGui = GuiManager::Instance(mViewport);
    if (mViewport == 0)
    {
        pGui->PushScreen("ifs_teamstats", GetPausingJoystick(), 0);
    }
    FLInputManager::s_instance.SetHotController(gActiveViewport);
}

// FUNCTION: SWBF2 0x00616ae0
void PauseMenu::UpdateAllInstances(float fDeltaTime)
{
    for (unsigned int i = 0; i < GameLoop::mNumCameras; ++i)
    {
        sPauseMenuInstance[i].Update(fDeltaTime);
    }
}

// FUNCTION: SWBF2 0x00616b30
bool PauseMenu::Update(float fDeltaTime)
{
    if (mVisible)
    {
        GameSoundEngine::Update(fDeltaTime);
    }

    int iViewport = mViewport;
    GUIInputs* pGuiInputs = FLInputManager::s_instance.GetPlayerGuiInputs(iViewport);
    mDebounceTime -= fDeltaTime;

    ProcessedInputs* pInputs = FLInputManager::s_instance.GetPlayerInputs(iViewport);
    bool bPlayerList = ((pInputs->bits.mBoolInputs >> 17) & 1) != 0;
    bool bStart = pGuiInputs->IsStartPressed();

    SpawnDisplay* pSpawn = SpawnDisplay::Instance(iViewport);
    if (*((bool*)pSpawn + 0x1fea) || !NetGame::IsEnabled())
    {
        bPlayerList = false;
    }

    bool bGameOver = GameLoop::IsGameOver();
    if (bStart && bGameOver && playerJoystickNumbers[11 * iViewport] != gActiveViewport)
    {
        bStart = false;
        bPlayerList = false;
    }
    if (bGameOver)
    {
        iViewport = 0;
    }

    if ((PauseMenu::sPausingViewport == -1 || PauseMenu::sPausingViewport == iViewport) && !gAttractMode && !mVisible &&
        mDebounceTime <= 0.0f && !FLInputManager::s_instance.AnyMissing() && pSpawn->mMode != SpawnDisplay::eFINISHED &&
        (bStart || bPlayerList))
    {
        if (PauseMenu::sEnablePause)
        {
            pGuiInputs->ClearStart();
            PauseMenu::sPausingViewport = iViewport;
            FLInputManager::s_instance.StopAllRumble();
            PauseMenu::sSkipToPlayerList = bPlayerList;
            gSkipToPlayerListState = 0;
            gSkipToPlayerListTime = GameLoop::GetMissionTime();
            _SetPaused(!mVisible);
        }
        else
        {
            PauseMenu::sEnablePause = true;
        }
    }
    else if (mVisible && bStart)
    {
        if (!_stricmp(GuiManager::Instance(iViewport)->GetCurrentFocus(), "ifs_pausemenu"))
        {
            _SetPaused(false);
        }
    }

    if (GameLoop::IsGameOver())
    {
        SpawnDisplay::EnableViewport(iViewport);
        EnableDisplays(iViewport, false);
        EnableScreenGroups(false);
    }

    if (mFreecamView && PauseMenu::IsPauseAllowed() && pGuiInputs->IsSelectPressed())
    {
        if (GameLoop::sPauseMode)
        {
            GameLoop::Resume();
            EnableDisplays(mViewport, false);
            EnableScreenGroups(false);
            _SetPauseAudio(false);
        }
        else
        {
            _SetPauseAudio(true);
            GameLoop::Pause();
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00616e90
bool PauseMenu::AllStatsDone()
{
    if (NetGame::IsEnabled() && NetGame::IsClient() && !PlayerStats::mGotMPStats)
    {
        return true;
    }
    if (NetGame::IsEnabled() && NetGame::IsHost() && NetGame::IsDedicated())
    {
        return true;
    }
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        if (!PauseMenu::sPauseMenuInstance[i].mStatsDone)
        {
            return false;
        }
    }
    return true;
}

namespace PopupText {
enum PopupType { DISABLED = 0, HINT = 1, OBJECTIVE = 2, SELECTION = 3, MAX_POPUP_TYPES = 4 };
extern PopupType gType;
extern bool gMapEnabled[1];
extern bool gSpawnDisplayEnabled[1];
extern int gNumHints;
extern Trigger gButtonTrigger;
bool IsEnabled();
}

namespace ObjectiveManager { int GetCurrentPopupText(); }

// FUNCTION: SWBF2 0x00620d20
void PopupText::DisablePopup()
{
    if (!PopupText::IsEnabled())
    {
        return;
    }

    Snd::Bus* pBus = Snd::Bus::FindByHashID(0xB38617C5);
    if (pBus)
    {
        pBus->Fade(0.5f, 1.0f, -1.0f);
    }
    if (GameLoop::sPauseMode)
    {
        GameLoop::Resume();
    }

    for (unsigned int i = 0; i < GameLoop::mNumCameras; ++i)
    {
        if (PopupText::gMapEnabled[i])
        {
            HUD::Event(HUD::GameEvents::gPlayerEvents[i].mapEnable, true).Send();
        }
        if (PopupText::gSpawnDisplayEnabled[i])
        {
            Character* pPlayer = NetGame::GetLocalPlayer(i);
            SpawnDisplay::Instance(i)->ChangeClass(pPlayer);
        }
    }

    HUD::EventClass* pDisable = 0;
    switch (PopupText::gType)
    {
    case PopupText::HINT: pDisable = HUD::GameEvents::gObjectiveEvents.hintPopupDisable; break;
    case PopupText::OBJECTIVE: pDisable = HUD::GameEvents::gObjectiveEvents.objectivePopupDisable; break;
    case PopupText::SELECTION: pDisable = HUD::GameEvents::gObjectiveEvents.selectionPopupDisable; break;
    default: break;
    }
    if (pDisable)
    {
        HUD::Event(pDisable, true).Send();
    }

    if (GameLoop::mNumCameras == 1)
    {
        HUD::ElementMap* pMap = HUD::ElementMap::FindByPlayerIndex(0);
        if (pMap && pMap->mMode == HUD::ElementMap::mode_Large)
        {
            HUD::Event(HUD::GameEvents::gPlayerEvents[0].objectiveListEnable, true).Send();
            HUD::Event(HUD::GameEvents::gPlayerEvents[0].pressSelectToReturn, true).Send();
            if (ObjectiveManager::GetCurrentPopupText())
            {
                HUD::Event(HUD::GameEvents::gPlayerEvents[0].objectiveDetails, true).Send();
            }
            if (PopupText::gNumHints > 0)
            {
                HUD::Event(HUD::GameEvents::gPlayerEvents[0].hintsAvailable, true).Send();
            }
        }
    }

    PopupText::gType = PopupText::DISABLED;
    PopupText::gButtonTrigger.Clear();
}

// FUNCTION: SWBF2 0x00616e40
void PauseMenu::_Reset()
{
    mVisible = false;
    DeactivateThread();
}

// FUNCTION: SWBF2 0x00616e50
void PauseMenu::Reset()
{
    for (unsigned int i = 0; i < GameLoop::mNumCameras; ++i)
    {
        sPauseMenuInstance[i]._Reset();
    }

    PauseMenu::sPausingViewport = -1;
}

// GLOBAL: SWBF2 0x01eaedbf

bool gPauseReverbEnabled;

// SYNTHETIC: SWBF2 0x00405ea0
// `dynamic initializer for 'gTogglePauseReverbCommand''
static RedCommandConsole gTogglePauseReverbCommand("debug.TogglePauseReverb", MultiVar(&gPauseReverbEnabled));

// GLOBAL: SWBF2 0x007e6320

bool sPauseMenuSoundEnabled = true;
