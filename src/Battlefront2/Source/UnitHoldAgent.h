#pragma once

#include <stddef.h>

#include "UnitAgent.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblVector.h"

// VTABLE: SWBF2 0x007afb78
class UnitHoldAgent : public UnitAgent
{
public:
    UnitHoldAgent(UnitController* pCtrl);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool ShouldQueueCurCmd(const UnitCmdInfo& cmd) const;
    virtual const char* NFINAL_GetStateString(int iState) const;
    virtual int EventHandler(AI::EventInfo* pInfo);

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateStatePostVision(float dt);

    void SetNewSearchHeading();

public:
    float mInsubValue;
    PblVector3 mSearchPos;

    static PblHash rttiHashUnitHoldAgent;
    static char rttiNameUnitHoldAgent[];
};
 
 
 
