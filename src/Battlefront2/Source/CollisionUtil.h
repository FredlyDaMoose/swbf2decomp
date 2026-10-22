#pragma once

class PblVector3;
class PblMatrix;

namespace CollisionUtil
{
    bool CollideSphereAABB(const PblVector3& rCenter, float fRadius,
                           const PblVector3& rMin, const PblVector3& rMax);
    bool CollideSphereRay(const PblVector3& rCenter, float fRadius,
                          const PblVector3& rFrom, const PblVector3& rDir, float fLength);
    bool CollideSpheres2(const PblVector3& rCenter1, float fRadius1,
                         const PblVector3& rCenter2, float fRadius2);

    bool CollideBoxes(const PblVector3& vVelocityA, const PblMatrix& mA, float fWidthA, float fHeightA,
                      float fDepthA, const PblVector3& vVelocityB, const PblMatrix& mB, float fWidthB,
                      float fHeightB, float fDepthB, PblVector3& vNormal, float& fDepth);
    bool CollideCylinderBox(const PblVector3& vVelocityA, const PblMatrix& mA, float fRadius, float fHeight,
                            const PblVector3& vVelocityB, const PblMatrix& mB, float fWidth, float fDepthB,
                            float fHeightB, PblVector3& vNormal, float& fDepth);
    bool CollideBoxCylinder(const PblVector3& vVelocityA, const PblMatrix& mA, float fWidth, float fDepthA,
                            float fHeightA, const PblVector3& vVelocityB, const PblMatrix& mB, float fRadius,
                            float fHeight, PblVector3& vNormal, float& fDepth);
    bool CollideAxisAlignedCylinders(PblVector3& vDelta, float fRadiusA, float fRadiusB, float fHeightA,
                                     float fHeightB, float& fDepth);
    bool CollideSphereCylinder(const PblVector3& vCenter, float fSphereRadius, const PblMatrix& matrix,
                               float fRadius, float fHalfHeight, PblVector3& vNormal, float& fDepth);
    float RayHitSphere(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblVector3& vCenter,
                       float fRadius, PblVector3* pNormal);
}

int FindAACenteredBoxRayIntersection(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                                     float fWidth, float fHeight, float fDepth, float& fHit);
namespace CollisionUtil { bool IsPointInsideAxisAlignedCylinder(const PblVector3& vOffset, float fRadius, float fHalfHeight); }

class PblMatrix;
namespace CollisionUtil {
bool CollideSpheres(const PblVector3& vCenter, float fRadius, const PblVector3& vSphereCenter, float fSphereRadius,
                PblVector3& vNormal, float& fDepth);
}
namespace CollisionUtil {
bool CollideBoxSphere(const PblMatrix& matrix, float fWidth, float fDepth, float fHeight, const PblVector3& vSphereCenter,
                float fSphereRadius, PblVector3& vNormal, float& fPenetration);
}
namespace CollisionUtil {
bool CollideCylinderSphere(const PblMatrix& matrix, float fRadius, float fHalfHeight, const PblVector3& vSphereCenter,
                float fSphereRadius, PblVector3& vNormal, float& fDepth);
}

namespace CollisionUtil {
bool GetClosestPointInPolygon(const PblVector3& vPos, int iNumCorners, const PblVector3* pCorners, PblVector3& vOutPos,
                float fMaxDist);
}

namespace CollisionUtil {
void GetClosestPoint_SegmentPoint(const PblVector3& vSegStart, const PblVector3& vSegEnd, const PblVector3& vPoint,
                PblVector3& vClosest);
}
namespace CollisionUtil {
void GetClosestPoints_SegmentSegment(const PblVector3& vSStart, const PblVector3& vSEnd, const PblVector3& vTStart,
                const PblVector3& vTEnd, PblVector3& vSPoint, PblVector3& vTPoint);
}

namespace CollisionUtil {
float RayHitBubble(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblMatrix& matrix,
                 const PblVector3& vSize, PblVector3* pNormal);
}

namespace CollisionUtil {
bool CollideCylinders(const PblMatrix& m1, float fRadius1, float fHalfHeight1, const PblMatrix& m2, float fRadius2,
                float fHalfHeight2, PblVector3& vNormal, float& fDepth);
}
namespace CollisionUtil {
bool CollideCapsules(const PblMatrix& m1, float fRadius1, float fHalfHeight1, const PblMatrix& m2, float fRadius2,
                float fHalfHeight2, PblVector3& vNormal, float& fDepth);
}

namespace CollisionUtil {
bool CollideSphereBox(const PblVector3& vCenter, float fRadius, const PblMatrix& matrix, float fWidth, float fDepth,
                float fHeight, PblVector3& vNormal, float& fPenetration);
}
