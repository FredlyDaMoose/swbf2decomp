#include "HUDGameEvents.h"
#include "HUDObjectiveList.h"

#include <float.h>
#include <math.h>
#include <stdio.h>

#include <d3dx9math.h>

#include "CameraManager.h"
#include "Character.h"
#include "Controllable.h"
#include "EntitySoldier.h"
#include "FLInputManager.h"
#include "FLJoystick.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "MissionTimer.h"
#include "NetGame.h"
#include "ScopeDisplay.h"
#include "Timer.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "CollisionObject.h"
#include "FlagItem.h"
#include "HUDElementVehicleSeating.h"
#include "LockOnManager.h"
#include "LuaCallbacks_Options.h"
#include "NetState.h"
#include "PlayerProfile.h"
#include "PlayerStats.h"
#include "SpaceAssault.h"
#include "Team.h"
#include "BonusManager.h"
#include "EntityDroideka.h"
#include "EntityItem.h"
#include "ToolTips.h"
#include "Weapon.h"

// GLOBAL: SWBF2 0x00789c00
char* HUD::GameEvents::gStatisticStrings[5] =
{
    "points", "kills", "deaths", "heropoints", "captures"
};

// GLOBAL: SWBF2 0x01e56c3b
namespace HUD { namespace GameEvents { bool gInitialized; } }

// GLOBAL: SWBF2 0x01e56c40
HUD::GameEvents::PlayerEvents HUD::GameEvents::gPlayerEvents[1];

// FUNCTION: SWBF2 0x005637e0
void HUD::GameEvents::DisplayMessage(unsigned int uViewport, const unsigned short* const pText,
                                     const RedColor* const pColor)
{
    PlayerEvents& events = gPlayerEvents[uViewport];
    if (pColor)
    {
        Event colorEvent(events.messageColor, pColor);
        colorEvent.Send();
    }
    Event textEvent(events.message, (const wchar_t*)pText);
    textEvent.Send();
}

// FUNCTION: SWBF2 0x00563830
void HUD::GameEvents::DisplayTextChatMessage(unsigned int uViewport, const unsigned short* const pText,
                                             const RedColor* const pColor)
{
    PlayerEvents& events = gPlayerEvents[uViewport];
    if (pColor)
    {
        Event colorEvent(events.messageColor, pColor);
        colorEvent.Send();
    }
    Event textEvent(events.message, (const wchar_t*)pText);
    textEvent.Send();
}

// FUNCTION: SWBF2 0x00563880
void HUD::GameEvents::ShowTeamPoints(int iTeam, bool bShow)
{
    const unsigned int uNumCameras = GameLoop::GetNumCameras();
    for (unsigned int i = 0; i < uNumCameras; ++i)
    {
        Character* pCharacter = NetGame::GetLocalPlayer(i);
        if (!pCharacter)
        {
            continue;
        }
        const int iMyTeam = NetGame::IsEnabled() ? NetGame::GetStartTeam(NetGame::GetLocalIndex(i))
                                                 : pCharacter->mTeamNumber;
        Team* pMyTeam = Team::sTeams[iMyTeam];
        Team* pOtherTeam = Team::sTeams[iMyTeam == 1 ? 2 : 1];
        if (iTeam == pMyTeam->mTeamNumber)
        {
            gPlayerData[i].teamData[0].showPoints = bShow;
        }
        if (iTeam == pOtherTeam->mTeamNumber)
        {
            gPlayerData[i].teamData[1].showPoints = bShow;
        }
    }
}

// GLOBAL: SWBF2 0x01e56f68
namespace HUD { namespace GameEvents { HUD::EventClass* gInitialize; } }

// GLOBAL: SWBF2 0x01e56f6c
HUD::GameEvents::ObjectiveEvents HUD::GameEvents::gObjectiveEvents;

// GLOBAL: SWBF2 0x01e56fa4
HUD::EventClass* HUD::GameEvents::gTargetResetCommon;

// GLOBAL: SWBF2 0x01e56fa8
Timer* HUD::GameEvents::gObjectiveTimer;

// GLOBAL: SWBF2 0x01e56fac
namespace HUD { namespace GameEvents { HUD::EventClass* gLevelHintText; } }

// GLOBAL: SWBF2 0x01e56fb0
namespace HUD { namespace GameEvents { HUD::EventClass* gTime; } }

// SYNTHETIC: SWBF2 0x0055e340
// HUD::GameEvents::PlayerData::PlayerData
// SYNTHETIC: SWBF2 0x0055e370
// HUD::GameEvents::WeaponData::WeaponData

// GLOBAL: SWBF2 0x01ec6290
// SYNTHETIC: SWBF2 0x00404050
// `dynamic initializer for 'HUD::GameEvents::gPlayerData''
// SYNTHETIC: SWBF2 0x00768bc0
// `dynamic atexit destructor for 'HUD::GameEvents::gPlayerData''
HUD::GameEvents::PlayerData HUD::GameEvents::gPlayerData[1];

