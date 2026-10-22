#pragma once

#include <stddef.h>

#include "CommandPost.h"
#include "EntityWalker.h"

// VTABLE: SWBF2 0x00798854
// VTABLE: SWBF2 0x007987d4
class CommandWalkerClass : public EntityWalkerClass, public CommandPostClass
{
public:
    CommandWalkerClass(unsigned int uHash);

    CommandWalkerClass(CommandWalkerClass* pBase, unsigned int uHash);

    virtual ~CommandWalkerClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    static PblHash rttiHashCommandWalkerClass;
    static char rttiNameCommandWalkerClass[];
};

   
 

#include "RedEngineFL/Memory/RedMemoryPool.h"

// VTABLE: SWBF2 0x00798690
// VTABLE: SWBF2 0x007989a0 CollisionObject
// VTABLE: SWBF2 0x00798628 RedSceneObject
// VTABLE: SWBF2 0x00798974 Damageable
// VTABLE: SWBF2 0x0079884c PblHandled
// VTABLE: SWBF2 0x007988a4 Controllable
// VTABLE: SWBF2 0x007987e0 Trackable
class CommandWalker : public EntityWalker
{
public:
    CommandWalker(CommandWalkerClass* pClass, const EntityDesc& desc);
    virtual ~CommandWalker();

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void Init();
    virtual void SetTeam(int iTeam);
    virtual EntityType GetEntityType() const;
    virtual void WriteCreate(NetPktGroup& group);
    virtual void ReadCreate(NetPktGroup& group);
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual void SkipReadObject(NetObjState* pState);
    virtual GameObject* AddToClient();
    virtual void Kill();
    virtual void Respawn();

    CommandPost* mPost;
    char _tailPad[0xc];

    static MemoryPool sMemoryPool;
    static PblHash rttiHashCommandWalker;
};
 
 
