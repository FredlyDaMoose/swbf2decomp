#pragma once

#include "IAllocator.h"
#include "Allocator.h"
#include "basevector.h"
#include "types.h"

namespace util
{

// VTABLE: SWBF2 0x00783510
class SubAllocator : public IAllocator
{
public:

    SubAllocator(unsigned int uiChunkSize, unsigned int uiPhysLimit)
        : alloc_(0), chunkSize_(uiChunkSize), elemSize_(0), physLimit_(uiPhysLimit)
    {
    }

    // FUNCTION: SWBF2 0x006df170
    virtual ~SubAllocator()
    {
        Clear();
    }

    // FUNCTION: SWBF2 0x006df5d0
    virtual void Allocate(Buffer& rBuf)
    {
        int iTotal = 0;
        AllocateEx(rBuf, iTotal);
    }

    // FUNCTION: SWBF2 0x006dfea0
    virtual void Deallocate(Buffer& rBuf)
    {
        int iTotal = 0;
        DeallocateEx(rBuf, iTotal);
    }

    // FUNCTION: SWBF2 0x006df640
    virtual void AllocateEx(Buffer& rBuf, int& riTotal)
    {
        if (freelist_.GetSize() == 0)
        {
            unsigned int uiBytes = chunkSize_ * elemSize_;
            Buffer page(0, uiBytes);
            if ((unsigned int)riTotal + uiBytes > physLimit_)
            {
                return;
            }
            alloc_->Allocate(page);
            if (!page.data_)
            {
                return;
            }
            riTotal += uiBytes;
            pagelist_.Add(page.data_);
            for (unsigned int i = 0; i < chunkSize_; i++)
            {
                freelist_.Add((char*)page.data_ + elemSize_ * i);
            }
        }
        rBuf.data_ = freelist_[--freelist_._numElements];
    }

    // FUNCTION: SWBF2 0x006dfef0
    virtual void DeallocateEx(Buffer& rBuf, int& riTotal)
    {
        (void)riTotal;
        freelist_.Add(rBuf.data_);
    }

    // FUNCTION: SWBF2 0x006df8c0
    void Clear()
    {
        for (int i = 0; i < pagelist_.GetSize(); i++)
        {
            Buffer page(pagelist_[i], elemSize_ * chunkSize_);
            alloc_->Deallocate(page);
        }
        freelist_.Clear();
        pagelist_.Clear();
    }

    // FUNCTION: SWBF2 0x006e14f0
    void SetElemSize(unsigned int uiElemSize, unsigned int uiPageBytes)
    {
        elemSize_ = uiElemSize;
        if (!uiElemSize)
        {
            return;
        }
        chunkSize_ = uiPageBytes / uiElemSize;
        if (chunkSize_ > 5)
        {
            uiPageBytes >>= 1;
            chunkSize_ = uiPageBytes / uiElemSize;
            if (chunkSize_ > 5)
            {
                uiPageBytes >>= 1;
                chunkSize_ = uiPageBytes / uiElemSize;
            }
        }
        if (uiPageBytes % uiElemSize)
        {
            chunkSize_++;
        }
        unsigned int uiFloor = 16;
        unsigned int uiWant = chunkSize_ * 2;
        pagelist_.Reserve(Max_(uiWant, uiFloor));
        uiWant = chunkSize_ * 2;
        freelist_.Reserve(Max_(uiWant, uiFloor));
    }

    IAllocator* alloc_;
    Vector<void*> freelist_;
    Vector<void*> pagelist_;
    unsigned int chunkSize_;
    unsigned int elemSize_;
    unsigned int physLimit_;
};

template <class T>
class StaticSubAlloc : public SubAllocator
{
public:
    StaticSubAlloc()
        : SubAllocator(0x200, 0x40000000)
    {
        Create(&valloc_, 0x1000, sizeof(T));
    }

    virtual ~StaticSubAlloc() {}

    void Create(IAllocator* pAlloc, int iPageBytes, unsigned int uiElemSize)
    {
        alloc_ = pAlloc;
        SetElemSize(uiElemSize, (unsigned int)iPageBytes);
    }

    VNewAllocator valloc_;
};

}
