#include "BootDisplay.h"
#include "CameraManager.h"
#include "Character.h"
#include "FLJoystick.h"
#include "GameLoop.h"
#include "HeroMessageDisplay.h"
#include "NetComm.h"
#include "NetGame.h"
#include "PauseMenu.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

#include "SoundFL/source/win/sndengine.h"
#include "PebbleFL/Common/PblTime.h"
#include "FLInputManager.h"
#include "GameVoiceChat.h"
#include "GameVoiceChatNode.h"
#include "GameVoiceChatUtil.h"
#include "VoiceChatNode.h"
#include "VoiceChatTransport.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "RedEngineFL/input/RedKeyboard.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "BoundaryManager.h"
#include "CaptureDisplay.h"
#include "Cloth.h"
#include "CollisionMesh.h"
#include "EntityHologram.h"
#include "GrassPatch.h"
#include "LeafPatch.h"
#include "LockOnManager.h"
#include "LuaScript.h"
#include "MessageDisplay.h"
#include "MuzzleFlash.h"
#include "OrdnanceGrapplingHook.h"
#include "OrdnanceTowCable.h"
#include "ParticleEmitter.h"
#include "ParticleEmitterObject.h"
#include "PatrolPath.h"
#include "ScreenTransition.h"
#include "SkyDome.h"
#include "SpectatorDisplay.h"
#include "StatIconDisplay.h"
#include "Tracker.h"
#include "VehicleSpawn.h"
#include "Battlefront2/Platform/Pc/pcGrassPatch.h"
#include "Battlefront2/Platform/Pc/pcLeafPatch.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Interface/RedGlyphCacheElement.h"
#include "RedEngineFL/Interface/RedInterfaceElement.h"
#include "RedEngineFL/Util/ext/IAllocator.h"
#include "RedEngineFL/Util/ext/hashtable.h"
#include "PebbleFL/Common/PblRegion.h"
#include "CollisionManager.h"
#include "CommandLine.h"
#include "GameMovie.h"
#include "ProceduralAnimation.h"
#include "ShellLoop.h"
#include "RedEngineFL/Interface/RedShadowBlob.h"

