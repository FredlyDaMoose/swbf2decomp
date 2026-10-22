#pragma once

#include "AIGoal.h"

namespace AI {

// VTABLE: SWBF2 0x00798f34
class DefendGoal : public AIGoal
{
public:
    DefendGoal(unsigned char uTeam, float fWeight, const GoalTarget& target);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

protected:

    virtual void Assign(Character* pChr);

public:
    static PblHash rttiHashDefendGoal;
    static char rttiNameDefendGoal[];

    GoalTarget mTarget;
};

}

 
 
