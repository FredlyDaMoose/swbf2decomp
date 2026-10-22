#include <stdio.h>
#include <string.h>

#include "HeroMessageDisplay.h"

#include "CameraManager.h"
#include "Character.h"
#include "Controllable.h"
#include "EntitySoldier.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "GameSound.h"
#include "HUDEvent.h"
#include "HUDGameEvents.h"
#include "MessageDisplay.h"
#include "NetGame.h"
#include "Team.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

class Controllable;
class PblVector3;

extern int netHeroNextPlayerID[3];
extern int netHeroPlayerID[3];
extern int netHeroAIID[3];
extern float netSecondsPerTurn;
extern bool netHeroesEnabled;
extern unsigned int netHeroSelectedTurn[3];
extern float spHeroSelectedTime[3];

namespace PblUnicode { unsigned int ustrncnv(unsigned short* pDest, const unsigned char* pSrc, unsigned int uiCount); }

// GLOBAL: SWBF2 0x01e56c28
HeroMessageDisplay* HeroMessageDisplay::sInstance[1];
// GLOBAL: SWBF2 0x01e56bf4
GameSound* HeroMessageDisplay::mHeroesUnlocked;
// GLOBAL: SWBF2 0x01e56c1c
GameSound* HeroMessageDisplay::mHeroUnlocked[3];
// GLOBAL: SWBF2 0x01e56be8
GameSound* HeroMessageDisplay::mHeroSelectable[3];
// GLOBAL: SWBF2 0x01e56c04
GameSound* HeroMessageDisplay::mHeroSpawned[3];
// GLOBAL: SWBF2 0x01e56c10
GameSound* HeroMessageDisplay::mHeroDefeated[3];
// GLOBAL: SWBF2 0x01e56bf8
GameSound* HeroMessageDisplay::mYouKilledHero[3];

// GLOBAL: SWBF2 0x007e68f3
bool sFirstUpdate = true;

namespace
{

const PblVector3* GetUnitPosition(Controllable* pUnit)
{
    return (const PblVector3*)&pUnit->GetMatrix().m[3][0];
}

void SendEvent(HUD::EventClass* pClass, bool bValue)
{
    HUD::Event event;
    event.mClass = pClass;
    event.mData.boolValue = bValue;
    event.Send();
}

void SendEvent(HUD::EventClass* pClass, float fValue)
{
    HUD::Event event;
    event.mClass = pClass;
    event.mData.floatValue = fValue;
    event.Send();
}

void SendEvent(HUD::EventClass* pClass, unsigned short* pString)
{
    HUD::Event event;
    event.mClass = pClass;
    event.mData.string = pString;
    event.Send();
}

Character* GetHeroCharacter(int iTeam)
{
    if (netHeroPlayerID[iTeam] == -1)
    {
        return Character::GetPtr(netHeroAIID[iTeam]);
    }
    return Character::FindPlayer(netHeroPlayerID[iTeam]);
}

}

// FUNCTION: SWBF2 0x005428b0
void HeroMessageDisplay::PlayHeroSoundUnlock(int iTeam)
{
    if (iTeam == -1)
    {
        mHeroesUnlocked->Play(0, 0, iTeam != 0, true);
    }
    else if ((unsigned int)iTeam <= 2)
    {
        mHeroUnlocked[iTeam]->Play(0, 0, iTeam != 0, true);
    }
}

// FUNCTION: SWBF2 0x00542910
void HeroMessageDisplay::PlayHeroSoundSpawn(int iTeam)
{
    if ((unsigned int)iTeam > 2)
    {
        return;
    }

    Character* pHero = GetHeroCharacter(iTeam);
    if (!pHero)
    {
        return;
    }

    if (pHero->mUnit)
    {
        mHeroSpawned[iTeam]->Play(GetUnitPosition(pHero->mUnit), 0, false, true);
    }

    const unsigned short* pFormat = RedLocalizeDB::Find(0x0F669681, false);
    if (!Team::sTeams[iTeam])
    {
        return;
    }

    unsigned short heroName[64];
    heroName[0] = 0;
    const GameObjectClass* pHeroClass = Team::sTeams[iTeam]->GetHeroClass();
    if (pHeroClass)
    {
        PblUnicode::ustrlcpy(heroName, pHeroClass->mLabel, 63);
    }

    if (netHeroPlayerID[iTeam] < 0 && netHeroAIID[iTeam] < 0)
    {
        return;
    }

    unsigned short playerName[64];
    if (NetGame::IsEnabled())
    {
        PblUnicode::ustrncnv(playerName, (const unsigned char*)NetGame::GetPlayerName(netHeroPlayerID[iTeam]), 64);
    }
    else
    {
        PblUnicode::ustrncpy(playerName, pHero->GetRegularName(), 64);
    }

    unsigned short message[256];
    PblUnicode::usnprintf(message, 256, pFormat, playerName, heroName);

    int iNumViewports = CameraManager::sInstance->m_iNumCam;
    for (int i = 0; i < iNumViewports; ++i)
    {
        MessageDisplay* pDisplay = MessageDisplay::Instance(i, MessageDisplay::DISPLAY_SYSTEM_MSG);
        if (!pDisplay)
        {
            continue;
        }
        Character* pLocal = NetGame::GetLocalPlayer(i);
        if (pLocal && pLocal->mTeamPtr)
        {
            pDisplay->AddMessage(message, pLocal->mTeamPtr->GetColor(iTeam));
        }
    }
}

