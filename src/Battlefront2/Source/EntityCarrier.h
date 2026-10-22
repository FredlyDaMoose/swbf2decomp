#pragma once

#include <stddef.h>

#include "EntityFlyer.h"
#include "GameLoop.h"
#include "GameSound.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "SoundFL/source/sndsoundparameterized.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "Zephyr/Source/ZephyrSkeleton.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class Aimer;
class ControllableDestination;
class EnergyBar;
class MountedTurret;
class Obstacle;
class ParticleEmitterObject;
class PassengerSlot;
class RedPose;
class Weapon;

// VTABLE: SWBF2 0x0079a2b8
class EntityCarrierClass : public EntityFlyerClass
{
public:

    struct CargoInfo
    {
        unsigned int mHash;
        PblVector3 mOffset;
    };

    EntityCarrierClass(unsigned int uHash);
    virtual ~EntityCarrierClass();

    EntityCarrierClass(EntityCarrierClass* pSrc, unsigned int uHash);

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    CargoInfo mCargoInfo[4];
    int mCargoCount;
    GameSound mSoundCargoPickup;
    GameSound mSoundCargoDropoff;

    static PblHash rttiHashEntityCarrierClass;
    static char rttiNameEntityCarrierClass[];
};
  
 
 
 
 
 

class EntityFlyer : public GameObject, public Controllable, public EntityPathFollower,
                    public VehicleMusic, public VehicleEngine
{
public:
    EntityFlyer(EntityFlyerClass* pClass, const EntityDesc& desc);
    virtual ~EntityFlyer();

    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    static void* operator new(unsigned int uSize);
    static void operator delete(void* pBlock);

    virtual void Init();
    virtual GameObject* GetGameObject();

    virtual const GameObject* GetGameObject() const;
    virtual Entity* GetEntity();
    virtual const Entity* GetEntity() const;

    virtual bool CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor);

    virtual float GetSoundRadius() const;
    virtual void ActivatePhysics();
    void InitAsLanded();
    bool ResetSquadronTime();

protected:
    void SendLandedEvent();

