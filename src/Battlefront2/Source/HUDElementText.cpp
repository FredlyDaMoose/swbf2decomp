#include <float.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "HUDElementText.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

// GLOBAL: SWBF2 0x007e6984
char* HUD::ElementText::Factory::sTextClip[1] =
{
    "Character"
};

// GLOBAL: SWBF2 0x007e6990
char* HUD::ElementText::Factory::sTextBreak[4] =
{
    "None", "Word", "No Reformat", "Word Reformat"
};

// GLOBAL: SWBF2 0x007e69a0
char* HUD::ElementText::Factory::sTextAlignVertical[4] =
{
    "Top", "Bottom", "Center", "Baseline"
};

// GLOBAL: SWBF2 0x007e69b0
char* HUD::ElementText::Factory::sTextAlignHorizontal[3] =
{
    "Left", "Right", "Center"
};

// GLOBAL: SWBF2 0x007e69bc
char* HUD::ElementText::Factory::sTextStyle[3] =
{
    "Normal", "Shadow", "Selected"
};

// FUNCTION: SWBF2 0x0055ae30
HUD::ElementText::Factory::Factory()
    : HUD::Element::Factory(0xBDE64E3Eu)
{
    mName = "Text";

    mTextBox.Init("TextBox", mProperties, Property::type_Vector2);
    mTextBox.InitVectorRange(Property::axis_X, 0.0f, 1.0f, 0.01f);
    mTextBox.InitVectorRange(Property::axis_Y, 0.0f, 1.0f, 0.01f);
    mTextBreak.Init("TextBreak", mProperties, sTextBreak, 0, 3);
    mTextClip.Init("TextClip", mProperties, sTextClip, 0, 0);
    mTextHAlignment.Init("TextHAlignment", mProperties, sTextAlignHorizontal, 0, 2);
    mTextVAlignment.Init("TextVAlignment", mProperties, sTextAlignVertical, 0, 3);
    mTextScale.Init("TextScale", mProperties, Property::type_Vector2);
    mTextScale.InitVectorRange(Property::axis_X, 0.0f, FLT_MAX, 0.01f);
    mTextScale.InitVectorRange(Property::axis_Y, 0.0f, FLT_MAX, 0.01f);
    mTextCharacterSpacing.Init("TextCharacterSpacing", mProperties, 0, -1);
    mTextTabSpacing.Init("TextTabSpacing", mProperties, 0, -1);
    mTextStyle.Init("TextStyle", mProperties, sTextStyle, 0, 2);
    mEventText.Init("EventText", mProperties, Property::type_Event);
    mEventNumber.Init("EventNumber", mProperties, Property::type_Event);
    mPercent.Init("Percent", mProperties, Property::type_Bool);
    mInfiniteDashes.Init("InfiniteDashes", mProperties, Property::type_Bool);
}

