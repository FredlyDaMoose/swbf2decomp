#pragma once

#include <stddef.h>

#include "Fsm.h"
#include "PebbleFL/Common/PblVector.h"

class Controllable;
class GameObject;
class MinefieldHint;
class UnitAgent;

// VTABLE: SWBF2 0x007aa508
class MineHelper : public HelperFsm
{
public:

    enum State
    {
        STATE_START = 1,
        STATE_GOTO = 2,
        STATE_LAYMINE = 3,
        STATE_FINAL = 4
    };

    MineHelper(UnitAgent* pAgent);

    // SYNTHETIC: SWBF2 0x005a3c30
    // MineHelper::`scalar deleting destructor'

    bool CheckForHintsAndInit();

    void HELPERClean();
    bool HELPERIsValidMineLayer(const Controllable* pMe) const;
    bool HELPERIsMinePosNearby(const MinefieldHint** ppHint, PblVector3* pPos) const;

    bool HELPERGrenadeEventInterrupt() const;
    bool HELPERDamagedInterrupt(bool b, const PblVector3& vPos) const;
    bool HELPERSeeEnemyInterrupt(const GameObject* pEnemy) const;
    bool HELPERHearEnemyInterrupt(const GameObject* pEnemy) const;
    bool HELPERVehicleNearbyInterrupt(const GameObject* pVehicle) const;

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);

    bool SetMineWpn(bool* pbSwitching);
    bool FindMinePos(const MinefieldHint* pHint, PblVector3* pPos) const;
    bool IsValidMineLayer(const Controllable* pMe) const;

    const Controllable* GetMe() const;
    Controllable* GetMe();

public:
    UnitAgent* mAgent;
    float mNextMineCheckTime;
    MinefieldHint* mHint;
    PblVector3 mMinePos;
    bool mLaidMine;
    PblVector3 mFacePos;
    float mMineTimer;
};
  
 
 
