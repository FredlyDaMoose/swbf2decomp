#include "RedCamera.h"

#include <math.h>
#include <d3dx9math.h>

// FUNCTION: SWBF2 0x006cb820
RedCamera::VisibilityT RedCamera::ClassifyPoints(const PblVector3* pPoints, int iNumPoints) const
{
    if (iNumPoints <= 0)
    {
        return VISIBILITY_NONE;
    }

    bool hasInside = false;
    bool hasOutside = false;
    unsigned int sharedMask = 0;
    for (int i = 0; i < iNumPoints; ++i)
    {
        D3DXVECTOR3 point;
        D3DXVec3TransformCoord(&point, (const D3DXVECTOR3*)&pPoints[i],
                               (const D3DXMATRIX*)&_MatrixInverse);
        float depth = -point.z;
        unsigned int mask = 0;
        if (depth >= _fNear)
        {
            mask |= 1;
        }
        if (_fFar >= depth)
        {
            mask |= 2;
        }
        float width = depth * _fTanHalfFOVWidth;
        if (width >= point.x)
        {
            mask |= 4;
        }
        if (width >= -point.x)
        {
            mask |= 8;
        }
        float height = depth * _fTanHalfFOVHeight;
        if (height >= point.y)
        {
            mask |= 16;
        }
        if (height >= -point.y)
        {
            mask |= 32;
        }
        if (mask == 63)
        {
            hasInside = true;
        }
        else
        {
            hasOutside = true;
        }
        if (hasInside && hasOutside)
        {
            return VISIBILITY_PARTIAL;
        }
        sharedMask |= mask;
    }
    if (hasInside)
    {
        return VISIBILITY_FULL;
    }
    return sharedMask == 63 ? VISIBILITY_PARTIAL : VISIBILITY_NONE;
}

// FUNCTION: SWBF2 0x006cb5c0
RedCamera::VisibilityT RedCamera::ClassifyPlane(const PblVector3& vNormal, float fDist) const
{
    const float fCenter = _Matrix.m[3][0] * vNormal.x
                        + _Matrix.m[3][1] * vNormal.y
                        + _Matrix.m[3][2] * vNormal.z;
    const float fForward = _Matrix.m[2][0] * vNormal.x
                         + _Matrix.m[2][1] * vNormal.y
                         + _Matrix.m[2][2] * vNormal.z;
    const float fNear = fCenter - _fNear * fForward;
    if (fDist > fNear)
    {
        return VISIBILITY_PARTIAL;
    }
    const float fFar = fCenter - _fFar * fForward;
    if (fDist > fFar)
    {
        return VISIBILITY_PARTIAL;
    }
    const float fRightAbs = (float)fabs(_Matrix.m[0][0] * vNormal.x
                                      + _Matrix.m[0][1] * vNormal.y
                                      + _Matrix.m[0][2] * vNormal.z);
    const float fUpAbs = (float)fabs(_Matrix.m[1][0] * vNormal.x
                                   + _Matrix.m[1][1] * vNormal.y
                                   + _Matrix.m[1][2] * vNormal.z);
    const float fSpread = -fRightAbs * _fTanHalfFOVWidth - fUpAbs * _fTanHalfFOVHeight;
    if (fDist > fFar + _fFar * fSpread)
    {
        return VISIBILITY_PARTIAL;
    }
    return fDist > fNear + _fNear * fSpread ? VISIBILITY_PARTIAL : VISIBILITY_NONE;
}

class PblMatrix;
extern PblMatrix gMatrixIdentity;

// FUNCTION: SWBF2 0x006cb070
RedCamera::RedCamera()
{
    _fZoom = 1.0f;
    SetPerspective(2.0f, 600.0f, 1.0471976f, 0.75f);
}

// FUNCTION: SWBF2 0x006cb0d0
RedCamera::~RedCamera()
{
}

