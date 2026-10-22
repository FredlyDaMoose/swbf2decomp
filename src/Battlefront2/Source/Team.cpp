#include <stdio.h>
#include <string.h>

#include "Team.h"

#include "Character.h"
#include "Controllable.h"
#include "Damageable.h"
#include "Entity.h"
#include "EntityDroideka.h"
#include "EventManager_Team.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

extern int netCurMaxPlayers;

extern int netHeroAIID[3];

namespace EventManager
{
template <>
void Event<Team, int>::Callback::Trigger(Team& item, int& param);
template <>
void Event<Team, float>::Callback::Trigger(Team& item, float& param);
}

// GLOBAL: SWBF2 0x01eafcd0
Team* Team::sHostTeams[8];

// GLOBAL: SWBF2 0x01eafca4
Team* Team::sClientTeams[8];

// GLOBAL: SWBF2 0x007e9aa0
Team** Team::sTeams = Team::sHostTeams;

// GLOBAL: SWBF2 0x007e9aa4
float gGameSlice = 1.0f;

// GLOBAL: SWBF2 0x007e9aa8
int Team::sTeamMaxReinforcementsOverride[2] = { -1, -1 };

// GLOBAL: SWBF2 0x01eafca0
unsigned int Team::sWinningTeams;

// GLOBAL: SWBF2 0x007ebc98
RedColor gFriendColor = { 0x01, 0x56, 0xd5, 0xff };
// GLOBAL: SWBF2 0x01f7ce80
// SYNTHETIC: SWBF2 0x00406890
// `dynamic initializer for 'gEnemyColor''
RedColor gEnemyColor = RedColor::RED;
// GLOBAL: SWBF2 0x01f6deb4
// SYNTHETIC: SWBF2 0x004068c0
// `dynamic initializer for 'gLocalColor''
RedColor gLocalColor = RedColor::WHITE;
// GLOBAL: SWBF2 0x01f75f0c
// SYNTHETIC: SWBF2 0x004068f0
// `dynamic initializer for 'gNeutralColor''
RedColor gNeutralColor = RedColor::YELLOW;

// FUNCTION: SWBF2 0x00652dd0
void Team::CreateAll()
{
    sTeams[0] = new Team(0, 0, 0);
    sTeams[1] = new Team(1, 300, 12);
    sTeams[2] = new Team(2, 300, 12);
    for (int i = 3; i < 8; ++i)
    {
        sTeams[i] = new Team(i, 300, 12);
        sTeams[i]->mMaxReinforcements = 0x7FFFFFFF;
        sTeams[i]->SetCurReinforcements(0x7FFFFFFF);
    }
}

// FUNCTION: SWBF2 0x00652f20
void Team::CreateClient()
{
    if (!netIsOneWorld)
    {
        EnterClient();
        CreateAll();
        LeaveClient();
    }
}

// FUNCTION: SWBF2 0x00652f40
void Team::DestroyAll()
{
    for (int i = 0; i < 8; ++i)
    {
        delete sTeams[i];
        sTeams[i] = 0;
    }
}

// FUNCTION: SWBF2 0x00652f70
void Team::DestroyClient()
{
    if (!netIsOneWorld)
    {
        DestroyAll();
    }
}

// FUNCTION: SWBF2 0x00652f80
void Team::EnterClient()
{
    if (!netIsOneWorld)
    {
        sTeams = sClientTeams;
    }
}

// FUNCTION: SWBF2 0x00652fa0
void Team::LeaveClient()
{
    if (!netIsOneWorld)
    {
        sTeams = sHostTeams;
    }
}

