#pragma once

#include <stddef.h>

#include "Entity.h"
#include "PebbleFL/Common/PblHashTable.h"

class FLEffectClass;

// VTABLE: SWBF2 0x0079b1a0
class EntityEffectClass : public EntityClass
{
public:
    EntityEffectClass(unsigned int uHash);

    virtual ~EntityEffectClass();
    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    static PblHash rttiHashEntityEffectClass;
    static char rttiNameEntityEffectClass[];

    FLEffectClass* mEffectClass;
};
  
 
