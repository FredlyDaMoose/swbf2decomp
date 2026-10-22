#pragma once

#include "PblVector.h"

class PblHermiteVector3
{
public:

    PblHermiteVector3() {}

    PblHermiteVector3(const PblVector3& vP0, const PblVector3& vP1, const PblVector3& vT0,
                      const PblVector3& vT1, float fScale);

    void Set(const PblVector3& vP0, const PblVector3& vP1, const PblVector3& vT0, const PblVector3& vT1,
             float fScale);

    float GetLengthApproximation(int iSteps) const;

    PblVector3 getValue(float fT) const
    {
        PblVector3 v;
        v.x = ((_a.x * fT + _b.x) * fT + _c.x) * fT + _d.x;
        v.y = ((_a.y * fT + _b.y) * fT + _c.y) * fT + _d.y;
        v.z = ((_a.z * fT + _b.z) * fT + _c.z) * fT + _d.z;
        return v;
    }
    PblVector3 getSlope(float fT) const
    {
        PblVector3 v;
        v.x = _c.x + fT * (_b.x * 2.0f + fT * _a.x * 3.0f);
        v.y = _c.y + fT * (_b.y * 2.0f + fT * _a.y * 3.0f);
        v.z = _c.z + fT * (_b.z * 2.0f + fT * _a.z * 3.0f);
        return v;
    }

    PblVector3 _a;
    PblVector3 _b;
    PblVector3 _c;
    PblVector3 _d;
    float _fApproximateLength;
};
 
