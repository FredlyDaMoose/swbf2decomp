#ifndef SWBF2_GRAPHICS_PC_PCREDPRIMITIVE_H
#define SWBF2_GRAPHICS_PC_PCREDPRIMITIVE_H

#include <stddef.h>
#include <d3d9.h>

#include "PebbleFL/Common/PblRef.h"
#include "PebbleFL/Common/PblVector.h"

class pcRedIndexBuffer;
class pcRedVertexBuffer;
class pcRedVertexFormat;

// VTABLE: SWBF2 0x00771c94
class pcRedPrimitive : public PblRef
{
public:

    virtual ~pcRedPrimitive();

    // SYNTHETIC: SWBF2 0x006ad730
    // pcRedPrimitive::`scalar deleting destructor'

    pcRedPrimitive(D3DPRIMITIVETYPE primType,
                   unsigned int uiVertexFormat,
                   unsigned int uiVertexSize,
                   unsigned int uiNumVertices,
                   unsigned int uiNumIndices,
                   bool bStatic);

    pcRedPrimitive(D3DPRIMITIVETYPE primType,
                   unsigned int uiPrimCount,
                   unsigned int uiVertexFormat,
                   unsigned int uiNumVertices,
                   unsigned int uiStartVertex,
                   pcRedVertexBuffer* pVertexBuffer,
                   unsigned int uiStartIndex,
                   pcRedIndexBuffer* pIndexBuffer);

    static pcRedPrimitive* Create(D3DPRIMITIVETYPE primType,
                                  unsigned int uiVertexFormat,
                                  unsigned int uiVertexSize,
                                  unsigned int uiNumVertices,
                                  unsigned int uiNumIndices,
                                  bool bStatic);

    static pcRedPrimitive* Create(D3DPRIMITIVETYPE primType,
                                  unsigned int uiPrimCount,
                                  unsigned int uiVertexFormat,
                                  unsigned int uiNumVertices,
                                  unsigned int uiStartVertex,
                                  pcRedVertexBuffer* pVertexBuffer,
                                  unsigned int uiStartIndex,
                                  pcRedIndexBuffer* pIndexBuffer);

    void* BeginVertexData();

    void EndVertexData();

    unsigned short* BeginIndexData();
    void EndIndexData();

    void Draw() const;

    static unsigned int GetNumD3DPrimitives(D3DPRIMITIVETYPE primType,
                                            unsigned int uiNumVertices);

    pcRedIndexBuffer* m_indexBuffer;
    pcRedVertexBuffer* m_vertexBuffer;
    D3DPRIMITIVETYPE m_primType;
    unsigned int m_startVertex;
    unsigned int m_startIndex;
    unsigned int m_primCount;
    unsigned int m_numVertices;
    unsigned int m_numIndices;
    pcRedVertexFormat* m_vertexFormat;
    PblBox m_bbox;
};

 
 
 

 #endif
