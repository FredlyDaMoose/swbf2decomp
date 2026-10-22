#include <string.h>

#include "pcRedVertexFormat.h"

#include "RedEngineFL/Memory/engineAlloc.h"

#include "pcRedRenderer.h"

// FUNCTION: SWBF2 0x00714220
pcRedVertexFormat::pcRedVertexFormat()
{

    m_node._pList = 0;
    m_node._pNext = 0;
    m_node._pPrev = 0;
    m_node._pObject = 0;

    m_uiVertexBufferFVF = 0;
    m_uiVertexInputRVF = 0;
    m_uiVertexBufferRVF = 0;
    m_pVertexDeclaration = 0;
}

// FUNCTION: SWBF2 0x00714270
pcRedVertexFormat::~pcRedVertexFormat()
{
    if (m_pVertexDeclaration)
    {
        m_pVertexDeclaration->Release();
    }
}

// GLOBAL: SWBF2 0x007e22bc
PblList<pcRedVertexFormat> pcRedVertexFormat::s_vertexFormatList;

// GLOBAL: SWBF2 0x007857c4

extern const unsigned char kVertexDeclarationTypes[5];
const unsigned char kVertexDeclarationTypes[5] = {
    D3DDECLTYPE_UNUSED, D3DDECLTYPE_FLOAT1, D3DDECLTYPE_FLOAT2,
    D3DDECLTYPE_FLOAT3, D3DDECLTYPE_FLOAT4
};

// FUNCTION: SWBF2 0x00714420
pcRedVertexFormat* pcRedVertexFormat::Create(unsigned int uiVertexBufferRVF,
                                             unsigned int uiVertexInputRVF)
{
    if (uiVertexBufferRVF == 0)
    {
        return 0;
    }

    const unsigned int uiInput = uiVertexInputRVF & 0x7FF;

    PblList<pcRedVertexFormat>::Node* pNode = s_vertexFormatList.GetFirst();
    while (pNode->_pObject != s_vertexFormatList._head._pObject)
    {
        pcRedVertexFormat* pFormat = pNode->_pObject;
        if (pFormat->m_uiVertexBufferRVF > uiVertexBufferRVF)
        {
            break;
        }
        if (pFormat->m_uiVertexBufferRVF == uiVertexBufferRVF)
        {
            if (pFormat->m_uiVertexInputRVF > uiInput)
            {
                break;
            }
            if (pFormat->m_uiVertexInputRVF == uiInput)
            {
                return pNode->_pObject;
            }
        }
        pNode = pNode->_pNext;
    }

    pcRedVertexFormat* pFormat = new pcRedVertexFormat;

    pFormat->m_uiVertexBufferRVF = uiVertexBufferRVF;
    pFormat->m_uiVertexInputRVF = uiInput;
    pFormat->m_pVertexDeclaration =
        CreateVertexDeclaration(uiVertexBufferRVF, uiInput);
    pFormat->m_uiVertexBufferFVF = RVFtoFVF(uiVertexBufferRVF);

    s_vertexFormatList.AddBefore(pNode, &pFormat->m_node, pFormat);

    return pFormat;
}

// FUNCTION: SWBF2 0x007144e0
pcRedVertexFormat* pcRedVertexFormat::Create(unsigned int uiVertexBufferRVF,
                                             const D3DVERTEXELEMENT9* pDeclaration)
{
    pcRedVertexFormat* pFormat = new pcRedVertexFormat;

    pFormat->m_uiVertexBufferRVF = uiVertexBufferRVF;
    pFormat->m_uiVertexInputRVF = 0x800;

    IDirect3DVertexDeclaration9* pVertexDeclaration;
    RedRenderer::pcGetDevice()->CreateVertexDeclaration(pDeclaration, &pVertexDeclaration);
    pFormat->m_pVertexDeclaration = pVertexDeclaration;
    pFormat->m_uiVertexBufferFVF = RVFtoFVF(uiVertexBufferRVF);

    s_vertexFormatList.AddLast(&pFormat->m_node, pFormat);

    return pFormat;
}

