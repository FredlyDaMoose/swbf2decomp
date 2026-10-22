#pragma once

#include <stddef.h>

#include "Aimer.h"
#include "Controllable.h"
#include "Entity.h"
#include "GameSound.h"
#include "Thread.h"
#include "Trackable.h"
#include "Weapon.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class GameObject;
class RedModel;
class RedPose;

// VTABLE: SWBF2 0x007aa720
class MountedTurretClass : public EntityClass, public ControllableClass
{
public:

    MountedTurretClass(unsigned int uHash);

    bool SetProperty(unsigned int uHash, const char* pcValue, const RedModel* pModel);

    virtual ~MountedTurretClass();
    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();

    AimerClass mAimerClass[4];
    WeaponClass* mWeaponClass[4];
    WeaponAmmoDesc mWeaponAmmo[4];
    int mWeaponChannel[4];
    int mWeaponCount;
    CameraTrackSetting mCameraSettings;
    PblVector3 mMountPos;
    PblVector3 mMountUp;
    float mPitchTurnFactor;
    float mYawTurnFactor;
    unsigned int mMountHash;
    unsigned int mParentHash;
    int mHierarchyLevel;
    float mMinYaw;
    float mMaxYaw;
    float mMinPitch;
    float mMaxPitch;
    float mTurnRate;
    float mTurnFilter;
    float mPitchRate;
    float mPitchFilter;
    bool mShiftYaw;
    bool mIsControllable;
    bool mIsAutoFire[4];
    bool mIsVirtual;
    bool mTrackUseRoot;
    bool mForceLockAimerToParentAimer;
    GameSound mTurretYawSound;
    float mTurretYawSoundPitch;
    GameSound mTurretPitchSound;
    float mTurretPitchSoundPitch;
    GameSound mTurretActivateSound;
    GameSound mTurretAmbientSound;
    GameSound mTurretDeactivateSound;
    GameSound mTurretStart;
    GameSound mTurretStop;
    int mFirstPersonModelHandle;

    static PblHash rttiHashMountedTurretClass;
    static char rttiNameMountedTurretClass[];
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x007aa618
class MountedTurret : public EntityEx, public Controllable
{
public:

    // VTABLE: SWBF2 0x007aa600
    class PostCollision : public Thread
    {
    public:
        PostCollision(MountedTurret* pOwner) : mOwner(pOwner) {}
        virtual bool Update(float fDeltaTime);

        MountedTurret* mOwner;
    };

    MountedTurret(GameObject* pParent, MountedTurretClass* pClass, RedPose* pPose);
    virtual ~MountedTurret();

    static void* operator new(size_t uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Init();
    virtual Entity* GetEntity();
    virtual const Entity* GetEntity() const;
    virtual GameObject* GetGameObject();
    virtual const GameObject* GetGameObject() const;
    virtual bool PostCollisionUpdate(float fDeltaTime);

    virtual bool Update(float fDeltaTime);
    virtual void Kill();
    virtual Trackable* GetTrackable();
    virtual const PblMatrix& GetMatrix() const;
    virtual float GetAimTurnRate() const;
    virtual int GetWeaponCount() const;
    virtual int GetWeaponIndex(int iChannel) const;
    virtual Weapon* GetWeaponPtr(int iIndex) const;
    virtual void SetWeaponIndex(int iIndex, int iChannel);
    virtual const Aimer* GetAimer(int iIndex) const;
    virtual ControllableClass* GetControllableClass();
    virtual const ControllableClass* GetControllableClass() const;
    virtual void SetupController();
    virtual void GetPilotMatrix(const PblMatrix& mIn, PblMatrix& mOut);
    virtual PilotDeathType GetPilotDeathType();
    virtual void GetPitchYawLimits(float& fPitchMin, float& fPitchMax,
                                   float& fYawMin, float& fYawMax) const;
    virtual bool UpdateIndirect(float fDeltaTime);

    void Read(NetPktGroup& group);

    void WriteLateState(NetPktGroup& group);
    void ReadLateState(NetPktGroup& group);

    void Write(NetPktGroup& group);

    virtual void SetupCameraTrackMatrixHUDTargeting(PblMatrix& rMatrix);
    virtual void SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime);
    virtual void SetupCameraZoomFactor(float& rZoom);
    virtual void SetupCameraFOV(float& rFOV);
    virtual void SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix);
    virtual Controllable* GetControllable();
    virtual const Controllable* GetControllable() const;
    virtual CameraTrackSetting* GetCameraTrackSettings();
    virtual bool RenderTrackable(unsigned int uView) const;

    int GetFirstPersonIndex();

    void ActivatePhysics();

    void DeactivatePhysics();

    int GetJoystickIndex();

    void Render(const PblMatrix& aRootMatrix, const PblMatrix& aInvRoot, RedPose& aPose,
                unsigned int uiRenderFlags, PblMatrix& aSaveMatrix) const;
    void PostRender(const PblMatrix& aSaveMatrix);

protected:

    void UpdateSound(int prevAngle, int angle, float fAngle, float fAngleMax, float minSoundPitch,
                     const PblVector3* const pos, const PblVector3* const vel, GameSound& sound,
                     GameSoundControllable& controllable, float& soundPitch, float& soundGain);

public:
    PblMatrix mAimMatrix;
    PblMatrix mRootMatrix;
    Aimer* mAimer[4];
    Weapon* mWeapon[4];
    int mWeaponIndex[2];
    float mPitch;
    float mOmegaPitch;
    float mYaw;
    float mOmegaYaw;
    GameObject* mParent;
    MountedTurretClass* mClass;
    RedPose* mPose;
    GameSoundControllable mTurretPitchSound;
    float mTurretPitchSoundPitch;
    float mTurretPitchSoundGain;
    GameSoundControllable mTurretYawSound;
    float mTurretYawSoundPitch;
    float mTurretYawSoundGain;
    PblVector3 mTurretWorldTranslation;
    GameSoundControllable mTurretAmbience;
    Controllable* mPrevPilot;
    float mNetActiveTimer;
    PostCollision mPostCollision;
    int mInterpIndex;

    static PblHash rttiHashMountedTurret;
    static char rttiNameMountedTurret[];
    static MemoryPool sMemoryPool;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
