#include "PlayerStats.h"
#include "PlayerProfile.h"
#include "Character.h"
#include "Controllable.h"
#include "FlagItem.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "MedalsMgr.h"
#include "NetComm.h"
#include "NetGame.h"
#include "NetPacket.h"
#include "Factory.h"
#include "CommandPost.h"
#include "PowerupUtility.h"
#include "SpaceAssault.h"
#include "Team.h"
#include "EntityDroideka.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "PebbleFL/Common/PblList.h"
#include <float.h>

// FUNCTION: SWBF2 0x0061f490
const EntityClass* PlayerStats::GetFavoriteSoldierClass(int) const
{
    float bestTime = 0.0f;
    int bestTeam = 0, bestClass = 0;
    for (int team = 0; team < 2; ++team)
    {
        for (int slot = 0; slot < MAX_STAT_CLASSES; ++slot)
        {
            if (mTimePlayedPerClassList[team][slot] && mTimePlayedPerClass[team][slot]._val > bestTime)
            {
                bestTime = mTimePlayedPerClass[team][slot]._val;
                bestTeam = team;
                bestClass = slot;
            }
        }
    }
    return mTimePlayedPerClassList[bestTeam][bestClass];
}

// FUNCTION: SWBF2 0x0061f540
float PlayerStats::GetFavoriteSoldierTime(int) const
{
    float bestTime = 0.0f;
    for (int team = 0; team < 2; ++team)
    {
        for (int slot = 0; slot < MAX_STAT_CLASSES; ++slot)
        {
            if (mTimePlayedPerClassList[team][slot] && mTimePlayedPerClass[team][slot]._val > bestTime)
            {
                bestTime = mTimePlayedPerClass[team][slot]._val;
            }
        }
    }
    return bestTime;
}

// FUNCTION: SWBF2 0x0061f5c0
const WeaponClass* PlayerStats::GetFavoriteWeaponClass(int) const
{
    float bestTime = 0.0f;
    int bestTeam = 0, bestWeapon = 0;
    for (int team = 0; team < 2; ++team)
    {
        for (int slot = 0; slot < MAX_STAT_WEAPONS; ++slot)
        {
            if (mTimePlayedPerWeaponList[team][slot] && mTimePlayedPerWeapon[team][slot]._val > bestTime)
            {
                bestTime = mTimePlayedPerWeapon[team][slot]._val;
                bestTeam = team;
                bestWeapon = slot;
            }
        }
    }
    return mTimePlayedPerWeaponList[bestTeam][bestWeapon];
}

// FUNCTION: SWBF2 0x0061f6f0
float PlayerStats::GetFavoriteWeaponTime(int) const
{
    float bestTime = 0.0f;
    for (int team = 0; team < 2; ++team)
    {
        for (int slot = 0; slot < MAX_STAT_WEAPONS; ++slot)
        {
            if (mTimePlayedPerWeaponList[team][slot] && mTimePlayedPerWeapon[team][slot]._val > bestTime)
            {
                bestTime = mTimePlayedPerWeapon[team][slot]._val;
            }
        }
    }
    return bestTime;
}

// FUNCTION: SWBF2 0x0061f7d0
const EntityClass* PlayerStats::GetFavoriteVehicleClass(int) const
{
    float bestTime = 0.0f;
    int bestTeam = 0, bestVehicle = 0;
    for (int team = 0; team < 2; ++team)
    {
        for (int slot = 0; slot < MAX_STAT_VEHICLES; ++slot)
        {
            if (mTimeInsideVehicleList[team][slot] && mTimeInsideVehicle[team][slot]._val > bestTime)
            {
                bestTime = mTimeInsideVehicle[team][slot]._val;
                bestTeam = team;
                bestVehicle = slot;
            }
        }
    }
    return mTimeInsideVehicleList[bestTeam][bestVehicle];
}

// FUNCTION: SWBF2 0x0061f880
float PlayerStats::GetFavoriteVehicleTime(int) const
{
    float bestTime = 0.0f;
    for (int team = 0; team < 2; ++team)
    {
        for (int slot = 0; slot < MAX_STAT_VEHICLES; ++slot)
        {
            if (mTimeInsideVehicleList[team][slot] && mTimeInsideVehicle[team][slot]._val > bestTime)
            {
                bestTime = mTimeInsideVehicle[team][slot]._val;
            }
        }
    }
    return bestTime;
}

// FUNCTION: SWBF2 0x0061fc20
int PlayerStats::GetExperiencePoints(int, int) const
{
    return 0;
}

// FUNCTION: SWBF2 0x0061fad0
Character* PlayerStats::GetBait(int, int iTeamStat) const
{
    const int iOwnTeam = Character::GetPtr(mStatId)->mTeamNumber;
    int iBest = 0, iBestIndex = 0;
    for (int iTeam = 0; iTeam < 2; ++iTeam)
    {
        for (int i = 0; i < Character::sCharacterLimit; ++i)
        {
            int iValue = mKillsByPlayer[iTeam][iTeamStat][i]._val;
            if (iValue > iBest || (iValue == iBest && iTeam != iOwnTeam))
            {
                iBest = iValue;
                iBestIndex = i;
            }
        }
    }
    return Character::GetPtr(iBestIndex);
}

// FUNCTION: SWBF2 0x0061fb90
int PlayerStats::GetBaitVal(int, int iTeamStat) const
{
    const int iOwnTeam = Character::GetPtr(mStatId)->mTeamNumber;
    int iBest = 0;
    for (int iTeam = 0; iTeam < 2; ++iTeam)
    {
        for (int i = 0; i < Character::sCharacterLimit; ++i)
        {
            int iValue = mKillsByPlayer[iTeam][iTeamStat][i]._val;
            if (iValue > iBest || (iValue == iBest && iTeam != iOwnTeam))
            {
                iBest = iValue;
            }
        }
    }
    return iBest;
}

// FUNCTION: SWBF2 0x0061f900
Character* PlayerStats::GetNemesis(int, int iTeamStat) const
{
    const int iOwnTeam = Character::GetPtr(mStatId)->mTeamNumber;
    int iBest = 0, iBestIndex = 0;
    for (int iTeam = 0; iTeam < 2; ++iTeam)
    {
        for (int i = 0; i < Character::sCharacterLimit; ++i)
        {
            PlayerStats& candidate = Character::sCharacters[i].GetPlayerStats();
            int iValue = candidate.mKillsByPlayer[iTeam][iTeamStat][mStatId]._val;
            if (iValue > iBest || (iValue == iBest && iTeam != iOwnTeam))
            {
                iBest = iValue;
                iBestIndex = i;
            }
        }
    }
    return Character::GetPtr(iBestIndex);
}

