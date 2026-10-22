#include <string.h>

#include "RedGlyphMemoryManager.h"

#include "RedEngineFL/Memory/RedMemory.h"

// FUNCTION: SWBF2 0x007223f0
RedGlyphMemoryManager::Iterator::Iterator(const Iterator& rOther)
{
    m_pCurrentBlock = rOther.m_pCurrentBlock;
    m_uiCurrentVertex = rOther.m_uiCurrentVertex;
}

// FUNCTION: SWBF2 0x00722410
RedGlyphMemoryManager::Iterator::Iterator(void* pBlock)
{
    m_pCurrentBlock = (VertexBlock*)pBlock;
    m_uiCurrentVertex = 0;
}

// FUNCTION: SWBF2 0x00722430
RedGlyphMemoryManager::RedGlyphMemoryManager(unsigned int uiNumVertices)
{
    const unsigned int uiNumBlocks = (uiNumVertices + 7) >> 3;
    const unsigned int uiBytes = uiNumBlocks * sizeof(VertexBlock);

    m_pPool = (VertexBlock*)RedAllocFromHeap(__RedCurrHeap, uiBytes, 0);
    memset(m_pPool, 0, uiBytes);

    m_pFirstFree = m_pPool;

    for (unsigned int i = 1; i < uiNumBlocks; i++)
    {
        m_pPool[i - 1].pNext = &m_pPool[i];
    }

    m_pPool[uiNumBlocks - 1].pNext = 0;
}

// FUNCTION: SWBF2 0x00722510
RedGlyphMemoryManager::Iterator RedGlyphMemoryManager::Alloc(unsigned int uiNumQuads)
{
    if (!uiNumQuads)
    {
        return Iterator(0);
    }

    VertexBlock* pBlock = AllocBlock();
    if (pBlock)
    {
        Iterator it(pBlock);

        while (uiNumQuads > 8)
        {
            VertexBlock* pNext = AllocBlock();
            pBlock->pNext = pNext;
            if (!pNext)
            {
                for (VertexBlock* p = it.m_pCurrentBlock; p;)
                {
                    VertexBlock* pFree = p->pNext;
                    FreeBlock(p);
                    p = pFree;
                }
                return Iterator(0);
            }
            uiNumQuads -= 8;
            pBlock = pNext;
        }

        pBlock->pNext = 0;
        pBlock->uiNumVertices = uiNumQuads * 6;
        return it;
    }

    return Iterator(0);
}

// FUNCTION: SWBF2 0x007225d0
RedGlyphMemoryManager::VertexBlock* RedGlyphMemoryManager::AllocBlock()
{
    VertexBlock* pBlock = m_pFirstFree;
    if (pBlock)
    {
        m_pFirstFree = pBlock->pNext;
        pBlock->uiNumVertices = 48;
    }
    return pBlock;
}

// FUNCTION: SWBF2 0x007225f0
void RedGlyphMemoryManager::Free(Iterator& it)
{
    VertexBlock* pBlock = it.m_pCurrentBlock;
    if (pBlock && pBlock->uiNumVertices)
    {
        do
        {
            VertexBlock* pNext = pBlock->pNext;
            FreeBlock(pBlock);
            pBlock = pNext;
        } while (pBlock);
    }
}

// FUNCTION: SWBF2 0x00722630
void RedGlyphMemoryManager::Free(void* pBlock)
{
    Iterator it(pBlock);
    Free(it);
}

// FUNCTION: SWBF2 0x00722660
void RedGlyphMemoryManager::FreeBlock(VertexBlock* pBlock)
{
    pBlock->uiNumVertices = 0;
    pBlock->pNext = m_pFirstFree;
    m_pFirstFree = pBlock;
}

// FUNCTION: SWBF2 0x007226b0
void RedGlyphMemoryManager::TraceMemory()
{
}

// FUNCTION: SWBF2 0x007224a0
RedGlyphMemoryManager::Iterator::operator void*() const
{
    return m_pCurrentBlock;
}

// FUNCTION: SWBF2 0x007224b0
RedGlyphMemoryManager::Iterator::operator bool() const
{
    return m_pCurrentBlock != 0;
}

// FUNCTION: SWBF2 0x007224c0
GlyphVertexT* RedGlyphMemoryManager::Iterator::operator->()
{
    return &m_pCurrentBlock->Vertices[m_uiCurrentVertex];
}

// FUNCTION: SWBF2 0x007224d0
GlyphVertexT& RedGlyphMemoryManager::Iterator::operator*()
{
    return m_pCurrentBlock->Vertices[m_uiCurrentVertex];
}

// FUNCTION: SWBF2 0x007224e0
const RedGlyphMemoryManager::Iterator& RedGlyphMemoryManager::Iterator::operator++()
{
    if (m_pCurrentBlock)
    {
        if (++m_uiCurrentVertex == m_pCurrentBlock->uiNumVertices)
        {
            NextBlock();
        }
    }
    return *this;
}

// FUNCTION: SWBF2 0x00722680
unsigned int RedGlyphMemoryManager::Iterator::GetBlockSize() const
{
    return m_pCurrentBlock->uiNumVertices;
}

// FUNCTION: SWBF2 0x00722690
void RedGlyphMemoryManager::Iterator::NextBlock()
{
    VertexBlock* pBlock = m_pCurrentBlock;
    m_uiCurrentVertex = 0;
    m_pCurrentBlock = pBlock->pNext;
}