// FUNCTION: SWBF2 0x00714570
IDirect3DVertexDeclaration9* pcRedVertexFormat::CreateVertexDeclaration(
    unsigned int uiVertexBufferRVF, unsigned int uiVertexInputRVF)
{
    D3DVERTEXELEMENT9 aElements[16];
    memset(aElements, 0, sizeof(aElements));
    for (int i = 0; i < 16; i++)
    {
        aElements[i].Type = D3DDECLTYPE_UNUSED;
    }

    if (uiVertexBufferRVF == 0 || uiVertexInputRVF == 0)
    {
        return 0;
    }

    unsigned short usOffset = 0;
    unsigned int uiCount = 0;

    if (uiVertexBufferRVF & 3)
    {
        const unsigned short usBufComponents =
            (unsigned short)((uiVertexBufferRVF & 3) + 1);
        const unsigned short usInComponents =
            (unsigned short)((uiVertexInputRVF & 3) + 1);

        if (uiVertexBufferRVF & 0x1000)
        {
            if (usInComponents > 2)
            {
                aElements[0].Type = D3DDECLTYPE_SHORT4;
                aElements[0].Offset = 0;
                aElements[0].Usage = D3DDECLUSAGE_POSITION;
                aElements[0].UsageIndex = 0;
                uiCount = 1;
                usOffset = 8;
            }
            else if (usInComponents != 0)
            {
                aElements[0].Type = D3DDECLTYPE_SHORT2;
                aElements[0].Offset = 0;
                aElements[0].Usage = D3DDECLUSAGE_POSITION;
                aElements[0].UsageIndex = 0;
                uiCount = 1;
                usOffset = 4;
            }
        }
        else
        {
            if (usInComponents > 1)
            {
                aElements[0].Type = kVertexDeclarationTypes[usInComponents];
                aElements[0].Offset = 0;

                aElements[0].UsageIndex = 0;
                aElements[0].Usage = (usInComponents == 4)
                                         ? D3DDECLUSAGE_POSITIONT
                                         : D3DDECLUSAGE_POSITION;
                uiCount = 1;
            }
            usOffset = (unsigned short)(usBufComponents * 4);
        }
    }

    if (uiVertexBufferRVF & 0x1C)
    {
        const unsigned int uiBufWeights = (uiVertexBufferRVF >> 2) & 7;
        unsigned int uiInWeights = (uiVertexInputRVF >> 2) & 7;
        if ((unsigned short)uiInWeights > (unsigned short)uiBufWeights)
        {
            uiInWeights = uiBufWeights;
        }

        if (uiBufWeights > 1)
        {
            if (uiVertexBufferRVF & 0x2000)
            {
                if ((unsigned short)uiInWeights > 1)
                {
                    aElements[uiCount].Type = D3DDECLTYPE_D3DCOLOR;
                    aElements[uiCount].Offset = usOffset;
                    aElements[uiCount].Usage = D3DDECLUSAGE_BLENDWEIGHT;
                    aElements[uiCount].UsageIndex = 0;
                    ++uiCount;
                }
                usOffset = (unsigned short)(usOffset + 4);
            }
            else
            {
                if ((unsigned short)uiInWeights > 1)
                {
                    aElements[uiCount].Offset = usOffset;
                    aElements[uiCount].Usage = D3DDECLUSAGE_BLENDWEIGHT;
                    aElements[uiCount].UsageIndex = 0;

                    aElements[uiCount].Type = kVertexDeclarationTypes[uiInWeights - 1];
                    ++uiCount;
                }
                usOffset = (unsigned short)(usOffset + uiBufWeights * 4 - 4);
            }
        }

        if ((unsigned short)uiInWeights != 0)
        {
            aElements[uiCount].Type = D3DDECLTYPE_D3DCOLOR;
            aElements[uiCount].Offset = usOffset;
            aElements[uiCount].Usage = D3DDECLUSAGE_BLENDINDICES;
            aElements[uiCount].UsageIndex = 0;
            ++uiCount;
        }
        usOffset = (unsigned short)(usOffset + 4);
    }

    if (uiVertexBufferRVF & 0x20)
    {
        if (uiVertexBufferRVF & 0x4000)
        {
            if (uiVertexInputRVF & 0x20)
            {
                aElements[uiCount].Type = D3DDECLTYPE_D3DCOLOR;
                aElements[uiCount].Offset = usOffset;
                aElements[uiCount].Usage = D3DDECLUSAGE_NORMAL;
                aElements[uiCount].UsageIndex = 0;
                ++uiCount;
            }
            usOffset = (unsigned short)(usOffset + 4);
        }
        else
        {
            if (uiVertexInputRVF & 0x20)
            {
                aElements[uiCount].Type = D3DDECLTYPE_FLOAT3;
                aElements[uiCount].Offset = usOffset;
                aElements[uiCount].Usage = D3DDECLUSAGE_NORMAL;
                aElements[uiCount].UsageIndex = 0;
                ++uiCount;
            }
            usOffset = (unsigned short)(usOffset + 12);
        }
    }

    if (uiVertexBufferRVF & 0x40)
    {
        if (uiVertexBufferRVF & 0x4000)
        {
            if (uiVertexInputRVF & 0x40)
            {
                aElements[uiCount].Type = D3DDECLTYPE_D3DCOLOR;
                aElements[uiCount].Offset = usOffset;
                aElements[uiCount].Usage = D3DDECLUSAGE_TANGENT;
                aElements[uiCount].UsageIndex = 0;
                aElements[uiCount + 1].Offset = (unsigned short)(usOffset + 4);
                aElements[uiCount + 1].Type = D3DDECLTYPE_D3DCOLOR;
                aElements[uiCount + 1].Usage = D3DDECLUSAGE_BINORMAL;
                aElements[uiCount + 1].UsageIndex = 0;
                uiCount += 2;
            }
            usOffset = (unsigned short)(usOffset + 8);
        }
        else
        {
            if (uiVertexInputRVF & 0x40)
            {
                aElements[uiCount].Type = D3DDECLTYPE_FLOAT3;
                aElements[uiCount].Offset = usOffset;
                aElements[uiCount].Usage = D3DDECLUSAGE_TANGENT;
                aElements[uiCount].UsageIndex = 0;
                aElements[uiCount + 1].Offset = (unsigned short)(usOffset + 12);
                aElements[uiCount + 1].Type = D3DDECLTYPE_FLOAT3;
                aElements[uiCount + 1].Usage = D3DDECLUSAGE_BINORMAL;
                aElements[uiCount + 1].UsageIndex = 0;
                uiCount += 2;
            }
            usOffset = (unsigned short)(usOffset + 24);
        }
    }

    unsigned char ucColourIndex = 0;
    if (uiVertexBufferRVF & 0x80)
    {
        if (uiVertexInputRVF & 0x80)
        {
            aElements[uiCount].Type = D3DDECLTYPE_D3DCOLOR;
            ucColourIndex = 1;
            aElements[uiCount].Offset = usOffset;
            aElements[uiCount].Usage = D3DDECLUSAGE_COLOR;
            aElements[uiCount].UsageIndex = 0;
            ++uiCount;
        }
        usOffset = (unsigned short)(usOffset + 4);
    }
    if (uiVertexBufferRVF & 0x100)
    {
        if (uiVertexInputRVF & 0x100)
        {
            aElements[uiCount].Type = D3DDECLTYPE_D3DCOLOR;
            aElements[uiCount].Offset = usOffset;
            aElements[uiCount].Usage = D3DDECLUSAGE_COLOR;
            aElements[uiCount].UsageIndex = ucColourIndex;
            ++uiCount;
        }
        usOffset = (unsigned short)(usOffset + 4);
    }

    if (uiVertexBufferRVF & 0x600)
    {
        const unsigned char ucIn = (unsigned char)((uiVertexInputRVF >> 9) & 3);
        const unsigned char ucBuf = (unsigned char)((uiVertexBufferRVF >> 9) & 3);
        const unsigned char ucNum = (ucIn > ucBuf) ? ucBuf : ucIn;

        if (uiVertexBufferRVF & 0x8000)
        {
            for (unsigned char uc = 0; uc < ucNum; uc++)
            {
                aElements[uiCount].UsageIndex = uc;
                aElements[uiCount].Offset = usOffset;
                usOffset = (unsigned short)(usOffset + 4);
                aElements[uiCount].Type = D3DDECLTYPE_SHORT2;
                aElements[uiCount].Usage = D3DDECLUSAGE_TEXCOORD;
                ++uiCount;
            }
        }
        else
        {
            for (unsigned char uc = 0; uc < ucNum; uc++)
            {
                aElements[uiCount].UsageIndex = uc;
                aElements[uiCount].Offset = usOffset;
                usOffset = (unsigned short)(usOffset + 8);
                aElements[uiCount].Type = D3DDECLTYPE_FLOAT2;
                aElements[uiCount].Usage = D3DDECLUSAGE_TEXCOORD;
                ++uiCount;
            }
        }
    }

    aElements[uiCount].Stream = 0xFF;
    aElements[uiCount].Offset = 0;
    aElements[uiCount].Type = D3DDECLTYPE_UNUSED;
    aElements[uiCount].Method = 0;
    aElements[uiCount].Usage = 0;
    aElements[uiCount].UsageIndex = 0;

    IDirect3DVertexDeclaration9* pDeclaration = 0;
    RedRenderer::pcGetDevice()->CreateVertexDeclaration(aElements, &pDeclaration);
    return pDeclaration;
}

