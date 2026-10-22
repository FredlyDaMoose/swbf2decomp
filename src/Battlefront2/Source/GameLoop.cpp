#include "PerformanceTimer.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "GameLoop.h"
#include "BootDisplay.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "BoundaryManager.h"
#include "CameraManager.h"
#include "CaptureDisplay.h"
#include "ChangeClassDisplay.h"
#include "ChatMessageDisplay.h"
#include "EndDisplay.h"
#include "Entity.h"
#include "EntityDoor.h"
#include "EnergyBar.h"
#include "EntitySound.h"
#include "EntityTerrain.h"
#include "FirstPerson.h"
#include "FishEffect.h"
#include "FlagItem.h"
#include "FLGuiManager.h"
#include "FLInputManager.h"
#include "GameMemory.h"
#include "GameSound.h"
#include "GameState.h"
#include "HeroMessageDisplay.h"
#include "HUDEventQueue.h"
#include "HUDEditor.h"
#include "HUDElement.h"
#include "HUDManager.h"
#include "LockOnManager.h"
#include "MissionPlayList.h"
#include "BonusManager.h"
#include "SoldierAnimatorLowRes.h"
#include "VehicleSpawn.h"
#include "ItemTimerDisplay.h"
#include "LoadDisplay.h"
#include "LuaHelper.h"
#include "MessageDisplay.h"
#include "NameDisplay.h"
#include "NetGame.h"
#include "OrdnanceGrapplingHook.h"
#include "OrdnanceTowCable.h"
#include "PauseMenu.h"
#include "NetSetup.h"
#include "PlayerProfile.h"
#include "QOSDisplay.h"
#include "ReticuleDisplay.h"
#include "ScopeDisplay.h"
#include "ShellLoop.h"
#include "SoldierAnimationBank.h"
#include "SoldierAnimatorClass.h"
#include "SoundDebugDisplay.h"
#include "SpaceAssault.h"
#include "SpawnManager.h"
#include "SpectatorDisplay.h"
#include "StatIconDisplay.h"
#include "StatusDisplay.h"
#include "TargetManager.h"
#include "Team.h"
#include "NetComm.h"
#include "ToolTips.h"
#include "ProceduralAnimation.h"
#include "VoiceOverManager.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/input/RedKeyboard.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "PebbleFL/Common/PblTimeBase.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Util/RedSceneManager.h"
#include "SoundFL/source/sndbus.h"
#include "SoundFL/source/sndsoundstream.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "AIDifficulty.h"
#include "AIUtil.h"
#include "Character.h"
#include "CommandFlyer.h"
#include "CommandLine.h"
#include "ControllerManager.h"
#include "FlashyTextElement.h"
#include "GameMusic.h"
#include "GameSoundEngine.h"
#include "LuaCallbacks.h"
#include "MedalsMgr.h"
#include "MotionBlurEffect.h"
#include "Music.h"
#include "ScreenTransition.h"
#include "Thread.h"

#include <stdio.h>
#include <time.h>
#include <windows.h>
#include "PauseMenu.h"
#include "PlayerStats.h"

class VideoEffect { public: static void Init(); };
class LetterboxEffect { public: static void Init(); };
class BirdParticleManager { public: static void PlatformInit(); };

class WeaponMeleeClass { public: static void PostLoad(); };
class WeaponMelee { public: static void PostLoad() { WeaponMeleeClass::PostLoad(); } };

class TeamAssistManager { public: static void Create(); static void Destroy(); };
namespace MissionTimer { void Init(); }
namespace ObjectiveManager { void Init(); void DestroyAll(); }
namespace EventManager { void Init(); void Cleanup(); }

namespace Lua_Callbacks { void Cleanup(); }
namespace RedMissionLog { bool Enabled(); }

class EntityWater
{
public:
    EntityWater();

    unsigned char mOpaqueWaterState[0xa4];
};

class EntitySoldier
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class EntityDroid
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class MountedTurret
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class PassengerSlot
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class PowerupItem
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class EntityMine
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class EntityPortableTurret
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};

class Weapon
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static void UpdateMemoryPoolSize(unsigned int uSize)
    {
        if (sMemoryPool.GetSize() < uSize)
        {
            sMemoryPool.SetSize(uSize);
        }
    }
    static MemoryPool sMemoryPool;
};
class AmmoCounter
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class StickInfo
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};

class LightFlash
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class EntityLight
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class LightningBoltEffectObject
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class ParticleEmitterObject
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class ParticleEmitterInfoData
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class ParticleEmitter
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
namespace ParticleTransformer
{
class PositionTransformer
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class SizeTransformer
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class RotationTransformer
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class ColorTransformer
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
}
class EntitySoundStatic
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class EntitySoundStream
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class SoundSpaceRegion
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};
class PlayerController
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
    static int sEnabled;
};
class TeamAssist
{
public:
    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }
    static MemoryPool sMemoryPool;
};

namespace AIDebug { void Create(); }
namespace TeamManager { void Init(); }
namespace SkyManager { void Activate(); }

void InitMouseCursor();
namespace PopupText { void Init(); }
extern float netSecondsPerTurn;
extern int netCurMaxPlayers;
extern HWND hGlobalWnd;

// FUNCTION: SWBF2 0x005337a0
void UpdateDedicatedNoRender()
{
    HDC hDC = GetDC(hGlobalWnd);
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = RedInterfaceScreen::s_screenFull.m_uiWidthInPixels;
    rect.bottom = RedInterfaceScreen::s_screenFull.m_uiHeightInPixels;
    char acText[1024] = "";
    FillRect(hDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
    HGDIOBJ hOldFont = SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));

    char acLine[512];
    _snprintf(acLine, sizeof(acLine) - 1, "Game: %s\n", NetSetup::GetHostName());
    acLine[sizeof(acLine) - 1] = 0;
    strlcat(acText, acLine, sizeof(acText));
    _snprintf(acLine, sizeof(acLine) - 1, "Mission: %s\n", GameLoop::mMissionScript);
    acLine[sizeof(acLine) - 1] = 0;
    strlcat(acText, acLine, sizeof(acText));

    const unsigned short* pTeam1 = Team::sTeams[1] ? Team::sTeams[1]->GetName() : 0;
    const unsigned short* pTeam2 = Team::sTeams[2] ? Team::sTeams[2]->GetName() : 0;
    if (pTeam1 && pTeam2)
    {
        char acTeam1[128];
        char acTeam2[128];
        PblUnicode::ustrncnv(acTeam1, pTeam1, sizeof(acTeam1));
        PblUnicode::ustrncnv(acTeam2, pTeam2, sizeof(acTeam2));
        _snprintf(acLine, sizeof(acLine) - 1, "Teams: %s vs %s\n", acTeam1, acTeam2);
        acLine[sizeof(acLine) - 1] = 0;
        strlcat(acText, acLine, sizeof(acText));
    }
    if (Team::sTeams[1] && Team::sTeams[2])
    {
        int iCount1, iCount2, iLimit1, iLimit2;
        bool bLimits, bPoints;
        NetGame::GetScore(&iCount1, &iCount2, &iLimit1, &iLimit2, &bLimits, &bPoints);
        _snprintf(acLine, sizeof(acLine) - 1, "Score: %d to %d\n", iCount1, iCount2);
        acLine[sizeof(acLine) - 1] = 0;
        strlcat(acText, acLine, sizeof(acText));
    }

    const int iSeconds = (int)((float)NetGame::GetTurnNumber() * netSecondsPerTurn);
    _snprintf(acLine, sizeof(acLine) - 1, "Time: %dm%ds\n", iSeconds / 60, iSeconds % 60);
    acLine[sizeof(acLine) - 1] = 0;
    strlcat(acText, acLine, sizeof(acText));

    _snprintf(acLine, sizeof(acLine) - 1, "Players: { ");
    acLine[sizeof(acLine) - 1] = 0;
    strlcat(acText, acLine, sizeof(acText));
    int iPlayers = 0;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (!NetComm::IsConnected(i))
        {
            continue;
        }
        ++iPlayers;
        _snprintf(acLine, sizeof(acLine) - 1, "'%s' ", NetGame::GetPlayerName(i));
        acLine[sizeof(acLine) - 1] = 0;
        strlcat(acText, acLine, sizeof(acText));
    }
    _snprintf(acLine, sizeof(acLine) - 1, "} (%d)\n", iPlayers);
    acLine[sizeof(acLine) - 1] = 0;
    strlcat(acText, acLine, sizeof(acText));

    DrawTextA(hDC, acText, (int)strlen(acText), &rect, DT_WORDBREAK);
    SelectObject(hDC, hOldFont);
    ReleaseDC(hGlobalWnd, hDC);
}

