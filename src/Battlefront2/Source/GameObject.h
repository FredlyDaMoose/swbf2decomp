#pragma once

#include <stddef.h>

#include "Damageable.h"
#include "EntityGeometry.h"
#include "GameSound.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblListSingle.h"

struct DamageEffectClass
{

    DamageEffectClass(float fPercent = 0.0f, float fPercentStop = 0.0f, unsigned int uNameHash = 0,
                      unsigned int uHardPoint = 0,
                      const PblVector3& vAttachOffset = PblVector3(0.0f, 0.0f, 0.0f),
                      float fScale = 1.0f, float fInheritVelocity = 0.0f)
        : mPercent(fPercent), mPercentStop(fPercentStop), mNameHash(uNameHash),
          mHardPoint(uHardPoint), mAttachOffset(vAttachOffset), mScale(fScale),
          mInheritVelocity(fInheritVelocity)
    {
    }

    float mPercent;
    float mPercentStop;
    unsigned int mNameHash;
    unsigned int mHardPoint;
    PblVector3 mAttachOffset;
    float mScale;
    float mInheritVelocity;
    GameSound mSound;
};
 
 

namespace AI { class VOSound; }

// VTABLE: SWBF2 0x0079fac8
class GameObjectClass : public EntityGeometryClass
{
public:

    enum SoldierType
    {
        SOLDIER_SCOUT = 0,
        SOLDIER_ROCKET = 1,
        SOLDIER_REPAIR = 2,
        SOLDIER_PILOT = 3,
        SOLDIER_TROOPER = 4,
        SOLDIER_UNKNOWN = 5
    };

    GameObjectClass(unsigned int uHash);

    GameObjectClass(GameObjectClass* pBase, unsigned int uHash);

    virtual bool IsRtti(unsigned int uRtti);
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();

    virtual ~GameObjectClass();

    virtual int GetSoldierType();
    virtual int GetVehicleType();
    virtual const void* GetControllableClass();
    virtual int GetWeaponCount();
    virtual const class WeaponClass* GetWeaponClass(int iIndex);
    virtual float GetCollisionInflict();
    virtual const DamageEffectClass* GetDamageEffectClass(int iIndex);
    virtual float GetTimeRequiredToEject();
    virtual float GetReboardTime();

    enum VehicleType
    {
        VEHICLE_SCOUT = 0,
        VEHICLE_LIGHT = 1,
        VEHICLE_MEDIUM = 2,
        VEHICLE_HEAVY = 3,
        VEHICLE_FIGHTER = 4,
        VEHICLE_TRANSPORT = 5,
        VEHICLE_CAPITOL = 6,
        VEHICLE_REMOTE_TERMINAL = 7,
        VEHICLE_ATAT = 8,
        VEHICLE_UNKNOWN = 9
    };

    void SetHeroFlag() { mHeroFlag = true; }

    // FUNCTION: SWBF2 0x00429270
    bool GetHeroFlag() const { return mHeroFlag; }

protected:

    VehicleType FindVehicleType(const char* pszName);

public:

    bool mHoldForPlayer;
    HealthType mHealthType;
    HealthType mHealthTypeForLockOn;
    float mMaxHealth;
    float mAddHealth;
    float mMaxShield;
    float mAddShield;
    float mPilotSkillRepairScale;
    float mScanningRange;
    float mTransmitRange;
    float mLockOnMinDist;
    float mLockOnMaxDist;
    float mLockOnMaxTrackDist;
    GameSound mSoundDeath;
    GameSound mSoundHurt;
    GameSound mSoundHurtFall;
    PblListSingle mSoundDamageRegionList;

    int mAIType;
    bool mAINoRepair : 1;
    bool mHeroFlag : 1;
    bool mIsNeutralWhenEmpty : 1;
    bool mAvailableForAnyTeam : 1;
    bool mIsNotTargetableByPlayer : 1;
    bool mbHideHealthBar : 1;

    bool IsNotTargetableByPlayer() const { return mIsNotTargetableByPlayer; }

    bool GetAINoRepair() const { return mAINoRepair; }

    bool IsNeutralWhenEmpty() { return mIsNeutralWhenEmpty; }

    // FUNCTION: SWBF2 0x005373e0
    bool AvailableForAnyTeam() const { return mAvailableForAnyTeam; }
    float mSwitchClassRadius;
    DamageEffectClass* mDamageEffectClass[10];
    AI::VOSound* mVOSoundList;
    int mVOUnitType;
    float mVORadius;
    float mAICollisionRadius;
    int mPointsToUnlock;
    unsigned int mDisabledEffect;
    float mRadiusDamageSizeScale;

    static PblHash rttiHashGameObjectClass;
    static char rttiNameGameObjectClass[];
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

class DamageRegionSound
{
public:

    DamageRegionSound(unsigned int uRegionID, const GameSound& sound, PblListSingle& list);

