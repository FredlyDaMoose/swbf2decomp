#pragma once

#include <stddef.h>

#include "HUDElement.h"
#include "HUDEvent.h"
#include "RedEngineFL/Interface/RedTextElement.h"

namespace HUD {
class ElementText : public HUD::Element, public RedTextElement
{
public:

    // VTABLE: SWBF2 0x007a1af4

    class Factory : public HUD::Element::Factory
    {
    public:
        // FUNCTION: SWBF2 0x0055ae30
        Factory();

        // SYNTHETIC: SWBF2 0x0055b0f0
        // HUD::ElementText::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);

        static char* sTextBreak[4];
        static char* sTextClip[1];
        static char* sTextAlignHorizontal[3];
        static char* sTextAlignVertical[4];
        static char* sTextStyle[3];

        Property mTextBox;
        Property mTextBreak;
        Property mTextClip;
        Property mTextHAlignment;
        Property mTextVAlignment;
        Property mTextScale;
        Property mTextCharacterSpacing;
        Property mTextTabSpacing;
        Property mEventText;
        Property mEventNumber;
        Property mTextStyle;
        Property mPercent;
        Property mInfiniteDashes;
    };

    // FUNCTION: SWBF2 0x0055b1c0
    ElementText(const char* const pName = 0, Factory::ItemCallback pCallback = 0,
                void* const pUserData = 0);

    // FUNCTION: SWBF2 0x0055bb30
    void CopyToTextElement(RedTextElement& textElement);

    virtual ~ElementText();
    virtual bool SetProperty(const unsigned int, const void*);

    virtual bool GetProperty(const unsigned int, void*);
    virtual const char* PropertyTranslateEnum(const unsigned int, const unsigned int);
    // FUNCTION: SWBF2 0x0055bc80
    virtual bool ReadData(PblConfig& config, PblConfig::Data& data);
    virtual void WriteData(HUD::File&, unsigned int);
    virtual void GetSize(float& fWidth, float& fHeight);

    static void EventText(const HUD::Event& event, void* const pData);
    static void EventNumber(const HUD::Event& event, void* const pData);

    // FUNCTION: SWBF2 0x0055c090
    static bool CopyString(char** ppDest, const char* const pSource);

    // FUNCTION: SWBF2 0x0055c0e0
    static void FreeString(char** ppString);
    // FUNCTION: SWBF2 0x0055b980
    void SaveTextBoxText(unsigned short** ppText);
    // FUNCTION: SWBF2 0x0055b9e0
    void RestoreTextBoxText(unsigned short** ppText);

    // FUNCTION: SWBF2 0x0055ba20
    ElementText& operator=(const ElementText& other);

    virtual void* GetParent();

    EventHandler mEventText;
    EventHandler mEventNumber;
    char* mIntegerFormat;
    char* mFloatFormat;
    char* mNumberToTime;
    char* mLocalizedText;
    char* mFontName;
    bool mPercent : 1;
    bool mShowInfiniteDashes : 1;
};
}

 
 
 
 
 
 
 
 
 
