#pragma once

#include <stddef.h>

#include "Fsm.h"
#include "AIGoal.h"
#include "SnipeHelper.h"
#include "RepairHelper.h"
#include "PatrolPathHelper.h"
#include "IdleHelper.h"
#include "MineHelper.h"
#include "PebbleFL/Common/PblVector.h"

class AILowLevel;
class BaseHint;
class Controllable;
class UnitAgent;
class UnitController;
#include "EventInfo.h"

class DefendHelper : public HelperFsm
{
public:

    enum State
    {
        STATE_START = 1,
        STATE_Goto = 2,
        STATE_Idle = 3,
        STATE_WaitForSpawn = 4,
        STATE_GotoFortification = 5,
        STATE_DoPatrol = 6,
        STATE_DoPatrolPath = 7,
        STATE_Snipe = 8,
        STATE_Repair = 9,
        STATE_Mine = 10,
        STATE_FINAL = 11
    };

    DefendHelper(UnitAgent* pOwner);

    virtual ~DefendHelper();

    // SYNTHETIC: SWBF2 0x0048b650
    // DefendHelper::`scalar deleting destructor'

    virtual void Kill();

    void Clean();
    void Init(const AI::GoalTarget& target);
    float GetUpdateRate() const;
    AI::EVTResult EventHandler(AI::EventInfo* pInfo);

    UnitController* GetCtrl();
    Controllable* GetMe();
    AILowLevel& GetLowLevel();

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);

    bool CheckTargetPosition(const PblVector3& vPos);
    bool ChooseHint(const BaseHint** ppHints, int iCount, const PblVector3& vPos);
    bool CheckEnterFortification(float dt, const PblVector3& vPos);
    bool CheckExitFortification(const PblVector3& vPos);

public:
    UnitAgent* mOwner;
    AI::GoalTarget mTarget;
    SnipeHelper mSnipeHelper;
    RepairHelper mRepairHelper;
    PatrolPathHelper mPatrolPathHelper;
    IdleHelper mIdleHelper;
    MineHelper mMineHelper;
    PblVector3 mLastSeekPos;
    PblVector3 mLastFortificationPos;
    float mFortificationTime;
    BaseHint* mFortificationHint;
};

 
 
 
 
 
 
 
 
 
