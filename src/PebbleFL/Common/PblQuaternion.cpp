#include "PblQuaternion.h"

#include <math.h>

#include "PblMath.h"
#include "PblMatrix.h"
#include "PblVector.h"

extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);
extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

enum QUAT_INDEX
{
    X_INDEX = 0,
    Y_INDEX = 1,
    Z_INDEX = 2
};

// GLOBAL: SWBF2 0x007e2d80
static const QUAT_INDEX QIndex[3] = { Y_INDEX, Z_INDEX, X_INDEX };

// FUNCTION: SWBF2 0x0072a0d0
PblQuaternion::PblQuaternion(const PblMatrix& rMatrix)
{
    float* pq = &_fX;
    float fTrace = rMatrix.m[0][0] + rMatrix.m[1][1] + rMatrix.m[2][2];
    if (fTrace > 0.0f)
    {
        float fS = (float)sqrt((double)(fTrace + 1.0f));
        _fS = fS * 0.5f;
        fS = 0.5f / fS;
        _fX = (rMatrix.m[2][1] - rMatrix.m[1][2]) * fS;
        _fY = (rMatrix.m[0][2] - rMatrix.m[2][0]) * fS;
        _fZ = (rMatrix.m[1][0] - rMatrix.m[0][1]) * fS;
    }
    else
    {
        int i = X_INDEX;
        if (rMatrix.m[1][1] > rMatrix.m[0][0])
        {
            i = Y_INDEX;
        }
        if (rMatrix.m[2][2] > rMatrix.m[i][i])
        {
            i = Z_INDEX;
        }
        int j = QIndex[i];
        int k = QIndex[j];

        float fS = (float)sqrt((double)(rMatrix.m[i][i] + 1.0f - rMatrix.m[j][j] - rMatrix.m[k][k]));
        pq[i] = fS * 0.5f;
        fS = 0.5f / fS;
        _fS = (rMatrix.m[k][j] - rMatrix.m[j][k]) * fS;
        pq[j] = (rMatrix.m[j][i] + rMatrix.m[i][j]) * fS;
        pq[k] = (rMatrix.m[k][i] + rMatrix.m[i][k]) * fS;
    }
    _fX = -_fX;
    _fY = -_fY;
    _fZ = -_fZ;
}

// FUNCTION: SWBF2 0x0072a2a0
PblQuaternion operator*(const PblQuaternion& q0, const PblQuaternion& q1)
{
    PblQuaternion q;
    q._fS = q1._fS * q0._fS - q1._fX * q0._fX - q1._fY * q0._fY - q1._fZ * q0._fZ;
    q._fX = q0._fX * q1._fS + q1._fX * q0._fS - q0._fZ * q1._fY + q1._fZ * q0._fY;
    q._fY = q0._fZ * q1._fX + q0._fY * q1._fS + q1._fY * q0._fS - q1._fZ * q0._fX;
    q._fZ = q0._fZ * q1._fS - q0._fY * q1._fX + q1._fY * q0._fX + q1._fZ * q0._fS;
    return q;
}

// FUNCTION: SWBF2 0x0072a380
PblQuaternion Interpolate(const PblQuaternion& q0, const PblQuaternion& q1, float fT)
{
    const float fCos = q0._fX * q1._fX + q1._fS * q0._fS + q1._fY * q0._fY + q1._fZ * q0._fZ;

    float fScale0;
    float fScale1;
    if (fCos >= 0.0f)
    {
        if (1.0f - fCos < 0.0001f)
        {
            fScale0 = 1.0f - fT;
            fScale1 = fT;
        }
        else
        {
            const float fAngle = (float)acos((double)fCos);
            const float fInvSin = 1.0f / PblMath::Sin(fAngle);
            fScale0 = PblMath::Sin((1.0f - fT) * fAngle) * fInvSin;
            fScale1 = PblMath::Sin(fAngle * fT) * fInvSin;
        }
    }
    else
    {
        const float fNegCos = -fCos;
        if (1.0f - fNegCos < 0.0001f)
        {
            fScale0 = 1.0f - fT;
            fScale1 = -fT;
        }
        else
        {
            const float fAngle = (float)acos((double)fNegCos);
            const float fInvSin = 1.0f / PblMath::Sin(fAngle);
            fScale0 = PblMath::Sin((1.0f - fT) * fAngle) * fInvSin;
            fScale1 = -(PblMath::Sin(fAngle * fT) * fInvSin);
        }
    }

    PblQuaternion q;
    q._fS = fScale0 * q0._fS + q1._fS * fScale1;
    q._fY = q0._fY * fScale0 + q1._fY * fScale1;
    q._fX = q0._fX * fScale0 + q1._fX * fScale1;
    q._fZ = q0._fZ * fScale0 + q1._fZ * fScale1;

    const float fLengthSq = q._fX * q._fX + q._fS * q._fS + q._fY * q._fY + q._fZ * q._fZ;
    if ((float)fabs((double)(fLengthSq - 1.0f)) > 0.0001f)
    {
        const float fInvLength = 1.0f / (float)sqrt((double)fLengthSq);
        PblQuaternion n;
        n._fX = fInvLength * q._fX;
        n._fY = fInvLength * q._fY;
        n._fZ = fInvLength * q._fZ;
        n._fS = fInvLength * q._fS;
        q = n;
    }
    return q;
}

// FUNCTION: SWBF2 0x0072a670
PblVector3 PblQuaternion::RotateVector(const PblVector3& v) const
{
    PblQuaternion qUnit;
    PblMatrix mRot;
    D3DXMatrixRotationQuaternion(&mRot, D3DXQuaternionNormalize(&qUnit, this));
    mRot.m[3][0] = 0.0f;
    mRot.m[3][1] = 0.0f;
    mRot.m[3][2] = 0.0f;
    mRot.m[3][3] = 1.0f;
    PblVector3 vOut;
    D3DXVec3TransformNormal(&vOut, &v, &mRot);
    return vOut;
}
