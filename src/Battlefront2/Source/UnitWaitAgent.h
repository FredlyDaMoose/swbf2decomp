#pragma once

#include <stddef.h>

#include "UnitAgent.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblVector.h"

// VTABLE: SWBF2 0x007afe14
class UnitWaitAgent : public UnitAgent
{
public:
    UnitWaitAgent(UnitController* pCtrl);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual const char* NFINAL_GetStateString(int iState) const;

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateStatePostVision(float dt);

    void SetNewSearchHeading();

public:
    PblVector3 mSearchPos;
    float mTargetTimer;

    static PblHash rttiHashUnitWaitAgent;
    static char rttiNameUnitWaitAgent[];
};
 
 
 
