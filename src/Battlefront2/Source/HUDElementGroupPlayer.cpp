#include "HUDElementGroupPlayer.h"

// FUNCTION: SWBF2 0x0054f640
HUD::ElementGroupPlayer::Factory::Factory(unsigned int uHashID)
    : HUD::ElementGroupBase::Factory(uHashID)
{
    mName = "GroupPlayer";
    mEventPlayerIndex.Init("EventPlayerIndex", mProperties, Property::type_Event);
}

static const unsigned int kEventPlayerIndexHash = 0x3c80f682;

// FUNCTION: SWBF2 0x0054f710
bool HUD::ElementGroupPlayer::SetProperty(const unsigned int uHashID, const void* pValue)
{
    if (uHashID == kEventPlayerIndexHash)
    {
        Item::SetEvent(*(const char* const*)pValue, mEventPlayerIndex);
        return true;
    }
    return ElementGroupBase::SetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x0054f740
bool HUD::ElementGroupPlayer::GetProperty(const unsigned int uHashID, void* pValue)
{
    if (uHashID == kEventPlayerIndexHash)
    {
        *(const char**)pValue = mEventPlayerIndex.GetRegisteredClassName(false);
        return true;
    }
    return ElementGroupBase::GetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x0054f770
bool HUD::ElementGroupPlayer::ReadData(PblConfig& config, PblConfig::Data& data)
{
    if (*(const unsigned int*)&data == kEventPlayerIndexHash)
    {
        Item::ReadEvent(data, mEventPlayerIndex);
        return true;
    }
    return ElementGroupBase::ReadData(config, data);
}

// FUNCTION: SWBF2 0x0054f7a0
void HUD::ElementGroupPlayer::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("EventPlayerIndex", uTabs,
                       mEventPlayerIndex.GetRegisteredClassName(true), "");
    ElementGroupBase::WriteData(file, uTabs);
}

// FUNCTION: SWBF2 0x0054f7e0
void HUD::ElementGroupPlayer::EventPlayerIndex(const HUD::Event& event, void* const pData)
{
    if (event.GetClass()->GetType() == EventClass::type_Int)
    {
        ElementGroupPlayer* pGroup = (ElementGroupPlayer*)pData;
        const int iPlayerIndex = event.GetData().intValue;
        if (iPlayerIndex != pGroup->mPlayerIndex)
        {
            pGroup->mPlayerIndex = iPlayerIndex;
            pGroup->PlayerIndexChanged();
        }
    }
}
