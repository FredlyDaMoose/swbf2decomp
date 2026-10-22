#pragma once

#include <stddef.h>

#include "Aimer.h"
#include "Battlefront2Lod.h"
#include "CollisionModel.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "GameObject.h"
#include "GameSound.h"
#include "PlayableEntitySounds.h"
#include "Trackable.h"
#include "VehicleEngine.h"
#include "Weapon.h"
#include "PebbleFL/Common/PblAngle.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "Zephyr/Source/ZephyrSkeleton.h"

class MemoryPool;
class Aimer;
class Entity;
class GameObject;
class PlayableEntitySounds;
class ExplosionClass;
class FLEffectClass;
class GameModel;
class RedAnimation;
class ZephyrAnim;

// VTABLE: SWBF2 0x0079aeb4
class EntityDroidekaClass : public GameObjectClass,
                            public ControllableClass,
                            public VehicleEngineClass,
                            public PlayableEntitySoundsClass
{
public:

    static PblHash rttiHashEntityDroidekaClass;
    static char rttiNameEntityDroidekaClass[];

    // FUNCTION: SWBF2 0x004294b0
    static unsigned int GetRtti() { return rttiHashEntityDroidekaClass; }

    EntityDroidekaClass(unsigned int uHash);

    EntityDroidekaClass(EntityDroidekaClass* pSrc, unsigned int uHash);
    virtual ~EntityDroidekaClass();

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
    virtual bool InitAnimations(const char* pszBank);

protected:

    void InitAnimation(int iIndex, const char* pszName);

public:

    RedLodData mLodData;
    GameModel* mUprightLowRes;
    GameModel* mBallLowRes;
    RedAnimation* mAnimObj;
    int mAnimationCount;
    ZephyrAnim** mAnimations;
    float* mTimeTotals;
    float* mAnimSpeeds;
    float* mAnimRates;
    float mAcceleration;
    float mMaxSpeed;
    float mMaxTurnSpeed;
    float mMaxYawSpeed;
    float mMaxPitchSpeed;
    float mUnrollTime;
    float mRollTime;
    float mMouseMaxTurnSpeed;
    float mMouseMaxYawSpeed;
    float mMouseMaxPitchSpeed;
    float mMouseTurnDecay;
    float mMinYaw;
    float mMaxYaw;
    float mMinPitch;
    float mMaxPitch;
    float mCollisionThreshold[3];
    float mCollisionScale[3];
    float mCollisionInflict;
    AimerClass mAimerClass[4];
    WeaponClass* mWeaponClass[4];
    WeaponAmmoDesc mWeaponAmmo[4];
    int mWeaponChannel[4];
    int mWeaponCount;
    unsigned int mStompEffect;
    int mVehicleType;
    float mNoiseFactor;
    float mAnimRateMove;
    float mAnimRateTurn;
    float mTurnThreshold;
    float mMaxTerrainAngle;
    CameraTrackSetting mCameraSettings[3];
    ExplosionClass* mExplosion;
    int mNoDeathExplosions;
    unsigned int mDeathDustClassHash;
    FLEffectClass* mDeathDustEffect;
    float mDeathDustDelay;
    PblVector3 mDeathDustOffset;
    float mDeathShakeDelay;
    float mDeathShakeForce;
    float mDeathShakeDuration;
    float mDeathShakeRadius;
    bool mRenderPilotFlag;
    unsigned char mNumFeet;
    int mFootPrimitives[3];
    GameSound mFootstepSounds[3];
    GameSound mHydraulicSound;
    float mHydraulicSoundHeight;
    unsigned int mAttachHash;
    PblVector3 mAttachOffset;
    float mWaterDamageInterval;
    float mWaterDamageAmount;
    float mBallRadius;
    float mBallMoveSpeed;
    float mBallStoppedTurnSpeed;
    float mBallTurnSpeed;
    float mBallSprintTurnSpeed;
    float mBallSlippage;
    float mBallMaxLean;
    float mBallAcceleration;
    float mBallRollingFriction;
    float mMaxBallAngle;
    float mSprintTimeForBowling;
    float mBowlingPush;
    float mBallSprintSpeedFactor;
    float mBallJumpVerticalSpeed;
    float mBallJumpForwardSpeedFactor;
    EnergyBarClass mEnergyBarClass;
    float mEnergyRestore;
    float mEnergyRestoreIdle;
    float mEnergyDrainSprint;
    float mEnergyCostJump;
    float mEnergyCostBowling;
    int mSpineIdx;
    int mJointIdx[3];
    int mArmJointIdx[2];
    CollisionModel mCollisionBall;
    unsigned int mWakeEffect;
    GameSound mSoundShield;
    GameSound mSoundShieldWearOff;
    GameSound mSoundShieldOff;
    GameSound mSoundJump;
    GameSound mSoundLand;
    int mFirstPersonModelHandle;
    float mUprightWaterDamageHeight;
    float mBallWaterDamageHeight;
    unsigned int mHealthGainEffect;
    unsigned int mAmmoGainEffect;
    unsigned int mEnergyGainEffect;
    unsigned int mBuffHealthEffect;
    unsigned int mBuffOffenseEffect;
    unsigned int mBuffDefenseEffect;
    unsigned int mDebuffEffect;
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

 
 
 

class CameraTrackSetting;
class CollisionResult;
class FoleyFX;
class EnergyBar;
class FLEffectObject;
class NetObjState;
class NetPktGroup;
class Restrictor;

class EntityDroideka : public GameObject,
                       public Controllable,
                       public PlayableEntitySounds,
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
        IdleToRoll = 11,
        Rolling = 12,
        RollToIdle = 13,
        StateCount = 14
    };

    enum eDirection
    {
        None = 0,
        Forward = 1,
        Backward = 2
    };

    struct StateTableEntry
    {
        eState next[8];
    };

    // VTABLE: SWBF2 0x0079ae28
    class PostCollision : public Thread
    {
    public:

        PostCollision(EntityDroideka* pOwner) : mOwner(pOwner) {}
        virtual ~PostCollision();
        virtual bool Update(float fDeltaTime);

        EntityDroideka* mOwner;
    };

    EntityDroideka(EntityDroidekaClass* pClass, const EntityDesc& desc);

    virtual ~EntityDroideka();

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
    virtual float GetMaxSpeed() const;
    virtual float GetMaxStrafe() const;
    virtual const Controllable* GetControllable() const;
    virtual Controllable* GetControllable();
    virtual const PlayableEntitySounds* GetPlayableEntitySounds() const;
    virtual PlayableEntitySounds* GetPlayableEntitySounds();
    virtual const PblVector3& GetAttachOffset() const;
    virtual void ActivatePhysics();
    virtual void DeactivatePhysics();
    virtual GameObjectType GetGameObjectType();
    virtual EntityType GetEntityType() const;
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual void AddSends(void (*pfnSend)(GameObject*, int), int iArg);
    virtual const PblMatrix& GetSmoothedMatrix() const;
    virtual void WriteLateState(NetPktGroup& group);
    virtual void ReadLateState(NetPktGroup& group);

    virtual bool PostCollisionUpdate(float fDeltaTime);

