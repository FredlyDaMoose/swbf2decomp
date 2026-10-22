#include "StatusDisplay.h"
#include "PebbleFL/Common/PblMath.h"

#include <math.h>
#include <stdio.h>

#include "BoundaryManager.h"
#include "CameraManager.h"
#include "Character.h"
#include "Controllable.h"
#include "Entity.h"
#include "EntityCarrier.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "NetGame.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "VoiceChatNode.h"
#include "GameVoiceChatTransport.h"

namespace FLRenderer
{
void AddInterfaceElement(unsigned int hView, RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);

RedInterfaceScreen* GetInterfaceView(int iView);
}

static bool IsFlyerAutoCorrecting(const Controllable* pControllable)
{
    return static_cast<const EntityFlyer*>(pControllable)->mFlyerAutoCorrectingToStayInBattlefield;
}

namespace GameVoiceChat
{
class Transport;
Transport* GetTransport();
}
namespace FriendBuddyList { bool __fastcall IsFriendBuddy(int ignored, int iPlayer); }

extern int gDebugCount;

extern RedColor gVoiceChatTeamColors[2];
extern RedColor gHudHighlightColor;

// GLOBAL: SWBF2 0x01eafb4c
StatusDisplay* StatusDisplay::sInstance[1];

// GLOBAL: SWBF2 0x01eafb41
bool StatusDisplay::mDisable;

// GLOBAL: SWBF2 0x01f6dd94
// SYNTHETIC: SWBF2 0x00406470
// `dynamic initializer for 'SquadCommandGroup::sRelativeX''
float SquadCommandGroup::sRelativeX = RedInterfaceScreen::RIGHT;
// GLOBAL: SWBF2 0x01f6de98
// SYNTHETIC: SWBF2 0x00406490
// `dynamic initializer for 'SquadCommandGroup::sRelativeY''
float SquadCommandGroup::sRelativeY = RedInterfaceScreen::BOTTOM;

float SquadCommandGroup::sFadeSpeed = 0.5f;

const float OutOfBoundsGroup::sOmegaSquared = 500.0f;

static void CountHostCall()
{
    if (NetGame::IsEnabled() && !NetGame::IsClient())
    {
        ++gDebugCount;
    }
}

const float DamageArrowGroup::sWidth = 128.0f;
const float DamageArrowGroup::sHeight = 16.0f;

// FUNCTION: SWBF2 0x006508c0
DamageArrowGroup::DamageArrowGroup()
{
    for (int i = 0; i < 4; ++i)
    {
        Arrow& arrow = mArrows[i];
        arrow.mBitmap.SetRect(sWidth, sHeight, RedBitmapElement::BITMAP_HALIGN_CENTER,
                              RedBitmapElement::BITMAP_VALIGN_BOTTOM);
        arrow.mBitmap.SetMatrixPosRotScale(sData[i].mPosX, sData[i].mPosY, sData[i].mRot, 1.0f, 1.0f);
        arrow.mGroup.AddElement(&arrow.mBitmap);
        arrow.mGroup.Enable(false);
        arrow.mGroup.Visible(false);
        arrow.mAlpha = 1.0f;
    }
}

// FUNCTION: SWBF2 0x00650a80
void DamageArrowGroup::Init()
{
    for (int i = 0; i < 4; ++i)
    {
        Arrow& arrow = mArrows[i];
        arrow.mBitmap.SetTexture(0x3e7f6167u);
    }
}

// FUNCTION: SWBF2 0x00650ab0
void DamageArrowGroup::Bind(unsigned int hView)
{
    for (int i = 0; i < 4; ++i)
    {
        FLRenderer::AddInterfaceElement(hView, &mArrows[i].mGroup, sData[i].mRelativeX,
                                        sData[i].mRelativeY, true);
    }
}

// SYNTHETIC: SWBF2 0x00650a10
// DamageArrowGroup::Arrow::~Arrow

// FUNCTION: SWBF2 0x00650a60
DamageArrowGroup::~DamageArrowGroup()
{
}

// FUNCTION: SWBF2 0x00650b00
void DamageArrowGroup::Enable(bool bEnable)
{
    for (int i = 0; i < 4; ++i)
    {
        mArrows[i].mGroup.Enable(bEnable);
        mArrows[i].mGroup.Visible(bEnable);
    }
}