// FUNCTION: SWBF2 0x0055e3a0
void HUD::GameEvents::Open()
{

    gInitialized = false;

    gObjectiveEvents.timer = EventClass::Create(EventClass::type_Float, "objectivetimer");
    gObjectiveEvents.timerDisable = EventClass::Create(EventClass::type_Bool, "objectivetimer.disable");
    gObjectiveEvents.victoryTimer = EventClass::Create(EventClass::type_Float, "victorytimer");
    gObjectiveEvents.victoryTimerDisable = EventClass::Create(EventClass::type_Bool, "victorytimer.disable");
    gObjectiveEvents.defeatTimer = EventClass::Create(EventClass::type_Float, "defeattimer");
    gObjectiveEvents.defeatTimerDisable = EventClass::Create(EventClass::type_Bool, "defeattimer.disable");
    gObjectiveEvents.hintPopup = EventClass::Create(EventClass::type_Uint, "hintPopup");
    gObjectiveEvents.hintPopupDisable = EventClass::Create(EventClass::type_Bool, "hintPopup.disable");
    gObjectiveEvents.hintPopupPageNumber = EventClass::Create(EventClass::type_String, "hintPopup.pageNumber");
    gObjectiveEvents.objectivePopup = EventClass::Create(EventClass::type_Uint, "objectivePopup");
    gObjectiveEvents.objectivePopupDisable = EventClass::Create(EventClass::type_Bool, "objectivePopup.disable");
    gObjectiveEvents.selectionPopup = EventClass::Create(EventClass::type_Uint, "selectionPopup");
    gObjectiveEvents.selectionPopupDisable = EventClass::Create(EventClass::type_Bool, "selectionPopup.disable");
    gTargetResetCommon = EventClass::Create(EventClass::type_Uint, "targetResetCommon");
    gLevelHintText = EventClass::Create(EventClass::type_Bool, "levelHintText");
    gInitialize = EventClass::Create(EventClass::type_Bool, "initialize");
    gTime = EventClass::Create(EventClass::type_Float, "time");

    PlayerEvents& events = gPlayerEvents[0];

    events.spawn = EventClass::Create(EventClass::type_Bool, "player%d.spawn", 1);
    events.die = EventClass::Create(EventClass::type_Bool, "player%d.die", 1);
    events.health = EventClass::Create(EventClass::type_Uint, "player%d.health", 1);
    events.healthDisable = EventClass::Create(EventClass::type_Bool, "player%d.healthDisable", 1);
    events.healthFraction = EventClass::Create(EventClass::type_Float, "player%d.healthFraction", 1);
    events.bonusHealthFraction = EventClass::Create(EventClass::type_Float, "player%d.bonusHealthFraction", 1);
    events.healthRegenPulseRate = EventClass::Create(EventClass::type_Float, "player%d.healthRegenPulseRate", 1);
    events.healthInVehicle = EventClass::Create(EventClass::type_Uint, "player%d.healthInVehicle", 1);
    events.healthInVehicleDisable = EventClass::Create(EventClass::type_Bool, "player%d.healthInVehicleDisable", 1);
    events.healthInVehicleFraction = EventClass::Create(EventClass::type_Float, "player%d.healthInVehicleFraction", 1);
    events.healthAsHero = EventClass::Create(EventClass::type_Uint, "player%d.hero.health", 1);
    events.healthAsHeroFraction = EventClass::Create(EventClass::type_Float, "player%d.hero.healthFraction", 1);
    events.healthAsHeroDisable = EventClass::Create(EventClass::type_Bool, "player%d.hero.healthDisable", 1);
    events.vehicleHealth = EventClass::Create(EventClass::type_Uint, "player%d.vehicle.health", 1);
    events.vehicleHealthFraction = EventClass::Create(EventClass::type_Float, "player%d.vehicle.healthFraction", 1);
    events.vehicleHealthDisable = EventClass::Create(EventClass::type_Bool, "player%d.vehicle.healthDisable", 1);
    events.vehicleSeatingMesh = EventClass::Create(EventClass::type_Model, "player%d.vehicle.seatingMesh", 1);
    events.jetDisable = EventClass::Create(EventClass::type_Bool, "player%d.jetDisable", 1);
    events.jetFuelFraction = EventClass::Create(EventClass::type_Float, "player%d.jetFuelFraction", 1);
    events.jetFuelWarning = EventClass::Create(EventClass::type_Float, "player%d.jetFuelWarning", 1);
    events.jetFuelThreshold = EventClass::Create(EventClass::type_Float, "player%d.jetFuelThreshold", 1);
    events.energy = EventClass::Create(EventClass::type_Float, "player%d.energy", 1);
    events.energyFraction = EventClass::Create(EventClass::type_Float, "player%d.energyFraction", 1);
    events.energyOverburn = EventClass::Create(EventClass::type_Float, "player%d.energyOverburn", 1);
    events.energyRegenPulseRate = EventClass::Create(EventClass::type_Float, "player%d.energyRegenPulseRate", 1);
    events.energyDisable = EventClass::Create(EventClass::type_Bool, "player%d.energyDisable", 1);
    events.objectiveListEnable = EventClass::Create(EventClass::type_Bool, "player%d.objectivelist.enable", 1);
    events.objectiveListDisable = EventClass::Create(EventClass::type_Bool, "player%d.objectivelist.disable", 1);
    events.objectivesUpdated = EventClass::Create(EventClass::type_Bool, "player%d.objectivesUpdated", 1);
    events.objectivesUpdatedDisable = EventClass::Create(EventClass::type_Bool, "player%d.objectivesUpdated.disable", 1);
    events.objectiveDetails = EventClass::Create(EventClass::type_Bool, "player%d.objectiveDetails", 1);
    events.objectiveDetailsDisable = EventClass::Create(EventClass::type_Bool, "player%d.objectiveDetails.disable", 1);
    events.hintsAvailable = EventClass::Create(EventClass::type_Bool, "player%d.hintsAvailable", 1);
    events.hintsAvailableDisable = EventClass::Create(EventClass::type_Bool, "player%d.hintsAvailable.disable", 1);
    events.pressSelectToReturn = EventClass::Create(EventClass::type_Bool, "player%d.pressSelectToReturn", 1);
    events.pressSelectToReturnDisable = EventClass::Create(EventClass::type_Bool, "player%d.pressSelectToReturn.disable", 1);
    events.spaceAssaultStatus = EventClass::Create(EventClass::type_Bool, "player%d.spaceAssaultStatus", 1);
    events.spaceAssaultStatusDisable = EventClass::Create(EventClass::type_Bool, "player%d.spaceAssaultStatus.disable", 1);
    events.vehicleHackingTime = EventClass::Create(EventClass::type_Float, "player%d.vehicle.hackingTime", 1);
    events.vehicleHackingTimeFraction = EventClass::Create(EventClass::type_String, "player%d.vehicle.hackingTimeFraction", 1);
    events.vehicleHackingTimeDisable = EventClass::Create(EventClass::type_Bool, "player%d.vehicle.hackingTimeDisable", 1);
    events.vehicleHackedTime = EventClass::Create(EventClass::type_Float, "player%d.vehicle.hackedTime", 1);
    events.vehicleHackedTimeFraction = EventClass::Create(EventClass::type_String, "player%d.vehicle.hackedTimeFraction", 1);
    events.vehicleHackedTimeDisable = EventClass::Create(EventClass::type_Bool, "player%d.vehicle.hackedTimeDisable", 1);

    for (unsigned int uWeapon = 0; uWeapon < 2; ++uWeapon)
    {
        WeaponEvents& weapon = events.weapons[uWeapon];

        weapon.change = EventClass::Create(EventClass::type_Uint, "player%d.weapon%d.change", 1, uWeapon + 1);
        weapon.disable = EventClass::Create(EventClass::type_Bool, "player%d.weapon%d.disable", 1, uWeapon + 1);
        weapon.ammoTotalBullets = EventClass::Create(EventClass::type_Uint, "player%d.weapon%d.totalAmmoBullets", 1, uWeapon + 1);
        weapon.ammoTotalFraction = EventClass::Create(EventClass::type_Float, "player%d.weapon%d.totalAmmoFraction", 1, uWeapon + 1);
        weapon.ammoClipBullets = EventClass::Create(EventClass::type_Uint, "player%d.weapon%d.totalClipBullets", 1, uWeapon + 1);
        weapon.ammoClipFraction = EventClass::Create(EventClass::type_Float, "player%d.weapon%d.totalClipFraction", 1, uWeapon + 1);
        weapon.ammoInfinite = EventClass::Create(EventClass::type_Bool, "player%d.weapon%d.ammoInfinite", 1, uWeapon + 1);
        weapon.heat = EventClass::Create(EventClass::type_Float, "player%d.weapon%d.heat", 1, uWeapon + 1);
        weapon.charge = EventClass::Create(EventClass::type_Float, "player%d.weapon%d.charge", 1, uWeapon + 1);
        weapon.refire = EventClass::Create(EventClass::type_Float, "player%d.weapon%d.refire", 1, uWeapon + 1);
        weapon.name = EventClass::Create(EventClass::type_String, "player%d.weapon%d.name", 1, uWeapon + 1);
        weapon.targetName = EventClass::Create(EventClass::type_String, "player%d.weapon%d.target.name", 1, uWeapon + 1);
        weapon.targetClassName = EventClass::Create(EventClass::type_String, "player%d.weapon%d.target.className", 1, uWeapon + 1);
        weapon.targetShieldName = EventClass::Create(EventClass::type_String, "player%d.weapon%d.target.shieldName", 1, uWeapon + 1);
        weapon.targetHealth = EventClass::Create(EventClass::type_Uint, "player%d.weapon%d.target.health", 1, uWeapon + 1);
        weapon.targetHealthFraction = EventClass::Create(EventClass::type_Float, "player%d.weapon%d.target.healthFraction", 1, uWeapon + 1);
        weapon.targetDisable = EventClass::Create(EventClass::type_Bool, "player%d.weapon%d.target.disable", 1, uWeapon + 1);
        weapon.targetDisableShieldName = EventClass::Create(EventClass::type_Bool, "player%d.weapon%d.target.disableShieldName", 1, uWeapon + 1);
        weapon.targetTeamColor = EventClass::Create(EventClass::type_Color, "player%d.weapon%d.target.teamColor", 1, uWeapon + 1);
        weapon.targetTeamColorBright = EventClass::Create(EventClass::type_Color, "player%d.weapon%d.target.teamColorBright", 1, uWeapon + 1);
        weapon.targetHit = EventClass::Create(EventClass::type_Bool, "player%d.weapon%d.target.hit", 1, uWeapon + 1);
        weapon.targetHitCritical = EventClass::Create(EventClass::type_Bool, "player%d.weapon%d.target.hitCritical", 1, uWeapon + 1);
        weapon.targetHitColor = EventClass::Create(EventClass::type_Color, "player%d.weapon%d.target.hitColor", 1, uWeapon + 1);
        weapon.reticuleDisable = EventClass::Create(EventClass::type_Bool, "player%d.weapon%d.reticule.disable", 1, uWeapon + 1);
        weapon.reticulePosition = EventClass::Create(EventClass::type_Vector3, "player%d.weapon%d.reticule.position", 1, uWeapon + 1);
        weapon.lockOnPosition = EventClass::Create(EventClass::type_Vector3, "player%d.weapon%d.lockOnPosition", 1, uWeapon + 1);
        weapon.lockOnDisable = EventClass::Create(EventClass::type_Bool, "player%d.weapon%d.lockOnDisable", 1, uWeapon + 1);
    }

    events.weaponsOverheat = EventClass::Create(EventClass::type_Bool, "player%d.weaponsOverheat", 1);
    events.weaponsEnable = EventClass::Create(EventClass::type_Bool, "player%d.weaponsEnable", 1);
    events.weaponsDisable = EventClass::Create(EventClass::type_Bool, "player%d.weaponsDisable", 1);
    events.mapHideCPs = EventClass::Create(EventClass::type_Bool, "player%d.map.hideCPs", 1);
    events.mapMode = EventClass::Create(EventClass::type_Int, "player%d.map.mode", 1);
    events.mapModeToggle = EventClass::Create(EventClass::type_Bool, "player%d.map.modeToggle", 1);
    events.mapEnable = EventClass::Create(EventClass::type_Bool, "player%d.map.enable", 1);
    events.mapDisable = EventClass::Create(EventClass::type_Bool, "player%d.map.disable", 1);
    events.mapSpawn = EventClass::Create(EventClass::type_Bool, "player%d.map.spawn", 1);
    events.mapSpawnLarge = EventClass::Create(EventClass::type_Bool, "player%d.map.spawnLarge", 1);
    events.mapSpawnLargeDisable = EventClass::Create(EventClass::type_Bool, "player%d.map.spawnLargeDisable", 1);
    events.index = EventClass::Create(EventClass::type_Int, "player%d.index", 1);
    events.mapRefreshTarget = EventClass::Create(EventClass::type_Uint, "player%d.map.refreshTarget", 1);
    events.mapRefreshPost = EventClass::Create(EventClass::type_Uint, "player%d.map.refreshPost", 1);
    events.mapRefreshMarker = EventClass::Create(EventClass::type_Uint, "player%d.map.refreshMarker", 1);
    events.targetResetPlayer = EventClass::Create(EventClass::type_Uint, "player%d.targetResetPlayer", 1);
    events.spawnDisplayEnable = EventClass::Create(EventClass::type_Bool, "player%d.spawnDisplay.enable", 1);
    events.spawnDisplayDisable = EventClass::Create(EventClass::type_Bool, "player%d.spawnDisplay.disable", 1);
    events.spawnDisplayMessage = EventClass::Create(EventClass::type_Uint, "player%d.spawnDisplay.message", 1);
    events.spawnDisplayVehText = EventClass::Create(EventClass::type_String, "player%d.spawnDisplay.vehicle", 1);
    events.spawnDisplayCPText = EventClass::Create(EventClass::type_String, "player%d.spawnDisplay.spawninfo", 1);
    events.displayToolTips = EventClass::Create(EventClass::type_Uint, "player%d.tooltips", 1);
    events.disableToolTips = EventClass::Create(EventClass::type_Bool, "player%d.tooltips.disable", 1);
    events.message = EventClass::Create(EventClass::type_String, "player%d.message", 1);
    events.messageColor = EventClass::Create(EventClass::type_Color, "player%d.message.color", 1);
    events.messageDisable = EventClass::Create(EventClass::type_Bool, "player%d.message.disable", 1);

    for (unsigned int uTeam = 0; uTeam < 2; ++uTeam)
    {
        TeamEvents& team = events.teams[uTeam];

        team.points = EventClass::Create(EventClass::type_Int, "player%d.team%d.points", 1, uTeam + 1);
        team.pointsDisable = EventClass::Create(EventClass::type_Bool, "player%d.team%d.pointsDisable", 1, uTeam + 1);
        team.pointsText = EventClass::Create(EventClass::type_String, "player%d.team%d.pointsText", 1, uTeam + 1);
        team.reinforcements = EventClass::Create(EventClass::type_Int, "player%d.team%d.reinforcements", 1, uTeam + 1);
        team.reinforcementsFraction = EventClass::Create(EventClass::type_Float, "player%d.team%d.reinforcementsFraction", 1, uTeam + 1);
        team.reinforcementsDisable = EventClass::Create(EventClass::type_Bool, "player%d.team%d.reinforcementsDisable", 1, uTeam + 1);
        team.texture = EventClass::Create(EventClass::type_Texture, "player%d.team%d.texture", 1, uTeam + 1);
        team.textureDisable = EventClass::Create(EventClass::type_Bool, "player%d.team%d.textureDisable", 1, uTeam + 1);
        team.bleedRate = EventClass::Create(EventClass::type_Float, "player%d.team%d.bleedRate", 1, uTeam + 1);
    }

    for (unsigned int uStatistic = 0; uStatistic < 5; ++uStatistic)
    {
        events.statistic[uStatistic] = EventClass::Create(EventClass::type_Int, "player%d.statistic.%s", 1, gStatisticStrings[uStatistic]);
        events.statisticDelta[uStatistic] = EventClass::Create(EventClass::type_String, "player%d.statistic.%sDelta", 1, gStatisticStrings[uStatistic]);
    }

    events.statisticsChanged = EventClass::Create(EventClass::type_Bool, "player%d.statistic.changed", 1);
    events.statisticsDisable = EventClass::Create(EventClass::type_Bool, "player%d.statistic.disable", 1);
    events.heroSelectTimerFraction = EventClass::Create(EventClass::type_Float, "player%d.heroSelect.timerFraction", 1);
    events.heroSelectTimer = EventClass::Create(EventClass::type_Float, "player%d.heroSelect.timer", 1);
    events.heroSelectMessage = EventClass::Create(EventClass::type_String, "player%d.heroSelect.message", 1);
    events.heroSelectDisable = EventClass::Create(EventClass::type_Bool, "player%d.heroSelect.disable", 1);
    events.flagFriendCarried = EventClass::Create(EventClass::type_Bool, "player%d.flag.friend.carried", 1);
    events.flagFriendCarriedDisable = EventClass::Create(EventClass::type_Bool, "player%d.flag.friend.carried.disable", 1);
    events.flagFriendCarriedNumber = EventClass::Create(EventClass::type_Int, "player%d.flag.friend.carried.number", 1);
    events.flagFriendCarriedNumberDisable = EventClass::Create(EventClass::type_Bool, "player%d.flag.friend.carried.number.disable", 1);
    events.flagFriendDropped = EventClass::Create(EventClass::type_Bool, "player%d.flag.friend.dropped", 1);
    events.flagFriendDroppedDisable = EventClass::Create(EventClass::type_Bool, "player%d.flag.friend.dropped.disable", 1);
    events.flagFriendDroppedNumber = EventClass::Create(EventClass::type_Int, "player%d.flag.friend.dropped.number", 1);
    events.flagFriendDroppedNumberDisable = EventClass::Create(EventClass::type_Bool, "player%d.flag.friend.dropped.number.disable", 1);
    events.flagPlayerCarried = EventClass::Create(EventClass::type_Bool, "player%d.flag.player.carried", 1);
    events.flagPlayerDropped = EventClass::Create(EventClass::type_Bool, "player%d.flag.player.dropped", 1);
    events.flagPlayerDisable = EventClass::Create(EventClass::type_Bool, "player%d.flag.player.disable", 1);
    events.flagEnemyCarried = EventClass::Create(EventClass::type_Bool, "player%d.flag.enemy.carried", 1);
    events.flagEnemyCarriedDisable = EventClass::Create(EventClass::type_Bool, "player%d.flag.enemy.carried.disable", 1);
    events.flagEnemyCarriedNumber = EventClass::Create(EventClass::type_Int, "player%d.flag.enemy.carried.number", 1);
    events.flagEnemyCarriedNumberDisable = EventClass::Create(EventClass::type_Bool, "player%d.flag.enemy.carried.number.disable", 1);
    events.flagEnemyDropped = EventClass::Create(EventClass::type_Bool, "player%d.flag.enemy.dropped", 1);
    events.flagEnemyDroppedDisable = EventClass::Create(EventClass::type_Bool, "player%d.flag.enemy.dropped.disable", 1);
    events.flagEnemyDroppedNumber = EventClass::Create(EventClass::type_Int, "player%d.flag.enemy.dropped.number", 1);
    events.flagEnemyDroppedNumberDisable = EventClass::Create(EventClass::type_Bool, "player%d.flag.enemy.dropped.number.disable", 1);
    events.commandPostCharge = EventClass::Create(EventClass::type_Float, "player%d.commandPost.charge", 1);
    events.commandPostDisable = EventClass::Create(EventClass::type_Bool, "player%d.commandPost.disable", 1);
    events.commandPostOwnerColor = EventClass::Create(EventClass::type_Color, "player%d.commandPost.color", 1);
    events.commandPostDisputeColor = EventClass::Create(EventClass::type_Color, "player%d.commandPost.disputeColor", 1);
    events.commandPostDisputeEnable = EventClass::Create(EventClass::type_Bool, "player%d.commandPost.disputeEnable", 1);
    events.commandPostDisputeDisable = EventClass::Create(EventClass::type_Bool, "player%d.commandPost.disputeDisable", 1);
    events.lockOnName = EventClass::Create(EventClass::type_String, "player%d.lockOnName", 1);
    events.lockOnClassName = EventClass::Create(EventClass::type_String, "player%d.lockOnClassName", 1);
    events.lockOnHealth = EventClass::Create(EventClass::type_Uint, "player%d.lockOnHealth", 1);
    events.lockOnHealthFraction = EventClass::Create(EventClass::type_Float, "player%d.lockOnHealthFraction", 1);
    events.lockOnDisable = EventClass::Create(EventClass::type_Bool, "player%d.lockOnDisable", 1);
    events.lockOnShieldName = EventClass::Create(EventClass::type_String, "player%d.lockOnShieldName", 1);
    events.lockOnDisableShieldName = EventClass::Create(EventClass::type_Bool, "player%d.lockOnDisableShieldName", 1);
    events.lockOnDistance = EventClass::Create(EventClass::type_Float, "player%d.lockOnDistance", 1);
    events.lockOnDirection = EventClass::Create(EventClass::type_Vector3, "player%d.lockOnDirection", 1);
    events.lockOnDirectionDisable = EventClass::Create(EventClass::type_Bool, "player%d.lockOnDirectionDisable", 1);
    events.lockOnTeamColor = EventClass::Create(EventClass::type_Color, "player%d.lockOnTeamColor", 1);
    events.lockOnFlagCarrier = EventClass::Create(EventClass::type_Bool, "player%d.lockOnFlagCarrier", 1);
    events.lockOnFlagCarrierDisable = EventClass::Create(EventClass::type_Bool, "player%d.lockOnFlagCarrierDisable", 1);
    events.missileLock = EventClass::Create(EventClass::type_Bool, "player%d.missileLock", 1);
    events.missileLockDisable = EventClass::Create(EventClass::type_Bool, "player%d.missileLockDisable", 1);
    events.missileLockDistance = EventClass::Create(EventClass::type_Uint, "player%d.missileLockDistance", 1);
    events.reticuleAlpha = EventClass::Create(EventClass::type_Float, "player%d.reticule.alpha", 1);

    PlayerData& data = gPlayerData[0];

    for (unsigned int uWeapon = 0; uWeapon < 2; ++uWeapon)
    {
        WeaponData& weapon = data.weaponData[uWeapon];

        weapon.weapon = 0;
        weapon.weaponClass = 0;
        weapon.weaponHash = 0;
        weapon.ammoTotalFraction = -1.0f;
        weapon.clipTotalFraction = -1.0f;

        weapon.target.mObject = 0;
        weapon.target.mSavedHandleId = 0;
        weapon.targetHealth = -1.0f;
        weapon.showReticule = false;
        weapon.targetLocked = 0;
    }

    for (unsigned int uTeam = 0; uTeam < 2; ++uTeam)
    {
        TeamData& team = data.teamData[uTeam];

        team.points = -99999;
        team.reinforcements = -1;
        team.showPoints = false;
        team.bleedRate = -1.0f;
    }

    data.mapMode = HUD::ElementMap::mode_Spawn;
    data.health = -1.0f;
    data.hasJetpack = false;
    data.jetFuel = -1.0f;
    data.hasShield = false;
    data.shield = -1.0f;

    data.inVehicle = false;
    data.vehicleHealth = -1.0f;
    data.teamNumber = -1;

    data.noWeapons = true;
    data.isHero = false;
    data.numFriendlyFlagsCarried = 0;
    data.numFriendlyFlagsDropped = 0;
    data.numEnemyFlagsCarried = 0;
    data.numEnemyFlagsDropped = 0;

    data.flagPlayerCarried = false;
    data.vehicleBeingHacked = false;
    data.vehicleHacking = false;
    data.lockedTarget = 0;
    data.lockedTargetHealth = -1.0f;
    data.unknownFloat00D0 = -1.0f;

    data.lockedDirectionEnabled = false;
    data.carriedFlagItem = false;
    data.missileLocked = false;
    data.scopeActive = false;
    data.unknownBool00D8 = false;

    data.statisticData[0] = -1;
    data.statisticData[1] = -1;
    data.statisticData[2] = -1;
    data.statisticData[3] = -1;
    data.statisticData[4] = -1;
}

