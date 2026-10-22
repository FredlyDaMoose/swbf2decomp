#pragma once

#include <stddef.h>

#include "UnitAgent.h"
#include "UnitCmdInfo.h"
#include "CombatHelper.h"

namespace AI { class EventInfo; }

class UnitCombatAgent : public UnitAgent
{
public:
    static PblHash rttiHashUnitCombatAgent;
    static char rttiNameUnitCombatAgent[];

    enum
    {
        STATE_START = 1,
        STATE_Combat = 2,
        STATE_FINAL = 3
    };

    UnitCombatAgent(UnitController* pCtrl, const UnitCombatInfo& info);

    int EventHandler(AI::EventInfo* pInfo);

    virtual void Kill();
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool ShouldQueueCurCmd(const UnitCmdInfo& cmd) const;
    virtual float GetUpdateRate() const;

    virtual const char* NFINAL_GetStateString(int iState) const;
protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateStatePostVision(float dt);
public:

    UnitCombatInfo mCombatInfo;
    CombatHelper mCombatHelper;
};

 
 