// FUNCTION: SWBF2 0x0061fa00
int PlayerStats::GetNemesisVal(int, int iTeamStat) const
{
    const int iOwnTeam = Character::GetPtr(mStatId)->mTeamNumber;
    int iBest = 0;
    for (int iTeam = 0; iTeam < 2; ++iTeam)
    {
        for (int i = 0; i < Character::sCharacterLimit; ++i)
        {
            PlayerStats& candidate = Character::sCharacters[i].GetPlayerStats();
            int iValue = candidate.mKillsByPlayer[iTeam][iTeamStat][mStatId]._val;
            if (iValue > iBest || (iValue == iBest && iTeam != iOwnTeam))
            {
                iBest = iValue;
            }
        }
    }
    return iBest;
}

class MountedTurret { public: static PblHash rttiHashMountedTurret; };

// GLOBAL: SWBF2 0x007e9038
bool PlayerStats::mStatsActive = true;

// GLOBAL: SWBF2 0x01eaedf9
bool PlayerStats::mGotMPStats;

// GLOBAL: SWBF2 0x01eaedfc
char PlayerStatsPoints[35];

// FUNCTION: SWBF2 0x0061d900
void PlayerStats::PointsProcessTeam(int iTeam, unsigned char ucType)
{
    SpaceAssault* pSpaceAssault = SpaceAssault::mInstance;
    if (pSpaceAssault && pSpaceAssault->mbEnabled && PlayerStatsPoints[ucType] > 0)
    {
        Team::GetPtr(iTeam)->AddPoints(PlayerStatsPoints[ucType]);
    }
}

// FUNCTION: SWBF2 0x0061d940
int PlayerStats::PointsProcess(unsigned char ucType, unsigned int uParam)
{
    if (NetGame::IsPreGame())
    {
        return 0;
    }

    Character* pCharacter = Character::GetPtr(mStatId);
    mPointsDelta = PlayerStatsPoints[ucType];

    SpaceAssault* pSpaceAssault = SpaceAssault::mInstance;
    if (pSpaceAssault && pSpaceAssault->mbEnabled)
    {
        if (ucType == 32)
        {
            short sPoints = pSpaceAssault->GetPointValue(uParam, pCharacter);
            if (sPoints)
            {
                mPointsDelta = sPoints;
            }
        }
        if (pCharacter->mPlayerId >= 0 || mPointsDelta > 0)
        {
            pCharacter->mTeamPtr->AddPoints(mPointsDelta);
        }
    }

    if (pCharacter->IsHero())
    {
        mHeroPoints += mPointsDelta;
        return mHeroPoints;
    }

    mPlayerPoints += mPointsDelta;
    if (MedalsMgr* pMedals = pCharacter->mMedalsMgr)
    {
        for (int i = 0; i < MedalsMgr::MEDAL_TYPES_COUNT; ++i)
        {
            pMedals->AddMedalPoints(i, false, false, NUM_HEALTH_TYPES, false, true, mPointsDelta);
        }
    }
    return mPlayerPoints;
}

// FUNCTION: SWBF2 0x0061da70
int PlayerStats::GetPoints() const
{
    Character* pCharacter = Character::GetPtr(mStatId);
    return GetPoints(!pCharacter->IsHero() || !pCharacter->mUnit);
}

// FUNCTION: SWBF2 0x0061dad0
int PlayerStats::GetPoints(bool bPlayer) const
{
    if (bPlayer)
    {
        return mPlayerPoints;
    }
    return mHeroPoints;
}

// FUNCTION: SWBF2 0x0061daf0
void PlayerStats::SetPoints(short sPoints)
{
    Character* pCharacter = Character::GetPtr(mStatId);
    SetPoints(!pCharacter->IsHero() || !pCharacter->mUnit, sPoints);
}

// FUNCTION: SWBF2 0x0061db60
void PlayerStats::SetPoints(bool bPlayer, short sPoints)
{
    if (bPlayer)
    {
        mPointsDelta = sPoints - mPlayerPoints;
        mPlayerPoints = sPoints;
    }
    else
    {
        mPointsDelta = sPoints - mHeroPoints;
        mHeroPoints = sPoints;
    }
}

// FUNCTION: SWBF2 0x0061dbb0
PlayerStats::PlayerStats()
{
    mCampingPos = PblVector3(0.0f, 0.0f, 0.0f);
    mNarrowEscapeFlag = false;
    mLastKillTime = 0.0f;
    mConsecutiveKillCnt = 0;
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            mKillsByPlayer[i][j] = 0;
        }
    }
    mPlayerPoints = 0;
    mHeroPoints = 0;
    mPointsDelta = 0;
}

// FUNCTION: SWBF2 0x0061ddb0
PlayerStats::~PlayerStats()
{
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            delete[] mKillsByPlayer[i][j];
        }
    }
    mPlayerPoints = 0;
    mHeroPoints = 0;
    mPointsDelta = 0;
}

// FUNCTION: SWBF2 0x0061de00
void PlayerStats::Init(int iStatId, int iNumPlayers)
{
    mStatId = (unsigned char)iStatId;
    mKillsByPlayerSize = iNumPlayers;
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            mKillsByPlayer[i][j] = new CntStat<unsigned char>[iNumPlayers];
            for (int k = 0; k < MAX_STAT_CLASSES; ++k)
            {
                mTimePlayedPerClassList[i][k] = 0;
            }
            for (int k = 0; k < MAX_STAT_VEHICLES; ++k)
            {
                mTimeInsideVehicleList[i][k] = 0;
            }
            for (int k = 0; k < MAX_STAT_WEAPONS; ++k)
            {
                mTimePlayedPerWeaponList[i][k] = 0;
            }
        }
    }
    mGotMPStats = false;
    mPauseCount = 0;
    mStatsActive = true;
    mNextNetAuthTime = 0.0f;
}

