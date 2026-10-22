#pragma once

#include "UnitController.h"

// VTABLE: SWBF2 0x007afae0
class UnitFlyController : public UnitController
{
public:
    UnitFlyController(Controllable* pOwner);

    // SYNTHETIC: SWBF2 0x00667d40
    // UnitFlyController::`scalar deleting destructor'

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    static PblHash rttiHashUnitFlyController;
    static char rttiNameUnitFlyController[];

protected:
    virtual void ApplyNextState();
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);
};

 
