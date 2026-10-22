#pragma once

#include <stddef.h>

#include "Fsm.h"
#include "CoverHelper.h"
#include "VehicleCoverHelper.h"
#include "JetJumpHintHelper.h"
#include "PebbleFL/Common/PblVector.h"

class AttackHelper;
class Controllable;
class JetJumpHint;
class UnitAgent;
class UnitController;

// VTABLE: SWBF2 0x007af6fc
class AttackPatterns : public HelperFsm
{
public:

    enum State
    {
        STATE_START = 1,
        STATE_STATIONARY = 2,
        STATE_SIMPLE = 3,
        STATE_SERPENTINE = 4,
        STATE_FLANK = 5,
        STATE_CRITICALHIT = 6,
        STATE_MELEE = 7,
        STATE_GLANCE = 8,
        STATE_RETREAT = 9,
        STATE_COVER = 10,
        STATE_VEHICLECOVER = 11,
        STATE_JETJUMP = 12,
        STATE_JETJUMPHINT = 13,
        STATE_JETJUMPCHARGE = 14,
        STATE_LOCATE = 15,
        STATE_FLY_STRAFE = 16,
        STATE_FLY_DIRECT = 17,
        STATE_FINAL = 18
    };

    AttackPatterns(UnitAgent* pOwner);

    virtual void Kill();
    virtual void HELPERSetNextState(int state, float fTime);

    void HELPERClean();

    bool HELPERGetOverrideFire() const { return mOverrideFire; }
    bool IsJetJumpHinting() const { return mCurState == STATE_JETJUMPHINT; }

protected:

    enum
    {
        LEFT = 0,
        FORWARD = 1,
        RIGHT = 2,
        BACKWARD = 3
    };

    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);

    const Controllable* GetMe() const;
    Controllable* GetMe();
    UnitController* GetCtrl();
    const AttackHelper* GetAttackHelper() const;
    AttackHelper* GetAttackHelper();

    friend class CoverHelper;
    static float GetNear(const Controllable* pMe);
    float GetNear() const;
    float GetFar() const;
    float GetOptimalish() const;
    float CalculateAngle(const Controllable* pCtrl) const;
    bool GetInRange();
    bool IsInEnemySights(float fAngle, const Controllable** ppEnemy);
    PblVector3 GetLastSeenTargetPos() const;
    bool ToggleToCover(int iReturnState, float fPercentMult);
    bool ToggleToVehicleCover(int iReturnState);
    bool ToggleToJetJump(int iReturnState);
    bool ToggleToJetJumpHint(int iReturnState);
    bool ToggleToJetJumpCharge(int iReturnState);
    bool ToggleToLocate(int iReturnState);
    void ToggleToReturnState();
    void SetSimpleTarget();
    void CycleSerpDir();
    void SetGlanceDest();
    bool IsGlanceMakingProgress();
    bool GonePastGlancePos() const;
    void PickRetreatSpot();

    void InitStationary();
    void InitSimple();
    void InitSerpentine();
    void InitFlank();
    void InitCriticalHit();
    void InitMelee();
    void InitGlance();
    void InitRetreat();
    void InitCover();
    void InitVehicleCover();
    void InitJetJump();
    void InitJetJumpHint();
    void InitJetJumpCharge();
    void InitLocate();
    void InitFlyStrafe();
    void InitFlyDirect();

    void DoStationary();
    void DoSimple();
    void DoSerpentine();
    void DoFlank();
    void DoCriticalHit();
    void DoMelee();
    void DoGlance();
    void DoRetreat();
    void DoCover(float dt);
    void DoVehicleCover(float dt);
    void DoJetJump(float dt);
    void DoJetJumpHint(float dt);
    void DoJetJumpCharge(float dt);
    void DoLocate();
    void DoFlyStrafe();
    void DoFlyDirect();

    void CleanMelee();
    void CleanGlance();
    void CleanCover();
    void CleanVehicleCover();
    void CleanJetJump();
    void CleanJetJumpHint();
    void CleanJetJumpCharge();
    void CleanLocate();
    void CleanFlyStrafe();
    void CleanFlyDirect();

    void GeneratePath();

public:
    UnitAgent* mOwner;
    CoverHelper mCoverHelper;
    float mCoverPercent;
    float mSwitchToCoverTimer;
    VehicleCoverHelper mVehicleCoverHelper;
    float mVehicleCoverPercent;
    float mSwitchToVehicleCoverTimer;
    JetJumpHintHelper mJetJumpHintHelper;
    bool mOverrideFire;
    int mReturnState;
    bool mIsPathing;
    ControllableHeight mHeight;
    bool mSimpleStopped;
    float mSimpleTimer;
    int mSerpAhead;
    int mSerpStrafe;
    int mSerpCur;
    float mSerpTimer;
    float mSerpRangeTimer;
    bool mSerpEnemySee;
    bool mSerpInRange;
    float mFlankTimer;
    bool mFlankInView;
    float mMeleePathTime;
    float mJumpTimer;
    bool mOnApproach;
    PblVector3 mGlanceSrc;
    PblVector3 mGlancePos;
    float mClosestTime;
    float mClosestXZSq;
    float mRetreatTimer;
    bool mJumping;
    float mSwitchToJetJumpTimer;
    float mJumpStartTime;
    JetJumpHint* mJetJumpHint;
    float mSwitchToJetJumpHintTimer;
    bool mLocateAtDest;
    bool mDoLoop;
    bool mUpsideDown;
};
  
 
 
 
 
 
 
