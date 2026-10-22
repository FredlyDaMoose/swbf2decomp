#include <float.h>
#include <math.h>
#include <string.h>
#include <d3dx9math.h>

#include "CollisionPrimitive.h"
#include "CollisionUtil.h"
#include "CollisionMesh.h"
#include "CollisionObject.h"
#include "EntitySoldier.h"
#include "RedEngineFL/Util/RedStack.h"
#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Application/RedTerrain.h"

static inline const PblVector3& GetTranslation(const PblMatrix& m)
{
    return *(const PblVector3*)&m.m[3][0];
}

static inline PblVector3 ApplyRotation(const PblMatrix& m, const PblVector3& v)
{
    D3DXVECTOR3 r;
    D3DXVec3TransformNormal(&r, (const D3DXVECTOR3*)&v, (const D3DXMATRIX*)&m);
    return PblVector3(r.x, r.y, r.z);
}

static void AddCorners(PblVector3* pCorners, const PblVector3& x, const PblVector3& y, const PblVector3& z)
{
    for (int i = 0; i < 8; ++i)
    {
        float sx = (i & 4) ? -1.0f : 1.0f;
        float sy = (i & 2) ? -1.0f : 1.0f;
        float sz = (i & 1) ? -1.0f : 1.0f;
        pCorners[i].x += (sx * x.x + sy * y.x) + sz * z.x;
        pCorners[i].y += (sx * x.y + sy * y.y) + sz * z.y;
        pCorners[i].z += (sx * x.z + sy * y.z) + sz * z.z;
    }
}

// FUNCTION: SWBF2 0x004644b0
CollisionPrimitive::CollisionPrimitive()
    : mName(0)
{
}

// FUNCTION: SWBF2 0x004646a0
void CollisionPrimitive::SetSphere(float fRadius)
{
    mType = SPHERE;
    mRadius = fRadius;
    mMeshCollisionRadius = fRadius;
    mHeight = fRadius * 2.0f;
}

// FUNCTION: SWBF2 0x004646d0
void CollisionPrimitive::SetBubble(const PblVector3& vSize)
{
    mType = BUBBLE;
    mWidth = vSize.x;
    mHeight = vSize.y;
    mDepth = vSize.z;
    mRadius = mWidth;
    if (mHeight > mRadius)
    {
        mRadius = mHeight;
    }
    if (mDepth > mRadius)
    {
        mRadius = mDepth;
    }
    mMeshCollisionRadius = mRadius;
}

// FUNCTION: SWBF2 0x00464730
void CollisionPrimitive::SetAACylinder(float fRadius, float fHeight)
{
    mHeight = fHeight;
    mType = AXIS_ALIGNED_CYLINDER;
    mRadius = fRadius;
    mMeshCollisionRadius = fHeight * 0.5f;
}

// FUNCTION: SWBF2 0x00464750
void CollisionPrimitive::SetCylinder(float fRadius, float fHeight)
{
    mHeight = fHeight;
    mType = CYLINDER;
    mRadius = fRadius;
    float fHalfHeight = fHeight * 0.5f;
    mMeshCollisionRadius = (float)sqrt(fHalfHeight * fHalfHeight + fRadius * fRadius + fRadius * fRadius);
}

// FUNCTION: SWBF2 0x004647a0
void CollisionPrimitive::SetBox(float fWidth, float fHeight, float fDepth, float fRadius)
{
    mType = BOX;
    mWidth = fWidth;
    mDepth = fDepth;
    mHeight = fHeight;
    if (fRadius < 0.0f)
    {
        mRadius = (float)sqrt(fWidth * fWidth + fDepth * fDepth + fHeight * fHeight);
    }
    else
    {
        mRadius = fRadius;
    }
    mMeshCollisionRadius = (float)sqrt(fWidth * fWidth + fHeight * fHeight + fDepth * fDepth);
}

// FUNCTION: SWBF2 0x00464860
PblBox CollisionPrimitive::GetWorldspaceAABB(const PblMatrix& matrix) const
{
    PblVector3 corners[8];
    for (int i = 0; i < 8; ++i)
    {
        corners[i] = GetTranslation(matrix);
    }

    switch (mType)
    {
    case BOX:
        AddCorners(corners,
                   ApplyRotation(matrix, PblVector3(mWidth, 0.0f, 0.0f)),
                   ApplyRotation(matrix, PblVector3(0.0f, mHeight, 0.0f)),
                   ApplyRotation(matrix, PblVector3(0.0f, 0.0f, mDepth)));
        break;
    case AXIS_ALIGNED_CYLINDER:
        break;
    case CYLINDER:
        AddCorners(corners,
                   ApplyRotation(matrix, PblVector3(mRadius, 0.0f, 0.0f)),
                   ApplyRotation(matrix, PblVector3(0.0f, mHeight * 0.5f, 0.0f)),
                   ApplyRotation(matrix, PblVector3(0.0f, 0.0f, mRadius)));
        break;
    default:
        corners[0].x += mRadius;
        corners[0].y += mRadius;
        corners[0].z += mRadius;
        corners[1].x -= mRadius;
        corners[1].y -= mRadius;
        corners[1].z -= mRadius;
        break;
    }

    PblVector3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
    PblVector3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (int i = 0; i < 8; ++i)
    {
        if (vMin.x > corners[i].x)
        {
            vMin.x = corners[i].x;
        }
        if (vMin.y > corners[i].y)
        {
            vMin.y = corners[i].y;
        }
        if (vMin.z > corners[i].z)
        {
            vMin.z = corners[i].z;
        }
        if (corners[i].x > vMax.x)
        {
            vMax.x = corners[i].x;
        }
        if (corners[i].y > vMax.y)
        {
            vMax.y = corners[i].y;
        }
        if (corners[i].z > vMax.z)
        {
            vMax.z = corners[i].z;
        }
    }

    PblBox box;
    box._vMin = vMin;
    box._vMax = vMax;
    return box;
}

// FUNCTION: SWBF2 0x004652c0
bool CollisionPrimitive::CheckForCollision(const PblMatrix& mMatrix, const PblVector3& vVelocity,
                                           const CollisionPrimitive& other, const PblMatrix& otherMatrix,
                                           const PblVector3& otherVelocity, PblVector3& vNormal,
                                           float& fDepth) const
{
    switch (mType)
    {
    case SPHERE:
        return other.CollideSphere(otherVelocity, vVelocity, otherMatrix, GetTranslation(mMatrix),
                                   mRadius, vNormal, fDepth);
    case BOX:
        return other.CollideBox(otherVelocity, vVelocity, otherMatrix, mMatrix,
                                mWidth, mHeight, mDepth, vNormal, fDepth);
    case AXIS_ALIGNED_CYLINDER:
        return other.CollideAACylinder(otherVelocity, vVelocity, otherMatrix, GetTranslation(mMatrix),
                                       mRadius, mHeight, vNormal, fDepth);
    case CYLINDER:
        return other.CollideCylinder(otherVelocity, vVelocity, otherMatrix, mMatrix,
                                     mRadius, mHeight, vNormal, fDepth);
    default:
        return false;
    }
}

static inline PblVector3 InverseRotate(const PblMatrix& m, float x, float y, float z)
{
    return PblVector3(m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z,
                      m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z,
                      m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z);
}

