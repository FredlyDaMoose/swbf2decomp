#include <string.h>

#include "pcRedIndexBuffer.h"

#include "RedEngineFL/Memory/engineAlloc.h"

// FUNCTION: SWBF2 0x006c9c40
pcRedIndexBuffer::pcRedIndexBuffer(unsigned int uiNumIndices, bool bStatic,
                                   bool bShared)
{

    m_listNode._pList = 0;
    m_listNode._pNext = 0;
    m_listNode._pPrev = 0;
    m_listNode._pObject = 0;

    memset(&m_desc, 0, sizeof(m_desc));
    m_desc.Format = D3DFMT_INDEX16;
    m_desc.Type = D3DRTYPE_INDEXBUFFER;
    m_desc.Size = uiNumIndices * 2;
    m_desc.Pool = bStatic ? D3DPOOL_MANAGED : D3DPOOL_DEFAULT;
    if (bStatic)
    {
        m_desc.Usage = D3DUSAGE_WRITEONLY;
    }
    else
    {
        m_desc.Usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
    }

    m_appendPos = 0;
    m_maxIndices = uiNumIndices;
    m_static = bStatic;
    m_shared = bShared;
    m_locked = 0;
    m_flush = false;
    m_numIndicesReserved = 0;
    m_buffer = 0;

    RedRenderer::pcGetDevice()->CreateIndexBuffer(m_desc.Size, m_desc.Usage,
                                                  m_desc.Format, m_desc.Pool,
                                                  &m_buffer, 0);
}

// FUNCTION: SWBF2 0x006c9d50
pcRedIndexBuffer::~pcRedIndexBuffer()
{

    Unlock();
    if (m_buffer)
    {
        m_buffer->Release();
        m_buffer = 0;
    }
}

// FUNCTION: SWBF2 0x006ca280
void pcRedIndexBuffer::Unlock()
{
    if (m_locked != 0 && m_buffer)
    {
        m_buffer->Unlock();
        --m_locked;
    }
}

// GLOBAL: SWBF2 0x007df0ec
PblList<pcRedIndexBuffer> pcRedIndexBuffer::s_staticIndexBufferList;
// GLOBAL: SWBF2 0x007df140
PblList<pcRedIndexBuffer> pcRedIndexBuffer::s_dynamicIndexBufferList;

// FUNCTION: SWBF2 0x006c9f80
template <>
void PblList<pcRedIndexBuffer>::Clear()
{
    while (_iCount > 0)
    {
        Node* pNode = _head._pNext;
        Remove(pNode);
    }
}

// FUNCTION: SWBF2 0x006c9fc0
pcRedIndexBuffer* pcRedIndexBuffer::Create(unsigned int uiNumIndices,
                                           bool bStatic, bool bShared)
{
    PblList<pcRedIndexBuffer>& rList =
        bStatic ? s_staticIndexBufferList : s_dynamicIndexBufferList;

    for (PblList<pcRedIndexBuffer>::Node* pNode = rList.GetFirst();
         pNode->_pObject != rList._head._pObject;
         pNode = pNode->_pNext)
    {
        pcRedIndexBuffer* pFound = pNode->_pObject;
        if (pFound->m_shared &&
            pFound->m_numIndicesReserved + uiNumIndices <= pFound->m_maxIndices)
        {
            pFound->m_numIndicesReserved += uiNumIndices;
            pFound->AddRef();
            return pFound;
        }
    }

    unsigned int uiCount = uiNumIndices;
    if (bStatic && bShared && uiNumIndices < 0x800)
    {
        uiCount = 0x800;
    }

    pcRedIndexBuffer* pBuffer = new pcRedIndexBuffer(uiCount, bStatic, bShared);
    pBuffer->m_numIndicesReserved = uiNumIndices;
    rList.AddFirst(&pBuffer->m_listNode, pBuffer);
    pBuffer->AddRef();
    return pBuffer;
}

