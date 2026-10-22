#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblVector.h"

enum SmoothType
{
    ST_SOLDIER = 0,
    ST_WALKER = 1,
    ST_VEHICLE = 2
};

struct SmoothInfo
{

    SmoothInfo()
    {
        mRotErr._fX = 0.0f;
        mRotErr._fY = 0.0f;
        mRotErr._fZ = 0.0f;
        mRotErr._fS = 1.0f;
    }

    void SetMat(const PblMatrix& mat, float fParam1, float fParam2);
    void ResetMat(const PblMatrix& mat);
    void SetRenderMatrix(const PblMatrix& mat);

    PblMatrix& GetRenderMatrix();

    PblMatrix lastRenderMat[4];
    PblMatrix lastPredictMat[2];
    float lastRenderTime[4];
    int predictCount;
    int renderCount;
    SmoothType type;
    int mPlayerId;
    PblVector3 mVelocity;
    PblVector3 mOmega;
    float mErrTime;
    PblVector3 mPosErr;
    PblVector3 mFrontErr;
    PblQuaternion mRotErr;
};

void Smooth(SmoothInfo& info, float fSmallJump, float fLargeJump);
void __fastcall Filter(SmoothInfo* pInfo);
void __fastcall Jump(SmoothInfo* pInfo);
void __fastcall SmoothVehicle(SmoothInfo* pInfo);
void __fastcall SmoothSoldier(SmoothInfo* pInfo);
void __fastcall SmoothWalker(SmoothInfo* pInfo);
void __fastcall SmoothLocal(SmoothInfo* pInfo);

 
 
 
 

class NetPredict
{
public:
    static SmoothInfo& GetSmoothInfo(int iIndex);
    static void SetRenderTime(float fTime);
};

class NetInterpolator
{
public:
    bool GetIndex(int& iIndex, int iId, unsigned int uHandleId);
    void GetVec(int iIndex, PblVector3& vVec);

    void Prev(int iIndex);
    void SetNext(int iIndex, const PblVector3* pVec);
    void SetNext(int iIndex, const PblMatrix* pMat);
    void GetMat(int iIndex, PblMatrix& mMat);
};

extern NetInterpolator netInterpolator;