// FUNCTION: SWBF2 0x00465550
float CollisionPrimitive::RayHit(const PblMatrix& matrix, const PblVector3& vStart, const PblVector3& vDir,
                                 float fLength, PblVector3* pNormal, float fRadius)
{
    if (fRadius > 0.0f)
    {
        PblVector3 vS(vStart.x - vDir.x * fRadius, vStart.y - vDir.y * fRadius, vStart.z - vDir.z * fRadius);
        switch (mType)
        {
        case SPHERE:
            return CollisionUtil::RayHitSphere(vS, vDir, fLength, GetTranslation(matrix), mRadius + fRadius,
                                               pNormal);
        case BUBBLE:
        {
            PblVector3 vSize(mWidth + fRadius, mHeight + fRadius, mDepth + fRadius);
            return CollisionUtil::RayHitBubble(vS, vDir, fLength, matrix, vSize, pNormal);
        }
        case BOX:
            return CollisionUtil::RayHitBox(vS, vDir, fLength, matrix, mWidth + fRadius, mDepth + fRadius, mHeight + fRadius,
                              pNormal);
        case AXIS_ALIGNED_CYLINDER:
            return CollisionUtil::RayHitAxisAlignedCylinder(vS, vDir, fLength, GetTranslation(matrix), mRadius + fRadius,
                              mHeight * 0.5f + fRadius, pNormal);
        case CYLINDER:
            return CollisionUtil::RayHitCylinder(vS, vDir, fLength, matrix, mRadius + fRadius, mHeight * 0.5f + fRadius, pNormal);
        default:
            return 0.0f;
        }
    }

    switch (mType)
    {
    case SPHERE:
        return CollisionUtil::RayHitSphere(vStart, vDir, fLength, GetTranslation(matrix), mRadius, pNormal);
    case BUBBLE:
    {
        PblVector3 vSize(mWidth, mHeight, mDepth);
        return CollisionUtil::RayHitBubble(vStart, vDir, fLength, matrix, vSize, pNormal);
    }
    case BOX:
        return CollisionUtil::RayHitBox(vStart, vDir, fLength, matrix, mWidth, mDepth, mHeight, pNormal);
    case AXIS_ALIGNED_CYLINDER:
        return CollisionUtil::RayHitAxisAlignedCylinder(vStart, vDir, fLength, GetTranslation(matrix), mRadius, mHeight * 0.5f, pNormal);
    case CYLINDER:
        return CollisionUtil::RayHitCylinder(vStart, vDir, fLength, matrix, mRadius, mHeight * 0.5f, pNormal);
    default:
        return 0.0f;
    }
}

namespace CollisionUtil {

// FUNCTION: SWBF2 0x0046b130
float RayHitBox(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblMatrix& matrix,
                 float fWidth, float fDepth, float fHeight, PblVector3* pNormal)
{
    const PblVector3& vPos = GetTranslation(matrix);
    PblVector3 vLocalStart = InverseRotate(matrix, vStart.x - vPos.x, vStart.y - vPos.y, vStart.z - vPos.z);
    PblVector3 vLocalDir = InverseRotate(matrix, vDir.x, vDir.y, vDir.z);

    float fHit = 0.0f;
    int face = FindAACenteredBoxRayIntersection(vLocalStart, vLocalDir, fLength, fWidth, fHeight, fDepth, fHit);
    if (face == 0)
    {
        return 1.0f;
    }
    if (pNormal)
    {
        switch (face)
        {
        case 1: *pNormal = matrix.GetXAxis(); break;
        case 2: *pNormal = matrix.GetYAxis(); break;
        case 3: *pNormal = matrix.GetZAxis(); break;
        case -1:
            *pNormal = PblVector3(matrix.m[0][0] * -1.0f, matrix.m[0][1] * -1.0f, matrix.m[0][2] * -1.0f);
            break;
        case -2:
            *pNormal = PblVector3(matrix.m[1][0] * -1.0f, matrix.m[1][1] * -1.0f, matrix.m[1][2] * -1.0f);
            break;
        case -3:
            *pNormal = PblVector3(matrix.m[2][0] * -1.0f, matrix.m[2][1] * -1.0f, matrix.m[2][2] * -1.0f);
            break;
        default:
            *pNormal = PblVector3(0.0f, 1.0f, 0.0f);
            break;
        }
    }
    return fHit;
}
}

// FUNCTION: SWBF2 0x0046b3e0
float RayHitAACylinder(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblVector3& vCenter,
                 float fRadius, float fHalfHeight, PblVector3* pNormal)
{
    PblVector3 vOffset(vStart.x - vCenter.x, vStart.y - vCenter.y, vStart.z - vCenter.z);
    if (CollisionUtil::IsPointInsideAxisAlignedCylinder(vOffset, fRadius, fHalfHeight))
    {
        if (pNormal)
        {
            pNormal->x = 0.0f;
            pNormal->z = 0.0f;
            if (vStart.y > vCenter.y)
            {
                pNormal->y = 1.0f;
            }
            else
            {
                pNormal->y = -1.0f;
            }
        }
        return 1.0f;
    }

    float fClipped = fLength;
    float fEndY = vDir.y * fLength + vStart.y;
    float fBottom = vCenter.y - fHalfHeight;
    float fTop = vCenter.y + fHalfHeight;
    if (fBottom > fEndY)
    {
        fClipped = (vStart.y - fBottom) / (vStart.y - fEndY) * fLength;
    }
    else if (fEndY > fTop)
    {
        fClipped = (vStart.y - fTop) / (vStart.y - fEndY) * fLength;
    }

    PblVector3 vFlat(vDir.x * fClipped, 0.0f, vDir.z * fClipped);
    float fFlatLen = sqrtf(vFlat.x * vFlat.x + vFlat.y * vFlat.y + vFlat.z * vFlat.z);
    if (fFlatLen < 0.0001f)
    {
        return 1.0f;
    }
    float fInv = 1.0f / fFlatLen;
    PblVector3 vFlatDir(vFlat.x * fInv, vFlat.y * fInv, vFlat.z * fInv);

    float fProj = (vCenter.x - vStart.x) * vFlatDir.x + (vCenter.y - vStart.y) * vFlatDir.y +
                  (vCenter.z - vStart.z) * vFlatDir.z;
    if (fProj < 0.0f || fProj > fFlatLen + fRadius)
    {
        return 1.0f;
    }
    float fDx = vStart.x + vFlatDir.x * fProj - vCenter.x;
    float fDz = vStart.z + vFlatDir.z * fProj - vCenter.z;
    float fDist2 = fDx * fDx + fDz * fDz;
    if (fDist2 > fRadius * fRadius)
    {
        return 1.0f;
    }
    float fEnter = fProj - sqrtf(fRadius * fRadius - fDist2);
    if (fEnter > fFlatLen)
    {
        return 1.0f;
    }

    float fHit = fEnter / fFlatLen * fClipped;
    if (pNormal)
    {
        *pNormal = PblVector3(vStart.x + vDir.x * fHit - vCenter.x, vStart.y + vDir.y * fHit - vCenter.y,
                              vStart.z + vDir.z * fHit - vCenter.z);
        pNormal->y = 0.0f;
        PblVector3 vUnit;
        D3DXVec3Normalize((D3DXVECTOR3*)&vUnit, (const D3DXVECTOR3*)pNormal);
        *pNormal = vUnit;
    }
    return fHit / fLength;
}

namespace CollisionUtil {

// FUNCTION: SWBF2 0x0046b710
float RayHitAxisAlignedCylinder(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblVector3& vCenter,
                 float fRadius, float fHalfHeight, PblVector3* pNormal)
{
    float fEndY = vDir.y * fLength + vStart.y;
    float fPlane;
    float fNormalY;

    float fTop = vCenter.y + fHalfHeight;
    if (vStart.y > fTop)
    {
        if (!(fTop > fEndY))
        {
            return 1.0f;
        }
        fPlane = fTop;
        fNormalY = 1.0f;
    }
    else
    {
        float fBottom = vCenter.y - fHalfHeight;
        if (!(fBottom > vStart.y))
        {
            return RayHitAACylinder(vStart, vDir, fLength, vCenter, fRadius, fHalfHeight, pNormal);
        }
        if (!(fEndY > fBottom))
        {
            return 1.0f;
        }
        fPlane = fBottom;
        fNormalY = -1.0f;
    }

    float t = (vStart.y - fPlane) / (vStart.y - fEndY);
    float dx = t * fLength * vDir.x + vStart.x - vCenter.x;
    float dz = t * fLength * vDir.z + vStart.z - vCenter.z;
    if (fRadius * fRadius > dx * dx + dz * dz)
    {
        if (pNormal)
        {
            *pNormal = PblVector3(0.0f, fNormalY, 0.0f);
        }
        return t;
    }

    t += 0.0001f;
    float fStep = t * fLength;
    PblVector3 vS(fStep * vDir.x + vStart.x, fStep * vDir.y + vStart.y, fStep * vDir.z + vStart.z);
    float fRest = 1.0f - t;
    float fHit = RayHitAACylinder(vS, vDir, fRest * fLength, vCenter, fRadius, fHalfHeight, pNormal);
    if (fHit < 1.0f)
    {
        return fRest * fHit + t;
    }
    return 1.0f;
}
}

