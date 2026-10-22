#pragma once

#include <stddef.h>

#include "CommandPost.h"
#include "EntityHover.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class MemoryPool;

// VTABLE: SWBF2 0x007983a0
// VTABLE: SWBF2 0x00798580
class CommandHoverClass : public EntityHoverClass, public CommandPostClass
{
public:
    CommandHoverClass(unsigned int uHash);

    CommandHoverClass(CommandHoverClass* pBase, unsigned int uHash);

    virtual ~CommandHoverClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    static PblHash rttiHashCommandHoverClass;
    static char rttiNameCommandHoverClass[];
};

   
 

class CommandHover : public EntityHover
{
public:

    CommandHover(CommandHoverClass* pClass, const EntityDesc& desc);

    virtual ~CommandHover();

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
    virtual void Kill();
    virtual void Respawn();
    virtual GameObject* AddToClient();

    CommandHoverClass* mClass;
    CommandPost* mPost;
    char _tailPad[0x8];

    static MemoryPool sMemoryPool;
    static PblHash rttiHashCommandHover;
    static char rttiNameCommandHover[];
};
 
 
 
