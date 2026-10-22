#include <vector>

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include "CommandPost.h"

#include <process.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "GameState.h"
#include "GameLoop.h"
#include "BonusManager.h"
#include "CollisionManager.h"
#include "NetGame.h"
#include "NetSetup.h"
#include "ShellLoop.h"
#include "CollisionBody.h"
#include "GameMemory.h"
#include "GameMovie.h"
#include "GameSound.h"
#include "GameSoundEngine.h"
#include "GameModel.h"
#include "LoadDisplay.h"
#include "MissionLog.h"
#include "StringDB.h"
#include "DownloadableContent.h"
#include "LuaHelper.h"
#include "HUDManager.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "ConnectivityGraph.h"
#include "DustEffect.h"
#include "EntityFormation.h"
#include "EntityPath.h"
#include "EntityPathFollower.h"
#include "FishEffect.h"
#include "FlagItem.h"
#include "FlashyTextElement.h"
#include "FLEffect.h"
#include "FLGuiManager.h"
#include "FLInputManager.h"
#include "GamePath.h"
#include "HeroMessageDisplay.h"
#include "ItemTimerDisplay.h"
#include "LeafEffect.h"
#include "LuaMemory.h"
#include "NameDisplay.h"
#include "PropGenerator.h"
#include "ReticuleDisplay.h"
#include "RumbleRegion.h"
#include "SkyObject.h"
#include "SoldierAnimatorLowRes.h"
#include "StatusDisplay.h"
#include "Thread.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Application/RedPath.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"

#include <yvals.h>
#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "SoundFL/source/sndbus.h"

#include "PebbleFL/Common/PblFile.h"

#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Application/Renderable.h"
#include "RedEngineFL/Application/pc/pcRedDropShadow.h"
#include "RedEngineFL/Application/pc/pcRedParticle.h"
#include "RedEngineFL/Graphics/PC/pcRedAutoSettings.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Interface/RedGlyphCacheElement.h"
#include "RedEngineFL/Util/RedWarning.h"
#include "AIVisRegion.h"
#include "Cloth.h"
#include "RedEngineFL/Util/ext/IAllocator.h"
#include "DamageRegion.h"
#include "EntitySoldier.h"

#include "Explosion.h"
#include "OrdnanceBeacon.h"
#include "OrdnanceBeam.h"
#include "OrdnanceBullet.h"
#include "OrdnanceEmitter.h"
#include "OrdnanceFatRay.h"
#include "OrdnanceGrapplingHook.h"
#include "OrdnanceGrenade.h"
#include "OrdnanceHaywire.h"
#include "OrdnanceLaser.h"
#include "OrdnanceMeleeThrow.h"
#include "OrdnanceMissile.h"
#include "OrdnanceShell.h"
#include "OrdnanceSticky.h"
#include "OrdnanceTowCable.h"
#include "WeaponAreaEffect.h"
#include "WeaponBinoculars.h"
#include "WeaponCannon.h"
#include "WeaponCatapult.h"
#include "WeaponDestruct.h"
#include "WeaponDetonator.h"
#include "WeaponDisguise.h"
#include "WeaponDispenser.h"
#include "WeaponGrapplingHook.h"
#include "WeaponGrenade.h"
#include "WeaponInvisibility.h"
#include "WeaponLaser.h"
#include "WeaponLauncher.h"
#include "WeaponMelee.h"
#include "WeaponMeleeThrow.h"
#include "WeaponRemote.h"
#include "WeaponRepair.h"
#include "WeaponShield.h"
#include "WeaponTowCable.h"

#include "EntityProp.h"
#include "EntityPropAnimated.h"
#include "EntityBuilding.h"
#include "EntityBuildingAnimated.h"
#include "EntityBuildingAnimatedArmed.h"
#include "EntityBuildingArmed.h"
#include "EntityBuildingArmedDynamic.h"
#include "EntityDoor.h"
#include "EntityTrap.h"
#include "EntityDroid.h"
#include "EntityWalker.h"
#include "EntityDroideka.h"
#include "EntityFlyer.h"
#include "EntityCarrier.h"
#include "EntityHover.h"
#include "EntityPortableTurret.h"
#include "EntityDefenseGridTurret.h"
#include "EntitySoundStatic.h"
#include "EntitySoundStream.h"
#include "CommandBuilding.h"
#include "CommandBuildingArmed.h"
#include "CommandBuildingAnimatedArmed.h"
#include "CommandHover.h"
#include "CommandWalker.h"
#include "CommandFlyer.h"
#include "VehicleSpawn.h"
#include "PowerupStation.h"
#include "PowerupItem.h"
#include "EntityMine.h"
#include "GrassPatch.h"
#include "LeafPatch.h"
#include "Godray.h"
#include "EntityLight.h"
#include "EntityHologram.h"
#include "EntityEffect.h"
#include "Asteroid.h"
#include "EntityRemoteTerminal.h"
#include "ControllerManager.h"
#include "Damageable.h"
#include "FlyRegion.h"
#include "FoleyFXRegion.h"
#include "EntitySound.h"
#include "ParticleSystem.h"
#include "PatrolPath.h"
#include "PlayerProfile.h"
#include "ScreenTransition.h"
#include "TentacleSimulator.h"
#include "TriggerSoundRegion.h"

#include "RedEngineFL/Util/ext/types.h"

#include "LoadUtil.h"

#include "LuaCallbacks.h"

#include "AIUtil.h"

#include "FirstPerson.h"

#include "BirdEffect.h"
#include "Chunk.h"
#include "TriggerRegionManager.h"

#include "CameraManager.h"

#include "RedEngineFL/Application/pc/pcRedDebug.h"
#include "Steam/gfGalaxy.h"

#include "CommandPostManager.h"
#include "BootDisplay.h"
#include "BoundaryManager.h"
#include "CaptureDisplay.h"
#include "ChangeClassDisplay.h"
#include "CollisionMesh.h"
#include "LockOnManager.h"
#include "LuaScript.h"
#include "MessageDisplay.h"
#include "MuzzleFlash.h"
#include "ParticleEmitter.h"
#include "SpectatorDisplay.h"
#include "StatIconDisplay.h"
#include "Tracker.h"
#include "RedEngineFL/Interface/RedInterfaceElement.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "PebbleFL/Common/PblRegion.h"

#include "RedEngineFL/Application/RedTerrain.h"
#include "LuaEvent.h"
#include "RedEngineFL/Interface/RedIFScreen.h"

#include "RedEngineFL/Util/RedAnimation.h"
#include "RedEngineFL/Graphics/RedModel.h"

#include "SoundDebugDisplay.h"
#include "GameVoiceChatNode.h"
#include "PebbleFL/Common/PblJournal.h"

namespace std
{
__declspec(noreturn) void __cdecl _Xbad_alloc();
__declspec(noreturn) void __cdecl _Xlength_error(const char*);
}

namespace DownloadableContent
{
bool IsDownloadableContent();
}

namespace LuaHelper
{
bool OpenScript(char const* pName);

bool CallGlobalProc(char const* pName);
}

namespace Lua_Callbacks
{
void InitMission();
}

class PauseMenu { public: static void Init(); static void Reset(); };
namespace PropGenerator { void Init(); }

namespace SkyManager { void Init(); }

namespace FLRenderer { void Init(bool bUberMode); }

namespace RedInputManager { void Init(); }

namespace RedInputManager { void Cleanup(); }

namespace SaveGameManager { void Init(); }

namespace StatsManager { void Init(); }

class CustomContent
{
public:
    static void Create(bool bMission);
};

namespace MuzzleFlash { bool Init(); }
extern int mSkipCount;
namespace RedLocalizeDB {

// FUNCTION: SWBF2 0x006c6010
void SetSkipCount(int count)
{
    mSkipCount = count;
}
}

