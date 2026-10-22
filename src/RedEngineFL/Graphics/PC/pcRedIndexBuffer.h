#ifndef SWBF2_GRAPHICS_PC_PCREDINDEXBUFFER_H
#define SWBF2_GRAPHICS_PC_PCREDINDEXBUFFER_H

#include <unknwn.h>

#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblRef.h"

#include "pcRedRenderer.h"

// VTABLE: SWBF2 0x00772d78
class pcRedIndexBuffer : public PblRef
{
public:

    virtual ~pcRedIndexBuffer();

    // SYNTHETIC: SWBF2 0x006c9dc0
    // pcRedIndexBuffer::`scalar deleting destructor'

    unsigned int m_appendPos;
    unsigned int m_maxIndices;
    bool m_static;
    bool m_shared;
    unsigned int m_locked;
    bool m_flush;
    D3DINDEXBUFFER_DESC m_desc;
    unsigned int m_numIndicesReserved;
    IDirect3DIndexBuffer9* m_buffer;
    PblList<pcRedIndexBuffer>::Node m_listNode;

    pcRedIndexBuffer(unsigned int uiNumIndices, bool bStatic, bool bShared);

    static pcRedIndexBuffer* Create(unsigned int uiNumIndices, bool bStatic,
                                    bool bShared);

    void Unlock();

    unsigned short* Lock(unsigned int uiNumIndices, unsigned int& ruiStartIndex);

    static void Init();

    static void Cleanup();

    void Flush();

    static void FlushAllDynamicIndexBuffers();

    static void Reset(ResetFlagsT eFlags);

    static PblList<pcRedIndexBuffer> s_staticIndexBufferList;
    static PblList<pcRedIndexBuffer> s_dynamicIndexBufferList;
};

 

template <> void PblList<pcRedIndexBuffer>::Clear();

 #endif
