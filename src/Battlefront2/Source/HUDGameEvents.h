#pragma once

#include <stddef.h>

#include "HUDElementMap.h"
#include "HUDEvent.h"
#include "PebbleFL/Common/PblHandle.h"

class GameObject;
class Weapon;
class WeaponClass;
class Timer;
class Controllable;

namespace HUD {
namespace GameEvents {

struct WeaponEvents
{
    EventClass* change;
    EventClass* disable;
    EventClass* ammoTotalBullets;
    EventClass* ammoTotalFraction;
    EventClass* ammoClipBullets;
    EventClass* ammoClipFraction;
    EventClass* ammoInfinite;
    EventClass* heat;
    EventClass* charge;
    EventClass* refire;
    EventClass* name;
    EventClass* targetName;
    EventClass* targetClassName;
    EventClass* targetShieldName;
    EventClass* targetHealth;
    EventClass* targetHealthFraction;
    EventClass* targetDisable;
    EventClass* targetDisableShieldName;
    EventClass* targetTeamColor;
    EventClass* targetTeamColorBright;
    EventClass* targetHit;
    EventClass* targetHitCritical;
    EventClass* targetHitColor;
    EventClass* reticuleDisable;
    EventClass* reticulePosition;
    EventClass* lockOnPosition;
    EventClass* lockOnDisable;
};

struct TeamEvents
{
    EventClass* points;
    EventClass* pointsDisable;
    EventClass* pointsText;
    EventClass* reinforcements;
    EventClass* reinforcementsFraction;
    EventClass* reinforcementsDisable;
    EventClass* texture;
    EventClass* textureDisable;
    EventClass* bleedRate;
};

struct ObjectiveEvents
{
    EventClass* timer;
    EventClass* timerDisable;
    EventClass* victoryTimer;
    EventClass* victoryTimerDisable;
    EventClass* defeatTimer;
    EventClass* defeatTimerDisable;
    EventClass* hintPopup;
    EventClass* hintPopupDisable;
    EventClass* hintPopupPageNumber;
    EventClass* objectivePopup;
    EventClass* objectivePopupDisable;
    EventClass* selectionPopup;
    EventClass* selectionPopupDisable;
};

struct PlayerEvents
{
    EventClass* spawn;
    EventClass* die;
    EventClass* health;
    EventClass* healthFraction;
    EventClass* bonusHealthFraction;
    EventClass* healthRegenPulseRate;
    EventClass* healthDisable;
    EventClass* healthInVehicle;
    EventClass* healthInVehicleFraction;
    EventClass* healthInVehicleDisable;
    EventClass* healthAsHero;
    EventClass* healthAsHeroFraction;
    EventClass* healthAsHeroDisable;
    EventClass* vehicleHealth;
    EventClass* vehicleHealthFraction;
    EventClass* vehicleHealthDisable;
    EventClass* vehicleSeatingMesh;
    EventClass* jetDisable;
    EventClass* jetFuelFraction;
    EventClass* jetFuelWarning;
    EventClass* jetFuelThreshold;
    EventClass* energy;
    EventClass* energyFraction;
    EventClass* energyRegenPulseRate;
    EventClass* energyOverburn;
    EventClass* energyDisable;
    WeaponEvents weapons[2];
    EventClass* weaponsOverheat;
    EventClass* weaponsEnable;
    EventClass* weaponsDisable;
    EventClass* mapHideCPs;
    EventClass* mapMode;
    EventClass* mapModeToggle;
    EventClass* mapEnable;
    EventClass* mapDisable;
    EventClass* mapSpawn;
    EventClass* mapSpawnLarge;
    EventClass* mapSpawnLargeDisable;
    EventClass* index;
    EventClass* mapRefreshTarget;
    EventClass* mapRefreshPost;
    EventClass* mapRefreshMarker;
    EventClass* targetResetPlayer;
    EventClass* spawnDisplayEnable;
    EventClass* spawnDisplayDisable;
    EventClass* spawnDisplayMessage;
    EventClass* spawnDisplayVehText;
    EventClass* spawnDisplayCPText;
    EventClass* displayToolTips;
    EventClass* disableToolTips;
    EventClass* message;
    EventClass* messageColor;
    EventClass* messageDisable;
    EventClass* objectiveListEnable;
    EventClass* objectiveListDisable;
    EventClass* objectivesUpdated;
    EventClass* objectivesUpdatedDisable;
    EventClass* objectiveDetails;
    EventClass* objectiveDetailsDisable;
    EventClass* hintsAvailable;
    EventClass* hintsAvailableDisable;
    EventClass* pressSelectToReturn;
    EventClass* pressSelectToReturnDisable;
    TeamEvents teams[2];
    EventClass* spaceAssaultStatus;
    EventClass* spaceAssaultStatusDisable;
    EventClass* statistic[5];
    EventClass* statisticDelta[5];
    EventClass* statisticsChanged;
    EventClass* statisticsDisable;
    EventClass* flagFriendCarried;
    EventClass* flagFriendCarriedDisable;
    EventClass* flagFriendCarriedNumber;
    EventClass* flagFriendCarriedNumberDisable;
    EventClass* flagFriendDropped;
    EventClass* flagFriendDroppedDisable;
    EventClass* flagFriendDroppedNumber;
    EventClass* flagFriendDroppedNumberDisable;
    EventClass* flagPlayerCarried;
    EventClass* flagPlayerDropped;
    EventClass* flagPlayerDisable;
    EventClass* flagEnemyCarried;
    EventClass* flagEnemyCarriedDisable;
    EventClass* flagEnemyCarriedNumber;
    EventClass* flagEnemyCarriedNumberDisable;
    EventClass* flagEnemyDropped;
    EventClass* flagEnemyDroppedDisable;
    EventClass* flagEnemyDroppedNumber;
    EventClass* flagEnemyDroppedNumberDisable;
    EventClass* vehicleHackingTimeFraction;
    EventClass* vehicleHackingTime;
    EventClass* vehicleHackingTimeDisable;
    EventClass* vehicleHackingTimeName;
    EventClass* vehicleHackedTimeFraction;
    EventClass* vehicleHackedTime;
    EventClass* vehicleHackedTimeDisable;
    EventClass* vehicleHackedTimeName;
    EventClass* heroSelectTimerFraction;
    EventClass* heroSelectTimer;
    EventClass* heroSelectMessage;
    EventClass* heroSelectDisable;
    EventClass* commandPostCharge;
    EventClass* commandPostDisable;
    EventClass* commandPostOwnerColor;
    EventClass* commandPostDisputeColor;
    EventClass* commandPostDisputeEnable;
    EventClass* commandPostDisputeDisable;
    EventClass* lockOnName;
    EventClass* lockOnClassName;
    EventClass* lockOnHealth;
    EventClass* lockOnHealthFraction;
    EventClass* lockOnDisable;
    EventClass* lockOnShieldName;
    EventClass* lockOnDisableShieldName;
    EventClass* lockOnDistance;
    EventClass* lockOnDirection;
    EventClass* lockOnDirectionDisable;
    EventClass* lockOnTeamColor;
    EventClass* lockOnFlagCarrier;
    EventClass* lockOnFlagCarrierDisable;
    EventClass* missileLock;
    EventClass* missileLockDisable;
    EventClass* missileLockDistance;
    EventClass* reticuleAlpha;
};

struct WeaponData
{
    ~WeaponData();
    Weapon* weapon;
    WeaponClass* weaponClass;
    unsigned int weaponHash;
    float ammoTotalFraction;
    float clipTotalFraction;

