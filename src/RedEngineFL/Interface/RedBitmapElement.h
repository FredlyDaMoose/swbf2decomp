#pragma once

#include <stddef.h>

#include "RedInterfaceElement.h"

class RedTexture;

class pcInterfaceShader;

// VTABLE: SWBF2 0x007837d8

class RedBitmapElement : public RedInterfaceElement
{
public:

    enum BitmapStyleT
    {
        BITMAP_STYLE_NORMAL = 0,
        BITMAP_STYLE_SHADOW = 1
    };

    enum BitmapHAlignT
    {
        BITMAP_HALIGN_LEFT = 0,
        BITMAP_HALIGN_CENTER = 1,
        BITMAP_HALIGN_RIGHT = 2
    };

    enum BitmapVAlignT
    {
        BITMAP_VALIGN_TOP = 0,
        BITMAP_VALIGN_CENTER = 1,
        BITMAP_VALIGN_BOTTOM = 2
    };

    struct BitmapVertexT
    {
        float x;
        float y;
        float z;
        float u0;
        float v0;
        float u1;
        float v1;
    };

    // FUNCTION: SWBF2 0x006e4830
    RedBitmapElement();

    RedBitmapElement(const char* pcName);

    // SYNTHETIC: SWBF2 0x00578810
    // RedBitmapElement::`vector deleting destructor'

    // FUNCTION: SWBF2 0x006e4c50
    virtual ~RedBitmapElement();

    // FUNCTION: SWBF2 0x0054a530
    virtual void SetBlendMode(BlendModeT eBlendMode) { m_eBlendMode = eBlendMode; }

    // FUNCTION: SWBF2 0x0054a540
    virtual BlendModeT GetBlendMode() const { return m_eBlendMode; }

    // FUNCTION: SWBF2 0x0054a520
    virtual bool IsBitmap() const { return true; }

    virtual void CreateHotSpot(float fRadius);

    // FUNCTION: SWBF2 0x006e48a0
    virtual void Cleanup();

protected:
    virtual void RenderUsingContext(const PblMatrix& mContext, const RedColor& color);

public:

    // FUNCTION: SWBF2 0x006e4960
    virtual void Init();

    virtual void SetRect(float fLeft, float fTop, float fRight, float fBottom);

    virtual void SetRect(float fX, float fY, BitmapHAlignT eHAlign,
                         BitmapVAlignT eVAlign);

    virtual void SetTexture(unsigned int uiTextureHash);

protected:
    // FUNCTION: SWBF2 0x0054a550
    virtual void OnRender() {}

public:

    // FUNCTION: SWBF2 0x006e4b90
    void SetTexCoords(float fU0, float fV0, float fU1, float fV1,
                      bool bSwapDiagonal);

    // FUNCTION: SWBF2 0x006e4930
    const RedTexture* GetTexture() const;

protected:

    void SetRect_(float fLeft, float fTop, float fRight, float fBottom);

public:

    void GetRect(float& rfLeft, float& rfTop, float& rfRight, float& rfBottom) const;
    void GetTexCoords(float& rfU0, float& rfV0, float& rfU1, float& rfV1) const;
    void SetTexture(const RedTexture* const pTexture);

    BitmapStyleT GetStyle() const { return m_eStyle; }
    void SetStyle(BitmapStyleT eStyle) { m_eStyle = eStyle; }

    BitmapStyleT m_eStyle;
    BlendModeT m_eBlendMode;

    float left_;
    float right_;
    float top_;
    float bottom_;

    BitmapVertexT m_vertices[4];
    pcInterfaceShader* m_pInterfaceBitmapShader;
};

 
 
 
 
 
 
 
 
 
 
