#pragma once

#include <stddef.h>

#include "GameSound.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"
#include "Zephyr/Source/ZephyrSkeleton.h"

class ZephyrSkeletonShared;

class AcklayData
{
public:
    AcklayData();
    ~AcklayData();

    static void Init();

    const RedPose* GetPose() const { return bHasAnimatedPose ? &mAnimatedPose : &mPose; }
    void SetupBasePose(const ZephyrSkeletonShared* pSkeleton);

    static MemoryPool sMemoryPool;

    static void* operator new(size_t uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    GameSound mFootSound[2];
    float mFeetElevation[4];
    ZephyrSkeleton<32> mZSkeleton;
    ZephyrPoseStatic<32> mZPoseStatic;
    RedPose mPose;
    RedPose mAnimatedPose;
    bool bHasAnimatedPose;
};
 
 
 
 
 
 
 

#include "Aimer.h"
#include "Battlefront2Lod.h"
#include "CollisionModel.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "GameMusic.h"
#include "GameObject.h"
#include "GameSound.h"
#include "PlayableEntitySounds.h"
#include "SoldierAnimationBank.h"
#include "Trackable.h"
#include "VehicleEngine.h"
#include "Weapon.h"

class AnimatedAddon;
class ExplosionClass;
class FLEffectClass;
class GameModel;
class ZephyrSkeletonShared;

// VTABLE: SWBF2 0x0079d2a4
class EntitySoldierClass : public GameObjectClass,
                           public ControllableClass,
                           public VehicleEngineClass,
                           public PlayableEntitySoundsClass
{
public:
    EntitySoldierClass(unsigned int uHash);
    EntitySoldierClass(EntitySoldierClass* pBase, unsigned int uHash);
    virtual ~EntitySoldierClass();

    int FindSoldierType(const char* pszName);

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void PostReadSetup();
    virtual int GetSoldierType();
    virtual const void* GetControllableClass();
    virtual int GetWeaponCount();
    virtual const WeaponClass* GetWeaponClass(int iIndex);

    unsigned int GetSndHeroSelectable() { return mSndHeroSelectable; }
    unsigned int GetSndHeroSpawned() { return mSndHeroSpawned; }
    unsigned int GetSndHeroDefeated() { return mSndHeroDefeated; }
    unsigned int GetSndForHeroKiller() { return mSndForHeroKiller; }
    static PblHash rttiHashEntitySoldierClass;

    // FUNCTION: SWBF2 0x004294a0
    static unsigned int GetRtti() { return rttiHashEntitySoldierClass; }

    float GetJetFuelMinBorder() const { return mJetFuelMinBorder; }

    bool JetJumpShowHUD() const { return mJetShowHud != 0; }

    RedLodData mLodData;
    GameModel* mModelLowRes;
    float mAcceleration;
    float mMaxSpeed;
    float mMaxStrafeSpeed;
    float mMaxTurnSpeed;
    float mMaxPitchSpeed;
    float mMinPitch;
    float mMaxPitch;
    float mJumpVerticalSpeed;
    float mJumpForwardSpeedFactor;
    float mJumpStrafeSpeedFactor;
    float mRollSpeedFactor;
    float mSprintAccelerateTime;

    unsigned int m_bRecoverFromTumble : 1;
    unsigned int m_bUseDirectionalJumps : 1;
    unsigned int mJetShowHud : 1;
    unsigned int mJetTypeHover : 1;
    unsigned int mFleeLikeAHero : 1;
    unsigned int m_bIsAcklay : 1;
    unsigned int m_bUseDirectionalDeaths : 1;
    unsigned int m_numTentacles : 3;
    unsigned int m_bonesPerTentacle : 4;
    unsigned int m_tentacleCollType : 2;
    unsigned int z_uiReserved0 : 16;
    float mThrustFactor[8];
    float mStrafeFactor[8];
    float mTurnFactor[8];
    float mCollisionThreshold[3];
    float mCollisionScale[3];
    float mCollisionInflict;
    WeaponClass* mWeaponClass[8];
    WeaponAmmoDesc mWeaponAmmo[8];
    char mWeaponChannel[8];
    char mWeaponCount;
    unsigned int mOverrideTextureHash;
    unsigned int mOverrideTextureHash2;
    EnergyBarClass mEnergyBarClass;
    float mEnergyRestore;
    float mEnergyRestoreIdle;
    float mEnergyDrainSprint;
    float mEnergyCostJump;
    float mEnergyCostJumpSprint;
    float mEnergyCostRoll;
    float mJetJump;
    float mJetPush;
    float mJetAcceleration;
    float mJetFuelRechargeRate;
    float mJetFuelCost;
    float mJetFuelInitialCost;
    float mJetFuelMinBorder;
    float mJetEnergyDrain;
    FLEffectClass* mJetPackEffect;
    FLEffectClass* mJetPackIdleEffect;
    unsigned int mHealthGainEffect;
    unsigned int mAmmoGainEffect;
    unsigned int mEnergyGainEffect;
    unsigned int mBuffHealthEffect;
    unsigned int mBuffOffenseEffect;
    unsigned int mBuffDefenseEffect;
    unsigned int mDebuffEffect;
    unsigned int mInvincibleEffect;
    unsigned int mFootSlideEffect;
    unsigned int mChokeEffect;
    float m_fBlurEffect;
    unsigned int mWaterSplashEffect;
    unsigned int mFootWaterSplashEffect;
    unsigned int mWakeWaterSplashEffect;
    float mNoiseFactor;
    int mSoldierType;
    bool mImmuneToMines;
    AimerClass* mAimerClass;
    CameraTrackSetting mCameraSettings[2][4];
    float mCameraSettingBlendTime[3];
    CollisionModel mCollisionModels[3];
    CollisionModel mDeadCollision;
    PblVector3 m_vCollisionHeadOffset;
    PblVector3 m_vCollisionTorsoOffset;
    float m_fCollisionHeadScale;
    float m_fCollisionTorsoScale;
    ZephyrSkeletonShared* mSpecialSkeleton;

    char mUpperBodyAnimMask[4];
    char mLowerBodyAnimMask[4];
    ZephyrSkeletonShared* mSpecialSkeletonLowRes;
    AnimatedAddon* mAddon;
    float mFootstepStride;
    float mRunSpeed;
    GameSound mAcquiredTargetSound;
    GameSound mHidingSound;
    GameSound mApproachingTargetSound;
    GameSound mFleeSound;
    GameSound mPreparingForDamageSound;
    GameSound mHeardEnemySound;
    GameSound mChargeSound;
    GameSound mAmbientSound;
    float mShockFadeOutTime;
    float mShockFadeInTime;
    float mShockFadeOutGain;
    GameSound mShockSound;
    GameSound mClothingRustleSound;
    GameSound mLowHealthSound;
    float mLowHealthThreshold;
    GameSound mChokeSound;
    int mDropItemCount;
    EntityClass* mDropItemClass[8];
    float mDropItemProbability[8];
    float mDrownTime;
    float mChunkFrequency;
    float mSkeletonRootScale;
    float mCollisionRootScale;
    float mSkeletonRootScaleLowRes;
    float mMinFootHeight;
    SoldierAnimationBank::BANK mAnimationBank;
    int mFirstPersonModelHandle;
    ExplosionClass* mExplosion;
    int mUpperBodyId[3];
    int mLowerBodyId[3];
    unsigned int mSndHeroSelectable;
    unsigned int mSndHeroSpawned;
    unsigned int mSndHeroDefeated;
    unsigned int mSndForHeroKiller;
    GameMusic mSoldierMusic;
    GameMusic mAISoldierMusic;
    GameSound mSoldierAnnouncement;
    PblVector3 mTargetPointOffset[3];
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

 
 
 

#include "CollisionBody.h"

namespace SoldierState
{
    enum SoldierState
    {
        STAND = 0,
        CROUCH = 1,
        PRONE = 2,
        SPRINT = 3,
        JUMP = 4,
        ROLL = 5,
        JET_JUMP = 6,
        JET_HOVER = 7,
        FALL = 8,
        FLY = 9,
        TUMBLE = 10,
        BOUNCE = 11,
        FLY_RECOVER = 12,
        TUMBLE_RECOVER = 13,
        CHOKE = 14,
        DEAD = 15,
        HERO_FLEE = 16,
        PILOT_ANIMATION = 17,
        ARMED_PASSENGER = 18,
        SLIDE = 19
    };
}

class SoldierAnimator;

class CameraTrackSetting;
class CollisionObject;
class CollisionResult;
class EntityCloth;
class FoleyFX;
class NetObjState;
class NetPktGroup;
class Ordnance;
class OrdnanceGrapplingHook;
class Restrictor;
class SoldierAnimatorLowRes;
class TargetInfo;
class TentacleSimulator;
class Weapon;

class EntitySoldier : public GameObject,
                      public Controllable,
                      public PlayableEntitySounds,
                      public VehicleEngine
{
public:

    // VTABLE: SWBF2 0x0079d094
    class PostCollision : public Thread
    {
    public:

        PostCollision(EntitySoldier* pOwner) : mOwner(pOwner) {}
        virtual ~PostCollision();
        virtual bool Update(float fDeltaTime);

        EntitySoldier* mOwner;
    };

    enum CollisionState
    {
        COLLSTATE_NONE = 0,
        COLLSTATE_COLLIDE_WITH_RIGID = 1,
        COLLSTATE_REVERT_TO_SAFE = 2
    };
    enum DAMAGEDIR
    {
        FORWARD = 0,
        LEFT = 1,
        BACKWARD = 2,
        RIGHT = 3
    };
    enum MOVEANIM
    {
        MOVEANIM_IDLE = 0,
        MOVEANIM_WALK = 1,
        MOVEANIM_RUN = 2
    };
    enum Flags
    {
        flags_footprintLeft = 1,
        flags_isGrounded = 2,
        flags_useBinocularsAI = 4,
        flags_forceChunkDeath = 8,
        flags_leftStep = 16,
        flags_leftFootInWater = 32,
        flags_rightFootInWater = 64,
        flags_fallTimer = 128
    };
    enum
    {
        INPUTLOCK_MOVEMENT = 1,
        INPUTLOCK_ACTION = 2,
        INPUTLOCK_WEAPON = 4,
        INPUTLOCK_ALL = 7
    };

    EntitySoldier(EntitySoldierClass* pClass, const EntityDesc& desc);

    virtual ~EntitySoldier();

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
    virtual int GetTargetBodyID(const PblVector3& vPos, const PblVector3& vDir) const;
    virtual PblVector3 GetTargetPoint(const PblVector3& vPos, const PblVector3& vDir,
                                      int iBodyId) const;
    virtual float GetMaxSpeed() const;
    virtual float GetMaxStrafe() const;
    virtual TargetInfo* GetTargetInfo();
    virtual const Controllable* GetControllable() const;
    virtual Controllable* GetControllable();
    virtual const PlayableEntitySounds* GetPlayableEntitySounds() const;
    virtual PlayableEntitySounds* GetPlayableEntitySounds();
    virtual float CalcAimSpreadVal() const;
    virtual bool ShouldShowReticule() const;
    virtual void ApplyRadiusDamage(const PblVector3& vPos, float fInnerRadius,
                                   float fOuterRadius, const DamageDesc& damage);
    virtual void ApplyRadiusPush(const PblVector3& vPos, float fInnerRadius,
                                 float fOuterRadius, float fPush, bool bArg);
    virtual void ApplyPush(const PblVector3& vPush, float fArg, bool bArg);
    virtual int GetReservableSlots(ReserveType eType) const;
    virtual void ActivatePhysics();
    virtual void DeactivatePhysics();
    virtual GameObjectType GetGameObjectType();
    virtual void AcquiredTarget();
    virtual void Hiding();
    virtual void ApproachingTarget();
    virtual void Flee();
    virtual void PreparingForDamage();
    virtual void HeardEnemy();
    virtual void Charge();
    virtual bool Deflect(const Ordnance* pOrdnance, const PblVector3& vPos,
                         const PblVector3& vDir);
    virtual EntityType GetEntityType() const;
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual void ResetObject();
    virtual void SkipReadObject(NetObjState* pState);
    virtual void AddSends(void (*pfnSend)(GameObject*, int), int iArg);
    virtual const PblMatrix& GetSmoothedMatrix() const;
    virtual void WriteLateState(NetPktGroup& group);
    virtual void ReadLateState(NetPktGroup& group);

    virtual bool PostCollisionUpdate(float fDeltaTime);
    virtual bool EnterControllable(Controllable* pControllable);
    virtual int GetWeaponChannelByWeaponIndex(int iIndex) const;

    virtual void SetPosition(const PblVector3& vPosition);
    virtual void SetMatrix(const PblMatrix& matrix);
    virtual float GetSmallestCollisionDim() const;
    virtual bool CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor);
    virtual bool CollideTrisWithAllPrimitives() const;

    virtual const PblMatrix& GetMatrix() const;
    virtual const RedPose* GetPose() const;
    virtual void RecalculateAABB();

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    virtual void Kill();
    virtual bool ApplyDamage(const DamageDesc& damage, const PblVector3& vDir,
                             const PblVector3& vPos, int iBodyPart, unsigned int uFlags);
    virtual float GetDamageMultiplier(int iBodyId) const;
    virtual void HealthChangeCallback(bool bArg, float fArg);

    virtual bool Update(float fDeltaTime);
    virtual Trackable* GetTrackable();
    virtual float GetTargetBubbleStanceModifier() const;
    virtual float GetSoundRadius() const;
    virtual float GetAimTurnRate() const;
    virtual int GetWeaponCount() const;
    virtual int GetWeaponIndex(int iChannel) const;
    virtual Weapon* GetWeaponPtr(int iIndex) const;
    virtual EnergyBar* GetEnergyBar();
    virtual SoldierAnimationBank::BANK GetSoldierAnimationBank() const;
    virtual void SetWeaponIndex(int iChannel, int iIndex);
    virtual const Aimer* GetAimer(int iIndex) const;
    virtual bool CanEnterRemote(const GameObject* pObject) const;
    virtual bool CanEnterVehicle(const GameObject* pObject) const;
    virtual bool EnterVehicle(GameObject* pObject);
    virtual bool CyclePosition(bool bForward);
    virtual bool ExitVehicle(bool bForce, bool bEject);
    virtual bool EnterRemote(GameObject* pObject);
    virtual bool ExitRemote();
    virtual ControllableClass* GetControllableClass();
    virtual const ControllableClass* GetControllableClass() const;
    virtual void SetupController();
    virtual void SetPilotInfo(const PblMatrix& mMatrix, const PblVector3& vOffset,
                              SoldierAnimationBank::ANIMATION eAnimation, bool bFlag,
                              float f0, float f1, float f2, float f3);
    virtual void ApplyWeaponKick(float fPitch, float fYaw);
    virtual void GetPitchYawLimits(float& fPitchMin, float& fPitchMax,
                                   float& fYawMin, float& fYawMax) const;
    virtual float GetOffenseMultiplier() const;
    virtual int GetActiveWeaponChannel();
    virtual bool UpdateIndirect(float fDeltaTime);
    virtual void DeactivateIndirect();

    virtual void SetupCameraTrackMatrixHUDTargeting(PblMatrix& rMatrix);
    virtual void SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime);
    virtual void SetupCameraZoomFactor(float& rZoom);
    virtual void SetupCameraFOV(float& rFOV);
    virtual void SetupCameraIconVisibility(unsigned int& rMask);
    virtual void SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix);
    virtual CameraTrackSetting* GetCameraTrackSettings();
    virtual bool RenderTrackable(unsigned int uView) const;
    virtual bool IsFirstPersonAvailablePS2() const;
    virtual bool IsForcedThirdPerson() const;

