#include <math.h>

#include "Primitives.h"
#include "basics.h"
#include "polygons.h"

namespace geo { bool __cdecl Overlap_(const geo::Frustum& rFrustum, const geo::Ray<geo::Vector3<float> >* pEdges, int iNumEdges); }

// FUNCTION: SWBF2 0x00724c60
bool geo::Overlap3D::Op(const geo::Frustum& rFrustum, const geo::Sphere& rSphere)
{
    const geo::Plane* pPlanes = rFrustum.GetPlanes();
    for (int i = 0; i < 6; ++i)
    {
        const float fDist = pPlanes[i].dir_.x * rSphere.center_.x
                          + pPlanes[i].dir_.y * rSphere.center_.y
                          + pPlanes[i].dir_.z * rSphere.center_.z
                          + rSphere.radius_;
        if (!(fDist > -pPlanes[i].dist_))
        {
            return false;
        }
    }
    return true;
}

namespace geo {

// FUNCTION: SWBF2 0x00724db0
bool __cdecl Overlap_(const geo::Frustum& rFrustum, const geo::Ray<geo::Vector3<float> >* pEdges, int iNumEdges)
{
    const geo::Plane* pPlanes = rFrustum.GetPlanes();
    for (int i = 0; i < iNumEdges; ++i)
    {
        for (int j = 0; j < 6; ++j)
        {
            geo::Vector3<float> hit;
            if (geo::Intersect3D::Op(pPlanes[j], pEdges[i], hit))
            {
                const geo::Sphere sphere(hit, 0.0001f);
                if (geo::Overlap3D::Op(rFrustum, sphere))
                {
                    return true;
                }
            }
        }
    }
    return false;
}
}

// FUNCTION: SWBF2 0x00723dc0
bool geo::Overlap2D::LineSegLineSeg(const geo::Vector2<float>& rA0, const geo::Vector2<float>& rA1,
                                    const geo::Vector2<float>& rB0, const geo::Vector2<float>& rB1)
{
    geo::Vector2<float> dirB;
    dirB.x = rB1.x - rB0.x;
    dirB.y = rB1.y - rB0.y;
    geo::Ray<geo::Vector2<float> > rayB(rB0, dirB);
    geo::Vector2<float> dirA;
    dirA.x = rA1.x - rA0.x;
    dirA.y = rA1.y - rA0.y;
    geo::Ray<geo::Vector2<float> > rayA(rA0, dirA);
    return Op(rayA, rayB);
}

// FUNCTION: SWBF2 0x007242c0
bool geo::Overlap2D::Op(const geo::Ray<geo::Vector2<float> >& rA, const geo::Ray<geo::Vector2<float> >& rB)
{
    float fDy = rA.orig_.y - rB.orig_.y;
    float fDx = rA.orig_.x - rB.orig_.x;
    float fDenom = rB.dir_.y * rA.dir_.x - rB.dir_.x * rA.dir_.y;
    float fTA = (fDy * rB.dir_.x - fDx * rB.dir_.y) / fDenom;
    float fTB = (fDy * rA.dir_.x - fDx * rA.dir_.y) / fDenom;
    return fTA >= 0.0f && fTA < 1.0f && fTB >= 0.0f && fTB < 1.0f;
}

