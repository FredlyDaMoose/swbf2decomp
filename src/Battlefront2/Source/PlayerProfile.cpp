#include <string.h>
#include "RedEngineFL/Application/pc/pcRedParticle.h"

#include "PlayerProfile.h"

#include "CommandLine.h"
#include "FLInputManager.h"
#include "GameSoundEngine.h"
#include "GameState.h"
#include "PauseMenu.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/PC/pcRedAutoSettings.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "ColorControl.h"
#include "Damageable.h"
#include "HDREffect.h"
#include "LockOnManager.h"
#include "NetComm.h"
#include "NetGame.h"
#include "PropCluster.h"
#include "VoiceChatNode.h"
#include "RedEngineFL/movie/RedMovie.h"
#include "SoundFL/source/win/sndengine.h"

#include <math.h>
#include "GameVoiceChatNode.h"
#include "NetMatch.h"
#include "RedEngineFL/input/RedJoystick.h"

BOOL CALLBACK EnumObjectsCallback(DIDEVICEOBJECTINSTANCEA const* pObject, void* pvRef);

// GLOBAL: SWBF2 0x01fa9264
unsigned int mLastDirtyCRC[1];

namespace PblBuildType { extern unsigned char gCreateMemoryProfile; }

extern bool pcWindowed;
extern int pcWidth;
extern int pcHeight;

namespace ParticleSystem { void SetFadeBias(float fBias); }
using ParticleSystem::SetFadeBias;

namespace GameVoiceChat { void* FindNodeByDeviceID(); }

// GLOBAL: SWBF2 0x01ef90d0
// SYNTHETIC: SWBF2 0x00405f40
// `dynamic initializer for 'mProfiles''
PlayerProfile::tProfile mProfiles[1];

// GLOBAL: SWBF2 0x01ef7fd0
// SYNTHETIC: SWBF2 0x00405f50
// `dynamic initializer for 'mDefaultProfile''
PlayerProfile::tProfile mDefaultProfile;

// FUNCTION: SWBF2 0x0061c870
bool PlayerProfile::tProfile::GetUnlockableState(int iIndex)
{
    return (mGameOptions.mUnlockableState & (1u << iIndex)) != 0;
}

// FUNCTION: SWBF2 0x0061c890
void PlayerProfile::tProfile::SetUnlockableState(int iIndex, bool)
{
    unsigned int bit = 1u << iIndex;
    if (!(mGameOptions.mUnlockableState & bit))
    {
        mGameOptions.mUnlockableState |= bit;
        UpdateProfCRC();
    }
}

