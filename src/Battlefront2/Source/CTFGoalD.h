#pragma once

#include <stddef.h>

#include "AIGoal.h"
#include "PebbleFL/Common/PblHandle.h"

class GameObject;

namespace AI {

struct GoalTarget;

// VTABLE: SWBF2 0x00798d78
class CTFGoalD : public AIGoal
{
public:

    CTFGoalD(unsigned char uTeam, float fWeight, const GoalTarget& target);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

protected:

    virtual void Assign(Character* pChr);

public:
    static PblHash rttiHashCTFGoalD;
    static char rttiNameCTFGoalD[];

    PblHandle<const GameObject> mFlag;
};

}

 
 
