#pragma once

#include <stddef.h>
#include "HUDTransformNumberColor.h"
#include "PebbleFL/Common/PblVector.h"

namespace HUD {
class TransformNumberVector3 : public HUD::TransformNumberType
{
public:

    TransformNumberVector3(const char* const pName, Factory::ItemCallback pCallback, void* const pUserData);

    static void EventInput(const HUD::Event& event, void* const pData);
    virtual ~TransformNumberVector3();

    // VTABLE: SWBF2 0x007a32e4

    class Factory : public HUD::Item::Factory
    {
    public:
        Factory();

        // SYNTHETIC: SWBF2 0x00564c20
        // HUD::TransformNumberVector3::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);
    };

    struct NumberVector3
    {
        float number;
        PblVector3 vector3;
    };

    void Read(PblConfig& config, Factory::ItemCallback pCallback, void* const pUserData);

protected:

    PblVector3 CalculateOutputVector3(float fInput) const;

    bool ReadData(PblConfig& config, PblConfig::Data& data);
    void WriteData(HUD::File& file, unsigned int uTabs);

public:
    NumberVector3* mMapping;
    unsigned int mNumMappings;
};
}
 
 
 
