#ifndef ENGINE_ALLOC_H
#define ENGINE_ALLOC_H

void* __cdecl operator new(unsigned int size);
void __cdecl operator delete(void* block);
void* __cdecl operator new[](unsigned int size);
void __cdecl operator delete[](void* block);

extern "C" int __RedCurrHeap;

#endif