// FUNCTION: SWBF2 0x0055f2e0
void HUD::GameEvents::Close()
{
    delete gObjectiveEvents.timer;
    delete gObjectiveEvents.timerDisable;
    delete gObjectiveEvents.victoryTimer;
    delete gObjectiveEvents.victoryTimerDisable;
    delete gObjectiveEvents.defeatTimer;
    delete gObjectiveEvents.defeatTimerDisable;
    delete gObjectiveEvents.hintPopup;
    delete gObjectiveEvents.hintPopupDisable;
    delete gObjectiveEvents.hintPopupPageNumber;
    delete gObjectiveEvents.objectivePopup;
    delete gObjectiveEvents.objectivePopupDisable;
    delete gObjectiveEvents.selectionPopup;
    delete gObjectiveEvents.selectionPopupDisable;
    delete gTargetResetCommon;
    delete gLevelHintText;
    delete gInitialize;
    delete gTime;

    for (int iPlayer = 0; iPlayer < 1; ++iPlayer)
    {
        PlayerEvents& events = gPlayerEvents[iPlayer];

        delete events.spawn;
        delete events.die;
        delete events.health;
        delete events.healthFraction;
        delete events.bonusHealthFraction;
        delete events.healthRegenPulseRate;
        delete events.healthDisable;
        delete events.healthInVehicle;
        delete events.healthInVehicleFraction;
        delete events.healthInVehicleDisable;
        delete events.healthAsHero;
        delete events.healthAsHeroFraction;
        delete events.healthAsHeroDisable;
        delete events.vehicleHealth;
        delete events.vehicleHealthFraction;
        delete events.vehicleHealthDisable;
        delete events.vehicleSeatingMesh;
        delete events.jetDisable;
        delete events.jetFuelFraction;
        delete events.jetFuelWarning;
        delete events.jetFuelThreshold;
        delete events.energy;
        delete events.energyFraction;
        delete events.energyOverburn;
        delete events.energyRegenPulseRate;
        delete events.energyDisable;
        delete events.vehicleHackedTime;
        delete events.vehicleHackedTimeFraction;
        delete events.vehicleHackedTimeDisable;
        delete events.vehicleHackingTime;
        delete events.vehicleHackingTimeFraction;
        delete events.vehicleHackingTimeDisable;

        for (unsigned int uWeapon = 0; uWeapon < 2; ++uWeapon)
        {
            WeaponEvents& weapon = events.weapons[uWeapon];
            delete weapon.change;
            delete weapon.disable;
            delete weapon.ammoTotalBullets;
            delete weapon.ammoTotalFraction;
            delete weapon.ammoClipBullets;
            delete weapon.ammoClipFraction;
            delete weapon.ammoInfinite;
            delete weapon.heat;
            delete weapon.charge;
            delete weapon.refire;
            delete weapon.name;
            delete weapon.targetName;
            delete weapon.targetClassName;
            delete weapon.targetShieldName;
            delete weapon.targetHealth;
            delete weapon.targetHealthFraction;
            delete weapon.targetDisable;
            delete weapon.targetDisableShieldName;
            delete weapon.targetTeamColor;
            delete weapon.targetTeamColorBright;
            delete weapon.targetHit;
            delete weapon.targetHitCritical;
            delete weapon.targetHitColor;
            delete weapon.reticuleDisable;
            delete weapon.reticulePosition;
            delete weapon.lockOnPosition;
            delete weapon.lockOnDisable;
        }

        delete events.weaponsOverheat;
        delete events.weaponsEnable;
        delete events.weaponsDisable;
        delete events.mapHideCPs;
        delete events.mapMode;
        delete events.mapModeToggle;
        delete events.mapEnable;
        delete events.mapDisable;
        delete events.mapSpawn;
        delete events.mapSpawnLarge;
        delete events.mapSpawnLargeDisable;
        delete events.index;
        delete events.mapRefreshTarget;
        delete events.mapRefreshPost;
        delete events.mapRefreshMarker;
        delete events.targetResetPlayer;
        delete events.spawnDisplayEnable;
        delete events.spawnDisplayDisable;
        delete events.spawnDisplayMessage;
        delete events.spawnDisplayVehText;
        delete events.spawnDisplayCPText;
        delete events.objectiveListEnable;
        delete events.objectiveListDisable;
        delete events.objectivesUpdated;
        delete events.objectivesUpdatedDisable;
        delete events.objectiveDetails;
        delete events.objectiveDetailsDisable;
        delete events.hintsAvailable;
        delete events.hintsAvailableDisable;
        delete events.pressSelectToReturn;
        delete events.pressSelectToReturnDisable;
        delete events.spaceAssaultStatus;
        delete events.spaceAssaultStatusDisable;
        delete events.message;
        delete events.messageColor;
        delete events.messageDisable;
        delete events.displayToolTips;
        delete events.disableToolTips;

        for (unsigned int uTeam = 0; uTeam < 2; ++uTeam)
        {
            TeamEvents& team = events.teams[uTeam];
            delete team.points;
            delete team.pointsDisable;
            delete team.pointsText;
            delete team.reinforcements;
            delete team.reinforcementsFraction;
            delete team.reinforcementsDisable;
            delete team.texture;
            delete team.textureDisable;
            delete team.bleedRate;
        }

        for (unsigned int uStatistic = 0; uStatistic < 5; ++uStatistic)
        {
            delete events.statistic[uStatistic];
            delete events.statisticDelta[uStatistic];
        }

        delete events.statisticsChanged;
        delete events.statisticsDisable;
        delete events.heroSelectTimerFraction;
        delete events.heroSelectTimer;
        delete events.heroSelectMessage;
        delete events.heroSelectDisable;
        delete events.flagFriendCarried;
        delete events.flagFriendCarriedDisable;
        delete events.flagFriendCarriedNumber;
        delete events.flagFriendCarriedNumberDisable;
        delete events.flagFriendDropped;
        delete events.flagFriendDroppedDisable;
        delete events.flagFriendDroppedNumber;
        delete events.flagFriendDroppedNumberDisable;
        delete events.flagPlayerCarried;
        delete events.flagPlayerDropped;
        delete events.flagPlayerDisable;
        delete events.flagEnemyCarried;
        delete events.flagEnemyCarriedDisable;
        delete events.flagEnemyCarriedNumber;
        delete events.flagEnemyCarriedNumberDisable;
        delete events.flagEnemyDropped;
        delete events.flagEnemyDroppedDisable;
        delete events.flagEnemyDroppedNumber;
        delete events.flagEnemyDroppedNumberDisable;
        delete events.commandPostCharge;
        delete events.commandPostDisable;
        delete events.commandPostOwnerColor;
        delete events.commandPostDisputeColor;
        delete events.commandPostDisputeEnable;
        delete events.commandPostDisputeDisable;
        delete events.lockOnName;
        delete events.lockOnClassName;
        delete events.lockOnHealth;
        delete events.lockOnHealthFraction;
        delete events.lockOnDisable;
        delete events.lockOnShieldName;
        delete events.lockOnDisableShieldName;
        delete events.lockOnDistance;
        delete events.lockOnDirection;
        delete events.lockOnDirectionDisable;
        delete events.lockOnTeamColor;
        delete events.lockOnFlagCarrier;
        delete events.lockOnFlagCarrierDisable;
        delete events.missileLock;
        delete events.missileLockDisable;
        delete events.missileLockDistance;
        delete events.reticuleAlpha;
    }

    gObjectiveTimer = 0;
}

// GLOBAL: SWBF2 0x01e56fa0
namespace HUD { namespace GameEvents { bool gInputMapLastPressed; } }

extern RedColor gLocalColor;

HUD::Event* InitializeEventWithData(HUD::Event* pEvent, HUD::EventClass* pClass, HUD::Event::Data data);

// FUNCTION: SWBF2 0x00563190
void HUD::GameEvents::DisableTeamElements(HUD::GameEvents::PlayerEvents& events, HUD::GameEvents::PlayerData& data)
{
    for (int i = 0; i < 2; ++i)
    {
        data.teamData[i].points = -99999;
        data.teamData[i].reinforcements = -1;
        data.teamData[i].bleedRate = -1.0f;
        HUD::Event(events.teams[i].textureDisable, true).Send();
        HUD::Event(events.teams[i].pointsDisable, true).Send();
        HUD::Event(events.teams[i].reinforcementsDisable, true).Send();
    }
}