// FUNCTION: SWBF2 0x00650b30
void DamageArrowGroup::Update(float fDeltaTime)
{
    for (int i = 0; i < 4; ++i)
    {
        Arrow& arrow = mArrows[i];
        if (!arrow.mGroup.IsEnabled())
        {
            continue;
        }
        arrow.mAlpha -= fDeltaTime;
        if (arrow.mAlpha < 0.0f)
        {
            arrow.mGroup.Enable(false);
            arrow.mGroup.Visible(false);
        }
        else
        {
            arrow.mGroup.m_color.a = (unsigned char)(int)(arrow.mAlpha * 255.0f);
        }
    }
}

// FUNCTION: SWBF2 0x00650ba0
void DamageArrowGroup::SetDirection(const PblVector3& vDir)
{
    const float fDiff = vDir.z - vDir.x;
    int iArrow;
    if (vDir.x + vDir.z < 0.0f)
    {
        if (fDiff < 0.0f)
        {
            return;
        }
        iArrow = 2;
    }
    else if (fDiff < 0.0f)
    {
        iArrow = 0;
    }
    else
    {
        iArrow = (vDir.y >= 0.0f) * 2 + 1;
    }
    Arrow& arrow = mArrows[iArrow];
    arrow.mGroup.Enable(true);
    arrow.mGroup.Visible(true);
    arrow.mAlpha = 1.0f;
}

// GLOBAL: SWBF2 0x0078b550
char* SquadCommandGroup::sContexts[4] = { "field", "driver", "passenger", "gunner" };

// GLOBAL: SWBF2 0x0078b560
char* SquadCommandGroup::sOrders[4] = { "up", "down", "right", "left" };

// GLOBAL: SWBF2 0x01eafb50
unsigned short* SquadCommandGroup::sCommands[4][4];

// FUNCTION: SWBF2 0x00650c20
void SquadCommandGroup::Create()
{
    for (int i = 0; i < 4; ++i)
    {
        for (unsigned int j = 0; j < 4; ++j)
        {
            char acBuffer[80];
            _snprintf(acBuffer, sizeof(acBuffer) - 1, "game.squadcommand.%s.%s", sContexts[i], sOrders[j]);
            acBuffer[sizeof(acBuffer) - 1] = 0;
            sCommands[i][j] = (unsigned short*)RedLocalizeDB::Find(PblHash(acBuffer), true);
        }
    }
}

// GLOBAL: SWBF2 0x007e9a60
SquadCommandGroup::CommandText SquadCommandGroup::sTexts[4] =
{
    { 0.0f, -25.0f, RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_BOTTOM },
    { 0.0f, 25.0f, RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_TOP },
    { 25.0f, 0.0f, RedTextElement::TEXT_HALIGN_LEFT, RedTextElement::TEXT_VALIGN_CENTER },
    { -25.0f, 0.0f, RedTextElement::TEXT_HALIGN_RIGHT, RedTextElement::TEXT_VALIGN_CENTER },
};

const float SquadCommandGroup::sPosX = -120.0f;
const float SquadCommandGroup::sPosY = -50.0f;

// FUNCTION: SWBF2 0x00650ca0
SquadCommandGroup::SquadCommandGroup()
{
    for (int i = 0; i < 4; ++i)
    {
        RedTextElement& text = mCommands[i].mText;
        const CommandText& def = sTexts[i];
        text.SetPosition(def.mPosX, def.mPosY);
        text.m_eStyle = RedGlyphCacheElement::TEXT_STYLE_SHADOW;
        text.SetTextAlign(def.mHAlign, def.mVAlign);
        if (def.mHAlign == RedTextElement::TEXT_HALIGN_LEFT)
        {
            float fHeight = fabs(sPosY);
            text.SetTextBox((unsigned int)(fabs(sPosX) - fabs(def.mPosX)), (unsigned int)fHeight);
            text.SetPosition(def.mPosX, def.mPosY - fHeight * 0.5f);
        }
        AddElement(&text);
    }
    Enable(false);
    Visible(false);
    mAlpha = 1.0f;
    SetPosition(sPosX, sPosY);
}

// SYNTHETIC: SWBF2 0x00650e70
// SquadCommandGroup::Command::~Command

// SYNTHETIC: SWBF2 0x00650e30
// SquadCommandGroup::`scalar deleting destructor'
// FUNCTION: SWBF2 0x00650e80
SquadCommandGroup::~SquadCommandGroup()
{
}

// FUNCTION: SWBF2 0x00650ef0
void SquadCommandGroup::Init()
{
    for (int i = 0; i < 4; ++i)
    {
        mCommands[i].mText.SetFont(RedFont::GAMEFONT_TINY);
    }
}

// FUNCTION: SWBF2 0x00650f20
void SquadCommandGroup::Bind(unsigned int hView)
{
    FLRenderer::AddInterfaceElement(hView, this, sRelativeX, sRelativeY, true);
}

