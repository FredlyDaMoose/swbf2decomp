#pragma once
class GameObject;
namespace AIUtil
{

GameObject* FindFirstPlayerInVehicle(GameObject* pVehicle);
GameObject* FindFirstAIInVehicle(GameObject* pVehicle);
}

#include "PebbleFL/Common/PblHandle.h"

class Aimer;
class BaseHint;
class CommandPost;
class Character;
class Controllable;
class ControllableDestination;
class ControllableHeading;
class Entity;
class EntityFlyer;
class GameObject;
class Obstacle;
class PblMatrix;
class PblVector3;
class RedRegion;
class SnipeHelper;
class SnipeHint;
class UnitController;
class WeaponClass;

namespace AIUtil
{

    void Reset();
    void ResetSpied(Character* pWatcher);

    bool CanSeePosition(const PblVector3& vFrom, Controllable* pMe, const PblVector3& vTo,
                        const GameObject* pIgnore);
    void GrenadeAlert(const PblVector3& vPos, const GameObject* pObject, float* pBeginTime,
                      float* pLastEventTime);
    void Init();

    enum EntityType
    {
        ENTITY_SOLDIER = 0,
        ENTITY_DROID = 1,
        ENTITY_WALKER = 2,
        ENTITY_DROIDEKA = 3,
        ENTITY_TAUNTAUN = 4,
        ENTITY_FLYER = 5,
        ENTITY_HOVER = 6,
        ENTITY_TURRET = 7,
        ENTITY_BUILDING_ARMED = 8,
        ENTITY_BUILDING = 9,
        ENTITY_ITEM = 10,
        ENTITY_REMOTE_TERMINAL = 11
    };

    EntityType GetEntityType(const Entity* pEntity);

    GameObject const* GetAlivePtr(PblHandle<GameObject const> const&, bool);

    PblVector3 CalculateBubbleOffset(const Controllable* pControllable, const PblVector3& vPos,
                                     const GameObject* pTarget, const WeaponClass* pWeaponClass);

    int CountHumansInVehicle(const Controllable* pControllable);

    PblVector3 UnrotateVector(const PblMatrix& rMatrix, const PblVector3& vDir);

    GameObject* GetAlivePtr(PblHandle<GameObject> const&, bool);

    bool IsNearNonAllyCommandPost(const GameObject* pObject, const CommandPost** ppPost);
    void DeployCargo(GameObject* pObject, const CommandPost* pPost);

    bool AvoidBarriers(EntityFlyer* pFlyer, const PblVector3& vDest, PblVector3* pNewDest);

    extern bool sUberMode;

    extern bool gGroundFlyerMap;
    extern bool gStayInTurrets;
    extern bool gSkipPlayer;

    bool IsUberMode();

    float GetRandomFloat(float fMin, float fMax);

    void TellAIToExit(GameObject* pObject);

    int GetRandomIntPerSecond(int iMin, int iMax);

    bool IsSnipeHintNearby(const GameObject* pObj, const SnipeHelper& helper,
                           const SnipeHint** ppHint, float* pfNextCheckTime);

    bool IsEndGame(int);
    bool IsSniperGuy(const GameObject* pObj);

    bool ResolveCollision(const GameObject* pMe, const GameObject* pOther);
    bool IsRocketGuy(const GameObject* pObj);
    bool IsHeroGuy(const GameObject* pObj);
    bool IsValidEnemy(const GameObject* pMe, const GameObject* pObj);
    PblVector3 GetSafeNormal(const PblVector3& v);

    bool GetLeadPosition(float fTime, const Controllable* pMe, const GameObject* pTarget,
                         PblVector3* pvLead);

    extern float gSnipeSuitabilityDist;

    extern float gStationaryWaitTime;

    extern float gMaxFlyHeight;
    extern float gMinFlyHeight;

    extern float gMinPlayerFlyHeight;
    extern float gMaxPlayerFlyHeight;
    extern bool gAIAllowBlindJetJumps;

    int GetRandomInt(int iMin, int iMax);

    bool IsPlayerTeam(int iTeam);

    bool IsHintAvailable(const GameObject* pObj, const BaseHint* pHint);

    PblVector3 GetCommandPostOffset(const CommandPost* pPost);
    PblVector3 GetCommandPostPos(const CommandPost* pPost);
    bool IsNearPost(const Controllable* pMe, const CommandPost* pPost, const PblVector3& vOffset);