// FUNCTION: SWBF2 0x006caf00
RedCamera::RedCamera(const RedCamera& rCamera)
{
    _Matrix = rCamera._Matrix;
    _MatrixInverse = rCamera._MatrixInverse;
    _fNear = rCamera._fNear;
    _fFar = rCamera._fFar;
    _fZoom = rCamera._fZoom;
    _fUnzoomedTanHalfFOVWidth = rCamera._fUnzoomedTanHalfFOVWidth;
    _fUnzoomedTanHalfFOVHeight = rCamera._fUnzoomedTanHalfFOVHeight;
    _fTanHalfFOVWidth = rCamera._fTanHalfFOVWidth;
    _fTanHalfFOVHeight = rCamera._fTanHalfFOVHeight;
    _fSecHalfFOVWidth = rCamera._fSecHalfFOVWidth;
    _fSecHalfFOVHeight = rCamera._fSecHalfFOVHeight;
    _fCosHalfFOVWidth = rCamera._fCosHalfFOVWidth;
    _fCosHalfFOVHeight = rCamera._fCosHalfFOVHeight;
    _box = rCamera._box;
    _iNumXZPlanes = rCamera._iNumXZPlanes;
    for (int i = 0; i < _iNumXZPlanes; ++i)
    {
        _XZPlane[i] = rCamera._XZPlane[i];
    }
}

// SYNTHETIC: SWBF2 0x006cb120
// RedCamera::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006cb150
void RedCamera::AddXZPlane(const PblVector3& rA, const PblVector3& rB)
{
    float fDZ = rB.z - rA.z;
    float fDX = rB.x - rA.x;
    float fInvLen = 1.0f / sqrtf(fDZ * fDZ + fDX * fDX);

    _XZPlane[_iNumXZPlanes].x = -(fDZ * fInvLen);
    _XZPlane[_iNumXZPlanes].z = fDX * fInvLen;
    _XZPlane[_iNumXZPlanes].y = -(_XZPlane[_iNumXZPlanes].x * rA.x +
                                  _XZPlane[_iNumXZPlanes].z * rA.z);
    ++_iNumXZPlanes;
}

// FUNCTION: SWBF2 0x006cb240
RedCamera::VisibilityT RedCamera::ClassifyBoxInternal(const PblMatrix& rMatrix, const PblVector3& vCenter, const PblVector3& vExtents) const
{
    VisibilityT eResult = VISIBILITY_FULL;

    float fRadius = (float)fabs(rMatrix.m[0][2]) * vExtents.x
                  + (float)fabs(rMatrix.m[1][2]) * vExtents.y
                  + (float)fabs(rMatrix.m[2][2]) * vExtents.z;
    float fDist = -vCenter.z - _fFar;
    if (fDist > fRadius)
    {
        return VISIBILITY_NONE;
    }
    if (fDist > -fRadius)
    {
        eResult = VISIBILITY_PARTIAL;
    }
    fDist = _fNear + vCenter.z;
    if (fDist > fRadius)
    {
        return VISIBILITY_NONE;
    }
    if (fDist > -fRadius)
    {
        eResult = VISIBILITY_PARTIAL;
    }

    float fTan = _fTanHalfFOVWidth;
    float fCos = _fCosHalfFOVWidth;
    fDist = (vCenter.x + fTan * vCenter.z) * fCos;
    fRadius = (float)fabs((rMatrix.m[1][0] + fTan * rMatrix.m[1][2]) * fCos) * vExtents.y
            + (float)fabs((rMatrix.m[0][0] + fTan * rMatrix.m[0][2]) * fCos) * vExtents.x
            + (float)fabs((rMatrix.m[2][0] + fTan * rMatrix.m[2][2]) * fCos) * vExtents.z;
    if (fDist > fRadius)
    {
        return VISIBILITY_NONE;
    }
    if (fDist > -fRadius)
    {
        eResult = VISIBILITY_PARTIAL;
    }
    fDist = (fTan * vCenter.z - vCenter.x) * fCos;
    fRadius = (float)fabs((fTan * rMatrix.m[1][2] - rMatrix.m[1][0]) * fCos) * vExtents.y
            + (float)fabs((fTan * rMatrix.m[0][2] - rMatrix.m[0][0]) * fCos) * vExtents.x
            + (float)fabs((fTan * rMatrix.m[2][2] - rMatrix.m[2][0]) * fCos) * vExtents.z;
    if (fDist > fRadius)
    {
        return VISIBILITY_NONE;
    }
    if (fDist > -fRadius)
    {
        eResult = VISIBILITY_PARTIAL;
    }

    fTan = _fTanHalfFOVHeight;
    fCos = _fCosHalfFOVHeight;
    fDist = (vCenter.y + fTan * vCenter.z) * fCos;
    fRadius = (float)fabs((rMatrix.m[0][1] + fTan * rMatrix.m[0][2]) * fCos) * vExtents.x
            + (float)fabs((rMatrix.m[1][1] + fTan * rMatrix.m[1][2]) * fCos) * vExtents.y
            + (float)fabs((rMatrix.m[2][1] + fTan * rMatrix.m[2][2]) * fCos) * vExtents.z;
    if (fDist > fRadius)
    {
        return VISIBILITY_NONE;
    }
    if (fDist > -fRadius)
    {
        eResult = VISIBILITY_PARTIAL;
    }
    fDist = (fTan * vCenter.z - vCenter.y) * fCos;
    fRadius = (float)fabs((fTan * rMatrix.m[1][2] - rMatrix.m[1][1]) * fCos) * vExtents.y
            + (float)fabs((fTan * rMatrix.m[0][2] - rMatrix.m[0][1]) * fCos) * vExtents.x
            + (float)fabs((fTan * rMatrix.m[2][2] - rMatrix.m[2][1]) * fCos) * vExtents.z;
    if (fDist > fRadius)
    {
        return VISIBILITY_NONE;
    }
    if (fDist > -fRadius)
    {
        eResult = VISIBILITY_PARTIAL;
    }
    return eResult;
}

