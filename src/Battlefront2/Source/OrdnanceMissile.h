#pragma once

#include <stddef.h>

#include "OrdnanceBullet.h"

// VTABLE: SWBF2 0x007acecc
class OrdnanceMissileClass : public OrdnanceBulletClass
{
public:
    OrdnanceMissileClass(unsigned int uHash);
    virtual ~OrdnanceMissileClass();
    OrdnanceMissileClass(OrdnanceMissileClass* pBase, unsigned int uHash);

    virtual OrdnanceClass* Derive(unsigned int uHash);
    virtual Ordnance* Build(const OrdnanceDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual float GetVelocity();

    static PblHash rttiHashOrdnanceMissileClass;
    static char rttiNameOrdnanceMissileClass[];

    float mScaleRate;
    float mTurnRate;
    float mWaverRate;
    float mWaverTurn;
    GameSound mLockedOnSound;
    float mMinSpeed;
    float mAcceleration;
};
  
 
 
 
 
 
 
 

class GameObject;

// VTABLE: SWBF2 0x007ace84
// VTABLE: SWBF2 0x007acec4
// VTABLE: SWBF2 0x007acefc
class OrdnanceMissile : public OrdnanceBullet
{
public:
    OrdnanceMissile(OrdnanceMissileClass* pClass, const OrdnanceDesc& desc);
    virtual ~OrdnanceMissile();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

protected:
    virtual bool Update(float fDeltaTime);

public:
    PblVector3 mTargetDir;
    PblHandle<GameObject const> mTarget;
    int mBodyID;
    float mScale;
    float mWaverPitch;
    float mWaverYaw;
    GameSoundControllable mLockedOnSoundCtrl;
    PblVector3 mVelocityDir;
    float mGoalSpeed;
    float mSpeed;

    static PblHash rttiHashOrdnanceMissile;
    static char rttiNameOrdnanceMissile[];
};
 
 
 
 
 