// FUNCTION: SWBF2 0x0055adb0
HUD::Item* HUD::ElementText::Factory::CreateItem(const char* const pName, ItemCallback pCallback,
                                                  void* const pUserData)
{
    HUD::Item* pItem = new ElementText(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

// SYNTHETIC: SWBF2 0x0055b120
// HUD::ElementText::Factory::~Factory

// FUNCTION: SWBF2 0x0055b1c0
HUD::ElementText::ElementText(const char* const pName, Factory::ItemCallback pCallback,
                              void* const pUserData)
    : HUD::Element(*static_cast<RedTextElement*>(this), pName, pCallback, pUserData),
      mIntegerFormat(0),
      mFloatFormat(0),
      mNumberToTime(0),
      mLocalizedText(0),
      mFontName(0)
{
    mPercent = false;
    mShowInfiniteDashes = true;
    mEventText.Init(EventText, this);
    mEventNumber.Init(EventNumber, this);
    SetIntercharacterSpacing(1);
    SetTabSpacing(10);
    SetFont(RedFont::GAMEFONT_TINY);
    RedTextElement::Enable(false);
}

// SYNTHETIC: SWBF2 0x0055b2e0
// HUD::ElementText::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0055c892
// [thunk]:HUD::ElementText::`scalar deleting destructor' adjustor{256}
// FUNCTION: SWBF2 0x0055b310
HUD::ElementText::~ElementText()
{
    FreeString(&mIntegerFormat);
    FreeString(&mFloatFormat);
    FreeString(&mNumberToTime);
    FreeName(&mLocalizedText);
}

// FUNCTION: SWBF2 0x0055b980
void HUD::ElementText::SaveTextBoxText(unsigned short** ppText)
{
    const unsigned short* pText = m_text;
    *ppText = 0;
    if (pText)
    {
        const unsigned int uLength = PblUnicode::ustrlen(pText) + 1;
        *ppText = new unsigned short[uLength];
        if (*ppText)
        {
            PblUnicode::ustrncpy(*ppText, pText, uLength);
        }
    }
}

// FUNCTION: SWBF2 0x0055b9e0
void HUD::ElementText::RestoreTextBoxText(unsigned short** ppText)
{
    if (*ppText)
    {
        SetText(*ppText, true);
        PblUnicode::ustrlen(*ppText);
        delete[] *ppText;
        *ppText = 0;
    }
}

// FUNCTION: SWBF2 0x0055ba20
HUD::ElementText& HUD::ElementText::operator=(const ElementText& other)
{
    unsigned short* pText;
    SaveTextBoxText(&pText);
    HUD::Element::operator=(other);
    SetFont(other.m_pFont);
    SetIntercharacterSpacing(other.m_iSpacing);
    m_leading = other.m_leading;
    SetTextClip((TextClipT)other.m_clip);
    unsigned int uWidth, uHeight;
    other.GetTextBox(uWidth, uHeight);
    SetTextBox(uWidth, uHeight);
    TextHAlignT eHAlign;
    TextVAlignT eVAlign;
    other.GetTextAlign(eHAlign, eVAlign);
    SetTextAlign(eHAlign, eVAlign);
    SetTextBreak((TextBreakT)other.m_break);
    SetTextScale(other.m_fScaleX, other.m_fScaleY);
    SetTextStyle(other.m_eStyle);
    SetFont(other.m_pFont);
    RestoreTextBoxText(&pText);
    return *this;
}

// FUNCTION: SWBF2 0x0055bb30
void HUD::ElementText::CopyToTextElement(RedTextElement& textElement)
{
    textElement.m_color = mColorDefault;
    textElement.SetZOrder((unsigned char)RedTextElement::m_zorder);
    textElement.SetBlendMode(static_cast<RedTextElement&>(*this).GetBlendMode());
    textElement.SetMatrix(m_mLocal);
    textElement.SetFont(m_pFont);
    textElement.SetIntercharacterSpacing(m_iSpacing);
    textElement.m_leading = m_leading;
    textElement.SetTextClip((TextClipT)m_clip);
    unsigned int uWidth, uHeight;
    GetTextBox(uWidth, uHeight);
    textElement.SetTextBox(uWidth, uHeight);
    TextHAlignT eHAlign;
    TextVAlignT eVAlign;
    GetTextAlign(eHAlign, eVAlign);
    textElement.SetTextAlign(eHAlign, eVAlign);
    textElement.SetTextBreak((TextBreakT)m_break);
    textElement.SetTextScale(m_fScaleX, m_fScaleY);
    textElement.SetTextStyle(m_eStyle);
    textElement.SetFont(m_pFont);
}

static const unsigned int kInfiniteDashesHash = 0x7A4E6E71u;
static const unsigned int kIntegerFormatHash = 0x406E77D2u;
static const unsigned int kTextBreakHash = 0x37D0F255u;
static const unsigned int kTextScaleHash = 0x133C15DCu;
static const unsigned int kTextStyleHash = 0x15746CB3u;
static const unsigned int kEventNumberHash = 0x3FC8DF9Eu;
static const unsigned int kTextTabSpacingHash = 0x4CE322C6u;
static const unsigned int kTextBoxHash = 0x5BACE35Bu;
static const unsigned int kPercentHash = 0x75F9FA5Au;
static const unsigned int kTextAlignmentHash = 0xC2200723u;
static const unsigned int kTextClipHash = 0xB069FF88u;
static const unsigned int kEventTextHash = 0x7E1311A8u;
static const unsigned int kTextFontHash = 0x90AE147Fu;
static const unsigned int kTextHash = 0xBDE64E3Eu;
static const unsigned int kNumberToTimeHash = 0xCE8AD9C0u;
static const unsigned int kFloatFormatHash = 0xD9E54312u;
static const unsigned int kTextCharacterSpacingHash = 0xF30ABEAAu;

// FUNCTION: SWBF2 0x0055bc80
bool HUD::ElementText::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kTextHash:
    {
        const unsigned short* pText = RedLocalizeDB::Find(PblHash(data.GetStringArg(0)), true);
        if (pText)
        {
            SetText(pText, true);
        }
        else
        {
            SetText(data.GetStringArg(0), true);
        }
        FreeName(&mLocalizedText);
        CopyName(&mLocalizedText, data.GetStringArg(0), 0);
        return true;
    }
    case kTextFontHash:
    {
        RedFont* pFont = RedFont::Find(data.GetStringArg(0));
        if (pFont)
        {
            SetFont(pFont);
            CopyName(&mFontName, data.GetStringArg(0), 0);
        }
        return true;
    }
    case kTextBoxHash:
        SetTextBox((unsigned int)(data.GetFloatArg(0) * sViewportWidth),
                   (unsigned int)(data.GetFloatArg(1) * sViewportHeight));
        return true;
    case kTextBreakHash:
    {
        const unsigned int uBreak = TranslateString((const char* const*)Factory::sTextBreak, 4,
                                               data.GetStringArg(0));
        if (uBreak < 4)
        {
            SetTextBreak((TextBreakT)uBreak);
        }
        return true;
    }
    case kTextClipHash:
    {
        const unsigned int uClip = TranslateString((const char* const*)Factory::sTextClip, 1,
                                              data.GetStringArg(0));
        if (uClip < 1)
        {
            SetTextClip((TextClipT)uClip);
        }
        return true;
    }
    case kTextAlignmentHash:
    {
        const unsigned int uHAlign = TranslateString((const char* const*)Factory::sTextAlignHorizontal,
                                                3, data.GetStringArg(0));
        const unsigned int uVAlign = TranslateString((const char* const*)Factory::sTextAlignVertical,
                                                4, data.GetStringArg(1));
        TextHAlignT eHAlign;
        TextVAlignT eVAlign;
        GetTextAlign(eHAlign, eVAlign);
        if (uHAlign < 3)
        {
            eHAlign = (TextHAlignT)uHAlign;
        }
        if (uVAlign < 4)
        {
            eVAlign = (TextVAlignT)uVAlign;
        }
        SetTextAlign(eHAlign, eVAlign);
        return true;
    }
    case kTextScaleHash:
        SetTextScale(data.GetFloatArg(0), data.GetFloatArg(1));
        return true;
    case kTextCharacterSpacingHash:
        SetIntercharacterSpacing(data.GetInt32Arg(0));
        return true;
    case kTextTabSpacingHash:
        SetTabSpacing(data.GetUint32Arg(0));
        return true;
    case kTextStyleHash:
    {
        const unsigned int uStyle = TranslateString((const char* const*)Factory::sTextStyle, 3,
                                               data.GetStringArg(0));
        if (uStyle < 3)
        {
            SetTextStyle((TextStyleT)uStyle);
        }
        return true;
    }
    case kEventTextHash:
        Item::ReadEvent(data, mEventText);
        return true;
    case kEventNumberHash:
        Item::ReadEvent(data, mEventNumber);
        return true;
    case kIntegerFormatHash:
        CopyString(&mIntegerFormat, data.GetStringArg(0));
        return true;
    case kFloatFormatHash:
        CopyString(&mFloatFormat, data.GetStringArg(0));
        return true;
    case kNumberToTimeHash:
        CopyString(&mNumberToTime, data.GetStringArg(0));
        return true;
    case kPercentHash:
        mPercent = data.GetBoolArg(0);
        return true;
    case kInfiniteDashesHash:
        mShowInfiniteDashes = data.GetBoolArg(0);
        return true;
    }
    return Element::ReadData(config, data);
}

