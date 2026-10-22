#pragma once

#include <stddef.h>

#include "AIGoal.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"

class GameObject;

namespace AI {

// VTABLE: SWBF2 0x00798d48
class CTFGoal : public AIGoal
{
public:

    CTFGoal(unsigned char uTeam, float fWeight, const GoalTarget& target);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    virtual void Update();

protected:

    virtual void Assign(Character* pChr);

public:
    static PblHash rttiHashCTFGoal;
    static char rttiNameCTFGoal[];

    PblHandle<const GameObject> mFlag;
    PblVector3 mCapturePos;
};

}

 
 
 
