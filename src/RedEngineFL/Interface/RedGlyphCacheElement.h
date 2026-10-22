#pragma once

#include <stddef.h>

#include "RedInterfaceElement.h"

#include "RedFont.h"

class pcInterfaceShader;
class RedGlyphMemoryManager;

// VTABLE: SWBF2 0x00783348

class RedGlyphCacheElement : public RedInterfaceElement
{
public:

    enum TextStyleT
    {
        TEXT_STYLE_NORMAL = 0,
        TEXT_STYLE_SHADOW = 1,
        TEXT_STYLE_SELECTED = 2
    };

    // FUNCTION: SWBF2 0x006dafc0
    RedGlyphCacheElement();
    RedGlyphCacheElement(const char* pcName);

    // SYNTHETIC: SWBF2 0x006db070
    // RedGlyphCacheElement::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x006db010
    virtual ~RedGlyphCacheElement();

    // FUNCTION: SWBF2 0x00525f10
    virtual void SetBlendMode(BlendModeT eBlendMode) { m_eBlendMode = eBlendMode; }

    // FUNCTION: SWBF2 0x00525f20
    virtual BlendModeT GetBlendMode() const { return m_eBlendMode; }

    // FUNCTION: SWBF2 0x006db240
    virtual void Cleanup();

protected:
    // FUNCTION: SWBF2 0x006db970
    virtual void RenderUsingContext(const PblMatrix& mContext, const RedColor& color);

public:

    // FUNCTION: SWBF2 0x006db770
    static void InitPool(unsigned int uiNumVertices);

    void AddGlyph(const RedFont::Glyph& rGlyph,
                  float fX, float fY, float fScaleX, float fScaleY);

    void Begin();

    // FUNCTION: SWBF2 0x006db330
    void Clear();

    // FUNCTION: SWBF2 0x006dbb20
    void SetFont(const char* pcName);
    // FUNCTION: SWBF2 0x006dbb60
    void SetFont(const RedFont* pFont);

    void SetFont(unsigned int uiFontHash);

    void SetTextStyle(TextStyleT eStyle) { m_eStyle = eStyle; }

    const RedFont* GetFont() const { return m_pFont; }
private:
    // FUNCTION: SWBF2 0x006db800
    void ReallocateCache();
public:

    void End();

    static unsigned int s_uiNumQuads;
    static bool s_bQuadsInUse;
    static RedGlyphMemoryManager* s_pMemoryManager;
    static void CleanupPool();

    struct Quad
    {
        float x0;
        float y0;
        float u0;
        float v0;
        float x1;
        float y1;
        float u1;
        float v1;
        unsigned char texnum;
    };

    static Quad s_aQuads[512];

    RedFont* m_pFont;
    BlendModeT m_eBlendMode;
    TextStyleT m_eStyle;
    unsigned int* m_puiNumGlyphs;
    void** m_ppGlyphData;
    pcInterfaceShader** m_ppInterfaceShader;
};

 
 
 
 
 
 
 
 
 