public:

    const ControllableClass* GetControllableClass() const;
    ControllableClass* GetControllableClass();
    int GetWeaponCount() const;

    float GetFlyerMaxSpeed() const;
    float GetFlyerMidSpeed() const;
    float GetFlyerMinSpeed() const;
    float GetMaxSpeed() const;
    int GetReservableSlots(ReserveType eType) const;

    // FUNCTION: SWBF2 0x00477710
    const PblVector3& GetVelocity() const
    {
        return mVelocity;
    }

    // FUNCTION: SWBF2 0x004778b0
    void SetVelocity(const PblVector3& v)
    {
        mVelocity = v;
    }

    // FUNCTION: SWBF2 0x004778f0
    const Controllable* GetControllable() const
    {
        return this ? static_cast<const Controllable*>(this) : 0;
    }

    // FUNCTION: SWBF2 0x00477900
    Controllable* GetControllable()
    {
        return this ? static_cast<Controllable*>(this) : 0;
    }

    // FUNCTION: SWBF2 0x00477980
    virtual GameObjectType GetGameObjectType() { return (GameObjectType)(GO_VEHICLE | GO_FLYER); }

    // FUNCTION: SWBF2 0x00496e10
    virtual EntityType GetEntityType() const { return ENT_FLYER; }
    virtual void AddSends(void (*pfnSend)(GameObject*, int), int iArg);

    // FUNCTION: SWBF2 0x00477a10
    void ReserveForPlayer(float fSeconds)
    {
        mReserveForPlayer = GameLoop::GetMissionTime() + fSeconds;
    }

    // FUNCTION: SWBF2 0x00477a30
    bool IsReservedForPlayer() const
    {
        return mReserveForPlayer > GameLoop::GetMissionTime();
    }

    // FUNCTION: SWBF2 0x00477a50
    void SetEjectTimer(float fSeconds)
    {
        if (!mProcedurallyAnimated)
        {
            mAIEng.ai.mEjectTimer = fSeconds;
        }
    }

    // FUNCTION: SWBF2 0x00477a70
    float GetEjectTimer() const
    {
        if (mProcedurallyAnimated)
        {
            return 0.0f;
        }
        return mAIEng.ai.mEjectTimer;
    }

    // FUNCTION: SWBF2 0x00477aa0
    void ResetTimeSinceLastHack()
    {
        if (!mProcedurallyAnimated)
        {
            mAIEng.ai.mTimeSinceLastHack = 0.0f;
        }
    }

    bool IsRtti(unsigned int uRtti) const;

    void InitAsFlying();

    void TakeOff();

    enum State { LANDED, TAKEOFF, FLYING, LANDING, CRASHING, CRASHED };
    enum Trick { FLIP_UP, FLIP_DOWN, FLIP_UP_RIGHT, FLIP_DOWN_RIGHT,
                 FLIP_UP_LEFT, FLIP_DOWN_LEFT, SIDE_ROLL, TRICK_MAX };
    State GetState() const { return mState; }
    const EntityFlyerClass* GetClass() const { return mClass; }

    void SetInLandingRegion(float fFactor)
    {
        mInLandingRegionFlag = true;
        mInLandingRegionFactor = fFactor;
    }

    // FUNCTION: SWBF2 0x00666770
    const PblHandle<GameObject> GetTowCableWalker() const { return mTowCableWalker; }

    void SetTowCableWalker(GameObject* pWalker);

    static PblHash rttiHashEntityFlyer;

    // FUNCTION: SWBF2 0x00438bc0
    static unsigned int GetRtti() { return rttiHashEntityFlyer; }
    static MemoryPool sMemoryPool;

    // VTABLE: SWBF2 0x0079b260
    class PostCollision : public Thread
    {
    public:

        PostCollision(EntityFlyer* pOwner) : mOwner(pOwner) {}

        virtual bool Update(float fDeltaTime);

        EntityFlyer* mOwner;
    };

    // FUNCTION: SWBF2 0x00477910
    int GetWeaponIndex(int iChannel) const
    {
        return mWeaponIndex[iChannel];
    }

    // FUNCTION: SWBF2 0x00477930
    Weapon* GetWeaponPtr(int iIndex) const
    {
        return mWeapon[iIndex];
    }

    // FUNCTION: SWBF2 0x00477950
    EnergyBar* GetEnergyBar()
    {
        return &mEnergyBar;
    }

    // FUNCTION: SWBF2 0x00477960
    const Aimer* GetAimer(int iIndex) const
    {
        return mAimer[iIndex];
    }

    // FUNCTION: SWBF2 0x00477990
    const RedPose* GetPose() const
    {
        return &mPose;
    }

    void SetPosition(const PblVector3& vPos);
    void SetMatrix(const PblMatrix& mMatrix);
    bool GetFollowingPath() const;
    Trackable* GetTrackable();
    unsigned int GetAllegiance();
    bool CanJoinSquadron() const;

    virtual float GetMaxStrafe() const;
    virtual ExplosionClass* GetExplosionClass();
    virtual bool ShouldShowReticule() const;
    virtual void ClientKill();
    virtual void DeactivatePhysics();
    virtual PblVector3 GetTargetPointMissVector() const;
    virtual const PblMatrix& GetMatrix() const;
    virtual bool UpdateIndirect(float fDeltaTime);

    virtual bool Update(float fDeltaTime);
    virtual const EntitySquadronClass* GetEntitySquadronClass() const;
    virtual EntityPathFollower::BranchParams GetBranchParams();
    virtual bool PostCollisionUpdate(float fDeltaTime);
    virtual void Render(unsigned char renderMode, float lod, unsigned int flags);

    virtual const PblMatrix& GetSmoothedMatrix() const;
    bool Smooth(float fDeltaTime) const;

    void Move(const PblMatrix& matrix);

    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual void WriteLateState(NetPktGroup& group);
    virtual void ReadLateState(NetPktGroup& group);

    void DoTrick(Trick eTrick);

    void SideRoll();

    virtual void Kill();
    virtual void HealthChangeCallback(bool bArg, float fDelta);
    virtual bool RenderTrackable(unsigned int uView) const;
    virtual CameraTrackSetting* GetCameraTrackSettings();
    virtual void SetupCameraZoomFactor(float& rZoom);
    virtual void SetupCameraFOV(float& rFOV);

    virtual void SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime);
    virtual void SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix);
    void RecalculateSpeed();
    void TurnOffContrailEffects();
    void UpdatePathFollower(float fDeltaTime, PblMatrix& rMatrix);
    void SideRollStop();
    void SideRollUpdate(float fDeltaTime);
    void RollUpdate(float fDeltaTime);
    void RollStop();

    void RollAdd(float fRoll);

    void FlipAdd(float fFlip);

    void FlipUpdate(float fDeltaTime, PblMatrix& rMatrix);
    void FlipStop();
    int GetFirstPersonIndex();
    int GetJoystickIndex();

    virtual bool ApplyDamage(const DamageDesc& damage, const PblVector3& vDir, const PblVector3& vPos,
                             int iBodyPart, unsigned int uFlags);

    void Land();

