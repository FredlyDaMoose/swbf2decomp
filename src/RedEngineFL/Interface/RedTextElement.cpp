#include "RedTextElement.h"

#include "RedFont.h"
#include "RedGlyphMemoryManager.h"
#include "RedEngineFL/Util/ext/types.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "PebbleFL/Common/PblHashTable.h"
#include <stdarg.h>

// FUNCTION: SWBF2 0x006d8510
int RedTextElement::SetTextFormattedV(const char* pcFormat, char* pArgs)
{
    unsigned short buffer[1024];
    PblUnicode::uvsnprintf(buffer, 0x3ff, pcFormat, pArgs);
    return SetText(buffer, true);
}

// FUNCTION: SWBF2 0x006d84f0
int RedTextElement::SetTextFormatted(const char* pcFormat, ...)
{
    va_list args;
    va_start(args, pcFormat);
    int result = SetTextFormattedV(pcFormat, args);
    va_end(args);
    return result;
}

unsigned short CURSOR_CHAR = 124;
float CURSOR_X = -0.5f;
float CURSOR_Y;

util::HashTable<unsigned short, unsigned int, util::NewAllocator>* mMagicCharMap_;
bool sFindReplaceError;

unsigned int FindReplaceMagicStrings_(const unsigned short* const pcSource,
                                      unsigned short* pcDest);

// FUNCTION: SWBF2 0x0045caf0
template const float& util::Min_<float>(const float&, const float&);

// FUNCTION: SWBF2 0x006d72f0
RedTextElement::RedTextElement()
{
    m_uiWidth = 0;
    m_uiHeight = 0;

    m_iTextExtent = 0;
    m_uiNumLines = 0;

    m_bUnknown0090 = 1;

    m_fScaleX = 1.0f;
    m_fScaleY = 1.0f;

    m_leading = 0;
    m_iSpacing = 1;

    m_uiTabSpacing = 0;
    m_uiCursorPos = 0xffff;

    m_halign = TEXT_HALIGN_LEFT;
    m_valign = TEXT_VALIGN_TOP;
    m_break = TEXT_BREAK_WORD;
    m_clip = TEXT_CLIP_CHARACTER;
    m_dirtyGlyphCache = 0;

    m_text.SetSize(32);
}

// FUNCTION: SWBF2 0x006d7560
RedTextElement::~RedTextElement()
{
}

// FUNCTION: SWBF2 0x006d7620
void RedTextElement::ClearCache()
{
    m_iTextExtent = 0;
    m_uiNumLines = 0;
    m_uiMaxLineExtent = 0;

    Clear();
}