// FUNCTION: SWBF2 0x00542af0
void HeroMessageDisplay::PlayHeroSoundDefeat(int iTeam)
{
    if ((unsigned int)iTeam > 2)
    {
        return;
    }

    Character* pHero = GetHeroCharacter(iTeam);
    if (pHero && pHero->mUnit)
    {
        mHeroDefeated[iTeam]->Play(GetUnitPosition(pHero->mUnit), 0, false, true);
    }
}

// FUNCTION: SWBF2 0x00542b80
void HeroMessageDisplay::Create()
{
    mHeroesUnlocked = new GameSound;

    for (unsigned int i = 0; i < 3; ++i)
    {
        mHeroUnlocked[i] = new GameSound;
        mHeroSelectable[i] = new GameSound;
        mHeroSpawned[i] = new GameSound;
        mHeroDefeated[i] = new GameSound;
        mYouKilledHero[i] = new GameSound;

        char name[32];
        _snprintf(name, sizeof(name), "HeroUnlocked%d", i);
        _snprintf(name, sizeof(name), "HeroSelectable%d", i);
        _snprintf(name, sizeof(name), "HeroSpawned%d", i);
        _snprintf(name, sizeof(name), "HeroDefeated%d", i);
        _snprintf(name, sizeof(name), "YouKilledHero%d", i);
    }

    sInstance[0] = new HeroMessageDisplay;
}

// FUNCTION: SWBF2 0x00542ce0
void HeroMessageDisplay::Destroy()
{
    delete mHeroesUnlocked;
    mHeroesUnlocked = 0;

    for (unsigned int i = 0; i < 3; ++i)
    {
        delete mHeroUnlocked[i];
        mHeroUnlocked[i] = 0;
        delete mHeroSelectable[i];
        mHeroSelectable[i] = 0;
        delete mHeroSpawned[i];
        mHeroSpawned[i] = 0;
        delete mHeroDefeated[i];
        mHeroDefeated[i] = 0;
        delete mYouKilledHero[i];
        mYouKilledHero[i] = 0;
    }

    if (sInstance[0])
    {
        delete sInstance[0];
        sInstance[0] = 0;
    }
}

// FUNCTION: SWBF2 0x00542dc0
HeroMessageDisplay::HeroMessageDisplay()
    : mCanRender(true), mLobbyMode(false), mIsInHeroSelecting(false)
{
}

// FUNCTION: SWBF2 0x00542e10
HeroMessageDisplay::~HeroMessageDisplay()
{
}

// FUNCTION: SWBF2 0x00542e20
void HeroMessageDisplay::Init()
{
    sFirstUpdate = true;
    Hide();
}

// FUNCTION: SWBF2 0x00542e30
void HeroMessageDisplay::Bind(unsigned int uJoystick)
{
    mJoystickIndex = uJoystick;
    mViewport = CameraManager::sInstance->m_hView[uJoystick];
    if (NetGame::IsEnabled())
    {
        ActivateThread(-101, PER_LOCAL_TURN);
    }
    else
    {
        ActivateThread(-1, PER_LOCAL_TURN);
    }
}

// FUNCTION: SWBF2 0x00542e80
void HeroMessageDisplay::Show(Character* pOwner)
{
    if (!mOwner)
    {
        mOwner = pOwner;
        mVisible = true;
    }
}

// FUNCTION: SWBF2 0x00542ea0
void HeroMessageDisplay::Hide(Character* pOwner)
{
    if (pOwner == mOwner)
    {
        Hide();
    }
}

// FUNCTION: SWBF2 0x00542ec0
void HeroMessageDisplay::Hide()
{
    mOwner = 0;
    mVisible = false;
    if ((unsigned int)mViewport < CameraManager::sInstance->m_iNumCam)
    {
        SendEvent(HUD::GameEvents::gPlayerEvents[mViewport].heroSelectDisable, true);
    }
}

// FUNCTION: SWBF2 0x00542f00
void HeroMessageDisplay::SetSpawnMode(bool bSpawnMode)
{
    if (mSpawnMode != bSpawnMode)
    {
        mSpawnMode = bSpawnMode;
    }
}

