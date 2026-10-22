#pragma once

namespace util
{

// FUNCTION: SWBF2 0x0045cb20
template <class TYPE>
const TYPE& Max_(const TYPE& a, const TYPE& b)
{
    return a >= b ? a : b;
}

// TEMPLATE: SWBF2 0x0053c010
// util::Min_<int>

template <class TYPE>
const TYPE& Min_(const TYPE& a, const TYPE& b)
{
    return b >= a ? a : b;
}

class Int_
{
public:
    static int ilog(int iValue);
};

// FUNCTION: SWBF2 0x004589d0
inline int Int_::ilog(int iValue)
{
    int iBits = 0;
    if (iValue != 0)
    {
        for (int iRest = iValue; iRest > 0; iRest >>= 1)
        {
            iBits++;
        }

        if ((1 << (iBits - 1)) > iValue)
        {
            iBits++;
        }
    }
    return iBits;
}

}
