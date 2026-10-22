#include <math.h>

#include "PblVector.h"
#include "PblMatrix.h"

// FUNCTION: SWBF2 0x004a9b50
const PblVector3 __cdecl NormalizeEst(const PblVector3& rv)
{

    const float fLength2EstAlpha = 0.94805944f;
    const float fLength2EstBeta = 0.39269909f;

    float fA = (float)fabs(rv.x);
    float fB = (float)fabs(rv.y);
    float fEst = fA > fB ? fA * fLength2EstAlpha + fB * fLength2EstBeta
                         : fA * fLength2EstBeta + fB * fLength2EstAlpha;

    fA = (float)fabs(fEst);
    fB = (float)fabs(rv.z);
    fEst = fA > fB ? fA * fLength2EstAlpha + fB * fLength2EstBeta
                   : fA * fLength2EstBeta + fB * fLength2EstAlpha;

    PblVector3 vResult;
    if (fEst != 0.0f)
    {
        const float fScale = 1.0f / fEst;
        vResult.x = rv.x * fScale;
        vResult.y = rv.y * fScale;
        vResult.z = rv.z * fScale;
    }
    else
    {
        vResult.x = rv.x;
        vResult.y = rv.y;
        vResult.z = rv.z;
    }
    return vResult;
}

// FUNCTION: SWBF2 0x00729f90
const PblBox PblBox::GetTransformedBox(const PblMatrix& rMatrix) const
{
    PblBox box;
    box._vMin = *(const PblVector3*)&rMatrix.m[3][0];
    box._vMax = box._vMin;
    const float* pMin = &_vMin.x;
    const float* pMax = &_vMax.x;
    float* pOutMin = &box._vMin.x;
    float* pOutMax = &box._vMax.x;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            float f = rMatrix.m[i][j];
            if (f < 0.0f)
            {
                pOutMax[j] += f * pMin[i];
                pOutMin[j] += f * pMax[i];
            }
            else
            {
                pOutMax[j] += f * pMax[i];
                pOutMin[j] += f * pMin[i];
            }
        }
    }
    return box;
}

// GLOBAL: SWBF2 0x007e2d14
PblVector3 gVectorUnitX(1.0f, 0.0f, 0.0f);

// GLOBAL: SWBF2 0x007e2d20
PblVector3 gVectorUnitY(0.0f, 1.0f, 0.0f);

// GLOBAL: SWBF2 0x007e2d2c
PblVector3 gVectorUnitZ(0.0f, 0.0f, 1.0f);