// FUNCTION: SWBF2 0x0061def0
void PlayerStats::Reset()
{
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            mKills[i][j].Set(0);
            mSameTeamKills[i][j].Set(0);
            mVehicleKills[i][j].Set(0);
            mFlagCaps[i][j].Set(0);
            mKillingSpree[i][j].SetMax(0);
            mKillsUnder25Health[i][j].Set(0);
            mKillsPerSpawn[i][j].ResetCurrent();
            mKillsPerSpawn[i][j].SetMax(0);
            mShotsFired[i][j].Set(0);
            mShotsHit[i][j].Set(0);
            mCriticalShots[i][j].Set(0);
            mDeaths[i][j].Set(0);
            for (int k = 0; k < Character::sCharacterLimit; ++k)
            {
                mKillsByPlayer[i][j][k].Set(0);
            }

            for (int k = 0; k < MAX_STAT_CLASSES; ++k)
            {
                mTimePlayedPerClassList[i][k] = 0;
                mTimePlayedPerClass[i][k].Set(0.0f);
            }
            for (int k = 0; k < MAX_STAT_VEHICLES; ++k)
            {
                mTimeInsideVehicleList[i][k] = 0;
                mTimeInsideVehicle[i][k].Set(0.0f);
            }
            for (int k = 0; k < MAX_STAT_WEAPONS; ++k)
            {
                mTimePlayedPerWeaponList[i][k] = 0;
                mTimePlayedPerWeapon[i][k].Set(0.0f);
            }
            mMaxAliveTime[i].SetMax(0.0f);
            mMaxAliveTime[i].ResetCurrent();
            mCampingTime[i].SetMax(0.0f);
            mCampingTime[i].ResetCurrent();
            mCommandPostPoints[i].Set(0);
            mNarrowEscapes[i].Set(0);
            mBattlePoints[i].Set(0);
        }
    }
    mNextNetAuthTime = 0.0f;
    ResetStats();
}

Controllable* GetControllablePilot(Controllable* pControllable);

namespace AI
{
enum VOType
{
    VOT_KillingSpree4 = 12,
    VOT_KillingSpree8 = 13,
    VOT_KillingSpree12 = 14,
    VOT_KillingSpree16 = 15
};
namespace VOHelper
{
int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const GameObject* pOther, float fDelay);
}
}

// FUNCTION: SWBF2 0x0061e070
void PlayerStats::AddKill(Character* pVictim, GameObject* pObject)
{
    if (!mStatsActive || GameLoop::IsGameOver() || NetGame::IsPreGame())
    {
        return;
    }

    int iTeam = _GetTeamNumber();
    int iHero = Character::GetPtr(mStatId)->IsHero();
    if (iHero)
    {
        Character* pCharacter = Character::GetPtr(mStatId);
        if (pCharacter && pCharacter->mUnit &&
            (!pCharacter->GetControllable() || !GetControllablePilot(pCharacter->GetControllable())))
        {
            GameObject* pUnitObject = pCharacter->mUnit->GetGameObject();
            float fHealth;
            if (mKillsPerSpawn[iTeam][iHero].GetCurrent() >= 30)
            {
                fHealth = 50.0f;
            }
            else if (mKillsPerSpawn[iTeam][iHero].GetCurrent() >= 15)
            {
                fHealth = 100.0f;
            }
            else
            {
                fHealth = 150.0f;
            }
            PowerupUtility::ApplyHealth(pUnitObject, fHealth, 0, true);
        }
    }

    bool bBonus = false;
    for (int i = 0; i < 16; ++i)
    {
        CommandPost* pPost = CommandPost::sPostArray[i];
        if (!pPost || !pPost->GetGameObject() || !pObject)
        {
            continue;
        }

        int iAffiliation = pVictim->GetTeamPtr()->GetAffiliation(pPost->mBiasTeam);
        const PblVector3& vPost = pPost->GetMatrix().GetTranslation();
        const PblVector3& vObject = pObject->GetMatrix().GetTranslation();
        float fX = vObject.x - vPost.x;
        float fY = vObject.y - vPost.y;
        float fZ = vObject.z - vPost.z;
        float fDistSq = fX * fX + fY * fY + fZ * fZ;
        if (iAffiliation < 0 && pPost->mBiasTeam == iTeam + 1 && pPost->IsInsideCaptureRegion(pObject))
        {
            PointsProcess(23, 0);
            bBonus = true;
            break;
        }
        if (iAffiliation <= 0 && fDistSq < 400.0f)
        {
            PointsProcess(24, 0);
            bBonus = true;
            break;
        }
    }

    for (PblList<FlagItem>::Itor it(*FlagItem::sList); (FlagItem*)it; ++it)
    {
        FlagItem* pFlag = it;
        if (pFlag->GetLastCarrierCharacter() == pVictim)
        {
            PointsProcess(30, 0);
            bBonus = true;
        }

        GameObject* pFlagObject = reinterpret_cast<GameObject*>(pFlag);
        if (pFlagObject->GetTeam() == pVictim->GetTeamNumber() || !pObject)
        {
            continue;
        }

        const PblVector3& vObject = pObject->GetMatrix().GetTranslation();
        const PblVector3& vFlag = pFlagObject->GetPosition();
        float fX = vFlag.x - vObject.x;
        float fY = vFlag.y - vObject.y;
        float fZ = vFlag.z - vObject.z;
        if (fX * fX + fY * fY + fZ * fZ <= 100.0f)
        {
            Controllable* pMine = Character::GetPtr(mStatId)->GetControllable();
            if (pFlag->GetCarrier())
            {
                if (pMine && pFlag->GetCarrier() != pMine->GetGameObject())
                {
                    PointsProcess(29, 0);
                    bBonus = true;
                }
            }
            else
            {
                PointsProcess(27, 0);
                bBonus = true;
            }
        }
    }

    mKills[iTeam][iHero].Inc(1);
    mKillsByPlayer[iTeam][iHero][pVictim->GetPlayerStats().mStatId].Inc(1);
    if (mLastHealth < 64)
    {
        mKillsUnder25Health[iTeam][iHero].Inc(1);
    }
    mKillsPerSpawn[iTeam][iHero].Inc(1);

    if (!bBonus)
    {
        if (pVictim->IsHero())
        {
            PointsProcess(20, 0);
        }
        else
        {
            PointsProcess(pVictim->IsPlayer() ? 1 : 0, 0);
        }
    }

    float fTime = GameLoop::GetMissionTime();
    if (fTime > mLastKillTime + 4.0f)
    {
        mConsecutiveKillCnt = 0;
    }
    ++mConsecutiveKillCnt;
    mLastKillTime = fTime;
    mKillingSpree[iTeam][iHero].Add(mConsecutiveKillCnt);

    Character* pCharacter = Character::GetPtr(mStatId);
    if (pCharacter && pCharacter->IsPlayer() && pCharacter->GetControllable() &&
        pCharacter->GetControllable()->GetGameObject())
    {
        switch (mConsecutiveKillCnt)
        {
        case 4:
            AI::VOHelper::Play(AI::VOT_KillingSpree4, pCharacter->GetControllable()->GetGameObject(), 0, 0, 0.0f);
            break;
        case 8:
            AI::VOHelper::Play(AI::VOT_KillingSpree8, pCharacter->GetControllable()->GetGameObject(), 0, 0, 0.0f);
            break;
        case 12:
            AI::VOHelper::Play(AI::VOT_KillingSpree12, pCharacter->GetControllable()->GetGameObject(), 0, 0, 0.0f);
            break;
        case 16:
            AI::VOHelper::Play(AI::VOT_KillingSpree16, pCharacter->GetControllable()->GetGameObject(), 0, 0, 0.0f);
            break;
        }
    }

    if (NetGame::IsEnabled() && !NetGame::IsClient())
    {
        Character* pPlayer = Character::GetPtr(mStatId);
        if (pPlayer && pPlayer->mPlayerId >= 0)
        {
            NetComm::SetPLKills(pPlayer->mPlayerId, GetKills(-1, -1));
        }
    }
}