    PblHandle<GameObject const> target;
    float targetHealth;
    bool showReticule;
    GameObject* targetLocked;
};

struct TeamData
{
    int points;
    int reinforcements;
    bool showPoints;
    float bleedRate;
};

struct PlayerData
{
    ~PlayerData();
    WeaponData weaponData[2];
    TeamData teamData[2];
    HUD::ElementMap::Mode mapMode;
    float health;
    bool hasJetpack;
    float jetFuel;
    bool hasShield;
    float shield;
    bool hasEnergy;
    float energy;
    bool inVehicle;
    float vehicleHealth;
    int teamNumber;
    bool noWeapons;
    bool isHero;
    int statisticData[5];
    int numFriendlyFlagsCarried;
    int numFriendlyFlagsDropped;
    int numEnemyFlagsCarried;
    int numEnemyFlagsDropped;
    bool flagPlayerCarried;
    bool vehicleBeingHacked;
    bool vehicleHacking;
    GameObject* lockedTarget;
    float lockedTargetHealth;
    float unknownFloat00D0;
    bool lockedDirectionEnabled;
    bool carriedFlagItem;
    bool missileLocked;
    bool scopeActive;
    bool unknownBool00D8;
};

extern char* gStatisticStrings[5];

extern PlayerEvents gPlayerEvents[1];
extern ObjectiveEvents gObjectiveEvents;
extern PlayerData gPlayerData[1];
extern EventClass* gTargetResetCommon;
extern Timer* gObjectiveTimer;

// FUNCTION: SWBF2 0x0055e3a0
void Open();

void Close();

const Timer* GetObjectiveTimer();
void SetObjectiveTimer(const Timer* const pTimer);

void Update(float fDeltaTime);

extern bool gInputMapLastPressed;

void Die(const Controllable* const pControllable);

void DisableTeamElements(PlayerEvents& events, PlayerData& data);
void DisablePlayerElements(PlayerEvents& events, PlayerData& data);
void Die(PlayerEvents& events, PlayerData& data);

void ShowTeamPoints(int iTeam, bool bShow);

void DisplayMessage(unsigned int uViewport, const unsigned short* const pText,
                    const RedColor* const pColor);
void DisplayTextChatMessage(unsigned int uViewport, const unsigned short* const pText,
                            const RedColor* const pColor);
void DisplayCenterMessage(unsigned int uTextHash, unsigned int uTeamMask, const RedColor* const pColor);

}
}

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
