#pragma once

#include "Allocator.h"
#include "basevector.h"
#include "baselist.h"

namespace util
{

class DummyNonAllocator : public IAllocator
{
public:
    virtual void Allocate(Buffer& rBuf);
    virtual void Deallocate(Buffer& rBuf);
};

class ListAllocator : public IAllocator
{
public:
    virtual void Allocate(Buffer& rBuf) = 0;
    virtual void Deallocate(Buffer& rBuf) = 0;
    virtual void SetMaxMem(int iMax) = 0;
    virtual ~ListAllocator();
    virtual void UnitTest_() = 0;

    static ListAllocator* Create(unsigned int uBaseMem, unsigned int uMaxMem);

protected:
    ListAllocator() {}
};

class ListAllocatorImpl : public ListAllocator
{
public:
    ListAllocatorImpl(unsigned int uBaseMem, unsigned int uMaxMem);
    virtual ~ListAllocatorImpl();

    virtual void Allocate(Buffer& rBuf);
    virtual void Deallocate(Buffer& rBuf);
    virtual void SetMaxMem(int iMax);
    virtual void UnitTest_();

    DLNode<Buffer>* Convert_(Buffer& rBuf)
    {
        DLNode<Buffer>* pNode = (DLNode<Buffer>*)rBuf.data_ - 1;
        rBuf = pNode->elem_;
        return pNode;
    }

    DLNode<Buffer>* ExternalAlloc_(unsigned int uSize);
    DLNode<Buffer>* FindFreeBlock(const Buffer& rBuf);
    bool MergeBlocks_(DLNode<Buffer>* pFirst, DLNode<Buffer>* pSecond);

    VNewAllocator valloc_;
    DummyNonAllocator dalloc_;
    IAllocator* baseAlloc_;
    int pageSize_;
    int roundsize_;
    unsigned int basemem_;
    unsigned int usedmem_;
    unsigned int maxmem_;
    int freemem_;
    List<Buffer, DLNode<Buffer> > freelist_;
    Vector<Buffer> pages_;
};

 
 

}
