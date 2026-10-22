#include <math.h>
#include <d3dx9math.h>

#include "CollisionUtil.h"
#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblMatrix.h"

// FUNCTION: SWBF2 0x00469060
bool CollisionUtil::CollideSphereAABB(const PblVector3& rCenter, float fRadius,
                                      const PblVector3& rMin, const PblVector3& rMax)
{
    float fDist2 = 0.0f;
    if (rCenter.x < rMin.x)
    {
        fDist2 += (rCenter.x - rMin.x) * (rCenter.x - rMin.x);
    }
    else if (rCenter.x > rMax.x)
    {
        fDist2 += (rCenter.x - rMax.x) * (rCenter.x - rMax.x);
    }
    if (rCenter.y < rMin.y)
    {
        fDist2 += (rCenter.y - rMin.y) * (rCenter.y - rMin.y);
    }
    else if (rCenter.y > rMax.y)
    {
        fDist2 += (rCenter.y - rMax.y) * (rCenter.y - rMax.y);
    }
    if (rCenter.z < rMin.z)
    {
        fDist2 += (rCenter.z - rMin.z) * (rCenter.z - rMin.z);
    }
    else if (rCenter.z > rMax.z)
    {
        fDist2 += (rCenter.z - rMax.z) * (rCenter.z - rMax.z);
    }
    return fRadius * fRadius >= fDist2;
}

// FUNCTION: SWBF2 0x004690f0
bool CollisionUtil::CollideSphereRay(const PblVector3& rCenter, float fRadius,
                                     const PblVector3& rFrom, const PblVector3& rDir, float fLength)
{
    float fX = rCenter.x - rFrom.x;
    float fY = rCenter.y - rFrom.y;
    float fZ = rCenter.z - rFrom.z;
    float fProj = fX * rDir.x + rDir.y * fY + rDir.z * fZ;
    float fDist2 = fX * fX + fY * fY + fZ * fZ;
    float fRadius2 = fRadius * fRadius;

    if (fRadius2 > fDist2)
    {
        return true;
    }
    if (fProj < 0.0f && fDist2 > fRadius2)
    {
        return false;
    }
    float fPerp2 = fDist2 - fProj * fProj;
    if (fPerp2 > fRadius2)
    {
        return false;
    }
    float fHalfChord = sqrtf(fRadius2 - fPerp2);
    float fHit = fDist2 > fRadius2 ? fProj - fHalfChord : fProj + fHalfChord;
    if (fHit < 0.0f)
    {
        return false;
    }
    return fHit <= fLength;
}

// FUNCTION: SWBF2 0x0046a810
bool CollisionUtil::CollideSpheres2(const PblVector3& rCenter1, float fRadius1,
                                    const PblVector3& rCenter2, float fRadius2)
{
    float fX = rCenter2.x - rCenter1.x;
    float fY = rCenter2.y - rCenter1.y;
    float fZ = rCenter2.z - rCenter1.z;
    return fX * fX + fY * fY + fZ * fZ < (fRadius1 + fRadius2) * (fRadius1 + fRadius2);
}

// FUNCTION: SWBF2 0x0046ae60
int FindAACenteredBoxRayIntersection(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                                     float fWidth, float fHeight, float fDepth, float& fHit)
{
    const float* pStart = &vStart.x;
    float aExtent[3] = { fWidth, fHeight, fDepth };
    float aPlane[3];
    int aSide[3];
    bool bInside = true;
    int i;
    for (i = 0; i < 3; i++)
    {
        if (-aExtent[i] > pStart[i])
        {
            aSide[i] = -1;
            aPlane[i] = -aExtent[i];
            bInside = false;
        }
        else if (pStart[i] > aExtent[i])
        {
            aSide[i] = 1;
            aPlane[i] = aExtent[i];
            bInside = false;
        }
        else
        {
            aSide[i] = 0;
            aPlane[i] = aExtent[i];
        }
    }
    if (bInside)
    {
        return 0;
    }

    float aRay[3] = { vDir.x * fLength, vDir.y * fLength, vDir.z * fLength };
    float aT[3];
    for (i = 0; i < 3; i++)
    {
        if (aSide[i] != 0 && fabsf(aRay[i]) > 0.0001f)
        {
            aT[i] = (aPlane[i] - pStart[i]) / aRay[i];
        }
        else
        {
            aT[i] = -1.0f;
        }
    }

    for (i = 0; i < 3; i++)
    {
        if (aT[i] > 0.0f)
        {
            float aHit[3];
            aHit[0] = pStart[0] + aRay[0] * aT[i];
            aHit[1] = pStart[1] + aRay[1] * aT[i];
            aHit[2] = pStart[2] + aRay[2] * aT[i];
            for (int j = 0; j < 3; j++)
            {
                if (j != i && (-aExtent[j] > aHit[j] || aHit[j] > aExtent[j]))
                {
                    aT[i] = -1.0f;
                    break;
                }
            }
        }
    }

    int nPlane = 0;
    if (aT[1] > 0.0f && (aT[nPlane] > aT[1] || aT[nPlane] < 0.0f))
    {
        nPlane = 1;
    }
    if (aT[2] > 0.0f && (aT[nPlane] > aT[2] || aT[nPlane] < 0.0f))
    {
        nPlane = 2;
    }
    if (aT[nPlane] < 0.0f)
    {
        return 0;
    }
    fHit = aT[nPlane];
    return aSide[nPlane] * (nPlane + 1);
}

