#include <stdio.h>
#include <string.h>

#include "pcSpawnDisplay.h"

#include "Battlefront2/Source/BootDisplay.h"
#include "Battlefront2/Source/CameraManager.h"
#include "Battlefront2/Source/Character.h"
#include "Battlefront2/Source/Controllable.h"
#include "Battlefront2/Source/CommandPost.h"
#include "Battlefront2/Source/Damageable.h"
#include "Battlefront2/Source/EventManager_Character.h"
#include "Battlefront2/Source/EntityDroideka.h"
#include "Battlefront2/Source/EntitySoldier.h"
#include "Battlefront2/Source/FlashyTextElement.h"
#include "Battlefront2/Source/LuaHelper.h"
#include "Battlefront2/Source/NetComm.h"
#include "Battlefront2/Source/FLGuiManager.h"
#include "Battlefront2/Source/FLInputManager.h"
#include "Battlefront2/Source/FLJoystick.h"
#include "Battlefront2/Source/GameLoop.h"
#include "Battlefront2/Source/GameObject.h"
#include "Battlefront2/Source/HeroMessageDisplay.h"
#include "Battlefront2/Source/HUDEvent.h"
#include "Battlefront2/Source/HUDGameEvents.h"
#include "Battlefront2/Source/HUDElementMap.h"
#include "Battlefront2/Source/MedalsMgr.h"
#include "Battlefront2/Source/NetGame.h"
#include "Battlefront2/Source/PauseMenu.h"
#include "Battlefront2/Source/PlayerController.h"
#include "Battlefront2/Source/PlayerStats.h"
#include "Battlefront2/Source/ScreenTransition.h"
#include "Battlefront2/Source/SpawnManager.h"
#include "Battlefront2/Source/Team.h"
#include "Battlefront2/Source/Tracker.h"
#include "Battlefront2/Source/Weapon.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Graphics/RedAmbientLight.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/input/pc/pcRedInputManager.h"
#include "RedEngineFL/input/RedKeyboard.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "SoundFL/source/sndbus.h"

// FUNCTION: SWBF2 0x0042cb80
void SpawnDisplay::UpdateSpectate(Character* pCharacter)
{
    if (!mSpectateMode)
    {
        return;
    }

    char screen[32];
    _snprintf(screen, sizeof(screen), "ifs_pc_Spectator");
    const char* pCurrent = m_pGuiManager->GetCurrentScreen();
    if (strcmp(screen, pCurrent) && strcmp(pCurrent, "ifs_pausemenu") &&
        !m_pGuiManager->IsScreenInStack(screen))
    {
        if (*pCurrent)
        {
            m_pGuiManager->PopScreen(0);
        }
        m_pGuiManager->PushScreen(screen, 0, 0);
    }

    if (mVisible)
    {
        Hide(pCharacter);
        return;
    }

    GUIInputs* pInputs = FLInputManager::s_instance.GetPlayerGuiInputs(mJoystickIndex);
    if (!pInputs->IsAcceptPressed())
    {
        return;
    }
    if (m_pButton_JoinGame->m_pHotSpot &&
        m_pButton_JoinGame->TestHotSpot((float)pInputs->mMouseCoords.mPosX,
                                         (float)pInputs->mMouseCoords.mPosY))
    {
        mSpectateMode = false;
        mMode = eSIDESELECT;
        Show(pCharacter);
    }
    else if (pCharacter && !Tracker::GetTracker(pCharacter->mPlayerId)->mTrackable)
    {
        CameraManager::sInstance->SetChaseCameraRandomShot(mJoystickIndex);
    }
}

// FUNCTION: SWBF2 0x0042b620
void __fastcall SpawnDisplay::EnableViewport(int iViewport)
{
    HUD::Event event;
    event.mClass = HUD::GameEvents::gPlayerEvents[iViewport].spawnDisplayDisable;
    event.mData.boolValue = true;
    event.Send();
}

extern int g_InGameMoviePlayerState;

namespace HUD { namespace GameEvents { void DisableAllForObjectiveDisplay(int iPlayer, bool bFlag); } }

// FUNCTION: SWBF2 0x005636b0
void HUD::GameEvents::DisableAllForObjectiveDisplay(int iPlayer, bool bFlag)
{
    if (iPlayer < 0)
    {
        return;
    }

    HUD::GameEvents::PlayerEvents& events = HUD::GameEvents::gPlayerEvents[iPlayer];
    HUD::GameEvents::PlayerData& data = HUD::GameEvents::gPlayerData[iPlayer];
    HUD::GameEvents::DisablePlayerElements(events, data);
    if (bFlag)
    {
        HUD::GameEvents::DisableTeamElements(events, data);
    }
    HUD::Event(events.mapDisable, true).Send();
    data.mapMode = (HUD::ElementMap::Mode)3;
    HUD::Event(events.disableToolTips, true).Send();
    HUD::Event(HUD::GameEvents::gObjectiveEvents.timerDisable, true).Send();
}

namespace GameLoop { bool GetCanSwitchSides(); }

extern bool gAttractMode;

extern bool netAutoAssignTeams;

extern bool netHeroesEnabled;
extern bool netHeroUnlocked[3];
extern int netHeroPlayerID[3];

extern int gDebugCount;
extern float netSecondsPerTurn;

// GLOBAL: SWBF2 0x01e2fd12

static bool gEnableHeroes;

// GLOBAL: SWBF2 0x01f9ae5c

static SpawnDisplay* sInstance;

// GLOBAL: SWBF2 0x0078871c

static const unsigned short sNewlineStr16[2] = { '\n', 0 };

// GLOBAL: SWBF2 0x01e2fd13

static bool sSlotsIncludeHeroes;

// GLOBAL: SWBF2 0x007e5ee4

float SpawnDisplay::sInGameBusGain = 0.2f;

// GLOBAL: SWBF2 0x007e5ee8

float SpawnDisplay::sInGameBusFadeTime = 3.0f;

// GLOBAL: SWBF2 0x01e2fd40
unsigned int SpawnDisplay::sNumLiveHumansTeam2;

// GLOBAL: SWBF2 0x01e2fd44
unsigned int SpawnDisplay::sNumLiveHumansTeam1;

// GLOBAL: SWBF2 0x01e2fd48

GuiManager* SpawnDisplay::m_pGuiManager;

namespace
{

void SendEvent(HUD::EventClass* pClass, bool bValue)
{
    HUD::Event event;
    event.mClass = pClass;
    event.mData.boolValue = bValue;
    event.Send();
}

void SendEvent(HUD::EventClass* pClass, int iValue)
{
    HUD::Event event;
    event.mClass = pClass;
    event.mData.intValue = iValue;
    event.Send();
}

const Damageable* GetDamageable(const GameObject* pObject)
{
    return static_cast<const Damageable*>(pObject);
}

}

 
 
 
 
 
 
 
 
 
 
 
 
 

namespace
{

RedTextElement* AsText(RedInterfaceElement* pElement)
{
    if (pElement->IsFlashyText())
    {
        return static_cast<RedTextElement*>(pElement);
    }
    return pElement->IsText() ? static_cast<RedTextElement*>(pElement) : 0;
}

}

// FUNCTION: SWBF2 0x0042b600
static bool IsLocal(int iPlayerIdx)
{
    return NetGame::GetLocalIndex(iPlayerIdx) >= 0;
}

