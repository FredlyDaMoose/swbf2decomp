#pragma once

#include "Navigator.h"

class Obstacle;

// VTABLE: SWBF2 0x007aa88c
class NavigatorPathFollower : public Navigator
{
public:

    enum
    {
        STATE_WAITFORPATH = 2,
        STATE_FOLLOWPATH = 3,
        STATE_FINAL = 4
    };

    static PblHash rttiHashNavigatorPathFollower;
    static char rttiNameNavigatorPathFollower[];

    NavigatorPathFollower(AILowLevel* pOwner, bool bDoCallbacksOnPts,
                          const Obstacle** ppIgnore, int iIgnoreCnt,
                          bool bRollIntoDest);
    NavigatorPathFollower(AILowLevel* pOwner, bool bDoCallbacksOnPts,
                          const Obstacle** ppIgnore, int iIgnoreCnt,
                          bool bRollIntoDest, AIPath* pPath);
    virtual ~NavigatorPathFollower();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void ReceivePathResult(AIPath* pPath);

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);

public:
    int mIgnoreCnt;
    Obstacle* mIgnore[2];
    bool mRollIntoDest;
    bool mDoCallbacksOnPts;
};

 
