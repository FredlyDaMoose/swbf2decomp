#pragma once

#include <stddef.h>

#include "CommandPost.h"
#include "EntityBuildingAnimatedArmed.h"

// VTABLE: SWBF2 0x00797888
// VTABLE: SWBF2 0x00797b08
class CommandBuildingAnimatedArmedClass : public EntityBuildingAnimatedArmedClass,
                                          public CommandPostClass
{
public:
    CommandBuildingAnimatedArmedClass(unsigned int uHash);

    CommandBuildingAnimatedArmedClass(CommandBuildingAnimatedArmedClass* pBase, unsigned int uHash);

    virtual ~CommandBuildingAnimatedArmedClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    static PblHash rttiHashCommandBuildingAnimatedArmedClass;
    static char rttiNameCommandBuildingAnimatedArmedClass[];
};

   
 

class CommandBuildingAnimatedArmed : public EntityBuildingAnimatedArmed
{
public:

    CommandBuildingAnimatedArmed(CommandBuildingAnimatedArmedClass* pClass, const EntityDesc& desc);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~CommandBuildingAnimatedArmed();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void Init();
    virtual void SetTeam(int iTeam);
    virtual EntityType GetEntityType() const;
    virtual void Kill();
    virtual void Respawn();
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual GameObject* AddToClient();

    static PblHash rttiHashCommandBuildingAnimatedArmed;
    static char rttiNameCommandBuildingAnimatedArmed[];

    CommandBuildingAnimatedArmedClass* mClass;
    CommandPost* mPost;
};
 
 
 
