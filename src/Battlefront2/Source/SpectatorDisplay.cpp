#include <stdio.h>

#include "SpectatorDisplay.h"
#include "CameraManager.h"
#include "Character.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

namespace FLRenderer
{
void AddInterfaceElement(unsigned int hView, RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
}

// GLOBAL: SWBF2 0x01eafb44
SpectatorDisplay* SpectatorDisplay::sInstance[1];

// GLOBAL: SWBF2 0x01eaf00f
bool SpectatorDisplay::sCanRender;

// GLOBAL: SWBF2 0x01eafb40
bool SpectatorDisplay::sDisable;

// FUNCTION: SWBF2 0x0064ee50
void SpectatorDisplay::Create()
{
    sInstance[0] = new SpectatorDisplay;
}

// FUNCTION: SWBF2 0x0064eee0
SpectatorDisplay::SpectatorDisplay()
{
    const float fScreenWidth = (float)RedInterfaceScreen::GetScreenWidth() *
                               RedInterfaceScreen::GetScreenSafeZone();
    const float fScreenHeight = (float)RedInterfaceScreen::GetScreenHeight() *
                                RedInterfaceScreen::GetScreenSafeZone();
    const float fHalfWidth = fScreenWidth * 0.5f;
    const float fHeight = NetGame::IsHost() ? 0.15f : 0.06f;
    const RedColor white = { 0xff, 0xff, 0xff, 0xff };

    sCanRender = false;
    sDisable = false;

    mMessageGroup.Enable(false);

    mBitmapBackground.Enable(false);
    mBitmapBackground.SetColor(white);
    mBitmapBackground.SetStyle(RedBitmapElement::BITMAP_STYLE_NORMAL);
    mBitmapBackground.SetRect(fScreenWidth * 0.4f, 20.0f, RedBitmapElement::BITMAP_HALIGN_CENTER,
                              RedBitmapElement::BITMAP_VALIGN_CENTER);
    mBitmapBackground.m_color.a = 60;
    mMessageGroup.AddElement(&mBitmapBackground);

    mSpectatorPrompt.SetIntercharacterSpacing(0);
    mSpectatorPrompt.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
    mSpectatorPrompt.SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_CENTER);
    mSpectatorPrompt.Enable(false);
    mMessageGroup.AddElement(&mSpectatorPrompt);

    mBitmapChangeView.Enable(false);
    mBitmapChangeView.SetColor(white);
    mBitmapChangeView.SetStyle(RedBitmapElement::BITMAP_STYLE_NORMAL);
    mBitmapChangeView.SetRect(15.0f, 15.0f, RedBitmapElement::BITMAP_HALIGN_LEFT,
                              RedBitmapElement::BITMAP_VALIGN_CENTER);
    mMessageGroup.AddElement(&mBitmapChangeView);

    mTxtChangeView.SetIntercharacterSpacing(0);
    mTxtChangeView.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
    mTxtChangeView.SetTextAlign(RedTextElement::TEXT_HALIGN_LEFT, RedTextElement::TEXT_VALIGN_CENTER);
    mTxtChangeView.Enable(false);
    mMessageGroup.AddElement(&mTxtChangeView);

    mBitmapJoinGame.Enable(false);
    mBitmapJoinGame.SetColor(white);
    mBitmapJoinGame.SetStyle(RedBitmapElement::BITMAP_STYLE_NORMAL);
    mBitmapJoinGame.SetRect(15.0f, 15.0f, RedBitmapElement::BITMAP_HALIGN_LEFT,
                            RedBitmapElement::BITMAP_VALIGN_CENTER);
    mMessageGroup.AddElement(&mBitmapJoinGame);

    mTxtJoinGame.SetIntercharacterSpacing(0);
    mTxtJoinGame.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
    mTxtJoinGame.SetTextAlign(RedTextElement::TEXT_HALIGN_LEFT, RedTextElement::TEXT_VALIGN_CENTER);
    mTxtJoinGame.Enable(false);
    mMessageGroup.AddElement(&mTxtJoinGame);

    mPregameGroup.Enable(false);
    mPregameGroup.SetPosition(fHalfWidth, -100.0f, 0.0f);