protected:
    void PlayDustEffect();
    void TurnOnThrustEffects();

    void TurnOffThrustEffects();
    void TargetReact(Controllable* pAttacker);
    void UpdateContrailEffects();

    bool TurnOnContrailEffect(int iIndex);

    virtual void SetupController();

    bool CalculateDest(float fDeltaTime, ControllableDestination* pDest, PblVector3* pPos,
                       float* pfSpeedScale, PblVector3* pDir);

    void RaiseHeight(const PblVector3& vPos, bool bTaper, PblVector3* pDest, float* pfSpeedScale,
                     float fTaperFactor);
    bool AtSlideDest() const;

    virtual float GetAimTurnRate() const;
    virtual EnergyMode GetEnergyMode() const;
    virtual void SetWeaponIndex(int iWeaponIndex, int iChannel);
    virtual bool IsForcedThirdPerson() const;

public:

    PblHandle<GameObject> mTowCableWalker;
    PblVector3 mVelocity;
    PblVector3 mOmega;
    float mSetSpeed, mSetStrafe, mSetStrafeVertical;
    State mState;
    float mFlightRatio, mBankPitchAngle, mBankPitchAngleDelta;
    float mBankRollAngle, mBankRollAngleDelta, mRoll, mRollTarget;
    float mFlip, mFlipTarget, mTrickCameraInterpDetach;
    float mTrickCameraInterpDir, mTrickCameraDetach;
    PblVector3 mSideRoll;
    Trick mLastTrickPerformed;
    float mPitchControllerPrevUpdate, mTurnControllerPrevUpdate;
    float mReserveForPlayer;
    bool mInRoll : 1, mInFlip : 1, mBoost : 1, mFollowingPath : 1;
    bool mSliding : 1, mInLandingRegionFlag : 1;
    bool mInLandingRegionFlag_ToolTip : 1;
    bool mFlyerAutoCorrectingToStayInBattlefield : 1;
    unsigned char _flightFlagPadding[3];
    float mGetSpeedSpeed, mInLandingRegionFactor, mLandedHeight;
    float mAutoLevelingAllowed, mWorldRoll, mCrashTimer;
    float mTrick, mTrickDoubleclick;
    EnergyBar mEnergyBar;
    PblVector3 mAmbientShift, mAmbientShiftTarget;
    float mAmbientShiftSpeedMax;
    PblVector3 mAmbientShiftSpeed;
    RedAnimation* mAnimObj;
    float mNextBarrierCheck;
    PblVector3 mSlidePos, mSlideBeginPos;
    EntityFlyerClass* mClass;
    PassengerSlot* mPassengerSlots[4];
    MountedTurret* mTurret[8];
    char mNumTurrets;
    unsigned char _turretPadding[3];
    float mAnimationTime;
    Aimer* mAimer[4];
    Weapon* mWeapon[4];
    int mWeaponIndex[2];
    float mLastAltitude;
    ZephyrSkeleton<32> mZephyrSkeleton;
    ZephyrPoseDyn<32> mZephyrPoseDyn;
    RedPose mPose;
    Snd::SoundParameterized mSoundEngine;
    float mSpeedPrev, mTurbulance, mLastDistanceToTarget;
    struct AITimers {
        float mFreeFlyTime, mTrickTime, mTimeToNextTrick, mSquadronFormationTime;
        float mReactionThinkTime, mReactionTime, mEjectTimer, mTimeSinceLastHack;
    };
    struct ProceduralEngineParams {
        unsigned char velocity[12];
        struct PreviousPosition { float mPosition[3]; float mValid; } prevPos;
         
    };
    union AIEng { AITimers ai; ProceduralEngineParams eng; } mAIEng;
    Obstacle* mObstacle;
    float mTotalUpdateDt;
    bool mJustRendered;
    unsigned char _renderPadding[3];
    float mTrickEngineValue;
    GameSoundControllable mSoundTrick;
    PostCollision mPostCollision;
    PblHandle<ParticleEmitterObject> mThrustEffect[8];
    PblHandle<ParticleEmitterObject> mContrailEffect[12];
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

