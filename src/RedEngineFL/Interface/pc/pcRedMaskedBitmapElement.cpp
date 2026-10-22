#include "RedEngineFL/Interface/RedMaskedBitmapElement.h"

#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcInterfaceShader.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

#include <string.h>

// FUNCTION: SWBF2 0x006f7f30
void RedMaskedBitmapElement::GetMaskTexCoords(float& rf0, float& rf1, float& rf2, float& rf3) const
{
    rf0 = m_vertices[0].u1;
    rf2 = m_vertices[0].v1;
    rf1 = m_vertices[3].u1;
    rf3 = m_vertices[3].v1;
}

// FUNCTION: SWBF2 0x006f7f70
const RedTexture* RedMaskedBitmapElement::GetMaskTexture() const
{
    if (!m_pInterfaceMaskedBitmapShader)
    {
        return 0;
    }
    return RedTexture::Find(m_pInterfaceMaskedBitmapShader->m_bitmapMaskHash);
}

// FUNCTION: SWBF2 0x006f7e00
RedMaskedBitmapElement::RedMaskedBitmapElement()
{
    m_pInterfaceMaskedBitmapShader = 0;
    SetMaskTexCoords(0.0f, 1.0f, 0.0f, 1.0f);
}

// FUNCTION: SWBF2 0x006f7e50
RedMaskedBitmapElement::~RedMaskedBitmapElement()
{
    Cleanup();
}

// FUNCTION: SWBF2 0x006f7ee0
void RedMaskedBitmapElement::Cleanup()
{
    RedBitmapElement::Cleanup();
    if (m_pInterfaceMaskedBitmapShader)
    {
        m_pInterfaceMaskedBitmapShader->Release();
        m_pInterfaceMaskedBitmapShader = 0;
    }
}

// FUNCTION: SWBF2 0x006f7fa0
void RedMaskedBitmapElement::Init()
{
    RedBitmapElement::Init();
    m_pInterfaceMaskedBitmapShader = (pcInterfaceShader*)pcRedShader::Create(
        PblHash("interface"), 0x402, 0, 0, 0, RedColor::WHITE, RedColor::BLACK,
        32.0f);

    m_pInterfaceMaskedBitmapShader->m_mode = pcInterfaceShader::INTERFACE_MODE_MASKEDBITMAP;
}

// FUNCTION: SWBF2 0x006f8000
void RedMaskedBitmapElement::RenderUsingContext(const PblMatrix& mContext,
                                                const RedColor& color)
{
    if (!m_pInterfaceMaskedBitmapShader ||
        !m_pInterfaceMaskedBitmapShader->m_bitmapMaskHash)
    {
        RedBitmapElement::RenderUsingContext(mContext, color);
        return;
    }

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
        D3DPT_TRIANGLESTRIP, 0x402, sizeof(BitmapVertexT), 4, 0, false);

    void* pVertexData = pPrimitive->BeginVertexData();
    if (!pVertexData)
    {
        return;
    }

    memcpy(pVertexData, m_vertices, sizeof(m_vertices));

    pPrimitive->EndVertexData();

    if (m_eBlendMode == BLEND_ADDITIVE)
    {
        m_pInterfaceMaskedBitmapShader->m_materialFlags |= 0x40;
    }
    else
    {
        m_pInterfaceMaskedBitmapShader->m_materialFlags &= ~0x40u;
    }

    m_pInterfaceMaskedBitmapShader->m_shadowEnable = (m_eStyle == BITMAP_STYLE_SHADOW);

    RedRenderer::pcRenderPrimitive(pPrimitive, m_pInterfaceMaskedBitmapShader,
                                   mContext, color, 0x200, 0, 0, 0);

    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x006f80d0
void RedMaskedBitmapElement::SetMaskTexCoords(float fU0, float fU1, float fV0, float fV1)
{
    m_vertices[0].u1 = fU0;
    m_vertices[0].v1 = fV0;
    m_vertices[1].u1 = fU0;
    m_vertices[1].v1 = fV1;
    m_vertices[2].u1 = fU1;
    m_vertices[2].v1 = fV0;
    m_vertices[3].u1 = fU1;
    m_vertices[3].v1 = fV1;
}

// FUNCTION: SWBF2 0x006f8130
void RedMaskedBitmapElement::SetMaskTexture(unsigned int uHash)
{
    if (!m_pInterfaceMaskedBitmapShader)
    {
        Init();
    }
    m_pInterfaceMaskedBitmapShader->SetBitmapMask(uHash);
}

// FUNCTION: SWBF2 0x006f8160
void RedMaskedBitmapElement::SetTexture(unsigned int uiTextureHash)
{
    RedBitmapElement::SetTexture(uiTextureHash);
    if (!m_pInterfaceMaskedBitmapShader)
    {
        Init();
    }
    m_pInterfaceMaskedBitmapShader->SetTexture(uiTextureHash);
}