namespace CollisionUtil {

// FUNCTION: SWBF2 0x00469670
bool IsPointInsideAxisAlignedCylinder(const PblVector3& vOffset, float fRadius, float fHalfHeight)
{
    if (fabsf(vOffset.y) > fHalfHeight)
    {
        return false;
    }
    return fRadius * fRadius > vOffset.x * vOffset.x + vOffset.z * vOffset.z;
}
}

static inline PblVector3 ToLocal(const PblMatrix& m, const PblVector3& v)
{
    return PblVector3(m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z,
                      m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z,
                      m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z);
}

static inline PblVector3 ToWorld(const PblMatrix& m, float x, float y, float z)
{
    return PblVector3(m.m[0][0] * x + m.m[1][0] * y + m.m[2][0] * z,
                      m.m[0][1] * x + m.m[1][1] * y + m.m[2][1] * z,
                      m.m[0][2] * x + m.m[1][2] * y + m.m[2][2] * z);
}

namespace CollisionUtil {

// FUNCTION: SWBF2 0x0046a700
bool CollideSpheres(const PblVector3& vCenter, float fRadius, const PblVector3& vSphereCenter, float fSphereRadius,
                PblVector3& vNormal, float& fDepth)
{
    float fX = vSphereCenter.x - vCenter.x;
    float fY = vSphereCenter.y - vCenter.y;
    float fZ = vSphereCenter.z - vCenter.z;
    float fRadii = fRadius + fSphereRadius;
    float fDist = fX * fX + fY * fY + fZ * fZ;
    if (fDist >= fRadii * fRadii)
    {
        return false;
    }
    if (fDist > 0.0f)
    {
        fDist = (float)sqrt((double)fDist);
    }
    fDepth = fRadii - fDist;
    if (fDist > 0.0f)
    {
        float fInv = 1.0f / fDist;
        vNormal = PblVector3(fX * fInv, fY * fInv, fZ * fInv);
    }
    else
    {
        vNormal = PblVector3(0.0f, 0.0f, 1.0f);
    }
    return true;
}
}

namespace CollisionUtil {

// FUNCTION: SWBF2 0x00469e90
bool CollideBoxSphere(const PblMatrix& matrix, float fWidth, float fDepth, float fHeight, const PblVector3& vSphereCenter,
                float fSphereRadius, PblVector3& vNormal, float& fPenetration)
{
    PblVector3 vOffset(vSphereCenter.x - matrix.m[3][0], vSphereCenter.y - matrix.m[3][1],
                       vSphereCenter.z - matrix.m[3][2]);
    PblVector3 vLocal = ToLocal(matrix, vOffset);
    bool bInside = true;
    int iSideX, iSideY, iSideZ;
    float fClampX, fClampY, fClampZ;

    if (-fWidth > vLocal.x)
    {
        iSideX = -1;
        fClampX = -fWidth;
        bInside = false;
    }
    else if (vLocal.x > fWidth)
    {
        iSideX = 1;
        fClampX = fWidth;
        bInside = false;
    }
    else
    {
        iSideX = 0;
        fClampX = vLocal.x;
    }
    if (-fHeight > vLocal.y)
    {
        iSideY = -1;
        fClampY = -fHeight;
        bInside = false;
    }
    else if (vLocal.y > fHeight)
    {
        iSideY = 1;
        fClampY = fHeight;
        bInside = false;
    }
    else
    {
        iSideY = 0;
        fClampY = vLocal.y;
    }
    if (-fDepth > vLocal.z)
    {
        iSideZ = -1;
        fClampZ = -fDepth;
        bInside = false;
    }
    else if (vLocal.z > fDepth)
    {
        iSideZ = 1;
        fClampZ = fDepth;
        bInside = false;
    }
    else
    {
        iSideZ = 0;
        fClampZ = vLocal.z;
    }
    if (bInside)
    {
        return false;
    }

    float fDX = iSideX ? (float)fabs((double)(fClampX - vLocal.x)) : 0.0f;
    float fDY = iSideY ? (float)fabs((double)(fClampY - vLocal.y)) : 0.0f;
    float fDZ = iSideZ ? (float)fabs((double)(fClampZ - vLocal.z)) : 0.0f;
    float fDist2 = fDX * fDX + fDY * fDY + fDZ * fDZ;
    if (!(fSphereRadius * fSphereRadius > fDist2))
    {
        return false;
    }

    float fDist = (float)sqrt((double)fDist2);
    fPenetration = fSphereRadius - fDist;
    vNormal = ToWorld(matrix, (float)iSideX * fDX, (float)iSideY * fDY, (float)iSideZ * fDZ);
    float fInv = 1.0f / fDist;
    vNormal = PblVector3(vNormal.x * fInv, vNormal.y * fInv, vNormal.z * fInv);
    return true;
}
}

