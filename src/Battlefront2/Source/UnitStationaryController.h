#pragma once

#include "UnitController.h"

// VTABLE: SWBF2 0x007afccc
class UnitStationaryController : public UnitController
{
public:
    UnitStationaryController(Controllable* pOwner);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    static PblHash rttiHashUnitStationaryController;
    static char rttiNameUnitStationaryController[];

protected:
    virtual void ApplyNextState();
};

 
