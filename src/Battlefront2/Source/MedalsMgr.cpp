#include <string.h>

#include "MedalsMgr.h"
#include "NetPacket.h"
#include "GameLoop.h"
#include "MessageDisplay.h"
#include "NetGame.h"
#include "Character.h"
#include "PlayerProfile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

// FUNCTION: SWBF2 0x005a3480
int MedalsMgr::GetRank() const
{
    if (NetGame::IsEnabled())
    {
        return 1;
    }
    int iTotal = 0;
    for (int i = 0; i < MEDAL_TYPES_COUNT; ++i)
    {
        iTotal += GetMedals(i);
    }
    if (iTotal >= 300)
    {
        return 3;
    }
    if (iTotal >= 100)
    {
        return 2;
    }
    return iTotal >= 20 ? 1 : 0;
}

// GLOBAL: SWBF2 0x0078a4f0
extern const PointCondition sMedalPoints_condition[MedalsMgr::MEDAL_TYPES_COUNT] =
{
    {  6, true,  false, 0xff, false, false, "gunslinger" },
    { 12, true,  false, 0xff, false, false, "frenzy" },
    {  4, false, true,  8,    false, false, "demolition" },
    {  1, false, false, 8,    true,  false, "technician" },
    {  6, false, true,  7,    false, false, "marksman" },
    {  8, true,  false, 0xff, false, false, "regulator" },
    { 12, false, false, 0xff, false, true,  "endurance" },
    { 24, false, false, 0xff, false, true,  "guardian" },
    { 36, false, false, 0xff, false, true,  "warhero" },
};

// GLOBAL: SWBF2 0x01ecb14c
// SYNTHETIC: SWBF2 0x004042b0
// `dynamic initializer for 'sMedalHudColor''
RedColor sMedalHudColor = RedColor::CYAN;

// GLOBAL: SWBF2 0x01ecb148
// SYNTHETIC: SWBF2 0x004042e0
// `dynamic initializer for 'sAwardHudColor''
RedColor sAwardHudColor = RedColor::YELLOW;

// FUNCTION: SWBF2 0x005a35c0
void MedalsMgr::HudIndication(int iType, const char* pPrefix, const RedColor& color, bool) const
{
    if (!NetGame::IsEnabled() || NetGame::IsClient() ||
        (NetGame::IsHost() && NetGame::IsOneWorld() && !NetGame::IsDedicated()))
    {
        for (unsigned int i = 0; i < GameLoop::GetNumCameras(); ++i)
        {
            if (NetGame::GetLocalPlayer(i) != mOwner)
            {
                continue;
            }

            char key[68];
            strncpy(key, pPrefix, 64);
            strncat(key, sMedalPoints_condition[iType].name, 64);

            unsigned short wideKey[64];
            const unsigned short* pText = RedLocalizeDB::Find(PblHash(key), true);
            if (!pText)
            {
                PblUnicode::ustrncnv(wideKey, key, 64);
                pText = wideKey;
            }
            MessageDisplay::Instance(i, MessageDisplay::DISPLAY_SYSTEM_MSG)->AddMessage(pText, color);
        }
    }
}

// FUNCTION: SWBF2 0x005a3090
MedalsMgr::MedalsMgr(Character* pOwner)
{
    mOwner = pOwner;
    Init();
}

// FUNCTION: SWBF2 0x005a30b0
void MedalsMgr::Init()
{
    for (int i = 0; i < MEDAL_TYPES_COUNT; ++i)
    {
        mMedalsGathered[i] = 0;
        mMedalPoints[i] = 0;
        mAwardTime[i] = 0.0f;
    }
    mAwardAvailable = 0;
}

// FUNCTION: SWBF2 0x005a3100
void MedalsMgr::SetMedals(int iType, short sMedals)
{
    mMedalsGathered[iType] = sMedals;
}

// FUNCTION: SWBF2 0x005a3810
void MedalsMgr::LoadMedals()
{
    if (NetGame::IsEnabled() || !mOwner)
    {
        return;
    }
    PlayerProfile::tProfile* pProfile = mOwner->GetProfile();
    if (!pProfile)
    {
        return;
    }
    for (int i = 0; i < MEDAL_TYPES_COUNT; ++i)
    {
        SetMedals(i, pProfile->mGameOptions.mMedalsGathered[i]);
    }
}

// FUNCTION: SWBF2 0x005a30f0
short MedalsMgr::GetMedals(int iType) const
{
    return mMedalsGathered[iType];
}

// FUNCTION: SWBF2 0x005a3120
MedalsMgr::CareerLevel MedalsMgr::GetLevel(int iType) const
{
    if (NetGame::IsEnabled())
    {
        return VETERAN;
    }
    short sMedals = GetMedals(iType);
    if (sMedals >= 64)
    {
        return LEGENDARY;
    }
    if (sMedals >= 32)
    {
        return ELITE;
    }
    if (sMedals >= 4)
    {
        return VETERAN;
    }
    return GREEN;
}

