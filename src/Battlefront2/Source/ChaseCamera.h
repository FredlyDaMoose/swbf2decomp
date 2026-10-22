#pragma once

#include <stddef.h>

#include "GameCamera.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblQuaternion.h"

class Trackable;

extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);

// VTABLE: SWBF2 0x00797178
class ChaseCamera : public GameCamera
{
public:
    ChaseCamera(CameraManager* pManager, int iView);
    virtual ~ChaseCamera();
    virtual void Update(float fDeltaTime);
    virtual void SetupCamera(RedCamera* pCamera, float fDeltaTime);
    virtual void SetupInterpolatedCamera(RedCamera* pCamera);
    virtual void SetupIconVisibility(int iView);
    virtual PblVector3 GetVelocity();

    void ApplyShake(float fAmount, float fLength)
    {
        if (mShakeCount < 4)
        {
            mShakeAmount[mShakeCount] = fAmount;
            mShakeDecay[mShakeCount++] = fAmount / fLength;
        }
    }

    void ClearShake()
    {
        mShakeCount = 0;
        mShake = 0.0f;
    }
    Trackable* GetOwner() { return mOwner; }
    const PblMatrix& GetMatrix() { return mMatrix; }
    void SetRotation(const PblQuaternion& qRot)
    {
        PblQuaternion qUnit;
        PblMatrix mRot;
        D3DXMatrixRotationQuaternion(&mRot, D3DXQuaternionNormalize(&qUnit, &qRot));
        mRot.m[3][0] = mMatrix.m[3][0];
        mRot.m[3][1] = mMatrix.m[3][1];
        mRot.m[3][2] = mMatrix.m[3][2];
        mRot.m[3][3] = 1.0f;
        mMatrix = mRot;
    }
    void SetPosition(const PblVector3& vPos)
    {
        mMatrix.m[3][0] = vPos.x;
        mMatrix.m[3][1] = vPos.y;
        mMatrix.m[3][2] = vPos.z;
        mMatrix.m[3][3] = 1.0f;
    }

    void SetOwner(Trackable* pOwner);

    Trackable* mOwner;
    PblMatrix mMatrix;
    PblMatrix mPreShakeMatrix;
    float mTimer;
    float mShake;
    float mShakeSuppressUntil;
    int mShakeCount;
    float mShakeAmount[4];
    float mShakeDecay[4];
    float mTrackTime;
    bool mDemoMode;
    float mCameraSwing;
    bool mDoSwingFlag;
    float mEndDt;
    float mCurrentDt;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
