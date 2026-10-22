#pragma once

#include <math.h>

namespace geo
{

template <class T>
class Vector2
{
public:
    T x;
    T y;
};

template <class T>
class Vector3
{
public:
    Vector3() : x(0), y(0), z(0) {}

    template <class U>
    Vector3(U ux, U uy, U uz) : x((T)ux), y((T)uy), z((T)uz) {}

    T x;
    T y;
    T z;
};

template <class T>
const Vector3<T> operator+(const Vector3<T>& a, const Vector3<T>& b)
{
    return Vector3<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

template <class T>
const Vector3<T> operator-(const Vector3<T>& a, const Vector3<T>& b)
{
    return Vector3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template <class T>
const Vector3<T> operator*(const Vector3<T>& v, T f)
{
    return Vector3<T>(v.x * f, v.y * f, v.z * f);
}

template <class T>
T dot(const Vector3<T>& a, const Vector3<T>& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <class T>
Vector3<T> cross(const Vector3<T>& a, const Vector3<T>& b)
{
    return Vector3<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

template <class T>
T length(const Vector3<T>& v)
{
    return (T)sqrt((double)dot(v, v));
}

template <class T>
Vector3<T> normalize(const Vector3<T>& v)
{
    return v * ((T)1 / length(v));
}

}
