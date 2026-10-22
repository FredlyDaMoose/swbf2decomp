#pragma once

#include "vector.h"
#include "Primitives.h"
#include "RedEngineFL/Util/ext/basevector.h"

namespace geo
{

struct Poly2D
{
    util::Vector<Vector2<float> > points_;
};

struct Poly3D
{
    util::Vector<Vector3<float> > points_;
};

struct BasePoly2D
{
    BasePoly2D(const Poly2D& rPoly)
        : points_(rPoly.points_._elements), numPoints_(rPoly.points_._numElements) {}

    BasePoly2D(Vector2<float>* pPoints, int iNumPoints)
        : points_(pPoints), numPoints_(iNumPoints) {}

    void GetEdges(Ray<Vector2<float> >* pEdges) const;

    Vector2<float>* points_;
    int numPoints_;
};

struct BasePoly3D
{
    BasePoly3D(const Poly3D& rPoly)
        : points_(rPoly.points_._elements), numPoints_(rPoly.points_._numElements) {}

    BasePoly3D(Vector3<float>* pPoints, int iNumPoints)
        : points_(pPoints), numPoints_(iNumPoints) {}

    void GetEdges(Ray<Vector3<float> >* pEdges) const;

    Vector3<float>* points_;
    int numPoints_;
};

struct BoundedPoly3D : public BasePoly3D
{
    static void Bound(const BasePoly3D& rPoly, Box& rBox);

    Box box_;
};

struct SphereBoundedPoly3D : public BasePoly3D
{
    SphereBoundedPoly3D(const BasePoly3D& rPoly, const Sphere& rSphere)
        : BasePoly3D(rPoly), sphere_(rSphere) {}

    Sphere sphere_;
};

struct ExtrudedPoly3D : public Poly2D
{
    // FUNCTION: SWBF2 0x006de820
    ExtrudedPoly3D()
        : min_(0), max_(0), xaxis_(1, 0, 0), yaxis_(0, 0, 1), zaxis_(0, 1, 0)
    {
    }

    ExtrudedPoly3D& operator=(const ExtrudedPoly3D& rOther);

    float min_;
    float max_;
    Vector3<float> xaxis_;
    Vector3<float> yaxis_;
    Vector3<float> zaxis_;
};

}
