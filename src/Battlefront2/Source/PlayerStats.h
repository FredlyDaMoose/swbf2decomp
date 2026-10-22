#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblVector.h"

class Character;
class Entity;
class EntityClass;
class GameObject;
class WeaponClass;
class NetPktGroup;

enum AwardStatT
{
    AWARDSTAT_TANK_BUSTER = 0,
    AWARDSTAT_KILLING_SPREE = 1,
    AWARDSTAT_CAMPER = 2,
    AWARDSTAT_SURVIVALIST = 3,
    AWARDSTAT_TRAITOR = 4,
    AWARDSTAT_PUBLIC_ENEMY = 5,
    AWARDSTAT_MEAT = 6,
    AWARDSTAT_DEADEYE = 7,
    AWARDSTAT_POINTMAN = 8,
    AWARDSTAT_COUNT = 9
};

template <class T>
class CntStat
{
public:
    CntStat() : _val(0) {}
    void Inc(T val) { _val += val; }
    void Set(T val) { _val = val; }

    T _val;
};

template <class T>
class MaxCntStat
{
public:
    MaxCntStat() : _val(0), _max(0) {}
    void Inc(T val)
    {
        _val += val;
        _max = _val > _max ? _val : _max;
    }
    T GetCurrent() const { return _val; }
    T GetMax() const { return _max; }
    void ResetCurrent() { _val = 0; }
    void SetMax(T val) { _max = val; }

    T _val;
    T _max;
};

template <class T>
class MaxStat
{
public:
    MaxStat() : _max(0) {}

    void Add(T val) { _max = val > _max ? val : _max; }
    T GetMax() const { return _max; }
    void SetMax(T val) { _max = val; }

    T _max;
};

class PlayerStats
{
public:

    static void AddGameStatsToProfile();

    enum UnselectableReason
    {
        TOO_MANY_UNITS = 0,
        NOT_ENOUGH_POINTS = 1
    };

    static const int MAX_STAT_WEAPONS = 10;
    static const int MAX_STAT_VEHICLES = 5;
    static const int MAX_STAT_CLASSES = 5;
    static const int CAMPING_DIST_SQ = 100;

    PlayerStats();
    ~PlayerStats();

    void Init(int iStatId, int iNumPlayers);
    void Reset();
    void ResetStats();
    void Read(NetPktGroup& group);

    int PointsProcess(unsigned char ucType, unsigned int uParam);

    static void PointsProcessTeam(int iTeam, unsigned char ucType);
    int GetPoints() const;
    int GetPoints(bool bPlayer) const;

    bool IsClassSelectable(int iClassIndex, UnselectableReason* pReason);

    void TimingPause()
    {
        if (!mPauseCount)
        {
            mPauseCount = 1;
        }
    }
    void TimingUnpause()
    {
        if (mPauseCount)
        {
            --mPauseCount;
        }
    }
    void SetPoints(short sPoints);
    void SetPoints(bool bPlayer, short sPoints);

    void AddAllyKill(Character* pVictim, GameObject* pObject);

    void AddKill(Character* pVictim, GameObject* pObject);
    void AddVehicleKill(int iVehicleType);
    void AddFlagCapture();
    void AddShot();
    void AddHit();
    void AddCriticalShot();
    void AddDeath(Character* pKiller);
    void AddTimePlayedWithWeapon(const WeaponClass* pWeaponClass, float fTime);
    void AddTimePlayedInsideVehicle(const Entity* pVehicle, float fTime);
    void AddTimePlayedAsClass(const EntityClass* pClass, float fTime);
    void AddCampingTime(PblVector3 vPos, float fTime);
    void AddCommandPostPoints(unsigned short usPoints, bool bCapture);
    void UpdateStatHealth(float fHealth);
    void SetClientMiniStats(unsigned short usKills, unsigned char ucTeamKills,
                            unsigned char ucDeaths, unsigned short usCommandPostPoints);

