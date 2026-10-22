#pragma once

#include <stddef.h>

#include "UnitAgent.h"
#include "EventInfo.h"
#include "PebbleFL/Common/PblHashTable.h"

class UnitUnBoardAgent : public UnitAgent
{
public:
    UnitUnBoardAgent(UnitController* pCtrl);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool ShouldQueueCurCmd(const UnitCmdInfo& cmd) const;
    virtual int EventHandler(AI::EventInfo* pInfo);
    virtual const char* NFINAL_GetStateString(int iState) const;

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateStatePostVision(float dt);

public:
    static PblHash rttiHashUnitUnBoardAgent;
    static char rttiNameUnitUnBoardAgent[];
};
 
