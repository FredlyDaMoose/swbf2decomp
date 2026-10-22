#pragma once

#include <stddef.h>

#include "Damageable.h"

class Character;
class NetPktGroup;
class RedColor;

struct PointCondition
{
    short pointsPerMedal;
    bool kill;
    bool critHit;
    short healthType;
    bool hack;
    bool score;
    char* name;
};
 

class MedalsMgr
{
public:

    enum MedalTypes
    {
        GUNSLINGER = 0,
        FRENZY = 1,
        DEMOLITION = 2,
        TECHNICIAN = 3,
        MARKSMAN = 4,
        REGULATOR = 5,
        ENDURANCE = 6,
        GUARDIAN = 7,
        WAR_HERO = 8,
        MEDAL_TYPES_COUNT = 9
    };

    enum CareerLevel
    {
        GREEN = 0,
        VETERAN = 1,
        ELITE = 2,
        LEGENDARY = 3,
        CAREER_LEVEL_COUNT = 4
    };

private:

    void HudIndication(int iType, const char* pPrefix, const RedColor& color, bool) const;

    bool IsAwardAvailableInternal(int iType) const;

    short GetPointsNeededForMedal(int iType) const;

public:

    void AddMedalPoints(int iType, bool bKill, bool bCritHit, HealthType eHealthType, bool bHack,
                        bool bScore, short sPoints);

    MedalsMgr(Character* pOwner);
    void Init();
    void LoadMedals();

    void SetMedals(int iType, short sMedals);

    short GetMedals(int iType) const;

    short GetMedalsThisGame(int iType) const;

    static int GetMedalLevel(int iCount);
    static int GetRankLevel(int iCount);
    CareerLevel GetLevel(int iType) const;
    void Update(float fDeltaTime);
    bool IsAwardAvailable(int iType) const;

    void SaveMedals();

    void WriteObject(NetPktGroup& group);
    void ReadObject(NetPktGroup& group);

    int GetRank() const;

    void ClearAllMedalPoints();

    Character* mOwner;
    short mMedalsGathered[MEDAL_TYPES_COUNT];
    short mMedalPoints[MEDAL_TYPES_COUNT];
    float mAwardTime[MEDAL_TYPES_COUNT];
    unsigned short mAwardAvailable;
};
 
 
