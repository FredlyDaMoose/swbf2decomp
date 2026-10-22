#pragma once

#include <stddef.h>
#include "ActionKey.h"

#include "RedEngineFL/Graphics/PC/pcRedAutoSettings.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

enum eGAME_DIFFICULTY
{
    eDIFFICULTY_EASY = 1,
    eDIFFICULTY_MEDIUM = 2,
    eDIFFICULTY_HARD = 3,
    eDIFFICULTY_LAST = 4
};

namespace PlayerProfile
{

struct tGameOpt
{
    bool mbCrossHair;
    bool mbRumble;
    bool mbTargetAutoTracking;
    bool mbSubtitle;
    bool mbWidescreen;
    bool mbHerosEnabled;
    bool mCockpitCam;
    bool mCockpitCamFlyer;
    bool mbMetaAllMaps;
    bool mbObjectiveDetails;

    unsigned char mAlignmentBeforeDifficulty[2];
    eGAME_DIFFICULTY mCurDiffLevel;
    bool mFlip[5];

    bool mMouseAssist[5];
    ActionKey mKeyboardBindings[5][43];
    float mMouseSensitivity[5];
    unsigned char mJoystickSensitivityX[5];
    unsigned char mJoystickSensitivityY[5];

    unsigned char mDeadZone;

    unsigned char mAlignmentAfterDeadZone;

    float mReticuleAlpha;
    short mMedalsGathered[9];
    unsigned char mCampaignProgress[16];
    unsigned char mControllerScale[2][5];
    unsigned char mFriendlyFire;

    unsigned char mAlignmentBeforeUnlockState[3];
    unsigned int mUnlockableState;
    bool mbAimAssist;
    bool mbStickyReticule;
    bool mbFlyerRolling;
    unsigned char mu8MovieSubtitles;
    bool mbSoldierHints;
    unsigned char mPadding[61];

    unsigned char mTailAlignment[2];

    void ResetToDefault();
    bool ResetBindings(int iPort);

    void ApplySettings(int iPort, int iViewport);

    bool NoteBindingChange();
};
 
 
 
 
 
 

struct tSoundOpt
{
    unsigned char mu8MusicVol;
    unsigned char mu8SFXVol;
    unsigned char mu8SpeechVol;
    unsigned char mPlaybackMode;
    unsigned char mMixerMode;
    unsigned char mEffectsEnabled;
    unsigned char mu8MasterVol;
    unsigned char mBassManagement;
    unsigned char mu8BattleSpeechVol;
    unsigned char mPadding[63];

    void ResetToDefault();
    void ApplySettings();
    void SanityCheck();

    void EnsureIsSafe();
};
 

struct tOnlineOpt
{
    unsigned char mu8AppearOffline;
    unsigned char mu8VoiceMask;
    unsigned char mu8VoiceToTVVol;
    unsigned char mu8DisplayGamertags;
    char mGamespyNickname[32];
    unsigned int mMustBeZero1;
    char mGamespyEmail[64];
    unsigned int mMustBeZero2;
    char mGamespyPassword[32];
    unsigned int mMustBeZero3;
    unsigned char mu8_SaveInfo;
    bool mHasSeenEULA;
    unsigned char mu8VoiceRecordVol;
    unsigned char mu8VoicePlayVol;
    unsigned char mu8PlayersSupported;
    unsigned char mu8TurnsPerSecond;
    unsigned char mOnlineIcon;
    unsigned char mVoiceEnable;
    unsigned char mShowAllRegions;
    unsigned char mu8GSID_Autologin;
    unsigned char mMinPlayers;

    unsigned char mAlignmentBeforeMaxPing;
    unsigned short mMaxPing;
    unsigned short mMapIdx;
    unsigned char mEraIdx;
    unsigned char mServerIdx;
    char mGameNameFilter[32];

    unsigned char mAlignmentBeforeMustBeZero[2];
    unsigned int mMustBeZero4;
    unsigned char mVoiceRecordDevice;
    unsigned char mVoiceRecordInput;
    unsigned char mVoicePlaybackDevice;

    unsigned char mAlignmentBeforeFavoriteServers;
    unsigned char mFavoriteServers[128][8];
    int mPendingBuddies[256];
    unsigned char mPadding[64];

