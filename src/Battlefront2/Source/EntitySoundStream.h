#pragma once

#include <stddef.h>

#include "EntitySound.h"
#include "Thread.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "SoundFL/source/sndsoundstream.h"

#include "RedEngineFL/Memory/RedMemoryPool.h"

// VTABLE: SWBF2 0x0079d458
class EntitySoundStreamClass : public EntitySoundClass
{
public:
    EntitySoundStreamClass(unsigned int uHash);
    EntitySoundStreamClass(EntitySoundStreamClass* pBase, unsigned int uHash);
    virtual ~EntitySoundStreamClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();

    static PblHash rttiHashEntitySoundStreamClass;
    static char rttiNameEntitySoundStreamClass[];
};
  

class EntitySoundStream : public EntitySound, public Thread
{
public:
    EntitySoundStream(EntitySoundStreamClass* pClass, const EntityDesc& desc);
    virtual ~EntitySoundStream();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Play();
    virtual bool Update(float fDeltaTime);

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    static PblHash rttiHashEntitySoundStream;
    static MemoryPool sMemoryPool;
    static char rttiNameEntitySoundStream[];

    Snd::SoundStream mSoundStream;
};
 
 
