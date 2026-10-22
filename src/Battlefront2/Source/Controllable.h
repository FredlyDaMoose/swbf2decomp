#pragma once

#include <stddef.h>

#include "SoldierAnimationBank.h"
#include "Thread.h"
#include "Trackable.h"
#include "Trigger.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"

class Aimer;
class Character;
class CollisionModel;
class CommandPost;
class Controller;
class EnergyBar;
class Entity;
class GameObject;
class OrdnanceMissile;
class RedModel;
class RedPose;
class Weapon;
enum EnergyMode;

enum ControllableHeight
{
    HEIGHT_STAND = 0,
    HEIGHT_CROUCH = 1,
    HEIGHT_PRONE = 2,
    HEIGHT_SPRINT = 3,
    HEIGHT_WALK = 4,
    HEIGHT_COUNT = 5
};

enum PilotDeathType
{
    PILOTDEATH_FLY = 0,
    PILOTDEATH_FALL = 1,
    NUM_PILOTDEATH_TYPES = 2
};

enum PilotType
{
    PILOT_NONE = 0,
    PILOT_SELF = 1,
    PILOT_VEHICLE = 2,
    PILOT_REMOTE = 3,
    PILOT_VEHICLESELF = 4
};

struct CamPathInfo
{
    PblVector3 Vec[6];
    PblVector3 Slope[6];
    PblVector3 Ore[6];
    PblVector3 RSlope[6];
    float Time[6];
    unsigned int numPoints;
};
 
 

class ControllableClass
{
public:
    ControllableClass();

    ControllableClass(ControllableClass* pBase);

    bool SetProperty(unsigned int uProperty, const char* pszValue, const RedModel* pModel);

    void UpdatePilotAnimation(Controllable* pPilot, const RedPose* pPose, const PblMatrix& mMatrix,
                              const PblVector3& vOffset, float fDeltaTime, float f1, float f2, float f3);

    bool DoesCombatInterrupt() const { return mCombatInterrupts; }

    bool mCombatInterrupts;
    bool mIgnoreHintNodes;
    float mShakeScale;
    CamPathInfo mBobPath;
    float mCockpitTension;
    unsigned int mLocalNameID;
    unsigned int mForceMode;
    float mFirstPersonFOV;
    float mThirdPersonFOV;
    unsigned int mPilotParentNodeHash;
    PblMatrix mPilotOffsetMatrix;
    SoldierAnimationBank::ANIMATION mPilotAnimation;
    bool mAnimatedPilotNodeFlag;
    PilotDeathType mPilotDeathType;
    PilotType mPilotType;
    bool mIsPilotExposed;
    bool mIsPilotAnimation9Pose;
    bool mCanCapturePosts;
    bool mCanEnterVehicles;
    bool mControlsUnit;
    bool mAnimatesUnitAsTurret;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

float __cdecl GetDefaultFOV();

class TargetInfo
{
public:
    TargetInfo();

    void SetAimPoint(const PblVector3& aimPoint);

    static float GetTargetPrio(const PblVector3& vStart, const PblVector3& vDir, const PblVector3& vTarget,
                               float fRadius, float fAngle, float fRange, bool bFlag);

    PblVector3 mAimStart;
    PblVector3 mAimPoint;
    bool mIsAiming;
};
 

class Controllable : public Thread, public Trackable
{
public:

    class AIThread : public Thread
    {
    public:

        AIThread(Controllable* pOwner) : mOwner(pOwner) {}

        virtual bool Update(float fDeltaTime);
        virtual void DeactivateThread();

        Controllable* mOwner;
    };

    Controllable(ControllableClass* pClass, int iPlayerId);
    virtual ~Controllable();

protected:
    virtual bool Update(float fDeltaTime);

public:
    virtual void Kill();
    virtual GameObject* GetGameObject();
    virtual const GameObject* GetGameObject() const;
    virtual Entity* GetEntity();
    virtual const Entity* GetEntity() const;
    virtual Trackable* GetTrackable();
    virtual CollisionModel* GetDefaultPilotCollision();
    virtual const PblMatrix& GetMatrix() const = 0;
    virtual float GetTargetBubbleStanceModifier() const;
    virtual float GetSoundRadius() const;
    virtual float GetAimTurnRate() const;
    virtual int GetWeaponCount() const;
    virtual int GetWeaponIndex(int iChannel) const;
    virtual Weapon* GetWeaponPtr(int iIndex) const;
    virtual EnergyBar* GetEnergyBar();

    float GetEnergy() const;
    virtual EnergyMode GetEnergyMode() const;
    virtual SoldierAnimationBank::BANK GetSoldierAnimationBank() const;
    virtual void SetWeaponIndex(int iChannel, int iIndex);
    virtual const Aimer* GetAimer(int iIndex) const;
    virtual bool CanEnterRemote(const GameObject* pObject) const;
    virtual bool CanEnterVehicle(const GameObject* pObject) const;
    virtual bool EnterVehicle(GameObject* pObject);
    virtual bool CyclePosition(bool bForward);
    virtual bool ExitVehicle(bool bForce, bool bEject);

