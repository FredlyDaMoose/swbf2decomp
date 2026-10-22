#include "AIDifficulty.h"

#include "AIUtil.h"
#include "Controllable.h"
#include "EventInfo.h"
#include "GameObject.h"
#include "NetGame.h"
#include "PlayerProfile.h"
#include "UnitController.h"

namespace AIDifficulty
{
// GLOBAL: SWBF2 0x007887a0
static const int sDifficultyPlayer[3] = { 6, 11, 11 };

// GLOBAL: SWBF2 0x007887ac
static const int sDifficultyEnemy[3] = { 4, 14, 17 };

// GLOBAL: SWBF2 0x01e2fda4
int mProfileDifficulty;

// GLOBAL: SWBF2 0x01e2fd90
int mScriptPlayerModifier[3];

// GLOBAL: SWBF2 0x01e2fd7c
int mScriptEnemyModifier[3];

// GLOBAL: SWBF2 0x01e2fda8
int mAutoBalancePlayerModifier;

// GLOBAL: SWBF2 0x01e2fda0
int mAutoBalanceEnemyModifier;

// GLOBAL: SWBF2 0x007e5886
bool mAutoBalanceEnabled = true;

// GLOBAL: SWBF2 0x007e5887
bool gAutoBalanceEnabled = true;

// GLOBAL: SWBF2 0x01e2fd88
float mNextFlyerKillTime;

// GLOBAL: SWBF2 0x01e2fd9c
float mNextCritSysDamageTime;

// GLOBAL: SWBF2 0x01e2fd8c
float mCritSysBias;

static float GetFloatSkill_RangedLinear(const UnitController* pController, float fMin,
                                        float fMax, bool bClamp);

static void SpaceAssault_Init()
{
    mNextFlyerKillTime = SpaceAssault_GetRandomFlyerKillTime();
    mNextCritSysDamageTime = SpaceAssault_GetRandomCritSysDamageTime();
    mCritSysBias = AIUtil::GetRandomFloat(0.0f, 1.0f);
}
}

// FUNCTION: SWBF2 0x00438bd0
void AIDifficulty::Init()
{
    if (NetGame::IsEnabled())
    {
        mProfileDifficulty = PlayerPrefs::mAllUserPrefs[0].mDifficulty;
    }
    else
    {
        mProfileDifficulty = PlayerProfile::GetProfile(0)->mGameOptions.mCurDiffLevel;
    }

    for (int i = 0; i < 3; ++i)
    {
        mScriptPlayerModifier[i] = 0;
        mScriptEnemyModifier[i] = 0;
    }
    EnableAutoBalance(true);
    SpaceAssault_Init();
}

// FUNCTION: SWBF2 0x00438c70
int AIDifficulty::GetProfileDifficulty()
{
    return mProfileDifficulty;
}

// FUNCTION: SWBF2 0x00438c80
int AIDifficulty::GetIntSkill(const UnitController* pController, bool bClamp)
{
    bool bPlayer = false;
    if (pController == (const UnitController*)1)
    {
        bPlayer = true;
    }
    else if (pController)
    {
        const Controllable* pMe = pController->GetMe();
        if (pMe && pMe->GetGameObject())
        {
            bPlayer = AIUtil::IsPlayerTeam(pMe->GetGameObject()->mTeam);
        }
    }

    int iDifficulty = GetProfileDifficulty();
    int iSkill;
    if (bPlayer)
    {
        iSkill = sDifficultyPlayer[iDifficulty - 1] + mScriptPlayerModifier[iDifficulty - 1] +
                 mAutoBalancePlayerModifier;
    }
    else
    {
        iSkill = sDifficultyEnemy[iDifficulty - 1] + mScriptEnemyModifier[iDifficulty - 1] +
                 mAutoBalanceEnemyModifier;
    }

    int iModifier = 0;
    if (pController > (const UnitController*)1)
    {
        AI::EventInfo info(AI::EVQ_GetSkillModifier);
        if (pController->SendEvent(&info))
        {
            iModifier = info.mInt;
        }
    }

    iSkill += iModifier;
    if (iSkill < 1)
    {
        iSkill = 1;
    }
    if (bClamp && iSkill > 20)
    {
        iSkill = 20;
    }
    return iSkill;
}

