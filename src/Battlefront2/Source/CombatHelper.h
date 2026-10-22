#pragma once

#include <stddef.h>

#include "Fsm.h"
#include "UnitAttackPatterns.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"

class AILowLevel;
class AttackHelper;
class Controllable;
class GameObject;
class UnitAgent;
class UnitCombatInfo;
class UnitController;
namespace AI { class EventInfo; }

// VTABLE: SWBF2 0x007974e8
class CombatHelper : public HelperFsm
{
public:

    enum State
    {
        STATE_START = 1,
        STATE_Turn = 2,
        STATE_LookAround = 3,
        STATE_Investigate = 4,
        STATE_Engage = 5,
        STATE_FleeEvent = 6,
        STATE_ShootDeadBody = 7,
        STATE_FINAL = 8
    };

    CombatHelper(UnitAgent* pOwner);
    virtual ~CombatHelper();
    virtual void Kill();

    void Clean();
    void Init(const UnitCombatInfo& info);
    int EventHandler(AI::EventInfo* pInfo);

    UnitController* GetCtrl();
    Controllable* GetMe();
    AILowLevel& GetLowLevel();
    AttackHelper& GetAttackHelper();
    float GetUpdateRate() const;

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);

    void SetRandomHeading();
    bool DeadBodyCheck();

public:
    UnitAgent* mOwner;
    AttackPatterns mAttackPatternHelper;
    AttackPatterns::State mAttackPattern;
    PblHandle<GameObject const> mTargetHandle;
    float mEnterStateTime;
    PblVector3 mTargetPos;
    bool mWaiting;
    float mWaitOverTime;
    float mLookAngle;
    int mLookCount;
    float mPauseTime;
    PblVector3 mFleeSrc;
    float mFleeTime;
    bool mFleeAllowRoll;
    unsigned char mDeadBodyWeaponIdx;
    bool mDeflect;
    float mDeflectTimer;
};

 
 
 
 
 
 
 
 
 
 
