#pragma once

namespace util
{

template <class TYPE>
class GreaterEqOp
{
public:
    bool operator()(TYPE const& a, TYPE const& b) const { return a >= b; }
};

template <class TYPE, class HEAP, class OP>
void HeapInsert_(HEAP& rHeap, int iIndex, TYPE const& rElem)
{
    while (iIndex)
    {
        int iParent = iIndex >> 1;
        if (OP()(rHeap[iParent], rElem))
        {
            break;
        }
        rHeap[iIndex] = rHeap[iParent];
        iIndex = iParent;
    }
    rHeap[iIndex] = rElem;
}

template <class TYPE, class HEAP, class OP>
void HeapPop_(HEAP& rHeap, int iIndex)
{
    if (!rHeap._numElements)
    {
        return;
    }

    int iLast = --rHeap._numElements;
    TYPE elem = rHeap[iLast];
    if (iIndex >= iLast)
    {
        return;
    }

    rHeap[iIndex] = elem;
    int iEnd = rHeap._numElements - 1;
    int iHalf = iEnd >> 1;
    OP op;
    while (iIndex <= iHalf)
    {
        int iChild = iIndex * 2;
        if (iChild < iEnd && !op(rHeap[iChild], rHeap[iChild + 1]))
        {
            ++iChild;
        }
        if (op(elem, rHeap[iChild]))
        {
            break;
        }
        rHeap[iIndex] = rHeap[iChild];
        iIndex = iChild;
    }
    rHeap[iIndex] = elem;
}

}
