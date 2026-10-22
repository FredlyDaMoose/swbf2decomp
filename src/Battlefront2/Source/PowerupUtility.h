#pragma once

class GameObject;
class DamageOwner;

namespace PowerupUtility
{
    bool ApplyHealth(GameObject* pObject, float fHealth, DamageOwner* pOwner, bool bIgnoreHero);

    bool CheckHealth(GameObject* pObject, float fHealth);
    bool ApplyAmmo(GameObject* pObject, float fAmmo, bool bSkipPowerupDispensers);
    bool CheckAmmo(GameObject* pObject, float fAmmo, bool bSkipPowerupDispensers);
    bool ApplyEnergy(GameObject* pObject, float fEnergy);
    bool ApplyBuff(GameObject* pObject, float fOffenseTimer, float fOffenseMult, float fDefenseTimer,
                   float fDefenseMult, float fHealthTimer, float fHealthRate, float fDamageTimer,
                   float fDamageRate);
}
