#include <stdlib.h>
#include <string.h>

#include "RedMemory.h"
#include "RedEngineFL/Memory/engineAlloc.h"

extern "C"
{
    bool isHeapObjInitialized;
    HeapObj* heapobj;
    int maxheaps;
    int __RedCurrHeap;
    int __RedDebugHeap = -1;
    int __RedTempHeap = -1;
    void (*OnFailure)();
    int sRecursionDepth;
}

// FUNCTION: SWBF2 0x006c3a50
extern "C" unsigned int RedGetHeapFree(int heap)
{
    unsigned int total = 0;
    for (Tag* tag = heapobj[heap].free.first; tag; tag = tag->next_)
    {
        total += tag->size;
    }
    return total;
}

namespace RedMemory
{
    int failHeap;
    unsigned int failSize;
}

extern "C" void Startup_RedInitHeap();

void TagAddList(TagList* list, Tag* tag);
void TagMergeIntoList(TagList* list, Tag* tag);
void TagRemoveList(TagList* list, Tag* tag);

// FUNCTION: SWBF2 0x006c3540
void* __cdecl operator new(unsigned int size)
{
    return RedAllocFromHeap(__RedCurrHeap, size, 0);
}

// FUNCTION: SWBF2 0x006c3560
void __cdecl operator delete(void* block)
{
    if (block)
    {
        RedFreeToHeap(__RedCurrHeap, block, 0);
    }
}

// FUNCTION: SWBF2 0x006c3580
void* __cdecl operator new[](unsigned int size)
{
    return RedAllocFromHeap(__RedCurrHeap, size, 0);
}

// FUNCTION: SWBF2 0x006c35a0
void __cdecl operator delete[](void* block)
{
    if (block)
    {
        RedFreeToHeap(__RedCurrHeap, block, 0);
    }
}

// FUNCTION: SWBF2 0x006c35c0
void* RedAllocFromHeapInternal(int heap, unsigned int size, int bTop)
{
    if (!isHeapObjInitialized)
    {
        Startup_RedInitHeap();
    }
    if (heap < 0 || heap >= maxheaps)
    {
        return 0;
    }

    HeapObj& h = heapobj[heap];
    unsigned int mask = ~(h.align - 1);
    unsigned int hdr = (h.align + sizeof(Tag) - 1) & mask;
    unsigned int need = ((size ? size : 1) + h.align - 1 + hdr) & mask;

    Tag* tag;
    if (bTop)
    {
        for (tag = h.free.last; tag; tag = tag->prev_)
        {
            if (need <= tag->size)
            {
                break;
            }
        }
    }
    else
    {
        for (tag = h.free.first; tag; tag = tag->next_)
        {
            if (need <= tag->size)
            {
                break;
            }
        }
    }
    if (!tag)
    {
        if ((unsigned int)h.start < 0x70000000 && sRecursionDepth <= 0)
        {
            RedMemory::failHeap = heap;
            RedMemory::failSize = need;
        }
        return 0;
    }

    unsigned int rest = tag->size - need;
    if (rest < h.align + hdr)
    {
        TagRemoveList(&h.free, tag);
    }
    else if (bTop)
    {
        tag->size = rest;
        tag = (Tag*)((char*)tag + rest);
        tag->next_ = 0;
        tag->prev_ = 0;
        tag->size = need;
    }
    else
    {
        Tag* split = (Tag*)((char*)tag + need);
        split->next_ = 0;
        split->prev_ = 0;
        split->size = 0;
        split->size = tag->size - need;
        split->prev_ = tag->prev_;
        split->next_ = tag->next_;
        tag->size = need;
        if (split->next_)
        {
            split->next_->prev_ = split;
        }
        else
        {
            h.free.last = split;
        }
        if (split->prev_)
        {
            split->prev_->next_ = split;
        }
        else
        {
            h.free.first = split;
        }
    }
    TagAddList(&h.used, tag);
    return (char*)tag + hdr;
}

// FUNCTION: SWBF2 0x006c3730
bool RedFreeToHeapInternal(int heap, void* block, unsigned int flags)
{
    (void)flags;
    if (!isHeapObjInitialized)
    {
        return true;
    }
    if (heap < 0 || heap >= maxheaps)
    {
        return false;
    }

    HeapObj& h = heapobj[heap];
    if (block < h.start || block > (char*)h.start + h.size)
    {
        return false;
    }
    unsigned int hdr = (h.align + sizeof(Tag) - 1) & ~(h.align - 1);
    Tag* tag = (Tag*)((char*)block - hdr);
    TagRemoveList(&h.used, tag);
    TagMergeIntoList(&h.free, tag);
    return true;
}

// FUNCTION: SWBF2 0x006c37b0
void TagAddList(TagList* list, Tag* tag)
{
    if (list->first)
    {
        list->first->prev_ = tag;
    }
    else
    {
        list->last = tag;
    }
    tag->prev_ = 0;
    tag->next_ = list->first;
    list->first = tag;
}

