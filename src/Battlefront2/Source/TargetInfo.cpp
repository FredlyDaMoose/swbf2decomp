#include "Controllable.h"

// FUNCTION: SWBF2 0x00652080
void TargetInfo::SetAimPoint(const PblVector3& aimPoint)
{
    mAimPoint = aimPoint;
}

extern PblVector3 gVectorZero;

// FUNCTION: SWBF2 0x00652050
TargetInfo::TargetInfo()
{
    mAimStart = gVectorZero;
    mAimPoint = gVectorZero;
    mIsAiming = false;
}

// FUNCTION: SWBF2 0x006520a0
float TargetInfo::GetTargetPrio(const PblVector3& vStart, const PblVector3& vDir, const PblVector3& vTarget,
                                float fRadius, float fAngle, float fRange, bool bFlag)
{
    float fX = vTarget.x - vStart.x;
    float fY = vTarget.y - vStart.y;
    float fZ = vTarget.z - vStart.z;
    float fAlong = vDir.x * fX + vDir.y * fY + vDir.z * fZ;
    if (fAlong < 0.0f || fAlong > fRadius + fRange)
    {
        return -1.0f;
    }
    fX -= vDir.x * fAlong;
    fY -= vDir.y * fAlong;
    fZ -= vDir.z * fAlong;
    float fOff = fX * fX + fY * fY + fZ * fZ;
    float fCone = fAlong * fAngle + fRadius;
    if (fOff > fCone * fCone)
    {
        return -1.0f;
    }
    return fOff * 16.0f + fAlong;
}
