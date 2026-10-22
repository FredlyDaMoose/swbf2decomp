#include "ChatMessageDisplay.h"
#include "GameLoop.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

namespace FLRenderer
{

void AddInterfaceElement(RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
}

// GLOBAL: SWBF2 0x01e30340
ChatMessageDisplay* ChatMessageDisplay::sInstance;

// GLOBAL: SWBF2 0x007e5eed
bool ChatMessageDisplay::sCanRender = true;

// GLOBAL: SWBF2 0x01e30344
bool ChatMessageDisplay::sDisable;

// FUNCTION: SWBF2 0x004546b0
void ChatMessageDisplay::Create()
{
    sInstance = new ChatMessageDisplay;
}

// FUNCTION: SWBF2 0x00454720
ChatMessageDisplay::ChatMessageDisplay()
{
}

// FUNCTION: SWBF2 0x004547d0
ChatMessageDisplay::~ChatMessageDisplay()
{
}

// FUNCTION: SWBF2 0x00454850
void ChatMessageDisplay::Init()
{
    float fScreenWidth = (float)RedInterfaceScreen::GetScreenWidth() *
                         RedInterfaceScreen::GetScreenSafeZone();
    unsigned int uiFont = RedFont::GAMEFONT_SMALL;
    if (fScreenWidth < 1024.0f)
    {
        uiFont = RedFont::GAMEFONT_TINY;
    }

    mTextElement.SetFont(uiFont);
    mBitmapBackground.SetTexture(0x298058FE);
    mTextElementPrompt.SetFont(uiFont);
    mTextElementPrompt.SetColor(RedColor::YELLOW);
    mTextElementPrompt.SetText(RedLocalizeDB::Find(0x3DFA66F3, false), true);

    sCanRender = true;
    sDisable = false;

    mMessageGroup.Enable(true);
    mMessageGroup.SetPosition(0.0f, -10.0f, 0.0f);

    mBitmapBackground.Enable(false);
    mBitmapBackground.m_eStyle = RedBitmapElement::BITMAP_STYLE_NORMAL;
    mBitmapBackground.SetPosition(-3.0f, -13.0f);
    RedColor white = { 0xff, 0xff, 0xff, 0xff };
    mBitmapBackground.SetColor(white);
    mBitmapBackground.SetRect(fScreenWidth + 6.0f, (float)mTextElement.m_pFont->m_uiHeight + 6.0f,
                              RedBitmapElement::BitmapHAlignT(0),
                              RedBitmapElement::BitmapVAlignT(0));
    mBitmapBackground.m_color.a = 0xa0;
    mBitmapBackground.SetZOrder(32);
    mMessageGroup.AddElement(&mBitmapBackground);

    mTextElementPrompt.SetIntercharacterSpacing(0);
    mTextElementPrompt.m_eStyle = RedGlyphCacheElement::TEXT_STYLE_SHADOW;
    mTextElementPrompt.Enable(false);
    mTextElementPrompt.SetPosition(0.0f, -10.0f);
    mTextElementPrompt.SetZOrder(0);
    mMessageGroup.AddElement(&mTextElementPrompt);

    mTextElement.SetIntercharacterSpacing(0);
    mTextElement.m_eStyle = RedGlyphCacheElement::TEXT_STYLE_SHADOW;
    mTextElement.Enable(false);
    mTextElement.SetPosition(0.0f, -10.0f);
    mTextElement.SetZOrder(0);
    mMessageGroup.AddElement(&mTextElement);

    FLRenderer::AddInterfaceElement(&mMessageGroup, 0.0f, 0.0f, true);
}

// FUNCTION: SWBF2 0x00454ac0
void ChatMessageDisplay::Show()
{
    ActivateThread(0, PER_FRAME);
}

// FUNCTION: SWBF2 0x00454ad0
void ChatMessageDisplay::Show(bool bShow)
{
    if (!bShow)
    {
        mTextElement.SetTextFormatted("%s", "");
    }
    mMessageGroup.Enable(bShow);
    mTextElement.Enable(bShow);
    mTextElementPrompt.Enable(bShow);
    mBitmapBackground.Enable(bShow);
}

// FUNCTION: SWBF2 0x00454b40
void ChatMessageDisplay::Show(bool bShow, unsigned short usType)
{
    Show(bShow);
    if (usType == 2)
    {
        mTextElementPrompt.SetText(RedLocalizeDB::Find(0xAEBD5182, false), true);
        mTextElementPrompt.SetColor(RedColor::GREEN);
        mTextElement.SetColor(RedColor::GREEN);
    }
    else
    {
        mTextElementPrompt.SetText(RedLocalizeDB::Find(0x3DFA66F3, false), true);
        mTextElementPrompt.SetColor(RedColor::YELLOW);
        mTextElement.SetColor(RedColor::YELLOW);
    }

    float fScreenWidth = (float)RedInterfaceScreen::GetScreenWidth() *
                         RedInterfaceScreen::GetScreenSafeZone();
    float fLeft, fTop, fRight, fBottom;
    mTextElementPrompt.GetRect(fLeft, fTop, fRight, fBottom);
    mTextElement.SetPosition(mTextElementPrompt.GetPosition().x + fRight + 4.0f,
                             mTextElementPrompt.GetPosition().y);
    unsigned int uiWidth, uiHeight;
    mTextElement.GetTextBox(uiWidth, uiHeight);
    mTextElement.SetTextBox((int)(fScreenWidth - mTextElement.GetPosition().x), uiHeight);
}

// FUNCTION: SWBF2 0x00454ce0
bool ChatMessageDisplay::Update(float fDeltaTime)
{
    mMessageGroup.Enable(true);
    if (!sCanRender || sDisable)
    {
        mTextElement.Enable(false);
        mTextElementPrompt.Enable(false);
    }
    if (sCanRender && !sDisable && fDeltaTime != 0.0f)
    {
        GameLoop::IsGameOver();
    }
    return true;
}

// FUNCTION: SWBF2 0x00454d60
void ChatMessageDisplay::AddMessage(const unsigned short* pusText, RedColor color)
{
    mTextElement.SetColor(color);
    mTextElement.SetText(pusText, true);
}
