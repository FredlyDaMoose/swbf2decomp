#pragma once

#include <stddef.h>

#include "Controllable.h"
#include "Entity.h"
#include "Trackable.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class MemoryPool;
class RedModel;

// VTABLE: SWBF2 0x007ad594
class PassengerSlotClass : public EntityClass, public ControllableClass
{
public:
    PassengerSlotClass(unsigned int uHash);

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);

    bool SetProperty(unsigned int uProperty, const char* pszValue, const RedModel* pModel);

    CameraTrackSetting mCameraSettings;
    float mPowerupDelay;
    float mAddHealth;
    float mAddAmmo;
    PblMatrix mMountMatrix;
};
 
 
 
 
 
 

class CameraTrackSetting;
class GameObject;
class RedPose;

// VTABLE: SWBF2 0x007ad448
// VTABLE: SWBF2 0x007ad47c Trackable
// VTABLE: SWBF2 0x007ad4c4 Controllable
class PassengerSlot : public EntityEx, public Controllable
{
public:

    static MemoryPool sMemoryPool;

    PassengerSlot(GameObject* pParent, PassengerSlotClass* pClass, RedPose* pPose);
    virtual ~PassengerSlot();
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Init();
    virtual Entity* GetEntity();
    virtual const Entity* GetEntity() const;
    virtual GameObject* GetGameObject();
    virtual const GameObject* GetGameObject() const;
    virtual Trackable* GetTrackable();
    virtual Controllable* GetControllable();
    virtual const Controllable* GetControllable() const;
    virtual void Kill();
    virtual bool Update(float fDeltaTime);
    virtual ControllableClass* GetControllableClass();
    virtual const ControllableClass* GetControllableClass() const;

    static void* operator new(size_t uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    void ActivatePhysics();
    void DeactivatePhysics();

    virtual void SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime);
    virtual void SetupCameraZoomFactor(float& rZoom);
    virtual void SetupCameraFOV(float& rFOV);
    virtual CameraTrackSetting* GetCameraTrackSettings();

    virtual const PblMatrix& GetMatrix() const;
    virtual bool UpdateIndirect(float fDeltaTime);

    virtual void SetupController();

    float mPowerupTimer;
    GameObject* mParent;
    PblMatrix mAimMatrix;
    PassengerSlotClass* mClass;

    static PblHash rttiHashPassengerSlot;
    static char rttiNamePassengerSlot[];
};
 
 
 
 