protected:

    virtual void UpdatePilot();
    virtual void UpdatePlayer(float fDeltaTime);
    virtual void UpdateState(float f0, float f1, float f2, float f3);
    virtual void UpdateStateRolling(float f0, float f1, float f2);
    virtual void NextState(int iState);

public:

    virtual void SetPosition(const PblVector3& vPos);
    virtual void SetMatrix(const PblMatrix& mMatrix);
    virtual float GetSmallestCollisionDim() const;
    virtual bool CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor);
    virtual void SetTerrainHeight(int iArg, float fHeight);
    virtual bool CollideTrisWithAllPrimitives() const;

    virtual const PblMatrix& GetMatrix() const;
    virtual const RedPose* GetPose() const;

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    virtual void Kill();
    virtual bool ApplyDamage(const DamageDesc& damage, const PblVector3& vDir,
                             const PblVector3& vPos, int iBodyPart, unsigned int uFlags);
    virtual float GetDamageMultiplier(int iBodyId) const;
    virtual void HealthChangeCallback(bool bArg, float fArg);

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
    virtual float GetOffenseMultiplier() const;
    virtual bool UpdateIndirect(float fDeltaTime);

    virtual void SetupCameraTrackMatrixHUDTargeting(PblMatrix& rMatrix);
    virtual void SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime);
    virtual void SetupCameraZoomFactor(float& rZoom);
    virtual void SetupCameraFOV(float& rFOV);
    virtual void SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix);
    virtual CameraTrackSetting* GetCameraTrackSettings();
    virtual bool RenderTrackable(unsigned int uView) const;
    virtual bool IsFirstPersonAvailablePS2() const;
    virtual bool IsForcedThirdPerson() const;

