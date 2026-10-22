#include "NetPredict.h"
#include "NetGame.h"

NetInterpolator netInterpolator;

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion*, const PblQuaternion*);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix*, const PblQuaternion*);

// GLOBAL: SWBF2 0x01ead160
unsigned int interpId[64];
// GLOBAL: SWBF2 0x01ead260
int interpCat[64];
// GLOBAL: SWBF2 0x01ead360
int interpNext;
// GLOBAL: SWBF2 0x01ead368
int interpCount[64];

extern PblVector3 interpPos[2][64];
extern PblMatrix interpMat[2][64];

// FUNCTION: SWBF2 0x005daa90
bool NetInterpolator::GetIndex(int& iIndex, int iId, unsigned int uHandleId)
{
    if (iIndex < 0 || interpCat[iIndex] != iId || interpId[iIndex] != uHandleId)
    {
        iIndex = interpNext;
        interpNext = interpNext + 1 < 64 ? interpNext + 1 : 0;
        interpCount[iIndex] = 0;
        interpCat[iIndex] = iId;
        interpId[iIndex] = uHandleId;
    }
    return true;
}

// FUNCTION: SWBF2 0x005daaf0
void NetInterpolator::Prev(int iIndex)
{
    --interpCount[iIndex];
}

// FUNCTION: SWBF2 0x005dab10
void NetInterpolator::SetNext(int iIndex, const PblVector3* pVec)
{
    if (iIndex < 0)
    {
        return;
    }
    if (!pVec)
    {
        interpCount[iIndex] = 0;
        return;
    }
    (&interpPos[0][0])[2 * iIndex + (interpCount[iIndex] & 1)] = *pVec;
    ++interpCount[iIndex];
}

// FUNCTION: SWBF2 0x005dac60
void NetInterpolator::SetNext(int iIndex, const PblMatrix* pMat)
{
    if (iIndex < 0)
    {
        return;
    }
    if (!pMat)
    {
        interpCount[iIndex] = 0;
        return;
    }
    (&interpMat[0][0])[2 * iIndex + (interpCount[iIndex] & 1)] = *pMat;
    ++interpCount[iIndex];
}

// FUNCTION: SWBF2 0x005dab70
void NetInterpolator::GetVec(int iIndex, PblVector3& vVec)
{
    if (iIndex < 0 || interpCount[iIndex] < 1)
    {
        return;
    }
    const int iNew = (interpCount[iIndex] - 1) & 1;
    const PblVector3& vNew = (&interpPos[0][0])[2 * iIndex + iNew];
    if (interpCount[iIndex] < 2)
    {
        vVec = vNew;
        return;
    }
    const PblVector3& vOld = (&interpPos[0][0])[2 * iIndex + (interpCount[iIndex] & 1)];
    const float fRatio = NetGame::GetTurnRatio();
    vVec.x = (vNew.x - vOld.x) * fRatio + vOld.x;
    vVec.y = (vNew.y - vOld.y) * fRatio + vOld.y;
    vVec.z = (vNew.z - vOld.z) * fRatio + vOld.z;
}

// FUNCTION: SWBF2 0x005dacd0
void NetInterpolator::GetMat(int iIndex, PblMatrix& mMat)
{
    if (iIndex < 0 || interpCount[iIndex] < 1)
    {
        return;
    }
    const int iNew = (interpCount[iIndex] - 1) & 1;
    const PblMatrix& mNew = (&interpMat[0][0])[2 * iIndex + iNew];
    if (interpCount[iIndex] < 2)
    {
        mMat = mNew;
        return;
    }
    const PblMatrix& mOld = (&interpMat[0][0])[2 * iIndex + (interpCount[iIndex] & 1)];
    float fRatio = NetGame::GetTurnRatio();
    PblQuaternion qOld(mOld), qNew(mNew);
    PblQuaternion qInterpolated = Interpolate(qOld, qNew, fRatio);
    PblQuaternion qNormalized;
    D3DXQuaternionNormalize(&qNormalized, &qInterpolated);
    D3DXMatrixRotationQuaternion(&mMat, &qNormalized);
    for (int i = 0; i < 3; ++i)
    {
        mMat.m[3][i] = (mNew.m[3][i] - mNew.m[3][i]) * fRatio + mNew.m[3][i];
    }
    mMat.m[3][3] = 1.0f;
}

extern int gDebugCount;
extern bool netIsLocalTurn;
extern bool netIsLastUpdateTurn;

void Smooth(SmoothInfo& info, float fParam1, float fParam2);

