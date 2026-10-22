#include <math.h>
#include <string.h>

#include "RedEngineFL/Interface/RedGlyphCacheElement.h"

#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Interface/RedGlyphMemoryManager.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcInterfaceShader.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

unsigned int RedGlyphCacheElement::s_uiNumQuads;
bool RedGlyphCacheElement::s_bQuadsInUse;
RedGlyphMemoryManager* RedGlyphCacheElement::s_pMemoryManager;

RedGlyphCacheElement::Quad RedGlyphCacheElement::s_aQuads[512];

// FUNCTION: SWBF2 0x006dafc0
RedGlyphCacheElement::RedGlyphCacheElement()
{
    m_pFont = 0;
    m_eBlendMode = BLEND_ALPHA;
    m_eStyle = TEXT_STYLE_NORMAL;
    m_puiNumGlyphs = 0;
    m_ppGlyphData = 0;
    m_ppInterfaceShader = 0;
}

// FUNCTION: SWBF2 0x006db010
RedGlyphCacheElement::~RedGlyphCacheElement()
{
    Cleanup();
}

// FUNCTION: SWBF2 0x006db0a0
void RedGlyphCacheElement::AddGlyph(const RedFont::Glyph& rGlyph,
                                    float fX, float fY, float fScaleX, float fScaleY)
{
    if (!m_puiNumGlyphs)
    {
        return;
    }

    unsigned int uiPage = rGlyph.texnum;
    if (rGlyph.value >= 0x7fff)
    {
        uiPage = uiPage - 128 + m_pFont->GetBaseNumTextures();
    }

    Quad& q = s_aQuads[s_uiNumQuads++];
    m_puiNumGlyphs[uiPage]++;
    q.texnum = (unsigned char)uiPage;

    q.x0 = (float)floor(rGlyph.left * fScaleX + fX + 0.5f);
    q.y0 = (float)floor(rGlyph.top * fScaleY + fY + 0.5f);
    q.x1 = (float)floor(rGlyph.right * fScaleX + fX + 0.5f);
    q.y1 = (float)floor(rGlyph.bottom * fScaleY + fY + 0.5f);

    q.u0 = rGlyph.uLeft;
    q.v0 = rGlyph.vTop;
    q.u1 = rGlyph.uRight;
    q.v1 = rGlyph.vBottom;
}

// FUNCTION: SWBF2 0x006db210
void RedGlyphCacheElement::Begin()
{
    Clear();
    if (m_puiNumGlyphs)
    {
        s_bQuadsInUse = true;
        s_uiNumQuads = 0;
    }
}

// FUNCTION: SWBF2 0x006db240
void RedGlyphCacheElement::Cleanup()
{
    if (m_puiNumGlyphs)
    {
        RedFreeToHeap(__RedCurrHeap, m_puiNumGlyphs, 0);
        m_puiNumGlyphs = 0;
    }

    if (m_ppGlyphData)
    {
        RedFreeToHeap(__RedCurrHeap, m_ppGlyphData, 0);
        m_ppGlyphData = 0;
    }

    if (m_ppInterfaceShader)
    {
        unsigned int uiNumTextures = m_pFont ? m_pFont->GetNumTextures() : 0;
        for (unsigned int i = 0; i < uiNumTextures; i++)
        {
            m_ppInterfaceShader[i]->Release();
            m_ppInterfaceShader[i] = 0;
        }

        RedFreeToHeap(__RedCurrHeap, m_ppInterfaceShader, 0);
        m_ppInterfaceShader = 0;
    }

    m_pFont = 0;
}

// FUNCTION: SWBF2 0x006db330
void RedGlyphCacheElement::Clear()
{
    unsigned int uiNumTextures = m_pFont ? m_pFont->GetNumTextures() : 0;

    for (unsigned int i = 0; i < uiNumTextures; i++)
    {
        m_puiNumGlyphs[i] = 0;

        if (m_ppGlyphData[i])
        {
            s_pMemoryManager->Free(m_ppGlyphData[i]);
            m_ppGlyphData[i] = 0;
        }
    }
}