namespace CollisionUtil {

// FUNCTION: SWBF2 0x0046b9c0
float RayHitCylinder(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblMatrix& matrix,
                 float fRadius, float fHalfHeight, PblVector3* pNormal)
{
    const PblVector3& vPos = GetTranslation(matrix);
    PblVector3 vLocalStart = InverseRotate(matrix, vStart.x - vPos.x, vStart.y - vPos.y, vStart.z - vPos.z);
    PblVector3 vLocalDir = InverseRotate(matrix, vDir.x, vDir.y, vDir.z);
    PblVector3 vCenter(0.0f, 0.0f, 0.0f);
    PblVector3 vLocalNormal;

    float fHit = CollisionUtil::RayHitAxisAlignedCylinder(vLocalStart, vLocalDir, fLength, vCenter, fRadius, fHalfHeight, &vLocalNormal);
    if (fHit < 1.0f && pNormal)
    {
        *pNormal = PblVector3(
            matrix.m[0][0] * vLocalNormal.x + matrix.m[1][0] * vLocalNormal.y + matrix.m[2][0] * vLocalNormal.z,
            matrix.m[0][1] * vLocalNormal.x + matrix.m[1][1] * vLocalNormal.y + matrix.m[2][1] * vLocalNormal.z,
            matrix.m[0][2] * vLocalNormal.x + matrix.m[1][2] * vLocalNormal.y + matrix.m[2][2] * vLocalNormal.z);
    }
    return fHit;
}
}

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x01eb9290
static PblMatrix sWorld2Collision;
// GLOBAL: SWBF2 0x01eb92d0
static PblMatrix sHostWorld2Collision;
// GLOBAL: SWBF2 0x01eb83d0
static PblMatrix sClientWorld2Collision;
// GLOBAL: SWBF2 0x01eb5394
static PblVector3 sLocalPos;
// GLOBAL: SWBF2 0x01eb3384
static PblVector3 sHostLocalPos;
// GLOBAL: SWBF2 0x01eb753c
static PblVector3 sClientLocalPos;
// GLOBAL: SWBF2 0x01f9c258
static float sRadius;
// GLOBAL: SWBF2 0x01f9c25c
static float sHostRadius;
// GLOBAL: SWBF2 0x01f9c260
static float sClientRadius;
// GLOBAL: SWBF2 0x01eb6680
static PblMatrix sPrimitiveInCollision;

extern PolyListContainer<100> sHostCollTriList;
extern PolyListContainer<100> sHostTriList;
extern PolyListContainer<100> sClientCollTriList;
extern PolyListContainer<100> sClientTriList;
// GLOBAL: SWBF2 0x007e6308
PolyListContainer<100>* sCollTriList = &sHostCollTriList;
// GLOBAL: SWBF2 0x007e630c
PolyListContainer<100>* sTriList = &sHostTriList;

// FUNCTION: SWBF2 0x004644c0
void CollisionPrimitive::EnterClient()
{
    sHostWorld2Collision = sWorld2Collision;
    sHostLocalPos = sLocalPos;
    sHostRadius = sRadius;
    sWorld2Collision = sClientWorld2Collision;
    sLocalPos = sClientLocalPos;
    sRadius = sClientRadius;
    sTriList = &sClientTriList;
    sCollTriList = &sClientCollTriList;
}

// FUNCTION: SWBF2 0x004645b0
void CollisionPrimitive::LeaveClient()
{
    sClientWorld2Collision = sWorld2Collision;
    sClientLocalPos = sLocalPos;
    sClientRadius = sRadius;
    sWorld2Collision = sHostWorld2Collision;
    sLocalPos = sHostLocalPos;
    sRadius = sHostRadius;
    sTriList = &sHostTriList;
    sCollTriList = &sHostCollTriList;
}

// FUNCTION: SWBF2 0x004653e0
bool CollisionPrimitive::RayTest(const PblMatrix& matrix, const PblVector3& vStart,
                                 const PblVector3& vDir, float fLength)
{
    float fHit;
    switch (mType)
    {
    case SPHERE:
        fHit = CollisionUtil::RayHitSphere(vStart, vDir, fLength, GetTranslation(matrix), mRadius, 0);
        break;
    case BUBBLE:
    {
        PblVector3 vSize(mWidth, mHeight, mDepth);
        fHit = CollisionUtil::RayHitBubble(vStart, vDir, fLength, matrix, vSize, 0);
        break;
    }
    case BOX:
        fHit = CollisionUtil::RayHitBox(vStart, vDir, fLength, matrix, mWidth, mDepth, mHeight, 0);
        break;
    case AXIS_ALIGNED_CYLINDER:
        fHit = CollisionUtil::RayHitAxisAlignedCylinder(vStart, vDir, fLength, GetTranslation(matrix), mRadius, mHeight * 0.5f, 0);
        break;
    case CYLINDER:
        fHit = CollisionUtil::RayHitCylinder(vStart, vDir, fLength, matrix, mRadius, mHeight * 0.5f, 0);
        break;
    default:
        return false;
    }
    return fHit < 1.0f;
}

// FUNCTION: SWBF2 0x0046c020
void CollisionPrimitive::Render(const PblMatrix&) const
{
}

// FUNCTION: SWBF2 0x00465cf0
bool CollisionPrimitive::CollideSphere(const PblVector3&, const PblVector3&, const PblMatrix& matrix,
                                       const PblVector3& vCenter, float fRadius, PblVector3& vNormal,
                                       float& fDepth) const
{
    switch (mType)
    {
    case SPHERE:
        return CollisionUtil::CollideSpheres(GetTranslation(matrix), mRadius, vCenter, fRadius, vNormal, fDepth);
    case BOX:
        return CollisionUtil::CollideBoxSphere(matrix, mWidth, mDepth, mHeight, vCenter, fRadius, vNormal, fDepth);
    case AXIS_ALIGNED_CYLINDER:
    {
        PblMatrix aligned = _IdentityMatrix;
        aligned.m[3][0] = matrix.m[3][0];
        aligned.m[3][1] = matrix.m[3][1];
        aligned.m[3][2] = matrix.m[3][2];
        aligned.m[3][3] = 1.0f;
        return CollisionUtil::CollideCylinderSphere(aligned, mRadius, mHeight * 0.5f, vCenter, fRadius, vNormal, fDepth);
    }
    case CYLINDER:
        return CollisionUtil::CollideCylinderSphere(matrix, mRadius, mHeight * 0.5f, vCenter, fRadius, vNormal, fDepth);
    default:
        return false;
    }
}

// FUNCTION: SWBF2 0x004658a0
bool CollisionPrimitive::CollideCylinder(const PblVector3& vVelocity, const PblVector3& otherVelocity,
                                         const PblMatrix& matrix, const PblMatrix& cylinderMatrix, float fRadius,
                                         float fHeight, PblVector3& vNormal, float& fDepth) const
{
    switch (mType)
    {
    case SPHERE:
        return CollisionUtil::CollideSphereCylinder(GetTranslation(matrix), mRadius, cylinderMatrix, fRadius,
                                                    fHeight, vNormal, fDepth);
    case BOX:
        return CollisionUtil::CollideBoxCylinder(vVelocity, matrix, mWidth, mDepth, mHeight, otherVelocity,
                                                 cylinderMatrix, fRadius, fHeight * 0.5f, vNormal, fDepth);
    case AXIS_ALIGNED_CYLINDER:
    {
        PblMatrix aligned = _IdentityMatrix;
        aligned.m[3][0] = matrix.m[3][0];
        aligned.m[3][1] = matrix.m[3][1];
        aligned.m[3][2] = matrix.m[3][2];
        aligned.m[3][3] = 1.0f;
        return CollisionUtil::CollideCylinders(aligned, mRadius, mHeight * 0.5f, cylinderMatrix, fRadius, fHeight * 0.5f, vNormal,
                          fDepth);
    }
    case CYLINDER:
        return CollisionUtil::CollideCylinders(matrix, mRadius, mHeight * 0.5f, cylinderMatrix, fRadius, fHeight * 0.5f, vNormal,
                          fDepth);
    default:
        return false;
    }
}

