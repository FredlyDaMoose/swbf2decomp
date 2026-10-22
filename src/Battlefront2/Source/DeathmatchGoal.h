#pragma once

#include "AIGoal.h"

namespace AI {

// VTABLE: SWBF2 0x00798ebc
class DeathmatchGoal : public AIGoal
{
public:
    DeathmatchGoal(unsigned char uTeam, float fWeight);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

protected:

    virtual void Assign(Character* pChr);

public:
    static PblHash rttiHashDeathmatchGoal;
    static char rttiNameDeathmatchGoal[];
};

}

 
