#include <float.h>
#include <stdlib.h>

#include "HUDProceduralBarBitmap.h"
#include "HUDObjectiveList.h"
#include "NetGame.h"

static const unsigned int kNoiseFreq = 0x86b8caa3;
static const unsigned int kNoiseRoughness = 0x37425e11;
static const unsigned int kGlowScaleSize = 0x84e84d97;
static const unsigned int kGlowAlphaBase = 0xba3e10e5;
static const unsigned int kGlowAlphaScale = 0xdd5384a2;

// FUNCTION: SWBF2 0x00566360
void HUD::ObjectiveList::SetState(unsigned int uiTextHashID, Objective::ObjectiveState state)
{
    Objective* pObjective = FindObjective(uiTextHashID);
    if (!pObjective)
    {
        return;
    }
    pObjective->SetState(state);
    SetupText();
    if (NetGame::IsEnabled() && !NetGame::IsClient())
    {
        NetGame::CreateEvent_UpdateObjective(pObjective, -1, -1);
    }
}

// FUNCTION: SWBF2 0x005663d0
HUD::Objective::ObjectiveState HUD::ObjectiveList::GetState(unsigned int uiTextHashID)
{
    Objective* pObjective = FindObjective(uiTextHashID);
    if (pObjective)
    {
        return pObjective->_state;
    }
    return Objective::INACTIVE;
}