// FUNCTION: SWBF2 0x006d7670
unsigned int RedTextElement::ClipString(const unsigned short* const pcText,
                                        TextLine* const aLines,
                                        unsigned int& ruiMaxLines,
                                        unsigned int& ruiNumLines,
                                        const unsigned short*& rpcEnd,
                                        unsigned int uiTextHeight,
                                        short& rsCursorX)
{
    const unsigned int eBreak = m_break;
    const bool bWordBreak = eBreak == TEXT_BREAK_WORD || eBreak == TEXT_BREAK_WORD_REFORMAT;
    const bool bReformat = eBreak == TEXT_BREAK_NONE_REFORMAT || eBreak == TEXT_BREAK_WORD_REFORMAT;

    const unsigned int uiTab = (unsigned short)GetTabSpacing();
    unsigned int uiLine = 0;

    ruiNumLines = 0;
    if (m_uiHeight)
    {
        ruiMaxLines = GetMaxLines(uiTextHeight);
    }

    bool bLastWasSpace = bReformat;
    const unsigned short* pcBreak = 0;
    short sBreakX = 0;
    short sLineStartX = 0;
    const unsigned short* pcLineStart = pcText;
    aLines[0].pcStart = (unsigned short*)pcText;
    rsCursorX = 0;
    bool bFull = false;

    for (rpcEnd = pcText; *rpcEnd; ++rpcEnd)
    {
        short sWidth = 0;
        short sAdvance = 0;
        bool bSpace = false;
        bool bNewline = false;

        const unsigned short c = *rpcEnd;
        if (c == '\n')
        {
            bNewline = true;
            bSpace = true;
        }
        else if (c == '\t')
        {
            bSpace = true;
            sWidth = (short)(unsigned short)(uiTab - rsCursorX % (short)uiTab);
            sAdvance = sWidth;
        }
        else
        {
            const RedFont::Glyph* pGlyph = m_pFont->FindGlyph(c);
            if (pGlyph)
            {
                sWidth = (short)(int)(pGlyph->width * m_fScaleX + 0.5f);
                sAdvance = (short)(sWidth + m_iSpacing);
                bSpace = !m_pFont->IsPrintingChar(*rpcEnd);
            }
        }

        if (bReformat && bSpace)
        {
            if (bLastWasSpace)
            {
                continue;
            }

            const RedFont::Glyph* pSpace = m_pFont->FindGlyph(' ');
            const int iWidth = pSpace ? (int)(pSpace->width * m_fScaleX + 0.5f)
                                      : (int)(uiTextHeight >> 1);
            sWidth = (short)iWidth;
            sAdvance = (short)(iWidth + m_iSpacing);
        }

        m_iTextExtent += sAdvance;

        if (bFull)
        {
            continue;
        }

        if (bSpace)
        {
            if (!bLastWasSpace)
            {
                pcBreak = rpcEnd;
                sBreakX = rsCursorX;
            }
        }
        else if (bLastWasSpace)
        {
            if (!aLines[uiLine].pcStart)
            {
                aLines[uiLine].pcStart = (unsigned short*)rpcEnd;
                rsCursorX = 0;
            }
            pcLineStart = rpcEnd;
            sLineStartX = rsCursorX;
        }
        bLastWasSpace = bSpace;

        if (bNewline)
        {
            TextLine& rLine = aLines[uiLine];
            rLine.pcEnd = (unsigned short*)rpcEnd;
            if (rLine.pcStart)
            {
                rLine.iWidth = rsCursorX;
                rLine.bClipped = m_uiWidth && rsCursorX > (int)m_uiWidth;
            }
            else
            {
                rLine.pcStart = (unsigned short*)rpcEnd;
                rLine.iWidth = 0;
                rLine.bClipped = false;
            }
            pcLineStart = rpcEnd + 1;
            rsCursorX = 0;

            if (rLine.iWidth > 0xffff)
            {
                m_uiMaxLineExtent = 0xffff;
            }
            else if (rLine.iWidth > 0 && m_uiMaxLineExtent < (unsigned short)rLine.iWidth)
            {
                m_uiMaxLineExtent = (unsigned short)rLine.iWidth;
            }

            uiLine = ++ruiNumLines % ruiMaxLines;
            if (ruiNumLines == ruiMaxLines && m_valign != TEXT_VALIGN_BOTTOM)
            {
                bFull = true;
            }
            else
            {
                aLines[uiLine].pcStart = (unsigned short*)pcLineStart;
            }
        }
        else if (bWordBreak && m_uiWidth && sWidth + rsCursorX > (int)m_uiWidth)
        {
            TextLine& rLine = aLines[uiLine];
            rLine.bClipped = false;
            if (!pcBreak)
            {
                rLine.pcEnd = (unsigned short*)rpcEnd;
                rLine.iWidth = rsCursorX;
                pcLineStart = rpcEnd;
                rsCursorX = 0;
            }
            else
            {
                rLine.pcEnd = (unsigned short*)pcBreak;
                rLine.iWidth = sBreakX;
                const unsigned short* pcOldBreak = pcBreak;
                pcBreak = 0;
                if (pcOldBreak < pcLineStart)
                {
                    rsCursorX -= sLineStartX;
                }
                else
                {
                    pcLineStart = 0;
                    rsCursorX = 0;
                }
            }

            if (rLine.iWidth > 0xffff)
            {
                m_uiMaxLineExtent = 0xffff;
            }
            else if (rLine.iWidth > 0 && m_uiMaxLineExtent < (unsigned short)rLine.iWidth)
            {
                m_uiMaxLineExtent = (unsigned short)rLine.iWidth;
            }

            uiLine = ++ruiNumLines % ruiMaxLines;
            if (ruiNumLines == ruiMaxLines && m_valign != TEXT_VALIGN_BOTTOM)
            {
                bFull = true;
                continue;
            }
            aLines[uiLine].pcStart = (unsigned short*)pcLineStart;
            rsCursorX += sAdvance;
        }
        else
        {
            rsCursorX += sAdvance;
        }
    }

    if (!aLines[uiLine].pcStart || bFull)
    {
        return uiLine;
    }

    aLines[uiLine].pcEnd = (unsigned short*)rpcEnd;
    aLines[uiLine].iWidth = rsCursorX;
    if (aLines[uiLine].iWidth > 0xffff)
    {
        m_uiMaxLineExtent = 0xffff;
    }
    else if (aLines[uiLine].iWidth > 0 && m_uiMaxLineExtent < (unsigned short)aLines[uiLine].iWidth)
    {
        m_uiMaxLineExtent = (unsigned short)aLines[uiLine].iWidth;
    }

    return ++ruiNumLines % ruiMaxLines;
}

