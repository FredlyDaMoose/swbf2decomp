#pragma once

#include <stddef.h>

#include "Thread.h"
#include "GameMusic.h"
#include "GameSoundStream.h"

// VTABLE: SWBF2 0x007b00d4
class VoiceOverManager : public Thread
{
public:

    enum GameStage
    {
        gameStage_beginning = 0,
        gameStage_middle = 1,
        gameStage_end = 2,
        gameStage_max = 3
    };

    class AmbientMusic
    {
    public:
        AmbientMusic();
        ~AmbientMusic();

        GameMusic mMusic;
        int mReinforcementCount;
        bool mTriggered;
    };

    VoiceOverManager();

    // SYNTHETIC: SWBF2 0x006753d0
    // VoiceOverManager::`scalar deleting destructor'
    virtual ~VoiceOverManager();
    virtual bool Update(float fDeltaTime);

    int GetPlayerTeamNumber() const;
    void PlayVoiceOverBleeding(int iTeam, bool bStart);
    void PlayVoiceOverLowReinforcements(int iTeam);

    static void PlayVoiceOver(unsigned int uSoundID, int iTeamMask);

    void SetBleedingVoiceOver(int iTeam, int iIndex, const char* pName, bool bStart);
    void SetLowReinforcementsVoiceOver(int iTeam, int iIndex, const char* pName,
                                       int iCount);
    void SetAmbientMusic(int iTeam, int iCount, const char* pName, GameStage eStage);
    void SetAttackingTeam(int iTeam);

    void Enable(bool bEnable);

    GameMusic& GetAmbientMusic();

private:
    GameStage DetermineGameStage() const;

public:
    float mBleedingTime[2];
    float mBleedingRepeatTime;
    GameSoundStream mVoiceOverStartBleed[2][2];
    GameSoundStream mVoiceOverStopBleed[2][2];
    GameSoundStream mVoiceOverLowReinforcements[2][2];
    int mLowReinforcements[2];
    bool mLowReinforcementsTriggered;
    bool mTeamBleeding[2];

    AmbientMusic mAmbientMusic[2][3];
    int mMusicTeamIndexPrev;
    int mAttackingTeam;
    bool mPlay;

    static void Create();

    static VoiceOverManager* sInstance;
};
 
 
 
 
 
 
 
 
 
 
 
 
