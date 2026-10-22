#pragma once

#include "Fsm.h"

class PblHash;

class Agent : public Fsm
{
public:
    virtual bool IsRtti(unsigned int hash) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool Update(float dt, bool b);

    static PblHash rttiHashAgent;
    static char rttiNameAgent[];
};
