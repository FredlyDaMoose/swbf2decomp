#include "ShellLoop.h"
#include "CameraManager.h"
#include "DownloadableContent.h"
#include "Entity.h"
#include "FLInputManager.h"
#include "GameLoop.h"
#include "GameMovie.h"
#include "GameState.h"
#include "LoadUtil.h"
#include "LuaHelper.h"
#include "LuaScript.h"
#include "MissionPlayList.h"
#include "NetGame.h"
#include "ProceduralAnimation.h"
#include "FLGuiManager.h"
#include "GameSoundEngine.h"
#include "LuaCallbacks.h"
#include "LuaCallbacks_Sound.h"
#include "ScreenTransition.h"
#include "Thread.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Interface/RedShadowBlob.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Util/RedSceneManager.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "PlayerProfile.h"
#include "GameMemory.h"

#include <windows.h>
#include "NetSetup.h"
#include "NetMatch.h"
#include "CommandLine.h"

// GLOBAL: SWBF2 0x01eaf030

ShellLoop::eLEGALSCREENS_SHOWTYPE ShellLoop::sLegalShowtype;

// GLOBAL: SWBF2 0x01eaf050
LuaScript* ShellLoop::ifScript;
// GLOBAL: SWBF2 0x01eaf05c
int ShellLoop::mLastGameNumCameras;
// GLOBAL: SWBF2 0x01eaf054
float ShellLoop::sDeltaTime;
// GLOBAL: SWBF2 0x01f6dd98
PblTimeBase::Stamp ShellLoop::sMissionTime;
// GLOBAL: SWBF2 0x01eaf038
float ShellLoop::clientInputAcc;
// GLOBAL: SWBF2 0x01eaf040
float ShellLoop::clientRenderAcc;
// GLOBAL: SWBF2 0x01eaf048
unsigned int ShellLoop::sSystemDeltaTime;
// GLOBAL: SWBF2 0x01eaf058
unsigned int ShellLoop::clientTimePrev;

namespace PblBuildType { extern bool gDemoBuild; }
namespace PblBuildType { extern unsigned char gCreateMemoryProfile; }
// GLOBAL: SWBF2 0x009cef73
namespace PblBuildType { bool gDemoHasSplitscreen; }
// GLOBAL: SWBF2 0x009cef74
namespace PblBuildType { bool gDemoHost; }
// GLOBAL: SWBF2 0x009cef77
namespace PblBuildType { bool gDemoNoInputRebind; }
// GLOBAL: SWBF2 0x007e2e0c
namespace PblBuildType { bool gFinalBuild = true; }
// GLOBAL: SWBF2 0x007e2e0d
namespace PblBuildType { bool gXBox_DVDDemo = true; }
// GLOBAL: SWBF2 0x007e2e0e
namespace PblBuildType { bool gE3Demo = true; }
// GLOBAL: SWBF2 0x007e2e0f
namespace PblBuildType { bool gDemoAutoAssign = true; }

namespace GameLoop { extern bool mbHistoricalRules; }
extern bool gAttractMode;
extern bool gDemoMode;

namespace ObjectiveManager { void Init(); void DestroyAll(); }

// FUNCTION: SWBF2 0x00635d50
float ShellLoop::GetMissionTime()
{
    return sMissionTime.GetElapsed();
}