    mBitmapBGPregame.Enable(false);
    mBitmapBGPregame.SetStyle(RedBitmapElement::BITMAP_STYLE_NORMAL);
    mBitmapBGPregame.SetPosition(0.0f, 90.0f);
    mBitmapBGPregame.SetColor(white);
    mBitmapBGPregame.SetRect(fScreenWidth * 0.9f, fHeight * fScreenHeight,
                             RedBitmapElement::BITMAP_HALIGN_CENTER,
                             RedBitmapElement::BITMAP_VALIGN_TOP);
    mBitmapBGPregame.m_color.a = 60;
    mPregameGroup.AddElement(&mBitmapBGPregame);

    mPregamePrompt.SetIntercharacterSpacing(0);
    mPregamePrompt.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
    mPregamePrompt.SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_CENTER);
    mPregamePrompt.Enable(false);
    mPregamePrompt.SetPosition(-0.45f, 102.0f);
    mPregameGroup.AddElement(&mPregamePrompt);

    mPregameTime.SetIntercharacterSpacing(0);
    mPregameTime.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
    mPregameTime.SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_CENTER);
    mPregameTime.Enable(false);
    mPregameTime.SetPosition(-0.45f, 123.0f);
    mPregameGroup.AddElement(&mPregameTime);

    mPregamePrompt1.SetIntercharacterSpacing(0);
    mPregamePrompt1.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
    mPregamePrompt1.SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_CENTER);
    mPregamePrompt1.Enable(false);
    mPregamePrompt1.SetPosition(-0.45f, 152.0f);
    mPregameGroup.AddElement(&mPregamePrompt1);

    FLRenderer::AddInterfaceElement(&mPregameGroup, 0.0f, 0.0f, true);
}

// FUNCTION: SWBF2 0x0064f480
SpectatorDisplay::~SpectatorDisplay()
{
}

// FUNCTION: SWBF2 0x0064f580
void SpectatorDisplay::Init()
{
    mMessageGroup.Enable(false);
    mBitmapBackground.SetTexture(0x298058FE);
    mSpectatorPrompt.SetFont(RedFont::GAMEFONT_SMALL);
    mSpectatorPrompt.SetColor(RedColor::YELLOW);
    mSpectatorPrompt.SetText(RedLocalizeDB::Find(0xAC69F600, false), true);

    mBitmapChangeView.SetTexture(0xDD7D0382);
    mTxtChangeView.SetFont(RedFont::GAMEFONT_MEDIUM);
    mTxtChangeView.SetColor(RedColor::GREEN);
    mTxtChangeView.SetText(RedLocalizeDB::Find(0xC7A58A6E, false), true);

    mBitmapJoinGame.SetTexture(0xDC7D01EF);
    mTxtJoinGame.SetFont(RedFont::GAMEFONT_MEDIUM);
    mTxtJoinGame.SetColor(RedColor::GREEN);
    mTxtJoinGame.SetText(RedLocalizeDB::Find(0x1812830D, false), true);

    mPregameGroup.Enable(false);
    mBitmapBGPregame.SetTexture(0x298058FE);
    mPregamePrompt.SetFont(RedFont::GAMEFONT_SMALL);
    mPregamePrompt.SetColor(RedColor::YELLOW);
    mPregamePrompt.m_color.a = 0xb4;
    mPregamePrompt.SetText(RedLocalizeDB::Find(0x2961292A, false), true);

    mPregameTime.SetFont(RedFont::GAMEFONT_SMALL);
    mPregameTime.SetColor(RedColor::GREEN);
    mPregameTime.m_color.a = 0xb4;
    mPregameTime.SetText("60", true);

    mPregamePrompt1.SetFont(RedFont::GAMEFONT_SMALL);
    mPregamePrompt1.m_color.a = 0xb4;
    mPregamePrompt1.SetText(RedLocalizeDB::Find(0x1E648D47, false), true);

    float fWidth = 0.0f;
    float fLeft, fTop, fRight, fBottom;
    mPregamePrompt1.GetRect(fLeft, fTop, fRight, fBottom);
    if (fRight - fLeft > 0.0f)
    {
        fWidth = fRight - fLeft;
    }
    mPregamePrompt.GetRect(fLeft, fTop, fRight, fBottom);
    if (fRight - fLeft > fWidth)
    {
        fWidth = fRight - fLeft;
    }

    float fScreenHeight = (float)RedInterfaceScreen::GetScreenHeight() *
                          RedInterfaceScreen::GetScreenSafeZone();
    float fHeight = NetGame::IsHost() ? 0.15f : 0.06f;
    mBitmapBGPregame.SetRect(fWidth + 32.0f, fHeight * fScreenHeight,
                             RedBitmapElement::BITMAP_HALIGN_CENTER,
                             RedBitmapElement::BITMAP_VALIGN_TOP);
}