// GLOBAL: SWBF2 0x01fa88e8
static float renderTime;

// FUNCTION: SWBF2 0x005d9c10
void NetPredict::SetRenderTime(float fTime)
{
    renderTime = fTime;
}
extern SmoothInfo smoothInfo[64];

// FUNCTION: SWBF2 0x005da0a0
void SmoothInfo::SetRenderMatrix(const PblMatrix& mat)
{
    SmoothInfo& info = *(SmoothInfo*)this;
    for (int i = 3; i > 0; --i)
    {
        info.lastRenderMat[i] = info.lastRenderMat[i - 1];
        info.lastRenderTime[i] = info.lastRenderTime[i - 1];
    }
    info.lastRenderMat[0] = mat;
    info.lastRenderTime[0] = renderTime;
    ++info.renderCount;
}

// FUNCTION: SWBF2 0x005da1b0
PblMatrix& SmoothInfo::GetRenderMatrix()
{
    int i = 0;
    for (; i < 4; ++i)
    {
        if (renderTime == lastRenderTime[i])
        {
            break;
        }
    }
    if (i == 4)
    {
        i = 0;
    }
    else if (i >= renderCount)
    {
        i = renderCount - 1;
    }
    return lastRenderMat[i];
}

// FUNCTION: SWBF2 0x005da540
void __fastcall Jump(SmoothInfo* pInfo)
{
    ((SmoothInfo&)(*pInfo)).SetRenderMatrix(pInfo->lastPredictMat[0]);
}

// FUNCTION: SWBF2 0x005da730
void __fastcall SmoothSoldier(SmoothInfo* pInfo)
{
    SmoothLocal(pInfo);
}

// FUNCTION: SWBF2 0x005da740
void __fastcall SmoothWalker(SmoothInfo* pInfo)
{
    Filter(pInfo);
}

// FUNCTION: SWBF2 0x005da210
void Smooth(SmoothInfo& info, float fSmallJump, float fLargeJump)
{
    if (&info - smoothInfo == 8)
    {
        ++gDebugCount;
    }
    const float fX = info.lastPredictMat[0].m[3][0] - info.lastRenderMat[0].m[3][0];
    const float fY = info.lastPredictMat[0].m[3][1] - info.lastRenderMat[0].m[3][1];
    const float fZ = info.lastPredictMat[0].m[3][2] - info.lastRenderMat[0].m[3][2];
    const float fDistanceSquared = fX * fX + fY * fY + fZ * fZ;
    if (fDistanceSquared >= fLargeJump * fLargeJump)
    {
        Jump(&info);
        return;
    }
    if (fDistanceSquared >= fSmallJump * fSmallJump)
    {
        Filter(&info);
        return;
    }
    if (NetGame::GetJoystickIndex(info.mPlayerId) >= 0)
    {
        SmoothLocal(&info);
        return;
    }
    if (info.renderCount <= 3)
    {
        if (info.renderCount > 0)
        {
            Filter(&info);
        }
        else
        {
            Jump(&info);
        }
        return;
    }
    switch (info.type)
    {
    case ST_SOLDIER: SmoothSoldier(&info); break;
    case ST_WALKER: SmoothWalker(&info); break;
    case ST_VEHICLE: SmoothVehicle(&info); break;
    }
}

// FUNCTION: SWBF2 0x005da300
void __fastcall Filter(SmoothInfo* pInfo)
{
    if (pInfo->renderCount <= 0)
    {
        Jump(pInfo);
        return;
    }
    const float fElapsed = renderTime - pInfo->lastRenderTime[0];
    PblQuaternion qOld(pInfo->lastPredictMat[0]);
    PblQuaternion qNew(pInfo->lastRenderMat[0]);
    PblQuaternion qBlended = Interpolate(qOld, qNew, fElapsed * 10.0f);
    PblQuaternion qNormal;
    D3DXQuaternionNormalize(&qNormal, &qBlended);
    PblMatrix mResult;
    D3DXMatrixRotationQuaternion(&mResult, &qNormal);
    float fPosRatio = fElapsed * 5.0f;
    for (int i = 0; i < 3; ++i)
    {
        float fOld = pInfo->lastRenderMat[0].m[3][i];
        float fNew = pInfo->lastPredictMat[0].m[3][i];
        mResult.m[3][i] = fPosRatio >= 0.0001f
            ? (fPosRatio < 1.0f ? (fNew - fOld) * fPosRatio + fOld : fNew)
            : fOld;
    }
    mResult.m[3][3] = 1.0f;
    if (pInfo->mErrTime > 0.0f)
    {
        for (int i = 0; i < 3; ++i)
        {
            (&pInfo->mPosErr.x)[i] = mResult.m[3][i] - pInfo->lastPredictMat[0].m[3][i];
            (&pInfo->mFrontErr.x)[i] = mResult.m[2][i] - pInfo->lastPredictMat[0].m[2][i];
        }
        pInfo->mRotErr = PblQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }
    ((SmoothInfo&)(*pInfo)).SetRenderMatrix(mResult);
}

