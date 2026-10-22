#pragma once

#include <stddef.h>

#include "RedGlyphCacheElement.h"
#include "RedEngineFL/Util/ext/basevector.h"

#include "RedEngineFL/Util/ext/hashtable.h"

// VTABLE: SWBF2 0x007731a4

class RedTextElement : public RedGlyphCacheElement
{
public:

    enum TextHAlignT
    {
        TEXT_HALIGN_LEFT = 0,
        TEXT_HALIGN_RIGHT = 1,
        TEXT_HALIGN_CENTER = 2
    };

    enum TextVAlignT
    {
        TEXT_VALIGN_TOP = 0,
        TEXT_VALIGN_BOTTOM = 1,
        TEXT_VALIGN_CENTER = 2,
        TEXT_VALIGN_BASELINE = 3
    };

    enum TextBreakT
    {
        TEXT_BREAK_NONE = 0,
        TEXT_BREAK_WORD = 1,
        TEXT_BREAK_NONE_REFORMAT = 2,
        TEXT_BREAK_WORD_REFORMAT = 3
    };

    enum TextClipT
    {
        TEXT_CLIP_CHARACTER = 0
    };

    struct TextLine
    {
        unsigned short* pcStart;
        unsigned short* pcEnd;
        int iWidth;
        bool bClipped;
    };

    // FUNCTION: SWBF2 0x006d72f0
    RedTextElement();

    RedTextElement(const char* pcName);

    // SYNTHETIC: SWBF2 0x006d75b0
    // RedTextElement::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x006d7560
    virtual ~RedTextElement();

    // FUNCTION: SWBF2 0x006d7c20
    virtual void Enable(bool bEnable);

    // FUNCTION: SWBF2 0x006d85a0
    virtual void Visible(bool bVisible);

    // FUNCTION: SWBF2 0x006d8260
    void SetIntercharacterSpacing(int iSpacing);
    // FUNCTION: SWBF2 0x006d8420
    void SetTextAlign(TextHAlignT eHAlign, TextVAlignT eVAlign);

    // FUNCTION: SWBF2 0x006d8390
    int SetText(const unsigned short* pText, bool bFlag);
    // FUNCTION: SWBF2 0x006d8340
    int SetText(const char* pcText, bool bFlag);

    int SetTextFormattedV(const char* pcFormat, char* pArgs);

    // FUNCTION: SWBF2 0x006d80c0
    void GetTextAlign(TextHAlignT& reHAlign, TextVAlignT& reVAlign) const;
    // FUNCTION: SWBF2 0x006d80f0
    void GetTextBox(unsigned int& ruiWidth, unsigned int& ruiHeight) const;
    // FUNCTION: SWBF2 0x006d8310
    void SetTabSpacing(unsigned int uiTabSpacing);
    // FUNCTION: SWBF2 0x006d7e50
    unsigned int GetMaxLines(unsigned int uiLineHeight) const;
    // FUNCTION: SWBF2 0x006d7e80
    void GetRect(float& rfLeft, float& rfTop, float& rfRight, float& rfBottom);

    void SetTextBox(unsigned int uiWidth, unsigned int uiHeight);

    void SetTextScale(float fScaleX, float fScaleY);

    void SetTextBreak(TextBreakT eBreak);

    void SetTextClip(TextClipT eClip);
    int SetTextFormatted(const char* pFormat, ...);

    // FUNCTION: SWBF2 0x00525f30
    virtual bool IsText() const { return true; }

    virtual void CreateHotSpot(float fRadius);

    virtual void Cleanup();

protected:
    // FUNCTION: SWBF2 0x006d8150
    virtual void RenderUsingContext(const PblMatrix& mContext, const RedColor& color);

private:

    // FUNCTION: SWBF2 0x006d85f0
    int WriteCache(const unsigned short* pcText);

    // FUNCTION: SWBF2 0x006d7620
    void ClearCache();

public:

    unsigned int ClipString(const unsigned short* const pcText,
                            TextLine* const aLines,
                            unsigned int& ruiMaxLines,
                            unsigned int& ruiNumLines,
                            const unsigned short*& rpcEnd,
                            unsigned int uiTextHeight,
                            short& rsCursorX);

    // FUNCTION: SWBF2 0x006d8060
    unsigned int GetTabSpacing() const;

    // FUNCTION: SWBF2 0x006d8110
    unsigned int GetTextHeight() const;

    unsigned char m_bUnknown0090;

    unsigned short m_uiWidth;
    unsigned short m_uiHeight;

    float m_fScaleX;
    float m_fScaleY;
    short m_iTextExtent;
    unsigned short m_uiNumLines;

    unsigned short m_uiMaxLineExtent;

    short m_leading;
    short m_iSpacing;
    unsigned short m_uiTabSpacing;
    unsigned short m_uiCursorPos;

    unsigned short m_halign : 2;
    unsigned short m_valign : 2;
    unsigned short m_break : 2;
    unsigned short m_clip : 2;
    unsigned short m_dirtyGlyphCache : 1;
    unsigned short z_reserved : 8;

    util::Vector<unsigned short,
                 util::BaseArray<unsigned short,
                                 util::TNewArrayAlloc<unsigned short> > > m_text;

    static util::HashTable<unsigned short, unsigned int, util::NewAllocator>*
        GetMagicCharMap();

    static void ClearMagicCharMap();
};

 
 
 
 
 
 
 
 
 
 
 
 
 
 

 