// FUNCTION: SWBF2 0x0064f8d0
void SpectatorDisplay::Show()
{
    sCanRender = true;
    if (NetGame::IsEnabled())
    {
        Show(NetGame::GetSpectator(NetGame::GetLocalIndex(mJoystickIndex)) != -1);
    }
}

// FUNCTION: SWBF2 0x0064f920
void SpectatorDisplay::Show(bool bShow)
{
    if (bShow)
    {
        ChangeSpectatedName();
    }
}

// FUNCTION: SWBF2 0x0064f940
void SpectatorDisplay::Hide()
{
    mMessageGroup.Enable(false);
    mPregameGroup.Enable(false);
}

// FUNCTION: SWBF2 0x0064f970
bool SpectatorDisplay::Update(float fDeltaTime)
{
    if (!sCanRender || sDisable || fDeltaTime == 0.0f || GameLoop::IsGameOver())
    {
        Hide();
    }
    else if (NetGame::IsEnabled() && (NetGame::IsClient() || NetGame::IsOneWorld()))
    {
        UpdatePregame();
        ChangeSpectatedName();
        return true;
    }
    return true;
}

// FUNCTION: SWBF2 0x0064fa00
void ShowReadySelectButton(bool bShow, int iIndex)
{
    char acName[64];
    _snprintf(acName, 64, "ifs_readyselect%i.Helptext_Back", iIndex);
    RedInterfaceElement* pElement = RedInterfaceElement::Find(acName);
    if (pElement && pElement->IsEnabled() != bShow)
    {
        pElement->Enable(bShow);
    }
}

// FUNCTION: SWBF2 0x0064fa50
void SpectatorDisplay::ChangeSpectatedName()
{
    if (!sCanRender)
    {
        return;
    }

    int iLocal = NetGame::GetLocalIndex(mJoystickIndex);
    int iSpectator = NetGame::GetSpectator(iLocal);
    SpawnDisplay* pSpawnDisplay = SpawnDisplay::Instance(mJoystickIndex);
    if (!pSpawnDisplay->GetOptionsAccepted() ||
        (!pSpawnDisplay->IsSpectateMode() && iSpectator == -1))
    {
        if (mMessageGroup.IsEnabled())
        {
            mMessageGroup.Enable(false);
            mSpectatorPrompt.Enable(false);
            mBitmapBackground.Enable(false);
            mBitmapChangeView.Enable(false);
            mTxtChangeView.Enable(false);
            mBitmapJoinGame.Enable(false);
            mTxtJoinGame.Enable(false);
        }
        ShowReadySelectButton(true, mJoystickIndex + 1);
        return;
    }

    Character* pPlayer;
    if (iSpectator != -1 && (pPlayer = Character::FindPlayer(iSpectator)) != 0 &&
        pPlayer->mUnit && iSpectator != iLocal)
    {
        unsigned short ausText[128] = { 0 };
        unsigned short ausName[33] = { 0 };
        PblUnicode::ustrncnv(ausName, NetGame::GetPlayerName(iSpectator), 32);
        PblUnicode::usnprintf(ausText, 127, RedLocalizeDB::Find(0xAC69F600, false), ausName);
        mSpectatorPrompt.SetText(ausText, true);
    }
    else
    {
        mSpectatorPrompt.SetText(RedLocalizeDB::Find(0x56889DCC, false), true);
    }

    if (!mMessageGroup.IsEnabled())
    {
        mMessageGroup.Enable(true);
        mSpectatorPrompt.Enable(true);
        mBitmapBackground.Enable(true);
    }
    ShowReadySelectButton(false, mJoystickIndex + 1);
}