protected:

    virtual bool Jump();
    virtual bool Sprint();
    virtual bool EndSprint();

    bool CanUnroll();

    void AdjustRotation(float fAngle);
    void BuildWorldMatrices();
    void SetState(int iState);

    void Die();
    float GetTurnState();

    float GetStateLimit();
    void SetFrameState(float fState);
    void ResetTurnState();

public:

    void DoFootImpactEffects();
    void UpdateBuffTimers(float fDeltaTime);

    void TurnOffEffects();
    FoleyFX* GetFoleyFX() const;

    bool IsRollingOrTransition() const;

    bool Smooth(float fDeltaTime) const;

    void Move(const PblMatrix& matrix);

    int GetJoystickIndex();

    void TriggerBuffHealthEffect();
    void TriggerBuffOffenseEffect();
    void TriggerBuffDefenseEffect();
    void TriggerDebuffEffect();

    void BuffOffense(float fTimer, float fMult);
    void BuffDefense(float fTimer, float fMult);
    void BuffHealth(float fTimer, float fRate);
    void DebuffDamage(float fTimer, float fRate, const DamageOwner& owner);
    void TriggerAmmoGainEffect();

    void TriggerHealthGainEffect();
    void TriggerEnergyGainEffect();

    void MoveForward(float fDistance);
    void RotateY(const PblAngle& rAngle);
    float UpdateMouseDecay(float fDeltaTime, float fInput);
    void UpdateWalkingGroundNormal();
    int GetFirstPersonIndex();

    static PblHash rttiHashEntityDroideka;
    static MemoryPool sMemoryPool;
    static char rttiNameEntityDroideka[];
    static StateTableEntry sStateTable[];

    bool mCallUpdateWalkingGroundNormal;
    PblVector3 mVelocity;
    RedAnimation* mAnimObj;
    EntityDroidekaClass* mClass;
    GameModel* mUprightLowRes;
    GameModel* mBallLowRes;
    float mPitchValue;
    Aimer* mAimer[4];
    Weapon* mWeapon[4];
    int mWeaponIndex[2];
    ZephyrSkeleton<32> mZephyrSkeleton;
    ZephyrPoseDyn<32> mZephyrPoseDyn;
    float mTime;
    RedPose mPose;
    float mLastFootHeight[3];
    unsigned int mFootState;
    float mDeathExplosionTime;
    float mDeathTime;
    int mState;
    eDirection mDirection;
    PblAngle mTurnOffset;
    PblAngle mPitchOffset;
    PblVector3 mPivotOffset;
    PblVector3 mAILastCollisionNormal;
    char mAILastCollisionDirection : 2;
    bool mFootprintLeft : 1;
    bool mIsDead : 1;
    bool mDoneDustCloud : 1;
    bool m_bSprint : 1;
    bool m_bIsGrounded : 1;
    PostCollision mPostCollision;
    float mWaterDamageTimer;
    float mSpeedPrev;
    PblAngle mBallOrientation;
    float mBallAngularVelocity;
    PblAngle mBallLean;
    PblVector3 mTranslationOffset;
    PblVector3 mGroundNormal;
    float mStompTime;
    float mFallTimer;

    unsigned char mEnergyBar[12];
    PblHandle<FLEffectObject> mWakeEmitter;
    float mTurnState;
    float mBuffOffenseTimer;
    float mBuffOffenseMult;
    float mBuffDefenseTimer;
    float mBuffDefenseMult;
    float mBuffHealthTimer;
    float mBuffHealthRate;
    float mDebuffDamageTimer;
    float mDebuffDamageRate;
    DamageOwner mDebuffDamageOwner;
    unsigned char mCanChangeClass : 1;
    unsigned char mDoNotPlayCollisionEffect : 4;
    PblHandle<FLEffectObject> mHealthGainEffectHandle;
    PblHandle<FLEffectObject> mAmmoGainEffectHandle;
    PblHandle<FLEffectObject> mEnergyGainEffectHandle;
    PblHandle<FLEffectObject> mBuffHealthEffectHandle;
    PblHandle<FLEffectObject> mBuffOffenseEffectHandle;
    PblHandle<FLEffectObject> mBuffDefenseEffectHandle;
    PblHandle<FLEffectObject> mDebuffEffectHandle;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

 
 
 
 
 
 
 
 
 
 
 