// FUNCTION: SWBF2 0x00650f60
void SquadCommandGroup::Update(float fDeltaTime)
{
    if (!IsEnabled())
    {
        return;
    }

    mAlpha -= fDeltaTime * sFadeSpeed;
    if (mAlpha < 0.0f)
    {
        mAlpha = 0.0f;
        Enable(false);
        Visible(false);
    }
    else
    {
        m_color.a = (unsigned char)(mAlpha * 255.0f);
    }
}

// FUNCTION: SWBF2 0x00650fd0
OutOfBoundsGroup::OutOfBoundsGroup(StatusDisplay* pParent) : mParent(pParent)
{
    mText.SetTextScale(1.0f, 1.0f);
    mText.SetIntercharacterSpacing(0);
    mText.SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_CENTER);
    mText.m_eStyle = RedGlyphCacheElement::TEXT_STYLE_SHADOW;
    mText.SetPosition(0.0f, 0.0f);
    AddElement(&mText);
    Enable(false);
    Visible(false);
}

// FUNCTION: SWBF2 0x006510d0
OutOfBoundsGroup::~OutOfBoundsGroup()
{
}

// FUNCTION: SWBF2 0x00651130
void OutOfBoundsGroup::Init()
{
    mText.SetFont(RedFont::GAMEFONT_LARGE);
    mText.Enable(true);
    mText.Visible(true);
    mText.SetColor(RedColor::RED);
    if (const unsigned short* pText = RedLocalizeDB::Find(0x3a1fed50u, false))
    {
        mText.SetText(pText, true);
    }
    else
    {
        mText.SetText("FIXME: Localize game.tooltips.leaving_battlefield", true);
    }
}

// FUNCTION: SWBF2 0x006511c0
void OutOfBoundsGroup::Bind(unsigned int hView)
{
    FLRenderer::AddInterfaceElement(hView, this, RedInterfaceScreen::CENTER,
                                    RedInterfaceScreen::CENTER, true);
}

// FUNCTION: SWBF2 0x006511f0
void OutOfBoundsGroup::Update(float fDeltaTime)
{
    Character* pOwner = mParent->mOwner;
    if (!pOwner)
    {
        return;
    }

    Controllable* pControllable = pOwner->GetControllable();
    if (!pOwner->IsOutOfBounds() &&
        !(pControllable->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer) &&
          IsFlyerAutoCorrecting(pControllable)))
    {
        Enable(false);
        Visible(false);
        return;
    }

    if (!IsEnabled())
    {
        Enable(true);
        Visible(true);
        mAlpha = 255.0f;
        mDAlpha = 0.0f;

        GameObject* pObject = pControllable->GetGameObject();
        BoundaryManager::Instance()->PlayOutOfBoundsVoiceOver(
            pObject->GetTeam());
    }

    mAlpha += mDAlpha * fDeltaTime;
    mDAlpha += (128.0f - mAlpha) * (fDeltaTime * sOmegaSquared);
    int iAlpha = (int)mAlpha;
    if (iAlpha > 255)
    {
        iAlpha = 255;
    }
    else if (iAlpha < 2)
    {
        iAlpha = 2;
    }
    mText.m_color.a = (unsigned char)iAlpha;
}

// FUNCTION: SWBF2 0x00651340
VoiceChatGroup::VoiceChatGroup(StatusDisplay* pParent) : mParent(pParent)
{
    SetPosition(0.0f, 0.0f);
    for (int i = 0; i < 4; ++i)
    {
        mLines[i].mId = -1;
        mLines[i].mText.Enable(false);
        mLines[i].mText.m_eStyle = RedGlyphCacheElement::TEXT_STYLE_SHADOW;
        mLines[i].mText.SetPosition(0.0f, (float)-i * 16.0f);
        AddElement(&mLines[i].mText);
    }
    Enable(false);
}

// FUNCTION: SWBF2 0x00651480
VoiceChatGroup::~VoiceChatGroup()
{
}

void VoiceChatGroup::Init()
{
}

// FUNCTION: SWBF2 0x006514f0
void VoiceChatGroup::Bind(unsigned int hView)
{
    RedFont* pFont = RedFont::Find("systemfont");
    RedInterfaceScreen* pScreen = FLRenderer::GetInterfaceView(hView);
    float fWidth, fHeight, fOffsetX, fOffsetY;
    pScreen->GetSafeZone(fWidth, fHeight, fOffsetX, fOffsetY);
    fWidth = (float)pScreen->m_uiWidthInPixels * fWidth;
    for (int i = 0; i < 4; ++i)
    {
        mLines[i].mText.SetFont(pFont);
        mLines[i].mText.SetTextBox((unsigned int)fWidth - 4, pFont->m_uiHeight);
        mLines[i].mText.SetTextAlign(RedTextElement::TEXT_HALIGN_RIGHT,
                                     RedTextElement::TEXT_VALIGN_CENTER);
    }
    FLRenderer::AddInterfaceElement(hView, this, RedInterfaceScreen::LEFT,
                                    RedInterfaceScreen::CENTER, true);
}

