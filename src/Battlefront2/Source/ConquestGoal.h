#pragma once

#include "AIGoal.h"

namespace AI {

// VTABLE: SWBF2 0x00798a58
class ConquestGoal : public AIGoal
{
public:
    ConquestGoal(unsigned char uTeam, float fWeight);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    virtual void Update();

protected:

    virtual void Assign(Character* pChr);

    static void sAssign(Character* pChr, const CommandPost* pPost);
    static void sIssueCmd(Character* pChr, const CommandPost* pPost);
    static const CommandPost* sGenerateCP(Character* pChr, const CommandPost* pPost, bool bSecondPass);
    static bool sIsPilotingVehicle(Character* pChr);
    static bool sIsPassenger(Character* pChr);
    static float sGetImportance(const CommandPost* pPost, int iTeam);

public:
    static PblHash rttiHashConquestGoal;
    static char rttiNameConquestGoal[];
};

}

 
