#include "Allocator.h"

namespace util
{

// FUNCTION: SWBF2 0x006e9030
void StackAllocator::Allocate(Buffer& rBuf)
{
    char* pPos = pos_;
    if (rBuf.size_ > (unsigned int)(pool_.data_ + pool_.size_ - pPos))
    {
        if (alloc_)
        {
            alloc_->Allocate(rBuf);
        }
        return;
    }

    rBuf.data_ = pPos;
    pos_ += rBuf.size_;
    sizes_[bpos_] = rBuf.size_;
    bpos_ = (bpos_ + 1) % 8;
}

// FUNCTION: SWBF2 0x006e9120
void StackAllocator::Deallocate(Buffer& rBuf)
{
    if (!dealloc_)
    {
        return;
    }

    if (alloc_ && ((char*)rBuf.data_ < pool_.data_
                   || (char*)rBuf.data_ >= pool_.data_ + pool_.size_))
    {
        alloc_->Deallocate(rBuf);
        return;
    }

    bpos_ = (bpos_ - 1) % 8;
    pos_ -= rBuf.size_;
    rBuf.data_ = 0;
}

// FUNCTION: SWBF2 0x006e8ff0
template void StaticMemoryBlock<4096>::Allocate(Buffer&);
// FUNCTION: SWBF2 0x006e90e0
template void StaticMemoryBlock<4096>::Deallocate(Buffer&);

// FUNCTION: SWBF2 0x006e9010
template void StaticMemoryBlock<1024>::Allocate(Buffer&);
// FUNCTION: SWBF2 0x006e9100
template void StaticMemoryBlock<1024>::Deallocate(Buffer&);

// SYNTHETIC: SWBF2 0x006e8fa0
// util::StackAllocator::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006e9090
void* Allocate_(unsigned int uiSize, IAllocator& rAlloc)
{
    Buffer buf(0, uiSize);
    rAlloc.Allocate(buf);
    return buf.data_;
}

HashMultiAllocator* HashMultiAllocator::sDefault;

// FUNCTION: SWBF2 0x006e90c0
void HashMultiAllocator::ClearDefault()
{
    delete sDefault;
    sDefault = 0;
}

// FUNCTION: SWBF2 0x006e91b0
IAllocator& DefaultStaticAllocator::GetInstance()
{
    static StackAllocator s_alloc(true, false, 32);
    static StaticMemoryBlock<4096> s_block;
    static bool s_bFirst = true;

    if (s_bFirst)
    {
        Buffer buf(0, 4096);
        s_block.Allocate(buf);
        s_alloc.Set(buf);
        s_bFirst = false;
    }
    return s_alloc;
}

// SYNTHETIC: SWBF2 0x00408b50
// `util::DefaultStaticAllocator::GetInstance'::`2'::`dynamic initializer for 's_alloc''
// SYNTHETIC: SWBF2 0x00769aa0
// `util::DefaultStaticAllocator::GetInstance'::`2'::`dynamic atexit destructor for 's_alloc''
// SYNTHETIC: SWBF2 0x00408b40
// `util::DefaultStackAllocator::GetInstance'::`2'::`dynamic initializer for 's_alloc''
// SYNTHETIC: SWBF2 0x00769a60
// `util::DefaultStackAllocator::GetInstance'::`2'::`dynamic atexit destructor for 's_alloc''

}

namespace util {

// FUNCTION: SWBF2 0x006e9180
void Deallocate_(void* p, unsigned int uiSize, util::IAllocator& rAlloc)
{
    if (p)
    {
        util::Buffer buf(p, uiSize);
        rAlloc.Deallocate(buf);
    }
}

}
