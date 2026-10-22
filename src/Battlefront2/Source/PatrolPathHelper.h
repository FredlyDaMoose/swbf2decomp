#pragma once

#include <stddef.h>

#include "Fsm.h"

class Controllable;
class UnitAgent;
class UnitController;
class PblVector3;
namespace AI { class PatrolPath; class EventInfo; }

// VTABLE: SWBF2 0x007ad64c
class PatrolPathHelper : public HelperFsm
{
public:

    enum State
    {
        STATE_START = 1,
        STATE_GotoStart = 2,
        STATE_RunPath = 3,
        STATE_FINAL = 4
    };

    PatrolPathHelper(UnitAgent* pAgent);

    // SYNTHETIC: SWBF2 0x00615e60
    // PatrolPathHelper::`scalar deleting destructor'

    bool CheckForPatrolPathsAndInit();
    void HELPERClean();
    int EventHandler(AI::EventInfo* pInfo);
    const char* NFINAL_GetStateString(int iState) const;

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);

    void DoNodeActions(const PblVector3& vPos);
    Controllable* GetMe();
    UnitController* GetCtrl();

    UnitAgent* mAgent;
    AI::PatrolPath* mPatrolPath;
};
  