// FUNCTION: SWBF2 0x0061e6a0
void PlayerStats::AddAllyKill(Character* pVictim, GameObject*)
{
    if (!mStatsActive || GameLoop::IsGameOver() || NetGame::IsPreGame())
    {
        return;
    }

    int iTeam = _GetTeamNumber();
    int iHero = Character::GetPtr(mStatId)->IsHero();
    mKills[iTeam][iHero].Inc(1);
    mSameTeamKills[iTeam][iHero].Inc(1);
    mKillsByPlayer[iTeam][iHero][pVictim->GetPlayerStats().mStatId].Inc(1);

    if (Character::GetPtr(mStatId) == pVictim)
    {
        PointsProcess(3, 0);
    }
    else
    {
        PblList<FlagItem>::Itor it(*FlagItem::sList);
        for (; (FlagItem*)it; ++it)
        {
            if (it->GetLastCarrierCharacter() == pVictim)
            {
                break;
            }
        }
        if ((FlagItem*)it)
        {
            PointsProcess(31, 0);
        }
        else
        {
            PointsProcess(4, 0);
        }
    }

    if (NetGame::IsEnabled() && !NetGame::IsClient())
    {
        Character* pCharacter = Character::GetPtr(mStatId);
        if (pCharacter && pCharacter->mPlayerId >= 0)
        {
            NetComm::SetPLKills(pCharacter->mPlayerId, GetKills(-1, -1));
        }
    }
}

// FUNCTION: SWBF2 0x0061e830
void PlayerStats::AddVehicleKill(int iVehicleType)
{
    if (!mStatsActive || GameLoop::IsGameOver() || NetGame::IsPreGame())
    {
        return;
    }

    int iTeam = _GetTeamNumber();
    int iHero = Character::GetPtr(mStatId)->IsHero();
    int iMyVehicleType = 0;
    if (Controllable* pVehicle = Character::GetPtr(mStatId)->mVehicle)
    {
        GameObject* pObject = pVehicle->GetGameObject();
        iMyVehicleType = static_cast<GameObjectClass*>(pObject->mEntityClass)->GetVehicleType();
    }

    if (iVehicleType == 8)
    {
        PointsProcess(12, 0);
    }
    else if (iMyVehicleType < 1 || iMyVehicleType > 3)
    {
        PointsProcess(5, 0);
    }
    else if (iMyVehicleType > iVehicleType)
    {
        PointsProcess(9, 0);
    }
    else
    {
        PointsProcess(6, 0);
    }
    mVehicleKills[iTeam][iHero].Inc(1);
}

// FUNCTION: SWBF2 0x0061e940
void PlayerStats::AddFlagCapture()
{
    if (!mStatsActive || GameLoop::IsGameOver() || NetGame::IsPreGame())
    {
        return;
    }

    int iTeam = _GetTeamNumber();
    int iHero = Character::GetPtr(mStatId)->IsHero();
    mFlagCaps[iTeam][iHero].Inc(1);
    PointsProcess(28, 0);
}

// FUNCTION: SWBF2 0x0061e9b0
void PlayerStats::AddShot()
{
    if (!mStatsActive || GameLoop::IsGameOver() || NetGame::IsPreGame())
    {
        return;
    }

    int iHero = Character::GetPtr(mStatId)->IsHero();
    mShotsFired[_GetTeamNumber()][iHero].Inc(1);
}

// FUNCTION: SWBF2 0x0061ea10
void PlayerStats::AddHit()
{
    if (!mStatsActive || GameLoop::IsGameOver() || NetGame::IsPreGame())
    {
        return;
    }

    int iHero = Character::GetPtr(mStatId)->IsHero();
    mShotsHit[_GetTeamNumber()][iHero].Inc(1);
}

// FUNCTION: SWBF2 0x0061ea70
void PlayerStats::AddCriticalShot()
{
    if (!mStatsActive || GameLoop::IsGameOver() || NetGame::IsPreGame())
    {
        return;
    }

    int iHero = Character::GetPtr(mStatId)->IsHero();
    mCriticalShots[_GetTeamNumber()][iHero].Inc(1);
}

// FUNCTION: SWBF2 0x0061ead0
void PlayerStats::AddDeath(Character*)
{
    if (!mStatsActive || GameLoop::IsGameOver() || NetGame::IsPreGame())
    {
        return;
    }

    int iHero = Character::GetPtr(mStatId)->IsHero();
    int iTeam = _GetTeamNumber();
    mDeaths[iTeam][iHero].Inc(1);
    mNarrowEscapeFlag = false;
    mKillsPerSpawn[iTeam][iHero].ResetCurrent();
    mMaxAliveTime[iTeam].ResetCurrent();

    if (NetGame::IsEnabled() && !NetGame::IsClient())
    {
        Character* pCharacter = Character::GetPtr(mStatId);
        if (pCharacter && pCharacter->mPlayerId >= 0)
        {
            NetComm::SetPLDeaths(pCharacter->mPlayerId, GetDeaths(-1, -1));
        }
    }
}

