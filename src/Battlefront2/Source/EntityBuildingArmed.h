#pragma once

#include <stddef.h>

#include "EntityBuilding.h"
#include "Thread.h"

class MountedTurretClass;

// VTABLE: SWBF2 0x00799c8c
class EntityBuildingArmedClass : public EntityBuildingClass
{
public:
    EntityBuildingArmedClass(unsigned int uHash);

    EntityBuildingArmedClass(EntityBuildingArmedClass* pBase, unsigned int uHash);

    virtual ~EntityBuildingArmedClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    const MountedTurretClass* GetTurretClass(int iIndex) const;

    MountedTurretClass* mTurretClass[8];

    static PblHash rttiHashEntityBuildingArmedClass;

    // FUNCTION: SWBF2 0x00476ed0
    static unsigned int GetRtti() { return rttiHashEntityBuildingArmedClass; }
    static char rttiNameEntityBuildingArmedClass[];

};
  
 

#include "PebbleFL/Common/PblHandle.h"
#include "RedEngineFL/Graphics/RedPose.h"

class Controllable;
class MountedTurret;
class Trackable;
class FLEffectObject;

class EntityBuildingArmed : public EntityBuilding, public Thread
{
public:

    EntityBuildingArmed(EntityBuildingArmedClass* pClass, const EntityDesc& desc);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~EntityBuildingArmed();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void Init();
    virtual const Controllable* GetControllable() const;
    virtual Controllable* GetControllable();
    virtual void ActivatePhysics();
    virtual void DeactivatePhysics();
    virtual GameObjectType GetGameObjectType();
    virtual EntityType GetEntityType() const;
    virtual void AddSends(void (*pfnSend)(GameObject*, int), int iArg);
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual GameObject* AddToClient();
    virtual Trackable* GetTrackable();
    virtual int GetTurretCount() const;
    virtual const Controllable* GetTurretPtr(int iIndex) const;
    virtual const RedPose* GetPose() const;
    virtual void Render(unsigned char uFlags, float fDist, unsigned int uMask);
    virtual void Kill();
    virtual void Respawn();
    virtual bool Update(float fDeltaTime);

    RedPose mPose;
    MountedTurret* mTurret[8];
    int mNumTurrets;
    PblHandle<FLEffectObject> mSmokePlume;
    bool mNoAIBoard : 1;

    static PblHash rttiHashEntityBuildingArmed;

    // FUNCTION: SWBF2 0x0043ef20
    static unsigned int GetRtti() { return rttiHashEntityBuildingArmed; }
    static char rttiNameEntityBuildingArmed[];
};
 
 
 
 
 
 
