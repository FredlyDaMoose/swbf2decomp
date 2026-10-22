#pragma once

#include <stddef.h>

#include "UnitAgent.h"
#include "CmdInfo.h"
#include "CombatHelper.h"
#include "DefendHelper.h"
#include "PebbleFL/Common/PblHandle.h"

class FlagItem;
int GetFormationType(const UnitController* pCtrl);

class UnitCTFOffenseAgent : public UnitAgent
{
public:
    static PblHash rttiHashUnitCTFOffenseAgent;
    static char rttiNameUnitCTFOffenseAgent[];

    UnitCTFOffenseAgent(UnitController* pCtrl, const FlagItem* pFlag, const PblVector3& vPos);
    virtual ~UnitCTFOffenseAgent();
    virtual void Kill();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual float GetUpdateRate() const;

    virtual const char* NFINAL_GetStateString(int iState) const;
    virtual int EventHandler(AI::EventInfo* pInfo);

    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateStatePostVision(float dt);

    bool IsFlagNeutral() const;
    bool IsFlagInMyHands() const;
    bool IsFlagInAlliedHandsButNotMine() const;
    bool IsFlagInEnemyHands() const;
    void FaceTargetIfClose(ControllableHeight eHeight);

    CmdInfo mCombatCmd;
    CombatHelper mCombatHelper;
    DefendHelper mDefendHelper;
    PblHandle<const GameObject> mFlag;
    PblVector3 mCapturePos;
    bool mPathing;
    float mPathTime;
    float mDefendCarrierOffset;
};

 
 
 
 
 