// FUNCTION: SWBF2 0x0061ebc0
void PlayerStats::AddTimePlayedWithWeapon(const WeaponClass* pWeaponClass, float fTime)
{
    if (!mStatsActive || GameLoop::IsGameOver() || mPauseCount > 0)
    {
        return;
    }

    int iTeam = _GetTeamNumber();
    float fMinTime = FLT_MAX;
    int iMin = 0;
    int iEmpty = -1;
    for (int i = 0; i < MAX_STAT_WEAPONS; ++i)
    {
        if (mTimePlayedPerWeaponList[iTeam][i])
        {
            if (mTimePlayedPerWeaponList[iTeam][i] == pWeaponClass)
            {
                mTimePlayedPerWeapon[iTeam][i].Inc(fTime);
                return;
            }
            if (mTimePlayedPerWeapon[iTeam][i]._val < fMinTime)
            {
                fMinTime = mTimePlayedPerWeapon[iTeam][i]._val;
                iMin = i;
            }
        }
        else if (iEmpty == -1)
        {
            iEmpty = i;
        }
    }
    if (iEmpty != -1)
    {
        iMin = iEmpty;
    }
    mTimePlayedPerWeaponList[iTeam][iMin] = pWeaponClass;
    mTimePlayedPerWeapon[iTeam][iMin].Set(fTime);
}

// FUNCTION: SWBF2 0x0061ec80
void PlayerStats::AddTimePlayedInsideVehicle(const Entity* pVehicle, float fTime)
{
    if (!mStatsActive || GameLoop::IsGameOver() || mPauseCount > 0)
    {
        return;
    }

    const EntityClass* pClass = pVehicle->GetEntityClass();
    if (pVehicle->IsRtti(MountedTurret::rttiHashMountedTurret))
    {
        pClass = pVehicle->GetGameObject()->GetEntityClass();
    }

    int iTeam = _GetTeamNumber();
    float fMinTime = FLT_MAX;
    int iMin = 0;
    int iEmpty = -1;
    for (int i = 0; i < MAX_STAT_VEHICLES; ++i)
    {
        if (mTimeInsideVehicleList[iTeam][i])
        {
            if (mTimeInsideVehicleList[iTeam][i] == pClass)
            {
                mTimeInsideVehicle[iTeam][i].Inc(fTime);
                return;
            }
            if (mTimeInsideVehicle[iTeam][i]._val < fMinTime)
            {
                fMinTime = mTimeInsideVehicle[iTeam][i]._val;
                iMin = i;
            }
        }
        else if (iEmpty == -1)
        {
            iEmpty = i;
        }
    }
    if (iEmpty != -1)
    {
        iMin = iEmpty;
    }
    mTimeInsideVehicleList[iTeam][iMin] = pClass;
    mTimeInsideVehicle[iTeam][iMin].Set(fTime);
}

// FUNCTION: SWBF2 0x0061ed90
void PlayerStats::AddTimePlayedAsClass(const EntityClass* pClass, float fTime)
{
    if (!mStatsActive || GameLoop::IsGameOver() || mPauseCount > 0)
    {
        return;
    }

    int iTeam = _GetTeamNumber();
    mTimePlayedPerTeam[iTeam].Inc(fTime);
    mMaxAliveTime[iTeam].Inc(fTime);

    float fMinTime = FLT_MAX;
    int iMin = 0;
    int iEmpty = -1;
    for (int i = 0; i < MAX_STAT_CLASSES; ++i)
    {
        if (mTimePlayedPerClassList[iTeam][i])
        {
            if (mTimePlayedPerClassList[iTeam][i] == pClass)
            {
                mTimePlayedPerClass[iTeam][i].Inc(fTime);
                return;
            }
            if (mTimePlayedPerClass[iTeam][i]._val < fMinTime)
            {
                fMinTime = mTimePlayedPerClass[iTeam][i]._val;
                iMin = i;
            }
        }
        else if (iEmpty == -1)
        {
            iEmpty = i;
        }
    }
    if (iEmpty != -1)
    {
        iMin = iEmpty;
    }
    mTimePlayedPerClassList[iTeam][iMin] = pClass;
    mTimePlayedPerClass[iTeam][iMin].Set(fTime);
}

// FUNCTION: SWBF2 0x0061eec0
void PlayerStats::AddCampingTime(PblVector3 vPos, float fTime)
{
    if (!mStatsActive || GameLoop::IsGameOver() || mPauseCount > 0)
    {
        return;
    }

    int iTeam = _GetTeamNumber();
    float fX = vPos.x - mCampingPos.x;
    float fY = vPos.y - mCampingPos.y;
    float fZ = vPos.z - mCampingPos.z;
    if (fX * fX + fY * fY + fZ * fZ > CAMPING_DIST_SQ)
    {
        mCampingPos = vPos;
        mCampingTime[iTeam].ResetCurrent();
    }
    else
    {
        mCampingTime[iTeam].Inc(fTime);
    }
}

// FUNCTION: SWBF2 0x0061ef90
void PlayerStats::AddCommandPostPoints(unsigned short, bool bCapture)
{
    if (!mStatsActive || GameLoop::IsGameOver() || NetGame::IsPreGame())
    {
        return;
    }

    int iTeam = _GetTeamNumber();
    if (bCapture)
    {
        PointsProcess(21, 0);
    }
    else
    {
        PointsProcess(22, 0);
    }
    mCommandPostPoints[iTeam].Inc(1);

    if (NetGame::IsEnabled() && !NetGame::IsClient())
    {
        Character* pCharacter = Character::GetPtr(mStatId);
        if (pCharacter && pCharacter->mPlayerId >= 0)
        {
            NetComm::SetPLCommandPostPoints(pCharacter->mPlayerId, GetCommandPostPoints(iTeam, -1));
        }
    }
}

// FUNCTION: SWBF2 0x0061f050
void PlayerStats::UpdateStatHealth(float fHealth)
{
    if (!mStatsActive || GameLoop::IsGameOver())
    {
        return;
    }

    int iTeam = _GetTeamNumber();
    float fClamped;
    if (fHealth < 0.0f)
    {
        fClamped = 0.0f;
    }
    else if (fHealth > 1.0f)
    {
        fClamped = 1.0f;
    }
    else
    {
        fClamped = fHealth;
    }
    mLastHealth = (unsigned char)(int)(fClamped * 255.0f);

    if (mNarrowEscapeFlag)
    {
        if (fHealth >= 1.0f)
        {
            mNarrowEscapeFlag = false;
            mNarrowEscapes[iTeam].Inc(1);
        }
    }
    else if (fHealth < 0.1f)
    {
        mNarrowEscapeFlag = true;
    }
}