// FUNCTION: SWBF2 0x00465a70
bool CollisionPrimitive::CollideAACylinder(const PblVector3& vVelocity, const PblVector3& otherVelocity,
                                           const PblMatrix& matrix, const PblVector3& vCenter, float fRadius,
                                           float fHeight, PblVector3& vNormal, float& fDepth) const
{
    switch (mType)
    {
    case SPHERE:
    {
        PblMatrix aligned = _IdentityMatrix;
        aligned.m[3][0] = vCenter.x;
        aligned.m[3][1] = vCenter.y;
        aligned.m[3][2] = vCenter.z;
        aligned.m[3][3] = 1.0f;
        return CollisionUtil::CollideSphereCylinder(GetTranslation(matrix), mRadius, aligned, fRadius, fHeight,
                                                    vNormal, fDepth);
    }
    case BOX:
    {
        PblMatrix aligned = _IdentityMatrix;
        aligned.m[3][0] = vCenter.x;
        aligned.m[3][1] = vCenter.y;
        aligned.m[3][2] = vCenter.z;
        aligned.m[3][3] = 1.0f;
        return CollisionUtil::CollideBoxCylinder(vVelocity, matrix, mWidth, mDepth, mHeight, otherVelocity, aligned,
                                                 fRadius, fHeight * 0.5f, vNormal, fDepth);
    }
    case AXIS_ALIGNED_CYLINDER:
        vNormal = PblVector3(vCenter.x - matrix.m[3][0], vCenter.y - matrix.m[3][1], vCenter.z - matrix.m[3][2]);
        return CollisionUtil::CollideAxisAlignedCylinders(vNormal, mRadius, fRadius, mHeight, fHeight, fDepth);
    case CYLINDER:
    {
        PblMatrix aligned = _IdentityMatrix;
        aligned.m[3][0] = vCenter.x;
        aligned.m[3][1] = vCenter.y;
        aligned.m[3][2] = vCenter.z;
        aligned.m[3][3] = 1.0f;
        return CollisionUtil::CollideCylinders(matrix, mRadius, mHeight * 0.5f, aligned, fRadius, fHeight * 0.5f, vNormal, fDepth);
    }
    default:
        return false;
    }
}

// FUNCTION: SWBF2 0x00465e70
bool CollisionPrimitive::CollideBox(const PblVector3& vVelocity, const PblVector3& otherVelocity,
                                    const PblMatrix& matrix, const PblMatrix& boxMatrix, float fWidth, float fHeight,
                                    float fDepth, PblVector3& vNormal, float& fPenetration) const
{
    switch (mType)
    {
    case SPHERE:
        return CollisionUtil::CollideSphereBox(GetTranslation(matrix), mRadius, boxMatrix, fWidth, fDepth, fHeight, vNormal,
                          fPenetration);
    case BOX:
        return CollisionUtil::CollideBoxes(vVelocity, matrix, mWidth, mHeight, mDepth, otherVelocity, boxMatrix,
                                           fWidth, fHeight, fDepth, vNormal, fPenetration);
    case AXIS_ALIGNED_CYLINDER:
    {
        PblMatrix aligned = _IdentityMatrix;
        aligned.m[3][0] = matrix.m[3][0];
        aligned.m[3][1] = matrix.m[3][1];
        aligned.m[3][2] = matrix.m[3][2];
        aligned.m[3][3] = 1.0f;
        return CollisionUtil::CollideCylinderBox(vVelocity, aligned, mRadius, mHeight * 0.5f, otherVelocity,
                                                 boxMatrix, fWidth, fDepth, fHeight, vNormal, fPenetration);
    }
    case CYLINDER:
        return CollisionUtil::CollideCylinderBox(vVelocity, matrix, mRadius, mHeight * 0.5f, otherVelocity,
                                                 boxMatrix, fWidth, fDepth, fHeight, vNormal, fPenetration);
    default:
        return false;
    }
}

// TEMPLATE: SWBF2 0x004620a0
// PolyListContainer<100>::AddPolygon
template void PolyListContainer<100>::AddPolygon(unsigned char, const PblVector3*);

// FUNCTION: SWBF2 0x00461ba0
void CollisionMesh::__GetTriHits(const PblVector3& vPos, float fRadius2, const PblVector3& vMin, const PblVector3& vMax, CollisionTreeNode* pNode, const CollisionAABB& aabb, PolyListContainer<100>& list, CollisionMeshCache* pCache) const
{
    const CollisionMesh* pMesh = (const CollisionMesh*)this;
    RedStack stack;
    const float* pPos = &vPos.x;
    const float* pBoxMin = &aabb.boxmin.x;
    const float* pBoxMax = &aabb.boxmax.x;
    int i;
    if (pCache)
    {
        float fDist2 = 0.0f;
        for (i = 0; i < 3; ++i)
        {
            if (pBoxMin[i] > pPos[i])
            {
                fDist2 += (pPos[i] - pBoxMin[i]) * (pPos[i] - pBoxMin[i]);
            }
            else if (pPos[i] > pBoxMax[i])
            {
                fDist2 += (pPos[i] - pBoxMax[i]) * (pPos[i] - pBoxMax[i]);
            }
            if (fDist2 > fRadius2)
            {
                return;
            }
        }
    }
    else
    {
        for (i = 0; i < 3; ++i)
        {
            if ((&vMin.x)[i] > pBoxMax[i])
            {
                return;
            }
            if (pBoxMin[i] > (&vMax.x)[i])
            {
                return;
            }
        }
    }

    if (!pNode->isleaf)
    {

        CollisionTreeNode* pLChild = pNode + 1;
        CollisionTreeNode* pGChild = (CollisionTreeNode*)pNode->data;
        CollisionAABB lAABB(aabb, pLChild->aabb);
        CollisionAABB gAABB(aabb, pGChild->aabb);
        ((CollisionMesh*)pMesh)->__GetTriHits(vPos, fRadius2, vMin, vMax, pLChild, lAABB, list, pCache);
        ((CollisionMesh*)pMesh)->__GetTriHits(vPos, fRadius2, vMin, vMax, pGChild, gAABB, list, pCache);
        return;
    }

    CollisionTreeLeaf* pLeaf = (CollisionTreeLeaf*)pNode;
    PblVector3* pVerts = (PblVector3*)stack.Alloc(100 * sizeof(PblVector3));
    int iNumVerts = pLeaf->numVerts;
    if (iNumVerts > 100)
    {
        iNumVerts = 100;
    }
    const unsigned short* pIndices = pLeaf->GetIndexPointer();
    for (i = 0; i < iNumVerts; ++i)
    {
        pVerts[i] = pMesh->mPoslList[pIndices[i]];
    }
    list.AddPolygon((unsigned char)iNumVerts, pVerts);
    if (pCache && pCache->mNumPolys < 70)
    {
        pCache->mPolys[pCache->mNumPolys++] = pLeaf;
    }
}

// FUNCTION: SWBF2 0x00461df0
CollisionMeshCache* CollisionMesh::_GetTriHits(const PblVector3& vPos, float fRadius, const PblVector3& vMin, const PblVector3& vMax, PolyListContainer<100>& list, EntitySoldier* pSoldier) const
{
    const CollisionMesh* pMesh = (const CollisionMesh*)this;
    RedStack stack;
    CollisionMeshCache* pCache = NULL;
    bool bSearch = true;
    list.Clear();
    if (pSoldier)
    {
        PblVector3* pVerts = (PblVector3*)stack.Alloc(100 * sizeof(PblVector3));

        CollisionMeshCache* pCaches = (CollisionMeshCache*)pSoldier->mCollisionMeshCache;
        for (int i = 0; i < 3; ++i)
        {
            CollisionMeshCache& cache = pCaches[i];
            if (cache.mMesh != pMesh)
            {
                continue;
            }
            pCache = &cache;
            float fX = vPos.x - cache.mInitialPos.x;
            float fY = vPos.y - cache.mInitialPos.y;
            float fZ = vPos.z - cache.mInitialPos.z;
            if (fRadius * (0.65f * 0.65f) * fRadius > fX * fX + fY * fY + fZ * fZ && cache.mNumPolys < 70)
            {
                for (int k = 0; k < cache.mNumPolys; ++k)
                {
                    CollisionTreeLeaf* pLeaf = cache.mPolys[k];
                    int iNumVerts = pLeaf->numVerts;
                    if (iNumVerts > 100)
                    {
                        iNumVerts = 100;
                    }
                    const unsigned short* pIndices = pLeaf->GetIndexPointer();
                    for (int j = 0; j < iNumVerts; ++j)
                    {
                        pVerts[j] = pMesh->mPoslList[pIndices[j]];
                    }
                    list.AddPolygon((unsigned char)iNumVerts, pVerts);
                }
                bSearch = false;
                break;
            }
        }
        if (!pCache)
        {
            pCache = &pCaches[0];
            for (int i = 1; i < 3; ++i)
            {
                if (pCaches[i].mNumPolys < pCache->mNumPolys)
                {
                    pCache = &pCaches[i];
                }
            }
            pCache->lastPos = vPos;
            pCache->aClosest = fRadius;
        }
    }
    if (bSearch)
    {
        if (pCache)
        {
            pCache->mInitialPos = vPos;
            pCache->mMesh = const_cast<CollisionMesh*>(pMesh);
            pCache->mNumPolys = 0;
            ((CollisionMesh*)pMesh)->__GetTriHits(vPos, fRadius * (1.65f * 1.65f) * fRadius, vMin, vMax, pMesh->mRoot, pMesh->mAABB, list, pCache);
        }
        else
        {
            ((CollisionMesh*)pMesh)->__GetTriHits(vPos, fRadius * fRadius, vMin, vMax, pMesh->mRoot, pMesh->mAABB, list, NULL);
        }
    }
    return pCache;
}

