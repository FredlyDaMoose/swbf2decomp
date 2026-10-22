#pragma once

#include <stddef.h>

#include "HUDItem.h"

namespace HUD {
// VTABLE: SWBF2 0x007a07ac
class ElementBar
{
public:

    ElementBar();

    virtual ~ElementBar() {}

    // SYNTHETIC: SWBF2 0x0054a160
    // HUD::ElementBar::`scalar deleting destructor'

    virtual float SetValue(const float fValue);
    virtual bool SetProperty(const unsigned int uHashID, const void* pValue);
    virtual bool GetProperty(const unsigned int uHashID, void* pValue);
    virtual const char* PropertyTranslateEnum(const unsigned int, const unsigned int);

protected:
    virtual bool ReadData(PblConfig& config, PblConfig::Data& data);
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File& file, unsigned int uTabs);

    static void EventValue(const HUD::Event& event, void* const pData);

public:

    EventHandler mEventValue;
    float mDefaultValue;
    float mValue;
    float mMinValue;
    float mMaxValue;

    class Factory
    {
    public:

        // FUNCTION: SWBF2 0x0054a190
        void Setup(HUD::Item::Factory* pFactory);

        HUD::Item::Factory::Property mValue;
        HUD::Item::Factory::Property mMinValue;
        HUD::Item::Factory::Property mMaxValue;
        HUD::Item::Factory::Property mEventValue;
    };
};
}

 
 
 
 