// FUNCTION: SWBF2 0x0064fc40
void SpectatorDisplay::UpdatePregame()
{
    if (NetGame::IsPreGame())
    {
        unsigned short ausTime[20] = { 0 };
        PblUnicode::usnprintf(ausTime, 10, "%d", NetGame::GetPreGameTime());
        mPregameTime.SetText(ausTime, true);
        if (!mPregameGroup.IsEnabled())
        {
            mPregameGroup.Enable(true);
            mBitmapBGPregame.Enable(true);
            mPregamePrompt.Enable(true);
            mPregameTime.Enable(true);
            if (NetGame::IsHost())
            {
                mPregamePrompt1.Enable(true);
            }
        }
    }
    else if (mPregameGroup.IsEnabled())
    {
        mPregameGroup.Enable(false);
        mBitmapBGPregame.Enable(false);
        mPregamePrompt.Enable(false);
        mPregameTime.Enable(false);
        mPregamePrompt1.Enable(false);
    }
}

// FUNCTION: SWBF2 0x0064fd80
void SpectatorDisplay::Bind(unsigned int uiViewport)
{
    mJoystickIndex = uiViewport;
    mViewport = CameraManager::sInstance->m_hView[uiViewport];
    ActivateThread(0, PER_FRAME);
    Show();
    SetPosition();
}

// FUNCTION: SWBF2 0x0064fdc0
void SpectatorDisplay::EnableViewport(int iViewport, bool bEnable)
{
    if (sInstance[0] && sInstance[0]->mViewport == iViewport)
    {
        sCanRender = bEnable;
        if (!bEnable)
        {
            sInstance[0]->Hide();
        }
    }
}

// FUNCTION: SWBF2 0x0064fdf0
void SpectatorDisplay::SetPosition()
{
    float fWidth = (float)RedInterfaceScreen::GetScreenWidth() *
                   RedInterfaceScreen::GetScreenSafeZone();
    float fHeight = (float)RedInterfaceScreen::GetScreenHeight() *
                    RedInterfaceScreen::GetScreenSafeZone();
    unsigned int uiNumCam = CameraManager::sInstance->m_iNumCam;
    if (uiNumCam == 2)
    {
        fHeight *= 0.5f;
    }
    else if (uiNumCam == 3 || uiNumCam == 4)
    {
        fWidth *= 0.5f;
        fHeight *= 0.5f;
    }

    float fPromptX = fWidth * 0.5f;
    float fPromptY = fHeight * 0.25f;
    float fButtonY = fHeight * 0.98f;
    float fJoinX = fWidth * 0.78f;
    if (uiNumCam == 2)
    {
        fButtonY = fHeight * 0.9f;
    }
    else if (uiNumCam == 3 || uiNumCam == 4)
    {
        fPromptY = fHeight * 0.35f;
        fJoinX = fWidth * 0.6f;
        fButtonY = fHeight * 0.9f;
    }

    mMessageGroup.SetPosition(0.0f, 0.0f, 0.0f);
    mBitmapBackground.SetPosition(fPromptX, fPromptY);
    mSpectatorPrompt.SetPosition(fPromptX, fPromptY - 3.0f);
    mBitmapChangeView.SetPosition(0.0f, fButtonY);
    mTxtChangeView.SetPosition(20.0f, fButtonY);
    mBitmapJoinGame.SetPosition(fJoinX, fButtonY);
    mTxtJoinGame.SetPosition(fJoinX + 20.0f, fButtonY);
    FLRenderer::AddInterfaceElement(mViewport, &mMessageGroup, RedInterfaceScreen::LEFT,
                                    RedInterfaceScreen::TOP, true);
}

// FUNCTION: SWBF2 0x0064eec0
void SpectatorDisplay::Destroy()
{
    if (sInstance[0])
    {
        delete sInstance[0];
        sInstance[0] = 0;
    }
}
