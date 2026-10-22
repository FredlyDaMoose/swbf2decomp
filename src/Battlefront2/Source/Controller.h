#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblVector.h"

class Controllable;

namespace AI
{

    enum TargetBubbleIncreaseType
    {
        TBI_ROLL = 0,
        TBI_SPRINT = 1,
        TBI_JUMP = 2,
        TBI_JETJUMP = 3,
        TBI_FLYERTRICK = 4
    };
}

// VTABLE: SWBF2 0x00798cf8
class Controller
{
public:
    Controller(Controllable* pOwner);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~Controller();
    virtual void KillController();

    static PblHash rttiHashController;
    static char rttiNameController[];

    PblVector3 GetTargetBubbleCircle() const;
    void IncreaseTargetBubbleSize(float fIncrease);
    void IncreaseTargetBubbleSize(AI::TargetBubbleIncreaseType eType);

    void UpdateTargetBubbleSize(float dt);
    void UpdateTotalVisFactor();

    bool GetCrouchedForVis();
    float GetFoliageVisFactor();
    float GetInvisibilityVisFactor();
    float GetRegionVisFactor();
    void UpdateFreeSpace();

    void SetRegionVisFactor(float fCrouch, float fStand);

    void SetFoliageVisFactor(float fCrouch, float fStand);

    void SetForceAudible(bool bForce) { mForceAudible = bForce; }

    const Controllable* GetMe() const { return mOwner; }

    bool IsInCrowd() const;

protected:
    Controller();
    float _GetTargetBubbleSize() const;

public:
    Controllable* mOwner;
    PblVector3 mTargetBubbleLastVel;
    float mTargetBubbleSize;
    float mAvailableBubbleCharge;
    float mTargetBubbleSize_Cached;
    PblVector3 mTargetBubbleAxis;
    PblQuaternion mTargetBubbleRotation;
    PblVector3 mTargetBubbleOffset;
    float mOpenRadius;
    float mOpenRadiusTime;
    float mFoliageVisCrouch;
    float mFoliageVisStand;
    float mRegionVisCrouch;
    float mRegionVisStand;
    float mRegionVisTime;
    float mTotalVisFactor;
    bool mForceAudible;
    float mCrowdTestTimer;
    int mCrowdEnemies;
    int mCrowdAllies;
};

 
 
 