extern bool gAttractMode;

namespace FLRenderer
{
void AddInterfaceElement(unsigned int hView, RedScreenGroupElement* pElement, float fX, float fY,
                         bool bFlag);
void AddInterfaceElement(RedScreenGroupElement* pElement, float fX, float fY, bool bFlag);

void RemoveInterfaceElement(unsigned int uiView, RedScreenGroupElement* pGroup);
void RemoveInterfaceElement(RedScreenGroupElement* pGroup);
}

extern bool netHeroUnlocked[3];
extern bool netHeroFirstUnlocked[3];

// GLOBAL: SWBF2 0x007e8ffc
int PlayerController::sEnabled = -1;

namespace GameLoop
{

char mMissionScript[64];
unsigned int mMissionScriptHash;
bool sPauseMode;

// GLOBAL: SWBF2 0x01e56132
bool gInvincible;

// GLOBAL: SWBF2 0x01e56060
char mWorldFileName[64];
// GLOBAL: SWBF2 0x01e560a0
char mTerrainFileName[64];
// GLOBAL: SWBF2 0x01e5600a
bool sGameOver;

bool _bFirstPersonEnabled = true;

unsigned int mNumCameras = 1;

// GLOBAL: SWBF2 0x01ec1000
PblTimeBase::Stamp sMissionTime;

// GLOBAL: SWBF2 0x01e56040
float DedicatedMsgX;
// GLOBAL: SWBF2 0x01e5602c
float DedicatedMsgY;

float DedicatedMsgDX = 8.0f;
float DedicatedMsgDY = 8.0f;

// GLOBAL: SWBF2 0x01e56030
bool mShowTeamPoints[2];

// GLOBAL: SWBF2 0x01e56033
bool mbMetagameRules;

// GLOBAL: SWBF2 0x01ec0ea0
// SYNTHETIC: SWBF2 0x00403c70
// `dynamic initializer for 'GameLoop::mAttractGroup''
// SYNTHETIC: SWBF2 0x007689f0
// `dynamic atexit destructor for 'GameLoop::mAttractGroup''
RedScreenGroupElement mAttractGroup;
// GLOBAL: SWBF2 0x01ec10b0
// SYNTHETIC: SWBF2 0x00403c90
// `dynamic initializer for 'GameLoop::mAttractInfoText''
// SYNTHETIC: SWBF2 0x00768a00
// `dynamic atexit destructor for 'GameLoop::mAttractInfoText''
RedTextElement mAttractInfoText;
// GLOBAL: SWBF2 0x01ec0d40
// SYNTHETIC: SWBF2 0x00403cb0
// `dynamic initializer for 'GameLoop::mDedicatedGroup''
// SYNTHETIC: SWBF2 0x00768a10
// `dynamic atexit destructor for 'GameLoop::mDedicatedGroup''
RedScreenGroupElement mDedicatedGroup;
// GLOBAL: SWBF2 0x01ec0de0
// SYNTHETIC: SWBF2 0x00403cd0
// `dynamic initializer for 'GameLoop::mDedicatedTitleText''
// SYNTHETIC: SWBF2 0x00768a20
// `dynamic atexit destructor for 'GameLoop::mDedicatedTitleText''
RedTextElement mDedicatedTitleText;
// GLOBAL: SWBF2 0x01ec0c80
// SYNTHETIC: SWBF2 0x00403cf0
// `dynamic initializer for 'GameLoop::mDedicatedTeam1Text''
// SYNTHETIC: SWBF2 0x00768a30
// `dynamic atexit destructor for 'GameLoop::mDedicatedTeam1Text''
RedTextElement mDedicatedTeam1Text;
// GLOBAL: SWBF2 0x01ec0f40
// SYNTHETIC: SWBF2 0x00403d10
// `dynamic initializer for 'GameLoop::mDedicatedTeam2Text''
// SYNTHETIC: SWBF2 0x00768a40
// `dynamic atexit destructor for 'GameLoop::mDedicatedTeam2Text''
RedTextElement mDedicatedTeam2Text;
// GLOBAL: SWBF2 0x01ec1010
// SYNTHETIC: SWBF2 0x00403d30
// `dynamic initializer for 'GameLoop::mWaitMessageGroup''
// SYNTHETIC: SWBF2 0x00768a50
// `dynamic atexit destructor for 'GameLoop::mWaitMessageGroup''
RedScreenGroupElement mWaitMessageGroup;
// GLOBAL: SWBF2 0x01ec1170
// SYNTHETIC: SWBF2 0x00403d50
// `dynamic initializer for 'GameLoop::mWaitTextElement''
// SYNTHETIC: SWBF2 0x00768a60
// `dynamic atexit destructor for 'GameLoop::mWaitTextElement''
RedTextElement mWaitTextElement;

// GLOBAL: SWBF2 0x01e56050
float sSystemDeltaTime;
// GLOBAL: SWBF2 0x01e56038
float clientInputAcc;
// GLOBAL: SWBF2 0x01e56048
float clientTimePrev;
// GLOBAL: SWBF2 0x01e5603c
float mWaitTimer;

}

// GLOBAL: SWBF2 0x01e56130

bool sMissionTimeInitialized;
// GLOBAL: SWBF2 0x01e56134

bool sInitCompleteFlag;

// GLOBAL: SWBF2 0x01e56135

bool sLoadLowDetailModels;

// GLOBAL: SWBF2 0x01e56133
namespace GameLoop { bool gUnlimitedAmmo; }
// GLOBAL: SWBF2 0x01e57722

bool sMissionActive;
// GLOBAL: SWBF2 0x01e5605c
namespace GameLoop { bool sShowStats; }

// GLOBAL: SWBF2 0x01e5605d
namespace GameLoop { bool mbHistoricalRules; }
// GLOBAL: SWBF2 0x01e56009
namespace GameLoop { bool sQuitRequested; }
// GLOBAL: SWBF2 0x01e56124
namespace GameLoop { unsigned int sQuitTurn; }
// GLOBAL: SWBF2 0x01e56128

unsigned int sMissionTurnCounter;
// GLOBAL: SWBF2 0x01e56024
namespace GameLoop { float sGameOverDelayTimer; }
// GLOBAL: SWBF2 0x01e56028

unsigned int sPendingClientTurns;
// GLOBAL: SWBF2 0x01e56034

unsigned int sPendingHostTurns;

// GLOBAL: SWBF2 0x01e56120
namespace GameLoop { float mAttractModeTime; }

class EndTimer
{
public:
    void Start(float fTime) { timer = fTime; }
    void Update(float fDeltaTime) { timer -= fDeltaTime; }
    bool IsFinished() { return timer <= 0.0f; }

    float timer;
};
 

// GLOBAL: SWBF2 0x01e5604c
EndTimer endTimer;

// GLOBAL: SWBF2 0x01e5612c
float gPrevFrameTime;

namespace GameLoop { float sDeltaTime; }
namespace GameLoop { extern float sClientDeltaTime; }

void EntityEx::InitFind()
{
    mIdMap.Clear();
    mIdMapNumEntries = 0;
}

void TreeGridStack::SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow)
{
    sMemoryPool.Setup(uCount, uSize, uGrow);
}

void SoldierAnimatorClass::PostLoad()
{
    if (sInstance)
    {
        sInstance->_PostLoad();
    }
}

void CaptureDisplay::InitAll()
{
    sInstance[0]->Init();
    for (unsigned int i = 0; i < CameraManager::Instance()->GetNumCameras(); ++i)
    {
        sInstance[i]->Bind(i);
    }
}

