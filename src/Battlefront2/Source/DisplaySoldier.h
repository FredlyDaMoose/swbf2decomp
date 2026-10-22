#pragma once

#include <stddef.h>

#include "DisplayEntity.h"
#include "PebbleFL/Common/PblBitVector.h"
#include "PebbleFL/Common/PblMatrix.h"

class Aimer;
class EntityCloth;
class EntitySoldier;
class EntitySoldierClass;
class SoldierAnimator;
class TentacleSimulator;
class WeaponClass;
class ZephyrSkeletonShared;

class SoldierElement : public EntityElement
{
public:
    SoldierElement();
    virtual ~SoldierElement();

private:
    virtual void RenderUsingContext(const PblMatrix& mContext, const RedColor& color);

public:
    virtual bool Setup();
    virtual bool Update(float fElapsed);
    virtual PblVector3 GetPos();
    virtual void SetSelected(bool bSelected);

    void SetClass(EntitySoldierClass* pClass)
    {
        if (pClass != mClass)
        {
            mClass = pClass;
            mSoldierAnimator = 0;
        }
    }
    void SetAim(const PblVector3& vAim) { mAim = vAim; }
    void Reset();

    EntitySoldier* mID;
    SoldierAnimator* mSoldierAnimator;
    ZephyrSkeletonShared* mSpecialSkeleton;
    PblBitVector<32> mUpperBodyAnimMask;
    PblBitVector<32> mLowerBodyAnimMask;
    unsigned int mAnimationHash;
    int mState;
    PblMatrix mMatrix;
    PblVector3 mGroundNormal;
    WeaponClass* mWeaponClass[8];
    int mWeaponIndex;
    EntitySoldierClass* mClass;
    Aimer* mAimer;
    bool m_bMoving;
    PblVector3 mAim;
    PblVector3 mPos;
    bool mIsSetup;
    bool mSelected;
    float mRandomActionTimer;
    float mSelectedTimer;
    EntityCloth* mClothPiece[8];
    TentacleSimulator* mTentacles;
};

 
 
 
 