// FUNCTION: SWBF2 0x005da550
void __fastcall SmoothVehicle(SmoothInfo* pInfo)
{
    const float fElapsed = renderTime - pInfo->lastRenderTime[0];
    const float fPredictTime = (renderTime + 0.25f) - renderTime;
    const float fRatio = fElapsed / ((renderTime + 0.25f) - pInfo->lastRenderTime[0]);
    PblMatrix mResult;
    PblQuaternion qOld(pInfo->lastRenderMat[0]);
    PblQuaternion qNew(pInfo->lastPredictMat[0]);
    PblQuaternion qBlended = Interpolate(qOld, qNew, fElapsed * 10.0f);
    PblQuaternion qNormal;
    D3DXQuaternionNormalize(&qNormal, &qBlended);
    D3DXMatrixRotationQuaternion(&mResult, &qNormal);
    for (int i = 0; i < 3; ++i)
    {
        float fOld = pInfo->lastRenderMat[0].m[3][i];
        float fExtrapolated = pInfo->lastPredictMat[0].m[3][i] + (&pInfo->mVelocity.x)[i] * fPredictTime;
        mResult.m[3][i] = fRatio >= 0.0001f
            ? (fRatio < 1.0f ? (fExtrapolated - fOld) * fRatio + fOld : fExtrapolated)
            : fOld;
    }
    mResult.m[3][3] = 1.0f;
    ((SmoothInfo&)(*pInfo)).SetRenderMatrix(mResult);
}

// FUNCTION: SWBF2 0x005da750
void __fastcall SmoothLocal(SmoothInfo* pInfo)
{
    if (pInfo - smoothInfo == 8)
    {
        ++gDebugCount;
    }
    if (!pInfo->mPlayerId)
    {
        ++gDebugCount;
    }
    const float fElapsed = renderTime - pInfo->lastRenderTime[0];
    const float fRemaining = pInfo->mErrTime - fElapsed;
    if (fRemaining <= 0.0f)
    {
        pInfo->mPosErr = PblVector3(0.0001f, 0.0001f, 0.0001f);
        pInfo->mFrontErr = PblVector3(0.0001f, 0.0001f, 0.0001f);
        ((SmoothInfo&)(*pInfo)).SetRenderMatrix(pInfo->lastPredictMat[0]);
        pInfo->mErrTime = fRemaining < 0.0f ? 0.0f : fRemaining;
        return;
    }
    const float fRatio = fElapsed / pInfo->mErrTime;
    for (int i = 0; i < 3; ++i)
    {
        (&pInfo->mPosErr.x)[i] = (0.0f - (&pInfo->mPosErr.x)[i]) * fRatio + (&pInfo->mPosErr.x)[i];
    }
    PblQuaternion qIdentity(0.0f, 0.0f, 0.0f, 1.0f);
    pInfo->mRotErr = Interpolate(pInfo->mRotErr, qIdentity, fRatio);
    PblQuaternion qNormal;
    D3DXQuaternionNormalize(&qNormal, &pInfo->mRotErr);
    PblMatrix mCorrection, mResult;
    D3DXMatrixRotationQuaternion(&mCorrection, &qNormal);
    PblMatrix mPredict = pInfo->lastPredictMat[0];
    mPredict.m[3][0] = mPredict.m[3][1] = mPredict.m[3][2] = 0.0f;
    mPredict.m[3][3] = 1.0f;
    D3DXMatrixMultiply(&mResult, &mPredict, &mCorrection);
    for (int i = 0; i < 3; ++i)
    {
        mResult.m[3][i] = pInfo->lastPredictMat[0].m[3][i] + (&pInfo->mPosErr.x)[i];
    }
    mResult.m[3][3] = 1.0f;
    ((SmoothInfo&)(*pInfo)).SetRenderMatrix(mResult);
    pInfo->mErrTime = fRemaining;
    for (int i = 0; i < 3; ++i)
    {
        (&pInfo->mFrontErr.x)[i] *= 1.0f - fRatio;
    }
}

