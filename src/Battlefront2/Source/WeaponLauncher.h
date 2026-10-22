#pragma once

#include <stddef.h>

#include "WeaponCannon.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblHashTable.h"

#include "PebbleFL/Common/PblHandle.h"

class GameObject;
class WeaponLauncherClass;

// VTABLE: SWBF2 0x007b12a4
class WeaponLauncher : public WeaponCannon
{
public:
    WeaponLauncher(const WeaponLauncherClass* pClass, const WeaponDesc& desc);

    virtual ~WeaponLauncher();
    bool IsRtti(unsigned int uRtti) const;
    unsigned int GetDerivedRtti() const;
    const char* GetDerivedRttiName() const;
    bool IsLocked() const;
    const GameObject* GetLocked() const;
    int GetLockedTargetBodyId() const;
    void Write(NetPktGroup& group);
    void Read(NetPktGroup& group);

protected:
    void EnterIdle();
    void ExitFire();
    void EnterCharge();
    bool UpdateCharge(float fDeltaTime);
    void ExitCharge();
    void EnterEmpty();
    bool CheckCharge() const;
    bool CheckFire() const;

    void SetupTarget(const GameObject** ppTarget, int* pBodyId) const;

public:

    static PblHash rttiHashWeaponLauncher;
    static char rttiNameWeaponLauncher[];

    PblHandle<const GameObject> mCurTarget;
    int mCurTargetBodyID;
    bool mLocked;
    float mTrackingSoundInterval;
    float mTrackingSoundTimer;

    char mUnmodeledTail[8];
};
 
 
 
 
 

// VTABLE: SWBF2 0x007b139c
class WeaponLauncherClass : public WeaponCannonClass
{
public:
    WeaponLauncherClass(unsigned int uHash);
    WeaponLauncherClass(WeaponLauncherClass* pBase, unsigned int uHash);
    virtual ~WeaponLauncherClass();

    static PblHash rttiHashWeaponLauncherClass;
    static char rttiNameWeaponLauncherClass[];

    virtual WeaponClass* Derive(unsigned int uHash);

    virtual WeaponLauncher* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    float mLockTime;
    bool mClearLockOnFire;
    bool mHasTrackingAmmo;
};
  
 
 
 
