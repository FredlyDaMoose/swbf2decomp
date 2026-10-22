#pragma once

#include <stddef.h>

#include "UnitAgent.h"
#include "EventInfo.h"
#include "CmdInfo.h"
#include "CombatHelper.h"

// VTABLE: SWBF2 0x007af8f8
class UnitDeathmatchAgent : public UnitAgent
{
public:

    enum
    {
        STATE_START = 1,
        STATE_RunPoint = 2,
        STATE_RunEnemy = 3,
        STATE_Combat = 4,
        STATE_Stationary = 5,
        STATE_FINAL = 6
    };

    static PblHash rttiHashUnitDeathmatchAgent;
    static char rttiNameUnitDeathmatchAgent[];

    UnitDeathmatchAgent(UnitController* pCtrl);

    virtual void Kill();
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    int EventHandler(AI::EventInfo* pInfo);

    virtual const char* NFINAL_GetStateString(int iState) const;
protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateStatePostVision(float dt);
    void DoJetJumpStuff();
public:

    CmdInfo mCombatCmd;
    CombatHelper mCombatHelper;
    float mNextJumpTime;
};

 
 
 
 
