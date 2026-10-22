#pragma once

#include <stddef.h>

#include "OrdnanceBullet.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x007acf88
class OrdnanceShellClass : public OrdnanceBulletClass
{
public:
    OrdnanceShellClass(unsigned int uHash);
    OrdnanceShellClass(OrdnanceShellClass* pBase, unsigned int uHash);

    virtual OrdnanceClass* Derive(unsigned int uHash);
    virtual Ordnance* Build(const OrdnanceDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual float GetGravity();

    float mGravity;

    static PblHash rttiHashOrdnanceShellClass;
    static char rttiNameOrdnanceShellClass[];
};
  
 

class OrdnanceShell : public OrdnanceBullet
{
public:
    OrdnanceShell(OrdnanceShellClass* pClass, const OrdnanceDesc& desc);
    virtual ~OrdnanceShell();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

protected:
    virtual bool Update(float fDeltaTime);

public:
    float mGravity;

    static PblHash rttiHashOrdnanceShell;
    static char rttiNameOrdnanceShell[];
};
 
 
