#pragma once

#include "Navigator.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"

class GameObject;
class Obstacle;

// VTABLE: SWBF2 0x007aa8d8
class NavigatorSlide : public Navigator
{
public:

    enum
    {
        SLIDE_NONE = 0,
        SLIDE_LEFT = 1,
        SLIDE_RIGHT = 2
    };
    enum
    {
        STATE_InfantryToPosition = 2,
        STATE_VehicleToPosition = 3,
        STATE_InfantryToObject = 4,
        STATE_FINAL = 5
    };

    static PblHash rttiHashNavigatorSlide;
    static char rttiNameNavigatorSlide[];

    NavigatorSlide(AILowLevel* pOwner, const PblVector3& globalDest,
                   const Obstacle** ppIgnore, int iIgnoreCnt);
    NavigatorSlide(AILowLevel* pOwner, const GameObject* pSlideObj, float fOffset,
                   bool bUseAttackHelper, const Obstacle** ppIgnore, int iIgnoreCnt);
    virtual ~NavigatorSlide();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;

    bool IsEqual(AILowLevel* pOwner, const PblVector3& globalDest);
    bool IsEqual(AILowLevel* pOwner, const GameObject* pSlideObj, float fOffset,
                 bool bUseAttackHelper);

protected:
    virtual void EnterState();
    virtual void ExitState();
    virtual void UpdateState(float dt);

private:
    PblVector3 GetBouncePosition() const;
    bool GetObjectSlideToPosition(PblVector3* pPos) const;
    void NextInfantrySlidePos(const PblVector3& dest);
    void NextVehicleSlidePos();
    void UpdateInfantrySlideObject();
    void UpdateInfantrySlidePos();
    void UpdateVehicleSlidePos();

public:
    int mIgnoreCnt;
    Obstacle* mIgnore[2];
    PblVector3 mGlobalDest;
    PblHandle<const GameObject> mSlideObj;
    bool mUseAttackHelper;
    float mOffset;
    bool mBounceAtLocalDest;
    PblVector3 mBarrierNormal;
    int mBounceCount;
    float mPrevMeleeTime;
    int mSlideDirection;
    PblVector3 mSlidePos;
    PblVector3 mSlidePathDest;
};

 

NavigatorSlide* NavigatorSlidePtr(Navigator* pNavigator);
