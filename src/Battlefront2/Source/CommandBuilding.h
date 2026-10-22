#pragma once

#include <stddef.h>

#include "CommandPost.h"
#include "EntityBuildingAnimated.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x007977f8
// VTABLE: SWBF2 0x007977e4
class CommandBuildingClass : public EntityBuildingAnimatedClass, public CommandPostClass
{
public:
    CommandBuildingClass(unsigned int uHash);

    CommandBuildingClass(CommandBuildingClass* pBase, unsigned int uHash);
    virtual ~CommandBuildingClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    static PblHash rttiHashCommandBuildingClass;
    static char rttiNameCommandBuildingClass[];
};

   
 

class CommandBuilding : public EntityBuildingAnimated
{
public:

    CommandBuilding(CommandBuildingClass* pClass, const EntityDesc& desc);

    virtual bool IsRtti(unsigned int uRtti) const;
    unsigned int GetDerivedRtti() const;
    const char* GetDerivedRttiName() const;
    virtual ~CommandBuilding();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void Init();
    virtual void SetTeam(int iTeam);
    virtual void Kill();
    virtual void Respawn();

    EntityType GetEntityType() const;
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual GameObject* AddToClient();

    static PblHash rttiHashCommandBuilding;
    static char rttiNameCommandBuilding[];

    CommandBuildingClass* mClass;
    CommandPost* mPost;
};
 
 
 
