#include "PowerupUtility.h"

#include <float.h>

#include "Controllable.h"
#include "Damageable.h"
#include "EntityDroideka.h"
#include "EntitySoldier.h"
#include "EnergyBar.h"
#include "GameObject.h"
#include "PowerupItem.h"
#include "WeaponCannonInstance.h"
#include "WeaponDispenser.h"
#include "PebbleFL/Common/PblList.h"

class AmmoCounter
{
public:

    bool HasInfiniteAmmo() const { return mMaxClips == FLT_MAX; }

    bool IsReserveFull() const { return mNumClips >= mMaxClips - 1.0f; }

    void UnloadClip();
    void AddClips(float fClips);

    AmmoCounterClass* m_pClass;
    float mDefaultMaxClips;
    float mMaxClips;
    float mNumClips;
    float mCurClip;
    unsigned int m_uiRefCount;
};
 
 
 
 

 
 

namespace PowerupUtility
{

// FUNCTION: SWBF2 0x00626690
static bool DoHealth(GameObject* pObject, float fHealth, bool bApply, DamageOwner* pOwner)
{
    if (fHealth != 0.0f)
    {
        if ((pObject->mCurHealth < pObject->mMaxHealth && pObject->mAddHealth >= 0.0f) || fHealth < 0.0f)
        {
            if (bApply)
            {
                float fNewHealth = pObject->mCurHealth + fHealth;
                if (fNewHealth > pObject->mMaxHealth)
                {
                    fNewHealth = pObject->mMaxHealth;
                }
                pObject->SetCurHealth(fNewHealth, true, pOwner);
            }
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00626700
bool ApplyHealth(GameObject* pObject, float fHealth, DamageOwner* pOwner, bool bIgnoreHero)
{
    if (!bIgnoreHero && pObject->GetGameObjectClass()->mHeroFlag)
    {
        return false;
    }
    return DoHealth(pObject, fHealth, true, pOwner);
}

// FUNCTION: SWBF2 0x00626730
bool CheckHealth(GameObject* pObject, float fHealth)
{
    return DoHealth(pObject, fHealth, false, 0);
}

// FUNCTION: SWBF2 0x00626740
static bool DoAmmo(GameObject* pObject, float fAmmo, bool bSkipPowerupDispensers, bool bApply)
{
    bool bResult = false;
    if (fAmmo == 0.0f)
    {
        return bResult;
    }

    for (PblList<Controllable>::Itor it(pObject->GetControllableList()); it; ++it)
    {
        int iNumWeapons = it->GetWeaponCount();
        for (int i = 0; i < iNumWeapons; ++i)
        {
            Weapon* pWeapon = it->GetWeaponPtr(i);
            if (!pWeapon)
            {
                continue;
            }
            if (bSkipPowerupDispensers &&
                pWeapon->mClass->IsRtti(WeaponDispenserClass::rttiHashWeaponDispenserClass))
            {
                EntityClass* pDispenseClass = ((WeaponDispenserClass*)pWeapon->mClass)->mDispenseClass;
                if (pDispenseClass && pDispenseClass->IsRtti(PowerupItemClass::rttiHashPowerupItemClass))
                {
                    continue;
                }
            }

            AmmoCounter* pAmmo = pWeapon->m_pAmmoCounter;
            bool bClipEmpty = pAmmo->mMaxClips <= 1.0f && pAmmo->mCurClip < 1.0f;
            bool bTook = false;
            if (fAmmo > 0.0f && !pAmmo->HasInfiniteAmmo() && (!pAmmo->IsReserveFull() || bClipEmpty))
            {
                bTook = true;
                if (bApply)
                {
                    if (bClipEmpty)
                    {
                        pWeapon->m_pAmmoCounter->UnloadClip();
                    }
                    pWeapon->m_pAmmoCounter->AddClips(fAmmo);
                }
            }
            bResult |= bTook;
        }
    }
    return bResult;
}

// FUNCTION: SWBF2 0x006268b0
bool ApplyAmmo(GameObject* pObject, float fAmmo, bool bSkipPowerupDispensers)
{
    bool bResult = DoAmmo(pObject, fAmmo, bSkipPowerupDispensers, true);
    if (bResult)
    {
        if (pObject->GetEntityType() == ENT_SOLDIER)
        {
            static_cast<EntitySoldier*>(pObject)->TriggerAmmoGainEffect();
        }
        else if (pObject->GetEntityType() == ENT_DROIDEKA)
        {
            static_cast<EntityDroideka*>(pObject)->TriggerAmmoGainEffect();
        }
    }
    return bResult;
}

// FUNCTION: SWBF2 0x00626900
bool CheckAmmo(GameObject* pObject, float fAmmo, bool bSkipPowerupDispensers)
{
    return DoAmmo(pObject, fAmmo, bSkipPowerupDispensers, false);
}

// FUNCTION: SWBF2 0x00626910
static bool DoEnergy(GameObject* pObject, float fEnergy, bool bApply)
{
    bool bResult = false;
    if (fEnergy == 0.0f)
    {
        return bResult;
    }

    for (PblList<Controllable>::Itor it(pObject->GetControllableList()); it; ++it)
    {
        EnergyBar* pBar = it->GetEnergyBar();
        if (!pBar)
        {
            continue;
        }
        float fCur = pBar->m_fEnergy;
        if (fEnergy > 0.0f)
        {
            float fMax = pBar->m_pClass->m_fEnergyMax;
            if (fMax > fCur)
            {
                float fAdd = fMax - fCur;
                bResult = true;
                if (fAdd > fEnergy)
                {
                    fAdd = fEnergy;
                }
                if (bApply)
                {
                    pBar->AddEnergy(fAdd);
                }
            }
        }
        else if (fCur > 0.0f)
        {
            float fSpend = -fEnergy;
            bResult = true;
            if (fSpend > fCur)
            {
                fSpend = fCur;
            }
            if (bApply)
            {
                pBar->SpendEnergy(fSpend);
            }
        }
    }
    return bResult;
}

// FUNCTION: SWBF2 0x006269c0
bool ApplyEnergy(GameObject* pObject, float fEnergy)
{
    bool bResult = DoEnergy(pObject, fEnergy, true);
    if (bResult)
    {
        if (pObject->GetEntityType() == ENT_SOLDIER)
        {
            static_cast<EntitySoldier*>(pObject)->TriggerEnergyGainEffect();
        }
        else if (pObject->GetEntityType() == ENT_DROIDEKA)
        {
            static_cast<EntityDroideka*>(pObject)->TriggerEnergyGainEffect();
        }
    }
    return bResult;
}

// FUNCTION: SWBF2 0x00626a10
static bool DoBuff(GameObject* pObject, float fOffenseTimer, float fOffenseMult, float fDefenseTimer,
                   float fDefenseMult, float fHealthTimer, float fHealthRate, float fDamageTimer,
                   float fDamageRate, bool bApply)
{
    bool bResult = false;
    if (fOffenseTimer > 0.0f || fDefenseTimer > 0.0f || fHealthTimer > 0.0f || fDamageTimer > 0.0f)
    {
        if (pObject->GetDerivedRtti() == EntitySoldier::rttiHashEntitySoldier)
        {
            EntitySoldier* pSoldier = static_cast<EntitySoldier*>(pObject);
            bResult = true;
            if (bApply)
            {
                if (fDamageTimer > 0.0f)
                {
                    pSoldier->DebuffDamage(fDamageTimer, fDamageRate, DamageOwner(0, 0, 0, 0, 0, 1.0f));
                }
                if (fOffenseTimer > 0.0f)
                {
                    pSoldier->BuffOffense(fOffenseTimer, fOffenseMult);
                }
                if (fDefenseTimer > 0.0f)
                {
                    pSoldier->BuffDefense(fDefenseTimer, fDefenseMult);
                }
                if (fHealthTimer > 0.0f)
                {
                    pSoldier->BuffHealth(fHealthTimer, fHealthRate);
                }
            }
        }
        else if (pObject->GetDerivedRtti() == EntityDroideka::rttiHashEntityDroideka)
        {
            EntityDroideka* pDroideka = static_cast<EntityDroideka*>(pObject);
            bResult = true;
            if (bApply)
            {
                if (fDamageTimer > 0.0f)
                {
                    pDroideka->DebuffDamage(fDamageTimer, fDamageRate, DamageOwner(0, 0, 0, 0, 0, 1.0f));
                }
                if (fOffenseTimer > 0.0f)
                {
                    pDroideka->BuffOffense(fOffenseTimer, fOffenseMult);
                }
                if (fDefenseTimer > 0.0f)
                {
                    pDroideka->BuffDefense(fDefenseTimer, fDefenseMult);
                }
                if (fHealthTimer > 0.0f)
                {
                    pDroideka->BuffHealth(fHealthTimer, fHealthRate);
                }
            }
        }
    }
    return bResult;
}

// FUNCTION: SWBF2 0x00626b90
bool ApplyBuff(GameObject* pObject, float fOffenseTimer, float fOffenseMult, float fDefenseTimer,
               float fDefenseMult, float fHealthTimer, float fHealthRate, float fDamageTimer,
               float fDamageRate)
{
    return DoBuff(pObject, fOffenseTimer, fOffenseMult, fDefenseTimer, fDefenseMult, fHealthTimer,
                  fHealthRate, fDamageTimer, fDamageRate, true);
}

}
