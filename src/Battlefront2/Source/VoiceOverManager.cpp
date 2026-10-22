#include "VoiceOverManager.h"
#include "GameLoop.h"
#include "Team.h"
#include "NetGame.h"
#include "Character.h"

extern bool gAttractMode;

// GLOBAL: SWBF2 0x01eafd5c
VoiceOverManager* VoiceOverManager::sInstance;

// FUNCTION: SWBF2 0x00675220
void VoiceOverManager::Create()
{
    sInstance = new VoiceOverManager;
}

// FUNCTION: SWBF2 0x00675290
VoiceOverManager::VoiceOverManager()
{
    mBleedingTime[0] = 0.0f;
    mBleedingTime[1] = 0.0f;
    mBleedingRepeatTime = 30.0f;
    mLowReinforcements[0] = 20;
    mLowReinforcements[1] = 20;
    mLowReinforcementsTriggered = false;
    mTeamBleeding[0] = false;
    mTeamBleeding[1] = false;
    mMusicTeamIndexPrev = 0;
    mPlay = false;
    if (NetGame::IsEnabled())
    {
        Enable(true);
    }
    ActivateThread(0, PER_LOCAL_TURN);
    mAttackingTeam = 0;
}

// FUNCTION: SWBF2 0x00675400
VoiceOverManager::AmbientMusic::~AmbientMusic()
{
}

// FUNCTION: SWBF2 0x00675410
VoiceOverManager::~VoiceOverManager()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x006754c0
VoiceOverManager::GameStage VoiceOverManager::DetermineGameStage() const
{
    int stage = gameStage_max;
    for (int team = 0; team < 2; ++team)
    {
        int reinforcements = Team::sTeams[team + 1]->mCurReinforcements;
        for (int i = gameStage_max - 1; i >= 0; --i)
        {
            if (reinforcements <= mAmbientMusic[team][i].mReinforcementCount)
            {
                if (i < stage)
                {
                    stage = i;
                }
                break;
            }
        }
    }
    if (stage == gameStage_max)
    {
        stage = gameStage_beginning;
    }
    return (GameStage)stage;
}

// FUNCTION: SWBF2 0x00675520
bool VoiceOverManager::Update(float fDeltaTime)
{
    unsigned int startBleeding = 0;
    unsigned int stopBleeding = 0;
    unsigned int repeatBleeding = 0;

    if (GameLoop::IsGameOver() || !mPlay)
    {
        return true;
    }

    unsigned int mask = 1;
    for (int team = 0; team < 2; ++team, mask = (mask << 1) | (mask >> 31))
    {
        if (Team::sTeams[team + 1]->IsBleeding())
        {
            mBleedingTime[team] += fDeltaTime;
            if (!mTeamBleeding[team])
            {
                startBleeding |= mask;
                mTeamBleeding[team] = true;
            }
            else if (mBleedingTime[team] > mBleedingRepeatTime)
            {
                repeatBleeding |= mask;
            }
            else
            {
                continue;
            }
        }
        else
        {
            if (!mTeamBleeding[team])
            {
                continue;
            }
            stopBleeding |= mask;
            mTeamBleeding[team] = false;
        }
        mBleedingTime[team] = 0.0f;
    }

    mask = 1;
    for (int team = 0; team < 2; ++team, mask = (mask << 1) | (mask >> 31))
    {
        Team* pTeam = Team::sTeams[team + 1];
        if ((startBleeding & mask) || ((repeatBleeding & mask) && team == GetPlayerTeamNumber()))
        {
            PlayVoiceOverBleeding(team, true);
        }
        else if (stopBleeding & mask)
        {
            PlayVoiceOverBleeding(team, false);
        }

        if (!mLowReinforcementsTriggered && pTeam->mCurReinforcements <= mLowReinforcements[team])
        {
            PlayVoiceOverLowReinforcements(team);
            mLowReinforcementsTriggered = true;
        }
    }

    if (!GameLoop::IsGameOver())
    {
        GameStage stage = DetermineGameStage();
        int musicTeam = GetPlayerTeamNumber();
        mAmbientMusic[musicTeam][stage].mMusic.Play();
        if (mMusicTeamIndexPrev != musicTeam)
        {
            mAmbientMusic[mMusicTeamIndexPrev][stage].mMusic.Stop();
        }
        if (stage > gameStage_beginning)
        {
            for (int team = 0; team < 2; ++team)
            {
                mAmbientMusic[team][stage - 1].mMusic.Stop();
            }
        }
        mMusicTeamIndexPrev = musicTeam;
    }
    return true;
}