namespace AIDifficulty
{
// FUNCTION: SWBF2 0x00438d60
static float GetFloatSkill(const UnitController* pController, bool bClamp)
{
    return (float)(GetIntSkill(pController, bClamp) - 1) / 19.0f;
}

// FUNCTION: SWBF2 0x00438d80
static float GetFloatSkill_RangedLinear(const UnitController* pController, float fMin,
                                        float fMax, bool bClamp)
{
    return GetFloatSkill(pController, bClamp) * (fMax - fMin) + fMin;
}
}

// FUNCTION: SWBF2 0x00438df0
float AIDifficulty::GetCoverPercentage()
{
    return 1.0f;
}

// FUNCTION: SWBF2 0x00438e00
float AIDifficulty::GetBarrageCountMultiplier(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 0.0f, 1.0f, false);
}

// FUNCTION: SWBF2 0x00438e20
float AIDifficulty::GetBarrageWaitMultiplier(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 0.1f, 0.9f, false);
}

// FUNCTION: SWBF2 0x00438e40
float AIDifficulty::GetTriggerSingleWaitDelayMultiplier(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 0.1f, 0.9f, false);
}

// FUNCTION: SWBF2 0x00438e60
float AIDifficulty::GetChargeWeaponBot(const UnitController* pController)
{
    return 0.15f;
}

// FUNCTION: SWBF2 0x00438e70
float AIDifficulty::GetChargeWeaponTop(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 0.0f, 1.0f, false);
}

// FUNCTION: SWBF2 0x00438e90
float AIDifficulty::GetCrowdGrenadeThrowPercentage(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 0.0f, 1.0f, true);
}

// FUNCTION: SWBF2 0x00438eb0
float AIDifficulty::GetPlayerThreatAngle()
{
    return GetFloatSkill_RangedLinear(0, 0.0f, 0.7f, true);
}

// FUNCTION: SWBF2 0x00438ed0
float AIDifficulty::GetTargetBubbleShrinkRate(bool bPlayer)
{
    return GetFloatSkill_RangedLinear((const UnitController*)bPlayer, -0.1f, -1.1f, false);
}

// FUNCTION: SWBF2 0x00438ef0
float AIDifficulty::GetMinBubbleSize(bool bPlayer)
{
    float fSize = GetFloatSkill_RangedLinear((const UnitController*)bPlayer, 1.6f, 0.4f, false);
    if (fSize < 0.0f)
    {
        fSize = 0.0f;
    }
    return fSize;
}

// FUNCTION: SWBF2 0x00438f20
float AIDifficulty::GetMaxBubbleSize(bool bPlayer)
{
    float fSize = GetFloatSkill_RangedLinear((const UnitController*)bPlayer, 2.2f, 1.2f, false);
    if (fSize < 0.0f)
    {
        fSize = 0.0f;
    }
    return fSize;
}

// FUNCTION: SWBF2 0x00438f50
float AIDifficulty::GetMaxBubbleSize_Flyer(bool bPlayer)
{
    float fSize = GetFloatSkill_RangedLinear((const UnitController*)bPlayer, 4.0f, 1.2f, false);
    if (fSize < 0.0f)
    {
        fSize = 0.0f;
    }
    return fSize;
}

// FUNCTION: SWBF2 0x00439220
float AIDifficulty::SpaceAssault_GetRandomFlyerKillTime()
{
    float fMin = GetFloatSkill_RangedLinear(0, 45.0f, 15.0f, true);
    float fMax = GetFloatSkill_RangedLinear(0, 100.0f, 60.0f, true);
    return AIUtil::GetRandomFloat(fMin, fMin > fMax ? fMin : fMax);
}

// FUNCTION: SWBF2 0x00439280
float AIDifficulty::SpaceAssault_GetRandomCritSysDamageTime()
{
    float fMin = GetFloatSkill_RangedLinear(0, 25.0f, 10.0f, true);
    float fMax = GetFloatSkill_RangedLinear(0, 45.0f, 12.0f, true);
    return AIUtil::GetRandomFloat(fMin, fMin > fMax ? fMin : fMax);
}

// FUNCTION: SWBF2 0x00439310
void AIDifficulty::EnableAutoBalance(bool bEnable)
{
    mAutoBalanceEnabled = bEnable;
}

