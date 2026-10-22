#pragma once

#include <stddef.h>
#include "Thread.h"

enum ToolTipType
{
    TTT_NONE = 0,
    TTT_JetTrooper_Jet = 1,
    TTT_SoldierSprint = 2,
    TTT_SoldierFire = 3,
    TTT_SoldierTarget = 4,
    TTT_SoldierTargetCP = 5,
    TTT_HoverSprint = 6,
    TTT_ApproachBuildPad = 7,
    TTT_ApproachEnemyCP = 8,
    TTT_ApproachSelfCP = 9,
    TTT_ApproachVehicle = 10,
    TTT_ApproachTurret = 11,
    TTT_AfterBoardFlyer = 12,
    TTT_AfterBoardMultiseat = 13,
    TTT_AfterBoardTurret = 14,
    TTT_ApproachHealthGonk = 15,
    TTT_ApproachAmmoGonk = 16,
    TTT_ApproachVehicleGonk = 17,
    TTT_JediFire = 18,
    TTT_JediBlock = 19,
    TTT_JediForceJump = 20,
    TTT_JediForcePower = 21,
    TTT_FlyerControls = 22,
    TTT_FlyerFirePrim = 23,
    TTT_FlyerFireSec = 24,
    TTT_FlyerStunt = 25,
    TTT_FlyerNoEnergy = 26,
    TTT_FlyerBoost = 27,
    TTT_FlyerTargetLock = 28,
    TTT_FlyerTargetClear = 29,
    TTT_FlyerLandingZone = 30,
    TTT_FriendsListColor = 31,
    TTT_COUNT = 32
};

struct TipClass
{
    float mInputHold;
    float mTimeout;
    float mActiveTime;
    float mResetTime;
    float mFailTime;
    float mCoolOff;
    unsigned char mMaxCnt;
    unsigned char mPriority;
    unsigned int hash;
};
 

extern TipClass sTipData[TTT_COUNT + 1];

class TipType
{
public:

    enum FsmState
    {
        STATE_None = 0,
        STATE_Off = 1,
        STATE_Waiting = 2,
        STATE_Active = 3,
        STATE_CoolOff = 4,
        STATE_Latched = 5,
        STATE_Resetting = 6
    };

    class FilteredInput
    {
    public:
        FilteredInput() : mTrueTime(0.0f), mFilterRate(2.0f) {}

        void Update(float fDeltaTime) { mTrueTime -= fDeltaTime; }

        void SetTrue() { mTrueTime = mFilterRate; }
        bool GetState() const { return mTrueTime > 0.0f; }

        float mTrueTime;
        float mFilterRate;
    };

    TipType() : mState(STATE_None), mShowCnt(0) {}

    void Setup(unsigned char cameraIdx, const TipClass* pClass);

    void SetShowCnt(unsigned char cnt);

    void CancelTip(bool bArg);
    void Reset();

    void CancelState();

    bool Update(float fDeltaTime, bool bArg);

private:

    bool SetState(FsmState eState, float fTime);

public:

    FilteredInput mInput;
    FsmState mState;
    unsigned char mCameraIdx;
    TipClass* mClass;
    float mTimer;
    unsigned char mShowCnt;
};
 
 
 
 
 
 

// VTABLE: SWBF2 0x007af5e8
class ToolTipsDisplay : public Thread
{
public:
    ToolTipsDisplay();
    virtual ~ToolTipsDisplay();
    virtual bool Update(float fDeltaTime);

    void Init();

    static void Create();
    static ToolTipsDisplay* sInstance;
    void SetAlwaysOn() { mAlwaysOn = true; mAlwaysOff = false; }
    void SetAuto() { mAlwaysOn = false; mAlwaysOff = false; }
    void SetAlwaysOff();

    void SetControllerDefault(bool bDefault) { mIsControllerDefault = bDefault; }

    void ShowTip(ToolTipType eType, int iParam);

    void CancelTimeoutTip(ToolTipType eType, int iParam);

    void ResetTipLatch(int iParam);

    void CancelTip(int iParam);

    bool ShouldTestForToolTipState(ToolTipType eType);

    unsigned char mCameraIndex;
    TipType mTips[TTT_COUNT];
    bool mAlwaysOn;
    bool mAlwaysOff;
    bool mIsControllerDefault;
    float mCurTipTime;
    float mNextTipTime;
};
 
 
 
 
 
 
 