namespace CollisionUtil {

// FUNCTION: SWBF2 0x00469e20
bool CollideSphereBox(const PblVector3& vCenter, float fRadius, const PblMatrix& matrix, float fWidth, float fDepth,
                float fHeight, PblVector3& vNormal, float& fPenetration)
{
    bool bCollided = CollisionUtil::CollideBoxSphere(matrix, fWidth, fDepth, fHeight, vCenter, fRadius, vNormal, fPenetration);
    vNormal.x *= -1.0f;
    vNormal.y *= -1.0f;
    vNormal.z *= -1.0f;
    return bCollided;
}
}

namespace CollisionUtil {

// FUNCTION: SWBF2 0x0046a280
bool CollideCylinderSphere(const PblMatrix& matrix, float fRadius, float fHalfHeight, const PblVector3& vSphereCenter,
                float fSphereRadius, PblVector3& vNormal, float& fDepth)
{
    PblVector3 vOffset(vSphereCenter.x - matrix.m[3][0], vSphereCenter.y - matrix.m[3][1],
                       vSphereCenter.z - matrix.m[3][2]);
    PblVector3 vLocal = ToLocal(matrix, vOffset);
    float fDist2 = vLocal.x * vLocal.x + vLocal.z * vLocal.z;

    if (!(vLocal.y > fHalfHeight || -fHalfHeight > vLocal.y))
    {

        float fDist = (float)sqrt((double)fDist2);
        fDepth = (fRadius + fSphereRadius) - fDist;
        if (!(fDepth > 0.0f))
        {
            return false;
        }
        if (fDist > 0.0001f)
        {
            float fInv = 1.0f / fDist;
            vNormal = ToWorld(matrix, fInv * vLocal.x, fInv * 0.0f, vLocal.z * fInv);
        }
        else
        {
            fDepth = fSphereRadius > fRadius ? fSphereRadius : fRadius;
            vNormal = PblVector3(matrix.m[2][0], matrix.m[2][1], matrix.m[2][2]);
        }
        return true;
    }

    if (fRadius * fRadius > fDist2)
    {

        fDepth = fSphereRadius - ((float)fabs((double)vLocal.y) - fHalfHeight);
        if (!(fDepth > 0.0f))
        {
            return false;
        }
        if (0.0f > vLocal.y)
        {
            vNormal = PblVector3(matrix.m[1][0] * -1.0f, matrix.m[1][1] * -1.0f, matrix.m[1][2] * -1.0f);
        }
        else
        {
            vNormal = PblVector3(matrix.m[1][0], matrix.m[1][1], matrix.m[1][2]);
        }
        return true;
    }

    float fScale = fRadius / (float)sqrt((double)fDist2);
    PblVector3 vEdge(fScale * vLocal.x, fScale * 0.0f, vLocal.z * fScale);
    float fCapY = 0.0f > vLocal.y ? -fHalfHeight : fHalfHeight;
    float fX = vLocal.x - vEdge.x;
    float fZ = vLocal.z - vEdge.z;
    float fY = vLocal.y - fCapY;
    float fEdgeDist2 = fX * fX + fY * fY + fZ * fZ;
    if (!(fSphereRadius * fSphereRadius > fEdgeDist2))
    {
        return false;
    }
    float fEdgeDist = (float)sqrt((double)fEdgeDist2);
    fDepth = fSphereRadius - fEdgeDist;
    float fInv = 1.0f / fEdgeDist;
    vNormal = ToWorld(matrix, fX * fInv, fY * fInv, fZ * fInv);
    return true;
}
}