// FUNCTION: SWBF2 0x0055c090
bool HUD::ElementText::CopyString(char** ppDest, const char* const pSource)
{
    if (pSource)
    {
        FreeString(ppDest);
        const unsigned int uLength = strlen(pSource);
        *ppDest = new char[uLength + 1];
        if (!*ppDest)
        {
            return false;
        }
        strncpy(*ppDest, pSource, uLength);
        (*ppDest)[uLength] = 0;
    }
    return true;
}

// FUNCTION: SWBF2 0x0055c0e0
void HUD::ElementText::FreeString(char** ppString)
{
    if (*ppString)
    {
        delete[] *ppString;
        *ppString = 0;
    }
}

// FUNCTION: SWBF2 0x0055c3e0
void HUD::ElementText::EventText(const HUD::Event& event, void* const pData)
{
    ElementText* pText = (ElementText*)pData;
    RedTextElement& textElement = static_cast<RedTextElement&>(pText->GetRedInterfaceElement());
    switch (event.GetClass()->GetType())
    {
    case EventClass::type_Uint:
        textElement.SetText(RedLocalizeDB::Find(event.GetData().uintValue, false), true);
        break;
    case EventClass::type_String:
    {
        const unsigned short* pString = event.GetData().string;
        if (pString)
        {
            textElement.SetText(pString, true);
        }
        else
        {
            textElement.SetText("", true);
        }
        break;
    }
    default:
        return;
    }
    EventChanged(event, pData);
}

