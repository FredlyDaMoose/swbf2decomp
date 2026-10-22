#define _CRT_SECURE_NO_WARNINGS

#include <string.h>

#include "RedMemoryPool.h"
#include "RedMemory.h"

// FUNCTION: SWBF2 0x006dc640
PblList<MemoryPool>* GetMemoryPoolList()
{
    static PblList<MemoryPool> sPoolList;
    return &sPoolList;
}

// FUNCTION: SWBF2 0x006dc2a0
MemoryPool::MemoryPool(char const* pcLabel, int iSize)
{
    mPoolOwner = true;
    mSize = ((unsigned int)iSize + 3) & ~3u;
    mGrow = 0;
    Reset();
    strncpy(mLabel, pcLabel, sizeof(mLabel) - 1);
    mLabel[sizeof(mLabel) - 1] = '\0';

    PblList<MemoryPool>* pList = GetMemoryPoolList();
    pList->AddLast(&mNode, this);
}

// FUNCTION: SWBF2 0x006dc440
void MemoryPool::Create(unsigned int uiCount, unsigned int uiSize, unsigned int uiGrow)
{
    if (uiSize == 0)
    {
        return;
    }

    uiSize = (uiSize + 3) & ~3u;
    mSize = uiSize;
    mCount = uiCount;
    if (uiSize != 0 && uiGrow == 0)
    {
        uiGrow = (uiSize + 63) / uiSize;
    }
    mGrow = uiGrow;
    mUsed = 0;
    mHeap = __RedCurrHeap;
    if (uiCount == 0)
    {
        return;
    }

    mPoolOwner = true;
    mPool = RedAllocFromHeap(__RedCurrHeap, uiSize * uiCount, 0);

    unsigned int i;
    for (i = 0; i < mCount - 1; ++i)
    {
        *(char**)((char*)mPool + i * mSize) = (char*)mPool + (i + 1) * mSize;
    }
    *(char**)((char*)mPool + (mCount - 1) * mSize) = 0;
    mFree = mPool;
}

bool MemoryPool::sErrorOnExpansion;

// FUNCTION: SWBF2 0x006dc370
void* MemoryPool::Allocate(unsigned int uiSize)
{
    (void)uiSize;

    if (mPool == 0)
    {
        Create(mCount, mSize, mGrow);
    }

    if (mFree == 0)
    {
        sErrorOnExpansion = true;
        if (mHeap == -1)
        {
            mHeap = __RedCurrHeap;
        }

        int iOldHeap = RedSetCurrentHeap(mHeap);
        mFree = RedAllocFromHeap(__RedCurrHeap, mSize * mGrow, 0);
        RedSetCurrentHeap(iOldHeap);
        if (mFree == 0)
        {
            return 0;
        }

        mCount += mGrow;
        unsigned int i;
        for (i = 0; i < mGrow - 1; ++i)
        {
            *(char**)((char*)mFree + i * mSize) = (char*)mFree + (i + 1) * mSize;
        }
        *(char**)((char*)mFree + (mGrow - 1) * mSize) = 0;

        if (mPool == 0)
        {
            mPool = mFree;
        }
    }

    void* pBlock = mFree;
    mFree = *(void**)pBlock;
    ++mUsed;
    return pBlock;
}

// FUNCTION: SWBF2 0x006dc5f0
void MemoryPool::Free(void* pBlock)
{
    if (pBlock == 0)
    {
        return;
    }

    char* pcBlock = (char*)pBlock;
    char* pcNode = (char*)mFree;
    if (pcNode == 0 || pcBlock <= pcNode)
    {
        *(void**)pBlock = mFree;
        --mUsed;
        mFree = pBlock;
        return;
    }

    while (*(char**)pcNode != 0)
    {
        char* pcNext = *(char**)pcNode;
        if (pcBlock <= pcNext)
        {
            break;
        }
        pcNode = pcNext;
    }
    *(void**)pBlock = *(void**)pcNode;
    *(void**)pcNode = pBlock;
    --mUsed;
}

// FUNCTION: SWBF2 0x006dc6a0
void MemoryPool::Reset()
{
    mCount = 0;
    mUsed = 0;
    mHeap = -1;
    mPool = 0;
    mFree = 0;
}

// FUNCTION: SWBF2 0x006dc750
void MemoryPool::Setup(unsigned int uiCount, unsigned int uiSize,
                       unsigned int uiGrow)
{
    if (mPool == 0)
    {
        mCount = uiCount;
        mSize = uiSize;
        mGrow = uiGrow;
    }
}

// FUNCTION: SWBF2 0x006dc320
MemoryPool::~MemoryPool()
{
    GetMemoryPoolList()->Remove(&mNode);
    Destroy();
}

// FUNCTION: SWBF2 0x006dc4f0
void MemoryPool::CreateAll()
{
    for (PblList<MemoryPool>::Itor itor(*GetMemoryPoolList()); itor; ++itor)
    {
        if (itor->mPool == 0)
        {
            itor->Create(itor->mCount, itor->mSize, itor->mGrow);
        }
    }
}

// FUNCTION: SWBF2 0x006dc530
void MemoryPool::Destroy()
{
    if (mPool != 0 && mPoolOwner)
    {
        RedFreeToHeap(__RedCurrHeap, mPool, 0);
    }
    Reset();
}

// FUNCTION: SWBF2 0x006dc560
void MemoryPool::DestroyAll()
{
    for (PblList<MemoryPool>::Itor itor(*GetMemoryPoolList()); itor; ++itor)
    {
        if (itor->mPool != 0)
        {
            itor->Destroy();
        }
    }
    sErrorOnExpansionPostLoad = false;
    sErrorOnExpansion = false;
}

// FUNCTION: SWBF2 0x006dc5a0
MemoryPool* MemoryPool::Find(char const* pcLabel)
{
    for (PblList<MemoryPool>::Itor itor(*GetMemoryPoolList()); itor; ++itor)
    {
        if (_strnicmp(itor->mLabel, pcLabel, sizeof(itor->mLabel) - 1) == 0)
        {
            return itor;
        }
    }
    return 0;
}

// GLOBAL: SWBF2 0x0099e7c9
bool MemoryPool::sErrorOnExpansionPostLoad;

// FUNCTION: SWBF2 0x006dc690
void MemoryPool::PoolErrorPostLoad()
{
    sErrorOnExpansionPostLoad = true;
}

// FUNCTION: SWBF2 0x006dc6d0
void MemoryPool::Set(MemoryPool const& rPool)
{
    mPoolOwner = false;
    mSize = rPool.mSize;
    mCount = rPool.mCount;
    mGrow = rPool.mGrow;
    mUsed = rPool.mUsed;
    mHeap = rPool.mHeap;
    mPool = rPool.mPool;
    mFree = rPool.mFree;
}

// FUNCTION: SWBF2 0x006dc710
void MemoryPool::SetCount(unsigned int uiCount)
{
    if (mPool == 0)
    {
        mCount = uiCount;
    }
}

// FUNCTION: SWBF2 0x006dc730
void MemoryPool::SetSize(unsigned int uiSize)
{
    if (mPool == 0)
    {
        mSize = uiSize;
    }
}
