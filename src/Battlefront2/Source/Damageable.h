#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHandle.h"
#include "GameSound.h"

class Character;
class EntityClass;
class FLEffectObject;
class GameObject;
class PblVector3;
class WeaponClass;

enum HealthType
{
    HEALTH_TYPE_PERSON = 0,
    HEALTH_TYPE_ANIMAL = 1,
    HEALTH_TYPE_DROID = 2,
    HEALTH_TYPE_VEHICLE = 3,
    HEALTH_TYPE_BUILDING = 4,
    HEALTH_TYPE_BUILDING_DEAD = 5,
    HEALTH_TYPE_BUILDING_UNBUILT = 6,
    HEALTH_TYPE_MINE = 7,
    NUM_HEALTH_TYPES = 8
};

class DamageOwner
{
public:

    DamageOwner() {}

    DamageOwner(Character* pCharacter, GameObject* pGameObject, int iTeam,
                EntityClass* pClass, WeaponClass* pWeapon, float fDamageMultiplier);

    DamageOwner& operator=(const DamageOwner& other);

    // FUNCTION: SWBF2 0x00438ab0
    ~DamageOwner() {}

    Character* mCharacter;
    PblHandle<GameObject> mGameObject;
    int mTeam;
    EntityClass* mClass;
    WeaponClass* mWeapon;
    float mDamageMultiplier;
};
 
 
 
 
 
 
 

class DamageDesc
{
public:

    DamageDesc() {}

    DamageDesc(DamageOwner owner, float fBaseValue);

    DamageOwner mDamageOwner;
    float mBaseValue;
    float mHealthScale[8];
    float mShieldScale;
};
 
 
 
 

class Damageable
{
public:

    // FUNCTION: SWBF2 0x00438010
    float GetMaxHealth() const { return mMaxHealth; }

    Damageable();
    virtual ~Damageable();
    virtual void Kill();
    virtual void Respawn();
    virtual GameObject* GetGameObject();
    virtual const GameObject* GetGameObject() const;
    virtual bool ApplyDamage(const DamageDesc& damage, const PblVector3& vDir, const PblVector3& vPos, int iBodyPart, unsigned int uFlags);
    virtual void InstantDeath(const DamageOwner& owner, bool);

    virtual float GetDamageMultiplier(int) const = 0;
    virtual void HealthChangeCallback(bool, float);
    virtual void ShieldChangeCallback(float);

    // FUNCTION: SWBF2 0x00429250
    bool IsAlive() const { return mIsAlive; }

    bool IsDisabled() const { return mDisableTime > 0.0f; }

    void SetAIDamageThreshold(float fThreshold) { mAIDamageThreshold = fThreshold; }

    void InstantVanish()
    {
        mIsAlive = false;
        mVanishing = true;
    }

    float GetCurShield() const { return mCurShield; }
    float GetMaxShield() const { return mMaxShield; }
    void SetMaxShield(float fShield) { mMaxShield = fShield; }
    void SetAddShield(float fShield) { mAddShield = fShield; }
    bool IsShielded() const { return mIsShielded; }
    void SetIsShielded(bool bShielded) { mIsShielded = bShielded; }

    static void StaticInit();
    static void SetTeamDamageScale(float fScale);

    static float sSameTeamDamageScale;

    void SetCurHealth(float fHealth, bool bNotify, DamageOwner* pOwner);

    bool ApplyDamage(const DamageDesc& damage, unsigned int uFlags);
    void SetCurShield(float fShield, bool bNotify, DamageOwner* pOwner);
    void UpdateStatus(float fDeltaTime);
    void SetDamageRegionTime();
    void ApplyNetClientDamage(const DamageDesc& damage, unsigned int uFlags, float fMultiplier,
                              bool bIsShot, const PblVector3& vPos);
    void ApplyDamageCommon(const DamageDesc& damage, unsigned int uFlags, float fMultiplier,
                           bool bIsShot, const PblVector3& vPos);
    void SpreadDamageEvent(const DamageDesc& damage, bool bIsShot, const PblVector3& vPos,
                           float fHealthLost, float fShieldLost);

    float mCurHealth;
    float mMaxHealth;
    float mAddHealth;
    float mCurShield;
    float mMaxShield;
    float mAddShield;
    float mDisableTime;
    float mAIDamageThreshold;
    DamageOwner mDamageOwner;
    PblHandle<FLEffectObject> mDamageEffect[10];
    GameSoundControllable mDamageEffectSound[10];
    float mDamageRegionTime;
    unsigned int mHealthType : 3;
    unsigned int mIsAlive : 1;
    unsigned int mVanishing : 1;
    unsigned int mIsShielded : 1;
};
 
 
 
 
 
 
 
 
 