void BootDisplay::InitAll()
{
    sInstance[0]->Init();
    for (unsigned int i = 0; i < CameraManager::Instance()->GetNumCameras(); ++i)
    {
        sInstance[i]->Bind(i);
    }
}

void SpectatorDisplay::InitAll()
{
    sInstance[0]->Init();
    for (unsigned int i = 0; i < CameraManager::Instance()->GetNumCameras(); ++i)
    {
        sInstance[i]->Bind(i);
    }
}

void ReticuleDisplay::BindAll()
{
    for (unsigned int i = 0; i < CameraManager::Instance()->GetNumCameras(); ++i)
    {
        sInstance[i]->Bind(i);
    }
}

void HeroMessageDisplay::InitAll()
{
    sInstance[0]->Init();
    for (unsigned int i = 0; i < CameraManager::Instance()->GetNumCameras(); ++i)
    {
        sInstance[i]->Bind(i);
    }
}

void ScopeDisplay::InitAll()
{
    sInstance[0]->Init();
    for (unsigned int i = 0; i < CameraManager::Instance()->GetNumCameras(); ++i)
    {
        sInstance[i]->Bind(i);
    }
}

void StatusDisplay::InitAll()
{
    sInstance[0]->Init();
    for (unsigned int i = 0; i < CameraManager::Instance()->GetNumCameras(); ++i)
    {
        sInstance[i]->Bind(i);
    }
}

namespace GameLoop { void CheckScreenShots(); }
namespace GameLoop { bool CheckToggleDisplay(); }
namespace TeamManager { void UpdatePositions(); }
namespace HUD { namespace Manager { void Update(float fDeltaTime); } }
namespace PlayerPrefs { int __fastcall FindByViewport(int iValue); }
namespace PopupText { bool IsEnabled(); void Update(float fDeltaTime); }
namespace HUD { namespace GameEvents { void Update(float fDeltaTime); } }
namespace SaveGameManager { void Update(float fDeltaTime); }
namespace GameLoop { void UpdateStats(float fDeltaTime, float fSystemDeltaTime); }

extern int g_InGameMoviePlayerState;
extern __int64 sLastPerformanceCount;
extern __int64 sPerformanceCountOffset;
extern double secondsPerPerformanceTick;
extern FlashyTextManager sFlashyTextManagerInstance;

extern float netSecondsPerTurn;
extern bool netFrameLock;
extern bool netIsClient;
extern int netMinPlayersToAutoLaunchDedicated;

// GLOBAL: SWBF2 0x0079f808

static const char sSideNamePrefixes[][4] = { "Neu", "All", "Imp", "Rep", "CIS", "Loc" };

static const float END_DISPLAY_TIME = 5.0f;

namespace GameLoop
{

// FUNCTION: SWBF2 0x00530ea0
float GetMissionTime()
{
    if (!sMissionTimeInitialized)
    {
        return 0.0f;
    }
    return sMissionTime.GetElapsed();
}

// FUNCTION: SWBF2 0x00530ed0
bool IsGameOver()
{
    return sGameOver;
}

// FUNCTION: SWBF2 0x005334d0
void Pause()
{
    sPauseMode = true;
}

// FUNCTION: SWBF2 0x005334e0
void Resume()
{
    sPauseMode = false;
}

}

// FUNCTION: SWBF2 0x005c4600
bool NetGame::ErrorDismissed()
{
    return NetSetup::IsErrorDismissed();
}

