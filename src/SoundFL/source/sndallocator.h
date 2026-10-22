#pragma once

#include "sndglobals.h"

namespace Snd
{

template <class T>
class Allocator
{
public:
    enum AllocType
    {
        allocType_heap = 0,
        allocType_tempHeap = 1,
        allocType_custom = 2
    };

    Allocator() {}
    ~Allocator() {}

    AllocType GetAllocType() const { return (AllocType)mAllocType; }

    static void* operator new(unsigned int uiSize, void* pMemory, bool bTemp)
    {
        const AllocType eType = pMemory ? allocType_custom : (bTemp ? allocType_tempHeap : allocType_heap);
        void* p = pMemory ? pMemory : (bTemp ? AllocTemp(uiSize) : Alloc(uiSize));
        if (p)
        {
            ((T*)p)->mAllocType = (unsigned char)eType;
        }
        return p;
    }

    static void operator delete(void* p, unsigned int  )
    {
        switch (((T*)p)->GetAllocType())
        {
        case allocType_heap:
            Free(p);
            break;
        case allocType_tempHeap:
            FreeTemp(p);
            break;
        default:
            break;
        }
    }

    static void operator delete(void* p, void* pMemory, bool bTemp)
    {
        (void)pMemory;
        (void)bTemp;
        if (!p)
        {
            return;
        }
        switch (((T*)p)->GetAllocType())
        {
        case allocType_heap:
            Free(p);
            break;
        case allocType_tempHeap:
            FreeTemp(p);
            break;
        default:
            break;
        }
    }

    unsigned char mAllocType;
};

}
