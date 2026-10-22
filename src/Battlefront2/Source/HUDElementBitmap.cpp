#include <float.h>

#include "HUDElementBitmap.h"
#include "RedEngineFL/Interface/RedGroupElement.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"

 
 

static const unsigned int kBitmapHash = 0x46544626;
static const unsigned int kBitmapStyleHash = 0xa28f0eeb;
static const unsigned int kBitmapWidthHeightHash = 0x967ad461;
static const unsigned int kRelativeWidthHeightHash = 0x0941e5b8;
static const unsigned int kBitmapHAlignHash = 0x47ce65fb;
static const unsigned int kBitmapVAlignHash = 0x8f47c6bd;
static const unsigned int kTexCoordsHash = 0x1ffad2e0;
static const unsigned int kEventBitmapHash = 0x5cd8e364;
static const unsigned int kBitmapRectHash = 0x625fa794;
static const unsigned int kMaskHash = 0xe7774569;
static const unsigned int kMaskTexCoordsHash = 0x7e272c84;

// GLOBAL: SWBF2 0x007e6920

char* HUD::ElementBitmapBase::Factory::sBitmapHAlignStrings[3] =
{
    "Left", "Center", "Right"
};

// GLOBAL: SWBF2 0x007e692c

char* HUD::ElementBitmapBase::Factory::sBitmapVAlignStrings[3] =
{
    "Top", "Center", "Bottom"
};

// GLOBAL: SWBF2 0x007e6938

char* HUD::ElementBitmapBase::Factory::sBitmapStyleStrings[2] =
{
    "Normal", "Shadow"
};

// FUNCTION: SWBF2 0x0054ccd0
HUD::ElementBitmapBase::Factory::Factory(unsigned int uHashID)
    : HUD::Element::Factory(uHashID)
{
    mBitmap.Init("Bitmap", mProperties, Property::type_Texture);
    mBitmapStyle.Init("BitmapStyle", mProperties, sBitmapStyleStrings, 0, 1);
    mBitmapWidthHeight.Init("BitmapWidthHeight", mProperties, Property::type_Vector2);
    mBitmapWidthHeight.InitVectorRange(Property::axis_X, 0.0f, FLT_MAX, 1.0f);
    mBitmapWidthHeight.InitVectorRange(Property::axis_Y, 0.0f, FLT_MAX, 1.0f);
    mRelativeWidthHeight.Init("RelativeWidthHeight", mProperties, HUD::gRelativeModeStrings, 0, 3);
    mBitmapHAlign.Init("BitmapHAlign", mProperties, sBitmapHAlignStrings, 0, 2);
    mBitmapVAlign.Init("BitmapVAlign", mProperties, sBitmapVAlignStrings, 0, 2);
    mTexCoords.Init("TexCoords", mProperties, Property::type_Vector4);
    mTexCoords.InitVectorRange(Property::axis_X, FLT_MIN, FLT_MAX, 0.01f);
    mTexCoords.InitVectorRange(Property::axis_Y, FLT_MIN, FLT_MAX, 0.01f);
    mTexCoords.InitVectorRange(Property::axis_Z, FLT_MIN, FLT_MAX, 0.01f);
    mTexCoords.InitVectorRange(Property::axis_W, FLT_MIN, FLT_MAX, 0.01f);
    mEventBitmap.Init("EventBitmap", mProperties, Property::type_Event);
}

HUD::ElementBitmap::Factory::Factory()
    : HUD::ElementBitmapBase::Factory(0x46544626u)
{
    mName = "Bitmap";
}

// FUNCTION: SWBF2 0x0054deb0
HUD::ElementBitmapMasked::Factory::Factory()
    : HUD::ElementBitmapBase::Factory(0xD23CB4F7u)
{
    mBitmapMask.Init("Mask", mProperties, Property::type_Texture);
}

HUD::ElementBitmapMasked::ElementBitmapMasked(const char* const pName,
                                              Factory::ItemCallback pCallback,
                                              void* const pUserData)
    : HUD::ElementBitmapBase(*static_cast<RedMaskedBitmapElement*>(this), pName, pCallback,
                             pUserData),
      mMaskTextureName(0)
{
    RedMaskedBitmapElement::Enable(false);
}