// FUNCTION: SWBF2 0x00429560
SpawnDisplay::SpawnDisplay(int  )
{
    mMode = eSIDESELECT;
    mNetWaitingForTeam = false;
    mCanRender = true;
    mStickMoved = false;
    mVisible = false;
    mFirstShow = true;
    mLastSpawnTime = 0.0f;
    mBaseTurn = 0;
    mBaseCycleTime = 0.0f;
    mVisibleTimer = 0.0f;
    mOwner = 0;
    mSpawn = 0;
    mPlayerIdx = 0;
    mLastAcceptPressed = false;
    mDPadPressed = false;
    mLastObjectClass = 0;
    mLastInfoClass = 0;
    mLastUnitCountClass = 0;
    mTeamDisplayRot = 0.0f;
    mTeamNumber = 0;
    mCommandPost = 0;
    m_pSideIcon[0] = 0;
    m_pSideIcon[1] = 0;
    m_AcceptBox = 0;
    m_pUnitCountText = 0;
    mJoystickIndex = 0;
    mSpectateMode = false;
    mWasOwnerAliveLastUpdate = false;
    sSlotsIncludeHeroes = false;
}

// SYNTHETIC: SWBF2 0x00429780
// SpawnDisplay::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004297c0
SpawnDisplay::~SpawnDisplay()
{
    mTextReady.Cleanup();
    mTextSpawnTimer.Cleanup();
    for (int i = 0; i < 2; ++i)
    {
        if (m_pSideIcon[i])
        {
            m_pSideIcon[i]->RemoveHotSpot();
        }
    }
    if (m_AcceptBox)
    {
        m_AcceptBox->RemoveHotSpot();
    }
}

// FUNCTION: SWBF2 0x00429940
void SpawnDisplay::CreateClient()
{
    sInstance->DeactivateThread();
    NetGame::EnterClient();
    sInstance->ActivateThread(0, PER_LOCAL_TURN);
    NetGame::LeaveClient();
}

// FUNCTION: SWBF2 0x00429970
void SpawnDisplay::Create()
{
    sInstance = new SpawnDisplay(0);
    if (!(NetGame::IsHost() && NetGame::IsDedicated()))
    {
        sInstance->ActivateThread(0, PER_TURN);
        m_pGuiManager = GuiManager::mInstances;
    }
}

// FUNCTION: SWBF2 0x00429a00
void SpawnDisplay::Destroy()
{
    if (sInstance)
    {
        delete sInstance;
        sInstance = 0;
    }
}

// FUNCTION: SWBF2 0x00429a20
SpawnDisplay* SpawnDisplay::Instance(int)
{
    return sInstance;
}

// FUNCTION: SWBF2 0x00429a30
void SpawnDisplay::Init()
{
    if (!(NetGame::IsHost() && NetGame::IsDedicated()))
    {
        sNumLiveHumansTeam1 = 0;
        sNumLiveHumansTeam2 = 0;
        Instance(0)->Initialize();
    }
}

// FUNCTION: SWBF2 0x00429a70
void SpawnDisplay::UpdateUnitCountText()
{
    if (!m_pUnitCountText)
    {
        return;
    }

    if (mLastUnitCountClass == mOwner->mClassPtr)
    {
        return;
    }
    mLastUnitCountClass = mOwner->mClassPtr;

    GameObjectClass* pClass = (GameObjectClass*)mOwner->mClassPtr;
    unsigned short text[256];
    unsigned short countText[128];
    PblUnicode::usnprintf(text, 255, "");
    PblUnicode::usnprintf(countText, 127, "");

    Team* pTeam = Team::sTeams[mTeamNumber];
    if (!pTeam)
    {
        return;
    }

    PlayerStats::UnselectableReason eReason;
    if (!mOwner->GetPlayerStats().IsClassSelectable(pTeam->GetClassIndex(pClass), &eReason))
    {
        switch (eReason)
        {
        case PlayerStats::TOO_MANY_UNITS:
            PblUnicode::ustrlcat(text, RedLocalizeDB::Find(0xD3CD201E, false), 256);
            break;
        case PlayerStats::NOT_ENOUGH_POINTS:
            PblUnicode::usnprintf(text, 255, RedLocalizeDB::Find(0x155AB074, false),
                                  pClass->mPointsToUnlock - mOwner->GetPlayerStats().GetPoints(true));
            break;
        }
    }

    int iIndex = pTeam->GetClassIndex(pClass);
    int iCurUnits = pTeam->GetClassCurrentUnits(iIndex);
    int iMaxUnits = pTeam->GetClassMaxUnits(iIndex);
    if (iMaxUnits > Character::sCharacterLimit || pClass->mPointsToUnlock <= 0)
    {
        PblUnicode::usnprintf(countText, 127, RedLocalizeDB::Find(0xECF8496F, false), iCurUnits);
    }
    else
    {
        PblUnicode::usnprintf(countText, 127, RedLocalizeDB::Find(0xCB22A09F, false), iCurUnits,
                              iMaxUnits);
    }
    PblUnicode::ustrlcat(text, countText, 256);
    m_pUnitCountText->SetText(text, true);
}

// FUNCTION: SWBF2 0x00429c40
void SpawnDisplay::UpdateObjectText(GameObjectClass* pClass, int iSlot)
{
    if (!IsLocal(mPlayerIdx))
    {
        return;
    }
    if (mLastObjectClass == pClass)
    {
        return;
    }
    mLastObjectClass = pClass;

    unsigned short text[256];
    memset(text, 0, sizeof(text));

    MedalsMgr* pMedals = 0;
    if (mOwner)
    {
        pMedals = mOwner->mMedalsMgr;
    }

    for (int i = 0; i < pClass->GetWeaponCount(); ++i)
    {
        const WeaponClass* pWeapon = pClass->GetWeaponClass(i);
        if (pWeapon)
        {
            bool bAvailable;
            if (pMedals)
            {
                bAvailable = pWeapon->mMedalsTypeToUnlock == -1 ||
                             pMedals->IsAwardAvailable(pWeapon->mMedalsTypeToUnlock);
                if (bAvailable && pWeapon->mMedalsTypeToLock != -1 &&
                    pMedals->IsAwardAvailable(pWeapon->mMedalsTypeToLock))
                {
                    continue;
                }
            }
            else
            {
                bAvailable = pWeapon->mMedalsTypeToUnlock == -1;
            }
            if (!bAvailable)
            {
                continue;
            }

            if (pWeapon->mLabel)
            {
                PblUnicode::ustrlcat(text, pWeapon->mLabel, 256);
            }
            else
            {
                unsigned short name[128];
                memset(name, 0, sizeof(name));
                PblUnicode::ustrncnv(name, pWeapon->mFilename, 128);
                PblUnicode::ustrlcat(text, name, 256);
            }
        }
        PblUnicode::ustrlcat(text, sNewlineStr16, 256);
    }
    m_pInfoBoxText[iSlot]->SetText(text, true);
}