namespace AIDifficulty
{

// FUNCTION: SWBF2 0x00438db0
static float GetFloatSkill_RangedCubed(const UnitController* pController, float fMin, float fMax, bool bClamp)
{
    float fSkill = GetFloatSkill(pController, bClamp);
    return (fMax - fMin) * (fSkill * fSkill * fSkill) + fMin;
}
}

// FUNCTION: SWBF2 0x00438f80
float AIDifficulty::GetAimerMaxAngAcc(const UnitController* pController)
{
    return AIDifficulty::GetFloatSkill_RangedCubed(pController, 0.8f, 14.0f, false);
}

// FUNCTION: SWBF2 0x00438fa0
float AIDifficulty::GetAimerDamping(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 0.8f, 2.4f, false);
}

// FUNCTION: SWBF2 0x00438fc0
float AIDifficulty::GetAimerSpread(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 3.0f, 1.0f, true);
}

// FUNCTION: SWBF2 0x00438fe0
float AIDifficulty::GetAimerYawSpread(const UnitController* pController)
{
    return GetFloatSkill_RangedLinear(pController, 2.0f, 0.0f, true);
}

// FUNCTION: SWBF2 0x00439000
float AIDifficulty::GetGrenadeReactionTime()
{
    return GetFloatSkill_RangedLinear(0, 2.0f, 0.0f, true);
}

// FUNCTION: SWBF2 0x00439020
float AIDifficulty::GetFlyerTurnAndAttackFrequency()
{
    return GetFloatSkill_RangedLinear(0, 0.0f, 0.0f, true);
}

// FUNCTION: SWBF2 0x00439040
float AIDifficulty::GetFlyerEvasiveFrequency()
{
    return GetFloatSkill_RangedLinear(0, 0.0f, 0.0f, true);
}

// FUNCTION: SWBF2 0x00439060
float AIDifficulty::GetFlyerTrickOnDamage()
{
    return GetFloatSkill_RangedLinear(0, 0.0f, 1.5f, true);
}

// FUNCTION: SWBF2 0x00439080
float AIDifficulty::GetFlyerFreeFlyOnDamage()
{
    return GetFloatSkill_RangedLinear(0, 0.0f, 0.2f, true);
}

// FUNCTION: SWBF2 0x004390a0
float AIDifficulty::GetFlyerReactionTime()
{
    return GetFloatSkill_RangedLinear(0, 0.5f, 1.5f, true);
}

// FUNCTION: SWBF2 0x004390c0
float AIDifficulty::GetRangedSpeed(float fSkill, float fMin, float fMid, float fMax, float fBoost)
{
    if (fSkill < 0.0f)
    {
        fSkill = 0.0f;
    }
    else if (fSkill > 1.5f)
    {
        fSkill = 1.5f;
    }

    if (fSkill < 0.5f)
    {
        return (fMid - fMin) * (fSkill * 2.0f) + fMin;
    }
    if (fSkill < 1.0f)
    {
        return (fMax - fMid) * ((fSkill - 0.5f) * 2.0f) + fMid;
    }
    return (fBoost - fMax) * ((fSkill - 1.0f) * 2.0f) + fMax;
}

// FUNCTION: SWBF2 0x00439160
void AIDifficulty::GetFlyerSpeedRange(const UnitController* pController, float fMin, float fMid, float fMax,
                                      float fBoost, float& rLow, float& rHigh)
{
    rLow = GetRangedSpeed(GetFloatSkill_RangedLinear(pController, 0.3f, 0.8f, true), fMin, fMid, fMax, fBoost);
    float fHigh = GetRangedSpeed(GetFloatSkill_RangedLinear(pController, 0.5f, 1.2f, true), fMin, fMid, fMax, fBoost);
    rHigh = (rLow > fHigh) ? rLow : fHigh;
}

namespace AIDifficulty {

// FUNCTION: SWBF2 0x004392e0
float SpaceAssault_GetFlyerKillForPlayerTeam()
{
    return 0.4f;
}
}

namespace AIDifficulty {

// FUNCTION: SWBF2 0x004392f0
float SpaceAssault_GetCritSysKillForPlayerTeam()
{
    return AIDifficulty::GetFloatSkill_RangedLinear(0, 0.4f, 0.25f, true);
}
}