// FUNCTION: SWBF2 0x006757d0
int VoiceOverManager::GetPlayerTeamNumber() const
{
    if (gAttractMode)
    {
        return 0;
    }
    int iTeam = 1;
    if (NetGame::GetLocalPlayer(0))
    {
        iTeam = NetGame::GetLocalPlayer(0)->mTeamNumber - 1;
    }
    if (NetGame::GetLocalPlayer(1) && iTeam != NetGame::GetLocalPlayer(1)->mTeamNumber - 1)
    {
        iTeam = mAttackingTeam;
    }
    return iTeam;
}

// FUNCTION: SWBF2 0x00675840
void VoiceOverManager::PlayVoiceOverBleeding(int iTeam, bool bStart)
{
    if (gAttractMode || GameLoop::IsGameOver())
    {
        return;
    }
    const int iPlayerTeam = GetPlayerTeamNumber();
    if (!bStart)
    {
        return;
    }
    if (iPlayerTeam == iTeam || !mTeamBleeding[iPlayerTeam])
    {
        mVoiceOverStartBleed[iPlayerTeam][iTeam].Play(0, 0, iTeam != 0, true);
    }
    else
    {
        mVoiceOverStopBleed[iPlayerTeam][iTeam].Play(0, 0, iTeam != 0, true);
    }
}

// FUNCTION: SWBF2 0x006758b0
void VoiceOverManager::PlayVoiceOverLowReinforcements(int iTeam)
{
    if (GameLoop::IsGameOver())
    {
        return;
    }
    mVoiceOverLowReinforcements[GetPlayerTeamNumber()][iTeam].Play(0, 0, iTeam != 0, true);
}

// FUNCTION: SWBF2 0x006756e0
void VoiceOverManager::Enable(bool bEnable)
{
    mPlay = bEnable;
}

// FUNCTION: SWBF2 0x006756c0
GameMusic& VoiceOverManager::GetAmbientMusic()
{
    return mAmbientMusic[mMusicTeamIndexPrev][DetermineGameStage()].mMusic;
}

// FUNCTION: SWBF2 0x006758f0
void VoiceOverManager::PlayVoiceOver(unsigned int uSoundID, int iTeamMask)
{
    GameSoundStream sound;
    sound.SetID(uSoundID, false, false);
    const int iLocalIndex = NetGame::GetLocalIndex(0);
    Character* pPlayer = Character::FindPlayer(iLocalIndex);
    if (pPlayer && (iTeamMask & (1 << pPlayer->mTeamNumber)))
    {
        sound.Play(0, 0, pPlayer->mTeamNumber, true);
    }
}

// FUNCTION: SWBF2 0x00675200
VoiceOverManager::AmbientMusic::AmbientMusic()
    : mReinforcementCount(0), mTriggered(false)
{
}

// FUNCTION: SWBF2 0x006756f0
void VoiceOverManager::SetBleedingVoiceOver(int iTeam, int iIndex, const char* pName, bool bStart)
{
    if (bStart)
    {
        mVoiceOverStartBleed[iTeam][iIndex].SetID(pName, false, false);
    }
    else
    {
        mVoiceOverStopBleed[iTeam][iIndex].SetID(pName, false, false);
    }
}

// FUNCTION: SWBF2 0x00675740
void VoiceOverManager::SetLowReinforcementsVoiceOver(int iTeam, int iIndex, const char* pName,
                                                     int iCount)
{
    mVoiceOverLowReinforcements[iTeam][iIndex].SetID(pName, false, false);
    mLowReinforcements[iTeam] = iCount;
}

// FUNCTION: SWBF2 0x00675780
void VoiceOverManager::SetAmbientMusic(int iTeam, int iCount, const char* pName, GameStage eStage)
{
    mAmbientMusic[iTeam][eStage].mReinforcementCount = iCount;
    mAmbientMusic[iTeam][eStage].mMusic.SetID(pName);
}

// FUNCTION: SWBF2 0x006757c0
void VoiceOverManager::SetAttackingTeam(int iTeam)
{
    mAttackingTeam = iTeam;
}
