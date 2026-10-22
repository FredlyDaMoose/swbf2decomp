#pragma once

#include "basearray.h"
#include "types.h"

namespace util
{

struct AssertBounds_
{
};

template <class TYPE, class ARRAY, class BOUNDS>
class IterArray : public ARRAY
{
public:

    void Clear()
    {
        this->Reserve_(0, true);
        this->_numElements = 0;
    }

    TYPE& operator[](int i) { return this->_elements[i]; }
    const TYPE& operator[](int i) const { return this->_elements[i]; }

    operator const TYPE*() const { return this->_elements; }
    TYPE* Begin() { return this->_elements; }

    TYPE* End() { return this->_elements + this->_numElements; }
};

template <class TYPE, class ARRAY = BaseArray<TYPE> >
class Vector : public IterArray<TYPE, ARRAY, AssertBounds_>
{
public:
    Vector() {}

    Vector(Vector const& rOther)
    {
        this->_elements = 0;
        this->_numElements = 0;
        this->_size = 0;

        if (this->Reserve_(rOther._numElements, true))
        {
            this->_numElements = rOther._numElements;
            for (int i = 0; i < this->_numElements; i++)
            {
                this->_elements[i] = rOther._elements[i];
            }
        }
    }

    TYPE* FindFirst(TYPE const& rElem)
    {
        for (int i = 0; i < this->_numElements; i++)
        {
            if (this->_elements[i] == rElem)
            {
                return &this->_elements[i];
            }
        }
        return this->End();
    }

    bool FindRemoveLazy(TYPE const& rElem)
    {
        TYPE* pSlot = FindFirst(rElem);
        if (pSlot == this->End())
        {
            return false;
        }
        *pSlot = 0;
        return true;
    }

    bool FindRemove(TYPE const& rElem)
    {
        TYPE* pSlot = FindFirst(rElem);
        if (pSlot == this->End())
        {
            return false;
        }
        *pSlot = this->End()[-1];
        this->_numElements--;
        return true;
    }

    bool Add(TYPE const& rElem)
    {
        if (this->_numElements >= this->_size)
        {
            int iGrown = this->_size * 2;
            int iFloor = 8;
            if (!this->Reserve_(Max_(iGrown, iFloor), true))
            {
                return false;
            }
        }

        new (this->_elements + this->_numElements) TYPE(rElem);
        this->_numElements++;
        return true;
    }

    bool SetNumElements(int iNum)
    {
        if (iNum > this->_size && !this->Reserve_(Max_(this->_size * 2, iNum), true))
        {
            return false;
        }
        this->_numElements = iNum;
        return true;
    }

    bool SetSize(int iSize) { return SetSize_(iSize, true); }

    bool SetSizeGrow(int iSize) { return SetSize_(iSize, false); }

    void SetAll(TYPE value)
    {
        for (int i = 0; i < this->_numElements; i++)
        {
            this->_elements[i] = value;
        }
    }

    // FUNCTION: SWBF2 0x0052b0d0
    bool SetSize_(int iSize, bool bForce)
    {
        if (iSize == 0 || this->_size == 0)
        {
            if (!this->Reserve_(iSize, bForce))
            {
                return false;
            }
        }
        else if (bForce && this->_size / 2 - 1 > iSize)
        {
            if (!this->Reserve_(1 << Int_::ilog(iSize), bForce))
            {
                return false;
            }
        }
        else if (this->_size < iSize)
        {
            int iMinGranule = 4;

            const int iLog = Int_::ilog(iSize);
            int iGranule = iLog >= 2 ? 1 << (iLog - 2) : 0;
            const int& riGranule = Max_(iGranule, iMinGranule);

            int iRounded = iSize;
            if (iSize % riGranule != 0)
            {
                iRounded = (iSize / riGranule + 1) * riGranule;
            }

            if (!this->Reserve_(iRounded, bForce))
            {
                return false;
            }
        }

        this->_numElements = iSize;
        return true;
    }

};

}
