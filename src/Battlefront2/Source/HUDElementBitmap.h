#pragma once

#include <stddef.h>

#include "HUDElement.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Interface/RedMaskedBitmapElement.h"

class RedGroupElement;

namespace HUD {

class ElementBitmapBase : public HUD::Element
{
public:

    // VTABLE: SWBF2 0x007a0c78

    class Factory : public HUD::Element::Factory
    {
    public:
        // FUNCTION: SWBF2 0x0054ccd0
        Factory(unsigned int uHashID);

        // SYNTHETIC: SWBF2 0x0054cef0
        // HUD::ElementBitmapBase::Factory::`scalar deleting destructor'

        static char* sBitmapStyleStrings[2];
        static char* sBitmapHAlignStrings[3];
        static char* sBitmapVAlignStrings[3];

        Property mBitmap;
        Property mBitmapStyle;
        Property mBitmapWidthHeight;
        Property mRelativeWidthHeight;
        Property mBitmapHAlign;
        Property mBitmapVAlign;
        Property mTexCoords;
        Property mEventBitmap;
    };

    ElementBitmapBase(RedBitmapElement& element, const char* const pName,
                      Factory::ItemCallback pCallback, void* const pUserData);
    virtual ~ElementBitmapBase();

    virtual bool SetProperty(const unsigned int uHashID, const void* const pValue);
    virtual bool GetProperty(const unsigned int uHashID, void* const pValue);
    virtual const char* PropertyTranslateEnum(const unsigned int uHashID,
                                              const unsigned int uValue);
    virtual void GetSize(float& rfWidth, float& rfHeight);

    void SetRect(float fX, float fY, RedBitmapElement::BitmapHAlignT eHAlign,
                 RedBitmapElement::BitmapVAlignT eVAlign);

    ElementBitmapBase& operator=(const ElementBitmapBase& rhs);

protected:
    virtual bool ReadData(PblConfig& config, PblConfig::Data& data);
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File& file, unsigned int uTabs);

    static void EventBitmap(const HUD::Event& event, void* const pData);

public:

    HUD::EventHandler mEventBitmap;
    char* mBitmapName;
    unsigned char mRelativeMode;
    unsigned char mHAlign;
    unsigned char mVAlign;
};

class ElementBitmap : public HUD::ElementBitmapBase, public RedBitmapElement
{
public:

    // VTABLE: SWBF2 0x007a0968

    class Factory : public HUD::ElementBitmapBase::Factory
    {
    public:
        Factory();

        // SYNTHETIC: SWBF2 0x0054a610
        // HUD::ElementBitmap::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);
    };

    ElementBitmap(const char* const pName = 0, Factory::ItemCallback pCallback = 0,
                  void* const pUserData = 0);
    virtual ~ElementBitmap();
    virtual void* GetParent();

    void SetTexture(const char* pName);
    virtual void SetTexture(unsigned int uiTextureHash);
    void CopyToBitmaps(RedBitmapElement* const pBitmaps, unsigned int uCount,
                       RedGroupElement* const pGroup, bool bCopyTexture);

    ElementBitmap& operator=(const ElementBitmap& rhs)
    {
        HUD::ElementBitmapBase::operator=(rhs);
        return *this;
    }
};

class ElementBitmapMasked : public HUD::ElementBitmapBase, public RedMaskedBitmapElement
{
public:

    // VTABLE: SWBF2 0x007a0c6c

    class Factory : public HUD::ElementBitmapBase::Factory
    {
    public:
        // FUNCTION: SWBF2 0x0054deb0
        Factory();

        // SYNTHETIC: SWBF2 0x0054df20
        // HUD::ElementBitmapMasked::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);

        Property mBitmapMask;
    };

    ElementBitmapMasked(const char* const pName = 0, Factory::ItemCallback pCallback = 0,
                        void* const pUserData = 0);
    virtual ~ElementBitmapMasked();

    virtual bool SetProperty(const unsigned int uHashID, const void* const pValue);
    virtual bool GetProperty(const unsigned int uHashID, void* const pValue);

    virtual void* GetParent();
    virtual void SetTexture(unsigned int uiTextureHash);

    void SetMaskTexture(const char* pName);
    void SetMaskTexture(unsigned int uHash);

protected:
    virtual bool ReadData(PblConfig& config, PblConfig::Data& data);
    virtual void WriteData(HUD::File& file, unsigned int uTabs);

public:
    char* mMaskTextureName;
};
}

 
 
 
 
 
 
 
 
 
 
 
 
 
 
