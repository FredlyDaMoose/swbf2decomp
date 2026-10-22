#pragma once

#include <stddef.h>

#include "UnitAgent.h"
#include "PebbleFL/Common/PblHashTable.h"

class GameObject;

// VTABLE: SWBF2 0x007afe84
class UnitWaitSecondaryAgent : public UnitAgent
{
public:
    UnitWaitSecondaryAgent(UnitController* pCtrl, float fWaitTime, bool bArg);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool ShouldQueueCurCmd(const UnitCmdInfo& cmd) const;
    virtual int EventHandler(AI::EventInfo* pInfo);
    virtual const char* NFINAL_GetStateString(int iState) const;

    void TakeoffNow() { mWaitUntilMissionTime = 0.0f; }

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateStatePostVision(float dt);

    GameObject* IsPlayerLookingAtVehicle();

public:
    float mWaitUntilMissionTime;
    float mPlayerWaitTimeout;
    float mImpatientSoundTime;

    static PblHash rttiHashUnitWaitSecondaryAgent;
    static char rttiNameUnitWaitSecondaryAgent[];
};
 
 
 