// FUNCTION: SWBF2 0x0061d6e0
int PlayerProfile::FindByName(const unsigned short* pusName)
{
    for (unsigned int i = 0; i < sizeof(mProfiles) / sizeof(mProfiles[0]); ++i)
    {
        if (!PblUnicode::ustrcmp(pusName, mProfiles[i].mName))
        {
            return (int)i;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x0061d7a0
void PlayerProfile::tProfile::SetTrueProfileName(const unsigned short* pusName)
{
    PblUnicode::ustrncpy(mName, pusName, 16);
}

// FUNCTION: SWBF2 0x0061d7c0
void PlayerProfile::BackupDefaultProfile()
{
    if (!mProfiles[PlayerProfile::mCurProfile].mbFakeProfile)
    {
        memcpy(&mDefaultProfile, &mProfiles[PlayerProfile::mCurProfile], sizeof(mDefaultProfile));
    }
}

// FUNCTION: SWBF2 0x0061d7f0
void PlayerProfile::DuplicateDefaultProfile(unsigned int uiIndex, const unsigned short* pusName)
{
    if (uiIndex < sizeof(mProfiles) / sizeof(mProfiles[0]) && mDefaultProfile.mName[0])
    {
        memcpy(&mProfiles[uiIndex], &mDefaultProfile, sizeof(mDefaultProfile));
        if (pusName)
        {
            mProfiles[uiIndex].SetTrueProfileName(pusName);
        }
    }
}

// FUNCTION: SWBF2 0x0061d840
const unsigned short* PlayerProfile::GetDefaultProfileName()
{
    return mDefaultProfile.mName;
}

// GLOBAL: SWBF2 0x01ef7fa0
// SYNTHETIC: SWBF2 0x00405f30
// `dynamic initializer for 'PlayerPrefs::mAllUserPrefs''
namespace PlayerPrefs { OnePrefs mAllUserPrefs[1]; }

// GLOBAL: SWBF2 0x01eaedf4
namespace PlayerProfile { int mCurProfile; }

// GLOBAL: SWBF2 0x01eaedf8
bool mSafeMode;

// GLOBAL: SWBF2 0x01fa9268
int mNumMemoryProfiles;

// GLOBAL: SWBF2 0x009661bc
bool RedParticleRenderer::s_blurEnable;

// FUNCTION: SWBF2 0x0061c410
void PlayerPrefs::OnePrefs::Clear()
{
    mPlayerId = -1;
    mViewport = 0;
    mJoystick = 0;
    mTeamNum = PlayerPrefs::gDefaultTeamNum;
    mDifficulty = (eGAME_DIFFICULTY)PlayerPrefs::gDefaultDifficulty;
    pProfile = 0;
    hasChallenge = false;
}

// FUNCTION: SWBF2 0x0061c440
void PlayerPrefs::InitAll()
{
    PlayerProfile::ResetAllProfiles();
    PlayerPrefs::mAllUserPrefs[0].Clear();

    PlayerPrefs::mAllUserPrefs[0].mPlayerId = 0;
    PlayerPrefs::mAllUserPrefs[0].mViewport = 0;
    PlayerPrefs::mAllUserPrefs[0].mJoystick = 0;

    PlayerPrefs::mAllUserPrefs[0].pProfile = PlayerProfile::GetProfile(0);
    *(PlayerProfile::tProfile**)((char*)&mDefaultProfile + 0x10) = PlayerProfile::GetProfile(1);
}

// FUNCTION: SWBF2 0x0061c4d0
double ConvertMouseScaleToRawScale(float fScale, int iMode)
{
    double dScale = pow(5.0, (fScale - 5.0) / 5.0);
    if (iMode == 0)
    {
        dScale *= 1.3;
    }
    return dScale;
}

// FUNCTION: SWBF2 0x0061c510
void PlayerProfile::tGameOpt::ResetToDefault()
{
    mbCrossHair = true;
    mbRumble = true;
    mbTargetAutoTracking = false;
    mbSubtitle = false;

    mbWidescreen = false;
    mbHerosEnabled = true;
    mCockpitCam = false;
    mCockpitCamFlyer = false;

    mbMetaAllMaps = false;
    mbObjectiveDetails = true;

    mCurDiffLevel = (eGAME_DIFFICULTY)PlayerPrefs::gDefaultDifficulty;

    mu8MovieSubtitles = 0;
    mbSoldierHints = false;

    eGAMELANGUAGE eLang = RedLocalizeDB::GetLanguageID();
    if (eLang == eLANG_SPANISH || eLang == eLANG_ITALIAN)
    {
        mbSubtitle = true;
        mu8MovieSubtitles = 1;
    }

    mCockpitCam = true;
    mReticuleAlpha = 0.5f;
    mFriendlyFire = 100;
    mUnlockableState = 0;
    mbAimAssist = false;
    mbStickyReticule = false;
    mbFlyerRolling = true;
    mbWidescreen = false;

    memset(mPadding, 0, sizeof(mPadding));
}

// FUNCTION: SWBF2 0x0061c5a0
bool PlayerProfile::tGameOpt::ResetBindings(int iPort)
{
    mFlip[iPort] = (iPort == 2);
    mControllerScale[0][iPort] = 4;
    mControllerScale[1][iPort] = 4;
    mMouseSensitivity[iPort] = 50.0f;
    mJoystickSensitivityX[iPort] = 4;
    mJoystickSensitivityY[iPort] = 4;
    mDeadZone = 5;
    mMouseAssist[iPort] = true;

    ActionKey aDefaults[43];
    memcpy(aDefaults, RawControllerInputs::sDefaultKeyboardBindings[iPort], sizeof(aDefaults));
    memcpy(mKeyboardBindings[iPort], aDefaults, sizeof(aDefaults));

    RawControllerInputs* pJoystick =
        FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport());
    memcpy(pJoystick->mCurKeyboardBindings[iPort], aDefaults,
           sizeof(aDefaults));

    return true;
}

// FUNCTION: SWBF2 0x0061c660
bool PlayerProfile::tGameOpt::NoteBindingChange()
{
    bool bChanged = false;
    for (int iMode = 0; iMode < 5; ++iMode)
    {
        RawControllerInputs* pJoystick =
            FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport());
        bool bFlip = pJoystick->GetFlipForMode(iMode);
        if (mFlip[iMode] != bFlip)
        {
            mFlip[iMode] = bFlip;
            bChanged = true;
        }
        bool bAssist = pJoystick->mMouseAssist[iMode];
        if (mMouseAssist[iMode] != bAssist)
        {
            mMouseAssist[iMode] = bAssist;
            bChanged = true;
        }
    }
    return bChanged;
}

// FUNCTION: SWBF2 0x0061c6b0
void PlayerProfile::tGameOpt::ApplySettings(int iPort, int  )
{
    if (!NetGame::IsEnabled())
    {
        Damageable::SetTeamDamageScale(mFriendlyFire / 100.0f);
    }

    RawControllerInputs* pJoystick = FLInputManager::s_instance.GetPlayerJoystick(iPort);
    unsigned char* pRaw = (unsigned char*)pJoystick;

    for (int iMode = 0; iMode < 5; ++iMode)
    {
        memcpy(pRaw + 0x20bc + iMode * sizeof(mKeyboardBindings[0]), mKeyboardBindings[iMode],
               sizeof(mKeyboardBindings[0]));
    }

    for (int iMode = 0; iMode < 5; ++iMode)
    {
        if (!mControllerScale[0][iMode])
        {
            mControllerScale[0][iMode] = 4;
        }
        if (!mControllerScale[1][iMode])
        {
            mControllerScale[1][iMode] = 4;
        }

        pRaw[0x2638 + iMode] = mControllerScale[0][iMode];
        pRaw[0x263d + iMode] = mControllerScale[1][iMode];
        pRaw[0x000c + iMode] = mFlip[iMode];
        *(double*)(pRaw + 0x2610 + iMode * 8) = ConvertMouseScaleToRawScale(mMouseSensitivity[iMode] / 10.0f, iMode);
        pRaw[0x2638 + iMode] = mJoystickSensitivityX[iMode];
        pRaw[0x263d + iMode] = mJoystickSensitivityY[iMode];
        pRaw[0x2642 + iMode] = mMouseAssist[iMode];

        pJoystick->ApplyDeadZone(mDeadZone);
    }

    FLInputManager::s_instance.EnableRumble(iPort, mbRumble);

    LockOnManager* pLockOn = iPort < 0 ? 0 : LockOnManager::GetByPlayerIndex(iPort);
    if (pLockOn)
    {
        pLockOn->SetDisplayIconsOnEverything(mbSoldierHints);
    }

    RedMovie::SubtitlesEnable(mu8MovieSubtitles != 0);
}

// FUNCTION: SWBF2 0x0061c8c0
void PlayerProfile::tSoundOpt::ResetToDefault()
{
    memset(mPadding, 0, sizeof(mPadding));

    mu8MusicVol = 7;
    mu8SFXVol = 7;
    mu8SpeechVol = 9;
    mPlaybackMode = GameSoundEngine::gDefaultSpeakerConfig
                        ? (unsigned char)GameSoundEngine::gDefaultSpeakerConfig
                        : 3;
    mMixerMode = 1;
    mEffectsEnabled = 1;
    mu8MasterVol = 8;
    mBassManagement = 0;
    mu8BattleSpeechVol = 10;
}

// FUNCTION: SWBF2 0x0061c900
void PlayerProfile::tSoundOpt::ApplySettings()
{
    if (mMixerMode)
    {
        unsigned int uiMixConfig = mMixerMode;
        if (uiMixConfig != (unsigned int)Snd::Engine::GetMixConfig() ||
            (mEffectsEnabled == 1) != (Snd::Engine::smSoftwareEffectsEnabled ||
                                       Snd::Engine::smHardwareEffectsAvailable))
        {
            GameSoundEngine::ChangeMixConfig(uiMixConfig, mEffectsEnabled != 0);
        }
    }
    mMixerMode = (unsigned char)Snd::Engine::GetMixConfig();

    if (mPlaybackMode && mPlaybackMode != Snd::EngineBase::smSpeakerConfig)
    {
        Snd::Engine::SetSpeakerConfig((Snd::EngineBase::SpeakerConfig)mPlaybackMode);
    }
    mPlaybackMode = (unsigned char)Snd::EngineBase::smSpeakerConfig;

    GameSoundEngine::UpdateBusGains(mu8SFXVol * 0.1f, mu8MusicVol * 0.1f, mu8SpeechVol * 0.1f,
                                    mu8BattleSpeechVol * 0.1f, mu8MasterVol * 0.1f);
    Snd::Engine::SetBassManagement(mBassManagement == 1);
}

// FUNCTION: SWBF2 0x0061ca00
void PlayerProfile::tSoundOpt::SanityCheck()
{
    if (mu8MusicVol > 10)
    {
        mu8MusicVol = 7;
    }
    if (mu8SFXVol > 10)
    {
        mu8SFXVol = 7;
    }
    if (mu8SpeechVol > 10)
    {
        mu8SpeechVol = 9;
    }
    if (mu8BattleSpeechVol > 10)
    {
        mu8BattleSpeechVol = 10;
    }
    if (mu8MasterVol > 10)
    {
        mu8MasterVol = 8;
    }
    for (unsigned int i = 0; i < sizeof(mPadding); ++i)
    {
        if (mPadding[i])
        {
            ResetToDefault();
        }
    }
}

// FUNCTION: SWBF2 0x0061ca50
void PlayerProfile::tSoundOpt::EnsureIsSafe()
{
    mMixerMode = 1;
    mEffectsEnabled = 0;
    mPlaybackMode = GameSoundEngine::gDefaultSpeakerConfig
                        ? (unsigned char)GameSoundEngine::gDefaultSpeakerConfig
                        : 3;
}

// FUNCTION: SWBF2 0x0061ca70
void PlayerProfile::tPCOpt::ResetToDefault()
{
    memset(mPadding, 0, sizeof(mPadding));

    mJoyEnabled = true;
    mWindowed = false;
    mVSync = false;
    mMultiSample = D3DMULTISAMPLE_NONE;
    mMultiSampleQuality = 0;

    renderOptions = RedAutoSettings::GetRenderOptions();

    mResolutionX = (unsigned short)renderOptions.defaultWidth;
    mResolutionY = (unsigned short)renderOptions.defaultHeight;

    mMustBeZero = 0;
    strlcpy(mJoinIp, "127.0.0.1", sizeof(mJoinIp));
}

// FUNCTION: SWBF2 0x0061cb00
void PlayerProfile::tPCOpt::ApplySettings()
{
    ApplyPostLoadSettings();
}

// FUNCTION: SWBF2 0x0061cb10
void PlayerProfile::tPCOpt::SanityCheck()
{
    if (mMustBeZero)
    {
        ResetToDefault();
    }

    if (renderOptions.mGammaBrightness < 0.2f)
    {
        renderOptions.mGammaBrightness = 0.2f;
    }
    else if (renderOptions.mGammaBrightness > 0.8f)
    {
        renderOptions.mGammaBrightness = 0.8f;
    }
    if (renderOptions.mGammaContrast < 0.2f)
    {
        renderOptions.mGammaContrast = 0.2f;
    }
    else if (renderOptions.mGammaContrast > 0.8f)
    {
        renderOptions.mGammaContrast = 0.8f;
    }
    if (renderOptions.mFarSceneRangeFactor < 0.0f)
    {
        renderOptions.mFarSceneRangeFactor = 0.0f;
    }
    else if (renderOptions.mFarSceneRangeFactor > 1.0f)
    {
        renderOptions.mFarSceneRangeFactor = 1.0f;
    }
    if (renderOptions.mNearSceneFadeFactor < 0.0f)
    {
        renderOptions.mNearSceneFadeFactor = 0.0f;
    }
    else if (renderOptions.mNearSceneFadeFactor > 1.0f)
    {
        renderOptions.mNearSceneFadeFactor = 1.0f;
    }

    for (unsigned int i = 0; i < sizeof(mPadding); ++i)
    {
        if (mPadding[i])
        {
            ResetToDefault();
        }
    }
}

// FUNCTION: SWBF2 0x0061cbd0
void PlayerProfile::tPCOpt::EnsureIsSafe()
{
    ResetToDefault();
    mResolutionX = 800;
    mResolutionY = 600;
    renderOptions.textureDetail = QUALITY_LOW;
    renderOptions.terrainQuality = QUALITY_LOW;
    renderOptions.waterQuality = QUALITY_LOW;
    renderOptions.particleQuality = QUALITY_LOW;
    renderOptions.shadowQuality = QUALITY2_OFF;
    renderOptions.specularQuality = QUALITY2_LOW;
    renderOptions.bumpMapping = false;
    renderOptions.motionBlur = false;
    renderOptions.distortion = false;
    renderOptions.mGraphicsTier = 0;
    renderOptions.mEnableHDR = false;
}

// FUNCTION: SWBF2 0x0061cc20
void PlayerProfile::tPCOpt::ApplyPreLoadSettings()
{
    RedRenderer::pcSetRenderOptions(renderOptions);

    const bool bIsShell = (GameState::GetState() == 0x11E1FC01);

    unsigned int uiWidth = 0;
    unsigned int uiHeight = 0;

    D3DMULTISAMPLE_TYPE msType = mMultiSample;
    unsigned long ulMSQuality = mMultiSampleQuality;

    const bool bWindowed = pcWindowed ? true : mWindowed;

    if (pcWidth > 0 && pcHeight > 0)
    {

        uiWidth = (unsigned int)pcWidth;
        uiHeight = (unsigned int)pcHeight;
    }
    else if (!bIsShell && mResolutionX && mResolutionY)
    {
        uiWidth = mResolutionX;
        uiHeight = mResolutionY;
    }
    else
    {
        uiWidth = 0;
        uiHeight = 0;
        msType = (D3DMULTISAMPLE_TYPE)0;
        ulMSQuality = 0;
    }

    if (!bWindowed)
    {
        if (!RedVideoConfig::VidMode_Usable((int)uiWidth, (int)uiHeight, msType,
                                            ulMSQuality))
        {

            msType = (D3DMULTISAMPLE_TYPE)0;
            ulMSQuality = 0;

            if (!RedVideoConfig::VidMode_Usable((int)uiWidth, (int)uiHeight,
                                                (D3DMULTISAMPLE_TYPE)0, 0))
            {

                uiWidth = 0;
                uiHeight = 0;
            }
        }
    }

    if (!uiWidth || !uiHeight)
    {
        D3DDISPLAYMODE modes[50];
        const D3DDISPLAYMODE fallback[2] = {
            { 800, 600, 0, D3DFMT_UNKNOWN },
            { 640, 480, 0, D3DFMT_UNKNOWN },
        };

        const unsigned int uiModes =
            RedRenderer::pcGetValidDisplayModes(modes, 50, 640, 480);

        int iFound = -1;
        for (unsigned int j = 0; j < 2; ++j)
        {
            for (unsigned int i = 0; i < uiModes; ++i)
            {
                if (modes[i].Width == fallback[j].Width
                    && modes[i].Height == fallback[j].Height)
                {
                    iFound = (int)i;
                    break;
                }
            }
            if (iFound >= 0)
            {
                break;
            }
        }

        if (iFound < 0)
        {
            iFound = 0;
        }

        uiWidth = modes[iFound].Width;
        uiHeight = modes[iFound].Height;
    }

    RedRenderer::pcSetDisplayMode(uiWidth, uiHeight, bWindowed, mVSync, msType,
                                  ulMSQuality);
}

// FUNCTION: SWBF2 0x0061cdc0
void PlayerProfile::tPCOpt::ApplyPostLoadSettings()
{
    RedRenderer::pcSetRenderOptions(renderOptions);

    RedParticleRenderer::s_blurEnable = (RedRenderer::pcGetDeviceCaps().DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES) &&
                  RedRenderer::pcGetRenderOptions().distortion;

    SetPropDistanceScale(RedRenderer::pcGetRenderOptions().mNearSceneFadeFactor * 1.7f);

    float fParticleBias;
    switch (RedRenderer::pcGetRenderOptions().particleQuality)
    {
    case QUALITY_MEDIUM:
        fParticleBias = 40.0f;
        break;
    case QUALITY_HIGH:
        fParticleBias = 80.0f;
        break;
    default:
        fParticleBias = 0.0f;
        break;
    }
    ParticleSystem::SetFadeBias(fParticleBias);

    if (ColorControl::s_pInstance)
    {
        ColorControl::s_pInstance->SetLevel(COLORCONTROL_GAMMA_CONTRAST,
                                            RedRenderer::pcGetRenderOptions().mGammaContrast);
        ColorControl::s_pInstance->SetLevel(COLORCONTROL_GAMMA_BRIGHTNESS,
                                            RedRenderer::pcGetRenderOptions().mGammaBrightness);
        ColorControl::s_pInstance->Update(0.0f);
    }

    if (HDREffect::s_pInstance)
    {
        HDREffect::s_pInstance->UpdateHDREnabled();
    }
}

// FUNCTION: SWBF2 0x0061cea0
void PlayerProfile::tOnlineOpt::ResetToDefault()
{
    memset(mPadding, 0, sizeof(mPadding));

    mu8AppearOffline = 0;
    mu8VoiceMask = 0;
    mu8VoiceToTVVol = 0;
    mu8DisplayGamertags = 0xff;

    memset(mGamespyNickname, 0, sizeof(mGamespyNickname));
    mMustBeZero1 = 0;
    memset(mGamespyEmail, 0, sizeof(mGamespyEmail));
    mMustBeZero2 = 0;
    memset(mGamespyPassword, 0, sizeof(mGamespyPassword));
    mMustBeZero3 = 0;

    mu8_SaveInfo = 0x11;
    mHasSeenEULA = false;
    mu8VoiceRecordVol = 10;
    mu8VoicePlayVol = 10;
    mu8PlayersSupported = 0;
    mu8TurnsPerSecond = 20;
    mOnlineIcon = 1;
    mVoiceEnable = 0;
    mShowAllRegions = 0;
    mu8GSID_Autologin = 0;
    mMinPlayers = 0;
    mMaxPing = 0;
    mMapIdx = 0;
    mEraIdx = 0;
    mServerIdx = 0;

    memset(mGameNameFilter, 0, sizeof(mGameNameFilter));
    mMustBeZero4 = 0;

    mVoiceRecordDevice = 0xff;
    mVoiceRecordInput = 0xff;
    mVoicePlaybackDevice = 0xff;
}

// FUNCTION: SWBF2 0x0061cf70
void PlayerProfile::tOnlineOpt::SanityCheck()
{
    if (mMustBeZero1 || mMustBeZero2 || mMustBeZero3 || mMustBeZero4)
    {
        ResetToDefault();
    }
    if (mu8VoiceToTVVol > 10)
    {
        mu8VoiceToTVVol = 0;
    }
    if (mu8VoiceRecordVol > 10)
    {
        mu8VoiceRecordVol = 0;
    }
    if (mu8VoicePlayVol > 10)
    {
        mu8VoicePlayVol = 0;
    }
    for (unsigned int i = 0; i < sizeof(mPadding); ++i)
    {
        if (mPadding[i])
        {
            ResetToDefault();
        }
    }
}

// FUNCTION: SWBF2 0x0061cff0
void PlayerProfile::tOnlineOpt::ApplySettings()
{
    int iIndex = 0;
    for (int i = 0; i < (int)(sizeof(mProfiles) / sizeof(mProfiles[0])); ++i)
    {
        if (this == &mProfiles[i].mOnlineOptions)
        {
            iIndex = i;
            break;
        }
    }

    unsigned int uiState = GameState::GetState();
    bool bShellOrNet = uiState == 0x11E1FC01 || NetGame::IsEnabled();

    VoiceChat::Node* pNode = (VoiceChat::Node*)GameVoiceChat::FindNodeByDeviceID();
    int iPlayer = FLInputManager::s_instance.GetPlayerFromHWPort(iIndex);
    if (uiState != 0x11E1FC01)
    {
        iIndex = iPlayer < 0 ? -1 : NetGame::GetLocalIndex(iPlayer);
    }

    if (pNode && iIndex >= 0)
    {
        void* pDecoder = &pNode->GetDecoder();
        pNode->GetEncoder().SetGain(mu8VoiceRecordVol / 10.0f);
        ((VoiceChat::Decoder*)pDecoder)->SetGain(mu8VoicePlayVol / 10.0f, 0);

        if (((VoiceChat::Node*)pNode)->IsOpen())
        {
            ((GameVoiceChat::Node*)pNode)->SetID((bShellOrNet && mVoiceEnable) ? (unsigned char)iIndex : 0xFF);
        }

        if (mVoiceEnable && !((VoiceChat::Encoder*)(&pNode->GetEncoder()))->IsEnabled())
        {
            mu8VoiceToTVVol = 10;
        }

        if (((VoiceChat::Decoder*)pDecoder)->GetNumOutputs() > 1)
        {
            ((VoiceChat::Decoder*)pDecoder)->SetGain(mu8VoiceToTVVol / 10.0f, 1);
        }

        if (!mu8VoiceToTVVol && !((VoiceChat::Encoder*)(&pNode->GetEncoder()))->IsEnabled())
        {
            ((GameVoiceChat::Node*)pNode)->SetID(0xFF);
        }

        if (pNode->GetID() != 0xFF)
        {
            return;
        }
    }

    if (bShellOrNet)
    {
        mu8VoiceToTVVol = 0;
        mVoiceEnable = 0;
    }
}

// FUNCTION: SWBF2 0x0061d190
void PlayerProfile::tToolTipOpt::ResetToDefault()
{
    memset(mToolTipCount, 0, sizeof(mToolTipCount));
    mOn = false;
    mOff = false;
    memset(mPadding, 0, sizeof(mPadding));
}

// FUNCTION: SWBF2 0x0061d1b0
void PlayerProfile::tToolTipOpt::SanityCheck()
{
    for (unsigned int i = 0; i < sizeof(mPadding); ++i)
    {
        if (mPadding[i])
        {
            ResetToDefault();
        }
    }
}

// FUNCTION: SWBF2 0x0061d1d0
void PlayerProfile::tAccumPoints::ResetToDefault()
{
    mTotalPoints = 0;
    mTotalKills = 0;
    mTotalDeaths = 0;
}

// FUNCTION: SWBF2 0x0061d1f0
PlayerProfile::tProfile::tProfile()
{
    mInitialized = false;
    mVersion = 1;
    mbFakeProfile = true;
    memset(mPadding, 0, sizeof(mPadding));
}

// FUNCTION: SWBF2 0x0061d2f0
void PlayerProfile::tProfile::ResetProfile()
{
    memset(this, 0, sizeof(*this));

    mInitialized = false;

    mGameOptions.ResetToDefault();
    memset(mGameOptions.mMedalsGathered, 0, sizeof(mGameOptions.mMedalsGathered));
    memset(mGameOptions.mCampaignProgress, 0, sizeof(mGameOptions.mCampaignProgress));

    ResetBindings();

    mSoundOptions.ResetToDefault();
    mOnlineOptions.ResetToDefault();
    mToolTipOptions.ResetToDefault();
    mAccumPoints.ResetToDefault();
    mPCOptions.ResetToDefault();

    memset(mName, 0, sizeof(mName));
}

// FUNCTION: SWBF2 0x0061d380
void PlayerProfile::tProfile::ResetToDefault()
{
    ResetProfile();
    PblUnicode::ustrncpy(mName, RedLocalizeDB::Find(0x62a0afc0, false), 16);
    mName[15] = 0;
}

// FUNCTION: SWBF2 0x0061d480
void PlayerProfile::tProfile::NoteBindingChange()
{
    mIsDirty |= mGameOptions.NoteBindingChange();
}

// FUNCTION: SWBF2 0x0061d4a0
void PlayerProfile::tProfile::ResetBindings()
{
    for (int i = 0; i < 5; ++i)
    {
        mIsDirty |= mGameOptions.ResetBindings(i);
    }
}

// FUNCTION: SWBF2 0x0061d4c0
void PlayerProfile::tProfile::SanityCheck()
{
    if (mMustBeZero1)
    {
        ResetToDefault();
    }

    if (!PblUnicode::ustrlen(mName))
    {
        PblUnicode::ustrncpy(mName, RedLocalizeDB::Find(0x62a0afc0, false), 16);
        mName[15] = 0;
    }
    if (!PblUnicode::ustrlen(mName))
    {
        mName[0] = 'A';
        mName[1] = 0;
    }
    mName[15] = 0;

    mSoundOptions.SanityCheck();
    mOnlineOptions.SanityCheck();
    mToolTipOptions.SanityCheck();
    mPCOptions.SanityCheck();
    if (mSafeMode)
    {
        mSoundOptions.EnsureIsSafe();
        mPCOptions.EnsureIsSafe();
    }

    for (unsigned int i = 0; i < sizeof(mPadding); ++i)
    {
        if (mPadding[i])
        {
            ResetToDefault();
        }
    }

    UpdateProfCRC();
}

// FUNCTION: SWBF2 0x0061d230
unsigned int PlayerProfile::tProfile::CalcProfCRC()
{
    return CalcCRC(this, offsetof(tProfile, mCRC), 0);
}

// FUNCTION: SWBF2 0x0061d250
void PlayerProfile::tProfile::UpdateProfCRC()
{
    bool bDirty = mIsDirty;
    unsigned int uiOldCRC = mCRC;
    mIsDirty = false;
    mCRC = CalcProfCRC();
    if (mCRC != uiOldCRC)
    {
        bDirty = true;
    }
    mIsDirty = bDirty;
    if (mbFakeProfile)
    {
        mIsDirty = false;
    }

    if (IsDefaultProfile() && !mbFakeProfile)
    {
        memcpy(&mDefaultProfile, this, sizeof(tProfile));
    }
}

// FUNCTION: SWBF2 0x0061d2c0
bool PlayerProfile::tProfile::IsDefaultProfile()
{
    return mName[0] && !PblUnicode::ustrcmp(mName, mDefaultProfile.mName);
}

// GLOBAL: SWBF2 0x01fac0e0

unsigned short sProfileViewportName[40];

// FUNCTION: SWBF2 0x0061d730
const unsigned short* PlayerProfile::tProfile::GetName()
{
    if (!NetComm::IsLAN())
    {
        int iViewport;
        for (iViewport = 0; iViewport < 1; ++iViewport)
        {
            if (this == &mProfiles[iViewport])
            {
                break;
            }
        }

        const char* pcName = NetMatch::GetLoginOverrideName(iViewport);
        if (pcName)
        {
            memset(sProfileViewportName, 0, sizeof(sProfileViewportName));
            PblUnicode::ustrncnv(sProfileViewportName, pcName, 32);
            return sProfileViewportName;
        }
    }
    return mName;
}

// FUNCTION: SWBF2 0x0061d590
void PlayerProfile::tProfile::ApplySettings()
{
    if (this == &mProfiles[0])
    {
        int iPort = -1;
        const char* pRecord = (const char*)&PlayerPrefs::mAllUserPrefs[0].mJoystick;
        for (int i = 0; i < 1; ++i, pRecord += 0x2c)
        {
            if (*(const int*)pRecord == 0)
            {
                iPort = i;
            }
        }
        mGameOptions.ApplySettings(iPort, 0);
    }

    mSoundOptions.ApplySettings();
    mOnlineOptions.ApplySettings();
    mPCOptions.ApplySettings();
}

// FUNCTION: SWBF2 0x0061d670
PlayerProfile::tProfile* PlayerProfile::GetProfile(unsigned int uiIndex)
{
    if (uiIndex >= sizeof(mProfiles) / sizeof(mProfiles[0]))
    {
        return 0;
    }

    return &mProfiles[uiIndex];
}

// FUNCTION: SWBF2 0x0061d5f0
PlayerProfile::tProfile* PlayerProfile::GetCurProfile()
{
    if ((unsigned int)PlayerProfile::mCurProfile >= sizeof(mProfiles) / sizeof(mProfiles[0]))
    {
        return 0;
    }

    return &mProfiles[PlayerProfile::mCurProfile];
}

// FUNCTION: SWBF2 0x0061d610
unsigned int PlayerProfile::GetCurProfileIndex()
{
    return PlayerProfile::mCurProfile;
}

// FUNCTION: SWBF2 0x0061d620
bool PlayerProfile::SetCurProfile(unsigned int uiIndex)
{
    PlayerProfile::mCurProfile = uiIndex;

    tProfile* pProfile = GetCurProfile();
    if (!pProfile)
    {
        return true;
    }

    pProfile->ApplySettings();

    bool bDirty = pProfile->mIsDirty;
    pProfile->mIsDirty = false;
    unsigned int uiCRC = pProfile->CalcProfCRC();
    pProfile->mIsDirty = bDirty;

    return uiCRC == pProfile->mCRC;
}

// FUNCTION: SWBF2 0x0061d6c0
int PlayerProfile::GetNumMemoryProfiles()
{
    return mNumMemoryProfiles;
}

// FUNCTION: SWBF2 0x0061d6d0
void PlayerProfile::SetNumMemoryProfiles(int iNum)
{
    mNumMemoryProfiles = iNum;
}

// FUNCTION: SWBF2 0x0061d690
void PlayerProfile::ResetAllProfiles()
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(0);

    pProfile->ResetProfile();
    pProfile->mIsDirty = false;

    PlayerProfile::mCurProfile = 0;
    mNumMemoryProfiles = 0;
}