// FUNCTION: SWBF2 0x006db390
void RedGlyphCacheElement::End()
{
    if (!m_puiNumGlyphs)
    {
        s_bQuadsInUse = false;
        return;
    }

    const unsigned int uiNumTextures = m_pFont->GetNumTextures();
    for (unsigned int i = 0; i < uiNumTextures; i++)
    {
        const unsigned int uiNumGlyphs = m_puiNumGlyphs[i];

        if (m_ppGlyphData[i])
        {
            s_pMemoryManager->Free(m_ppGlyphData[i]);
            m_ppGlyphData[i] = 0;
        }

        if (!uiNumGlyphs)
        {
            continue;
        }

        RedGlyphMemoryManager::Iterator it = s_pMemoryManager->Alloc(uiNumGlyphs);
        m_ppGlyphData[i] = it;
        if (!m_ppGlyphData[i])
        {
            continue;
        }

        const Quad* pQuad = s_aQuads;
        for (unsigned int n = 0; n < uiNumGlyphs; n++, pQuad++)
        {
            while (pQuad->texnum != i)
            {
                pQuad++;
            }

            it->x = pQuad->x0; it->y = pQuad->y0; it->u = pQuad->u0; it->v = pQuad->v0;
            ++it;
            it->x = pQuad->x0; it->y = pQuad->y1; it->u = pQuad->u0; it->v = pQuad->v1;
            ++it;
            it->x = pQuad->x1; it->y = pQuad->y0; it->u = pQuad->u1; it->v = pQuad->v0;
            ++it;
            it->x = pQuad->x0; it->y = pQuad->y1; it->u = pQuad->u0; it->v = pQuad->v1;
            ++it;
            it->x = pQuad->x1; it->y = pQuad->y1; it->u = pQuad->u1; it->v = pQuad->v1;
            ++it;
            it->x = pQuad->x1; it->y = pQuad->y0; it->u = pQuad->u1; it->v = pQuad->v0;
            ++it;
        }
    }

    s_bQuadsInUse = false;
    s_pMemoryManager->TraceMemory();
}

// FUNCTION: SWBF2 0x006db770
void RedGlyphCacheElement::InitPool(unsigned int uiNumVertices)
{
    s_uiNumQuads = 0;
    s_bQuadsInUse = false;

    s_pMemoryManager = new RedGlyphMemoryManager(uiNumVertices);
}

// FUNCTION: SWBF2 0x006db800
void RedGlyphCacheElement::ReallocateCache()
{
    unsigned int uiNumTextures = m_pFont ? m_pFont->GetNumTextures() : 0;

    if (m_puiNumGlyphs)
    {
        RedFreeToHeap(__RedCurrHeap, m_puiNumGlyphs, 0);
        m_puiNumGlyphs = 0;
    }
    if (m_ppGlyphData)
    {
        RedFreeToHeap(__RedCurrHeap, m_ppGlyphData, 0);
        m_ppGlyphData = 0;
    }
    if (m_ppInterfaceShader)
    {
        RedFreeToHeap(__RedCurrHeap, m_ppInterfaceShader, 0);
        m_ppInterfaceShader = 0;
    }

    if (!uiNumTextures)
    {
        return;
    }

    const unsigned int uiBytes = uiNumTextures * 4;
    m_puiNumGlyphs = (unsigned int*)RedAllocFromHeap(__RedCurrHeap, uiBytes, 0);
    memset(m_puiNumGlyphs, 0, uiBytes);
    m_ppGlyphData = (void**)RedAllocFromHeap(__RedCurrHeap, uiBytes, 0);
    memset(m_ppGlyphData, 0, uiBytes);
    m_ppInterfaceShader = (pcInterfaceShader**)RedAllocFromHeap(__RedCurrHeap, uiBytes, 0);

    for (unsigned int i = 0; i < uiNumTextures; i++)
    {
        const RedTexture* pTexture = m_pFont->GetTexture((unsigned char)i);
        unsigned int uiTextureHash = 0;
        if (pTexture)
        {
            uiTextureHash = pTexture->m_uiNameHash;
        }
        m_ppInterfaceShader[i] = (pcInterfaceShader*)pcRedShader::Create(
            PblHash("interface"), 0x202, &uiTextureHash, 1, 0,
            RedColor::WHITE, RedColor::BLACK, 32.0f);
        m_ppInterfaceShader[i]->m_mode = pcInterfaceShader::INTERFACE_MODE_BITMAP;
    }
}