// FUNCTION: SWBF2 0x006d7c20
void RedTextElement::Enable(bool bEnable)
{
    Visible(bEnable);

    m_bEnabled = bEnable;

    if (m_dirtyGlyphCache && bEnable)
    {
        WriteCache(m_text.GetData());

        if (!m_bVisible)
        {
            ClearCache();
        }
    }
}

// FUNCTION: SWBF2 0x006d8260
void RedTextElement::SetIntercharacterSpacing(int iSpacing)
{
    m_dirtyGlyphCache = 1;
    m_iSpacing = (short)iSpacing;
}

// FUNCTION: SWBF2 0x006d8420
void RedTextElement::SetTextAlign(TextHAlignT eHAlign, TextVAlignT eVAlign)
{
    m_halign = eHAlign;
    m_valign = eVAlign;
    m_dirtyGlyphCache = 1;
}

// FUNCTION: SWBF2 0x006d8460
void RedTextElement::SetTextBox(unsigned int uiWidth, unsigned int uiHeight)
{
    m_dirtyGlyphCache = 1;
    m_uiWidth = (unsigned short)uiWidth;
    m_uiHeight = (unsigned short)uiHeight;
}

// FUNCTION: SWBF2 0x006d8490
void RedTextElement::SetTextBreak(TextBreakT eBreak)
{
    m_break = eBreak;
    m_dirtyGlyphCache = 1;
}

// FUNCTION: SWBF2 0x006d84c0
void RedTextElement::SetTextClip(TextClipT eClip)
{
    m_clip = eClip;
    m_dirtyGlyphCache = 1;
}

// FUNCTION: SWBF2 0x006d8570
void RedTextElement::SetTextScale(float fScaleX, float fScaleY)
{
    m_dirtyGlyphCache = 1;
    m_fScaleX = fScaleX;
    m_fScaleY = fScaleY;
}

// FUNCTION: SWBF2 0x006d85a0
void RedTextElement::Visible(bool bVisible)
{
    const bool bWasVisible = m_bVisible != 0;

    if (!bWasVisible && bVisible)
    {
        if (m_text.GetData())
        {
            WriteCache(m_text.GetData());
        }
    }
    else if (bWasVisible && !bVisible)
    {
        ClearCache();
    }

    m_bVisible = bVisible;
}

