#include "LuaCallbacks_Mission.h"
#include "LuaHelper.h"
#include "EnergyBar.h"

#include <stdlib.h>
#include <string.h>

#include "LuaFL/include/lauxlib.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "AIUtil.h"
#include "AIDifficulty.h"
#include "BonusManager.h"
#include "CameraManager.h"
#include "CollisionManager.h"
#include "ChaseCamera.h"
#include "Character.h"
#include "Controllable.h"
#include "Damageable.h"
#include "FirstPerson.h"
#include "FLEffect.h"
#include "GameLoop.h"
#include "GameCamera.h"
#include "GameMemory.h"
#include "GameState.h"
#include "HeroMessageDisplay.h"
#include "LoadUtil.h"
#include "LuaCallbacks_Options.h"
#include "LuaScript.h"
#include "MapCamera.h"
#include "MissionPlayList.h"
#include "NetGame.h"
#include "PlayerProfile.h"
#include "PlayerStats.h"
#include "SoldierAnimatorLowRes.h"
#include "SpawnManager.h"
#include "SpaceAssault.h"
#include "Team.h"
#include "TriggerRegionManager.h"
#include "WeaponCannonInstance.h"
#include "Weapon.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "CombatUtil.h"
#include "AIGoalManager.h"
#include "CommandPost.h"
#include "ConnectivityGraph.h"
#include "PathFinder.h"
#include "AIGoal.h"
#include "Entity.h"
#include "EntityGeometry.h"
#include "EntitySoldier.h"
#include "EntityWalker.h"
#include "EntityHover.h"
#include "EntityFlyer.h"
#include "GameModel.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Application/RedPath.h"
#include "EntityCarrier.h"
#include "Asteroid.h"
#include "EndDisplay.h"
#include "FlagItem.h"
#include "FishEffect.h"
#include "BirdEffect.h"
#include "GameObject.h"
#include "MissionTimer.h"
#include "Timer.h"
#include "HUDGameEvents.h"
#include "HUDElementMap.h"
#include "ProceduralAnimation.h"
#include "VisionManager.h"
#include "TargetManager.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Application/pc/pcRedDebug.h"
#include "HUDEvent.h"
#include "StringDB.h"
#include "PebbleFL/Common/PblRegion.h"
#include "EntityPath.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblHermite.h"
#include <math.h>
extern "C" void* __stdcall D3DXMatrixMultiply(void*, const void*, const void*);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationAxis(PblMatrix*, const PblVector3*, float);
namespace PopupText { void __fastcall ShowPopup(int iType, unsigned int uTextId); }

namespace GameLoop { extern bool mbHistoricalRules; }
extern float sNotifyRadius;
extern int WORLD_EXTENTS;
namespace ParticleSystem { void SetMaxDistanceBias(float fBias); }

namespace PopupText { extern int gNumHints; extern unsigned int gHints[64]; }

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

namespace PopupText {

// FUNCTION: SWBF2 0x00620ec0
static void AddHint(unsigned int uHint)
{
    if (PopupText::gNumHints < 64)
    {
        PopupText::gHints[PopupText::gNumHints++] = uHint;
    }
}
}

namespace VehicleManager {

// FUNCTION: SWBF2 0x0066e380
static void SetNotifyRadius(float fRadius)
{
    sNotifyRadius = fRadius;
}
}

// FUNCTION: SWBF2 0x0058d8d0
void* GetRegion(lua_State* L, int iIndex)
{
    if (lua_type(L, iIndex) == LUA_TLIGHTUSERDATA)
    {
        return (void*)lua_topointer(L, iIndex);
    }
    return PblRegion::Find(PblHash(luaL_checklstring(L, iIndex, 0)));
}

// FUNCTION: SWBF2 0x00491440
void EndDisplay::SetEndMovie(const char* pcFirst, const char* pcSecond)
{
    strlcpy(mMovieFileName, pcFirst, sizeof(mMovieFileName));
    strlcpy(mMovieIdName, pcSecond, sizeof(mMovieIdName));
}

// FUNCTION: SWBF2 0x00555830
void HUD::ElementMap::SetNorth(float fNorth, bool bFlipNorth)
{
    HUD::ElementMap::sNorth = fNorth;
    HUD::ElementMap::sFlipNorth = bFlipNorth;
}