namespace GameLoop
{

// FUNCTION: SWBF2 0x00530ee0
int Init()
{
    sMissionTimeInitialized = false;
    for (int i = 0; i < 3; ++i)
    {
        netHeroFirstUnlocked[i] = false;
        netHeroUnlocked[i] = false;
    }
    SetLastGameWinner(-1);
    sInitCompleteFlag = true;
    sMissionActive = false;

    SetWorldFilename(0);
    SetTerrainFilename(0);

    EventManager::Init();
    Team::CreateAll();
    CameraManager::Instance()->SetNumCameras(0);
    CameraManager::Instance()->SetNumCameras(mNumCameras);

    SpawnManager::Create();
    SpawnDisplay::Create();
    PAManager::Create();
    MissionTimer::Init();
    SpaceAssault::Create();

    NetGame::EnterClient();
    MissionTimer::Init();
    VideoEffect::Init();
    LetterboxEffect::Init();
    HUD::EventQueue::Create();
    TargetManager::Init();
    ObjectiveManager::Init();
    ReticuleDisplay::Create();
    HeroMessageDisplay::Create();
    ChangeClassDisplay::Create();
    StatIconDisplay::Create();
    StatusDisplay::Create();
    MessageDisplay::Create();
    BootDisplay::Create();
    ChatMessageDisplay::Create();
    SpectatorDisplay::Create();
    ToolTipsDisplay::Create();
    CaptureDisplay::Create();
    FlagItem::Create();
    ScopeDisplay::Create();
    EndDisplay::Create();
    NameDisplay::Create();
    ItemTimerDisplay::Create();
    QOSDisplay::Create();
    VoiceOverManager::Create();
    BoundaryManager::Create();
    new SoundDebugDisplay;
    AIDebug::Create();
    NetGame::LeaveClient();

    EntitySoldier::SetupMemoryPool(600, EntitySoldier::sMemoryPool.GetSize(), EntitySoldier::sMemoryPool.GetGrow());
    EntityDroid::SetupMemoryPool(16, EntityDroid::sMemoryPool.GetSize(), EntityDroid::sMemoryPool.GetGrow());
    MountedTurret::SetupMemoryPool(50, MountedTurret::sMemoryPool.GetSize(), MountedTurret::sMemoryPool.GetGrow());
    PassengerSlot::SetupMemoryPool(0, PassengerSlot::sMemoryPool.GetSize(), PassengerSlot::sMemoryPool.GetGrow());
    PowerupItem::SetupMemoryPool(25, PowerupItem::sMemoryPool.GetSize(), PowerupItem::sMemoryPool.GetGrow());
    EntityMine::SetupMemoryPool(40, EntityMine::sMemoryPool.GetSize(), EntityMine::sMemoryPool.GetGrow());
    EntityPortableTurret::SetupMemoryPool(6, EntityPortableTurret::sMemoryPool.GetSize(), EntityPortableTurret::sMemoryPool.GetGrow());
    CommandFlyer::SetupMemoryPool(0, CommandFlyer::sMemoryPool.GetSize(), CommandFlyer::sMemoryPool.GetGrow());
    Aimer::SetupMemoryPool(100, Aimer::sMemoryPool.GetSize(), Aimer::sMemoryPool.GetGrow());
    TreeGridStack::SetupMemoryPool(550, TreeGridStack::sMemoryPool.GetSize(), TreeGridStack::sMemoryPool.GetGrow());
    Weapon::SetupMemoryPool(2400, Weapon::sMemoryPool.GetSize(), Weapon::sMemoryPool.GetGrow());
    AmmoCounter::SetupMemoryPool(2400, AmmoCounter::sMemoryPool.GetSize(), AmmoCounter::sMemoryPool.GetGrow());
    EnergyBar::SetupMemoryPool(2400, EnergyBar::sMemoryPool.GetSize(), EnergyBar::sMemoryPool.GetGrow());

    Weapon::UpdateMemoryPoolSize(0x150);
    Weapon::UpdateMemoryPoolSize(0x110);
    Weapon::UpdateMemoryPoolSize(0x160);
    Weapon::UpdateMemoryPoolSize(0x120);
    Weapon::UpdateMemoryPoolSize(0x120);
    Weapon::UpdateMemoryPoolSize(0x120);
    Weapon::UpdateMemoryPoolSize(0x120);
    Weapon::UpdateMemoryPoolSize(0x170);
    Weapon::UpdateMemoryPoolSize(0x120);
    Weapon::UpdateMemoryPoolSize(0x130);
    Weapon::UpdateMemoryPoolSize(0x120);
    Weapon::UpdateMemoryPoolSize(0x180);
    Weapon::UpdateMemoryPoolSize(0x1c0);
    Weapon::UpdateMemoryPoolSize(0x160);
    Weapon::UpdateMemoryPoolSize(0x130);
    Weapon::UpdateMemoryPoolSize(0x120);
    Weapon::UpdateMemoryPoolSize(0x1a0);
    Weapon::UpdateMemoryPoolSize(0x170);

    StickInfo::SetupMemoryPool(60, StickInfo::sMemoryPool.GetSize(), StickInfo::sMemoryPool.GetGrow());
    Ordnance::SetupMemoryPool(120, Ordnance::sMemoryPool.GetSize(), Ordnance::sMemoryPool.GetGrow());

    Ordnance::UpdateMemoryPoolSize(324);
    Ordnance::UpdateMemoryPoolSize(300);
    Ordnance::UpdateMemoryPoolSize(308);
    Ordnance::UpdateMemoryPoolSize(356);
    Ordnance::UpdateMemoryPoolSize(316);
    Ordnance::UpdateMemoryPoolSize(320);
    Ordnance::UpdateMemoryPoolSize(328);
    Ordnance::UpdateMemoryPoolSize(384);
    Ordnance::UpdateMemoryPoolSize(360);
    Ordnance::UpdateMemoryPoolSize(304);
    Ordnance::UpdateMemoryPoolSize(320);

    LightFlash::SetupMemoryPool(16, LightFlash::sMemoryPool.GetSize(), LightFlash::sMemoryPool.GetGrow());
    EntityLight::SetupMemoryPool(100, EntityLight::sMemoryPool.GetSize(), EntityLight::sMemoryPool.GetGrow());
    LightningBoltEffectObject::SetupMemoryPool(16, LightningBoltEffectObject::sMemoryPool.GetSize(), LightningBoltEffectObject::sMemoryPool.GetGrow());
    ParticleEmitterObject::SetupMemoryPool(192, ParticleEmitterObject::sMemoryPool.GetSize(), ParticleEmitterObject::sMemoryPool.GetGrow());
    ParticleEmitterInfoData::SetupMemoryPool(1024, ParticleEmitterInfoData::sMemoryPool.GetSize(), ParticleEmitterInfoData::sMemoryPool.GetGrow());
    ParticleEmitter::SetupMemoryPool(1024, ParticleEmitter::sMemoryPool.GetSize(), ParticleEmitter::sMemoryPool.GetGrow());
    ParticleTransformer::PositionTransformer::SetupMemoryPool(1024, ParticleTransformer::PositionTransformer::sMemoryPool.GetSize(), ParticleTransformer::PositionTransformer::sMemoryPool.GetGrow());
    ParticleTransformer::SizeTransformer::SetupMemoryPool(1024, ParticleTransformer::SizeTransformer::sMemoryPool.GetSize(), ParticleTransformer::SizeTransformer::sMemoryPool.GetGrow());
    ParticleTransformer::RotationTransformer::SetupMemoryPool(0, ParticleTransformer::RotationTransformer::sMemoryPool.GetSize(), ParticleTransformer::RotationTransformer::sMemoryPool.GetGrow());
    ParticleTransformer::ColorTransformer::SetupMemoryPool(1536, ParticleTransformer::ColorTransformer::sMemoryPool.GetSize(), ParticleTransformer::ColorTransformer::sMemoryPool.GetGrow());
    EntitySoundStatic::SetupMemoryPool(112, EntitySoundStatic::sMemoryPool.GetSize(), EntitySoundStatic::sMemoryPool.GetGrow());
    EntitySoundStream::SetupMemoryPool(30, EntitySoundStream::sMemoryPool.GetSize(), EntitySoundStream::sMemoryPool.GetGrow());
    SoundSpaceRegion::SetupMemoryPool(30, SoundSpaceRegion::sMemoryPool.GetSize(), SoundSpaceRegion::sMemoryPool.GetGrow());
    PlayerController::SetupMemoryPool(64, PlayerController::sMemoryPool.GetSize(), PlayerController::sMemoryPool.GetGrow());
    TeamAssist::SetupMemoryPool(64, TeamAssist::sMemoryPool.GetSize(), TeamAssist::sMemoryPool.GetGrow());

    EntityEx::InitFind();
    TeamManager::Init();
    SoldierAnimationBank::Reset();
    EntityDoor::ResetDoorList();

    if (Snd::Bus* pBus = Snd::Bus::FindByHashID(0x985c8f54))
    {
        pBus->Fade(0.0f, 0.0f, -1.0f);
    }

    LuaHelper::InitState();
    LuaHelper::CallGlobalProc("ScriptInit");
    SpawnManager::Instance()->SetupSoldierMemoryPool();
    RedShadowProxy::CleanUp();
    RedTerrain::Activate();
    GuiManager::InitAll();
    new EntityTerrain;
    new EntityWater;
    GameSound::SetupDeferred();
    LoadDisplay::Instance()->End();
    WeaponMelee::PostLoad();
    SoldierAnimatorClass::PostLoad();
    FirstPerson::InitPlatform();
    GameMemory::ReleaseTempHeap();
    NetGame::CreateClient();

    bool bOneWorldHost = NetGame::IsEnabled() && NetGame::IsHost() && NetGame::IsOneWorld();
    if (!bOneWorldHost)
    {
        NetGame::EnterClient();
    }

    CaptureDisplay::InitAll();
    BootDisplay::InitAll();
    ChatMessageDisplay::Instance()->Init();
    SpectatorDisplay::InitAll();
    ToolTipsDisplay::sInstance->Init();
    ReticuleDisplay::BindAll();
    HeroMessageDisplay::InitAll();
    ScopeDisplay::InitAll();
    StatusDisplay::InitAll();
    EndDisplay::sInstance->Init();
    NameDisplay::Init();
    ItemTimerDisplay::Init();
    QOSDisplay::InitClient();
    BirdParticleManager::PlatformInit();
    FishParticleManager::PlatformInit();
    OrdnanceTowCable::PlatformInit();
    OrdnanceGrapplingHook::PlatformInit();
    SoundDebugDisplay::sInstance->Init();
    ChatMessageDisplay::Instance()->Show();
    SkyManager::Activate();

    if (!bOneWorldHost)
    {
        NetGame::LeaveClient();
    }

    QOSDisplay::InitHost();

    sMissionTime.Mark(&GameState::_TimeBase);
    sMissionTimeInitialized = true;
    GameLoop::sDeltaTime = 0.0f;
    sSystemDeltaTime = 0.0f;
    GameLoop::sClientDeltaTime = 0.0f;
    clientInputAcc = 0.0f;
    clientTimePrev = 0.0f;
    GameLoop::sGameOverDelayTimer = 5.0f;
    sGameOver = false;
    GameLoop::sShowStats = false;
    GameLoop::sQuitRequested = false;
    GameLoop::sQuitTurn = 0;
    sMissionTurnCounter = 0;

    EntitySound::PlayAll();

    FLInputManager::s_instance.SetHotController(-1);
    FLInputManager::s_instance.Update(0.0f);
    if (FLInputManager::s_instance.IsControllerBound(0))
    {
        FLInputManager::s_instance.GetPlayerJoystick(0)->SetInputMode(0);
        PlayerProfile::GetProfile(0)->ApplySettings();
    }
    FLInputManager::s_instance.RescanForMissing();

    endTimer.Start(0.0f);
    return 0;
}

// FUNCTION: SWBF2 0x00531a70
int PostLoad()
{
    RedRenderer::PostLoad();
    InitMouseCursor();
    NetGame::CreateLast();
    FirstPerson::PostLoad();

    if (!NetGame::IsEnabled() || NetGame::IsHost() || !NetGame::IsOneWorld())
    {
        BonusManager::sInstance.InitialUpdate();
    }

    MemoryPool::CreateAll();

    if (!NetGame::IsEnabled() || NetGame::IsHost() || !NetGame::IsOneWorld())
    {
        Team::InitAll();
        SpawnManager::Instance()->InitialUpdate();
        Team::InitialUpdateAll();
    }
    if (NetGame::IsEnabled() && (!NetGame::IsHost() || !NetGame::IsOneWorld()))
    {
        NetGame::EnterClient();
        Team::InitAll();
        Team::CopyFromHost();
        SpawnManager::Instance()->InitialUpdate();
        Team::InitialUpdateAll();
        NetGame::LeaveClient();
    }

    for (PblList<VehicleSpawn>::Itor it(VehicleSpawn::GetVehicleSpawnList()); (VehicleSpawn*)it; ++it)
    {
        it->Update(0.0f);
    }

    NetGame::EnterClient();
    LockOnManager::CreateAll();
    if (!NetGame::IsEnabled() || !NetGame::IsDedicated())
    {
        const unsigned int uNumCameras = CameraManager::Instance()->GetNumCameras();
        for (unsigned int i = 0; i < uNumCameras; ++i)
        {
            if (i < 1 && playerJoystickNumbers[i * 11] >= 0)
            {
                PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(playerJoystickNumbers[i * 11]);
                LockOnManager* pLockOn = LockOnManager::sLockOnManagers[i];
                if (pProfile && pLockOn)
                {
                    pLockOn->SetDisplayIconsOnEverything(
                        NetGame::IsEnabled() ? false : pProfile->mGameOptions.mbSoldierHints);
                }
            }
        }
    }
    SpawnDisplay::Init();
    PopupText::Init();
    TeamAssistManager::Create();
    NetGame::LeaveClient();

    sPendingHostTurns = 0;
    sPendingClientTurns = 0;
    if (gAttractMode)
    {
        GameLoop::mAttractModeTime = 115.0f;
        mAttractInfoText.SetFont(RedFont::GAMEFONT_LARGE);
        mAttractInfoText.SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_CENTER);
        mAttractInfoText.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
        if (const unsigned short* pText = RedLocalizeDB::Find(0x74eca16f, false))
        {
            mAttractInfoText.SetText(pText, true);
        }
        mAttractGroup.AddElement(&mAttractInfoText);
        mAttractGroup.Visible(true);
        for (unsigned int i = 0; i < CameraManager::Instance()->GetNumCameras(); ++i)
        {
            FLRenderer::AddInterfaceElement(CameraManager::Instance()->m_hView[i], &mAttractGroup, RedInterfaceScreen::CENTER, RedInterfaceScreen::CENTER,
                                            true);
        }
    }
    else
    {
        CameraManager::Instance()->SetChaseCameraShot(0, 0);
        CameraManager::Instance()->QuickSetupCameras();
    }