// FUNCTION: SWBF2 0x00462050
CollisionMeshCache* CollisionMesh::GetTriHits(const PblVector3& vPos, float fRadius, const PblVector3& vMin, const PblVector3& vMax, PolyListContainer<100>& list, EntitySoldier* pSoldier) const
{
    const CollisionMesh* pMesh = (const CollisionMesh*)this;
    return ((CollisionMesh*)pMesh)->_GetTriHits(vPos, fRadius, vMin, vMax, list, pSoldier);
}

// FUNCTION: SWBF2 0x00467090
bool CollisionPrimitive::InitMeshCollision(const PblMatrix& matrix, const CollisionMesh* pMesh,
                                           const PblMatrix& meshMatrix, CollisionObject* pObject,
                                           CollisionMeshCache*& pCache) const
{
    if (mType != AXIS_ALIGNED_CYLINDER && mType != SPHERE && mType != CYLINDER && mType != BOX)
    {
        return false;
    }

    sRadius = mMeshCollisionRadius;
    sWorld2Collision = meshMatrix.GetInverseFast();
    D3DXVec3TransformCoord((D3DXVECTOR3*)&sLocalPos, (const D3DXVECTOR3*)&GetTranslation(matrix),
                           (const D3DXMATRIX*)&sWorld2Collision);
    for (int iRow = 0; iRow < 3; ++iRow)
    {
        D3DXVec3TransformNormal((D3DXVECTOR3*)&sPrimitiveInCollision.m[iRow][0],
                                (const D3DXVECTOR3*)&matrix.m[iRow][0], (const D3DXMATRIX*)&sWorld2Collision);
        sPrimitiveInCollision.m[iRow][3] = 0.0f;
    }
    sPrimitiveInCollision.m[3][0] = sLocalPos.x;
    sPrimitiveInCollision.m[3][1] = sLocalPos.y;
    sPrimitiveInCollision.m[3][2] = sLocalPos.z;
    sPrimitiveInCollision.m[3][3] = 1.0f;

    if (!CollisionObject::sCollidingWithMesh)
    {

        const PblVector3& vBoxMin = pObject->mAABB._vMin;
        const PblVector3& vBoxMax = pObject->mAABB._vMax;
        PblVector3 aCorners[8];
        int i;
        for (i = 0; i < 8; ++i)
        {
            aCorners[i] = PblVector3((i & 4) ? vBoxMax.x : vBoxMin.x, (i & 2) ? vBoxMax.y : vBoxMin.y,
                                     (i & 1) ? vBoxMax.z : vBoxMin.z);
        }
        for (i = 0; i < 8; ++i)
        {
            PblVector3 v;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&v, (const D3DXVECTOR3*)&aCorners[i],
                                   (const D3DXMATRIX*)&sWorld2Collision);
            aCorners[i] = v;
        }
        PblVector3 vMin = aCorners[0];
        PblVector3 vMax = aCorners[0];
        for (i = 1; i < 8; ++i)
        {
            if (vMin.x > aCorners[i].x)
            {
                vMin.x = aCorners[i].x;
            }
            else if (aCorners[i].x > vMax.x)
            {
                vMax.x = aCorners[i].x;
            }
            if (vMin.y > aCorners[i].y)
            {
                vMin.y = aCorners[i].y;
            }
            else if (aCorners[i].y > vMax.y)
            {
                vMax.y = aCorners[i].y;
            }
            if (vMin.z > aCorners[i].z)
            {
                vMin.z = aCorners[i].z;
            }
            else if (aCorners[i].z > vMax.z)
            {
                vMax.z = aCorners[i].z;
            }
        }

        sTriList->Clear();
        sCollTriList->Clear();
        EntitySoldier* pSoldier = NULL;
        if (pObject->GetEntity() && pObject->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            pSoldier = static_cast<EntitySoldier*>(pObject->GetEntity());
        }

        float fRadius = pObject->GetCollisionRadius();
        PblVector3 vCenter;
        D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter, (const D3DXVECTOR3*)&pObject->GetCollisionCenter(),
                               (const D3DXMATRIX*)&sWorld2Collision);
        pCache = ((CollisionMesh*)pMesh)->GetTriHits(vCenter, fRadius, vMin, vMax, *sTriList, pSoldier);
    }

    for (int iPoly = 0; iPoly < sCollTriList->mPolyCount; ++iPoly)
    {
        int iNumVerts;
        const PblVector3* pVerts;
        sCollTriList->GetPolygon(iPoly, iNumVerts, pVerts);
        sTriList->AddPolygon((unsigned char)iNumVerts, pVerts);
    }
    sCollTriList->Clear();
    return sTriList->mPolyCount > 0;
}

// FUNCTION: SWBF2 0x00467550
void CollisionPrimitive::MoveMeshTestPos(const PblVector3& vMove)
{
    PblVector3 v = ApplyRotation(sWorld2Collision, vMove);
    sLocalPos.x += v.x;
    sLocalPos.y += v.y;
    sLocalPos.z += v.z;
}

// TEMPLATE: SWBF2 0x004685f0
// PolyListContainer<100>::GetPolygon
template void PolyListContainer<100>::GetPolygon(int, int&, const PblVector3*&);

// TEMPLATE: SWBF2 0x00468670
// PolyListContainer<100>::RemovePolygon
template void PolyListContainer<100>::RemovePolygon(int);