// FUNCTION: SWBF2 0x00714910
unsigned int pcRedVertexFormat::RVFtoFVF(unsigned int uiRVF)
{
    if (uiRVF & 0x800)
    {
        return 0;
    }
    if (uiRVF & 0x40)
    {
        return 0;
    }
    if (uiRVF & 0xF000)
    {
        return 0;
    }

    unsigned int uiFVF = 0;

    const unsigned int uiPosition = uiRVF & 3;
    if (uiPosition == 1)
    {
        return 0;
    }
    if (uiPosition == 2)
    {
        uiFVF = D3DFVF_XYZ;
    }
    else if (uiPosition == 3)
    {
        uiFVF = D3DFVF_XYZRHW;
    }

    if (uiRVF & 4)
    {
        uiFVF |= D3DFVF_XYZB1;
    }
    else if (uiRVF & 8)
    {
        uiFVF |= D3DFVF_XYZB2;
    }
    else if (uiRVF & 0x0C)
    {
        uiFVF |= D3DFVF_XYZB3;
    }
    else if (uiRVF & 0x10)
    {
        uiFVF |= D3DFVF_XYZB4;
    }

    if (uiRVF & 0x1C)
    {
        uiFVF |= D3DFVF_LASTBETA_UBYTE4;
    }
    if (uiRVF & 0x20)
    {
        uiFVF |= D3DFVF_NORMAL;
    }
    if (uiRVF & 0x180)
    {
        uiFVF |= D3DFVF_DIFFUSE;
    }
    if ((uiRVF & 0x80) && (uiRVF & 0x100))
    {
        uiFVF |= D3DFVF_SPECULAR;
    }

    if (uiRVF & 0x200)
    {
        return uiFVF | D3DFVF_TEX1;
    }
    if (uiRVF & 0x400)
    {
        uiFVF |= D3DFVF_TEX2;
    }

    return uiFVF;
}

// FUNCTION: SWBF2 0x00714310
void pcRedVertexFormat::Cleanup()
{
    while (s_vertexFormatList.GetCount())
    {
        PblList<pcRedVertexFormat>::Node* pNode = s_vertexFormatList.GetFirst();
        pcRedVertexFormat* pFormat = pNode->_pObject;
        s_vertexFormatList.Remove(pNode);
        while (pFormat->Release())
            ;
    }
    s_vertexFormatList.RemoveAll();
}
