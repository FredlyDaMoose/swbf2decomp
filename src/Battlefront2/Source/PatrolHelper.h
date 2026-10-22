#pragma once

#include <stddef.h>

#include "Fsm.h"
#include "CommandPost.h"
#include "Controllable.h"
#include "PebbleFL/Common/PblVector.h"

class BaseHint;
class Controllable;
class GameObject;
class PatrolHint;
class UnitAgent;
class UnitController;

// VTABLE: SWBF2 0x007ad61c
class PatrolHelper : public HelperFsm
{
public:

    enum State
    {
        STATE_START = 1,
        STATE_ACQUIRE = 2,
        STATE_GOTO_HINT = 3,
        STATE_GOTO_BASE = 4,
        STATE_GUARD_HINT = 5,
        STATE_GUARD_BASE = 6,
        STATE_FINAL = 7
    };

    PatrolHelper(UnitAgent* pAgent);

    // SYNTHETIC: SWBF2 0x00614e20
    // PatrolHelper::`scalar deleting destructor'

    void HELPERInit(const ConstCommandPostHandle& post);
    void HELPERClean();
    bool HELPERGetOverrideDefaultFacePos() const;
    const PblVector3& HELPERGetDefaultFacePos() const;
    ControllableHeight HELPERGetHeight() const;
    bool HELPERGrenadeEventInterrupt() const;
    bool HELPERDamagedInterrupt(bool b, const PblVector3& vPos) const;
    bool HELPERSeeEnemyInterrupt(const GameObject* pEnemy) const;
    bool HELPERHearEnemyInterrupt(const GameObject* pEnemy) const;
    bool HELPERVehicleNearbyInterrupt(const GameObject* pVehicle) const;
    bool HELPERWaitSecondaryInterrupt() const;
    const char* NFINAL_GetStateString(int iState) const;

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);

    const PatrolHint* SelectHint(int i, const BaseHint** ppHint);
    const Controllable* GetMe() const;
    Controllable* GetMe();
    UnitController* GetCtrl();
    const UnitController* GetCtrl() const;

public:
    UnitAgent* mAgent;
    ConstCommandPostHandle mPost;
    PatrolHint* mHint;
    float mBeginTime;
    bool mOverrideDefaultFacePos;
    PblVector3 mDefaultFacePos;
    ControllableHeight mHeight;
    float mNextPatrolTime;
};
  
 
 
