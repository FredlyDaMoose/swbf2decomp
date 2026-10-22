#pragma once

#include <stddef.h>

#include "Weapon.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x007b16ac
class WeaponMeleeThrowClass : public WeaponClass
{
public:
    WeaponMeleeThrowClass(unsigned int uHash);
    WeaponMeleeThrowClass(WeaponMeleeThrowClass* pBase, unsigned int uHash);
    virtual ~WeaponMeleeThrowClass();

    virtual WeaponClass* Derive(unsigned int uHash);
    virtual Weapon* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    int m_iPrimaryWeaponIndex;
    float m_fInitialSalvoDelay;

    static PblHash rttiHashWeaponMeleeThrowClass;

    // FUNCTION: SWBF2 0x0046dbf0
    static unsigned int GetRtti() { return rttiHashWeaponMeleeThrowClass; }
    static char rttiNameWeaponMeleeThrowClass[];
};
  
 
 

class Ordnance;
class WeaponMelee;

// VTABLE: SWBF2 0x007b16e4
class WeaponMeleeThrow : public Weapon
{
public:
    WeaponMeleeThrow(const WeaponMeleeThrowClass* pClass, const WeaponDesc& desc);
    virtual ~WeaponMeleeThrow();
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool IsMeleeThrow() const;
    virtual void SetOrdnance(Ordnance* pOrdnance);
    virtual Ordnance* CreateOrdnance(OrdnanceDesc& desc);
    void NotifyThrow();
    void NotifyReturned();
    virtual bool IsBusy() const;
    virtual bool Update(float fDeltaTime);
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);
    virtual void Render(const PblMatrix& matrix, const RedPose* pPose,
                        const RedColor& color, unsigned int uiFlags, bool bRenderModel) const;

protected:
    virtual void EnterFire();
    virtual bool UpdateFire(float fDeltaTime);
    virtual void ExitFire();
    virtual void EnterEmpty();
    virtual bool CheckFire() const;

    void Fire();
    virtual bool FireDone() const;
    virtual bool CheckEmpty() const;
    virtual bool ReloadDone() const;

public:

    WeaponMelee* m_pPrimaryWeapon;
    unsigned int m_bFired : 1;
    unsigned int m_bThrow : 1;
    unsigned int m_bReturnedThisFrame : 1;
    unsigned int z_uiReserved0 : 29;
    PblMatrix m_mThrow0Local;

    static PblHash rttiHashWeaponMeleeThrow;
    static char rttiNameWeaponMeleeThrow[];
};
  
 
 