    bool HasCPMoved(const CommandPost* pPost, Controllable* pMe, const PblVector3& vLastPos,
                    float fLastTime, const PblVector3& vDir);

    bool IsVehicle(const GameObject* pObj);

    bool IsATAT(const Controllable* pControllable);
    bool AreEnemiesInRange(int iTeam, const PblVector3& vPos, float fRange);

    PblVector3 GetMyVisionPos(const Controllable* pMe);
    PblVector3 GetHisVisionTargetPos(const Controllable* pMe, const GameObject* pObj);
    bool CanSeePosition(Controllable* pMe, const PblVector3& vPos, const GameObject* pObj);
    bool CanSee(Controllable* pMe, const GameObject* pObj, PblVector3* pPos);
    bool DirectLineToTarget(const GameObject* pObj, const PblVector3& vPos);
    void Slide(GameObject* pMe, const GameObject* pTarget, bool bUseAttackHelper,
               bool* pbIsPathing, float* pfPathTime, float fMinDist, float fOffset);
    int CountUnitsInVehicle(const Controllable* pControllable);
    PblVector3 GetRandomAccessiblePoint(const UnitController* pCtrl);

    PblVector3 GetSafeNormalXZ(const PblVector3& v);
    Obstacle* GetGameObjectBarrier(GameObject* pObj);
    const Obstacle* GetGameObjectBarrier(const GameObject* pObj);

    float GetCollisionRadius(const GameObject* pObj);
    int GetAIType(const GameObject* pObj);

    float StopDist(const Controllable* pControllable);
    float StopDistIntermediate(const Controllable* pControllable);

    void CalculateHeadingControls(const Controllable* pMe, float fScale,
                                  const ControllableDestination& rDest,
                                  const ControllableHeading& rHeading, float fTurnRate,
                                  float* pfTurn, float fPitchRate, float* pfPitch, bool bFromEye);

    void CalculateFlyerSteerPoint(const PblVector3& vGoto, const PblVector3& vDir,
                                  const PblVector3& vPos, const PblVector3& vForward,
                                  float fTurn, PblVector3& vSteer, int eDestSide);
    void CalculateSoldierSteerPoint(const PblVector3& vPrev, const PblVector3& vGoto,
                                    const PblVector3& vPos, float fSpeed,
                                    PblVector3& vSteer);

    bool GetAlivePtrCommon(const GameObject* pObj, bool bIncludeBuildings);
    PblVector3 GetRandomPtInRegion(const RedRegion* pRegion);
    const Obstacle* _GetGameObjectBarrier(const GameObject* pObj);
    PblVector3 DumbDown(const GameObject* pObject, float fDeltaTime, Aimer* pAimer,
                        const PblVector3& vDir);

    bool GetTangentPointToCircle(const PblVector3& vCenter, const PblVector3& vPoint, float fRadius,
                                 PblVector3& vTangent, PblVector3& vDir, bool bClockwise);
}

namespace AIUtil {
void UpdatePhysics(const Controllable* pMe, float fScale, const ControllableDestination& rDest,
                const ControllableHeading& rHeading, float fThrustRate, float fStrafeRate,
                float fTurnRate, float fPitchRate, float* pfThrust, float* pfStrafe,
                float* pfTurn, float* pfPitch);
}
namespace AIUtil { bool __fastcall CanPotentiallySee(Controllable* pMe, const GameObject* pObject, float* pOut); }
namespace AIUtil { void __fastcall AddSpied(Character* pWatcher, Character* pTarget); }
namespace AIUtil { bool __fastcall IsSpied(const Character* pWatcher, const Character* pTarget); }
namespace AIUtil { bool __fastcall ShouldStayInVehicle(Controllable* pMe); }
namespace AIUtil { bool ShouldSoldierDodgeThisType(int eType); }
namespace AIUtil { bool IsImportant(const GameObject* pObj); }
namespace AIUtil { float GetThreatSwitchTime(const UnitController* pCtrl); }

namespace AIUtil { float GetTargetSpeedForRealSpeed(float fNum, float fDenom); }
namespace AIUtil { float GetThrustForTargetSpeed(float fT, float fA, float fB, float fC); }
