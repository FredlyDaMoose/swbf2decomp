#pragma once

#include <stddef.h>

#include "Damageable.h"
#include "Thread.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"

class OrdnanceClass;

class OrbitalStrikeClass
{
public:
    OrbitalStrikeClass(unsigned int uHash);

    OrbitalStrikeClass(OrbitalStrikeClass* pBase, unsigned int uHash);

    bool SetProperty(unsigned int uProperty, const char* pszValue);

    float mAimAzimuth;
    float mAimElevation;
    float mAimDistance;
    int mSalvoCount;
    float mSalvoDelay;
    float mInitialSalvoDelay;
    float mScatterDistance;
    OrdnanceClass* mStrikeClass;
};
 
 
 
 
 
 
 
 
 

class OrbitalStrike : public Thread, public PblHandled
{
public:
    OrbitalStrike(const OrbitalStrikeClass* pClass, struct OrbitalStrikeDesc& desc);
    virtual ~OrbitalStrike();
    virtual bool Update(float fDeltaTime);

    OrbitalStrikeClass* mClass;
    PblVector3 mPosition;
    DamageOwner mDamageOwner;
    int mSalvoCount;
    float mSalvoDelay;
};

struct OrbitalStrikeDesc
{
    PblVector3 mPosition;
    DamageOwner mDamageOwner;
};
 
 
 
 
 
 
 