namespace RedMissionLog { void Init(); }
namespace RedProfile { void Init(); }

class RedVertexCache
{
public:
    static void Init();
};
class RedFootprintManager : public RedVertexCache
{
public:
    static void Init();
};
 
 

// GLOBAL: SWBF2 0x007e68f1
bool gDrawFlyerShadows = true;

// GLOBAL: SWBF2 0x007e68f4
int WORLD_EXTENTS = 0x400;

// GLOBAL: SWBF2 0x007e5eef
bool CommandPost::sEnableVO = true;
// GLOBAL: SWBF2 0x007e651c
bool EntitySoldier::sEnableAnnouncementVO = true;
// GLOBAL: SWBF2 0x007e651d
bool EntitySoldier::sEnableMusic = true;

namespace FrameUpdate { void Update(); }

namespace PblProfile { void EndFrame(); }
namespace PblProfile { void StartFrame(); }
namespace RedProfile { void DisplayCountersByGroup(); }

namespace QOSDisplay { void Reset(); }

class SoldierAnimatorClass { public: static void Destroy(); };

namespace RedMissionLog { void Reset(); } namespace RedProfile { void Cleanup(); }
namespace StatsManager { void Cleanup(); }
namespace ReserveManager { void Reset(); }
namespace SkyManager { void Reset(); }

namespace SaveGameManager { void Cleanup(); }
namespace GameVoiceChat { unsigned int GetNumNodes(); }
namespace GameVoiceChat { void* GetNode(); }

extern int gNetSetup_errState;

GameState::State* _pNextState;
bool _bDoChangeState;
unsigned int mNextStateHash;
unsigned int mCurrentStateHash;
// GLOBAL: SWBF2 0x007eb97c
// SYNTHETIC: SWBF2 0x00403e60
// `dynamic initializer for '_MissionState''
// SYNTHETIC: SWBF2 0x00768aa0
// `dynamic atexit destructor for '_MissionState''
GameState::MissionState _MissionState;
// GLOBAL: SWBF2 0x007eb998
// SYNTHETIC: SWBF2 0x00403e50
// `dynamic initializer for '_ShellState''
// SYNTHETIC: SWBF2 0x00768a90
// `dynamic atexit destructor for '_ShellState''
GameState::ShellState _ShellState;

GameState::State* _pCurrentState;

GameState::MissionState::State* GameState::MissionState::_pCurrentState;

bool gDisableNetworkOnCleanup;

bool sKioskMode;

bool sbDidOneTimeInit;

// GLOBAL: SWBF2 0x009cef88
double secondsPerPerformanceTick;

namespace GameState
{
// GLOBAL: SWBF2 0x01ec5f40
// SYNTHETIC: SWBF2 0x00403e20
// `dynamic initializer for 'GameState::_TimeBase''
PblTimeBase _TimeBase;
// GLOBAL: SWBF2 0x01ec5f58
// SYNTHETIC: SWBF2 0x00403e40
// `dynamic initializer for 'GameState::_TotalTimeBase''
PblTimeBase _TotalTimeBase;
// GLOBAL: SWBF2 0x01ec5f90
// SYNTHETIC: SWBF2 0x00403e30
// `dynamic initializer for 'GameState::_SystemTimeBase''
PblTimeBase _SystemTimeBase;
}

PblTimeBase::Stamp SystemTimeStamp;
PblTimeBase::Stamp AppTimeStamp;
PblTimeBase::Stamp TotalTimeStamp;
float sDeltaTime;
float sSystemDeltaTime;
LARGE_INTEGER perfCount;
LARGE_INTEGER oldPerfCount;
__int64 maxAdvanceTicks;
__int64 normalAdvanceTicks;
float g_fFrameRateScale;
bool sIgnoreNextTime;
bool gMemoryPoolErrors;

// GLOBAL: SWBF2 0x01f9c2c4
namespace GameState { int sNumFontsLoaded; }

LARGE_INTEGER Frequency;

// GLOBAL: SWBF2 0x01f9c2a0
char* sTTYGrid;

// GLOBAL: SWBF2 0x01f9c2a4
int sTTYInvalidIndex;

// GLOBAL: SWBF2 0x01f9c2a8
int sTTYRows;

// GLOBAL: SWBF2 0x01f9c2ac
int sTTYColumns;

// FUNCTION: SWBF2 0x0052bfe0
void TTYInit()
{
    sTTYRows = 0x50;
    sTTYColumns = 0x50;
    sTTYInvalidIndex = -1;
    sTTYGrid = (char*)RedAllocFromHeap(__RedCurrHeap, 0xC800, 0);

    for (int i = 0; i < 0x50; ++i)
    {
        sTTYGrid[i * 0x280] = 0x0A;
    }
}

// FUNCTION: SWBF2 0x0053a9e0
void GameState::OneTimeInit()
{
    if (sbDidOneTimeInit)
    {
        return;
    }

    sbDidOneTimeInit = true;
    TotalTimeStamp.Mark(&_TotalTimeBase);

    PlayerPrefs::InitAll();
    NetGame::Init();
    RedInputManager::Init();
    RedRenderer::InitStatic();

    LARGE_INTEGER counterFrequency;
    QueryPerformanceFrequency(&counterFrequency);
    secondsPerPerformanceTick = 1.0 / (double)counterFrequency.QuadPart;

    RedRenderer::OpenDynamic();
    RedRenderer::SetTimeBase(&_TimeBase);

    if (!GameSoundEngine::Init(0, true))
    {
        RedWarning::SetLogData(RedWarning::RED_SEVERITY_FATAL, "Source\\GameState.cpp", 804,
                               "Oct 19 2017", "14:22:39");
        RedWarning::LogMessage("Could not initialize sound engine!");
        RedWarning::DialogBoxMessage("Could not initialize sound engine!");
        _exit(0);
    }

    TTYInit();

    QueryPerformanceFrequency(&Frequency);
    maxAdvanceTicks = Frequency.QuadPart;
    normalAdvanceTicks = Frequency.QuadPart >> 4;

    CloseJournal();
}

// FUNCTION: SWBF2 0x0053aaf0
void GameState::OneTimeCleanup()
{
    if (!sbDidOneTimeInit)
    {
        return;
    }

    sbDidOneTimeInit = false;
    GameSoundEngine::Term(true);
    RedRenderer::CleanupStatic();
    RedInputManager::Cleanup();
}

// FUNCTION: SWBF2 0x0053ab20
void GameState::UpdateTime()
{
    if (g_bNoRender)
    {
        Sleep(1);
    }

    if (netInShell ? netEnabled : netEnabledNext)
    {
        NetGame::FrameLock();
    }

    __int64 iDeltaTicks = 0;

    if (g_fFixedRate > 0.0f)
    {
        float fDeltaTime = 1.0f / g_fFixedRate;
        if (g_fFrameRateScale > 0.0f)
        {
            fDeltaTime = g_fFrameRateScale * fDeltaTime;
        }

        if (!GameLoop::sPauseMode)
        {
            _TimeBase.AdvanceTime(fDeltaTime);
        }
        _SystemTimeBase.AdvanceTime(fDeltaTime);
    }
    else
    {

        __int64 iMinTicks = Frequency.QuadPart / 80;
        for (;;)
        {
            QueryPerformanceCounter(&perfCount);
            iDeltaTicks = perfCount.QuadPart - oldPerfCount.QuadPart;
            if (iDeltaTicks >= iMinTicks)
            {
                break;
            }

            int iSleep = -15 - (int)((double)(unsigned __int64)(iMinTicks - iDeltaTicks)
                                     * -1000.0 / (double)Frequency.QuadPart);
            if (iSleep > 0)
            {
                Sleep(iSleep);
            }
        }

        if (iDeltaTicks > maxAdvanceTicks)
        {
            iDeltaTicks = normalAdvanceTicks;
        }

        if (g_fFrameRateScale > 0.0f)
        {
            iDeltaTicks = (__int64)((float)iDeltaTicks * g_fFrameRateScale + 0.5f);
        }

        if (!GameLoop::sPauseMode)
        {
            _TimeBase.AdvanceTicks(iDeltaTicks);
        }
        _SystemTimeBase.AdvanceTicks(iDeltaTicks);

        oldPerfCount = perfCount;
    }

    if (!sIgnoreNextTime)
    {
        _TotalTimeBase.AdvanceTicks(iDeltaTicks);
    }
    sIgnoreNextTime = false;

    sDeltaTime = AppTimeStamp.GetElapsed();
    sSystemDeltaTime = SystemTimeStamp.GetElapsed();

    float fMaxDeltaTime = (netInShell ? netEnabled : netEnabledNext) ? 1.0f : 0.1f;
    if (sDeltaTime > fMaxDeltaTime)
    {
        sDeltaTime = fMaxDeltaTime;
    }
    if (sSystemDeltaTime > fMaxDeltaTime)
    {
        sSystemDeltaTime = fMaxDeltaTime;
    }

    AppTimeStamp.Mark();
    SystemTimeStamp.Mark();
}

