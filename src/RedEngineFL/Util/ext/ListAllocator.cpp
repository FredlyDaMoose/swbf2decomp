#include "ListAllocator.h"

namespace util
{

// FUNCTION: SWBF2 0x006e9210
ListAllocatorImpl::ListAllocatorImpl(unsigned int uBaseMem, unsigned int uMaxMem)
    : baseAlloc_(&valloc_),
      pageSize_(0x2000),
      roundsize_(32),
      basemem_(uBaseMem),
      usedmem_(0),
      maxmem_(uMaxMem),
      freemem_(0),
      freelist_(0)
{
    pages_.Reserve(16);
    ExternalAlloc_(basemem_);
}

// FUNCTION: SWBF2 0x006e9310
ListAllocator::~ListAllocator()
{
}

// FUNCTION: SWBF2 0x006e9320
ListAllocatorImpl::~ListAllocatorImpl()
{
    if (baseAlloc_)
    {
        for (int i = 0; i < pages_.GetSize(); i++)
        {
            baseAlloc_->Deallocate(pages_[i]);
        }
    }
}

// SYNTHETIC: SWBF2 0x006e93c0
// util::ListAllocator::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x006e93f0
// util::ListAllocatorImpl::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006e9420
template bool Vector<Buffer>::Add(const Buffer&);

// FUNCTION: SWBF2 0x006e9490
void DummyNonAllocator::Allocate(Buffer&)
{
}

// FUNCTION: SWBF2 0x006e94a0
void ListAllocatorImpl::Allocate(Buffer& rBuf)
{
    if (rBuf.size_ % roundsize_)
    {
        rBuf.size_ = (rBuf.size_ / roundsize_ + 1) * roundsize_;
    }

    DLNode<Buffer>* pNode = FindFreeBlock(rBuf);
    if (!pNode)
    {
        if (baseAlloc_)
        {
            int iSize = rBuf.size_;
            if (ExternalAlloc_(Max_(pageSize_, iSize)))
            {
                Allocate(rBuf);
            }
            else
            {
                rBuf.data_ = 0;
            }
        }
        return;
    }

    int iRemain = pNode->elem_.size_ - rBuf.size_;
    if (iRemain > 32)
    {
        freemem_ -= pNode->elem_.size_;
        rBuf.data_ = pNode->elem_.data_;
        pNode->elem_.size_ = rBuf.size_;

        DLNode<Buffer>* pRest = (DLNode<Buffer>*)((char*)rBuf.data_ + rBuf.size_);
        pRest->elem_.data_ = pRest + 1;
        pRest->elem_.size_ = iRemain - sizeof(DLNode<Buffer>);
        pRest->next_ = 0;
        pRest->prev_ = 0;

        pRest->prev_ = pNode;
        pRest->next_ = pNode->next_;
        pNode->next_ = pRest;
        if (pRest->next_)
        {
            pRest->next_->prev_ = pRest;
        }
        else
        {
            freelist_.last_ = pRest;
        }
        freelist_.size_++;

        freemem_ += pRest->elem_.size_;
        freelist_.Remove_(pNode);
    }
    else
    {
        rBuf = pNode->elem_;
        freelist_.Remove_(pNode);
        freemem_ -= rBuf.size_;
    }
}

// FUNCTION: SWBF2 0x006e95a0
template Buffer* TypedArrayAllocator<Buffer>::Allocate_(int&);

// FUNCTION: SWBF2 0x006e95f0
ListAllocator* ListAllocator::Create(unsigned int uBaseMem, unsigned int uMaxMem)
{
    return new ListAllocatorImpl(uBaseMem, uMaxMem);
}

// FUNCTION: SWBF2 0x006e9660
void DummyNonAllocator::Deallocate(Buffer&)
{
}

// FUNCTION: SWBF2 0x006e9670
void ListAllocatorImpl::Deallocate(Buffer& rBuf)
{
    if (!rBuf.data_)
    {
        return;
    }

    DLNode<Buffer>* pNode = Convert_(rBuf);
    pNode->next_ = 0;
    pNode->prev_ = 0;

    DLNode<Buffer>* pLast = 0;
    DLNode<Buffer>* pAfter = 0;
    for (DLNode<Buffer>* p = freelist_.first_; p; p = p->next_)
    {
        pLast = p;
        if (p->elem_.data_ > rBuf.data_)
        {
            pAfter = p;
            break;
        }
    }

    if (freelist_.size_ > 0 && !pAfter)
    {

        pNode->prev_ = pLast;
        pNode->next_ = 0;
        if (pLast)
        {
            pLast->next_ = pNode;
        }
        else
        {
            freelist_.first_ = pNode;
        }
        freelist_.last_ = pNode;
    }
    else
    {

        pNode->prev_ = pAfter ? pAfter->prev_ : 0;
        pNode->next_ = pAfter;
        if (pNode->prev_)
        {
            pNode->prev_->next_ = pNode;
        }
        else
        {
            freelist_.first_ = pNode;
        }
        if (pAfter)
        {
            pAfter->prev_ = pNode;
        }
        else
        {
            freelist_.last_ = pNode;
        }
    }
    freelist_.size_++;
    freemem_ += rBuf.size_;

    DLNode<Buffer>* pPrev = pNode->prev_;
    DLNode<Buffer>* pNext = pNode->next_;
    if (pPrev && (!pNext || pPrev->elem_.size_ < pNext->elem_.size_))
    {
        MergeBlocks_(pPrev, pNode);
    }
    else if (pNext)
    {
        MergeBlocks_(pNode, pNext);
    }
}

// FUNCTION: SWBF2 0x006e9760
DLNode<Buffer>* ListAllocatorImpl::ExternalAlloc_(unsigned int uSize)
{
    if (usedmem_ + uSize > maxmem_)
    {
        return 0;
    }

    Buffer buf(0, uSize);
    baseAlloc_->Allocate(buf);
    if (!buf.data_)
    {
        return 0;
    }

    pages_.Add(buf);

    DLNode<Buffer>* pNode = (DLNode<Buffer>*)buf.data_;
    buf.size_ -= sizeof(DLNode<Buffer>);
    pNode->elem_.data_ = pNode + 1;
    pNode->elem_.size_ = buf.size_;
    pNode->next_ = 0;
    pNode->prev_ = 0;
    buf.data_ = pNode->elem_.data_;
    Deallocate(buf);

    usedmem_ += uSize;
    return pNode;
}

// FUNCTION: SWBF2 0x006e97f0
DLNode<Buffer>* ListAllocatorImpl::FindFreeBlock(const Buffer& rBuf)
{
    for (DLNode<Buffer>* p = freelist_.first_; p; p = p->next_)
    {
        if (p->next_)
        {
            MergeBlocks_(p, p->next_);
        }
        if (p->elem_.size_ >= rBuf.size_ + sizeof(DLNode<Buffer>))
        {
            return p;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006e98a0
bool ListAllocatorImpl::MergeBlocks_(DLNode<Buffer>* pFirst, DLNode<Buffer>* pSecond)
{
    if ((char*)pSecond->elem_.data_ - (char*)pFirst->elem_.data_ - pFirst->elem_.size_
        - sizeof(DLNode<Buffer>) != 0)
    {
        return false;
    }

    pFirst->elem_.size_ += pSecond->elem_.size_ + sizeof(DLNode<Buffer>);
    freemem_ += sizeof(DLNode<Buffer>);
    freelist_.Remove_(pSecond);
    return true;
}

// FUNCTION: SWBF2 0x006e9920
template bool List<Buffer, DLNode<Buffer> >::Remove_(DLNode<Buffer>*);

// FUNCTION: SWBF2 0x006e9980
template bool BaseArray<Buffer>::Reserve_(int, bool);

// FUNCTION: SWBF2 0x006e9a60
void ListAllocatorImpl::SetMaxMem(int iMax)
{
    maxmem_ = iMax;
}

// FUNCTION: SWBF2 0x006e9a70
void ListAllocatorImpl::UnitTest_()
{
}

}