// FUNCTION: SWBF2 0x00692c70
RedCamera::VisibilityT RedCamera::ClassifyBox(const PblMatrix& rMatrix, const PblVector3& vExtents) const
{
    PblMatrix m;
    D3DXMatrixMultiply((D3DXMATRIX*)&m, (const D3DXMATRIX*)&rMatrix, (const D3DXMATRIX*)&_MatrixInverse);
    return ClassifyBoxInternal(m, *(const PblVector3*)&m.m[3][0], vExtents);
}

// FUNCTION: SWBF2 0x00433eb0
RedCamera::VisibilityT RedCamera::ClassifyAABox(const PblVector3& vCenter, const PblVector3& vExtents) const
{
    PblVector3 v;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&v, (const D3DXVECTOR3*)&vCenter, (const D3DXMATRIX*)&_MatrixInverse);
    return ClassifyBoxInternal(_MatrixInverse, v, vExtents);
}

// FUNCTION: SWBF2 0x006cb980
RedCamera::VisibilityT RedCamera::ClassifySphere(const PblVector3& vCenter, float fRadius, float fFar) const
{
    VisibilityT eResult = VISIBILITY_FULL;
    float fFarPlane = fFar >= 0.0f ? fFar : _fFar;

    float fDX = vCenter.x - _Matrix.m[3][0];
    float fDY = vCenter.y - _Matrix.m[3][1];
    float fDZ = vCenter.z - _Matrix.m[3][2];

    float fZ = -(_Matrix.m[2][0] * fDX + _Matrix.m[2][1] * fDY + _Matrix.m[2][2] * fDZ);
    if (fZ < _fNear + fRadius)
    {
        if (fZ < _fNear - fRadius)
        {
            return VISIBILITY_NONE;
        }
        eResult = VISIBILITY_PARTIAL;
    }
    else if (fZ > fFarPlane - fRadius)
    {
        if (fZ > fFarPlane + fRadius)
        {
            return VISIBILITY_NONE;
        }
        eResult = VISIBILITY_PARTIAL;
    }

    float fX = (float)fabs(_Matrix.m[0][0] * fDX + _Matrix.m[0][1] * fDY + _Matrix.m[0][2] * fDZ)
             - _fTanHalfFOVWidth * fZ;
    if (fX > _fSecHalfFOVWidth * fRadius)
    {
        return VISIBILITY_NONE;
    }
    if (fX > -(_fSecHalfFOVWidth * fRadius))
    {
        eResult = VISIBILITY_PARTIAL;
    }

    float fY = (float)fabs(_Matrix.m[1][0] * fDX + _Matrix.m[1][1] * fDY + _Matrix.m[1][2] * fDZ)
             - _fTanHalfFOVHeight * fZ;
    if (fY > _fSecHalfFOVHeight * fRadius)
    {
        return VISIBILITY_NONE;
    }
    if (fY > -(_fSecHalfFOVHeight * fRadius))
    {
        eResult = VISIBILITY_PARTIAL;
    }

    return eResult;
}