// FUNCTION: SWBF2 0x0053ae10
void GameState::SetState(unsigned int uiState)
{

    _bDoChangeState = true;

    if (uiState == 0x8FF60339)
    {
        _pNextState = &_MissionState;
        mNextStateHash = 0x8FF60339;
    }
    else if (uiState == 0x11E1FC01)
    {
        _pNextState = &_ShellState;
        mNextStateHash = 0x11E1FC01;
    }
    else if (uiState == 0)
    {
        _pNextState = 0;
        mNextStateHash = 0;
    }
    else
    {
        _bDoChangeState = false;
    }
}

// FUNCTION: SWBF2 0x0053ae80
unsigned int GameState::GetState()
{
    return mCurrentStateHash;
}

// FUNCTION: SWBF2 0x0053ae90
bool GameState::Update()
{
    if (_bDoChangeState)
    {

        _SetState(_pNextState);
        _bDoChangeState = false;
        _pNextState = 0;
    }

    if (!_pCurrentState)
    {
        return false;
    }

    PblProfile::StartFrame();
    FrameUpdate::Update();
    UpdateTime();
    _pCurrentState->Update(sDeltaTime, sSystemDeltaTime);
    PblProfile::EndFrame();
    return true;
}

// FUNCTION: SWBF2 0x0053adc0
void GameState::_SetState(State* pState)
{
    State* pOld = _pCurrentState;
    if (pOld)
    {
        pOld->Exit();
    }

    _pCurrentState = pState;
    mCurrentStateHash = mNextStateHash;
    mNextStateHash = 0;

    if (pState)
    {
        pState->Enter();
    }
}

namespace RedMissionLog {

// FUNCTION: SWBF2 0x006cccc0
void Init()
{
}
}

namespace RedProfile {

// FUNCTION: SWBF2 0x006f7450
void Init()
{
}
}

// FUNCTION: SWBF2 0x006b3ca0
void RedRenderer::SetSky(bool)
{
}

// FUNCTION: SWBF2 0x006f6ae0
void RedVertexCache::Init()
{
}

// FUNCTION: SWBF2 0x006f7460
void RedFootprintManager::Init()
{
}

// GLOBAL: SWBF2 0x01eaf000
namespace SaveGameManager { unsigned int mReqSaveFile2Data; }

namespace SaveGameManager { extern void (*mOnDone)(); }
// GLOBAL: SWBF2 0x01eaf008
namespace SaveGameManager { unsigned int mReqSaveFile2Size; }
// GLOBAL: SWBF2 0x01eaf010
namespace SaveGameManager { unsigned int mReqSaveFile1Data; }
// GLOBAL: SWBF2 0x01eaf014
namespace SaveGameManager { unsigned int mReqSaveFile1Size; }
// GLOBAL: SWBF2 0x01eaeff7
namespace SaveGameManager { unsigned char mForceNoSaveDevice; }
// GLOBAL: SWBF2 0x009cef72
namespace PblBuildType { unsigned char gCreateMemoryProfile; }

namespace SaveGameManager {

// FUNCTION: SWBF2 0x00632ee0
void Init()
{
    SaveGameManager::mReqSaveFile2Data = 0;
    SaveGameManager::mOnDone = 0;
    SaveGameManager::mReqSaveFile2Size = 0;
    SaveGameManager::mReqSaveFile1Data = 0;
    SaveGameManager::mReqSaveFile1Size = 0;
    SaveGameManager::mForceNoSaveDevice = PblBuildType::gCreateMemoryProfile;
}
}

// FUNCTION: SWBF2 0x005396b0
void GameState::CreateBaseExplosionClasses()
{
    new ExplosionClass(PblHash("explosion"));
}

// FUNCTION: SWBF2 0x00539710
void GameState::CreateBaseOrdnanceClasses()
{
    new OrdnanceLaserClass(PblHash("bolt"));
    new OrdnanceBeaconClass(PblHash("beacon"));
    new OrdnanceBeamClass(PblHash("beam"));
    new OrdnanceBulletClass(PblHash("bullet"));
    new OrdnanceEmitterClass(PblHash("emitterordnance"));
    new OrdnanceFatRayClass(PblHash("fatray"));
    new OrdnanceGrapplingHookClass(PblHash("grapplinghook"));
    new OrdnanceGrenadeClass(PblHash("grenade"));
    new OrdnanceHaywireClass(PblHash("haywire"));
    new OrdnanceLaserClass(PblHash("laser"));
    new OrdnanceMeleeThrowClass(PblHash("meleethrowordnance"));
    new OrdnanceMissileClass(PblHash("missile"));
    new OrdnanceShellClass(PblHash("shell"));
    new OrdnanceStickyClass(PblHash("sticky"));
    new OrdnanceTowCableClass(PblHash("towcable"));
}

// FUNCTION: SWBF2 0x00539aa0
void GameState::CreateBaseWeaponClasses()
{
    new WeaponCannonClass(PblHash("weapon"));
    new WeaponAreaEffectClass(PblHash("areaeffectweapon"));
    new WeaponBinocularsClass(PblHash("binoculars"));
    new WeaponCannonClass(PblHash("cannon"));
    new WeaponCatapultClass(PblHash("catapult"));
    new WeaponDestructClass(PblHash("destruct"));
    new WeaponDetonatorClass(PblHash("detonator"));
    new WeaponDisguiseClass(PblHash("disguise"));
    new WeaponDispenserClass(PblHash("dispenser"));
    new WeaponGrapplingHookClass(PblHash("grapplinghookweapon"));
    new WeaponGrenadeClass(PblHash("grenade"));
    new WeaponInvisibilityClass(PblHash("invisibility"));
    new WeaponLaserClass(PblHash("laser"));
    new WeaponLauncherClass(PblHash("launcher"));
    new WeaponRepairClass(PblHash("repair"));
    new WeaponMeleeClass(PblHash("melee"));
    new WeaponMeleeThrowClass(PblHash("meleethrow"));
    new WeaponRemoteClass(PblHash("remote"));
    new WeaponShieldClass(PblHash("shield"));
    new WeaponTowCableClass(PblHash("towcableweapon"));
}