// FUNCTION: SWBF2 0x00635d70
int ShellLoop::Init()
{
    mLastGameNumCameras = GameLoop::mNumCameras;
    NetGame::ClearAllCDKeyInfo();
    EntityEx::InitFind();
    FLInputManager::s_instance.SetHotController(-1);
    GameLoop::mbMetagameRules = false;
    if (FLInputManager::Instance()->GetSecondaryPort() < 0)
    {
        GameLoop::mNumCameras = 1;
    }
    GameLoop::mbHistoricalRules = false;
    GameLoop::SetCanSwitchSides(true);

    CameraManager::Instance()->SetNumCameras(0);
    CameraManager::Instance()->SetNumCameras(GameLoop::mNumCameras);
    CameraManager::Instance()->MapMode(0);
    if (mLastGameNumCameras == 1 && FLInputManager::Instance()->GetPrimaryPort())
    {
        FLInputManager::s_instance.UnbindController(1);
    }

    ObjectiveManager::Init();
    PAManager::Create();
    LoadUtil::ReadDataFile("shell.lvl", 0, 0, 0, 0);

    LuaHelper::InitState();
    lua_setgcthreshold(LuaHelper::mState, 0x400);
    if (PblBuildType::gDemoBuild)
    {
        lua_pushboolean(LuaHelper::mState, true);
        LuaHelper::SetGlobal("gDemoBuild");
        if (PblBuildType::gDemoNoInputRebind)
        {
            lua_pushboolean(LuaHelper::mState, true);
            LuaHelper::SetGlobal("gNoControllerChanges");
        }
        if (PblBuildType::gCreateMemoryProfile)
        {
            lua_pushboolean(LuaHelper::mState, true);
            LuaHelper::SetGlobal("gNoNewProfiles");
        }
        if (PblBuildType::gXBox_DVDDemo)
        {
            lua_pushboolean(LuaHelper::mState, true);
            LuaHelper::SetGlobal("gXBox_DVDDemo");
        }
        if (PblBuildType::gE3Demo)
        {
            lua_pushboolean(LuaHelper::mState, true);
            LuaHelper::SetGlobal("gE3Demo");
        }
        lua_pushboolean(LuaHelper::mState, true);
        LuaHelper::SetGlobal("gDemoHasMP");
        if (PblBuildType::gDemoHasSplitscreen)
        {
            lua_pushboolean(LuaHelper::mState, true);
            LuaHelper::SetGlobal("gDemoHasSplitscreen");
        }
        lua_pushboolean(LuaHelper::mState, PblBuildType::gDemoHost);
        LuaHelper::SetGlobal("gDemoHost");
    }
    if (PblBuildType::gFinalBuild)
    {
        lua_pushboolean(LuaHelper::mState, true);
        LuaHelper::SetGlobal("gFinalBuild");
    }
    lua_pushboolean(LuaHelper::mState, true);
    LuaHelper::SetGlobal("gMovieAttractMode");

    ifScript = new LuaScript;
    ifScript->LoadScript("shell_interface");

    if (sLegalShowtype == eLEGALSCREENS_SHOWFAST)
    {
        LuaHelper::GetGlobal("ifs_legal");
        LuaHelper::TableWriteVal("bEverSkipped", true);
        lua_settop(LuaHelper::mState, -2);
    }
    DownloadableContent::AddMissions(ifScript);
    MissionPlayList::PushToLua();
    LuaScriptFactory::PurgeUnused();
    NetGame::CreateClient();
    NetGame::EnterClient();

    sMissionTime.Mark(&GameState::_SystemTimeBase);
    sDeltaTime = 0.0f;
    ShellLoop::sSystemDeltaTime = 0;
    ShellLoop::clientInputAcc = 0.0f;
    ShellLoop::clientRenderAcc = 0.0f;
    ShellLoop::clientTimePrev = 0;
    if (gAttractMode)
    {
        gDemoMode = false;
        gAttractMode = false;
    }
    return 0;
}

// GLOBAL: SWBF2 0x01eaf00d
bool ShellLoop::mbNetCableErrorOnEnter;
// GLOBAL: SWBF2 0x01eaf034
unsigned int ShellLoop::mbNetCableScreenHash;
// GLOBAL: SWBF2 0x01eaf03c
int ShellLoop::mbNetCableErrorTimer;
// GLOBAL: SWBF2 0x01eaf044
unsigned int ShellLoop::mpNoCableMessageHash;
// GLOBAL: SWBF2 0x01eaf04c
int ShellLoop::sIngameErrorSeverity;

void InitMouseCursor();

// FUNCTION: SWBF2 0x006361f0
int ShellLoop::PostLoad()
{
    RedRenderer::PostLoad();
    InitMouseCursor();
    GameMemory::ReleaseTempHeap();
    GameMovie::Open();
    RedShadowBlob::Create();
    RedSceneManager::SetupStaticWorld();
    CameraManager::Instance()->QuickSetupCameras();

    FLInputManager::s_instance.SetHotController(-1);
    FLInputManager::s_instance.Update(0.0f);
    if (FLInputManager::s_instance.IsControllerBound(0))
    {
        PlayerProfile::GetProfile(0)->ApplySettings();
    }
    FLInputManager::s_instance.RescanForMissing();

    RedRenderer::SetSoftnessEnable(false);
    return 0;
}

