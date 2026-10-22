#pragma once

#include <stddef.h>
#include "HUDElement.h"
#include "RedEngineFL/Interface/RedGroupElement.h"

namespace HUD { class Item; }

namespace HUD {
// VTABLE: SWBF2 0x007a381c
class ViewPort : public HUD::Element, public RedGroupElement
{
public:

    // VTABLE: SWBF2 0x007a38a8

    class Factory : public HUD::Element::Factory
    {
    public:
        // FUNCTION: SWBF2 0x00568b00
        Factory();

        // SYNTHETIC: SWBF2 0x00568c70
        // HUD::ViewPort::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);

        Property mViewport0Position;
        Property mViewport1Position;
        Property mViewport2Position;
        Property mViewport3Position;
        Property mViewport0UseSafezone;
        Property mViewport1UseSafezone;
        Property mViewport2UseSafezone;
        Property mViewport3UseSafezone;
    };

    ViewPort(const char* const pName, Factory::ItemCallback pCallback, void* const pUserData);
    virtual ~ViewPort();

    virtual void Read(PblConfig& config, Factory::ItemCallback pCallback, void* const pUserData);
    virtual bool SetProperty(const unsigned int uHashID, const void* const pValue);
    virtual bool GetProperty(const unsigned int uHashID, void* const pValue);
    virtual const char* PropertyTranslateEnum(const unsigned int uHashID, const unsigned int uValue);
    virtual void GetSize(float& fWidth, float& fHeight);

protected:
    virtual bool ReadData(PblConfig& config, PblConfig::Data& data);
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File& file, unsigned int uTabs);

    void FreeEventFilter();
    static void ItemConstructionCallback(HUD::Item* const pItem, void* const pUserData);

public:
    char* mEventFilter;
    unsigned char mCurViewPort;
    HUD::Item** mItems;
    unsigned int mNumItems;
    unsigned char mViewportPosSet;
};
}
 
 
 
 
 
 

 
 
 
