#ifndef SWBF2_GRAPHICS_PC_PCREDVERTEXBUFFER_H
#define SWBF2_GRAPHICS_PC_PCREDVERTEXBUFFER_H

#include <unknwn.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblRef.h"

#include "pcRedRenderer.h"

// VTABLE: SWBF2 0x00772d98
class pcRedVertexBuffer : public PblRef
{
public:

    virtual ~pcRedVertexBuffer();

    // SYNTHETIC: SWBF2 0x006ca420
    // pcRedVertexBuffer::`scalar deleting destructor'

    unsigned int m_vertexSize;
    unsigned int m_appendPos;
    unsigned int m_maxVertices;
    unsigned int m_locked;
    unsigned int m_numBytesUsed;
    bool m_static;
    bool m_shared;
    bool m_flush;
    D3DVERTEXBUFFER_DESC m_desc;
    IDirect3DVertexBuffer9* m_buffer;
    PblList<pcRedVertexBuffer>::Node m_listNode;

    pcRedVertexBuffer(unsigned int uiMaxVertices, unsigned int uiVertexSize,
                      bool bStatic, bool bShared);

    static pcRedVertexBuffer* Create(unsigned int uiNumVertices,
                                     unsigned int uiVertexSize,
                                     bool bStatic, bool bShared);

    void* Lock(unsigned int uiNumVertices, unsigned int& ruiStartVertex);

    void Unlock();

    static void Init();

    static void Cleanup();

    void Flush();

    static void FlushAllDynamicVertexBuffers();

    static void Reset(ResetFlagsT eFlags);
};

 

template <> void PblList<pcRedVertexBuffer>::Clear();

 #endif