// FUNCTION: SWBF2 0x0061f0d0
void PlayerStats::SetClientMiniStats(unsigned short usKills, unsigned char ucTeamKills,
                                     unsigned char ucDeaths, unsigned short usCommandPostPoints)
{
    int iHero = Character::GetPtr(mStatId)->IsHero();
    mKills[0][iHero].Set(usKills);
    mSameTeamKills[0][iHero].Set(ucTeamKills);
    mDeaths[0][iHero].Set(ucDeaths);
    mCommandPostPoints[0].Set(usCommandPostPoints);
}

// FUNCTION: SWBF2 0x0061f130
int PlayerStats::GetKills(int iTeam, int iHero) const
{
    if (iHero == -1)
    {
        iHero = Character::GetPtr(mStatId)->IsHero();
    }
    if (iTeam < 0)
    {
        return mKills[0][iHero]._val + mKills[1][iHero]._val -
               2 * (mSameTeamKills[0][iHero]._val + mSameTeamKills[1][iHero]._val);
    }
    return mKills[iTeam][iHero]._val - 2 * mSameTeamKills[iTeam][iHero]._val;
}

// FUNCTION: SWBF2 0x0061f1b0
int PlayerStats::GetTeamKills(int iTeam, int iHero) const
{
    if (iHero == -1)
    {
        iHero = Character::GetPtr(mStatId)->IsHero();
    }
    if (iTeam < 0)
    {
        return mSameTeamKills[0][iHero]._val + mSameTeamKills[1][iHero]._val;
    }
    return mSameTeamKills[iTeam][iHero]._val;
}

// FUNCTION: SWBF2 0x0061f220
int PlayerStats::GetEnemyKills(int iTeam, int iHero) const
{
    if (iHero == -1)
    {
        iHero = Character::GetPtr(mStatId)->IsHero();
    }
    if (iTeam < 0)
    {
        return mKills[0][iHero]._val + mKills[1][iHero]._val;
    }
    return mKills[iTeam][iHero]._val;
}

// FUNCTION: SWBF2 0x0061f290
int PlayerStats::GetFlagCaptures(int iTeam, int iHero) const
{
    if (iTeam < 0)
    {
        return mFlagCaps[0][iHero]._val + mFlagCaps[1][iHero]._val;
    }
    return mFlagCaps[iTeam][iHero]._val;
}

// FUNCTION: SWBF2 0x0061f2c0
int PlayerStats::GetDeaths(int iTeam, int iHero) const
{
    if (iHero == -1)
    {
        iHero = Character::GetPtr(mStatId)->IsHero();
    }
    if (iTeam < 0)
    {
        return mDeaths[0][iHero]._val + mDeaths[1][iHero]._val;
    }
    return mDeaths[iTeam][iHero]._val;
}

// FUNCTION: SWBF2 0x0061f330
int PlayerStats::GetCommandPostPoints(int iTeam, int) const
{
    if (iTeam < 0)
    {
        return mCommandPostPoints[0]._val + mCommandPostPoints[1]._val;
    }
    return mCommandPostPoints[iTeam]._val;
}

// FUNCTION: SWBF2 0x0061f360
int PlayerStats::GetCriticalShots(int iTeam, int iHero) const
{
    if (iTeam < 0)
    {
        return mCriticalShots[0][iHero]._val + mCriticalShots[1][iHero]._val;
    }
    return mCriticalShots[iTeam][iHero]._val;
}

// FUNCTION: SWBF2 0x0061fe80
int PlayerStats::_GetPlayerAwardVal(AwardStatT eAward, int iTeam, int) const
{
    const int iHero = Character::GetPtr(mStatId)->IsHero();
    switch (eAward)
    {
    case AWARDSTAT_TANK_BUSTER:    return mVehicleKills[iTeam][iHero]._val;
    case AWARDSTAT_KILLING_SPREE:  return mKillingSpree[iTeam][iHero].GetMax();
    case AWARDSTAT_CAMPER:         return (int)mCampingTime[iTeam].GetMax();
    case AWARDSTAT_SURVIVALIST:    return mKillsUnder25Health[iTeam][iHero]._val;
    case AWARDSTAT_TRAITOR:        return mSameTeamKills[iTeam][iHero]._val;
    case AWARDSTAT_PUBLIC_ENEMY:   return mKillsPerSpawn[iTeam][iHero].GetMax();
    case AWARDSTAT_MEAT:           return mDeaths[iTeam][iHero]._val;
    case AWARDSTAT_DEADEYE:        return mCriticalShots[iTeam][iHero]._val;
    case AWARDSTAT_POINTMAN:       return mCommandPostPoints[iTeam]._val;
    default:                       return 0;
    }
}

// FUNCTION: SWBF2 0x0061fde0
void PlayerStats::_GetPlayerAwardMax(AwardStatT eAward, int& iValue,
                                     int& iCharacter, int& iTeam)
{
    iValue = 0;
    iCharacter = 0;
    iTeam = 0;
    for (int team = 0; team < 2; ++team)
    {
        for (int character = 0; character < Character::sCharacterLimit; ++character)
        {
            Character* pCharacter = Character::GetPtr(character);
            PlayerStats& stats = pCharacter->GetPlayerStats();
            int value = stats._GetPlayerAwardVal(eAward, team, character);
            if (value > iValue)
            {
                iValue = value;
                iCharacter = character;
                iTeam = team;
            }
        }
    }
}

// FUNCTION: SWBF2 0x0061fd70
int PlayerStats::GetAwardVal(AwardStatT eAward)
{
    int value, character, team;
    _GetPlayerAwardMax(eAward, value, character, team);
    return value;
}

// FUNCTION: SWBF2 0x0061fd90
Character* PlayerStats::GetAwardChr(AwardStatT eAward)
{
    int value, character, team;
    _GetPlayerAwardMax(eAward, value, character, team);
    return Character::GetPtr(character);
}

// FUNCTION: SWBF2 0x0061fd30
int PlayerStats::_GetTeamNumber() const
{
    int iTeam = Character::GetPtr(mStatId)->mTeamNumber - 1;
    if (iTeam < 0 || iTeam > 1)
    {
        return 0;
    }
    return iTeam;
}

