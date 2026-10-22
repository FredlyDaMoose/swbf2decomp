#pragma once

#include "Fsm.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class AILowLevel;
class AIPath;
class GameObject;
class Obstacle;
class PathFollower;
class PblMatrix;
class PblVector3;

// VTABLE: SWBF2 0x007aa790
class Navigator : public Fsm
{
public:

    enum
    {
        STATE_START = 1
    };

    static void* operator new(unsigned int uiSize);
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }
    static MemoryPool sMemoryPool;

    static PblHash rttiHashNavigator;
    static char rttiNameNavigator[];

    virtual ~Navigator();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void ReceivePathResult(AIPath* pPath);
    virtual const PblVector3& GetGlobalDest() const;
    virtual const PblVector3& GetLocalDest() const;

    PathFollower* GetPathFollower();
    const PathFollower* GetPathFollower() const;
    bool IsStuck() const;
    bool IsFailedPathSearch() const;
    void SetFailedPathSearch();
    void Update(float dt);
    void UpdateSkip();

protected:
    Navigator(AILowLevel* pOwner);

    const PblMatrix& GetMatrix() const;

    GameObject* GetGameObject();
    const GameObject* GetGameObject() const;
    float GetMyRadius() const;
    int GetMyProhibit() const;
    const Obstacle* GetMyBarrier() const;
    bool AttemptWallFollow(float fStep, bool bRight);
    bool LookForPowerUps(float fRadius);
    bool LookForCoverNodes();

    AILowLevel* mOwner;
    PathFollower* mPathFollower;
    float mWallFollowTime;
    float mPowerUpTime;
    float mCoverNodeTime;
    bool mFailedPathSearch;
};

 
