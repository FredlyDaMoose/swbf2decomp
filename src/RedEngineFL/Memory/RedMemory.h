#pragma once

struct Tag
{
    Tag* prev_;
    Tag* next_;
    unsigned int size;
};
 

struct TagList
{
    Tag* first;
    Tag* last;
};
 

struct HeapObj
{
    void* start;
    TagList free;
    TagList used;
    unsigned int size;
    char* name;
    unsigned int align;
    bool frozen;
    bool permanent;
};
 

extern "C"
{
    char* RedInitHeap(void* start, void* end, int maxheaps);
    int RedCreateHeap(void* start, void* end, int align, char const* name);
    void RedDestroyHeap(int heap);
    int RedSetCurrentHeap(int heap);
    void RedSetDebugHeap(int heap);
    void* RedAllocFromHeap(int heap, unsigned int size, int bTop);
    void RedFreeToHeap(int heap, void* block, int flags);
    void RedMarkHeapPermanent(int heap, bool permanent);
    void RedSetTempHeap(int heap);

    unsigned int RedGetHeapFree(int heap);

    extern int __RedCurrHeap;
    extern int __RedDebugHeap;
    extern int __RedTempHeap;
}
