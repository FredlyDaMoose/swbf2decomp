#pragma once

#include <stddef.h>

#include "EntityBuildingAnimated.h"

class MountedTurretClass;

// VTABLE: SWBF2 0x00799b3c
class EntityBuildingAnimatedArmedClass : public EntityBuildingAnimatedClass
{
public:
    EntityBuildingAnimatedArmedClass(unsigned int uHash);

    EntityBuildingAnimatedArmedClass(EntityBuildingAnimatedArmedClass* pBase, unsigned int uHash);

    virtual ~EntityBuildingAnimatedArmedClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    static PblHash rttiHashEntityBuildingAnimatedArmedClass;
    static char rttiNameEntityBuildingAnimatedArmedClass[];

    MountedTurretClass* mTurretClass[8];
};
  
 

class Controllable;
class MountedTurret;
class Trackable;

class EntityBuildingAnimatedArmed : public EntityBuildingAnimated
{
public:
    EntityBuildingAnimatedArmed(EntityBuildingAnimatedArmedClass* pClass, const EntityDesc& desc);
    virtual ~EntityBuildingAnimatedArmed();
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual void Init();
    virtual void ActivatePhysics();
    virtual void DeactivatePhysics();
    virtual bool Update(float fDeltaTime);
    virtual void Kill();
    virtual void Respawn();
    virtual void Render(unsigned char uFlags, float fDist, unsigned int uMask);
    unsigned int GetDerivedRtti() const;
    const char* GetDerivedRttiName() const;

    GameObjectType GetGameObjectType();
    EntityType GetEntityType() const;
    virtual Trackable* GetTrackable();
    virtual int GetTurretCount() const;
    virtual void AddSends(void (*pfnSend)(GameObject*, int), int iArg);
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual GameObject* AddToClient();
    virtual const Controllable* GetTurretPtr(int iIndex) const;

    static PblHash rttiHashEntityBuildingAnimatedArmed;
    static char rttiNameEntityBuildingAnimatedArmed[];

    MountedTurret* mTurret[8];
    int mNumTurrets;

    char mTailPadding[0xc];
};
 
 
 