// FUNCTION: SWBF2 0x0054cb10
HUD::Item* HUD::ElementBitmapMasked::Factory::CreateItem(const char* const pName,
                                                         ItemCallback pCallback,
                                                         void* const pUserData)
{
    HUD::Item* pItem = new ElementBitmapMasked(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

// SYNTHETIC: SWBF2 0x0054e1a9
// [thunk]:HUD::ElementBitmapMasked::`scalar deleting destructor' adjustor{288}

// FUNCTION: SWBF2 0x0054cbc0
HUD::ElementBitmapMasked::~ElementBitmapMasked()
{
    FreeName(&mMaskTextureName);
}

// FUNCTION: SWBF2 0x0054cc30
void* HUD::ElementBitmapMasked::GetParent()
{
    return this;
}

// SYNTHETIC: SWBF2 0x0054cc40
// HUD::ElementBitmapMasked::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0054cf20
HUD::ElementBitmapBase::ElementBitmapBase(RedBitmapElement& element, const char* const pName,
                                          Factory::ItemCallback pCallback,
                                          void* const pUserData)
    : HUD::Element(element, pName, pCallback, pUserData),
      mBitmapName(0),
      mRelativeMode(relativeMode_Viewport),
      mHAlign(0),
      mVAlign(0)
{
    mEventBitmap.Init(EventBitmap, this);
}

// SYNTHETIC: SWBF2 0x0054cf80
// HUD::ElementBitmapBase::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0054cfb0
HUD::ElementBitmapBase::~ElementBitmapBase()
{
    FreeName(&mBitmapName);
}

// FUNCTION: SWBF2 0x0054d010
void HUD::ElementBitmapBase::GetSize(float& rfWidth, float& rfHeight)
{
    float fLeft, fTop, fRight, fBottom;
    ((RedBitmapElement*)mElement)->GetRect(fLeft, fTop, fRight, fBottom);
    rfWidth = fRight - fLeft;
    rfHeight = fBottom - fTop;
}

// FUNCTION: SWBF2 0x0054d060
void HUD::ElementBitmapBase::SetRect(float fX, float fY, RedBitmapElement::BitmapHAlignT eHAlign,
                                     RedBitmapElement::BitmapVAlignT eVAlign)
{
    ((RedBitmapElement*)mElement)->SetRect(fX, fY, eHAlign, eVAlign);
    mHAlign = (unsigned char)eHAlign;
    mVAlign = (unsigned char)eVAlign;
}

// FUNCTION: SWBF2 0x0054d0a0
void HUD::ElementBitmap::SetTexture(const char* pName)
{
    FreeName(&mBitmapName);
    CopyName(&mBitmapName, pName, 0);
    SetTexture((unsigned int)PblHash(pName));
}

// FUNCTION: SWBF2 0x0054d0f0
void HUD::ElementBitmap::SetTexture(unsigned int uiTextureHash)
{
    RedBitmapElement::SetTexture(uiTextureHash);
}

// FUNCTION: SWBF2 0x0054d100
bool HUD::ElementBitmapBase::SetProperty(const unsigned int uHashID, const void* const pValue)
{
    RedBitmapElement* const pBitmap = (RedBitmapElement*)mElement;
    if (uHashID == kTexCoordsHash)
    {
        const float* pCoords = (const float*)pValue;
        pBitmap->SetTexCoords(pCoords[0], pCoords[1], pCoords[2], pCoords[3], false);
    }
    else if (uHashID == kRelativeWidthHeightHash)
    {
        mRelativeMode = (unsigned char)*(const unsigned int*)pValue;
    }
    else if (uHashID == kBitmapHash)
    {
        pBitmap->SetTexture((*(const RedTexture* const*)pValue)->m_uiNameHash);
    }
    else if (uHashID == kBitmapHAlignHash)
    {
        float fLeft, fTop, fRight, fBottom;
        pBitmap->GetRect(fLeft, fTop, fRight, fBottom);
        mHAlign = (unsigned char)*(const unsigned int*)pValue;
        pBitmap->SetRect(fRight - fLeft, fBottom - fTop,
                         (RedBitmapElement::BitmapHAlignT)mHAlign,
                         (RedBitmapElement::BitmapVAlignT)mVAlign);
    }
    else if (uHashID == kEventBitmapHash)
    {
        Item::SetEvent(*(const char* const*)pValue, mEventBitmap);
    }
    else if (uHashID == kBitmapVAlignHash)
    {
        float fLeft, fTop, fRight, fBottom;
        pBitmap->GetRect(fLeft, fTop, fRight, fBottom);
        mVAlign = (unsigned char)*(const unsigned int*)pValue;
        pBitmap->SetRect(fRight - fLeft, fBottom - fTop,
                         (RedBitmapElement::BitmapHAlignT)mHAlign,
                         (RedBitmapElement::BitmapVAlignT)mVAlign);
    }
    else if (uHashID == kBitmapWidthHeightHash)
    {
        const float* pSize = (const float*)pValue;
        pBitmap->SetRect(pSize[0], pSize[1], (RedBitmapElement::BitmapHAlignT)mHAlign,
                         (RedBitmapElement::BitmapVAlignT)mVAlign);
    }
    else if (uHashID == kBitmapStyleHash)
    {
        pBitmap->SetStyle(*(const RedBitmapElement::BitmapStyleT*)pValue);
    }
    else
    {
        return HUD::Element::SetProperty(uHashID, pValue);
    }
    return true;
}

// FUNCTION: SWBF2 0x0054d2c0
bool HUD::ElementBitmapBase::GetProperty(const unsigned int uHashID, void* const pValue)
{
    RedBitmapElement* const pBitmap = (RedBitmapElement*)mElement;
    if (uHashID == kTexCoordsHash)
    {
        float afCoords[4];
        pBitmap->GetTexCoords(afCoords[0], afCoords[1], afCoords[2], afCoords[3]);
        float* pCoords = (float*)pValue;
        pCoords[0] = afCoords[0];
        pCoords[1] = afCoords[1];
        pCoords[2] = afCoords[2];
        pCoords[3] = afCoords[3];
    }
    else if (uHashID == kRelativeWidthHeightHash)
    {
        *(unsigned int*)pValue = mRelativeMode;
    }
    else if (uHashID == kBitmapHash)
    {
        *(const RedTexture**)pValue = pBitmap->GetTexture();
    }
    else if (uHashID == kBitmapHAlignHash)
    {
        *(unsigned int*)pValue = mHAlign;
    }
    else if (uHashID == kEventBitmapHash)
    {
        *(const char**)pValue = mEventBitmap.GetRegisteredClassName(false);
    }
    else if (uHashID == kBitmapVAlignHash)
    {
        *(unsigned int*)pValue = mVAlign;
    }
    else if (uHashID == kBitmapWidthHeightHash)
    {
        float fLeft, fTop, fRight, fBottom;
        pBitmap->GetRect(fLeft, fTop, fRight, fBottom);
        float* pSize = (float*)pValue;
        pSize[0] = fRight - fLeft;
        pSize[1] = fBottom - fTop;
    }
    else if (uHashID == kBitmapStyleHash)
    {
        *(RedBitmapElement::BitmapStyleT*)pValue = pBitmap->GetStyle();
    }
    else
    {
        return HUD::Element::GetProperty(uHashID, pValue);
    }
    return true;
}

// FUNCTION: SWBF2 0x0054d410
const char* HUD::ElementBitmapBase::PropertyTranslateEnum(const unsigned int uHashID,
                                                          const unsigned int uValue)
{
    if (uHashID == kBitmapHAlignHash)
    {
        return Factory::sBitmapHAlignStrings[uValue];
    }
    if (uHashID == kRelativeWidthHeightHash)
    {
        return gRelativeModeStrings[uValue];
    }
    if (uHashID == kBitmapVAlignHash)
    {
        return Factory::sBitmapVAlignStrings[uValue];
    }
    if (uHashID == kBitmapStyleHash)
    {
        return Factory::sBitmapStyleStrings[uValue];
    }
    return HUD::Element::PropertyTranslateEnum(uHashID, uValue);
}

// FUNCTION: SWBF2 0x0054d480
HUD::ElementBitmapBase& HUD::ElementBitmapBase::operator=(const ElementBitmapBase& rhs)
{
    HUD::Element::operator=(rhs);
    const RedBitmapElement* pSource = (const RedBitmapElement*)rhs.mElement;
    RedBitmapElement* pDest = (RedBitmapElement*)mElement;

    float fLeft, fTop, fRight, fBottom;
    pSource->GetRect(fLeft, fTop, fRight, fBottom);
    pDest->SetRect(fLeft, fTop, fRight, fBottom);

    float fU0, fV0, fU1, fV1;
    pSource->GetTexCoords(fU0, fV0, fU1, fV1);
    pDest->SetTexCoords(fU0, fV0, fU1, fV1, false);

    pDest->SetStyle(pSource->GetStyle());
    pDest->SetTexture(pSource->GetTexture());

    mHAlign = rhs.mHAlign;
    mVAlign = rhs.mVAlign;
    mRelativeMode = rhs.mRelativeMode;
    return *this;
}

// FUNCTION: SWBF2 0x0054d590
void HUD::ElementBitmapBase::PostReadSetup()
{
    HUD::Element::PostReadSetup();
    RedBitmapElement* pBitmap = (RedBitmapElement*)mElement;

    float fLeft, fTop, fRight, fBottom;
    pBitmap->GetRect(fLeft, fTop, fRight, fBottom);
    fLeft = ConvertRelativeToPixels((RelativeMode)mRelativeMode, fLeft, GetContainerFrameWidth(),
                                    GetContainerViewWidth(),
                                    (float)RedInterfaceScreen::GetScreenWidth());
    fRight = ConvertRelativeToPixels((RelativeMode)mRelativeMode, fRight, GetContainerFrameWidth(),
                                     GetContainerViewWidth(),
                                     (float)RedInterfaceScreen::GetScreenWidth());
    fTop = ConvertRelativeToPixels((RelativeMode)mRelativeMode, fTop, GetContainerFrameHeight(),
                                   GetContainerViewHeight(),
                                   (float)RedInterfaceScreen::GetScreenHeight());
    fBottom = ConvertRelativeToPixels((RelativeMode)mRelativeMode, fBottom,
                                      GetContainerFrameHeight(), GetContainerViewHeight(),
                                      (float)RedInterfaceScreen::GetScreenHeight());
    pBitmap->SetRect(fLeft, fTop, fRight, fBottom);
    pBitmap->Scale(1.0f, 0.75f / RedRenderer::GetScreenAspectRatio());
}

// FUNCTION: SWBF2 0x0054d760
bool HUD::ElementBitmapBase::ReadData(PblConfig& config, PblConfig::Data& data)
{
    RedBitmapElement* pBitmap = (RedBitmapElement*)mElement;
    const unsigned int uId = data.GetId();
    if (uId == kBitmapHash)
    {
        pBitmap->SetTexture((unsigned int)PblHash(data.GetStringArg(0)));
        FreeName(&mBitmapName);
        CopyName(&mBitmapName, data.GetStringArg(0), 0);
    }
    else if (uId == kTexCoordsHash)
    {
        pBitmap->SetTexCoords(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2),
                              data.GetFloatArg(3), false);
    }
    else if (uId == kEventBitmapHash)
    {
        Item::ReadEvent(data, mEventBitmap);
    }
    else if (uId == kBitmapRectHash)
    {
        const unsigned int uNumArgs = data.GetNumArgs();
        unsigned int uHAlign = 0;
        unsigned int uVAlign = 0;
        if (uNumArgs > 2)
        {
            const unsigned int uIndex =
                TranslateString(Factory::sBitmapHAlignStrings, 3, data.GetStringArg(2));
            if (uIndex < 3)
            {
                uHAlign = uIndex;
            }
        }
        if (uNumArgs > 3)
        {
            const unsigned int uIndex =
                TranslateString(Factory::sBitmapVAlignStrings, 3, data.GetStringArg(3));
            if (uIndex < 3)
            {
                uVAlign = uIndex;
            }
        }
        int iMode = relativeMode_Viewport;
        const float fX = data.GetFloatArg(0);
        const float fY = data.GetFloatArg(1);
        if (data.GetNumArgs() == 5)
        {
            if (data.IsStringArg(4))
            {
                iMode = (int)TranslateString(gRelativeModeStrings, 3, data.GetStringArg(4));
            }
            else
            {
                iMode = data.GetBoolArg(4) ? relativeMode_Viewport : relativeMode_Screen;
            }
            if (iMode < 0)
            {
                iMode = relativeMode_Viewport;
            }
        }
        pBitmap->SetRect(fX, fY, (RedBitmapElement::BitmapHAlignT)uHAlign,
                         (RedBitmapElement::BitmapVAlignT)uVAlign);
        mHAlign = (unsigned char)uHAlign;
        mVAlign = (unsigned char)uVAlign;
        mRelativeMode = (unsigned char)iMode;
    }
    else if (uId == kBitmapStyleHash)
    {
        const unsigned int uStyle =
            TranslateString(Factory::sBitmapStyleStrings, 2, data.GetStringArg(0));
        if (uStyle < 2)
        {
            pBitmap->SetStyle((RedBitmapElement::BitmapStyleT)uStyle);
        }
    }
    else
    {
        return HUD::Element::ReadData(config, data);
    }
    return true;
}

// FUNCTION: SWBF2 0x0054d990
void HUD::ElementBitmapBase::WriteData(HUD::File& file, unsigned int uTabs)
{
    RedBitmapElement* pBitmap = (RedBitmapElement*)mElement;
    float fU0, fV0, fU1, fV1;
    pBitmap->GetTexCoords(fU0, fV0, fU1, fV1);

    file.WriteProperty("Bitmap", uTabs, mBitmapName, "");
    file.WriteProperty("BitmapStyle", uTabs, Factory::sBitmapStyleStrings[pBitmap->GetStyle()],
                       Factory::sBitmapStyleStrings[0]);

    float fLeft, fTop, fRight, fBottom;
    pBitmap->GetRect(fLeft, fTop, fRight, fBottom);
    const float fWidth = fRight - fLeft;
    const float fHeight = fBottom - fTop;
    if (fWidth != 0.0f || fHeight != 0.0f || mHAlign != 0 || mVAlign != 0)
    {
        file.WriteTabs(uTabs);
        file.WriteStringf("BitmapRect(%f, %f, \"%s\", \"%s\", \"%s\")\n",
                          ConvertPixelsToRelative((RelativeMode)mRelativeMode, fWidth,
                                                  GetContainerFrameWidth(),
                                                  GetContainerViewWidth(),
                                                  (float)RedInterfaceScreen::GetScreenWidth()),
                          ConvertPixelsToRelative((RelativeMode)mRelativeMode, fHeight,
                                                  GetContainerFrameHeight(),
                                                  GetContainerViewHeight(),
                                                  (float)RedInterfaceScreen::GetScreenHeight()),
                          Factory::sBitmapHAlignStrings[mHAlign],
                          Factory::sBitmapVAlignStrings[mVAlign],
                          gRelativeModeStrings[mRelativeMode]);
    }

    file.WriteProperty("TexCoords", uTabs, fU0, fV0, fU1, fV1, 0.0f, 0.0f, 1.0f, 1.0f);
    file.WriteProperty("EventBitmap", uTabs, mEventBitmap.GetRegisteredClassName(true), "");
    HUD::Element::WriteData(file, uTabs);
}

// FUNCTION: SWBF2 0x0054dbf0
void HUD::ElementBitmapBase::EventBitmap(const HUD::Event& event, void* const pData)
{
    const EventClass::Type eType = event.GetClass()->GetType();
    if (eType == EventClass::type_Uint)
    {
        ((RedBitmapElement*)((ElementBitmapBase*)pData)->mElement)
            ->SetTexture(event.GetData().uintValue);
        EventChanged(event, pData);
    }
    else if (eType == EventClass::type_Texture)
    {
        ((RedBitmapElement*)((ElementBitmapBase*)pData)->mElement)
            ->SetTexture((const RedTexture*)event.GetData().texture);
        EventChanged(event, pData);
    }
}

// FUNCTION: SWBF2 0x0054dc70
void HUD::ElementBitmap::CopyToBitmaps(RedBitmapElement* const pBitmaps, unsigned int uCount,
                                       RedGroupElement* const pGroup, bool bCopyTexture)
{
    const unsigned char uAlpha = (unsigned char)(int)(mAlpha * 255.0f);
    float fLeft, fTop, fRight, fBottom;
    GetRect(fLeft, fTop, fRight, fBottom);
    float fU0, fV0, fU1, fV1;
    GetTexCoords(fU0, fV0, fU1, fV1);

    for (unsigned int i = 0; i < uCount; ++i)
    {
        RedBitmapElement& bitmap = pBitmaps[i];
        bitmap.m_color = m_color;
        bitmap.m_color.a = uAlpha;
        bitmap.SetBlendMode(GetBlendMode());

        PblMatrix matrix = m_mLocal;
        *(PblVector3*)matrix.m[3] = bitmap.GetPosition();
        matrix.m[3][3] = 1.0f;
        bitmap.SetMatrix(matrix);

        bitmap.SetRect(fLeft, fTop, fRight, fBottom);
        bitmap.SetStyle(GetStyle());
        bitmap.SetZOrder((unsigned char)m_zorder);
        bitmap.Enable(RedBitmapElement::IsEnabled());

        if (bCopyTexture)
        {
            const RedTexture* pTexture = GetTexture();
            bitmap.SetTexCoords(fU0, fV0, fU1, fV1, false);
            if (pTexture)
            {
                bitmap.SetTexture(pTexture->m_uiNameHash);
            }
        }
        if (pGroup)
        {
            bitmap.Enable(pGroup->AddElement(&bitmap));
        }
    }
}

// FUNCTION: SWBF2 0x0054df50
void HUD::ElementBitmapMasked::SetTexture(unsigned int uiTextureHash)
{
    RedMaskedBitmapElement::SetTexture(uiTextureHash);
}

// FUNCTION: SWBF2 0x0054df60
void HUD::ElementBitmapMasked::SetMaskTexture(const char* pName)
{
    FreeName(&mMaskTextureName);
    CopyName(&mMaskTextureName, pName, 0);
    SetMaskTexture((unsigned int)PblHash(pName));
}

// FUNCTION: SWBF2 0x0054dfb0
void HUD::ElementBitmapMasked::SetMaskTexture(unsigned int uHash)
{
    RedMaskedBitmapElement::SetMaskTexture(uHash);
}

// FUNCTION: SWBF2 0x0054dfc0
bool HUD::ElementBitmapMasked::SetProperty(const unsigned int uHashID, const void* const pValue)
{
    if (uHashID != kMaskHash)
    {
        return HUD::ElementBitmapBase::SetProperty(uHashID, pValue);
    }
    SetMaskTexture((*(const RedTexture* const*)pValue)->m_uiNameHash);
    return true;
}

// FUNCTION: SWBF2 0x0054dff0
bool HUD::ElementBitmapMasked::GetProperty(const unsigned int uHashID, void* const pValue)
{
    if (uHashID != kMaskHash)
    {
        return HUD::ElementBitmapBase::GetProperty(uHashID, pValue);
    }
    *(const RedTexture**)pValue = GetMaskTexture();
    return true;
}

// FUNCTION: SWBF2 0x0054e020
bool HUD::ElementBitmapMasked::ReadData(PblConfig& config, PblConfig::Data& data)
{
    RedMaskedBitmapElement* pBitmap = (RedMaskedBitmapElement*)mElement;
    const unsigned int uId = data.GetId();
    if (uId == kMaskTexCoordsHash)
    {
        pBitmap->SetMaskTexCoords(data.GetFloatArg(0), data.GetFloatArg(2), data.GetFloatArg(1),
                                  data.GetFloatArg(3));
    }
    else if (uId == kMaskHash)
    {
        pBitmap->SetMaskTexture((unsigned int)PblHash(data.GetStringArg(0)));
        FreeName(&mMaskTextureName);
        CopyName(&mMaskTextureName, data.GetStringArg(0), 0);
    }
    else
    {
        return HUD::ElementBitmapBase::ReadData(config, data);
    }
    return true;
}

// FUNCTION: SWBF2 0x0054e0e0
void HUD::ElementBitmapMasked::WriteData(HUD::File& file, unsigned int uTabs)
{
    RedMaskedBitmapElement* pBitmap = (RedMaskedBitmapElement*)mElement;
    file.WriteProperty("Mask", uTabs, mMaskTextureName ? mMaskTextureName : "", "");
    float f0, f1, f2, f3;
    pBitmap->GetMaskTexCoords(f0, f1, f2, f3);
    file.WriteProperty("MaskTexCoords", uTabs, f0, f2, f1, f3, 0.0f, 0.0f, 1.0f, 1.0f);
    HUD::ElementBitmapBase::WriteData(file, uTabs);
}
