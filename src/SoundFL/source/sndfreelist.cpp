#include "sndfreelist.h"

#include "sndglobals.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00738610
bool FreeList::AllocBlocks(unsigned int uiNumBlocks, Block** ppFirstBlock)
{

    if (mAllocFunc == 0 || mFreeFunc == 0)
    {
        mAllocFunc = Globals::smAlloc;
        mFreeFunc = Globals::smFree;
    }

    while (uiNumBlocks != 0)
    {
        --uiNumBlocks;

        const unsigned int uiBlockBytes = mEntriesPerBlock * mEntrySize;
        const unsigned int uiBitmapBytes = (mEntriesPerBlock + 7) / 8;

        void* const pAllocAddress =
            mAllocFunc(mAlignment + (sizeof(Block) - 1)
                       + uiBlockBytes + uiBitmapBytes);
        if (!pAllocAddress)
        {
            return false;
        }

        char* const pEntries =
            (char*)((((unsigned int)pAllocAddress + mAlignment - 1) / mAlignment)
                    * mAlignment);
        Block* const pBlock = (Block*)(pEntries + uiBlockBytes);

        Block* const pNew = new (pBlock, false) Block(pAllocAddress);

        pNew->mBitmap.Initialize((unsigned char*)pNew + sizeof(Block),
                                 mEntriesPerBlock);

        mBlockList.AddLast(&pNew->mNode);

        if (ppFirstBlock && *ppFirstBlock == 0)
        {
            *ppFirstBlock = pNew;
        }
    }

    return true;
}

// FUNCTION: SWBF2 0x00738720
void* FreeList::Block::AllocEntry(unsigned int uiEntrySize, unsigned int uiEntriesPerBlock)
{
    AllocBitmap::Handle handle;
    handle.mStartEntry = 0;
    handle.mNumEntries = 0;
    if (!mBitmap.Alloc(1, handle))
    {
        return 0;
    }
    return (char*)this + (handle.mStartEntry - uiEntriesPerBlock) * uiEntrySize;
}

// FUNCTION: SWBF2 0x00738560
void* FreeList::Alloc()
{
    for (PblListSingle::Node* pNode = mBlockList.mHead.mNext;
         pNode != &mBlockList.mHead;
         pNode = pNode->mNext)
    {
        Block* const pBlock = reinterpret_cast<Block*>(reinterpret_cast<char*>(pNode) - offsetof(Block, mNode));
        void* const pEntry = pBlock->AllocEntry(mEntrySize, mEntriesPerBlock);
        if (pEntry)
        {
            return pEntry;
        }
    }

    if (mFlags & flags_locked)
    {
        return 0;
    }

    Block* pBlock = 0;
    if (!AllocBlocks(1, &pBlock))
    {
        return 0;
    }
    return pBlock->AllocEntry(mEntrySize, mEntriesPerBlock);
}

// FUNCTION: SWBF2 0x00738770
void FreeList::Free(void* p)
{
    if (!mBlockList.IsEmpty())
    {
        const unsigned int uiEntrySize = mEntrySize;
        const unsigned int uiBlockBytes =
            mEntriesPerBlock * uiEntrySize;

        for (PblListSingle::Node* pNode = mBlockList.mHead.mNext;
             pNode != &mBlockList.mHead;
             pNode = pNode->mNext)
        {

            Block* const pBlock = reinterpret_cast<Block*>(reinterpret_cast<char*>(pNode) - offsetof(Block, mNode));
            char* const pEntries = (char*)pBlock - uiBlockBytes;

            if ((char*)p < pEntries)
            {
                continue;
            }
            if ((char*)p >= (char*)pBlock)
            {
                continue;
            }

            const unsigned int uiIndex =
                (unsigned int)((char*)p - pEntries) / uiEntrySize;
            pBlock->mBitmap.Reserve(uiIndex, 1, false);
            break;
        }
    }

    const unsigned char ucFlags = mFlags;
    if ((ucFlags & 1) != 0 && (ucFlags & 2) == 0)
    {
        PurgeBlocks();
    }
}

// FUNCTION: SWBF2 0x00738800
void FreeList::PurgeBlocks()
{

    if (mAllocFunc == 0 || mFreeFunc == 0)
    {
        mAllocFunc = Globals::smAlloc;
        mFreeFunc = Globals::smFree;
    }

    PblListSingle::Node* pNode = mBlockList.mHead.mNext;
    while (pNode != &mBlockList.mHead)
    {
        Block* const pBlock = reinterpret_cast<Block*>(reinterpret_cast<char*>(pNode) - offsetof(Block, mNode));
        pNode = pNode->mNext;

        if (pBlock->mBitmap.GetFreeEntries() != mEntriesPerBlock)
        {
            continue;
        }

        void* const pAllocAddress = pBlock->mAllocAddress;

        if (pBlock->mNode.mNext != 0)
        {
            pBlock->mNode.Remove();
        }

        if (pBlock->mAllocType == Block::allocType_heap)
        {
            Snd::Free(pBlock);
        }
        else if (pBlock->mAllocType == Block::allocType_tempHeap)
        {
            Snd::FreeTemp(pBlock);
        }

        mFreeFunc(pAllocAddress);
    }
}

}
