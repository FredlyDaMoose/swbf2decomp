#pragma once

#include <stddef.h>

#include "HUDElementGroup.h"
#include "HUDElementText.h"

namespace HUD {
class ElementMultilineText : public HUD::ElementGroup
{
public:

    enum State
    {
        state_Invalid = 0,
        state_Waiting = 1,
        state_DisablingLine = 2,
        state_Scrolling = 3,
        state_EnablingLine = 4
    };

    // VTABLE: SWBF2 0x007a14b8

    class __declspec(align(16)) Line : public HUD::ElementText
    {
    public:
        // FUNCTION: SWBF2 0x00557120
        Line();

        // SYNTHETIC: SWBF2 0x00558010
        // HUD::ElementMultilineText::Line::`vector deleting destructor'

        // FUNCTION: SWBF2 0x00557210
        virtual ~Line();

        void SetGroup(HUD::ElementMultilineText* const pGroup) { mGroup = pGroup; }

        Line& operator=(const Line& rhs)
        {
            HUD::ElementText::operator=(rhs);
            mGroup = rhs.mGroup;
            return *this;
        }

    protected:

        virtual bool SetProperty(const unsigned int uHashID, const void* pValue);

    public:
        HUD::ElementMultilineText* mGroup;
    };

    // VTABLE: SWBF2 0x007a1464

    class Factory : public HUD::ElementGroup::Factory
    {
    public:
        // FUNCTION: SWBF2 0x00557220
        Factory();

        // SYNTHETIC: SWBF2 0x005573c0
        // HUD::ElementMultilineText::Factory::`scalar deleting destructor'

        // SYNTHETIC: SWBF2 0x005573f0
        // HUD::ElementMultilineText::Factory::~Factory

        // FUNCTION: SWBF2 0x00557190
        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);

        Property mNumLines;
        Property mDisplayTime;
        Property mAlwaysScroll;
        Property mDisableOnEmpty;
        Property mScrollSpeed;
        Property mEventText;
        Property mEventColor;
        Property mAddToTop;
    };

    // FUNCTION: SWBF2 0x00557460
    ElementMultilineText(const char* const pName, Factory::ItemCallback pCallback,
                         void* const pUserData);
    // FUNCTION: SWBF2 0x00557630
    virtual ~ElementMultilineText();
    // SYNTHETIC: SWBF2 0x00557600
    // HUD::ElementMultilineText::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x00558680
    virtual void Read(PblConfig& config, Factory::ItemCallback pCallback, void* const pUserData);
    // FUNCTION: SWBF2 0x00558480
    virtual bool SetProperty(const unsigned int uHashID, const void* pValue);
    // FUNCTION: SWBF2 0x00558580
    virtual bool GetProperty(const unsigned int uHashID, void* pValue);
    // FUNCTION: SWBF2 0x005579a0
    virtual void Update(const float fElapsed);

    // FUNCTION: SWBF2 0x00558070
    bool AddText(const unsigned short* const pText, const RedColor* const pColor);
    // FUNCTION: SWBF2 0x00557e10
    void RefreshFormatting(bool bAddToGroup);

protected:

    // FUNCTION: SWBF2 0x00558760
    virtual bool ReadData(PblConfig& config, PblConfig::Data& data);
    // FUNCTION: SWBF2 0x005588b0
    virtual void PostReadSetup();
    // FUNCTION: SWBF2 0x00558920
    virtual void WriteData(HUD::File& file, unsigned int uTabs);

    // FUNCTION: SWBF2 0x005577b0
    unsigned int GetNextPendingLineIndex() const;
    // FUNCTION: SWBF2 0x005577e0
    unsigned int GetEldestLineIndex(unsigned int uIndex) const;
    // FUNCTION: SWBF2 0x00557810
    Line* GetTop();
    // FUNCTION: SWBF2 0x00557840
    Line* GetBottom();
    // FUNCTION: SWBF2 0x00557870
    bool TopLineAtTop();

    // FUNCTION: SWBF2 0x005578a0
    bool BottomLineOutOfBottomRegion(float fHeight, float fLineHeight);
    // FUNCTION: SWBF2 0x005578d0
    void EnableNextLine(unsigned int uIndex);
    // FUNCTION: SWBF2 0x005576e0
    void DisableAndScroll(Line& line, unsigned int uIndex);
    // FUNCTION: SWBF2 0x00557d30
    void RefreshTextBoxSize(unsigned int uLineHeight, unsigned int uWidth, unsigned int uHeight);
    // FUNCTION: SWBF2 0x00557d70
    void FreeLines();
    // FUNCTION: SWBF2 0x00557f10
    void SetupLines(unsigned int uNumLines);

    static void EventText(const HUD::Event& event, void* const pData);
    static void EventColor(const HUD::Event& event, void* const pData);

public:
    Line* mLines;
    unsigned int mNumLines;
    unsigned int mMaxDisplayLines;
    unsigned int mLinesPending;
    unsigned int mNumLinesDisplayed;
    unsigned int mNextLine;
    float mDisplayInterval;
    Line mFormat;
    EventHandler mEventText;
    EventHandler mEventColor;
    State mState;
    float mScrollSpeed;
    bool mAlwaysScroll : 1;
    bool mAddToTop : 1;
    bool mDisableOnEmpty : 1;
};
}

 
 
 

 
 
 
 
 
 
 
 
 
 
