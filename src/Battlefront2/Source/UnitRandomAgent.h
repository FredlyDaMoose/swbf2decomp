#pragma once

#include <stddef.h>

#include "UnitAgent.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblVector.h"

// VTABLE: SWBF2 0x007afc58
class UnitRandomAgent : public UnitAgent
{
public:
    UnitRandomAgent(UnitController* pCtrl);

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

    bool IsPointValid(const PblVector3& vPoint);
    void PickRandomDest();

public:
    PblVector3 mStartPos;
    float mRandTime;

    static PblHash rttiHashUnitRandomAgent;
    static char rttiNameUnitRandomAgent[];
};
 
 
 
