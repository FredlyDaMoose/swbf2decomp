#pragma once

#include <stddef.h>

#include "EntitySound.h"
#include "GameSound.h"
#include "Thread.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x0079d408
class EntitySoundStaticClass : public EntitySoundClass
{
public:
    EntitySoundStaticClass(unsigned int uHash);
    EntitySoundStaticClass(EntitySoundStaticClass* pBase, unsigned int uHash);
    virtual ~EntitySoundStaticClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();

    static PblHash rttiHashEntitySoundStaticClass;
    static char rttiNameEntitySoundStaticClass[];
};
  

// VTABLE: SWBF2 0x0079d3d0
// VTABLE: SWBF2 0x0079d3b8 Thread
class EntitySoundStatic : public EntitySound, public Thread
{
public:
    EntitySoundStatic(EntitySoundStaticClass* pClass, const EntityDesc& desc);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~EntitySoundStatic();
    virtual void Play();
    virtual bool Update(float fDeltaTime);

    static void* operator new(unsigned int uSize);

    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    static PblHash rttiHashEntitySoundStatic;
    static char rttiNameEntitySoundStatic[];
    static MemoryPool sMemoryPool;

    GameSoundControllable mSound;
    GameSound mSoundProperties;
};
 
 
 