    if (NetGame::IsDedicated())
    {
        mDedicatedTitleText.SetName("DedicatedTitleText");
        mDedicatedTitleText.SetFont(RedFont::GAMEFONT_MEDIUM);
        mDedicatedTitleText.SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_TOP);
        mDedicatedTitleText.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
        mDedicatedTeam1Text.SetFont(RedFont::GAMEFONT_SMALL);
        mDedicatedTeam1Text.SetTextAlign(RedTextElement::TEXT_HALIGN_LEFT, RedTextElement::TEXT_VALIGN_BOTTOM);
        mDedicatedTeam1Text.SetColor(RedColor::GREEN);
        mDedicatedTeam2Text.SetFont(RedFont::GAMEFONT_SMALL);
        mDedicatedTeam2Text.SetTextAlign(RedTextElement::TEXT_HALIGN_RIGHT, RedTextElement::TEXT_VALIGN_BOTTOM);
        mDedicatedTeam2Text.SetColor(RedColor::RED);
        mDedicatedGroup.Visible(false);
        mDedicatedTitleText.SetText("", true);
        mDedicatedTeam1Text.SetText("", true);
        mDedicatedTeam2Text.SetText("", true);
        if (const unsigned short* pTitle = RedLocalizeDB::Find(0xd67ed788, false))
        {
            mDedicatedTitleText.SetText(pTitle, true);
            unsigned int uiTeam1Hash;
            unsigned int uiTeam2Hash;
            MissionPlayList::GetCurrentTeamNames(uiTeam1Hash, uiTeam2Hash);
            const unsigned short* pTeam1 = RedLocalizeDB::Find(uiTeam1Hash, true);
            const unsigned short* pTeam2 = RedLocalizeDB::Find(uiTeam2Hash, true);
            if (pTeam1 && pTeam2)
            {
                mDedicatedTeam1Text.SetText(pTeam1, true);
                mDedicatedTeam2Text.SetText(pTeam2, true);
            }
        }
        const float fScale = (float)RedInterfaceScreen::GetScreenWidth() / 800.0f;
        mDedicatedTitleText.SetPosition(0.0f, fScale * -180.0f);
        const float fTeamY = fScale * 178.0f;
        mDedicatedTeam1Text.SetPosition(-100.0f, fTeamY);
        mDedicatedTeam2Text.SetPosition(100.0f, fTeamY);
        mDedicatedGroup.AddElement(&mDedicatedTitleText);
        mDedicatedGroup.AddElement(&mDedicatedTeam1Text);
        mDedicatedGroup.AddElement(&mDedicatedTeam2Text);
        mDedicatedGroup.SetZOrder(100);
        FLRenderer::AddInterfaceElement(CameraManager::Instance()->m_hView[0], &mDedicatedGroup, RedInterfaceScreen::CENTER, RedInterfaceScreen::CENTER, false);
        DedicatedMsgY = 0.0f;
        DedicatedMsgX = 0.0f;
        mDedicatedGroup.SetPosition(PblVector3(0.0f, 0.0f, 0.0f));
    }

    SoldierAnimatorLowResClass::PostLoad();
    RedSceneManager::SetupStaticWorld();

    mWaitMessageGroup.SetPosition(0.0f, -30.0f);
    mWaitMessageGroup.Visible(false);
    mWaitTextElement.SetIntercharacterSpacing(0);
    mWaitTextElement.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
    mWaitTextElement.SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_BASELINE);
    mWaitTextElement.Visible(true);
    mWaitMessageGroup.AddElement(&mWaitTextElement);
    mWaitTimer = 0.0f;
    FLRenderer::AddInterfaceElement(&mWaitMessageGroup, RedInterfaceScreen::CENTER, RedInterfaceScreen::CENTER, true);

    sMissionActive = true;
    LuaHelper::CallGlobalProc("ScriptPostLoad");
    LuaHelper::CloseScript();
    FLInputManager::s_instance.Update(0.0f);
    NetGame::CreateVeryLast();
    return 0;
}

// FUNCTION: SWBF2 0x00532100
void UpdateDedicatedMsg(float fDeltaTime)
{
    if (g_bNoRender)
    {
        UpdateDedicatedNoRender();
        return;
    }

    const float fSafeZone = RedInterfaceScreen::GetScreenSafeZone();
    const float fRangeX = (float)RedInterfaceScreen::GetScreenWidth() * fSafeZone - 360.0f;
    const float fRangeY = (float)RedInterfaceScreen::GetScreenHeight() * fSafeZone - 380.0f;
    DedicatedMsgX += fDeltaTime * DedicatedMsgDX;
    DedicatedMsgY += fDeltaTime * DedicatedMsgDY;

    float fX = (float)fmod(DedicatedMsgX, fRangeX * 2.0f);
    float fY = (float)fmod(DedicatedMsgY, fRangeY * 2.0f);
    if (fX > fRangeX)
    {
        fX = fRangeX * 2.0f - fX;
    }
    if (fY > fRangeY)
    {
        fY = fRangeY * 2.0f - fY;
    }
    mDedicatedGroup.SetPosition(PblVector3(fX - fRangeX * 0.5f, fY - fRangeY * 0.5f - 10.0f, 0.0f));

    unsigned short acName[128];
    unsigned short acCount[128];
    RedTextElement* apText[2] = { &mDedicatedTeam1Text, &mDedicatedTeam2Text };
    for (int i = 0; i < 2; ++i)
    {
        Team* pTeam = Team::sTeams[i + 1];
        const int iCount = mShowTeamPoints[i] ? pTeam->mPoints : pTeam->mCurReinforcements;
        if (pTeam->mName)
        {
            PblUnicode::ustrlcpy(acName, pTeam->mName, 128);
        }
        else
        {
            PblUnicode::ustrncnv(acName, "", 128);
        }
        PblUnicode::usnprintf(acCount, 128, " : %d", iCount);
        PblUnicode::ustrlcat(acName, acCount, 128);
        apText[i]->SetText(acName, true);
    }

    mDedicatedGroup.Visible(!PauseMenu::GetPaused(0));
}