// FUNCTION: SWBF2 0x00566430
HUD::Objective* HUD::ObjectiveList::FindObjective(unsigned int uiTextHashID)
{
    for (PblList<Objective>::Itor it(_objectives); it != _objectives.End(); ++it)
    {
        if (it->_textHashID == uiTextHashID)
        {
            return it;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005667d0
HUD::ProceduralBarBitmap::Factory::Factory()
{
    mHashID = 0x86E6EF58u;
    mName = "ProceduralBarBitmap";

    mNoiseFreq.Init("NoiseFreq", mProperties, 0.0f, FLT_MAX, 0.01f);
    mNoiseRoughness.Init("NoiseRoughness", mProperties, 0.0f, FLT_MAX, 0.01f);
    mGlowScaleSize.Init("GlowScaleSize", mProperties, 0.0f, FLT_MAX, 0.01f);
    mGlowAlphaBase.Init("GlowAlphaBase", mProperties, 0.0f, FLT_MAX, 0.01f);
    mGlowAlphaScale.Init("GlowAlphaScale", mProperties, 0.0f, FLT_MAX, 0.01f);
}

// FUNCTION: SWBF2 0x00566750
HUD::Item* HUD::ProceduralBarBitmap::Factory::CreateItem(const char* const pName, ItemCallback pCallback,
                                                          void* const pUserData)
{
    HUD::Item* pItem = new ProceduralBarBitmap(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00566d90
float HUD::ProceduralBarBitmap::SetValue(const float fValue)
{
    value_ = HUD::ElementBar::SetValue(fValue);
    return value_;
}

// SYNTHETIC: SWBF2 0x00566a50
// HUD::ProceduralBarBitmap::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00566f1f
// [thunk]:HUD::ProceduralBarBitmap::`scalar deleting destructor' adjustor{288}
// SYNTHETIC: SWBF2 0x00566f35
// [thunk]:HUD::ProceduralBarBitmap::`scalar deleting destructor' adjustor{544}
// FUNCTION: SWBF2 0x005669a0
HUD::ProceduralBarBitmap::ProceduralBarBitmap(const char* const pName, Factory::ItemCallback pCallback,
                                              void* const pUserData)
    : HUD::ElementBarBitmap(pName, pCallback, pUserData),
      halign_(RedBitmapElement::BITMAP_HALIGN_LEFT),
      valign_(RedBitmapElement::BITMAP_VALIGN_TOP),
      width_(0.0f),
      height_(0.0f),
      value_(0.0f),
      mGlowAlphaBase(0.5f),
      mGlowAlphaScale(1.0f),
      mNoiseFreq(60.0f),
      mNoiseRoughness(0.3f),
      mGlowScaleSize(0.05f),
      mGlowScaleBase(1.3f)
{
}

// SYNTHETIC: SWBF2 0x00566f14
// [thunk]:HUD::ProceduralBarBitmap::PostReadSetup adjustor{544}
// FUNCTION: SWBF2 0x00566bc0
void HUD::ProceduralBarBitmap::PostReadSetup()
{
    HUD::ElementBarBitmap::PostReadSetup();
    mSavedAlpha = mAlpha;
}

// SYNTHETIC: SWBF2 0x00566f2a
// [thunk]:HUD::ProceduralBarBitmap::ReadData adjustor{544}
// FUNCTION: SWBF2 0x00566e70
bool HUD::ProceduralBarBitmap::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kNoiseFreq:
        mNoiseFreq = data.GetFloatArg(0);
        return true;
    case kNoiseRoughness:
        mNoiseRoughness = data.GetFloatArg(0);
        return true;
    case kGlowScaleSize:
        mGlowScaleSize = data.GetFloatArg(0);
        return true;
    case kGlowAlphaBase:
        mGlowAlphaBase = data.GetFloatArg(0);
        return true;
    case kGlowAlphaScale:
        mGlowAlphaScale = data.GetFloatArg(0);
        return true;
    }
    return HUD::ElementBarBitmap::ReadData(config, data);
}

// SYNTHETIC: SWBF2 0x00566f40
// [thunk]:HUD::ProceduralBarBitmap::SetProperty adjustor{544}
// FUNCTION: SWBF2 0x00566a80
bool HUD::ProceduralBarBitmap::SetProperty(const unsigned int uHashID, const void* const pValue)
{
    switch (uHashID)
    {
    case kNoiseFreq:
        mNoiseFreq = *(const float*)pValue;
        static_cast<ElementBar*>(this)->SetValue(mValue);
        return true;
    case kNoiseRoughness:
        mNoiseRoughness = *(const float*)pValue;
        static_cast<ElementBar*>(this)->SetValue(mValue);
        return true;
    case kGlowScaleSize: mGlowScaleSize = *(const float*)pValue; return true;
    case kGlowAlphaBase: mGlowAlphaBase = *(const float*)pValue; return true;
    case kGlowAlphaScale: mGlowAlphaScale = *(const float*)pValue; return true;
    }
    return ElementBarBitmap::SetProperty(uHashID, pValue);
}

// SYNTHETIC: SWBF2 0x00566f09
// [thunk]:HUD::ProceduralBarBitmap::GetProperty adjustor{544}
// FUNCTION: SWBF2 0x00566b30
bool HUD::ProceduralBarBitmap::GetProperty(const unsigned int uHashID, void* const pValue)
{
    float* value = (float*)pValue;
    switch (uHashID)
    {
    case kNoiseFreq: *value = mNoiseFreq; return true;
    case kNoiseRoughness: *value = mNoiseRoughness; return true;
    case kGlowScaleSize: *value = mGlowScaleSize; return true;
    case kGlowAlphaBase: *value = mGlowAlphaBase; return true;
    case kGlowAlphaScale: *value = mGlowAlphaScale; return true;
    }
    return ElementBarBitmap::GetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x00566dc0
void HUD::ProceduralBarBitmap::OnRender()
{
    const float alphaRandom = (float)(rand() % 0x7fff) / 32767.0f;
    mAlpha = (mGlowAlphaScale * alphaRandom + mGlowAlphaBase) * mSavedAlpha;
    const float scaleRandom = (float)(rand() % 0x7fff) / 32767.0f;
    const float scale = (scaleRandom * 0.5f + 0.5f) * mGlowScaleSize + mGlowScaleBase;
    SetRect_(width_ * value_ * scale, height_ * scale);
}

// SYNTHETIC: SWBF2 0x00566f4b
// [thunk]:HUD::ProceduralBarBitmap::WriteData adjustor{544}
// FUNCTION: SWBF2 0x00566f00
void HUD::ProceduralBarBitmap::WriteData(HUD::File& file, unsigned int uTabs)
{
    ElementBarBitmap::WriteData(file, uTabs);
}

// FUNCTION: SWBF2 0x00566be0
void HUD::ProceduralBarBitmap::SetRect(float fLeft, float fTop, float fRight, float fBottom)
{
    left_ = fLeft;
    right_ = fRight;
    top_ = fTop;
    bottom_ = fBottom;
    RedBitmapElement::SetRect_(fLeft, fTop, fRight, fBottom);
}

// FUNCTION: SWBF2 0x00566c40
void HUD::ProceduralBarBitmap::SetRect(float fWidth, float fHeight,
                                       RedBitmapElement::BitmapHAlignT eHAlign,
                                       RedBitmapElement::BitmapVAlignT eVAlign)
{
    halign_ = eHAlign;
    width_ = fWidth;
    height_ = fHeight;
    valign_ = eVAlign;
    SetRect_(fWidth, fHeight);
}

// FUNCTION: SWBF2 0x00566c80
void HUD::ProceduralBarBitmap::SetRect_(float fWidth, float fHeight)
{
    float fLeft = 0.0f;
    float fTop = 0.0f;
    float fRight = 0.0f;
    float fBottom = 0.0f;
    const float fCenterX = (left_ + right_) * 0.5f;
    const float fCenterY = (top_ + bottom_) * 0.5f;

    switch (halign_)
    {
    case RedBitmapElement::BITMAP_HALIGN_LEFT:
        fLeft += left_;
        fRight = left_ + fWidth;
        break;
    case RedBitmapElement::BITMAP_HALIGN_CENTER:
        fLeft = fCenterX - fWidth * 0.5f;
        fRight = fWidth * 0.5f + fCenterX;
        break;
    case RedBitmapElement::BITMAP_HALIGN_RIGHT:
        fRight = right_;
        fLeft = fRight - fWidth;
        break;
    }

    switch (valign_)
    {
    case RedBitmapElement::BITMAP_VALIGN_TOP:
        fTop += top_;
        fBottom = top_ + fHeight;
        break;
    case RedBitmapElement::BITMAP_VALIGN_CENTER:
        fTop = fCenterY - fHeight * 0.5f;
        bottom_ = fHeight * 0.5f + fCenterY;
        break;
    case RedBitmapElement::BITMAP_VALIGN_BOTTOM:
        fBottom = bottom_;
        fTop = fBottom - fHeight;
        break;
    }

    RedBitmapElement::SetRect(fLeft, fTop, fRight, fBottom);
}
