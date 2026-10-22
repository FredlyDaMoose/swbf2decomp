#pragma once

#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Portals/Portal.h"

const float PROJECTION_FAR_PLANE = 10000.0f;

// VTABLE: SWBF2 0x00772e0c pvs::DynDisplayable
// VTABLE: SWBF2 0x00772e54 pvs::ICamera
class RedCamera : public pvs::BaseCamera
{
public:
    RedCamera();

    RedCamera(const RedCamera &);
    virtual ~RedCamera();

    virtual void GetAABound(geo::Box &) const;
    virtual void GetSphereBound(geo::Sphere &) const;
    virtual void GetPos(geo::Vector3<float> &) const;
    virtual void GetViewTrans(PblMatrix &) const;
    virtual void GetInvViewTrans(PblMatrix &) const;

    void SetMatrix(const PblMatrix& rMatrix);
    void SetPerspective(float fNear, float fFar, float fFOV, float fAspectRatio);
    void SetNearFarPlane(float fNear, float fFar);
    void GetFrustum(float& fNear, float& fFar, float& fWidth, float& fHeight) const;
    void ComputeProjMatrix(PblMatrix& rMatrix) const;

    float GetFOV() const;
    void SetFOV(float fFOV, float fAspectRatio);
    void SetFrustum(float fNear, float fFar, float fWidth, float fHeight);
    void SetZoom(float fZoom);
    const PblVector3 TransformCameraPointToProjectionSpace(const PblVector3& rPoint) const;

    const PblVector3 TransformWorldPointToProjectionSpace(const PblVector3& rPoint) const;

    enum VisibilityT
    {
        VISIBILITY_NONE = -1,
        VISIBILITY_PARTIAL = 0,
        VISIBILITY_FULL = 1
    };

    VisibilityT ClassifySphere(const PblVector3& vCenter, float fRadius, float fFar) const;

    VisibilityT ClassifyPoint(const PblVector3& vPoint, float fFar) const;

    VisibilityT ClassifyPoints(const PblVector3* pPoints, int iNumPoints) const;

    VisibilityT ClassifyPlane(const PblVector3& vNormal, float fDist) const;
    VisibilityT ClassifyBox(const PblMatrix& rMatrix, const PblVector3& vExtents) const;
    VisibilityT ClassifyAABox(const PblVector3& vCenter, const PblVector3& vExtents) const;

    VisibilityT ClassifyXZBox(const PblVector3& vCenter, const PblVector3& vExtents) const;

    const PblMatrix& GetMatrix() const { return _Matrix; }
    const PblMatrix& GetMatrixInverse() const { return _MatrixInverse; }
    float GetNearPlane() const { return _fNear; }
    float GetFarPlane() const { return _fFar; }

    float GetZoom() const { return _fZoom; }

    const PblBox& GetCameraBox() const { return _box; }

private:

    VisibilityT ClassifyBoxInternal(const PblMatrix& rMatrix, const PblVector3& vCenter, const PblVector3& vExtents) const;
    void AddXZPlane(const PblVector3& rA, const PblVector3& rB);
    void UpdateMatrixDependents();
    void UpdateFrustumDependents();
    void UpdateMatrixAndFrustumDependents();

public:
    PblMatrix _Matrix;
    PblMatrix _MatrixInverse;
    PblMatrix _ProjMatrix;
    PblMatrix _ViewProjMatrix;
    float _fNear;
    float _fFar;
    float _fUnzoomedTanHalfFOVWidth;
    float _fUnzoomedTanHalfFOVHeight;
    float _fZoom;
    float _fTanHalfFOVWidth;
    float _fTanHalfFOVHeight;
    float _fSecHalfFOVWidth;
    float _fSecHalfFOVHeight;
    float _fCosHalfFOVWidth;
    float _fCosHalfFOVHeight;
    PblBox _box;
    PblVector3 _XZPlane[5];
    int _iNumXZPlanes;
};

 
 
 
 
 
 
 
 
 
 
 
 
