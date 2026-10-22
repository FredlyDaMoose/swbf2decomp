#pragma once

#include <stddef.h>

#include "Controller.h"
#include "PebbleFL/Common/PblList.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class Controllable;
class ProcessedInputs;

// VTABLE: SWBF2 0x007ad808
class PlayerController : public Controller
{
public:
    PlayerController(Controllable* pOwner);

    void LogVODeath(int iVictimTeam, int iKillerTeam);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Update(float dt);

    float GetMouseBuildup();

    void SetMouseBuildup(float fBuildup) { mMouseTurnBuildup = fBuildup; }

    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static MemoryPool sMemoryPool;

    static PblHash rttiHashPlayerController;
    static char rttiNamePlayerController[];

protected:
    void DeployCargo();

    void CheckForAutoFollow(float dt);
    void CheckForVisibleCriticalPts();

public:
    PblList<PlayerController>::Node mManagerNode;
    ProcessedInputs* mJoystick;
    float mMouseTurnBuildup;
    bool mLanding;
    float mAutoAimScale;
    float mNextCritHitTime;
    float mEnemyDeaths[4];
    float mAllyDeaths[4];
    float mTeamKills[4];
    float mAutoFollowTime;
    bool mIsFireButtonDebounced;

    bool mUseButtonDebounced;
public:
    static int sEnabled;
};

PlayerController* PlayerControllerPtr(Controller* pCtrl);
const PlayerController* PlayerControllerPtr(const Controller* pCtrl);
PlayerController* PlayerControllerPtr(Controllable* pControllable);
const PlayerController* PlayerControllerPtr(const Controllable* pControllable);

 
 
 
 
 
