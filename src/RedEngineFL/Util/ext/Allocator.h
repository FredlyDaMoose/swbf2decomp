#pragma once

#include <stdio.h>

#include "RedEngineFL/Memory/RedMemory.h"

#include "IAllocator.h"

namespace util
{

class NewAllocator
{
public:

    static void Allocate(Buffer& rBuf)
    {
        rBuf.data_ = RedAllocFromHeap(__RedCurrHeap, rBuf.size_, 0);
        if (!rBuf.data_)
        {
            printf("util::NewAllocator::Allocate failed - new[] failed - out of memory!");
        }
    }

    static void Deallocate(Buffer& rBuf)
    {
        RedFreeToHeap(__RedCurrHeap, rBuf.data_, 0);
    }
};

template <class TYPE, class ALLOC = NewAllocator>
class TypedArrayAllocator : public ALLOC
{
public:

    // FUNCTION: SWBF2 0x0052b3f0

    static TYPE* Allocate_(int& riCount)
    {
        Buffer buf(0, riCount * sizeof(TYPE));
        ALLOC::Allocate(buf);
        riCount = (int)buf.size_ / (int)sizeof(TYPE);
        return (TYPE*)buf.data_;
    }

    static void Deallocate_(TYPE* p, int iCount)
    {
        Buffer buf(p, iCount * sizeof(TYPE));
        ALLOC::Deallocate(buf);
    }
};

template <class TYPE>
class TNewArrayAlloc : public TypedArrayAllocator<TYPE, NewAllocator>
{
};

// VTABLE: SWBF2 0x007834f0
class VNewAllocator : public IAllocator, public NewAllocator
{
public:
    // FUNCTION: SWBF2 0x006df5f0
    virtual void Allocate(Buffer& rBuf) { NewAllocator::Allocate(rBuf); }

    // FUNCTION: SWBF2 0x006dfec0
    virtual void Deallocate(Buffer& rBuf) { NewAllocator::Deallocate(rBuf); }
};
 

template <class TYPE>
class TBuffer
{
public:
    TBuffer() : data_(0), size_(0) {}
    TBuffer(Buffer& rBuf) : data_((TYPE*)rBuf.data_), size_(rBuf.size_) {}

    TYPE* data_;
    unsigned int size_;
};

// VTABLE: SWBF2 0x007838f4
class StackAllocator : public IAllocator
{
public:
    StackAllocator(bool bClear, bool bDealloc, unsigned int uiAlign)
        : alloc_(0), clear_(bClear), dealloc_(bDealloc), align_(uiAlign), pos_(0)
    {
    }

    virtual ~StackAllocator()
    {
        if (alloc_)
        {
            Buffer buf(pool_.data_, pool_.size_);
            alloc_->Deallocate(buf);
        }
    }

    virtual void Allocate(Buffer& rBuf);
    virtual void Deallocate(Buffer& rBuf);

    void Set(Buffer& rBuf)
    {
        pool_ = TBuffer<char>(rBuf);
        pos_ = pool_.data_;
    }

    IAllocator*   alloc_;
    bool          clear_;
    bool          dealloc_;
    unsigned int  align_;
    char*         pos_;
    TBuffer<char> pool_;
    unsigned int  sizes_[8];
    int           bpos_;
};
 

template <int SIZE>
class StaticMemoryBlock : public IAllocator
{
public:
    StaticMemoryBlock() : allocated_(false) {}

    virtual void Allocate(Buffer& rBuf)
    {
        rBuf.data_ = buffer_;
        allocated_ = true;
    }

    virtual void Deallocate(Buffer& rBuf)
    {
        rBuf.data_ = 0;
        allocated_ = false;
    }

    char buffer_[SIZE];
    bool allocated_;
};

class DefaultStaticAllocator
{
public:
    static IAllocator& GetInstance();
};

void* Allocate_(unsigned int uiSize, IAllocator& rAlloc);

template <class A>
class SingletonAllocator : public A
{
public:
    static void Allocate(Buffer& rBuf) { A::GetInstance().Allocate(rBuf); }
    static void Deallocate(Buffer& rBuf) { A::GetInstance().Deallocate(rBuf); }
};

class SubAllocator;
template <class ELEM, class KEY, class ALLOC> class HashTable;

template <class TABLE>
class MultiAllocator : public IAllocator
{
public:
    virtual ~MultiAllocator();
};

class HashMultiAllocator
    : public MultiAllocator<HashTable<SubAllocator*, int, TNewArrayAlloc<SubAllocator*> > >
{
public:
    static void ClearDefault();

private:

    static HashMultiAllocator* sDefault;
};

}
