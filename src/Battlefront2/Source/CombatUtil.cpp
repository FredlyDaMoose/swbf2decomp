#include "CombatUtil.h"

#include <math.h>

#include "AIDifficulty.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Damageable.h"
#include "Entity.h"
#include "EntityCarrier.h"
#include "EntityBuildingArmed.h"
#include "EntityRemoteTerminal.h"
#include "EntitySoldier.h"
#include "Explosion.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "MedalsMgr.h"
#include "MountedTurret.h"
#include "Ordnance.h"
#include "OrdnanceMissile.h"
#include "SoldierAnimationBank.h"
#include "UnitAttackPatterns.h"
#include "UnitController.h"
#include "WeaponCannon.h"
#include "WeaponGrenade.h"
#include "WeaponMeleeThrow.h"
#include "WeaponRemote.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblVector.h"

#include "EventInfo.h"

class AmmoCounter
{
public:
    int GetMaxNumRounds() const;
    int GetNumRounds() const;
};

namespace ReserveManager
{

    bool IsReservedBy(const GameObject* pObj, const GameObject* pReserver, ReserveType eType);

    int GetOpenReservableSlots(const GameObject* pObj, int iTeam, ReserveType eType);
    void Reserve(const GameObject* pObj, const GameObject* pReserver, ReserveType eType);
}

// GLOBAL: SWBF2 0x01efcf3e
bool sDenseEnvironment;

// GLOBAL: SWBF2 0x01e30735
bool sHasSetDenseFlag;

namespace
{

    const PblHash kRttiHashEntitySoldier("EntitySoldier");
    const PblHash kRttiHashEntityDroideka("EntityDroideka");
    const PblHash kRttiHashEntityDroid("EntityDroid");
    const PblHash kRttiHashOrdnanceMissileClass("OrdnanceMissileClass");
    const PblHash kRttiHashOrdnanceBeamClass("OrdnanceBeamClass");
    const PblHash kRttiHashWeaponCannonClass("WeaponCannonClass");
    const PblHash kRttiHashWeaponGrenadeClass("WeaponGrenadeClass");
    const PblHash kRttiHashWeaponMeleeThrowClass("WeaponMeleeThrowClass");
    const PblHash kRttiHashWeaponMeleeClass("WeaponMeleeClass");
    const PblHash kRttiHashWeaponMelee("WeaponMelee");
    const PblHash kRttiHashEntityRemoteTerminal("EntityRemoteTerminal");
    const PblHash kRttiHashWeaponRemoteClass("WeaponRemoteClass");
    const PblHash kRttiHashEntityFlyerClass("EntityFlyerClass");
    const PblHash kRttiHashEntityFlyer("EntityFlyer");
    const PblHash kRttiHashEntityWalker("EntityWalker");
    const PblHash kRttiHashEntityHover("EntityHover");

    const PblHash kRttiHashCommandFlyer("CommandFlyer");
    const PblHash kRttiHashEntityBuilding("EntityBuilding");
    const PblHash kRttiHashEntityBuildingArmed("EntityBuildingArmed");
    const PblHash kRttiHashEntityTrap("EntityTrap");
    const PblHash kRttiHashMountedTurret("MountedTurret");
    const PblHash kRttiHashOrdnanceGrenadeClass("OrdnanceGrenadeClass");
    const PblHash kRttiHashOrdnanceStickyClass("OrdnanceStickyClass");
    const PblHash kRttiHashOrdnanceHaywireClass("OrdnanceHaywireClass");
    const PblHash kRttiHashOrdnanceShellClass("OrdnanceShellClass");

    const WeaponClass* WeaponGetClass(const Weapon* pWpn)
    {
        return pWpn->mClass;
    }

    const AmmoCounter* WeaponGetAmmoCounter(const Weapon* pWpn)
    {
        return pWpn->m_pAmmoCounter;
    }

    bool WeaponIsMelee(const Weapon* pWpn)
    {
        return pWpn->IsMelee();
    }

    bool WeaponIsMeleeThrow(const Weapon* pWpn)
    {
        return pWpn->IsMeleeThrow();
    }

    int WeaponGetState(const Weapon* pWpn)
    {
        return pWpn->mState;
    }

    const GameObject* HandleObject(const PblHandle<GameObject>& handle)
    {
        PblHandle<GameObject>& h = const_cast<PblHandle<GameObject>&>(handle);
        if (h.mObject && h.mObject->mHandleId != h.mSavedHandleId)
        {
            h.mObject = 0;
            h.mSavedHandleId = 0;
        }
        return h.mObject;
    }

    const PblVector3& GetPosition(const GameObject* pObj)
    {
        return static_cast<const CollisionObject*>(pObj)->GetPosition();
    }

    int GetHealthType(const GameObject* pObj)
    {
        return pObj->mHealthType;
    }

    int GetTeam(const GameObject* pObj)
    {
        return pObj->mTeam;
    }

    EntityClass* GetEntityClass(GameObject* pObj)
    {
        return pObj->GetEntityClass();
    }

    float GetMaxSpeed(const GameObject* pObj)
    {
        return pObj->GetMaxSpeed();
    }

    Controllable* GetControllable(GameObject* pObj)
    {
        return pObj->GetControllable();
    }

    const Controllable* GetControllable(const GameObject* pObj)
    {
        return pObj->GetControllable();
    }

    const PblVector3& GetVelocity(const GameObject* pObj)
    {
        return pObj->GetVelocity();
    }

    PblVector3 GetTargetPoint(const GameObject* pObj, const PblVector3& vFrom,
                              const PblVector3& vDir, int iBodyId)
    {
        return pObj->GetTargetPoint(vFrom, vDir, iBodyId);
    }

    const Damageable* GetDamageable(const GameObject* pObj)
    {
        return static_cast<const Damageable*>(pObj);
    }

    bool IsTargetable(const GameObject* pObj)
    {
        return pObj->mIsTargetable;
    }

    int GetFlyerState(const GameObject* pObj)
    {
        return static_cast<const EntityFlyer*>(pObj)->mState;
    }

    bool EntityIsRtti(const GameObject* pObj, unsigned int uRtti)
    {
        return pObj->GetEntity()->IsRtti(uRtti);
    }

    float LengthSq(const PblVector3& v)
    {
        return v.x * v.x + v.y * v.y + v.z * v.z;
    }

    float Distance(const PblVector3& v0, const PblVector3& v1)
    {
        float dx = v1.x - v0.x;
        float dy = v1.y - v0.y;
        float dz = v1.z - v0.z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }
}

// FUNCTION: SWBF2 0x0046dc00
void CombatUtil::SetDenseEnvironment(bool bDense)
{
    sHasSetDenseFlag = true;
    sDenseEnvironment = bDense;
}

// FUNCTION: SWBF2 0x0046dc10
bool CombatUtil::IsDenseEnvironment()
{
    return sDenseEnvironment;
}

// FUNCTION: SWBF2 0x0046dc20
WeaponClass::Label CombatUtil::GetWeaponLabel(const Weapon* pWpn)
{
    if (pWpn)
    {
        if (WeaponGetClass(pWpn)->mIsOffhand)
        {
            return WeaponClass::THROWN;
        }
        if (WeaponIsMeleeThrow(pWpn))
        {
            return WeaponClass::MELEE;
        }
        switch (SoldierAnimationBank::GetWeaponRoot(
            (SoldierAnimationBank::WEAPON)WeaponGetClass(pWpn)->mSoldierAnimationWeapon))
        {
        case SoldierAnimationBank::RIFLE:
            return WeaponClass::LASER;
        case SoldierAnimationBank::INVALID_WEAPON:
        case SoldierAnimationBank::BAZOOKA:
            return WeaponClass::LAUNCHER;
        case SoldierAnimationBank::TOOL:
            return WeaponClass::NONE;
        case SoldierAnimationBank::MELEE:
            return WeaponClass::MELEE;
        }
    }
    return WeaponClass::BLASTER;
}

// FUNCTION: SWBF2 0x0046dca0
float CombatUtil::sGetWpnMaxRange(const Controllable* pMe, const WeaponClass* pClass)
{
    float fRange = pClass->mMaxRange;
    if (pClass->mMaxRangeDefault)
    {
        if (!pMe->GetEntity()->IsRtti(kRttiHashEntitySoldier) &&
            !pMe->GetEntity()->IsRtti(kRttiHashEntityDroideka) &&
            !pMe->GetEntity()->IsRtti(kRttiHashEntityDroid))
        {
            fRange *= 2.0f;
        }
    }
    float fMaxRange = pMe->GetWpnDistFactor() * fRange;
    if (fMaxRange > pClass->mExtremeRange)
    {
        return (pClass->mExtremeRange > fRange) ? pClass->mExtremeRange : fRange;
    }
    return fMaxRange;
}