// FUNCTION: SWBF2 0x00539f60
void GameState::CreateBaseEntityClasses()
{
    new EntityPropClass(PblHash("prop"));
    new EntityPropClass(PblHash("building"));
    new EntityPropAnimatedClass(PblHash("animatedprop"));
    new EntityBuildingClass(PblHash("destructablebuilding"));
    new EntityBuildingAnimatedClass(PblHash("animatedbuilding"));
    new EntityBuildingAnimatedArmedClass(PblHash("armedanimatedbuilding"));
    new EntityBuildingArmedClass(PblHash("armedbuilding"));
    new EntityBuildingArmedDynamicClass(PblHash("armedbuildingdynamic"));
    new EntityDoorClass(PblHash("door"));
    new EntityTrapClass(PblHash("trap"));
    new EntitySoldierClass(PblHash("soldier"));
    new EntityDroidClass(PblHash("droid"));
    new EntityWalkerClass(PblHash("walker"));
    new EntityDroidekaClass(PblHash("walkerdroid"));
    new EntityFlyerClass(PblHash("flyer"));
    new EntityCarrierClass(PblHash("carrier"));
    new EntityHoverClass(PblHash("hover"));
    new EntityHoverClass(PblHash("treaded"));
    new EntityPortableTurretClass(PblHash("portableturret"));
    new EntityDefenseGridTurretClass(PblHash("defensegridturret"));
    new EntitySoundStaticClass(PblHash("SoundAmbienceStatic"));
    new EntitySoundStreamClass(PblHash("SoundAmbienceStreaming"));
    new CommandBuildingClass(PblHash("commandpost"));
    new CommandBuildingArmedClass(PblHash("commandarmedbuilding"));
    new CommandBuildingAnimatedArmedClass(PblHash("commandarmedanimatedbuilding"));
    new CommandHoverClass(PblHash("commandhover"));
    new CommandWalkerClass(PblHash("commandwalker"));
    new CommandFlyerClass(PblHash("commandflyer"));
    new VehicleSpawnClass(PblHash("vehiclespawn"), false);
    new VehicleSpawnClass(PblHash("vehiclepad"), true);
    new PowerupStationClass(PblHash("powerupstation"));
    new PowerupItemClass(PblHash("powerupitem"));
    new EntityMineClass(PblHash("mine"));
    new DustEffectClass(PblHash("dusteffect"));
    new CloudClusterClass(PblHash("cloudcluster"));
    new GrassPatchClass(PblHash("grasspatch"));
    new LeafPatchClass(PblHash("leafpatch"));
    new GodrayClass(0xBA5B0537u);
    new EntityLightClass(PblHash("light"));
    new EntityHologramClass(PblHash("hologram"));
    new RumbleEffectClass(PblHash("rumbleeffect"));
    new EntityEffectClass(PblHash("effectprop"));
    new AsteroidClass(PblHash("asteroid"));
    new EntityClothClass(PblHash("cloth"));
    new EntityRemoteTerminalClass(PblHash("remoteterminal"));
    new FlagItemClass(PblHash("flag"));
}

// FUNCTION: SWBF2 0x0053af10
void GameState::PreStateInit(bool bMission)
{

    GameMemory::BuildHeaps(!bMission);
    RedSetCurrentHeap(GameMemory::ApplicationHeap);
    OneTimeInit();

    gDrawFlyerShadows = true;
    PblRandom::SetGlobalSeed(0);

    if (!sKioskMode)
    {
        sIgnoreNextTime = true;
    }

    RedMissionLog::Init();
    MissionLog::Init();

    GameState::sNumFontsLoaded = 0;
    RedSetCurrentHeap(GameMemory::RunTimeHeap);

    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        pProfile->mPCOptions.ApplyPreLoadSettings();
    }

    DownloadableContent::SetCurrentMission(GameLoop::mMissionScriptHash);
    LuaHelper::InitState();
    RedProfile::Init();
    HUD::Manager::Open();
    Lua_Callbacks::Init();
    StringDB::Init();

    if (bMission)
    {
        unsigned int uiMissionHash = GameLoop::mMissionScriptHash;
        if (!LoadUtil::ReadDataFile("MISSION.lvl", 0, &uiMissionHash, 1, 0))
        {
            RedTexture::Init();
            RedRenderer::Init();
            RedWarning::SetLogData(RedWarning::RED_SEVERITY_FATAL, "Source\\GameState.cpp", 1278,
                                   "Oct 19 2017", "14:22:39");
            RedWarning::LogMessage("Could not open mission.lvl");
            RedWarning::DialogBoxMessage("Could not open mission.lvl");
            _exit(0);
        }

        bool bOpened = LuaHelper::OpenScript(GameLoop::mMissionScript);
        if (!bOpened)
        {
            char acName[256];
            memset(acName, 0, sizeof(acName) - 1);
            _snprintf(acName, sizeof(acName), "MISSION\\%s.lvl", GameLoop::mMissionScript);
            if (!LoadUtil::ReadDataFile(acName, 0, 0, 0, 0))
            {
                RedWarning::SetLogData(RedWarning::RED_SEVERITY_FATAL, "Source\\GameState.cpp",
                                       1291, "Oct 19 2017", "14:22:39");
                RedWarning::LogMessage("Could not open %s", acName);
                RedWarning::DialogBoxMessage("Could not open %s", acName);
                _exit(0);
            }
            bOpened = LuaHelper::OpenScript(GameLoop::mMissionScript);
        }

        WORLD_EXTENTS = 0x400;

        if (bOpened)
        {
            Lua_Callbacks::InitMission();
            LuaHelper::CallGlobalProc("ScriptPreInit");
        }
    }

    RedTexture::Init();
    RedRenderer::Init();

    if (pProfile && !pProfile->mInitialized)
    {
        pProfile->mPCOptions.renderOptions = RedAutoSettings::GetRenderOptions();
    }

    GameSoundEngine::Create();
    CommandPost::sEnableVO = true;
    EntitySoldier::sEnableMusic = true;
    EntitySoldier::sEnableAnnouncementVO = true;
    FLInputManager::InitCustomFontData();
    RedFont::Init();
    RedGlyphCacheElement::InitPool(0x1770);

    if (!bMission)
    {
        GameMovie::Create();
    }

    SaveGameManager::Init();
    RedRenderer::SetFogEnable(true);
    RedRenderer::SetSky(1);
    RedVertexCache::Init();
    RedParticleRenderer::Init();
    RedFootprintManager::Init();
    RedDropShadowManager::Init();
    FLInputManager::Init();
    GuiManager::InitAll();
    ScreenTransition::Create();
    StatsManager::Init();
    ParticleSystem::Init();
    ClothData::Init();
    TentacleSimulator::Init();
    AcklayData::Init();
    CreateBaseExplosionClasses();
    CreateBaseOrdnanceClasses();
    CreateBaseWeaponClasses();
    CreateBaseEntityClasses();
    WeaponMeleeClass::Init();

    new SoundStaticRegionFactory();
    new SoundStreamRegionFactory();
    new SoundSpaceRegionFactory();
    new TriggerSoundRegionFactory();
    new FoleyFXRegionFactory();
    new FlyRegionManager();
    new RumbleRegionManager();
    new DamageRegionFactory();
    new AIVisRegionManager();
    new EntityFormationFactory();
    new EntityPathFactory();
    new EntityPath::BranchRegionFactory();
    new AI::PatrolPathFactory();

    ControllerManager::Reset();
    NetGame::Create(bMission);
    CustomContent::Create(bMission);
    JournalStartTimes();
    PblRandom::SetGlobalSeed((unsigned int)_time64(0));

    if (bMission)
    {

        float fExtent = (float)WORLD_EXTENTS;
        float aMax[3] = { fExtent, 0.0f, fExtent };
        float aMin[3] = { -fExtent, 0.0f, -fExtent };
        CollisionManager::Create(*(const PblVector3*)aMin, *(const PblVector3*)aMax);

        ChunkClusterManager::Create();
        TriggerRegionManager::Create();
        DamageRegion::Create();
        CommandPost::Create();
        MuzzleFlash::Init();
        LeafParticleManager::Create();
        BirdParticleManager::Create();
        FishParticleManager::Create();
        ControllerManager::Init();
        AIUtil::Init();
        ConnectivityGraphManager::Init();
    }

    Damageable::StaticInit();
    NetGame::EnterClient();
    FlashyTextManager::Init(bMission);

    if (bMission)
    {
        PauseMenu::Init();
        PropGenerator::Init();
    }

    SkyManager::Init();
    NetGame::LeaveClient();
    NetGame::EnterClient();
    FLRenderer::Init(AIUtil::IsUberMode());
    CameraManager::Create(GameLoop::GetNumCameras());

    FLEffect::InitAll(bMission);
    NetGame::LeaveClient();
    CommandPostManager::Create();

    RedDebug::Init();
    FirstPerson::PreInit();

    if (bMission && DownloadableContent::IsDownloadableContent())
    {
        RedLocalizeDB::SetSkipCount(1);
    }

    LoadUtil::ReadDataFile("core.lvl", 0, 0, 0, 0);

    if (bMission && DownloadableContent::IsDownloadableContent())
    {
        PblFile file;
        char acFullName[300];
        LoadUtil::MakeFullName("core.lvl", LoadUtil::FILE_TYPE_NONE, acFullName,
                               sizeof(acFullName), true);
        if (file.Open(acFullName, false, false, true))
        {
            LoadUtil::ReadDataFile(file, 0, 0, 0, 0);
            file.Close();
        }
    }

    LoadUtil::ReadDataFile("common.lvl", 0, 0, 0, 0);
    RedFont::PostCommonInit();
    GameSoundEngine::InitializeBusGains();
    gSoundEnabled = true;
    LoadDisplay::Create();
    LoadDisplay::sInstance->Begin();
    GameSound::InitDeferred();
    NetGame::EnterClient();
    NetGame::LeaveClient();
    RedRenderer::SetFogEnable(bMission);
}