// FUNCTION: SWBF2 0x00542f20
void HeroMessageDisplay::UpdateHeroSelect(float fDeltaTime, int iTeam, float& fTotalTime,
                                          float& fTimeLeft, Character* pCharacter)
{
    fTotalTime = 30.0f;
    if (NetGame::IsEnabled())
    {
        fTimeLeft = fTotalTime - (float)(NetGame::GetTurnNumber() - netHeroSelectedTurn[iTeam]) * netSecondsPerTurn;
    }
    else
    {
        fTimeLeft = fTotalTime - (NetGame::GetLocalTime() - spHeroSelectedTime[iTeam]);
    }

    if (!(fTimeLeft > 0.0f))
    {
        return;
    }

    const unsigned short* pFormat = RedLocalizeDB::Find(0xADD5467C, false);

    int iTeamNumber = 1;
    if (netHeroNextPlayerID[iTeam] >= 0)
    {
        iTeamNumber = NetGame::GetStartTeam(netHeroNextPlayerID[iTeam]);
    }
    if (!NetGame::IsEnabled())
    {
        iTeamNumber = pCharacter->mTeamNumber;
    }

    Team* pTeam = Team::sTeams[iTeamNumber];
    if (!pTeam)
    {
        return;
    }

    unsigned short heroName[64];
    heroName[0] = 0;
    int iClassCount = pTeam->GetClassCount();
    for (int i = 0; i < iClassCount; ++i)
    {
        GameObjectClass* pClass = pTeam->GetClassPtr(i);
        if (pClass && pClass->mHeroFlag)
        {
            if (pClass->mLabel)
            {
                memcpy(heroName, pClass->mLabel, sizeof(heroName));
            }
            break;
        }
    }

    unsigned short message[256];
    PblUnicode::usnprintf(message, 256, pFormat, heroName);

    if ((unsigned int)mViewport < CameraManager::sInstance->m_iNumCam)
    {
        HUD::GameEvents::PlayerEvents& events = HUD::GameEvents::gPlayerEvents[mViewport];
        SendEvent(events.heroSelectTimer, fTimeLeft);
        SendEvent(events.heroSelectTimerFraction, fTimeLeft / fTotalTime);
        SendEvent(events.heroSelectMessage, message);
    }
}

// FUNCTION: SWBF2 0x00543100
bool HeroMessageDisplay::Update(float fDeltaTime)
{
    if (sFirstUpdate)
    {
        sFirstUpdate = false;
        for (int iTeam = 0; iTeam < 3; ++iTeam)
        {
            Team* pTeam = Team::sTeams[iTeam];
            if (!pTeam)
            {
                continue;
            }
            for (int i = 0; i < pTeam->GetClassCount(); ++i)
            {
                GameObjectClass* pClass = pTeam->GetClassPtr(i);
                if (pClass && pClass->mHeroFlag &&
                    pClass->IsRtti(EntitySoldierClass::rttiHashEntitySoldierClass))
                {
                    EntitySoldierClass* pSoldier = static_cast<EntitySoldierClass*>(pClass);
                    mHeroSelectable[iTeam]->SetID(pSoldier->GetSndHeroSelectable(), false, false);
                    mHeroSpawned[iTeam]->SetID(pSoldier->GetSndHeroSpawned(), false, false);
                    mHeroDefeated[iTeam]->SetID(pSoldier->GetSndHeroDefeated(), false, false);
                    mYouKilledHero[iTeam]->SetID(pSoldier->GetSndForHeroKiller(), false, false);
                    break;
                }
            }
        }
    }

    mIsInHeroSelecting = false;
    SendEvent(HUD::GameEvents::gPlayerEvents[mViewport].heroSelectDisable, true);

    Character* pCharacter = Character::FindPlayer(NetGame::GetLocalIndex(mJoystickIndex));
    if (!pCharacter)
    {
        Hide();
    }
    else if (pCharacter->mUnit)
    {
        if (!mVisible)
        {
            Show(pCharacter);
        }
    }
    else if (mVisible)
    {
        Hide(pCharacter);
    }

    if (mCanRender && fDeltaTime != 0.0f && !GameLoop::IsGameOver() && mVisible && netHeroesEnabled)
    {
        for (int i = 0; i < 3; ++i)
        {
            float fTotalTime = 5.0f;
            float fTimeLeft = 0.0f;
            if (netHeroNextPlayerID[i] >= 0 && netHeroSelectedTurn[i] != 0 &&
                NetGame::GetLocalIndex(mJoystickIndex) == netHeroNextPlayerID[i] &&
                netHeroPlayerID[i] == -1)
            {
                mIsInHeroSelecting = true;
                UpdateHeroSelect(fDeltaTime, i, fTotalTime, fTimeLeft, pCharacter);
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00543330
void HeroMessageDisplay::EnableViewport(int iViewport, bool bEnable)
{
    HeroMessageDisplay* pDisplay = sInstance[0];
    if (pDisplay && pDisplay->mViewport == iViewport)
    {
        pDisplay->mCanRender = bEnable;
        if (!bEnable)
        {
            pDisplay->Hide();
        }
    }
}

// FUNCTION: SWBF2 0x005428f0
void HeroMessageDisplay::PlayHeroSoundSelectable(int iTeam)
{
    if ((unsigned int)iTeam <= 2)
    {
        HeroMessageDisplay::mHeroSelectable[iTeam]->Play(0, 0, iTeam, true);
    }
}