namespace CollisionUtil {

// FUNCTION: SWBF2 0x004696b0
bool GetClosestPointInPolygon(const PblVector3& vPos, int iNumCorners, const PblVector3* pCorners, PblVector3& vOutPos,
                float fMaxDist)
{
    const PblVector3& c0 = pCorners[0];
    float e0x = pCorners[1].x - c0.x;
    float e0y = pCorners[1].y - c0.y;
    float e0z = pCorners[1].z - c0.z;
    float e1x = pCorners[2].x - pCorners[1].x;
    float e1y = pCorners[2].y - pCorners[1].y;
    float e1z = pCorners[2].z - pCorners[1].z;
    float nx = e0y * e1z - e0z * e1y;
    float ny = e0z * e1x - e0x * e1z;
    float nz = e0x * e1y - e0y * e1x;
    float fNormal2 = nx * nx + ny * ny + nz * nz;
    if (fNormal2 < 0.0001f)
    {
        return false;
    }

    float fDist = nx * (vPos.x - c0.x) + ny * (vPos.y - c0.y) + nz * (vPos.z - c0.z);
    if (fDist < 0.0f)
    {
        return false;
    }
    float fMaxDist2 = fMaxDist * fMaxDist;
    if (fDist * fDist > fNormal2 * fMaxDist2)
    {
        return false;
    }

    float fT = fDist / fNormal2;
    float px = vPos.x - nx * fT;
    float py = vPos.y - ny * fT;
    float pz = vPos.z - nz * fT;

    float fFirstStart = e0x * (px - c0.x) + e0y * (py - c0.y) + e0z * (pz - c0.z);
    float fEnd = -(e0x * (px - pCorners[1].x) + e0y * (py - pCorners[1].y) + e0z * (pz - pCorners[1].z));
    if (fFirstStart >= 0.0f && fEnd >= 0.0f &&
        (ny * e0z - nz * e0y) * (px - c0.x) + (nz * e0x - nx * e0z) * (py - c0.y) +
                (nx * e0y - ny * e0x) * (pz - c0.z) < 0.0f)
    {
        float fS = fFirstStart / (e0x * e0x + e0y * e0y + e0z * e0z);
        float qx = e0x * fS + c0.x;
        float qy = e0y * fS + c0.y;
        float qz = e0z * fS + c0.z;
        if ((vPos.x - qx) * (vPos.x - qx) + (vPos.y - qy) * (vPos.y - qy) + (vPos.z - qz) * (vPos.z - qz) <=
            fMaxDist2)
        {
            vOutPos = PblVector3(qx, qy, qz);
            return true;
        }
        return false;
    }

    for (int i = 1; i < iNumCorners; ++i)
    {
        const PblVector3& a = pCorners[i];
        const PblVector3& b = pCorners[i + 1 == iNumCorners ? 0 : i + 1];
        float ex = b.x - a.x;
        float ey = b.y - a.y;
        float ez = b.z - a.z;
        float fStart = ex * (px - a.x) + ey * (py - a.y) + ez * (pz - a.z);
        float fNextEnd = -(ex * (px - b.x) + ey * (py - b.y) + ez * (pz - b.z));
        if (fStart >= 0.0f && fNextEnd >= 0.0f &&
            (ny * ez - nz * ey) * (px - a.x) + (nz * ex - nx * ez) * (py - a.y) + (nx * ey - ny * ex) * (pz - a.z) <
                0.0f)
        {
            float fS = fStart / (ex * ex + ey * ey + ez * ez);
            float qx = ex * fS + a.x;
            float qy = ey * fS + a.y;
            float qz = ez * fS + a.z;
            if ((vPos.x - qx) * (vPos.x - qx) + (vPos.y - qy) * (vPos.y - qy) + (vPos.z - qz) * (vPos.z - qz) >
                fMaxDist2)
            {
                return false;
            }
            vOutPos = PblVector3(qx, qy, qz);
            return true;
        }
        if (fEnd <= 0.0f && fStart <= 0.0f)
        {
            if ((vPos.x - a.x) * (vPos.x - a.x) + (vPos.y - a.y) * (vPos.y - a.y) + (vPos.z - a.z) * (vPos.z - a.z) >
                fMaxDist2)
            {
                return false;
            }
            vOutPos = a;
            return true;
        }
        fEnd = fNextEnd;
    }

    if (fEnd > 0.0f || fFirstStart > 0.0f)
    {
        vOutPos = PblVector3(px, py, pz);
        return true;
    }
    if ((vPos.x - c0.x) * (vPos.x - c0.x) + (vPos.y - c0.y) * (vPos.y - c0.y) + (vPos.z - c0.z) * (vPos.z - c0.z) >
        fMaxDist2)
    {
        return false;
    }
    vOutPos = c0;
    return true;
}
}