// FUNCTION: SWBF2 0x00429dd0
void SpawnDisplay::Show(Character* pCharacter)
{
    if (!mVisible)
    {
        if (CameraManager::sInstance->m_iNumCam == 1)
        {
            if (Snd::Bus* pBus = Snd::Bus::FindByHashID(0xFA930D1C))
            {
                pBus->Fade(sInGameBusFadeTime, sInGameBusGain, -1.0f);
            }
        }
        SendEvent(HUD::GameEvents::gPlayerEvents[mJoystickIndex].spawnDisplayEnable, true);
        mVisible = true;
        mVisibleTimer = 0.0f;
        ScreenTransition::sInstance->SetTransition(0, ScreenTransition::BLUR);
        mTeamNumber = pCharacter->mTeamNumber;
        mClassIndex = pCharacter->mClassIndex;
        SetupSlots();
        if (!GetCommandPost())
        {
            SelectPost(SpawnManager::Instance()->GetRandomSpawnPoint(mTeamNumber));
        }
        if (CanShowHeroes())
        {
            Team* pTeam = Team::sTeams[mTeamNumber];
            for (int i = 0; i < pTeam->GetClassCount(); ++i)
            {
                if (pTeam->GetClassPtr(i)->mHeroFlag)
                {
                    SelectClass(i);
                    break;
                }
            }
        }
    }

    mOwner = pCharacter;
    mStickMoved = true;
    mLastAcceptPressed = false;
    mDPadPressed = false;
    if (pCharacter && !(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        pCharacter->GetPlayerStats().TimingPause();
    }

    if (!IsLocal(mPlayerIdx))
    {
        return;
    }

    mGroupMainTL.Enable(true);
    mGroupMainTL.Visible(true);
    mGroupMainCC.Enable(true);
    mGroupMainCC.Visible(true);
    mGroupMainCB.Enable(true);
    mGroupMainCB.Visible(true);
    mTextReady.Enable(true);
    mTextReady.Visible(true);
    SendEvent(HUD::GameEvents::gPlayerEvents[mPlayerIdx].mapMode, 2);
    SendEvent(HUD::GameEvents::gPlayerEvents[mPlayerIdx].index, mPlayerIdx);
    BootDisplay::Instance(mPlayerIdx)->SetSpawnMode(true);
    HeroMessageDisplay::Instance(mPlayerIdx)->SetSpawnMode(true);
    mTextSpawnTimer.Enable(false);
    mTextSpawnTimer.Visible(false);

    if (!NetGame::IsEnabled() && mMode == eSIDESELECT && (!GameLoop::GetCanSwitchSides() || gAttractMode))
    {
        mMode = eSPAWNDISPLAY;
    }

    switch (mMode)
    {
    case eSIDESELECT:
        mGroupTimer.Enable(false);
        mGroupTimer.Visible(false);
        mBackdropModel.Enable(false);
        mBackdropModel.Visible(false);
        break;
    case eSPAWNDISPLAY:
        if (!GameLoop::GetCanSwitchSides())
        {
            for (int i = 0; i < 2; ++i)
            {
                m_pSideBG[i]->Enable(false);
                m_pSideBG[i]->Visible(false);
                m_pSideIcon[i]->Enable(false);
                m_pSideIcon[i]->Visible(false);
                m_pSideModel[i]->Enable(false);
                m_pSideModel[i]->Visible(false);
            }
        }
        RedInputManager::GetMouse(0)->SetCursorPos(RedInterfaceScreen::GetScreenWidth() / 2,
                                  RedInterfaceScreen::GetScreenHeight() / 2);

        mGroupTimer.Enable(false);
        mGroupTimer.Visible(false);
        mTextSpawnTimer.SetText("", true);
        SetupSlots();
        mBackdropModel.Enable(false);
        mBackdropModel.Visible(false);
        GetCommandPost();
        break;
    case eCHANGECLASS:
        for (int i = 0; i < 2; ++i)
        {
            m_pSideBG[i]->Enable(false);
            m_pSideBG[i]->Visible(false);
            m_pSideIcon[i]->Enable(false);
            m_pSideIcon[i]->Visible(false);
            m_pSideModel[i]->Enable(false);
            m_pSideModel[i]->Visible(false);
        }
        RedInputManager::GetMouse(0)->SetCursorPos(RedInterfaceScreen::GetScreenWidth() / 2,
                                  RedInterfaceScreen::GetScreenHeight() / 2);
        mGroupTimer.Enable(false);
        mGroupTimer.Visible(false);
        mTextSpawnTimer.SetText("", true);
        SetupSlots();
        break;
    case eFINISHED:
        SendEvent(HUD::GameEvents::gPlayerEvents[mPlayerIdx].mapMode, 0);
        BootDisplay::Instance(mPlayerIdx)->SetSpawnMode(false);
        HeroMessageDisplay::Instance(mPlayerIdx)->SetSpawnMode(false);
        mGroupTimer.Enable(true);
        mGroupTimer.Visible(true);
        mTextSpawnTimer.Enable(true);
        mTextSpawnTimer.Visible(true);
        mBackdropModel.Enable(false);
        mBackdropModel.Visible(false);
        break;
    }

    if (mFirstShow)
    {
        mFirstShow = false;
    }
    else
    {
        CameraManager::sInstance->SetChaseCameraRandomShot(0);
    }
}

// FUNCTION: SWBF2 0x0042a390
void SpawnDisplay::Hide(Character* pCharacter)
{
    if (!pCharacter)
    {
        mOwner = 0;
    }
    else if (!mOwner)
    {
        mOwner = pCharacter;
    }

    bool bWasVisible = mVisible;
    if (bWasVisible)
    {
        if (CameraManager::sInstance->m_iNumCam == 1)
        {
            if (Snd::Bus* pBus = Snd::Bus::FindByHashID(0xFA930D1C))
            {
                pBus->Fade(sInGameBusFadeTime, 1.0f, -1.0f);
            }
        }
        mVisible = false;
        ScreenTransition::sInstance->SetTransition(0, ScreenTransition::BLUR);
    }

    if (mOwner && !(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        mOwner->GetPlayerStats().TimingUnpause();
    }

    mMode = eSPAWNDISPLAY;

    if (!bWasVisible || !IsLocal(mPlayerIdx))
    {
        return;
    }

    int iPlayerIdx = mPlayerIdx;
    if (strcmp("", m_pGuiManager->mCurrentScope))
    {
        m_pGuiManager->PopScreen(0);
    }
    mSpawnPointAccept.Play(0, 0, false, true);
    mTextReady.Enable(false);
    mTextReady.Visible(false);
    mTextSpawnTimer.Enable(false);
    mTextSpawnTimer.Visible(false);
    mGroupMainTL.Enable(false);
    mGroupMainTL.Visible(false);
    mGroupMainCC.Enable(false);
    mGroupMainCC.Visible(false);
    mGroupMainCB.Enable(false);
    mGroupMainCB.Visible(false);
    mGroupTimer.Enable(false);
    mGroupTimer.Visible(false);
    mTextReady.Enable(false);
    mTextReady.Visible(false);
    mTextSpawnTimer.Enable(false);
    mTextSpawnTimer.Visible(false);
    mTextSpawnTimer.SetText("", true);
    SendEvent(HUD::GameEvents::gPlayerEvents[iPlayerIdx].mapMode, 0);
    BootDisplay::Instance(iPlayerIdx)->SetSpawnMode(false);
    HeroMessageDisplay::Instance(iPlayerIdx)->SetSpawnMode(false);
    mBackdropModel.Enable(false);
    mBackdropModel.Visible(false);
    SendEvent(HUD::GameEvents::gPlayerEvents[mJoystickIndex].spawnDisplayDisable, true);
}

// FUNCTION: SWBF2 0x0042a670
void SpawnDisplay::ChangeClass(Character* pCharacter)
{
    PlayerController::sEnabled = 0;
    HUD::GameEvents::DisableAllForObjectiveDisplay(mJoystickIndex, false);
    mMode = eCHANGECLASS;
    Show(pCharacter);
}

// FUNCTION: SWBF2 0x0042a6b0
bool SpawnDisplay::Update(float fDeltaTime)
{
    if (g_InGameMoviePlayerState)
    {
        return true;
    }

    static PblVector3 sLightDir(-0.57735f, -0.57735f, -0.57735f);
    static RedColorValue sLightColor(1.0f, 1.0f, 1.0f, 1.0f);
    static RedColorValue sAmbientTop(0.5f, 0.5f, 0.5f, 1.0f);
    static RedColorValue sAmbientBottom(0.5f, 0.5f, 0.5f, 1.0f);
    RedInterfaceScreen::s_directionalLight[0].SetDirection(sLightDir);
    RedInterfaceScreen::s_directionalLight[0].SetColor(sLightColor);
    RedAmbientLight& ambient = RedInterfaceScreen::s_lightingState.ambientLight;
    ambient.SetBottomColor(sAmbientBottom);
    ambient.SetTopColor(sAmbientTop);
    RedColorValue average((sAmbientBottom.r + sAmbientTop.r) * 0.5f,
                          (sAmbientBottom.g + sAmbientTop.g) * 0.5f,
                          (sAmbientBottom.b + sAmbientTop.b) * 0.5f,
                          (sAmbientBottom.a + sAmbientTop.a) * 0.5f);
    ambient.SetColor(average);

    m_pEntityElement->SetSelected(true);

    if (!mCanRender)
    {
        return true;
    }
    if (NetGame::IsEnabled() && mNetWaitingForTeam)
    {
        return true;
    }

    if (sSlotsIncludeHeroes != CanShowHeroes())
    {
        SetupSlots();
    }

    if (IsLocal(mPlayerIdx) && PauseMenu::GetPaused(0))
    {
        mGroupMainCC.Enable(false);
        mGroupMainCC.Visible(false);
        mGroupMainTL.Enable(false);
        mGroupMainTL.Visible(false);
        return true;
    }

    bool bOwnerAlive = false;
    if (mOwner && mOwner->GetControllable() && mOwner->GetControllable()->GetGameObject())
    {
        bOwnerAlive = GetDamageable(mOwner->GetControllable()->GetGameObject())->IsAlive();
    }
    if (mWasOwnerAliveLastUpdate && !bOwnerAlive)
    {
        mMode = eSPAWNDISPLAY;
    }
    mWasOwnerAliveLastUpdate = bOwnerAlive;

    Character* pPlayer = Character::FindPlayer(NetGame::GetLocalIndex(0));
    if (mMode == eCHANGECLASS || !pPlayer || GameLoop::IsGameOver())
    {
        if (GameLoop::IsGameOver() || !IsLocal(mPlayerIdx))
        {
            Hide(pPlayer);
        }
    }
    else if ((pPlayer->mTeamPtr && pPlayer->mTeamPtr->mCurReinforcements <= 0) || pPlayer->mUnit ||
             pPlayer->GetDelayTimer() > 0.0f)
    {
        if (!mVisible)
        {
            return true;
        }
        Hide(pPlayer);
    }
    else if (mSpectateMode)
    {
        UpdateSpectate(pPlayer);
        return true;
    }
    else if (!mVisible)
    {
        Show(pPlayer);
    }

    if (!mVisible || fDeltaTime == 0.0f)
    {
        return true;
    }

    CommandPost* pPost = GetCommandPost();
    if (pPost != mCommandPost)
    {
        SelectPost(pPost);
        if (mMode == eFINISHED)
        {
            mMode = eSPAWNDISPLAY;
        }
    }

    mVisibleTimer += fDeltaTime;
    if (mTeamDisplayRot > 0.0f)
    {
        mTeamDisplayRot -= fDeltaTime * 15.0f;
    }
    else
    {
        mTeamDisplayRot = 0.0f;
    }

    GUIInputs* pInputs = FLInputManager::s_instance.GetPlayerGuiInputs(mPlayerIdx);
    float fMouseX = (float)pInputs->mMouseCoords.mPosX;
    float fMouseY = (float)pInputs->mMouseCoords.mPosY;
    switch (mMode)
    {
    case eSIDESELECT:
        if (m_pEntityElement)
        {
            m_pEntityElement->Update(fDeltaTime);
        }
        if (pInputs->IsAcceptPressed() && GetTeamSelection())
        {
            AcceptSideSelect();
            mMode = eSPAWNDISPLAY;
        }
        break;
    case eSPAWNDISPLAY:
    case eCHANGECLASS:
        mTeamNumber = mOwner->mTeamNumber;
        if (mSpawnReady)
        {
            mSpawnReady = false;
            SendSpawnCancelMessage();
        }
        UpdateInput();
        m_pEntityElement->SetColor(
            mOwner->GetPlayerStats().IsClassSelectable(mOwner->mClassIndex, 0) ? RedColor::WHITE
                                                                               : RedColor::BLACK);
        UpdateUnitCountText();
        m_pEntityElement->Update(fDeltaTime);
        if (mMode == eSPAWNDISPLAY && pInputs->IsAcceptPressed() &&
            m_pBackButton1->TestHotSpot(fMouseX, fMouseY) && NetGame::IsEnabled() && !netAutoAssignTeams)
        {
            mMode = eSIDESELECT;
        }
        break;
    case eFINISHED:
        if (!mSpawnReady)
        {
            mSpawnReady = true;
            SendSpawnReadyMessage();
        }
        if (pInputs->IsBackPressed())
        {
            mMode = eSPAWNDISPLAY;
        }
        if (pInputs->IsAcceptPressed())
        {
            if (m_pBackButton->TestHotSpot(fMouseX, fMouseY))
            {
                mMode = eSPAWNDISPLAY;
            }
            else
            {
                CameraManager::sInstance->SetChaseCameraRandomShot(0);
            }
        }
        break;
    }

    if (IsLocal(mPlayerIdx))
    {
        ChangeMode(mMode);
    }
    UpdateTimer();
    return true;
}

// FUNCTION: SWBF2 0x0042acb0
void SpawnDisplay::UpdateTimer()
{
    if (mMode == eFINISHED && !(mCommandPost && mCommandPost->CanTeamSpawnHere(mTeamNumber)))
    {
        mMode = eSPAWNDISPLAY;
        return;
    }

    float fTime;
    bool bCountdown = false;
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        unsigned int uTurn = NetGame::GetTurnNumber();
        fTime = mLastSpawnTime;
        if (fTime > 0.0f)
        {
            fTime = mBaseCycleTime - (float)(int)(uTurn - mBaseTurn) * netSecondsPerTurn;
            if (fTime < 0.0f)
            {
                fTime = 0.0f;
            }
        }
        if (mTextSpawnTimer.IsEnabled() && fTime != mLastSpawnTime)
        {
            mLastSpawnTime = fTime;
            char acText[32];
            _snprintf(acText, 32, "%d", (int)fTime + 1);
            mTextSpawnTimer.SetText(acText, true);
            if (fTime == 0.0f)
            {
                if (Character* pPlayer = Character::FindPlayer(NetGame::GetLocalIndex(0)))
                {
                    pPlayer->SetSpawnTime(NetComm::GetTime());
                }
            }
        }
    }
    else
    {
        SpawnManager* pManager = SpawnManager::Instance();
        int iTeam = mOwner->mTeamNumber;
        float fCycles = (float)(mOwner->GetSpawnCycle() - pManager->GetCycleIndex(iTeam)) - 1.0f;
        fTime = pManager->GetCycleDelay(iTeam) * fCycles + pManager->GetCycleTimer(iTeam);
        if (!(fCycles < 0.0f))
        {
            mBaseTurn = NetGame::GetTurnNumber();
            mBaseCycleTime = fTime;
            bCountdown = true;
        }
        if (NetGame::IsEnabled() &&
            (!NetGame::IsOneWorld() || NetGame::IsDedicated() || NetGame::GetLocalIndex(0) < 0))
        {
            return;
        }
        if (!mTextSpawnTimer.IsEnabled())
        {
            return;
        }
    }

    unsigned int uLiveHumans = mTeamNumber == 1 ? sNumLiveHumansTeam1 : sNumLiveHumansTeam2;
    int iUnitCount = SpawnManager::Instance()->GetUnitCount(mTeamNumber);
    if (NetGame::IsEnabled() && fTime < 0.0001f && uLiveHumans >= (unsigned int)iUnitCount)
    {
        unsigned short usText[256];
        PblUnicode::usnprintf(usText, 255,
                              RedLocalizeDB::Find(PblHash("game.spawndisplay.numtospawn"), false),
                              NetGame::GetSpawnOrder(NetGame::GetLocalIndex(0)) + 1);
        mTextSpawnTimer.SetText(usText, true);
    }
    else if (bCountdown)
    {
        char acText[32];
        _snprintf(acText, 32, "%d", (int)fTime + 1);
        mTextSpawnTimer.SetText(acText, true);
    }
}

// FUNCTION: SWBF2 0x0042b590
bool SpawnDisplay::CanShowHeroes()
{
    if (gEnableHeroes)
    {
        return true;
    }
    if (!netHeroesEnabled)
    {
        return false;
    }
    if (NetGame::IsPreGame())
    {
        return false;
    }
    int iLocal = NetGame::GetLocalIndex(0);
    if (iLocal < 0)
    {
        return false;
    }
    Character* pPlayer = Character::FindPlayer(iLocal);
    if (!pPlayer)
    {
        return false;
    }
    int iTeam = pPlayer->mTeamNumber;
    return netHeroUnlocked[iTeam] && netHeroPlayerID[iTeam] > -1 && netHeroPlayerID[iTeam] == iLocal;
}

// FUNCTION: SWBF2 0x0042b740
void SpawnDisplay::SelectTeam(int iTeam)
{
    mTeamNumber = iTeam;
    if (mOwner)
    {
        mOwner->SelectTeam(iTeam);
    }

    Team* pTeam = Team::sTeams[iTeam];
    int iClass = mClassIndex;
    if (iClass < 0)
    {
        iClass = 0;
    }
    else if (iClass > pTeam->GetClassCount() - 1)
    {
        iClass = pTeam->GetClassCount() - 1;
    }
    SelectClass(iClass);
    if (!CanShowHeroes())
    {
        iClass = mClassIndex;
        int iLast = pTeam->GetClassCount() - pTeam->GetSpecialClassCount() - 1;
        if (iClass < 0)
        {
            iClass = 0;
        }
        else if (iClass > iLast)
        {
            iClass = iLast;
        }
        SelectClass(iClass);
    }

    CommandPost* pPost = GetCommandPost();
    if (!pPost || !pPost->GetGameObject() || pPost->GetGameObject()->mTeam != iTeam)
    {
        SelectPost(SpawnManager::Instance()->GetRandomSpawnPoint(mTeamNumber));
    }

    if (mMode != eSIDESELECT)
    {
        mSpawnPointChange.Play(0, 0, false, true);
    }
    m_pSideModel[0]->SetColor(pTeam->mColor[1]);
    m_pSideModel[1]->SetColor(pTeam->mColor[2]);
    LuaHelper::CallGlobalProc("ifs_pc_spawnselect_animateicons", (float)iTeam - 1.0f);
    mTeamDisplayRot = 3.14159265f;
    m_pInfoBoxText[mClassIndex]->m_color.a = 0x80;
    m_pUnitNameText[mClassIndex]->m_color.a = 0x80;
    m_pInfoBoxText[0]->m_color.a = 0xff;
    m_pUnitNameText[0]->m_color.a = 0xff;
    SetupSlots();
}

// FUNCTION: SWBF2 0x0042b8f0
void SpawnDisplay::SelectClass(int iClass)
{
    mClassIndex = iClass;
    if (mOwner)
    {
        mOwner->SelectClass(iClass);
    }
    SetEntityElement(Team::sTeams[mTeamNumber]->GetClassPtr(mClassIndex));
    if (mMode != eSIDESELECT)
    {
        mSpawnPointChange.Play(0, 0, false, true);
    }
}

// FUNCTION: SWBF2 0x0042b960
void SpawnDisplay::SelectPost(CommandPost* pPost)
{
    CommandPost* pGranted = SpawnManager::Instance()->Request(pPost, mTeamNumber);
    SetCommandPost(pGranted);
    if (mOwner)
    {
        mOwner->SelectPost(pGranted);
    }
    if (mMode != eSIDESELECT)
    {
        mSpawnPointChange.Play(0, 0, false, true);
    }
}

// FUNCTION: SWBF2 0x0042b9b0
CommandPost* SpawnDisplay::GetCommandPost()
{
    return SpawnManager::Instance()->Request(mCommandPost, mTeamNumber);
}

// FUNCTION: SWBF2 0x0042b9d0
void SpawnDisplay::SetCommandPost(CommandPost* pPost)
{
    if (!pPost)
    {
        ++gDebugCount;
    }
    mCommandPost = pPost;
    GameObject* pObject = 0;
    if (pPost)
    {
        pObject = pPost->GetGameObject();
    }
    mCommandObject = pObject;
}

// FUNCTION: SWBF2 0x0042ba40
void SpawnDisplay::ChangeMode(Mode)
{
    char acBuffer[128];
    char acScreen[32];
    const char* pcCurrent = m_pGuiManager->GetCurrentScreen();
    switch (mMode)
    {
    case eSIDESELECT:
    {
        _snprintf(acScreen, 32, "ifs_sideselect1");
        if (strcmp(acScreen, pcCurrent) && strcmp(pcCurrent, "ifs_pausemenu") &&
            !m_pGuiManager->IsScreenInStack(acScreen))
        {
            if (*pcCurrent)
            {
                m_pGuiManager->PopScreen(0);
            }
            m_pGuiManager->PushScreen(acScreen, 0, 0);
        }
        sprintf(acBuffer, "ifs_sideselect1.Info");
        if (RedGroupElement* pGroup = m_pEntityElement->GetGroup())
        {
            pGroup->RemoveElement(m_pEntityElement);
        }
        static_cast<RedGroupElement*>(RedInterfaceElement::Find(acBuffer))->AddElement(m_pEntityElement);
        m_pEntityElement->Enable(true);
        m_pEntityElement->Visible(true);
        SendEvent(HUD::GameEvents::gPlayerEvents[mPlayerIdx].mapDisable, true);
        BootDisplay::Instance(0)->SetSpawnMode(false);
        HeroMessageDisplay::Instance(0)->SetSpawnMode(false);
        UpdateSideSelectText(1);
        int iTeam = GetTeamSelection();
        for (int i = 0; i < 2; ++i)
        {
            if (iTeam == i + 1 && iTeam != mTeamNumber)
            {
                SelectTeam(iTeam);
            }
        }
        break;
    }
    case eSPAWNDISPLAY:
    case eCHANGECLASS:
        if (m_pEntityElement->GetGroup())
        {
            m_pEntityElement->GetGroup()->RemoveElement(m_pEntityElement);
        }
        sprintf(acBuffer, "ifs_pc_SpawnSelect.Info");
        static_cast<RedGroupElement*>(RedInterfaceElement::Find(acBuffer))->AddElement(m_pEntityElement);
        m_pEntityElement->Enable(true);
        m_pEntityElement->Visible(true);
        mTextSpawnTimer.Enable(false);
        mTextSpawnTimer.Visible(false);
        sprintf(acBuffer, "ifs_pc_SpawnSelect");
        if (strcmp(acBuffer, pcCurrent) && strcmp(pcCurrent, "ifs_pausemenu") &&
            !m_pGuiManager->IsScreenInStack(acBuffer))
        {
            if (*pcCurrent)
            {
                m_pGuiManager->PopScreen(0);
            }
            m_pGuiManager->PushScreen(acBuffer, 0, 0);
            SelectTeam(mTeamNumber);
        }
        SendEvent(HUD::GameEvents::gPlayerEvents[mPlayerIdx].mapMode, 2);
        mTextReady.Enable(false);
        mTextReady.Visible(false);
        mGroupTimer.Enable(false);
        mGroupTimer.Visible(false);

        if (NetGame::IsEnabled() && !netAutoAssignTeams)
        {
            m_pBackButton1->Enable(true);
            m_pBackButton1->Enable(true);
        }
        else
        {
            m_pBackButton1->Enable(false);
            m_pBackButton1->Enable(false);
        }
        break;
    case eFINISHED:
        m_pEntityElement->Enable(false);
        m_pEntityElement->Visible(false);
        sprintf(acScreen, "ifs_readyselect%i", 1);
        if (strcmp(acScreen, pcCurrent) && !m_pGuiManager->IsScreenInStack(acScreen))
        {
            if (*pcCurrent)
            {
                m_pGuiManager->PopScreen(0);
            }
            m_pGuiManager->PushScreen(acScreen, 0, 0);
        }
        SendEvent(HUD::GameEvents::gPlayerEvents[mPlayerIdx].mapMode, 0);
        mGroupTimer.Enable(true);
        mGroupTimer.Visible(true);
        mTextReady.Enable(true);
        mTextReady.Visible(true);
        mTextSpawnTimer.Enable(true);
        mTextSpawnTimer.Visible(true);
        BootDisplay::Instance(0)->SetSpawnMode(false);
        HeroMessageDisplay::Instance(0)->SetSpawnMode(false);
        break;
    }
}

// FUNCTION: SWBF2 0x0042bf40
void SpawnDisplay::Initialize()
{
    char acBuffer[128];

    LuaHelper::CallProc("ifs_pc_SpawnSelect_fnInit");
    FLRenderer::AddInterfaceElement(&mGroupTimer, 0.5f, 0.5f, true);
    FLRenderer::AddInterfaceElement(&mGroupMainCC, 0.5f, 0.5f, true);
    FLRenderer::AddInterfaceElement(&mGroupMainTC, 0.5f, 0.0f, true);
    FLRenderer::AddInterfaceElement(&mGroupMainNoSafeCC, 0.5f, 0.5f, false);
    FLRenderer::AddInterfaceElement(&mGroupMainTL, 0.0f, 0.0f, false);
    FLRenderer::AddInterfaceElement(&mGroupMainCB, 0.0f, 1.0f, true);
    mGroupMainTL.SetZOrder(10);
    mGroupMainCC.SetZOrder(10);
    mGroupMainTL.SetUsePerspectiveHack(true);
    mGroupMainCC.SetUsePerspectiveHack(true);

    mTextSpawnTimer.Enable(false);
    mTextSpawnTimer.Visible(false);
    mTextSpawnTimer.SetColor(RedColor::YELLOW);
    mTextSpawnTimer.SetPosition(0.0f, 0.0f);
    mTextSpawnTimer.SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_CENTER);
    mTextSpawnTimer.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
    mGroupTimer.AddElement(&mTextSpawnTimer);

    sprintf(acBuffer, "ifs_pc_SpawnSelect.Info");
    if (RedInterfaceElement* pInfo = RedInterfaceElement::Find(acBuffer))
    {
        pInfo->SetZOrder(10);
    }
    sprintf(acBuffer, "ifs_pc_SpawnSelect.Info.PopupWindow.Text");
    if (RedInterfaceElement* pElement = RedInterfaceElement::Find(acBuffer))
    {
        m_pPopUpText = AsText(pElement);
    }
    sprintf(acBuffer, "ifs_pc_SpawnSelect.Info.PopupWindow.titleBarElement");
    if (RedInterfaceElement* pElement = RedInterfaceElement::Find(acBuffer))
    {
        m_pPopUpTitle = AsText(pElement);
    }
    sprintf(acBuffer, "ifs_pc_SpawnSelect.Info.PopupWindow");
    m_pPopUpBorder = static_cast<RedGroupElement*>(RedInterfaceElement::Find(acBuffer));

    Character* pPlayer = Character::FindPlayer(0);
    Team* pTeam = pPlayer ? pPlayer->mTeamPtr : Team::sTeams[1];
    if (NetGame::IsEnabled())
    {
        pTeam = Team::sTeams[NetGame::GetStartTeam(mPlayerIdx)];
    }

    for (int i = 0; i < 2; ++i)
    {
        Team* pSide = Team::sTeams[i + 1];
        sprintf(acBuffer, "ifs_pc_SpawnSelect.Info.SideIcon%i", i);
        m_pSideBG[i] = static_cast<RedGroupElement*>(RedInterfaceElement::Find(acBuffer));
        sprintf(acBuffer, "ifs_pc_SpawnSelect.Info.SideIcon%i.skin", i);
        m_pSideIcon[i] = static_cast<RedBitmapElement*>(RedInterfaceElement::Find(acBuffer));
        m_pSideIcon[i]->CreateHotSpot(38.4f);
        sprintf(acBuffer, "ifs_pc_SpawnSelect.Info.SideModel%i", i);
        m_pSideModel[i] = static_cast<Red3DModelElement*>(RedInterfaceElement::Find(acBuffer));
        unsigned int uModel = 0;
        switch (pSide->GetSide())
        {
        case Team::side_Alliance:
        case Team::side_HEROES:
            uModel = 0x1BE6F06A;
            break;
        case Team::side_Empire:
        case Team::side_VILLAINS:
            uModel = 0xF5198C5A;
            break;
        case Team::side_Republic:
            uModel = 0x86B9BD17;
            break;
        case Team::side_CIS:
            uModel = 0x073E0A94;
            break;
        }
        m_pSideModel[i]->SetModel(uModel);
        m_pSideModel[i]->SetColor(pTeam->mColor[i + 1]);
        m_pSideModel[i]->Enable(true);
        m_pSideModel[i]->Visible(true);
        m_pSideModel[i]->BringToFront();
        m_pSideModel[i]->SetModelScale(RedRenderer::GetScreenAspectRatio() / 0.75f);
    }

    sprintf(acBuffer, "ifs_pc_SpawnSelect.Info.UnitCount");
    m_pUnitCountText = static_cast<RedTextElement*>(RedInterfaceElement::Find(acBuffer));
    sprintf(acBuffer, "ifs_pc_SpawnSelect.Info.Ok.label");
    m_AcceptBox = static_cast<RedBitmapElement*>(RedInterfaceElement::Find(acBuffer));
    sprintf(acBuffer, "ifs_readyselect1.Helptext_Back.label");
    m_pBackButton = RedInterfaceElement::Find(acBuffer);
    sprintf(acBuffer, "ifs_pc_spawnselect.Helptext_Back.label");
    m_pBackButton1 = RedInterfaceElement::Find(acBuffer);
    sprintf(acBuffer, "ifs_pc_Spectator.join_game.label");
    m_pButton_JoinGame = RedInterfaceElement::Find(acBuffer);
    sprintf(acBuffer, "ifs_pc_Spectator.change_view.label");
    m_pButton_ChangeView = RedInterfaceElement::Find(acBuffer);

    for (int i = 0; i < 10; ++i)
    {
        sprintf(acBuffer, "ifs_pc_SpawnSelect.Info.SlotWindow%i.titleBarElement", i);
        if (RedInterfaceElement* pElement = RedInterfaceElement::Find(acBuffer))
        {
            m_pUnitNameText[i] = AsText(pElement);
        }
        sprintf(acBuffer, "ifs_pc_SpawnSelect.Info.SlotWindow%i.InfoText", i);
        if (RedInterfaceElement* pElement = RedInterfaceElement::Find(acBuffer))
        {
            m_pInfoBoxText[i] = AsText(pElement);
        }
        sprintf(acBuffer, "ifs_pc_SpawnSelect.Info.SlotWindow%i", i);
        m_pSlotBorder[i] = static_cast<RedGroupElement*>(RedInterfaceElement::Find(acBuffer));
    }

    mTextReady.SetFont(RedFont::GAMEFONT_MEDIUM);
    if (const unsigned short* pText = RedLocalizeDB::Find(0xE67D2442, false))
    {
        mTextReady.SetText(pText, true);
    }
    mTextSpawnTimer.SetFont(RedFont::GAMEFONT_LARGE);

    m_pEntityElement = 0;
    SetEntityElement(pTeam->GetClassPtr(0));

    unsigned int uiWidth, uiHeight;
    m_pInfoBoxText[0]->GetTextBox(uiWidth, uiHeight);

    float fScreenScale = (float)RedInterfaceScreen::GetScreenWidth() / 800.0f;
    float fDepth = 0.006f / fScreenScale;
    m_pEntityElement->SetDepth(fDepth);
    float fLeft, fTop, fRight, fBottom;
    RedInterfaceScreen::s_screenFull.GetSafeArea(fLeft, fTop, fRight, fBottom);
    m_pEntityElement->SetPosition((fRight - fLeft) * 0.5f, (fBottom - fTop) * 0.85f);
    m_pEntityElement->Enable(true);
    m_pEntityElement->Visible(true);
    m_pEntityElement->SetZOrder(0);
    float fScale = 1.0f - fScreenScale * 0.05f;
    m_pEntityElement->SetScale(fScale);
    sprintf(acBuffer, "ifs_pc_SpawnSelect.Info");
    static_cast<RedGroupElement*>(RedInterfaceElement::Find(acBuffer))->AddElement(m_pEntityElement);

    m_DroidekaElement.SetDepth(fDepth * 1.2f);
    m_DroidekaElement.SetPosition((fRight - fLeft) * 0.5f, (fBottom - fTop) * 0.85f);
    m_DroidekaElement.Enable(false);
    m_DroidekaElement.Visible(false);
    m_DroidekaElement.SetZOrder(0);
    m_DroidekaElement.SetScale(fScale);

    mBackdropModel.SetModel(0x6472A227);
    mBackdropModel.SetDepth(0.009f);
    mBackdropModel.SetModelScale((float)RedInterfaceScreen::GetScreenWidth() / 800.0f * 1.25f);
    mBackdropModel.Enable(false);
    mBackdropModel.Visible(false);
    mBackdropModel.SendToBack();
    mGroupMainCC.AddElement(&mBackdropModel);

    SelectTeam(pTeam->mTeamNumber);
}