// FUNCTION: SWBF2 0x006cb700
RedCamera::VisibilityT RedCamera::ClassifyPoint(const PblVector3& vPoint, float fFar) const
{
    float fFarPlane = fFar >= 0.0f ? fFar : _fFar;

    float fDX = vPoint.x - _Matrix.m[3][0];
    float fDY = vPoint.y - _Matrix.m[3][1];
    float fDZ = vPoint.z - _Matrix.m[3][2];

    float fZ = -(_Matrix.m[2][0] * fDX + _Matrix.m[2][1] * fDY + _Matrix.m[2][2] * fDZ);
    if (fZ < _fNear || fZ > fFarPlane)
    {
        return VISIBILITY_NONE;
    }

    if ((float)fabs(_Matrix.m[0][0] * fDX + _Matrix.m[0][1] * fDY + _Matrix.m[0][2] * fDZ) > _fTanHalfFOVWidth * fZ)
    {
        return VISIBILITY_NONE;
    }

    if ((float)fabs(_Matrix.m[1][0] * fDX + _Matrix.m[1][1] * fDY + _Matrix.m[1][2] * fDZ) > _fTanHalfFOVHeight * fZ)
    {
        return VISIBILITY_NONE;
    }

    return VISIBILITY_FULL;
}

// FUNCTION: SWBF2 0x006cbb00
RedCamera::VisibilityT RedCamera::ClassifyXZBox(const PblVector3& vCenter, const PblVector3& vExtents) const
{
    VisibilityT eResult = VISIBILITY_FULL;
    for (int i = 0; i < _iNumXZPlanes; ++i)
    {
        const PblVector3& rPlane = _XZPlane[i];
        float fDist = rPlane.x * vCenter.x + rPlane.z * vCenter.z + rPlane.y;
        float fRadius = (float)fabs(rPlane.x * vExtents.x) + (float)fabs(rPlane.z * vExtents.z);
        if (fDist > fRadius)
        {
            return VISIBILITY_NONE;
        }
        if (fDist > -fRadius)
        {
            eResult = VISIBILITY_PARTIAL;
        }
    }
    return eResult;
}

// FUNCTION: SWBF2 0x006cbbc0
void RedCamera::ComputeProjMatrix(PblMatrix& rMatrix) const
{
    float fNear, fFar, fWidth, fHeight;
    GetFrustum(fNear, fFar, fWidth, fHeight);

    rMatrix = gMatrixIdentity;
    rMatrix.m[2][3] = 1.0f;
    rMatrix.m[3][3] = 0.0f;
    rMatrix.m[0][0] = fNear * 2.0f / fWidth;
    rMatrix.m[1][1] = fNear * 2.0f / fHeight;
    rMatrix.m[2][2] = PROJECTION_FAR_PLANE / (PROJECTION_FAR_PLANE - fNear);
    rMatrix.m[3][2] = fNear * -PROJECTION_FAR_PLANE / (PROJECTION_FAR_PLANE - fNear);
}

// FUNCTION: SWBF2 0x006cbc90
void RedCamera::GetAABound(geo::Box&) const
{
}

// FUNCTION: SWBF2 0x006cbca0
float RedCamera::GetFOV() const
{
    return atanf(_fUnzoomedTanHalfFOVWidth) * 2.0f;
}

// FUNCTION: SWBF2 0x006cbcd0
void RedCamera::GetFrustum(float& fNear, float& fFar, float& fWidth, float& fHeight) const
{
    float fScale = _fNear * 2.0f / _fZoom;
    fNear = _fNear;
    fFar = _fFar;
    fWidth = _fUnzoomedTanHalfFOVWidth * fScale;
    fHeight = _fUnzoomedTanHalfFOVHeight * fScale;
}

// FUNCTION: SWBF2 0x006cbd30
void RedCamera::GetInvViewTrans(PblMatrix& rMatrix) const
{
    rMatrix = _Matrix;
}

// FUNCTION: SWBF2 0x006cbd70
void RedCamera::GetPos(geo::Vector3<float>& rPos) const
{
    rPos = geo::Vector3<float>(_Matrix.m[3][0], _Matrix.m[3][1], _Matrix.m[3][2]);
}