    int GetKills(int iTeam, int iHero) const;
    int GetTeamKills(int iTeam, int iHero) const;
    int GetEnemyKills(int iTeam, int iHero) const;
    int GetFlagCaptures(int iTeam, int iHero) const;
    int GetDeaths(int iTeam, int iHero) const;
    int GetCommandPostPoints(int iTeam, int iHero) const;
    int GetCriticalShots(int iTeam, int iHero) const;
    const EntityClass* GetFavoriteSoldierClass(int iTeam) const;
    float GetFavoriteSoldierTime(int iTeam) const;
    const WeaponClass* GetFavoriteWeaponClass(int iTeam) const;
    float GetFavoriteWeaponTime(int iTeam) const;
    const EntityClass* GetFavoriteVehicleClass(int iTeam) const;
    float GetFavoriteVehicleTime(int iTeam) const;
    int GetExperiencePoints(int iTeam, int iHero) const;
    void Write(NetPktGroup& group);
    Character* GetNemesis(int iStat, int iTeam) const;
    int GetNemesisVal(int iStat, int iTeam) const;
    Character* GetBait(int iStat, int iTeam) const;
    int GetBaitVal(int iStat, int iTeam) const;

    static int GetAwardVal(AwardStatT eAward);
    static Character* GetAwardChr(AwardStatT eAward);

    static bool mStatsActive;
    static bool mGotMPStats;

    static void ReadAll(NetPktGroup& group);
    static void WriteAll(NetPktGroup& group);

    static void SetPointsTable(int iType, int iPoints);

private:
    int _GetTeamNumber() const;
    static void _GetPlayerAwardMax(AwardStatT eAward, int& iValue,
                                   int& iCharacter, int& iTeam);
    int _GetPlayerAwardVal(AwardStatT eAward, int iTeam, int iStat) const;

public:
    int mRankOrder;
    CntStat<unsigned short> mKills[2][2];
    CntStat<unsigned char> mSameTeamKills[2][2];
    CntStat<unsigned char> mVehicleKills[2][2];
    CntStat<unsigned char> mFlagCaps[2][2];
    CntStat<unsigned char>* mKillsByPlayer[2][2];
    int mKillsByPlayerSize;
    MaxStat<unsigned char> mKillingSpree[2][2];
    float mLastKillTime;
    unsigned char mConsecutiveKillCnt;
    CntStat<unsigned char> mKillsUnder25Health[2][2];
    MaxCntStat<unsigned char> mKillsPerSpawn[2][2];
    CntStat<unsigned short> mShotsFired[2][2];
    CntStat<unsigned short> mShotsHit[2][2];
    CntStat<unsigned short> mCriticalShots[2][2];
    CntStat<unsigned char> mDeaths[2][2];
    char mPauseCount;
    const WeaponClass* mTimePlayedPerWeaponList[2][MAX_STAT_WEAPONS];
    CntStat<float> mTimePlayedPerWeapon[2][MAX_STAT_WEAPONS];
    const EntityClass* mTimeInsideVehicleList[2][MAX_STAT_VEHICLES];
    CntStat<float> mTimeInsideVehicle[2][MAX_STAT_VEHICLES];
    const EntityClass* mTimePlayedPerClassList[2][MAX_STAT_CLASSES];
    CntStat<float> mTimePlayedPerClass[2][MAX_STAT_CLASSES];
    CntStat<float> mTimePlayedPerTeam[2];
    MaxCntStat<float> mMaxAliveTime[2];
    PblVector3 mCampingPos;
    MaxCntStat<float> mCampingTime[2];
    CntStat<unsigned short> mCommandPostPoints[2];
    unsigned char mLastHealth;
    bool mNarrowEscapeFlag;
    CntStat<unsigned char> mNarrowEscapes[2];
    CntStat<unsigned short> mBattlePoints[2];
    unsigned char mStatId;
    float mNextNetAuthTime;
    int mClientExperiencePoints[2];
    short mPlayerPoints;
    short mHeroPoints;
    short mPointsDelta;
};

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
