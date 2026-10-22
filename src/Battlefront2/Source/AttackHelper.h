#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"

class Controllable;
class GameObject;
class UnitAgent;

class AttackHelper
{

    friend class CombatHelper;

public:
    AttackHelper(UnitAgent* pAgent);

    void Reset();
    void Update();
    bool UpdateTarget();
    void Aim();
    void Fire();
    void SelectBestWeapon();
    void SetVisibleTarget(GameObject const* pTarget);
    void SetWaitDelay(float fDelay);
    void ResetWaitDelay();
    void AllowEnemyUnseen(bool bAllow);

    bool HasTarget() const;
    GameObject const* GetVisibleTarget() const;
    PblVector3 const& GetLastSeenTargetPos() const;
    float GetLastTimeSeen() const;

    friend class UnitController;

    friend class AttackPatterns;

protected:
    Controllable const* GetMe() const;
    Controllable* GetMe();
    GameObject const* GetTarget() const;
    bool TargetIsHiding() const;
    void ResetWaitOverTime();
    void SetTarget(GameObject const* pTarget);
    void SetTargetInfo(GameObject const* pTarget, PblVector3 const& vPos);

public:
    PblHandle<GameObject const> mTargetHandle;
    bool mTargetVisible;
    PblVector3 mLastKnownTargetPos;
    float mLastTimeSeen;
    float mCurPriority;
    float mWaitDelay;
    bool mAllowEnemyUnseen;
    UnitAgent* mAgent;
    bool mWaitingForTarget;
    float mWaitOverTime;
};

 
 
 
 
 
 
 
 
 
 