// FUNCTION: SWBF2 0x00620a10
void PlayerStats::ResetStats()
{
    for (int i = 0; i < 2; ++i)
    {
        mClientExperiencePoints[i] = 0;
        mTimePlayedPerTeam[i].Set(0.0f);
    }
    mCampingPos = PblVector3(0.0f, 0.0f, 0.0f);
    mLastHealth = 0;
    mNarrowEscapeFlag = false;
    mLastKillTime = 0.0f;
    mConsecutiveKillCnt = 0;
    mRankOrder = 0;
    mPauseCount = 0;
    mPlayerPoints = 0;
    mHeroPoints = 0;
    mPointsDelta = 0;
}

// FUNCTION: SWBF2 0x0061f380
bool PlayerStats::IsClassSelectable(int iClassIndex, UnselectableReason* pReason)
{
    Team* pTeam = Team::sTeams[Character::GetPtr(mStatId)->mTeamNumber];
    GameObjectClass* pClass;
    if (!pTeam || !(pClass = pTeam->GetClassPtr(iClassIndex)) ||
        (GetPoints(true) < pClass->mPointsToUnlock && pClass->mPointsToUnlock))
    {
        if (pReason)
        {
            *pReason = NOT_ENOUGH_POINTS;
        }
        return false;
    }

    if (pClass->mPointsToUnlock <= 0 ||
        pTeam->GetClassCurrentUnits(iClassIndex) < pTeam->GetClassMaxUnits(iClassIndex))
    {
        if (!pClass->IsRtti(EntityDroidekaClass::rttiHashEntityDroidekaClass))
        {
            return true;
        }
        MemoryPool* pPool = MemoryPool::Find("EntityDroideka");
        if (pPool && pPool->mCount != pPool->mUsed)
        {
            return true;
        }
    }

    if (pReason)
    {
        *pReason = TOO_MANY_UNITS;
    }
    return false;
}

namespace PopupText { extern bool gPopupShownLatch; }

// FUNCTION: SWBF2 0x006210f0
void ClearFireTriggerBlock()
{
    PopupText::gPopupShownLatch = false;
}

// FUNCTION: SWBF2 0x00621100
bool IsFireTriggerBlocked()
{
    return PopupText::gPopupShownLatch;
}

// FUNCTION: SWBF2 0x0061fc30
void PlayerStats::AddGameStatsToProfile()
{
    for (int side = 0; side < 2; ++side)
    {
        Team* team = Team::sTeams[side + 1];
        const int count = team->mMemberCount;
        for (int i = 0; i < count; ++i)
        {
            Character* character = team->GetMemberPtr(i);
            if (!character || !character->GetProfile())
            {
                continue;
            }
            PlayerStats& stats = character->GetPlayerStats();
            PlayerProfile::tProfile* profile = character->GetProfile();
            profile->mAccumPoints.mTotalDeaths += stats.GetDeaths(side, 1) + stats.GetDeaths(side, 0);
            profile->mAccumPoints.mTotalKills += stats.GetKills(side, 1) + stats.GetKills(side, 0);
            profile->mAccumPoints.mTotalPoints += stats.GetPoints(true) + stats.GetPoints(false);
            profile->UpdateProfCRC();
        }
    }
}

namespace
{
unsigned int ReadStatBits(NetPktGroup& group, int iBits)
{
    unsigned int uValue = 0;
    group.ReadBits(uValue, iBits);
    return uValue;
}
}

// FUNCTION: SWBF2 0x00620570
void PlayerStats::Read(NetPktGroup& group)
{
    const unsigned int iPlayer = ReadStatBits(group, 8);
    const unsigned char uPlayerKill = static_cast<unsigned char>(ReadStatBits(group, 8));
    mKillsByPlayer[0][0][iPlayer].Set(uPlayerKill);
    mKillsByPlayer[0][1][iPlayer].Set(uPlayerKill);

    const unsigned int iOther = ReadStatBits(group, 8);
    const unsigned char uOtherKill = static_cast<unsigned char>(ReadStatBits(group, 8));
    Character* pOther = Character::GetPtr(iOther);
    if (!pOther)
    {
        return;
    }
    PlayerStats& other = pOther->GetPlayerStats();
    other.mKillsByPlayer[0][0][mStatId].Set(uOtherKill);
    other.mKillsByPlayer[0][1][mStatId].Set(uOtherKill);

    for (int iSide = 0; iSide < 2; ++iSide)
    {
        for (int iHero = 0; iHero < 2; ++iHero)
        {
            mKills[iSide][iHero].Set(static_cast<unsigned short>(ReadStatBits(group, 16)));
            mSameTeamKills[iSide][iHero].Set(static_cast<unsigned char>(ReadStatBits(group, 8)));
            mVehicleKills[iSide][iHero].Set(static_cast<unsigned char>(ReadStatBits(group, 8)));
            mKillingSpree[iSide][iHero].SetMax(static_cast<unsigned char>(ReadStatBits(group, 8)));
            mKillsUnder25Health[iSide][iHero].Set(static_cast<unsigned char>(ReadStatBits(group, 8)));
            mKillsPerSpawn[iSide][iHero].ResetCurrent();
            mKillsPerSpawn[iSide][iHero].SetMax(static_cast<unsigned char>(ReadStatBits(group, 8)));
            mFlagCaps[iSide][iHero].Set(static_cast<unsigned char>(ReadStatBits(group, 8)));
            mShotsFired[iSide][iHero].Set(static_cast<unsigned short>(ReadStatBits(group, 16)));
            mShotsHit[iSide][iHero].Set(static_cast<unsigned short>(ReadStatBits(group, 16)));
            mCriticalShots[iSide][iHero].Set(static_cast<unsigned short>(ReadStatBits(group, 16)));
            mDeaths[iSide][iHero].Set(static_cast<unsigned char>(ReadStatBits(group, 8)));
        }
        mMaxAliveTime[iSide].ResetCurrent();
        mMaxAliveTime[iSide].SetMax(static_cast<float>(ReadStatBits(group, 32)));
        mCampingTime[iSide].ResetCurrent();
        mCampingTime[iSide].SetMax(static_cast<float>(ReadStatBits(group, 32)));
        mTimePlayedPerTeam[iSide].Set(static_cast<float>(ReadStatBits(group, 32)));
        mCommandPostPoints[iSide].Set(static_cast<unsigned short>(ReadStatBits(group, 16)));
        mNarrowEscapes[iSide].Set(static_cast<unsigned char>(ReadStatBits(group, 8)));
        mBattlePoints[iSide].Set(static_cast<unsigned short>(ReadStatBits(group, 16)));
        mClientExperiencePoints[iSide] = static_cast<int>(ReadStatBits(group, 32));
    }

    mTimePlayedPerWeaponList[0][0] = Factory<Weapon, WeaponClass, WeaponDesc>::Find(ReadStatBits(group, 32));
    mTimePlayedPerWeapon[0][0].Set(static_cast<float>(ReadStatBits(group, 16)));
    mTimeInsideVehicleList[0][0] = Factory<Entity, EntityClass, EntityDesc>::Find(ReadStatBits(group, 32));
    mTimeInsideVehicle[0][0].Set(static_cast<float>(ReadStatBits(group, 16)));
    mTimePlayedPerClassList[0][0] = Factory<Entity, EntityClass, EntityDesc>::Find(ReadStatBits(group, 32));
    mTimePlayedPerClass[0][0].Set(static_cast<float>(ReadStatBits(group, 16)));
    mPlayerPoints = static_cast<short>(ReadStatBits(group, 16));
    mHeroPoints = static_cast<short>(ReadStatBits(group, 16));
}

