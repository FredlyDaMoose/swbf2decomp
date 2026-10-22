#pragma once

#include <stddef.h>
#include "HUDTransformNumberColor.h"

namespace HUD {
// VTABLE: SWBF2 0x007a3668
class TransformNumberColorBlend : public HUD::TransformNumberColor
{
public:

    TransformNumberColorBlend(const char* const pName, Factory::ItemCallback pCallback, void* const pUserData);
    static void EventInput(const Event& event, void* const pData);
    static void EventBlend(const Event& event, void* const pData);
    static void EventAlpha(const Event& event, void* const pData);

    enum BlendMode
    {
        blendMode_Invalid  = 0,
        blendMode_Alpha    = 1,
        blendMode_Additive = 2
    };

    virtual ~TransformNumberColorBlend();

    bool ReadData(PblConfig& config, PblConfig::Data& data);

    void WriteData(HUD::File& file, unsigned int uTabs);

    static char* sBlendMode[2];

    // VTABLE: SWBF2 0x007a32d8

    class Factory : public HUD::Item::Factory
    {
    public:
        Factory();

        // SYNTHETIC: SWBF2 0x00564ac0
        // HUD::TransformNumberColorBlend::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);
    };

    EventHandler mEventBlend;
    EventHandler mEventAlpha;
    char mBlendColor[4];
    BlendMode mBlendMode;
    unsigned char mAlpha;
};
}
 
 
 
 
 
