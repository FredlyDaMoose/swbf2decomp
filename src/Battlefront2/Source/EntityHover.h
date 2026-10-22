#pragma once

#include <stddef.h>

#include "Aimer.h"
#include "CollisionModel.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "GameObject.h"
#include "GameSound.h"
#include "Trackable.h"
#include "VehicleEngine.h"
#include "VehicleMusic.h"
#include "Weapon.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "Zephyr/Source/ZephyrSkeleton.h"

class MemoryPool;
class ExplosionClass;
class MountedTurretClass;
class PassengerSlotClass;
class RedAnimation;
class ZephyrAnim;

// VTABLE: SWBF2 0x0079ba80
class EntityHoverClass : public GameObjectClass,
                         public ControllableClass,
                         public VehicleMusicClass,
                         public VehicleEngineClass
{
public:

    struct ColliderBodyInfo
    {
        unsigned int mCollisionBodyId;
        float mSpringLength;
        float mVelocitySpringFactor;
        float mVelocityDampFactor;
        float mOmegaXSpringFactor;
        float mOmegaXDampFactor;
        float mOmegaZSpringFactor;
        float mOmegaZDampFactor;
    };

    EntityHoverClass(unsigned int uHash);

    EntityHoverClass(EntityHoverClass* pBase, unsigned int uHash);

    void SetupDefaultColliderBodyFactors(ColliderBodyInfo& info, int iBodyId);
    bool InitAnimations(const char* pszName);

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual int GetVehicleType();
    virtual const void* GetControllableClass();
    virtual int GetWeaponCount();
    virtual const WeaponClass* GetWeaponClass(int iIndex);
    virtual float GetTimeRequiredToEject();
    virtual float GetReboardTime();

    static PblHash rttiHashEntityHoverClass;
    static char rttiNameEntityHoverClass[];

    float mSetAltitude;
    float mGravityScale;
    float mLiftSpring;
    float mLiftDamp;
    float mAcceleration;
    float mDeceleration;
    float mTraction;
    float mForwardSpeed;
    float mReverseSpeed;
    float mStrafeSpeed;
    float mSpinRate;
    float mTurnRate;
    float mYawRate;
    float mPitchRate;
    float mTurnFilter;
    float mYawMin;
    float mYawMax;
    float mPitchMin;
    float mPitchMax;
    float mThrustPitchAngle;
    float mStrafeRollAngle;
    float mBankAngle;
    float mBankFilter;
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
    int mVehicleType;
    RedAnimation* mAnimObj;
    ZephyrAnim* mAnimFins;
    float mNoiseFactor;
    float mBarrierX;
    float mBarrierZ;
    float mBarrierOX;
    float mBarrierOZ;
    CameraTrackSetting mCameraSettings;
    PassengerSlotClass* mPassengerSlotClass;
    unsigned char mNumPassengerSlots;
    MountedTurretClass* mTurretClass[8];
    ExplosionClass* mExplosion;
    unsigned int mAttachHash;
    PblVector3 mAttachOffset;
    unsigned int mWakeEffect;
    unsigned int mWaterEffect;
    float mBlurEffect;
    float mBlurStartV;
    int mWakeEmitterPrimitives[6];
    unsigned char mNumWakeEmitters;
    GameSound mSoundCloseToGround;
    float mSoundCloseToGroundHeight;
    CollisionModel mDefaultPilotCollision;
    float mMaxSuspensionOffset;
    float mMidSuspensionOffset;
    unsigned int mSuspensionCenterHash;
    unsigned int mSuspensionLeftArmHash;
    unsigned int mSuspensionRightArmHash;
    PblVector3 mBaseSuspensionPos;
    PblVector3 mLeftSuspensionPos;
    PblVector3 mRightSuspensionPos;
    int mFirstPersonModelHandle;
    bool mFPModelAttachedtoCam;
    bool mMovingTurnOnly;
    bool mFloatsOnWater;
    float mVelocitySpring;
    float mVelocityDamp;
    float mOmegaXSpring;
    float mOmegaXDamp;
    float mOmegaZSpring;
    float mOmegaZDamp;
    bool mRandomSpring;
    ColliderBodyInfo mColliderBodyInfo[6];
    int mNumColliderBodies;
    EnergyBarClass mEnergyBarClass;
    float mEnergyAutoRestore;
    float mEnergyBoostDrain;
    float mBoostSpeed;
    float mBoostAcceleration;
    float mBoostFOV;
    float mBoostTurnMultiplier;
    float mTimeRequiredToEject;
    float mEjectResist;
    float mTimeTilReboard;
    float mJumpTimeMin;
    float mJumpTimeMax;
    float mJumpForce;
    float mJumpMinSpeedMult;
    float mJumpEnergyPerSec;
    int mWheelCount;
    unsigned int mWheelTextureId[4];
    float mWheelVelocToU[4];
    float mWheelVelocToV[4];
    float mWheelOmegaToU[4];
    float mWheelOmegaToV[4];
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

 
 
 

class Aimer;
class CameraTrackSetting;
class CollisionResult;
class EnergyBar;
class Entity;
class FLEffectObject;
class GameObject;
class MountedTurret;
class NetObjState;
class NetPktGroup;
class Obstacle;
class PassengerSlot;
class RedPose;
class Restrictor;
class Weapon;

// VTABLE: SWBF2 0x0079b960
// VTABLE: SWBF2 0x0079bc98 CollisionObject
// VTABLE: SWBF2 0x0079bbec RedSceneObject
// VTABLE: SWBF2 0x0079bc54 Damageable
// VTABLE: SWBF2 0x0079bacc PblHandled
// VTABLE: SWBF2 0x0079bad4 Controllable
// VTABLE: SWBF2 0x0079bba4 Trackable
class EntityHover : public GameObject,
                    public Controllable,
                    public VehicleMusic,
                    public VehicleEngine
{
public:

    // VTABLE: SWBF2 0x0079bc80
    class PostCollision : public Thread
    {
    public:

        PostCollision(EntityHover* pOwner) : mOwner(pOwner) {}
        virtual ~PostCollision();
        virtual bool Update(float fDeltaTime);

        EntityHover* mOwner;
    };

    struct ColliderBody
    {
        ColliderBody() {}

        float mOverlap;
        float mOldOverlap;
        PblVector3 mNormal;
    };

    EntityHover(EntityHoverClass* pClass, const EntityDesc& desc);

    virtual ~EntityHover();

    static void* operator new(unsigned int uSize);
    static void operator delete(void* pBlock);

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
    virtual float GetMaxSpeed() const;
    virtual float GetMaxStrafe() const;
    virtual const Controllable* GetControllable() const;
    virtual Controllable* GetControllable();
    virtual int GetReservableSlots(ReserveType eType) const;
    virtual const PblVector3& GetAttachOffset() const;
    virtual void ActivatePhysics();
    virtual void DeactivatePhysics();
    virtual GameObjectType GetGameObjectType();
    virtual void ReserveForPlayer(float fSeconds);
    virtual bool IsReservedForPlayer() const;
    virtual EntityType GetEntityType() const;
    virtual ExplosionClass* GetExplosionClass();
    virtual float GetEjectTimer() const;
    virtual void SetEjectTimer(float fSeconds);
    virtual void ResetTimeSinceLastHack();
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual void AddSends(void (*pfnSend)(GameObject*, int), int iArg);
    virtual const PblMatrix& GetSmoothedMatrix() const;
    virtual void WriteLateState(NetPktGroup& group);
    virtual void ReadLateState(NetPktGroup& group);

    virtual void SetPosition(const PblVector3& vPosition);
    virtual void SetMatrix(const PblMatrix& mMatrix);
    virtual bool CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor);

    virtual const PblMatrix& GetMatrix() const;
    virtual const RedPose* GetPose() const;

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    virtual void Kill();
    virtual bool ApplyDamage(const DamageDesc& damage, const PblVector3& vDir,
                             const PblVector3& vPos, int iBodyPart,
                             unsigned int uFlags);

public:
    virtual bool Update(float fDeltaTime);
    virtual Trackable* GetTrackable();
    virtual CollisionModel* GetDefaultPilotCollision();
    virtual float GetSoundRadius() const;
    virtual float GetAimTurnRate() const;
    virtual int GetWeaponCount() const;
    virtual int GetWeaponIndex(int iChannel) const;
    virtual Weapon* GetWeaponPtr(int iIndex) const;
    virtual EnergyBar* GetEnergyBar();
    virtual void SetWeaponIndex(int iChannel, int iIndex);
    virtual const Aimer* GetAimer(int iIndex) const;
    virtual ControllableClass* GetControllableClass();
    virtual const ControllableClass* GetControllableClass() const;
protected:
    virtual void SetupController();

public:
    virtual bool UpdateIndirect(float fDeltaTime);

    virtual void SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime);
    virtual void SetupCameraZoomFactor(float& rZoom);
    virtual void SetupCameraFOV(float& rFOV);
    virtual void SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix);
    virtual CameraTrackSetting* GetCameraTrackSettings();
    virtual bool RenderTrackable(unsigned int uView) const;
    virtual bool IsForcedThirdPerson() const;

    bool PostCollisionUpdate(float fDeltaTime);

    bool Smooth(float fDeltaTime) const;

    void Move(const PblMatrix& matrix);

    int GetFirstPersonIndex();
    int GetJoystickIndex();

