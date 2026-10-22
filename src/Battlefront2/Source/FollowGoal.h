#pragma once

#include "AIGoal.h"

namespace AI {

// VTABLE: SWBF2 0x0079f2cc
class FollowGoal : public AIGoal
{
public:
    FollowGoal(unsigned char uTeam, float fWeight, const GoalTarget& target);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    virtual void Update();

protected:

    virtual void Assign(Character* pChr);

public:
    static PblHash rttiHashFollowGoal;
    static char rttiNameFollowGoal[];

    GoalTarget mTarget;
};

}

 
 
