#include <string.h>

#include "pcRedVertexBuffer.h"

#include "RedEngineFL/Memory/engineAlloc.h"

// FUNCTION: SWBF2 0x006ca2a0
pcRedVertexBuffer::pcRedVertexBuffer(unsigned int uiMaxVertices,
                                     unsigned int uiVertexSize,
                                     bool bStatic, bool bShared)
{

    m_listNode._pList = 0;
    m_listNode._pNext = 0;
    m_listNode._pPrev = 0;
    m_listNode._pObject = 0;

    m_static = bStatic;
    m_maxVertices = uiMaxVertices;
    m_vertexSize = uiVertexSize;
    m_shared = bShared;
    m_numBytesUsed = 0;
    m_appendPos = 0;
    m_locked = 0;
    m_flush = false;

    memset(&m_desc, 0, sizeof(m_desc));
    m_desc.Format = D3DFMT_VERTEXDATA;
    m_desc.Type = D3DRTYPE_VERTEXBUFFER;
    m_desc.Size = uiMaxVertices * uiVertexSize;
    m_desc.Pool = bStatic ? D3DPOOL_MANAGED : D3DPOOL_DEFAULT;
    m_desc.FVF = 0;
    if (bStatic)
    {
        m_desc.Usage = D3DUSAGE_WRITEONLY;
    }
    else
    {
        m_desc.Usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
    }

    RedRenderer::pcGetDevice()->CreateVertexBuffer(m_desc.Size, m_desc.Usage,
                                                   m_desc.FVF, m_desc.Pool,
                                                   &m_buffer, 0);
}

// FUNCTION: SWBF2 0x006ca3b0
pcRedVertexBuffer::~pcRedVertexBuffer()
{

    Unlock();
    if (m_buffer)
    {
        m_buffer->Release();
        m_buffer = 0;
    }
}

// GLOBAL: SWBF2 0x007df16c
PblList<pcRedVertexBuffer> s_nonSharedVertexBufferList;

// GLOBAL: SWBF2 0x00960f58

int sStaticVertexBufferCount;

// GLOBAL: SWBF2 0x00960f5c
// SYNTHETIC: SWBF2 0x00407e60
// `dynamic initializer for 'sStaticVertexBufferHashTable''

PblHashTable<PblList<pcRedVertexBuffer>, 1024> sStaticVertexBufferHashTable;
// GLOBAL: SWBF2 0x00962f68

int sDynamicVertexBufferCount;

// GLOBAL: SWBF2 0x00962f6c
// SYNTHETIC: SWBF2 0x00407e40
// `dynamic initializer for 'sDynamicVertexBufferHashTable''

PblHashTable<PblList<pcRedVertexBuffer>, 1024> sDynamicVertexBufferHashTable;

// FUNCTION: SWBF2 0x006ca790
template <>
void PblList<pcRedVertexBuffer>::Clear()
{
    while (_iCount > 0)
    {
        Node* pNode = _head._pNext;
        Remove(pNode);
    }
}

// FUNCTION: SWBF2 0x006ca450
void pcRedVertexBuffer::Cleanup()
{
    RedRenderer::pcUnregisterResetFunction(&pcRedVertexBuffer::Reset);

    while (s_nonSharedVertexBufferList.GetCount())
    {
        PblList<pcRedVertexBuffer>::Node* pNode = s_nonSharedVertexBufferList.GetFirst();
        pcRedVertexBuffer* pBuffer = pNode->_pObject;
        s_nonSharedVertexBufferList.Remove(pNode);
        while (pBuffer->Release())
        {
        }
    }

    for (PblHashTable<PblList<pcRedVertexBuffer>, 1024>::Itor it = sDynamicVertexBufferHashTable.Begin();
         (PblList<pcRedVertexBuffer>*)it != (PblList<pcRedVertexBuffer>*)sDynamicVertexBufferHashTable.End(); ++it)
    {
        PblList<pcRedVertexBuffer>* pList = it;
        while (pList->GetCount())
        {
            PblList<pcRedVertexBuffer>::Node* pNode = pList->GetFirst();
            pcRedVertexBuffer* pBuffer = pNode->_pObject;
            pList->Remove(pNode);
            while (pBuffer->Release())
            {
            }
        }
        delete pList;
    }

    for (PblHashTable<PblList<pcRedVertexBuffer>, 1024>::Itor it = sStaticVertexBufferHashTable.Begin();
         (PblList<pcRedVertexBuffer>*)it != (PblList<pcRedVertexBuffer>*)sStaticVertexBufferHashTable.End(); ++it)
    {
        PblList<pcRedVertexBuffer>* pList = it;
        while (pList->GetCount())
        {
            PblList<pcRedVertexBuffer>::Node* pNode = pList->GetFirst();
            pcRedVertexBuffer* pBuffer = pNode->_pObject;
            pList->Remove(pNode);
            while (pBuffer->Release())
            {
            }
        }
        delete pList;
    }

    sDynamicVertexBufferHashTable.Clear();
    sDynamicVertexBufferCount = 0;

    sStaticVertexBufferHashTable.Clear();
    sStaticVertexBufferCount = 0;
}