static inline float Fabs(float f)
{
    return (float)fabs((double)f);
}

static inline void GetAxis(const PblMatrix& m, int i, PblVector3& v)
{
    switch (i)
    {
    case 0:
        v = PblVector3(m.m[0][0], m.m[0][1], m.m[0][2]);
        break;
    case 1:
        v = PblVector3(m.m[1][0], m.m[1][1], m.m[1][2]);
        break;
    case 2:
        v = PblVector3(m.m[2][0], m.m[2][1], m.m[2][2]);
        break;
    }
}

// FUNCTION: SWBF2 0x00468810
bool CollisionUtil::CollideBoxes(const PblVector3& vVelocityA, const PblMatrix& mA, float fWidthA, float fHeightA,
                                 float fDepthA, const PblVector3& vVelocityB, const PblMatrix& mB, float fWidthB,
                                 float fHeightB, float fDepthB, PblVector3& vNormal, float& fDepth)
{
    fDepth = fWidthA + fHeightA + fDepthA + fWidthB + fHeightB + fDepthB;

    PblMatrix mInvA = mA.GetInverseFast();
    PblMatrix mRel;
    D3DXMatrixMultiply((D3DXMATRIX*)&mRel, (const D3DXMATRIX*)&mB, (const D3DXMATRIX*)&mInvA);

    PblVector3 vRelVel(vVelocityB.x - vVelocityA.x, vVelocityB.y - vVelocityA.y, vVelocityB.z - vVelocityA.z);
    PblVector3 vRelDir;
    D3DXVec3Normalize((D3DXVECTOR3*)&vRelDir, (const D3DXVECTOR3*)&vRelVel);

    const float (*R)[4] = mRel.m;
    PblVector3 vAxis[3] = { PblVector3(R[0][0], R[0][1], R[0][2]), PblVector3(R[1][0], R[1][1], R[1][2]),
                            PblVector3(R[2][0], R[2][1], R[2][2]) };
    PblVector3 vOffset(R[3][0], R[3][1], R[3][2]);
    float fExtentA[3] = { fWidthA, fHeightA, fDepthA };
    float fExtentB[3] = { fWidthB, fHeightB, fDepthB };
    int i;

    for (i = 0; i < 3; ++i)
    {
        float fT = (&vOffset.x)[i];
        float fSep = Fabs((&vAxis[0].x)[i]) * fWidthB + Fabs((&vAxis[1].x)[i]) * fHeightB +
                     Fabs((&vAxis[2].x)[i]) * fDepthB + fExtentA[i] - Fabs(fT);
        if (0.0f > fSep)
        {
            return false;
        }
        if (fDepth > fSep)
        {
            fDepth = fSep;
            GetAxis(mA, i, vNormal);
            if (0.0f > fT)
            {
                vNormal = PblVector3(-vNormal.x, -vNormal.y, -vNormal.z);
            }
        }
    }

    PblVector3 v;
    for (i = 0; i < 3; ++i)
    {
        GetAxis(mRel, i, v);
        float fProj = R[3][0] * v.x + R[3][1] * v.y + R[3][2] * v.z;
        float fSep = Fabs(v.x) * fWidthA + Fabs(v.y) * fHeightA + Fabs(v.z) * fDepthA + fExtentB[i] - Fabs(fProj);
        if (0.0f > fSep)
        {
            return false;
        }
        if (fDepth > fSep)
        {
            fDepth = fSep;
            GetAxis(mB, i, vNormal);
            if (0.0f > fProj)
            {
                vNormal = PblVector3(-vNormal.x, -vNormal.y, -vNormal.z);
            }
        }
    }

    const float* T = R[3];

    if (0.0f > (Fabs(R[0][1]) * fDepthA + Fabs(R[0][2]) * fHeightA) +
                   (Fabs(R[1][0]) * fDepthB + Fabs(R[2][0]) * fHeightB) - Fabs(R[0][1] * T[2] - R[0][2] * T[1]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[1][1]) * fDepthA + Fabs(R[1][2]) * fHeightA) +
                   (Fabs(R[2][0]) * fWidthB + Fabs(R[0][0]) * fDepthB) - Fabs(R[1][1] * T[2] - R[1][2] * T[1]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[2][1]) * fDepthA + Fabs(R[2][2]) * fHeightA) +
                   (Fabs(R[1][0]) * fWidthB + Fabs(R[0][0]) * fHeightB) - Fabs(R[2][1] * T[2] - R[2][2] * T[1]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[0][2]) * fWidthA + Fabs(R[0][0]) * fDepthA) +
                   (Fabs(R[2][1]) * fHeightB + Fabs(R[1][1]) * fDepthB) - Fabs(R[0][2] * T[0] - R[0][0] * T[2]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[1][0]) * fDepthA + Fabs(R[1][2]) * fWidthA) +
                   (Fabs(R[0][1]) * fDepthB + Fabs(R[2][1]) * fWidthB) - Fabs(R[1][2] * T[0] - R[1][0] * T[2]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[2][0]) * fDepthA + Fabs(R[2][2]) * fWidthA) +
                   (Fabs(R[0][1]) * fHeightB + Fabs(R[1][1]) * fWidthB) - Fabs(R[2][2] * T[0] - R[2][0] * T[2]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[0][1]) * fWidthA + Fabs(R[0][0]) * fHeightA) +
                   (Fabs(R[2][2]) * fHeightB + Fabs(R[1][2]) * fDepthB) - Fabs(R[0][0] * T[1] - R[0][1] * T[0]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[1][0]) * fHeightA + Fabs(R[1][1]) * fWidthA) +
                   (Fabs(R[0][2]) * fDepthB + Fabs(R[2][2]) * fWidthB) - Fabs(R[1][0] * T[1] - R[1][1] * T[0]))
    {
        return false;
    }

    if (0.0f > (Fabs(R[2][0]) * fHeightA + Fabs(R[2][1]) * fWidthA) +
                   (Fabs(R[0][2]) * fHeightB + Fabs(R[1][2]) * fWidthB) - Fabs(R[2][0] * T[1] - R[2][1] * T[0]))
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x00468740
bool CollisionUtil::CollideCylinderBox(const PblVector3& vVelocityA, const PblMatrix& mA, float fRadius, float fHeight,
                                       const PblVector3& vVelocityB, const PblMatrix& mB, float fWidth, float fDepthB,
                                       float fHeightB, PblVector3& vNormal, float& fDepth)
{
    return CollideBoxes(vVelocityA, mA, fRadius, fHeight, fRadius, vVelocityB, mB, fWidth, fHeightB, fDepthB, vNormal,
                        fDepth);
}

