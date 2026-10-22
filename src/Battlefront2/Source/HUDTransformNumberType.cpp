#include "HUDTransformNumberColor.h"

static const unsigned int kInputFactorHash = 0xf73d9a74;
static const unsigned int kEventInputFactorHash = 0x9d63ddbe;
static const unsigned int kWrapInputHash = 0x4b4864db;

 
 
 

// FUNCTION: SWBF2 0x00568430
HUD::TransformNumberType::TransformNumberType(const char* const pName, Factory::ItemCallback pCallback,
                                              void* const pUserData)
    : HUD::Transform(pName, pCallback, pUserData),
      mInputFactorConstant(1.0f),
      mInputFactor(1.0f)
{
    mWrapInput = false;
    mEventInputFactor.Init(EventInputFactor, this);
}

// SYNTHETIC: SWBF2 0x00568480
// HUD::TransformNumberType::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005684b0
HUD::TransformNumberType::~TransformNumberType()
{
}

// FUNCTION: SWBF2 0x005684d0
bool HUD::TransformNumberType::ReadData(PblConfig& config, PblConfig::Data& data,
                                        EventClass::Type eType)
{
    switch (data.GetId())
    {
    case kWrapInputHash:
        mWrapInput = *(const float*)&data.m_auiArgs[0] != 0.0f;
        return true;
    case kEventInputFactorHash:
        Item::ReadEvent(data, mEventInputFactor);
        return true;
    case kInputFactorHash:
        mInputFactorConstant = *(const float*)&data.m_auiArgs[0];
        return true;
    }
    return Transform::ReadData(config, data, eType);
}

// FUNCTION: SWBF2 0x00568550
void HUD::TransformNumberType::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("InputFactor", uTabs, mInputFactorConstant, 1.0f);
    file.WriteProperty("EventInputFactor", uTabs,
                       mEventInputFactor.GetRegisteredClassName(false), "");
    file.WriteProperty("WrapInput", uTabs, (int)mWrapInput, 0);
    Transform::WriteData(file, uTabs);
}

// FUNCTION: SWBF2 0x005685c0
float HUD::TransformNumberType::CalculateInputValue(float fInput, float fMin, float fMax) const
{
    const float fRange = fMax - fMin;
    if (mWrapInput)
    {
        if (fInput < fMin)
        {
            fInput = fMin - fInput;
            if (fInput < fMin)
            {
                fInput = fMax - fInput;
            }
        }
        if (fInput > fMax)
        {
            fInput = fInput - (float)(int)(fInput / fRange) * fRange + fMin;
        }
        return fInput;
    }
    if (fInput < fMin)
    {
        return fMin;
    }
    if (fInput > fMax)
    {
        return fMax;
    }
    return fInput;
}

// FUNCTION: SWBF2 0x00568630
void HUD::TransformNumberType::EventInputFactor(const HUD::Event& event, void* const pData)
{
    const EventClass::Type type = event.mClass->mType;
    if (type >= EventClass::type_Int && type <= EventClass::type_Float)
    {
        static_cast<TransformNumberType*>(pData)->mInputFactor = event.GetDataFloat();
    }
}