// FUNCTION: SWBF2 0x006cbda0
void RedCamera::GetSphereBound(geo::Sphere& rSphere) const
{
    geo::Sphere sphere;
    GetPos(sphere.center_);
    sphere.radius_ = _fZoom > 1.01f ? 16.0f : 4.0f;
    rSphere = sphere;
}

// FUNCTION: SWBF2 0x006cbe20
void RedCamera::GetViewTrans(PblMatrix& rMatrix) const
{
    rMatrix = _MatrixInverse;
}

// FUNCTION: SWBF2 0x006cbe60
void RedCamera::SetFOV(float fFOV, float fAspectRatio)
{
    _fUnzoomedTanHalfFOVWidth = tanf(fFOV * 0.5f);
    _fUnzoomedTanHalfFOVHeight = _fUnzoomedTanHalfFOVWidth * fAspectRatio;
    UpdateFrustumDependents();
}

// FUNCTION: SWBF2 0x006cbea0
void RedCamera::SetFrustum(float fNear, float fFar, float fWidth, float fHeight)
{
    float fScale = 0.5f / fNear;
    _fNear = fNear;
    _fFar = fFar;
    _fUnzoomedTanHalfFOVWidth = fScale * fWidth;
    _fUnzoomedTanHalfFOVHeight = fScale * fHeight;
    UpdateFrustumDependents();
}

// FUNCTION: SWBF2 0x006cbef0
void RedCamera::SetMatrix(const PblMatrix& rMatrix)
{
    _Matrix = rMatrix;
    UpdateMatrixDependents();
}

// FUNCTION: SWBF2 0x006cbf30
void RedCamera::SetNearFarPlane(float fNear, float fFar)
{
    _fNear = fNear;
    _fFar = fFar;
    UpdateMatrixAndFrustumDependents();
}

// FUNCTION: SWBF2 0x006cbf60
void RedCamera::SetPerspective(float fNear, float fFar, float fFOV, float fAspectRatio)
{
    _fUnzoomedTanHalfFOVWidth = tanf(fFOV * 0.5f);
    _fNear = fNear;
    _fFar = fFar;
    _fUnzoomedTanHalfFOVHeight = _fUnzoomedTanHalfFOVWidth * fAspectRatio;
    UpdateFrustumDependents();
}

// FUNCTION: SWBF2 0x006cbfc0
void RedCamera::SetZoom(float fZoom)
{
    _fZoom = fZoom;
    UpdateFrustumDependents();
}

// FUNCTION: SWBF2 0x006cbfe0
const PblVector3 RedCamera::TransformCameraPointToProjectionSpace(const PblVector3& rPoint) const
{
    float fInvDepth = 1.0f / (_fNear - _fFar);
    float fW = -1.0f / rPoint.z;
    return PblVector3(rPoint.x / _fTanHalfFOVWidth * fW,
                      -(rPoint.y / _fTanHalfFOVHeight) * fW,
                      (-((_fFar + _fNear) * fInvDepth * rPoint.z) - _fNear * 2.0f * _fFar * fInvDepth) * fW);
}

// FUNCTION: SWBF2 0x006cc090
void RedCamera::UpdateFrustumDependents()
{
    _fTanHalfFOVWidth = _fUnzoomedTanHalfFOVWidth / _fZoom;
    _fTanHalfFOVHeight = _fUnzoomedTanHalfFOVHeight / _fZoom;
    _fSecHalfFOVWidth = sqrtf(_fTanHalfFOVWidth * _fTanHalfFOVWidth + 1.0f);
    _fSecHalfFOVHeight = sqrtf(_fTanHalfFOVHeight * _fTanHalfFOVHeight + 1.0f);
    _fCosHalfFOVWidth = 1.0f / _fSecHalfFOVWidth;
    _fCosHalfFOVHeight = 1.0f / _fSecHalfFOVHeight;
    UpdateMatrixAndFrustumDependents();
}