// FUNCTION: SWBF2 0x00563210
void HUD::GameEvents::DisablePlayerElements(HUD::GameEvents::PlayerEvents& events, HUD::GameEvents::PlayerData& data)
{
    HUD::Event(events.healthDisable, true).Send();
    data.health = -1.0f;
    HUD::Event(events.healthInVehicleDisable, true).Send();
    HUD::Event(events.jetDisable, true).Send();
    data.hasJetpack = false;
    data.jetFuel = -1.0f;
    data.hasShield = false;
    data.shield = -1.0f;
    HUD::Event(events.energyDisable, true).Send();
    data.hasEnergy = false;
    data.energy = -1.0f;
    HUD::Event(events.vehicleHealthDisable, true).Send();
    data.vehicleHealth = -1.0f;
    HUD::Event(events.healthAsHeroDisable, true).Send();
    data.isHero = false;
    HUD::Event(events.vehicleHackingTimeDisable, true).Send();
    data.vehicleHacking = false;
    HUD::Event(events.vehicleHackedTimeDisable, true).Send();
    data.vehicleBeingHacked = false;

    for (int i = 0; i < 2; ++i)
    {
        HUD::GameEvents::WeaponData& weapon = data.weaponData[i];
        HUD::GameEvents::WeaponEvents& weaponEvents = events.weapons[i];
        weapon.weapon = 0;
        weapon.weaponClass = 0;
        weapon.weaponHash = 0;
        weapon.ammoTotalFraction = -1.0f;
        weapon.clipTotalFraction = -1.0f;
        weapon.target = 0;
        weapon.showReticule = false;
        weapon.targetLocked = 0;
        HUD::Event(weaponEvents.disable, true).Send();
        HUD::Event(weaponEvents.reticuleDisable, true).Send();
        HUD::Event(weaponEvents.targetTeamColor, &gLocalColor).Send();
        HUD::Event(weaponEvents.targetTeamColorBright, &gLocalColor).Send();
        HUD::Event(weaponEvents.targetDisable, true).Send();
        HUD::Event(weaponEvents.targetDisableShieldName, true).Send();
        HUD::Event(weaponEvents.lockOnDisable, true).Send();
    }

    HUD::Event(events.weaponsDisable, true).Send();
    data.teamNumber = -1;
    data.noWeapons = true;
    data.numFriendlyFlagsCarried = 0;
    HUD::Event(events.flagFriendCarriedDisable, true).Send();
    HUD::Event(events.flagFriendCarriedNumberDisable, true).Send();
    data.numFriendlyFlagsDropped = 0;
    HUD::Event(events.flagFriendDroppedDisable, true).Send();
    HUD::Event(events.flagFriendDroppedNumberDisable, true).Send();
    data.numEnemyFlagsCarried = 0;
    HUD::Event(events.flagEnemyCarriedDisable, true).Send();
    HUD::Event(events.flagEnemyCarriedNumberDisable, true).Send();
    data.numEnemyFlagsDropped = 0;
    HUD::Event(events.flagEnemyDroppedDisable, true).Send();
    HUD::Event(events.flagEnemyDroppedNumberDisable, true).Send();
    data.flagPlayerCarried = false;
    HUD::Event(events.flagPlayerDisable, true).Send();
    HUD::Event(events.statisticsDisable, true).Send();
    HUD::Event(events.lockOnDisable, true).Send();
    HUD::Event(events.lockOnDisableShieldName, true).Send();
    HUD::Event(events.lockOnDirectionDisable, true).Send();
    data.lockedTarget = 0;
    data.lockedTargetHealth = -1.0f;
    data.unknownFloat00D0 = -1.0f;
    data.lockedDirectionEnabled = false;
    data.carriedFlagItem = false;
    HUD::Event(events.missileLockDisable, true).Send();
    data.missileLocked = false;
}

// FUNCTION: SWBF2 0x00563630
void HUD::GameEvents::Die(HUD::GameEvents::PlayerEvents& events, HUD::GameEvents::PlayerData& data)
{
    HUD::GameEvents::DisablePlayerElements(events, data);
    HUD::Event(events.die, true).Send();
}

class EntitySoldierClass;
namespace HUD { namespace GameEvents {
void UpdateLockOn(unsigned int uIndex, const GameObject* const pObject, PlayerEvents& events, PlayerData& data);
void UpdateWeaponEvents(const RedCamera* const pCamera, const Character* const pCharacter,
                        const GameObject* const pObject, const Controllable* const pControllable,
                        const Controllable* const pUnit, PlayerEvents& events, WeaponEvents& weaponEvents,
                        PlayerData& data, WeaponData& weaponData, unsigned int uWeapon);
void UpdateHealth(const Character* const pCharacter, const GameObject* const pObject, PlayerEvents& events,
                  PlayerData& data);
void UpdateVehicleHealth(int iIndex, const Character* const pCharacter, PlayerEvents& events, PlayerData& data);
void UpdateJetpack(const GameObject* const pObject, const EntitySoldierClass* const pClass, PlayerEvents& events,
                   PlayerData& data);
void UpdateShield(const GameObject* const pObject, PlayerEvents& events, PlayerData& data);
void UpdateFlags(const Character* const pCharacter, const Controllable* const pControllable, PlayerEvents& events,
                 PlayerData& data);
void UpdateEnergy(const Controllable* const pControllable, PlayerEvents& events, PlayerData& data);
void UpdateMissleLock(const GameObject* const pObject, const Controllable* const pControllable,
                      PlayerEvents& events, PlayerData& data);
void UpdateTeam(const Character* const pCharacter, PlayerEvents& events, PlayerData& data);
void UpdateStatistics(const Character* const pCharacter, PlayerEvents& events, PlayerData& data);
void HideObjectiveDetailsElements(PlayerEvents& events, PlayerData& data);
void UpdateHackWeapon(const GameObject* const pObject, EventClass* const pTimeEvent,
                      EventClass* const pTextEvent, EventClass* const pDisableEvent,
                      unsigned int uFormatID, bool& bActive);
void GameObjectGetNames(PblHandle<GameObject const>& hObject, const unsigned short** const ppName,
                        const unsigned short** const ppClassName);
} }
namespace ObjectiveManager { extern PblList<HUD::ObjectiveList>* gObjectiveLists; }

namespace ObjectiveManager {

// FUNCTION: SWBF2 0x005f6920
int GetCurrentPopupText()
{
    PblList<HUD::ObjectiveList>* lists = ObjectiveManager::gObjectiveLists;
    if (!lists || lists->_iCount <= 0)
    {
        return 0;
    }
    HUD::ObjectiveList* list = lists->_head._pNext->_pObject;
    PblList<HUD::Objective>::Node* node = list->_objectives._head._pNext;
    while (HUD::Objective* objective = node->_pObject)
    {
        if (objective->_state == HUD::Objective::ACTIVE && objective->_popupTextID)
        {
            return objective->_popupTextID;
        }
        node = node->_pNext;
    }
    return 0;
}
}
namespace PopupText { int GetNumHints(); }

// FUNCTION: SWBF2 0x00562b90
void HUD::GameEvents::SetObjectiveTimer(const Timer* const pTimer)
{
    if (gObjectiveTimer && !pTimer)
    {
        HUD::Event(gObjectiveEvents.timerDisable, true).Send();
    }
    gObjectiveTimer = const_cast<Timer*>(pTimer);
}

// FUNCTION: SWBF2 0x00562bd0
const Timer* HUD::GameEvents::GetObjectiveTimer()
{
    return gObjectiveTimer;
}

// FUNCTION: SWBF2 0x00562be0
void HUD::GameEvents::Update(float fDeltaTime)
{
    (void)fDeltaTime;
    const unsigned int uNumLocal = NetGame::GetNumLocalPlayers();

    if (!gInitialized)
    {
        char acBuffer[256];
        _snprintf(acBuffer, sizeof(acBuffer), "level.%s.hints", GameLoop::mWorldFileName);
        unsigned int uHash = PblHash(acBuffer);
        bool bFound = RedLocalizeDB::Find(uHash, true) != 0;
        if (!bFound)
        {
            _snprintf(acBuffer, sizeof(acBuffer), "level.%s.hints", GameLoop::mTerrainFileName);
            uHash = PblHash(acBuffer);
            bFound = RedLocalizeDB::Find(uHash, true) != 0;
        }
        if (bFound)
        {
            HUD::Event event;
            HUD::Event::Data data;
            data.uintValue = uHash;
            InitializeEventWithData(&event, gLevelHintText, data)->Send();
        }
        HUD::Event(gInitialize, true).Send();
        gInitialized = true;
    }

    HUD::Event(gTime, GameLoop::GetMissionTime()).Send();

    Timer* pTimer = gObjectiveTimer;
    const bool bTimer = pTimer != 0;
    HUD::ElementMap* pMap = HUD::ElementMap::FindByPlayerIndex(0);
    if ((CameraManager::Instance()->GetNumCameras() == 1 && pMap && pMap->GetMode()) || !bTimer)
    {
        HUD::Event(gObjectiveEvents.timerDisable, true).Send();
    }
    else
    {
        float fTime = pTimer->mTime + 0.5f;
        if (!(fTime > 0.0f))
        {
            SetObjectiveTimer(0);
        }
        HUD::Event(gObjectiveEvents.timer, fTime).Send();
    }

    if (Timer* pMissionTimer = MissionTimer::GetTimer())
    {
        float fTime = pMissionTimer->mTime;
        if (fTime < 0.0f)
        {
            fTime = 0.0f;
        }
        if (Character* pPlayer = NetGame::GetLocalPlayer(0))
        {
            switch (MissionTimer::GetTeamShow(pPlayer->mTeamNumber))
            {
            case MissionTimer::SHOW_DEFEAT:
                HUD::Event(gObjectiveEvents.defeatTimer, fTime).Send();
                break;
            case MissionTimer::SHOW_NEUTRAL:
                HUD::Event(gObjectiveEvents.timer, fTime).Send();
                break;
            case MissionTimer::SHOW_VICTORY:
                HUD::Event(gObjectiveEvents.victoryTimer, fTime).Send();
                break;
            }
        }
    }

    for (unsigned int i = 0; i < uNumLocal; ++i)
    {
        Character* pCharacter = NetGame::GetLocalPlayer(i);
        RedCamera* pCamera = CameraManager::Instance()->GetCamera(i);
        if (!pCharacter)
        {
            continue;
        }

        PlayerEvents& events = gPlayerEvents[i];
        PlayerData& data = gPlayerData[i];
        Controllable* pControllable = pCharacter->GetControllable();
        Controllable* pUnit = pCharacter->mUnit;
        SpawnDisplay* pSpawn = SpawnDisplay::Instance(i);
        bool bSpawnVisible = pSpawn && pSpawn->mVisible;

        if (!pControllable || !pUnit || bSpawnVisible)
        {
            HUD::GameEvents::DisableTeamElements(events, data);
            if (data.health != 0.0f || data.vehicleHealth != 0.0f || bSpawnVisible)
            {
                HUD::GameEvents::Die(events, data);
            }
            continue;
        }

        const bool bMapPressed =
            FLInputManager::Instance()->GetPlayerInputs(i)->GetDigital(ePROCESSEDINPUT_Map);
        if (bMapPressed && !gInputMapLastPressed)
        {
            HUD::Event(events.mapModeToggle, true).Send();
            HUD::Event(events.objectivesUpdatedDisable, true).Send();
        }
        gInputMapLastPressed = bMapPressed;

        HUD::ElementMap* pPlayerMap = HUD::ElementMap::FindByPlayerIndex(i);
        const HUD::ElementMap::Mode eMode = pPlayerMap ? pPlayerMap->GetMode() : (HUD::ElementMap::Mode)0;
        if (eMode != data.mapMode)
        {
            if (eMode)
            {
                HUD::GameEvents::DisablePlayerElements(events, data);
                HUD::GameEvents::DisableTeamElements(events, data);
            }
            if (eMode == 1)
            {
                HUD::Event(events.pressSelectToReturn, true).Send();
                if (i == 0 && GameLoop::mNumCameras == 1)
                {
                    if (ObjectiveManager::GetCurrentPopupText())
                    {
                        HUD::Event(events.objectiveDetails, true).Send();
                    }
                    if (PopupText::GetNumHints() > 0)
                    {
                        HUD::Event(events.hintsAvailable, true).Send();
                    }
                }
            }
            else
            {
                HideObjectiveDetailsElements(events, data);
            }
            data.mapMode = eMode;
        }
        if (eMode)
        {
            continue;
        }

        UpdateTeam(pCharacter, events, data);
        GameObject* pObject = pControllable->GetGameObject();
        if (!pObject)
        {
            continue;
        }

        const bool bScope = ScopeDisplay::Instance(i)->IsActive();
        const bool bInputFlag = FLInputManager::Instance()->mLCDPageToggle[0] != 0;
        if (data.scopeActive != bScope || data.unknownBool00D8 != bInputFlag)
        {
            HUD::GameEvents::DisablePlayerElements(events, data);
            data.scopeActive = bScope;
            data.unknownBool00D8 = bInputFlag;
        }
        UpdateStatistics(pCharacter, events, data);
        UpdateLockOn(i, pObject, events, data);
        UpdateMissleLock(pObject, pControllable, events, data);

        bool bNoWeapons = true;
        for (unsigned int uWeapon = 0; uWeapon < 2; ++uWeapon)
        {
            UpdateWeaponEvents(pCamera, pCharacter, pObject, pControllable, pUnit, events,
                               events.weapons[uWeapon], data, data.weaponData[uWeapon], uWeapon);
            bNoWeapons = bNoWeapons & (data.weaponData[uWeapon].weapon == 0);
        }
        bNoWeapons = bNoWeapons | (bScope || bInputFlag);
        if (bNoWeapons != data.noWeapons)
        {
            HUD::Event(bNoWeapons ? events.weaponsDisable : events.weaponsEnable, true).Send();
            data.noWeapons = bNoWeapons;
        }
        if (!bScope)
        {
            UpdateVehicleHealth(i, pCharacter, events, data);
            UpdateEnergy(pControllable, events, data);
        }

        GameObject* pUnitObject = pUnit->GetGameObject();
        if (!pUnitObject)
        {
            continue;
        }
        if (!bScope)
        {
            UpdateHealth(pCharacter, pUnitObject, events, data);
            GameObjectClass* pClass = pUnitObject->GetGameObjectClass();
            UpdateJetpack(pUnitObject,
                          pClass->IsRtti(EntitySoldierClass::rttiHashEntitySoldierClass)
                              ? (const EntitySoldierClass*)pClass
                              : 0,
                          events, data);
        }
        UpdateShield(pObject, events, data);
        UpdateFlags(pCharacter, pControllable, events, data);
    }
}