// FUNCTION: SWBF2 0x00468790
bool CollisionUtil::CollideBoxCylinder(const PblVector3& vVelocityA, const PblMatrix& mA, float fWidth, float fDepthA,
                                       float fHeightA, const PblVector3& vVelocityB, const PblMatrix& mB, float fRadius,
                                       float fHeight, PblVector3& vNormal, float& fDepth)
{
    return CollideBoxes(vVelocityA, mA, fWidth, fHeightA, fDepthA, vVelocityB, mB, fRadius, fHeight, fRadius, vNormal,
                        fDepth);
}

namespace CollisionUtil {

// FUNCTION: SWBF2 0x004687e0
bool CollideCylinders(const PblMatrix& m1, float fRadius1, float fHalfHeight1, const PblMatrix& m2, float fRadius2,
                float fHalfHeight2, PblVector3& vNormal, float& fDepth)
{
    return CollisionUtil::CollideCapsules(m1, fRadius1, fHalfHeight1, m2, fRadius2, fHalfHeight2, vNormal, fDepth);
}
}

namespace CollisionUtil {

// FUNCTION: SWBF2 0x00469200
bool CollideCapsules(const PblMatrix& m1, float fRadius1, float fHalfHeight1, const PblMatrix& m2, float fRadius2,
                float fHalfHeight2, PblVector3& vNormal, float& fDepth)
{
    PblVector3 vStart1(m1.m[3][0] - m1.m[1][0] * fHalfHeight1, m1.m[3][1] - m1.m[1][1] * fHalfHeight1,
                       m1.m[3][2] - m1.m[1][2] * fHalfHeight1);
    PblVector3 vEnd1(m1.m[3][0] + m1.m[1][0] * fHalfHeight1, m1.m[3][1] + m1.m[1][1] * fHalfHeight1,
                     m1.m[3][2] + m1.m[1][2] * fHalfHeight1);
    PblVector3 vStart2(m2.m[3][0] - m2.m[1][0] * fHalfHeight2, m2.m[3][1] - m2.m[1][1] * fHalfHeight2,
                       m2.m[3][2] - m2.m[1][2] * fHalfHeight2);
    PblVector3 vEnd2(m2.m[3][0] + m2.m[1][0] * fHalfHeight2, m2.m[3][1] + m2.m[1][1] * fHalfHeight2,
                     m2.m[3][2] + m2.m[1][2] * fHalfHeight2);
    PblVector3 vPoint1, vPoint2;
    CollisionUtil::GetClosestPoints_SegmentSegment(vStart1, vEnd1, vStart2, vEnd2, vPoint1, vPoint2);

    float fX = vPoint2.x - vPoint1.x;
    float fY = vPoint2.y - vPoint1.y;
    float fZ = vPoint2.z - vPoint1.z;
    float fDist2 = fX * fX + fY * fY + fZ * fZ;
    if (0.0001f > fDist2)
    {
        vNormal = PblVector3(1.0f, 0.0f, 0.0f);
        if (0.0001f > 1.0f - (vNormal.x * m1.m[1][0] + vNormal.y * m1.m[1][1] + vNormal.z * m1.m[1][2]))
        {
            vNormal = PblVector3(0.0f, 0.0f, 1.0f);
        }
        fDepth = fRadius1 > fRadius2 ? fRadius1 : fRadius2;
        return true;
    }
    float fDist = (float)sqrt((double)fDist2);
    fDepth = fRadius1 + fRadius2 - fDist;
    if (!(fDepth > 0.0001f))
    {
        return false;
    }
    float fInv = 1.0f / fDist;
    vNormal = PblVector3(fX * fInv, fY * fInv, fZ * fInv);
    return true;
}
}

