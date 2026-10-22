#pragma once

#include <stddef.h>
#include "WalkerAnimation.h"

#include "Aimer.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "GameObject.h"
#include "GameSound.h"
#include "Trackable.h"
#include "VehicleEngine.h"
#include "VehicleMusic.h"
#include "Weapon.h"

class MemoryPool;
class ExplosionClass;
class FLEffectClass;
class MountedTurretClass;
class PassengerSlotClass;
class RedAnimation;
struct WalkerLegPairInfo;
class ZephyrSkeletonShared;

#include "PebbleFL/Common/PblAngle.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"
#include "Zephyr/Source/ZephyrSkeleton.h"

class CameraTrackSetting;
class CollisionResult;
class EnergyBar;
class Entity;
class FoleyFX;
class MountedTurret;
class NetObjState;
class NetPktGroup;
class PassengerSlot;
class RedStencilShadow;
class Restrictor;
class WalkerLegPair;
class Weapon;
class DamageDesc;
class EntityWalkerClass;
class PblAngle;

// VTABLE: SWBF2 0x0079db50
// VTABLE: SWBF2 0x0079ddf4 CollisionObject
// VTABLE: SWBF2 0x0079dae8 RedSceneObject
// VTABLE: SWBF2 0x0079dce4 Damageable
// VTABLE: SWBF2 0x0079dd1c PblHandled
// VTABLE: SWBF2 0x0079dd24 Controllable
// VTABLE: SWBF2 0x0079daa0 Trackable
class EntityWalker : public GameObject,
                     public Controllable,
                     public VehicleMusic,
                     public VehicleEngine
{
public:

    enum eState
    {
        Idle = 0,
        TurnLeft = 1,
        TurnRight = 2,
        Dying = 3,
        Dead = 4,
        IdleToLeft = 5,
        IdleToRight = 6,
        LeftToRight = 7,
        RightToLeft = 8,
        LeftToIdle = 9,
        RightToIdle = 10,
        StateCount = 11
    };

    enum eDirection
    {
        None = 0,
        Forward = 1,
        Backward = 2
    };

    enum eTransition
    {
        ToIdle = 0,
        ToIdleBackwards = 1,
        ToForward = 2,
        ToBackward = 3,
        ToLeft = 4,
        ToRight = 5,
        ToDying = 6,
        TransitionCount = 7
    };

    struct StateTableEntry
    {
        eState next[TransitionCount];
    };

    // VTABLE: SWBF2 0x0079da88
    class PostCollision : public Thread
    {
    public:

        PostCollision(EntityWalker* pOwner) : mOwner(pOwner) {}
        virtual ~PostCollision();
        virtual bool Update(float fDeltaTime);

        EntityWalker* mOwner;
    };

    EntityWalker(EntityWalkerClass* pClass, const EntityDesc& desc);
    virtual ~EntityWalker();

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
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
    virtual float GetMaxSpeed() const;
    virtual float GetMaxStrafe() const;
    virtual const Controllable* GetControllable() const;
    virtual Controllable* GetControllable();
    virtual bool ShouldShowReticule() const;
    virtual int GetReservableSlots(ReserveType eType) const;
    virtual const PblVector3& GetAttachOffset() const;
    virtual void ActivatePhysics();
    virtual void DeactivatePhysics();
    virtual GameObjectType GetGameObjectType();
    virtual void ReserveForPlayer(float fTime);
    virtual bool IsReservedForPlayer() const;
    virtual EntityType GetEntityType() const;
    virtual ExplosionClass* GetExplosionClass();
    virtual float GetEjectTimer() const;
    virtual void SetEjectTimer(float fTime);
    virtual void ResetTimeSinceLastHack();
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual void AddSends(void (*pfnSend)(GameObject*, int), int iArg);
    virtual const PblMatrix& GetSmoothedMatrix() const;
    virtual void WriteLateState(NetPktGroup& group);
    virtual void ReadLateState(NetPktGroup& group);
    virtual bool PostCollisionUpdate(float fDeltaTime);

protected:
    virtual void UpdatePilot();
    virtual void UpdatePlayer();
    virtual void UpdateState(float fDeltaTime, float fMove, float fStrafe, float& rTurnSpeed,
                             float fTurn);
    virtual void ApplyBlend();
    virtual void CorrectJoints();
    virtual void BuildWorldMatrices();
    virtual void RestoreJoints();
    virtual void UpdateDone();

public:

    virtual void SetPosition(const PblVector3& vPosition);
    virtual void SetMatrix(const PblMatrix& mMatrix);
    virtual bool CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor);
    virtual void SetTerrainHeight(int iIndex, float fHeight);

    virtual const PblMatrix& GetMatrix() const;
    virtual const RedPose* GetPose() const;

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    virtual void Kill();
    virtual bool ApplyDamage(const DamageDesc& damage, const PblVector3& vDir,
                             const PblVector3& vPos, int iBodyPart,
                             unsigned int uFlags);

    virtual bool Update(float fDeltaTime);
    virtual Trackable* GetTrackable();
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
    virtual void SetupController();
    virtual PilotDeathType GetPilotDeathType();