// GLOBAL: SWBF2 0x01ee8390
// SYNTHETIC: SWBF2 0x004055f0
// `dynamic initializer for 'smoothInfo''
SmoothInfo smoothInfo[64];

// FUNCTION: SWBF2 0x005d9c00
SmoothInfo& NetPredict::GetSmoothInfo(int iIndex)
{
    return smoothInfo[iIndex];
}

// FUNCTION: SWBF2 0x005d9d20
void SmoothInfo::SetMat(const PblMatrix& mat, float fParam1, float fParam2)
{
    if (this - smoothInfo == 8)
    {
        ++gDebugCount;
    }

    if (netIsLocalTurn)
    {
        lastPredictMat[1] = lastPredictMat[0];
        lastPredictMat[0] = mat;
        ++predictCount;
        Smooth(*this, fParam1, fParam2);
        return;
    }
    if (!netIsLastUpdateTurn)
    {
        return;
    }

    if (this - smoothInfo == 8)
    {
        ++gDebugCount;
    }
    if (!mPlayerId)
    {
        ++gDebugCount;
    }

    bool bLocal = NetGame::GetJoystickIndex(mPlayerId) >= 0;
    if (!bLocal && type != ST_SOLDIER)
    {
        return;
    }
    if ((unsigned int)renderCount < 4)
    {
        return;
    }

    unsigned int i = 0;
    for (; i < 4; ++i)
    {
        if (renderTime == lastRenderTime[i])
        {
            break;
        }
    }
    if (i >= 4)
    {
        return;
    }

    const PblMatrix& rendered = lastRenderMat[i];
    PblVector3 vPosErr(rendered.m[3][0] - mat.m[3][0], rendered.m[3][1] - mat.m[3][1],
                       rendered.m[3][2] - mat.m[3][2]);
    PblVector3 vFrontErr(rendered.m[2][0] - mat.m[2][0], rendered.m[2][1] - mat.m[2][1],
                         rendered.m[2][2] - mat.m[2][2]);
    if (vPosErr.y * vPosErr.y + vPosErr.x * vPosErr.x + vPosErr.z * vPosErr.z >
            mPosErr.x * mPosErr.x + mPosErr.y * mPosErr.y + mPosErr.z * mPosErr.z ||
        vFrontErr.x * vFrontErr.x + vFrontErr.y * vFrontErr.y + vFrontErr.z * vFrontErr.z >
            mFrontErr.x * mFrontErr.x + mFrontErr.y * mFrontErr.y + mFrontErr.z * mFrontErr.z)
    {
        mErrTime = bLocal ? 0.25f : 0.5f;
        mPosErr = vPosErr;
        mFrontErr = vFrontErr;

        PblMatrix oldRot = rendered;
        oldRot.m[3][0] = 0.0f;
        oldRot.m[3][1] = 0.0f;
        oldRot.m[3][2] = 0.0f;
        oldRot.m[3][3] = 1.0f;
        PblMatrix newRot = mat;
        newRot.m[3][0] = 0.0f;
        newRot.m[3][1] = 0.0f;
        newRot.m[3][2] = 0.0f;
        newRot.m[3][3] = 1.0f;
        PblMatrix newInverse = newRot.GetInverseFast();
        PblMatrix relMatrix;
        D3DXMatrixMultiply(&relMatrix, &newInverse, &oldRot);
        mRotErr = PblQuaternion(relMatrix);
    }
}

// FUNCTION: SWBF2 0x005d9c20
void SmoothInfo::ResetMat(const PblMatrix& mat)
{
    lastPredictMat[0] = mat;
    predictCount = 1;
    lastRenderMat[0] = mat;
    renderCount = 1;
    lastRenderTime[0] = renderTime;
    mErrTime = 0.0f;
    mPosErr = PblVector3(0.0001f, 0.0001f, 0.0001f);
    mFrontErr = PblVector3(0.0001f, 0.0001f, 0.0001f);
    mRotErr._fX = 0.0f;
    mRotErr._fY = 0.0f;
    mRotErr._fZ = 0.0f;
    mRotErr._fS = 1.0f;
}

// GLOBAL: SWBF2 0x01ee7d90
// SYNTHETIC: SWBF2 0x00405680
// `dynamic initializer for 'interpPos''

PblVector3 interpPos[2][64];

// GLOBAL: SWBF2 0x01eeff90
// SYNTHETIC: SWBF2 0x004056a0
// `dynamic initializer for 'interpMat''

PblMatrix interpMat[2][64];
