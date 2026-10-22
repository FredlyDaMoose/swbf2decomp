#include "HUDTransformNumberColorBlend.h"

HUD::TransformNumberColorBlend::Factory::Factory()
    : HUD::Item::Factory(0xf3c47270u)
{
    mName = "TransformNumberColorBlend";
}

// FUNCTION: SWBF2 0x00568020
HUD::TransformNumberColorBlend::TransformNumberColorBlend(const char* const pName,
                                                         Factory::ItemCallback pCallback,
                                                         void* const pUserData)
    : TransformNumberColor(pName, pCallback, pUserData), mBlendMode(blendMode_Alpha), mAlpha(127)
{
    mEventInput.Init(EventInput, this);
    mEventBlend.Init(EventBlend, this);
    mEventAlpha.Init(EventAlpha, this);
}

// FUNCTION: SWBF2 0x00568210
void HUD::TransformNumberColorBlend::EventInput(const Event& event, void* const pData)
{
    const EventClass::Type eType = event.GetClass()->GetType();
    if (eType < EventClass::type_Int || eType > EventClass::type_Float)
    {
        return;
    }
    TransformNumberColorBlend* pTransform = (TransformNumberColorBlend*)pData;
    if (!pTransform->mEventClassOutput)
    {
        return;
    }
    RedColor color = pTransform->CalculateOutputColor(event.GetDataFloat());
    const unsigned int uAlpha = pTransform->mAlpha;
    const unsigned int uOther = 255 - uAlpha;
    const unsigned char* pBlend = (const unsigned char*)pTransform->mBlendColor;
    if (pTransform->mBlendMode == blendMode_Alpha)
    {
        color.b = (unsigned char)(((uAlpha * color.b) >> 8) + ((uOther * pBlend[0]) >> 8));
        color.g = (unsigned char)(((uAlpha * color.g) >> 8) + ((uOther * pBlend[1]) >> 8));
        color.r = (unsigned char)(((uAlpha * color.r) >> 8) + ((uOther * pBlend[2]) >> 8));
    }
    else if (pTransform->mBlendMode == blendMode_Additive)
    {
        unsigned int b = pBlend[0] + ((uAlpha * color.b) >> 8);
        unsigned int g = pBlend[1] + ((uAlpha * color.g) >> 8);
        unsigned int r = pBlend[2] + ((uAlpha * color.r) >> 8);
        color.b = (unsigned char)(b > 255 ? 255 : b);
        color.g = (unsigned char)(g > 255 ? 255 : g);
        color.r = (unsigned char)(r > 255 ? 255 : r);
    }
    Event(pTransform->mEventClassOutput, &color).Send();
}

// FUNCTION: SWBF2 0x00568380
void HUD::TransformNumberColorBlend::EventBlend(const Event& event, void* const pData)
{
    if (event.GetClass()->GetType() == EventClass::type_Color)
    {
        *(RedColor*)((TransformNumberColorBlend*)pData)->mBlendColor = *event.mData.color;
    }
}

// FUNCTION: SWBF2 0x005683c0
void HUD::TransformNumberColorBlend::EventAlpha(const Event& event, void* const pData)
{
    TransformNumberColorBlend* pTransform = (TransformNumberColorBlend*)pData;
    switch (event.GetClass()->GetType())
    {
    case EventClass::type_Int:
    case EventClass::type_Uint:
        pTransform->mAlpha = (unsigned char)event.mData.uintValue;
        break;
    case EventClass::type_Float:
        pTransform->mAlpha = (unsigned char)(int)(event.mData.floatValue * 255.0f);
        break;
    }
}

// GLOBAL: SWBF2 0x007e69d0
char* HUD::TransformNumberColorBlend::sBlendMode[2] = { "Alpha", "Additive" };

// SYNTHETIC: SWBF2 0x00568090
// HUD::TransformNumberColorBlend::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005680c0
HUD::TransformNumberColorBlend::~TransformNumberColorBlend()
{
}

// FUNCTION: SWBF2 0x005680f0
bool HUD::TransformNumberColorBlend::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case 0x5d8b6dab:
        mAlpha = (unsigned char)(int)(data.GetFloatArg(0) * 255.0f);
        return true;
    case 0x211b0ef6:
        Item::ReadEvent(data, mEventBlend);
        return true;
    case 0x41b355c1:
        Item::ReadEvent(data, mEventAlpha);
        return true;
    case 0xfa784eab:
    {
        unsigned int uMode = HUD::TranslateString((const char**)sBlendMode, 2, data.GetStringArg(0)) + 1;
        if (uMode <= blendMode_Additive)
        {
            mBlendMode = (BlendMode)uMode;
        }
        return true;
    }
    }
    return TransformNumberColor::ReadData(config, data);
}

// FUNCTION: SWBF2 0x00568190
void HUD::TransformNumberColorBlend::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("BlendMode", uTabs, sBlendMode[mBlendMode - 1], sBlendMode[0]);
    file.WriteProperty("EventBlend", uTabs, mEventBlend.GetRegisteredClassName(true), "");
    file.WriteProperty("EventAlpha", uTabs, mEventAlpha.GetRegisteredClassName(true), "");
    TransformNumberColor::WriteData(file, uTabs);
}
