#pragma once

#include <stddef.h>

#include "basevector.h"

namespace util
{

template <class CHAR, class VECTOR>
class ContString_ : public VECTOR
{
public:
    ContString_() {}

    ContString_(ContString_ const& rOther) : VECTOR(rOther) {}

    ContString_ const& operator=(ContString_ const& rOther)
    {
        if (this->Reserve_(rOther._numElements, true))
        {
            this->_numElements = rOther._numElements;
            for (int i = 0; i < this->_numElements; i++)
            {
                this->_elements[i] = rOther._elements[i];
            }
        }
        return *this;
    }

    void Set(CHAR const* pText)
    {
        if (!pText)
        {
            return;
        }
        int iLen = 0;
        while (pText[iLen])
        {
            ++iLen;
        }
        int iCount = iLen + 1;
        this->SetSize(iCount);
        for (int i = 0; i < iCount; ++i)
        {
            this->_elements[i] = pText[i];
        }
    }
};

class BaseString : public ContString_<char, Vector<char, BaseArray<char, TNewArrayAlloc<char> > > >
{
public:
    BaseString() {}

    // FUNCTION: SWBF2 0x006224f0
    BaseString(BaseString const& rOther)
        : ContString_<char, Vector<char, BaseArray<char, TNewArrayAlloc<char> > > >(rOther)
    {
    }

    operator char*() { return this->_elements; }
};

}

 
 
 
 