// FUNCTION: SWBF2 0x0046dd60
float CombatUtil::sGetCombatRange(const Controllable* pMe, const WeaponClass* pClass)
{
    float fMoveRange = GetMaxSpeed(pMe->GetGameObject()) * 5.0f;
    return sGetWpnMaxRange(pMe, pClass) + fMoveRange;
}

// FUNCTION: SWBF2 0x0046dda0
float CombatUtil::GetMaxCombatRange(const Controllable* pMe)
{
    float fMaxRange = 0.0f;
    for (int i = 0; i < pMe->GetWeaponCount(); ++i)
    {
        const Weapon* pWpn = pMe->GetWeaponPtr(i);
        if (pWpn)
        {
            const WeaponClass* pClass = WeaponGetClass(pWpn);
            do
            {
                float fRange = sGetCombatRange(pMe, pClass);
                fMaxRange = (fMaxRange > fRange) ? fMaxRange : fRange;
                pClass = pClass->mNextCharge;
            } while (pClass != pClass->mNextCharge);
        }
    }
    return fMaxRange;
}

// FUNCTION: SWBF2 0x0046de20
bool CombatUtil::HasNoWeapons(const Controllable* pMe)
{
    return pMe->GetCurWpn() == 0;
}

// FUNCTION: SWBF2 0x0046de30
void CombatUtil::sCalcExplosionDamage(const ExplosionClass* pExplosion, const GameObject* pTarget,
                                      float* pShieldDamage, float* pHealthDamage)
{
    float fDamage = pTarget->CalcRadiusEffect(
                        2.0f, pExplosion->mDamageRadius[0], pExplosion->mDamageRadius[1]) *
                    pExplosion->mDamage.mBaseValue;
    *pShieldDamage = pExplosion->mDamage.mShieldScale * fDamage;
    *pHealthDamage = pExplosion->mDamage.mHealthScale[GetHealthType(pTarget)] * fDamage;
}

// FUNCTION: SWBF2 0x0046de90
float CombatUtil::sGetMaxAmmo(const Weapon* pWpn, const WeaponClass* pClass)
{
    return (float)WeaponGetAmmoCounter(pWpn)->GetMaxNumRounds();
}

// FUNCTION: SWBF2 0x0046deb0
float CombatUtil::sCalcImpactTime_Missile(const DamageOwner& owner,
                                          const OrdnanceMissileClass* pClass,
                                          const GameObject* pTarget)
{
    const GameObject* pShooter = HandleObject(owner.mGameObject);
    OrdnanceMissileClass* pMissile = const_cast<OrdnanceMissileClass*>(pClass);
    float fDistance = Distance(GetPosition(pTarget), GetPosition(pShooter));
    float fAccel = pClass->mAcceleration;
    float fStartSpeed;
    if (fAccel < 9000.0f)
    {
        fStartSpeed = pClass->mMinSpeed;
    }
    else
    {
        fStartSpeed = pMissile->GetVelocity();
    }
    float fMaxSpeed = pMissile->GetVelocity();

    float fTime =
        ((float)sqrt((double)(fAccel * 2.0f * fDistance + fStartSpeed * fStartSpeed)) - fStartSpeed) /
        fAccel;
    if (fTime * fAccel + fStartSpeed < fMaxSpeed)
    {
        return fTime;
    }

    float fAccelTime = (fMaxSpeed - fStartSpeed) / fAccel;
    float fAccelDistance = fAccel * 0.5f * fAccelTime * fAccelTime + fAccelTime * fStartSpeed;
    return (fDistance - fAccelDistance) / fMaxSpeed + fAccelTime;
}

// FUNCTION: SWBF2 0x0046e030
float CombatUtil::sCalcImpactTime(const DamageOwner& owner, const OrdnanceClass* pClass,
                                  const GameObject* pTarget)
{
    OrdnanceClass* pOrdnance = const_cast<OrdnanceClass*>(pClass);
    if (pOrdnance->IsRtti(kRttiHashOrdnanceMissileClass))
    {
        return sCalcImpactTime_Missile(owner, (const OrdnanceMissileClass*)pClass, pTarget);
    }
    if (pOrdnance->IsRtti(kRttiHashOrdnanceBeamClass))
    {
        return 0.0f;
    }
    if (pOrdnance->GetVelocity() <= 0.0f)
    {
        return 0.0f;
    }
    const GameObject* pShooter = HandleObject(owner.mGameObject);
    float fDistance = Distance(GetPosition(pTarget), GetPosition(pShooter));
    return fDistance / pOrdnance->GetVelocity();
}

// FUNCTION: SWBF2 0x0046e140
void CombatUtil::sGetDamageValues(const DamageOwner& owner, const OrdnanceClass* pClass,
                                  const GameObject* pTarget, const float fStrength,
                                  float& fHealthDamage, float& fShieldDamage)
{
    float fImpactTime = sCalcImpactTime(owner, pClass, pTarget);
    DamageDesc damage = pClass->GetDamage(owner, fStrength, 0.0f, fImpactTime);
    fShieldDamage = damage.mShieldScale * damage.mBaseValue;
    fHealthDamage = damage.mHealthScale[GetHealthType(pTarget)] * damage.mBaseValue;

    if (pClass->mExplosionImpact)
    {
        float fShield = 0.0f;
        float fHealth = 0.0f;
        sCalcExplosionDamage(pClass->mExplosionImpact, pTarget, &fShield, &fHealth);
        fShieldDamage += fShield;
        fHealthDamage += fHealth;
    }
    if (pClass->mExplosionExpire)
    {
        float fShield = 0.0f;
        float fHealth = 0.0f;
        sCalcExplosionDamage(pClass->mExplosionExpire, pTarget, &fShield, &fHealth);
        fShieldDamage += fShield;
        fHealthDamage += fHealth;
    }
}

// FUNCTION: SWBF2 0x0046e240
void CombatUtil::GetAverageDamageValues(const DamageOwner& owner, const OrdnanceClass* pClass,
                                        const GameObject* pTarget, float& fHealthDamage,
                                        float& fShieldDamage)
{
    if (!pClass)
    {
        return;
    }

    float fChargeBot = 0.15f;
    float fChargeTop = 1.0f;
    if (HandleObject(owner.mGameObject))
    {
        const UnitController* pCtrl =
            UnitControllerPtr(GetControllable(const_cast<GameObject*>(HandleObject(owner.mGameObject))));
        if (pCtrl)
        {
            fChargeBot = AIDifficulty::GetChargeWeaponBot(pCtrl);
            fChargeTop = AIDifficulty::GetChargeWeaponTop(pCtrl);
        }
    }

    const OrdnanceClass* pBot = pClass;
    while (fChargeBot > pBot->mNextCharge->mStrengthLimit && pBot->mNextCharge != pBot)
    {
        pBot = pBot->mNextCharge;
    }
    sGetDamageValues(owner, pBot, pTarget, fChargeBot, fHealthDamage, fShieldDamage);

    const OrdnanceClass* pTop = pClass;
    while (fChargeTop > pTop->mNextCharge->mStrengthLimit && pTop->mNextCharge != pTop)
    {
        pTop = pTop->mNextCharge;
    }
    if (pTop != pBot)
    {
        float fHealth;
        float fShield;

        sGetDamageValues(owner, pBot, pTarget, fChargeTop, fHealth, fShield);
        fHealthDamage = (fHealthDamage + fHealth) * 0.5f;
        fShieldDamage = (fShieldDamage + fShield) * 0.5f;
    }
}

// FUNCTION: SWBF2 0x0046e390
float CombatUtil::sCalcDamagePerSecond_Cannon(const DamageOwner& owner,
                                              const WeaponCannonClass* pClass,
                                              const GameObject* pTarget)
{
    float fHealthDamage = 0.0f;
    float fShieldDamage = 0.0f;
    GetAverageDamageValues(owner, pClass->mOrdnanceClass, pTarget, fHealthDamage, fShieldDamage);

    float fShotDelay = (pClass->mShotDelayMin + pClass->mShotDelayMax) * 0.5f;
    float fReloadTime = pClass->mReloadTime;
    int iRoundsPerClip = pClass->mAmmoCounterClass.mRoundsPerClip;
    if (fReloadTime > 0.0f && (float)iRoundsPerClip < 1000000.0f)
    {
        return (float)iRoundsPerClip * fHealthDamage /
               ((float)(iRoundsPerClip / pClass->mShotsPerSalvo) * fShotDelay + fReloadTime);
    }
    return (float)pClass->mShotsPerSalvo * fHealthDamage / fShotDelay;
}