// FUNCTION: SWBF2 0x00651610
void VoiceChatGroup::Update(float fDeltaTime)
{
    AddPlayers();
    UpdateLines();
    Enable(true);
}

// FUNCTION: SWBF2 0x00651630
void VoiceChatGroup::AddPlayers()
{
    GameVoiceChat::Transport* pTransport = GameVoiceChat::GetTransport();
    int iNumNodes = ((const VoiceChat::Transport*)(pTransport))->GetNumNodes();
    int iLocal = NetGame::GetLocalIndex(mParent->mJoystickIndex);
    Character* pLocal = Character::FindPlayer(iLocal);

    int iLine = 0;
    for (int i = 0; i < iNumNodes; ++i)
    {
        Character* pCharacter = Character::FindPlayer(i);
        if (!pCharacter)
        {
            continue;
        }
        if (!pTransport->GetSending((unsigned char)i))
        {
            continue;
        }
        if (!pTransport->GetSendPossible((unsigned char)iLocal, (unsigned char)i))
        {
            continue;
        }

        Line& line = mLines[iLine];
        if (line.mId != i)
        {
            line.mId = i;
            line.mText.SetText(pCharacter->mName, true);
            if (pLocal)
            {
                line.mText.m_color = gVoiceChatTeamColors[pLocal->mTeamNumber != pCharacter->mTeamNumber];
            }
        }
        for (int j = iLine + 1; j < 4; ++j)
        {
            if (mLines[j].mId == i)
            {
                mLines[j].mId = -1;
                mLines[j].mText.Enable(false);
            }
        }
        if (++iLine >= 4)
        {
            break;
        }
    }
}