    void ResetToDefault();
    void ApplySettings();
    void SanityCheck();
};
 
 

struct tToolTipOpt
{
    unsigned char mToolTipCount[32];
    bool mOn;
    bool mOff;
    unsigned char mPadding[32];

    void ResetToDefault();
    void SanityCheck();
};
 

struct tAccumPoints
{
    unsigned int mTotalPoints;
    unsigned int mTotalKills;
    unsigned int mTotalDeaths;

    void ResetToDefault();
};
 

struct tPCOpt
{
    bool mJoyEnabled;
    bool mMouseInvert;
    unsigned short mResolutionX;
    unsigned short mResolutionY;
    bool mWindowed;
    bool mVSync;
    D3DMULTISAMPLE_TYPE mMultiSample;
    unsigned long mMultiSampleQuality;
    RenderOptionsT renderOptions;
    char mJoinIp[32];
    unsigned int mMustBeZero;
    unsigned char mPadding[128];

    void ResetToDefault();
    void SanityCheck();

    void EnsureIsSafe();

    void ApplyPreLoadSettings();

    void ApplyPostLoadSettings();
    void ApplySettings();
};
 
 
 

 
 
 
 
 
 
 

struct tProfile
{
    bool mInitialized;
    bool mAutoSaveOn;
    unsigned short mName[16];
    unsigned int mMustBeZero1;
    unsigned int mVersion;
    bool mbFakeProfile;

    unsigned char mAlignmentBeforeGameOptions[3];
    tGameOpt mGameOptions;
    tSoundOpt mSoundOptions;
    tOnlineOpt mOnlineOptions;
    tToolTipOpt mToolTipOptions;

    unsigned char mAlignmentBeforeAccumPoints[2];
    tAccumPoints mAccumPoints;
    tPCOpt mPCOptions;
    unsigned char mPadding[96];
    unsigned char mCurMission;

    unsigned char mAlignmentBeforeBonus[3];
    unsigned int mBonus;
    bool mIsDirty;

    unsigned char mAlignmentBeforeCRC[3];
    unsigned int mCRC;

    tProfile();

    void ResetProfile();
    void ResetBindings();

    void NoteBindingChange();

    void ResetToDefault();
    void ApplySettings();
    unsigned int CalcProfCRC();

    const unsigned short* GetName();

    void UpdateProfCRC();

    void InitProfile(unsigned short const* pusName);
    bool GetUnlockableState(int iIndex);
    void SetUnlockableState(int iIndex, bool bState);

    bool IsDirty(bool bArg);
    bool IsDefaultProfile();
    void SetTrueProfileName(unsigned short const* pusName);
    void SanityCheck();
    unsigned short* GetTrueProfileName() { return mName; }
};
 
 
 
 
 
 
 
 

tProfile* GetProfile(unsigned int uiIndex);

tProfile* GetCurProfile();

unsigned int GetCurProfileIndex();
bool SetCurProfile(unsigned int uiIndex);

int GetNumMemoryProfiles();
void SetNumMemoryProfiles(int iNum);
int FindByName(unsigned short const* pusName);
void BackupDefaultProfile();
void DuplicateDefaultProfile(unsigned int uiIndex, unsigned short const* pusName);
unsigned short const* GetDefaultProfileName();
int GetDefaultProfileIdx();

}

namespace PlayerPrefs { void InitAll(); }

double ConvertMouseScaleToRawScale(float fScale, int iMode);

namespace PlayerProfile { void ResetAllProfiles(); }

namespace PlayerPrefs
{
struct OnePrefs
{
    OnePrefs() { Clear(); }
    void Clear();

    int mPlayerId;
    int mViewport;
    int mJoystick;
    int mTeamNum;
    eGAME_DIFFICULTY mDifficulty;
    PlayerProfile::tProfile* pProfile;
    bool hasChallenge;
    char challenge[16];
};
 
 
 
 
 

extern OnePrefs mAllUserPrefs[1];
}

// GLOBAL: SWBF2 0x01ef7fa8

static int* const playerJoystickNumbers = &PlayerPrefs::mAllUserPrefs[0].mJoystick;
// GLOBAL: SWBF2 0x01ef7fac

static int* const playerTeamNumbers = &PlayerPrefs::mAllUserPrefs[0].mTeamNum;

namespace PlayerProfile { extern int mCurProfile; }
extern int mNumMemoryProfiles;