// FUNCTION: SWBF2 0x00563660
void HUD::GameEvents::Die(const Controllable* const pControllable)
{
    const int iIndex = NetGame::GetJoystickIndex(pControllable->mInitialPlayerId);
    if (iIndex < 0)
    {
        return;
    }
    PlayerEvents& events = gPlayerEvents[iIndex];
    HUD::GameEvents::Die(events, gPlayerData[iIndex]);
    HideObjectiveDetailsElements(events, gPlayerData[iIndex]);
}

class EntityFlyer { public: static PblHash rttiHashEntityFlyer; };
class MountedTurret { public: static PblHash rttiHashMountedTurret; };
class EntityBuildingArmed { public: static PblHash rttiHashEntityBuildingArmed; };
class EntityBuildingClass { public: static PblHash rttiHashEntityBuildingClass; };
class WeaponRepair { public: static PblHash rttiHashWeaponRepair; };

namespace Lua_Callbacks { extern int gUberScoreLimit; }

extern bool netFrameLock;

namespace Lua_Callbacks { extern int gHuntShowScoreLimit; }

// FUNCTION: SWBF2 0x00561c70
void HUD::GameEvents::UpdateHackWeapon(const GameObject* const pObject, EventClass* const pTimeEvent,
                                       EventClass* const pTextEvent, EventClass* const pDisableEvent,
                                       unsigned int uFormatID, bool& bActive)
{
    float fTime;
    bool bHacking;
    if (pObject && (fTime = pObject->GetEjectTimer()) > 0.1f)
    {
        bHacking = true;
    }
    else
    {
        fTime = 0.0f;
        bHacking = false;
    }

    if (bHacking != bActive)
    {
        if (!bHacking)
        {
            HUD::Event(pDisableEvent, true).Send();
        }
        bActive = bHacking;
    }

    if (bActive)
    {
        const float fRequired = pObject->GetGameObjectClass()->GetTimeRequiredToEject();
        if (fTime > fRequired)
        {
            fTime = fRequired;
        }
        unsigned short acText[128];
        PblUnicode::usnprintf(acText, 127, RedLocalizeDB::Find(uFormatID, false),
                              (int)(fTime / fRequired * 100.0f));
        HUD::Event(pTextEvent, (const wchar_t*)acText).Send();
        HUD::Event(pTimeEvent, fRequired - fTime).Send();
    }
}

// FUNCTION: SWBF2 0x00560440
void HUD::GameEvents::GameObjectGetNames(PblHandle<GameObject const>& hObject, const unsigned short** const ppName,
                                         const unsigned short** const ppClassName)
{
    const Controllable* pControllable = hObject->GetControllable();
    const Character* pCharacter = pControllable ? pControllable->mCharacter : 0;
    const unsigned short* pName = pCharacter ? pCharacter->GetName() : 0;
    const GameObject* pObject = pControllable ? pControllable->GetGameObject() : hObject->GetGameObject();
    const unsigned short* pLabel = pObject ? pObject->GetEntityClass()->mLabel : 0;

    *ppName = pName;
    *ppClassName = 0;
    if (pLabel)
    {
        char acLabel[256];
        PblUnicode::ustrncnv(acLabel, pLabel, 255);
        const unsigned short* pLocalized = RedLocalizeDB::Find(PblHash(acLabel), true);
        if (pLocalized)
        {
            pLabel = pLocalized;
        }
        *ppClassName = pLabel;
    }
}

// FUNCTION: SWBF2 0x00560520
void HUD::GameEvents::UpdateLockOn(unsigned int uIndex, const GameObject* const pObject, PlayerEvents& events,
                                   PlayerData& data)
{
    LockOnManager* pLockOn = LockOnManager::sLockOnManagers[uIndex];
    GameObject* pTarget = 0;
    if (pObject->GetEntity() &&
        (pObject->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer) ||
         (pObject->IsRtti(EntityBuildingArmed::rttiHashEntityBuildingArmed) && pObject->GetControllable() &&
          pObject->GetControllable()->mPilotType == PILOT_REMOTE)))
    {
        pTarget = pLockOn->mLockedObj;
    }

    if (pTarget != data.lockedTarget)
    {
        data.lockedTargetHealth = -1.0f;
        data.lockedDirectionEnabled = false;
        data.carriedFlagItem = false;
        HUD::Event(events.lockOnDisable, true).Send();
        HUD::Event(events.lockOnDisableShieldName, true).Send();
        HUD::Event(events.lockOnDirectionDisable, true).Send();
        HUD::Event(events.lockOnFlagCarrierDisable, true).Send();
        if (pTarget)
        {
            PblHandle<GameObject const> hTarget(pTarget);
            const unsigned short* pName;
            const unsigned short* pClassName;
            GameObjectGetNames(hTarget, &pName, &pClassName);
            if (pName)
            {
                HUD::Event(events.lockOnName, (const wchar_t*)pName).Send();
            }
            if (pClassName)
            {
                HUD::Event(events.lockOnClassName, (const wchar_t*)pClassName).Send();
            }
            const RedColor color = pTarget->GetTeam()
                                       ? Team::GetColorByAffiliation(
                                             Team::sTeams[pObject->GetTeam()]->GetAffiliation(pTarget->GetTeam()))
                                       : gLocalColor;
            HUD::Event(events.lockOnTeamColor, &color).Send();
        }
        data.lockedTarget = pTarget;
    }

    if (!data.lockedTarget)
    {
        return;
    }

    bool bShielded = true;
    if (pTarget->IsRtti(EntityDroideka::rttiHashEntityDroideka))
    {
        bShielded = pTarget->mIsShielded;
    }
    const float fShield = pTarget->mCurShield;
    if (fShield > 0.0001f && bShielded)
    {
        if (data.lockedTargetHealth != fShield)
        {
            const float fMaxShield = pTarget->mMaxShield;
            HUD::Event health;
            HUD::Event::Data payload;
            payload.intValue = (int)fShield;
            InitializeEventWithData(&health, events.lockOnHealth, payload)->Send();
            HUD::Event(events.lockOnHealthFraction, fShield / fMaxShield).Send();
            data.lockedTargetHealth = fShield;
        }
        HUD::Event(events.lockOnShieldName, (const wchar_t*)RedLocalizeDB::Find(0x66049aa8, false)).Send();
    }
    else
    {
        HUD::Event(events.lockOnDisableShieldName, true).Send();
        if (!pTarget->GetGameObjectClass()->mbHideHealthBar)
        {
            float fHealth = pTarget->mCurHealth;
            if (!(fHealth > 0.0f))
            {
                fHealth = 0.0f;
            }
            if (fHealth != data.lockedTargetHealth)
            {
                const float fMaxHealth = pTarget->mMaxHealth;
                HUD::Event health;
                HUD::Event::Data payload;
                payload.intValue = (int)fHealth;
                InitializeEventWithData(&health, events.lockOnHealth, payload)->Send();
                HUD::Event(events.lockOnHealthFraction, fHealth / fMaxHealth).Send();
                data.lockedTargetHealth = fHealth;
            }
        }
    }

    const PblMatrix& mPlayer = pObject->GetMatrix();
    const PblVector3& vTargetPos = pTarget->GetMatrix().GetTranslation();
    const float fDeltaX = vTargetPos.x - mPlayer.m[3][0];
    const float fDeltaY = vTargetPos.y - mPlayer.m[3][1];
    const float fDeltaZ = vTargetPos.z - mPlayer.m[3][2];
    const float fDistance = sqrtf(fDeltaY * fDeltaY + fDeltaX * fDeltaX + fDeltaZ * fDeltaZ);
    HUD::Event(events.lockOnDistance, fDistance).Send();

    const float fInvDistance = 1.0f / fDistance;
    const PblVector3& vUp = mPlayer.GetYAxis();
    const float fDot = vUp.y * (fDeltaY * fInvDistance) + vUp.x * (fDeltaX * fInvDistance) +
                       vUp.z * (fDeltaZ * fInvDistance);
    const bool bShowDirection = fabs(fDot) > 0.2f;
    if (bShowDirection != data.lockedDirectionEnabled)
    {
        if (bShowDirection)
        {
            const PblVector3 vRotation(0.0f, 0.0f, fDot > 0.0f ? 0.0f : 180.0f);
            HUD::Event(events.lockOnDirection, &vRotation).Send();
        }
        else
        {
            HUD::Event(events.lockOnDirectionDisable, true).Send();
        }
        data.lockedDirectionEnabled = bShowDirection;
    }

    const bool bFlagCarrier = data.lockedTarget->GetCarriedFlagItem() != 0;
    if (data.carriedFlagItem != bFlagCarrier)
    {
        HUD::Event(bFlagCarrier ? events.lockOnFlagCarrier : events.lockOnFlagCarrierDisable, true).Send();
        data.carriedFlagItem = bFlagCarrier;
    }
}

