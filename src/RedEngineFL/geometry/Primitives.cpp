#include "Primitives.h"
#include "basics.h"
#include "polygons.h"

#include <math.h>

namespace geo
{
namespace
{

// FUNCTION: SWBF2 0x00722d70
Vector3<float> Convert_(const Vector3<float>& rLocal, const Vector3<float>& rX,
                        const Vector3<float>& rY, const Vector3<float>& rZ)
{
    Vector3<float> v;
    v.x = rX.x * rLocal.x + rY.x * rLocal.y + rZ.x * rLocal.z;
    v.y = rX.y * rLocal.x + rY.y * rLocal.y + rZ.y * rLocal.z;
    v.z = rX.z * rLocal.x + rY.z * rLocal.y + rZ.z * rLocal.z;
    return v;
}

}

// FUNCTION: SWBF2 0x00722880
Sphere::Sphere(const BasePoly3D& rPoly)
{
    center_ = Vector3<float>(0, 0, 0);
    for (int i = 0; i < rPoly.numPoints_; ++i)
    {
        center_.x += rPoly.points_[i].x;
        center_.y += rPoly.points_[i].y;
        center_.z += rPoly.points_[i].z;
    }
    float fInv = 1.0f / (float)rPoly.numPoints_;
    center_.x *= fInv;
    center_.y *= fInv;
    center_.z *= fInv;

    for (int i = 0; i < rPoly.numPoints_; ++i)
    {
        float fX = center_.x - rPoly.points_[i].x;
        float fY = center_.y - rPoly.points_[i].y;
        float fZ = center_.z - rPoly.points_[i].z;
        float fDist = (float)sqrt((double)(fX * fX + fY * fY + fZ * fZ));
        radius_ = Max_(radius_, fDist);
    }
}

// FUNCTION: SWBF2 0x007229d0
Sphere::Sphere(const ExtrudedPoly3D& rPoly)
{
    const Vector2<float>* pPts = rPoly.points_._elements;
    const int nPts = rPoly.points_._numElements;

    center_ = Vector3<float>(0, 0, 0);
    for (int i = 0; i < nPts; ++i)
    {
        Vector3<float> top = Convert_(Vector3<float>(pPts[i].x, pPts[i].y, rPoly.max_),
                                      rPoly.xaxis_, rPoly.yaxis_, rPoly.zaxis_);
        Vector3<float> bot = Convert_(Vector3<float>(pPts[i].x, pPts[i].y, rPoly.min_),
                                      rPoly.xaxis_, rPoly.yaxis_, rPoly.zaxis_);
        center_.x += top.x + bot.x;
        center_.y += top.y + bot.y;
        center_.z += top.z + bot.z;
    }
    float fInv = 0.5f / (float)nPts;
    center_.x *= fInv;
    center_.y *= fInv;
    center_.z *= fInv;

    for (int i = 0; i < nPts; ++i)
    {
        Vector3<float> top = Convert_(Vector3<float>(pPts[i].x, pPts[i].y, rPoly.max_),
                                      rPoly.xaxis_, rPoly.yaxis_, rPoly.zaxis_);
        float fX = center_.x - top.x;
        float fY = center_.y - top.y;
        float fZ = center_.z - top.z;
        float fDist = (float)sqrt((double)(fX * fX + fY * fY + fZ * fZ));
        radius_ = Max_(radius_, fDist);

        Vector3<float> bot = Convert_(Vector3<float>(pPts[i].x, pPts[i].y, rPoly.min_),
                                      rPoly.xaxis_, rPoly.yaxis_, rPoly.zaxis_);
        fX = center_.x - bot.x;
        fY = center_.y - bot.y;
        fZ = center_.z - bot.z;
        fDist = (float)sqrt((double)(fX * fX + fY * fY + fZ * fZ));
        radius_ = Max_(radius_, fDist);
    }
}

// FUNCTION: SWBF2 0x007226c0
Box::Box()
    : min_(), max_()
{
    min_ = Vector3<float>(1e21f, 1e21f, 1e21f);
    max_ = Vector3<float>(-1e21f, -1e21f, -1e21f);
}

// FUNCTION: SWBF2 0x00722730
Plane::Plane(const BasePoly3D& rPoly)
{
    if (rPoly.numPoints_ >= 3)
    {
        const Vector3<float>* pPts = rPoly.points_;
        dir_ = normalize(cross(pPts[0] - pPts[1], pPts[2] - pPts[1]));
        dist_ = -dot(dir_, pPts[0]);
    }
}

// FUNCTION: SWBF2 0x00722cf0
void Box::Bound(const Vector3<float>& rPoint)
{
    min_.x = Min_(min_.x, rPoint.x);
    max_.x = Max_(max_.x, rPoint.x);
    min_.y = Min_(min_.y, rPoint.y);
    max_.y = Max_(max_.y, rPoint.y);
    min_.z = Min_(min_.z, rPoint.z);
    max_.z = Max_(max_.z, rPoint.z);
}

// FUNCTION: SWBF2 0x00722ca0
void BoundedPoly3D::Bound(const BasePoly3D& rPoly, Box& rBox)
{
    for (int i = 0; i < rPoly.numPoints_; ++i)
    {
        const Vector3<float>& rPoint = rPoly.points_[i];
        if (i == 0)
        {
            rBox.min_ = rBox.max_ = rPoint;
        }
        else
        {
            rBox.Bound(rPoint);
        }
    }
}

}
