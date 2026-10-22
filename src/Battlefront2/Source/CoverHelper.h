#pragma once

#include <stddef.h>

#include "Fsm.h"
#include "Controllable.h"
#include "PebbleFL/Common/PblHandle.h"

class BaseHint;
class CoverHint;
class Controllable;
class UnitController;
class GameObject;
class UnitAgent;

enum CoverType
{
    COVERTYPE_Combat = 0,
    COVERTYPE_ForwardCover = 1,
    COVERTYPE_Idle = 2
};

// VTABLE: SWBF2 0x00798d1c
class CoverHelper : public HelperFsm
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

    CoverHelper(UnitAgent* pOwner);

    virtual ~CoverHelper() {}

    void HELPERClean();
    bool InitAtHint(CoverType eType, const BaseHint* pHint);

    bool CheckForHintAndInit(CoverType eType, const GameObject* pTarget);
    ControllableHeight HELPERGetHeight() const;
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
    const CoverHint* FindHint() const;
    const CoverHint* SelectHint(int iCount, const BaseHint** ppHints) const;
    float IsHintSuitable(const BaseHint* pHint) const;
    float IsHintSuitable_Combat(const BaseHint* pHint) const;

    float IsHintSuitable_ForwardCover(const BaseHint* pHint) const;
    float IsHintSuitable_Idle(const BaseHint* pHint) const;

    BaseHint* mHint;
    UnitAgent* mOwner;
    CoverType mCoverType;
    PblHandle<GameObject const> mCoverTarget;
    ControllableHeight mStance;
    float mTimer;
    float mNextHintCheck;
    float mNewHintCheck;
};
  
