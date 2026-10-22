#include <string.h>

#include "HUDTransformNumberVector3.h"

HUD::TransformNumberVector3::Factory::Factory()
    : HUD::Item::Factory(0x2ef8f006u)
{
    mName = "TransformNumberVector3";
}

static const unsigned int kNumberVector3Hash = 0x37737dd4;

static const unsigned int kMaxMappings = 256;

// SYNTHETIC: SWBF2 0x005686a0
// HUD::TransformNumberVector3::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005686d0
HUD::TransformNumberVector3::~TransformNumberVector3()
{
    if (mMapping)
    {
        delete[] mMapping;
        mMapping = 0;
    }
}

// FUNCTION: SWBF2 0x00568730
void HUD::TransformNumberVector3::Read(PblConfig& config, Factory::ItemCallback pCallback,
                                       void* const pUserData)
{
    NumberVector3 aMapping[kMaxMappings];
    mMapping = aMapping;
    Item::Read(config, pCallback, pUserData);
    if (mNumMappings)
    {
        mMapping = new NumberVector3[mNumMappings];
        if (mMapping)
        {
            memcpy(mMapping, aMapping, mNumMappings * sizeof(NumberVector3));
        }
    }
    else
    {
        mMapping = 0;
    }
}

// FUNCTION: SWBF2 0x005687b0
bool HUD::TransformNumberVector3::ReadData(PblConfig& config, PblConfig::Data& data)
{
    if (data.GetId() != kNumberVector3Hash)
    {
        return TransformNumberType::ReadData(config, data, EventClass::type_Vector3);
    }

    if (mNumMappings < kMaxMappings)
    {
        NumberVector3& mapping = mMapping[mNumMappings++];
        mapping.number = data.GetFloatArg(0);
        mapping.vector3.x = data.GetFloatArg(1);
        mapping.vector3.y = data.GetFloatArg(2);
        mapping.vector3.z = data.GetFloatArg(3);
    }
    return true;
}

// FUNCTION: SWBF2 0x00568810
void HUD::TransformNumberVector3::WriteData(HUD::File& file, unsigned int uTabs)
{
    for (unsigned int i = 0; i < mNumMappings; ++i)
    {
        const NumberVector3& mapping = mMapping[i];
        file.WriteTabs(uTabs);
        file.WriteStringf("NumberVector3(%f, %f, %f, %f)\n", mapping.number,
                          mapping.vector3.x, mapping.vector3.y, mapping.vector3.z);
    }
    TransformNumberType::WriteData(file, uTabs);
}

// FUNCTION: SWBF2 0x00568660
HUD::TransformNumberVector3::TransformNumberVector3(const char* const pName,
                                                    Factory::ItemCallback pCallback,
                                                    void* const pUserData)
    : HUD::TransformNumberType(pName, pCallback, pUserData),
      mMapping(0),
      mNumMappings(0)
{
    mEventInput.Init(EventInput, this);
}

// FUNCTION: SWBF2 0x005688c0
PblVector3 HUD::TransformNumberVector3::CalculateOutputVector3(float fInput) const
{
    const float fValue = CalculateInputValue(fInput, mMapping[0].number, mMapping[mNumMappings - 1].number);
    unsigned int i = 0;
    while (i < mNumMappings && mMapping[i].number < fValue)
    {
        ++i;
    }
    const NumberVector3& high = mMapping[(i == mNumMappings) ? i - 1 : i];
    const NumberVector3& low = mMapping[i ? i - 1 : 0];
    const float fDelta = high.number - low.number;
    if (fDelta == 0.0f)
    {
        return high.vector3;
    }
    return Interpolate(low.vector3, high.vector3, (fValue - low.number) / fDelta);
}

// FUNCTION: SWBF2 0x005689e0
void HUD::TransformNumberVector3::EventInput(const HUD::Event& event, void* const pData)
{
    switch (event.GetClass()->GetType())
    {
    case EventClass::type_Int:
    case EventClass::type_Uint:
    case EventClass::type_Float:
    {
        TransformNumberVector3* pTransform = (TransformNumberVector3*)pData;
        if (!pTransform->mEventClassOutput || !pTransform->mNumMappings)
        {
            break;
        }
        const PblVector3 vector = pTransform->CalculateOutputVector3(
            event.GetDataFloat() * pTransform->mInputFactorConstant * pTransform->mInputFactor);
        HUD::Event(pTransform->mEventClassOutput, &vector).Send();
        break;
    }
    default:
        break;
    }
}