// FUNCTION: SWBF2 0x006d85f0
int RedTextElement::WriteCache(const unsigned short* pcText)
{
    TextLine aLines[80];
    unsigned short acBuffer[1024];

    const unsigned short* pcWork = pcText;

    m_iTextExtent = 0;
    m_uiNumLines = 0;
    m_uiMaxLineExtent = 0;

    if (!m_pFont)
    {
        return 0;
    }

    Begin();
    m_dirtyGlyphCache = 0;

    if (!pcText)
    {
        End();
        return 0;
    }

    if (*pcText != 0)
    {
        m_dirtyGlyphCache = 0;
    }

    if (m_bUnknown0090 && PblUnicode::ustrlen(pcText) < 0x3ff)
    {
        const int iLength = FindReplaceMagicStrings_(pcText, acBuffer);

        if (sFindReplaceError)
        {
            sFindReplaceError = false;
            End();
            return 0;
        }

        if (iLength > 0)
        {
            pcWork = acBuffer;
        }
    }

    const unsigned int uiTabSpacing = (unsigned short)GetTabSpacing();
    const unsigned int uiTextHeight = GetTextHeight();

    unsigned int uiMaxLines = 80;
    unsigned int uiNumLines = 0;
    const unsigned short* pcClipEnd = 0;
    short sCursorX = 0;

    unsigned int uiFirstLine = ClipString(pcWork, aLines, uiMaxLines, uiNumLines,
                                          pcClipEnd, uiTextHeight, sCursorX);

    const unsigned short* pc = 0;
    int iY = 0;

    if (uiNumLines != 0)
    {
        bool bCursorDone = ((int)(pcClipEnd - pcWork) < (int)m_uiCursorPos);

        unsigned int uiLines;
        if (uiNumLines < uiMaxLines)
        {
            uiLines = (unsigned short)uiNumLines;
            uiFirstLine = 0;
        }
        else
        {
            uiLines = (unsigned short)uiMaxLines;
        }

        m_uiNumLines = (unsigned short)uiLines;

        switch (m_valign)
        {
        case TEXT_VALIGN_BOTTOM:
            iY = (int)m_uiHeight - (int)((unsigned short)uiLines * uiTextHeight);
            break;

        case TEXT_VALIGN_CENTER:
            iY = ((int)m_uiHeight - (int)((unsigned short)uiLines * uiTextHeight)) / 2;
            break;

        case TEXT_VALIGN_BASELINE:
            iY = -(int)(m_pFont->m_iAscent * m_fScaleY + 0.5f);
            break;

        default:
            iY = 0;
            break;
        }

        for (unsigned int uiLine = 0; uiLine < m_uiNumLines; uiLine++)
        {
            const unsigned int uiSlot = (uiFirstLine + uiLine) % uiMaxLines;

            int iLineX = 0;
            if (m_halign == TEXT_HALIGN_CENTER)
            {
                iLineX = ((int)m_uiWidth - aLines[uiSlot].iWidth) / 2;
            }
            else if (m_halign == TEXT_HALIGN_RIGHT)
            {
                iLineX = (int)m_uiWidth - aLines[uiSlot].iWidth;
            }

            int iX = iLineX;

            const unsigned short* pcLineEnd = aLines[uiSlot].pcEnd;

            for (pc = aLines[uiSlot].pcStart; pc < pcLineEnd; pc++)
            {
                if (!bCursorDone && pc >= pcWork + m_uiCursorPos)
                {
                    const RedFont::Glyph* pCursor = m_pFont->FindGlyph(CURSOR_CHAR);
                    if (pCursor)
                    {
                        AddGlyph(*pCursor,
                                 (float)iX - pCursor->width * 0.5f + CURSOR_X,
                                 (float)iY + CURSOR_Y,
                                 m_fScaleX, m_fScaleY);
                    }

                    bCursorDone = true;
                }

                if (*pc == 9)
                {

                    iX += (short)uiTabSpacing - (iX - iLineX) % (short)uiTabSpacing;
                    continue;
                }

                const RedFont::Glyph* pGlyph = m_pFont->FindGlyph(*pc);
                if (!pGlyph)
                {
                    continue;
                }

                const float fOne = 1.0f;
                const float fFit = (float)uiTextHeight
                                   / (float)(pGlyph->bottom - pGlyph->top)
                                   * 1.2f;

                const float fScale = util::Min_(fFit, fOne);

                const float fScaleY = fScale * m_fScaleY;
                const float fScaleX = fScale * m_fScaleX;

                const int iAdvance = (int)(pGlyph->width * fScaleX + 0.5f);

                if (m_pFont->IsPrintingChar(*pc))
                {
                    if (m_clip == 0 && m_uiWidth != 0
                        && iAdvance + iX > (int)m_uiWidth)
                    {
                        break;
                    }

                    AddGlyph(*pGlyph, (float)iX, (float)iY, fScaleX, fScaleY);
                }

                iX += m_iSpacing + iAdvance;
            }

            iY += uiTextHeight;
        }

        if (!bCursorDone && pc >= pcWork + m_uiCursorPos)
        {
            const RedFont::Glyph* pCursor = m_pFont->FindGlyph(CURSOR_CHAR);
            if (pCursor)
            {
                iY -= uiTextHeight;

                AddGlyph(*pCursor,
                         (float)sCursorX - pCursor->width * 0.5f + CURSOR_X,
                         (float)iY + CURSOR_Y,
                         m_fScaleX, m_fScaleY);
            }
        }
    }
    else
    {
        m_uiNumLines = 0;
    }

    End();

    if (m_uiNumLines == 0)
    {
        return 0;
    }

    return (int)(aLines[m_uiNumLines - 1].pcEnd - aLines[0].pcStart);
}