namespace FLRenderer
{
void AddInterfaceElement(unsigned int hView, RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
}

namespace PblUnicode { unsigned int ustrncnv(unsigned short* pDest, const unsigned char* pSrc, unsigned int uiCount); }

// FUNCTION: SWBF2 0x0044d280
void BootDisplay::Create()
{
    sInstance[0] = new BootDisplay;
}

// FUNCTION: SWBF2 0x0044d310
BootDisplay::BootDisplay()
{
    mCanRender = true;

    mStatusText0.Enable(false);
    mStatusText0.SetPosition(20.0f, 140.0f);
    mStatusText0.SetIntercharacterSpacing(0);
    mStatusText0.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
    mStatusText0.Enable(false);

    mStatusText1.Enable(false);
    mStatusText1.SetPosition(20.0f, 155.0f);
    mStatusText1.SetIntercharacterSpacing(0);
    mStatusText1.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
    mStatusText1.Enable(false);

    mStatusText2.Enable(false);
    mStatusText2.SetPosition(20.0f, 170.0f);
    mStatusText2.SetIntercharacterSpacing(0);
    mStatusText2.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
    mStatusText2.Enable(false);

    mLobbyMode = false;
    mVisible = true;
}

// FUNCTION: SWBF2 0x0044d4a0
BootDisplay::~BootDisplay()
{
}

// FUNCTION: SWBF2 0x0044d520
void BootDisplay::Init()
{
    mStatusText0.SetFont(RedFont::GAMEFONT_TINY);
    mStatusText1.SetFont(RedFont::GAMEFONT_TINY);
    mStatusText2.SetFont(RedFont::GAMEFONT_TINY);
}

// FUNCTION: SWBF2 0x0044d560
void BootDisplay::Bind(unsigned int uiViewport)
{
    mJoystickIndex = uiViewport;
    mViewport = CameraManager::sInstance->m_hView[uiViewport];
    mStatusGroup.AddElement(&mStatusText0);
    mStatusGroup.AddElement(&mStatusText1);
    mStatusGroup.AddElement(&mStatusText2);
    FLRenderer::AddInterfaceElement(mViewport, &mStatusGroup, RedInterfaceScreen::LEFT,
                                    RedInterfaceScreen::TOP, true);
    ActivateThread(NetGame::IsEnabled() ? -101 : -1, PER_LOCAL_TURN);
}

// FUNCTION: SWBF2 0x0044d610
void BootDisplay::Show(Character* pOwner)
{
    if (!mOwner)
    {
        mOwner = pOwner;
        mVisible = true;
    }
}

// FUNCTION: SWBF2 0x0044d630
void BootDisplay::Hide(Character* pOwner)
{
    if (pOwner == mOwner)
    {
        Hide();
    }
}

// FUNCTION: SWBF2 0x0044d650
void BootDisplay::Hide()
{
    mOwner = NULL;
    mVisible = false;
    mStatusText0.Enable(false);
    mStatusText1.Enable(false);
    mStatusText2.Enable(false);
}

// FUNCTION: SWBF2 0x0044de40
void BootDisplay::EnableViewport(int iViewport, bool bEnable)
{
    BootDisplay* pDisplay = sInstance[0];
    if (pDisplay && pDisplay->mViewport == iViewport)
    {
        pDisplay->mCanRender = bEnable;
        if (!bEnable)
        {
            pDisplay->Hide();
        }
    }
}

// FUNCTION: SWBF2 0x0044d6a0
void BootDisplay::SetSpawnMode(bool bSpawnMode)
{
    if (mSpawnMode != bSpawnMode)
    {
        mSpawnMode = bSpawnMode;
        UpdateMode();
    }
}

// FUNCTION: SWBF2 0x0044d6c0
void BootDisplay::SetLobbyMode(bool bLobbyMode)
{
    if (mLobbyMode != bLobbyMode)
    {
        mLobbyMode = bLobbyMode;
        UpdateMode();
    }
}

// FUNCTION: SWBF2 0x0044d6e0
void BootDisplay::UpdateMode()
{
    mStatusGroup.m_fScreenRelativeX = RedInterfaceScreen::LEFT;
    mStatusGroup.m_fScreenRelativeY = RedInterfaceScreen::TOP;
    if (mLobbyMode)
    {
        mStatusText0.SetPosition(20.0f, 140.0f);
        mStatusText1.SetPosition(20.0f, 155.0f);
        mStatusText2.SetPosition(20.0f, 170.0f);
    }
    else if (mSpawnMode)
    {
        mStatusText0.SetPosition((float)RedInterfaceScreen::GetScreenWidth() * 0.3f + 20.0f, 140.0f);
        mStatusText1.SetPosition((float)RedInterfaceScreen::GetScreenWidth() * 0.3f + 20.0f, 155.0f);
        mStatusText2.SetPosition((float)RedInterfaceScreen::GetScreenWidth() * 0.3f + 20.0f, 170.0f);
    }
    else
    {
        mStatusText0.SetPosition(0.0f, (float)RedInterfaceScreen::GetScreenHeight() * 0.29f);
        mStatusText1.SetPosition(0.0f, (float)RedInterfaceScreen::GetScreenHeight() * 0.29f + 15.0f);
        mStatusText2.SetPosition(0.0f, (float)RedInterfaceScreen::GetScreenHeight() * 0.29f + 30.0f);
    }
}

// FUNCTION: SWBF2 0x0044d920
const unsigned short* BootDisplay::GetBootReasonString(int iReason)
{
    switch (iReason)
    {
    case 0: return RedLocalizeDB::Find(0x649f6f47u, false);
    case 1: return RedLocalizeDB::Find(0x79f094d6u, false);
    case 2: return RedLocalizeDB::Find(0x2e3928adu, false);
    case 3: return RedLocalizeDB::Find(0xf10330b9u, false);
    case 4: return RedLocalizeDB::Find(0xd1cbb520u, false);
    case 5: return RedLocalizeDB::Find(0x145b8b50u, false);
    }
    return NULL;
}

// FUNCTION: SWBF2 0x0044d9b0
bool BootDisplay::Update(float fDeltaTime)
{
    RedTextElement* pText0 = &mStatusText0;
    pText0->Enable(false);
    RedTextElement* pText1 = &mStatusText1;
    pText1->Enable(false);
    RedTextElement* pText2 = &mStatusText2;
    pText2->Enable(false);

    if (!mCanRender || fDeltaTime == 0.0f || GameLoop::IsGameOver() ||
        PauseMenu::GetPaused(mJoystickIndex))
    {
        return true;
    }
    if (HeroMessageDisplay::Instance(mJoystickIndex)->IsInHeroSelecting())
    {
        return true;
    }

    Character* pPlayer = Character::FindPlayer(NetGame::GetLocalIndex(mJoystickIndex));
    if (!pPlayer)
    {
        Hide();
    }
    else if (mSpawnMode || pPlayer->mUnit)
    {
        if (!mVisible)
        {
            Show(pPlayer);
        }
    }
    else
    {
        if (!mVisible)
        {
            return true;
        }
        Hide(pPlayer);
    }
    if (!mVisible)
    {
        return true;
    }

    int iBootPlayer = NetGame::GetBootPlayer();
    float fTimeLeft;
    if (iBootPlayer < 0 || (fTimeLeft = NetGame::GetBootTime() - NetComm::GetTime()) <= 0.0f)
    {
        NetGame::SetBootVoted(NetGame::GetJoystickIndex(pPlayer->mPlayerId), false);
        return true;
    }

    RedColor color;
    if (NetGame::IsBootVoted(mJoystickIndex))
    {
        color = RedColor::GREEN;
    }
    else if (fTimeLeft > 15.0f)
    {
        color = RedColor::WHITE;
    }
    else
    {
        color = RedColor::RED;
    }
    mStatusText0.SetColor(color);
    mStatusText1.SetColor(color);
    mStatusText2.SetColor(color);

    unsigned short text[256];
    unsigned short name[64];
    const unsigned short* pFormat = RedLocalizeDB::Find(0x719fdb6fu, false);
    PblUnicode::ustrncnv(name, (const unsigned char*)NetGame::GetPlayerName(iBootPlayer), 64);
    int iSeconds = (int)fTimeLeft;
    PblUnicode::usnprintf(text, 256, pFormat, name, NetGame::GetBootVotes(), iSeconds);

    int iCountA, iCountB, iNeeded;
    NetGame::GetBootCount(iCountA, iCountB, iNeeded);
    iNeeded -= NetGame::GetBootVotes();

    const unsigned short* pReason = GetBootReasonString(NetGame::GetBootReason());
    PblUnicode::usnprintf(text, 256, RedLocalizeDB::Find(0xd9a2b95au, false), name, pReason);
    pText0->SetText(text, true);
    pText0->Enable(true);

    PblUnicode::usnprintf(text, 256, RedLocalizeDB::Find(0xd8a2b7c7u, false), iSeconds, iNeeded);
    pText1->SetText(text, true);
    pText1->Enable(true);

    pText2->Enable(false);

    int iPlayerId = pPlayer->mPlayerId;
    int iJoystick = NetGame::GetJoystickIndex(iPlayerId);
    if (NetGame::IsBootVoted(iJoystick))
    {
        return true;
    }

    Character* pBooted = Character::FindPlayer(iBootPlayer);
    if (iPlayerId == iBootPlayer || !pBooted || pPlayer->mTeamNumber != pBooted->mTeamNumber)
    {
        return true;
    }

    PblUnicode::usnprintf(text, 256, RedLocalizeDB::Find(0xd7a2b634u, false));
    pText2->SetText(text, true);
    pText2->Enable(true);

    ProcessedInputs* pInputs = NetGame::GetJoystick(iPlayerId);
    if (pInputs)
    {

        if ((pInputs->bits.mBoolInputs >> 10) & 1)
        {
            NetGame::ConfirmBootPlayer(iJoystick);
        }
        else if ((pInputs->bits.mBoolInputs >> 11) & 1)
        {
            NetGame::SetBootVoted(iJoystick, true);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0044d2f0
void BootDisplay::Destroy()
{
    if (sInstance[0])
    {
        delete sInstance[0];
        sInstance[0] = 0;
    }
}

// GLOBAL: SWBF2 0x01e3031c
BootDisplay* BootDisplay::sInstance[1];
