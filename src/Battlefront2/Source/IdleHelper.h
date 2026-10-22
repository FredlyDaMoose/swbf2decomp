#pragma once

#include <stddef.h>

#include "Fsm.h"
#include "CoverHelper.h"

class AILowLevel;
class BaseHint;
class Controllable;
class UnitAgent;
class UnitController;

enum AllowedIdles
{
    IH_Stationary = 1,
    IH_Cover = 2,
    IH_ALL = -1
};

// VTABLE: SWBF2 0x007a38b4
class IdleHelper : public HelperFsm
{
public:

    enum State
    {
        STATE_START = 1,
        STATE_Stationary = 2,
        STATE_Cover = 3,
        STATE_VehicleIdle = 4,
        STATE_FINAL = 5
    };

    IdleHelper(UnitAgent* pOwner);

    virtual ~IdleHelper() {}

    // SYNTHETIC: SWBF2 0x00569940
    // IdleHelper::`scalar deleting destructor'

    virtual void Kill();
    void Init(unsigned int uAllowedIdles, const BaseHint* pHint);

    void Clean();

    static void StartStationaryIdle(UnitController* pCtrl, float& fIdleTime, float& fWanderSuppressTime);

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);

public:

    UnitAgent* mOwner;
    CoverHelper mCoverHelper;
    float mIdleTime;
    float mWanderSuppressTime;
    unsigned int mAllowedIdles;
    BaseHint* mHintNode;
};
  
 
 