// FUNCTION: SWBF2 0x00560ab0
void HUD::GameEvents::UpdateWeaponEvents(const RedCamera* const pCamera, const Character* const pCharacter,
                                         const GameObject* const pObject, const Controllable* const pControllable,
                                         const Controllable* const pUnit, PlayerEvents& events,
                                         WeaponEvents& weaponEvents, PlayerData& data, WeaponData& weaponData,
                                         unsigned int uWeapon)
{
    (void)pUnit;
    const int iWeaponIndex = pControllable->GetWeaponIndex(uWeapon);
    Weapon* pWeapon = iWeaponIndex >= 0 ? pControllable->GetWeaponPtr(iWeaponIndex) : 0;
    WeaponClass* pWeaponClass = pWeapon ? pWeapon->mClass : 0;
    const unsigned int uWeaponHash = pWeapon ? pWeapon->GetNameHash() : 0;

    if (pWeapon != weaponData.weapon || pWeaponClass != weaponData.weaponClass ||
        uWeaponHash != weaponData.weaponHash)
    {
        HUD::Event(weaponEvents.disable, true).Send();
        HUD::Event(weaponEvents.lockOnDisable, true).Send();
        HUD::Event(weaponEvents.targetDisable, true).Send();
        HUD::Event(weaponEvents.targetDisableShieldName, true).Send();
        UpdateHackWeapon(0, events.vehicleHackingTime, events.vehicleHackingTimeFraction,
                         events.vehicleHackingTimeDisable, 0x6d2f8d98, data.vehicleHacking);
        if (pWeapon && !data.scopeActive && !data.unknownBool00D8)
        {
            HUD::Event change;
            HUD::Event::Data payload;
            payload.uintValue = uWeaponHash;
            InitializeEventWithData(&change, weaponEvents.change, payload)->Send();

            unsigned short acFilename[256];
            unsigned short acName[256];
            const unsigned short* pLabel = pWeaponClass->mLabel;
            if (!pLabel)
            {
                PblUnicode::ustrncnv(acFilename, pWeaponClass->mFilename, 256);
                pLabel = acFilename;
            }
            PblUnicode::ustrlcpy(acName, pLabel, 256);
            HUD::Event(weaponEvents.name, (const wchar_t*)acName).Send();
        }
        weaponData.weapon = pWeapon;
        weaponData.weaponClass = pWeaponClass;
        weaponData.weaponHash = uWeaponHash;
        weaponData.clipTotalFraction = -1.0f;
        weaponData.ammoTotalFraction = -1.0f;
        weaponData.targetLocked = 0;
        data.unknownFloat00D0 = -1.0f;
    }

    if (const PlayerProfile::tProfile* pProfile = pCharacter->GetProfile())
    {
        const float fAlpha = pProfile->mGameOptions.mReticuleAlpha;
        HUD::Event(events.reticuleAlpha, fAlpha).Send();
        data.unknownFloat00D0 = fAlpha;
    }

    const bool bShowReticule = pWeapon && pObject->ShouldShowReticule();
    if (bShowReticule != weaponData.showReticule)
    {
        if (!bShowReticule)
        {
            HUD::Event(weaponEvents.reticuleDisable, true).Send();
        }
        weaponData.showReticule = bShowReticule;
    }

    if (pWeapon)
    {

        const AmmoCounter* pAmmo = pWeapon->m_pAmmoCounter;
        const float fNumClips = pAmmo->mNumClips;
        const float fRoundsPerClip = (float)pWeaponClass->mAmmoCounterClass.mRoundsPerClip;
        const bool bNoClips =
            fRoundsPerClip >= 2147483648.0f || pWeaponClass->mAmmoCounterClass.mAmmoPerRound == 0.0f;
        const bool bInfinite = fNumClips == FLT_MAX;
        float fClipFraction;
        float fTotalFraction;
        if (bNoClips)
        {
            fClipFraction = 0.0f;
            fTotalFraction = 1.0f;
        }
        else
        {
            fClipFraction = pAmmo->mCurClip;
            fTotalFraction = bInfinite ? 1.0f : (fNumClips + fClipFraction) / pAmmo->mMaxClips;
        }

        if (weaponData.ammoTotalFraction != fTotalFraction || weaponData.clipTotalFraction != fClipFraction ||
            fClipFraction <= 0.4f)
        {
            int iClipBullets;
            int iTotalBullets;
            if (bNoClips)
            {
                iClipBullets = -1;
                iTotalBullets = -1;
            }
            else
            {
                iClipBullets = (int)(fClipFraction * fRoundsPerClip);
                iTotalBullets = bInfinite ? -1 : (int)((fClipFraction + fNumClips) * fRoundsPerClip);
            }
            if (!data.scopeActive && !data.unknownBool00D8)
            {
                HUD::Event total;
                HUD::Event::Data payload;
                payload.intValue = iTotalBullets;
                InitializeEventWithData(&total, weaponEvents.ammoTotalBullets, payload)->Send();
                HUD::Event(weaponEvents.ammoTotalFraction, fTotalFraction).Send();
            }
            HUD::Event clip;
            HUD::Event::Data payload;
            payload.intValue = iClipBullets;
            InitializeEventWithData(&clip, weaponEvents.ammoClipBullets, payload)->Send();
            HUD::Event(weaponEvents.ammoClipFraction, fClipFraction).Send();
            if (iTotalBullets == -1)
            {
                HUD::Event(weaponEvents.ammoInfinite, true).Send();
            }
            weaponData.ammoTotalFraction = fTotalFraction;
            weaponData.clipTotalFraction = fClipFraction;
        }

        float fHeat = 0.0f;
        float fRefire = 0.0f;
        const float fStateLimit = pWeapon->mStateLimit;
        const float fStateTimer = pWeapon->mStateTimer;
        switch (pWeapon->mState)
        {
        case Weapon::FIRE:
        case Weapon::FIRE2:
            if (!pWeapon->mClass->mDisplayRefire)
            {
                break;
            }

        case Weapon::RELOAD:
            if (fStateLimit != 0.0f)
            {
                float fFraction = fStateTimer / fStateLimit;
                if (0.0f > fFraction)
                {
                    fFraction = 0.0f;
                }
                else if (fFraction > 1.0f)
                {
                    fFraction = 1.0f;
                }
                fRefire = 1.0f - fFraction;
            }
            else
            {
                fRefire = 0.0f;
            }
            break;
        case Weapon::CHARGE:
            fHeat = fStateLimit != 0.0f ? fStateTimer / fStateLimit : 0.0f;
            break;
        case Weapon::OVERHEAT:
        {
            const EnergyBar* pBar = pWeapon->m_pEnergyBar;
            const EnergyBarClass* pBarClass = pBar->m_pClass;
            if (1.0f - pBar->m_fEnergy / pBarClass->m_fEnergyMax >
                1.0f - pBarClass->m_fEnergyOverheat / pBarClass->m_fEnergyMax)
            {
                HUD::Event(events.weaponsOverheat, true).Send();
                pBarClass = pWeapon->m_pEnergyBar->m_pClass;
                float fFraction = (pBarClass->m_fEnergyMax - pBarClass->m_fEnergyOverheat) /
                                  (pBarClass->m_fEnergyMax - pWeapon->m_pEnergyBar->m_fEnergy);
                if (0.0f > fFraction)
                {
                    fFraction = 0.0f;
                }
                else if (fFraction > 1.0f)
                {
                    fFraction = 1.0f;
                }
                fRefire = 1.0f - fFraction;
                fHeat = 0.0f;
            }
            break;
        }
        default:
            break;
        }

        if (!bNoClips)
        {
            HUD::Event(weaponEvents.heat, fHeat).Send();
            HUD::Event(weaponEvents.refire, fRefire).Send();
        }
        else
        {
            const EnergyBar* pBar = pWeapon->m_pEnergyBar;
            float fEnergyHeat = 1.0f - pBar->m_fEnergy / pBar->m_pClass->m_fEnergyMax;
            if (0.0f > fEnergyHeat)
            {
                fEnergyHeat = 0.0f;
            }
            else if (fEnergyHeat > 1.0f)
            {
                fEnergyHeat = 1.0f;
            }
            HUD::Event(weaponEvents.heat, fEnergyHeat).Send();
            pBar = pWeapon->m_pEnergyBar;
            const float fUsed = 1.0f - pBar->m_fEnergy / pBar->m_pClass->m_fEnergyMax;
            const float fOverheat = 1.0f - pBar->m_pClass->m_fEnergyOverheat / pBar->m_pClass->m_fEnergyMax;
            HUD::Event(weaponEvents.refire, fOverheat > fUsed ? 0.0f : fRefire).Send();
        }

        PblHandle<GameObject const> hTarget =
            pWeapon->mTarget ? pWeapon->mTarget : pControllable->mReticuleTarget[uWeapon];
        if (hTarget)
        {
            const GameObjectClass* pClass = hTarget->GetGameObjectClass();
            EntityClass* pEntityClass = const_cast<EntityClass*>(hTarget->GetEntityClass());
            const bool bBuilding =
                pEntityClass && pEntityClass->IsRtti(EntityBuildingClass::rttiHashEntityBuildingClass);
            if ((pClass && pClass->mbHideHealthBar) || (!bBuilding && !hTarget->mIsAlive))
            {
                hTarget = 0;
            }
        }

        if (weaponData.target != hTarget)
        {
            HUD::Event(weaponEvents.targetDisable, true).Send();
            HUD::Event(weaponEvents.targetDisableShieldName, true).Send();
            weaponData.targetHealth = -1.0f;
            if (hTarget)
            {
                const unsigned short* pName;
                const unsigned short* pClassName;
                GameObjectGetNames(hTarget, &pName, &pClassName);
                if (pName)
                {
                    HUD::Event(weaponEvents.targetName, (const wchar_t*)pName).Send();
                }
                if (pClassName)
                {
                    HUD::Event(weaponEvents.targetClassName, (const wchar_t*)pClassName).Send();
                }
            }
            else
            {
                HUD::Event(weaponEvents.targetTeamColor, &gLocalColor).Send();
                HUD::Event(weaponEvents.targetTeamColorBright, &gLocalColor).Send();
                UpdateHackWeapon(0, events.vehicleHackingTime, events.vehicleHackingTimeFraction,
                                 events.vehicleHackingTimeDisable, 0x6d2f8d98, data.vehicleHacking);
            }
            weaponData.target = hTarget;
        }

        if (weaponData.target)
        {
            const GameObject* pTargetObject = weaponData.target->GetGameObject();

            const RedColor color =
                hTarget->GetTeam()
                    ? Team::GetColorByAffiliation(Team::sTeams[pObject->GetTeam()]->GetAffiliation(hTarget->GetTeam()))
                    : gLocalColor;
            HUD::Event(weaponEvents.targetTeamColor, &color).Send();

            int iMax = color.g > color.b ? color.g : color.b;
            if (color.r > iMax)
            {
                iMax = color.r;
            }
            const float fScale = 382.5f / (float)iMax;
            float fR = (float)color.r * fScale;
            if (0.0f > fR)
            {
                fR = 0.0f;
            }
            else if (fR > 255.0f)
            {
                fR = 255.0f;
            }
            float fG = (float)color.g * fScale;
            if (0.0f > fG)
            {
                fG = 0.0f;
            }
            else if (fG > 255.0f)
            {
                fG = 255.0f;
            }
            float fB = (float)color.b * fScale;
            if (0.0f > fB)
            {
                fB = 0.0f;
            }
            else if (fB > 255.0f)
            {
                fB = 255.0f;
            }
            RedColor bright;
            bright.g = (unsigned char)(int)fG;
            bright.r = (unsigned char)(int)fR;
            bright.a = 0xff;
            bright.b = (unsigned char)(int)fB;
            HUD::Event(weaponEvents.targetTeamColorBright, &bright).Send();

            if (pTargetObject)
            {
                bool bShielded = true;
                if (pTargetObject->IsRtti(EntityDroideka::rttiHashEntityDroideka))
                {
                    bShielded = pTargetObject->mIsShielded;
                }
                const float fShield = pTargetObject->mCurShield;
                if (fShield > 0.0001f && bShielded)
                {
                    if (fShield != weaponData.targetHealth)
                    {
                        const float fMaxShield = pTargetObject->mMaxShield;
                        HUD::Event health;
                        HUD::Event::Data payload;
                        payload.intValue = (int)fShield;
                        InitializeEventWithData(&health, weaponEvents.targetHealth, payload)->Send();
                        HUD::Event(weaponEvents.targetHealthFraction, fShield / fMaxShield).Send();
                        weaponData.targetHealth = fShield;
                    }
                    HUD::Event(weaponEvents.targetShieldName,
                               (const wchar_t*)RedLocalizeDB::Find(0x66049aa8, false)).Send();
                }
                else
                {
                    HUD::Event(weaponEvents.targetDisableShieldName, true).Send();
                    if (!pTargetObject->GetGameObjectClass()->mbHideHealthBar)
                    {
                        float fHealth = pTargetObject->mCurHealth;
                        if (!(fHealth > 0.0f))
                        {
                            fHealth = 0.0f;
                        }
                        if (fHealth != weaponData.targetHealth)
                        {
                            const float fMaxHealth = pTargetObject->mMaxHealth;
                            HUD::Event health;
                            HUD::Event::Data payload;
                            payload.intValue = (int)fHealth;
                            InitializeEventWithData(&health, weaponEvents.targetHealth, payload)->Send();
                            HUD::Event(weaponEvents.targetHealthFraction, fHealth / fMaxHealth).Send();

                            weaponData.targetHealth = fShield;
                        }
                    }
                }
            }

            if (pWeapon->IsRtti(WeaponRepair::rttiHashWeaponRepair))
            {
                if (!pObject || !pObject->IsMyEnemy(pTargetObject))
                {
                    pTargetObject = 0;
                }
                UpdateHackWeapon(pTargetObject, events.vehicleHackingTime, events.vehicleHackingTimeFraction,
                                 events.vehicleHackingTimeDisable, 0x6d2f8d98, data.vehicleHacking);
            }
        }

        const GameObject* pLocked =
            (pWeapon->IsRtti(WeaponLauncher::rttiHashWeaponLauncher) && pWeapon->mState == Weapon::CHARGE)
                ? pWeapon->GetLocked()
                : 0;
        if (pLocked != weaponData.targetLocked)
        {
            if (!pLocked)
            {
                HUD::Event(weaponEvents.lockOnDisable, true).Send();
            }
            weaponData.targetLocked = const_cast<GameObject*>(pLocked);
        }
        if (pLocked)
        {
            const GameObject* pRoot = pLocked;
            if (pLocked->GetCollisionType() == COLL_ATTACHED)
            {
                pRoot = Controllable::GetVehicleIfAttached(const_cast<GameObject*>(pLocked));
                if (pRoot == pLocked)
                {
                    pRoot = 0;
                }
            }
            float fOffsetX = 0.0f;
            float fOffsetY = 0.0f;
            float fOffsetZ = 0.0f;
            if (pRoot && pRoot->IsInScene())
            {
                PblMatrix mSmoothed = pRoot->GetSmoothedMatrix();
                if (!netFrameLock && NetGame::IsEnabled())
                {
                    gameObjectInterpolator.GetMatrix(pRoot->GetNetUniqueId(), mSmoothed);
                }
                const PblVector3& vPos = pRoot->GetMatrix().GetTranslation();
                fOffsetX = mSmoothed.m[3][0] - vPos.x;
                fOffsetY = mSmoothed.m[3][1] - vPos.y;
                fOffsetZ = mSmoothed.m[3][2] - vPos.z;
            }

            const int iBodyId = pWeapon->GetLockedTargetBodyId();
            const PblVector3 vAim =
                pLocked->GetTargetPoint(pControllable->mTargetInfo.mAimStart, pControllable->mEyeDir, iBodyId);
            PblVector3 vPoint;
            vPoint.x = fOffsetX + vAim.x;
            vPoint.y = vAim.y + fOffsetY;
            vPoint.z = vAim.z + fOffsetZ;
            PblVector3 vCamera;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vCamera, (const D3DXVECTOR3*)&vPoint,
                                   (const D3DXMATRIX*)&pCamera->GetMatrixInverse());
            if (vCamera.z >= 0.0f)
            {
                HUD::Event(weaponEvents.lockOnDisable, true).Send();
            }
            else
            {
                PblVector3 vScreen = pCamera->TransformCameraPointToProjectionSpace(vCamera);
                vScreen.x = vScreen.x * 0.5f + 0.5f;
                vScreen.y = vScreen.y * 0.5f + 0.5f;
                vScreen.z = 0.0f;
                HUD::Event(weaponEvents.lockOnPosition, &vScreen).Send();
            }
        }
    }

    const float fHitFriend = pCharacter->GetObjectHitValue(1);
    const float fHitEnemy = pCharacter->GetObjectHitValue(-1);
    if (fHitFriend != 0.0f || pCharacter->GetObjectHitValue(0) != 0.0f || fHitEnemy != 0.0f)
    {
        float fMultiplier = pCharacter->GetObjectHitMultiplier(0);
        const float fFriendMultiplier = pCharacter->GetObjectHitMultiplier(1);
        if (!(fMultiplier > fFriendMultiplier))
        {
            fMultiplier = fFriendMultiplier;
        }
        const float fEnemyMultiplier = pCharacter->GetObjectHitMultiplier(-1);
        if (fEnemyMultiplier > fMultiplier)
        {
            fMultiplier = fEnemyMultiplier;
        }
        HUD::Event(fMultiplier > 1.0f ? weaponEvents.targetHitCritical : weaponEvents.targetHit, true).Send();
        const RedColor color = fHitFriend != 0.0f ? Team::GetColorByAffiliation(1)
                             : fHitEnemy != 0.0f ? Team::GetColorByAffiliation(-1)
                                                 : gLocalColor;
        HUD::Event(weaponEvents.targetHitColor, &color).Send();
    }
}