// FUNCTION: SWBF2 0x00724970
bool __cdecl geo::Overlap3D::Op(const geo::Sphere& rSphere, const geo::Ray<geo::Vector3<float> >& rRay)
{
    float fDx = rRay.orig_.x - rSphere.center_.x;
    float fDy = rRay.orig_.y - rSphere.center_.y;
    float fDz = rRay.orig_.z - rSphere.center_.z;
    float fR2 = rSphere.radius_ * rSphere.radius_;
    float fC = fDy * fDy + fDx * fDx + fDz * fDz - fR2;
    if (fC < 0.0f)
    {
        return true;
    }

    float fEx = rRay.dir_.x + rRay.orig_.x - rSphere.center_.x;
    float fEy = rRay.dir_.y + rRay.orig_.y - rSphere.center_.y;
    float fEz = rRay.dir_.z + rRay.orig_.z - rSphere.center_.z;
    if (fEy * fEy + fEx * fEx + fEz * fEz - fR2 < 0.0f)
    {
        return true;
    }

    float fA = rRay.dir_.y * rRay.dir_.y + rRay.dir_.x * rRay.dir_.x + rRay.dir_.z * rRay.dir_.z;
    float fB = rRay.dir_.y * fDy * 2.0f + rRay.dir_.x * fDx * 2.0f + rRay.dir_.z * fDz * 2.0f;
    float fDisc = fB * fB - fA * 4.0f * fC;
    if (fDisc < 0.0f)
    {
        return false;
    }

    float fRoot = sqrtf(fDisc);
    float fInv = 0.5f / fA;
    float fT1 = (fRoot - fB) * fInv;
    float fT2 = (-fB - fRoot) * fInv;
    if (fT1 > 0.0f && fT1 < 1.0f)
    {
        return true;
    }
    if (fT2 > 0.0f && fT2 < 1.0f)
    {
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00724230
bool geo::Overlap2D::Op(const geo::Circle& rCircle, const geo::Ray<geo::Vector2<float> >& rRay)
{
    geo::Sphere sphere(geo::Vector3<float>(rCircle.center_.x, rCircle.center_.y, 0.0f), rCircle.radius_);
    geo::Ray<geo::Vector3<float> > ray(geo::Vector3<float>(rRay.orig_.x, rRay.orig_.y, 0.0f),
                                       geo::Vector3<float>(rRay.dir_.x, rRay.dir_.y, 0.0f));
    return geo::Overlap3D::Op(sphere, ray);
}

namespace geo
{

// FUNCTION: SWBF2 0x00723530
void BasePoly3D::GetEdges(Ray<Vector3<float> >* pEdges) const
{
    for (int i = 0; i < numPoints_; ++i)
    {
        const Vector3<float>& rPoint = points_[i];
        const Vector3<float>& rNext = points_[(i + 1) % numPoints_];
        pEdges[i].orig_ = rPoint;
        pEdges[i].dir_.x = rNext.x - rPoint.x;
        pEdges[i].dir_.y = rNext.y - rPoint.y;
        pEdges[i].dir_.z = rNext.z - rPoint.z;
    }
}

// FUNCTION: SWBF2 0x00722eb0
void Frustum::GetBaseEdges(Ray<Vector3<float> >* pEdges) const
{
    Vector3<float> right(up_.y * fwd_.z - up_.z * fwd_.y,
                         up_.z * fwd_.x - up_.x * fwd_.z,
                         up_.x * fwd_.y - up_.y * fwd_.x);
    const float fInvLength = 1.0f / (float)sqrt((double)(right.x * right.x + right.y * right.y + right.z * right.z));
    right.x *= fInvLength;
    right.y *= fInvLength;
    right.z *= fInvLength;
    for (int i = 0; i < 4; ++i)
    {
        const float fUp = (i == 0 || i == 3) ? height_ : -height_;
        const float fRight = (i == 0 || i == 1) ? width_ : -width_;
        pEdges[i].orig_ = center_;
        pEdges[i].dir_.x = (fwd_.x * far_ + up_.x * fUp) + right.x * fRight;
        pEdges[i].dir_.y = (fwd_.y * far_ + up_.y * fUp) + right.y * fRight;
        pEdges[i].dir_.z = (fwd_.z * far_ + up_.z * fUp) + right.z * fRight;
    }
}

// FUNCTION: SWBF2 0x007248a0
bool Overlap3D::Op(const ExtrudedPoly3D& rPoly, const Vector3<float>& rPoint)
{
    float fH = rPoly.zaxis_.x * rPoint.x + rPoly.zaxis_.y * rPoint.y + rPoly.zaxis_.z * rPoint.z;
    if (fH < rPoly.min_ || fH > rPoly.max_)
    {
        return false;
    }

    BasePoly2D poly(rPoly);
    if (poly.numPoints_ < 1)
    {
        return false;
    }

    Vector2<float> pt;
    pt.x = rPoly.xaxis_.x * rPoint.x + rPoly.xaxis_.y * rPoint.y + rPoly.xaxis_.z * rPoint.z;
    pt.y = rPoly.yaxis_.x * rPoint.x + rPoly.yaxis_.y * rPoint.y + rPoly.yaxis_.z * rPoint.z;
    return PolyPoly2D::PointInPolyCN(pt, poly);
}

// FUNCTION: SWBF2 0x007234b0
void BasePoly2D::GetEdges(Ray<Vector2<float> >* pEdges) const
{
    for (int i = 0; i < numPoints_; ++i)
    {
        const Vector2<float>& rCur = points_[i];
        const Vector2<float>& rNext = points_[(i + 1) % numPoints_];
        pEdges[i].orig_ = rCur;
        Vector2<float> dir;
        dir.x = rNext.x - rCur.x;
        dir.y = rNext.y - rCur.y;
        pEdges[i].dir_ = dir;
    }
}

// FUNCTION: SWBF2 0x007246e0
bool Overlap3D::Op(const ExtrudedPoly3D& rPoly, const Sphere& rSphere)
{
    const Vector3<float>& c = rSphere.center_;
    float fH = rPoly.zaxis_.x * c.x + rPoly.zaxis_.y * c.y + rPoly.zaxis_.z * c.z;
    if (rPoly.min_ > fH + rSphere.radius_ || fH - rSphere.radius_ > rPoly.max_)
    {
        return false;
    }

    BasePoly2D poly(rPoly);
    if (poly.numPoints_ < 1)
    {
        return false;
    }

    if (Op(rPoly, c))
    {
        return true;
    }

    Ray<Vector2<float> > edges[128];
    poly.GetEdges(edges);

    Vector2<float> center;
    center.x = rPoly.xaxis_.x * c.x + rPoly.xaxis_.y * c.y + rPoly.xaxis_.z * c.z;
    center.y = rPoly.yaxis_.x * c.x + rPoly.yaxis_.y * c.y + rPoly.yaxis_.z * c.z;
    Circle circle(center, rSphere.radius_);

    for (int i = 0; i < poly.numPoints_; ++i)
    {
        if (Overlap2D::Op(circle, edges[i]))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00722e20
float Box::Volume()
{
    float fDx = max_.x - min_.x;
    float fDy = max_.y - min_.y;
    float fDz = max_.z - min_.z;
    return Max_(fDy, 0.0f) * Max_(fDx, 0.0f) * Max_(fDz, 0.0f);
}

// FUNCTION: SWBF2 0x00723f20
bool Intersect3D::Op(const Box& rA, const Box& rB, Box& rOut)
{
    rOut.min_ = Vector3<float>(Max_(rA.min_.x, rB.min_.x), Max_(rA.min_.y, rB.min_.y), Max_(rA.min_.z, rB.min_.z));
    rOut.max_ = Vector3<float>(Min_(rA.max_.x, rB.max_.x), Min_(rA.max_.y, rB.max_.y), Min_(rA.max_.z, rB.max_.z));
    return rOut.Volume() > 0.0f;
}

// FUNCTION: SWBF2 0x00724d40
float SepDistance::Op(const Sphere& rA, const Sphere& rB)
{
    return length(rA.center_ - rB.center_) - rA.radius_ - rB.radius_;
}

// FUNCTION: SWBF2 0x007235c0
void Frustum::GetLeftPlane(Plane& rPlane) const
{
    Vector3<float> side = normalize(cross(up_, fwd_)) * width_;
    Vector3<float> edge = normalize(fwd_ * far_ - side);
    rPlane.dir_ = normalize(cross(up_, edge));
    rPlane.dist_ = dot(rPlane.dir_, center_) * -1.0f;
}

// FUNCTION: SWBF2 0x00723900
void Frustum::GetRightPlane(Plane& rPlane) const
{
    Vector3<float> side = normalize(cross(up_, fwd_)) * width_;
    Vector3<float> edge = normalize(side + fwd_ * far_);
    rPlane.dir_ = normalize(cross(edge, up_));
    rPlane.dist_ = dot(rPlane.dir_, center_) * -1.0f;
}

// FUNCTION: SWBF2 0x00723240
void Frustum::GetBottomPlane(Plane& rPlane) const
{
    Vector3<float> side = normalize(cross(up_, fwd_));
    Vector3<float> edge = normalize(fwd_ * far_ - up_ * height_);
    rPlane.dir_ = normalize(cross(edge, side));
    rPlane.dist_ = dot(rPlane.dir_, center_) * -1.0f;
}

// FUNCTION: SWBF2 0x00723b50
void Frustum::GetTopPlane(Plane& rPlane) const
{
    Vector3<float> side = normalize(cross(up_, fwd_));
    Vector3<float> edge = normalize(fwd_ * far_ + up_ * height_);
    rPlane.dir_ = normalize(cross(side, edge));
    rPlane.dist_ = dot(rPlane.dir_, center_) * -1.0f;
}

// FUNCTION: SWBF2 0x00723810
void Frustum::GetPlanes_(Plane* pPlanes) const
{
    pPlanes[0].dir_ = fwd_;
    pPlanes[0].dist_ = (dot(pPlanes[0].dir_, center_) + near_) * -1.0f;
    pPlanes[1].dir_ = fwd_ * -1.0f;
    pPlanes[1].dist_ = (dot(pPlanes[1].dir_, center_) - far_) * -1.0f;
    GetLeftPlane(pPlanes[2]);
    GetRightPlane(pPlanes[3]);
    GetBottomPlane(pPlanes[4]);
    GetTopPlane(pPlanes[5]);
}

}

// FUNCTION: SWBF2 0x00723e50
unsigned int geo::Classify3D::Op(const geo::Frustum& rFrustum, const geo::Sphere& rSphere)
{
    const geo::Plane* pPlanes = rFrustum.GetPlanes();
    unsigned int uResult = 1;
    for (int i = 0; i < 6; ++i)
    {
        float fD = geo::dot(pPlanes[i].dir_, rSphere.center_);
        float fNeg = -pPlanes[i].dist_;
        unsigned int uSide = (fD - rSphere.radius_ > fNeg ? 1 : 0) | (fNeg > rSphere.radius_ + fD ? 2 : 0);
        if (uSide == 2)
        {
            return 2;
        }
        if (uSide == 0)
        {
            uResult = 0;
        }
    }
    return uResult;
}

namespace geo {

// FUNCTION: SWBF2 0x00724e80
bool __cdecl Overlap_(const geo::Plane* pPlanes, const geo::Vector3<float>& rPoint)
{
    for (int i = 0; i < 6; ++i)
    {
        if (!(geo::dot(pPlanes[i].dir_, rPoint) > -pPlanes[i].dist_))
        {
            return false;
        }
    }
    return true;
}
}

// FUNCTION: SWBF2 0x00724000
bool __cdecl geo::Intersect3D::Op(const geo::Plane& rPlane, const geo::Ray<geo::Vector3<float> >& rRay, geo::Vector3<float>& rHit)
{
    float fDenom = geo::dot(rPlane.dir_, rRay.dir_);
    if (geo::Max_(geo::Max_((float)fabs(fDenom), (float)fabs(0.0f)), 1.0f) * 1.52587890625e-05f
        >= (float)fabs(fDenom - 0.0f))
    {
        return false;
    }

    float fT = (-rPlane.dist_ - geo::dot(rPlane.dir_, rRay.orig_)) / fDenom;
    if (!(fT >= 0.0f - geo::Max_(geo::Max_((float)fabs(fT), (float)fabs(0.0f)), 1.0f) * 6.103515625e-05f))
    {
        return false;
    }
    if (!(geo::Max_(geo::Max_((float)fabs(fT), (float)fabs(1.0f)), 1.0f) * 6.103515625e-05f + 1.0f >= fT))
    {
        return false;
    }

    rHit = rRay.orig_ + rRay.dir_ * fT;
    return true;
}

// FUNCTION: SWBF2 0x00724b20
bool __cdecl geo::Overlap3D::Op(const geo::Frustum& rFrustum, const geo::BasePoly3D& rPoly)
{
    const geo::Plane* pPlanes = rFrustum.GetPlanes();
    int iNumPoints = rPoly.numPoints_;
    for (int i = 0; i < iNumPoints; ++i)
    {
        if (geo::Overlap_(pPlanes, rPoly.points_[i]))
        {
            return true;
        }
    }

    geo::Ray<geo::Vector3<float> > edges[128];
    rPoly.GetEdges(edges);
    if (geo::Overlap_(rFrustum, edges, rPoly.numPoints_))
    {
        return true;
    }

    rFrustum.GetBaseEdges(edges);
    for (int i = 0; i < 4; ++i)
    {
        if (geo::Overlap3D::Op(rPoly, edges[i]))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00724d00
bool geo::Overlap3D::Op(const geo::Frustum& rFrustum, const geo::SphereBoundedPoly3D& rPoly)
{
    unsigned int uClass = geo::Classify3D::Op(rFrustum, rPoly.sphere_);
    if (uClass == 1)
    {
        return true;
    }
    if (uClass == 2)
    {
        return false;
    }
    return geo::Overlap3D::Op(rFrustum, static_cast<const geo::BasePoly3D&>(rPoly));
}

namespace geo
{

// FUNCTION: SWBF2 0x00724350
bool Overlap3D::Op(const BasePoly3D& rPoly, const Ray<Vector3<float> >& rRay)
{
    if (rPoly.numPoints_ < 3)
    {
        return false;
    }

    Plane plane(rPoly);
    Vector3<float> hit;
    if (!geo::Intersect3D::Op(plane, rRay, hit))
    {
        return false;
    }

    Vector3<float> skew = normalize(Vector3<float>(0.353234f, 1.2242f, 0.98245f));
    Vector3<float> axis1 = normalize(cross(plane.dir_, skew));
    Vector3<float> yaxis = normalize(cross(plane.dir_, axis1));
    Vector3<float> xaxis = normalize(cross(plane.dir_, yaxis));

    Vector2<float> points[256];
    for (int i = 0; i < 256; ++i)
    {
        points[i].x = 0.0f;
        points[i].y = 0.0f;
    }
    BasePoly2D poly2(points, rPoly.numPoints_);
    geo::Transform::Op(rPoly, poly2, xaxis, yaxis);

    Vector2<float> pt;
    pt.x = dot(xaxis, hit);
    pt.y = dot(yaxis, hit);
    return PolyPoly2D::PointInPolyCN(pt, poly2);
}

}