// FUNCTION: SWBF2 0x0042c7f0
void SpawnDisplay::SetSlotInfo(EntityClass* pClass, int iSlot)
{
    if (pClass->mLabel)
    {
        m_pUnitNameText[iSlot]->SetText(pClass->mLabel, true);
    }
    else
    {
        char acName[64];
        sprintf(acName, "0x%08X", pClass->mId);
        m_pUnitNameText[iSlot]->SetText(acName, true);
    }
    if (m_pUnitNameText[iSlot]->IsFlashyText())
    {
        FlashyTextElement* pFlashy = static_cast<FlashyTextElement*>(m_pUnitNameText[iSlot]);
        pFlashy->mTimeDelay = (float)iSlot * 0.05f;
        pFlashy->StartFlashiness();
    }
}

// FUNCTION: SWBF2 0x0042c880
void SpawnDisplay::SetupSlots()
{
    Team* pTeam = Team::sTeams[mTeamNumber];
    int iCount = pTeam->GetClassCount() - pTeam->GetSpecialClassCount();
    sSlotsIncludeHeroes = CanShowHeroes();
    if (sSlotsIncludeHeroes)
    {
        iCount = pTeam->GetClassCount();
    }
    if (iCount > 10)
    {
        iCount = 10;
    }

    for (int i = 0; i < iCount; ++i)
    {
        GameObjectClass* pClass = pTeam->GetClassPtr(i);
        if (pClass->IsRtti(GameObjectClass::rttiHashGameObjectClass))
        {
            UpdateObjectText(pClass, i);
            SetSlotInfo(pClass, i);
            m_pInfoBoxText[i]->Enable(true);
            m_pInfoBoxText[i]->Visible(true);
            m_pUnitNameText[i]->Enable(true);
            m_pUnitNameText[i]->Visible(true);
            m_pSlotBorder[i]->Enable(true);
            m_pSlotBorder[i]->Visible(true);
            m_pInfoBoxText[i]->m_color.a = 0x80;
            m_pUnitNameText[i]->m_color.a = 0x80;
        }
    }
    for (int i = iCount; i < 10; ++i)
    {
        m_pInfoBoxText[i]->Enable(false);
        m_pInfoBoxText[i]->Visible(false);
        m_pUnitNameText[i]->Enable(false);
        m_pUnitNameText[i]->Visible(false);
        m_pSlotBorder[i]->Enable(false);
        m_pSlotBorder[i]->Visible(false);
    }

    if (mClassIndex >= iCount)
    {
        mClassIndex = 0;
    }
    m_pInfoBoxText[mClassIndex]->m_color.a = 0xff;
    m_pUnitNameText[mClassIndex]->m_color.a = 0xff;
    SetEntityElement(pTeam->GetClassPtr(mClassIndex));
}