// FUNCTION: SWBF2 0x006cc160
void RedCamera::UpdateMatrixAndFrustumDependents()
{
    ComputeProjMatrix(_ProjMatrix);
    PblMatrix mViewProj;
    D3DXMatrixMultiply((D3DXMATRIX*)&mViewProj, (const D3DXMATRIX*)&_MatrixInverse,
                       (const D3DXMATRIX*)&_ProjMatrix);
    _ViewProjMatrix = mViewProj;

    const float* vRight = _Matrix.m[0];
    const float* vFwd = _Matrix.m[2];
    const float* vPos = _Matrix.m[3];

    PblVector3 vUp;
    vUp.x = vFwd[1] * vRight[2] - vFwd[2] * vRight[1];
    vUp.y = vFwd[2] * vRight[0] - vFwd[0] * vRight[2];
    vUp.z = vFwd[0] * vRight[1] - vFwd[1] * vRight[0];

    float fFarW = _fTanHalfFOVWidth * _fFar;
    float fFarH = _fFar * _fTanHalfFOVHeight;

    PblVector3 v[5];
    v[0].x = vPos[0];
    v[0].y = vPos[1];
    v[0].z = vPos[2];
    v[1].x = vPos[0] - vFwd[0] * _fFar + vRight[0] * fFarW - vUp.x * fFarH;
    v[1].y = vPos[1] - vFwd[1] * _fFar + vRight[1] * fFarW - vUp.y * fFarH;
    v[1].z = vPos[2] - vFwd[2] * _fFar + vRight[2] * fFarW - vUp.z * fFarH;
    v[2].x = vPos[0] - vFwd[0] * _fFar + vRight[0] * fFarW + vUp.x * fFarH;
    v[2].y = vPos[1] - vFwd[1] * _fFar + vRight[1] * fFarW + vUp.y * fFarH;
    v[2].z = vPos[2] - vFwd[2] * _fFar + vRight[2] * fFarW + vUp.z * fFarH;
    v[3].x = vPos[0] - vFwd[0] * _fFar - vRight[0] * fFarW + vUp.x * fFarH;
    v[3].y = vPos[1] - vFwd[1] * _fFar - vRight[1] * fFarW + vUp.y * fFarH;
    v[3].z = vPos[2] - vFwd[2] * _fFar - vRight[2] * fFarW + vUp.z * fFarH;
    v[4].x = vPos[0] - vFwd[0] * _fFar - vRight[0] * fFarW - vUp.x * fFarH;
    v[4].y = vPos[1] - vFwd[1] * _fFar - vRight[1] * fFarW - vUp.y * fFarH;
    v[4].z = vPos[2] - vFwd[2] * _fFar - vRight[2] * fFarW - vUp.z * fFarH;

    float fRightY = _fTanHalfFOVWidth * vFwd[1] + vRight[1];
    float fLeftY = _fTanHalfFOVWidth * vFwd[1] - vRight[1];
    float fTopY = _fTanHalfFOVHeight * vFwd[1] + vUp.y;
    float fBottomY = _fTanHalfFOVHeight * vFwd[1] - vUp.y;
    float fDirY = -vFwd[1];

    _box._vMin = v[0];
    _box._vMax = v[0];
    for (int i = 1; i < 5; i++)
    {
        _box.AddPoint(v[i]);
    }

    _iNumXZPlanes = 0;

    if (fDirY == 0.0f)
    {
        bool bUp = fTopY > 0.0f;
        if ((fRightY >= 0.0f) == bUp)
        {
            if (bUp) { AddXZPlane(v[0], v[1]); AddXZPlane(v[1], v[3]); AddXZPlane(v[3], v[0]); }
            else     { AddXZPlane(v[0], v[3]); AddXZPlane(v[3], v[1]); AddXZPlane(v[1], v[0]); }
        }
        else
        {
            if (bUp) { AddXZPlane(v[0], v[2]); AddXZPlane(v[2], v[4]); AddXZPlane(v[4], v[0]); }
            else     { AddXZPlane(v[0], v[4]); AddXZPlane(v[4], v[2]); AddXZPlane(v[2], v[0]); }
        }
        return;
    }

    bool bDown = fDirY < 0.0f;
    if ((fRightY <= 0.0f) == bDown)
    {
        if ((fTopY <= 0.0f) == bDown)
        {
            if (bDown) { AddXZPlane(v[0], v[3]); AddXZPlane(v[3], v[4]); AddXZPlane(v[4], v[1]); AddXZPlane(v[1], v[0]); }
            else       { AddXZPlane(v[0], v[1]); AddXZPlane(v[1], v[4]); AddXZPlane(v[4], v[3]); AddXZPlane(v[3], v[0]); }
        }
        else if ((fBottomY <= 0.0f) == bDown)
        {
            if (bDown) { AddXZPlane(v[0], v[2]); AddXZPlane(v[2], v[3]); AddXZPlane(v[3], v[4]); AddXZPlane(v[4], v[0]); }
            else       { AddXZPlane(v[0], v[4]); AddXZPlane(v[4], v[3]); AddXZPlane(v[3], v[2]); AddXZPlane(v[2], v[0]); }
        }
        else
        {
            if (bDown) { AddXZPlane(v[0], v[2]); AddXZPlane(v[2], v[3]); AddXZPlane(v[3], v[4]); AddXZPlane(v[4], v[1]); AddXZPlane(v[1], v[0]); }
            else       { AddXZPlane(v[0], v[1]); AddXZPlane(v[1], v[4]); AddXZPlane(v[4], v[3]); AddXZPlane(v[3], v[2]); AddXZPlane(v[2], v[0]); }
        }
    }
    else if ((fLeftY <= 0.0f) == bDown)
    {
        if ((fTopY <= 0.0f) == bDown)
        {
            if (bDown) { AddXZPlane(v[0], v[4]); AddXZPlane(v[4], v[1]); AddXZPlane(v[1], v[2]); AddXZPlane(v[2], v[0]); }
            else       { AddXZPlane(v[0], v[2]); AddXZPlane(v[2], v[1]); AddXZPlane(v[1], v[4]); AddXZPlane(v[4], v[0]); }
        }
        else if ((fBottomY <= 0.0f) == bDown)
        {
            if (bDown) { AddXZPlane(v[0], v[1]); AddXZPlane(v[1], v[2]); AddXZPlane(v[2], v[3]); AddXZPlane(v[3], v[0]); }
            else       { AddXZPlane(v[0], v[3]); AddXZPlane(v[3], v[2]); AddXZPlane(v[2], v[1]); AddXZPlane(v[1], v[0]); }
        }
        else
        {
            if (bDown) { AddXZPlane(v[0], v[4]); AddXZPlane(v[4], v[1]); AddXZPlane(v[1], v[2]); AddXZPlane(v[2], v[3]); AddXZPlane(v[3], v[0]); }
            else       { AddXZPlane(v[0], v[3]); AddXZPlane(v[3], v[2]); AddXZPlane(v[2], v[1]); AddXZPlane(v[1], v[4]); AddXZPlane(v[4], v[0]); }
        }
    }
    else if ((fTopY < 0.0f) == bDown)
    {
        if (bDown) { AddXZPlane(v[0], v[3]); AddXZPlane(v[3], v[4]); AddXZPlane(v[4], v[1]); AddXZPlane(v[1], v[2]); AddXZPlane(v[2], v[0]); }
        else       { AddXZPlane(v[0], v[2]); AddXZPlane(v[2], v[1]); AddXZPlane(v[1], v[4]); AddXZPlane(v[4], v[3]); AddXZPlane(v[3], v[0]); }
    }
    else if ((fBottomY < 0.0f) == bDown)
    {
        if (bDown) { AddXZPlane(v[0], v[1]); AddXZPlane(v[1], v[2]); AddXZPlane(v[2], v[3]); AddXZPlane(v[3], v[4]); AddXZPlane(v[4], v[0]); }
        else       { AddXZPlane(v[0], v[4]); AddXZPlane(v[4], v[3]); AddXZPlane(v[3], v[2]); AddXZPlane(v[2], v[1]); AddXZPlane(v[1], v[0]); }
    }
    else
    {

        if (bDown) { AddXZPlane(v[2], v[3]); AddXZPlane(v[3], v[4]); AddXZPlane(v[4], v[1]); AddXZPlane(v[1], v[2]); }
        else       { AddXZPlane(v[2], v[1]); AddXZPlane(v[1], v[4]); AddXZPlane(v[4], v[3]); AddXZPlane(v[3], v[2]); }
    }
}

// FUNCTION: SWBF2 0x006ccc10
void RedCamera::UpdateMatrixDependents()
{
    _MatrixInverse = _Matrix.GetInverseFast();
    UpdateMatrixAndFrustumDependents();
}
