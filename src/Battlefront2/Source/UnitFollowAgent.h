#pragma once

#include <stddef.h>

#include "UnitAgent.h"
#include "CoverHelper.h"
#include "CmdInfo.h"
#include "CombatHelper.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblHashTable.h"

class GameObject;

// VTABLE: SWBF2 0x007afb10
class UnitFollowAgent : public UnitAgent
{
public:

    UnitFollowAgent(UnitController* pCtrl, PblHandle<GameObject const> hTarget, bool bTightFollow);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Kill();
    virtual bool ShouldQueueCurCmd(const UnitCmdInfo& cmd) const;
    virtual float GetUpdateRate() const;
    virtual const char* NFINAL_GetStateString(int iState) const;
    virtual int EventHandler(AI::EventInfo* pInfo);

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateStatePostVision(float dt);

    bool CheckForForwardHintNodes();
    bool IsInLooseRange();

public:
    CoverHelper mCoverHelper;
    CmdInfo mCombatCmd;
    CombatHelper mCombatHelper;
    PblHandle<GameObject const> mTarget;
    bool mTightFollowMode;

    static PblHash rttiHashUnitFollowAgent;
    static char rttiNameUnitFollowAgent[];
};

 
 
 
 
 
 
