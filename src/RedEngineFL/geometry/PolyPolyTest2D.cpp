#include "Primitives.h"
#include "basics.h"
#include "polygons.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

// FUNCTION: SWBF2 0x00724ef0
int __cdecl geo::PolyPoly2D::LineSegPolyC(const geo::Vector2<float>& rA0, const geo::Vector2<float>& rA1, const geo::BasePoly2D& rPoly)
{
    int iCount = 0;
    for (int i = 0; i < rPoly.numPoints_; ++i)
    {
        if (geo::Overlap2D::LineSegLineSeg(rA0, rA1, rPoly.points_[i], rPoly.points_[(i + 1) % rPoly.numPoints_]))
        {
            ++iCount;
        }
    }
    return iCount;
}

namespace geo
{

// FUNCTION: SWBF2 0x00725140
bool PolyPoly2D::PointInPolyCN(const Vector2<float>& rPoint, const BasePoly2D& rPoly)
{
    Vector2<float> farPoint;
    farPoint.x = rPoint.x + 1000000000.0f;
    farPoint.y = rPoint.y + 2000000000.0f;
    return (geo::PolyPoly2D::LineSegPolyC(rPoint, farPoint, rPoly) % 2) != 0;
}

template <class T>
const Vector3<T> operator*(const PblMatrix& rMat, const Vector3<T>& rVec)
{
    PblVector3 vIn(rVec.x, rVec.y, rVec.z);
    PblVector3 vOut;
    D3DXVec3TransformCoord(&vOut, &vIn, &rMat);
    return Vector3<T>(vOut.x, vOut.y, vOut.z);
}

// FUNCTION: SWBF2 0x00724f40
void ProjClip::Op(const BasePoly3D& rIn, BasePoly3D& rOut)
{
    for (int i = 0; i < rIn.numPoints_; ++i)
    {
        Vector3<float> v = rIn.points_[i];
        float fNegZ = -v.z;
        float fW = Max_(fNegZ, 1e-7f);
        float fInv = 1.0f / fW;
        rOut.points_[i] = Vector3<float>(v.x * fInv, v.y * fInv, fInv * fW);
    }
}

// FUNCTION: SWBF2 0x00724ff0
void Transform::Op(const BasePoly3D& rIn, BasePoly3D& rOut, const PblMatrix& rTrans)
{
    for (int i = 0; i < rIn.numPoints_; ++i)
    {
        rOut.points_[i] = rTrans * rIn.points_[i];
    }
}

}

// FUNCTION: SWBF2 0x007250a0
void __cdecl geo::Transform::Op(const geo::BasePoly3D& rIn, geo::BasePoly2D& rOut, const geo::Vector3<float>& rX,
                        const geo::Vector3<float>& rY)
{
    for (int i = 0; i < rIn.numPoints_; ++i)
    {
        rOut.points_[i].x = geo::dot(rIn.points_[i], rX);
        rOut.points_[i].y = geo::dot(rIn.points_[i], rY);
    }
}