// FUNCTION: SWBF2 0x006ca7d0
pcRedVertexBuffer* pcRedVertexBuffer::Create(unsigned int uiNumVertices,
                                             unsigned int uiVertexSize,
                                             bool bStatic, bool bShared)
{
    if (!bShared)
    {
        pcRedVertexBuffer* pBuffer =
            new pcRedVertexBuffer(uiNumVertices, uiVertexSize, bStatic, true);
        pBuffer->m_numBytesUsed = uiNumVertices * uiVertexSize;
        s_nonSharedVertexBufferList.AddFirst(&pBuffer->m_listNode, pBuffer);
        pBuffer->AddRef();
        return pBuffer;
    }

    PblList<pcRedVertexBuffer>* pList =
        bStatic ? sStaticVertexBufferHashTable.Find(uiVertexSize) : sDynamicVertexBufferHashTable.Find(uiVertexSize);

    unsigned int uiBytes = uiNumVertices * uiVertexSize;

    if (pList)
    {

        for (PblList<pcRedVertexBuffer>::Node* pNode = pList->GetFirst();
             pNode->_pObject != pList->_head._pObject;
             pNode = pNode->_pNext)
        {
            pcRedVertexBuffer* pFound = pNode->_pObject;
            if (pFound->m_shared &&
                pFound->m_numBytesUsed + uiBytes <= pFound->m_desc.Size)
            {
                pFound->m_numBytesUsed += uiBytes;
                pFound->AddRef();
                return pFound;
            }
        }

        unsigned int uiCount = uiNumVertices;
        if (bStatic && uiBytes < 0x8000)
        {
            uiCount = 0x8000 / uiVertexSize;
        }

        pcRedVertexBuffer* pBuffer =
            new pcRedVertexBuffer(uiCount, uiVertexSize, bStatic, bShared);
        pBuffer->m_numBytesUsed = uiBytes;
        pList->AddFirst(&pBuffer->m_listNode, pBuffer);
        pBuffer->AddRef();
        return pBuffer;
    }

    unsigned int uiCount = uiNumVertices;
    if (bStatic && uiNumVertices * uiVertexSize < 0x8000)
    {
        uiCount = 0x8000 / uiVertexSize;
    }

    pcRedVertexBuffer* pBuffer =
        new pcRedVertexBuffer(uiCount, uiVertexSize, bStatic, bShared);

    PblList<pcRedVertexBuffer>* pNewList = new PblList<pcRedVertexBuffer>;

    pBuffer->m_numBytesUsed = uiNumVertices * uiVertexSize;
    pNewList->AddFirst(&pBuffer->m_listNode, pBuffer);

    if (bStatic)
    {
        if (sStaticVertexBufferHashTable.Store(uiVertexSize, pNewList))
        {
            sStaticVertexBufferCount++;
        }
    }
    else
    {
        if (sDynamicVertexBufferHashTable.Store(uiVertexSize, pNewList))
        {
            sDynamicVertexBufferCount++;
        }
    }

    pBuffer->AddRef();
    return pBuffer;
}

// FUNCTION: SWBF2 0x006cabc0
void pcRedVertexBuffer::Init()
{
    sDynamicVertexBufferHashTable.Clear();
    sDynamicVertexBufferCount = 0;

    sStaticVertexBufferHashTable.Clear();
    sStaticVertexBufferCount = 0;

    s_nonSharedVertexBufferList.Clear();

    RedRenderer::pcRegisterResetFunction(&pcRedVertexBuffer::Reset);
}

// FUNCTION: SWBF2 0x006cac10
void* pcRedVertexBuffer::Lock(unsigned int uiNumVertices,
                              unsigned int& ruiStartVertex)
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
    else if (!m_flush && m_appendPos + uiNumVertices <= m_maxVertices)
    {
        dwFlags = D3DLOCK_NOOVERWRITE;
    }
    else
    {
        m_flush = false;
        dwFlags = D3DLOCK_DISCARD;
        m_appendPos = 0;
    }

    if (SUCCEEDED(m_buffer->Lock(m_appendPos * m_vertexSize,
                                 m_vertexSize * uiNumVertices, &pData, dwFlags)))
    {
        ++m_locked;
        ruiStartVertex = m_appendPos;
        m_appendPos += uiNumVertices;
    }

    return pData;
}

