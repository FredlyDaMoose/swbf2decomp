#pragma once

#include <stddef.h>

#include "HUDElementBarBitmap.h"

namespace HUD {
class ProceduralBarBitmap : public HUD::ElementBarBitmap
{
public:

    // VTABLE: SWBF2 0x007a3520

    class Factory : public HUD::ElementBarBitmap::Factory
    {
    public:
        // FUNCTION: SWBF2 0x005667d0
        Factory();

        // SYNTHETIC: SWBF2 0x00566940
        // HUD::ProceduralBarBitmap::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);

        Property mGlowAlphaBase;
        Property mGlowAlphaScale;
        Property mNoiseFreq;
        Property mNoiseRoughness;
        Property mGlowScaleSize;
    };

    ProceduralBarBitmap(const char* const pName, Factory::ItemCallback pCallback, void* const pUserData);

    virtual bool SetProperty(const unsigned int uHashID, const void* const pValue);
    virtual bool GetProperty(const unsigned int uHashID, void* const pValue);

    virtual float SetValue(const float fValue);

    virtual void SetRect(float fLeft, float fTop, float fRight, float fBottom);
    virtual void SetRect(float fWidth, float fHeight, RedBitmapElement::BitmapHAlignT eHAlign,
                         RedBitmapElement::BitmapVAlignT eVAlign);

    void SetRect_(float fWidth, float fHeight);

protected:

    virtual bool ReadData(PblConfig& config, PblConfig::Data& data);
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File& file, unsigned int uTabs);
    virtual void OnRender();

public:

    RedBitmapElement::BitmapHAlignT halign_;
    RedBitmapElement::BitmapVAlignT valign_;
    float width_;
    float height_;
    float value_;
    float mGlowAlphaBase;
    float mGlowAlphaScale;
    float mSavedAlpha;
    float mNoiseFreq;
    float mNoiseRoughness;
    float mGlowScaleSize;
    float mGlowScaleBase;
};
}

 
 
 
 
 
 
 
