#pragma once

#include <stddef.h>

#include "CommandPost.h"
#include "EntityFlyer.h"
#include "EntityCarrier.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

// VTABLE: SWBF2 0x00797ff8
// VTABLE: SWBF2 0x00797fc8
class CommandFlyerClass : public EntityFlyerClass, public CommandPostClass
{
public:
    CommandFlyerClass(unsigned int uHash);
    virtual ~CommandFlyerClass();

    CommandFlyerClass(CommandFlyerClass* pBase, unsigned int uHash);

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    static PblHash rttiHashCommandFlyerClass;
    static char rttiNameCommandFlyerClass[];
};

   
 

class CommandFlyer : public EntityFlyer
{
public:

    CommandFlyer(CommandFlyerClass* pClass, const EntityDesc& desc);

    virtual ~CommandFlyer();

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow) { sMemoryPool.Setup(uCount, uSize, uGrow); }

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void Init();
    virtual void SetTeam(int iTeam);
    virtual EntityType GetEntityType() const;
    virtual void Kill();
    virtual void Respawn();
    virtual bool Update(float fDeltaTime);

    bool IsLanded();

    Controllable* GetPassengerSlot(int);

    void EnablePost(bool bEnable);

    CommandFlyerClass* mClass;
    CommandPost* mPost;
    bool mbHasTakenOffBefore;
    char _tailPad[0x7];

    static MemoryPool sMemoryPool;
    static PblHash rttiHashCommandFlyer;
    static char rttiNameCommandFlyer[];
};
 
 
 
 