// FUNCTION: SWBF2 0x006c37f0
void TagMergeIntoList(TagList* list, Tag* tag)
{
    Tag* prev = 0;
    Tag* next = list->first;
    while (next && tag > next)
    {
        prev = next;
        next = next->next_;
    }
    tag->next_ = next;
    tag->prev_ = prev;

    if (next)
    {
        next->prev_ = tag;
        if ((char*)tag + tag->size == (char*)next)
        {
            tag->size += next->size;
            next = next->next_;
            tag->next_ = next;
            if (next)
            {
                next->prev_ = tag;
            }
            else
            {
                list->last = tag;
            }
        }
    }
    else
    {
        list->last = tag;
    }

    if (prev)
    {
        prev->next_ = tag;
        if ((char*)prev + prev->size == (char*)tag)
        {
            prev->next_ = next;
            prev->size += tag->size;
            if (next)
            {
                next->prev_ = prev;
            }
            else
            {
                list->last = prev;
            }
        }
    }
    else
    {
        list->first = tag;
    }
}

// FUNCTION: SWBF2 0x006c3880
void TagRemoveList(TagList* list, Tag* tag)
{
    if (tag->next_)
    {
        tag->next_->prev_ = tag->prev_;
    }
    else
    {
        list->last = tag->prev_;
    }
    if (tag->prev_)
    {
        tag->prev_->next_ = tag->next_;
    }
    else
    {
        list->first = tag->next_;
    }
    tag->next_ = 0;
    tag->prev_ = 0;
}

// FUNCTION: SWBF2 0x006c38d0
void* RedAllocFromHeap(int heap, unsigned int size, int bTop)
{
    void* p = RedAllocFromHeapInternal(heap, size, bTop);
    if (!p)
    {
        if (OnFailure)
        {
            OnFailure();
        }
        return 0;
    }
    return p;
}

// FUNCTION: SWBF2 0x006c3900
int RedCreateHeap(void* start, void* end, int align, char const* name)
{
    unsigned int mask = ~(align - 1);
    char* s = (char*)(((unsigned int)start + align - 1) & mask);
    char* e = (char*)((unsigned int)end & mask);
    unsigned int size = e - s;
    if (size)
    {
        memset(s, 0, size);
    }

    int i;
    for (i = 0; i < maxheaps; ++i)
    {
        if (heapobj[i].size == 0)
        {
            break;
        }
    }
    if (i >= maxheaps)
    {
        return -1;
    }

    Tag* tag = (Tag*)s;
    tag->size = size;
    tag->next_ = 0;
    tag->prev_ = 0;

    HeapObj& heap = heapobj[i];
    heap.start = s;
    heap.free.first = tag;
    heap.free.last = tag;
    heap.size = size;
    heap.used.first = 0;
    heap.used.last = 0;
    heap.align = align;
    heap.name = (char*)name;
    heap.frozen = false;
    return i;
}

// FUNCTION: SWBF2 0x006c39b0
void RedDestroyHeap(int heap)
{
    HeapObj& h = heapobj[heap];
    h.free.first = 0;
    h.free.last = 0;
    h.used.first = 0;
    h.used.last = 0;
    h.size = 0;
    h.name = 0;
    h.align = 4;
    h.frozen = false;
}

// FUNCTION: SWBF2 0x006c3a00
void RedFreeToHeap(int heap, void* block, int flags)
{
    if (RedFreeToHeapInternal(heap, block, flags))
    {
        return;
    }
    for (int i = 0; i < maxheaps; ++i)
    {
        if (RedFreeToHeapInternal(i, block, flags))
        {
            return;
        }
    }
    free(block);
}

// FUNCTION: SWBF2 0x006c3aa0
char* RedInitHeap(void* start, void* end, int maxheaps)
{
    (void)end;
    ::maxheaps = maxheaps;
    heapobj = (HeapObj*)(((unsigned int)start + 3) & ~3);
    char* pNext = (char*)(((unsigned int)heapobj + maxheaps * sizeof(HeapObj) + 3) & ~3);
    for (int i = 0; i < maxheaps; ++i)
    {
        HeapObj& h = heapobj[i];
        h.start = 0;
        h.free.first = 0;
        h.free.last = 0;
        h.used.first = 0;
        h.used.last = 0;
        h.size = 0;
        h.align = 4;
        h.frozen = false;
        h.name = 0;
    }
    isHeapObjInitialized = true;
    return pNext;
}

// FUNCTION: SWBF2 0x006c3b20
void RedMarkHeapPermanent(int heap, bool permanent)
{
    heapobj[heap].permanent = permanent;
}

// FUNCTION: SWBF2 0x006c3c10
int RedSetCurrentHeap(int heap)
{
    if (heap >= 0 && heap < maxheaps)
    {
        int old = __RedCurrHeap;
        __RedCurrHeap = heap;
        return old;
    }
    return __RedCurrHeap;
}

// FUNCTION: SWBF2 0x006c3c40
void RedSetDebugHeap(int heap)
{
    __RedDebugHeap = heap;
}

// FUNCTION: SWBF2 0x006c3c50
void RedSetTempHeap(int heap)
{
    __RedTempHeap = heap;
}

// FUNCTION: SWBF2 0x006c3a80
unsigned int RedGetHeapSize(int heap)
{
    return heapobj[heap].size;
}