// FUNCTION: SWBF2 0x00466060
int CollisionPrimitive::GetClosestPoint(PblVector3& vMidPoint, float& fMaxDist, PolyListContainer<100>& triList, PblVector3& vOutPoint, PblVector3& vClosestCylPoint, const PblMatrix&, const PblMatrix&) const
{
    const CollisionPrimitive* pPrim = (const CollisionPrimitive*)this;
    const PblMatrix& m = sPrimitiveInCollision;
    int iClosest = -1;
    float fClosestDist = -1.0f;
    if (pPrim->mType == CollisionPrimitive::BOX)
    {
        fMaxDist = 1.0f;
    }

    for (int iPoly = 0; iPoly < triList.mPolyCount; ++iPoly)
    {
        int iNumVerts;
        const PblVector3* pVerts;
        triList.GetPolygon(iPoly, iNumVerts, pVerts);
        if (iNumVerts > 100)
        {
            iNumVerts = 100;
        }
        float fRadius = pPrim->mRadius;

        PblVector3 aBoxVerts[100];
        if (pPrim->mType == CollisionPrimitive::BOX)
        {
            memcpy(aBoxVerts, pVerts, iNumVerts * sizeof(PblVector3));
            for (int k = 0; k < iNumVerts; ++k)
            {
                PblVector3& v = aBoxVerts[k];
                float dx = v.x - vMidPoint.x;
                float dy = v.y - vMidPoint.y;
                float dz = v.z - vMidPoint.z;
                float a = (dx * m.m[0][0] + dy * m.m[0][1] + dz * m.m[0][2]) / pPrim->mWidth;
                float b = (dx * m.m[1][0] + dy * m.m[1][1] + dz * m.m[1][2]) / pPrim->mHeight;
                float c = (dx * m.m[2][0] + dy * m.m[2][1] + dz * m.m[2][2]) / pPrim->mDepth;
                v.x = a * m.m[0][0] + b * m.m[1][0] + c * m.m[2][0] + vMidPoint.x;
                v.y = a * m.m[0][1] + b * m.m[1][1] + c * m.m[2][1] + vMidPoint.y;
                v.z = a * m.m[0][2] + b * m.m[1][2] + c * m.m[2][2] + vMidPoint.z;
            }
            pVerts = aBoxVerts;
        }

        PblVector3 vOutPos;
        if (!CollisionUtil::GetClosestPointInPolygon(vMidPoint, iNumVerts, pVerts, vOutPos, fMaxDist))
        {
            continue;
        }

        if (pPrim->mType == CollisionPrimitive::BOX)
        {
            float dx = vOutPos.x - vMidPoint.x;
            float dy = vOutPos.y - vMidPoint.y;
            float dz = vOutPos.z - vMidPoint.z;
            float fDist = sqrtf(dx * dx + dy * dy + dz * dz);
            if (fMaxDist > fDist && fDist < 1.0f)
            {
                fMaxDist = fDist;

                float a = (dx * m.m[0][0] + dy * m.m[0][1] + dz * m.m[0][2]) * pPrim->mWidth;
                float b = (dx * m.m[1][0] + dy * m.m[1][1] + dz * m.m[1][2]) * pPrim->mHeight;
                float c = (dx * m.m[2][0] + dy * m.m[2][1] + dz * m.m[2][2]) * pPrim->mDepth;
                vOutPoint.x = a * m.m[0][0] + b * m.m[1][0] + c * m.m[2][0] + vMidPoint.x;
                vOutPoint.y = a * m.m[0][1] + b * m.m[1][1] + c * m.m[2][1] + vMidPoint.y;
                vOutPoint.z = a * m.m[0][2] + b * m.m[1][2] + c * m.m[2][2] + vMidPoint.z;
                float ox = vOutPoint.x - vMidPoint.x;
                float oy = vOutPoint.y - vMidPoint.y;
                float oz = vOutPoint.z - vMidPoint.z;
                fClosestDist = sqrtf(ox * ox + oy * oy + oz * oz);
                iClosest = iPoly;
            }
        }
        else if (pPrim->mType == CollisionPrimitive::CYLINDER)
        {
            float fHalfHeight = pPrim->mHeight * 0.5f;
            float ux = fHalfHeight * m.m[1][0];
            float uy = fHalfHeight * m.m[1][1];
            float uz = fHalfHeight * m.m[1][2];
            PblVector3 vSegStart(sLocalPos.x - ux, sLocalPos.y - uy, sLocalPos.z - uz);
            PblVector3 vSegEnd(ux + sLocalPos.x, uy + sLocalPos.y, uz + sLocalPos.z);
            PblVector3 vAxisPoint;
            CollisionUtil::GetClosestPoint_SegmentPoint(vSegStart, vSegEnd, vOutPos, vAxisPoint);

            float ax = m.m[1][0];
            float ay = m.m[1][1];
            float az = m.m[1][2];
            float fX = vAxisPoint.x - vOutPos.x;
            float fY = vAxisPoint.y - vOutPos.y;
            float fZ = vAxisPoint.z - vOutPos.z;
            if ((float)fabs(fX * ax + fY * ay + fZ * az) < 0.0001f && fRadius * fRadius > fX * fX + fY * fY + fZ * fZ)
            {
                vClosestCylPoint = vAxisPoint;
                vOutPoint = vOutPos;
                iClosest = iPoly;
                continue;
            }

            float dx = vOutPos.x - sLocalPos.x;
            float dy = vOutPos.y - sLocalPos.y;
            float dz = vOutPos.z - sLocalPos.z;
            if (dx * ax + dy * ay + dz * az < 0.0f)
            {
                ax = -ax;
                ay = -ay;
                az = -az;
            }
            float n1x = dz * ay - dy * az;
            float n1y = dx * az - dz * ax;
            float n1z = dy * ax - dx * ay;
            PblVector3 vRadialIn(n1y * az - n1z * ay, n1z * ax - n1x * az, n1x * ay - n1y * ax);
            D3DXVECTOR3 vRadial;
            D3DXVec3Normalize(&vRadial, (const D3DXVECTOR3*)&vRadialIn);
            if (dx * vRadial.x + vRadial.y * dy + vRadial.z * dz < 0.0f)
            {
                vRadial = D3DXVECTOR3(-vRadial.x, -vRadial.y, -vRadial.z);
            }
            float fH = pPrim->mHeight * 0.5f;
            float fR = pPrim->mRadius;
            PblVector3 vSEnd(fH * ax + sLocalPos.x + fR * vRadial.x, fH * ay + sLocalPos.y + vRadial.y * fR,
                             fH * az + sLocalPos.z + vRadial.z * fR);
            PblVector3 vTangentIn(n1z * dy - n1y * dz, n1x * dz - n1z * dx, n1y * dx - n1x * dy);
            D3DXVECTOR3 vTangent;
            D3DXVec3Normalize(&vTangent, (const D3DXVECTOR3*)&vTangentIn);
            float fProj = (vSEnd.x - sLocalPos.x) * vTangent.x + (vSEnd.y - sLocalPos.y) * vTangent.y +
                          (vSEnd.z - sLocalPos.z) * vTangent.z;
            PblVector3 vTEnd(vTangent.x * fProj + vOutPos.x, vTangent.y * fProj + vOutPos.y,
                             vTangent.z * fProj + vOutPos.z);
            PblVector3 vSPoint, vTPoint;
            CollisionUtil::GetClosestPoints_SegmentSegment(sLocalPos, vSEnd, vOutPos, vTEnd, vSPoint, vTPoint);
            float sx = vSPoint.x - vTPoint.x;
            float sy = vSPoint.y - vTPoint.y;
            float sz = vSPoint.z - vTPoint.z;
            if (sx * sx + sy * sy + sz * sz >= 0.0001f)
            {
                continue;
            }

            float e0x = pVerts[1].x - pVerts[0].x;
            float e0y = pVerts[1].y - pVerts[0].y;
            float e0z = pVerts[1].z - pVerts[0].z;
            float e1x = pVerts[2].x - pVerts[0].x;
            float e1y = pVerts[2].y - pVerts[0].y;
            float e1z = pVerts[2].z - pVerts[0].z;
            float nx = e0y * e1z - e0z * e1y;
            float ny = e0z * e1x - e0x * e1z;
            float nz = e0x * e1y - e0y * e1x;
            bool bInside = true;
            int k = 0;
            while (bInside && k < iNumVerts - 1)
            {
                float px = pVerts[k].x - vSPoint.x;
                float py = pVerts[k].y - vSPoint.y;
                float pz = pVerts[k].z - vSPoint.z;
                float qx = pVerts[k + 1].x - vSPoint.x;
                float qy = pVerts[k + 1].y - vSPoint.y;
                float qz = pVerts[k + 1].z - vSPoint.z;
                if ((py * qz - pz * qy) * nx + (pz * qx - px * qz) * ny + (px * qy - py * qx) * nz >= 0.0f)
                {
                    ++k;
                }
                else
                {
                    bInside = false;
                }
            }
            if (!bInside)
            {
                continue;
            }

            float vx = fH * m.m[1][0];
            float vy = fH * m.m[1][1];
            float vz = fH * m.m[1][2];
            PblVector3 vStart(sLocalPos.x - vx, sLocalPos.y - vy, sLocalPos.z - vz);
            PblVector3 vEnd(vx + sLocalPos.x, vy + sLocalPos.y, vz + sLocalPos.z);
            CollisionUtil::GetClosestPoint_SegmentPoint(vStart, vEnd, vSPoint, vAxisPoint);
            vClosestCylPoint = vAxisPoint;
            vOutPoint = vSPoint;
            iClosest = iPoly;
        }
        else
        {
            float dx = vOutPos.x - vMidPoint.x;
            float dy = vOutPos.y - vMidPoint.y;
            float dz = vOutPos.z - vMidPoint.z;
            float fDist2 = dx * dx + dy * dy + dz * dz;
            if (iClosest < 0 || fMaxDist * fMaxDist > fDist2)
            {
                fMaxDist = sqrtf(fDist2);
                vOutPoint = vOutPos;
                iClosest = iPoly;
            }
        }
    }

    if (pPrim->mType == CollisionPrimitive::BOX)
    {
        fMaxDist = fClosestDist;
    }
    return iClosest;
}