// FUNCTION: SWBF2 0x0046e440
float CombatUtil::sCalcDamagePerSecond_Grenade(const DamageOwner& owner,
                                               const WeaponGrenadeClass* pClass,
                                               const GameObject* pTarget)
{
    float fHealthDamage = 0.0f;
    float fShieldDamage = 0.0f;
    GetAverageDamageValues(owner, pClass->mOrdnanceClass, pTarget, fHealthDamage, fShieldDamage);

    float fCycleTime = pClass->mShotDelay * pClass->mRoundsPerShot + pClass->mReloadTime;
    if (fCycleTime <= 0.0f)
    {
        fCycleTime = 1.0f;
    }
    return (float)pClass->mAmmoCounterClass.mRoundsPerClip * fHealthDamage / fCycleTime;
}

// FUNCTION: SWBF2 0x0046e4b0
float CombatUtil::sCalcDamagePerSecond_MeleeThrow(const DamageOwner& owner,
                                                  const WeaponMeleeThrowClass* pClass,
                                                  const GameObject* pTarget)
{
    float fHealthDamage = 0.0f;
    float fShieldDamage = 0.0f;
    GetAverageDamageValues(owner, pClass->mOrdnanceClass, pTarget, fHealthDamage, fShieldDamage);
    return fHealthDamage / (sCalcImpactTime(owner, pClass->mOrdnanceClass, pTarget) * 2.0f);
}

