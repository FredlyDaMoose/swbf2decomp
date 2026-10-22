#pragma once

#include "UnitController.h"

class UnitMobileController : public UnitController
{
public:

    UnitMobileController(Controllable* pOwner);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    static PblHash rttiHashUnitMobileController;
    static char rttiNameUnitMobileController[];

protected:

    virtual void ApplyNextState();
};
