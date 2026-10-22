#pragma once

#include <stddef.h>

#include "UnitAgent.h"
#include "AIGoal.h"
#include "CmdInfo.h"
#include "CombatHelper.h"
#include "DefendHelper.h"

// VTABLE: SWBF2 0x007af960
class UnitDefendAgent : public UnitAgent
{
public:

    UnitDefendAgent(UnitController* pCtrl, const AI::GoalTarget& target);
    virtual ~UnitDefendAgent();
    virtual void Kill();
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual float GetUpdateRate() const;
    virtual const char* NFINAL_GetStateString(int iState) const;
    virtual int EventHandler(AI::EventInfo* pInfo);

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateStatePostVision(float dt);

public:
    DefendHelper mDefendHelper;
    CmdInfo mCombatCmd;
    CombatHelper mCombatHelper;
    AI::GoalTarget mTarget;
    bool mMaintainVisualContact;

    static PblHash rttiHashUnitDefendAgent;
    static char rttiNameUnitDefendAgent[];
};

 
 
 
 
 