// FUNCTION: SWBF2 0x0042ca40
void SpawnDisplay::SetEntityElement(EntityClass* pClass)
{
    if (m_pEntityElement)
    {
        m_pEntityElement->SetSelected(false);
        m_pEntityElement->Enable(false);
        m_pEntityElement->Visible(false);
    }
    m_pEntityElement = 0;

    if (pClass->IsRtti(EntitySoldierClass::rttiHashEntitySoldierClass))
    {
        m_SoldierElement.SetAim(PblVector3(0.0f, 10.0f, 4000.0f));
        m_SoldierElement.SetClass(static_cast<EntitySoldierClass*>(pClass));
        m_pEntityElement = &m_SoldierElement;
    }
    else if (pClass->IsRtti(EntityDroidekaClass::rttiHashEntityDroidekaClass))
    {
        m_DroidekaElement.SetClass(static_cast<EntityDroidekaClass*>(pClass));
        m_pEntityElement = &m_DroidekaElement;
    }

    if (m_pEntityElement)
    {
        if (pClass->IsRtti(GameObjectClass::rttiHashGameObjectClass))
        {
            m_pEntityElement->SetModel(
                const_cast<GameModel*>(static_cast<GameObjectClass*>(pClass)->GetModel()));
        }
        m_pEntityElement->Setup();
        m_pEntityElement->SetZOrder(0xbe);
        m_pEntityElement->SetSelected(true);
    }
    m_pEntityElement->Enable(true);
    m_pEntityElement->Visible(true);
}

