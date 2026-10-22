#include "PblHermite.h"

// FUNCTION: SWBF2 0x0072a6e0
PblHermiteVector3::PblHermiteVector3(const PblVector3& vP0, const PblVector3& vP1, const PblVector3& vT0,
                                     const PblVector3& vT1, float fScale)
{
    Set(vP0, vP1, vT0, vT1, fScale);
}

// FUNCTION: SWBF2 0x0072a710
void PblHermiteVector3::Set(const PblVector3& vP0, const PblVector3& vP1, const PblVector3& vT0,
                            const PblVector3& vT1, float fScale)
{
    float fInvCube = 1.0f / (fScale * fScale * fScale);
    _a.x = ((vP0.x - vP1.x) * 2.0f + (vT1.x + vT0.x) * fScale) * fInvCube;
    _a.y = ((vP0.y - vP1.y) * 2.0f + (vT1.y + vT0.y) * fScale) * fInvCube;
    _a.z = ((vP0.z - vP1.z) * 2.0f + (vT1.z + vT0.z) * fScale) * fInvCube;

    float fInvSquare = 1.0f / (fScale * fScale);
    _b.x = ((vP1.x - vP0.x) * 3.0f - (vT1.x + vT0.x * 2.0f) * fScale) * fInvSquare;
    _b.y = ((vP1.y - vP0.y) * 3.0f - (vT1.y + vT0.y * 2.0f) * fScale) * fInvSquare;
    _b.z = ((vP1.z - vP0.z) * 3.0f - (vT1.z + vT0.z * 2.0f) * fScale) * fInvSquare;

    _c = vT0;
    _d = vP0;
}