// FUNCTION: SWBF2 0x005619f0
void HUD::GameEvents::UpdateHealth(const Character* const pCharacter, const GameObject* const pObject,
                                   PlayerEvents& events, PlayerData& data)
{
    float fHealth = pObject->mCurHealth;
    if (0.0f > fHealth)
    {
        fHealth = 0.0f;
    }

    const bool bHero = pObject->GetGameObjectClass()->mHeroFlag;
    if (data.isHero != bHero)
    {
        HUD::Event(bHero ? events.healthAsHeroDisable : events.healthDisable, true).Send();
        data.isHero = bHero;
    }

    if (BonusManager::Instance()->IsTeamBonusActive(pCharacter->mTeamNumber, 0xc10b9cdc))
    {
        const int iPlayer = pCharacter->mPlayerId;
        if (iPlayer >= 0 && iPlayer < 1)
        {
            HUD::Event(gPlayerEvents[iPlayer].healthRegenPulseRate, fHealth > data.health ? 0.5f : FLT_MAX).Send();
        }
    }

    if (data.unknownBool00D8)
    {
        data.health = 0.0f;
        HUD::Event(bHero ? events.healthAsHeroDisable : events.healthDisable, true).Send();
    }
    else if (fHealth != data.health)
    {
        const float fMaxHealth = pObject->mMaxHealth;
        float fFraction = 0.0f;
        float fBonusFraction = 0.0f;
        if (fMaxHealth != 0.0f)
        {
            fFraction = fHealth / fMaxHealth;
            if (fFraction > 1.0f && !bHero)
            {
                fBonusFraction = fFraction - 1.0f;
                if (!(1.0f > fBonusFraction))
                {
                    fBonusFraction = 1.0f;
                }
            }
        }
        if (data.health == -1.0f)
        {
            HUD::Event(events.spawn, true).Send();
        }

        EventClass* pHealth;
        EventClass* pFraction;
        if (bHero)
        {
            pHealth = events.healthAsHero;
            pFraction = events.healthAsHeroFraction;
        }
        else if (pCharacter->mVehicle)
        {
            pHealth = events.healthInVehicle;
            pFraction = events.healthInVehicleFraction;
        }
        else
        {
            pHealth = events.health;
            pFraction = events.healthFraction;
        }
        HUD::Event health;
        HUD::Event::Data payload;
        payload.intValue = (int)fHealth;
        InitializeEventWithData(&health, pHealth, payload)->Send();
        HUD::Event(pFraction, fFraction).Send();
        HUD::Event(events.bonusHealthFraction, fBonusFraction).Send();
        data.health = fHealth;
        if (fHealth == 0.0f)
        {
            HUD::GameEvents::Die(events, data);
        }
    }

    bool bCanChangeClass = false;
    const Entity* pEntity = pObject->GetEntity();
    if (pEntity->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        bCanChangeClass = static_cast<const EntitySoldier*>(pEntity)->CanChangeClass();
    }
    else if (pEntity->IsRtti(EntityDroideka::rttiHashEntityDroideka))
    {
        bCanChangeClass = static_cast<const EntityDroideka*>(pEntity)->mCanChangeClass;
    }
    if (ToolTipsDisplay::sInstance && bCanChangeClass)
    {
        ToolTipsDisplay::sInstance->ResetTipLatch(0);
        ToolTipsDisplay::sInstance->ShowTip(TTT_SoldierTargetCP, 0);
    }
}

// FUNCTION: SWBF2 0x00561d70
void HUD::GameEvents::UpdateVehicleHealth(int iIndex, const Character* const pCharacter, PlayerEvents& events,
                                          PlayerData& data)
{
    const Controllable* pVehicle = pCharacter->mVehicle;
    const GameObject* pObject = pVehicle ? pVehicle->GetGameObject() : 0;
    const bool bInVehicle = pObject != 0;
    const bool bChanged = bInVehicle != data.inVehicle;
    if (bChanged)
    {
        EventClass* pDisable;
        if (bInVehicle)
        {
            pDisable = events.healthDisable;
        }
        else
        {
            pDisable = events.healthInVehicleDisable;
            HUD::Event(events.vehicleHealthDisable, true).Send();
            data.vehicleHealth = -1.0f;
        }
        data.health = -1.0f;
        HUD::Event(pDisable, true).Send();
        data.inVehicle = bInVehicle;
    }

    if (data.inVehicle)
    {
        float fHealth = pObject->mCurHealth;
        if (0.0f > fHealth)
        {
            fHealth = 0.0f;
        }
        if (data.unknownBool00D8)
        {
            HUD::Event(events.vehicleHealthDisable, true).Send();
            data.vehicleHealth = -1.0f;
        }
        else if (fHealth != data.vehicleHealth)
        {
            const float fMaxHealth = pObject->mMaxHealth;
            float fFraction = 0.0f;
            if (fMaxHealth != 0.0f)
            {
                fFraction = fHealth / fMaxHealth;
            }
            HUD::Event health;
            HUD::Event::Data payload;
            payload.intValue = (int)fHealth;
            InitializeEventWithData(&health, events.vehicleHealth, payload)->Send();
            HUD::Event(events.vehicleHealthFraction, fFraction).Send();
            data.vehicleHealth = fHealth;
            if (fHealth == 0.0f)
            {
                HUD::GameEvents::Die(events, data);
            }
        }

        HUD::ElementVehicleSeating* pSeating = HUD::ElementVehicleSeating::FindByPlayerIndex(iIndex);
        if (bChanged || (pSeating && pSeating->GetSeatingDisplayChanged()))
        {
            const unsigned int uModelID = pObject->GetEntityClass()->mHUDModelID;
            const RedModel* pModel = uModelID ? RedModel::_HashTable.Find(uModelID) : 0;
            HUD::Event(events.vehicleSeatingMesh, pModel).Send();
        }
    }

    UpdateHackWeapon(pObject, events.vehicleHackedTime, events.vehicleHackedTimeFraction,
                     events.vehicleHackedTimeDisable, 0x67fdfe71, data.vehicleBeingHacked);
}

// FUNCTION: SWBF2 0x00561f60
void HUD::GameEvents::UpdateJetpack(const GameObject* const pObject, const EntitySoldierClass* const pClass,
                                    PlayerEvents& events, PlayerData& data)
{
    const bool bJetpack = pClass && pClass->mJetFuelRechargeRate > 0.0f && pClass->JetJumpShowHUD();
    if (data.hasJetpack != bJetpack)
    {
        if (bJetpack)
        {
            HUD::Event(events.jetFuelThreshold, pClass->GetJetFuelMinBorder()).Send();
        }
        else
        {
            HUD::Event(events.jetDisable, true).Send();
            data.jetFuel = -1.0f;
        }
        data.hasJetpack = bJetpack;
    }

    if (!data.hasJetpack)
    {
        return;
    }
    const float fJetFuel = static_cast<const EntitySoldier*>(pObject)->mJetFuel;
    if (fJetFuel == data.jetFuel)
    {
        return;
    }
    float fWarning = 0.0f;
    const float fMinBorder = pClass->GetJetFuelMinBorder();
    if (!(fJetFuel > fMinBorder))
    {
        fWarning = 1.0f - fJetFuel / fMinBorder;
    }
    HUD::Event(events.jetFuelWarning, fWarning).Send();
    HUD::Event(events.jetFuelFraction, fJetFuel).Send();
    data.jetFuel = fJetFuel;
}

// FUNCTION: SWBF2 0x00562060
void HUD::GameEvents::UpdateShield(const GameObject* const pObject, PlayerEvents& events, PlayerData& data)
{
    const float fMaxShield = pObject->mMaxShield;
    const float fShield = pObject->mCurShield;
    const bool bShield = fMaxShield > 0.0f && fShield > 0.0001f;
    if (bShield != data.hasShield)
    {
        if (!bShield)
        {
            HUD::Event(events.jetDisable, true).Send();
            data.shield = -1.0f;
        }
        data.hasShield = bShield;
    }

    if (!data.hasShield)
    {
        return;
    }
    if (data.unknownBool00D8)
    {
        HUD::Event(events.jetDisable, true).Send();
        data.shield = -1.0f;
        return;
    }
    if (fShield == data.shield)
    {
        return;
    }
    float fFraction = fShield / fMaxShield;
    if (0.0f > fFraction)
    {
        fFraction = 0.0f;
    }
    else if (fFraction > 1.0f)
    {
        fFraction = 1.0f;
    }
    HUD::Event(events.jetFuelFraction, fFraction).Send();
    HUD::Event(events.jetFuelWarning, 1.0f - fFraction).Send();
    HUD::Event(events.jetFuelThreshold, 0.0f).Send();
}

