#pragma once

#include <stddef.h>

#include "Fsm.h"
#include "PebbleFL/Common/PblHandle.h"

class BaseHint;
class Controllable;
class GameObject;
class UnitAgent;
class UnitController;
class VehicleCoverHint;

// VTABLE: SWBF2 0x007afff8
class VehicleCoverHelper : public HelperFsm
{
public:

    enum State
    {
        STATE_START = 1,
        STATE_GOTO = 2,
        STATE_COVERED = 3,
        STATE_POP_UP = 4,
        STATE_STEP_OUT = 5,
        STATE_FINAL = 6
    };

    VehicleCoverHelper(UnitAgent* pOwner);

    virtual ~VehicleCoverHelper() {}

    // SYNTHETIC: SWBF2 0x0066bcf0
    // VehicleCoverHelper::`scalar deleting destructor'

    bool CheckForHintAndInit(const GameObject* pTarget);

    void HELPERClean();
    bool HELPERFaceTarget() const;

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);

    const Controllable* GetMe() const;
    Controllable* GetMe();
    UnitController* GetCtrl();
    const UnitController* GetCtrl() const;
    float GetMaxHintDist() const;
    const VehicleCoverHint* FindHint() const;
    const VehicleCoverHint* SelectHint(int iCount, const BaseHint** ppHints) const;
    float IsHintSuitable_Combat(const VehicleCoverHint* pHint) const;
    float IsHintSuitable(const VehicleCoverHint* pHint) const;

    VehicleCoverHint* mHint;
    UnitAgent* mOwner;
    PblHandle<GameObject const> mVehicleCoverTarget;
    float mTimer;
    float mNextHintCheck;
    float mNewHintCheck;
};
  
