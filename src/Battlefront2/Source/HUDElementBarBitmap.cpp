#include "HUDElementBarBitmap.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Interface/RedGroupElement.h"
#include <math.h>

static const unsigned int kScaleSize = 0xf5b1a000;
static const unsigned int kScaleTexture = 0x79b5f340;
static const unsigned int kFlashyScale = 0xf5989fb2;
static const unsigned int kFlashyIncFadeOutTime = 0x2b76908f;
static const unsigned int kFlashyDecFadeOutTime = 0x2deba703;

// FUNCTION: SWBF2 0x0054a800
HUD::ElementBarBitmap::Factory::Factory()
{
    mHashID = 0x3BD46D91u;
    mName = "BarBitmap";

    mScaleSize.Init("ScaleSize", mProperties, Property::type_Bool);
    mScaleTexture.Init("ScaleTexture", mProperties, Property::type_Bool);
    mFlashyScale.Init("FlashyScale", mProperties, Property::type_Float);
    mFlashyIncFadeOutTime.Init("FlashIncFadeOutTime", mProperties, Property::type_Float);
    mFlashyDecFadeOutTime.Init("FlashDecFadeOutTime", mProperties, Property::type_Float);

    Setup(this);
}

// FUNCTION: SWBF2 0x0054a780
HUD::Item* HUD::ElementBarBitmap::Factory::CreateItem(const char* const pName, ItemCallback pCallback,
                                                       void* const pUserData)
{
    HUD::Item* pItem = new ElementBarBitmap(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

HUD::ElementBitmap::ElementBitmap(const char* const pName, Factory::ItemCallback pCallback,
                                  void* const pUserData)
    : HUD::ElementBitmapBase(*static_cast<RedBitmapElement*>(this), pName, pCallback, pUserData)
{
    RedBitmapElement::Enable(false);
}

// FUNCTION: SWBF2 0x0054a560
HUD::Item* HUD::ElementBitmap::Factory::CreateItem(const char* const pName, ItemCallback pCallback,
                                                    void* const pUserData)
{
    HUD::Item* pItem = new ElementBitmap(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0054a600
void* HUD::ElementBitmap::GetParent()
{
    return this;
}

// SYNTHETIC: SWBF2 0x0054a640
// HUD::ElementBitmapBase::Factory::~Factory

// SYNTHETIC: SWBF2 0x0054a720
// HUD::ElementBitmap::Factory::~Factory

// SYNTHETIC: SWBF2 0x0054a6b0
// HUD::ElementBitmap::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0054b78d
// [thunk]:HUD::ElementBitmap::`scalar deleting destructor' adjustor{288}

// FUNCTION: SWBF2 0x0054a730
HUD::ElementBitmap::~ElementBitmap()
{
}

// SYNTHETIC: SWBF2 0x0054abe0
// HUD::ElementBarBitmap::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0054b761
// [thunk]:HUD::ElementBarBitmap::`scalar deleting destructor' adjustor{288}
// SYNTHETIC: SWBF2 0x0054b777
// [thunk]:HUD::ElementBarBitmap::`scalar deleting destructor' adjustor{544}
// FUNCTION: SWBF2 0x0054ac10
HUD::ElementBarBitmap::~ElementBarBitmap()
{
}

// SYNTHETIC: SWBF2 0x0054a9c0
// HUD::ElementBarBitmap::Factory::~Factory

// FUNCTION: SWBF2 0x0054aa30
HUD::ElementBarBitmap::ElementBarBitmap(const char* const pName, HUD::Item::Factory::ItemCallback pCallback,
                                        void* const pUserData)
    : HUD::ElementBitmap(pName, pCallback, pUserData),
      mFlashyScale(0.2f),
      mFlashyIncFadeOutTime(0.0f),
      mFlashyDecFadeOutTime(0.0f),
      mBarWidth(1.0f),
      mBarU1(1.0f)
{
    mScaleTex = true;
    mScaleSize = true;
    mIncreasedValue = false;
    mInitialValueSet = false;

    mFlashyBit.SetWriteEnabled(false);
    mFlashyBit.SetDynamicallyAllocated(false);
    RedBitmapElement& rFlashy = mFlashyBit;
    rFlashy.SetTexture(0xDE020766u);
    rFlashy.SetColor(RedColor::WHITE);
    mFlashyBit.mFader.SetPhaseTime(HUD::Envelope::phase_Hold, 0.0f);
    mFlashyBit.mFader.SetPhaseTime(HUD::Envelope::phase_Sustain, 0.0f);
    mEventDisable.Init(EventDisable, this);
}

// SYNTHETIC: SWBF2 0x0054b756
// [thunk]:HUD::ElementBarBitmap::PostReadSetup adjustor{544}
// FUNCTION: SWBF2 0x0054b320
void HUD::ElementBarBitmap::PostReadSetup()
{
    HUD::ElementBitmapBase::PostReadSetup();
    HUD::ElementBar::PostReadSetup();

    RedBitmapElement* pBitmap = (RedBitmapElement*)mElement;
    float fLeft, fTop, fRight, fBottom;
    pBitmap->GetRect(fLeft, fTop, fRight, fBottom);
    mBarWidth = fRight - fLeft;
    pBitmap->GetTexCoords(fLeft, fTop, mBarU1, fBottom);
    RefreshFlashyBit();
}

// FUNCTION: SWBF2 0x0054b390
void HUD::ElementBarBitmap::RefreshFlashyBit()
{
    RedBitmapElement& rFlashy = mFlashyBit;
    RedGroupElement* pGroup = RedBitmapElement::GetGroup();
    RedGroupElement* pFlashyGroup = rFlashy.GetGroup();
    if (pGroup != pFlashyGroup)
    {
        if (pFlashyGroup)
        {
            pFlashyGroup->RemoveElement(&mFlashyBit);
        }
        if (pGroup)
        {
            pGroup->AddElement(&mFlashyBit);
        }
    }
    rFlashy.SetMatrix(m_mLocal);
    mFlashyBit.mAlpha = mAlpha;
    rFlashy.SetZOrder(m_zorder ? m_zorder - 1 : 0);
    rFlashy.SetBlendMode(GetBlendMode());
    mFlashyBit.mFader.SetPhaseTime(HUD::Envelope::phase_Release,
                                   mIncreasedValue ? mFlashyIncFadeOutTime : mFlashyDecFadeOutTime);
    const RedTexture* pTexture = GetTexture();
    if (pTexture)
    {
        rFlashy.SetTexture(pTexture->GetName());
    }
}

// SYNTHETIC: SWBF2 0x0054b76c
// [thunk]:HUD::ElementBarBitmap::ReadData adjustor{544}
// FUNCTION: SWBF2 0x0054b480
bool HUD::ElementBarBitmap::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kFlashyDecFadeOutTime:
        mFlashyDecFadeOutTime = data.GetFloatArg(0);
        return true;
    case kFlashyIncFadeOutTime:
        mFlashyIncFadeOutTime = data.GetFloatArg(0);
        return true;
    case kScaleTexture:
        mScaleTex = data.GetBoolArg(0);
        return true;
    case kFlashyScale:
        mFlashyScale = data.GetFloatArg(0);
        return true;
    case kScaleSize:
        mScaleSize = data.GetBoolArg(0);
        return true;
    }
    return HUD::ElementBar::ReadData(config, data) || HUD::ElementBitmapBase::ReadData(config, data);
}

// FUNCTION: SWBF2 0x0054aec0
void HUD::ElementBarBitmap::Update(const float fElapsed)
{
    HUD::Element::Update(fElapsed);
    if (!mFlashyBit.RedInterfaceElement::IsEnabled() || mIncreasedValue)
    {
        return;
    }

    float fLeft, fTop, fRight, fBottom;
    RedBitmapElement::GetRect(fLeft, fTop, fRight, fBottom);
    float fFlashyLeft, fFlashyTop, fFlashyRight, fFlashyBottom;
    mFlashyBit.RedBitmapElement::GetRect(fFlashyLeft, fFlashyTop, fFlashyRight, fFlashyBottom);

    float fT = 0.0f;
    if (mFlashyBit.mFader.GetPhase() == HUD::Envelope::phase_Release)
    {
        fT = mFlashyBit.mFader.GetElapsed();
        if (fT < 0.0f)
        {
            fT = 0.0f;
        }
        else if (fT > 1.0f)
        {
            fT = 1.0f;
        }
    }

    mFlashyBit.RedBitmapElement::SetRect(
        fFlashyLeft, fFlashyBottom - ((fBottom - fTop) * (1.0f - fT) + mFlashyScale * fT), fFlashyRight,
        fFlashyBottom);

    RedBitmapElement::GetTexCoords(fLeft, fTop, fRight, fBottom);
    mFlashyBit.RedBitmapElement::GetTexCoords(fFlashyLeft, fFlashyTop, fFlashyRight, fFlashyBottom);
    mFlashyBit.SetTexCoords(fFlashyLeft,
                            fFlashyBottom - ((fBottom - fTop) * (1.0f - fT) + mFlashyScale * fT),
                            fFlashyRight, fFlashyBottom, false);
}

// FUNCTION: SWBF2 0x0054b740
void HUD::ElementBarBitmap::EventDisable(const HUD::Event& event, void* const pData)
{
    ((ElementBarBitmap*)pData)->mInitialValueSet = false;
    HUD::Element::EventDisable(event, pData);
}

// SYNTHETIC: SWBF2 0x0054b782
// [thunk]:HUD::ElementBarBitmap::SetProperty adjustor{544}
// FUNCTION: SWBF2 0x0054acb0
bool HUD::ElementBarBitmap::SetProperty(const unsigned int uHashID, const void* const pValue)
{
    switch (uHashID)
    {
    case kFlashyIncFadeOutTime: mFlashyIncFadeOutTime = *(const float*)pValue; return true;
    case kFlashyDecFadeOutTime: mFlashyDecFadeOutTime = *(const float*)pValue; return true;
    case kFlashyScale: mFlashyScale = *(const float*)pValue; return true;
    case kScaleTexture:
        mScaleTex = *(const bool*)pValue;
        static_cast<ElementBar*>(this)->SetValue(mValue);
        return true;
    case kScaleSize:
        mScaleSize = *(const bool*)pValue;
        static_cast<ElementBar*>(this)->SetValue(mValue);
        return true;
    case 0x1ffad2e0u: mBarU1 = ((const float*)pValue)[2]; break;
    case 0x967ad461u: mBarWidth = *(const float*)pValue; break;
    }
    return ElementBitmapBase::SetProperty(uHashID, pValue) || ElementBar::SetProperty(uHashID, pValue);
}

// SYNTHETIC: SWBF2 0x0054b7a3
// [thunk]:HUD::ElementBarBitmap::GetProperty adjustor{544}
// FUNCTION: SWBF2 0x0054adc0
bool HUD::ElementBarBitmap::GetProperty(const unsigned int uHashID, void* const pValue)
{
    switch (uHashID)
    {
    case kFlashyIncFadeOutTime: *(float*)pValue = mFlashyIncFadeOutTime; return true;
    case kFlashyDecFadeOutTime: *(float*)pValue = mFlashyDecFadeOutTime; return true;
    case kFlashyScale: mFlashyScale = *(const float*)pValue; return true;
    case kScaleTexture: *(bool*)pValue = mScaleTex; return true;
    case kScaleSize: *(bool*)pValue = mScaleSize; return true;
    }
    return ElementBitmapBase::GetProperty(uHashID, pValue) || ElementBar::GetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x0054ae90
void HUD::ElementBarBitmap::GetSize(float& rfWidth, float& rfHeight)
{
    ElementBitmapBase::GetSize(rfWidth, rfHeight);
    rfWidth = mBarWidth;
}

// SYNTHETIC: SWBF2 0x0054b798
// [thunk]:HUD::ElementBarBitmap::WriteData adjustor{544}
// FUNCTION: SWBF2 0x0054b580
void HUD::ElementBarBitmap::WriteData(HUD::File& file, unsigned int uTabs)
{
    float fU0, fV0, fU1, fV1;
    RedBitmapElement::GetTexCoords(fU0, fV0, fU1, fV1);
    RedBitmapElement::SetTexCoords(fU0, fV0, mBarU1, fV1, false);
    file.WriteProperty("ScaleSize", uTabs, (int)mScaleSize, 1);
    file.WriteProperty("ScaleTexture", uTabs, (int)mScaleTex, 1);
    file.WriteProperty("FlashyScale", uTabs, mFlashyScale, 0.2f);
    file.WriteProperty("FlashyIncFadeOutTime", uTabs, mFlashyIncFadeOutTime, 0.0f);
    file.WriteProperty("FlashyDecFadeOutTime", uTabs, mFlashyDecFadeOutTime, 0.0f);
    ElementBar::WriteData(file, uTabs);
    float fLeft, fTop, fRight, fBottom;
    RedBitmapElement::GetRect(fLeft, fTop, fRight, fBottom);
    RedBitmapElement::SetRect(fLeft, fTop, fLeft + mBarWidth, fBottom);
    ElementBitmapBase::WriteData(file, uTabs);
    RedBitmapElement::SetRect(fLeft, fTop, fRight, fBottom);
}

// FUNCTION: SWBF2 0x0054b070
float HUD::ElementBarBitmap::SetValue(const float fValue)
{
    RedBitmapElement* pBitmap = (RedBitmapElement*)mElement;
    const float fOldValue = mValue;
    const float fNewValue = HUD::ElementBar::SetValue(fValue);

    if ((float)fabs((double)(fOldValue - fNewValue)) > 0.0001f)
    {
        float fU0, fV0, fU1, fV1;
        pBitmap->GetTexCoords(fU0, fV0, fU1, fV1);
        const float fScaledU1 = mScaleTex ? mBarU1 * fNewValue + fU0 : fU1;
        pBitmap->SetTexCoords(fU0, fV0, fScaledU1, fV1, false);

        if (mScaleSize)
        {
            float fLeft, fTop, fRight, fBottom;
            pBitmap->GetRect(fLeft, fTop, fRight, fBottom);
            const float fScaledRight = mBarWidth * fNewValue + fLeft;
            pBitmap->SetRect(fLeft, fTop, fScaledRight, fBottom);

            if (mFlashyBit.RedInterfaceElement::IsEnabled())
            {
                mFlashyBit.HUD::Element::Enable(false);
            }

            mIncreasedValue = fScaledRight > fRight;
            if (mInitialValueSet &&
                ((mIncreasedValue && mFlashyIncFadeOutTime > 0.0001f) ||
                 (!mIncreasedValue && mFlashyDecFadeOutTime > 0.0001f)))
            {
                RefreshFlashyBit();
                if (mFlashyBit.GetTexture() == pBitmap->GetTexture())
                {
                    mFlashyBit.SetTexCoords(fScaledU1, fV0, fU1, fV1, false);
                }
                mFlashyBit.RedBitmapElement::SetRect(fScaledRight, fTop, fRight, fBottom);
                mFlashyBit.HUD::Element::Enable(true);
            }
        }
        Changed();
    }

    mInitialValueSet = RedInterfaceElement::IsEnabled();
    return fNewValue;
}