// FUNCTION: SWBF2 0x00555870
HUD::ElementMap* HUD::ElementMap::FindByIndex(int iIndex)
{
    PblListSingle::Node* pNode = HUD::ElementMap::sMapList.mHead.mNext;
    while (pNode != &HUD::ElementMap::sMapList.mHead)
    {
        if (iIndex-- == 0)
        {
            return (HUD::ElementMap*)((char*)pNode - offsetof(HUD::ElementMap, mMapNode));
        }
        pNode = pNode->mNext;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0045f5e0
void CollisionManager::SetMaxCollisionDistance(float fDistance)
{
    if (CollisionManager::sInstance)
    {
        CollisionManager::sInstance->mMaxCollisionDistanceSq = fDistance * fDistance;
    }
}

// FUNCTION: SWBF2 0x00438c50
void AIDifficulty::SetAILuaModifier(int iPlayer, int iEnemy, int iDifficulty)
{
    if ((unsigned int)(iDifficulty - 1) < 3)
    {
        AIDifficulty::mScriptPlayerModifier[iDifficulty - 1] = iPlayer;
        AIDifficulty::mScriptEnemyModifier[iDifficulty - 1] = iEnemy;
    }
}

// FUNCTION: SWBF2 0x00657b90
const char* Timer::GetName()
{
    return StringDB::Find(mId);
}

unsigned int Hash16(unsigned short const* pusString);

class AmmoCounter
{
public:
    static MemoryPool sMemoryPool;
};

extern bool netHeroesEnabled;
extern int netHeroRuleRespawn;
extern int netHeroRuleRespawnVal;
extern bool netHeroUnlocked[3];
extern bool netHeroFirstUnlocked[3];
namespace PblBuildType { extern bool gDemoBuild; }
extern int netHeroRuleTeam;
extern int netHeroRuleUnlock;
extern int netHeroRulePlayer;
namespace PblBuildType { extern bool gDemoHost; }
namespace PblBuildType { extern bool gDemoNoInputRebind; }
namespace PblBuildType { extern bool gFinalBuild; }
extern bool gMapDisplayAll;
extern bool sMissionActive;

namespace AIDebug { bool OnlyOneAI(); }

inline void CameraManager::SetChaseCameraPosition(const PblVector3& vPos, int iCam)
{
    mChaseCamera[iCam]->SetPosition(vPos);
}

namespace PathFinder { Obstacle* FindBarrierByName(unsigned int uName); }

// GLOBAL: SWBF2 0x01e57724
static void* sTempHeapBlock;

// GLOBAL: SWBF2 0x01e57940
namespace Lua_Callbacks { int gTeamSetReinforcementMult; }

namespace Lua_Callbacks
{

// GLOBAL: SWBF2 0x007e7380
static const CallbackEntryT sMissionCallbacks[220] =
{
    { "FindEntityClass", FindEntityClass },
    { "SetPS2ModelMemory", SetPS2ModelMemory },
    { "SetWorldExtents", SetWorldExtents },
    { "StealArtistHeap", StealArtistHeap },
    { "ReadDataFile", ReadDataFile },
    { "ReadDataFileInGame", ReadDataFileInGame },
    { "SetupTempHeap", SetupTempHeap },
    { "ClearTempHeap", ClearTempHeap },
    { "AddCameraShot", AddCameraShot },
    { "SetCameraRotation", SetCameraRotation },
    { "SetCameraPosition", SetCameraPosition },
    { "MoveCameraToEntity", MoveCameraToEntity },
    { "GetMapCameraPosition", GetMapCameraPosition },
    { "SetMapCameraPosition", SetMapCameraPosition },
    { "SetMapCameraOffset", SetMapCameraOffset },
    { "SetMapCameraPitch", SetMapCameraPitch },
    { "SetMapCameraZoom", SetMapCameraZoom },
    { "GetMapCameraZoom", GetMapCameraZoom },
    { "SnapMapCamera", SnapMapCamera },
    { "SetState", SetState },
    { "SetHistorical", SetHistorical },
    { "SetTeamName", SetTeamName },
    { "SetTeamAggressiveness", SetTeamAggressiveness },
    { "SetMinFlyHeight", SetMinFlyHeight },
    { "SetMaxFlyHeight", SetMaxFlyHeight },
    { "SetMinPlayerFlyHeight", SetMinPlayerFlyHeight },
    { "SetMaxPlayerFlyHeight", SetMaxPlayerFlyHeight },
    { "SetUberMode", SetUberMode },
    { "SetAllowBlindJetJumps", SetAllowBlindJetJumps },
    { "SetStationaryWaitTime", SetStationaryWaitTime },
    { "SetTeamIcon", SetTeamIcon },
    { "SetTeamAsEnemy", SetTeamAsEnemy },
    { "SetTeamAsNeutral", SetTeamAsNeutral },
    { "SetTeamAsFriend", SetTeamAsFriend },
    { "SetUnitCount", SetUnitCount },
    { "GetUnitCount", GetUnitCount },
    { "SetReinforcementCount", SetReinforcementCount },
    { "GetReinforcementCount", GetReinforcementCount },
    { "AddReinforcements", AddReinforcements },
    { "SetTeamPoints", SetTeamPoints },
    { "GetTeamPoints", GetTeamPoints },
    { "AddTeamPoints", AddTeamPoints },
    { "AddFlagCapturePoints", AddFlagCapturePoints },
    { "AddAssaultDestroyPoints", AddAssaultDestroyPoints },
    { "AddSpaceAssaultDestroyPoints", AddSpaceAssaultDestroyPoints },
    { "GetTeamFactionId", GetTeamFactionId },
    { "AddUnitClass", AddUnitClass },
    { "SetHeroClass", SetHeroClass },
    { "EnableSPHeroRules", EnableSPHeroRules },
    { "SetHeroUnlockRule", SetHeroUnlockRule },
    { "SetHeroTeamRule", SetHeroTeamRule },
    { "SetHeroPlayerRule", SetHeroPlayerRule },
    { "SetHeroRespawnRule", SetHeroRespawnRule },
    { "EnableSPScriptedHeroes", EnableSPScriptedHeroes },
    { "UnlockHeroForTeam", UnlockHeroForTeam },
    { "AcceptHero", AcceptHero },
    { "ShowAllUnitsOnMinimap", ShowAllUnitsOnMinimap },
    { "ShowTeamPoints", ShowTeamPoints },
    { "ActivateBonus", ActivateBonus },
    { "SetBleedRate", SetBleedRate },
    { "GetBleedRate", GetBleedRate },
    { "SetCarrierClass", SetCarrierClass },
    { "SetSpawnDelay", SetSpawnDelay },
    { "SetSpawnDelayTeam", SetSpawnDelayTeam },
    { "SetUrbanEnvironment", SetDenseEnvironment },
    { "SetDenseEnvironment", SetDenseEnvironment },
    { "SetAttackerSnipeRange", SetAttackerSnipeRange },
    { "SetDefenderSnipeRange", SetDefenderSnipeRange },
    { "SetStayInTurrets", SetStayInTurrets },
    { "SetMemoryPoolSize", SetMemoryPoolSize },
    { "SetNumBirdTypes", SetNumBirdTypes },
    { "SetBirdType", SetBirdType },
    { "SetBirdFlockMinHeight", SetBirdFlockMinHeight },
    { "SetNumFishTypes", SetNumFishTypes },
    { "SetFishType", SetFishType },
    { "FillAsteroidRegion", FillAsteroidRegion },
    { "FillAsteroidPath", FillAsteroidPath },
    { "AddDeathRegion", AddDeathRegion },
    { "AddLandingRegion", AddLandingRegion },
    { "RemoveRegion", RemoveRegion },
    { "ClearWalkers", ClearWalkers },
    { "AddWalkerType", AddWalkerType },
    { "SetAIVehicleNotifyRadius", SetAIVehicleNotifyRadius },
    { "SetAIViewMultiplier", SetAIViewMultiplier },
    { "AISnipeSuitabilityDist", AISnipeSuitabilityDist },
    { "SetGroundFlyerMap", SetGroundFlyerMap },
    { "EnableAIAutoBalance", EnableAIAutoBalance },
    { "DisableAIAutoBalance", DisableAIAutoBalance },
    { "SetAIDifficulty", SetAIDifficulty },
    { "AddMissionObjective", AddMissionObjective },
    { "ActivateObjective", ActivateObjective },
    { "CompleteObjective", CompleteObjective },
    { "AddMissionHint", AddMissionHint },
    { "SetFlagGameplayType", SetFlagGameplayType },
    { "SpaceAssaultEnable", SpaceAssaultEnable },
    { "SpaceAssaultAddCriticalSystem", SpaceAssaultAddCriticalSystem },
    { "SpaceAssaultLinkCriticalSystems", SpaceAssaultLinkCriticalSystems },
    { "SpaceAssaultSetupTeamNumbers", SpaceAssaultSetupTeamNumbers },
    { "SpaceAssaultSetupBitmaps", SpaceAssaultSetupBitmaps },
    { "SpaceAssaultGetScoreLimit", SpaceAssaultGetScoreLimit },
    { "IsCampaign", IsCampaign },
    { "GetWorldFilename", GetWorldFilename },
    { "ShowMessageText", ShowMessageText },
    { "ShowPopup", ShowPopup },
    { "ShowObjectiveTextPopup", ShowObjectiveTextPopup },
    { "ShowTimer", ShowTimer },
    { "ShowSelectionTextPopup", ShowSelectionTextPopup },
    { "BroadcastVoiceOver", BroadcastVoiceOver },
    { "ScriptCB_PlayerSuicide", ScriptCB_PlayerSuicide },
    { "SetMaxCollisionDistance", SetMaxCollisionDistance },
    { "SetParticleLODBias", SetParticleLODBias },
    { "MapAddRegionMarker", MapAddRegionMarker },
    { "MapRemoveRegionMarker", MapRemoveRegionMarker },
    { "MapAddEntityMarker", MapAddEntityMarker },
    { "MapRemoveEntityMarker", MapRemoveEntityMarker },
    { "MapAddClassMarker", MapAddClassMarker },
    { "MapRemoveClassMarker", MapRemoveClassMarker },
    { "MapHideCommandPosts", MapHideCommandPosts },
    { "EnableBuildingLockOn", EnableBuildingLockOn },
    { "FindTimer", FindTimer },
    { "CreateTimer", CreateTimer },
    { "DestroyTimer", DestroyTimer },
    { "StartTimer", StartTimer },
    { "StopTimer", StopTimer },
    { "SetTimerValue", SetTimerValue },
    { "GetTimerValue", GetTimerValue },
    { "SetTimerRate", SetTimerRate },
    { "GetTimerRate", GetTimerRate },
    { "GetTimerName", GetTimerName },
    { "SetDefeatTimer", SetDefeatTimer },
    { "SetMissionTimer", SetMissionTimer },
    { "SetVictoryTimer", SetVictoryTimer },
    { "GetObjectPtr", GetObjectPtr },
    { "ActivateObject", ActivateObject },
    { "DeactivateObject", DeactivateObject },
    { "SetObjectTeam", SetObjectTeam },
    { "GetObjectTeam", GetObjectTeam },
    { "IsObjectAlive", IsObjectAlive },
    { "GetObjectHealth", GetObjectHealth },
    { "GetObjectShield", GetObjectShield },
    { "GetObjectLastHitWeaponClass", GetObjectLastHitWeaponClass },
    { "KillObject", KillObject },
    { "RespawnObject", RespawnObject },
    { "GetEntityPtr", GetEntityPtr },
    { "GetEntityName", GetEntityName },
    { "GetEntityClass", GetEntityClass },
    { "GetEntityClassName", GetEntityClassName },
    { "SetEntityMatrix", SetEntityMatrix },
    { "GetEntityMatrix", GetEntityMatrix },
    { "SetProperty", SetProperty },
    { "CreateEntity", CreateEntity },
    { "DeleteEntity", DeleteEntity },
    { "GetEntityClassPtr", GetEntityClassPtr },
    { "SetClassProperty", SetClassProperty },
    { "EntityFlyerTakeOff", EntityFlyerTakeOff },
    { "EntityFlyerLand", EntityFlyerLand },
    { "EntityFlyerInitAsFlying", EntityFlyerInitAsFlying },
    { "EntityFlyerInitAsLanded", EntityFlyerInitAsLanded },
    { "EnterVehicle", EnterVehicle },
    { "ExitVehicle", ExitVehicle },
    { "EnableFlyerPath", EnableFlyerPath },
    { "GetPathPoint", GetPathPoint },
    { "GetNumTeamMembersAlive", GetNumTeamMembersAlive },
    { "CreateMatrix", CreateMatrix },
    { "GetWorldPosition", GetWorldPosition },
    { "GetScreenPosition", GetScreenPosition },
    { "PlayAnimation", PlayAnimation },
    { "PauseAnimation", PauseAnimation },
    { "RewindAnimation", RewindAnimation },
    { "PlayAnimationFromTo", PlayAnimationFromTo },
    { "SetAnimationStartPoint", SetAnimationStartPoint },
    { "GetTeamSize", GetTeamSize },
    { "GetTeamClassCount", GetTeamClassCount },
    { "GetTeamMember", GetTeamMember },
    { "GetCharacterTeam", GetCharacterTeam },
    { "SelectCharacterTeam", SelectCharacterTeam },
    { "IsCharacterHuman", IsCharacterHuman },
    { "BatchChangeTeams", BatchChangeTeams },
    { "AllowAISpawn", AllowAISpawn },
    { "GetCharacterClass", GetCharacterClass },
    { "SelectCharacterClass", SelectCharacterClass },
    { "SpawnCharacter", SpawnCharacter },
    { "GetCharacterUnit", GetCharacterUnit },
    { "GetCharacterVehicle", GetCharacterVehicle },
    { "GetCharacterRemote", GetCharacterRemote },
    { "GetCharacterControllable", GetCharacterControllable },
    { "GetCommandPostCaptureRegion", GetCommandPostCaptureRegion },
    { "GetCommandPostBleedValue", GetCommandPostBleedValue },
    { "GetCommandPostTeam", GetCommandPostTeam },
    { "MissionVictory", MissionVictory },
    { "MissionDefeat", MissionDefeat },
    { "SetMissionEndMovie", SetMissionEndMovie },
    { "ClearAIGoals", ClearAIGoals },
    { "DeleteAIGoal", DeleteAIGoal },
    { "AddAIGoal", AddAIGoal },
    { "TogglePlanningGraphArcs", TogglePlanningGraphArcs },
    { "BlockPlanningGraphArcs", BlockPlanningGraphArcs },
    { "UnblockPlanningGraphArcs", UnblockPlanningGraphArcs },
    { "ToggleBarriers", ToggleBarriers },
    { "EnableBarriers", EnableBarriers },
    { "DisableBarriers", DisableBarriers },
    { "ForceAIOutOfVehicles", ForceAIOutOfVehicles },
    { "AICanCaptureCP", AICanCaptureCP },
    { "SetAIDamageThreshold", SetAIDamageThreshold },
    { "TranslateAICommand", TranslateAICommand },
    { "AddShieldStrength", AddShieldStrength },
    { "SetMapNorthAngle", SetMapNorthAngle },
    { "ForceHumansOntoTeam1", ForceHumansOntoTeam1 },
    { "CanCharacterInteractWithFlag", CanCharacterInteractWithFlag },
    { "GetFlagCarrier", GetFlagCarrier },
    { "Print3D", Print3D },
    { "DisableSmallMapMiniMap", DisableSmallMapMiniMap },
    { "CreateEffect", CreateEffect },
    { "RemoveEffect", RemoveEffect },
    { "AttachEffectToObject", AttachEffectToObject },
    { "AttachEffectToMatrix", AttachEffectToMatrix },
    { "GetEffectMatrix", GetEffectMatrix },
    { "SetEffectMatrix", SetEffectMatrix },
    { "IsEffectActive", IsEffectActive },
    { "SetEffectActive", SetEffectActive },
};

// FUNCTION: SWBF2 0x0058ab60
int SetPS2ModelMemory(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isnumber(LuaHelper::mState, 1))
    {
        LuaHelper::GetInt(1);
    }
    return 0;
}

}

unsigned int RedGetHeapSize(int heap);

bool RedMoveHeapEnd(int heap, int iDelta);
extern "C" HeapObj* heapobj;
void* RedAllocFromHeapInternal(int heap, unsigned int size, int bTop);
bool RedFreeToHeapInternal(int heap, void* block, unsigned int flags);
void TagRemoveList(TagList* pList, Tag* pTag);
void TagAddList(TagList* pList, Tag* pTag);

namespace ObjectiveManager { void AddObjective(unsigned int uName, unsigned int uText, int iTeam); }
namespace ObjectiveManager { void SetObjectiveState(unsigned int uName, int iState); }

// FUNCTION: SWBF2 0x0058dfe0
static Timer* GetTimer(lua_State* L, int iIndex)
{
    if (lua_type(LuaHelper::mState, iIndex) == LUA_TLIGHTUSERDATA)
    {
        return (Timer*)lua_topointer(LuaHelper::mState, iIndex);
    }
    if (lua_isstring(LuaHelper::mState, iIndex))
    {
        return Timer::Find(LuaHelper::GetString(iIndex));
    }
    return 0;
}

unsigned int FlagItem::GetRtti()
{
    return rttiHashFlagItem;
}

// TEMPLATE: SWBF2 0x005910e0
// _GetEntity<GameObject>

// TEMPLATE: SWBF2 0x00591050
// _GetEntity<EntityGeometry>

// TEMPLATE: SWBF2 0x00591170
// _GetEntity<EntityEx>

// TEMPLATE: SWBF2 0x005912e0
// _GetEntity<Entity const >

// TEMPLATE: SWBF2 0x00591370
// _GetEntity<FlagItem>

template <class T>
static T* _GetEntity(lua_State* L, int iIndex)
{
    Entity* pEntity;
    if (lua_type(LuaHelper::mState, iIndex) == LUA_TLIGHTUSERDATA)
    {
        pEntity = (Entity*)lua_topointer(LuaHelper::mState, iIndex);
    }
    else
    {
        pEntity = EntityEx::mIdMap.Find(PblHash(LuaHelper::GetString(iIndex)));
    }
    if (pEntity && pEntity->IsRtti(T::GetRtti()))
    {
        return (T*)pEntity;
    }
    return 0;
}

// TEMPLATE: SWBF2 0x00591240
// _GetEntityClass<EntityClass>

template <class T>
static T* _GetEntityClass(lua_State* L, int iIndex)
{
    T* pClass;
    if (lua_type(LuaHelper::mState, iIndex) == LUA_TLIGHTUSERDATA)
    {
        pClass = (T*)lua_topointer(LuaHelper::mState, iIndex);
    }
    else
    {
        pClass = T::Find(PblHash(luaL_checklstring(LuaHelper::mState, iIndex, 0)));
    }
    if (pClass && pClass->IsRtti(T::rttiHashEntityClass))
    {
        return pClass;
    }
    return 0;
}

// TEMPLATE: SWBF2 0x00591200
// LuaHelper::PushObject<PblMatrix>

namespace LuaHelper
{
template <class T>
void PushObject(const T& object)
{
    void* pUserdata = lua_newuserdata(mState, sizeof(T));
    memcpy(pUserdata, &object, sizeof(T));
}
template void PushObject<PblMatrix>(const PblMatrix& object);
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x0058ac00
int StealArtistHeap(lua_State* L)
{
    int iSize = (int)luaL_checknumber(LuaHelper::mState, 1);
    if (RedGetHeapSize(GameMemory::ArtistHeap) > (unsigned int)iSize)
    {
        RedMoveHeapEnd(GameMemory::ArtistHeap, -iSize);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058ac50
int ReadDataFile(lua_State* L)
{
    int iArgs = lua_gettop(LuaHelper::mState);
    if (iArgs > 0 && lua_isstring(LuaHelper::mState, 1) && iArgs < 256)
    {
        const char* pcFile = LuaHelper::GetString(1);
        unsigned int uiHashes[256];
        const char* pcNames[256];
        unsigned int uiCount = 0;
        for (int i = 2; i <= iArgs; ++i)
        {
            if (lua_isstring(LuaHelper::mState, i))
            {
                const char* pcName = LuaHelper::GetString(i);
                pcNames[uiCount] = pcName;
                uiHashes[uiCount++] = PblHash(pcName);
            }
            else if (lua_type(LuaHelper::mState, i) == LUA_TTABLE)
            {
                lua_pushnil(LuaHelper::mState);
                while (lua_next(LuaHelper::mState, i))
                {
                    const char* pcName = LuaHelper::GetString(-1);
                    pcNames[uiCount] = pcName;
                    uiHashes[uiCount++] = PblHash(pcName);
                    lua_settop(LuaHelper::mState, -2);
                }
            }
        }
        if (uiCount)
        {
            LoadUtil::ReadDataFile(pcFile, 0, uiHashes, uiCount, pcNames);
        }
        else
        {
            LoadUtil::ReadDataFile(pcFile, 0, 0, 0, 0);
        }

        if (!_stricmp(pcFile, "ingame.lvl"))
        {
            FirstPerson::Init();
            SoldierAnimatorLowResClass::Create();
            if (PblBuildType::gDemoBuild)
            {
                lua_pushboolean(LuaHelper::mState, true);
                LuaHelper::SetGlobal("gDemoBuild");
                if (PblBuildType::gDemoNoInputRebind)
                {
                    lua_pushboolean(LuaHelper::mState, true);
                    LuaHelper::SetGlobal("gNoControllerChanges");
                }
                lua_pushboolean(LuaHelper::mState, PblBuildType::gDemoHost);
                LuaHelper::SetGlobal("gDemoHost");
            }
            if (PblBuildType::gFinalBuild)
            {
                lua_pushboolean(LuaHelper::mState, true);
                LuaHelper::SetGlobal("gFinalBuild");
            }
            LuaScript::LoadScript("game_interface");
            MissionPlayList::PushToLua();
            LuaScriptFactory::PurgeUnused();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058af00
int ReadDataFileInGame(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState))
    {
        LoadUtil::ReadDataFileOnHeap(LuaHelper::GetString(1), -1, 0, 0, 0, 0, 0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058af40
int SetupTempHeap(lua_State* L)
{
    int iSize = LuaHelper::GetInt(1);
    sTempHeapBlock = RedAllocFromHeap(GameMemory::RunTimeHeap, iSize, 1);
    GameMemory::TempLoadHeap =
        RedCreateHeap(sTempHeapBlock, (char*)sTempHeapBlock + iSize - 256, 16, "Temp");
    RedSetTempHeap(GameMemory::TempLoadHeap);
    return 0;
}

// FUNCTION: SWBF2 0x0058afa0
int ClearTempHeap(lua_State* L)
{
    if (sTempHeapBlock)
    {
        if (GameMemory::TempLoadHeap)
        {
            RedDestroyHeap(GameMemory::TempLoadHeap);
            RedSetTempHeap(-1);
            GameMemory::TempLoadHeap = 0;
        }
        RedFreeToHeap(GameMemory::RunTimeHeap, sTempHeapBlock, 0);
        sTempHeapBlock = 0;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058aff0
int AddCameraShot(lua_State* L)
{
    PblQuaternion qRot;
    qRot._fS = LuaHelper::GetFloat(1);
    qRot._fX = LuaHelper::GetFloat(2);
    qRot._fY = LuaHelper::GetFloat(3);
    qRot._fZ = LuaHelper::GetFloat(4);
    float fX = LuaHelper::GetFloat(5);
    float fY = LuaHelper::GetFloat(6);
    float fZ = LuaHelper::GetFloat(7);
    CameraManager::Instance()->AddCameraShot(qRot, PblVector3(fX, fY, fZ));
    return 0;
}

// FUNCTION: SWBF2 0x0058b0e0
int SetCameraRotation(lua_State* L)
{
    PblQuaternion qRot;
    qRot._fS = LuaHelper::GetFloat(1);
    qRot._fX = LuaHelper::GetFloat(2);
    qRot._fY = LuaHelper::GetFloat(3);
    qRot._fZ = LuaHelper::GetFloat(4);
    CameraManager::Instance()->SetChaseCameraRotation(qRot, 0);
    return 0;
}

// FUNCTION: SWBF2 0x0058b1d0
int SetCameraPosition(lua_State* L)
{
    float fX = LuaHelper::GetFloat(1);
    float fY = LuaHelper::GetFloat(2);
    float fZ = LuaHelper::GetFloat(3);
    CameraManager::Instance()->SetChaseCameraPosition(PblVector3(fX, fY, fZ), 0);
    return 0;
}

// FUNCTION: SWBF2 0x0058b310
int GetMapCameraPosition(lua_State* L)
{
    const PblMatrix& mTarget = CameraManager::Instance()->GetMapCamera(0)->mTarget;
    lua_pushnumber(LuaHelper::mState, mTarget.m[3][0]);
    lua_pushnumber(LuaHelper::mState, mTarget.m[3][1]);
    lua_pushnumber(LuaHelper::mState, mTarget.m[3][2]);
    return 3;
}

// FUNCTION: SWBF2 0x0058b3a0
int SetMapCameraPosition(lua_State* L)
{
    MapCamera* pCamera = CameraManager::Instance()->GetMapCamera(0);
    PblMatrix mMatrix = pCamera->mTarget;
    float fZ = LuaHelper::GetFloat(3);
    float fY = LuaHelper::GetFloat(2);
    float fX = LuaHelper::GetFloat(1);
    mMatrix.m[3][0] = fX;
    mMatrix.m[3][1] = fY;
    mMatrix.m[3][2] = fZ;
    mMatrix.m[3][3] = 1.0f;
    pCamera->SetMatrix(mMatrix);
    return 0;
}

// FUNCTION: SWBF2 0x0058b460
int SetMapCameraOffset(lua_State* L)
{
    float fZ = LuaHelper::GetFloat(4);
    float fY = LuaHelper::GetFloat(3);
    float fX = LuaHelper::GetFloat(2);
    MapCamera::SetOffset((MapCamera::ZoomLevel)LuaHelper::GetInt(1), PblVector3(fX, fY, fZ));
    return 0;
}

// FUNCTION: SWBF2 0x0058b4f0
int SetMapCameraPitch(lua_State* L)
{
    float fPitch = LuaHelper::GetFloat(2);
    MapCamera::SetPitch((MapCamera::ZoomLevel)(unsigned int)luaL_checknumber(LuaHelper::mState, 1), fPitch);
    return 0;
}

// FUNCTION: SWBF2 0x0058b530
int SetMapCameraZoom(lua_State* L)
{
    CameraManager::Instance()->GetMapCamera(0)->SetZoomLevel(
        (MapCamera::ZoomLevel)(unsigned int)luaL_checknumber(LuaHelper::mState, 1));
    return 0;
}

// FUNCTION: SWBF2 0x0058b560
int GetMapCameraZoom(lua_State* L)
{
    MapCamera* pCamera = CameraManager::Instance()->GetMapCamera(0);
    lua_pushnumber(LuaHelper::mState, (float)pCamera->GetZoomLevel());
    lua_pushnumber(LuaHelper::mState, (float)pCamera->GetZoomDir());
    lua_pushnumber(LuaHelper::mState, pCamera->GetZoomRatio());
    return 3;
}

// FUNCTION: SWBF2 0x0058b5d0
int SnapMapCamera(lua_State* L)
{
    CameraManager::Instance()->GetMapCamera(0)->Snap();
    return 0;
}

// FUNCTION: SWBF2 0x0058b5f0
int SetState(lua_State* L)
{
    GameState::SetState(PblHash(LuaHelper::GetString(1)));
    return 0;
}

// FUNCTION: SWBF2 0x0058b620
int SetHistorical(lua_State* L)
{
    SpawnManager::Instance()->SetHistorical(true);
    return 0;
}

// FUNCTION: SWBF2 0x0058b630
int SetTeamName(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    Team::sTeams[iTeam]->SetSide(LuaHelper::GetString(2));
    return 0;
}

// FUNCTION: SWBF2 0x0058b680
int SetTeamAggressiveness(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    Team::sTeams[iTeam]->SetAggressiveness(LuaHelper::GetFloat(2));
    return 0;
}

// FUNCTION: SWBF2 0x0058b6d0
int SetMinFlyHeight(lua_State* L)
{
    AIUtil::gMinFlyHeight = LuaHelper::GetFloat(1);
    return 0;
}

// FUNCTION: SWBF2 0x0058b6f0
int SetMaxFlyHeight(lua_State* L)
{
    AIUtil::gMaxFlyHeight = LuaHelper::GetFloat(1);
    return 0;
}

// FUNCTION: SWBF2 0x0058b710
int SetMinPlayerFlyHeight(lua_State* L)
{
    AIUtil::gMinPlayerFlyHeight = LuaHelper::GetFloat(1);
    return 0;
}

// FUNCTION: SWBF2 0x0058b730
int SetMaxPlayerFlyHeight(lua_State* L)
{
    AIUtil::gMaxPlayerFlyHeight = LuaHelper::GetFloat(1);
    return 0;
}

// FUNCTION: SWBF2 0x0058b750
int SetUberMode(lua_State* L)
{
    AIUtil::sUberMode = LuaHelper::GetFloat(1) != 0.0f;
    return 0;
}

// FUNCTION: SWBF2 0x0058b790
int SetAllowBlindJetJumps(lua_State* L)
{
    AIUtil::gAIAllowBlindJetJumps = LuaHelper::GetFloat(1) != 0.0f;
    return 0;
}

// FUNCTION: SWBF2 0x0058b7d0
int SetStationaryWaitTime(lua_State* L)
{
    AIUtil::gStationaryWaitTime = LuaHelper::GetFloat(1);
    return 0;
}

// FUNCTION: SWBF2 0x0058b7f0
int SetTeamIcon(lua_State* L)
{
    int iArgs = lua_gettop(LuaHelper::mState);
    int iTeam = LuaHelper::GetInt(1);
    const char* pcIcon = LuaHelper::GetString(2);
    const char* pcConquestIcon = iArgs >= 3 ? LuaHelper::GetString(3) : 0;
    const char* pcCTFIcon = iArgs >= 4 ? LuaHelper::GetString(4) : 0;
    Team* pTeam = Team::sTeams[iTeam];
    pTeam->SetIcon(PblHash(pcIcon));
    if (pcConquestIcon)
    {
        pTeam->SetConquestIcon(PblHash(pcConquestIcon));
    }
    if (pcCTFIcon)
    {
        pTeam->SetCTFIcon(PblHash(pcCTFIcon));
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058b8d0
int SetTeamAsEnemy(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    Team::sTeams[iTeam]->SetAffiliation(LuaHelper::GetInt(2), -1);
    return 0;
}

// FUNCTION: SWBF2 0x0058b920
int SetTeamAsNeutral(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    Team::sTeams[iTeam]->SetAffiliation(LuaHelper::GetInt(2), 0);
    return 0;
}

// FUNCTION: SWBF2 0x0058b970
int SetTeamAsFriend(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    Team::sTeams[iTeam]->SetAffiliation(LuaHelper::GetInt(2), 1);
    return 0;
}

// FUNCTION: SWBF2 0x0058b9c0
int SetUnitCount(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    int iCount = LuaHelper::GetInt(2);
    if (AIDebug::OnlyOneAI())
    {
        iCount = iTeam < 3;
    }
    SpawnManager::Instance()->SetUnitCount(iTeam, iCount);
    return 0;
}

// FUNCTION: SWBF2 0x0058ba20
int GetUnitCount(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    if (AIDebug::OnlyOneAI())
    {
        if (iTeam < 3)
        {
            lua_pushnumber(LuaHelper::mState, 1.0f);
        }
        else
        {
            lua_pushnumber(LuaHelper::mState, 0.0f);
        }
    }
    else
    {
        lua_pushnumber(LuaHelper::mState, (float)SpawnManager::Instance()->GetUnitCount(iTeam));
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058ba90
int SetReinforcementCount(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    int iCount = LuaHelper::GetInt(2);
    if (iCount < 0)
    {
        iCount = 0x7fffffff;
    }
    else
    {
        unsigned int uBit = 1 << iTeam;
        if (!(Lua_Callbacks::gTeamSetReinforcementMult & uBit))
        {
            const char* pcMode = strrchr(GameLoop::mMissionScript, '_');
            if (pcMode)
            {
                if (!_stricmp(pcMode, "_con"))
                {
                    iCount = iCount * gCONReinforcementMult[gCurRules] / 100;
                }
                else if (!_stricmp(pcMode, "_dm") || !_stricmp(pcMode, "_tdm"))
                {
                    iCount = iCount * gDMReinforcementMult[gCurRules] / 100;
                }
            }
            Lua_Callbacks::gTeamSetReinforcementMult |= uBit;
        }
    }
    Team::sTeams[iTeam]->mMaxReinforcements = iCount;
    Team::sTeams[iTeam]->SetCurReinforcements(iCount);
    return 0;
}

// FUNCTION: SWBF2 0x0058bb90
int GetReinforcementCount(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    lua_pushnumber(LuaHelper::mState, (float)Team::sTeams[iTeam]->GetCurReinforcements());
    return 1;
}

// FUNCTION: SWBF2 0x0058bbe0
int AddReinforcements(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    Team::sTeams[iTeam]->AddReinforcements(LuaHelper::GetInt(2));
    return 0;
}

// FUNCTION: SWBF2 0x0058bc30
int SetTeamPoints(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    Team::sTeams[iTeam]->SetPoints(LuaHelper::GetInt(2));
    return 0;
}

// FUNCTION: SWBF2 0x0058bc80
int GetTeamPoints(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    lua_pushnumber(LuaHelper::mState, (float)Team::sTeams[iTeam]->GetPoints());
    return 1;
}

// FUNCTION: SWBF2 0x0058bcd0
int AddTeamPoints(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    Team::sTeams[iTeam]->AddPoints(LuaHelper::GetInt(2));
    return 0;
}

// FUNCTION: SWBF2 0x0058bd20
int AddFlagCapturePoints(lua_State* L)
{
    Character* pCharacter = Character::GetPtr(LuaHelper::GetInt(1));
    if (pCharacter)
    {
        Character::GetPlayerStats(pCharacter).AddFlagCapture();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058bd70
int AddAssaultDestroyPoints(lua_State* L)
{
    Character* pCharacter = Character::GetPtr(LuaHelper::GetInt(1));
    if (pCharacter)
    {
        Character::GetPlayerStats(pCharacter).PointsProcess(0x20, 0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058bdd0
int AddSpaceAssaultDestroyPoints(lua_State* L)
{
    if (!lua_isnoneornil(LuaHelper::mState, 1))
    {
        int iCharacter = LuaHelper::GetInt(1);
        const char* pcSystem = LuaHelper::GetString(2);
        Character* pCharacter = Character::GetPtr(iCharacter);
        if (pCharacter)
        {
            Character::GetPlayerStats(pCharacter).PointsProcess(0x20, PblHash(pcSystem));
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058be60
int GetTeamFactionId(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    lua_pushnumber(LuaHelper::mState, (float)Team::sTeams[iTeam]->GetSide());
    return 1;
}

// FUNCTION: SWBF2 0x0058beb0
int AddUnitClass(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    const char* pcClass = LuaHelper::GetString(2);
    int iMinUnits = lua_isnumber(LuaHelper::mState, 3) ? LuaHelper::GetInt(3) : 0;
    int iMaxUnits = lua_isnumber(LuaHelper::mState, 4) ? LuaHelper::GetInt(4) : 0x7fffffff;
    if (AIDebug::OnlyOneAI())
    {
        if (iTeam < 3 && !Team::sTeams[iTeam]->GetClassCount())
        {
            iMinUnits = 1;
            iMaxUnits = 1;
        }
        else
        {
            iMinUnits = 0;
            iMaxUnits = 0;
        }
    }
    Team* pTeam = Team::sTeams[iTeam];
    int iSlot = -1;
    if (pTeam)
    {
        GameObjectClass* pClass = pTeam->FindClass(pcClass);
        if (pClass)
        {
            pTeam->SetClassUnitCounts(pClass, iMinUnits, iMaxUnits);
            iSlot = pTeam->GetClassIndex(pClass);
        }
        else
        {
            iSlot = pTeam->GetClassCount();
            pTeam->AddClass(pcClass, iMinUnits, iMaxUnits);
        }
    }
    lua_pushnumber(LuaHelper::mState, (float)iSlot);
    return 1;
}

// FUNCTION: SWBF2 0x0058bfe0
int SetHeroClass(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    const char* pcClass = LuaHelper::GetString(2);
    if (!AIDebug::OnlyOneAI())
    {
        Team::sTeams[iTeam]->AddSpecialClass(PblHash(pcClass));
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058c040
int SetHeroUnlockRule(lua_State* L)
{
    return 0;
}

// FUNCTION: SWBF2 0x0058c050
int SetHeroTeamRule(lua_State* L)
{
    if (!NetGame::IsEnabled())
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
        if (!pProfile || pProfile->mGameOptions.mbHerosEnabled)
        {
            switch (PblHash(LuaHelper::GetString(1)))
            {
            case 0x9ec703f1:
                netHeroRuleTeam = 4;
                break;
            case 0xba2719ef:
                netHeroRuleTeam = 1;
                break;
            case 0xea81eea2:
                netHeroRuleTeam = 3;
                break;
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058c0e0
int SetHeroPlayerRule(lua_State* L)
{
    if (!NetGame::IsEnabled())
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
        if (!pProfile || pProfile->mGameOptions.mbHerosEnabled)
        {
            switch (PblHash(LuaHelper::GetString(1)))
            {
            case 0x082ba9d5:
                netHeroRulePlayer = 1;
                break;
            case 0x5fb6bb4f:
                netHeroRulePlayer = 2;
                break;
            case 0x2471dd69:
                netHeroRulePlayer = 3;
                break;
            case 0xc01e36e1:
                netHeroRulePlayer = 4;
                break;
            case 0xa97b828b:
                netHeroRulePlayer = 5;
                break;
            case 0x76bdd7f5:
                netHeroRulePlayer = 6;
                break;
            case 0x144f0c62:
                netHeroRulePlayer = 7;
                break;
            case 0xe614fa73:
                netHeroRulePlayer = 8;
                break;
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058c200
int SetHeroRespawnRule(lua_State* L)
{
    if (!NetGame::IsEnabled())
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
        if (!pProfile || pProfile->mGameOptions.mbHerosEnabled)
        {
            const char* pcRule = LuaHelper::GetString(1);
            if (PblHash(pcRule) == 0x0ac95089)
            {
                netHeroRuleRespawn = 1;
            }
            else
            {
                netHeroRuleRespawn = 2;
                netHeroRuleRespawnVal = atoi(pcRule);
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058c290
int EnableSPHeroRules(lua_State* L)
{
    return 0;
}

// FUNCTION: SWBF2 0x0058c2a0
int EnableSPScriptedHeroes(lua_State* L)
{
    if (!NetGame::IsEnabled())
    {
        netHeroesEnabled = true;
        netHeroRuleUnlock = 7;
        netHeroRuleRespawn = 1;
        netHeroRuleRespawnVal = -1;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058c2f0
int UnlockHeroForTeam(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    bool bNet = NetGame::IsEnabled();
    netHeroUnlocked[iTeam] = true;
    netHeroFirstUnlocked[iTeam] = true;
    if (!bNet)
    {
        HeroMessageDisplay::PlayHeroSoundUnlock(iTeam);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058c350
int AcceptHero(lua_State* L)
{
    Character* pCharacter = Character::GetPtr(LuaHelper::GetInt(1));
    if (pCharacter)
    {
        NetGame::AcceptHero(pCharacter->mTeamNumber, pCharacter->mPlayerId);
        if (pCharacter->mVehicle)
        {
            pCharacter->mUnit->ExitVehicle(true, true);
        }
        if (pCharacter->mRemote)
        {
            pCharacter->mUnit->ExitRemote();
        }
        if (pCharacter->mUnit && pCharacter->mUnit->GetGameObject())
        {
            ((Damageable*)((char*)pCharacter->mUnit->GetGameObject() + 320))->InstantVanish();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058c420
int ShowAllUnitsOnMinimap(lua_State* L)
{
    gMapDisplayAll = lua_toboolean(LuaHelper::mState, 1) != 0;
    return 0;
}

// FUNCTION: SWBF2 0x0058c440
int ShowTeamPoints(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    bool bShow = lua_toboolean(LuaHelper::mState, 2) != 0;
    HUD::GameEvents::ShowTeamPoints(iTeam, bShow);
    GameLoop::SetShowTeamPoints(iTeam, bShow);
    return 0;
}

// FUNCTION: SWBF2 0x0058c490
int ActivateBonus(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    const char* pcBonus = LuaHelper::GetString(2);
    bool bActive = lua_gettop(LuaHelper::mState) < 3 || lua_toboolean(LuaHelper::mState, 3) != 0;
    BonusManager::Instance()->SetTeamBonusActive(iTeam, PblHash(pcBonus), bActive);
    return 0;
}

// FUNCTION: SWBF2 0x0058c510
int SetBleedRate(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    Team::sTeams[iTeam]->SetBleedRate(LuaHelper::GetFloat(2));
    return 0;
}

// FUNCTION: SWBF2 0x0058c560
int GetBleedRate(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    Team* pTeam = Team::sTeams[iTeam];
    lua_pushnumber(LuaHelper::mState, pTeam ? pTeam->mBleedRate : 0.0f);
    return 1;
}

// FUNCTION: SWBF2 0x0058c8a0
int SetBirdFlockMinHeight(lua_State* L)
{
    BirdParticleManager::sFlockMinHeight = (float)luaL_checknumber(LuaHelper::mState, 1);
    return 0;
}

// FUNCTION: SWBF2 0x0058d680
int GetWorldFilename(lua_State* L)
{
    lua_pushstring(LuaHelper::mState, GameLoop::mWorldFileName);
    return 1;
}

// FUNCTION: SWBF2 0x0058c610
int SetSpawnDelay(lua_State* L)
{
    float fCycleDelay = (float)luaL_checknumber(LuaHelper::mState, 1);
    float fSlotDelay = (float)luaL_checknumber(LuaHelper::mState, 2);
    if (NetGame::IsEnabled())
    {
        fCycleDelay = 15.0f;
    }
    for (int iTeam = 1; iTeam <= 2; ++iTeam)
    {
        SpawnManager::sInstance->SetSpawnDelay(fCycleDelay, fSlotDelay, iTeam);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058c720
int SetDenseEnvironment(lua_State* L)
{
    bool bDense = false;
    if (lua_isstring(LuaHelper::mState, 1))
    {
        const char* pcValue = LuaHelper::GetString(1);
        if (!_stricmp(pcValue, "true"))
        {
            bDense = true;
        }
        else if (!_stricmp(pcValue, "false"))
        {
            bDense = false;
        }
    }
    else
    {
        bDense = lua_toboolean(LuaHelper::mState, 1) != 0;
    }
    CombatUtil::SetDenseEnvironment(bDense);
    if (!NetGame::IsEnabled() &&
        Hash16(PlayerProfile::GetProfile(0)->GetName()) == 0x106408EBu &&
        GameLoop::mMissionScriptHash == 0x56D4EA52u)
    {
        unsigned int uiClassHash = 0xD85CB472u;
        const char* pcClass = "snw_inf_wampa";
        LoadUtil::ReadDataFile("SIDE\\snw.lvl", 0, &uiClassHash, 1, &pcClass);
        Team::sTeams[1]->AddClass(pcClass, 0, 0x7FFFFFFF);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058c840
int SetDefenderSnipeRange(lua_State* L)
{
    return 0;
}

// FUNCTION: SWBF2 0x0058c8e0
int AddDeathRegion(lua_State* L)
{
    TriggerRegionManager::sInstance->AddDeathRegion(LuaHelper::GetString(1));
    return 0;
}

// FUNCTION: SWBF2 0x0058cd50
int SetMemoryPoolSize(lua_State* L)
{
    int iArgs = lua_gettop(LuaHelper::mState);
    const char* pcName = LuaHelper::GetString(1);
    int iCount = LuaHelper::GetInt(2);
    MemoryPool* pPool = MemoryPool::Find(pcName);
    if (pPool)
    {
        if (iArgs >= 3)
        {
            iCount = LuaHelper::GetInt(3);
        }
        else if (!_stricmp(pcName, "EntityLight"))
        {
            iCount += 60;
        }
        pPool->SetCount(iCount);
        if (pPool == &Weapon::sMemoryPool)
        {
            if (AmmoCounter::sMemoryPool.GetCount() < Weapon::sMemoryPool.GetCount())
            {
                AmmoCounter::sMemoryPool.SetCount(Weapon::sMemoryPool.GetCount());
            }
            if (EnergyBar::sMemoryPool.GetCount() < Weapon::sMemoryPool.GetCount())
            {
                EnergyBar::sMemoryPool.SetCount(Weapon::sMemoryPool.GetCount());
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058ce20
int ClearWalkers(lua_State* L)
{
    MemoryPool* pWalkerPool = MemoryPool::Find("EntityWalker");
    MemoryPool* pLegPairPool = MemoryPool::Find("WalkerLegPair");
    MemoryPool* pBlendPool = MemoryPool::Find("WalkerBlend");
    MemoryPool* pBlendUnitPool = MemoryPool::Find("WalkerBlendUnit");
    MemoryPool* pDroidekaPool = MemoryPool::Find("EntityDroideka");
    pWalkerPool->SetCount(0);
    pLegPairPool->SetCount(0);
    pBlendPool->SetCount(0);
    pBlendUnitPool->SetCount(0);
    pDroidekaPool->SetCount(0);
    return 0;
}

// FUNCTION: SWBF2 0x0058ceb0
int AddWalkerType(lua_State* L)
{
    MemoryPool* pWalkerPool = MemoryPool::Find("EntityWalker");
    MemoryPool* pLegPairPool = MemoryPool::Find("WalkerLegPair");
    MemoryPool* pBlendPool = MemoryPool::Find("WalkerBlend");
    MemoryPool* pBlendUnitPool = MemoryPool::Find("WalkerBlendUnit");
    MemoryPool* pDroidekaPool = MemoryPool::Find("EntityDroideka");
    int iLegPairs = LuaHelper::GetInt(1);
    int iCount = LuaHelper::GetInt(2);
    int iBlends = (iLegPairs + 1) * iCount;
    if (iLegPairs > 0)
    {
        pWalkerPool->SetCount(pWalkerPool->GetCount() + iCount);
        pLegPairPool->SetCount(pLegPairPool->GetCount() + iCount * iLegPairs);
        pBlendPool->SetCount(pBlendPool->GetCount() + iBlends);
        pBlendUnitPool->SetCount(pBlendUnitPool->GetCount() + iBlends * 3);
    }
    else
    {
        pDroidekaPool->SetCount(pDroidekaPool->GetCount() + iCount);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058f190
int GetNumTeamMembersAlive(lua_State* L)
{
    Team* pTeam = Team::GetPtr(LuaHelper::GetInt(1));
    if (pTeam)
    {
        lua_pushnumber(LuaHelper::mState, (float)pTeam->GetNumMembersAlive());
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058f730
int GetTeamClassCount(lua_State* L)
{
    Team* pTeam = Team::sTeams[LuaHelper::GetInt(1)];
    lua_pushnumber(LuaHelper::mState, pTeam ? (float)pTeam->GetClassCount() : 0.0f);
    return 1;
}

// FUNCTION: SWBF2 0x0058f820
int GetCharacterTeam(lua_State* L)
{
    Character* pChr = Character::GetPtr(LuaHelper::GetInt(1));
    if (pChr)
    {
        lua_pushnumber(LuaHelper::mState, (float)pChr->GetTeamNumber());
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058ff40
int GetCommandPostBleedValue(lua_State* L)
{
    CommandPost* pPost = CommandPost::FindPostByName(LuaHelper::GetString(1));
    int iTeam = LuaHelper::GetInt(2);
    if (pPost)
    {
        lua_pushnumber(LuaHelper::mState, (float)pPost->GetBleedValue(iTeam));
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058d090
int SetGroundFlyerMap(lua_State* L)
{
    AIUtil::gGroundFlyerMap = (int)luaL_checknumber(LuaHelper::mState, 1) != 0;
    return 0;
}

// FUNCTION: SWBF2 0x0058d1d0
int AddMissionObjective(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    const char* pcName = LuaHelper::GetString(2);
    const char* pcText = luaL_optstring(LuaHelper::mState, 3, 0);
    ObjectiveManager::AddObjective(PblHash(pcName), PblHash(pcText), iTeam);
    return 0;
}

// FUNCTION: SWBF2 0x0058d240
int ActivateObjective(lua_State* L)
{
    ObjectiveManager::SetObjectiveState(PblHash(LuaHelper::GetString(1)), 2);
    return 0;
}

// FUNCTION: SWBF2 0x0058d280
int CompleteObjective(lua_State* L)
{
    ObjectiveManager::SetObjectiveState(PblHash(LuaHelper::GetString(1)), 3);
    return 0;
}

// FUNCTION: SWBF2 0x0058d2f0
int SetFlagGameplayType(lua_State* L)
{
    unsigned int uType = PblHash(LuaHelper::GetString(1));
    if (uType == 0x216b8b26)
    {
        FlagItem::SetGameplayType(FlagItem::FLAG_GAMEPLAY_1FLAG);
    }
    else if (uType == 0x9db77da7)
    {
        FlagItem::SetGameplayType(FlagItem::FLAG_GAMEPLAY_2FLAG);
    }
    else
    {
        FlagItem::SetGameplayType(uType == 0xbed023b3 ? FlagItem::FLAG_GAMEPLAY_CAMPAIGN
                                                      : FlagItem::FLAG_GAMEPLAY_NONE);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058e030
int FindTimer(lua_State* L)
{
    if (GetTimer(L, 1))
    {
        lua_pushlightuserdata(LuaHelper::mState, GetTimer(L, 1));
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058e070
int CreateTimer(lua_State* L)
{
    const char* pcName = LuaHelper::GetString(1);
    Timer* pTimer = Timer::Find(pcName);
    if (!pTimer)
    {
        if (Timer::sMemoryPool.IsFull())
        {
            lua_pushnil(LuaHelper::mState);
            return 1;
        }
        pTimer = new Timer(pcName);
    }
    lua_pushlightuserdata(LuaHelper::mState, pTimer);
    return 1;
}

// FUNCTION: SWBF2 0x0058e130
int DestroyTimer(lua_State* L)
{
    Timer* pTimer = GetTimer(L, 1);
    if (pTimer)
    {
        pTimer->Stop();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058e150
int StartTimer(lua_State* L)
{
    Timer* pTimer = GetTimer(L, 1);
    if (pTimer)
    {
        pTimer->Start();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058e170
int StopTimer(lua_State* L)
{
    Timer* pTimer = GetTimer(L, 1);
    if (pTimer)
    {
        pTimer->Stop();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058e190
int SetTimerValue(lua_State* L)
{
    Timer* pTimer = GetTimer(L, 1);
    if (pTimer)
    {
        pTimer->SetTime(LuaHelper::GetFloat(2));
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058e1c0
int GetTimerValue(lua_State* L)
{
    Timer* pTimer = GetTimer(L, 1);
    if (pTimer)
    {
        lua_pushnumber(LuaHelper::mState, pTimer->GetTime());
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058e200
int SetTimerRate(lua_State* L)
{
    Timer* pTimer = GetTimer(L, 1);
    if (pTimer)
    {
        pTimer->SetRate(LuaHelper::GetFloat(2));
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058e2b0
int ShowTimer(lua_State* L)
{
    if (NetGame::IsEnabled() && !NetGame::IsHost())
    {
        return 0;
    }
    if (GetTimer(L, 1))
    {
        HUD::GameEvents::SetObjectiveTimer(GetTimer(L, 1));
    }
    else
    {
        HUD::GameEvents::SetObjectiveTimer(0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058e380
int SetDefeatTimer(lua_State* L)
{
    Timer* pTimer = GetTimer(L, 1);
    MissionTimer::SetDefeat(pTimer, (int)LuaHelper::GetFloat(2, 0.0f));
    return 0;
}

// FUNCTION: SWBF2 0x0058e400
int SetVictoryTimer(lua_State* L)
{
    Timer* pTimer = GetTimer(L, 1);
    MissionTimer::SetVictory(pTimer, (int)LuaHelper::GetFloat(2, 0.0f));
    return 0;
}

// FUNCTION: SWBF2 0x00590910
int MissionVictory(lua_State* L)
{
    unsigned int uWinningTeams = 0;
    if (lua_isnumber(LuaHelper::mState, 1))
    {
        float fTeam = (float)luaL_checknumber(LuaHelper::mState, 1);
        uWinningTeams = 1u << (int)fTeam;
    }
    else if (lua_type(LuaHelper::mState, 1) == LUA_TTABLE)
    {
        int iCount = luaL_getn(LuaHelper::mState, 1);
        for (int i = 1; i <= iCount; ++i)
        {
            lua_rawgeti(LuaHelper::mState, 1, i);
            if (lua_isnumber(LuaHelper::mState, -1))
            {
                float fTeam = (float)luaL_checknumber(LuaHelper::mState, -1);
                uWinningTeams |= 1u << (int)fTeam;
            }
        }
    }
    Team::sWinningTeams = uWinningTeams;
    return 0;
}

// FUNCTION: SWBF2 0x005909f0
int MissionDefeat(lua_State* L)
{
    unsigned int uLosingTeams = 0;
    if (lua_isnumber(LuaHelper::mState, 1))
    {
        float fTeam = (float)luaL_checknumber(LuaHelper::mState, 1);
        uLosingTeams = 1u << (int)fTeam;
    }
    else if (lua_type(LuaHelper::mState, 1) == LUA_TTABLE)
    {
        int iCount = luaL_getn(LuaHelper::mState, 1);
        for (int i = 1; i <= iCount; ++i)
        {
            lua_rawgeti(LuaHelper::mState, 1, i);
            if (lua_isnumber(LuaHelper::mState, -1))
            {
                float fTeam = (float)luaL_checknumber(LuaHelper::mState, -1);
                uLosingTeams |= 1u << (int)fTeam;
            }
        }
    }
    Team::sWinningTeams = ~uLosingTeams;
    return 0;
}

// FUNCTION: SWBF2 0x0058df10
int MapHideCommandPosts(lua_State* L)
{
    bool bHide = lua_gettop(LuaHelper::mState) <= 0 ||
                 lua_toboolean(LuaHelper::mState, 1) != 0;
    HUD::Event(HUD::GameEvents::gPlayerEvents[0].mapHideCPs, bHide).Send();
    return 0;
}

// FUNCTION: SWBF2 0x0058e4c0
int SetObjectTeam(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    if (pObject)
    {
        pObject->SetTeam((unsigned int)luaL_checknumber(LuaHelper::mState, 2));
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058e4f0
int GetObjectTeam(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    if (pObject)
    {
        lua_pushnumber(LuaHelper::mState, (float)pObject->GetTeam());
    }
    else
    {
        lua_pushnumber(LuaHelper::mState, 0.0f);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058e550
int IsObjectAlive(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    if (pObject)
    {
        lua_pushboolean(LuaHelper::mState, pObject->IsAlive());
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058e710
int KillObject(lua_State* L)
{
    if (sMissionActive)
    {
        GameObject* pObject = _GetEntity<GameObject>(L, 1);
        if (pObject)
        {
            Damageable* pDamageable = pObject;
            pDamageable->SetCurHealth(0.0f, true, 0);
            pDamageable->Kill();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058e7b0
int GetEntityName(lua_State* L)
{
    EntityEx* pEntity = _GetEntity<EntityEx>(L, 1);
    if (pEntity)
    {
        lua_pushstring(LuaHelper::mState, StringDB::Find(pEntity->mId));
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058ea50
int SetProperty(lua_State* L)
{
    EntityEx* pEntity = _GetEntity<EntityEx>(L, 1);
    if (pEntity)
    {
        const char* pcProperty = LuaHelper::GetString(2);
        const char* pcValue;
        if (lua_type(LuaHelper::mState, 3) == LUA_TBOOLEAN)
        {
            pcValue = lua_toboolean(LuaHelper::mState, 3) ? "1" : "0";
        }
        else
        {
            pcValue = LuaHelper::GetString(3);
        }
        pEntity->SetProperty(PblHash(pcProperty), pcValue);
        GameObject* pObject = pEntity->GetGameObject();
        GameObject* pClient = NetGame::GetClientNDObject(pObject);
        if (pClient && pClient != pObject)
        {
            pClient->GetEntity()->SetProperty(PblHash(pcProperty), pcValue);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00590070
int ClearAIGoals(lua_State* L)
{
    int iTeam = LuaHelper::GetInt(1);
    if (iTeam >= 0 && iTeam < 8)
    {
        AI::AIGoalManager::Instance()->ClearAllGoals(iTeam);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005900b0
int DeleteAIGoal(lua_State* L)
{
    if (lua_type(LuaHelper::mState, 1) == LUA_TLIGHTUSERDATA)
    {
        AI::AIGoal* pGoal = (AI::AIGoal*)lua_topointer(LuaHelper::mState, 1);
        if (pGoal)
        {
            pGoal->UnAssignAll();
            AI::AIGoalManager::Instance()->DeleteGoal(pGoal);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00590100
int AddAIGoal(lua_State* L)
{
    int iArgs = lua_gettop(LuaHelper::mState);
    if (iArgs != 3 && iArgs != 4 && iArgs != 5)
    {
        return 0;
    }
    int iTeam = LuaHelper::GetInt(1);
    if (iTeam < 0 || iTeam > 7)
    {
        return 0;
    }

    AI::GoalTarget target;
    target.mType = AI::GTT_NONE;
    const char* pcGoal = LuaHelper::GetString(2);
    AI::AIGoalType eType = AI::AIGOAL_NONE;

    if (lua_type(LuaHelper::mState, 4) == LUA_TLIGHTUSERDATA || lua_isstring(LuaHelper::mState, 4))
    {
        const Entity* pEntity = _GetEntity<const Entity>(L, 4);
        if (pEntity && pEntity->IsRtti(GameObject::GetRtti()))
        {
            target.mType = AI::GTT_GAMEOBJECT;
            target.mGameObj = (const GameObject*)pEntity;
        }
    }
    if (lua_isnumber(LuaHelper::mState, 4) && target.mType == AI::GTT_NONE)
    {
        int iCharacter = (int)LuaHelper::GetFloat(4, -1.0f);
        if (iCharacter >= 0 && iCharacter < Character::sCharacterLimit)
        {
            target.mType = AI::GTT_CHARACTER;
            target.mChr = Character::GetPtr(iCharacter);
        }
    }

    switch (PblHash(pcGoal))
    {
    case 0x5aabb29b:
        eType = AI::AIGOAL_CONQUEST;
        break;
    case 0xc45b6745:
        eType = AI::AIGOAL_DESTROY;
        break;
    case 0x4fed19c1:
        eType = AI::AIGOAL_DEFEND;
        break;
    case 0x73f5632e:
        eType = AI::AIGOAL_DEATHMATCH;
        break;
    case 0x9a8b1b9e:
    {
        PblRegion* pRegion = PblRegion::Find(PblHash(LuaHelper::GetString(4)));
        if (pRegion)
        {
            eType = AI::AIGOAL_CTFOFFENSE;
            target.mType = AI::GTT_FLAG;
            target.mPos = pRegion->mMatrix.GetTranslation();
            target.mGameObj = iArgs >= 5 ? (const GameObject*)lua_topointer(LuaHelper::mState, 5) : 0;
        }
        break;
    }
    case 0xe12bcbe0:
        eType = AI::AIGOAL_CTFDEFENSE;
        target.mType = AI::GTT_FLAG;
        break;
    case 0x5b6c4f70:
        eType = AI::AIGOAL_FOLLOW;
        break;
    default:
        return 0;
    }

    float fWeight = LuaHelper::GetFloat(3);
    lua_pushlightuserdata(LuaHelper::mState,
                          AI::AIGoalManager::Instance()->AddGoal((unsigned char)iTeam, eType, fWeight, &target));
    return 1;
}

// FUNCTION: SWBF2 0x00590360
static void GetPlanningGraphArcsFromArgs(unsigned int* puArcs, int& iNumArcs, unsigned int* puTeams, int& iNumTeams)
{
    int iArgs = lua_gettop(LuaHelper::mState);
    iNumArcs = 0;
    iNumTeams = 0;
    for (int i = 1; i <= iArgs; ++i)
    {
        if (!lua_isstring(LuaHelper::mState, i) && !lua_isnumber(LuaHelper::mState, i))
        {
            iNumArcs = 0;
            return;
        }
        if (lua_isnumber(LuaHelper::mState, i))
        {
            int iTeam = LuaHelper::GetInt(i);
            if (iTeam < 1 || iTeam > 8)
            {
                iNumTeams = 0;
                iNumArcs = 0;
                return;
            }
            puTeams[iNumTeams++] = iTeam;
        }
        else if (lua_isstring(LuaHelper::mState, i))
        {
            unsigned int uName = PblHash(LuaHelper::GetString(i));
            if (!ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1)->IsDynamicArc(uName))
            {
                iNumTeams = 0;
                iNumArcs = 0;
                return;
            }
            puArcs[iNumArcs++] = uName;
        }
    }
}

// FUNCTION: SWBF2 0x005904f0
int BlockPlanningGraphArcs(lua_State* L)
{
    unsigned int auArcs[32];
    unsigned int auTeams[32];
    int iNumArcs = 0;
    int iNumTeams = 0;
    GetPlanningGraphArcsFromArgs(auArcs, iNumArcs, auTeams, iNumTeams);
    if (iNumArcs || iNumTeams)
    {
        ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1)
            ->BlockDynamicArcs(auArcs, iNumArcs, auTeams, iNumTeams);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00590560
int UnblockPlanningGraphArcs(lua_State* L)
{
    unsigned int auArcs[32];
    unsigned int auTeams[32];
    int iNumArcs = 0;
    int iNumTeams = 0;
    GetPlanningGraphArcsFromArgs(auArcs, iNumArcs, auTeams, iNumTeams);
    if (iNumArcs || iNumTeams)
    {
        ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1)
            ->UnblockDynamicArcs(auArcs, iNumArcs, auTeams, iNumTeams);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005905d0
static void DynamicBarriers(int iMode)
{
    int iArgs = lua_gettop(LuaHelper::mState);
    for (int i = 1; i <= iArgs; ++i)
    {
        if (!lua_isstring(LuaHelper::mState, i))
        {
            break;
        }
        Obstacle* pObstacle = PathFinder::FindBarrierByName(PblHash(LuaHelper::GetString(i)));
        if (pObstacle)
        {
            if (iMode == 2)
            {
                pObstacle->ToggleEnabled();
            }
            else
            {
                pObstacle->SetEnabled(iMode != 0);
            }
        }
    }
}

// FUNCTION: SWBF2 0x00590680
int DisableBarriers(lua_State* L)
{
    DynamicBarriers(0);
    return 0;
}

// FUNCTION: SWBF2 0x005907d0
int SetAIDamageThreshold(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    if (pObject)
    {
        pObject->SetAIDamageThreshold(LuaHelper::GetFloat(2));
    }
    return 0;
}

// FUNCTION: SWBF2 0x00591010
void InitMission()
{
    for (int i = 0; i < 220; ++i)
    {
        LuaHelper::Register(sMissionCallbacks[i].pcName, sMissionCallbacks[i].pfnCallback, 0);
    }
}

}

// FUNCTION: SWBF2 0x00446e40
FLEffectClass* FLEffect::Get(unsigned int uHash)
{
    return s_EffectClasses.Find(uHash);
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x0058c830
int SetAttackerSnipeRange(lua_State*)
{
    return 0;
}

// FUNCTION: SWBF2 0x0058d790
int ShowPopup(lua_State*)
{
    return 0;
}

// FUNCTION: SWBF2 0x0058d660
int IsCampaign(lua_State* L)
{
    lua_pushboolean(L, GameLoop::mbHistoricalRules);
    return 1;
}

// FUNCTION: SWBF2 0x0058d030
int AISnipeSuitabilityDist(lua_State* L)
{
    float fDistance = (float)luaL_checknumber(L, 1);
    if (fDistance < 0.0f)
    {
        fDistance = 0.0f;
    }
    else if (fDistance > 100.0f)
    {
        fDistance = 100.0f;
    }
    AIUtil::gSnipeSuitabilityDist = fDistance;
    return 0;
}

// FUNCTION: SWBF2 0x0058d630
int SpaceAssaultGetScoreLimit(lua_State* L)
{
    lua_pushnumber(L, (float)gSpaceAssaultScoreLimit[gCurRules]);
    return 1;
}

// FUNCTION: SWBF2 0x0058e440
int GetObjectPtr(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    if (pObject)
    {
        lua_pushlightuserdata(L, pObject);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058e480
int ActivateObject(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    if (pObject)
    {
        pObject->ActivatePhysics();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058e4a0
int DeactivateObject(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    if (pObject)
    {
        pObject->DeactivatePhysics();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058e590
int GetObjectHealth(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    Damageable* pDamage = pObject;
    lua_pushnumber(L, pDamage ? pDamage->mCurHealth : 0.0f);
    lua_pushnumber(L, pDamage ? pDamage->mMaxHealth : 0.0f);
    lua_pushnumber(L, pDamage ? pDamage->mAddHealth : 0.0f);
    return 3;
}

// FUNCTION: SWBF2 0x0058e630
int GetObjectShield(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    Damageable* pDamage = pObject;
    lua_pushnumber(L, pDamage ? pDamage->mCurShield : 0.0f);
    lua_pushnumber(L, pDamage ? pDamage->mMaxShield : 0.0f);
    lua_pushnumber(L, pDamage ? pDamage->mAddShield : 0.0f);
    return 3;
}

// FUNCTION: SWBF2 0x0058e770
int GetEntityPtr(lua_State* L)
{
    EntityEx* pEntity = _GetEntity<EntityEx>(L, 1);
    if (pEntity)
    {
        lua_pushlightuserdata(L, pEntity);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058e7f0
int GetEntityClass(lua_State* L)
{
    EntityEx* pEntity = _GetEntity<EntityEx>(L, 1);
    if (pEntity)
    {
        lua_pushlightuserdata(L, pEntity->GetEntityClass());
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058c850
int SetStayInTurrets(lua_State* L)
{
    float fEnabled = (float)luaL_checknumber(L, 1);
    AIUtil::gStayInTurrets = fEnabled != 0.0f;
    if (AIUtil::gStayInTurrets)
    {
        AIUtil::gStationaryWaitTime = 10000.0f;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058d0e0
int SetAIDifficulty(lua_State* L)
{
    int iCount = lua_gettop(L);
    if (iCount != 2 && iCount != 3)
    {
        return 0;
    }
    int iPlayer = (int)(float)luaL_checknumber(L, 1);
    int iEnemy = (int)(float)luaL_checknumber(L, 2);
    if (iCount == 3)
    {
        switch (PblHash(luaL_checkstring(L, 3)))
        {
        case 0x05537a1b: AIDifficulty::SetAILuaModifier(iPlayer, iEnemy, 1); return 0;
        case 0x35afd776: AIDifficulty::SetAILuaModifier(iPlayer, iEnemy, 2); return 0;
        case 0xe4c35bdc: AIDifficulty::SetAILuaModifier(iPlayer, iEnemy, 3); return 0;
        }
    }
    for (int i = 1; i <= 3; ++i)
    {
        AIDifficulty::SetAILuaModifier(iPlayer, iEnemy, i);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058c9d0
int SetNumFishTypes(lua_State* L)
{
    FishParticleManager::SetNumFishTypes((unsigned int)(float)luaL_checknumber(L, 1));
    return 0;
}

// FUNCTION: SWBF2 0x0058c9f0
int SetFishType(lua_State* L)
{
    int iType = (int)(float)luaL_checknumber(L, 1);
    float fSize = (float)luaL_checknumber(L, 2);
    const char* pcTexture = luaL_checklstring(L, 3, 0);
    FishParticleManager::SetFishType(iType, fSize, pcTexture);
    return 0;
}

// FUNCTION: SWBF2 0x0058c910
int AddLandingRegion(lua_State* L)
{
    TriggerRegionManager::sInstance->AddLandingRegion(luaL_checklstring(L, 1, 0));
    return 0;
}

// FUNCTION: SWBF2 0x0058c940
int RemoveRegion(lua_State* L)
{
    TriggerRegionManager::sInstance->RemoveRegion(luaL_checklstring(L, 1, 0));
    return 0;
}

// FUNCTION: SWBF2 0x0058c690
int SetSpawnDelayTeam(lua_State* L)
{
    float fCycleDelay = (float)luaL_checknumber(L, 1);
    float fSlotDelay = (float)luaL_checknumber(L, 2);
    int iTeam = (int)(float)luaL_checknumber(L, 3);
    if (NetGame::IsEnabled() && (iTeam == 1 || iTeam == 2))
    {
        fCycleDelay = 15.0f;
    }
    SpawnManager::sInstance->SetSpawnDelay(fCycleDelay, fSlotDelay, iTeam);
    return 0;
}

// FUNCTION: SWBF2 0x0058f6e0
int GetTeamSize(lua_State* L)
{
    int iTeam = (int)(float)luaL_checknumber(L, 1);
    lua_pushnumber(L, (float)Team::sTeams[iTeam]->mMemberCount);
    return 1;
}

// FUNCTION: SWBF2 0x0058fa40
int AllowAISpawn(lua_State* L)
{
    int iTeam = (int)(float)luaL_checknumber(L, 1);
    Team::sTeams[iTeam]->mbAllowAISpawn = lua_toboolean(L, 2) != 0;
    return 0;
}

// FUNCTION: SWBF2 0x0058ed50
int EntityFlyerTakeOff(lua_State* L)
{
    EntityEx* pEntity = _GetEntity<EntityEx>(L, 1);
    if (pEntity && pEntity->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        static_cast<EntityFlyer*>(pEntity)->TakeOff();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058ed80
int EntityFlyerLand(lua_State* L)
{
    EntityEx* pEntity = _GetEntity<EntityEx>(L, 1);
    if (pEntity && pEntity->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        static_cast<EntityFlyer*>(pEntity)->Land();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058edb0
int EntityFlyerInitAsFlying(lua_State* L)
{
    EntityEx* pEntity = _GetEntity<EntityEx>(L, 1);
    if (pEntity && pEntity->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        static_cast<EntityFlyer*>(pEntity)->InitAsFlying();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058ede0
int EntityFlyerInitAsLanded(lua_State* L)
{
    EntityEx* pEntity = _GetEntity<EntityEx>(L, 1);
    if (pEntity && pEntity->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        static_cast<EntityFlyer*>(pEntity)->InitAsLanded();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00590f90
int IsEffectActive(lua_State* L)
{
    FLEffectObject* pEffect = (FLEffectObject*)lua_topointer(L, 1);
    if (!pEffect)
    {
        return 0;
    }
    lua_pushboolean(L, pEffect->IsEffectActive());
    return 1;
}

// FUNCTION: SWBF2 0x00590fd0
int SetEffectActive(lua_State* L)
{
    FLEffectObject* pEffect = (FLEffectObject*)lua_topointer(L, 1);
    if (pEffect)
    {
        if (lua_toboolean(L, 2))
        {
            pEffect->ActivateEffect();
        }
        else
        {
            pEffect->DeactivateEffect();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00590660
int ToggleBarriers(lua_State*)
{
    DynamicBarriers(2);
    return 0;
}

// FUNCTION: SWBF2 0x00590670
int EnableBarriers(lua_State*)
{
    DynamicBarriers(1);
    return 0;
}

// FUNCTION: SWBF2 0x00590ba0
int ForceHumansOntoTeam1(lua_State*)
{
    NetGame::SetForceHumansToTeam1(true);
    return 0;
}

// FUNCTION: SWBF2 0x0058e230
int GetTimerRate(lua_State* L)
{
    Timer* pTimer = GetTimer(L, 1);
    if (pTimer)
    {
        lua_pushnumber(L, pTimer->mRate);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058e270
int GetTimerName(lua_State* L)
{
    Timer* pTimer = GetTimer(L, 1);
    if (pTimer)
    {
        lua_pushstring(L, pTimer->GetName());
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058e750
int RespawnObject(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    if (pObject)
    {
        Damageable* pDamage = pObject;
        pDamage->Respawn();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058e6d0
int GetObjectLastHitWeaponClass(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    if (pObject)
    {
        Damageable* pDamage = pObject;
        WeaponClass* pWeapon = pDamage->mDamageOwner.mWeapon;
        if (pWeapon)
        {
            lua_pushstring(L, StringDB::Find(pWeapon->mId));
            return 1;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058ccf0
int SetMaxCollisionDistance(lua_State* L)
{
    CollisionManager::SetMaxCollisionDistance((float)luaL_checknumber(L, 1));
    return 0;
}

// FUNCTION: SWBF2 0x0058cd20
int SetParticleLODBias(lua_State* L)
{
    ParticleSystem::SetMaxDistanceBias((float)luaL_checknumber(L, 1));
    return 0;
}

// FUNCTION: SWBF2 0x0058d0c0
int EnableAIAutoBalance(lua_State*)
{
    AIDifficulty::EnableAutoBalance(true);
    return 0;
}

// FUNCTION: SWBF2 0x0058d0d0
int DisableAIAutoBalance(lua_State*)
{
    AIDifficulty::EnableAutoBalance(false);
    return 0;
}

// FUNCTION: SWBF2 0x0058cfa0
int SetAIVehicleNotifyRadius(lua_State* L)
{
    VehicleManager::SetNotifyRadius((float)luaL_checknumber(L, 1));
    return 0;
}

// FUNCTION: SWBF2 0x0058d2c0
int AddMissionHint(lua_State* L)
{
    PopupText::AddHint(PblHash(luaL_checklstring(L, 1, 0)));
    return 0;
}

// FUNCTION: SWBF2 0x0058d360
int SpaceAssaultEnable(lua_State* L)
{
    SpaceAssault::mInstance->Enable(lua_toboolean(L, 1) != 0);
    return 0;
}

// FUNCTION: SWBF2 0x0058d390
int SpaceAssaultAddCriticalSystem(lua_State* L)
{
    const char* pcName = luaL_checklstring(L, 1, 0);
    short iPoints = (short)(int)(float)luaL_checknumber(L, 2);
    float fHudX = (float)luaL_checknumber(L, 3);
    float fHudY = (float)luaL_checknumber(L, 4);
    bool bShowMarker = lua_type(L, 5) <= LUA_TNIL || lua_toboolean(L, 5) != 0;
    SpaceAssault::mInstance->AddCriticalSystem(pcName, iPoints, fHudX, fHudY, bShowMarker);
    return 0;
}

// FUNCTION: SWBF2 0x0058d440
int SpaceAssaultLinkCriticalSystems(lua_State* L)
{
    if (lua_type(L, 1) == LUA_TTABLE)
    {
        unsigned int auHashes[32];
        int iCount = 0;
        lua_pushnil(L);
        while (lua_next(L, 1))
        {
            unsigned int uHash = PblHash(luaL_checklstring(L, -1, 0));
            if (iCount < 32)
            {
                auHashes[iCount] = uHash;
            }
            ++iCount;
            lua_pop(L, 1);
        }
        if (iCount <= 32)
        {
            SpaceAssault::mInstance->LinkCriticalSystems(auHashes, iCount);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058d4f0
int SpaceAssaultSetupTeamNumbers(lua_State* L)
{
    int iATT = (int)(float)luaL_checknumber(L, 1);
    int iDEF = (int)(float)luaL_checknumber(L, 2);
    SpaceAssault::mInstance->SetTeamNumbers(iATT, iDEF);
    return 0;
}

// FUNCTION: SWBF2 0x0058d540
int SpaceAssaultSetupBitmaps(lua_State* L)
{
    unsigned int auHashes[6];
    for (int i = 0; i < 6; ++i)
    {
        auHashes[i] = PblHash(luaL_optlstring(L, i + 1, 0, 0));
    }
    SpaceAssault::mInstance->SetHUDBitmaps(auHashes[0], auHashes[1], auHashes[2],
                                          auHashes[3], auHashes[4], auHashes[5]);
    return 0;
}

// FUNCTION: SWBF2 0x0058aae0
int FindEntityClass(lua_State* L)
{
    if (lua_gettop(L) <= 0)
    {
        return 0;
    }
    EntityClass* pClass = EntityClass::Find(PblHash(luaL_checklstring(L, 1, 0)));
    lua_pushlightuserdata(L, pClass);
    return 1;
}

// FUNCTION: SWBF2 0x0058aba0
int SetWorldExtents(lua_State* L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        int iExtent = (int)(float)luaL_checknumber(L, 1);
        if (iExtent > 0)
        {
            WORLD_EXTENTS = iExtent;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058c5c0
int SetCarrierClass(lua_State* L)
{
    int iTeam = (int)(float)luaL_checknumber(L, 1);
    const char* pcClass = luaL_checklstring(L, 2, 0);
    Team::sTeams[iTeam]->SetCarrier(pcClass);
    return 0;
}

// FUNCTION: SWBF2 0x0058c8c0
int SetNumBirdTypes(lua_State* L)
{
    BirdParticleManager::SetNumBirdTypes((unsigned int)(float)luaL_checknumber(L, 1));
    return 0;
}

// FUNCTION: SWBF2 0x0058c970
int SetBirdType(lua_State* L)
{
    int iType = (int)(float)luaL_checknumber(L, 1);
    float fSize = (float)luaL_checknumber(L, 2);
    const char* pcTexture = luaL_checklstring(L, 3, 0);
    BirdParticleManager::SetBirdType(iType, fSize, pcTexture);
    return 0;
}

// FUNCTION: SWBF2 0x0058f910
int IsCharacterHuman(lua_State* L)
{
    Character* pCharacter = Character::GetPtr((int)(float)luaL_checknumber(L, 1));
    if (!pCharacter)
    {
        return 0;
    }
    lua_pushboolean(L, pCharacter->mPlayerId >= 0);
    return 1;
}

// FUNCTION: SWBF2 0x0058fa90
int GetCharacterClass(lua_State* L)
{
    Character* pCharacter = Character::GetPtr((int)(float)luaL_checknumber(L, 1));
    if (pCharacter)
    {
        lua_pushnumber(L, (float)pCharacter->mClassIndex);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058fcd0
int GetCharacterUnit(lua_State* L)
{
    Character* pCharacter = Character::GetPtr((int)(float)luaL_checknumber(L, 1));
    if (pCharacter && pCharacter->mUnit)
    {
        lua_pushlightuserdata(L, pCharacter->mUnit->GetGameObject());
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058fd50
int GetCharacterVehicle(lua_State* L)
{
    Character* pCharacter = Character::GetPtr((int)(float)luaL_checknumber(L, 1));
    if (pCharacter && pCharacter->mVehicle)
    {
        lua_pushlightuserdata(L, pCharacter->mVehicle->GetGameObject());
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058fdd0
int GetCharacterRemote(lua_State* L)
{
    Character* pCharacter = Character::GetPtr((int)(float)luaL_checknumber(L, 1));
    if (pCharacter && pCharacter->mRemote)
    {
        lua_pushlightuserdata(L, pCharacter->mRemote->GetGameObject());
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058fe50
int GetCharacterControllable(lua_State* L)
{
    Character* pCharacter = Character::GetPtr((int)(float)luaL_checknumber(L, 1));
    Controllable* pControllable = 0;
    if (pCharacter)
    {
        pControllable = pCharacter->mRemote ? pCharacter->mRemote :
                        pCharacter->mVehicle ? pCharacter->mVehicle : pCharacter->mUnit;
    }
    if (pControllable)
    {
        lua_pushlightuserdata(L, pControllable->GetGameObject());
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058f790
int GetTeamMember(lua_State* L)
{
    int iTeam = (int)(float)luaL_checknumber(L, 1);
    Team* pTeam = Team::sTeams[iTeam];
    int iMember = (int)(float)luaL_checknumber(L, 2);
    Character* pMember = iMember >= pTeam->mMemberCount ? 0 : pTeam->mMemberArray[iMember];
    lua_pushnumber(L, (float)((pMember - Character::sCharacters)));
    return 1;
}

// FUNCTION: SWBF2 0x0058f8a0
int SelectCharacterTeam(lua_State* L)
{
    Character* pCharacter = Character::GetPtr((int)(float)luaL_checknumber(L, 1));
    if (pCharacter)
    {
        pCharacter->SelectTeam((unsigned int)(float)luaL_checknumber(L, 2));
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058df70
int EnableBuildingLockOn(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    if (pObject)
    {
        bool bEnable = lua_gettop(L) <= 1 || lua_toboolean(L, 2) != 0;
        Damageable* pDamage = pObject;
        if (pDamage->mHealthType == 4)
        {
            pObject->mHealthTypeForLockOn = bEnable ? 3 : 8;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00590da0
int RemoveEffect(lua_State* L)
{
    FLEffectObject* pEffect = (FLEffectObject*)lua_topointer(L, 1);
    if (pEffect)
    {
        pEffect->StopAndFinish();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058e830
int GetEntityClassName(lua_State* L)
{
    EntityEx* pEntity = _GetEntity<EntityEx>(L, 1);
    const char* pcName = 0;
    if (pEntity)
    {
        EntityClass* pClass = pEntity->GetEntityClass();
        if (pClass)
        {
            pcName = StringDB::Find(pClass->mId);
        }
    }
    if (pcName)
    {
        lua_pushstring(L, pcName);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058fef0
int GetCommandPostCaptureRegion(lua_State* L)
{
    CommandPost* pPost = CommandPost::FindPostByName(luaL_checklstring(L, 1, 0));
    if (pPost)
    {
        lua_pushlightuserdata(L, pPost->mCaptureRegion);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058ffc0
int GetCommandPostTeam(lua_State* L)
{
    CommandPost* pPost = CommandPost::FindPostByName(luaL_checklstring(L, 1, 0));
    GameObject* pObject = pPost ? pPost->GetGameObject() : 0;
    if (pObject)
    {
        lua_pushnumber(L, (float)pObject->GetTeam());
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058e3c0
int SetMissionTimer(lua_State* L)
{
    Timer* pTimer = GetTimer(L, 1);
    int iTeam = (int)(float)luaL_optnumber(L, 2, 0.0f);
    MissionTimer::Set(pTimer, MissionTimer::SHOW_NEUTRAL, iTeam);
    return 0;
}

// FUNCTION: SWBF2 0x0058e300
int BroadcastVoiceOver(lua_State* L)
{
    unsigned int uVoiceHash = PblHash(luaL_checklstring(L, 1, 0));
    int iTeamMask = -1;
    if (lua_type(L, 2) > LUA_TNIL)
    {
        iTeamMask = 1 << (int)(float)luaL_checknumber(L, 2);
    }
    NetGame::CreateEvent_PlayVoiceOver(uVoiceHash, iTeamMask);
    return 0;
}

// FUNCTION: SWBF2 0x0058ec60
int DeleteEntity(lua_State* L)
{
    EntityEx* pEntity = _GetEntity<EntityEx>(L, 1);
    if (pEntity)
    {
        delete pEntity;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00590800
int TranslateAICommand(lua_State* L)
{
    const char* pcCommand;
    switch ((int)(float)luaL_checknumber(L, 1))
    {
    case 4:  pcCommand = "waitforpickup"; break;
    case 16: pcCommand = "followchr"; break;
    case 18: pcCommand = "stopfollowchr"; break;
    case 19: pcCommand = "getintovehicle"; break;
    case 20: pcCommand = "getoutofvehicle"; break;
    default: pcCommand = "Invalid AI Command"; break;
    }
    lua_pushstring(L, pcCommand);
    return 1;
}

// FUNCTION: SWBF2 0x0058e880
int SetEntityMatrix(lua_State* L)
{
    EntityGeometry* pEntity = _GetEntity<EntityGeometry>(L, 1);
    if (pEntity)
    {
        PblMatrix matrix;
        memcpy(&matrix, lua_touserdata(L, 2), sizeof(matrix));
        pEntity->SetMatrix(matrix);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058ef80
int EnableFlyerPath(lua_State* L)
{
    if (lua_gettop(L) == 2)
    {
        const char* pcPath = luaL_checklstring(L, 1, 0);
        bool bEnable = (int)(float)luaL_checknumber(L, 2) != 0;
        EntityPath::EnablePathByName(PblHash(pcPath), bEnable);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058f4f0
int PlayAnimation(lua_State* L)
{
    PAAnimationGroup* pGroup = PAAnimationGroup::_HashTable.Find(PblHash(luaL_checklstring(L, 1, 0)));
    if (pGroup)
    {
        pGroup->Play();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058f540
int PauseAnimation(lua_State* L)
{
    PAAnimationGroup* pGroup = PAAnimationGroup::_HashTable.Find(PblHash(luaL_checklstring(L, 1, 0)));
    if (pGroup)
    {
        pGroup->Pause();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058f590
int RewindAnimation(lua_State* L)
{
    PAAnimationGroup* pGroup = PAAnimationGroup::_HashTable.Find(PblHash(luaL_checklstring(L, 1, 0)));
    if (pGroup)
    {
        pGroup->SetCurrentTime(0.0f);
        pGroup->SetEndTime(-1.0f);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058f5f0
int SetAnimationStartPoint(lua_State* L)
{
    PAAnimationGroup* pGroup = PAAnimationGroup::_HashTable.Find(PblHash(luaL_checklstring(L, 1, 0)));
    if (pGroup)
    {
        pGroup->UpdateStartPositions();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058f640
int PlayAnimationFromTo(lua_State* L)
{
    const char* pcGroup = luaL_checklstring(L, 1, 0);
    float fStart = (float)luaL_checknumber(L, 2);
    float fEnd = (float)luaL_checknumber(L, 3);
    PAAnimationGroup* pGroup = PAAnimationGroup::_HashTable.Find(PblHash(pcGroup));
    if (pGroup && fEnd > fStart && fStart >= 0.0f && fEnd >= 0.0f)
    {
        pGroup->Pause();
        pGroup->SetCurrentTime(fStart);
        pGroup->SetEndTime(fEnd);
        pGroup->Play();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058cfe0
int SetAIViewMultiplier(lua_State* L)
{
    float fMultiplier = (float)luaL_checknumber(L, 1);
    if (fMultiplier < 0.1f)
    {
        fMultiplier = 0.1f;
    }
    else if (fMultiplier > 1.0f)
    {
        fMultiplier = 1.0f;
    }
    VisionManager::SetGlobalVisModifier(fMultiplier);
    return 0;
}

// FUNCTION: SWBF2 0x0058e8f0
int GetEntityMatrix(lua_State* L)
{
    EntityGeometry* pEntity = _GetEntity<EntityGeometry>(L, 1);
    if (!pEntity)
    {
        lua_pushnil(L);
        return 1;
    }
    PblMatrix matrix = pEntity->GetMatrix();
    if (lua_isstring(L, 2))
    {
        unsigned int uBone = PblHash(luaL_checklstring(L, 2, 0));
        const PblMatrix* pBone = 0;
        const RedPose* pPose = pEntity->GetPose();
        if (pPose)
        {
            pBone = pPose->Find(uBone);
        }
        else
        {
            const GameModel* pGameModel = pEntity->GetModel();
            if (pGameModel && pGameModel->_mLod[0])
            {
                PblMatrix bone;
                if (pGameModel->_mLod[0]->GetBoneMatrix(uBone, bone))
                {
                    PblMatrix combined;
                    D3DXMatrixMultiply(&combined, &bone, &matrix);
                    matrix = combined;
                }
            }
        }
        if (pBone)
        {
            PblMatrix combined;
            D3DXMatrixMultiply(&combined, pBone, &matrix);
            matrix = combined;
        }
    }
    LuaHelper::PushObject<PblMatrix>(matrix);
    return 1;
}

// FUNCTION: SWBF2 0x0058d6a0
int ShowMessageText(lua_State* L)
{
    unsigned int uTextHash = PblHash(luaL_checklstring(L, 1, 0));
    unsigned int uTeamMask = -1;
    if (lua_type(L, 2) > LUA_TNIL)
    {
        uTeamMask = 1u << (int)(float)luaL_checknumber(L, 2);
    }
    if (NetGame::IsEnabled())
    {
        bool bClient = NetGame::IsClient();
        bool bLocal = bClient || (NetGame::IsOneWorld() && NetGame::IsHost() && !NetGame::IsDedicated());
        if (!bClient)
        {
            NetGame::CreateEvent_ScriptMessage(uTextHash, uTeamMask);
        }
        if (!bLocal)
        {
            return 0;
        }
    }
    HUD::GameEvents::DisplayCenterMessage(uTextHash, uTeamMask, &RedColor::YELLOW);
    return 0;
}

// FUNCTION: SWBF2 0x0058d930
int MapAddRegionMarker(lua_State* L)
{
    const RedRegion* pRegion = (const RedRegion*)GetRegion(L, 1);
    if (pRegion && lua_type(L, 2) > LUA_TNIL)
    {
        const char* pcTexture = luaL_checklstring(L, 2, 0);
        float fScale = (float)luaL_checknumber(L, 3);
        if (fScale > 0.0f)
        {
            int iTeam = (int)(float)luaL_checknumber(L, 4);
            unsigned int uTeamMask = iTeam ? 1u << iTeam : ~0u;
            const char* pcColor = luaL_optlstring(L, 5, 0, 0);
            bool bShowHud = lua_gettop(L) <= 5 || lua_toboolean(L, 6) != 0;
            bool bFlash = lua_gettop(L) <= 6 || lua_toboolean(L, 7) != 0;
            bool bPulse = lua_gettop(L) <= 7 || lua_toboolean(L, 8) != 0;
            TargetManager::AddMarker(0, pRegion, PblHash(pcTexture), fScale, uTeamMask,
                                     PblHash(pcColor), bShowHud, bFlash, bPulse, fScale * 0.3f);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058dab0
int MapRemoveRegionMarker(lua_State* L)
{
    const RedRegion* pRegion = (const RedRegion*)GetRegion(L, 1);
    if (pRegion)
    {
        unsigned int uTeamMask = lua_type(L, 2) > LUA_TNIL ?
            1u << (int)(float)luaL_checknumber(L, 2) : ~0u;
        TargetManager::RemoveMarker(0, pRegion, uTeamMask);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058db20
int MapAddEntityMarker(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    if (pObject && lua_type(L, 2) > LUA_TNIL)
    {
        const char* pcTexture = luaL_checklstring(L, 2, 0);
        float fScale = (float)luaL_checknumber(L, 3);
        if (fScale > 0.0f)
        {
            int iTeam = (int)(float)luaL_checknumber(L, 4);
            unsigned int uTeamMask = iTeam ? 1u << iTeam : ~0u;
            const char* pcColor = luaL_optlstring(L, 5, 0, 0);
            bool bShowHud = lua_gettop(L) <= 5 || lua_toboolean(L, 6) != 0;
            bool bFlash = lua_gettop(L) <= 6 || lua_toboolean(L, 7) != 0;
            bool bPulse = lua_gettop(L) <= 7 || lua_toboolean(L, 8) != 0;
            TargetManager::AddMarker(pObject, 0, PblHash(pcTexture), fScale, uTeamMask,
                                     PblHash(pcColor), bShowHud, bFlash, bPulse, fScale * 0.3f);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058dca0
int MapRemoveEntityMarker(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    if (pObject)
    {
        unsigned int uTeamMask = lua_type(L, 2) > LUA_TNIL ?
            1u << (int)(float)luaL_checknumber(L, 2) : ~0u;
        TargetManager::RemoveMarker(pObject, 0, uTeamMask);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058dd10
int MapAddClassMarker(lua_State* L)
{
    if (lua_type(L, 2) <= LUA_TNIL)
    {
        return 0;
    }
    const char* pcClass = luaL_checklstring(L, 1, 0);
    const char* pcTexture = luaL_checklstring(L, 2, 0);
    float fScale = (float)luaL_checknumber(L, 3);
    if (fScale <= 0.0f)
    {
        return 0;
    }
    int iTeam = (int)(float)luaL_checknumber(L, 4);
    unsigned int uTeamMask = iTeam ? 1u << iTeam : ~0u;
    const char* pcColor = luaL_optlstring(L, 5, 0, 0);
    bool bShowHud = lua_gettop(L) <= 5 || lua_toboolean(L, 6) != 0;
    bool bFlash = lua_gettop(L) <= 6 || lua_toboolean(L, 7) != 0;
    bool bPulse = lua_gettop(L) <= 7 || lua_toboolean(L, 8) != 0;
    TargetManager::AddClassMarker(PblHash(pcClass), PblHash(pcTexture), fScale,
                                   uTeamMask, PblHash(pcColor), bShowHud, bFlash,
                                   bPulse, fScale * 0.3f);
    return 0;
}

// FUNCTION: SWBF2 0x0058dea0
int MapRemoveClassMarker(lua_State* L)
{
    const char* pcClass = luaL_checklstring(L, 1, 0);
    unsigned int uTeamMask = lua_type(L, 2) > LUA_TNIL ?
        1u << (int)(float)luaL_checknumber(L, 2) : ~0u;
    TargetManager::RemoveClassMarker(PblHash(pcClass), uTeamMask);
    return 0;
}

// FUNCTION: SWBF2 0x00590d40
int CreateEffect(lua_State* L)
{
    FLEffectClass* pClass = FLEffect::Get(PblHash(luaL_checklstring(L, 1, 0)));
    if (!pClass)
    {
        return 0;
    }
    FLEffectObject* pEffect = pClass->BuildEffect();
    if (!pEffect)
    {
        return 0;
    }
    lua_pushlightuserdata(L, pEffect);
    return 1;
}

// FUNCTION: SWBF2 0x00590e50
int AttachEffectToMatrix(lua_State* L)
{
    FLEffectObject* pEffect = (FLEffectObject*)lua_topointer(L, 1);
    if (!pEffect)
    {
        return 0;
    }
    PblMatrix matrix;
    memcpy(&matrix, lua_touserdata(L, 2), sizeof(matrix));
    const PblMatrix* pOffset = lua_type(L, 3) > LUA_TNIL ?
        (const PblMatrix*)lua_touserdata(L, 3) : &_IdentityMatrix;
    pEffect->AttachEffectToMatrix(&matrix, *pOffset, false);
    return 0;
}

// FUNCTION: SWBF2 0x00590f20
int SetEffectMatrix(lua_State* L)
{
    FLEffectObject* pEffect = (FLEffectObject*)lua_topointer(L, 1);
    if (pEffect)
    {
        PblMatrix matrix;
        memcpy(&matrix, lua_touserdata(L, 2), sizeof(matrix));
        pEffect->SetMatrix(matrix);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005908a0
int AddShieldStrength(lua_State* L)
{
    GameObject* pObject = _GetEntity<GameObject>(L, 1);
    if (pObject)
    {
        float fMax = pObject->GetMaxShield() + (float)luaL_checknumber(L, 2);
        if (fMax < 0.0f)
        {
            fMax = 0.0f;
        }
        pObject->SetMaxShield(fMax);
        if (pObject->GetCurShield() > fMax)
        {
            pObject->SetCurShield(fMax, true, 0);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058b230
int MoveCameraToEntity(lua_State* L)
{
    EntityGeometry* pEntity = _GetEntity<EntityGeometry>(L, 1);
    if (pEntity)
    {
        PblMatrix matrix = pEntity->GetMatrix();
        for (int i = 0; i < 3; ++i)
        {
            matrix.m[0][i] = -matrix.m[0][i];
            matrix.m[2][i] = -matrix.m[2][i];
        }
        matrix.m[0][3] = 0.0f;
        matrix.m[2][3] = 0.0f;
        CameraManager::sInstance->mCurGameCamera[0]->SetMatrix(matrix);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058ca50
int FillAsteroidRegion(lua_State* L)
{
    const char* pcRegion = luaL_checklstring(L, 1, 0);
    const char* pcClass = luaL_checklstring(L, 2, 0);
    int iCount = (int)(float)luaL_checknumber(L, 3);
    PblVector3 vMin;
    vMin.x = (float)luaL_checknumber(L, 4);
    vMin.y = (float)luaL_checknumber(L, 5);
    vMin.z = (float)luaL_checknumber(L, 6);
    PblVector3 vMax;
    vMax.x = (float)luaL_checknumber(L, 7);
    vMax.y = (float)luaL_checknumber(L, 8);
    vMax.z = (float)luaL_checknumber(L, 9);
    EntityClass* pClass = EntityClass::Find(PblHash(pcClass));
    if (pClass)
    {
        static_cast<AsteroidClass*>(pClass)->FillRegion(pcRegion, iCount, vMin, vMax);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058d820
int ScriptCB_PlayerSuicide(lua_State* L)
{
    if (GameState::GetState() != 0x8ff60339 || !lua_isnumber(L, 1))
    {
        return 0;
    }
    int iCamera = (int)(float)luaL_checknumber(L, 1);
    if (iCamera < 0 || iCamera >= GameLoop::mNumCameras)
    {
        return 0;
    }
    Character* pPlayer = Character::FindPlayer(iCamera);
    Controllable* pUnit = pPlayer ? pPlayer->mUnit : 0;
    if (NetGame::IsEnabled())
    {
        NetGame::RequestSuicide(iCamera);
    }
    else if (pUnit)
    {
        pUnit->Kill();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058ee10
int EnterVehicle(lua_State* L)
{
    EntityEx* pEntity = _GetEntity<EntityEx>(L, 2);
    if (!pEntity || !(pEntity->IsRtti(EntityWalker::rttiHashEntityWalker) ||
                      pEntity->IsRtti(EntityHover::rttiHashEntityHover) ||
                      pEntity->IsRtti(EntityFlyer::rttiHashEntityFlyer)))
    {
        return 0;
    }
    Character* pCharacter = Character::GetPtr((int)(float)luaL_checknumber(L, 1));
    Controllable* pSoldier = 0;
    if (pCharacter && pCharacter->mClassPtr && pCharacter->mUnit &&
        pCharacter->mClassPtr->IsRtti(EntitySoldierClass::rttiHashEntitySoldierClass))
    {
        Entity* pSoldierEntity = pCharacter->mUnit->GetEntity();
        if (pSoldierEntity)
        {
            pSoldier = static_cast<EntitySoldier*>(pSoldierEntity);
        }
    }
    if (pSoldier)
    {
        pSoldier->EnterVehicle(pEntity->GetGameObject());
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058eef0
int ExitVehicle(lua_State* L)
{
    Character* pCharacter = Character::GetPtr((int)(float)luaL_checknumber(L, 1));
    Controllable* pSoldier = 0;
    if (pCharacter && pCharacter->mClassPtr && pCharacter->mUnit &&
        pCharacter->mClassPtr->IsRtti(EntitySoldierClass::rttiHashEntitySoldierClass))
    {
        Entity* pSoldierEntity = pCharacter->mUnit->GetEntity();
        if (pSoldierEntity)
        {
            pSoldier = static_cast<EntitySoldier*>(pSoldierEntity);
        }
    }
    if (pSoldier)
    {
        pSoldier->ExitVehicle(true, true);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00590690
int ForceAIOutOfVehicles(lua_State* L)
{
    Team* pTeam = Team::GetPtr((int)(float)luaL_checknumber(L, 1));
    if (!pTeam)
    {
        return 0;
    }
    pTeam->mbAIMembersIgnoreVehicles = lua_toboolean(L, 2) != 0;
    if (pTeam->mbAIMembersIgnoreVehicles)
    {
        for (int i = 0; i < pTeam->mMemberCount; ++i)
        {
            Character* pCharacter = pTeam->mMemberArray[i];
            Controllable* pSoldier = 0;
            if (pCharacter && pCharacter->mClassPtr && pCharacter->mUnit &&
                pCharacter->mClassPtr->IsRtti(EntitySoldierClass::rttiHashEntitySoldierClass))
            {
                Entity* pSoldierEntity = pCharacter->mUnit->GetEntity();
                if (pSoldierEntity)
                {
                    pSoldier = static_cast<EntitySoldier*>(pSoldierEntity);
                }
            }
            if (pSoldier)
            {
                pSoldier->ExitVehicle(true, true);
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058fb10
int SelectCharacterClass(lua_State* L)
{
    Character* pCharacter = Character::GetPtr((int)(float)luaL_checknumber(L, 1));
    if (!pCharacter)
    {
        return 0;
    }
    if (lua_isnumber(L, 2))
    {
        pCharacter->SelectClass((int)(float)luaL_checknumber(L, 2));
        return 0;
    }
    int iClass = 0;
    if (lua_isstring(L, 2))
    {
        EntityClass* pClass = EntityClass::Find(PblHash(luaL_checklstring(L, 2, 0)));
        Team* pTeam = pCharacter->mTeamPtr;
        for (iClass = 0; iClass < pTeam->mClassCount; ++iClass)
        {
            if (pTeam->mClassArray[iClass] == pClass)
            {
                break;
            }
        }
    }
    pCharacter->SelectClass(iClass);
    return 0;
}

// FUNCTION: SWBF2 0x0058fc20
int SpawnCharacter(lua_State* L)
{
    Character* pCharacter = Character::GetPtr((int)(float)luaL_checknumber(L, 1));
    if (pCharacter && lua_isuserdata(L, 2))
    {
        pCharacter->SelectPost(0);
        PblMatrix matrix;
        memcpy(&matrix, lua_touserdata(L, 2), sizeof(matrix));
        pCharacter->Spawn(matrix);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00590480
int TogglePlanningGraphArcs(lua_State* L)
{
    unsigned int auArcs[32], auTeams[32];
    int iNumArcs = 0, iNumTeams = 0;
    GetPlanningGraphArcsFromArgs(auArcs, iNumArcs, auTeams, iNumTeams);
    if (iNumArcs || iNumTeams)
    {
        ConnectivityGraphManager::GetWorldGraph((ConnectivityGraph::ProhibitFlag)-1)
            ->ToggleDynamicArcs(auArcs, iNumArcs, auTeams, iNumTeams);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00590c90
int Print3D(lua_State* L)
{
    const PblMatrix* pMatrix = (const PblMatrix*)lua_touserdata(L, 1);
    const char* pcText = luaL_checklstring(L, 2, 0);
    const PblVector3& vPosition = *(const PblVector3*)&pMatrix->m[3][0];
    RedDebug::Printf3D(vPosition, "%s", pcText);
    return 0;
}

// FUNCTION: SWBF2 0x0058f980
int BatchChangeTeams(lua_State* L)
{
    int iFrom = (int)(float)luaL_checknumber(L, 1);
    int iTo = (int)(float)luaL_checknumber(L, 2);
    int iRequested = (int)(float)luaL_checknumber(L, 3);
    SpawnManager* pSpawns = SpawnManager::sInstance;
    int iAvailable = pSpawns->GetUnitCount(iFrom);
    if (iAvailable)
    {
        int iMoved = iRequested < iAvailable ? iRequested : iAvailable;
        pSpawns->mMaxUnitCount[iTo] = pSpawns->GetUnitCount(iTo) + iMoved;
        Team* pTeam = Team::GetPtr(iFrom);
        for (int i = iMoved - 1; i >= 0; --i)
        {
            if (i < pTeam->mMemberCount && pTeam->mMemberArray[i])
            {
                pTeam->mMemberArray[i]->SelectTeam(iTo);
            }
        }
        pSpawns->mMaxUnitCount[iFrom] = iAvailable - iMoved;
    }
    return 0;
}
}

EntityClass* GetEntityClassArg1()
{
    return _GetEntityClass<EntityClass>(LuaHelper::mState, 1);
}

FlagItem* GetFlagItemArg1()
{
    return _GetEntity<FlagItem>(LuaHelper::mState, 1);
}

// FUNCTION: SWBF2 0x006c3b40
bool RedMoveHeapEnd(int iHeap, int iDelta)
{
    int iSource = iDelta < 0 ? iHeap + 1 : iHeap;
    int iTarget = iDelta < 0 ? iHeap : iHeap + 1;
    unsigned int uAmount = iDelta < 0 ? -iDelta : iDelta;
    unsigned int uTargetAlign = heapobj[iTarget].align;
    unsigned int uTagSize = (uTargetAlign + sizeof(Tag) - 1) & ~(uTargetAlign - 1);
    unsigned int uAlign = heapobj[iSource].align;
    if (uTargetAlign > uAlign)
    {
        uAlign = uTargetAlign;
    }
    unsigned int uAllocation = (uAmount & ~(uAlign - 1)) - uTagSize;
    void* pBlock = RedAllocFromHeapInternal(iTarget, uAllocation, iDelta < 0);
    Tag* pTag = (Tag*)((char*)pBlock - uTagSize);
    heapobj[iHeap].size += iDelta;
    heapobj[iHeap + 1].start = (char*)heapobj[iHeap + 1].start + iDelta;
    heapobj[iHeap + 1].size -= iDelta;
    TagRemoveList(&heapobj[iTarget].used, pTag);
    TagAddList(&heapobj[iSource].used, pTag);
    return RedFreeToHeapInternal(iSource, pBlock, 0);
}

// FUNCTION: SWBF2 0x00446250
void GetPoint(const float* pKeys, int iNumKeys, float fTime,
                           const float* pCoefficients, float* pPosition, float* pVelocity)
{
    int iLow = 0;
    int iHigh = iNumKeys - 2;
    int iKey;
    float fStart, fEnd;
    for (;;)
    {
        iKey = (iHigh + iLow) / 2;
        fStart = pKeys[iKey * 2];
        if (fTime >= fStart)
        {
            fEnd = pKeys[iKey * 2 + 2];
            if (fTime <= fEnd)
            {
                break;
            }
        }
        if (fStart <= fTime)
        {
            if (fTime > fStart)
            {
                iLow = iKey + 1;
            }
        }
        else
        {
            iHigh = iKey - 1;
        }
    }
    const unsigned int iCoefficient = *(const unsigned int*)&pKeys[iKey * 2 + 1];
    const float* pC = pCoefficients + 13 * iCoefficient;
    const float u = (fTime - fStart) / (fEnd - fStart);
    for (int i = 0; i < 3; ++i)
    {
        pPosition[i] = pC[9 + i] + ((pC[3 + i] + pC[i] * u) * u + pC[6 + i]) * u;
        pVelocity[i] = pC[6 + i] + ((pC[i] * u * 3.0f + pC[3 + i] * 2.0f) * u);
    }
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

// FUNCTION: SWBF2 0x00445c90
void AsteroidClass::FillRegion(const char* pcRegion, int iCount, PblVector3 vMin, PblVector3 vMax)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    PblRegion* pRegion = PblRegion::Find(PblHash(pcRegion));
    if (!pRegion)
    {
        return;
    }
    for (int i = 0; i < iCount; ++i)
    {
        PblVector3 vPosition;
        switch (pRegion->mType)
        {
        case PblRegion::type_sphere:
        {
            PblVector3 vRand = PblRandom::GetSim().GetVector3();
            PblVector3 vUnit(vRand.x * 2.0f - 1.0f, vRand.y * 2.0f - 1.0f, vRand.z * 2.0f - 1.0f);
            PblVector3 vDir;
            D3DXVec3Normalize(&vDir, &vUnit);
            float fRadius = PblRandom::GetSim().GetFloat() * pRegion->mSize.w;
            const PblVector3& vCenter = pRegion->mMatrix.GetTranslation();
            vPosition.x = vCenter.x + vDir.x * fRadius;
            vPosition.y = vCenter.y + vDir.y * fRadius;
            vPosition.z = vCenter.z + vDir.z * fRadius;
            break;
        }
        case PblRegion::type_box:
        case PblRegion::type_box_aligned:
        {
            PblVector3 vRand = PblRandom::GetSim().GetVector3();
            PblVector3 vLocal(pRegion->mSize.x * (vRand.x * 2.0f - 1.0f),
                              pRegion->mSize.y * (vRand.y * 2.0f - 1.0f),
                              pRegion->mSize.z * (vRand.z * 2.0f - 1.0f));
            D3DXVec3TransformCoord(&vPosition, &vLocal, &pRegion->mMatrix);
            break;
        }
        case PblRegion::type_cylinder:
        case PblRegion::type_cylinder_xaligned:
        case PblRegion::type_cylinder_yaligned:
        case PblRegion::type_cylinder_zaligned:
        {
            PblVector3 vRand = PblRandom::GetSim().GetVector3();
            float fX = vRand.x * 2.0f - 1.0f;
            float fY = vRand.y * 2.0f - 1.0f;
            float fZ = vRand.z * 2.0f - 1.0f;
            float fLength = sqrtf(fX * fX + fZ * fZ);
            float fRadius = PblRandom::GetSim().GetFloat() * pRegion->mSize.x;
            PblVector3 vLocal(fX / fLength * fRadius, pRegion->mSize.y * fY, fZ / fLength * fRadius);
            D3DXVec3TransformCoord(&vPosition, &vLocal, &pRegion->mMatrix);
            break;
        }
        default:
            break;
        }

        PblVector3 vDir;
        vDir.x = (vMax.x - vMin.x) * PblRandom::GetSim().GetFloat() + vMin.x;
        vDir.y = (vMax.y - vMin.y) * PblRandom::GetSim().GetFloat() + vMin.y;
        vDir.z = (vMax.z - vMin.z) * PblRandom::GetSim().GetFloat() + vMin.z;
        const PblVector3& vUp =
            fabs(vDir.x * gVectorUnitY.x + vDir.y * gVectorUnitY.y + vDir.z * gVectorUnitY.z) >= 0.99f
                ? gVectorUnitY
                : gVectorUnitZ;
        EntityDesc desc;
        desc.mMatrix = _IdentityMatrix;
        desc.mPlayerId = 0;
        desc.mMatrix = DirectionMatrix(vDir, vUp, vPosition);
        Build(desc);
    }
}

// FUNCTION: SWBF2 0x00446400
void AsteroidClass::FillPath(const char* pcPath, float fDensity, int iCount,
                               PblVector3 vMin, PblVector3 vMax)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    const RedPath* pPath = RedPath::Find(PblHash(pcPath));
    if (!pPath)
    {
        return;
    }

    int iNumPoints = pPath->GetNumPoints();
    int iNumKeys = 10 * iNumPoints - 9;
    float (*pKeys)[2] = new float[iNumKeys][2];
    PblHermiteVector3* pSegments = new PblHermiteVector3[iNumPoints - 1];
    float fLength = 0.0f;
    pKeys[0][0] = 0.0f;
    *(int*)&pKeys[0][1] = 0;
    float (*pKey)[2] = pKeys + 1;
    for (int i = 1; i < iNumPoints; ++i)
    {
        PblMatrix mStart(pPath->GetPointRotation(i - 1), pPath->GetPointPosition(i - 1));
        PblMatrix mEnd(pPath->GetPointRotation(i), pPath->GetPointPosition(i));
        PblHermiteVector3& segment = pSegments[i - 1];
        segment.Set(mStart.GetTranslation(), mEnd.GetTranslation(), mStart.GetZAxis(), mEnd.GetZAxis(), 1.0f);
        for (int j = 0; j < 10; ++j)
        {
            PblVector3 vFrom = segment.getValue((float)j / 10.0f);
            PblVector3 vTo = segment.getValue((float)(j + 1) / 10.0f);
            float fDX = vTo.x - vFrom.x;
            float fDY = vTo.y - vFrom.y;
            float fDZ = vTo.z - vFrom.z;
            fLength += sqrtf(fDX * fDX + fDY * fDY + fDZ * fDZ);
            (*pKey)[0] = fLength;
            *(int*)&(*pKey)[1] = i - 1;
            ++pKey;
        }
    }

    for (int i = 0; i < iCount; ++i)
    {
        PblVector3 vPosition, vDir;
        GetPoint(&pKeys[0][0], iNumKeys, PblRandom::GetSim().GetFloat() * fLength,
                   &pSegments[0]._a.x, &vPosition.x, &vDir.x);
        const PblVector3& vPathUp =
            fabs(vDir.x * gVectorUnitY.x + vDir.y * gVectorUnitY.y + vDir.z * gVectorUnitY.z) >= 0.99f
                ? gVectorUnitY
                : gVectorUnitZ;
        PblMatrix mPath = DirectionMatrix(vDir, vPathUp, vPosition);

        PblVector3 vRand = PblRandom::GetSim().GetVector3();
        float fX = vRand.x * 2.0f - 1.0f;
        float fY = vRand.y * 2.0f - 1.0f;
        float fZ = vRand.z * 2.0f - 1.0f;
        float fOffsetLength = sqrtf(fX * fX + fZ * fZ);
        float fRadius = PblRandom::GetSim().GetFloat() * fDensity;
        PblVector3 vOffset(fX / fOffsetLength * fRadius, fY / fOffsetLength * fRadius, 0.0f);
        PblVector3 vOffsetPosition;
        D3DXVec3TransformCoord(&vOffsetPosition, &vOffset, &mPath);
        vPosition = vOffsetPosition;

        vDir.x = (vMax.x - vMin.x) * PblRandom::GetSim().GetFloat() + vMin.x;
        vDir.y = (vMax.y - vMin.y) * PblRandom::GetSim().GetFloat() + vMin.y;
        vDir.z = (vMax.z - vMin.z) * PblRandom::GetSim().GetFloat() + vMin.z;
        const PblVector3& vUp =
            fabs(vDir.x * gVectorUnitY.x + vDir.y * gVectorUnitY.y + vDir.z * gVectorUnitY.z) >= 0.99f
                ? gVectorUnitY
                : gVectorUnitZ;
        EntityDesc desc;
        desc.mMatrix = _IdentityMatrix;
        desc.mPlayerId = 0;
        desc.mMatrix = DirectionMatrix(vDir, vUp, vPosition);
        Build(desc);
    }
    delete[] pKeys;
    delete[] pSegments;
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x0058d7a0
int ShowObjectiveTextPopup(lua_State* L)
{
    unsigned int uText = PblHash(luaL_checklstring(LuaHelper::mState, 1, 0));
    if (PlayerProfile::GetProfile(0)->mGameOptions.mbObjectiveDetails)
    {
        PopupText::ShowPopup(2, uText);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058d7e0
int ShowSelectionTextPopup(lua_State* L)
{
    PopupText::ShowPopup(3, PblHash(luaL_checklstring(LuaHelper::mState, 1, 0)));
    return 0;
}

// FUNCTION: SWBF2 0x0058ec80
int GetEntityClassPtr(lua_State* L)
{
    EntityClass* pClass = _GetEntityClass<EntityClass>(L, 1);
    if (pClass)
    {
        lua_pushlightuserdata(LuaHelper::mState, pClass);
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058ecc0
int SetClassProperty(lua_State* L)
{
    EntityClass* pClass = _GetEntityClass<EntityClass>(L, 1);
    if (pClass)
    {
        const char* pcProperty = luaL_checklstring(LuaHelper::mState, 2, 0);
        const char* pcValue;
        if (lua_type(LuaHelper::mState, 3) == LUA_TBOOLEAN)
        {
            pcValue = lua_toboolean(LuaHelper::mState, 3) ? "1" : "0";
        }
        else
        {
            pcValue = luaL_checklstring(LuaHelper::mState, 3, 0);
        }
        pClass->SetProperty(PblHash(pcProperty), pcValue);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00590b40
int SetMapNorthAngle(lua_State* L)
{
    bool bFlipNorth = lua_gettop(LuaHelper::mState) > 1 && lua_toboolean(LuaHelper::mState, 2) != 0;
    float fNorth = (float)luaL_checknumber(LuaHelper::mState, 1) * 0.017453292f;
    HUD::ElementMap::SetNorth(fNorth, bFlipNorth);
    return 0;
}

// FUNCTION: SWBF2 0x00590760
int AICanCaptureCP(lua_State* L)
{
    CommandPost* pPost = CommandPost::FindPostByName(luaL_checklstring(LuaHelper::mState, 1, 0));
    if (pPost)
    {
        int iTeam = (int)(float)luaL_checknumber(LuaHelper::mState, 2);
        bool bCanCapture = lua_toboolean(LuaHelper::mState, 3) != 0;
        pPost->SetAICanCapture(iTeam, bCanCapture);
        pPost->mNeutralizeTimer = 0.0f;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00590c30
int GetFlagCarrier(lua_State* L)
{
    FlagItem* pFlag = _GetEntity<FlagItem>(LuaHelper::mState, 1);

    Character* pCarrier = pFlag ? pFlag->mOwner : 0;
    if (pCarrier)
    {
        lua_pushnumber(LuaHelper::mState, (float)(pCarrier - Character::sCharacters));
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x00590d00
int DisableSmallMapMiniMap(lua_State*)
{
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        HUD::ElementMap* pMap = HUD::ElementMap::FindByIndex(i);
        if (pMap)
        {
            pMap->mAllowMiniMap = false;
            pMap->mAllowToggleOff = true;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00590ef0
int GetEffectMatrix(lua_State* L)
{
    FLEffectObject* pEffect = (FLEffectObject*)lua_topointer(LuaHelper::mState, 1);
    if (!pEffect)
    {
        return 0;
    }
    LuaHelper::PushObject<PblMatrix>(pEffect->m_mWorldMatrix);
    return 1;
}

// FUNCTION: SWBF2 0x0058eb20
int CreateEntity(lua_State* L)
{
    EntityClass* pClass = _GetEntityClass<EntityClass>(L, 1);
    if (pClass)
    {
        EntityDesc desc;
        desc.mMatrix.Reset();
        if (lua_isuserdata(LuaHelper::mState, 2))
        {
            memcpy(&desc.mMatrix, lua_touserdata(LuaHelper::mState, 2), sizeof(desc.mMatrix));
        }
        if (lua_isstring(LuaHelper::mState, 3))
        {
            const char* pcName = luaL_checklstring(LuaHelper::mState, 3, 0);
            desc.mNameId = PblHash(pcName);
            StringDB::Store(desc.mNameId, pcName);
        }
        Entity* pEntity = pClass->Build(desc);
        if (pEntity)
        {
            lua_pushlightuserdata(LuaHelper::mState, pEntity);
            return 1;
        }
    }
    lua_pushnil(LuaHelper::mState);
    return 1;
}

// FUNCTION: SWBF2 0x0058f200
int CreateMatrix(lua_State* L)
{
    PblVector3 axis;
    axis.z = (float)luaL_checknumber(LuaHelper::mState, 4);
    axis.y = (float)luaL_checknumber(LuaHelper::mState, 3);
    axis.x = (float)luaL_checknumber(LuaHelper::mState, 2);
    float fAngle = (float)luaL_checknumber(LuaHelper::mState, 1);
    PblMatrix matrix;
    D3DXMatrixRotationAxis(&matrix, &axis, fAngle);
    matrix.m[3][2] = (float)luaL_checknumber(LuaHelper::mState, 7);
    matrix.m[3][1] = (float)luaL_checknumber(LuaHelper::mState, 6);
    matrix.m[3][0] = (float)luaL_checknumber(LuaHelper::mState, 5);
    matrix.m[3][3] = 1.0f;
    if (lua_isuserdata(LuaHelper::mState, 8))
    {
        PblMatrix combined;
        D3DXMatrixMultiply(&combined, &matrix, lua_touserdata(LuaHelper::mState, 8));
        matrix = combined;
    }
    LuaHelper::PushObject<PblMatrix>(matrix);
    return 1;
}

// FUNCTION: SWBF2 0x0058f3b0
int GetWorldPosition(lua_State* L)
{
    EntityGeometry* pEntity = _GetEntity<EntityGeometry>(LuaHelper::mState, 1);
    if (!pEntity)
    {
        return 0;
    }
    const PblVector3& position = pEntity->GetCollisionCenter();
    lua_pushnumber(LuaHelper::mState, position.x);
    lua_pushnumber(LuaHelper::mState, position.y);
    lua_pushnumber(LuaHelper::mState, position.z);
    return 3;
}

// FUNCTION: SWBF2 0x0058f450
int GetScreenPosition(lua_State* L)
{
    EntityGeometry* pEntity = _GetEntity<EntityGeometry>(LuaHelper::mState, 1);
    if (!pEntity)
    {
        return 0;
    }
    const PblVector3& position = pEntity->GetCollisionCenter();
    const PblVector3 screen = RedRenderer::TransformProjectionPointToScreenSpace(
        RedRenderer::TransformWorldPointToProjectionSpace(position));
    lua_pushnumber(LuaHelper::mState, screen.x);
    lua_pushnumber(LuaHelper::mState, screen.y);
    lua_pushnumber(LuaHelper::mState, screen.z);
    return 3;
}

// FUNCTION: SWBF2 0x00590ac0
int SetMissionEndMovie(lua_State* L)
{
    luaL_checklstring(LuaHelper::mState, 1, 0);
    const char* pcMovieFile;
    switch (RedLocalizeDB::GetLanguageID())
    {
    case 3: pcMovieFile = "ingamefr.mvs"; break;
    case 4: pcMovieFile = "ingamegr.mvs"; break;
    default: pcMovieFile = "ingame.mvs"; break;
    }
    EndDisplay::sInstance->SetEndMovie(pcMovieFile, luaL_checklstring(LuaHelper::mState, 2, 0));
    return 0;
}

// FUNCTION: SWBF2 0x00590bb0
int CanCharacterInteractWithFlag(lua_State* L)
{
    int iCharacter = (int)(float)luaL_checknumber(LuaHelper::mState, 1);
    bool bCanInteract = false;
    if (Character::sCharacters && iCharacter >= 0 && iCharacter < Character::sCharacterLimit)
    {
        Character* pCharacter = &Character::sCharacters[iCharacter];
        if (!pCharacter->mRemote && pCharacter->TimeSinceLastVehicleOrRemoteChange() > 0.3f)
        {
            bCanInteract = true;
        }
    }
    lua_pushboolean(LuaHelper::mState, bCanInteract);
    return 1;
}

// FUNCTION: SWBF2 0x00590dc0
int AttachEffectToObject(lua_State* L)
{
    FLEffectObject* pEffect = (FLEffectObject*)lua_topointer(LuaHelper::mState, 1);
    if (pEffect)
    {
        EntityGeometry* pEntity = _GetEntity<EntityGeometry>(LuaHelper::mState, 2);
        const char* pcBone = luaL_optlstring(LuaHelper::mState, 3, 0, 0);
        const PblMatrix* pOffset = lua_type(LuaHelper::mState, 4) > LUA_TNIL ?
            (const PblMatrix*)lua_touserdata(LuaHelper::mState, 4) : 0;
        PblMatrix identity;
        if (!pOffset)
        {
            identity.Reset();
            pOffset = &identity;
        }
        pEffect->AttachEffectToObject(pEntity, PblHash(pcBone), *pOffset, false);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058cb90
int FillAsteroidPath(lua_State* L)
{
    const char* pcPath = luaL_checklstring(LuaHelper::mState, 1, 0);
    float fDensity = (float)luaL_checknumber(LuaHelper::mState, 2);
    const char* pcClass = luaL_checklstring(LuaHelper::mState, 3, 0);
    int iCount = (int)(float)luaL_checknumber(LuaHelper::mState, 4);
    PblVector3 vMin;
    vMin.z = (float)luaL_checknumber(LuaHelper::mState, 7);
    vMin.y = (float)luaL_checknumber(LuaHelper::mState, 6);
    vMin.x = (float)luaL_checknumber(LuaHelper::mState, 5);
    PblVector3 vMax;
    vMax.z = (float)luaL_checknumber(LuaHelper::mState, 10);
    vMax.y = (float)luaL_checknumber(LuaHelper::mState, 9);
    vMax.x = (float)luaL_checknumber(LuaHelper::mState, 8);
    EntityClass* pClass = EntityClass::Find(PblHash(pcClass));
    if (pClass)
    {
        static_cast<AsteroidClass*>(pClass)->FillPath(pcPath, fDensity, iCount, vMin, vMax);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058efe0
int GetPathPoint(lua_State* L)
{
    const RedPath* pPath = RedPath::Find(PblHash(luaL_checklstring(LuaHelper::mState, 1, 0)));
    if (!pPath)
    {
        lua_pushnil(LuaHelper::mState);
        return 1;
    }
    int iPoint = lua_isnumber(LuaHelper::mState, 2) ? (int)(float)luaL_checknumber(LuaHelper::mState, 2) : 0;
    PblQuaternion quaternion = pPath->GetPointRotation(iPoint).Inverse();
    PblQuaternion normalized;
    D3DXQuaternionNormalize(&normalized, &quaternion);
    PblMatrix matrix;
    D3DXMatrixRotationQuaternion(&matrix, &normalized);
    const PblVector3& point = pPath->GetPointPosition(iPoint);
    for (int i = 0; i < 3; ++i)
    {
        matrix.m[0][i] = -matrix.m[0][i];
        matrix.m[2][i] = -matrix.m[2][i];
        matrix.m[3][i] = (&point.x)[i];
    }
    matrix.m[0][3] = 0.0f;
    matrix.m[2][3] = 0.0f;
    matrix.m[3][3] = 1.0f;
    LuaHelper::PushObject<PblMatrix>(matrix);
    return 1;
}
}
