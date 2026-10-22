#pragma once

#include <stddef.h>

#include "Controllable.h"
#include "CommandPost.h"
#include "PathFollower.h"
#include "SoldierAnimType.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"

class AIPath;
class GameObject;
class Navigator;
class Obstacle;
class UnitController;
class Weapon;

enum FoldStatus
{
    FOLD_NONE = 0,
    FOLD_UNFOLD = 1,
    FOLD_FOLD = 2
};

namespace AI
{
    enum HeadLookState
    {
        HEAD_LOOK_OFF = 0,
        HEAD_LOOK_OBJECT = 1,
        HEAD_LOOK_RIGHT = 2,
        HEAD_LOOK_LEFT = 3
    };
}

class ControllableDestination
{
public:

    ControllableDestination() {}
    ControllableDestination(const ControllableDestination& other);

    bool mHasDest;
    PblVector3 mPos;
    PblVector3 mOriginalPos;
    PblVector3 mDirection;
    SafeHeight mSafeHeightEnforce;
    float mTaperFactor;
    float mMaxSpeed;
};
 
 
 

class ControllableHeading
{
public:

    enum
    {
        HEADING_OBJ = 0,
        HEADING_POS = 1,
        HEADING_DEFAULT = 2
    };

    int mType;
    PblVector3 mDest;
    float mDestWander;
    PblHandle<const GameObject> mObj;
};

class AILowLevel
{
public:

    enum BarrageState
    {
        STATE_UNDEFINED = 0,
        STATE_SETUP = 1,
        STATE_WAITING = 2,
        STATE_FIRING = 3
    };

    enum TriggerSingleState
    {
        STATE_TRIGGER_DOWN = 0,
        STATE_TRIGGER_UP = 1
    };

    AILowLevel(UnitController* pOwner);

    ~AILowLevel();

    void SetNavigator_Stationary();

    void SetFastTurn();
    void SetTarget(const GameObject* pTarget);
    void HeadLook_Glance(bool bRight, float fTurnToTime, float fStayTime, float fTurnBackTime);
    void HeadLook_Object(const GameObject* pObj, float fTurnToTime, float fStayTime, float fTurnBackTime);

    void SetRoll(PblVector3 dir)
    {
        mRoll = true;
        mRollDir = dir;
    }

    void SetJetJump(PblVector3 dir)
    {
        mDoJetJump = true;
        mAIJetJumpDir = dir;
    }

    void SetJump(PblVector3 dir)
    {
        mDoJump = true;
        mAIJetJumpDir = dir;
    }
    bool IsJumping() const { return mDoJump || mJumping; }
    void SetCombat() { mInCombat = true; }

    void SetHeight(ControllableHeight eHeight);

    ControllableHeight GetHeight() const { return mHeight; }

    void ResetControls();
    void Initialize();
    void SetDest(const PblVector3& pos, const PblVector3& originalPos,
                 const PblVector3& direction, SafeHeight safeHeight,
                 float fTaperFactor, float fMaxSpeed);
    void SetDestCurrent();
    unsigned char GetPathQueueDepth();
    bool HasPath() const;
    bool IsAtDest() const;
    int IsStuck() const;
    void SetHeading(const PblVector3& dest, float fWander);
    void SetHeadingDefault();
    void SetExitVehicle(const CommandPost* pCP);

    void ClearControllable();
    void SetHeading(const GameObject* pObj);
    void SetBoardVehicle(GameObject* pVehicle);
    void SetAimPoint(const PblVector3& aimPoint);
    const GameObject* GetTarget() const;
    bool IsSafeToRoll(const PblVector3& dir);
    void SetNavigator_GotoDirect(const PblVector3& dest, bool bFlyerIgnoreBarriers,
                                 bool bUnused);