// FUNCTION: SWBF2 0x006d7de0
util::HashTable<unsigned short, unsigned int, util::NewAllocator>*
RedTextElement::GetMagicCharMap()
{
    if (!mMagicCharMap_)
    {
        mMagicCharMap_ = new util::HashTable<unsigned short, unsigned int,
                                             util::NewAllocator>();
    }

    return mMagicCharMap_;
}

// FUNCTION: SWBF2 0x006d7640
void RedTextElement::ClearMagicCharMap()
{
    delete mMagicCharMap_;
    mMagicCharMap_ = 0;
}

// FUNCTION: SWBF2 0x006d7c80
unsigned short FindMagicChar_(unsigned int uiHash)
{
    unsigned short c = (*mMagicCharMap_)[uiHash];
    if (c == 0)
    {
        return RedFont::GetMagicCharFontData().defaultMap_[uiHash];
    }
    return c;
}

// FUNCTION: SWBF2 0x006d7cb0
unsigned int FindReplaceMagicStrings_(const unsigned short* const pcSource,
                                      unsigned short* pcDest)
{
    unsigned short acMagic[256];
    char acMagicA[256];
    unsigned int uiCount = 0;
    bool bInMagic = false;
    unsigned short uiLen = 0;

    for (const unsigned short* pc = pcSource; *pc; ++pc)
    {
        if (*pc == '{')
        {
            bInMagic = true;
            uiLen = 0;
        }
        else if (bInMagic)
        {
            if (*pc == '}')
            {
                bInMagic = false;
                acMagic[uiLen] = 0;
                PblUnicode::ustrncnv(acMagicA, acMagic, 256);
                const unsigned short c = FindMagicChar_(PblHash(acMagicA));
                if (c)
                {
                    *pcDest++ = c;
                    ++uiCount;
                }
                else
                {
                    *pcDest++ = ' ';
                }
            }
            else
            {
                acMagic[uiLen++] = *pc;
            }
        }
        else
        {
            *pcDest++ = *pc;
        }
    }

    *pcDest = 0;
    return uiCount;
}

// FUNCTION: SWBF2 0x006d7e50
unsigned int RedTextElement::GetMaxLines(unsigned int uiLineHeight) const
{
    if (m_uiHeight > uiLineHeight && uiLineHeight)
    {
        return m_uiHeight / uiLineHeight;
    }
    return 1;
}

// FUNCTION: SWBF2 0x006d7e80
void RedTextElement::GetRect(float& rfLeft, float& rfTop, float& rfRight, float& rfBottom)
{
    if (m_dirtyGlyphCache ||
        (m_iTextExtent == 0 && m_uiNumLines == 0 && m_text.GetData() && *m_text.GetData()))
    {
        WriteCache(m_text.GetData());
    }

    rfLeft = 0.0f;
    rfTop = 0.0f;

    const float fWidth = (float)(m_uiNumLines > 1 ? m_uiMaxLineExtent : m_iTextExtent);
    if (m_halign == TEXT_HALIGN_RIGHT)
    {
        rfLeft = (float)m_uiWidth - fWidth;
    }
    else if (m_halign == TEXT_HALIGN_CENTER)
    {
        rfLeft = (float)m_uiWidth * 0.5f - fWidth * 0.5f;
    }

    const float fHeight =
        (float)(unsigned int)(m_pFont->m_uiHeight * m_fScaleY + 0.5f) * (float)m_uiNumLines;
    switch (m_valign)
    {
    case TEXT_VALIGN_BOTTOM:
        rfTop = (float)m_uiHeight - fHeight;
        break;
    case TEXT_VALIGN_CENTER:
        rfTop = (float)m_uiHeight * 0.5f - fHeight * 0.5f;
        break;
    case TEXT_VALIGN_BASELINE:
        rfTop = -(float)(int)(m_pFont->m_iAscent * m_fScaleY + 0.5f);
        break;
    }

    rfRight = rfLeft + fWidth;
    rfBottom = rfTop + fHeight;
}