// FUNCTION: SWBF2 0x006517f0
void VoiceChatGroup::UpdateLines()
{
    int iLocal = NetGame::GetLocalIndex(mParent->mJoystickIndex);
    Character* pLocal = Character::FindPlayer(iLocal);

    for (int i = 0; i < 4; ++i)
    {
        Line& line = mLines[i];
        if (line.mId < 0)
        {
            continue;
        }

        float fAge = VoiceChat::Node::GetTime() - VoiceChat::Node::GetLastReceiveTime((unsigned char)line.mId);
        float fAlpha = 1.0f - fAge * fAge;
        if (fAlpha < 0.0f)
        {
            line.mId = -1;
            line.mText.Enable(false);
            continue;
        }

        Character* pCharacter = Character::FindPlayer(line.mId);
        line.mText.Enable(true);
        line.mText.m_color.a = (unsigned char)(fAlpha * 255.0f);
        if (pCharacter && pLocal)
        {
            line.mText.m_color = gVoiceChatTeamColors[pLocal->mTeamNumber != pCharacter->mTeamNumber];
            if (FriendBuddyList::IsFriendBuddy(0, line.mId))
            {
                line.mText.m_color = gHudHighlightColor;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00651a00
StatusDisplay::StatusDisplay() : mOutOfBoundsGroup(this), mVoiceChatGroup(this)
{
    mCanRender = true;
    mOwner = 0;
    mJoystickIndex = -1;
    mViewport = (unsigned int)-1;
    mVisible = false;
}

// FUNCTION: SWBF2 0x00651ae0
StatusDisplay::~StatusDisplay()
{
}

// FUNCTION: SWBF2 0x00651980
void StatusDisplay::Create()
{
    sInstance[0] = new StatusDisplay;
    SquadCommandGroup::Create();
}

// FUNCTION: SWBF2 0x006519e0
void StatusDisplay::Destroy()
{
    if (sInstance[0])
    {
        delete sInstance[0];
        sInstance[0] = 0;
    }
}

// FUNCTION: SWBF2 0x00651b70
void StatusDisplay::Init()
{
    mOutOfBoundsGroup.Init();
    mDamageArrowGroup.Init();
    mSquadCommandGroup.Init();
    mVoiceChatGroup.Init();
}

// FUNCTION: SWBF2 0x00651ba0
void StatusDisplay::Bind(unsigned int uiViewport)
{
    mViewport = uiViewport;
    mChargeBlinkTimer = 0.0f;
    mJoystickIndex = uiViewport;
    unsigned int hView = CameraManager::sInstance->m_hView[uiViewport];
    mOutOfBoundsGroup.Bind(hView);
    mDamageArrowGroup.Bind(hView);
    mSquadCommandGroup.Bind(hView);
    mVoiceChatGroup.Bind(hView);
    ActivateThread(0, PER_FRAME);
}

// FUNCTION: SWBF2 0x00651c10
void StatusDisplay::Show(Character* pCharacter)
{
    CountHostCall();
    if (!mOwner)
    {
        mOwner = pCharacter;
        mVisible = true;
    }
}

// FUNCTION: SWBF2 0x00651c70
void StatusDisplay::Hide(Character* pCharacter)
{
    CountHostCall();
    if (pCharacter != mOwner)
    {
        return;
    }

    mOwner = 0;
    mVisible = false;
    mOutOfBoundsGroup.Enable(false);
    mOutOfBoundsGroup.Visible(false);
    mDamageArrowGroup.Enable(false);
    mSquadCommandGroup.Enable(false);
    mSquadCommandGroup.Visible(false);
    mVoiceChatGroup.Enable(false);
}

// FUNCTION: SWBF2 0x00651ea0
void StatusDisplay::EnableViewport(unsigned int uiViewport, bool bEnable)
{
    StatusDisplay* pDisplay = sInstance[0];
    if (pDisplay && pDisplay->mViewport == uiViewport)
    {
        pDisplay->mCanRender = bEnable;
        if (!bEnable)
        {
            pDisplay->Hide(pDisplay->mOwner);
        }
    }
}

// FUNCTION: SWBF2 0x00651d40
bool StatusDisplay::Update(float fDeltaTime)
{
    int iPlayer = NetGame::GetLocalIndex(mJoystickIndex);
    int iSpectator = NetGame::GetSpectator(iPlayer);
    if (iSpectator != -1)
    {
        iPlayer = iSpectator;
    }

    Character* pCharacter = Character::FindPlayer(iPlayer);
    if (!pCharacter)
    {
        return true;
    }

    if (mDisable)
    {
        Hide(pCharacter);
        return true;
    }

    if (pCharacter->mUnit && !GameLoop::IsGameOver())
    {
        if (!mVisible)
        {
            Show(pCharacter);
        }
        if (mOwner != pCharacter)
        {
            mOwner = pCharacter;
        }
    }
    else if (mVisible)
    {
        Hide(pCharacter);
    }

    if (mVisible && mOwner && mCanRender && fDeltaTime != 0.0f && mOwner->mUnit)
    {
        Controllable* pPilot = mOwner->GetControllable()->GetPilot();
        GameObject* pPilotObject = pPilot ? pPilot->GetGameObject() : 0;
        mDamageArrowGroup.Update(fDeltaTime);
        mOutOfBoundsGroup.Update(fDeltaTime);
        mVoiceChatGroup.Update(fDeltaTime);
        if (pPilotObject)
        {
            mSquadCommandGroup.Update(fDeltaTime);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x006509c0
DamageArrowGroup::Arrow::Arrow()
{
}

// FUNCTION: SWBF2 0x00650e60
SquadCommandGroup::Command::Command()
{
}

// GLOBAL: SWBF2 0x007ec2d0
// SYNTHETIC: SWBF2 0x00406400
// `dynamic initializer for 'DamageArrowGroup::sData''
DamageArrowGroup::Data DamageArrowGroup::sData[4] =
{
    { RedInterfaceScreen::RIGHT, RedInterfaceScreen::CENTER, 0.0f, 0.0f, -PblMath::fHalfPi },
    { RedInterfaceScreen::CENTER, RedInterfaceScreen::BOTTOM, 0.0f, 0.0f, 0.0f },
    { RedInterfaceScreen::LEFT, RedInterfaceScreen::CENTER, 0.0f, 0.0f, PblMath::fHalfPi },
    { RedInterfaceScreen::CENTER, RedInterfaceScreen::TOP, 0.0f, 0.0f, PblMath::fPi },
};

extern RedColor gFriendColor;
extern RedColor gEnemyColor;

// GLOBAL: SWBF2 0x007ec388

// SYNTHETIC: SWBF2 0x00406940
// `dynamic initializer for 'gVoiceChatTeamColors''
RedColor gVoiceChatTeamColors[2] = { gFriendColor, gEnemyColor };

// GLOBAL: SWBF2 0x007ec390, BGRA bytes CA F9 10 FF.

RedColor gHudHighlightColor = { 0x10, 0xF9, 0xCA, 0xFF };
