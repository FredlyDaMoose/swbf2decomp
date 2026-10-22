#pragma once

#include <stddef.h>

#include "HUDElementBar.h"
#include "HUDElementBitmap.h"

namespace HUD {

class ElementBarBitmap : public HUD::ElementBitmap, public HUD::ElementBar
{
public:

    ElementBarBitmap(const char* const pName, HUD::Item::Factory::ItemCallback pCallback,
                     void* const pUserData);

    virtual ~ElementBarBitmap();

    virtual float SetValue(const float fValue);

    virtual bool SetProperty(const unsigned int uHashID, const void* const pValue);
    virtual bool GetProperty(const unsigned int uHashID, void* const pValue);
    virtual void Update(const float fElapsed);
    virtual void GetSize(float& rfWidth, float& rfHeight);

    void RefreshFlashyBit();

protected:

    virtual bool ReadData(PblConfig& config, PblConfig::Data& data);
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File& file, unsigned int uTabs);

    static void EventDisable(const HUD::Event& event, void* const pData);

public:

    // VTABLE: SWBF2 0x007a0864

    class Factory : public HUD::ElementBitmap::Factory, public HUD::ElementBar::Factory
    {
    public:
        // FUNCTION: SWBF2 0x0054a800
        Factory();

        // SYNTHETIC: SWBF2 0x0054a960
        // HUD::ElementBarBitmap::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);

        Property mScaleSize;
        Property mScaleTexture;
        Property mFlashyScale;
        Property mFlashyIncFadeOutTime;
        Property mFlashyDecFadeOutTime;
    };

    HUD::ElementBitmap mFlashyBit;
    float mFlashyScale;
    float mFlashyIncFadeOutTime;
    float mFlashyDecFadeOutTime;
    float mBarWidth;
    float mBarU1;
    bool mScaleTex : 1;
    bool mScaleSize : 1;
    bool mIncreasedValue : 1;
    bool mInitialValueSet : 1;
};
}

 
 
 
 
 
 
 
 
