#include <float.h>

#include "HUDElementBar.h"

// FUNCTION: SWBF2 0x0054a190
void HUD::ElementBar::Factory::Setup(HUD::Item::Factory* pFactory)
{
    mValue.Init("Value", pFactory->mProperties, 0.0f, FLT_MAX, 0.01f);
    mMinValue.Init("MinValue", pFactory->mProperties, 0.0f, FLT_MAX, 0.01f);
    mMaxValue.Init("MaxValue", pFactory->mProperties, 0.0f, FLT_MAX, 0.01f);
    mEventValue.Init("EventValue", pFactory->mProperties,
                     HUD::Item::Factory::Property::type_Event);
}

static const unsigned int kValueHash      = 0x425ed3ca;
static const unsigned int kMinValueHash   = 0xf1a5a54c;
static const unsigned int kMaxValueHash   = 0x7332c626;
static const unsigned int kEventValueHash = 0x8500add4;

// FUNCTION: SWBF2 0x0054a150
const char* HUD::ElementBar::PropertyTranslateEnum(const unsigned int, const unsigned int)
{
    return 0;
}

// FUNCTION: SWBF2 0x0054a220
HUD::ElementBar::ElementBar()
    : mDefaultValue(0.0f), mValue(1.0f), mMinValue(0.0f), mMaxValue(1.0f)
{
    mEventValue.Init(EventValue, this);
}

// FUNCTION: SWBF2 0x0054a260
float HUD::ElementBar::SetValue(const float fValue)
{
    const float fRange = mMaxValue - mMinValue;
    if (mMinValue > fValue)
    {
        mValue = mMinValue;
    }
    else if (fValue > mMaxValue)
    {
        mValue = mMaxValue;
    }
    else
    {
        mValue = fValue;
    }

    if (fRange == 0.0f)
    {
        return 0.0f;
    }
    return (mValue - mMinValue) / fRange;
}

// FUNCTION: SWBF2 0x0054a2c0
bool HUD::ElementBar::SetProperty(const unsigned int uHashID, const void* pValue)
{
    if (uHashID == kValueHash)
    {
        mDefaultValue = mValue = *(const float*)pValue;
    }
    else if (uHashID == kMinValueHash)
    {
        mMinValue = *(const float*)pValue;
    }
    else if (uHashID == kMaxValueHash)
    {
        mMaxValue = *(const float*)pValue;
    }
    else if (uHashID == kEventValueHash)
    {
        Item::SetEvent(*(const char* const*)pValue, mEventValue);
        return true;
    }
    else
    {
        return false;
    }
    SetValue(mValue);
    return true;
}

// FUNCTION: SWBF2 0x0054a340
bool HUD::ElementBar::GetProperty(const unsigned int uHashID, void* pValue)
{
    if (uHashID == kValueHash)
    {
        *(float*)pValue = mValue;
    }
    else if (uHashID == kMinValueHash)
    {
        *(float*)pValue = mMinValue;
    }
    else if (uHashID == kMaxValueHash)
    {
        *(float*)pValue = mMaxValue;
    }
    else if (uHashID == kEventValueHash)
    {
        *(const char**)pValue = mEventValue.GetRegisteredClassName(false);
    }
    else
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x0054a3b0
void HUD::ElementBar::PostReadSetup()
{
    if (mMinValue > mMaxValue)
    {
        mMaxValue = mMinValue;
    }
    SetValue(mValue);
}

// FUNCTION: SWBF2 0x0054a3e0
bool HUD::ElementBar::ReadData(PblConfig& config, PblConfig::Data& data)
{
    const unsigned int uId = data.GetId();
    if (uId == kValueHash)
    {
        mDefaultValue = mValue = data.GetFloatArg(0);
    }
    else if (uId == kMinValueHash)
    {
        mMinValue = data.GetFloatArg(0);
    }
    else if (uId == kMaxValueHash)
    {
        mMaxValue = data.GetFloatArg(0);
    }
    else if (uId == kEventValueHash)
    {
        Item::ReadEvent(data, mEventValue);
    }
    else
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x0054a450
void HUD::ElementBar::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("Value", uTabs, mDefaultValue, 0.0f);
    file.WriteProperty("MinValue", uTabs, mMinValue, 0.0f);
    file.WriteProperty("MaxValue", uTabs, mMaxValue, 1.0f);
    file.WriteProperty("EventValue", uTabs, mEventValue.GetRegisteredClassName(true), "");
}

// FUNCTION: SWBF2 0x0054a4e0
void HUD::ElementBar::EventValue(const HUD::Event& event, void* const pData)
{
    const EventClass::Type eType = event.GetClass()->GetType();
    if (eType == EventClass::type_Int || eType == EventClass::type_Uint ||
        eType == EventClass::type_Float)
    {
        ((ElementBar*)pData)->SetValue(event.GetDataFloat());
    }
}
