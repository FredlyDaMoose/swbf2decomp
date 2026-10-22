#pragma once

#include "AIGoal.h"

namespace AI {

// VTABLE: SWBF2 0x00798f88
class DestroyGoal : public AIGoal
{
public:
    DestroyGoal(unsigned char uTeam, float fWeight, const GoalTarget& target);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    virtual void Update();

protected:

    virtual void Assign(Character* pChr);

public:
    static PblHash rttiHashDestroyGoal;
    static char rttiNameDestroyGoal[];

    GoalTarget mTarget;
};

}

 
 
