#pragma once

#include <stddef.h>

#include "HUDElement.h"
#include "HUDEvent.h"
#include "RedEngineFL/Interface/RedGroupElement.h"

namespace HUD {

class ElementGroupBase : public HUD::Element
{
public:

    // VTABLE: SWBF2 0x007a0df0

    class Factory : public HUD::Element::Factory
    {
    public:
        // FUNCTION: SWBF2 0x0054e1c0
        Factory(unsigned int uHashID);

        // SYNTHETIC: SWBF2 0x0054e380
        // HUD::ElementGroupBase::Factory::`scalar deleting destructor'

        static char* sHAlignStrings[3];

        static char* sVAlignStrings[3];

        Property mRect;
        Property mRelativeSize;
        Property mHAlign;
        Property mVAlign;
        Property mPropagateAlpha;
        Property mEventPosition;
        Property mEventScale;
        Property mEventAlpha;
        Property mEventRotation;
    };

    enum HAlign
    {
        hAlign_left = 0,
        hAlign_center = 1,
        hAlign_right = 2
    };
    enum VAlign
    {
        vAlign_top = 0,
        vAlign_center = 1,
        vAlign_bottom = 2
    };

    ElementGroupBase(RedGroupElement& group, const char* const pName,
                     Factory::ItemCallback pCallback, void* const pUserData);

    virtual ~ElementGroupBase();
    // SYNTHETIC: SWBF2 0x0054e490
    // HUD::ElementGroupBase::`scalar deleting destructor'

    virtual void GetSize(float& fWidth, float& fHeight);

    virtual void Read(PblConfig& config, Factory::ItemCallback pCallback,
                      void* const pUserData);
    virtual bool SetProperty(const unsigned int, const void*);
    virtual bool GetProperty(const unsigned int, void*);
    virtual const char* PropertyTranslateEnum(const unsigned int, const unsigned int);
    virtual bool ReadData(PblConfig&, PblConfig::Data&);
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File&, unsigned int);

    virtual bool ReadChild(PblConfig& config, PblConfig::Data& data,
                           Factory::ItemCallback pCallback, void* const pUserData);

    bool AddElement(HUD::Element* const pElement);
    void RemoveElement(HUD::Element* const pElement);
    void RemoveAllElements();
    unsigned int GetWidthPixels() const;
    unsigned int GetHeightPixels() const;
    float GetWidthPixelsFloat() const;
    float GetHeightPixelsFloat() const;

protected:

    void SetAlignment(HAlign eHAlign, VAlign eVAlign, bool bReapply, bool bAdd, bool bStore);

    static void EventPosition(const HUD::Event& event, void* const pData);
    static void EventScale(const HUD::Event& event, void* const pData);

    static void EventAlpha(const HUD::Event& event, void* const pData);
    static void EventRotation(const HUD::Event& event, void* const pData);

public:

    float mWidth;
    float mHeight;
    EventHandler mEventPosition;
    EventHandler mEventScale;
    EventHandler mEventAlpha;
    EventHandler mEventRotation;
    unsigned char mRelativeMode;
    unsigned char mHAlign;
    unsigned char mVAlign;
    PblListDouble mElementList;
    bool mShowChildren : 1;
};

class ElementGroup : public HUD::ElementGroupBase, public RedGroupElement
{
public:

    // VTABLE: SWBF2 0x007a0a28

    class Factory : public HUD::ElementGroupBase::Factory
    {
    public:
        Factory();

        // SYNTHETIC: SWBF2 0x0054b830
        // HUD::ElementGroup::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);
    };

    ElementGroup(const char* const pName, Factory::ItemCallback pCallback,
                 void* const pUserData);
    virtual ~ElementGroup();
    virtual void* GetParent();
};
}

 
 
 
 
 
 
 
 
 
