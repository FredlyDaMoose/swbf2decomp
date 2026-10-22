#pragma once

#include <stddef.h>

#include "vector.h"

class PblMatrix;

namespace geo
{

template <class T> class Ray;

struct BasePoly3D;
struct ExtrudedPoly3D;

struct Sphere
{
    Sphere() : radius_(0) {}

    Sphere(const Vector3<float>& rCenter, float fRadius)
        : center_(rCenter), radius_(fRadius) {}

    Sphere(const BasePoly3D& rPoly);
    Sphere(const ExtrudedPoly3D& rPoly);

    Vector3<float> center_;
    float radius_;
};

struct BasePoly2D;

class PolyPoly2D
{
public:
    static bool PointInPolyCN(const Vector2<float>& rPoint, const BasePoly2D& rPoly);

    static int LineSegPolyC(const Vector2<float>& rA0, const Vector2<float>& rA1, const BasePoly2D& rPoly);
};

class Overlap3D
{
public:
    static bool Op(const ExtrudedPoly3D& rPoly, const Vector3<float>& rPoint);
    static bool Op(const ExtrudedPoly3D& rPoly, const Sphere& rSphere);

    static bool Op(const BasePoly3D& rPoly, const Ray<Vector3<float> >& rRay);

    static bool Op(const class Frustum& rFrustum, const Sphere& rSphere);

    static bool Op(const Sphere& rSphere, const Ray<Vector3<float> >& rRay);
    static bool Op(const class Frustum& rFrustum, const BasePoly3D& rPoly);
    static bool Op(const class Frustum& rFrustum, const struct SphereBoundedPoly3D& rPoly);
};

struct Box;

class Intersect3D
{
public:
    static bool Op(const Box& rA, const Box& rB, Box& rOut);

    static bool Op(const struct Plane& rPlane, const Ray<Vector3<float> >& rRay, Vector3<float>& rHit);
};

struct Circle;
class Overlap2D
{
public:
    static bool LineSegLineSeg(const Vector2<float>& rA0, const Vector2<float>& rA1,
                               const Vector2<float>& rB0, const Vector2<float>& rB1);
    static bool Op(const Circle& rCircle, const Ray<Vector2<float> >& rRay);
    static bool Op(const Ray<Vector2<float> >& rA, const Ray<Vector2<float> >& rB);
};

class Classify3D
{
public:
    static unsigned int Op(const class Frustum& rFrustum, const Sphere& rSphere);
};

class SepDistance
{
public:
    static float Op(const Sphere& rA, const Sphere& rB);
};

class ProjClip
{
public:
    static void Op(const BasePoly3D& rIn, BasePoly3D& rOut);
};

class Transform
{
public:
    static void Op(const BasePoly3D& rIn, BasePoly3D& rOut, const PblMatrix& rTrans);

    static void Op(const BasePoly3D& rIn, BasePoly2D& rOut, const Vector3<float>& rX, const Vector3<float>& rY);
};

struct Circle
{
    Circle(const Vector2<float>& rCenter, float fRadius) : center_(rCenter), radius_(fRadius) {}

    Vector2<float> center_;
    float radius_;
};

struct Box
{

    Box();

    Box(const Vector3<float>& rMin, const Vector3<float>& rMax) : min_(rMin), max_(rMax) {}

    void Bound(const Vector3<float>& rPoint);

    float Volume();

    Vector3<float> min_;
    Vector3<float> max_;
};

struct Plane
{

    Plane() : dist_(0.0f) {}

    Plane(const BasePoly3D& rPoly);

    Vector3<float> dir_;
    float dist_;
};
 
 
 
 

class Frustum
{
public:
    Frustum();
    Frustum(const Vector3<float>& rCenter, const Vector3<float>& rUp, const Vector3<float>& rFwd,
            float fWidth, float fHeight, float fNear, float fFar);

    const Plane* GetPlanes() const
    {
        if (!initdone_)
        {
            GetPlanes_(planes_);
            initdone_ = true;
        }
        return planes_;
    }

    void GetPlanes_(Plane* pPlanes) const;
    void GetLeftPlane(Plane& rPlane) const;
    void GetRightPlane(Plane& rPlane) const;
    void GetBottomPlane(Plane& rPlane) const;
    void GetTopPlane(Plane& rPlane) const;

    void GetBaseEdges(Ray<Vector3<float> >* pEdges) const;

    Vector3<float> center_;
    Vector3<float> up_;
    Vector3<float> fwd_;
    float width_;
    float height_;
    float near_;
    float far_;
    mutable Plane planes_[6];
    mutable bool initdone_;
};

template <class T>
class Line
{
public:

    Line() : orig_(), dir_() {}
    Line(const T& rOrig, const T& rDir) : orig_(rOrig), dir_(rDir) {}

    T orig_;
    T dir_;
};

template <class T>
class Ray : public Line<T>
{
public:
    Ray() {}
    Ray(const T& rOrig, const T& rDir) : Line<T>(rOrig, rDir) {}
};

}