    void SetNavigator_Goto(const PblVector3& dest, const Obstacle* pIgnore,
                           bool bFlyerIgnoreBarriers, bool bRollIntoDest);
    void SetNavigator_FollowTarget(const GameObject* pTarget,
                                   const PblVector3& followOffset);
    void SetNavigator_Slide(const PblVector3& dest, const Obstacle* pIgnore);
    void SetNavigator_Slide(const GameObject* pSlideObj, float fOffset,
                            bool bUseAttackHelper, const Obstacle* pIgnore);
    void ProcessSniperWeapon(float dt, ControllableHeading* pHeading, bool* pFire);
    void SetFireRequest(bool bImmediate, bool bUponAim);

    void ProcessFire(float dt, ControllableHeading* pHeading);
    bool GetAlert();
    void SetAlert();
    bool UseFastTurn() const;
    void SetNextWeapon(int iIndex);

    bool UpdateIndirect(float fDeltaTime);
    void Update(float dt);

    void UpdateSkip();
    bool IsNavigatorInProgress();
    bool IsNavigatorFailedPath();
    void Navigator_Wait(float fTime);
    void SetNavigator_FollowPath(AIPath* pPath, bool bDoCallbacksOnPts,
                                 bool bFlyerIgnoreBarriers,
                                 bool bFlyerIgnoreSafeHeight);
    void ReceivePathResult(AIPath* pPath);
    PblVector3 GetGlobalDest();
    PblVector3 GetLocalDest();
    AI::HeadLookState GetHeadLookState() const;

    bool CalcHeadLookMatrix(const PblMatrix* pHead, PblMatrix* pOut);

    void SetMinControlSpeed(float fSpeed) { mMinControlSpeed = fSpeed; }

private:
    const PblMatrix& GetMatrix() const;
    void DeleteNavigator();
    void UpdateHeadLook(float dt);

    void ProcessChargeWpn(const Weapon* pWpn, bool* pFire);
    void ProcessBarrageWpn(const Weapon* pWpn, bool* pFire);
    void ProcessTriggerSingle(float dt, ControllableHeading* pHeading,
                              Weapon* pWpn, bool* pFire);
    bool AvoidPlayer(float dt, const Weapon* pWpn);

public:
    PblVector3 mRollDir;
    bool mRoll : 1;
    bool mRolling : 1;
    bool mDoJetJump : 1;
    bool mDoJump : 1;
    bool mJumping : 1;
    bool mJetJumping : 1;
    bool mJetJumpStop : 1;
    SoldierAnimType::MovementAnimation mDoIdle;
    UnitController* mOwner;
    Navigator* mNavigator;
    ControllableDestination mDest;
    ControllableHeading mHeading;
    ControllableHeight mHeight;
    float mDesiredHoverHeight;
    bool mFlyerIgnoreBarriers : 1;
    bool mFlyerIgnoreSafeHeight : 1;
    bool mBoardVehicle : 1;
    bool mExitVehicle : 1;
    bool mInCombat : 1;
    bool mAllowHoverStrafe : 1;
    bool mLandAtDest : 1;
    bool mImmediateFire : 1;
    bool mFireUponAim : 1;
    bool mIsCharging : 1;
    bool mIsAlert : 1;
    bool mFastTurn : 1;
    ConstCommandPostHandle mExitCP;
    PblHandle<GameObject> mVehicle;
    PblVector3 mAIJetJumpDir;
    float mMinControlSpeed;
    FoldStatus mFoldStatus;
    float mGrenadeOffTimer;
    float mChargeLimit;
    PblHandle<const GameObject> mTarget;
    float mNextSniperFireTime;
    BarrageState mBarrageState;
    TriggerSingleState mTriggerSingleState;
    float mTriggerSingleWaitUntil;
    float mBarrageHeat;
    float mBarrageBeginDelay;
    int mBarrageBeginAmmo;
    int mBarrageTotal;
    int mNextWeaponIndex;
    AI::HeadLookState mHeadLookState;
    float mHeadLook_TurnToTime;
    float mHeadLook_StayTime;
    float mHeadLook_TurnBackTime;
    float mHeadLook_Time;
    PblHandle<const GameObject> mHeadLook_Object;
};

 
 
 
