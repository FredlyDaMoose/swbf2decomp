#pragma once

#include "NavigatorGraphFollower.h"
#include "PebbleFL/Common/PblHandle.h"

class GameObject;

// VTABLE: SWBF2 0x007aa7e4
class NavigatorFollowTarget : public NavigatorGraphFollower
{
public:

    enum
    {
        STATE_FOLLOWCRUMBS = 3,
        STATE_FINAL = 4
    };

    enum
    {
        NUM_CRUMBS = 32
    };

    static PblHash rttiHashNavigatorFollowTarget;
    static char rttiNameNavigatorFollowTarget[];

    NavigatorFollowTarget(AILowLevel* pOwner, const GameObject* pTarget,
                          const Obstacle** ppIgnore, int iIgnoreCnt,
                          const PblVector3& followOffset);
    virtual ~NavigatorFollowTarget();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void ReceivePathResult(AIPath* pPath);
    virtual const PblVector3& GetGlobalDest() const;

    bool IsEqual(AILowLevel* pOwner, const GameObject* pTarget, const PblVector3& followOffset);

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);

private:
    bool LookForCrumbs();
    void UpdateTarget();
    void UpdateFollowCrumbs();

public:
    PblHandle<const GameObject> mFollowTarget;
    PblVector3 mFollowOffset;
    ConnectivityGraphFollower* mGraphFollower;
    PblVector3 mCrumbBuffer[NUM_CRUMBS];
    int mCrumbHead;
    int mCrumbTail;
    float mLastShortcutCheck;
};

NavigatorFollowTarget* NavigatorFollowTargetPtr(Navigator* pNavigator);

 
 
 
 
