#pragma once

#include <stddef.h>

struct GlyphVertexT
{
    float x;
    float y;
    float z;
    float u;
    float v;
};

class RedGlyphMemoryManager
{
public:

    // FUNCTION: SWBF2 0x00722430
    RedGlyphMemoryManager(unsigned int uiNumVertices);

    struct VertexBlock
    {
        GlyphVertexT Vertices[48];
        unsigned int uiNumVertices;
        VertexBlock* pNext;
    };

    class Iterator
    {
    public:
        // FUNCTION: SWBF2 0x00722410
        Iterator(void* pBlock);

        Iterator(const Iterator& rOther);

        operator void*() const;

        // FUNCTION: SWBF2 0x007224b0
        operator bool() const;

        GlyphVertexT* operator->();

        // FUNCTION: SWBF2 0x007224d0
        GlyphVertexT& operator*();

        const Iterator& operator++();

        // FUNCTION: SWBF2 0x00722680
        unsigned int GetBlockSize() const;

        // FUNCTION: SWBF2 0x00722690
        void NextBlock();

        VertexBlock* m_pCurrentBlock;
        unsigned int m_uiCurrentVertex;
    };

    void Free(void* pBlock);

    void Free(Iterator& it);

    Iterator Alloc(unsigned int uiNumQuads);

    void TraceMemory();

private:

    VertexBlock* AllocBlock();

    void FreeBlock(VertexBlock* pBlock);

public:
    VertexBlock* m_pPool;
    VertexBlock* m_pFirstFree;
};
 
 
 