// FUNCTION: SWBF2 0x0061d3b0
void PlayerProfile::tProfile::InitProfile(unsigned short const* pusName)
{
    mInitialized = true;
    if (pusName)
    {
        PblUnicode::ustrncpy(mName, pusName, 16);
        mName[15] = 0;
        mbFakeProfile = false;
    }
    else
    {
        memset(mName, 0, 16);
        mbFakeProfile = true;
    }

    memset(mGameOptions.mMedalsGathered, 0, sizeof(mGameOptions.mMedalsGathered));
    memset(mGameOptions.mCampaignProgress, 0, sizeof(mGameOptions.mCampaignProgress));
    mGameOptions.ResetToDefault();
    ResetBindings();
    mSoundOptions.ResetToDefault();
    mOnlineOptions.ResetToDefault();
    mToolTipOptions.ResetToDefault();
    mAccumPoints.ResetToDefault();
    mPCOptions.ResetToDefault();

    mMustBeZero1 = 0;
    mIsDirty = false;
    mCRC = CalcProfCRC();
    mIsDirty = mName[0] != 0;
}

namespace PlayerProfile {

// FUNCTION: SWBF2 0x0061d850
int GetDefaultProfileIdx()
{
    if (!mDefaultProfile.mName[0])
    {
        return -1;
    }
    for (int i = 0; i < 1; ++i)
    {
        if (mProfiles[i].IsDefaultProfile())
        {
            return i;
        }
    }
    return -1;
}
}