// FUNCTION: SWBF2 0x004675b0
bool CollisionPrimitive::CollideMesh(const PblMatrix& matrix, const CollisionMesh* pMesh,
                                     const PblMatrix& meshMatrix, PblVector3& vCollisionDir,
                                     float& fCollisionOverlap, float& fClosest) const
{
    (void)pMesh;
    const PblMatrix& m = sPrimitiveInCollision;
    fClosest = sRadius;
    PblVector3 vOutPoint, vClosestCylPoint;
    int iIndex = GetClosestPoint(sLocalPos, fClosest, *sTriList, vOutPoint, vClosestCylPoint, matrix, meshMatrix);
    if (iIndex < 0)
    {
        return false;
    }

    if (mType == BOX)
    {
        float dx = sLocalPos.x - vOutPoint.x;
        float dy = sLocalPos.y - vOutPoint.y;
        float dz = sLocalPos.z - vOutPoint.z;
        vCollisionDir = PblVector3(dx, dy, dz);

        float fX = dx * m.m[0][0] + dy * m.m[0][1] + dz * m.m[0][2];
        float fY = dx * m.m[1][0] + dy * m.m[1][1] + dz * m.m[1][2];
        float fZ = dx * m.m[2][0] + dy * m.m[2][1] + dz * m.m[2][2];
        float a = fX / mWidth;
        float b = fY / mHeight;
        float c = fZ / mDepth;
        float ux = a * m.m[0][0] + b * m.m[1][0] + c * m.m[2][0];
        float uy = a * m.m[0][1] + b * m.m[1][1] + c * m.m[2][1];
        float uz = a * m.m[0][2] + b * m.m[1][2] + c * m.m[2][2];
        float fAbsX = (float)fabs(ux * m.m[0][0] + uy * m.m[0][1] + uz * m.m[0][2]);
        float fAbsY = (float)fabs(ux * m.m[1][0] + uy * m.m[1][1] + uz * m.m[1][2]);
        float fAbsZ = (float)fabs(ux * m.m[2][0] + uy * m.m[2][1] + uz * m.m[2][2]);
        int iAxis;
        float fSize, fAlong;
        if (fAbsX > fAbsY && fAbsX > fAbsZ)
        {
            iAxis = 0;
            fSize = mWidth;
            fAlong = fX;
        }
        else if (fAbsX <= fAbsY && fAbsY > fAbsZ)
        {
            iAxis = 1;
            fSize = mHeight;
            fAlong = fY;
        }
        else
        {
            iAxis = 2;
            fSize = mDepth;
            fAlong = fZ;
        }
        if (0.0f > fAlong)
        {
            vCollisionDir = PblVector3(-m.m[iAxis][0], -m.m[iAxis][1], -m.m[iAxis][2]);
        }
        else
        {
            vCollisionDir = PblVector3(m.m[iAxis][0], m.m[iAxis][1], m.m[iAxis][2]);
        }
        fCollisionOverlap = fSize - (float)fabs(fAlong);
    }
    else if (mType == CYLINDER)
    {
        float cx = vClosestCylPoint.x - vOutPoint.x;
        float cy = vClosestCylPoint.y - vOutPoint.y;
        float cz = vClosestCylPoint.z - vOutPoint.z;
        float fRadialOverlap = mRadius - sqrtf(cx * cx + cy * cy + cz * cz);
        float fHalfHeight = mHeight * 0.5f;
        float fAxial = (float)fabs((vOutPoint.x - sLocalPos.x) * m.m[1][0] + (vOutPoint.y - sLocalPos.y) * m.m[1][1] +
                                   (vOutPoint.z - sLocalPos.z) * m.m[1][2]);
        float fCapOverlap = fHalfHeight - fAxial;
        PblVector3 vToCyl(cx, cy, cz);
        if (fHalfHeight - fCapOverlap >= 0.0001f)
        {
            float dx = sLocalPos.x - vOutPoint.x;
            float dy = sLocalPos.y - vOutPoint.y;
            float dz = sLocalPos.z - vOutPoint.z;
            vCollisionDir = PblVector3(dx, dy, dz);
            float fDist = sqrtf(dx * dx + dy * dy + dz * dz);
            float fOverlap = fDist / (mHeight * 0.5f - fCapOverlap) * fCapOverlap;
            if ((mHeight * 0.25f * mHeight + mRadius * mRadius) * 0.8f < (fOverlap + fDist) * (fOverlap + fDist))
            {
                fCollisionOverlap = fRadialOverlap;
                D3DXVec3Normalize((D3DXVECTOR3*)&vCollisionDir, (const D3DXVECTOR3*)&vToCyl);
            }
            else
            {
                fCollisionOverlap = fOverlap;
                float fInv = 1.0f / fDist;
                vCollisionDir.x = fInv * vCollisionDir.x;
                vCollisionDir.y = fInv * vCollisionDir.y;
                vCollisionDir.z = fInv * vCollisionDir.z;
            }
        }
        else
        {
            fCollisionOverlap = fRadialOverlap;
            D3DXVec3Normalize((D3DXVECTOR3*)&vCollisionDir, (const D3DXVECTOR3*)&vToCyl);
        }
    }
    else
    {
        float dx = sLocalPos.x - vOutPoint.x;
        float dy = sLocalPos.y - vOutPoint.y;
        float dz = sLocalPos.z - vOutPoint.z;
        vCollisionDir = PblVector3(dx, dy, dz);
        fCollisionOverlap = sqrtf(dx * dx + dy * dy + dz * dz);
        if (fCollisionOverlap < 0.0001f)
        {
            return false;
        }
        float fInv = 1.0f / fCollisionOverlap;
        vCollisionDir.x = fInv * vCollisionDir.x;
        vCollisionDir.y = fInv * vCollisionDir.y;
        vCollisionDir.z = fInv * vCollisionDir.z;
        fCollisionOverlap = sRadius - fCollisionOverlap;
    }

    vCollisionDir = ApplyRotation(meshMatrix, vCollisionDir);
    if (CollisionObject::sCollideTrisWithAllPrimitives)
    {
        int iNumVerts;
        const PblVector3* pVerts = NULL;
        sTriList->GetPolygon(iIndex, iNumVerts, pVerts);
        sCollTriList->AddPolygon((unsigned char)iNumVerts, pVerts);
    }
    sTriList->RemovePolygon(iIndex);
    return true;
}

