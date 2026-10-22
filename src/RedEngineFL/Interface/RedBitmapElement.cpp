#include "RedBitmapElement.h"

#include "RedEngineFL/Graphics/PC/pcRedTexture.h"

// FUNCTION: SWBF2 0x006e4c50
RedBitmapElement::~RedBitmapElement()
{
    Cleanup();
}

// FUNCTION: SWBF2 0x006e4db0
void RedBitmapElement::GetRect(float& rfLeft, float& rfTop, float& rfRight, float& rfBottom) const
{
    rfLeft = left_;
    rfTop = top_;
    rfRight = right_;
    rfBottom = bottom_;
}

// FUNCTION: SWBF2 0x006e4de0
void RedBitmapElement::SetRect(float fLeft, float fTop, float fRight, float fBottom)
{
    left_ = fLeft;
    right_ = fRight;
    top_ = fTop;
    bottom_ = fBottom;
    SetRect_(fLeft, fTop, fRight, fBottom);
}

// FUNCTION: SWBF2 0x006e4e40
void RedBitmapElement::SetRect(float fWidth, float fHeight, BitmapHAlignT eHAlign,
                               BitmapVAlignT eVAlign)
{
    float fLeft = 0.0f, fRight = 0.0f, fTop = 0.0f, fBottom = 0.0f;
    switch (eHAlign)
    {
    case BITMAP_HALIGN_LEFT:
        fRight = fWidth;
        fLeft = 0.0f;
        break;
    case BITMAP_HALIGN_CENTER:
        fLeft = fWidth * -0.5f;
        fRight = -fLeft;
        break;
    case BITMAP_HALIGN_RIGHT:
        fLeft = -fWidth;
        break;
    }
    switch (eVAlign)
    {
    case BITMAP_VALIGN_TOP:
        fTop = 0.0f;
        fBottom = fHeight;
        break;
    case BITMAP_VALIGN_CENTER:
        fTop = fHeight * -0.5f;
        fBottom = -fTop;
        break;
    case BITMAP_VALIGN_BOTTOM:
        fTop = -fHeight;
        break;
    }
    SetRect(fLeft, fTop, fRight, fBottom);
}

// FUNCTION: SWBF2 0x006e4ef0
void RedBitmapElement::SetTexture(const RedTexture* const pTexture)
{
    SetTexture(pTexture ? pTexture->GetName() : 0u);
}

// FUNCTION: SWBF2 0x006e4cb0
void RedBitmapElement::CreateHotSpot(float fRadius)
{
    float fLeft, fTop, fRight, fBottom;
    GetRect(fLeft, fTop, fRight, fBottom);

    fLeft -= fRadius;
    fTop -= fRadius;
    fRight += fRadius;
    fBottom += fRadius;

    if (!m_pHotSpot)
    {
        m_pHotSpot = new RedHotSpot;
    }
    m_pHotSpot->SetRect(fLeft, fTop, fRight, fBottom);
}