// FUNCTION: SWBF2 0x0055c480
void HUD::ElementText::EventNumber(const HUD::Event& event, void* const pData)
{
    const EventClass::Type eType = event.GetClass()->GetType();
    ElementText* pText = (ElementText*)pData;
    char text[256];

    if (pText->mNumberToTime)
    {
        switch (eType)
        {
        case EventClass::type_Int:
        case EventClass::type_Uint:
        case EventClass::type_Float:
            break;
        default:
            return;
        }

        float fSeconds = event.GetDataFloat();
        float fMinutes = floor(fSeconds / 60.0f);
        fSeconds -= fMinutes * 60.0f;
        float fHours = floor(fMinutes / 60.0f);
        fMinutes -= fHours * 60.0f;
        const float fDays = floor(fHours / 24.0f);
        fHours -= fDays * 24.0f;

        char format[32];
        char buffer[256];
        unsigned int uFormatLength = 0;
        unsigned int uTextLength = 0;
        text[0] = 0;
        for (const char* pc = pText->mNumberToTime; *pc; ++pc)
        {
            const char c = *pc;
            if (uFormatLength != 0 && uFormatLength < 30)
            {
                int iValue;
                if (c == 'd')
                {
                    iValue = (int)fDays;
                }
                else if (c == 'h')
                {
                    iValue = (int)fHours;
                }
                else if (c == 'm')
                {
                    iValue = (int)fMinutes;
                }
                else if (c == 's')
                {
                    format[uFormatLength] = 'd';
                    format[uFormatLength + 1] = 0;
                    if (pText->mFloatFormat)
                    {
                        _snprintf(buffer, 255, pText->mFloatFormat, fSeconds);
                    }
                    else
                    {
                        _snprintf(buffer, 255, pText->mIntegerFormat ? pText->mIntegerFormat : format,
                                  (int)fSeconds);
                    }
                    strlcat(text, buffer, 255);
                    uTextLength = strlen(text);
                    uFormatLength = 0;
                    continue;
                }
                else
                {
                    format[uFormatLength++] = c;
                    continue;
                }
                format[uFormatLength] = 'd';
                format[uFormatLength + 1] = 0;
                _snprintf(buffer, 255, format, iValue);
                strlcat(text, buffer, 255);
                uTextLength = strlen(text);
                uFormatLength = 0;
            }
            else if (c == '%')
            {
                text[uTextLength] = 0;
                format[uFormatLength++] = c;
            }
            else
            {
                text[uTextLength++] = c;
            }
        }
        text[uTextLength] = 0;
    }
    else
    {
        switch (eType)
        {
        case EventClass::type_Int:
        case EventClass::type_Uint:
        {
            const bool bInfinite = (eType == EventClass::type_Uint)
                                       ? event.GetData().uintValue == 0xffffffffu
                                       : event.GetData().intValue == 0x7fffffff;
            const char* pFormat;
            if (bInfinite)
            {
                pFormat = pText->mShowInfiniteDashes ? "--" : "";
            }
            else
            {
                pFormat = pText->mIntegerFormat ? pText->mIntegerFormat : "%d";
            }
            _snprintf(text, 255, pFormat, event.GetData().intValue);
            break;
        }
        case EventClass::type_Float:
        {
            const bool bPercent = pText->mPercent;
            const float fValue = bPercent ? event.GetData().floatValue * 100.0f : event.GetData().floatValue;
            const char* pFormat;
            if (event.GetData().floatValue == FLT_MAX)
            {
                pFormat = pText->mShowInfiniteDashes ? "--" : "";
            }
            else if (bPercent)
            {
                pFormat = "%.0f";
            }
            else
            {
                pFormat = pText->mFloatFormat ? pText->mFloatFormat : "%f";
            }
            _snprintf(text, 255, pFormat, fValue);
            if (pText->mPercent)
            {
                strlcat(text, "%", 256);
            }
            break;
        }
        default:
            return;
        }
    }

    static_cast<RedTextElement*>(pText)->SetText(text, true);
    EventChanged(event, pData);
}

