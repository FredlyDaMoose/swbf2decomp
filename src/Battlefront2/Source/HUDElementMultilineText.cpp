#include <float.h>
#include <stdio.h>
#include <string.h>

#include "HUDElementMultilineText.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "RedEngineFL/Util/RedStack.h"

// FUNCTION: SWBF2 0x00557220
HUD::ElementMultilineText::Factory::Factory()
{
    mHashID = 0xD53E50D7u;
    mName = "MultilineText";

    mNumLines.Init("NumLines", mProperties, 1, 80);
    mDisplayTime.Init("DisplayTime", mProperties, 0.0f, FLT_MAX, 0.01f);
    mAlwaysScroll.Init("AlwaysScroll", mProperties, Property::type_Bool);
    mAddToTop.Init("AddToTop", mProperties, Property::type_Bool);
    mDisableOnEmpty.Init("DisableOnEmpty", mProperties, Property::type_Bool);
    mScrollSpeed.Init("ScrollSpeed", mProperties, 0.0001f, FLT_MAX, 0.01f);
    mEventText.Init("EventText", mProperties, Property::type_Event);
    mEventColor.Init("EventColor", mProperties, Property::type_Event);
}

// FUNCTION: SWBF2 0x00557190
HUD::Item* HUD::ElementMultilineText::Factory::CreateItem(const char* const pName,
                                                          ItemCallback pCallback,
                                                          void* const pUserData)
{
    HUD::Item* pItem = new ElementMultilineText(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

static const unsigned int kNumLines = 0x0269babe;
static const unsigned int kDisplayTime = 0xdaef1514;
static const unsigned int kAlwaysScroll = 0xb74de20d;
static const unsigned int kAddToTop = 0x09771c74;
static const unsigned int kDisableOnEmpty = 0xcf649993;
static const unsigned int kScrollSpeed = 0xdfc96107;
static const unsigned int kEventText = 0x7e1311a8;
static const unsigned int kEventColor = 0x49f429ae;

static const unsigned int kTextFactory = 0xBDE64E3Eu;

static inline RedTextElement& GetText(HUD::ElementMultilineText::Line& line)
{
    return line;
}

float HUD::Element::GetFadeHoldElapsed() const
{
    if (mFader.GetPhase() < HUD::Envelope::phase_Hold)
    {
        return 0.0f;
    }
    if (mFader.GetPhase() == HUD::Envelope::phase_Hold)
    {
        return mFader.GetElapsed();
    }
    return FLT_MAX;
}

// SYNTHETIC: SWBF2 0x00558b63
// [thunk]:HUD::ElementMultilineText::Line::`scalar deleting destructor' adjustor{256}
// FUNCTION: SWBF2 0x00557120
HUD::ElementMultilineText::Line::Line()
    : HUD::ElementText(0, 0, 0),
      mGroup(0)
{
}

// FUNCTION: SWBF2 0x00557210
HUD::ElementMultilineText::Line::~Line()
{
}

// FUNCTION: SWBF2 0x00557150
bool HUD::ElementMultilineText::Line::SetProperty(const unsigned int uHashID,
                                                  const void* pValue)
{
    const bool bResult = HUD::ElementText::SetProperty(uHashID, pValue);
    if (bResult && mGroup)
    {
        mGroup->RefreshFormatting(false);
    }
    return bResult;
}

// FUNCTION: SWBF2 0x00557460
HUD::ElementMultilineText::ElementMultilineText(const char* const pName,
                                                Factory::ItemCallback pCallback,
                                                void* const pUserData)
    : HUD::ElementGroup(pName, pCallback, pUserData),
      mLines(0),
      mNumLines(0),
      mMaxDisplayLines(0),
      mLinesPending(0),
      mNumLinesDisplayed(0),
      mNextLine(0),
      mDisplayInterval(2.0f),
      mState(state_Waiting),
      mScrollSpeed(1.0f)
{
    mAlwaysScroll = false;
    mAddToTop = false;
    mDisableOnEmpty = false;
    mFormat.SetFactory(HUD::Item::Factory::FindByHashID(kTextFactory));
    mFormat.mDynamicAlloc = false;
    mFormat.mWriteEnabled = false;
    mFormat.SetGroup(this);

    mFormat.HUD::Item::SetName("Format");
    mEventText.Init(EventText, this);
    mEventColor.Init(EventColor, this);
}

// SYNTHETIC: SWBF2 0x00558b6e
// [thunk]:HUD::ElementMultilineText::`scalar deleting destructor' adjustor{368}

// FUNCTION: SWBF2 0x00557630
HUD::ElementMultilineText::~ElementMultilineText()
{
    FreeLines();
}

// FUNCTION: SWBF2 0x005577b0
unsigned int HUD::ElementMultilineText::GetNextPendingLineIndex() const
{
    if (mNextLine < mLinesPending)
    {
        return mNextLine + mNumLines - mLinesPending;
    }
    return mNextLine - mLinesPending;
}

// FUNCTION: SWBF2 0x005577e0
unsigned int HUD::ElementMultilineText::GetEldestLineIndex(unsigned int uIndex) const
{
    if (uIndex < mNumLinesDisplayed)
    {
        return uIndex + mNumLines - mNumLinesDisplayed;
    }
    return uIndex - mNumLinesDisplayed;
}

// FUNCTION: SWBF2 0x00557810
HUD::ElementMultilineText::Line* HUD::ElementMultilineText::GetTop()
{
    if (!mNumLinesDisplayed)
    {
        return 0;
    }
    return &mLines[GetEldestLineIndex(GetNextPendingLineIndex())];
}

// FUNCTION: SWBF2 0x00557840
HUD::ElementMultilineText::Line* HUD::ElementMultilineText::GetBottom()
{
    if (!mNumLinesDisplayed)
    {
        return 0;
    }
    unsigned int uIndex = GetNextPendingLineIndex();
    if (!uIndex)
    {
        uIndex = mNumLines;
    }
    return &mLines[uIndex - 1];
}

// FUNCTION: SWBF2 0x00557870
bool HUD::ElementMultilineText::TopLineAtTop()
{
    Line* pTop = GetTop();
    if (!pTop)
    {
        return false;
    }
    return GetText(*pTop).GetPosition().y <= 0.0001f;
}

// FUNCTION: SWBF2 0x005578a0
bool HUD::ElementMultilineText::BottomLineOutOfBottomRegion(float fHeight, float fLineHeight)
{
    Line* pBottom = GetBottom();
    if (!pBottom)
    {
        return true;
    }
    return fHeight - fLineHeight >= GetText(*pBottom).GetPosition().y + fLineHeight;
}

// FUNCTION: SWBF2 0x005578d0
void HUD::ElementMultilineText::EnableNextLine(unsigned int uIndex)
{
    if (mLinesPending)
    {
        const float fLineHeight = (float)GetText(mFormat).GetTextHeight();
        const float fHeight = (float)GetHeightPixels();
        if (BottomLineOutOfBottomRegion(fHeight, fLineHeight))
        {
            const float fY = mAddToTop ? mNumLinesDisplayed * fLineHeight : fHeight - fLineHeight;
            GetText(mLines[uIndex]).SetPosition(0.0f, fY);
            mLines[uIndex].HUD::Element::Enable(true);
            mState = state_EnablingLine;
            return;
        }
    }
    mState = state_Waiting;
}

// FUNCTION: SWBF2 0x005576e0
void HUD::ElementMultilineText::DisableAndScroll(Line& line, unsigned int uIndex)
{
    if (mNumLinesDisplayed &&
        (mAlwaysScroll ||
         (mLinesPending && (!mAddToTop || mNumLinesDisplayed == mMaxDisplayLines))))
    {
        if (line.GetFadeHoldElapsed() > mDisplayInterval)
        {
            RedTextElement& text = GetText(line);
            if (text.m_bEnabled && text.GetPosition().y <= 0.1f)
            {
                line.HUD::Element::Enable(false);
                mState = state_DisablingLine;
            }
            else
            {
                mState = state_Scrolling;
            }
            return;
        }
    }
    EnableNextLine(uIndex);
}

// FUNCTION: SWBF2 0x005579a0
void HUD::ElementMultilineText::Update(const float fElapsed)
{
    const unsigned int uNext = GetNextPendingLineIndex();
    unsigned int uEldest = GetEldestLineIndex(uNext);

    switch (mState)
    {
    case state_Waiting:
        if (mAlwaysScroll || mNumLinesDisplayed == mMaxDisplayLines || mLinesPending ||
            !mNumLinesDisplayed)
        {
            DisableAndScroll(mLines[uEldest], uNext);
        }
        break;

    case state_DisablingLine:
        if (mLines[uEldest].mEnableState == enableState_Disabled)
        {
            GetText(mLines[uEldest]).SetText((const unsigned short*)0, true);
            if (mNumLinesDisplayed && --mNumLinesDisplayed == 0 && mDisableOnEmpty)
            {
                HUD::Element::Enable(false);
            }
            mState = state_Scrolling;
        }
        break;

    case state_Scrolling:
    {
        const float fHeight = (float)GetHeightPixels();
        const unsigned int uLineHeight = GetText(mFormat).GetTextHeight();
        if (!GetTop() || TopLineAtTop() ||
            (!mAddToTop && mLinesPending &&
             BottomLineOutOfBottomRegion(fHeight, (float)uLineHeight)) ||
            !GetText(mLines[uEldest]).m_bEnabled)
        {
            EnableNextLine(uNext);
            break;
        }
        if (fElapsed == 0.0f)
        {
            break;
        }

        bool bAtTop = false;
        float fScroll = mScrollSpeed * fElapsed;
        if (!(fScroll < 1.0f))
        {
            fScroll = 1.0f;
        }
        float fDelta = uLineHeight * fScroll;
        const float fTopY = GetText(mLines[uEldest]).GetPosition().y;
        if (fDelta > fTopY)
        {
            fDelta = fTopY;
            bAtTop = true;
        }
        for (unsigned int i = 0; i < mNumLinesDisplayed; ++i)
        {
            RedTextElement& text = GetText(mLines[uEldest]);
            text.SetPosition(text.GetPosition().x, text.GetPosition().y - fDelta);
            if (++uEldest == mNumLines)
            {
                uEldest = 0;
            }
        }
        if (bAtTop)
        {
            EnableNextLine(uNext);
        }
        break;
    }

    case state_EnablingLine:
    {
        Line& line = mLines[uNext];
        if (line.mEnableState == enableState_Enabled)
        {
            --mLinesPending;
            ++mNumLinesDisplayed;
            mState = state_Waiting;
        }
        else if (line.mEnableState == enableState_Disabling ||
                 line.mEnableState == enableState_Disabled)
        {
            line.HUD::Element::Enable(true);
        }
        break;
    }
    }

    HUD::ElementGroup::Update(fElapsed);

    if (mEnableState == enableState_Disabled && (mNumLinesDisplayed || mLinesPending))
    {
        for (unsigned int i = 0; i < mNumLines; ++i)
        {
            mLines[i].mFader.SetPhaseTime(HUD::Envelope::phase_Release, 0.0f);
            mLines[i].HUD::Element::Enable(false);
            mLines[i].Update(0.0001f);
            mLines[i].mFader.SetPhaseTime(HUD::Envelope::phase_Release,
                                          mFormat.mFader.GetPhaseTime(HUD::Envelope::phase_Release));
        }
        mNumLinesDisplayed = 0;
        mLinesPending = 0;
        mNextLine = 0;
    }
}

// FUNCTION: SWBF2 0x00557d30
void HUD::ElementMultilineText::RefreshTextBoxSize(unsigned int uLineHeight, unsigned int uWidth,
                                                   unsigned int uHeight)
{
    GetText(mFormat).SetTextBox(uWidth, uHeight);
    mMaxDisplayLines = GetText(mFormat).GetMaxLines(uLineHeight);
}

// FUNCTION: SWBF2 0x00557d70
void HUD::ElementMultilineText::FreeLines()
{
    if (mLines)
    {
        for (unsigned int i = 0; i < mNumLines; ++i)
        {
            HUD::ElementGroupBase::RemoveElement(&mLines[i]);
        }
        delete[] mLines;
        mLines = 0;
        mLinesPending = 0;
        mNumLines = 0;
        mNumLinesDisplayed = 0;
        mNextLine = 0;
    }
}

// FUNCTION: SWBF2 0x00557e10
void HUD::ElementMultilineText::RefreshFormatting(bool bAddToGroup)
{
    const unsigned int uWidth = GetWidthPixels();
    const unsigned int uLineHeight = GetText(mFormat).GetTextHeight();
    for (unsigned int i = 0; i < mNumLines; ++i)
    {
        mLines[i].mDynamicAlloc = false;
        if (bAddToGroup)
        {
            HUD::ElementGroupBase::AddElement(&mLines[i]);
        }
        mLines[i].mWriteEnabled = false;
        mLines[i] = mFormat;
        GetText(mLines[i]).SetPosition(0.0f, 0.0f);
        GetText(mLines[i]).SetTextBox(uWidth, uLineHeight);
    }
}

// FUNCTION: SWBF2 0x00557f10
void HUD::ElementMultilineText::SetupLines(unsigned int uNumLines)
{
    RefreshTextBoxSize(GetText(mFormat).GetTextHeight(), GetWidthPixels(), GetHeightPixels());
    const bool bChanged = (uNumLines != mNumLines);
    if (bChanged)
    {
        FreeLines();
        mLines = new Line[uNumLines];
        if (mLines)
        {
            RedTextElement::TextHAlignT eHAlign;
            RedTextElement::TextVAlignT eVAlign;
            GetText(mFormat).GetTextAlign(eHAlign, eVAlign);
            mNumLines = uNumLines;
        }
    }
    RefreshFormatting(bChanged);
}

// FUNCTION: SWBF2 0x00558070
bool HUD::ElementMultilineText::AddText(const unsigned short* const pText,
                                        const RedColor* const pColor)
{
    const unsigned int uLineHeight = GetText(mFormat).GetTextHeight();
    RedStack stack;
    unsigned int uMaxLines = 80;
    const unsigned int uFreeLines = mNumLines - mNumLinesDisplayed - mLinesPending;

    unsigned int uSize = (PblUnicode::ustrlen(pText) + 1) * sizeof(unsigned short);
    if (uSize & 0xf)
    {
        uSize = (uSize & ~0xfu) + 0x10;
    }
    unsigned short* pcText = (unsigned short*)stack.Alloc(uSize);
    if (!pcText)
    {
        return false;
    }
    memcpy(pcText, pText, (PblUnicode::ustrlen(pText) + 1) * sizeof(unsigned short));

    unsigned int uWidth;
    unsigned int uHeight;
    GetText(mFormat).GetTextBox(uWidth, uHeight);
    GetText(mFormat).SetTextBox(uWidth, mNumLines * uLineHeight);
    RedTextElement::TextLine aLines[80];
    unsigned int uNumLines;
    const unsigned short* pcEnd;
    short sCursorX;
    GetText(mFormat).ClipString(pcText, aLines, uMaxLines, uNumLines, pcEnd, uLineHeight, sCursorX);
    GetText(mFormat).SetTextBox(uWidth, uHeight);

    if (uNumLines > uFreeLines)
    {
        const float fLineHeight = (float)GetText(mFormat).GetTextHeight();
        for (unsigned int i = uFreeLines; i < uNumLines; ++i)
        {
            const unsigned int uNext = GetNextPendingLineIndex();
            Line& line = mLines[GetEldestLineIndex(uNext)];
            GetText(line).SetText((const unsigned short*)0, true);
            line.HUD::Element::Enable(false);
            GetText(line).m_bEnabled = false;
            if (mNumLinesDisplayed)
            {
                --mNumLinesDisplayed;
            }
            else if (mLinesPending)
            {
                --mLinesPending;
            }

            unsigned int uIndex = GetEldestLineIndex(uNext);
            for (unsigned int j = 0; j < mNumLinesDisplayed; ++j)
            {
                RedTextElement& text = GetText(mLines[uIndex]);
                text.SetPosition(text.GetPosition().x, text.GetPosition().y - fLineHeight);
                if (++uIndex == mNumLines)
                {
                    uIndex = 0;
                }
            }
        }
    }

    for (unsigned int i = 0; i < uNumLines; ++i)
    {
        unsigned short acLine[257];
        unsigned int uBytes = (unsigned int)((const char*)aLines[i].pcEnd -
                                             (const char*)aLines[i].pcStart);
        if (uBytes > sizeof(acLine))
        {
            uBytes = sizeof(acLine) - sizeof(unsigned short);
        }
        memcpy(acLine, aLines[i].pcStart, uBytes);
        acLine[uBytes / sizeof(unsigned short)] = 0;

        GetText(mLines[mNextLine]).m_color = GetText(mFormat).m_color;
        mLines[mNextLine].mColorDefault = GetText(mFormat).m_color;
        mLines[mNextLine].mAlpha = mFormat.mAlpha;
        GetText(mLines[mNextLine]).SetText(acLine, false);
        ++mLinesPending;
        if (++mNextLine == mNumLines)
        {
            mNextLine = 0;
        }
        if (uNumLines > uFreeLines)
        {
            EnableNextLine(GetNextPendingLineIndex());
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00558480
bool HUD::ElementMultilineText::SetProperty(const unsigned int uHashID, const void* pValue)
{
    switch (uHashID)
    {
    case kNumLines:
        SetupLines(*(const unsigned int*)pValue);
        return true;
    case kDisplayTime:
        mDisplayInterval = *(const float*)pValue;
        return true;
    case kAlwaysScroll:
        mAlwaysScroll = *(const bool*)pValue;
        return true;
    case kAddToTop:
        mAddToTop = *(const bool*)pValue;
        return true;
    case kDisableOnEmpty:
        mDisableOnEmpty = *(const bool*)pValue;
        return true;
    case kScrollSpeed:
        mScrollSpeed = *(const float*)pValue;
        return true;
    case kEventText:
        Item::SetEvent(*(const char* const*)pValue, mEventText);
        return true;
    case kEventColor:
        Item::SetEvent(*(const char* const*)pValue, mEventColor);
        return true;
    }
    return HUD::ElementGroup::SetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x00558580
bool HUD::ElementMultilineText::GetProperty(const unsigned int uHashID, void* pValue)
{
    switch (uHashID)
    {
    case kNumLines:
        *(unsigned int*)pValue = mNumLines;
        return true;
    case kDisplayTime:
        *(float*)pValue = mDisplayInterval;
        return true;
    case kAlwaysScroll:
        *(bool*)pValue = mAlwaysScroll;
        return true;
    case kAddToTop:
        *(bool*)pValue = mAddToTop;
        return true;
    case kDisableOnEmpty:
        *(bool*)pValue = mDisableOnEmpty;
        return true;
    case kScrollSpeed:
        *(float*)pValue = mScrollSpeed;
        return true;
    case kEventText:
        *(const char**)pValue = mEventText.GetRegisteredClassName(false);
        return true;
    case kEventColor:
        *(const char**)pValue = mEventColor.GetRegisteredClassName(false);
        return true;
    }
    return HUD::ElementGroup::GetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x00558680
void HUD::ElementMultilineText::Read(PblConfig& config, Factory::ItemCallback pCallback,
                                     void* const pUserData)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (config.m_pChunk->PeekNextChildId() == 0x504f4353)
        {
            if (mFormat.GetNameHash() == data.GetId())
            {
                PblConfig scope(config.ReadNextScope(), true);
                mFormat.Read(scope, 0, 0);
            }
            else
            {
                ReadChild(config, data, pCallback, pUserData);
            }
        }
        else
        {
            ReadData(config, data);
        }
    }
    PostReadSetup();
}

// FUNCTION: SWBF2 0x00558760
bool HUD::ElementMultilineText::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kNumLines:
        mNumLines = (int)data.GetFloatArg(0);
        return true;
    case kDisplayTime:
        mDisplayInterval = data.GetFloatArg(0);
        return true;
    case kAlwaysScroll:
        mAlwaysScroll = data.GetBoolArg(0);
        return true;
    case kAddToTop:
        mAddToTop = data.GetBoolArg(0);
        return true;
    case kDisableOnEmpty:
        mDisableOnEmpty = data.GetBoolArg(0);
        return true;
    case kScrollSpeed:
        mScrollSpeed = data.GetFloatArg(0);
        return true;
    case kEventText:
        Item::ReadEvent(data, mEventText);
        return true;
    case kEventColor:
        Item::ReadEvent(data, mEventColor);
        return true;
    }
    return HUD::ElementGroup::ReadData(config, data);
}

// FUNCTION: SWBF2 0x005588b0
void HUD::ElementMultilineText::PostReadSetup()
{
    HUD::ElementGroup::PostReadSetup();
    const unsigned int uNumLines = mNumLines;
    mNumLines = 0;
    SetupLines(uNumLines);

    char acName[256];
    _snprintf(acName, sizeof(acName), "%s.%s", GetName(), mFormat.GetName());
    mFormat.SetDisplayName(acName);
}

// FUNCTION: SWBF2 0x00558920
void HUD::ElementMultilineText::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("EventText", uTabs, mEventText.GetRegisteredClassName(false), "");
    file.WriteProperty("EventColor", uTabs, mEventColor.GetRegisteredClassName(false), "");
    file.WriteProperty("NumLines", uTabs, (int)mNumLines, 0);
    file.WriteProperty("DisplayTime", uTabs, mDisplayInterval, 2.0f);
    file.WriteProperty("AlwaysScroll", uTabs, (int)mAlwaysScroll, 0);
    file.WriteProperty("DisableOnEmpty", uTabs, (int)mDisableOnEmpty, 0);
    file.WriteProperty("ScrollSpeed", uTabs, mScrollSpeed, 1.0f);
    file.WriteProperty("AddToTop", uTabs, (int)mAddToTop, 0);
    HUD::ElementGroup::WriteData(file, uTabs);
    mFormat.Write(file, uTabs, "Format");
}

// FUNCTION: SWBF2 0x00558a40
void HUD::ElementMultilineText::EventText(const HUD::Event& event, void* const pData)
{
    const unsigned short* pText;
    switch (event.GetClass()->GetType())
    {
    case EventClass::type_Uint:
        pText = RedLocalizeDB::Find(event.mData.uintValue, false);
        break;
    case EventClass::type_String:
        pText = event.mData.string;
        if (!pText)
        {
            static const unsigned short acNewline[] = { '\n', 0 };
            pText = acNewline;
        }
        break;
    default:
        return;
    }
    ((ElementMultilineText*)pData)->AddText(pText, 0);
    Element::EventChanged(event, pData);
}

// FUNCTION: SWBF2 0x00558ad0
void HUD::ElementMultilineText::EventColor(const HUD::Event& event, void* const pData)
{
    if (event.GetClass()->GetType() == EventClass::type_Color)
    {
        ElementMultilineText* pDisplay = (ElementMultilineText*)pData;
        const RedColor& color = *event.mData.color;
        GetText(pDisplay->mFormat).m_color = color;
        pDisplay->mFormat.mColorDefault = color;
        pDisplay->mFormat.mAlpha = (float)color.a / 255.0f;
    }
}

// FUNCTION: SWBF2 0x00557110
void* HUD::ElementText::GetParent()
{
    return this;
}
