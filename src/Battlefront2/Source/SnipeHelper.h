#pragma once

#include <stddef.h>

#include "Fsm.h"
#include "Controllable.h"
#include "PebbleFL/Common/PblVector.h"

class AttackHelper;
class GameObject;
class SnipeHint;
class Team;
class UnitAgent;
class UnitController;

// VTABLE: SWBF2 0x007ae688
class SnipeHelper : public HelperFsm
{
public:

    enum State
    {
        STATE_START = 1,
        STATE_GOTO = 2,
        STATE_SNIPE = 3,
        STATE_FINAL = 4
    };

    SnipeHelper(UnitAgent* pOwner);

    bool LookForSnipeHintAndInit();

    void HELPERClean();

    void ResetHintCheckTime() { mNextHintCheckTime = 0.0f; }

    bool HELPERGrenadeEventInterrupt() const;
    bool HELPERDamagedInterrupt(bool b, const PblVector3& vPos) const;
    bool HELPERSeeEnemyInterrupt(const GameObject* pEnemy) const;
    bool HELPERHearEnemyInterrupt(const GameObject* pEnemy) const;
    bool HELPERVehicleNearbyInterrupt(const GameObject* pVehicle) const;

    bool HELPERGetOverrideDefaultFacePos() const;
    bool HELPERGetOverrideFaceTarget() const;
    const PblVector3& HELPERGetDefaultFacePos() const;
    ControllableHeight HELPERGetHeight() const;
    const SnipeHint* HELPERFindSnipeHint() const;

protected:

    const Controllable* GetMe() const;
    Controllable* GetMe();
    UnitController* GetCtrl();
    const AttackHelper* GetAttackHelper() const;

    void HELPERInit(const SnipeHint* pHint);
    const SnipeHint* FindSnipeHint() const;
    bool IsWithinSnipeFOV(const PblVector3& vPos) const;
    bool IsHintSuitable(const SnipeHint* pHint, const Team* pTeam) const;

    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);

    UnitAgent* mOwner;
    SnipeHint* mHint;
    bool mOverrideDefaultFacePos;
    PblVector3 mDefaultFacePos;
    ControllableHeight mHeight;
    bool mOverrideFaceTarget;
    float mCheckTime;
    float mSnipeTimer;
    float mNextHintCheckTime;
};
  
