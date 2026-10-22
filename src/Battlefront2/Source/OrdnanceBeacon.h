#pragma once

#include <stddef.h>

#include "OrdnanceSticky.h"

class OrdnanceClass;

// VTABLE: SWBF2 0x007ac588
class OrdnanceBeaconClass : public OrdnanceStickyClass
{
public:
    OrdnanceBeaconClass(unsigned int uHash);
    OrdnanceBeaconClass(OrdnanceBeaconClass* pBase, unsigned int uHash);

    virtual OrdnanceClass* Derive(unsigned int uHash);
    virtual Ordnance* Build(const OrdnanceDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    static PblHash rttiHashOrdnanceBeaconClass;
    static char rttiNameOrdnanceBeaconClass[];

    float mAimAzimuth;
    float mAimElevation;
    float mAimDistance;
    int mSalvoCount;
    float mSalvoDelay;
    float mScatterDistance;
    OrdnanceClass* mOrdnanceClass;
};
  
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x007ac53c
class OrdnanceBeacon : public OrdnanceSticky
{
public:
    OrdnanceBeacon(OrdnanceBeaconClass* pClass, const OrdnanceDesc& desc);
    virtual ~OrdnanceBeacon();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool Update(float fDeltaTime);

    int mSalvoCount;

    static PblHash rttiHashOrdnanceBeacon;
    static char rttiNameOrdnanceBeacon[];
};
 
 