// FUNCTION: SWBF2 0x005621a0
void HUD::GameEvents::UpdateFlags(const Character* const pCharacter, const Controllable* const pControllable,
                                  PlayerEvents& events, PlayerData& data)
{
    int iFriendlyCarried = 0;
    int iFriendlyDropped = 0;
    int iEnemyCarried = 0;
    int iEnemyDropped = 0;
    const bool bFirstPerson = pControllable->CheckFirstPerson(pCharacter->mPlayerId);
    bool bPlayerCarrier = false;
    for (PblList<FlagItem>::Itor it(*FlagItem::sList); (FlagItem*)it; ++it)
    {
        const FlagItem* pFlag = it;
        const Team* pTeam = pCharacter->mTeamPtr;
        const int iAffiliation = pTeam->GetAffiliation(reinterpret_cast<const GameObject*>(pFlag)->GetTeam());
        const Character* pCarrier = reinterpret_cast<const EntityItem*>(pFlag)->mOwner;
        if (pCarrier)
        {
            bPlayerCarrier |= pCarrier == pCharacter;
            if (iAffiliation > 0)
            {
                ++iFriendlyCarried;
            }
            else if (iAffiliation < 0)
            {
                ++iEnemyCarried;
            }
            else if (pTeam->GetAffiliation(pCarrier->mTeamNumber) > 0)
            {
                ++iEnemyCarried;
            }
            else
            {
                ++iFriendlyCarried;
            }
        }
        else if (pFlag->IsMoved())
        {
            if (iAffiliation > 0)
            {
                ++iFriendlyDropped;
            }
            else
            {
                ++iEnemyDropped;
            }
        }
    }

    const bool bCarried = bFirstPerson && bPlayerCarrier;
    if (bCarried != data.flagPlayerCarried)
    {
        if (bCarried)
        {
            HUD::Event(events.flagPlayerCarried, true).Send();
            data.flagPlayerCarried = true;
        }
        else
        {
            data.flagPlayerCarried = false;
            HUD::Event(events.flagPlayerDisable, true).Send();
        }
    }

    if (iFriendlyCarried != data.numFriendlyFlagsCarried)
    {
        if (iFriendlyCarried > 0)
        {
            HUD::Event(events.flagFriendCarried, true).Send();
            if (iFriendlyCarried > 1)
            {
                HUD::Event(events.flagFriendCarriedNumber, iFriendlyCarried).Send();
            }
            else
            {
                HUD::Event(events.flagFriendCarriedNumberDisable, true).Send();
            }
        }
        else
        {
            HUD::Event(events.flagFriendCarriedDisable, true).Send();
            HUD::Event(events.flagFriendCarriedNumberDisable, true).Send();
        }
        data.numFriendlyFlagsCarried = iFriendlyCarried;
    }

    if (iFriendlyDropped != data.numFriendlyFlagsDropped)
    {
        if (iFriendlyDropped > 0)
        {
            HUD::Event(events.flagFriendDropped, true).Send();
            if (iFriendlyDropped > 1)
            {
                HUD::Event(events.flagFriendDroppedNumber, iFriendlyDropped).Send();
            }
            else
            {
                HUD::Event(events.flagFriendDroppedNumberDisable, true).Send();
            }
        }
        else
        {
            HUD::Event(events.flagFriendDroppedDisable, true).Send();
            HUD::Event(events.flagFriendDroppedNumberDisable, true).Send();
        }
        data.numFriendlyFlagsDropped = iFriendlyDropped;
    }

    if (iEnemyCarried != data.numEnemyFlagsCarried)
    {
        if (iEnemyCarried > 0)
        {
            HUD::Event(events.flagEnemyCarried, true).Send();
            if (iEnemyCarried > 1)
            {
                HUD::Event(events.flagEnemyCarriedNumber, iEnemyCarried).Send();
            }
            else
            {
                HUD::Event(events.flagEnemyCarriedNumberDisable, true).Send();
            }
        }
        else
        {
            HUD::Event(events.flagEnemyCarriedDisable, true).Send();
            HUD::Event(events.flagEnemyCarriedNumberDisable, true).Send();
        }
        data.numEnemyFlagsCarried = iEnemyCarried;
    }

    if (iEnemyDropped != data.numEnemyFlagsDropped)
    {
        if (iEnemyDropped > 0)
        {
            HUD::Event(events.flagEnemyDropped, true).Send();
            if (iEnemyDropped > 1)
            {
                HUD::Event(events.flagEnemyDroppedNumber, iEnemyDropped).Send();
            }
            else
            {
                HUD::Event(events.flagEnemyDroppedNumberDisable, true).Send();
            }
        }
        else
        {
            HUD::Event(events.flagEnemyDroppedDisable, true).Send();
            HUD::Event(events.flagEnemyDroppedNumberDisable, true).Send();
        }
        data.numEnemyFlagsDropped = iEnemyDropped;
    }
}

// FUNCTION: SWBF2 0x005624a0
void HUD::GameEvents::UpdateEnergy(const Controllable* const pControllable, PlayerEvents& events, PlayerData& data)
{
    const EnergyBar* pBar = const_cast<Controllable*>(pControllable)->GetEnergyBar();
    if (!pBar)
    {
        return;
    }

    const bool bHasEnergy = pBar->m_pClass->m_fEnergyMax > 0.0f;
    if (bHasEnergy != data.hasEnergy)
    {
        if (!bHasEnergy)
        {
            HUD::Event(events.energyDisable, true).Send();
            data.energy = -1.0f;
        }
        data.hasEnergy = bHasEnergy;
    }
    if (!data.hasEnergy)
    {
        return;
    }

    const float fEnergy = pBar->m_fEnergy;
    const Character* pCharacter = pControllable->mCharacter;
    if (pCharacter && BonusManager::Instance()->IsTeamBonusActive(pCharacter->mTeamNumber, 0x85ad826d))
    {
        const int iPlayer = pCharacter->mPlayerId;
        if (iPlayer >= 0 && iPlayer < 1)
        {
            HUD::Event(gPlayerEvents[iPlayer].energyRegenPulseRate, fEnergy > data.energy ? 0.5f : FLT_MAX).Send();
        }
    }

    if (data.unknownBool00D8)
    {
        HUD::Event(events.energyDisable, true).Send();
        data.energy = -1.0f;
        return;
    }
    if (fEnergy == data.energy)
    {
        return;
    }
    HUD::Event(events.energy, fEnergy).Send();
    const float fEnergyMax = pBar->m_pClass->m_fEnergyMax;
    float fFraction = 0.0f;
    if (fEnergyMax > 0.0f)
    {
        fFraction = pBar->m_fEnergy / fEnergyMax;
    }
    HUD::Event(events.energyFraction, (float)fabs(fFraction)).Send();
    if (pBar->m_bOverheat)
    {
        HUD::Event(events.energyOverburn, 0.0f > fEnergy ? 1.0f : 0.0f).Send();
    }
    data.energy = fEnergy;
}

// FUNCTION: SWBF2 0x00562690
void HUD::GameEvents::UpdateTeam(const Character* const pCharacter, PlayerEvents& events, PlayerData& data)
{
    SpaceAssault* pSpaceAssault = SpaceAssault::mInstance;
    const int iTeam = pCharacter->mTeamNumber;
    Team* apTeams[2] = { Team::sTeams[iTeam], Team::sTeams[iTeam == 1 ? 2 : 1] };

    if (iTeam != data.teamNumber)
    {
        if (iTeam >= 1)
        {
            for (unsigned int i = 0; i < 2; ++i)
            {
                Team* pTeam = apTeams[i];
                if (pSpaceAssault && pSpaceAssault->mbEnabled)
                {
                    ShowTeamPoints(pTeam->mTeamNumber, true);
                }
                else if (FlagItem::sFlagGameplayType == FlagItem::FLAG_GAMEPLAY_1FLAG ||
                         FlagItem::sFlagGameplayType == FlagItem::FLAG_GAMEPLAY_2FLAG)
                {
                    RedTexture* pTexture =
                        RedTexture::Find(data.teamData[i].showPoints ? pTeam->mCTFIcon : pTeam->mConquestIcon);
                    if (pTexture || (pTexture = RedTexture::Find(pTeam->mIcon)) != 0)
                    {
                        HUD::Event(events.teams[i].texture, pTexture).Send();
                    }
                }
                data.teamData[i].points = -99999;
                data.teamData[i].reinforcements = -1;
                data.teamData[i].bleedRate = -1.0f;
            }
        }
        data.teamNumber = iTeam;
    }

    if (data.teamNumber < 1)
    {
        return;
    }

    for (unsigned int i = 0; i < 2; ++i)
    {
        Team* pTeam = apTeams[i];
        const int iMaxReinforcements = pTeam->mMaxReinforcements;
        const int iReinforcements =
            iMaxReinforcements == 0x7fffffff ? 0x7fffffff : pTeam->mCurReinforcements;
        const int iPoints = pTeam->mPoints;
        TeamData& teamData = data.teamData[i];
        TeamEvents& teamEvents = events.teams[i];

        if (teamData.showPoints && iPoints != teamData.points)
        {
            HUD::Event(teamEvents.points, iPoints).Send();
            int iLimit;
            if (pSpaceAssault && pSpaceAssault->mbEnabled)
            {
                iLimit = Lua_Callbacks::gSpaceAssaultScoreLimit[Lua_Callbacks::gCurRules];
            }
            else
            {
                switch (Lua_Callbacks::gHuntShowScoreLimit)
                {
                case 1:
                    iLimit = Lua_Callbacks::gHuntScoreLimit[Lua_Callbacks::gCurRules];
                    break;
                case 2:
                    iLimit = Lua_Callbacks::gSpaceAssaultScoreLimit[Lua_Callbacks::gCurRules];
                    break;
                case 3:
                    iLimit = Lua_Callbacks::gUberScoreLimit;
                    break;
                default:
                    iLimit = 0;
                    break;
                }
            }
            unsigned short acText[256];
            if (iLimit)
            {
                PblUnicode::usnprintf(acText, 255, "%d/%d", iPoints, iLimit);
            }
            else
            {
                PblUnicode::usnprintf(acText, 255, "%d", iPoints);
            }
            HUD::Event(teamEvents.pointsText, (const wchar_t*)acText).Send();
            teamData.points = iPoints;
        }

        if (!teamData.showPoints)
        {
            if (iReinforcements != teamData.reinforcements)
            {
                HUD::Event(teamEvents.reinforcements, iReinforcements).Send();
                HUD::Event(teamEvents.reinforcementsFraction,
                           (float)iReinforcements / (float)iMaxReinforcements).Send();
                teamData.reinforcements = iReinforcements;
            }
            const float fBleedRate = pTeam->mBleedRate;
            if (fBleedRate != teamData.bleedRate)
            {
                HUD::Event(teamEvents.bleedRate, fBleedRate == 0.0f ? FLT_MAX : 1.0f / fBleedRate).Send();
                teamData.bleedRate = fBleedRate;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00562970
void HUD::GameEvents::UpdateStatistics(const Character* const pCharacter, PlayerEvents& events, PlayerData& data)
{
    PlayerStats& stats = Character::GetPlayerStats(const_cast<Character*>(pCharacter));
    const int aiStatistics[5] = {
        stats.GetPoints(true) + stats.GetPoints(false),
        stats.GetKills(-1, 1) + stats.GetKills(-1, 0),
        stats.GetDeaths(-1, 1) + stats.GetDeaths(-1, 0),
        stats.GetPoints(false),
        stats.GetCommandPostPoints(-1, 0),
    };

    bool bChanged = false;
    for (unsigned int i = 0; i < 5; ++i)
    {
        const int iValue = aiStatistics[i];
        if (iValue == data.statisticData[i])
        {
            continue;
        }
        if (data.statisticData[i] == -1)
        {
            data.statisticData[i] = 0;
        }
        HUD::Event(events.statistic[i], iValue).Send();

        unsigned short acDelta[128];
        const int iDelta = iValue - data.statisticData[i];
        if (iDelta)
        {
            char acBuffer[128];
            _snprintf(acBuffer, 127, "%s%d", iDelta > 0 ? "+" : "", iDelta);
            PblUnicode::ustrncnv(acDelta, acBuffer, 128);
        }
        else
        {
            acDelta[0] = 0;
        }
        HUD::Event(events.statisticDelta[i], (const wchar_t*)acDelta).Send();
        data.statisticData[i] = iValue;
        bChanged = true;
    }
    if (bChanged)
    {
        HUD::Event(events.statisticsChanged, true).Send();
    }
}

// FUNCTION: SWBF2 0x00562ae0
void HUD::GameEvents::UpdateMissleLock(const GameObject* const pObject, const Controllable* const pControllable,
                                       PlayerEvents& events, PlayerData& data)
{
    const bool bLocked = (pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer) ||
                          pObject->IsRtti(MountedTurret::rttiHashMountedTurret)) &&
                         pControllable->GetEnemyLockedOnMeState() > 0.0f;
    if (bLocked != data.missileLocked)
    {
        HUD::Event(bLocked ? events.missileLock : events.missileLockDisable, true).Send();
        data.missileLocked = bLocked;
    }
    if (bLocked)
    {
        HUD::Event distance;
        HUD::Event::Data payload;
        payload.intValue = (int)pControllable->mEnemyLockedOnMeDistance;
        InitializeEventWithData(&distance, events.missileLockDistance, payload)->Send();
    }
}

// FUNCTION: SWBF2 0x005635e0
void HUD::GameEvents::HideObjectiveDetailsElements(PlayerEvents& events, PlayerData& data)
{
    (void)data;
    HUD::Event(events.pressSelectToReturnDisable, true).Send();
    HUD::Event(events.objectiveDetailsDisable, true).Send();
    HUD::Event(events.hintsAvailableDisable, true).Send();
}

// FUNCTION: SWBF2 0x00563740
void HUD::GameEvents::DisplayCenterMessage(unsigned int uTextHash, unsigned int uTeamMask,
                                           const RedColor* const pColor)
{
    const unsigned int uNumCameras = GameLoop::GetNumCameras();
    for (unsigned int i = 0; i < uNumCameras; ++i)
    {
        Character* pCharacter = NetGame::GetLocalPlayer(i);
        if (!pCharacter || !(uTeamMask & (1u << pCharacter->mTeamNumber)))
        {
            continue;
        }
        HUD::GameEvents::PlayerEvents& events = HUD::GameEvents::gPlayerEvents[i];
        if (pColor)
        {
            HUD::Event(events.messageColor, pColor).Send();
        }
        const unsigned short* pText = RedLocalizeDB::Find(uTextHash, false);
        HUD::Event(events.message, (const wchar_t*)pText).Send();
    }
}

// FUNCTION: SWBF2 0x0055e360
HUD::GameEvents::PlayerData::~PlayerData()
{
}

// FUNCTION: SWBF2 0x0055e390
HUD::GameEvents::WeaponData::~WeaponData()
{
}