protected:

    virtual bool Stand();
    virtual bool Crouch();
    virtual bool Prone();
    virtual bool Jump();
    virtual bool Roll();
    virtual bool Sprint();
    virtual bool EndSprint();
    virtual bool JetJump();
    virtual bool EndJetJump();
    virtual void SetFirstPersonView(int iView, bool bFirstPerson);

    bool CanEnterVehicleInternal(const GameObject* pObject, bool bArg) const;

    void MoveOffGround(float fDeltaTime, float fTurn, float fForward, float fStrafe);

    bool UpdateBoardVsChangeClass();

    bool SetState(SoldierState::SoldierState eState);
    GameObject* TryBoard(bool bArg, bool& rArg);

    float GetOmega(float fArg) const;
    void Move(float f0, float f1, float f2, float f3, float f4);
    void SetCollisionStance(int iStance);

public:

    float GetClassMaxTurnSpeed() const;
    void SetAlert(bool bAlert);
    int GetJoystickIndex();

    void TriggerBuffHealthEffect();
    void TriggerBuffOffenseEffect();
    void TriggerBuffDefenseEffect();
    void TriggerDebuffEffect();
    void UpdateSmolderTimers(float fDeltaTime);
    void TurnOffInvincibleEffect();
    int GetFirstPersonIndex();
    void UpdateBuffTimers(float fDeltaTime);

    void BuffOffense(float fTimer, float fMult);
    void BuffDefense(float fTimer, float fMult);
    void BuffHealth(float fTimer, float fRate);
    void DebuffDamage(float fTimer, float fRate, const DamageOwner& owner);
    void TriggerAmmoGainEffect();
    void TriggerEnergyGainEffect();

    void SetupSmolderEffect(int iIndex, unsigned int uiBone, unsigned int uiEffect);
    void ActivateSmolder(float fTimer, bool bVanishDeath, float fDamageRate, const DamageOwner& owner);

    void UpdateOrdnanceCollision(float fDeltaTime);

    void SetAlert(float fTime);
    void TriggerSmolderEffect(int iIndex, unsigned int uiBone, unsigned int uiEffect);

    void SetInvisibilityFactor(float fFactor);
    void UpdateInvisibility(unsigned char uiOldAlpha);

    void TurnOffEffects();

    void TriggerHealthGainEffect();
    void DirectionKill(const PblVector3& vDir);

    using RedSceneObject::SetRenderPos;

    void TurnOffJetEffect();

    void CheckForZoom(float fDeltaTime);

    void UpdateWeaponAndAimer(float fDeltaTime);

    bool Smooth(float fDeltaTime) const;

    FoleyFX* GetFoleyFX() const;

    void TurnOffJetIdleEffect();
    void UpdateFoleyFX(const RedPose& pose, const PblMatrix& mWorld);
    void UpdateAcklayFoleyFX(const RedPose& pose, const PblMatrix& mMatrix);

    void UpdateFoleyFX(float fDeltaTime);
    void PlayFootstep(float fSpeed, bool bRightFoot, const PblVector3* pPos,
                    const PblVector3* pVel);
    bool Stand(bool bArg);
    void TurnOnInvincibleEffect();
    bool CheckMoveControlForRoll(float fThrust, float fStrafe);
    bool CheckMoveControlForSprint(float fThrust, float fStrafe);
    bool RollUsingEnergy();
    bool JumpUsingEnergy();
    bool SprintUsingEnergy();

