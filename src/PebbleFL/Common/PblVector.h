#pragma once

#include <stddef.h>

class PblVector3
{
public:

    // FUNCTION: SWBF2 0x0040c9a0
    PblVector3() {}

    // FUNCTION: SWBF2 0x0040c9b0
    PblVector3(float fX, float fY, float fZ)
    {
        x = fX;
        y = fY;
        z = fZ;
    }

    // FUNCTION: SWBF2 0x0040c9e0
    PblVector3(const PblVector3& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }

    // FUNCTION: SWBF2 0x0043ec00
    void SetAll(float fValue)
    {
        x = y = z = fValue;
    }

    // FUNCTION: SWBF2 0x00480480
    bool operator!=(const PblVector3& v) const
    {
        return x != v.x || y != v.y || z != v.z;
    }

    // FUNCTION: SWBF2 0x0040ca00
    float GetX() const { return x; }
    // FUNCTION: SWBF2 0x0040ca10
    float GetY() const { return y; }
    // FUNCTION: SWBF2 0x0040ca20
    float GetZ() const { return z; }

    float x;
    float y;
    float z;
};

 
 
 
 

inline const PblVector3 Interpolate(const PblVector3& v0, const PblVector3& v1, float fT)
{
    PblVector3 v;
    v.x = (v1.x - v0.x) * fT + v0.x;
    v.y = (v1.y - v0.y) * fT + v0.y;
    v.z = (v1.z - v0.z) * fT + v0.z;
    return v;
}

extern PblVector3 gVectorUnitX;
extern PblVector3 gVectorUnitY;
extern PblVector3 gVectorUnitZ;

class PblVector4
{
public:

    // FUNCTION: SWBF2 0x00463a30
    PblVector4() {}
    PblVector4(float fX, float fY, float fZ, float fW) : x(fX), y(fY), z(fZ), w(fW) {}

    void Set(float fX, float fY, float fZ, float fW)
    {
        x = fX;
        y = fY;
        z = fZ;
        w = fW;
    }

    float x;
    float y;
    float z;
    float w;
};

class PblSphere : public PblVector4
{
public:
    const PblVector3& GetCenter() const { return *(const PblVector3*)this; }
    float GetRadius() const { return w; }

    void SetCenter(const PblVector3& rCenter) { x = rCenter.x; y = rCenter.y; z = rCenter.z; }
    void SetRadius(float fRadius) { w = fRadius; }
};

const PblVector3 __cdecl NormalizeEst(const PblVector3& rv);

class PblBox
{
public:

    void AddPoint(const PblVector3& rv)
    {
        if (_vMin.x > _vMax.x)
        {
            _vMax = rv;
            _vMin = rv;
            return;
        }
        if (_vMin.x > rv.x)
        {
            _vMin.x = rv.x;
        }
        else if (rv.x > _vMax.x)
        {
            _vMax.x = rv.x;
        }
        if (_vMin.y > rv.y)
        {
            _vMin.y = rv.y;
        }
        else if (rv.y > _vMax.y)
        {
            _vMax.y = rv.y;
        }
        if (_vMin.z > rv.z)
        {
            _vMin.z = rv.z;
        }
        else if (rv.z > _vMax.z)
        {
            _vMax.z = rv.z;
        }
    }

    const PblBox GetTransformedBox(const class PblMatrix& rMatrix) const;

    PblBox() {}

    explicit PblBox(const PblVector3& rv) : _vMin(rv), _vMax(rv) {}
    // FUNCTION: SWBF2 0x0040c930
    PblBox(float fMinX, float fMinY, float fMinZ, float fMaxX, float fMaxY, float fMaxZ)
        : _vMin(fMinX, fMinY, fMinZ), _vMax(fMaxX, fMaxY, fMaxZ) {}

    // FUNCTION: SWBF2 0x0040c980
    const PblVector3& GetMin() const { return _vMin; }
    // FUNCTION: SWBF2 0x0040c990
    const PblVector3& GetMax() const { return _vMax; }

    PblVector3 _vMin;
    PblVector3 _vMax;
};

 
 
 
 
 
 
 
 
 
 
 
 
