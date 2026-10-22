#pragma once

#include <stddef.h>

#include "OrdnanceSticky.h"

// VTABLE: SWBF2 0x007acba0
class OrdnanceHaywireClass : public OrdnanceStickyClass
{
public:
    OrdnanceHaywireClass(unsigned int uHash);
    OrdnanceHaywireClass(OrdnanceHaywireClass* pBase, unsigned int uHash);

    virtual OrdnanceClass* Derive(unsigned int uHash);
    virtual Ordnance* Build(const OrdnanceDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    float mDisableTime;

    static PblHash rttiHashOrdnanceHaywireClass;
    static char rttiNameOrdnanceHaywireClass[];
};
  
 

// VTABLE: SWBF2 0x007acb5c
class OrdnanceHaywire : public OrdnanceSticky
{
public:
    OrdnanceHaywire(OrdnanceHaywireClass* pClass, const OrdnanceDesc& desc);
    virtual ~OrdnanceHaywire();
    virtual bool Update(float fDeltaTime);
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    static PblHash rttiHashOrdnanceHaywire;
    static char rttiNameOrdnanceHaywire[];
};
 
