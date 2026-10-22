#pragma once

#include <stddef.h>

#include "Weapon.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x007b1010
class WeaponInvisibilityClass : public WeaponClass
{
public:
    WeaponInvisibilityClass(unsigned int uHash);
    WeaponInvisibilityClass(WeaponInvisibilityClass* pBase, unsigned int uHash);
    virtual ~WeaponInvisibilityClass();

    static PblHash rttiHashWeaponInvisibilityClass;
    static char rttiNameWeaponInvisibilityClass[];

    enum TRIGGER_TYPE
    {
        TRIGGER_PRESS = 0,
        TRIGGER_TOGGLE = 1,
        TRIGGER_HOLD = 2
    };

    virtual WeaponClass* Derive(unsigned int uHash);
    virtual Weapon* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    TRIGGER_TYPE m_eInvisibilityTrigger;
    float m_fInvisibilityMin;
    float m_fInvisibilityMax;
    float m_fSpeedForInvisibilityMin;
    float m_fSpeedForInvisibilityMax;
    float m_fInvisibilityIncRate;
    float m_fInvisibilityDecRate;
    float m_fInvisibilityPersistTime;
    float m_fFlickerAmplitude;
    float m_fFlickerTimeMin;
    float m_fFlickerTimeMax;
    float mInitialSalvoDelay;
    float mInitialEnergyDrain;
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x007b1048
class WeaponInvisibility : public Weapon
{
public:
    WeaponInvisibility(const WeaponInvisibilityClass* pClass, const WeaponDesc& desc);
    virtual ~WeaponInvisibility();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Select(unsigned int uFlags, bool bOffhand);
    virtual void Deselect(unsigned int uFlags, bool bOffhand);
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

protected:
    virtual void EnterIdle();
    virtual bool UpdateIdle(float fDeltaTime);
    virtual void ExitIdle();
    virtual void EnterFire();
    virtual bool UpdateFire(float fDeltaTime);
    virtual void ExitFire();
    virtual bool UpdateReload(float fDeltaTime);
    virtual void EnterEmpty();
    virtual bool UpdateEmpty(float fDeltaTime);
    virtual void ExitEmpty();
    virtual bool CheckFire() const;
    virtual bool FireDone() const;
    virtual bool CheckEmpty() const;
    virtual bool CheckReload() const;

    void Fire();
    void InputInvisibilityOn();
    void InputInvisibilityOff();
    void StartInvisibility();
    void StopInvisibility();
    void UpdateInvisibility(float fDeltaTime);

public:

    void StartFlicker();

    static PblHash rttiHashWeaponInvisibility;
    static char rttiNameWeaponInvisibility[];

    float m_fInvisibilityFactor;
    float m_fInvisibilityPersistTimer;
    float m_fInvisibilityFlickerTimer;
    bool m_bInvisibilityOn;
    bool m_bInvisibilityActive;
    bool mFired;
    float mOwnerHealth;
};
  
 
 
 
 
