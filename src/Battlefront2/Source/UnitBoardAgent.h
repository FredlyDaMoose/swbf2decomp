#pragma once

#include <stddef.h>

#include "UnitAgent.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblHashTable.h"

class GameObject;

// VTABLE: SWBF2 0x007af724
class UnitBoardAgent : public UnitAgent
{
public:
    UnitBoardAgent(UnitController* pCtrl, GameObject* pVehicle, int iVOResponseHandle);

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

public:
    PblHandle<GameObject> mVehicle;
    int mVOResponseHandle;
    float mStateEnterTime;

    static PblHash rttiHashUnitBoardAgent;
    static char rttiNameUnitBoardAgent[];
};
 
 
 
 