// FUNCTION: SWBF2 0x005391b0
GamePath* AI::PatrolPathFactory::CreatePath() const
{
    return new PatrolPath();
}

// FUNCTION: SWBF2 0x00539270
GamePath* EntityPathFactory::CreatePath() const
{
    return new EntityPath();
}

// FUNCTION: SWBF2 0x00539330
void* SoundStaticRegionFactory::CreateRegion(RedRegionDesc& desc, const char* pcName) const
{
    EntitySoundStaticClass soundClass(0xEDE52A96);
    return EntitySoundClass::CreateSoundFromRegion(desc, pcName, GetPrefix(), soundClass);
}

// FUNCTION: SWBF2 0x005393d0
void* SoundStreamRegionFactory::CreateRegion(RedRegionDesc& desc, const char* pcName) const
{
    EntitySoundStreamClass soundClass(0x93D6C380);
    return EntitySoundClass::CreateSoundFromRegion(desc, pcName, GetPrefix(), soundClass);
}

// FUNCTION: SWBF2 0x00539470
bool IsNetObjectFeatureEnabled()
{
    return false;
}

// FUNCTION: SWBF2 0x00539480
GamePath* EntityFormationFactory::CreatePath() const
{
    return new EntityFormation();
}

// FUNCTION: SWBF2 0x00539510
void GameState::State::Enter()
{
}

// FUNCTION: SWBF2 0x00539520
void GameState::State::Update(float, float)
{
}

// FUNCTION: SWBF2 0x00539530
void GameState::State::Exit()
{
}

// FUNCTION: SWBF2 0x005395a0
void GameState::MissionState::State::Enter()
{
}

// FUNCTION: SWBF2 0x005395b0
void GameState::MissionState::State::Update(float, float)
{
}

// FUNCTION: SWBF2 0x005395c0
void GameState::MissionState::State::Exit()
{
}

// FUNCTION: SWBF2 0x005395d0
void GameState::MissionState::PlayState::State::Enter()
{
}

// FUNCTION: SWBF2 0x005395e0
void GameState::MissionState::PlayState::State::Update(float, float)
{
}

// FUNCTION: SWBF2 0x005395f0
void GameState::MissionState::PlayState::State::Exit()
{
}

// FUNCTION: SWBF2 0x0053ba90
void GameState::MissionState::SetState(unsigned int uiState)
{
    if (uiState == 0)
    {
        _SetState(0);
    }
    else if (uiState == 0xc2cbd863)
    {
        _SetState(&_PlayState);
    }
}

void GameState::MissionState::_SetState(State* pState)
{
    if (pState == _pCurrentState)
    {
        return;
    }
    if (_pCurrentState)
    {
        _pCurrentState->Exit();
    }
    _pCurrentState = pState;
    if (pState)
    {
        pState->Enter();
    }
}

// FUNCTION: SWBF2 0x0053bcd0
void GameState::MissionState::PlayState::Enter()
{
}

// FUNCTION: SWBF2 0x0053bcf0
void GameState::MissionState::PlayState::Update(float fDeltaTime, float fSystemDeltaTime)
{
    GameLoop::Update(fDeltaTime, fSystemDeltaTime);
}

// FUNCTION: SWBF2 0x0053bce0
void GameState::MissionState::PlayState::Exit()
{
}

// FUNCTION: SWBF2 0x0053bd10
void GameState::MissionState::PlayState::NormalState::Enter()
{
}

// FUNCTION: SWBF2 0x0053bd30
void GameState::MissionState::PlayState::NormalState::Update(float, float)
{
}

// FUNCTION: SWBF2 0x0053bd20
void GameState::MissionState::PlayState::NormalState::Exit()
{
}

// FUNCTION: SWBF2 0x0053bd40
void GameState::MissionState::PlayState::PausedState::Enter()
{
}

// FUNCTION: SWBF2 0x0053bd60
void GameState::MissionState::PlayState::PausedState::Update(float, float)
{
}

// FUNCTION: SWBF2 0x0053bd50
void GameState::MissionState::PlayState::PausedState::Exit()
{
}

// FUNCTION: SWBF2 0x00539690
void GameState::SetIgnoreNextUpdate(bool bIgnore)
{
    sIgnoreNextTime = bIgnore;
}

// FUNCTION: SWBF2 0x005396a0
bool GameState::GetKioskMode()
{
    return sKioskMode;
}

// FUNCTION: SWBF2 0x0053b980
void GameState::ShellState::Enter()
{
    PreStateInit(false);
    ShellLoop::Init();
    GameSound::SetupDeferred();

    Snd::Bus* pBus = Snd::Bus::FindByHashID(0x985C8F54);
    if (pBus)
    {
        pBus->Fade(0.0f, 1.0f, -1.0f);
        Snd::Bus::UpdateAll(0.1f);
    }

    LoadDisplay::Instance()->End();
    ShellLoop::PostLoad();

    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        pProfile->mPCOptions.ApplyPostLoadSettings();
    }

    RedProfile::DisplayCountersByGroup();

    if (gMemoryPoolErrors)
    {
        MemoryPool::PoolErrorPostLoad();
    }

    AppTimeStamp.Mark(&_TimeBase);
    SystemTimeStamp.Mark(&_SystemTimeBase);
}