static char sChangeClassParam;

// FUNCTION: SWBF2 0x0042b650
void SpawnDisplay::SendSpawnCancelMessage()
{
    if (NetGame::IsEnabled())
    {
        NetGame::CancelSpawn(mPlayerIdx);
    }
    else
    {
        mOwner->SetSpawnReady(false);
    }
}

// FUNCTION: SWBF2 0x0042b690
void SpawnDisplay::SendSpawnReadyMessage()
{
    if (NetGame::IsEnabled())
    {
        mLastSpawnTime = 15.0f;
        NetGame::SelectSpawn(mPlayerIdx, mTeamNumber, mClassIndex, GetCommandPost());
        mOwner->SelectSpawn(mTeamNumber, mClassIndex, GetCommandPost());
    }
    else
    {
        mOwner->SelectSpawn(mTeamNumber, mClassIndex, GetCommandPost());
        mOwner->SetSpawnReady(true);
    }
}

// FUNCTION: SWBF2 0x0042b0c0
void SpawnDisplay::UpdateInput()
{
    GUIInputs* pInputs = FLInputManager::s_instance.GetPlayerGuiInputs(mPlayerIdx);
    RawControllerInputs* pJoystick = FLInputManager::s_instance.GetPlayerJoystick(mPlayerIdx);
    const float fMouseX = (float)pInputs->mMouseCoords.mPosX;
    const float fMouseY = (float)pInputs->mMouseCoords.mPosY;
    const float fPulse = PblMath::Sin(mVisibleTimer * 8.0f) * 100.0f + 155.0f;

    for (int i = 0; i < 10; ++i)
    {
        Team* pTeam = Team::sTeams[mTeamNumber];
        int iCount = pTeam->GetClassCount();
        if (!CanShowHeroes())
        {
            iCount -= pTeam->GetSpecialClassCount();
        }
        if (i >= iCount)
        {
            break;
        }

        RedGroupElement* pBorder = m_pSlotBorder[i];
        if (pBorder->IsEnabled() && pBorder->TestHotSpot(fMouseX, fMouseY))
        {
            if (i != mClassIndex)
            {
                m_pInfoBoxText[i]->m_color.a = (unsigned char)(int)fPulse;
                m_pUnitNameText[i]->m_color.a = (unsigned char)(int)fPulse;
            }
        }
        else if (i != mClassIndex)
        {
            m_pInfoBoxText[i]->m_color.a = 0x80;
            m_pUnitNameText[i]->m_color.a = 0x80;
        }
    }

    for (int i = 0; i < 2; ++i)
    {
        RedBitmapElement* pIcon = m_pSideIcon[i];
        if (pIcon->IsEnabled() && pIcon->TestHotSpot(fMouseX, fMouseY) &&
            !(NetGame::IsEnabled() && netAutoAssignTeams))
        {
            pIcon->m_color.a = (unsigned char)(int)fPulse;
        }
        else
        {
            pIcon->m_color.a = 0x80;
        }
    }

    int iClass;
    if (pInputs->IsAcceptPressed())
    {
        const int iSide = mTeamNumber % 2;
        if (m_pSideIcon[iSide]->TestHotSpot(fMouseX, fMouseY) && GameLoop::GetCanSwitchSides() &&
            mMode != eCHANGECLASS)
        {
            SelectTeam(iSide + 1);
            return;
        }

        for (iClass = 0; iClass < 10; ++iClass)
        {
            Team* pTeam = Team::sTeams[mTeamNumber];
            int iCount = pTeam->GetClassCount();
            if (!CanShowHeroes())
            {
                iCount -= pTeam->GetSpecialClassCount();
            }
            if (iClass >= iCount)
            {
                break;
            }

            RedGroupElement* pBorder = m_pSlotBorder[iClass];
            if (pBorder->IsEnabled() && pBorder->TestHotSpot(fMouseX, fMouseY))
            {
                goto select_class;
            }
        }

        if (m_AcceptBox->TestHotSpot(fMouseX, fMouseY))
        {
            Character* pCharacter = Character::FindPlayer(NetGame::GetLocalIndex(mPlayerIdx));
            if ((mMode == eCHANGECLASS || mMode == eSPAWNDISPLAY) &&
                Character::GetPlayerStats(pCharacter).IsClassSelectable(mClassIndex, 0))
            {
                mMode = eFINISHED;
                pCharacter->ChangeClass(mClassIndex);
                PlayerController::sEnabled = -1;
                EventManager::CharacterEvent::mChangeClass.Trigger(*pCharacter, sChangeClassParam);
                SendSpawnReadyMessage();
            }
            else
            {
                mBack.Play(0, 0, false, true);
                Show(pCharacter);
                ScreenTransition::sInstance->SetTransition(mPlayerIdx, ScreenTransition::BLUR);
                FLInputManager::s_instance.GetPlayerJoystick(mPlayerIdx)->ClearInputsToOff();
            }
            return;
        }

        if (CommandPost* pPost = FindPost(fMouseX, fMouseY))
        {
            SelectPost(pPost);
        }
        return;
    }

    if (pInputs->IsBackPressed())
    {
        if (!netAutoAssignTeams)
        {
            mMode = eSIDESELECT;
        }
        return;
    }

    iClass = -1;
    for (unsigned char ucKey = 2; ucKey <= 11; ++ucKey)
    {
        if (pJoystick->GetKeyboard()->IsKeyDown(ucKey))
        {
            iClass = ucKey - 2;
            break;
        }
    }
    {
        Team* pTeam = Team::sTeams[mTeamNumber];
        int iCount = pTeam->GetClassCount();
        if (!CanShowHeroes())
        {
            iCount -= pTeam->GetSpecialClassCount();
        }
        if (iClass < 0 || iClass >= iCount || iClass == mClassIndex)
        {
            return;
        }
    }

select_class:
    m_pInfoBoxText[mClassIndex]->m_color.a = 0x80;
    m_pUnitNameText[mClassIndex]->m_color.a = 0x80;
    SelectClass(iClass);
    m_pInfoBoxText[iClass]->m_color.a = 0xff;
    m_pUnitNameText[iClass]->m_color.a = 0xff;
}

// FUNCTION: SWBF2 0x00429500
CommandPost* SpawnDisplay::FindPost(float fX, float fY)
{
    HUD::ElementMap* pMap = HUD::ElementMap::FindByPlayerIndex(mPlayerIdx);
    if (!pMap)
    {
        return 0;
    }
    CommandPost* pPost = pMap->GetPost(fX, fY, mCommandPost, mTeamNumber);
    if (!pPost || !(GameObject*)pPost->mObject)
    {
        return 0;
    }
    return pPost;
}
