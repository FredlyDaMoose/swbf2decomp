#pragma once

#include <stddef.h>

#include "DisplayEntity.h"
#include "PebbleFL/Common/PblAngle.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "Zephyr/Source/ZephyrSkeleton.h"

class EntityDroidekaClass;

// VTABLE: SWBF2 0x00798fac
class DroidekaElement : public EntityElement
{
public:
    DroidekaElement();
    virtual ~DroidekaElement();

private:
    virtual void RenderUsingContext(const PblMatrix& mContext, const RedColor& color);

public:
    virtual bool Setup();
    virtual bool Update(float fElapsed);
    virtual PblVector3 GetPos();
    virtual void SetScale(float fScale);
    virtual void SetSelected(bool bSelected);

    void SetClass(EntityDroidekaClass* pClass) { mClass = pClass; }

    void SetState(int iState);
    void NextState(int iInput);
    void UpdateState(bool bRoll);
    void Reset();

private:
    void BuildWorldMatrices();

public:
    RedPose mPose;
    EntityDroidekaClass* mClass;
    float mTime;
    int mState;
    ZephyrSkeleton<32> mZephyrSkeleton;
    ZephyrPoseDyn<32> mZephyrPoseDyn;
    PblAngle mBallOrientation;
    PblVector3 mTranslationOffset;
    PblAngle mTurnOffset;
    PblAngle mPitchOffset;
    PblVector3 mPivotOffset;
};

 
 
 
 
 
 
 