// FUNCTION: SWBF2 0x004694d0
bool CollisionUtil::CollideAxisAlignedCylinders(PblVector3& vDelta, float fRadiusA, float fRadiusB, float fHeightA,
                                                float fHeightB, float& fDepth)
{
    float fAbsY = Fabs(vDelta.y);
    float fDistXZ = sqrtf(vDelta.x * vDelta.x + vDelta.z * vDelta.z);
    float fRadii = fRadiusA + fRadiusB;
    if (fDistXZ >= fRadii)
    {
        return false;
    }
    float fHalfHeight = (fHeightA + fHeightB) * 0.5f;
    if (fAbsY >= fHalfHeight)
    {
        return false;
    }

    if (fHalfHeight - fAbsY > fRadii - fDistXZ)
    {
        vDelta.y = 0.0f;
        fDepth = fDistXZ;
        if (fDistXZ == 0.0f)
        {
            vDelta.z = -1.0f;
        }
        else
        {
            float fInv = 1.0f / fDistXZ;
            vDelta.x = vDelta.x * fInv;
            vDelta.y = fInv * vDelta.y;
            vDelta.z = vDelta.z * fInv;
        }
        fDepth = fRadii - fDepth;
    }
    else
    {
        vDelta.x = 0.0f;
        vDelta.z = 0.0f;
        fDepth = fAbsY;
        if (fAbsY == 0.0f)
        {
            vDelta.y = 1.0f;
        }
        else
        {
            float fInv = 1.0f / fAbsY;
            vDelta.x = vDelta.x * fInv;
            vDelta.y = fInv * vDelta.y;
            vDelta.z = vDelta.z * fInv;
        }
        fDepth = fHalfHeight - fDepth;
    }
    return true;
}

// FUNCTION: SWBF2 0x0046a210
bool CollisionUtil::CollideSphereCylinder(const PblVector3& vCenter, float fSphereRadius, const PblMatrix& matrix,
                                          float fRadius, float fHalfHeight, PblVector3& vNormal, float& fDepth)
{
    bool bHit = CollisionUtil::CollideCylinderSphere(matrix, fRadius, fHalfHeight, vCenter, fSphereRadius, vNormal, fDepth);
    vNormal = PblVector3(vNormal.x * -1.0f, vNormal.y * -1.0f, vNormal.z * -1.0f);
    return bHit;
}