// FUNCTION: SWBF2 0x0053ba70
void GameState::ShellState::Update(float fDeltaTime, float)
{
    ShellLoop::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x0053ba50
void GameState::ShellState::Exit()
{
    RedProfile::DisplayCountersByGroup();
    ShellLoop::Cleanup();
    PostStateCleanup(false);
}

// FUNCTION: SWBF2 0x0053bae0
void GameState::MissionState::Enter()
{
    gDisableNetworkOnCleanup = false;
    PreStateInit(true);
    GameLoop::Init();
    GameLoop::PostLoad();

    unsigned int uiCount = *(unsigned int*)((char*)CameraManager::sInstance + 0x1c);
    for (unsigned int i = 0; i < uiCount; ++i)
    {
        int iIndex = FLInputManager::s_instance.GetHWPort(i);
        if (iIndex >= 0)
        {
            PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(iIndex);
            if (pProfile)
            {
                pProfile->mOnlineOptions.ApplySettings();
            }
        }
    }

    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        pProfile->mPCOptions.ApplyPostLoadSettings();
    }

    AppTimeStamp.Mark(&_TimeBase);
    SystemTimeStamp.Mark(&_SystemTimeBase);

    RedProfile::DisplayCountersByGroup();
    CameraManager::sInstance->EnableScene(true);
    MissionState::SetState(0xC2CBD863);

    int iFree = RedGetHeapFree(GameMemory::RunTimeHeap) - 0x20800;
    if (iFree > 0x8000)
    {
        int iLow = util::Min_(0x20000, iFree);
        int iHigh = util::Min_((iFree - iLow) / 2 + iLow, 0x40000);
        EntityCloth::SetMemUseage(iLow, iHigh);
        EntityCloth::SetEnable(true);
    }
    else
    {
        EntityCloth::SetEnable(false);
    }

    Snd::Bus* pBus = Snd::Bus::FindByHashID(0x985C8F54);
    if (pBus)
    {
        pBus->Fade(2.0f, 1.0f, -1.0f);
        Snd::Bus::UpdateAll(0.1f);
    }
}

// FUNCTION: SWBF2 0x0053bca0
void GameState::MissionState::Update(float fDeltaTime, float fSystemDeltaTime)
{
    if (_pCurrentState)
    {
        _pCurrentState->Update(fDeltaTime, fSystemDeltaTime);
    }
}

// FUNCTION: SWBF2 0x0053bc60
void GameState::MissionState::Exit()
{
    SetState(0);
    CameraManager::sInstance->EnableScene(false);
    RedProfile::DisplayCountersByGroup();
    GameLoop::Cleanup();
    PostStateCleanup(true);

    if (gDisableNetworkOnCleanup)
    {
        netEnabledNext = false;
        gDisableNetworkOnCleanup = false;
    }
}

// FUNCTION: SWBF2 0x0053be00
void GameState::JournalStartTimes()
{
    _TimeBase.PblJournal_Record();
    _SystemTimeBase.PblJournal_Record();
}

// TEMPLATE: SWBF2 0x0053be20
// PblList<LuaEvent>::RemoveAll

// TEMPLATE: SWBF2 0x0053be60
// PblList<IFScreen>::RemoveAll

// TEMPLATE: SWBF2 0x0053beb0
// PblList<Factory<Entity,EntityClass,EntityDesc> >::RemoveAll

// TEMPLATE: SWBF2 0x0053bf00
// PblList<Factory<Ordnance,OrdnanceClass,OrdnanceDesc> >::RemoveAll

// TEMPLATE: SWBF2 0x0053bf50
// PblList<Factory<Weapon,WeaponClass,WeaponDesc> >::RemoveAll

// TEMPLATE: SWBF2 0x0053bfa0
// PblList<Factory<Ordnance,ExplosionClass,OrdnanceDesc> >::RemoveAll

// FUNCTION: SWBF2 0x0053b640
void GameState::PostStateCleanup(bool bMission)
{
    gSoundEnabled = false;
    if (!sKioskMode)
    {
        sIgnoreNextTime = true;
    }

    RedMissionLog::Reset();
    DamageRegion::ClearList();
    AI::PatrolPath::DestroyAll();
    EntityPathFollower::ClearList();
    EntityFormation::DestroyAll();
    EntityPath::BranchRegion::DestroyAll();
    EntityPath::DestroyAll();
    GamePathFactory::ClearFactoryList();
    gNetSetup_errState = 0;
    RedParticleRenderer::Cleanup();

    NetGame::EnterClient();
    GameSoundEngine::Destroy();
    NetGame::LeaveClient();

    StringDB::Cleanup();
    RedLocalizeDB::Reset();
    LuaHelper::CleanupState();
    LuaMemory::Reset();
    gf::friends::reset();

    delete SoundDebugDisplay::sInstance;

    SoldierAnimatorLowResClass::Destroy();
    FLInputManager::Reset();
    RedAnimation::ClearAll();
    GuiManager::ResetAll();
    RedDebug::Cleanup();

    if (bMission)
    {
        ChunkClusterManager::Destroy();
        TriggerRegionManager::Destroy();
        DamageRegion::Destroy();

        BonusManager::Destroy();
        CommandPostManager::Destroy();
        CollisionManager::Destroy();
    }

    MuzzleFlash::Destroy();
    LeafParticleManager::Destroy();
    BirdParticleManager::Destroy();
    FishParticleManager::Destroy();
    OrdnanceTowCable::PlatformCleanup();
    OrdnanceGrapplingHook::PlatformCleanup();
    ReserveManager::Reset();
    SkyObjectManager::Clear();
    BoundaryManager::Destroy();
    LoadDisplay::Destroy();
    FirstPerson::Deinit();
    LockOnManager::DestroyAll();
    ReticuleDisplay::Destroy();
    HeroMessageDisplay::Destroy();
    ChangeClassDisplay::Destroy();
    StatIconDisplay::Destroy();
    StatusDisplay::Destroy();
    MessageDisplay::Destroy();
    BootDisplay::Destroy();
    SpectatorDisplay::Destroy();
    RedPath::ClearPaths();

    Factory<Entity, EntityClass, EntityDesc>::Reset();
    Factory<Weapon, WeaponClass, WeaponDesc>::Reset();
    Factory<Ordnance, OrdnanceClass, OrdnanceDesc>::Reset();
    Factory<Ordnance, ExplosionClass, OrdnanceDesc>::Reset();

    EntityLightClass::Reset();
    EntityHologramClass::Reset();
    AIUtil::Reset();
    ConnectivityGraphManager::Clean();
    PauseMenu::Reset();
    FlashyTextManager::Clean();
    RumbleRegionManager::Reset();
    NetGame::EnterClient();
    SkyManager::Reset();

    if (bMission)
    {
        NameDisplay::Reset();
        ItemTimerDisplay::Reset();
    }

    QOSDisplay::Reset();
    SpawnDisplay::Destroy();
    NetGame::LeaveClient();

    RedModel::ClearAll();
    RedTerrain::Reset();
    CollisionMesh::ClearAll();
    CollisionBodyPrimitive::ClearAll();
    LuaScriptFactory::Cleanup();
    Tracker::Cleanup();
    PropGenerator::Cleanup();
    HUD::Manager::Close();
    FLRenderer::Cleanup();
    EntityCloth::ClearSpecialAllocator();
    ParticleEmitter::Cleanup();
    GameModel::Reset();

    ScreenTransition::Destroy();
    NetGame::EnterClient();
    FLEffect::CleanupAll();
    NetGame::LeaveClient();
    VehicleSpawn::Reset();
    DustEffectClass::Reset();
    LeafPatchClass::Reset();
    GrassPatchClass::Reset();
    LuaEventManager::Reset();
    PblRegion::Reset();

    RedRegionFactory::RemoveAll();
    SoldierAnimatorClass::Destroy();
    RedShadowProxy::Reset();
    FlagItem::Reset();
    StatsManager::Cleanup();
    SaveGameManager::Cleanup();
    RedDropShadowManager::Cleanup();
    GameMovie::Destroy(true);
    IFScreen::Reset();
    RedInterfaceElement::Reset();
    RedInterfaceScreen::s_screenFull.RemoveAllElements();
    RedGlyphCacheElement::CleanupPool();
    RedFont::Cleanup();
    RedTexture::Cleanup();
    ClothData::Cleanup();
    Thread::Reset();
    RedRenderer::Cleanup();
    RedProfile::Cleanup();

    if (!bMission)
    {
        for (int i = GameVoiceChat::GetNumNodes(); i != 0; --i)
        {
            void* pObject = GameVoiceChat::GetNode();
            if (pObject)
            {
                ((GameVoiceChat::Node*)pObject)->SetID(0xff);
            }
        }
    }

    NetGame::Reset();
    MemoryPool::DestroyAll();
}