// FUNCTION: SWBF2 0x0055b3c0
void HUD::ElementText::GetSize(float& fWidth, float& fHeight)
{
    float fLeft, fTop, fRight, fBottom;
    GetRect(fLeft, fTop, fRight, fBottom);
    fWidth = fRight - fLeft;
    fHeight = fBottom - fTop;
}

// FUNCTION: SWBF2 0x0055b900
const char* HUD::ElementText::PropertyTranslateEnum(const unsigned int uHashID,
                                                     const unsigned int uValue)
{
    switch (uHashID)
    {
    case 0xAF7A0A77u: return Factory::sTextAlignHorizontal[uValue];
    case kTextStyleHash: return Factory::sTextStyle[uValue];
    case kTextBreakHash: return Factory::sTextBreak[uValue];
    case kTextClipHash: return Factory::sTextClip[uValue];
    case 0xFD78EA2Du: return Factory::sTextAlignVertical[uValue];
    default: return Element::PropertyTranslateEnum(uHashID, uValue);
    }
}

// FUNCTION: SWBF2 0x0055b6c0
bool HUD::ElementText::GetProperty(const unsigned int uHashID, void* pValue)
{
    switch (uHashID)
    {
    case kPercentHash: *(bool*)pValue = mPercent; return true;
    case kInfiniteDashesHash: *(bool*)pValue = mShowInfiniteDashes; return true;
    case kTextStyleHash: *(unsigned int*)pValue = m_eStyle; return true;
    case kTextBreakHash: *(unsigned int*)pValue = m_break; return true;
    case kTextClipHash: *(unsigned int*)pValue = m_clip; return true;
    case kTextCharacterSpacingHash: *(int*)pValue = m_iSpacing; return true;
    case kTextTabSpacingHash: *(unsigned int*)pValue = GetTabSpacing(); return true;
    case kTextScaleHash:
        ((float*)pValue)[0] = m_fScaleX;
        ((float*)pValue)[1] = m_fScaleY;
        return true;
    case kTextBoxHash:
    {
        unsigned int uWidth, uHeight;
        GetTextBox(uWidth, uHeight);
        ((float*)pValue)[0] = (float)uWidth / sViewportWidth;
        ((float*)pValue)[1] = (float)uHeight / sViewportHeight;
        return true;
    }
    case 0xAF7A0A77u: *(unsigned int*)pValue = m_halign; return true;
    case 0xFD78EA2Du: *(unsigned int*)pValue = m_valign; return true;
    case kEventTextHash: *(const char**)pValue = mEventText.GetRegisteredClassName(false); return true;
    case kEventNumberHash: *(const char**)pValue = mEventNumber.GetRegisteredClassName(false); return true;
    default: return Element::GetProperty(uHashID, pValue);
    }
}