namespace PlayerProfile {

// FUNCTION: SWBF2 0x0061d8a0
bool tProfile::IsDirty(bool bArg)
{
    for (int i = 0; i < 1; ++i)
    {
        if (this != &mProfiles[i])
        {
            continue;
        }
        if (PblBuildType::gCreateMemoryProfile)
        {
            return false;
        }
        if (bArg)
        {
            return mIsDirty;
        }
        if (!mIsDirty || mLastDirtyCRC[i] == mCRC)
        {
            return false;
        }
        mLastDirtyCRC[i] = mCRC;
        return true;
    }
    return false;
}
}

// FUNCTION: SWBF2 0x006c5b00
void RedJoystick::SetDeadZoneScale(float f)
{
    mDeadZoneScale = f;
    pc_device->EnumObjects(EnumObjectsCallback, this, 0);
}

// GLOBAL: SWBF2 0x00788140

static const float sDeadZoneScaleByLevel[11] =
{
    0.0f, 0.002f, 0.004f, 0.006f, 0.008f, 0.01f, 0.05f, 0.1f, 0.15f, 0.2f, 0.3f
};

// FUNCTION: SWBF2 0x00415c90
void RawControllerInputs::ApplyDeadZone(int iLevel)
{
    if (iLevel < 0)
    {
        iLevel = 0;
    }
    else if (iLevel > 10)
    {
        iLevel = 10;
    }

    float fValue = sDeadZoneScaleByLevel[iLevel];
    RedJoystick** ppObjects = _redJoystick;
    for (int i = 0; i < 10; ++i)
    {
        if (ppObjects[i])
        {
            ppObjects[i]->SetDeadZoneScale(fValue);
        }
    }
}
