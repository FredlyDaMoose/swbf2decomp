#pragma once

#include <stddef.h>
#include "Red3DModelElementLite.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblVector.h"

class Red3DModelAnimator;
class Red3DModelElement;

class Red3DModelElement : public Red3DModelElementLite
{
public:

    Red3DModelElement();

    Red3DModelElement(const char* pcName);

    virtual ~Red3DModelElement();

protected:
    void EnsureAnimatorExists();

    virtual void RenderUsingContext(const PblMatrix& mContext, const RedColor& color);

public:

    void SetModelScale(float fScale) { m_scale = fScale; }

    void SetOmegaY(float fOmegaY) { m_OmegaY = fOmegaY; }

    void SetPosedMatrix(const PblMatrix& mPosed);
    void SetDrawMatrix(const PblMatrix& mDraw);
    void QueueAnimation(unsigned int uAnimHash);

    Red3DModelAnimator* GetAnimator() { return mAnimator; }
    void SetAnimator(Red3DModelAnimator* pAnimator) { mAnimator = pAnimator; }
    void SetAnimationBanks(const unsigned int* puBanks);
    void SetAnimationTime(float fTime);

    float m_scale;
    float m_OmegaY;
    float m_Yaw;
    PblQuaternion m_rotation;
    PblVector3 m_translation;
    PblMatrix m_PosedMatrix;
    PblMatrix m_DrawMatrix;
    unsigned int m_uiHardpointHash;
    Red3DModelElement* m_AttachedObj;
    Red3DModelAnimator* mAnimator;
    bool m_bAmAttached : 1;
    bool m_bHavePoseMatrix : 1;
    bool m_bApplyRotationScale : 1;
};
 
 
 
 
 
 
 
 
 
 
 
