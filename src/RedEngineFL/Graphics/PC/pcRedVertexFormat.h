#ifndef SWBF2_GRAPHICS_PC_PCREDVERTEXFORMAT_H
#define SWBF2_GRAPHICS_PC_PCREDVERTEXFORMAT_H

#include <stddef.h>
#include <d3d9.h>
#include <unknwn.h>

#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblRef.h"

// VTABLE: SWBF2 0x007857bc
class pcRedVertexFormat : public PblRef
{
public:

    virtual ~pcRedVertexFormat();

    // SYNTHETIC: SWBF2 0x007142e0
    // pcRedVertexFormat::`scalar deleting destructor'

    unsigned int m_uiVertexBufferFVF;
    unsigned int m_uiVertexInputRVF;
    unsigned int m_uiVertexBufferRVF;
    IDirect3DVertexDeclaration9* m_pVertexDeclaration;
    PblList<pcRedVertexFormat>::Node m_node;

    pcRedVertexFormat();

    static pcRedVertexFormat* Create(unsigned int uiVertexBufferRVF,
                                     unsigned int uiVertexInputRVF);

    static pcRedVertexFormat* Create(unsigned int uiRVF)
    {
        return Create(uiRVF, uiRVF);
    }

    static pcRedVertexFormat* Create(unsigned int uiVertexBufferRVF,
                                     const D3DVERTEXELEMENT9* pDeclaration);

    static IDirect3DVertexDeclaration9* CreateVertexDeclaration(
        unsigned int uiVertexBufferRVF, unsigned int uiVertexInputRVF);

    static unsigned int RVFtoFVF(unsigned int uiRVF);

    static void Cleanup();

    static PblList<pcRedVertexFormat> s_vertexFormatList;
};

 
 

 #endif
