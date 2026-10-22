#pragma once

#include <stddef.h>

#include "OrdnanceBullet.h"

// VTABLE: SWBF2 0x007ac8d4
class OrdnanceFatRayClass : public OrdnanceBulletClass
{
public:
    OrdnanceFatRayClass(unsigned int uHash);
    virtual ~OrdnanceFatRayClass();
    OrdnanceFatRayClass(OrdnanceFatRayClass* pBase, unsigned int uHash);

    virtual OrdnanceClass* Derive(unsigned int uHash);
    virtual Ordnance* Build(const OrdnanceDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    float mMaxDamage;
    float mMinDamage;
    float mDamageDeduction;
    float mMaxPush;
    float mMinPush;
    float mPushDeduction;
    float mRayRadius;

    static PblHash rttiHashOrdnanceFatRayClass;
    static char rttiNameOrdnanceFatRayClass[];
};
  
 
 
 
 
 
 
 

class OrdnanceFatRay : public OrdnanceBullet
{
public:
    OrdnanceFatRay(OrdnanceFatRayClass* pClass, const OrdnanceDesc& desc);
    virtual ~OrdnanceFatRay();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    virtual bool CheckCollision();

    float mDamage;
    float mPush;

    static PblHash rttiHashOrdnanceFatRay;
    static char rttiNameOrdnanceFatRay[];
};
 
 
 
