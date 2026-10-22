#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblList.h"

class MemoryPool
{
public:

    MemoryPool(char const* pcLabel, int iSize);

    void Create(unsigned int uiCount, unsigned int uiSize, unsigned int uiGrow);

    void Reset();

    void Setup(unsigned int uiCount, unsigned int uiSize, unsigned int uiGrow);

    bool IsPresent(void* pBlock) const
    {
        const char* pcPool = (const char*)mPool;
        if (pcPool == 0 || (const char*)pBlock < pcPool)
        {
            return false;
        }
        return (const char*)pBlock < pcPool + mCount * mSize;
    }

    // FUNCTION: SWBF2 0x004374e0
    unsigned int GetSize() const { return mSize; }
    // FUNCTION: SWBF2 0x004374f0
    unsigned int GetGrow() const { return mGrow; }

    void* Allocate(unsigned int uiSize);

    void Free(void* pBlock);

    ~MemoryPool();

    void Destroy();
    void SetCount(unsigned int uiCount);

    // FUNCTION: SWBF2 0x0044c530
    unsigned int GetCount() const { return mCount; }

    bool IsFull() const { return mUsed >= mCount; }

    bool IsSufficientSpace(unsigned int uCount) const { return mUsed + uCount <= mCount; }
    void SetSize(unsigned int uiSize);

    static void CreateAll();
    static void DestroyAll();
    static MemoryPool* Find(char const* pcLabel);

    void Set(MemoryPool const& rPool);

    static void PoolErrorPostLoad();

    static bool sErrorOnExpansionPostLoad;

    static bool sErrorOnExpansion;

    PblList<MemoryPool>::Node mNode;
    char mLabel[32];
    unsigned int mSize;
    unsigned int mCount;
    unsigned int mGrow;
    unsigned int mUsed;
    int mHeap;
    void* mPool;
    bool mPoolOwner;
    void* mFree;
};
 
 
 
 
 
 
 
 
 
 

PblList<MemoryPool>* GetMemoryPoolList();