struct StatsLeaderboardNode
{
    StatsLeaderboardNode* _Next;
    StatsLeaderboardNode* _Prev;
    void* mPayloadFirst;
    void* mPayloadSecond;
};

struct StatsLeaderboardBucketVector
{
    void** mFirst;
    void** mLast;
    void** mEnd;
};

struct StatsLeaderboardContainer
{

    StatsLeaderboardContainer();
    ~StatsLeaderboardContainer();

    StatsLeaderboardNode* mListHead;
    unsigned int mListSize;
    StatsLeaderboardBucketVector mBuckets;
    unsigned int mBucketMask;
    unsigned int mBucketCount;
    float mMaxLoadFactor;
};
 

// GLOBAL: SWBF2 0x01ec0c20
// SYNTHETIC: SWBF2 0x00403c20
// `dynamic initializer for 'sStatsLeaderboard''
// SYNTHETIC: SWBF2 0x00768910
// `dynamic atexit destructor for 'sStatsLeaderboard''
StatsLeaderboardContainer sStatsLeaderboard;

void ClearStatsLeaderboardList();
StatsLeaderboardContainer::~StatsLeaderboardContainer()
{
    if (mBuckets.mFirst)
    {
        ::operator delete(mBuckets.mFirst);
        mBuckets.mFirst = 0;
        mBuckets.mLast = 0;
        mBuckets.mEnd = 0;
    }
    ClearStatsLeaderboardList();
    ::operator delete(mListHead);
}

// GLOBAL: SWBF2 0x007e6668
namespace StatsManager { int m_state = 1; }

// GLOBAL: SWBF2 0x01e56008
bool sStatsRequestBusy;

// GLOBAL: SWBF2 0x01e55fc0
unsigned char sStatsMatchData[0x40];

// FUNCTION: SWBF2 0x00530cc0
StatsLeaderboardNode* AllocateStatsLeaderboardHead()
{
    StatsLeaderboardNode* pNode = (StatsLeaderboardNode*)
        ::operator new(sizeof(StatsLeaderboardNode));
    if (!pNode)
    {
        std::_Xbad_alloc();
    }

    pNode->_Next = pNode;
    StatsLeaderboardNode** ppPrev = &pNode->_Prev;
    if (ppPrev)
    {
        *ppPrev = pNode;
    }

    return pNode;
}

// FUNCTION: SWBF2 0x00530c80
unsigned int GrowStatsBucketCapacity(const StatsLeaderboardBucketVector* pThis, unsigned int uiCount)
{
    unsigned int uiCapacity =
        (unsigned int)(pThis->mEnd - pThis->mFirst);

    if (0x3FFFFFFFu - uiCapacity / 2 < uiCapacity)
    {
        uiCapacity = 0;
    }
    else
    {
        uiCapacity += uiCapacity / 2;
    }

    if (uiCapacity < uiCount)
    {
        uiCapacity = uiCount;
    }

    return uiCapacity;
}

// FUNCTION: SWBF2 0x00530d00
void FillStatsBucketRange(void** ppFirst, void** ppLast, void* const* ppVal)
{
    unsigned int uiCount = (unsigned int)(ppLast - ppFirst);
    if (ppFirst > ppLast)
    {
        uiCount = 0;
    }

    for (unsigned int i = 0; i < uiCount; i++)
    {
        *ppFirst++ = *ppVal;
    }
}

// FUNCTION: SWBF2 0x00530d40
void FillStatsBucketCount(void** ppDest, unsigned int uiCount, void* const* ppVal)
{
    for (; uiCount != 0; uiCount--)
    {
        if (ppDest)
        {
            *ppDest = *ppVal;
        }
        ppDest++;
    }
}

// FUNCTION: SWBF2 0x00530d70
void** CopyStatsBucketRange(void** ppFirst, void** ppLast, void** ppDest)
{
    unsigned int uiCount = (unsigned int)(ppLast - ppFirst);
    if (ppFirst > ppLast)
    {
        uiCount = 0;
    }

    for (unsigned int i = 0; i < uiCount; i++)
    {
        if (ppDest)
        {
            *ppDest = ppFirst[i];
        }
        ppDest++;
    }

    return ppDest;
}

// FUNCTION: SWBF2 0x00530ab0
void InsertStatsLeaderboardBuckets(StatsLeaderboardBucketVector* pThis, void*** pppResult,
                       void** ppWhere, void**  , void* const* ppVal)
{
    const unsigned int uiOff = (unsigned int)(ppWhere - pThis->mFirst);

    if ((unsigned int)(pThis->mEnd - pThis->mLast) < 0x10u)
    {

        const unsigned int uiSize =
            (unsigned int)(pThis->mLast - pThis->mFirst);
        if (0x3FFFFFFFu - uiSize < 0x10u)
        {
            std::_Xlength_error("vector<T> too long");
        }

        const unsigned int uiCapacity = GrowStatsBucketCapacity(pThis, uiSize + 0x10);

        void** ppNew = 0;
        if (uiCapacity != 0)
        {
            if (uiCapacity > 0x3FFFFFFFu)
            {
                std::_Xbad_alloc();
            }
            ppNew = (void**)::operator new(uiCapacity * 4);
            if (!ppNew)
            {
                std::_Xbad_alloc();
            }
        }

        FillStatsBucketCount(ppNew + uiOff, 0x10, ppVal);
        CopyStatsBucketRange(pThis->mFirst, ppWhere, ppNew);
        CopyStatsBucketRange(ppWhere, pThis->mLast, ppNew + uiOff + 0x10);

        const unsigned int uiNewSize = uiSize + 0x10;
        if (pThis->mFirst)
        {
            ::operator delete(pThis->mFirst);
        }

        pThis->mFirst = ppNew;
        pThis->mEnd = ppNew + uiCapacity;
        pThis->mLast = ppNew + uiNewSize;
    }
    else
    {
        void** ppOldEnd = pThis->mLast;

        void* pVal = *ppVal;

        if ((unsigned int)(ppOldEnd - ppWhere) < 0x10u)
        {

            CopyStatsBucketRange(ppWhere, ppOldEnd, ppWhere + 0x10);
            FillStatsBucketCount(ppOldEnd, 0x10 - (unsigned int)(ppOldEnd - ppWhere), &pVal);
            pThis->mLast += 0x10;
            FillStatsBucketRange(ppWhere, pThis->mLast - 0x10, &pVal);
        }
        else
        {

            pThis->mLast =
                CopyStatsBucketRange(ppOldEnd - 0x10, ppOldEnd, ppOldEnd);

            void** ppSrc = ppOldEnd - 0x10;
            while (ppWhere != ppSrc)
            {
                ppSrc--;
                ppSrc[0x10] = ppSrc[0];
            }

            FillStatsBucketRange(ppWhere, ppWhere + 0x10, &pVal);
        }
    }

    *pppResult = pThis->mFirst + uiOff;
}

// FUNCTION: SWBF2 0x00530a00
void ClearStatsLeaderboardList()
{
    StatsLeaderboardNode* pHead = sStatsLeaderboard.mListHead;
    StatsLeaderboardNode* pNode = pHead->_Next;

    pHead->_Next = pHead;
    sStatsLeaderboard.mListHead->_Prev = sStatsLeaderboard.mListHead;
    sStatsLeaderboard.mListSize = 0;

    while (pNode != sStatsLeaderboard.mListHead)
    {
        StatsLeaderboardNode* pNext = pNode->_Next;
        ::operator delete(pNode);
        pNode = pNext;
    }
}

