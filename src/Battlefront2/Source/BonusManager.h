#pragma once

#include <stddef.h>

#include "Thread.h"

class CommandPost;
class EntityDefenseGridTurret;
class PblMatrix;

// VTABLE: SWBF2 0x00797098
class BonusManager : public Thread
{
public:
     

    struct CommandPostData
    {
        CommandPost* pCommandPost;
        EntityDefenseGridTurret** ppTurrets;
    };

    BonusManager();
    virtual ~BonusManager();

    static bool Create();
    static bool Destroy();

    void InitialUpdate();
    virtual bool Update(float fDeltaTime);

    void ClearTeamBonuses();
    void SetTeamBonusActive(int iTeam, unsigned int uBonus, bool bActive);

    static BonusManager* Instance() { return &sInstance; }
    bool IsTeamBonusActive(int iTeam, unsigned int uBonus);

    void ClearIndividualBonuses();
    void SetIndividualBonusActive(int iPlayer, unsigned int uBonus, bool bActive);
    bool IsIndividualBonusActive(int iPlayer, unsigned int uBonus);

private:
    static void Reset();

    int GetTeamIndex(int iTeam);

    void UpdateTeamBonuses(float fDeltaTime);
    void UpdateIndividualBonuses(float fDeltaTime);

    void UpdateTeamBonusBactaTanks(int iTeam, float fDeltaTime);
    void UpdateTeamBonusLeader(int iTeam, float fDeltaTime);
    void UpdateTeamBonusGarrison(int iTeam, float fDeltaTime);

    void InitialUpdateTeamBonusAutoturrets();
    void UpdateTeamBonusAutoturrets(int iTeam, float fDeltaTime);
    void CleanupTeamBonusAutoturrets();
    void AddTurretsToCP(CommandPostData& data, int iTeam);
    void RemoveTurretsFromCP(CommandPostData& data);
    EntityDefenseGridTurret* MakeNewAutoturret(const PblMatrix& matrix, int iTeam);

    void PlayTeamBonusVO(unsigned int uBonus, int iTeam);

public:
    unsigned int mTeamBonuses[2];
    unsigned int mIndividualBonuses[600];
    int mNumCPs;
    bool mbIsBonusAutoturretsInitialized;
    CommandPostData* mpCommandPostData;
    float mUnlockHeroTimer[2];
    bool mbDidUseGarrison[2];
    float mVOStartTimer;

    static const float sTeamBonusSuppliesMultiplier;
    static const float sTeamBonusEnergyBoostMultiplier;
    static const float sTeamBonusSabotageDamagePercentage;
    static const float sTeamBonusCombatShieldingPercentage;
    static const float sTeamBonusLeaderUnlockTime;
    static const float sTeamBonusGarrisonThreshold;
    static const float sTeamBonusGarrisonExtraReinforcements;

    static BonusManager sInstance;
};
 
 
 
 
 
 
 
 
 
 
