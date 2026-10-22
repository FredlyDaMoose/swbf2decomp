#pragma once

#include <stddef.h>

#include "Controllable.h"
#include "GameObject.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class EntityRemoteTerminal;
class EntitySoldier;
class GameModel;
class RedPose;

// VTABLE: SWBF2 0x0079cca8
class EntityRemoteTerminalClass : public GameObjectClass, public ControllableClass
{
public:
    EntityRemoteTerminalClass(unsigned int uHash);

    EntityRemoteTerminalClass(EntityRemoteTerminalClass* pBase, unsigned int uHash);
    virtual ~EntityRemoteTerminalClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual int GetVehicleType();

    int mVehicleType;

    static PblHash rttiHashEntityRemoteTerminalClass;
    static char rttiNameEntityRemoteTerminalClass[];
};
  
 
 

// VTABLE: SWBF2 0x0079ca9c
class EntityRemoteTerminal : public GameObject, public Controllable
{
public:

    EntityRemoteTerminal(EntityRemoteTerminalClass* pClass, const EntityDesc& desc);
    virtual ~EntityRemoteTerminal();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void Init();
    virtual const Entity* GetEntity() const;
    virtual Entity* GetEntity();
    virtual const GameObject* GetGameObject() const;
    virtual GameObject* GetGameObject();
    virtual float GetMaxSpeed() const;
    virtual float GetMaxStrafe() const;
    virtual const Controllable* GetControllable() const;
    virtual Controllable* GetControllable();
    virtual void ActivatePhysics();
    virtual void DeactivatePhysics();
    virtual GameObjectType GetGameObjectType();
    virtual EntityType GetEntityType() const;
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual GameObject* AddToClient();
    virtual void AddSends(void (*pfnSend)(GameObject*, int), int iArg);

    virtual const PblMatrix* GetRenderMatrix() const;
    virtual const RedPose* GetRenderPose() const;

    virtual void SetMatrix(const PblMatrix& matrix);
    virtual const PblMatrix& GetMatrix() const;

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    virtual void HealthChangeCallback(bool bUnused, float fDelta);

    virtual bool Update(float fDeltaTime);
    virtual Trackable* GetTrackable();
    virtual ControllableClass* GetControllableClass();
    virtual const ControllableClass* GetControllableClass() const;
    virtual void SetupController();
    virtual void SetPilot(Controllable* pPilot);
    virtual bool UpdateIndirect(float fDeltaTime);

    GameObject* GetRemoteGameObject();
    EntityRemoteTerminal* GetNextLinkedTerminal();
    EntityRemoteTerminal* GetPrevLinkedTerminal();
    bool IsTerminalAvailable(const EntitySoldier* pSoldier);
    EntityRemoteTerminal* GetNextAvailableTerminal(const EntitySoldier* pSoldier);
    EntityRemoteTerminal* GetPrevAvailableTerminal(const EntitySoldier* pSoldier);
    void UpdateHealthRemote(float fAddHealth);

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    static PblHash rttiHashEntityRemoteTerminal;

    // FUNCTION: SWBF2 0x0043ef30
    static unsigned int GetRtti() { return rttiHashEntityRemoteTerminal; }
    static char rttiNameEntityRemoteTerminal[];
    static MemoryPool sMemoryPool;

    EntityRemoteTerminalClass* mClass;
    unsigned int mRemoteGameObjectHash;
    unsigned int mNextLinkedTerminalHash;
    unsigned int mPrevLinkedTerminalHash;
    PblHandle<GameObject> mRemoteGameObject;
    EntityRemoteTerminal* mNextLinkedTerminal;
    EntityRemoteTerminal* mPrevLinkedTerminal;
};
 
 
 
 
 
 
 
 
 
 
 
 