// FUNCTION: SWBF2 0x005323e0
void Update(float fDeltaTime, float fSystemDeltaTime)
{
    if (g_InGameMoviePlayerState)
    {
        FLInputManager::Instance()->Update(fSystemDeltaTime);
        FLRenderer::Render();
        return;
    }

    Lua_Callbacks::DoPeriodicLuaGC(false);

    if (NetGame::IsEnabled())
    {
        if (NetGame::DoesHostKnowILoaded())
        {
            if (!NetGame::IsReadyToPlay())
            {
                if (!mWaitMessageGroup.IsEnabled())
                {
                    mWaitTextElement.SetFont(RedFont::GAMEFONT_SMALL);
                }
                mWaitTextElement.SetText(RedLocalizeDB::Find(0xcf273b03, false), true);
                mWaitTextElement.Enable(!PauseMenu::GetPaused(PauseMenu::GetPausingViewport()));
                PauseMenu::UpdateAllInstances(fSystemDeltaTime);
            }
            mWaitMessageGroup.Enable(!NetGame::IsReadyToPlay() && netMinPlayersToAutoLaunchDedicated != 0);
        }
        else
        {
            if (!mWaitMessageGroup.IsEnabled())
            {
                const PblHash hWait("common.waitforhost");
                mWaitTextElement.SetFont(RedFont::GAMEFONT_SMALL);
                mWaitTextElement.SetText(RedLocalizeDB::Find(hWait, false), true);
            }
            mWaitMessageGroup.Enable(true);
        }

        float fAlpha = (1.0f - (float)fabs((mWaitTimer - (float)(int)mWaitTimer) * 2.0f - 1.0f)) * 255.0f;
        if (fAlpha < 0.0f)
        {
            fAlpha = 0.0f;
        }
        else if (fAlpha > 255.0f)
        {
            fAlpha = 255.0f;
        }
        mWaitTimer += fDeltaTime;
        mWaitTextElement.m_color.a = (unsigned char)(int)fAlpha;
    }

    if (GameLoop::sShowStats && !GameLoop::sQuitRequested && (NetGame::IsEnabled() || !PauseMenu::GetPaused(0)))
    {
        UpdateStats(fDeltaTime, fSystemDeltaTime);
        return;
    }

    ScreenTransition::sInstance->Update(fSystemDeltaTime);
    sSystemDeltaTime = fSystemDeltaTime;

    if (gAttractMode)
    {
        GameLoop::mAttractModeTime -= fDeltaTime;
        if (GameLoop::mAttractModeTime < 0.0f)
        {
            GameSoundEngine::StopAll();
            ShellLoop::SetDemoOver(true);
            sGameOver = true;
            GameState::SetState(0x11e1fc01);
        }
    }

    if (NetGame::IsDedicated())
    {
        UpdateDedicatedMsg(fSystemDeltaTime);
    }

    LARGE_INTEGER liCount;
    QueryPerformanceCounter(&liCount);
    sLastPerformanceCount = liCount.QuadPart;

    MotionBlurEffect::Stop(0, 0);
    GameLoop::sDeltaTime = fDeltaTime;
    GameLoop::sClientDeltaTime = fDeltaTime;
    GameSoundEngine::Update(fDeltaTime);
    if (PopupText::IsEnabled())
    {
        PopupText::Update(fSystemDeltaTime);
    }

    bool bGameOverNow = false;
    if ((sPauseMode || IsGameOver()) && !NetGame::IsEnabled())
    {
        if (IsGameOver())
        {
            ItemTimerDisplay::HideAll();
        }
        CameraManager::Instance()->ClearShake();
        FLInputManager::Instance()->Update(fSystemDeltaTime);
        GameLoop::CheckToggleDisplay();
        GameLoop::CheckScreenShots();
        if (CameraManager::Instance()->IsFreeLook(0))
        {
            CameraManager::Instance()->Update(fSystemDeltaTime);
        }
        FLRenderer::Render();
        PauseMenu::DoUpdate(fSystemDeltaTime);
        static_cast<Thread&>(sFlashyTextManagerInstance).Update(fSystemDeltaTime);
    }
    else
    {
        if (NetGame::FinishedTimeScale())
        {
            clientInputAcc = 0.0f;
        }
        clientInputAcc += NetGame::GetTimeScale() * GameLoop::sDeltaTime + NetGame::GetTimeAdjust();

        if (netIsClient)
        {
            if (!NetGame::IsEnabled())
            {
                FLInputManager::Instance()->Update(clientInputAcc);
                clientInputAcc = 0.0f;
                NetGame::SubmitMove(1);
                NetGame::SetTurnRatio(0.0f);
            }
            else
            {

                const float fThreshold = netSecondsPerTurn * (netFrameLock ? 0.85f : 0.95f);
                int iTurns = 0;
                while (clientInputAcc >= fThreshold && iTurns < 64)
                {
                    clientInputAcc -= netSecondsPerTurn;
                    ++iTurns;
                }
                if (iTurns > 0)
                {
                    FLInputManager::Instance()->Update((float)iTurns * netSecondsPerTurn);
                    NetGame::SubmitMove(iTurns);
                    NetGame::EnterClient();
                    NetGame::ReceiveClient();
                    NetGame::LeaveClient();
                }
                NetGame::SetTurnRatio((netSecondsPerTurn - fThreshold + clientInputAcc) / netSecondsPerTurn);
            }
            NetGame::SendClient();
            GameLoop::CheckToggleDisplay();
            GameLoop::CheckScreenShots();
        }
        else if (NetGame::IsDedicated())
        {
            int iTurns = 0;
            while (clientInputAcc >= netSecondsPerTurn)
            {
                clientInputAcc -= netSecondsPerTurn;
                ++iTurns;
            }
            if (iTurns > 0)
            {
                FLInputManager::Instance()->Update((float)iTurns * netSecondsPerTurn);
                NetGame::SubmitMove(iTurns);
            }
        }

        if (NetGame::IsHost())
        {
            const float fSavedDelta = GameLoop::sDeltaTime;
            NetGame::ReceiveHost();
            NetGame::ApplyLateMoves();
            int iTurnsRun = 0;
            for (int i = NetGame::GetMaxSimTurns(); i != 0; ++iTurnsRun)
            {
                --i;
                if (!NetGame::IsTurnReady())
                {
                    break;
                }
                GameLoop::sDeltaTime = NetGame::GetTurnStep();
                NetGame::SaveLateState();
                TeamManager::UpdatePositions();
                ControllerManager::Update(GameLoop::sDeltaTime);
                if (!g_InGameMoviePlayerState)
                {
                    Thread::UpdateAll(GameLoop::sDeltaTime, false, false);
                }
                NetGame::FinishTurn();
                NetGame::ReceiveHost();
                NetGame::HostChangeSpectator();
                NetGame::HostChangePregame();
            }
            NetGame::UpdatePreGame();
            if (iTurnsRun && !sGameOver && Team::sWinningTeams)
            {
                bGameOverNow = true;
                sGameOver = true;
            }
            NetGame::SendHost();
            GameLoop::sDeltaTime = fSavedDelta;
            if (NetGame::IsEnabled())
            {
                Thread::UpdateAll(GameLoop::sDeltaTime, true, false);
            }
        }
        else
        {
            NetGame::EnterClient();
            if (!sGameOver && Team::sWinningTeams)
            {
                sGameOver = true;
                bGameOverNow = true;
            }
            NetGame::LeaveClient();
        }

        if (netIsClient)
        {
            const float fFrameDelta = GameLoop::sDeltaTime;
            const bool bOneWorldHost = NetGame::IsEnabled() && NetGame::IsHost() && NetGame::IsOneWorld();
            bool bRender = true;
            if (!bOneWorldHost)
            {
                if (NetGame::IsEnabled())
                {
                    const float fTime = NetGame::GetLocalTime();
                    GameLoop::sClientDeltaTime = fTime - clientTimePrev;
                    if (GameLoop::sClientDeltaTime < 0.0f)
                    {
                        GameLoop::sClientDeltaTime = 0.0f;
                    }
                    clientTimePrev = fTime;
                }
                NetGame::EnterClient();
                bRender = NetGame::Predict();
                GameLoop::sDeltaTime = fFrameDelta;
                NetGame::LeaveTurn();
                Thread::UpdateAll(GameLoop::sDeltaTime, true, false);
                NetGame::EnterTurn();
            }
            if (bRender)
            {
                NetGame::LeaveTurn();
                LockOnManager::UpdateAll(GameLoop::sDeltaTime);
                HUD::Manager::Update(fSystemDeltaTime);
                if (!sPauseMode)
                {
                    HUD::GameEvents::Update(fDeltaTime);
                }
                FLRenderer::Render();
                NetGame::EnterTurn();
            }
            if (!bOneWorldHost)
            {
                NetGame::LeaveClient();
            }
        }
        else if (NetGame::IsDedicated())
        {
            NetGame::LeaveTurn();
            HUD::Manager::Update(fSystemDeltaTime);
            if (!sPauseMode)
            {
                HUD::GameEvents::Update(fDeltaTime);
            }
            const unsigned int hView = CameraManager::Instance()->m_hView[0];
            FLRenderer::SetSceneEnable(hView, false);
            FLRenderer::Render();
            FLRenderer::SetSceneEnable(hView, true);
            NetGame::EnterTurn();
        }

        if (NetGame::IsDedicated())
        {
            PauseMenu::DoUpdate(fSystemDeltaTime);
            static_cast<Thread&>(sFlashyTextManagerInstance).Update(fSystemDeltaTime);
        }
    }

    SaveGameManager::Update(GameLoop::sDeltaTime);
    NetGame::EnterClient();
    GuiManager::UpdateAll(fSystemDeltaTime);
    NetGame::LeaveClient();
    SaveGameManager::Update(GameLoop::sDeltaTime);

    QueryPerformanceCounter(&liCount);
    gPrevFrameTime = (float)((double)(unsigned __int64)(liCount.QuadPart - sLastPerformanceCount + sPerformanceCountOffset) * secondsPerPerformanceTick);
    sPerformanceCountOffset = 0;

    if (NetGame::DetectError() && NetGame::ErrorDismissed())
    {
        if (postLoadHostGame)
        {
            postLoadHostGame->Cancel();
        }
        else if (postLoadJoinGame)
        {
            postLoadJoinGame->Cancel();
        }
        GameState::SetState(0x11e1fc01);
    }

    if (GameLoop::sQuitRequested && NetGame::CanQuit())
    {
        if (postLoadHostGame)
        {
            postLoadHostGame->Cancel();
        }
        else if (postLoadJoinGame)
        {
            postLoadJoinGame->Cancel();
        }
        GameState::SetState(0x11e1fc01);

        const Character* pPlayer = Character::FindPlayer(PlayerPrefs::FindByViewport(PauseMenu::GetPausingViewport()));
        if (pPlayer && AreMetagameRulesOn() && GetLastGameWinner() < 0)
        {
            SetLastGameWinner(3 - pPlayer->mTeamNumber);
        }

        if (GameLoop::sShowStats && !NetGame::IsEnabled())
        {
            for (int i = 0; i < Character::sCharacterLimit; ++i)
            {
                Character* pCharacter = Character::GetPtr(i);
                if (pCharacter && pCharacter->mMedalsMgr)
                {
                    pCharacter->mMedalsMgr->SaveMedals();
                }
            }
        }
    }

    bool bLeaveClient = true;
    if (NetGame::IsEnabled() && NetGame::IsHost() && NetGame::IsDedicated())
    {
        bLeaveClient = false;
    }
    else
    {
        NetGame::EnterClient();
    }

    if (bGameOverNow && !EndDisplay::sInstance->HasBeenDisplayed())
    {
        EndDisplay::sInstance->Show(Team::sWinningTeams, END_DISPLAY_TIME);
        SpawnDisplay::Instance(0)->Hide(0);
    }

    if (sGameOver)
    {
        if (!EndDisplay::sInstance->IsFinished())
        {
            EndDisplay::sInstance->Update(fSystemDeltaTime);
        }

        if (sGameOver && EndDisplay::sInstance->IsFinished())
        {
            if (!NetGame::IsEnabled() || netIsClient)
            {
                ScopeDisplay::HideAll();
            }

            if (gLogFinalScore)
            {
                FILE* pFile = fopen("finalscore.log", "a+");
                if (pFile)
                {
                    char acDate[16];
                    char acTime[16];
                    _strdate(acDate);
                    _strtime(acTime);
                    fprintf(pFile, "%s %s\t%s\t%4ds", acDate, acTime, mMissionScript, (int)GetMissionTime());
                    for (int iTeam = 1; iTeam <= 2; ++iTeam)
                    {
                        const Team* pTeam = Team::sTeams[iTeam];
                        const bool bPlayerTeam = AIUtil::IsPlayerTeam(iTeam);
                        fprintf(pFile, "\t%c=%s\t%du@%d%c\t%3dr\t%2dp",
                                (Team::sWinningTeams & (1 << iTeam)) ? 'W' : 'L',
                                sSideNamePrefixes[pTeam->mSide],
                                pTeam->mMemberCount,
                                AIDifficulty::GetIntSkill((const UnitController*)bPlayerTeam, false),
                                bPlayerTeam ? 'P' : ' ',
                                pTeam->mMaxReinforcements >= 0x7fffffff ? -1 : pTeam->mCurReinforcements,
                                pTeam->mPoints);
                    }
                    fprintf(pFile, "\n");
                    fflush(pFile);
                    fclose(pFile);
                }
            }

            sGameOver = true;
            if (gAttractMode)
            {
                GameState::SetState(0x11e1fc01);
            }
            endTimer.Update(fSystemDeltaTime);
            if (!NetGame::IsEnabled() || NetGame::IsGameOver())
            {
                if ((EndDisplay::sInstance->IsFinished() || endTimer.IsFinished()) && !GameLoop::sShowStats &&
                    (NetGame::IsEnabled() || !PauseMenu::GetPaused(0)))
                {
                    EndDisplay::sInstance->Hide();
                    PauseMenu::SetPaused(0, false);
                    GameLoop::sShowStats = true;
                    if (!NetGame::IsEnabled())
                    {
                        PlayerStats::AddGameStatsToProfile();
                    }
                    PauseMenu::SetStatVisible();
                    GameSoundEngine::StopAll();
                    if (Music* pMusic = VoiceOverManager::sInstance->GetAmbientMusic().GetMusic())
                    {
                        Snd::SoundStream::Properties* pProps =
                            const_cast<Snd::SoundStream::Properties*>(pMusic->GetProperties(false));
                        if (pProps)
                        {
                            pProps->SetLooping(true);
                            Snd::SoundStream::Play(pProps, true);
                        }
                    }
                }
            }
        }
    }

    if (bLeaveClient)
    {
        NetGame::LeaveClient();
    }
}