// FUNCTION: SWBF2 0x0046a850
float CollisionUtil::RayHitSphere(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                                  const PblVector3& vCenter, float fRadius, PblVector3* pNormal)
{
    if (fRadius == 0.0f)
    {
        return 1.0f;
    }
    float fX = vCenter.x - vStart.x;
    float fY = vCenter.y - vStart.y;
    float fZ = vCenter.z - vStart.z;
    float fProj = vDir.x * fX + vDir.y * fY + vDir.z * fZ;
    if (-fRadius > fProj)
    {
        return 1.0f;
    }
    if (fProj > fLength + fRadius)
    {
        return 1.0f;
    }

    float fNearX = vStart.x + vDir.x * fProj;
    float fNearY = vStart.y + vDir.y * fProj;
    float fNearZ = vStart.z + vDir.z * fProj;
    float fDist2 = (fNearX - vCenter.x) * (fNearX - vCenter.x) + (fNearY - vCenter.y) * (fNearY - vCenter.y) +
                   (fNearZ - vCenter.z) * (fNearZ - vCenter.z);
    float fRadius2 = fRadius * fRadius;
    if (fDist2 > fRadius2)
    {
        return 1.0f;
    }

    float fHalfChord2 = fRadius2 - fDist2;
    if (fHalfChord2 > fProj * fProj)
    {
        if (pNormal)
        {
            float fLen2 = fX * fX + fY * fY + fZ * fZ;
            if (fLen2 > 0.0001f)
            {
                float fLen = (float)sqrt((double)fLen2);
                if (fLen > 0.0001f)
                {
                    float fInv = 1.0f / fLen;
                    *pNormal = PblVector3(fInv * fX, fInv * fY, fInv * fZ);
                    return 0.0f;
                }
            }
            *pNormal = vDir;
        }
        return 0.0f;
    }

    if (0.0f > fProj)
    {
        return 1.0f;
    }
    float fHalfChord = (float)sqrt((double)fHalfChord2);
    float fHit = fProj - fHalfChord;
    if (fHit > fLength)
    {
        return 1.0f;
    }
    if (pNormal)
    {
        float fInv = 1.0f / fRadius;
        *pNormal = PblVector3((fNearX - vDir.x * fHalfChord - vCenter.x) * fInv,
                              (fNearY - vDir.y * fHalfChord - vCenter.y) * fInv,
                              (fNearZ - vDir.z * fHalfChord - vCenter.z) * fInv);
    }
    return fHit / fLength;
}

namespace CollisionUtil {

// FUNCTION: SWBF2 0x0046ab20
float RayHitBubble(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblMatrix& matrix,
                 const PblVector3& vSize, PblVector3* pNormal)
{
    float fInvX = 1.0f / vSize.x;
    float fInvY = 1.0f / vSize.y;
    float fInvZ = 1.0f / vSize.z;

    float fDX = vStart.x - matrix.m[3][0];
    float fDY = vStart.y - matrix.m[3][1];
    float fDZ = vStart.z - matrix.m[3][2];
    float fSX = (matrix.m[0][0] * fDX + matrix.m[0][1] * fDY + matrix.m[0][2] * fDZ) * fInvX;
    float fSY = (matrix.m[1][0] * fDX + matrix.m[1][1] * fDY + matrix.m[1][2] * fDZ) * fInvY;
    float fSZ = (matrix.m[2][0] * fDX + matrix.m[2][1] * fDY + matrix.m[2][2] * fDZ) * fInvZ;
    float fC = fSX * fSX + fSY * fSY + fSZ * fSZ;
    if (fC < 1.0f)
    {
        return 1.0f;
    }

    float fRX = (matrix.m[0][0] * vDir.x + matrix.m[0][1] * vDir.y + matrix.m[0][2] * vDir.z) * (fInvX * fLength);
    float fRY = (matrix.m[1][0] * vDir.x + matrix.m[1][1] * vDir.y + matrix.m[1][2] * vDir.z) * (fInvY * fLength);
    float fRZ = (matrix.m[2][0] * vDir.x + matrix.m[2][1] * vDir.y + matrix.m[2][2] * vDir.z) * (fInvZ * fLength);
    float fB = fRX * fSX + fRY * fSY + fRZ * fSZ;
    if (fB >= 0.0f)
    {
        return 1.0f;
    }

    float fA = fRX * fRX + fRY * fRY + fRZ * fRZ;
    float fDisc = fB * fB - (fC - 1.0f) * fA;
    if (fDisc <= 0.0f)
    {
        return 1.0f;
    }
    float fEnd = fA + fB;
    if (fEnd < 0.0f && fEnd * fEnd > fDisc)
    {
        return 1.0f;
    }

    float fT = (-fB - (float)sqrt((double)fDisc)) / fA;
    if (pNormal)
    {
        PblVector3 vLocal((fSX + fRX * fT) * fInvX, (fSY + fRY * fT) * fInvY, (fSZ + fRZ * fT) * fInvZ);
        PblVector3 vWorld, vUnit;
        D3DXVec3TransformNormal((D3DXVECTOR3*)&vWorld, (const D3DXVECTOR3*)&vLocal, (const D3DXMATRIX*)&matrix);
        D3DXVec3Normalize((D3DXVECTOR3*)&vUnit, (const D3DXVECTOR3*)&vWorld);
        *pNormal = vUnit;
    }
    return fT;
}
}
