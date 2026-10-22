#pragma once

class UnitController;

namespace AIDifficulty
{
    void Init();
    int GetProfileDifficulty();

    void SetAILuaModifier(int iPlayer, int iEnemy, int iDifficulty);

    int GetIntSkill(const UnitController* pController, bool bClamp);

    float GetCoverPercentage();
    float GetBarrageCountMultiplier(const UnitController* pController);
    float GetBarrageWaitMultiplier(const UnitController* pController);
    float GetTriggerSingleWaitDelayMultiplier(const UnitController* pController);
    float GetChargeWeaponBot(const UnitController* pController);
    float GetChargeWeaponTop(const UnitController* pController);
    float GetCrowdGrenadeThrowPercentage(const UnitController* pController);
    float GetPlayerThreatAngle();
    float GetTargetBubbleShrinkRate(bool bPlayer);
    float GetMinBubbleSize(bool bPlayer);
    float GetMaxBubbleSize(bool bPlayer);
    float GetMaxBubbleSize_Flyer(bool bPlayer);
    float GetAimerMaxAngAcc(const UnitController* pController);
    float GetAimerDamping(const UnitController* pController);
    float GetAimerSpread(const UnitController* pController);
    float GetAimerYawSpread(const UnitController* pController);

    float SpaceAssault_GetRandomFlyerKillTime();

    float GetGrenadeReactionTime();
    float SpaceAssault_GetRandomCritSysDamageTime();
    void EnableAutoBalance(bool bEnable);

    float GetFlyerTurnAndAttackFrequency();
    float GetFlyerEvasiveFrequency();
    float GetFlyerTrickOnDamage();
    float GetFlyerFreeFlyOnDamage();
    float GetFlyerReactionTime();
    float GetRangedSpeed(float fSkill, float fMin, float fMid, float fMax, float fBoost);
    void GetFlyerSpeedRange(const UnitController* pController, float fMin, float fMid, float fMax, float fBoost,
                            float& rLow, float& rHigh);

    extern int mProfileDifficulty;
    extern int mScriptPlayerModifier[3];
    extern int mScriptEnemyModifier[3];
    extern int mAutoBalancePlayerModifier;
    extern int mAutoBalanceEnemyModifier;
    extern bool mAutoBalanceEnabled;
    extern bool gAutoBalanceEnabled;
    extern float mNextFlyerKillTime;
    extern float mNextCritSysDamageTime;
    extern float mCritSysBias;
}
