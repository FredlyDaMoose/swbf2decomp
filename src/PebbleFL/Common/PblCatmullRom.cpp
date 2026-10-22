#include "PblCatmullRom.h"

// FUNCTION: SWBF2 0x0072af70
void PblCatmullRom::CalculatePoint(float t, PblVector3& vResult) const
{
    CalculatePoint(t, mPoints[0], mPoints[1], mPoints[2], mPoints[3], vResult);
}

// FUNCTION: SWBF2 0x0072ac00
PblCatmullRom::PblCatmullRom()
{
    for (int i = 0; i < 4; ++i)
    {
        mPoints[i].SetAll(0.0f);
    }
}

// FUNCTION: SWBF2 0x0072b260
PblVector3* PblCatmullRom::GetControlPoints()
{
    return mPoints;
}

// FUNCTION: SWBF2 0x0072b270
const PblVector3* PblCatmullRom::GetControlPoints() const
{
    return mPoints;
}

// FUNCTION: SWBF2 0x0072ac60
void PblCatmullRom::AddPoint(const PblVector3& vPoint)
{
    mPoints[0] = mPoints[1];
    mPoints[1] = mPoints[2];
    mPoints[2] = mPoints[3];
    mPoints[3] = vPoint;
}

// FUNCTION: SWBF2 0x0072acb0
void PblCatmullRom::CalculateFirstDerivative(float t, PblVector3& vResult) const
{
    CalculateFirstDerivative(t, mPoints[0], mPoints[1], mPoints[2], mPoints[3], vResult);
}

// FUNCTION: SWBF2 0x0072ace0
void PblCatmullRom::CalculateFirstDerivative(float t, const PblVector3& p0, const PblVector3& p1,
                                             const PblVector3& p2, const PblVector3& p3,
                                             PblVector3& vResult)
{
    PblVector3 a, b, c;
    a.x = p0.x * -0.5f + p1.x * 1.5f + p2.x * -1.5f + p3.x * 0.5f;
    a.y = p0.y * -0.5f + p1.y * 1.5f + p2.y * -1.5f + p3.y * 0.5f;
    a.z = p0.z * -0.5f + p1.z * 1.5f + p2.z * -1.5f + p3.z * 0.5f;
    b.x = p0.x + p1.x * -2.5f + p2.x * 2.0f + p3.x * -0.5f;
    b.y = p0.y + p1.y * -2.5f + p2.y * 2.0f + p3.y * -0.5f;
    b.z = p0.z + p1.z * -2.5f + p2.z * 2.0f + p3.z * -0.5f;
    c.x = p0.x * -0.5f + p2.x * 0.5f;
    c.y = p0.y * -0.5f + p2.y * 0.5f;
    c.z = p0.z * -0.5f + p2.z * 0.5f;
    vResult.x = (a.x * 3.0f * t + b.x * 2.0f) * t + c.x;
    vResult.y = (a.y * 3.0f * t + b.y * 2.0f) * t + c.y;
    vResult.z = (a.z * 3.0f * t + b.z * 2.0f) * t + c.z;
}

// FUNCTION: SWBF2 0x0072b000
void PblCatmullRom::CalculatePoint(float t, const PblVector3& p0, const PblVector3& p1,
                                   const PblVector3& p2, const PblVector3& p3,
                                   PblVector3& vResult)
{
    PblVector3 a, b, c;
    a.x = p0.x * -0.5f + p1.x * 1.5f + p2.x * -1.5f + p3.x * 0.5f;
    a.y = p0.y * -0.5f + p1.y * 1.5f + p2.y * -1.5f + p3.y * 0.5f;
    a.z = p0.z * -0.5f + p1.z * 1.5f + p2.z * -1.5f + p3.z * 0.5f;
    b.x = p0.x + p1.x * -2.5f + p2.x * 2.0f + p3.x * -0.5f;
    b.y = p0.y + p1.y * -2.5f + p2.y * 2.0f + p3.y * -0.5f;
    b.z = p0.z + p1.z * -2.5f + p2.z * 2.0f + p3.z * -0.5f;
    c.x = p0.x * -0.5f + p2.x * 0.5f;
    c.y = p0.y * -0.5f + p2.y * 0.5f;
    c.z = p0.z * -0.5f + p2.z * 0.5f;
    vResult.x = ((a.x * t + b.x) * t + c.x) * t + p1.x;
    vResult.y = ((a.y * t + b.y) * t + c.y) * t + p1.y;
    vResult.z = ((a.z * t + b.z) * t + c.z) * t + p1.z;
}

// FUNCTION: SWBF2 0x0072b280
void PblCatmullRom::Init(const PblVector3& p0, const PblVector3& p1, const PblVector3& p2, const PblVector3& p3)
{
    mPoints[0] = p0;
    mPoints[1] = p1;
    mPoints[2] = p2;
    mPoints[3] = p3;
}