// FUNCTION: SWBF2 0x00533370
int Cleanup()
{
    if (RedMissionLog::Enabled())
    {
        _exit(0);
    }

    TeamAssistManager::Destroy();

    if (mbMetagameRules && mLastGameWinner > 0 && Team::sTeams[1]->GetSide() > Team::sTeams[2]->GetSide())
    {
        mLastGameWinner = 3 - mLastGameWinner;
    }

    Lua_Callbacks::Cleanup();
    sPauseMode = false;
    sGameOver = false;
    PlayerController::sEnabled = -1;

    if (NetGame::IsDedicated())
    {
        FLRenderer::RemoveInterfaceElement(CameraManager::Instance()->m_hView[0], &mDedicatedGroup);
    }

    mWaitMessageGroup.Enable(false);
    mWaitMessageGroup.RemoveAllElements();
    mWaitTextElement.Cleanup();
    FLRenderer::RemoveInterfaceElement(&mWaitMessageGroup);
    RedTerrain::Deactivate();

    sPauseMode = false;
    mAttractGroup.Enable(false);
    mAttractGroup.RemoveAllElements();
    mAttractInfoText.Cleanup();

    mDedicatedGroup.Enable(false);
    mDedicatedGroup.RemoveAllElements();
    mDedicatedTitleText.Cleanup();
    mDedicatedTeam1Text.Cleanup();
    mDedicatedTeam2Text.Cleanup();

    SpawnManager::Destroy();
    Team::DestroyAll();
    ObjectiveManager::DestroyAll();
    PAManager::Destroy();
    EventManager::Cleanup();
    HUD::EventQueue::Destroy();
    SpaceAssault::Destroy();
    NetGame::DestroyClient();

    if (ShellLoop::GetNetCableErrorOnEnter())
    {
        netEnabled = false;
        netEnabledNext = false;
        NetSetup::SetInNetGame(false);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00533740
void SetWorldFilename(char const* pcName)
{
    if (pcName)
    {
        if (!mWorldFileName[0])
        {
            strlcpy(mWorldFileName, pcName, sizeof(mWorldFileName));
        }
    }
    else
    {
        memset(mWorldFileName, 0, sizeof(mWorldFileName));
    }
}

// FUNCTION: SWBF2 0x00533770
void SetTerrainFilename(char const* pcName)
{
    if (pcName)
    {
        if (!mTerrainFileName[0])
        {
            strlcpy(mTerrainFileName, pcName, sizeof(mTerrainFileName));
        }
    }
    else
    {
        memset(mTerrainFileName, 0, sizeof(mTerrainFileName));
    }
}

// FUNCTION: SWBF2 0x005336b0
void SetJoystickYFlip(int iFlip)
{
    RawControllerInputs* pJoystick = FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport());
    if (pJoystick)
    {
        pJoystick->SetFlip(iFlip != 0);
    }
}

// FUNCTION: SWBF2 0x005336e0
int GetJoystickYFlip()
{
    RawControllerInputs* pJoystick = FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport());
    if (pJoystick)
    {
        return pJoystick->GetFlip();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00533700
void SetNextMission(char const* pcName)
{

    if (pcName)
    {
        strlcpy(mMissionScript, pcName, sizeof(mMissionScript));
    }
    else
    {
        memset(mMissionScript, 0, sizeof(mMissionScript));
    }

    mMissionScriptHash = PblHash(mMissionScript);
}

}

// GLOBAL: SWBF2 0x007e651f
namespace GameLoop { bool mbCanSwitchSides = true; }

extern bool netAutoAssignTeams;

// GLOBAL: SWBF2 0x01ed86a7
// SYNTHETIC: SWBF2 0x00404740
// `dynamic initializer for 'netAutoAssignTeamsOriginalSetting''
bool netAutoAssignTeamsOriginalSetting = netAutoAssignTeams;

namespace GameLoop {

// FUNCTION: SWBF2 0x005335b0
void SetCanSwitchSides(bool bFlag)
{
    GameLoop::mbCanSwitchSides = bFlag;
    if (NetGame::IsEnabled())
    {
        netAutoAssignTeams = !bFlag;
        if (netInShell)
        {
            netAutoAssignTeamsOriginalSetting = !bFlag;
        }
    }
}
}

namespace RedScreenShot { void TakeScreenShot(); }

namespace GameLoop {

// FUNCTION: SWBF2 0x005334f0
void CheckScreenShots()
{
    RawControllerInputs* pInputs = FLInputManager::Instance()->GetPlayerJoystick(0);
    if (pInputs->mScreenshotButtonState[1])
    {
        const float fSaved = GameLoop::sClientDeltaTime;
        GameLoop::sClientDeltaTime = 0.0f;
        RedScreenShot::TakeScreenShot();
        GameLoop::sClientDeltaTime = fSaved;
    }
}
}

namespace GameLoop {

// FUNCTION: SWBF2 0x00533540
bool CheckToggleDisplay()
{
    return FLInputManager::Instance()->GetPlayerJoystick(0)->GetKeyboard()->IsKeyDown(0x46);
}
}

namespace GameLoop {

// FUNCTION: SWBF2 0x00533560
bool GetCanSwitchSides()
{
    if (NetGame::IsEnabled())
    {
        return !netAutoAssignTeams;
    }
    return GameLoop::mbCanSwitchSides && !GameLoop::mbMetagameRules;
}
}

// FUNCTION: SWBF2 0x00549eb0
void HUD::Element::UpdateAll(float fDeltaTime)
{
    for (PblListDouble::Node* pNode = HUD::Element::sList.mTerminator.mNext;
         pNode != &HUD::Element::sList.mTerminator; pNode = pNode->mNext)
    {
        HUD::Element* pElement = (HUD::Element*)((char*)pNode - offsetof(HUD::Element, mNode));
        pElement->Update(fDeltaTime);
    }
}

namespace HUD { namespace Manager {

// FUNCTION: SWBF2 0x005658e0
void Update(float fDeltaTime)
{
    float fScale = 1.0f;
    if (gEditor)
    {
        gEditor->Update(FLInputManager::Instance()->GetPlayerJoystick(0), fDeltaTime);
        if (gEditor->mEnabled)
        {
            fScale = 0.0001f;
        }
    }
    HUD::Element::UpdateAll(fScale * fDeltaTime);
}
} }

extern bool netIsDedicated;

// FUNCTION: SWBF2 0x00533110
void GameLoop::UpdateStats(float fDeltaTime, float fSystemDeltaTime)
{
    if (gAttractMode)
    {
        return;
    }
    if (g_bNoRender || PauseMenu::AllStatsDone())
    {
        GameLoop::sShowStats = false;
        if (!NetGame::IsEnabled())
        {
            for (int i = 0; i < Character::sCharacterLimit; ++i)
            {
                Character* character = Character::GetPtr(i);
                if (character && character->mMedalsMgr)
                {
                    character->mMedalsMgr->SaveMedals();
                }
            }
        }
        if (NetGame::IsEnabled() && !netIsDedicated)
        {
            NetGame::EnterClient();
            NetGame::LeaveClient();
        }
        if (MissionPlayList::SelectNextEntry())
        {
            GameState::SetState(0x8ff60339);
        }
        else
        {
            if (postLoadHostGame)
            {
                postLoadHostGame->Cancel();
            }
            else if (postLoadJoinGame)
            {
                postLoadJoinGame->Cancel();
            }
            GameState::SetState(0x11e1fc01);
        }
        return;
    }

    FLInputManager::Instance()->Update(GameLoop::sDeltaTime);
    if (FLInputManager::s_instance.AnyMissing() && NetGame::IsEnabled())
    {
        LuaHelper::CallTableProcWThis(GuiManager::Instance()->mCurrentScope, "Update", fSystemDeltaTime);
    }
    NetGame::SubmitMove(1);
    NetGame::SendClient();
    if (NetGame::IsHost())
    {
        while (NetGame::IsTurnReady())
        {
            NetGame::FinishTurn();
        }
        NetGame::SendHost();
    }
    NetGame::EnterClient();
    CameraManager::Instance()->Update(GameLoop::sDeltaTime);
    FLRenderer::SetSceneEnable(0, false);
    FLRenderer::Render();
    PauseMenu::DoUpdate(fSystemDeltaTime);
    static_cast<Thread&>(sFlashyTextManagerInstance).Update(fSystemDeltaTime);
    GameSoundEngine::Update(fSystemDeltaTime);
    GuiManager::UpdateAll(fSystemDeltaTime);
    ScreenTransition::sInstance->Update(fSystemDeltaTime);
    NetGame::ReceiveClient();
    NetGame::LeaveClient();
    if (NetGame::IsHost())
    {
        NetGame::ReceiveHost();
    }
}

// GLOBAL: SWBF2 0x01e2fde5
bool gNoAIRespawn;

// GLOBAL: SWBF2 0x007eb948
// SYNTHETIC: SWBF2 0x00403d70
// `dynamic initializer for 'frameTimer''
// SYNTHETIC: SWBF2 0x00768a70
// `dynamic atexit destructor for 'frameTimer''
PblPerformanceTimer frameTimer;
