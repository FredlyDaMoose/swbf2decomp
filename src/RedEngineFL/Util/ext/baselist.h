#pragma once

#include "IAllocator.h"

namespace util
{

template <class T>
class SLNode
{
public:
    SLNode() : next_(0) {}

    SLNode(const T& rElem) : elem_(rElem), next_(0) {}

    static void* operator new(unsigned int uiSize, IAllocator& rAlloc)
    {
        Buffer buf(0, uiSize);
        rAlloc.Allocate(buf);
        return buf.data_;
    }

    static void operator delete(void* p, IAllocator& rAlloc)
    {
        Buffer buf(p, sizeof(SLNode));
        rAlloc.Deallocate(buf);
    }

    T elem_;
    SLNode* next_;
};

template <class T>
class DLNode
{
public:
    DLNode() : next_(0), prev_(0) {}

    static void* operator new(unsigned int uiSize, IAllocator& rAlloc)
    {
        Buffer buf(0, uiSize);
        rAlloc.Allocate(buf);
        return buf.data_;
    }

    static void operator delete(void* p, IAllocator& rAlloc)
    {
        Buffer buf(p, sizeof(DLNode));
        rAlloc.Deallocate(buf);
    }

    T elem_;
    DLNode* next_;
    DLNode* prev_;
};

template <class T, class NODE>
class List
{
public:
    List(IAllocator* pAlloc)
        : alloc_(pAlloc), first_(0), last_(0), size_(0)
    {
        if (alloc_)
        {
            NODE* pNode = new (*alloc_) NODE;
            NODE::operator delete(pNode, *alloc_);
        }
    }

    bool InsertLast(const T& rElem)
    {
        if (!alloc_)
        {
            return false;
        }
        NODE* pNode = new (*alloc_) NODE(rElem);
        return InsertLast_(pNode);
    }

    bool InsertLast_(NODE* pNode)
    {
        if (last_)
        {
            last_->next_ = pNode;
        }
        if (!first_)
        {
            first_ = pNode;
        }
        ++size_;
        last_ = pNode;
        return true;
    }

    bool FindRemoveSlow(const T& rElem)
    {
        bool bFound = false;
        NODE* pPrev = 0;
        NODE* pNode = first_;
        while (pNode)
        {
            NODE* pNext = pNode->next_;
            if (pNode->elem_ == rElem)
            {
                if (pPrev)
                {
                    pPrev->next_ = pNext;
                    if (!pNext)
                    {
                        last_ = pPrev;
                    }
                    else if (pNode == last_)
                    {
                        last_ = pNext;
                    }
                }
                else
                {
                    first_ = pNext;
                    if (pNode == last_)
                    {
                        last_ = 0;
                    }
                }
                pNode->next_ = 0;
                --size_;
                if (alloc_)
                {
                    NODE::operator delete(pNode, *alloc_);
                }
                bFound = true;
            }
            else
            {
                pPrev = pNode;
            }
            pNode = pNext;
        }
        return bFound;
    }

    bool Remove_(NODE* pNode)
    {
        NODE* pNext = pNode->next_;
        NODE* pPrev = pNode->prev_;
        if (pNext)
        {
            pNext->prev_ = pPrev;
        }
        if (pPrev)
        {
            pPrev->next_ = pNext;
            if (!pNext)
            {
                last_ = pPrev;
            }
            else if (pNode == last_)
            {
                last_ = pNext;
            }
        }
        else
        {
            first_ = pNext;
            if (pNode == last_)
            {
                last_ = 0;
            }
        }
        pNode->next_ = 0;
        pNode->prev_ = 0;
        --size_;
        return true;
    }

    IAllocator* alloc_;
    NODE* first_;
    NODE* last_;
    int size_;
};

}