protected:
    virtual bool Jump();

public:
    virtual bool UpdateIndirect(float fDeltaTime);

    virtual void SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime);
    virtual void SetupCameraZoomFactor(float& rZoom);
    virtual void SetupCameraFOV(float& rFOV);
    virtual void SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix);
    virtual CameraTrackSetting* GetCameraTrackSettings();
    virtual bool RenderTrackable(unsigned int uView) const;

protected:
    virtual bool IsForcedThirdPerson() const;

public:

    void RotateY(const PblAngle& rAngle);
    void DoFootImpactEffects();
    int GetFirstPersonIndex();
    int GetJoystickIndex();
    void MoveForward(float fDistance);

    int IsTowCableCollisionValid(int iBodyId, int iLastBodyId, int iPrevBodyId);

    const PblVector3& GetTowCableCollisionOffset(int iBodyID);

protected:
    FoleyFX* GetFoleyFX() const;
    void Die();
    void NextState(int iInput);
    void UpdateOrientation(float fDeltaTime);

    void SetState(int iState);
    void ResetRotation();
    void AdjustRotation(PblAngle angle);
    void CheckTurn(float fMove, float fStrafe);

    void ApplyAttackDamage(const PblVector3& vPosition, float fRadius, float fDamage,
                           float fPush) const;

public:

    bool Smooth(float fDeltaTime) const;

    void Move(const PblMatrix& matrix);

public:
    static PblHash rttiHashEntityWalker;
    static MemoryPool sMemoryPool;
    static char rttiNameEntityWalker[];
    static StateTableEntry sStateTable[];

    EntityWalkerClass* mClass;
    RedAnimation* mAnimObj;
    PblVector3 mVelocity;
    PassengerSlot* mPassengerSlots[4];
    MountedTurret* mTurret[8];
    int mNumTurrets;
    RedStencilShadow* mStencilShadow;
    float mLegAngle;
    float mYawValue;
    float mPitchValue;
    float mBodyPitch;
    float mBodyRoll;
    float mStopTime;
    Aimer* mAimer[4];
    Weapon* mWeapon[4];
    int mWeaponIndex[2];
    ZephyrSkeleton<64> mZephyrSkeleton;
    ZephyrPoseStatic<64> mZephyrPoseStatic;
    WalkerBlend* mBlend;
    PblBitVector<64> mMainMask;
    RedPose mPose;
    float mTotalTurn;
    float mReserveForPlayer;
    unsigned int mFootprintLeft : 1;
    unsigned int mIsDead : 1;
    unsigned int mIsStopped : 1;
    unsigned int mExplodedDeath : 1;
    unsigned int mExplodedFinal : 1;
    unsigned int mDoneDustCloud : 1;
    unsigned int m_bIsGrounded : 1;
    unsigned int m_bIsJumping : 1;
    unsigned int m_uiAttackControls : 2;
    unsigned int m_uiAttackAnim : 3;
    unsigned int z_uiReserved0 : 18;
    int mLegPairCount;
    WalkerLegPair* mLegPairs[3];
    float mLastFootHeight[6];
    float mMinFootHeight[6];
    unsigned int mFootState;
    unsigned int mFootWaterState;
    float mDeathExplosionTime;
    float mDeathTime;
    float m_fGroundedTimer;
    int mState;
    PblVector3 mPrevAimerPos;
    eDirection mDirection;
    char mEnergyBar[12];
    PblAngle mTurnOffset;
    PblAngle mPitchOffset;
    PblAngle mRollOffset;
    PblVector3 mPivotOffset;
    PblVector3 mVelocityNoGrav;
    PostCollision mPostCollision;
    float mWaterDamageTimer;
    float mSpeedPrev;
    bool mBoost : 1;
    float mEjectTimer;
    float mTimeSinceLastHack;
    float mAttackControlTimer;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x0079dc94
