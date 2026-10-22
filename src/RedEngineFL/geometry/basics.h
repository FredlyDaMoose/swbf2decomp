#pragma once

namespace geo
{

template <class T>
const T& Max_(const T& a, const T& b)
{
    return a >= b ? a : b;
}

template <class T>
const T& Min_(const T& a, const T& b)
{
    return b >= a ? a : b;
}

}
