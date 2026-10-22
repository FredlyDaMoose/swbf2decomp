#pragma once

#include <stddef.h>

#include "Aimer.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "GameObject.h"
#include "GameSound.h"
#include "Trackable.h"
#include "Weapon.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class MemoryPool;
class Aimer;
class Entity;
class EnergyBar;
class ExplosionClass;
class FLEffectObject;
class GameObject;
class Weapon;

void* __fastcall GetFirstWeaponFactoryNode(const void* pList);
void* __fastcall InitializeWeaponFactoryIterator(void* pItor, void*, void* pNode);
void* __fastcall GetWeaponFactoryIteratorObject(const void* pItor);
void* __fastcall DereferenceWeaponFactoryIterator(const void* pItor);
void* __fastcall ConvertWeaponFactoryIteratorToPointer(const void* pItor);

// VTABLE: SWBF2 0x0079aac0
class EntityDroidClass : public GameObjectClass, public ControllableClass
{
public:
    EntityDroidClass(unsigned int uHash);
    EntityDroidClass(EntityDroidClass* pBase, unsigned int uHash);
    virtual ~EntityDroidClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual const void* GetControllableClass();
    virtual int GetWeaponCount();
    virtual const WeaponClass* GetWeaponClass(int iIndex);

    static PblHash rttiHashEntityDroidClass;
    static char rttiNameEntityDroidClass[];

    float mSetAltitude;
    float mGravityScale;
    float mLiftSpring;
    float mLiftDamp;
    float mAcceleration;
    float mMaxSpeed;
    float mMaxStrafeSpeed;
    float mMaxTurnSpeed;
    float mMaxPitchSpeed;
    float mMinPitch;
    float mMaxPitch;
    float mLevelSpring;
    float mLevelDamp;
    float mCollisionThreshold[3];
    float mCollisionScale[3];
    float mCollisionInflict;
    AimerClass mAimerClass[4];
    WeaponClass* mWeaponClass[4];
    WeaponAmmoDesc mWeaponAmmo[4];
    int mWeaponChannel[4];
    int mWeaponCount;
    float mNoiseFactor;
    CameraTrackSetting mCameraSettings;
    ExplosionClass* mExplosion;
    EnergyBarClass mEnergyBarClass;
    float mEnergyAutoRestore;
    PblVector3 mCameraDestructTrackOffset;
    float mCameraDestructTiltValue;
    unsigned int mDestructChargeEffect;
    GameSound mAmbientSound;
    GameSound mAmbient2Sound;
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

 

class CollisionResult;
class Restrictor;

// VTABLE: SWBF2 0x0079ab0c
// VTABLE: SWBF2 0x0079ac7c CollisionObject
// VTABLE: SWBF2 0x0079aa14 RedSceneObject
// VTABLE: SWBF2 0x0079aa94 Damageable
// VTABLE: SWBF2 0x0079acd0 PblHandled
// VTABLE: SWBF2 0x0079a944 Controllable
// VTABLE: SWBF2 0x0079ac34 Trackable
class EntityDroid : public GameObject, public Controllable
{
public:
    enum State
    {
        NORMAL = 0,
        FROZEN = 1,
        EXPLODE = 2,
        KILLTIMER = 3
    };

    // VTABLE: SWBF2 0x0079aa7c
    class PostCollision : public Thread
    {
    public:

        PostCollision(EntityDroid* pOwner) : mOwner(pOwner) {}
        virtual bool Update(float fDeltaTime);

        EntityDroid* mOwner;
    };

    EntityDroid(EntityDroidClass* pClass, const EntityDesc& desc);

    virtual ~EntityDroid();

    static void* operator new(size_t uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Init();
    virtual const Entity* GetEntity() const;
    virtual Entity* GetEntity();
    virtual const GameObject* GetGameObject() const;
    virtual GameObject* GetGameObject();
    virtual const PblVector3& GetVelocity() const;
    virtual void SetVelocity(const PblVector3& vVelocity);
    virtual void ClientKill();
    virtual float GetMaxSpeed() const;
    virtual float GetMaxStrafe() const;
    virtual TargetInfo* GetTargetInfo();
    virtual const Controllable* GetControllable() const;
    virtual Controllable* GetControllable();
    virtual bool ShouldShowReticule() const;
    virtual void ApplyRadiusPush(const PblVector3& vPos, float fInnerRadius,
                                 float fOuterRadius, float fPush, bool bArg);
    virtual void ActivatePhysics();
    virtual void DeactivatePhysics();
    virtual EntityType GetEntityType() const;

    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual void AddSends(void (*pfnSend)(GameObject*, int), int iArg);
    virtual const PblMatrix& GetSmoothedMatrix() const;
    virtual void WriteLateState(NetPktGroup& group);
    virtual void ReadLateState(NetPktGroup& group);

    virtual bool PostCollisionUpdate(float fDeltaTime);
    virtual void SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix,
                                                    float fDeltaTime);

    virtual float GetSmallestCollisionDim() const;
    virtual bool CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor);

    virtual const PblMatrix& GetMatrix() const;
    virtual void RecalculateAABB();

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    virtual void Kill();

    virtual bool Update(float fDeltaTime);
    virtual Trackable* GetTrackable();
    virtual float GetSoundRadius() const;
    virtual float GetAimTurnRate() const;
    virtual int GetWeaponCount() const;
    virtual int GetWeaponIndex(int iChannel) const;
    virtual Weapon* GetWeaponPtr(int iIndex) const;
    virtual EnergyBar* GetEnergyBar();
    virtual void SetWeaponIndex(int iIndex, int iChannel);
    virtual const Aimer* GetAimer(int iIndex) const;
    virtual ControllableClass* GetControllableClass();
    virtual const ControllableClass* GetControllableClass() const;
    virtual void SetupController();
    virtual bool UpdateIndirect(float fDeltaTime);

    virtual void SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime);
    virtual void SetupCameraZoomFactor(float& rZoom);
    virtual void SetupCameraFOV(float& rFOV);
    virtual bool RenderTrackable(unsigned int uView) const;
    virtual bool IsForcedThirdPerson() const;

    void TriggerDestructChargeEffect();
    int GetFirstPersonIndex();
    int GetJoystickIndex();
    void KillDelay(float fDelay);
    void FreezeMovement();

protected:

    void Move(float fDeltaTime, float fMove, float fStrafe, float fTurn);

public:

    void Move(const PblMatrix& matrix);

    bool Smooth(float fDeltaTime) const;
    static PblHash rttiHashEntityDroid;
    static MemoryPool sMemoryPool;
    static char rttiNameEntityDroid[];

    EntityDroidClass* mClass;
    PblVector3 mGroundNormal;
    PblVector3 mVelocity;
    float mPitchValue;
    float mGroundRatio;
    Aimer* mAimer[4];
    Weapon* mWeapon[4];
    int mWeaponIndex[2];
    GameSoundControllable mAmbientSound;
    GameSoundControllable mAmbient2Sound;
    char mEnergyBar[12];
    State mState;
    float mKillTimer;
    PblHandle<FLEffectObject> mDestructChargeEffectHandle;
    PostCollision mPostCollision;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