// FUNCTION: SWBF2 0x0055b410
bool HUD::ElementText::SetProperty(const unsigned int uHashID, const void* pValue)
{
    switch (uHashID)
    {
    case kPercentHash: mPercent = *(const bool*)pValue; return true;
    case kInfiniteDashesHash: mShowInfiniteDashes = *(const bool*)pValue; return true;
    case kEventTextHash:
        Item::SetEvent(*(const char* const*)pValue, mEventText);
        return true;
    case kEventNumberHash:
        Item::SetEvent(*(const char* const*)pValue, mEventNumber);
        return true;
    case kTextScaleHash:
        SetTextScale(((const float*)pValue)[0], ((const float*)pValue)[1]);
        return true;
    case 0xAF7A0A77u:
    case 0xFD78EA2Du:
    {
        unsigned short* pText;
        SaveTextBoxText(&pText);
        TextHAlignT eHAlign;
        TextVAlignT eVAlign;
        GetTextAlign(eHAlign, eVAlign);
        if (uHashID == 0xAF7A0A77u)
        {
            eHAlign = (TextHAlignT)*(const unsigned int*)pValue;
        }
        else
        {
            eVAlign = (TextVAlignT)*(const unsigned int*)pValue;
        }
        SetTextAlign(eHAlign, eVAlign);
        RestoreTextBoxText(&pText);
        return true;
    }
    case kTextStyleHash:
    case kTextBreakHash:
    case kTextClipHash:
    case kTextCharacterSpacingHash:
    case kTextTabSpacingHash:
    case kTextBoxHash:
    {
        unsigned short* pText;
        SaveTextBoxText(&pText);
        switch (uHashID)
        {
        case kTextStyleHash: SetTextStyle((TextStyleT)*(const unsigned int*)pValue); break;
        case kTextBreakHash: SetTextBreak((TextBreakT)*(const unsigned int*)pValue); break;
        case kTextClipHash: SetTextClip((TextClipT)*(const unsigned int*)pValue); break;
        case kTextCharacterSpacingHash: SetIntercharacterSpacing(*(const int*)pValue); break;
        case kTextTabSpacingHash: SetTabSpacing(*(const unsigned int*)pValue); break;
        case kTextBoxHash:
            SetTextBox((unsigned int)(((const float*)pValue)[0] * sViewportWidth),
                       (unsigned int)(((const float*)pValue)[1] * sViewportHeight));
            break;
        }
        RestoreTextBoxText(&pText);
        return true;
    }
    default: return Element::SetProperty(uHashID, pValue);
    }
}

// FUNCTION: SWBF2 0x0055c100
void HUD::ElementText::WriteData(HUD::File& file, unsigned int uTabs)
{
    unsigned int uWidth, uHeight;
    GetTextBox(uWidth, uHeight);
    file.WriteProperty("TextBox", uTabs, (float)uWidth / sViewportWidth,
                       (float)uHeight / sViewportHeight, 0.0f, 0.0f);
    file.WriteProperty("TextBreak", uTabs, Factory::sTextBreak[m_break], Factory::sTextBreak[0]);
    file.WriteProperty("TextClip", uTabs, Factory::sTextClip[m_clip], Factory::sTextClip[0]);
    file.WriteProperty("TextStyle", uTabs, Factory::sTextStyle[m_eStyle], Factory::sTextStyle[0]);
    file.WriteProperty("Text", uTabs, mLocalizedText ? mLocalizedText : "", "");
    TextHAlignT eHAlign;
    TextVAlignT eVAlign;
    GetTextAlign(eHAlign, eVAlign);
    if (eHAlign != TEXT_HALIGN_LEFT || eVAlign != TEXT_VALIGN_TOP)
    {
        file.WriteTabs(uTabs);
        file.WriteStringf("TextAlignment(\"%s\", \"%s\")\n",
                          Factory::sTextAlignHorizontal[eHAlign], Factory::sTextAlignVertical[eVAlign]);
    }
    file.WriteProperty("TextScale", uTabs, m_fScaleX, m_fScaleY, 1.0f, 1.0f);
    file.WriteProperty("TextCharacterSpacing", uTabs, (int)m_iSpacing, 1);
    file.WriteProperty("TextTabSpacing", uTabs, (int)GetTabSpacing(), 10);
    file.WriteProperty("TextFont", uTabs, mFontName ? mFontName : "", "");
    file.WriteProperty("EventText", uTabs, mEventText.GetRegisteredClassName(true), "");
    file.WriteProperty("EventNumber", uTabs, mEventNumber.GetRegisteredClassName(true), "");
    file.WriteProperty("IntegerFormat", uTabs, mIntegerFormat ? mIntegerFormat : "", "");
    file.WriteProperty("FloatFormat", uTabs, mFloatFormat ? mFloatFormat : "", "");
    file.WriteProperty("NumberToTime", uTabs, mNumberToTime ? mNumberToTime : "", "");
    file.WriteProperty("Percent", uTabs, (int)mPercent, 0);
    file.WriteProperty("InfiniteDashes", uTabs, (int)mShowInfiniteDashes, 1);
    Element::WriteData(file, uTabs);
}