    // FUNCTION: SWBF2 0x00477550
    virtual bool EnterRemote(GameObject* pObject) { return false; }
    // FUNCTION: SWBF2 0x00477560
    virtual bool ExitRemote() { return false; }
    virtual const ControllableClass* GetControllableClass() const;
    virtual ControllableClass* GetControllableClass();
    virtual void SetupController();
    virtual void SetPilot(Controllable* pPilot);
    virtual void SetPilotInfo(const PblMatrix& mMatrix, const PblVector3& vOffset,
                              SoldierAnimationBank::ANIMATION eAnimation, bool bFlag,
                              float f0, float f1, float f2, float f3);
    virtual void GetPilotMatrix(const PblMatrix& mIn, PblMatrix& mOut);
    virtual PilotDeathType GetPilotDeathType();
    virtual void ApplyWeaponKick(float fPitch, float fYaw);
    virtual void GetPitchYawLimits(float& fPitchMin, float& fPitchMax,
                                   float& fYawMin, float& fYawMax) const;
    virtual bool Stand();
    virtual bool Crouch();
    virtual bool Prone();
    virtual bool Jump();
    virtual bool Roll();
    virtual bool Sprint();
    virtual bool EndSprint();
    virtual bool JetJump();
    virtual bool EndJetJump();
    virtual float GetOffenseMultiplier() const;
    virtual int GetActiveWeaponChannel();
    virtual bool UpdateIndirect(float fDeltaTime) = 0;

protected:
    virtual void DeactivateIndirect();

public:
    void Initialize();
    void ClearControllable();
    void ResetControls();
    void SetSwitch(float fValue, int iIndex);

    float GetTurnBuildup() const { return mTurnBuildup; }
    void SetTurnBuildup(float fBuildup) { mTurnBuildup = fBuildup; }
    float GetPitchBuildup() const { return mPitchBuildup; }
    void SetPitchBuildup(float fBuildup) { mPitchBuildup = fBuildup; }

    float GetThrust() const { return mControlMove; }

    // FUNCTION: SWBF2 0x0051ecf0
    void SetInitialPlayerId(int iPlayerId) { mInitialPlayerId = iPlayerId; }

    // FUNCTION: SWBF2 0x004959b0
    PilotType GetPilotType() const { return mPilotType; }

    // FUNCTION: SWBF2 0x00483450
    bool IsRemoteVehicle() const { return mPilotType == PILOT_REMOTE; }

    // FUNCTION: SWBF2 0x0047a700
    int GetPlayerId() const { return mPlayerId; }
    Controllable* GetEnteredRemoteOrVehicleIfNotMe();

    Controllable* GetPilot();

    const Controllable* GetPilot() const;
    void SetExitVehicle(const CommandPost* pPost);
    void AddController(Controller* pController, bool bIndirect);
    void ReleaseController();
    void SetCharacter(Character* pCharacter);
    void SetCurrentCollision(GameObject* pObject);
    GameObject* GetCurrentCollision() const;
    const Aimer* GetCurAimer() const;

    const PblVector3& GetEyePoint() const { return mEyePoint; }
    const Weapon* GetCurWpn() const;
    Weapon* GetCurWpn();
    int GetWpnChannel() const;
    float GetWpnDistFactor() const;
    void SetWpnDistFactor(float fFactor);

    float GetEnemyLockedOnMeState() const { return mEnemyLockedOnMeState; }
    void SetEnemyLockedOnMeState(float fState, OrdnanceMissile* pMissile, float fDistance);
    float GetTurnAdjusted();
    float GetPitchAdjusted();
    GameObject* GetTargetLockedObj();
    void SetTargetLockedObj(GameObject* pObject);

    static GameObject* GetVehicleIfAttached(GameObject* pObject);
    void TargetTrackingForceClear();

protected:
    void SetIndirectControl(bool bIndirect);

public:

    // FUNCTION: SWBF2 0x00437540
    Character* GetCharacter() const { return mCharacter; }
    Trigger mControlFire[2];
    Trigger mControlReload;
    Trigger mControlJump;
    Trigger mControlCrouch;
    Trigger mControlSprint;
    Trigger mControlUse;
    Trigger mControlZoom;
    Trigger mControlView;
    Trigger mControlLockTarget;
    Trigger mControlSquadCommand;

    Trigger mControlSpecialMove;
    Trigger mControlThrustFwd;
    Trigger mControlThrustBack;
    Trigger mControlStrafeLeft;
    Trigger mControlStrafeRight;
    Trigger mTriggerControlSwitch[2];
    float mControlMove;
    float mControlStrafe;
    float mControlTurn;
    float mControlPitch;
    float mControlSwitch[2];
    int mWpnChannel;
    AIThread mAIThread;
    PblList<Controllable>::Node mNode;
    Controller* mCtrl;
    Character* mCharacter;
    Controllable* mPilot;
    int mPlayerId;
    int mInitialPlayerId;
    PblVector3 mEyePoint;
    PblVector3 mEyeDir;
    float mTurnBuildup;
    float mPitchBuildup;
    bool mIndirectControlFlag;
    bool mUsingTurnAdjusted;
    bool mPlayAnim;
    unsigned int mAnimNameId;
    float mWpnDistFactor;
    PblHandle<GameObject> mCollisionObj;
    float mEnemyLockedOnMeState;
    float mEnemyLockedOnMeTimestamp;
    OrdnanceMissile* mEnemyLockedOnMeMissile;
    float mEnemyLockedOnMeDistance;
    float mTurnAdjusted;
    float mPitchAdjusted;
    float mTurnAuto;
    float mPitchAuto;
    float mLockBreakTimer;
    float mInCaptureRegionTimestamp;
    PblHandle<GameObject> mTargetLockedObj;
    int mAllowedLockTypes;
    PilotType mPilotType;
    TargetInfo mTargetInfo;
    PblHandle<const GameObject> mReticuleTarget[2];
    PblHandle<const GameObject> mAIReticuleTarget[2];
    int mReticuleAffiliation[2];
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
