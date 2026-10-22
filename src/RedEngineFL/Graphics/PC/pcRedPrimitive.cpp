#include "pcRedPrimitive.h"

#include "RedEngineFL/Memory/engineAlloc.h"

#include "pcRedIndexBuffer.h"
#include "pcRedVertexBuffer.h"
#include "pcRedVertexFormat.h"

#include "StateManager_DX.h"
#include "pcRedRenderer.h"
#include "RedEngineFL/Util/ext/Globals.h"

// FUNCTION: SWBF2 0x006ad3e0
pcRedPrimitive::pcRedPrimitive(D3DPRIMITIVETYPE primType,
                               unsigned int uiPrimCount,
                               unsigned int uiVertexFormat,
                               unsigned int uiNumVertices,
                               unsigned int uiStartVertex,
                               pcRedVertexBuffer* pVertexBuffer,
                               unsigned int uiStartIndex,
                               pcRedIndexBuffer* pIndexBuffer)
{

    m_primType = primType;
    m_primCount = uiPrimCount;
    m_numVertices = uiNumVertices;
    m_startVertex = uiStartVertex;
    m_vertexBuffer = pVertexBuffer;
    m_startIndex = uiStartIndex;
    m_indexBuffer = pIndexBuffer;
    m_numIndices = 0;

    m_vertexFormat = pcRedVertexFormat::Create(uiVertexFormat, uiVertexFormat);

    m_vertexBuffer->AddRef();
    if (m_indexBuffer)
    {
        m_indexBuffer->AddRef();
    }
}

// FUNCTION: SWBF2 0x006ad4b0
pcRedPrimitive::pcRedPrimitive(D3DPRIMITIVETYPE primType,
                               unsigned int uiVertexFormat,
                               unsigned int uiVertexSize,
                               unsigned int uiNumVertices,
                               unsigned int uiNumIndices,
                               bool bStatic)
{

    m_numVertices = uiNumVertices;
    m_primType = primType;
    m_numIndices = uiNumIndices;

    m_vertexFormat = pcRedVertexFormat::Create(uiVertexFormat, uiVertexFormat);

    m_vertexBuffer = pcRedVertexBuffer::Create(uiNumVertices, uiVertexSize,
                                               bStatic, true);
    m_startVertex = m_vertexBuffer->m_appendPos;

    if (uiNumIndices == 0)
    {
        m_primCount = GetNumD3DPrimitives(primType, uiNumVertices);
        m_indexBuffer = 0;
    }
    else
    {
        m_primCount = GetNumD3DPrimitives(primType, uiNumIndices);
        m_indexBuffer = pcRedIndexBuffer::Create(uiNumIndices, bStatic, true);
        m_startIndex = m_indexBuffer->m_appendPos;
    }
}

// FUNCTION: SWBF2 0x006ad670
pcRedPrimitive::~pcRedPrimitive()
{

    if (PblRef* p = m_vertexBuffer)
    {
        p->Release();
    }
    if (PblRef* p = m_indexBuffer)
    {
        p->Release();
    }
}

// FUNCTION: SWBF2 0x006ad7c0
void* pcRedPrimitive::BeginVertexData()
{
    return m_vertexBuffer->Lock(m_numVertices, m_startVertex);
}

// FUNCTION: SWBF2 0x006ad7d0
pcRedPrimitive* pcRedPrimitive::Create(D3DPRIMITIVETYPE primType,
                                       unsigned int uiPrimCount,
                                       unsigned int uiVertexFormat,
                                       unsigned int uiNumVertices,
                                       unsigned int uiStartVertex,
                                       pcRedVertexBuffer* pVertexBuffer,
                                       unsigned int uiStartIndex,
                                       pcRedIndexBuffer* pIndexBuffer)
{
    return new pcRedPrimitive(primType, uiPrimCount, uiVertexFormat,
                              uiNumVertices, uiStartVertex, pVertexBuffer,
                              uiStartIndex, pIndexBuffer);
}

// FUNCTION: SWBF2 0x006ad850
pcRedPrimitive* pcRedPrimitive::Create(D3DPRIMITIVETYPE primType,
                                       unsigned int uiVertexFormat,
                                       unsigned int uiVertexSize,
                                       unsigned int uiNumVertices,
                                       unsigned int uiNumIndices,
                                       bool bStatic)
{
    return new pcRedPrimitive(primType, uiVertexFormat, uiVertexSize,
                              uiNumVertices, uiNumIndices, bStatic);
}

// FUNCTION: SWBF2 0x006ad8d0
void pcRedPrimitive::Draw() const
{
    util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >& perFrame =
        util::Globals::GetInstance().GetSet("perFrame");

    RedRenderer::pcSetStreamSource(0, m_vertexBuffer);
    if (m_indexBuffer)
    {
        RedRenderer::pcSetIndices(m_indexBuffer);
        dx::StateManager_DX::GetInstance().DrawIndexedPrimitive(m_primType, m_startVertex, 0,
                                                                m_numVertices, m_startIndex,
                                                                m_primCount);
        static int& s_numDrawIP = perFrame.GetInt("numDrawIP");
        ++s_numDrawIP;
    }
    else if (m_primCount)
    {
        dx::StateManager_DX::GetInstance().DrawPrimitive(m_primType, m_startVertex, m_primCount);
        static int& s_numDrawP = perFrame.GetInt("numDrawP");
        ++s_numDrawP;
    }

    static int& s_numVerts = perFrame.GetInt("numVerts");
    s_numVerts += m_numVertices;
    static int& s_numPrims = perFrame.GetInt("numPrims");
    s_numPrims += m_primCount;
}

// FUNCTION: SWBF2 0x006adaf0
void pcRedPrimitive::EndVertexData()
{
    m_vertexBuffer->Unlock();
}

// FUNCTION: SWBF2 0x006aea70
unsigned int pcRedPrimitive::GetNumD3DPrimitives(D3DPRIMITIVETYPE primType,
                                                 unsigned int uiNumVertices)
{
    switch (primType)
    {
    case D3DPT_POINTLIST:
        return uiNumVertices;
    case D3DPT_LINELIST:
        return uiNumVertices / 2;
    case D3DPT_LINESTRIP:
        return uiNumVertices - 1;
    case D3DPT_TRIANGLELIST:
        return uiNumVertices / 3;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN:
        return uiNumVertices - 2;
    default:
        return 0;
    }
}

// FUNCTION: SWBF2 0x006ad7b0
unsigned short* pcRedPrimitive::BeginIndexData()
{
    return m_indexBuffer->Lock(m_numIndices, m_startIndex);
}

// FUNCTION: SWBF2 0x006adae0
void pcRedPrimitive::EndIndexData()
{
    m_indexBuffer->Unlock();
}
