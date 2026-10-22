#pragma once

#include <stddef.h>

#include "UnitAgent.h"
#include "SnipeHelper.h"
#include "PatrolHelper.h"
#include "RepairHelper.h"
#include "MineHelper.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblVector.h"

class CommandPost;
class GameObject;
class UnitController;
namespace AI { class EventInfo; }

// VTABLE: SWBF2 0x007afd40
class UnitTrooperAgent : public UnitAgent
{
public:

    enum State
    {
        STATE_START = 1,
        STATE_Goto = 2,
        STATE_Convoy = 3,
        STATE_Patrol = 4,
        STATE_MineLaying = 5,
        STATE_Aid = 6,
        STATE_Snipe = 7
    };

    UnitTrooperAgent(UnitController* pCtrl, const CommandPost* pPost);

    virtual void Kill();
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual float GetUpdateRate() const;

    virtual const char* NFINAL_GetStateString(int iState) const;

protected:
    virtual void EnterState();
    virtual void ExitState();

    virtual void UpdateStatePostVision(float dt);

    bool CheckForVehicleConvoy();

public:
    virtual int EventHandler(AI::EventInfo* pInfo);

    ConstCommandPostHandle mPost;
    PblVector3 mCPPos;
    PblVector3 mCPOffset;
    float mCPTime;
    PblVector3 mSrcPos;
    SnipeHelper mSnipeHelper;
    PatrolHelper mPatrolHelper;
    RepairHelper mRepairHelper;
    MineHelper mMineHelper;
    PblHandle<GameObject> mConvoyParent;

    static PblHash rttiHashUnitTrooperAgent;
    static char rttiNameUnitTrooperAgent[];
};

 
 
 
 
 
 
 
