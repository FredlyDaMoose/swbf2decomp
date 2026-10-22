#pragma once

#include <stddef.h>

#include "CommandPost.h"
#include "EntityBuildingArmed.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x00797cc8
// VTABLE: SWBF2 0x00797d14
class CommandBuildingArmedClass : public EntityBuildingArmedClass, public CommandPostClass
{
public:
    CommandBuildingArmedClass(unsigned int uHash);

    CommandBuildingArmedClass(CommandBuildingArmedClass* pBase, unsigned int uHash);

    virtual ~CommandBuildingArmedClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    static PblHash rttiHashCommandBuildingArmedClass;
    static char rttiNameCommandBuildingArmedClass[];
};

   
 

class CommandBuildingArmed : public EntityBuildingArmed
{
public:

    CommandBuildingArmed(CommandBuildingArmedClass* pClass, const EntityDesc& desc);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~CommandBuildingArmed();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void Init();
    virtual void SetTeam(int iTeam);
    virtual EntityType GetEntityType() const;
    virtual void Kill();
    virtual void Respawn();
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual GameObject* AddToClient();

    static PblHash rttiHashCommandBuildingArmed;
    static char rttiNameCommandBuildingArmed[];

    CommandBuildingArmedClass* mClass;
    CommandPost* mPost;
};
 
 
 