// FUNCTION: SWBF2 0x006caee0
void pcRedVertexBuffer::Unlock()
{
    if (m_locked != 0 && m_buffer)
    {
        m_buffer->Unlock();
        --m_locked;
    }
}

// FUNCTION: SWBF2 0x006caae0
void pcRedVertexBuffer::Flush()
{
    m_flush = true;
    m_numBytesUsed = 0;
}

// FUNCTION: SWBF2 0x006caaf0
void pcRedVertexBuffer::FlushAllDynamicVertexBuffers()
{
    for (PblHashTable<PblList<pcRedVertexBuffer>, 1024>::Itor it = sDynamicVertexBufferHashTable.Begin();
         (PblList<pcRedVertexBuffer>*)it != (PblList<pcRedVertexBuffer>*)sDynamicVertexBufferHashTable.End(); ++it)
    {
        PblList<pcRedVertexBuffer>* pList = it;
        for (PblList<pcRedVertexBuffer>::Itor li = pList->Begin();
             (pcRedVertexBuffer*)li != (pcRedVertexBuffer*)pList->End(); ++li)
        {
            li->Flush();
        }
    }

    for (PblList<pcRedVertexBuffer>::Itor li = s_nonSharedVertexBufferList.Begin();
         (pcRedVertexBuffer*)li != (pcRedVertexBuffer*)s_nonSharedVertexBufferList.End(); ++li)
    {
        if (!li->m_static)
        {
            li->Flush();
        }
    }
}

// FUNCTION: SWBF2 0x006caca0
void pcRedVertexBuffer::Reset(ResetFlagsT eFlags)
{
    if (eFlags & RESETFLAGS_FREE_RESOURCES)
    {
        for (PblHashTable<PblList<pcRedVertexBuffer>, 1024>::Itor it = sDynamicVertexBufferHashTable.Begin();
             (PblList<pcRedVertexBuffer>*)it != (PblList<pcRedVertexBuffer>*)sDynamicVertexBufferHashTable.End(); ++it)
        {
            PblList<pcRedVertexBuffer>* pList = it;
            for (PblList<pcRedVertexBuffer>::Itor li = pList->Begin();
                 (pcRedVertexBuffer*)li != (pcRedVertexBuffer*)pList->End(); ++li)
            {
                li->m_buffer->Release();
                li->m_appendPos = 0;
                li->m_numBytesUsed = 0;
            }
        }

        for (PblList<pcRedVertexBuffer>::Itor li = s_nonSharedVertexBufferList.Begin();
             (pcRedVertexBuffer*)li != (pcRedVertexBuffer*)s_nonSharedVertexBufferList.End(); ++li)
        {
            if (!li->m_static)
            {
                li->m_buffer->Release();
                li->m_appendPos = 0;
                li->m_numBytesUsed = 0;
            }
        }
    }

    if (eFlags & RESETFLAGS_CREATE_RESOURCES)
    {
        for (PblHashTable<PblList<pcRedVertexBuffer>, 1024>::Itor it = sDynamicVertexBufferHashTable.Begin();
             (PblList<pcRedVertexBuffer>*)it != (PblList<pcRedVertexBuffer>*)sDynamicVertexBufferHashTable.End(); ++it)
        {
            PblList<pcRedVertexBuffer>* pList = it;
            for (PblList<pcRedVertexBuffer>::Itor li = pList->Begin();
                 (pcRedVertexBuffer*)li != (pcRedVertexBuffer*)pList->End(); ++li)
            {
                RedRenderer::pcGetDevice()->CreateVertexBuffer(
                    li->m_desc.Size, li->m_desc.Usage, li->m_desc.FVF, li->m_desc.Pool,
                    &li->m_buffer, 0);
                li->m_appendPos = 0;
                li->m_numBytesUsed = 0;
            }
        }

        for (PblList<pcRedVertexBuffer>::Itor li = s_nonSharedVertexBufferList.Begin();
             (pcRedVertexBuffer*)li != (pcRedVertexBuffer*)s_nonSharedVertexBufferList.End(); ++li)
        {
            if (!li->m_static)
            {
                RedRenderer::pcGetDevice()->CreateVertexBuffer(
                    li->m_desc.Size, li->m_desc.Usage, li->m_desc.FVF, li->m_desc.Pool,
                    &li->m_buffer, 0);
                li->m_appendPos = 0;
                li->m_numBytesUsed = 0;
            }
        }
    }
}