// FUNCTION: SWBF2 0x00620020
void PlayerStats::ReadAll(NetPktGroup& group)
{
    mGotMPStats = true;
    mStatsActive = false;
    const int iCount = Character::sCharacterLimit;
    if (NetGame::IsHost())
    {
        PlayerStats temporary;
        temporary.Init(0, iCount);
        mGotMPStats = true;
        mStatsActive = false;
        for (int i = 0; i < iCount; ++i)
        {
            if (ReadStatBits(group, 1))
            {
                temporary.Reset();
                temporary.Read(group);
            }
        }
    }
    else
    {
        for (int i = 0; i < iCount; ++i)
        {
            Character* pCharacter = Character::GetPtr(i);
            if (pCharacter)
            {
                pCharacter->GetPlayerStats().Reset();
            }
        }
        for (int i = 0; i < iCount; ++i)
        {
            if (ReadStatBits(group, 1))
            {
                Character* pCharacter = Character::GetPtr(i);
                pCharacter->GetPlayerStats().Read(group);
            }
        }
    }
}

// FUNCTION: SWBF2 0x0061ffa0
void PlayerStats::WriteAll(NetPktGroup& group)
{
    for (int i = 0; i < Character::sCharacterLimit; ++i)
    {
        Character* pCharacter = Character::GetPtr(i);
        PlayerStats* pStats = pCharacter ? &pCharacter->GetPlayerStats() : 0;
        if (pStats)
        {
            group.WriteBits(1, 1);
            pStats->Write(group);
        }
        else
        {
            group.WriteBits(0, 1);
        }
    }
    mStatsActive = false;
}

// FUNCTION: SWBF2 0x006201c0
void PlayerStats::Write(NetPktGroup& group)
{
    const int iBaitTeam = GetBaitVal(0, 0) > GetBaitVal(0, 1) ? 0 : 1;
    Character* pBait = GetBait(0, iBaitTeam);
    group.WriteBits((reinterpret_cast<int>(pBait) - reinterpret_cast<int>(Character::sCharacters)) / static_cast<int>(sizeof(Character)), 8);
    group.WriteBits(GetBaitVal(0, iBaitTeam), 8);

    const int iNemesisTeam = GetNemesisVal(0, 0) > GetNemesisVal(0, 1) ? 0 : 1;
    Character* pNemesis = GetNemesis(0, iNemesisTeam);
    group.WriteBits((reinterpret_cast<int>(pNemesis) - reinterpret_cast<int>(Character::sCharacters)) / static_cast<int>(sizeof(Character)), 8);
    group.WriteBits(GetNemesisVal(0, iNemesisTeam), 8);

    const int iExperience = GetExperiencePoints(0, 0);
    for (int iSide = 0; iSide < 2; ++iSide)
    {
        for (int iHero = 0; iHero < 2; ++iHero)
        {
            group.WriteBits(mKills[iSide][iHero]._val, 16);
            group.WriteBits(mSameTeamKills[iSide][iHero]._val, 8);
            group.WriteBits(mVehicleKills[iSide][iHero]._val, 8);
            group.WriteBits(mKillingSpree[iSide][iHero]._max, 8);
            group.WriteBits(mKillsUnder25Health[iSide][iHero]._val, 8);
            group.WriteBits(mKillsPerSpawn[iSide][iHero]._max, 8);
            group.WriteBits(mFlagCaps[iSide][iHero]._val, 8);
            group.WriteBits(mShotsFired[iSide][iHero]._val, 16);
            group.WriteBits(mShotsHit[iSide][iHero]._val, 16);
            group.WriteBits(mCriticalShots[iSide][iHero]._val, 16);
            group.WriteBits(mDeaths[iSide][iHero]._val, 8);
        }
        group.WriteBits(static_cast<unsigned int>(mMaxAliveTime[iSide]._max), 32);
        group.WriteBits(static_cast<unsigned int>(mCampingTime[iSide]._max), 32);
        group.WriteBits(static_cast<unsigned int>(mTimePlayedPerTeam[iSide]._val), 32);
        group.WriteBits(mCommandPostPoints[iSide]._val, 16);
        group.WriteBits(mNarrowEscapes[iSide]._val, 8);
        group.WriteBits(mBattlePoints[iSide]._val, 16);
        group.WriteBits(iExperience, 32);
    }

    const WeaponClass* pWeapon = GetFavoriteWeaponClass(0);
    group.WriteBits(pWeapon ? pWeapon->mId : 0, 32);
    group.WriteBits(static_cast<unsigned int>(GetFavoriteWeaponTime(0)), 16);
    const EntityClass* pVehicle = GetFavoriteVehicleClass(0);
    group.WriteBits(pVehicle ? pVehicle->mId : 0, 32);
    group.WriteBits(static_cast<unsigned int>(GetFavoriteVehicleTime(0)), 16);
    const EntityClass* pClass = GetFavoriteSoldierClass(0);
    group.WriteBits(pClass ? pClass->mId : 0, 32);
    group.WriteBits(static_cast<unsigned int>(GetFavoriteSoldierTime(0)), 16);
    group.WriteBits(mPlayerPoints, 16);
    group.WriteBits(mHeroPoints, 16);
}

// FUNCTION: SWBF2 0x00620a00
void PlayerStats::SetPointsTable(int iType, int iPoints)
{
    if (iType < 35)
    {
        PlayerStatsPoints[iType] = (char)iPoints;
    }
}