// FUNCTION: SWBF2 0x0046e510
float CombatUtil::sCalcDamagePerSecond(Controllable* pMe, const WeaponClass* pClass,
                                       const GameObject* pTarget)
{
    if (!pClass || !pTarget)
    {
        return 0.0f;
    }

    GameObject* pObj = pMe->GetGameObject();
    DamageOwner owner(pMe->mCharacter, pMe->GetGameObject(), GetTeam(pMe->GetGameObject()),
                      GetEntityClass(pObj), const_cast<WeaponClass*>(pClass),
                      pMe->GetOffenseMultiplier());

    WeaponClass* pWeapon = const_cast<WeaponClass*>(pClass);
    if (pWeapon->IsRtti(kRttiHashWeaponCannonClass))
    {
        return sCalcDamagePerSecond_Cannon(owner, (const WeaponCannonClass*)pClass, pTarget);
    }
    if (pWeapon->IsRtti(kRttiHashWeaponGrenadeClass))
    {
        return sCalcDamagePerSecond_Grenade(owner, (const WeaponGrenadeClass*)pClass, pTarget);
    }
    if (pWeapon->IsRtti(kRttiHashWeaponMeleeThrowClass))
    {
        return sCalcDamagePerSecond_MeleeThrow(owner, (const WeaponMeleeThrowClass*)pClass, pTarget);
    }
    if (pWeapon->IsRtti(kRttiHashWeaponMeleeClass))
    {
        return 100.0f;
    }
    if (pWeapon->IsRtti(kRttiHashWeaponRemoteClass))
    {
        return 1000.0f;
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x0046e660
float CombatUtil::CalcDamagePerSecond(Controllable* pMe, const WeaponClass* pClass,
                                      const GameObject* pTarget)
{
    if (!pClass || !pTarget)
    {
        return 0.0f;
    }

    GameObject* pObj = pMe->GetGameObject();
    DamageOwner owner(pMe->mCharacter, pMe->GetGameObject(), GetTeam(pMe->GetGameObject()),
                      GetEntityClass(pObj), const_cast<WeaponClass*>(pClass),
                      pMe->GetOffenseMultiplier());

    float fChargeBot = 0.15f;
    float fChargeTop = 1.0f;
    const GameObject* pOwnerObj = HandleObject(owner.mGameObject);
    if (pOwnerObj)
    {
        const UnitController* pCtrl =
            UnitControllerPtr(GetControllable(const_cast<GameObject*>(pOwnerObj)));
        if (pCtrl)
        {
            fChargeBot = AIDifficulty::GetChargeWeaponBot(pCtrl);
            fChargeTop = AIDifficulty::GetChargeWeaponTop(pCtrl);
        }
    }

    const WeaponClass* pBot = pClass;
    while (fChargeBot > pBot->mNextCharge->mStrengthLimit && pBot->mNextCharge != pBot)
    {
        pBot = pBot->mNextCharge;
    }
    float fDamagePerSecond = sCalcDamagePerSecond(pMe, pBot, pTarget);

    const WeaponClass* pTop = pBot;
    while (fChargeTop > pTop->mNextCharge->mStrengthLimit && pTop->mNextCharge != pTop)
    {
        pTop = pTop->mNextCharge;
    }
    if (pTop == pBot)
    {
        return fDamagePerSecond;
    }
    return (sCalcDamagePerSecond(pMe, pTop, pTarget) + fDamagePerSecond) * 0.5f;
}

// FUNCTION: SWBF2 0x0046e7c0
bool CombatUtil::sCauseDamage(Controllable* pMe, const Weapon* pWpn, const WeaponClass* pClass,
                              const GameObject* pTarget, float* pDamagePerSecond)
{
    *pDamagePerSecond = 0.0f;
    WeaponClass* pWeaponClass = const_cast<WeaponClass*>(pClass);
    OrdnanceClass* pOrdnance = pClass->mOrdnanceClass;
    if (!pOrdnance)
    {
        if (pWeaponClass->IsRtti(kRttiHashWeaponRemoteClass))
        {
            EntityClass* pDispense = ((const WeaponRemoteClass*)pClass)->mDispenseClass;
            if (pDispense && pDispense->IsRtti(kRttiHashEntityFlyerClass))
            {
                return false;
            }
            *pDamagePerSecond = 1000.0f;
            return true;
        }
        if (pWeaponClass->IsRtti(kRttiHashWeaponMeleeClass) &&
            (EntityIsRtti(pTarget, kRttiHashEntitySoldier) ||
             EntityIsRtti(pTarget, kRttiHashEntityDroideka) ||
             EntityIsRtti(pTarget, kRttiHashEntityWalker) ||
             EntityIsRtti(pTarget, kRttiHashEntityHover) ||
             EntityIsRtti(pTarget, kRttiHashEntityBuildingArmed)))
        {
            *pDamagePerSecond = 1000.0f;
            return true;
        }
    }

    if (pWpn->CheckOverheat())
    {
        return false;
    }
    if (EntityIsRtti(pTarget, kRttiHashEntityTrap))
    {
        return false;
    }
    if (!(pClass->mAITargetTypes & (1 << GetHealthType(pTarget))) &&
        !(EntityIsRtti(pTarget, kRttiHashEntityDroideka) && (pClass->mAITargetTypes & 8)))
    {
        if (!pMe->GetEntity()->IsRtti(kRttiHashEntitySoldier))
        {
            return false;
        }
        if (!EntityIsRtti(pTarget, kRttiHashEntitySoldier))
        {
            return false;
        }
        const Controllable* pHim = GetControllable(pTarget);
        if (!pHim || !pHim->mCtrl || !pHim->mCtrl->IsInCrowd())
        {
            return false;
        }
    }
    if (!pOrdnance)
    {
        return false;
    }

    float fDamagePerSecond = CalcDamagePerSecond(pMe, pClass, pTarget);
    if (fDamagePerSecond * 10.0f > GetDamageable(pTarget)->mMaxHealth * 0.5f)
    {
        *pDamagePerSecond = fDamagePerSecond;
        return true;
    }

    if (pMe->GetEntity()->IsRtti(kRttiHashEntityHover) ||
        pMe->GetEntity()->IsRtti(kRttiHashEntityWalker) ||
        pMe->GetEntity()->IsRtti(kRttiHashEntityBuildingArmed) ||
        pMe->GetEntity()->IsRtti(kRttiHashMountedTurret))
    {
        if (fDamagePerSecond > 0.0f && GetDamageable(pTarget)->mCurHealth < 1.0e16f)
        {
            *pDamagePerSecond = fDamagePerSecond;
            return true;
        }
    }

    bool bSlowFlyer = false;
    if (EntityIsRtti(pTarget, kRttiHashEntityFlyer))
    {
        int iState = GetFlyerState(pTarget);
        if (iState == 0 || iState == 3 || iState == 1 || LengthSq(GetVelocity(pTarget)) < 256.0f)
        {
            bSlowFlyer = true;
        }
    }
    if (!bSlowFlyer && !EntityIsRtti(pTarget, kRttiHashEntityHover) &&
        !EntityIsRtti(pTarget, kRttiHashEntityWalker) &&
        !(IsTargetable(pTarget) && EntityIsRtti(pTarget, kRttiHashEntityBuilding)))
    {
        return false;
    }

    if (fDamagePerSecond > 0.0f)
    {
        if (pOrdnance->IsRtti(kRttiHashOrdnanceGrenadeClass) ||
            pOrdnance->IsRtti(kRttiHashOrdnanceStickyClass))
        {
            if (LengthSq(GetVelocity(pTarget)) < 144.0f)
            {
                *pDamagePerSecond = fDamagePerSecond;
                return true;
            }
        }
        else if (pOrdnance->IsRtti(kRttiHashOrdnanceBeamClass) ||
                 pOrdnance->IsRtti(kRttiHashOrdnanceHaywireClass) ||
                 pOrdnance->IsRtti(kRttiHashOrdnanceMissileClass) ||
                 pOrdnance->IsRtti(kRttiHashOrdnanceShellClass))
        {
            *pDamagePerSecond = fDamagePerSecond;
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0046ec30
void CombatUtil::sUseGrenadeAgainstCrowds(const Controllable* pMe, bool bSoldier,
                                          const GameObject* pTarget, const WeaponClass* pClass,
                                          bool* pbUse)
{
    if (!const_cast<WeaponClass*>(pClass)->IsRtti(kRttiHashWeaponGrenadeClass) || !bSoldier)
    {
        return;
    }

    const Entity* pEntity = reinterpret_cast<const Entity*>(pTarget);
    if (pEntity->IsRtti(kRttiHashEntitySoldier))
    {
        *pbUse = false;
    }
    const Controllable* pHim = GetControllable(pTarget);
    if ((pHim && pHim->mCtrl && pHim->mCtrl->IsInCrowd()) || pEntity->IsRtti(kRttiHashEntityDroideka))
    {
        Controller* pCtrl = pMe->mCtrl;
        int iRoll = (((UnitController*)pCtrl)->GetRandomSeed() -
                     (int)(GameLoop::GetMissionTime() * -10.0f)) %
                    100;
        if (iRoll <= (int)(AIDifficulty::GetCrowdGrenadeThrowPercentage(UnitControllerPtr(pCtrl)) * 100.0f))
        {
            *pbUse = true;
        }
    }
}

// FUNCTION: SWBF2 0x0046ed10
bool CombatUtil::CanGetFireReservation(const WeaponClass* pClass, const GameObject* pMe,
                                       const GameObject* pTarget)
{
    OrdnanceClass* pOrdnance = pClass->mOrdnanceClass;
    if (pOrdnance)
    {
        bool bGrenade = pOrdnance->IsRtti(kRttiHashOrdnanceGrenadeClass);
        bool bMissile = pOrdnance->IsRtti(kRttiHashOrdnanceMissileClass);
        if (bGrenade || bMissile)
        {
            if (!ReserveManager::IsReservedBy(pTarget, pMe, RT_HeavyWeapons) &&
                ReserveManager::GetOpenReservableSlots(pTarget, GetTeam(pMe), RT_HeavyWeapons) <= 0)
            {
                return false;
            }
            ReserveManager::Reserve(pTarget, pMe, RT_HeavyWeapons);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0046ed90
bool CombatUtil::sIsWeaponUnlocked(const Controllable* pMe, const WeaponClass* pClass)
{
    if (pMe && pMe->mCharacter)
    {
        const MedalsMgr* pMedals = pMe->mCharacter->mMedalsMgr;
        if (pMedals && pClass->mMedalsTypeToUnlock != -1)
        {
            bool bUnlocked = pMedals->IsAwardAvailable(pClass->mMedalsTypeToUnlock);
            if (bUnlocked && pClass->mMedalsTypeToLock != -1 &&
                pMedals->IsAwardAvailable(pClass->mMedalsTypeToLock))
            {
                bUnlocked = false;
            }
            return bUnlocked;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0046ee00
bool CombatUtil::sSelectBestWeaponType(Controllable* pMe, const GameObject* pTarget, bool bFire,
                                       WeaponClass::Label* pLabel, bool* pbInRange,
                                       bool* pbInOptimalRange, bool bIgnoreRange)
{
    *pLabel = WeaponClass::NONE;
    *pbInRange = false;
    *pbInOptimalRange = false;

    float fBestDamage = -1.0f;
    bool bBestMultiShot = false;
    bool bBestInRange = false;
    int iBest = -1;

    PblVector3 vPoint = GetTargetPoint(pTarget, pMe->mEyePoint, pMe->mEyeDir, -1);
    float fDistSq = (vPoint.y - pMe->mEyePoint.y) * (vPoint.y - pMe->mEyePoint.y) +
                    (vPoint.x - pMe->mEyePoint.x) * (vPoint.x - pMe->mEyePoint.x) +
                    (vPoint.z - pMe->mEyePoint.z) * (vPoint.z - pMe->mEyePoint.z);
    bool bSoldier = pMe->GetEntity()->IsRtti(kRttiHashEntitySoldier);

    for (int i = 0; i < pMe->GetWeaponCount(); ++i)
    {
        const Weapon* pWpn = pMe->GetWeaponPtr(i);
        if (!pWpn || WeaponGetAmmoCounter(pWpn)->GetNumRounds() <= 0)
        {
            continue;
        }

        const WeaponClass* pClass = WeaponGetClass(pWpn);
        float fDamage = 0.0f;
        if (bFire)
        {
            if (!CanGetFireReservation(pClass, pMe->GetGameObject(), pTarget))
            {
                continue;
            }
            if (!WeaponIsMelee(pWpn) && (WeaponGetState(pWpn) == 4 || WeaponGetState(pWpn) == 5))
            {
                continue;
            }
        }
        if (!sIsWeaponUnlocked(pMe, pClass))
        {
            continue;
        }
        if (!sCauseDamage(pMe, pWpn, pClass, pTarget, &fDamage))
        {
            continue;
        }

        float fMaxAmmo = sGetMaxAmmo(pWpn, pClass);
        float fMaxRange = sGetWpnMaxRange(pMe, pClass);
        float fMinRange = pClass->mMinRange;
        bool bMultiShot = fMaxAmmo > 2.0f;
        bool bBetter = fDamage > fBestDamage;
        if (fDamage == fBestDamage)
        {
            bBetter = AIUtil::GetRandomInt(0, 2) != 0;
        }

        if (fDistSq > fMinRange * fMinRange && fMaxRange * fMaxRange > fDistSq)
        {
            bool bUse = false;
            if (!bBestInRange)
            {
                bUse = true;
            }
            else if (bBestMultiShot)
            {
                if (bMultiShot && bBetter)
                {
                    bUse = true;
                }
            }
            else if (bMultiShot || bBetter)
            {
                bUse = true;
            }

            sUseGrenadeAgainstCrowds(pMe, bSoldier, pTarget, pClass, &bUse);
            if (!bUse)
            {
                continue;
            }

            fBestDamage = fDamage;
            bBestMultiShot = bMultiShot;
            bBestInRange = true;
            iBest = i;
            *pbInRange = true;
            float fOptimalMin = (pClass->mOptimalRange + fMinRange) * 0.5f;
            float fOptimalMax = (pClass->mOptimalRange + fMaxRange) * 0.5f;
            *pbInOptimalRange = fDistSq > fOptimalMin * fOptimalMin && fOptimalMax * fOptimalMax > fDistSq;
        }
        else
        {
            if (bBestInRange || WeaponIsMeleeThrow(pWpn))
            {
                continue;
            }
            float fCombatRange = sGetCombatRange(pMe, pClass);
            if (!bIgnoreRange && !(fCombatRange * fCombatRange > fDistSq))
            {
                continue;
            }
            if (bBestMultiShot ? (bMultiShot && bBetter) : (bMultiShot || bBetter))
            {
                fBestDamage = fDamage;
                iBest = i;
                bBestMultiShot = bMultiShot;
            }
        }
    }

    if (iBest < 0)
    {
        return false;
    }

    *pLabel = GetWeaponLabel(pMe->GetWeaponPtr(iBest));
    if ((!pMe->GetCurWpn() || pMe->GetWeaponIndex(pMe->GetWpnChannel()) != iBest) && bFire)
    {
        reinterpret_cast<AILowLevel*>(UnitControllerPtr(pMe->mCtrl)->mLowLevel)->SetNextWeapon(iBest);
    }
    return true;
}

struct CombatUtil::CombatDesc
{
    CombatDesc(AttackPatterns::State* pPattern, float* pCoverPercent, bool* pSuitableWpn,
               bool bSwitchWpn, bool bIgnoreRanges)
        : mPattern(pPattern), mCoverPercent(pCoverPercent), mSuitableWpn(pSuitableWpn),
          mSwitchWpn(bSwitchWpn), mIgnoreRanges(bIgnoreRanges)
    {
    }

    AttackPatterns::State* mPattern;
    float* mCoverPercent;
    bool* mSuitableWpn;
    bool mSwitchWpn;
    bool mIgnoreRanges;
};
 
 
 

namespace
{

    const AttackPatterns::State kNoPattern = (AttackPatterns::State)0;
}

namespace CombatUtil
{
    void SoldierVsTauntaun(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void TauntaunVsAll(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void FlyerVsFlyer(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void HoverVsFlyer(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void InfantryVsVehicle(Controllable*, GameObjectClass::SoldierType, const GameObject*, CombatDesc*);
    bool FlyerWeapon(Controllable*, const GameObject*, bool, bool*, bool*, bool);
    bool HoverWeapon(Controllable*, const GameObject*, bool, bool*, bool*, bool);
}

// FUNCTION: SWBF2 0x0046f5c0
void CombatUtil::SoldierVsTauntaun(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    EntitySoldier* pSoldier = (EntitySoldier*)pMe->GetEntity();
    CombatUtil::InfantryVsVehicle(pMe, (GameObjectClass::SoldierType)pSoldier->mClass->GetSoldierType(), pTarget, pDesc);
}

// FUNCTION: SWBF2 0x0046f860
void CombatUtil::TauntaunVsAll(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    WeaponClass::Label eLabel;
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = CombatUtil::sSelectBestWeaponType(pMe, pTarget, pDesc->mSwitchWpn,
                                                             &eLabel, &bInRange, &bInOptimalRange,
                                                             pDesc->mIgnoreRanges);
}

// FUNCTION: SWBF2 0x0046f950
void CombatUtil::FlyerVsFlyer(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = CombatUtil::FlyerWeapon(pMe, pTarget, pDesc->mSwitchWpn,
                                                   &bInRange, &bInOptimalRange, pDesc->mIgnoreRanges);
}

// FUNCTION: SWBF2 0x0046fab0
void CombatUtil::HoverVsFlyer(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = CombatUtil::HoverWeapon(pMe, pTarget, pDesc->mSwitchWpn,
                                                   &bInRange, &bInOptimalRange, pDesc->mIgnoreRanges);
}

namespace CombatUtil
{
    void sRegularInfantryAttackPattern(Controllable* pMe, const GameObject* pTarget, bool bInRange,
                                       bool bInOptimalRange, WeaponClass::Label eLabel,
                                       CombatDesc* pDesc);
    void InfantryVsFoot(Controllable* pMe, GameObjectClass::SoldierType eType,
                        const GameObject* pTarget, CombatDesc* pDesc);
    void SoldierVsSoldier(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void SoldierVsDroideka(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void SoldierVsDroid(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void InfantryVsVehicle(Controllable* pMe, GameObjectClass::SoldierType eType,
                           const GameObject* pTarget, CombatDesc* pDesc);
    void SoldierVsWalker(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void SoldierVsHover(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void SoldierVsBuildings(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void InfantryVsFlyer(Controllable* pMe, GameObjectClass::SoldierType eType,
                         const GameObject* pTarget, CombatDesc* pDesc);
    void SoldierVsFlyer(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void WalkerVsFlyer(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    bool FlyerWeapon(Controllable* pMe, const GameObject* pTarget, bool bSwitchWpn, bool* pbInRange,
                     bool* pbInOptimalRange, bool bIgnoreRanges);
    void FlyerVsIgnore(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void FlyerVsGroundTarget(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    bool VehicleWeapon(Controllable* pMe, const GameObject* pTarget, bool bSwitchWpn, bool* pbInRange,
                       bool* pbInOptimalRange, bool bIgnoreRanges);
    void WalkerVsInfantry(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void WalkerVsVehicle(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void DroidekaVsInfantry(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    bool HoverWeapon(Controllable* pMe, const GameObject* pTarget, bool bSwitchWpn, bool* pbInRange,
                     bool* pbInOptimalRange, bool bIgnoreRanges);
    void HoverVsInfantry(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void HoverVsVehicle(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    void TurretVsAll(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc);
    bool SelectCombatResponse(Controllable* pMe, const GameObject* pTarget,
                              AttackPatterns::State* pPattern, float* pCoverPercent,
                              bool* pSuitableWpn, bool bSwitchWpn, bool bIgnoreRanges);
}

// FUNCTION: SWBF2 0x0046f190
void CombatUtil::sRegularInfantryAttackPattern(Controllable* pMe, const GameObject* pTarget,
                                               bool bInRange, bool bInOptimalRange,
                                               WeaponClass::Label eLabel, CombatDesc* pDesc)
{
    if (eLabel == WeaponClass::THROWN)
    {
        if (bInRange)
        {
            *pDesc->mPattern = AttackPatterns::STATE_STATIONARY;
        }
        return;
    }

    static const AttackPatterns::State kPatterns[4] = {
        AttackPatterns::STATE_STATIONARY, AttackPatterns::STATE_SIMPLE,
        AttackPatterns::STATE_SERPENTINE, AttackPatterns::STATE_FLANK
    };
    static const int kWeights[3][4] = {
        { 3, 1, 1, 1 },
        { 2, 2, 1, 1 },
        { 1, 1, 5, 1 }
    };

    *pDesc->mCoverPercent = 0.5f;
    int iDifficulty = AIDifficulty::GetProfileDifficulty() - 1;
    float fOpenRadius = pMe->mCtrl->mOpenRadius;
    int iTotal = 0;
    if (fOpenRadius > -1.0f)
    {
        iTotal = kWeights[iDifficulty][0] + kWeights[iDifficulty][1];
    }
    if (fOpenRadius > 10.0f)
    {
        iTotal += kWeights[iDifficulty][2];
    }
    if (fOpenRadius > 5.0f)
    {
        iTotal += kWeights[iDifficulty][3];
    }

    int iRoll = ((UnitController*)pMe->mCtrl)->GetRandomSeed() % iTotal;
    int i;
    for (i = 0; i < 4; ++i)
    {
        iRoll -= kWeights[iDifficulty][i];
        if (iRoll < 0)
        {
            break;
        }
    }
    if (i == 4)
    {
        *pDesc->mPattern = AttackPatterns::STATE_SIMPLE;
        return;
    }
    *pDesc->mPattern = kPatterns[i];
    if (!bInOptimalRange && *pDesc->mPattern == AttackPatterns::STATE_STATIONARY)
    {
        *pDesc->mPattern = AttackPatterns::STATE_SIMPLE;
    }
}

// FUNCTION: SWBF2 0x0046f2b0
void CombatUtil::InfantryVsFoot(Controllable* pMe, GameObjectClass::SoldierType eType,
                                const GameObject* pTarget, CombatDesc* pDesc)
{
    WeaponClass::Label eLabel = WeaponClass::NONE;
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = sSelectBestWeaponType(pMe, pTarget, pDesc->mSwitchWpn, &eLabel, &bInRange,
                                                 &bInOptimalRange, pDesc->mIgnoreRanges);
    if (eType == GameObjectClass::SOLDIER_SCOUT)
    {
        if (!*pDesc->mSuitableWpn)
        {
            *pDesc->mPattern = kNoPattern;
            return;
        }
        if (eLabel == WeaponClass::LASER)
        {
            *pDesc->mPattern = bInRange ? AttackPatterns::STATE_STATIONARY : kNoPattern;
            return;
        }
    }
    else
    {
        if (!*pDesc->mSuitableWpn)
        {
            *pDesc->mPattern = kNoPattern;
            return;
        }
        if (eLabel == WeaponClass::MELEE)
        {
            *pDesc->mCoverPercent = 0.0f;
            *pDesc->mPattern = AttackPatterns::STATE_MELEE;
            return;
        }
    }
    sRegularInfantryAttackPattern(pMe, pTarget, bInRange, bInOptimalRange, eLabel, pDesc);
}

// FUNCTION: SWBF2 0x0046f370
void CombatUtil::SoldierVsSoldier(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    EntitySoldier* pSoldier = (EntitySoldier*)pMe->GetEntity();
    InfantryVsFoot(pMe, (GameObjectClass::SoldierType)pSoldier->mClass->GetSoldierType(), pTarget, pDesc);
}

// FUNCTION: SWBF2 0x0046f3a0
void CombatUtil::SoldierVsDroideka(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    EntitySoldier* pSoldier = (EntitySoldier*)pMe->GetEntity();
    InfantryVsFoot(pMe, (GameObjectClass::SoldierType)pSoldier->mClass->GetSoldierType(), pTarget, pDesc);
}

// FUNCTION: SWBF2 0x0046f3d0
void CombatUtil::SoldierVsDroid(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    EntitySoldier* pSoldier = (EntitySoldier*)pMe->GetEntity();
    InfantryVsFoot(pMe, (GameObjectClass::SoldierType)pSoldier->mClass->GetSoldierType(), pTarget, pDesc);
}

// FUNCTION: SWBF2 0x0046f400
bool CombatUtil::HasCriticalHitLocation(const GameObject* pObj)
{
    const GameObjectClass* pClass = pObj->GetGameObjectClass();
    for (int i = 0; i < pClass->mNumHitLocations; ++i)
    {
        if (pClass->mHitLocations[i].mDamageMultiplier > 1.0f)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0046f440
void CombatUtil::InfantryVsVehicle(Controllable* pMe, GameObjectClass::SoldierType eType,
                                   const GameObject* pTarget, CombatDesc* pDesc)
{
    WeaponClass::Label eLabel;
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mCoverPercent = 0.75f;
    *pDesc->mSuitableWpn = sSelectBestWeaponType(pMe, pTarget, pDesc->mSwitchWpn, &eLabel, &bInRange,
                                                 &bInOptimalRange, pDesc->mIgnoreRanges);
    if (!*pDesc->mSuitableWpn)
    {
        *pDesc->mPattern = kNoPattern;
        return;
    }

    switch (eLabel)
    {
    case WeaponClass::BLASTER:
    case WeaponClass::LASER:
    case WeaponClass::THROWN:
    case WeaponClass::LAUNCHER:
        if (HasCriticalHitLocation(pTarget))
        {
            *pDesc->mPattern = AttackPatterns::STATE_CRITICALHIT;
            return;
        }
        switch (((UnitController*)pMe->mCtrl)->GetRandomSeed() % 2)
        {
        case 0:
            *pDesc->mPattern = AttackPatterns::STATE_FLANK;
            break;
        case 1:
            if (bInRange)
            {
                *pDesc->mPattern = AttackPatterns::STATE_STATIONARY;
            }
            break;
        }
        break;
    case WeaponClass::MELEE:
        if (EntityIsRtti(pTarget, kRttiHashEntityBuildingArmed) ||
            EntityIsRtti(pTarget, kRttiHashEntityWalker) ||
            EntityIsRtti(pTarget, kRttiHashEntityHover))
        {
            *pDesc->mCoverPercent = 0.0f;
            *pDesc->mPattern = AttackPatterns::STATE_MELEE;
            return;
        }
        *pDesc->mPattern = kNoPattern;
        break;
    case WeaponClass::NONE:
        *pDesc->mPattern = kNoPattern;
        break;
    }
}

// FUNCTION: SWBF2 0x0046f590
void CombatUtil::SoldierVsWalker(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    EntitySoldier* pSoldier = (EntitySoldier*)pMe->GetEntity();
    InfantryVsVehicle(pMe, (GameObjectClass::SoldierType)pSoldier->mClass->GetSoldierType(), pTarget, pDesc);
}

// FUNCTION: SWBF2 0x0046f5f0
void CombatUtil::SoldierVsHover(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    EntitySoldier* pSoldier = (EntitySoldier*)pMe->GetEntity();
    InfantryVsVehicle(pMe, (GameObjectClass::SoldierType)pSoldier->mClass->GetSoldierType(), pTarget, pDesc);
}

// FUNCTION: SWBF2 0x0046f620
void CombatUtil::SoldierVsBuildings(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    EntitySoldier* pSoldier = (EntitySoldier*)pMe->GetEntity();
    InfantryVsVehicle(pMe, (GameObjectClass::SoldierType)pSoldier->mClass->GetSoldierType(), pTarget, pDesc);
}

// FUNCTION: SWBF2 0x0046f650
void CombatUtil::InfantryVsFlyer(Controllable* pMe, GameObjectClass::SoldierType eType,
                                 const GameObject* pTarget, CombatDesc* pDesc)
{
    WeaponClass::Label eLabel;
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = sSelectBestWeaponType(pMe, pTarget, pDesc->mSwitchWpn, &eLabel, &bInRange,
                                                 &bInOptimalRange, pDesc->mIgnoreRanges);
}

// FUNCTION: SWBF2 0x0046f690
void CombatUtil::SoldierVsFlyer(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    EntitySoldier* pSoldier = (EntitySoldier*)pMe->GetEntity();
    GameObjectClass::SoldierType eType = (GameObjectClass::SoldierType)pSoldier->mClass->GetSoldierType();
    if (GetFlyerState(pTarget) == 0 &&
        const_cast<GameObject*>(pTarget)->IsRtti(kRttiHashCommandFlyer))
    {
        InfantryVsVehicle(pMe, eType, pTarget, pDesc);
    }
    else
    {
        InfantryVsFlyer(pMe, eType, pTarget, pDesc);
    }
}

// FUNCTION: SWBF2 0x0046f6f0
bool CombatUtil::VehicleWeapon(Controllable* pMe, const GameObject* pTarget, bool bSwitchWpn,
                               bool* pbInRange, bool* pbInOptimalRange, bool bIgnoreRanges)
{
    WeaponClass::Label eLabel;
    return sSelectBestWeaponType(pMe, pTarget, bSwitchWpn, &eLabel, pbInRange, pbInOptimalRange,
                                 bIgnoreRanges);
}

// FUNCTION: SWBF2 0x0046f720
void CombatUtil::WalkerVsInfantry(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = VehicleWeapon(pMe, pTarget, pDesc->mSwitchWpn, &bInRange, &bInOptimalRange,
                                         pDesc->mIgnoreRanges);
    if (GetHealthType(pMe->GetGameObject()) == HEALTH_TYPE_ANIMAL && *pDesc->mSuitableWpn)
    {
        *pDesc->mPattern = AttackPatterns::STATE_MELEE;
    }
}

// FUNCTION: SWBF2 0x0046f780
void CombatUtil::WalkerVsVehicle(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = VehicleWeapon(pMe, pTarget, pDesc->mSwitchWpn, &bInRange, &bInOptimalRange,
                                         pDesc->mIgnoreRanges);
    if (!*pDesc->mSuitableWpn)
    {
        *pDesc->mPattern = kNoPattern;
    }
    else if (bInRange)
    {
        *pDesc->mPattern = AttackPatterns::STATE_STATIONARY;
    }
}

// FUNCTION: SWBF2 0x0046f7e0
void CombatUtil::WalkerVsFlyer(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = VehicleWeapon(pMe, pTarget, pDesc->mSwitchWpn, &bInRange, &bInOptimalRange,
                                         pDesc->mIgnoreRanges);
}

// FUNCTION: SWBF2 0x0046f810
void CombatUtil::DroidekaVsInfantry(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = VehicleWeapon(pMe, pTarget, pDesc->mSwitchWpn, &bInRange, &bInOptimalRange,
                                         pDesc->mIgnoreRanges);
    if (*pDesc->mSuitableWpn && bInRange)
    {
        *pDesc->mPattern = AttackPatterns::STATE_STATIONARY;
    }
}

// FUNCTION: SWBF2 0x0046f8a0
bool CombatUtil::FlyerWeapon(Controllable* pMe, const GameObject* pTarget, bool bSwitchWpn,
                             bool* pbInRange, bool* pbInOptimalRange, bool bIgnoreRanges)
{
    WeaponClass::Label eLabel;
    return sSelectBestWeaponType(pMe, pTarget, bSwitchWpn, &eLabel, pbInRange, pbInOptimalRange,
                                 bIgnoreRanges);
}

// FUNCTION: SWBF2 0x0046f8d0
void CombatUtil::FlyerVsIgnore(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = FlyerWeapon(pMe, pTarget, pDesc->mSwitchWpn, &bInRange, &bInOptimalRange,
                                       pDesc->mIgnoreRanges);
}

// FUNCTION: SWBF2 0x0046f900
void CombatUtil::FlyerVsGroundTarget(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = FlyerWeapon(pMe, pTarget, pDesc->mSwitchWpn, &bInRange, &bInOptimalRange,
                                       pDesc->mIgnoreRanges);
    if (*pDesc->mSuitableWpn && AIUtil::CountHumansInVehicle(pMe) == 0)
    {
        *pDesc->mPattern = AttackPatterns::STATE_FLY_STRAFE;
    }
}

// FUNCTION: SWBF2 0x0046f980
bool CombatUtil::HoverWeapon(Controllable* pMe, const GameObject* pTarget, bool bSwitchWpn,
                             bool* pbInRange, bool* pbInOptimalRange, bool bIgnoreRanges)
{
    WeaponClass::Label eLabel;
    return sSelectBestWeaponType(pMe, pTarget, bSwitchWpn, &eLabel, pbInRange, pbInOptimalRange,
                                 bIgnoreRanges);
}

// FUNCTION: SWBF2 0x0046f9b0
void CombatUtil::HoverVsInfantry(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = HoverWeapon(pMe, pTarget, pDesc->mSwitchWpn, &bInRange, &bInOptimalRange,
                                       pDesc->mIgnoreRanges);
}

// FUNCTION: SWBF2 0x0046f9e0
void CombatUtil::HoverVsVehicle(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = HoverWeapon(pMe, pTarget, pDesc->mSwitchWpn, &bInRange, &bInOptimalRange,
                                       pDesc->mIgnoreRanges);
    if (bInRange)
    {
        *pDesc->mCoverPercent = 0.5f;
    }
    if (!*pDesc->mSuitableWpn)
    {
        return;
    }

    if (IsDenseEnvironment())
    {
        if (bInRange)
        {
            *pDesc->mPattern = AttackPatterns::STATE_STATIONARY;
        }
        return;
    }
    if (pMe->GetGameObject()->GetMaxSpeed() >= 14.0f && 5.0f > pMe->GetGameObject()->GetMaxStrafe())
    {
        *pDesc->mPattern = AttackPatterns::STATE_GLANCE;
        return;
    }
    *pDesc->mPattern = bInRange ? AttackPatterns::STATE_STATIONARY : AttackPatterns::STATE_SIMPLE;
}

// FUNCTION: SWBF2 0x0046fae0
void CombatUtil::TurretVsAll(Controllable* pMe, const GameObject* pTarget, CombatDesc* pDesc)
{
    WeaponClass::Label eLabel;
    bool bInRange;
    bool bInOptimalRange;
    *pDesc->mSuitableWpn = sSelectBestWeaponType(pMe, pTarget, pDesc->mSwitchWpn, &eLabel, &bInRange,
                                                 &bInOptimalRange, pDesc->mIgnoreRanges);
    if (*pDesc->mSuitableWpn && bInRange)
    {
        *pDesc->mPattern = AttackPatterns::STATE_STATIONARY;
    }
}

// FUNCTION: SWBF2 0x0046fb30
bool CombatUtil::SelectCombatResponse(Controllable* pMe, const GameObject* pTarget,
                                      AttackPatterns::State* pPattern, float* pCoverPercent,
                                      bool* pSuitableWpn, bool bSwitchWpn, bool bIgnoreRanges)
{
    *pPattern = kNoPattern;
    *pCoverPercent = 0.0f;
    *pSuitableWpn = false;
    CombatDesc desc(pPattern, pCoverPercent, pSuitableWpn, bSwitchWpn, bIgnoreRanges);

    AIUtil::EntityType eMe = AIUtil::GetEntityType(pMe->GetEntity());
    AIUtil::EntityType eHim = AIUtil::GetEntityType(pTarget);
    switch (eMe)
    {
    case AIUtil::ENTITY_SOLDIER:
        switch (eHim)
        {
        case AIUtil::ENTITY_SOLDIER: SoldierVsSoldier(pMe, pTarget, &desc); break;
        case AIUtil::ENTITY_DROID: SoldierVsDroid(pMe, pTarget, &desc); break;
        case AIUtil::ENTITY_WALKER: SoldierVsWalker(pMe, pTarget, &desc); break;
        case AIUtil::ENTITY_DROIDEKA: SoldierVsDroideka(pMe, pTarget, &desc); break;
        case AIUtil::ENTITY_TAUNTAUN: SoldierVsTauntaun(pMe, pTarget, &desc); break;
        case AIUtil::ENTITY_FLYER: SoldierVsFlyer(pMe, pTarget, &desc); break;
        case AIUtil::ENTITY_HOVER: SoldierVsHover(pMe, pTarget, &desc); break;
        case AIUtil::ENTITY_BUILDING_ARMED:
        case AIUtil::ENTITY_BUILDING: SoldierVsBuildings(pMe, pTarget, &desc); break;
        }
        break;
    case AIUtil::ENTITY_DROID:
        switch (eHim)
        {
        case AIUtil::ENTITY_SOLDIER:
        case AIUtil::ENTITY_DROID:
        case AIUtil::ENTITY_DROIDEKA:
            InfantryVsFoot(pMe, GameObjectClass::SOLDIER_TROOPER, pTarget, &desc);
            break;
        case AIUtil::ENTITY_WALKER:
        case AIUtil::ENTITY_TAUNTAUN:
        case AIUtil::ENTITY_HOVER:
        case AIUtil::ENTITY_BUILDING_ARMED:
        case AIUtil::ENTITY_BUILDING:
            InfantryVsVehicle(pMe, GameObjectClass::SOLDIER_TROOPER, pTarget, &desc);
            break;
        case AIUtil::ENTITY_FLYER:
            InfantryVsFlyer(pMe, GameObjectClass::SOLDIER_TROOPER, pTarget, &desc);
            break;
        }
        break;
    case AIUtil::ENTITY_WALKER:
        switch (eHim)
        {
        case AIUtil::ENTITY_SOLDIER:
        case AIUtil::ENTITY_DROID:
        case AIUtil::ENTITY_DROIDEKA:
            WalkerVsInfantry(pMe, pTarget, &desc);
            break;
        case AIUtil::ENTITY_WALKER:
        case AIUtil::ENTITY_TAUNTAUN:
        case AIUtil::ENTITY_HOVER:
        case AIUtil::ENTITY_BUILDING_ARMED:
        case AIUtil::ENTITY_BUILDING:
            WalkerVsVehicle(pMe, pTarget, &desc);
            break;
        case AIUtil::ENTITY_FLYER:
            WalkerVsFlyer(pMe, pTarget, &desc);
            break;
        }
        break;
    case AIUtil::ENTITY_DROIDEKA:
        switch (eHim)
        {
        case AIUtil::ENTITY_SOLDIER:
        case AIUtil::ENTITY_DROID:
        case AIUtil::ENTITY_DROIDEKA:
            DroidekaVsInfantry(pMe, pTarget, &desc);
            break;
        case AIUtil::ENTITY_WALKER:
        case AIUtil::ENTITY_TAUNTAUN:
        case AIUtil::ENTITY_HOVER:
        case AIUtil::ENTITY_BUILDING_ARMED:
        case AIUtil::ENTITY_BUILDING:
            WalkerVsVehicle(pMe, pTarget, &desc);
            break;
        case AIUtil::ENTITY_FLYER:
            WalkerVsFlyer(pMe, pTarget, &desc);
            break;
        }
        break;
    case AIUtil::ENTITY_TAUNTAUN:
        TauntaunVsAll(pMe, pTarget, &desc);
        break;
    case AIUtil::ENTITY_FLYER:
        switch (eHim)
        {
        case AIUtil::ENTITY_SOLDIER:
        case AIUtil::ENTITY_DROID:
        case AIUtil::ENTITY_DROIDEKA:
        case AIUtil::ENTITY_TAUNTAUN:
        case AIUtil::ENTITY_HOVER:
            FlyerVsIgnore(pMe, pTarget, &desc);
            break;
        case AIUtil::ENTITY_WALKER:
        case AIUtil::ENTITY_BUILDING_ARMED:
        case AIUtil::ENTITY_BUILDING:
        case AIUtil::ENTITY_REMOTE_TERMINAL:
            FlyerVsGroundTarget(pMe, pTarget, &desc);
            break;
        case AIUtil::ENTITY_FLYER:
            FlyerVsFlyer(pMe, pTarget, &desc);
            break;
        }
        break;
    case AIUtil::ENTITY_HOVER:
        switch (eHim)
        {
        case AIUtil::ENTITY_SOLDIER:
        case AIUtil::ENTITY_DROID:
        case AIUtil::ENTITY_DROIDEKA:
            HoverVsInfantry(pMe, pTarget, &desc);
            break;
        case AIUtil::ENTITY_WALKER:
        case AIUtil::ENTITY_TAUNTAUN:
        case AIUtil::ENTITY_HOVER:
        case AIUtil::ENTITY_BUILDING_ARMED:
        case AIUtil::ENTITY_BUILDING:
            HoverVsVehicle(pMe, pTarget, &desc);
            break;
        case AIUtil::ENTITY_FLYER:
            HoverVsFlyer(pMe, pTarget, &desc);
            break;
        }
        break;
    case AIUtil::ENTITY_TURRET:
        TurretVsAll(pMe, pTarget, &desc);
        break;
    }
    return *pPattern != kNoPattern;
}

// FUNCTION: SWBF2 0x00470000
bool CombatUtil::EngageInCombat(const UnitController* pCtrl, const GameObject* pTarget, bool bIgnoreRanges)
{
    Controllable* pMe = pCtrl->mOwner;
    if (!pMe->GetWeaponPtr(0))
    {
        return false;
    }
    AttackPatterns::State ePattern;
    float fCoverPercent;
    bool bSuitableWpn;
    return SelectCombatResponse(pMe, pTarget, &ePattern, &fCoverPercent, &bSuitableWpn, false,
                                bIgnoreRanges);
}

// FUNCTION: SWBF2 0x00470050
float CombatUtil::GetWeaponOptimalRange(const Controllable* pMe)
{
    const Weapon* pWpn = pMe->GetCurWpn();
    if (!pWpn)
    {
        return 0.0f;
    }
    return WeaponGetClass(pWpn)->mOptimalRange;
}

// FUNCTION: SWBF2 0x00470070
float CombatUtil::GetWeaponMaxRange(const Controllable* pMe)
{
    const Weapon* pWpn = pMe->GetCurWpn();
    if (!pWpn)
    {
        return 0.0f;
    }
    return sGetWpnMaxRange(pMe, WeaponGetClass(pWpn));
}

// FUNCTION: SWBF2 0x00470090
float CombatUtil::GetWeaponMinRange(const Controllable* pMe)
{
    const Weapon* pWpn = pMe->GetCurWpn();
    if (!pWpn)
    {
        return 0.0f;
    }
    return WeaponGetClass(pWpn)->mMinRange;
}

namespace CombatUtil {

// FUNCTION: SWBF2 0x004700b0
float GetCombatRange(const Controllable* pMe)
{
    return CombatUtil::sGetCombatRange(pMe, WeaponGetClass(pMe->GetCurWpn()));
}
}

// FUNCTION: SWBF2 0x004700d0
bool CombatUtil::IsSniperWpn(const Weapon* pWpn)
{
    if (pWpn)
    {
        const WeaponClass* pClass = WeaponGetClass(pWpn);
        if (pClass->mTriggerSingle && pClass->mZoomMax >= 6.0f)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00470100
bool CombatUtil::sIsTargetLinedUpPos(const Controllable* pMe, const PblVector3& vPos)
{
    const Weapon* pWpn = pMe->GetCurWpn();
    const Aimer* pAimer = pMe->GetCurAimer();
    if (!pAimer || !pWpn)
    {
        return false;
    }

    PblVector3 vDelta;
    vDelta.x = vPos.x - pMe->mEyePoint.x;
    vDelta.y = vPos.y - pMe->mEyePoint.y;
    vDelta.z = vPos.z - pMe->mEyePoint.z;
    PblVector3 vDir = AIUtil::GetSafeNormal(vDelta);
    float fDot = pMe->mEyeDir.x * vDir.x + pMe->mEyeDir.y * vDir.y + pMe->mEyeDir.z * vDir.z;

    static const float sCosNarrow = PblMath::Cos(PblMath::fPi / 8.0f);
    static const float sCosWide = PblMath::Cos(PblMath::fPi / 4.0f);
    float fCos = sCosNarrow;
    if (pWpn->IsRtti(kRttiHashWeaponMelee))
    {
        fCos = sCosWide;
    }
    if (!(fDot > fCos))
    {
        return false;
    }

    vDir = AIUtil::GetSafeNormal(pAimer->GetDirection());
    if (!(pMe->mEyeDir.x * vDir.x + pMe->mEyeDir.y * vDir.y + pMe->mEyeDir.z * vDir.z > fCos) &&
        !pWpn->IsRtti(kRttiHashWeaponMelee) &&
        !pMe->GetGameObject()->IsRtti(kRttiHashEntityDroideka))
    {
        return false;
    }

    if (LengthSq(vDelta) < 2500.0f)
    {
        return true;
    }

    float fTolerance;
    if (AIUtil::GetEntityType(pMe->GetEntity()) == AIUtil::ENTITY_FLYER)
    {
        fTolerance = 200.0f;
    }
    else if (AIUtil::IsVehicle(pMe->GetGameObject()))
    {
        fTolerance = 50.0f;
    }
    else
    {
        fTolerance = 20.0f;
    }

    const PblVector3& vEye = pMe->mEyeDir;
    float fCrossX = vEye.z * vDelta.y - vEye.y * vDelta.z;
    float fCrossY = vEye.x * vDelta.z - vEye.z * vDelta.x;
    float fCrossZ = vEye.y * vDelta.x - vEye.x * vDelta.y;
    return fTolerance * fTolerance > fCrossY * fCrossY + fCrossX * fCrossX + fCrossZ * fCrossZ;
}

// FUNCTION: SWBF2 0x00470400
bool CombatUtil::IsTargetLinedUpObj(const Controllable* pMe, const GameObject* pTarget)
{
    const Controller* pCtrl = pMe->mCtrl;
    const GameObject* pCurTarget = UnitControllerPtr(pCtrl)->GetLowLevel().GetTarget();
    if (!pCurTarget || pCurTarget != pTarget)
    {
        return false;
    }
    PblVector3 vLead;
    if (!AIUtil::GetLeadPosition(0.0f, pMe, pTarget, &vLead))
    {
        return false;
    }
    return sIsTargetLinedUpPos(pMe, vLead);
}

namespace CombatUtil {

// FUNCTION: SWBF2 0x00470460
bool TestWpnConstraintsPos(Controllable* pMe, const PblVector3& vAim, const PblVector3& vPos)
{
    if (!CombatUtil::sIsTargetLinedUpPos(pMe, vAim))
    {
        return false;
    }
    float fRange = CombatUtil::GetWeaponMaxRange(pMe);
    float dx = vPos.x - pMe->mEyePoint.x;
    float dy = vPos.y - pMe->mEyePoint.y;
    float dz = vPos.z - pMe->mEyePoint.z;
    return fRange * fRange > dx * dx + dy * dy + dz * dz;
}
}

// FUNCTION: SWBF2 0x004704d0
bool CombatUtil::TestWpnConstraintsObj(Controllable* pMe, const GameObject* pTarget)
{
    if (!pTarget)
    {
        return false;
    }
    const GameObject* pCurTarget = UnitControllerPtr(pMe->mCtrl)->GetLowLevel().GetTarget();
    const Aimer* pAimer = pMe->GetCurAimer();
    if (!pCurTarget || pCurTarget != pTarget || !pAimer)
    {
        return false;
    }
    PblVector3 vLead;
    AIUtil::GetLeadPosition(0.0f, pMe, pTarget, &vLead);
    PblVector3 vPoint = GetTargetPoint(pTarget, pMe->mEyePoint, pMe->mEyeDir, -1);
    return CombatUtil::TestWpnConstraintsPos(pMe, vLead, vPoint);
}

// FUNCTION: SWBF2 0x00470560
bool CombatUtil::CanTurretFaceDisturbance(const Controllable* pMe, const MountedTurretClass* pClass,
                                          const GameObject* pObj, const PblVector3& vPos)
{
    const PblVector3& vTurret = pMe->GetMatrix().GetTranslation();
    PblVector3 vDelta;
    vDelta.x = vPos.x - vTurret.x;
    vDelta.y = vPos.y - vTurret.y;
    vDelta.z = vPos.z - vTurret.z;
    PblVector3 vLocal = AIUtil::UnrotateVector(static_cast<const CollisionObject&>(*pObj).GetMatrix(), vDelta);
    float fYaw = atan2f(vLocal.x, vLocal.z);
    if (fYaw >= pClass->mMinYaw && pClass->mMaxYaw >= fYaw)
    {
        float fPitch = asinf(vLocal.y / sqrtf(LengthSq(vLocal)));
        if (fPitch >= pClass->mMinPitch && pClass->mMaxPitch >= fPitch)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00470680
bool CombatUtil::CanFaceDisturbance(const GameObject* pObj, const PblVector3& vPos)
{
    if (pObj->IsRtti(kRttiHashEntityRemoteTerminal))
    {
        pObj = reinterpret_cast<EntityRemoteTerminal*>(const_cast<GameObject*>(pObj))->GetRemoteGameObject();
    }
    if (!pObj->IsRtti(kRttiHashEntityBuildingArmed))
    {
        return false;
    }

    const EntityBuildingArmed* pBuilding = static_cast<const EntityBuildingArmed*>(pObj);
    const EntityBuildingArmedClass* pClass =
        static_cast<const EntityBuildingArmedClass*>(pBuilding->GetEntityClass());
    for (int i = 0; i < 8; ++i)
    {
        const MountedTurretClass* pTurretClass = pClass->GetTurretClass(i);
        if (!pTurretClass)
        {
            continue;
        }
        const Controllable* pTurret = pBuilding->GetTurretPtr(i);
        if (pTurret && CanTurretFaceDisturbance(pTurret, pTurretClass, pObj, vPos))
        {
            return true;
        }
    }
    return false;
}

namespace CombatUtil {

// FUNCTION: SWBF2 0x00470720
void SendDamageEvent(const Controllable* pOwner, const GameObject* pTarget, const Trigger* pTrigger)
{
    if (!pTarget)
    {
        pTarget = pOwner->mReticuleTarget[pTrigger - pOwner->mControlFire];
        if (!pTarget)
        {
            return;
        }
    }
    if (!pOwner->mCharacter)
    {
        return;
    }
    if (Team::GetPtr(pOwner->GetGameObject()->GetTeam())->GetAffiliation(pTarget->GetTeam()) >= 0)
    {
        return;
    }

    const PblList<Controllable>& list = pTarget->GetControllableList();
    for (PblList<Controllable>::Itor it = list.Begin(); (Controllable*)it; ++it)
    {
        Controllable* pRider = it;
        if (pRider->mCharacter && pRider->mCharacter->GetControllable())
        {
            pRider = pRider->mCharacter->GetControllable();
        }
        if (pRider->mCtrl && pRider->mPlayerId < 0)
        {
            AI::EventInfo_Damaged info(pOwner->mCharacter, true, pOwner->GetMatrix().GetTranslation(), 0.0f, false);
            static_cast<UnitController*>(pRider->mCtrl)->SendEvent(&info);
        }
    }
}

}
