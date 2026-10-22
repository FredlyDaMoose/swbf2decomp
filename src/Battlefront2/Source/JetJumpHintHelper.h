#pragma once

#include <stddef.h>

#include "Fsm.h"

class AttackPatterns;
class BaseHint;
class Controllable;
class JetJumpHint;
class UnitController;

// VTABLE: SWBF2 0x007a392c
class JetJumpHintHelper : public HelperFsm
{
public:

    enum State
    {
        STATE_START = 1,
        STATE_GOTO = 2,
        STATE_JUMPING = 3,
        STATE_FINAL = 4
    };

    JetJumpHintHelper(AttackPatterns* pOwner);

    // SYNTHETIC: SWBF2 0x0056af90
    // JetJumpHintHelper::`scalar deleting destructor'

    void HELPERInit(const JetJumpHint* pHint);

    void HELPERClean();
    const JetJumpHint* HELPERFindHint() const;

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);

    Controllable* GetMe();
    UnitController* GetCtrl();
    const Controllable* GetMe() const;
    const UnitController* GetCtrl() const;

    const JetJumpHint* FindHint() const;
    const JetJumpHint* SelectHint(int iCount, const BaseHint** ppHints) const;
    float IsHintSuitable(const JetJumpHint* pHint) const;

    JetJumpHint* mHint;
    AttackPatterns* mOwner;
    float mJumpTime;
};
  
