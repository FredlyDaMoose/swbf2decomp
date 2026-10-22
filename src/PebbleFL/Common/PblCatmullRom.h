#pragma once

#include "PblVector.h"

class PblCatmullRom
{
public:

    static void CalculatePoint(float t, const PblVector3& p0, const PblVector3& p1,
                               const PblVector3& p2, const PblVector3& p3,
                               PblVector3& vResult);

    void CalculatePoint(float t, PblVector3& vResult) const;

    void CalculateFirstDerivative(float t, PblVector3& vResult) const;

    static void CalculateFirstDerivative(float t, const PblVector3& p0, const PblVector3& p1,
                                         const PblVector3& p2, const PblVector3& p3,
                                         PblVector3& vResult);
    void AddPoint(const PblVector3& vPoint);

    void Init(const PblVector3& p0, const PblVector3& p1, const PblVector3& p2, const PblVector3& p3);

    PblCatmullRom();
    PblVector3* GetControlPoints();
    const PblVector3* GetControlPoints() const;

    PblVector3 mPoints[4];
};

 