    GameSound mSound;
    unsigned int mRegionID;
    PblListSingle::Node mNode;
};
 

class GameObjectInterpolator
{
public:
    GameObjectInterpolator();

    void GetMatrix(int iNetUniqueId, PblMatrix& mMatrix);

    void SetNext(int iNetUniqueId, const PblMatrix* pMatrix);
    void SetNextVector(int iNetUniqueId, const PblVector3* pVector);

    void SetNextFloat(int iNetUniqueId, int iIndex, float fValue);

    void GetVector(int iNetUniqueId, PblVector3& vVector);

    float GetLoopingFloat(int iNetUniqueId, int iIndex, float fMax, float fDirection, bool& bWrapped);

    float GetFloat(int iNetUniqueId, int iIndex);

    float GetCurFloat(int iNetUniqueId, int iIndex);
    float GetPrevFloat(int iNetUniqueId, int iIndex);

    int count[64];
    PblMatrix mat[2][64];
    float mFloats[2][3][64];
    PblVector3 vec[2][64];
};
 

extern GameObjectInterpolator gameObjectInterpolator;

enum EntityType
{
    ENT_NULL, ENT_SOLDIER, ENT_FLYER, ENT_WALKER, ENT_DROIDEKA, ENT_TAUNTAUN,
    ENT_DROID, ENT_HOVER, ENT_MINE, ENT_POWERUP, ENT_ARMED_DYNAMIC, ENT_POST,
    ENT_POST_HOVER, ENT_POST_WALKER, ENT_POST_FLYER, ENT_ARMED, ENT_ANIMATED,
    ENT_BUILDING, ENT_POWERUPSTATION, ENT_TRAP, ENT_ASTEROID, ENT_FLAG, ENT_OTHER
};
enum GameObjectType
{
    GO_UNKNOWN = 0, GO_BUILDING = 1, GO_ARMEDBUILDING = 2, GO_SOLDIER = 4,
    GO_VEHICLE = 8, GO_WALKER = 16, GO_FLYER = 32, GO_HOVER = 64, GO_MINE = 128
};
enum ReserveType
{
    RT_REPAIR = 0, RT_BOARD = 1, RT_ATTACK = 2, RT_FORMATION = 3, RT_HeavyWeapons = 4
};

class Controllable;
class ExplosionClass;
class FLEffectObject;
class FlagItem;
class NetObjState;
class NetPktGroup;
class Ordnance;
class PlayableEntitySounds;
class TargetInfo;
extern PblVector3 gVectorZero;

class GameObject : public EntityGeometry, public Damageable, public PblHandled
{
public:

    GameObject(GameObjectClass* pClass, const EntityDesc& desc, CollisionObjectType eType,
               const RedLodData* pLodData);

    virtual ~GameObject();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    // FUNCTION: SWBF2 0x00444310
    virtual const GameObject* GetGameObject() const { return this; }
    // SYNTHETIC: SWBF2 0x00446e09
    // [thunk]:GameObject::GetGameObject`adjustor{320}'
    // SYNTHETIC: SWBF2 0x00446e1c
    // [thunk]:GameObject::GetGameObject`adjustor{12}'
    // FUNCTION: SWBF2 0x00444300
    virtual GameObject* GetGameObject() { return this; }
    virtual void Init();
    virtual void ClientKill();
    // FUNCTION: SWBF2 0x00444230
    virtual bool IsUnbuilt() const { return false; }
    virtual float GetMaxSpeed() const = 0;
    virtual float GetMaxStrafe() const = 0;

    // FUNCTION: SWBF2 0x00444240
    virtual TargetInfo* GetTargetInfo() { return 0; }
    virtual const Controllable* GetControllable() const;
    virtual Controllable* GetControllable();

    // FUNCTION: SWBF2 0x00444270
    virtual const PlayableEntitySounds* GetPlayableEntitySounds() const { return 0; }

    // FUNCTION: SWBF2 0x00444280
    virtual PlayableEntitySounds* GetPlayableEntitySounds() { return 0; }

    // FUNCTION: SWBF2 0x00444290
    virtual float CalcAimSpreadVal() const { return 0.0f; }

    // FUNCTION: SWBF2 0x004442a0
    virtual bool ShouldShowReticule() const { return mIsAlive; }
    virtual void ApplyRadiusDamage(const PblVector3& vPos, float fInnerRadius,
                                   float fOuterRadius, const DamageDesc& damage);
    virtual void ApplyRadiusPush(const PblVector3& vPos, float fInnerRadius,
                                 float fOuterRadius, float fPush, bool bArg);

    // FUNCTION: SWBF2 0x004442c0
    virtual void ApplyPush(const PblVector3& vPush, float fArg, bool bArg) {}
    virtual void SetTeam(int iTeam);
    virtual bool IsVisibleToMe(const GameObject* pObject) const;
    // FUNCTION: SWBF2 0x004442d0
    virtual bool IsSuitableEnemy() const { return true; }
    // FUNCTION: SWBF2 0x004442e0
    virtual int GetReservableSlots(ReserveType eType) const { return 1; }