// FUNCTION: SWBF2 0x006d8060
unsigned int RedTextElement::GetTabSpacing() const
{
    if (m_uiTabSpacing)
    {
        return m_uiTabSpacing;
    }

    if (m_pFont)
    {
        const RedFont::Glyph* pSpace = m_pFont->FindGlyph(' ');
        if (pSpace)
        {
            return ((int)(pSpace->width * m_fScaleX + 0.5f) + m_iSpacing) << 3;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006d80c0
void RedTextElement::GetTextAlign(TextHAlignT& reHAlign, TextVAlignT& reVAlign) const
{
    reHAlign = (TextHAlignT)m_halign;
    reVAlign = (TextVAlignT)m_valign;
}

// FUNCTION: SWBF2 0x006d80f0
void RedTextElement::GetTextBox(unsigned int& ruiWidth, unsigned int& ruiHeight) const
{
    ruiWidth = m_uiWidth;
    ruiHeight = m_uiHeight;
}

// FUNCTION: SWBF2 0x006d8110
unsigned int RedTextElement::GetTextHeight() const
{
    return (unsigned int)(m_pFont->m_uiHeight * m_fScaleY + 0.5f) + m_leading;
}

// FUNCTION: SWBF2 0x006d8150
void RedTextElement::RenderUsingContext(const PblMatrix& mContext, const RedColor& color)
{
    if (color.a)
    {
        if (m_dirtyGlyphCache)
        {
            WriteCache(m_text.GetData());
        }
        RedGlyphCacheElement::RenderUsingContext(mContext, color);
    }
}

// FUNCTION: SWBF2 0x006d8280
template bool util::Vector<unsigned short,
                           util::BaseArray<unsigned short,
                                           util::TNewArrayAlloc<unsigned short> > >::SetSizeGrow(int);

// FUNCTION: SWBF2 0x006d8310
void RedTextElement::SetTabSpacing(unsigned int uiTabSpacing)
{
    ClearCache();
    m_dirtyGlyphCache = 1;
    m_uiTabSpacing = (unsigned short)uiTabSpacing;
}

// FUNCTION: SWBF2 0x006d8340
int RedTextElement::SetText(const char* pcText, bool bFlag)
{
    unsigned short acBuffer[1024];
    PblUnicode::ustrncnv(acBuffer, pcText, 0x3ff);
    return SetText(acBuffer, bFlag);
}

// FUNCTION: SWBF2 0x006d8390
int RedTextElement::SetText(const unsigned short* pcText, bool bFlag)
{
    if (!pcText)
    {
        ClearCache();
        return 0;
    }

    m_text.SetSizeGrow(PblUnicode::ustrlen(pcText) + 1);
    PblUnicode::ustrncpy(m_text.GetData(), pcText, PblUnicode::ustrlen(pcText) + 1);

    if (bFlag)
    {
        const int iResult = WriteCache(pcText);
        if (!m_bVisible)
        {
            ClearCache();
        }
        return iResult;
    }

    m_dirtyGlyphCache = 1;
    return 0;
}

// FUNCTION: SWBF2 0x006d7150
RedTextElement::RedTextElement(const char* pcName)
    : RedGlyphCacheElement(pcName)
{
    m_uiWidth = 0;
    m_uiHeight = 0;

    m_iTextExtent = 0;
    m_uiNumLines = 0;

    m_bUnknown0090 = 1;

    m_fScaleX = 1.0f;
    m_fScaleY = 1.0f;

    m_leading = 0;
    m_iSpacing = 1;

    m_uiTabSpacing = 0;
    m_uiCursorPos = 0xffff;

    m_halign = TEXT_HALIGN_LEFT;
    m_valign = TEXT_VALIGN_TOP;
    m_break = TEXT_BREAK_WORD;
    m_clip = TEXT_CLIP_CHARACTER;
    m_dirtyGlyphCache = 0;

    m_text.SetSize(32);
}

// FUNCTION: SWBF2 0x006d75e0
void RedTextElement::Cleanup()
{
    SetFont("");
    ClearCache();
    if (m_text)
    {
        m_text.Clear();
    }
}

// FUNCTION: SWBF2 0x006d7b20
void RedTextElement::CreateHotSpot(float fRadius)
{
    float fLeft, fTop, fRight, fBottom;
    GetRect(fLeft, fTop, fRight, fBottom);

    fLeft -= fRadius;
    fTop -= fRadius;
    fRight += fRadius;
    fBottom += fRadius;

    if (!m_pHotSpot)
    {
        m_pHotSpot = new RedHotSpot;
    }
    m_pHotSpot->SetRect(fLeft, fTop, fRight, fBottom);
}