protected:

    void MoveJetHover(float fDeltaTime, float fForward, float fStrafe, float fTurn, float fAcceleration);

    void Fly();

public:
    bool InNonFirstPersonState();

    float GetInvisibilityFactor() const { return m_uiInvisibilityAlpha * (1.0f / 255.0f); }

    unsigned char GetInvisibilityAlpha() const { return m_uiInvisibilityAlpha; }
    bool CanChangeClass() const { return mCanChangeClass; }

    const EntitySoldierClass* GetClass() const { return mClass; }
    void SetGrapplingHook(OrdnanceGrapplingHook* pHook);

    void LockInput(float fTime)
    {
        m_uiInputLockMask = 7;
        m_bSlide = 0;
        mInputLockTime = fTime;
    }

    void Move(const PblMatrix& mMatrix, const PblVector3& vVelocity);
    void ApplyChoke(const PblVector3& vDir, float fUnused, SoldierAnimationBank::ANIMATION eAnimation);

    static PblHash rttiHashEntitySoldier;

    // FUNCTION: SWBF2 0x0043ab00
    static unsigned int GetRtti() { return rttiHashEntitySoldier; }

    float GetJetFuel() const { return mJetFuel; }
    static MemoryPool sMemoryPool;
    static char rttiNameEntitySoldier[];

    PblHandle<GameObject> mStandingOnObj;
    int mStandingOnBody;
    PblVector3 mStandingOnObjLastVelocity;
    OrdnanceGrapplingHook* mHook;
    EntitySoldierClass* mClass;
    TentacleSimulator* mTentacles;
    AcklayData* mAcklayData;
    PblVector3 mGroundNormal;
    PblVector3 mAILastCollisionNormal;
    PblVector3 mAvgFlyGroundCollisionNormal;
    unsigned char mNumFlyGroundCollisions;
    char mAILastCollisionDirection : 2;
    unsigned char m_uiInputLockMask : 3;
    unsigned char m_bSlide : 1;
    unsigned char mAttachedToProp : 1;
    unsigned char mFlyingInFirstPerson : 1;
    unsigned char m_bPrimaryWeaponIsMelee : 1;
    unsigned char mCanChangeClass : 1;
    char mLastRenderedLod : 2;
    unsigned char mMarkedForDeath : 1;
    unsigned char z_uiReserved0 : 1;
    unsigned char mSkip : 1;
    unsigned char mDoProneRotation : 1;
    unsigned char mTotalProneCollisions;
    float mInputLockTime;
    PblMatrix mLastVehicleMatrix;
    PostCollision mPostCollision;
    PblVector3 mVelocity;
    float mPitchValue;
    float mPitchOffset;
    float mPitchVelocity;
    float mKickScale;
    float mKickBuild;
    float mTurnAngle;
    Aimer mAimer;
    Weapon* mWeapon[8];
    char mWeaponIndex[2];
    char mActiveIndex : 4;
    char mActiveChannel : 2;
    unsigned char mStanceIndex : 2;
    unsigned char m_uiInvisibilityAlpha;
    SoldierState::SoldierState mState;
    SoldierState::SoldierState mOldState;
    float mStateTimer;
    SoldierAnimator* mSoldierAnimator;
    PblVector3 m_vChokeDir;
    float mAlertTimer;
    PblVector3 m_vLastPosition;
    float mLegAngle;
    CollisionBodyPrimitive mCollisionBodyHead;
    CollisionBodyPrimitive mCollisionBodyTorso;
    CollisionModel mDynamicCollisionModel;
    GameModel* mModelLowRes;
    SoldierAnimatorLowRes* mSoldierAnimatorLowRes;
    unsigned char mFlags;
    unsigned char z_uiReserved1 : 3;
    unsigned char mLegAngleForward : 1;
    unsigned char mMoveAnim : 2;
    unsigned char m_uiLastDamageDir : 2;
    unsigned short mSoundState;
    float mSkipTime;
    float mSkipIndirectTime;
    float mPrevZoom;
    float mSquadCommandClearTimer;
    SoldierAnimationBank::ANIMATION m_ePilotAnimation;
    SoldierAnimationBank::MAP m_eSoldierAnimationMap;
    unsigned char m_uiWeaponAnimIndex;
    float m_fWeaponAnimTimer;
    unsigned int mAnimationHash;
    float mForceFrameNum1;
    float mForceFrameNum2;
    float mForceFrameBlend;
    float mLeftFootElevation;
    float mRightFootElevation;

    unsigned char mEnergyBar[12];
    int mCameraSettingPrev;
    float mCameraSettingTimer;
    float mDrownTime;
    float mFoliageCollisionTime;
    GameSoundControllable mLowHealthSound;
    GameSoundControllable mLongFoleySound;
    GameSoundControllable mAmbientSound;
    GameSoundControllable mChokeSound;
    GameSound mCurFoleySound;
    float mSpeedPrev;
    float mDistanceMoved;
    PblHandle<FLEffectObject> mJetEffectHandle;
    PblHandle<FLEffectObject> mJetIdleEffectHandle;
    PblHandle<FLEffectObject> mHealthGainEffectHandle;
    PblHandle<FLEffectObject> mAmmoGainEffectHandle;
    PblHandle<FLEffectObject> mEnergyGainEffectHandle;
    PblHandle<FLEffectObject> mBuffHealthEffectHandle;
    PblHandle<FLEffectObject> mBuffOffenseEffectHandle;
    PblHandle<FLEffectObject> mBuffDefenseEffectHandle;
    PblHandle<FLEffectObject> mDebuffEffectHandle;
    PblHandle<FLEffectObject> mInvincibleEffectHandle;
    PblHandle<FLEffectObject> mSmolderEffectHandle[5];
    unsigned int mSmolderEffectBone[5];
    unsigned int mSmolderEffect[5];
    unsigned int mLastPilotInfoFrame;
    float mJetFuel;
    PblVector3 m_vPosFootRight;
    PblVector3 m_vPosFootLeft;
    PblHandle<FLEffectObject> mFootSlideEffectRightHandle;
    PblHandle<FLEffectObject> mFootSlideEffectLeftHandle;
    PblHandle<FLEffectObject> mChokeEffectHandle;
    PblMatrix mSafeMatrix;
    CollisionState mCollisionState;

    unsigned char mCollisionMeshCache[3][316];
    float mBuffOffenseTimer;
    float mBuffOffenseMult;
    float mBuffDefenseTimer;
    float mBuffDefenseMult;
    float mBuffHealthTimer;
    float mBuffHealthRate;
    float mDebuffDamageTimer;
    float mDebuffDamageRate;
    DamageOwner mDebuffDamageOwner;
    float mSmolderTimer;
    bool mSmolderVanishDeath : 1;
    float mSmolderDamageRate;
    DamageOwner mSmolderDamageOwner;
    Controllable* mPrevRemoteControllable;
    PblHandle<GameObject> mPrevRemoteGameObj;
    GameMusic mSoldierMusic;
    GameMusic mAISoldierMusic;
    unsigned char mNetJustChangedState;
    EntityCloth* mClothPiece[8];
public:
    static bool sEnableAnnouncementVO;
    static bool sEnableMusic;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

#include "SoldierAnimator.h"
