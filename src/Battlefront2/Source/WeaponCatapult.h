#pragma once

#include <stddef.h>

#include "Weapon.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x007b06d0
class WeaponCatapultClass : public WeaponClass
{
public:
    WeaponCatapultClass(unsigned int uHash);

    WeaponCatapultClass(WeaponCatapultClass* pBase, unsigned int uHash);

    virtual ~WeaponCatapultClass();

    static PblHash rttiHashWeaponCatapultClass;
    static char rttiNameWeaponCatapultClass[];

    virtual WeaponClass* Derive(unsigned int uHash);
    virtual Weapon* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    RedModel* mOrdnanceModel;
    unsigned int mArmHash;
    float mCockedAngle;
};
  
 
 
 

#include "WeaponCannonInstance.h"

class EntityBuildingArmed;

// VTABLE: SWBF2 0x007b0708
class WeaponCatapult : public Weapon
{
public:
    WeaponCatapult(const WeaponCatapultClass* pClass, const WeaponDesc& desc);
    virtual ~WeaponCatapult();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Render(const PblMatrix& matrix, const RedPose* pPose, const RedColor& color,
                        unsigned int uFlags, bool bArg) const;

protected:
    virtual void EnterFire();
    virtual bool UpdateFire(float fDeltaTime);
    virtual void EnterReload();
    virtual bool UpdateReload(float fDeltaTime);
    virtual void ExitReload();
    virtual bool CheckFire() const;
    virtual bool FireDone() const;

    virtual bool CheckReload() const;

    void SetArmAngle(float fAngle);

    bool Fire();

public:
    static PblHash rttiHashWeaponCatapult;
    static char rttiNameWeaponCatapult[];

    PblMatrix mArmPoseMatrix;
    WeaponCatapultClass* mCatapultClass;
    float mLastArmAngle;
    float mNextArmAngle;
    float mArmAccel;
    float mArmSpeed;
    float mCurrArmAngle;
    bool mShowOrdnance;
    EntityBuildingArmed* mCatapultBuilding;
};
  
 
 
 
 
 
 
 
 
 