// FUNCTION: SWBF2 0x00530920
StatsLeaderboardContainer::StatsLeaderboardContainer()
{
    mListHead = 0;
    mListSize = 0;
    mListHead = AllocateStatsLeaderboardHead();

    mBuckets.mFirst = 0;
    mBuckets.mEnd = 0;
    mBuckets.mLast = 0;
    mMaxLoadFactor = 1.0f;

    void* pHead = mListHead;
    void** ppResult;
    InsertStatsLeaderboardBuckets(&mBuckets, &ppResult, 0, 0, &pHead);

    mBucketMask = 7;
    mBucketCount = 8;
}

namespace StatsManager {

// FUNCTION: SWBF2 0x00530640
void Init()
{
    StatsManager::m_state = 1;
    sStatsRequestBusy = false;

    ClearStatsLeaderboardList();

    void* pHead = sStatsLeaderboard.mListHead;
    sStatsLeaderboard.mBuckets.mLast = sStatsLeaderboard.mBuckets.mFirst;

    void** ppResult;
    InsertStatsLeaderboardBuckets(&sStatsLeaderboard.mBuckets, &ppResult,
               sStatsLeaderboard.mBuckets.mFirst, &pHead, &pHead);

    memset(sStatsMatchData, 0, 0x40);
    sStatsLeaderboard.mBucketMask = 7;
    sStatsLeaderboard.mBucketCount = 8;
}
}

void StatsManagerSetStoppedState();

namespace StatsManager {

// FUNCTION: SWBF2 0x005307d0
void Stop()
{
    if (StatsManager::m_state == 7 || StatsManager::m_state == 3)
    {
        StatsManagerSetStoppedState();
    }
    else if (StatsManager::m_state == 0)
    {
        StatsManager::m_state = 1;
    }
    sStatsRequestBusy = false;

    ClearStatsLeaderboardList();

    void* pHead = sStatsLeaderboard.mListHead;
    sStatsLeaderboard.mBuckets.mLast = sStatsLeaderboard.mBuckets.mFirst;

    void** ppResult;
    InsertStatsLeaderboardBuckets(&sStatsLeaderboard.mBuckets, &ppResult,
               sStatsLeaderboard.mBuckets.mFirst, &pHead, &pHead);

    sStatsLeaderboard.mBucketMask = 7;
    sStatsLeaderboard.mBucketCount = 8;
    if (StatsManager::m_state == 1)
    {
        StatsManager::Init();
    }
}
}

// GLOBAL: SWBF2 0x01e55fb7
bool sStatsHeapAllocated;

// GLOBAL: SWBF2 0x01e55fbc
void* sStatsHeapBlockA;

// GLOBAL: SWBF2 0x01e56000
void* sStatsHeapBlockB;

// GLOBAL: SWBF2 0x01e56004
void* sStatsHeapBlockC;

// GLOBAL: SWBF2 0x01e5600c
void* sStatsHeapBlockD;

// GLOBAL: SWBF2 0x01e56010
void* sStatsHeapBlockE;

// FUNCTION: SWBF2 0x00530620
void __fastcall FreeGamespyBlock(void* pBlock)
{
    RedFreeToHeap(GameMemory::GamespyHeap, pBlock, 0);
}

namespace StatsManager {

// FUNCTION: SWBF2 0x005306b0
void Cleanup()
{
    StatsManager::m_state = 1;
    sStatsRequestBusy = false;

    ClearStatsLeaderboardList();

    void* pHead = sStatsLeaderboard.mListHead;
    sStatsLeaderboard.mBuckets.mLast = sStatsLeaderboard.mBuckets.mFirst;

    void** ppResult;
    InsertStatsLeaderboardBuckets(&sStatsLeaderboard.mBuckets, &ppResult,
               sStatsLeaderboard.mBuckets.mFirst, &pHead, &pHead);

    sStatsLeaderboard.mBucketMask = 7;
    sStatsLeaderboard.mBucketCount = 8;

    if (sStatsHeapAllocated)
    {
        sStatsHeapAllocated = false;
        if (sStatsHeapBlockB)
        {
            FreeGamespyBlock(sStatsHeapBlockB);
            sStatsHeapBlockB = 0;
        }
        if (sStatsHeapBlockE)
        {
            FreeGamespyBlock(sStatsHeapBlockE);
            sStatsHeapBlockE = 0;
        }
        if (sStatsHeapBlockD)
        {
            FreeGamespyBlock(sStatsHeapBlockD);
            sStatsHeapBlockD = 0;
        }
        if (sStatsHeapBlockC)
        {
            FreeGamespyBlock(sStatsHeapBlockC);
            sStatsHeapBlockC = 0;
        }
        if (sStatsHeapBlockA)
        {
            FreeGamespyBlock(sStatsHeapBlockA);
            sStatsHeapBlockA = 0;
        }
    }
}
}
namespace StatsManager {

// FUNCTION: SWBF2 0x00530890
void ReadStatsChallenge()
{
}
}

namespace StatsManager {

// FUNCTION: SWBF2 0x005308b0
void ReadStatsResponse()
{
}
}

// FUNCTION: SWBF2 0x00530a90
void __stdcall IgnoreStatsBucketArguments(int, int) {}

// FUNCTION: SWBF2 0x00530aa0
void* __fastcall ReturnStatsBucketPointer(void* pThis) { return pThis; }

// FUNCTION: SWBF2 0x00530c70
void* __fastcall ReturnStatsVectorPointer(void* pThis) { return pThis; }

// FUNCTION: SWBF2 0x00530cf0
void __stdcall IgnoreStatsVectorArgument(int) {}

// FUNCTION: SWBF2 0x0053c000
template void PblJournal::Record<unsigned int>(unsigned int& value, PblJournalType type);

// FUNCTION: SWBF2 0x00530a70
void __stdcall IgnoreStatsListArguments(int, int)
{
}

// FUNCTION: SWBF2 0x00530a80
void* __fastcall ReturnStatsListPointer(void* pThis)
{
    return pThis;
}

// FUNCTION: SWBF2 0x0053bff0
unsigned int __fastcall GetFactorySecondWord(const unsigned int* pThis)
{
    return pThis[1];
}

// GLOBAL: SWBF2 0x007eb99c
// SYNTHETIC: SWBF2 0x00403e70
// `dynamic initializer for 'GameState::MissionState::_PlayState''
// SYNTHETIC: SWBF2 0x00768ab0
// `dynamic atexit destructor for 'GameState::MissionState::_PlayState''
GameState::MissionState::PlayState GameState::MissionState::_PlayState;

// GLOBAL: SWBF2 0x007eb9b8
// SYNTHETIC: SWBF2 0x00403e80
// `dynamic initializer for 'GameState::MissionState::PlayState::_NormalState''
// SYNTHETIC: SWBF2 0x00768ac0
// `dynamic atexit destructor for 'GameState::MissionState::PlayState::_NormalState''
GameState::MissionState::PlayState::NormalState GameState::MissionState::PlayState::_NormalState;

// GLOBAL: SWBF2 0x007eb978
// SYNTHETIC: SWBF2 0x00403e90
// `dynamic initializer for 'GameState::MissionState::PlayState::_PausedState''
// SYNTHETIC: SWBF2 0x00768ad0
// `dynamic atexit destructor for 'GameState::MissionState::PlayState::_PausedState''
GameState::MissionState::PlayState::PausedState GameState::MissionState::PlayState::_PausedState;

// GLOBAL: SWBF2 0x01ec0c14
// SYNTHETIC: SWBF2 0x00403c50
// `dynamic initializer for 'sStatsScratchBytes''
// SYNTHETIC: SWBF2 0x007689a0
// `dynamic atexit destructor for 'sStatsScratchBytes''
std::vector<unsigned char> sStatsScratchBytes;