    // FUNCTION: SWBF2 0x004442f0
    virtual const PblVector3& GetAttachOffset() const { return gVectorZero; }

    // FUNCTION: SWBF2 0x00492310
    virtual void ActivatePhysics() {}

    // FUNCTION: SWBF2 0x00492320
    virtual void DeactivatePhysics() {}

    // FUNCTION: SWBF2 0x004992f0
    virtual GameObjectType GetGameObjectType() { return GO_UNKNOWN; }

    // FUNCTION: SWBF2 0x00444320
    virtual void AcquiredTarget() {}

    // FUNCTION: SWBF2 0x00444330
    virtual void Hiding() {}

    // FUNCTION: SWBF2 0x00444340
    virtual void ApproachingTarget() {}

    // FUNCTION: SWBF2 0x00444350
    virtual void Flee() {}

    // FUNCTION: SWBF2 0x00444360
    virtual void PreparingForDamage() {}
    virtual void HeardEnemy();

    // FUNCTION: SWBF2 0x00444380
    virtual void Charge() {}
    virtual void ReserveForPlayer(float fTime);
    // FUNCTION: SWBF2 0x004443a0
    virtual bool IsReservedForPlayer() const { return false; }
    virtual bool Deflect(const Ordnance* pOrdnance, const PblVector3& vPos,
                         const PblVector3& vDir);

    // FUNCTION: SWBF2 0x004caad0
    virtual EntityType GetEntityType() const { return ENT_OTHER; }

    // FUNCTION: SWBF2 0x004443c0
    virtual ExplosionClass* GetExplosionClass() { return 0; }
    virtual bool AddToTeamManager() const;

    // FUNCTION: SWBF2 0x004443d0
    virtual float GetEjectTimer() const { return 0.0f; }

    // FUNCTION: SWBF2 0x004443e0
    virtual void SetEjectTimer(float fTime) {}

    // FUNCTION: SWBF2 0x004443f0
    virtual void ResetTimeSinceLastHack() {}
    virtual void WriteCreate(NetPktGroup& group);
    virtual void ReadCreate(NetPktGroup& group);
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual void ResetObject();
    virtual void SkipReadObject(NetObjState* pState);
    virtual GameObject* AddToClient();
    virtual void AddSends(void (*pfnSend)(GameObject*, int), int iArg);
    virtual const PblMatrix& GetSmoothedMatrix() const;
    virtual void WriteLateState(NetPktGroup& group);
    virtual void ReadLateState(NetPktGroup& group);

    virtual void Kill();
    virtual void Respawn();
    virtual float GetDamageMultiplier(int iBodyId) const;

    GameObjectClass* GetGameObjectClass() const { return (GameObjectClass*)mEntityClass; }
    const PblList<Controllable>& GetControllableList() const;

    // FUNCTION: SWBF2 0x00499310
    int GetNetUniqueId() const { return mNetUniqueId; }

    void SetNetUniqueId(int iNetUniqueId);

    // FUNCTION: SWBF2 0x00429260
    int GetTeam() const { return mTeam; }

    void SetOwningTeam(int iTeam) { mOwningTeam = iTeam; }

    // FUNCTION: SWBF2 0x0044c540
    void SetPerceivedTeam(int iTeam) { mPerceivedTeam = iTeam; }

    // FUNCTION: SWBF2 0x0047a680
    int GetPerceivedTeam() const { return mPerceivedTeam; }

    // FUNCTION: SWBF2 0x004ddff0
    float GetCreatedTime() const { return mCreatedTime; }

    static unsigned int GetRtti() { return rttiHashGameObject; }
    int GetAffiliationToMe(const GameObject* pObject) const;
    bool IsMyFriend(const GameObject* pObject) const;
    bool IsMyEnemy(const GameObject* pObject) const;
    void AddPilotSkillCount(int iCount);
    void PlayDeathSound();
    void PlayHurtSound(bool bByCharacter);
    void PlayDamageRegionSound(unsigned int uRegionId);
    FlagItem* GetCarriedFlagItem() const;
    float CalcRadiusEffect(float fDist, float fInnerRadius, float fOuterRadius) const;
    void AwardVehicleKillPoints(int iVehicleType, bool bEmpty);
    void TriggerDisabledEffect();

    bool FoliageCollision();

    PblList<Controllable> mControllableList;
    int mNetUniqueId;
    void* mPrevFoliageObj;
    GameSoundControllable mGameObjectAttachedSound;
    PblHandle<FLEffectObject> mDisabledEffectHandle;
    float mCreatedTime;
    int mTeam : 4;
    int mPerceivedTeam : 4;
    int mOwningTeam : 4;
    unsigned int mHealthTypeForLockOn : 4;
    bool mIsTargetable : 1;

    static PblHash rttiHashGameObject;
    static char rttiNameGameObject[];
};
 

 
 
 
 
 
 
 
 
