#pragma once

#include <stddef.h>

#include "Weapon.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x007b02d4
class WeaponAreaEffectClass : public WeaponClass
{
public:
    WeaponAreaEffectClass(unsigned int uHash);
    WeaponAreaEffectClass(WeaponAreaEffectClass* pBase, unsigned int uHash);
    virtual ~WeaponAreaEffectClass();

    static PblHash rttiHashWeaponAreaEffectClass;
    static char rttiNameWeaponAreaEffectClass[];

    enum FIRE_TYPE
    {
        FIRE_REPEAT = 0,
        FIRE_HOLD = 1,
        FIRE_CHARGE = 2
    };
    enum EFFECT_TYPE
    {
        EFFECT_PUSH = 0,
        EFFECT_PULL = 1,
        EFFECT_CHOKE = 2,
        EFFECT_MINDTRICK = 3,
        EFFECT_STUN = 4
    };
    enum AREA_TYPE
    {
        AREA_SPHERE = 0,
        AREA_CYLINDER = 1,
        AREA_ARC = 2,
        AREA_RAY = 3
    };

    virtual WeaponClass* Derive(unsigned int uHash);
    virtual Weapon* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    FIRE_TYPE m_eFireType;
    EFFECT_TYPE m_eEffectType;
    AREA_TYPE m_eAreaType;
    float m_fInitialSalvoDelay;
    float m_fAreaRange;
    float m_fAreaRadius;
    float m_fAreaHeight;
    int m_iMaxTargets;
    float m_fDamage;
    float m_fEffectStrength;
    float m_fEffectFalloff;
    float m_fEnergyDrainRate;
    float m_fChargeMaxTime;
    float m_fChargeMinStrength;
    float m_fThrustFactor;
    bool m_bTargetLock;

    int m_eSoldierAnimation;
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblHandle.h"

class GameObject;
class Ordnance;
struct OrdnanceDesc;

// VTABLE: SWBF2 0x007b030c
class WeaponAreaEffect : public Weapon
{
public:

    static const int MAX_TARGETS = 4;

    WeaponAreaEffect(const WeaponAreaEffectClass* pClass, const WeaponDesc& desc);
    virtual ~WeaponAreaEffect();
    virtual bool Update(float fDeltaTime);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void NotifySoldierState(int iState, float fTime, bool b0, bool b1,
                                    const PblVector3& vec);
    virtual bool OverrideSoldierControls();
    virtual bool OverrideSoldierEnergyRestore(float& fRestore) const;
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

protected:
    virtual void EnterFire();
    virtual bool UpdateFire(float fDeltaTime);
    virtual void ExitFire();
    virtual void EnterCharge();
    virtual bool UpdateCharge(float fDeltaTime);
    virtual void ExitCharge();
    virtual void EnterEmpty();
    virtual bool CheckCharge() const;
    virtual bool CheckFire() const;
    virtual bool FireDone() const;
    virtual bool CheckEmpty() const;
    virtual bool ReloadDone() const;

public:
    virtual void SetOrdnance(Ordnance* pOrdnance);
    virtual Ordnance* CreateOrdnance(OrdnanceDesc& desc);

protected:

    const WeaponAreaEffectClass* _GetClass() const
    {
        return static_cast<const WeaponAreaEffectClass*>(mClass);
    }

    int SetTargets();
    void ApplyEffectsToTarget(GameObject* pTarget, const PblVector3& vDir, float fStrength);

    int UpdateTargets();
    int UpdateTargets(GameObject** ppTargets, int iNumTargets);
    bool AddTarget(GameObject* pTarget, float fRange);
    void ApplyEffectsToTargets(float fDeltaTime);
    void Fire();

public:
    static PblHash rttiHashWeaponAreaEffect;

    PblHandle<GameObject> m_apTarget[MAX_TARGETS];
    float m_afTargetRange[MAX_TARGETS];
    int m_iNumTargets;
    float m_fStrength;
    bool m_bFired;
};
  
 
 
 
 
 