// GLOBAL: SWBF2 0x007eb950

__int64 sLastPerformanceCount;
// GLOBAL: SWBF2 0x007eb958

__int64 sPerformanceCountOffset;

namespace SaveGameManager { void Update(float fDeltaTime); }

namespace FLRenderer { void Render(); }

// FUNCTION: SWBF2 0x00636270
static void fnErrorBoxOk(bool)
{
}

// FUNCTION: SWBF2 0x00636280
int ShellLoop::Update(float fDeltaTime)
{
    LARGE_INTEGER liCount;
    QueryPerformanceCounter(&liCount);
    sLastPerformanceCount = liCount.QuadPart;

    Lua_Callbacks::UpdateShellMusic(fDeltaTime);
    GameSoundEngine::Update(fDeltaTime);
    sDeltaTime = fDeltaTime;
    ShellLoop::clientInputAcc += fDeltaTime;
    ShellLoop::clientRenderAcc += fDeltaTime;
    Lua_Callbacks::DoPeriodicLuaGC(false);
    FLInputManager::Instance()->Update(sDeltaTime);
    GuiManager::UpdateAll(sDeltaTime);
    SaveGameManager::Update(sDeltaTime);
    if (ScreenTransition::sInstance)
    {
        ScreenTransition::sInstance->Update(sDeltaTime);
    }
    Thread::UpdateAll(sDeltaTime, false, false);
    Thread::UpdateAll(sDeltaTime, true, false);
    GameMovie::Update(fDeltaTime);
    if (RedShadowBlob::Instance())
    {
        RedShadowBlob::Instance()->Update();
    }
    FLRenderer::Render();

    QueryPerformanceCounter(&liCount);
    sPerformanceCountOffset += liCount.QuadPart - sLastPerformanceCount;

    if (mbNetCableErrorOnEnter)
    {
        const unsigned int uiScreen =
            PblHash(GuiManager::Instance()->mCurrentScope);
        int iTimer;
        if (mbNetCableScreenHash != uiScreen)
        {
            mbNetCableScreenHash = uiScreen;
            iTimer = 0;
        }
        else
        {
            iTimer = ++mbNetCableErrorTimer;
            if (iTimer > 5)
            {
                mbNetCableErrorOnEnter = false;
                GuiManager::Instance()->OpenError(
                    (GuiManager::eErrorSeverity)sIngameErrorSeverity,
                    RedLocalizeDB::Find(mpNoCableMessageHash, false),
                    0, fnErrorBoxOk);
                iTimer = mbNetCableErrorTimer;
            }
        }
        if (uiScreen == 0xDE500916)
        {
            iTimer = 0;
        }
        mbNetCableErrorTimer = iTimer;
    }
    return 1;
}

// FUNCTION: SWBF2 0x006364c0
void ShellLoop::SetNetCableErrorOnEnter(int severity, const char* message)
{
    mbNetCableErrorOnEnter = true;
    mbNetCableErrorTimer = 0;
    sIngameErrorSeverity = severity;
    mpNoCableMessageHash = message ? (unsigned int)PblHash(message) : 0x7d77adfe;
    NetSetup::mNoticeNoCable = false;
    if (severity >= 8)
    {
        NetMatch::CancelLogin(0, false);
    }
    NetMatch::EndRegisterGame();
}

// FUNCTION: SWBF2 0x00636520
bool ShellLoop::GetNetCableErrorOnEnter()
{
    return mbNetCableErrorOnEnter;
}

// FUNCTION: SWBF2 0x00636470
int ShellLoop::Cleanup()
{
    GameMovie::Close();
    delete ShellLoop::ifScript;
    ShellLoop::ifScript = 0;
    gGamespyAutoConnectAddr[0] = 0;
    ShellLoop::mbNetCableErrorOnEnter = false;
    ShellLoop::mbNetCableErrorTimer = 0;
    ObjectiveManager::DestroyAll();
    RedShadowBlob::Destroy();
    PAManager::Destroy();
    NetGame::DestroyClient();
    return 0;
}

// GLOBAL: SWBF2 0x01eaf00e
bool ShellLoop::mDemoIsOver = false;