protected:

    void Move(float fDeltaTime, float fThrust, float fStrafe, float fTurn, bool& rJump);

    int UpdateCollisionOnColliderBody(unsigned int uBodyId, float fOverlap, const PblVector3& vNormal);
    void UpdateColliderBody(int iBody, float fDeltaTime);
    void ApplyCollisionImpact(float fApproach, const PblVector3& vNormal, CollisionObject* pOther);

public:

    static PblHash rttiHashEntityHover;
    static MemoryPool sMemoryPool;
    static char rttiNameEntityHover[];

    PblVector3 mVelocity;
    PblVector3 mOmega;
    float mGroundRatio;
    float mLastSuspensionOffset;
    float mNextSuspensionOffset;
    float mCurrSuspensionOffset;
    float mSuspensionVelocity;
    EntityHoverClass* mClass;
    PblVector3 mGroundNormal;
    PassengerSlot* mPassengerSlots[4];
    MountedTurret* mTurret[8];
    char mNumTurrets;
    unsigned char mCloseToGround;
    float mBankPitchAngle;
    float mBankRollAngle;
    float mFinPitchValue;
    float mFinRollValue;
    float mYawValue;
    float mPitchValue;
    Aimer* mAimer[4];
    Weapon* mWeapon[4];
    int mWeaponIndex[2];
    int mStuckState;
    PblVector3 mStuckDir;
    float mStuckTimer;
    float mStuckCycle;
    float mWheelTextureU[4];
    float mWheelTextureV[4];
    PblHandle<FLEffectObject> mWakeEmitters[6];
    PblHandle<FLEffectObject> mWaterEmitter;
    PblVector3 mWaterEmitterPos;
    ZephyrSkeleton<32> mZephyrSkeleton;
    ZephyrPoseDyn<32> mZephyrPoseDyn;
    RedPose mPose;
    PblMatrix mProceduralPoseMatrix[3];
    float mForceFrameNum1;
    float mForceFrameNum2;
    float mForceFrameBlend;
    float mSpeedPrev;
    float mGroundRatioPrev;
    float mStrain;
    float mAccel;
    Obstacle* mObstacle;
    PostCollision mPostCollision;
    ColliderBody mColliderBody[6];
    float mGroundPitch;
    float mGroundBank;
    float mGroundPitchVel;
    float mGroundBankVel;
    char mEnergyBar[12];
    float mEjectTimer;
    float mTimeSinceLastHack;
    float mJumpTimer;
    float mReserveForPlayer;
    bool mBoost;
    bool mIsDead : 1;
    bool mJumpMode : 1;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
