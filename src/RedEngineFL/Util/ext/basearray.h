#pragma once

#include <new>

#include "Allocator.h"

namespace util
{

template <class TYPE, class ALLOC = TNewArrayAlloc<TYPE> >
class BaseArray : public ALLOC
{
public:

    BaseArray()
        : _elements(0), _numElements(0), _size(0)
    {
    }

    ~BaseArray()
    {
        if (_elements)
        {
            for (int i = 0; i < _numElements; i++)
            {
                _elements[i].~TYPE();
            }

            ALLOC::Deallocate_(_elements, _size);
            _elements = 0;
        }

        _size = 0;
        _numElements = 0;
    }

    TYPE* GetData() { return _elements; }

    int GetSize() const { return _numElements; }

    bool Reserve(int iCount) { return Reserve_(iCount, true); }

    void Swap(BaseArray& rOther)
    {
        TYPE* pElements = _elements;
        int iNumElements = _numElements;
        int iSize = _size;

        _elements = rOther._elements;
        _numElements = rOther._numElements;
        _size = rOther._size;

        rOther._elements = pElements;
        rOther._numElements = iNumElements;
        rOther._size = iSize;
    }

    // FUNCTION: SWBF2 0x006d8190
    bool Reserve_(int iCount, bool bForce)
    {
        if (iCount == _size)
        {
            return true;
        }
        if (!bForce && iCount <= _size)
        {
            return true;
        }

        TYPE* pNew = 0;
        if (iCount > 0)
        {
            pNew = ALLOC::Allocate_(iCount);
            if (!pNew)
            {
                return false;
            }
        }

        const int iKeep = (_numElements < iCount) ? _numElements : iCount;

        int i = 0;
        for (; i < iKeep; i++)
        {
            if (pNew)
            {
                new (pNew + i) TYPE(_elements[i]);
            }
        }
        for (; i < iCount; i++)
        {
            if (pNew)
            {
                new (pNew + i) TYPE();
            }
        }

        if (_elements)
        {
            for (int j = 0; j < _numElements; j++)
            {
                _elements[j].~TYPE();
            }

            ALLOC::Deallocate_(_elements, _size);
            _elements = 0;
        }

        _numElements = iKeep;
        _size = iCount;
        _elements = pNew;

        return pNew != 0;
    }

    TYPE* _elements;
    int _numElements;
    int _size;
};

}