// FUNCTION: SWBF2 0x00652fc0
void Team::CopyFromHost()
{
    if (netIsOneWorld)
    {
        return;
    }

    for (int i = 0; i < 8; ++i)
    {
        Team* pHost = sHostTeams[i];
        const int iClassCount = pHost->mClassCount;
        for (int j = 0; j < iClassCount; ++j)
        {
            GameObjectClass* pClass = pHost->GetClassPtr(j);
            if (pClass->mHeroFlag)
            {
                sTeams[i]->AddSpecialClass(pClass);
            }
            else
            {
                sTeams[i]->AddClass(pClass, pHost->GetClassMinUnits(j), pHost->GetClassMaxUnits(j));
            }
        }

        Team* pTeam = sTeams[i];
        pTeam->SetName(pHost->mName);
        const unsigned int uIcon = sHostTeams[i]->GetIcon();
        pTeam->mIcon = uIcon;
        if (!pTeam->mConquestIcon)
        {
            pTeam->mConquestIcon = uIcon;
        }
        if (!pTeam->mCTFIcon)
        {
            pTeam->mCTFIcon = uIcon;
        }
        pTeam->SetSide(sHostTeams[i]->mSide);
        pTeam->mMaxReinforcements = sHostTeams[i]->mMaxReinforcements;
        pTeam->SetCurReinforcements(sHostTeams[i]->mCurReinforcements);
        for (int j = 0; j < 8; ++j)
        {
            sTeams[i]->mColor[j] = sHostTeams[i]->mColor[j];
            sTeams[i]->mAffiliation[j] = sHostTeams[i]->mAffiliation[j];
        }
        sTeams[i]->SetPoints(sHostTeams[i]->mPoints);
    }
}

// FUNCTION: SWBF2 0x006532f0
void Team::InitAll()
{
    sWinningTeams = 0;
    for (int i = 1; i < 8; ++i)
    {
        sTeams[i]->Init();
    }
}

