#pragma once

#include <stddef.h>
#include "HUDTransform.h"
#include "RedEngineFL/Graphics/RedColor.h"

class PblConfig;

namespace HUD {
class TransformNumberType : public HUD::Transform
{
public:

    TransformNumberType(const char* const pName, Factory::ItemCallback pCallback, void* const pUserData);
    virtual ~TransformNumberType();

protected:

    static void EventInputFactor(const HUD::Event& event, void* const pData);
    void WriteData(HUD::File&, unsigned int);
    virtual bool ReadData(PblConfig&, PblConfig::Data&, EventClass::Type);

    float CalculateInputValue(float fInput, float fMin, float fMax) const;

public:
    float mInputFactorConstant;
    float mInputFactor;
    HUD::EventHandler mEventInputFactor;
    bool mWrapInput : 1;
};
}

namespace HUD {
// VTABLE: SWBF2 0x007a3628
class TransformNumberColor : public HUD::TransformNumberType
{
public:

    TransformNumberColor(const char* const pName, Factory::ItemCallback pCallback, void* const pUserData);

    static void EventInput(const HUD::Event& event, void* const pData);
    virtual ~TransformNumberColor();

protected:

    RedColor CalculateOutputColor(float fInput) const;

public:

    bool ReadData(PblConfig& config, PblConfig::Data& data);
    void WriteData(HUD::File& file, unsigned int uTabs);

    // VTABLE: SWBF2 0x007a3290

    class Factory : public HUD::Item::Factory
    {
    public:
        Factory();

        // SYNTHETIC: SWBF2 0x00564a10
        // HUD::TransformNumberColor::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);
    };

    struct NumberColor
    {
        float number;
        RedColor color;
    };

    void Read(PblConfig& config, Factory::ItemCallback pCallback, void* const pUserData);

    NumberColor* mMapping;
    unsigned int mNumMappings;
};
}
 
 
 
 
