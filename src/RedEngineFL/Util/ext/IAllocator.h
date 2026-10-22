#pragma once

namespace util
{

struct Buffer
{
    Buffer() : data_(0), size_(0) {}
    Buffer(void* pData, unsigned int uiSize) : data_(pData), size_(uiSize) {}

    void* data_;
    unsigned int size_;
};

class IAllocator
{
public:
    virtual void Allocate(Buffer& rBuf) = 0;
    virtual void Deallocate(Buffer& rBuf) = 0;
    virtual void AllocateEx(Buffer& rBuf, int& riTotal);
    virtual void DeallocateEx(Buffer& rBuf, int& riTotal);
    virtual void ClearMemHash();
    virtual unsigned int GetMemHash();
    virtual void SetMaxMem(int iMax);
};

}