struct EntityFlyerRuntimeLayout
{
    void* mPrimaryVptr;
    unsigned char _toCollisionVptr[0x08];
    void* mCollisionVptr;
    unsigned char _toVptr94[0x84];
    void* mVptr94;
    unsigned char _toVptr140[0xa8];
    void* mVptr140;
    unsigned char _toVptr200[0xbc];
    void* mVptr200;
    unsigned char _toControllable[0x3c];
    void* mControllableVptr;
    unsigned char _toVptr258[0x14];
    void* mVptr258;
    unsigned char _toPathFollower[0x170];
    void* mPathFollowerVptr;
    unsigned char _toVelocity[0x170];
    PblVector3 mVelocity;
    unsigned char _toState[0x18];
    EntityFlyer::State mState;
    unsigned char _toClass[0xc4];
    EntityFlyerClass* mClass;
    unsigned char _toPose[0x1230];
    unsigned char mPoseAndTail[0x530];
};
 
 
 
 
 
 
 
 
 
 
 
 

class EntityCarrier : public EntityFlyer
{
public:
    struct CargoSlot
    {
        PblVector3 mOffset;
        PblHandle<GameObject> mObject;

        // FUNCTION: SWBF2 0x00496ed0
        CargoSlot() : mObject() {}

        // FUNCTION: SWBF2 0x00496ef0
        ~CargoSlot() {}
    };

    CargoSlot mCargoArray[4];
    int mCargoCount;

    char _tailPad[0xc];

    static PblHash rttiHashEntityCarrier;
    static char rttiNameEntityCarrier[];

    static MemoryPool sMemoryPool;

    EntityCarrier(EntityCarrierClass* pClass, const EntityDesc& desc);
    virtual ~EntityCarrier();
    virtual void Kill();
    virtual bool Update(float fDeltaTime);

    static void* operator new(unsigned int uSize);
    // FUNCTION: SWBF2 0x00496e90
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    bool IsRtti(unsigned int uRtti) const;
    unsigned int GetDerivedRtti() const;
    const char* GetDerivedRttiName() const;

    bool AttachCargo(int, GameObject* pGameObject);
    bool DetachCargo(int iSlot);
    void UpdateLandedHeight();
    void ActivatePhysics();

    EntityCarrierClass* GetClass() const
    {
        return static_cast<EntityCarrierClass*>(mClass);
    }
};
 
 
 
 

struct EntityCarrierRuntimeLayout
{
    EntityFlyerRuntimeLayout mFlyer;
    struct CargoSlot
    {
        PblVector3 mOffset;
        void* mObject;
        unsigned int mSavedHandleId;
    } mCargoArray[4];
    int mCargoCount;
    unsigned char _tail[0x0c];
};
 
 
 
