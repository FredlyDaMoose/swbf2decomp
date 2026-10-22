#include "RedEngineFL/Interface/RedBitmapElement.h"

#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcInterfaceShader.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

#include <math.h>
#include <string.h>

// FUNCTION: SWBF2 0x006e4830
RedBitmapElement::RedBitmapElement()
{
    m_eStyle = BITMAP_STYLE_NORMAL;
    m_eBlendMode = BLEND_ALPHA;
    m_pInterfaceBitmapShader = 0;

    memset(m_vertices, 0, sizeof(m_vertices));

    SetTexCoords(0.0f, 0.0f, 1.0f, 1.0f, false);
}

// FUNCTION: SWBF2 0x006e48a0
void RedBitmapElement::Cleanup()
{
    if (m_pInterfaceBitmapShader)
    {
        m_pInterfaceBitmapShader->Release();
        m_pInterfaceBitmapShader = 0;
    }
}

// FUNCTION: SWBF2 0x006e48f0
void RedBitmapElement::GetTexCoords(float& rfU0, float& rfV0, float& rfU1, float& rfV1) const
{
    rfU0 = m_vertices[0].u0;
    rfV0 = m_vertices[0].v0;
    rfU1 = m_vertices[3].u0;
    rfV1 = m_vertices[3].v0;
}

// FUNCTION: SWBF2 0x006e4930
const RedTexture* RedBitmapElement::GetTexture() const
{
    if (m_pInterfaceBitmapShader)
    {
        return RedTexture::Find(m_pInterfaceBitmapShader->GetTexture());
    }
    return 0;
}

// FUNCTION: SWBF2 0x006e4960
void RedBitmapElement::Init()
{
    m_pInterfaceBitmapShader = (pcInterfaceShader*)pcRedShader::Create(
        PblHash("interface"), 0x202, 0, 0, 0, RedColor::WHITE, RedColor::BLACK,
        32.0f);

    m_pInterfaceBitmapShader->m_mode = pcInterfaceShader::INTERFACE_MODE_BITMAP;
}

// FUNCTION: SWBF2 0x006e49b0
void RedBitmapElement::RenderUsingContext(const PblMatrix& mContext,
                                          const RedColor& color)
{
    if (!m_pInterfaceBitmapShader)
    {
        return;
    }

    OnRender();

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
        D3DPT_TRIANGLESTRIP, 0x202, sizeof(BitmapVertexT), 4, 0, false);

    void* pVertexData = pPrimitive->BeginVertexData();
    if (!pVertexData)
    {
        return;
    }

    memcpy(pVertexData, m_vertices, sizeof(m_vertices));

    pPrimitive->EndVertexData();

    if (m_eBlendMode == BLEND_ADDITIVE)
    {
        m_pInterfaceBitmapShader->m_materialFlags |= 0x40;
    }
    else
    {
        m_pInterfaceBitmapShader->m_materialFlags &= ~0x40u;
    }

    m_pInterfaceBitmapShader->m_shadowEnable = (m_eStyle == BITMAP_STYLE_SHADOW);

    RedRenderer::pcRenderPrimitive(pPrimitive, m_pInterfaceBitmapShader,
                                   mContext, color, 0x200, 0, 0, 0);

    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x006e4a90
void RedBitmapElement::SetRect_(float fLeft, float fTop, float fRight, float fBottom)
{
    fLeft = (float)floor(fLeft + 0.5f);
    fTop = (float)floor(fTop + 0.5f);
    fRight = (float)floor(fRight + 0.5f);
    fBottom = (float)floor(fBottom + 0.5f);
    m_vertices[0].x = fLeft;
    m_vertices[1].x = fLeft;
    m_vertices[0].y = fTop;
    m_vertices[1].y = fBottom;
    m_vertices[3].x = fRight;
    m_vertices[3].y = fBottom;
    m_vertices[2].x = fRight;
    m_vertices[2].y = fTop;
}

// FUNCTION: SWBF2 0x006e4b90
void RedBitmapElement::SetTexCoords(float fU0, float fV0, float fU1, float fV1,
                                    bool bSwapDiagonal)
{
    m_vertices[0].u0 = bSwapDiagonal ? fU1 : fU0;
    m_vertices[0].v0 = bSwapDiagonal ? fV1 : fV0;

    m_vertices[1].u0 = fU0;
    m_vertices[1].v0 = fV1;

    m_vertices[2].u0 = fU1;
    m_vertices[2].v0 = fV0;

    m_vertices[3].u0 = bSwapDiagonal ? fU0 : fU1;
    m_vertices[3].v0 = bSwapDiagonal ? fV0 : fV1;
}

// FUNCTION: SWBF2 0x006e4c20
void RedBitmapElement::SetTexture(unsigned int uiTextureHash)
{
    if (!m_pInterfaceBitmapShader)
    {
        Init();
    }
    m_pInterfaceBitmapShader->SetTexture(uiTextureHash);
}

// FUNCTION: SWBF2 0x006e47b0
RedBitmapElement::RedBitmapElement(const char* pcName)
    : RedInterfaceElement(pcName)
{
    m_eStyle = BITMAP_STYLE_NORMAL;
    m_eBlendMode = BLEND_ALPHA;
    m_pInterfaceBitmapShader = 0;

    memset(m_vertices, 0, sizeof(m_vertices));

    SetTexCoords(0.0f, 0.0f, 1.0f, 1.0f, false);
}