// FUNCTION: SWBF2 0x006db970
void RedGlyphCacheElement::RenderUsingContext(const PblMatrix& mContext,
                                              const RedColor& color)
{
    if (!m_puiNumGlyphs)
    {
        return;
    }

    unsigned int uiNumTextures = m_pFont->GetNumTextures();

    for (unsigned int i = 0; i < uiNumTextures; i++)
    {
        unsigned int uiNumGlyphs = m_puiNumGlyphs[i];
        if (!uiNumGlyphs)
        {
            continue;
        }

        pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLELIST, 0x202, sizeof(GlyphVertexT), uiNumGlyphs * 6, 0,
            false);

        char* pVertexData = (char*)pPrimitive->BeginVertexData();
        if (!pVertexData)
        {
            return;
        }

        for (RedGlyphMemoryManager::Iterator it(m_ppGlyphData[i]); it;
             it.NextBlock())
        {
            memcpy(pVertexData, &*it, it.GetBlockSize() * sizeof(GlyphVertexT));
            pVertexData += 48 * sizeof(GlyphVertexT);
        }

        pPrimitive->EndVertexData();

        if (m_eBlendMode == BLEND_ADDITIVE)
        {
            m_ppInterfaceShader[i]->m_materialFlags |= 0x40;
        }
        else
        {
            m_ppInterfaceShader[i]->m_materialFlags &= ~0x40u;
        }

        m_ppInterfaceShader[i]->m_shadowEnable = (m_eStyle == TEXT_STYLE_SHADOW);

        RedRenderer::pcRenderPrimitive(pPrimitive, m_ppInterfaceShader[i],
                                       mContext, color, 0x200, 0, 0, 0);

        pPrimitive->Release();
    }
}

// FUNCTION: SWBF2 0x006dbae0
void RedGlyphCacheElement::SetFont(unsigned int uiFontHash)
{
    if (uiFontHash)
    {
        SetFont(RedFont::Find(uiFontHash));
    }
    else
    {
        SetFont((const RedFont*)0);
    }
}

// FUNCTION: SWBF2 0x006dbb20
void RedGlyphCacheElement::SetFont(const char* pcName)
{
    if (pcName && *pcName)
    {
        SetFont(RedFont::Find(pcName));
    }
    else
    {
        SetFont((const RedFont*)0);
    }
}

// FUNCTION: SWBF2 0x006dbb60
void RedGlyphCacheElement::SetFont(const RedFont* pFont)
{
    Clear();

    unsigned int uiNumTextures = m_pFont ? m_pFont->GetNumTextures() : 0;
    for (unsigned int i = 0; i < uiNumTextures; i++)
    {
        m_ppInterfaceShader[i]->Release();
        m_ppInterfaceShader[i] = 0;
    }

    m_pFont = const_cast<RedFont*>(pFont);
    ReallocateCache();
}

// FUNCTION: SWBF2 0x006daf70
RedGlyphCacheElement::RedGlyphCacheElement(const char* pcName)
    : RedInterfaceElement(pcName)
{
    m_pFont = 0;
    m_eBlendMode = BLEND_ALPHA;
    m_eStyle = TEXT_STYLE_NORMAL;
    m_puiNumGlyphs = 0;
    m_ppGlyphData = 0;
    m_ppInterfaceShader = 0;
}

// FUNCTION: SWBF2 0x006db320
void RedGlyphCacheElement::CleanupPool()
{
    s_pMemoryManager = 0;
}
