#include <string.h>

#include "HUDTransformNumberColor.h"

HUD::TransformNumberColor::Factory::Factory()
    : HUD::Item::Factory(0x0d34920du)
{
    mName = "TransformNumberColor";
}

static const unsigned int kNumberColorHash = 0xed01cbb7;

static const unsigned int kMaxMappings = 256;

// SYNTHETIC: SWBF2 0x00567bb0
// HUD::TransformNumberColor::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00567be0
HUD::TransformNumberColor::~TransformNumberColor()
{
    if (mMapping)
    {
        delete[] mMapping;
        mMapping = 0;
    }
}

// FUNCTION: SWBF2 0x00567c40
void HUD::TransformNumberColor::Read(PblConfig& config, Factory::ItemCallback pCallback,
                                     void* const pUserData)
{
    NumberColor aMapping[kMaxMappings];
    mMapping = aMapping;
    Item::Read(config, pCallback, pUserData);
    if (mNumMappings)
    {
        mMapping = new NumberColor[mNumMappings];
        if (mMapping)
        {
            memcpy(mMapping, aMapping, mNumMappings * sizeof(NumberColor));
        }
    }
    else
    {
        mMapping = 0;
    }
}

static inline float ClampColorComponent(float f)
{
    if (f < 0.0f)
    {
        return 0.0f;
    }
    if (f > 255.0f)
    {
        return 255.0f;
    }
    return f;
}

// FUNCTION: SWBF2 0x00567cc0
bool HUD::TransformNumberColor::ReadData(PblConfig& config, PblConfig::Data& data)
{
    if (data.GetId() != kNumberColorHash)
    {
        return TransformNumberType::ReadData(config, data, EventClass::type_Color);
    }

    if (mNumMappings < kMaxMappings)
    {
        NumberColor& mapping = mMapping[mNumMappings++];
        mapping.number = data.GetFloatArg(0);
        mapping.color.a = 0xff;
        mapping.color.b = (unsigned char)(int)ClampColorComponent(data.GetFloatArg(3));
        mapping.color.g = (unsigned char)(int)ClampColorComponent(data.GetFloatArg(2));
        mapping.color.r = (unsigned char)(int)ClampColorComponent(data.GetFloatArg(1));
    }
    return true;
}

// FUNCTION: SWBF2 0x00567d80
void HUD::TransformNumberColor::WriteData(HUD::File& file, unsigned int uTabs)
{
    for (unsigned int i = 0; i < mNumMappings; ++i)
    {
        const NumberColor& mapping = mMapping[i];
        file.WriteTabs(uTabs);
        file.WriteStringf("NumberColor(%f, %d, %d, %d)\n", mapping.number,
                          mapping.color.r, mapping.color.g, mapping.color.b);
    }
    TransformNumberType::WriteData(file, uTabs);
}

// FUNCTION: SWBF2 0x00567b70
HUD::TransformNumberColor::TransformNumberColor(const char* const pName,
                                                Factory::ItemCallback pCallback,
                                                void* const pUserData)
    : HUD::TransformNumberType(pName, pCallback, pUserData),
      mMapping(0),
      mNumMappings(0)
{
    mEventInput.Init(EventInput, this);
}

const RedColor Interpolate(const RedColor& a, const RedColor& b, float t);

// FUNCTION: SWBF2 0x00567e00
RedColor HUD::TransformNumberColor::CalculateOutputColor(float fInput) const
{
    const float fValue = CalculateInputValue(fInput, mMapping[0].number, mMapping[mNumMappings - 1].number);
    unsigned int i = 0;
    while (i < mNumMappings && mMapping[i].number < fValue)
    {
        ++i;
    }
    const NumberColor& high = mMapping[(i == mNumMappings) ? i - 1 : i];
    const NumberColor& low = mMapping[i ? i - 1 : 0];
    const float fDelta = high.number - low.number;
    if (fDelta == 0.0f)
    {
        return high.color;
    }
    return Interpolate(low.color, high.color, (fValue - low.number) / fDelta);
}

// FUNCTION: SWBF2 0x00567fb0
void HUD::TransformNumberColor::EventInput(const HUD::Event& event, void* const pData)
{
    switch (event.GetClass()->GetType())
    {
    case EventClass::type_Int:
    case EventClass::type_Uint:
    case EventClass::type_Float:
    {
        TransformNumberColor* pTransform = (TransformNumberColor*)pData;
        if (!pTransform->mEventClassOutput || !pTransform->mNumMappings)
        {
            break;
        }
        const RedColor color = pTransform->CalculateOutputColor(
            event.GetDataFloat() * pTransform->mInputFactorConstant * pTransform->mInputFactor);
        HUD::Event(pTransform->mEventClassOutput, &color).Send();
        break;
    }
    default:
        break;
    }
}