// FUNCTION: SWBF2 0x005a31a0
short MedalsMgr::GetPointsNeededForMedal(int iType) const
{
    if (GetLevel(iType) >= ELITE)
    {
        short sPoints = (short)(sMedalPoints_condition[iType].pointsPerMedal * 3 / 4);
        return sPoints ? sPoints : 1;
    }
    return sMedalPoints_condition[iType].pointsPerMedal;
}

bool GetNetAwardsEnabled();

// FUNCTION: SWBF2 0x005a31f0
void MedalsMgr::AddMedalPoints(int iType, bool bKill, bool bCritHit, HealthType eHealthType, bool bHack,
                               bool bScore, short sPoints)
{
    if (NetGame::IsEnabled() && (NetGame::IsClient() || !GetNetAwardsEnabled()))
    {
        return;
    }
    if (sPoints <= 0)
    {
        return;
    }

    const PointCondition& condition = sMedalPoints_condition[iType];
    if (condition.kill && !bKill)
    {
        return;
    }
    if (condition.critHit && !bCritHit)
    {
        return;
    }
    if (eHealthType < NUM_HEALTH_TYPES && !((1 << eHealthType) & condition.healthType))
    {
        return;
    }
    if (condition.hack && !bHack)
    {
        return;
    }
    if (condition.score && !bScore)
    {
        return;
    }

    short sNeeded = GetPointsNeededForMedal(iType);
    if (mMedalPoints[iType] < sNeeded && mMedalPoints[iType] + sPoints >= sNeeded)
    {
        if (NetGame::IsEnabled())
        {
            mMedalsGathered[iType] = 1;
        }
        else
        {
            ++mMedalsGathered[iType];
            HudIndication(iType, "game.awards.medal.", sMedalHudColor, false);
        }
    }
    mMedalPoints[iType] += sPoints;
    if (mMedalPoints[iType] > sNeeded)
    {
        mMedalPoints[iType] = sNeeded;
    }
}

// FUNCTION: SWBF2 0x005a3350
bool MedalsMgr::IsAwardAvailableInternal(int iType) const
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return false;
    }
    if (iType < 0)
    {
        return true;
    }
    CareerLevel eLevel = GetLevel(iType);
    if (eLevel >= VETERAN && mMedalPoints[iType] >= GetPointsNeededForMedal(iType))
    {
        return true;
    }
    return eLevel >= LEGENDARY;
}

// FUNCTION: SWBF2 0x005a33d0
bool MedalsMgr::IsAwardAvailable(int iType) const
{
    return (mAwardAvailable & (1 << iType)) != 0;
}

// FUNCTION: SWBF2 0x005a3510
void MedalsMgr::Update(float fDeltaTime)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    for (int i = 0; i < MEDAL_TYPES_COUNT; ++i)
    {
        bool bAvailable = IsAwardAvailableInternal(i);
        if (IsAwardAvailable(i) != bAvailable)
        {
            if (bAvailable)
            {
                mAwardTime[i] = GameLoop::GetMissionTime();
                mAwardAvailable |= 1 << i;
                HudIndication(i, "game.awards.bonus.", sAwardHudColor, true);
            }
            else
            {
                mAwardAvailable &= ~(1 << i);
            }
        }
    }
}

// FUNCTION: SWBF2 0x005a3700
void MedalsMgr::WriteObject(NetPktGroup& group)
{
    group.WriteBits(mAwardAvailable, 16);
}

// FUNCTION: SWBF2 0x005a3720
void MedalsMgr::ReadObject(NetPktGroup& group)
{
    unsigned int uOld = mAwardAvailable;
    unsigned int uNew;
    group.ReadBits(uNew, 16);
    mAwardAvailable = (unsigned short)uNew;
    for (int i = 0; i < MEDAL_TYPES_COUNT; ++i)
    {
        unsigned short uBit = (unsigned short)(1 << i);
        if (!(uOld & uBit) && (mAwardAvailable & uBit))
        {
            HudIndication(i, "game.awards.bonus.", sAwardHudColor, true);
        }
    }
}

// FUNCTION: SWBF2 0x005a3790
void MedalsMgr::SaveMedals()
{
    if (NetGame::IsEnabled() || !mOwner)
    {
        return;
    }
    PlayerProfile::tProfile* profile = mOwner->GetProfile();
    if (!profile)
    {
        return;
    }
    bool changed = false;
    for (int i = 0; i < MEDAL_TYPES_COUNT; ++i)
    {
        if (profile->mGameOptions.mMedalsGathered[i] != GetMedals(i))
        {
            profile->mGameOptions.mMedalsGathered[i] = GetMedals(i);
            changed = true;
        }
    }
    if (changed)
    {
        profile->UpdateProfCRC();
    }
}

// FUNCTION: SWBF2 0x005a33f0
void MedalsMgr::ClearAllMedalPoints()
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    for (int i = 0; i < MEDAL_TYPES_COUNT; ++i)
    {
        if (IsAwardAvailableInternal(i) && GameLoop::GetMissionTime() - mAwardTime[i] <= 30.0f)
        {
            mAwardAvailable |= 1 << i;
        }
        else
        {
            mMedalPoints[i] = 0;
            mAwardTime[i] = 0.0f;
        }
    }
}