// FUNCTION: SWBF2 0x006ca130
void pcRedIndexBuffer::Init()
{
    s_staticIndexBufferList.Clear();
    s_dynamicIndexBufferList.Clear();

    RedRenderer::pcRegisterResetFunction(&pcRedIndexBuffer::Reset);
}

// FUNCTION: SWBF2 0x006c9df0
void pcRedIndexBuffer::Cleanup()
{
    RedRenderer::pcUnregisterResetFunction(&pcRedIndexBuffer::Reset);

    while (s_dynamicIndexBufferList.GetCount())
    {
        PblList<pcRedIndexBuffer>::Node* pNode = s_dynamicIndexBufferList.GetFirst();
        pcRedIndexBuffer* pBuffer = pNode->_pObject;
        s_dynamicIndexBufferList.Remove(pNode);
        while (pBuffer->Release())
        {
        }
    }

    while (s_staticIndexBufferList.GetCount())
    {
        PblList<pcRedIndexBuffer>::Node* pNode = s_staticIndexBufferList.GetFirst();
        pcRedIndexBuffer* pBuffer = pNode->_pObject;
        s_staticIndexBufferList.Remove(pNode);
        while (pBuffer->Release())
        {
        }
    }

    s_dynamicIndexBufferList.Clear();
    s_staticIndexBufferList.Clear();
}

// FUNCTION: SWBF2 0x006ca0f0
void pcRedIndexBuffer::Flush()
{
    m_flush = true;
    m_numIndicesReserved = 0;
}

// FUNCTION: SWBF2 0x006ca100
void pcRedIndexBuffer::FlushAllDynamicIndexBuffers()
{
    for (PblList<pcRedIndexBuffer>::Itor it = s_dynamicIndexBufferList.Begin();
         (pcRedIndexBuffer*)it != (pcRedIndexBuffer*)s_dynamicIndexBufferList.End(); ++it)
    {
        it->Flush();
    }
}

// FUNCTION: SWBF2 0x006ca1e0
void pcRedIndexBuffer::Reset(ResetFlagsT eFlags)
{
    if (eFlags & RESETFLAGS_FREE_RESOURCES)
    {
        for (PblList<pcRedIndexBuffer>::Itor it = s_dynamicIndexBufferList.Begin();
             (pcRedIndexBuffer*)it != (pcRedIndexBuffer*)s_dynamicIndexBufferList.End(); ++it)
        {
            it->m_buffer->Release();
        }
    }

    if (eFlags & RESETFLAGS_CREATE_RESOURCES)
    {
        for (PblList<pcRedIndexBuffer>::Itor it = s_dynamicIndexBufferList.Begin();
             (pcRedIndexBuffer*)it != (pcRedIndexBuffer*)s_dynamicIndexBufferList.End(); ++it)
        {
            RedRenderer::pcGetDevice()->CreateIndexBuffer(
                2 * it->m_maxIndices, it->m_desc.Usage, it->m_desc.Format,
                it->m_desc.Pool, &it->m_buffer, 0);
            it->m_appendPos = 0;
            it->m_numIndicesReserved = 0;
        }
    }
}

// FUNCTION: SWBF2 0x006ca150
unsigned short* pcRedIndexBuffer::Lock(unsigned int uiNumIndices, unsigned int& ruiStartIndex)
{
    void* pData = 0;

    if (!m_buffer)
    {
        return 0;
    }

    DWORD dwFlags;
    if (m_static)
    {
        dwFlags = D3DLOCK_NOSYSLOCK;
    }
    else if (!m_flush && m_appendPos + uiNumIndices <= m_maxIndices)
    {
        dwFlags = D3DLOCK_NOOVERWRITE;
    }
    else
    {
        m_flush = false;
        dwFlags = D3DLOCK_DISCARD;
        m_appendPos = 0;
    }

    if (m_buffer->Lock(m_appendPos * sizeof(unsigned short),
                       uiNumIndices * sizeof(unsigned short), &pData, dwFlags) == D3D_OK)
    {
        ++m_locked;
        ruiStartIndex = m_appendPos;
        m_appendPos += uiNumIndices;
    }

    return (unsigned short*)pData;
}
