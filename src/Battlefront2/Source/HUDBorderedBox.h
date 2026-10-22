#pragma once

#include <stddef.h>

#include "HUDElementGroup.h"
#include "RedEngineFL/Interface/RedBorderElement.h"

namespace HUD {
class BorderedBox : public HUD::ElementGroup
{
public:

    // VTABLE: SWBF2 0x007a0388

    class Factory : public HUD::ElementGroupBase::Factory
    {
    public:
        // FUNCTION: SWBF2 0x005442c0
        Factory();

        // SYNTHETIC: SWBF2 0x00544360
        // HUD::BorderedBox::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);

        Property mBorder;
        Property mBackground;
    };

    // FUNCTION: SWBF2 0x005443a0
    BorderedBox(const char* const pName, Factory::ItemCallback pCallback,
                void* const pUserData);

    virtual ~BorderedBox();
    virtual bool SetProperty(const unsigned int, const void*);
    virtual bool GetProperty(const unsigned int, void*);
    virtual bool ReadData(PblConfig&, PblConfig::Data&);
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File&, unsigned int);

    void SetupElements();

    void SetTexture(const char* const pName);
    void SetBorder(float fBorderWidth, float fBorderHeight);

    char* mTextureName;
    float mBorderWidth;
    float mBorderHeight;
    unsigned int mTextureHash;

    RedBorderElement mBorderElement;
};
}

 
 
 
 
 
 
 
