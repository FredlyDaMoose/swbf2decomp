#pragma once

#include "Weapon.h"

class Controllable;
class DamageOwner;
class ExplosionClass;
class GameObject;
class MountedTurretClass;
class PblVector3;
class UnitController;
class OrdnanceClass;
class OrdnanceMissileClass;
class Weapon;
class WeaponCannonClass;
class WeaponGrenadeClass;
class WeaponMeleeThrowClass;

namespace CombatUtil
{

    inline void Init() {}
    void SetDenseEnvironment(bool bDense);
    bool IsDenseEnvironment();
    WeaponClass::Label GetWeaponLabel(const Weapon* pWpn);
    float sGetWpnMaxRange(const Controllable* pMe, const WeaponClass* pClass);
    float sGetCombatRange(const Controllable* pMe, const WeaponClass* pClass);
    float GetMaxCombatRange(const Controllable* pMe);
    bool HasNoWeapons(const Controllable* pMe);
    void sCalcExplosionDamage(const ExplosionClass* pExplosion, const GameObject* pTarget,
                              float* pShieldDamage, float* pHealthDamage);
    float sGetMaxAmmo(const Weapon* pWpn, const WeaponClass* pClass);
    float sCalcImpactTime_Missile(const DamageOwner& owner, const OrdnanceMissileClass* pClass,
                                  const GameObject* pTarget);
    float sCalcImpactTime(const DamageOwner& owner, const OrdnanceClass* pClass,
                          const GameObject* pTarget);
    void sGetDamageValues(const DamageOwner& owner, const OrdnanceClass* pClass,
                          const GameObject* pTarget, const float fStrength,
                          float& fHealthDamage, float& fShieldDamage);
    void GetAverageDamageValues(const DamageOwner& owner, const OrdnanceClass* pClass,
                                const GameObject* pTarget, float& fHealthDamage,
                                float& fShieldDamage);
    float sCalcDamagePerSecond_Cannon(const DamageOwner& owner, const WeaponCannonClass* pClass,
                                      const GameObject* pTarget);
    float sCalcDamagePerSecond_Grenade(const DamageOwner& owner, const WeaponGrenadeClass* pClass,
                                       const GameObject* pTarget);
    float sCalcDamagePerSecond_MeleeThrow(const DamageOwner& owner,
                                          const WeaponMeleeThrowClass* pClass,
                                          const GameObject* pTarget);
    float sCalcDamagePerSecond(Controllable* pMe, const WeaponClass* pClass,
                               const GameObject* pTarget);
    float CalcDamagePerSecond(Controllable* pMe, const WeaponClass* pClass,
                              const GameObject* pTarget);
    bool sCauseDamage(Controllable* pMe, const Weapon* pWpn, const WeaponClass* pClass,
                      const GameObject* pTarget, float* pDamagePerSecond);
    void sUseGrenadeAgainstCrowds(const Controllable* pMe, bool bSoldier, const GameObject* pTarget,
                                  const WeaponClass* pClass, bool* pbUse);
    bool CanGetFireReservation(const WeaponClass* pClass, const GameObject* pMe,
                               const GameObject* pTarget);
    bool sIsWeaponUnlocked(const Controllable* pMe, const WeaponClass* pClass);
    bool sSelectBestWeaponType(Controllable* pMe, const GameObject* pTarget, bool bFire,
                               WeaponClass::Label* pLabel, bool* pbInRange,
                               bool* pbInOptimalRange, bool bIgnoreRange);

    bool IsSniperWpn(const Weapon* pWpn);

    struct CombatDesc;
    bool HasCriticalHitLocation(const GameObject* pObj);
    bool EngageInCombat(const UnitController* pCtrl, const GameObject* pTarget, bool bIgnoreRanges);
    float GetWeaponOptimalRange(const Controllable* pMe);
    float GetWeaponMaxRange(const Controllable* pMe);
    float GetWeaponMinRange(const Controllable* pMe);
    bool sIsTargetLinedUpPos(const Controllable* pMe, const PblVector3& vPos);
    bool IsTargetLinedUpObj(const Controllable* pMe, const GameObject* pTarget);
    bool TestWpnConstraintsObj(Controllable* pMe, const GameObject* pTarget);
    bool CanTurretFaceDisturbance(const Controllable* pMe, const MountedTurretClass* pClass,
                                  const GameObject* pObj, const PblVector3& vPos);
    bool CanFaceDisturbance(const GameObject* pObj, const PblVector3& vPos);
}

namespace CombatUtil { float GetCombatRange(const Controllable* pMe); }
namespace CombatUtil { bool TestWpnConstraintsPos(Controllable* pMe, const PblVector3& vAim, const PblVector3& vPos); }