class EntityWalkerClass : public GameObjectClass,
                          public ControllableClass,
                          public VehicleMusicClass,
                          public VehicleEngineClass
{
public:

    struct WalkerAttackAnim
    {
        unsigned int uiAnimName;
        unsigned int uiControls;
        PblVector3 vPosition;
        float fTime;
        float fRadius;
        float fDamage;
        float fPush;
    };

    EntityWalkerClass(unsigned int uHash);

    EntityWalkerClass(EntityWalkerClass* pBase, unsigned int uHash);

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void PostReadSetup();
    virtual int GetVehicleType();
    virtual const void* GetControllableClass();
    virtual int GetWeaponCount();
    virtual const WeaponClass* GetWeaponClass(int iIndex);
    virtual float GetCollisionInflict();
    virtual float GetTimeRequiredToEject();
    virtual float GetReboardTime();
    virtual bool InitAnimations(const char* pszName);

    PblBitVector<64> HierarchyJointMask(const ZephyrSkeletonShared* pSkeleton, int iJoint);

protected:

    friend class EntityWalker;
    float GetAnimationBlendDuration(EntityWalker::eState eFrom, EntityWalker::eState eTo) const;

    int IsTowCableCollisionValid(int iBodyId, int iLastBodyId, int iPrevBodyId);
    const PblVector3& GetTowCableCollisionOffset(int iBodyID);

public:

    static PblHash rttiHashEntityWalkerClass;
    static char rttiNameEntityWalkerClass[];

    RedAnimation* mAnimObj[3];
    int mNumAnimObjs;
    ZephyrSkeletonShared* m_pZephyrSkeletonShared;
    float mSkeletonRootScale;
    int mAnimationCount;
    WalkerAnimation** mAnimations;
    bool m_bAnimationBlending;
    unsigned char m_uiNumAttackAnims;
    WalkerAttackAnim m_aAttackAnims[4];
    int mVehicleType;
    float mAcceleration;
    float mMaxSpeed;
    float mBoostSpeed;
    float mJumpVerticalSpeed;
    float mJumpForwardSpeedBoost;
    float mJumpMinSpeed;
    float mMaxTurnSpeed;
    float mMaxYawSpeed;
    float mMinYaw;
    float mMaxYaw;
    float mMaxPitchSpeed;
    float mMinPitch;
    float mMaxPitch;
    float mStoppedTurnSpeed;
    float mForwardTurnSpeed;
    float mBoostTurnSpeed;
    float mTurnThreshold;
    float mMaxTerrainAngle;
    bool mOrientRoll;
    float mWidth;
    EnergyBarClass mEnergyBarClass;
    float mEnergyAutoRestore;
    float mEnergyBoostDrain;
    float mEnergyCostJump;
    float mCollisionThreshold[3];
    float mCollisionScale[3];
    float mCollisionInflict;
    AimerClass mAimerClass[4];
    WeaponClass* mWeaponClass[4];
    WeaponAmmoDesc mWeaponAmmo[4];
    int mWeaponChannel[4];
    int mWeaponCount;
    float mStompRumbleLight;
    float mStompRumbleHeavy;
    float mStompRumbleLightDuration;
    float mStompRumbleHeavyDuration;
    unsigned int mStompEffect;
    unsigned int mStompDecal;
    unsigned int mFootWaterSplashEffect;
    float mStompDecalSize;
    float mNoiseFactor;
    CameraTrackSetting mCameraSettings;
    PassengerSlotClass* mPassengerSlotClass;
    unsigned char mNumPassengerSlots;
    MountedTurretClass* mTurretClass[8];
    ExplosionClass* mExplosion;
    PblVector3 mExplosionOffset;
    ExplosionClass* mFinalExplosion;
    PblVector3 mFinalExplosionOffset;
    int mDeathAnimationExplosionCount;
    unsigned int mDeathAnimationExplosions[4];
    float mDeathAnimationExplosionTime;
    unsigned int mDeathDustClassHash;
    FLEffectClass* mDeathDustEffect;
    float mDeathDustDelay;
    PblVector3 mDeathDustOffset;
    float mDeathShakeDelay;
    float mDeathShakeForce;
    float mDeathShakeDuration;
    float mDeathShakeRadius;
    float mDeathFadeTimeStart;
    float mDeathFadeTimeEnd;
    float m_fThrustAngleToStrafe_Tan;
    float m_fThrustAngleToTurn_Tan;
    float m_fThrustAngleToStop_Tan;
    float m_fSteerTowardAimRate;
    bool mAIUseHoverPhysics : 1;
    bool mSteerAtYawLimit : 1;
    bool mSteerAtStrafeLimit : 1;
    bool mCorrectAimForSteering : 1;
    bool mReverseBackwardSteering : 1;
    bool mRenderPilotFlag : 1;
    bool mSmashParkedFlyers : 1;
    unsigned char mStompDetectionType;
    unsigned char mNumFeet;
    int mFootPrimitives[6];
    float mStompThreshold;
    float mLegRayHitLength;
    int mLegPairCount;
    WalkerLegPairInfo* mLegPairInfo;
    unsigned int mAttachHash;
    PblVector3 mAttachOffset;
    float mWaterDamageInterval;
    float mWaterDamageAmount;
    GameSound mHydraulicSound;
    float mHydraulicSoundHeight;
    GameSound mHydraulicLowerSound;

    float mHydrualicLowerHeight;
    GameSound mFootstepSounds[6];
    float mFPWalkScale;
    unsigned int mFPPoseHash;
    int mTowCableCollisionBodyID[12];
    PblVector3 mTowCableCollisionOffset[12];
    unsigned char mNumTowCableCollisions;
    int mFirstPersonModelHandle;
    int mIgnorableCollisions;
    float mDropShadowSize;
    float mTimeRequiredToEject;
    float mEjectResist;
    float mTimeTilReboard;
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

 
 
 