// FUNCTION: SWBF2 0x00653320
void Team::InitialUpdateAll()
{
    for (int i = 1; i < 8; ++i)
    {
        sTeams[i]->InitialUpdate();
    }

    if (NetGame::IsEnabled())
    {
        for (int i = 0; i < 2; ++i)
        {
            const int iOverride = sTeamMaxReinforcementsOverride[i];
            if (iOverride >= 0)
            {
                sTeams[i + 1]->mMaxReinforcements = iOverride;
                sTeams[i + 1]->SetCurReinforcements(iOverride);
            }
        }
        if (gGameSlice > 0.0f)
        {
            for (int i = 1; i <= 2; ++i)
            {
                const int iCount = (int)((float)sTeams[i]->mCurReinforcements / gGameSlice);
                if (iCount >= 0)
                {
                    sTeams[i]->SetCurReinforcements(iCount);
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x006533e0
Team::Team(int iTeamNumber, int iMemberLimit, int iClassLimit)
{
    mTeamNumber = iTeamNumber;
    mClassLimit = iClassLimit;
    mMemberLimit = iMemberLimit;
    mName = 0;
    mIcon = 0;
    mConquestIcon = 0;
    mCTFIcon = 0;
    mCurReinforcements = 0;
    mMaxReinforcements = 0;
    mBleedRate = 0.0f;
    mMemberCount = 0;
    mMemberActive = 0;
    mClassCount = 0;
    mSpecialClassCount = 0;
    mCarrier = 0;
    mPoints = 0;
    mSide = side_Neutral;
    mAggressiveness = 1.0f;
    mbAIMembersIgnoreVehicles = false;
    mbAllowAISpawn = true;
    mbInitialUpdateDone = false;

    mMemberArray = new Character*[mMemberLimit];
    mClassArray = new GameObjectClass*[mClassLimit];
    mClassMinUnits = new int[mClassLimit];
    mClassMaxUnits = new int[mClassLimit];

    for (int i = 0; i < 8; ++i)
    {
        mAffiliation[i] = 0;
    }
    if (mTeamNumber)
    {
        for (int i = 1; i < 8; ++i)
        {
            mAffiliation[i] = (mTeamNumber == i) ? 1 : -1;
        }
    }
    mSideNameLocal[0] = 0;
    mCaptureTeam = mTeamNumber;
}

// SYNTHETIC: SWBF2 0x006535d0
// Team::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00653600
Team::~Team()
{
    delete[] mClassMaxUnits;
    delete[] mClassMinUnits;
    delete[] mClassArray;
    delete[] mMemberArray;
}

// FUNCTION: SWBF2 0x00653630
RedColor Team::GetColorByAffiliation(int iAffiliation)
{
    if (iAffiliation > 0)
    {
        return gFriendColor;
    }
    if (iAffiliation < 0)
    {
        return gEnemyColor;
    }
    return gNeutralColor;
}

// FUNCTION: SWBF2 0x006536a0
Team::Side Team::TranslateTeamName(const char* pcName)
{
    switch ((unsigned int)PblHash(pcName))
    {
    case 0x13254BC4:
    case 0x1D083CC0:
        return side_Alliance;
    case 0x93E73CA1:
    case 0x2C5F5065:
        return side_Empire;
    case 0x2CF46160:
    case 0xC7836FE1:
        return side_Republic;
    case 0xF387C5D6:
        return side_CIS;
    case 0x269992EB:
    case 0x8C4B1AD8:
        return side_Neutral;
    case 0xF4B48D8C:
    case 0x1B3B5C59:
    case 0xA8C4C105:
        return side_HEROES;
    case 0x74123D64:
    case 0x500090A8:
    case 0xCBE408C1:
        return side_VILLAINS;
    default:
        return side_Locals;
    }
}

// FUNCTION: SWBF2 0x00653770
void Team::Init()
{
    mNameRand.SetSeed(mSide);

    const char* pcSide;
    switch (mSide)
    {
    case side_Alliance:
        pcSide = "common.sides.all";
        break;
    case side_Empire:
        pcSide = "common.sides.imp";
        break;
    case side_Republic:
        pcSide = "common.sides.rep";
        break;
    case side_CIS:
        pcSide = "common.sides.cis";
        break;
    default:
        pcSide = "common.sides.neu";
        break;
    }

    char acKey[64];
    for (mFirstCount = 0; mFirstCount < 64; ++mFirstCount)
    {
        _snprintf(acKey, sizeof(acKey), "%s.firstnames.%02d", pcSide, mFirstCount + 1);
        const unsigned short* pName = RedLocalizeDB::Find(PblHash(acKey), true);
        if (!pName)
        {
            break;
        }
        mFirstNames[mFirstCount] = (unsigned short*)pName;
    }
    for (mLastCount = 0; mLastCount < 64; ++mLastCount)
    {
        _snprintf(acKey, sizeof(acKey), "%s.lastnames.%02d", pcSide, mLastCount + 1);
        const unsigned short* pName = RedLocalizeDB::Find(PblHash(acKey), true);
        if (!pName)
        {
            break;
        }
        mLastNames[mLastCount] = (unsigned short*)pName;
    }
}

// FUNCTION: SWBF2 0x00653890
void Team::InitialUpdate()
{
    if (mTeamNumber > 2)
    {
        if (mAffiliation[1] > 0 && mAffiliation[2] > 0)
        {
            mCaptureTeam = 0;
        }
        else if (mAffiliation[1] > 0)
        {
            mCaptureTeam = 1;
        }
        else if (mAffiliation[2] > 0)
        {
            mCaptureTeam = 2;
        }
    }

    for (int i = 0; i < 8; ++i)
    {
        if (sTeams[i]->GetSide() == side_Locals && mAffiliation[i] == 0)
        {
            mColor[i] = RedColor::YELLOW;
        }
        else if (mAffiliation[i] > 0)
        {
            RedColor friendColor = { 0x01, 0x56, 0xd5, 0xff };
            mColor[i] = friendColor;
        }
        else if (mAffiliation[i] < 0)
        {
            mColor[i] = RedColor::RED;
        }
        else
        {
            mColor[i] = RedColor::LTGREY;
        }
    }

    char acKey[64];
    switch (mSide)
    {
    case side_Alliance:
        mName = (unsigned short*)RedLocalizeDB::Find(0x9A2AD740u, true);
        if (!mName)
        {
            mName = (unsigned short*)RedLocalizeDB::Find(0x9A2AD740u, false);
        }
        break;
    case side_Empire:
        mName = (unsigned short*)RedLocalizeDB::Find(0xB08A43AFu, true);
        if (!mName)
        {
            mName = (unsigned short*)RedLocalizeDB::Find(0xB08A43AFu, false);
        }
        break;
    case side_Republic:
        mName = (unsigned short*)RedLocalizeDB::Find(0x373D9190u, true);
        if (!mName)
        {
            mName = (unsigned short*)RedLocalizeDB::Find(0x373D9190u, false);
        }
        break;
    case side_CIS:
        mName = (unsigned short*)RedLocalizeDB::Find(0xEEEC5DE2u, true);
        if (!mName)
        {
            mName = (unsigned short*)RedLocalizeDB::Find(0xEEEC5DE2u, false);
        }
        break;
    case side_Locals:
        if (strlen(mSideNameLocal))
        {
            _snprintf(acKey, sizeof(acKey), "level.%s.%s", GameLoop::mWorldFileName, mSideNameLocal);
            mName = (unsigned short*)RedLocalizeDB::Find(PblHash(acKey), true);
            if (mName)
            {
                break;
            }
        }
        _snprintf(acKey, sizeof(acKey), "level.%s.locals", GameLoop::mWorldFileName);
        mName = (unsigned short*)RedLocalizeDB::Find(PblHash(acKey), true);
        if (mName)
        {
            break;
        }
        _snprintf(acKey, sizeof(acKey), "level.%s.locals", GameLoop::mTerrainFileName);
        mName = (unsigned short*)RedLocalizeDB::Find(PblHash(acKey), true);
        if (!mName)
        {
            mName = (unsigned short*)RedLocalizeDB::Find(PblHash(acKey), false);
        }
        break;
    case side_HEROES:
        mName = (unsigned short*)RedLocalizeDB::Find(0xA7217A49u, true);
        if (!mName)
        {
            mName = (unsigned short*)RedLocalizeDB::Find(0xA7217A49u, false);
        }
        break;
    case side_VILLAINS:
        mName = (unsigned short*)RedLocalizeDB::Find(0x81FD9981u, true);
        if (!mName)
        {
            mName = (unsigned short*)RedLocalizeDB::Find(0x81FD9981u, false);
        }
        break;
    default:
        break;
    }

    typedef void(__thiscall * CharacterInitialUpdateFn)(Character*);
    for (int i = 0; i < mMemberCount; ++i)
    {
        Character* pMember = mMemberArray[i];
        pMember->InitialUpdate();
    }

    mbInitialUpdateDone = true;
    for (int i = 0; i < mClassCount; ++i)
    {
        _ClampClassCounts(i);
    }
}

// FUNCTION: SWBF2 0x00653b70
void Team::SetSide(const char* pcName)
{
    SetSide(TranslateTeamName(pcName));
    if (mSide == side_Locals)
    {
        strlcpy(mSideNameLocal, pcName, MAX_SIDE_NAME_LOCAL_LEN);
    }
    else
    {
        mSideNameLocal[0] = 0;
    }
}

// FUNCTION: SWBF2 0x00653bb0
void Team::Update(float)
{
}

// FUNCTION: SWBF2 0x00653bc0
void Team::SetCurReinforcements(int iCount)
{
    mCurReinforcements = iCount;
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        EventManager::TeamEvent::mTicketCountChange.Trigger(*this, mCurReinforcements);
    }
}

// FUNCTION: SWBF2 0x00653c10
void Team::AddReinforcements(int iCount)
{
    if (NetGame::IsPreGame())
    {
        return;
    }
    mCurReinforcements += iCount;
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        EventManager::TeamEvent::mTicketCountChange.Trigger(*this, mCurReinforcements);
    }
}

// FUNCTION: SWBF2 0x00653c70
void Team::SetValues(int iCurReinforcements, int iMemberActive, int iPoints)
{
    mCurReinforcements = iCurReinforcements;
    mMemberActive = iMemberActive;
    mPoints = iPoints;
}

// FUNCTION: SWBF2 0x00653c90
void Team::GetValues(int& iCurReinforcements, int& iMemberActive, int& iPoints)
{
    iCurReinforcements = mCurReinforcements;
    iMemberActive = mMemberActive;
    iPoints = mPoints;
}

// FUNCTION: SWBF2 0x00653cb0
Character* Team::FindByUnit(Controllable* pUnit)
{
    for (int i = 0; i < mMemberCount; ++i)
    {
        if (mMemberArray[i]->mUnit == pUnit)
        {
            return mMemberArray[i];
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00653d00
void Team::CreateCharacter(const unsigned short* pName, int iPlayerId)
{
    if (mMemberCount < mMemberLimit)
    {
        Character* pCharacter = Character::CreateCharacter();
        pCharacter->Init(mTeamNumber, pName, iPlayerId);
        mMemberArray[mMemberCount++] = pCharacter;
    }
}

// FUNCTION: SWBF2 0x00653d50
bool Team::IsMemberAlive(int iIndex) const
{
    Character* pCharacter = GetMemberPtr(iIndex);
    if (pCharacter && pCharacter->mUnit)
    {
        GameObject* pObject = pCharacter->mUnit->GetGameObject();
        return static_cast<const Damageable*>(pObject)->mIsAlive;
    }
    return false;
}

// FUNCTION: SWBF2 0x00653da0
int Team::GetNumMembersAlive() const
{
    int iCount = 0;
    for (int i = 0; i < mMemberCount; ++i)
    {
        if (IsMemberAlive(i))
        {
            ++iCount;
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x00653dd0
Character* Team::GetRandomLiveCharacter() const
{
    int iCount = 0;
    for (int i = 0; i < mMemberCount; ++i)
    {
        if (mMemberArray[i]->mUnit)
        {
            ++iCount;
        }
    }
    if (iCount > 0)
    {
        unsigned int uPick = PblRandom::GetGlobalInt() % iCount;
        for (int i = 0; i < mMemberCount; ++i)
        {
            if (mMemberArray[i]->mUnit && uPick-- == 0)
            {
                return mMemberArray[i];
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00653e60
bool Team::AddClass(GameObjectClass* pClass, int iMinUnits, int iMaxUnits)
{
    if (pClass && mClassCount < mClassLimit)
    {
        mClassArray[mClassCount++] = pClass;
        _SetClassUnitCounts(mClassCount - 1, iMinUnits, iMaxUnits);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00653ea0
bool Team::AddClass(const char* pcName, int iMinUnits, int iMaxUnits)
{
    GameObjectClass* pClass = static_cast<GameObjectClass*>(EntityClass::Find(PblHash(pcName)));
    if (pClass)
    {
        return AddClass(pClass, iMinUnits, iMaxUnits);
    }
    return true;
}

// FUNCTION: SWBF2 0x00653f00
bool Team::AddSpecialClass(GameObjectClass* pClass)
{
    if (mClassCount < mClassLimit)
    {
        pClass->SetHeroFlag();
        mClassArray[mClassCount++] = pClass;
        ++mSpecialClassCount;
        _SetClassUnitCounts(mClassCount - 1, 0, 1);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00653f50
bool Team::AddSpecialClass(unsigned int uHash)
{
    GameObjectClass* pClass = static_cast<GameObjectClass*>(EntityClass::Find(uHash));
    if (pClass)
    {
        return AddSpecialClass(pClass);
    }
    return false;
}

// FUNCTION: SWBF2 0x00653f90
GameObjectClass* Team::FindClass(const char* pcName)
{
    return FindClass(static_cast<GameObjectClass*>(EntityClass::Find(PblHash(pcName))));
}

// FUNCTION: SWBF2 0x00653fe0
GameObjectClass* Team::FindClass(GameObjectClass* pClass)
{
    for (int i = 0; i < mClassCount; ++i)
    {
        if (mClassArray[i] == pClass)
        {
            return mClassArray[i];
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00654020
int Team::GetClassIndex(GameObjectClass* pClass)
{
    for (int i = 0; i < mClassCount; ++i)
    {
        if (mClassArray[i] == pClass)
        {
            return i;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x00654050
bool Team::SetClassUnitCounts(GameObjectClass* pClass, int iMinUnits, int iMaxUnits)
{
    for (int i = 0; i < mClassCount; ++i)
    {
        if (mClassArray[i] == pClass)
        {
            _SetClassUnitCounts(i, iMinUnits, iMaxUnits);
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00654090
void Team::SetCarrier(const char* pcName)
{
    SetCarrier(static_cast<GameObjectClass*>(EntityClass::Find(PblHash(pcName))));
}

// FUNCTION: SWBF2 0x006540e0
void Team::SetBleedRate(float fRate)
{
    mBleedRate = fRate;
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        EventManager::TeamEvent::mTicketBleedChange.Trigger(*this, mBleedRate);
    }
}

// FUNCTION: SWBF2 0x00654130
void Team::SetPoints(int iPoints)
{
    mPoints = iPoints;
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        EventManager::TeamEvent::mPointsChange.Trigger(*this, mPoints);
    }
}

// FUNCTION: SWBF2 0x00654180
void Team::AddPoints(int iPoints)
{
    if (NetGame::IsPreGame())
    {
        return;
    }
    mPoints += iPoints;
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        EventManager::TeamEvent::mPointsChange.Trigger(*this, mPoints);
    }
}

// FUNCTION: SWBF2 0x006541e0
int Team::PickRandomClassIndex(Character* pCharacter) const
{
    int iClassCount = mClassCount;
    if (!iClassCount)
    {
        return -1;
    }

    int aiUnits[12];
    int aiOrder[12];
    memset(aiUnits, 0, sizeof(aiUnits));

    bool bReserve = false;
    for (int i = 0; i < mMemberCount; ++i)
    {
        Character* pMember = GetMemberPtr(i);
        if (pMember->mClassPtr)
        {
            ++aiUnits[pMember->mClassIndex];
        }
        if (pMember->mPlayerId >= 0)
        {
            bReserve = true;
        }
    }
    if (GameLoop::GetMissionTime() < 60.0f)
    {
        bReserve = true;
    }

    for (int i = 0; i < iClassCount; ++i)
    {
        aiOrder[i] = i;
    }
    for (int n = iClassCount; n > 1; --n)
    {
        int j = PblRandom::GetSim().GetInt() % n;
        int iTemp = aiOrder[j];
        aiOrder[j] = aiOrder[n - 1];
        aiOrder[n - 1] = iTemp;
    }

    for (int i = 0; i < mClassCount; ++i)
    {
        int iIndex = aiOrder[i];
        if (mClassArray[iIndex]->mHeroFlag)
        {
            if (pCharacter == Character::GetPtr(netHeroAIID[mTeamNumber]))
            {
                return iIndex;
            }
        }
        else if (!bReserve || aiUnits[iIndex] < mClassMaxUnits[iIndex] - 1)
        {
            if (aiUnits[iIndex] < mClassMinUnits[iIndex])
            {
                return iIndex;
            }
        }
    }
    for (int i = 0; i < mClassCount; ++i)
    {
        int iIndex = aiOrder[i];
        if (!mClassArray[iIndex]->mHeroFlag &&
            (!bReserve || aiUnits[iIndex] < mClassMaxUnits[iIndex] - 1) &&
            aiUnits[iIndex] < mClassMaxUnits[iIndex])
        {
            return iIndex;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x00654420
int Team::GetClassMinUnits(int iIndex) const
{
    return mClassMinUnits[iIndex];
}

// FUNCTION: SWBF2 0x00654430
int Team::GetClassMaxUnits(int iIndex) const
{
    return mClassMaxUnits[iIndex];
}

static inline const GameObjectClass* GetUnitClass(Controllable* pUnit)
{
    return *(GameObjectClass* const*)((const char*)pUnit->GetGameObject() + 8);
}

// FUNCTION: SWBF2 0x00654440
int Team::GetClassCurrentUnits(int iIndex) const
{
    int iCount = 0;
    for (int i = 0; i < mMemberCount; ++i)
    {
        Controllable* pUnit = GetMemberPtr(i)->mUnit;
        if (pUnit && GetUnitClass(pUnit) == mClassArray[iIndex])
        {
            ++iCount;
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x006544a0
void Team::GenerateRandomName(unsigned short* const pName, int iTeam) const
{
    memset(pName, 0, 64);

    if (mSide == side_Locals)
    {
        char acKey[64];
        _snprintf(acKey, sizeof(acKey), "level.%s.%s", GameLoop::mWorldFileName, mSideNameLocal);
        const unsigned short* pusName = RedLocalizeDB::Find(PblHash(acKey), true);
        if (!pusName)
        {
            _snprintf(acKey, sizeof(acKey), "level.%s.local", GameLoop::mWorldFileName);
            pusName = RedLocalizeDB::Find(PblHash(acKey), true);
            if (!pusName)
            {
                _snprintf(acKey, sizeof(acKey), "level.%s.local", GameLoop::mTerrainFileName);
                pusName = RedLocalizeDB::Find(PblHash(acKey), true);
                if (!pusName)
                {
                    pusName = RedLocalizeDB::Find(PblHash(acKey), false);
                }
            }
        }
        PblUnicode::ustrncpy(pName, pusName, 64);
        return;
    }

    unsigned int uRand = mNameRand.GetInt();
    if (mFirstCount)
    {
        PblUnicode::ustrlcat(pName, mFirstNames[uRand % mFirstCount], 64);
    }
    else
    {
        pName[0] = (unsigned short)('A' + uRand % 26);
        pName[1] = (unsigned short)('A' + mNameRand.GetInt() % 26);
        pName[2] = 0;
    }

    unsigned int uLen = PblUnicode::ustrlen(pName);
    pName[uLen] = ' ';
    pName[uLen + 1] = 0;

    uRand = mNameRand.GetInt();
    if (mLastCount)
    {
        PblUnicode::ustrlcat(pName, mLastNames[uRand % mLastCount], 64);
    }
    else
    {
        pName[uLen + 1] = (unsigned short)('0' + uRand % 10);
        pName[uLen + 2] = (unsigned short)('0' + mNameRand.GetInt() % 10);
        pName[uLen + 3] = (unsigned short)('0' + mNameRand.GetInt() % 10);
        pName[uLen + 4] = 0;
    }
}

// FUNCTION: SWBF2 0x00654760
void Team::SetAggressiveness(float fAggressiveness)
{
    mAggressiveness = fAggressiveness;
}

// FUNCTION: SWBF2 0x00654770
int Team::GetTeamMemberCount(int& iTeam1, int& iTeam2)
{
    int aiCount[3];
    aiCount[1] = 0;
    aiCount[2] = 0;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (!NetGame::IsPlaying(i, false))
        {
            continue;
        }
        Character* pCharacter = Character::GetPtr(i);
        if (pCharacter && pCharacter->mUnit && pCharacter->mTeamNumber > 0)
        {
            ++aiCount[pCharacter->mTeamNumber];
        }
    }
    iTeam1 = aiCount[1];
    iTeam2 = aiCount[2];
    return iTeam1 >= iTeam2 ? 2 : 1;
}

// FUNCTION: SWBF2 0x00654800
int Team::GetAutoAssignTeamNumber()
{
    if (NetGame::IsEnabled())
    {
        int iTeam1 = 0;
        int iTeam2 = 0;
        if (NetGame::GetForceHumansToTeam1())
        {
            return 1;
        }
        return GetTeamMemberCount(iTeam1, iTeam2);
    }
    return PblRandom::GetGlobalInt() % 2 + 1;
}

// FUNCTION: SWBF2 0x00654880
const GameObjectClass* Team::GetHeroClass() const
{
    for (int i = 0; i < mClassCount; ++i)
    {
        GameObjectClass* pClass = GetClassPtr(i);
        if (pClass && pClass->mHeroFlag && pClass->mLabel)
        {
            return pClass;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006548c0
void Team::_SetClassUnitCounts(int iIndex, int iMinUnits, int iMaxUnits)
{
    mClassMinUnits[iIndex] = iMinUnits;
    mClassMaxUnits[iIndex] = iMaxUnits;
    _ClampClassCounts(iIndex);
}

// FUNCTION: SWBF2 0x006548f0
void Team::_ClampClassCounts(int iIndex)
{
    if (!mbInitialUpdateDone)
    {
        return;
    }
    GameObjectClass* pClass = GetClassPtr(iIndex);
    if (!pClass || !pClass->IsRtti(EntityDroidekaClass::rttiHashEntityDroidekaClass))
    {
        return;
    }

    MemoryPool* pPool = MemoryPool::Find("EntityDroideka");
    int iMin = mClassMinUnits[iIndex];
    if (iMin < 0)
    {
        iMin = 0;
    }
    else if (iMin > (int)pPool->GetCount())
    {
        iMin = pPool->GetCount();
    }
    mClassMinUnits[iIndex] = iMin;

    int iMax = mClassMaxUnits[iIndex];
    if (iMax < 0)
    {
        iMax = 0;
    }
    else if (iMax > (int)pPool->GetCount())
    {
        iMax = pPool->GetCount();
    }
    mClassMaxUnits[iIndex] = iMax;
}

namespace EventManager
{

// FUNCTION: SWBF2 0x00654980
template <>
void Event<Team, int>::Callback::Trigger(Team& item, int& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild->IsAlive())
            {
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
                if (!pChild)
                {
                    break;
                }
            }
            if (pChild)
            {
                pChild->Release();
            }
        }
    }
    Release();
}

// FUNCTION: SWBF2 0x00654aa0
template <>
void Event<Team, float>::Callback::Trigger(Team& item, float& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild->IsAlive())
            {
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
                if (!pChild)
                {
                    break;
                }
            }
            if (pChild)
            {
                pChild->Release();
            }
        }
    }
    Release();
}
}