static inline PblVector3 Cross(const PblVector3& a, const PblVector3& b)
{
    return PblVector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

#pragma fenv_access(on)

static inline float TerrainDepth(const PblVector3& vPos, float fY, float fOffset)
{
    const float fAbove = RedTerrain::GetHoleHeight(vPos.x, vPos.z) - fY;
    return fAbove + fOffset;
}

// FUNCTION: SWBF2 0x00467dd0
bool CollisionPrimitive::CollideTerrain(const PblMatrix& matrix, PblVector3& vNormal, float& fDepth,
                                        PblVector3& vPos) const
{
    const PblVector3& vTrans = GetTranslation(matrix);
    float fOffset = mHeight * 0.5f;
    PblVector3 vSide(0.0f, 0.0f, 0.0f);
    vPos = PblVector3(vTrans.x - 0.0f, vTrans.y - fOffset, vTrans.z - 0.0f);

    if (mType == SPHERE)
    {
        fOffset = mRadius;
        vPos = PblVector3(vTrans.x - 0.0f, vTrans.y - mRadius, vTrans.z - 0.0f);
    }
    else if (mType == CYLINDER)
    {
        const float fHalfHeight = mHeight * 0.5f;
        const float fA = matrix.m[0][1] * mRadius;
        const float fB = matrix.m[1][1] * fHalfHeight;
        const float fC = matrix.m[2][1] * mRadius;
        fOffset = sqrtf(fA * fA + fB * fB + fC * fC);

        PblVector3 vDir(matrix.m[1][0], matrix.m[1][1], matrix.m[1][2]);
        if (vDir.y > 0.0f)
        {
            vDir = PblVector3(-vDir.x, -vDir.y, -vDir.z);
        }
        vPos = PblVector3(vTrans.x + vDir.x * fHalfHeight, vTrans.y + vDir.y * fHalfHeight,
                          vTrans.z + vDir.z * fHalfHeight);

        vSide = Cross(Cross(vDir, PblVector3(0.0f, -1.0f, 0.0f)), vDir);
        if (vSide.x * vSide.x + vSide.y * vSide.y + vSide.z * vSide.z > 0.01f)
        {
            PblVector3 vUnit;
            D3DXVec3Normalize((D3DXVECTOR3*)&vUnit, (const D3DXVECTOR3*)&vSide);
            vSide = vUnit;
        }
        vPos.x += mRadius * vSide.x;
        vPos.y += mRadius * vSide.y;
        vPos.z += mRadius * vSide.z;
        fDepth = TerrainDepth(vPos, vTrans.y, fOffset);
        if (fDepth < 0.0f || fDepth > fOffset * 4.0f)
        {
            const PblVector3 vRim = vPos;
            vPos.x -= mHeight * vDir.x;
            vPos.y -= mHeight * vDir.y;
            vPos.z -= mHeight * vDir.z;
            fDepth = TerrainDepth(vPos, vTrans.y, fOffset);
            if (fDepth < 0.0f || fDepth > fOffset * 4.0f)
            {
                const float fDiameter = mRadius * 2.0f;
                vPos = PblVector3(vRim.x - fDiameter * vSide.x, vRim.y - fDiameter * vSide.y,
                                  vRim.z - fDiameter * vSide.z);
                fDepth = TerrainDepth(vPos, vTrans.y, fOffset);
            }
        }
    }
    else if (mType == BOX)
    {

        float fX = 0.0f, fY = 0.0f, fZ = 0.0f;
        if (matrix.m[0][1] < -0.0001f)
        {
            fX = mWidth;
        }
        else if (matrix.m[0][1] > 0.0001f)
        {
            fX = -mWidth;
        }
        if (matrix.m[1][1] < -0.0001f)
        {
            fY = mHeight;
        }
        else if (matrix.m[1][1] > 0.0001f)
        {
            fY = -mHeight;
        }
        if (matrix.m[2][1] < -0.0001f)
        {
            fZ = mDepth;
        }
        else if (matrix.m[2][1] > 0.0001f)
        {
            fZ = -mDepth;
        }
        vPos = PblVector3(vTrans.x + matrix.m[0][0] * fX + matrix.m[1][0] * fY + matrix.m[2][0] * fZ,
                          vTrans.y + matrix.m[0][1] * fX + matrix.m[1][1] * fY + matrix.m[2][1] * fZ,
                          vTrans.z + matrix.m[0][2] * fX + matrix.m[1][2] * fY + matrix.m[2][2] * fZ);
        fOffset = vTrans.y - vPos.y;
    }

    if (mType != CYLINDER)
    {
        fDepth = TerrainDepth(vPos, vTrans.y, fOffset);
    }
    if (fDepth < 0.0f || fDepth > fOffset * 2.0f)
    {
        return false;
    }

    vNormal = RedTerrain::GetInterpolatedNormal(vPos.x, vPos.z);
    if (mType == CYLINDER)
    {

        const float fDot = matrix.m[1][0] * vNormal.x + matrix.m[1][1] * vNormal.y + matrix.m[1][2] * vNormal.z;
        if (1.0f - fabsf(fDot) < 0.0001f)
        {
            vPos.x -= mRadius * vSide.x;
            vPos.y -= mRadius * vSide.y;
            vPos.z -= mRadius * vSide.z;
        }
    }
    fDepth = vNormal.x * 0.0f + vNormal.y * fDepth + vNormal.z * 0.0f;
    return true;
}
#pragma fenv_access(off)

namespace CollisionUtil {

// FUNCTION: SWBF2 0x0046bf30
void GetClosestPoint_SegmentPoint(const PblVector3& vSegStart, const PblVector3& vSegEnd, const PblVector3& vPoint,
                PblVector3& vClosest)
{
    float fDX = vSegEnd.x - vSegStart.x;
    float fDY = vSegEnd.y - vSegStart.y;
    float fDZ = vSegEnd.z - vSegStart.z;
    float fProj = (vPoint.x - vSegStart.x) * fDX + (vPoint.y - vSegStart.y) * fDY + (vPoint.z - vSegStart.z) * fDZ;
    if (fProj <= 0.0f)
    {
        vClosest = vSegStart;
        return;
    }
    float fLen2 = fDX * fDX + fDY * fDY + fDZ * fDZ;
    if (fProj >= fLen2)
    {
        vClosest = vSegEnd;
        return;
    }
    float fT = fProj / fLen2;
    vClosest = PblVector3(vSegStart.x + fDX * fT, vSegStart.y + fDY * fT, vSegStart.z + fDZ * fT);
}
}

namespace CollisionUtil {

// FUNCTION: SWBF2 0x0046bc00
void GetClosestPoints_SegmentSegment(const PblVector3& vSStart, const PblVector3& vSEnd, const PblVector3& vTStart,
                const PblVector3& vTEnd, PblVector3& vSPoint, PblVector3& vTPoint)
{
    const float SMALL = 0.0001f;
    float fUX = vSEnd.x - vSStart.x, fUY = vSEnd.y - vSStart.y, fUZ = vSEnd.z - vSStart.z;
    float fVX = vTEnd.x - vTStart.x, fVY = vTEnd.y - vTStart.y, fVZ = vTEnd.z - vTStart.z;
    float fWX = vSStart.x - vTStart.x, fWY = vSStart.y - vTStart.y, fWZ = vSStart.z - vTStart.z;
    float a = fUX * fUX + fUY * fUY + fUZ * fUZ;
    float b = fVX * fUX + fVY * fUY + fVZ * fUZ;
    float c = fVX * fVX + fVY * fVY + fVZ * fVZ;
    float d = fWX * fUX + fWY * fUY + fWZ * fUZ;
    float e = fWX * fVX + fWY * fVY + fWZ * fVZ;
    float D = c * a - b * b;
    float sN, sD = D, tN, tD = D;

    if (D < SMALL)
    {
        sN = 0.0f;
        sD = 1.0f;
        tN = e;
        tD = c;
    }
    else
    {
        sN = e * b - d * c;
        tN = e * a - d * b;
        if (sN < 0.0f)
        {
            sN = 0.0f;
            tN = e;
            tD = c;
        }
        else if (sN > sD)
        {
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    if (tN < 0.0f)
    {
        tN = 0.0f;
        if (-d < 0.0f)
        {
            sN = 0.0f;
        }
        else if (-d > a)
        {
            sN = sD;
        }
        else
        {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD)
    {
        tN = tD;
        if (b - d < 0.0f)
        {
            sN = 0.0f;
        }
        else if (b - d > a)
        {
            sN = sD;
        }
        else
        {
            sN = b - d;
            sD = a;
        }
    }

    float sc = fabs(sN) < SMALL ? 0.0f : sN / sD;
    float tc = fabs(tN) < SMALL ? 0.0f : tN / tD;
    vSPoint = PblVector3(vSStart.x + fUX * sc, vSStart.y + fUY * sc, vSStart.z + fUZ * sc);
    vTPoint = PblVector3(vTStart.x + fVX * tc, vTStart.y + fVY * tc, vTStart.z + fVZ * tc);
}
}

// GLOBAL: SWBF2 0x01eb7548
// SYNTHETIC: SWBF2 0x004024e0
// `dynamic initializer for 'sHostTriList''

PolyListContainer<100> sHostTriList;

// GLOBAL: SWBF2 0x01eb5800
// SYNTHETIC: SWBF2 0x004024f0
// `dynamic initializer for 'sHostCollTriList''

PolyListContainer<100> sHostCollTriList;

// GLOBAL: SWBF2 0x01eb66c0
// SYNTHETIC: SWBF2 0x00402500
// `dynamic initializer for 'sClientTriList''

PolyListContainer<100> sClientTriList;

// GLOBAL: SWBF2 0x01eb8410
// SYNTHETIC: SWBF2 0x00402510
// `dynamic initializer for 'sClientCollTriList''

PolyListContainer<100> sClientCollTriList;
