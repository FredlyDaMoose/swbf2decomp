#include "EndDisplay.h"
#include "CameraManager.h"
#include "Character.h"
#include "FLInputManager.h"
#include "GameLoop.h"
#include "GameMovie.h"
#include "GameSoundEngine.h"
#include "Music.h"
#include "NetGame.h"
#include "ScreenTransition.h"
#include "Team.h"
#include "VoiceOverManager.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "SoundFL/source/sndbus.h"
#include "SoundFL/source/win/sndstream.h"

#include <string.h>
#include <windows.h>

namespace FLRenderer
{
void RemoveInterfaceElement(unsigned int uiView, RedScreenGroupElement* pGroup);
void SetTransitionOverlayColor(int iView, RedColor const& rColor);
void SetSceneEnable(unsigned int uiView, bool bEnable);
void SetInterfaceEnable(unsigned int uiView, bool bEnable);

void AddInterfaceElement(unsigned int hView, RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
}

namespace GameLoop { extern bool mbHistoricalRules; }

// GLOBAL: SWBF2 0x01e3094c
EndDisplay* EndDisplay::sInstance;

// GLOBAL: SWBF2 0x007e6322
bool EndDisplay::sCanRender = true;

// GLOBAL: SWBF2 0x007eb7e4
static RedColor sColor[2] = { { 0xc0, 0x40, 0x40, 0xff }, { 0x01, 0x56, 0xd5, 0xff } };
// GLOBAL: SWBF2 0x00788f00
static float sTexcoords[4][2] = { { 0.0f, 0.5f }, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.5f } };

// FUNCTION: SWBF2 0x00490b40
void EndDisplay::Create()
{
    sInstance = new EndDisplay;
}

// FUNCTION: SWBF2 0x00490bb0
EndDisplay::EndDisplay()
{
    sCanRender = true;
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        FLRenderer::AddInterfaceElement(CameraManager::sInstance->m_hView[i], &mEndGroup[i], 0.5f, 0.5f,
                                        true);
        mEndText[i].Enable(false);
        mEndGroup[i].AddElement(&mEndText[i]);
    }
    mDisplayed = false;
    mMoviePlayed = false;
    mTransition = false;
    mHideIt = false;
    mMinDisplayTime = 0.0f;
    mMovieFileName[0] = 0;
    mMovieIdName[0] = 0;
    ActivateThread(0, PER_FRAME);
}

// FUNCTION: SWBF2 0x00490d30
EndDisplay::~EndDisplay()
{
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        FLRenderer::RemoveInterfaceElement(CameraManager::sInstance->m_hView[i], &mEndGroup[i]);
        mEndGroup[i].RemoveAllElements();
    }
    DeactivateThread();
}

// FUNCTION: SWBF2 0x00490e10
void EndDisplay::Init()
{
    RedBitmapElement* pText = mEndText;
    pText->Enable(false);
    pText->SetTexture(0x0623b25a);
    pText->SetRect(256.0f, 64.0f, RedBitmapElement::BITMAP_HALIGN_CENTER,
                   RedBitmapElement::BITMAP_VALIGN_CENTER);
}

// FUNCTION: SWBF2 0x00490e50
void EndDisplay::Show(unsigned int uiWinners, float fMinDisplayTime)
{
    bool bFirstShow = !mDisplayed;
    mDisplayed = true;
    mHideIt = false;
    if (!uiWinners)
    {
        return;
    }

    GameLoop::SetLastGameWinner((uiWinners & 2) ? 1 : 2);
    if (NetGame::IsEnabled() && NetGame::IsHost() && NetGame::IsDedicated())
    {
        mMinDisplayTime = fMinDisplayTime;
        return;
    }

    int iTeam = VoiceOverManager::sInstance->GetPlayerTeamNumber();
    bool bWon = ((uiWinners >> (iTeam + 1)) & 1) != 0;
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        RedBitmapElement* pText = &mEndText[i];
        pText->Enable(false);
    }
    if (!sCanRender)
    {
        return;
    }

    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        Character* pPlayer = NetGame::GetLocalPlayer(i);
        if (pPlayer)
        {
            int iWon = (uiWinners >> pPlayer->mTeamNumber) & 1;
            RedBitmapElement* pText = &mEndText[i];
            pText->Enable(true);
            pText->SetColor(sColor[iWon]);
            pText->SetTexCoords(sTexcoords[iWon * 2][0], sTexcoords[iWon * 2][1],
                                sTexcoords[iWon * 2 + 1][0], sTexcoords[iWon * 2 + 1][1], false);
        }
    }
    if (bFirstShow)
    {
        Snd::Bus* pBus = Snd::Bus::FindByHashID(0xeceb308a);
        if (pBus)
        {
            pBus->SetGain(0.0f, true);
        }
        GameSoundEngine::StopAll();
        mVictoryDefeat[iTeam][!bWon].Play();
        mTransition = false;
    }
    mMinDisplayTime = 7.6f;
}

// FUNCTION: SWBF2 0x00491050
void EndDisplay::Hide()
{
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        RedBitmapElement* pText = &mEndText[i];
        pText->Enable(false);
    }
    if (mHideIt)
    {
        for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
        {
            RedColor clear = { 0, 0, 0, 0 };
            FLRenderer::SetTransitionOverlayColor(i, clear);
        }
    }
    mMinDisplayTime = 0.0f;
    Snd::Bus* pBus = Snd::Bus::FindByHashID(0x985c8f54);
    if (pBus)
    {
        pBus->SetGain(0.0f, true);
    }
    if (mMoviePlayed && GameMovie::IsPlaying())
    {
        FLRenderer::SetSceneEnable(0, true);
    }
    Snd::Stream::EnableStreaming(false);
}

// FUNCTION: SWBF2 0x00491120
void EndDisplay::HideCallback(void* pData)
{
    EndDisplay* pThis = (EndDisplay*)pData;
    pThis->mHideIt = true;
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        RedColor black = { 0, 0, 0, 0xff };
        FLRenderer::SetTransitionOverlayColor(i, black);
    }
}

// FUNCTION: SWBF2 0x00491170
bool EndDisplay::Update(float fDeltaTime)
{
    if (!mDisplayed)
    {
        return true;
    }

    mMinDisplayTime -= fDeltaTime;
    if (!mTransition && mMinDisplayTime <= 0.0f)
    {
        for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
        {
            ScreenTransition::sInstance->BeginTransition(i, 1.0f, 0.0001f, 0.0f,
                                                         ScreenTransition::FADE,
                                                         ScreenTransition::FADE, HideCallback,
                                                         this);
        }
        GameSoundEngine::StopAll();
        GameSoundEngine::Update(0.0001f);
        Snd::Stream::EnableStreaming(false);
        mTransition = true;
    }
    Music::UpdateAll(fDeltaTime);

    if (mMinDisplayTime <= 0.0f && !mMoviePlayed && mHideIt && mMovieFileName[0] &&
        mMovieIdName[0] && (Team::GetWinningTeams() & 2) && GameLoop::mbHistoricalRules)
    {
        FLInputManager::s_instance.StopAllRumble();
        GameSoundEngine::StopAll();
        GameSoundEngine::Update(0.0001f);
        Snd::Stream::EnableStreaming(false);
        GameMovie::StartInGameMoviePlay(mMovieFileName, mMovieIdName);
        mMoviePlayed = true;
    }

    if (IsReallyFinished() && (mHideIt || !ScreenTransition::sInstance))
    {
        Hide();
    }
    return true;
}

extern int g_InGameMoviePlayerState;
extern bool g_InGameMoviePlayerUpdateEnable;
extern int g_InGameMoviePlayerFrame;
extern unsigned __int64 g_InGameMovieStartTime;
// GLOBAL: SWBF2 0x01e56170

char ingame_movie_idname[256];
// GLOBAL: SWBF2 0x01e56288
char g_InGameMovieFilename[256];

// FUNCTION: SWBF2 0x00534bd0
void GameMovie::StartInGameMoviePlay(const char* pcFile, const char* pcName)
{
    if (g_InGameMoviePlayerState)
    {
        return;
    }

    g_InGameMoviePlayerUpdateEnable = true;
    g_InGameMoviePlayerState = 1;
    const unsigned int uiView = CameraManager::sInstance->m_hView[0];
    g_InGameMoviePlayerFrame = 0;
    FLRenderer::SetSceneEnable(uiView, false);
    FLRenderer::SetInterfaceEnable(uiView, false);
    strcpy(g_InGameMovieFilename, "Movies\\");
    strcat(g_InGameMovieFilename, pcFile);
    strcpy(ingame_movie_idname, pcName);
    LARGE_INTEGER start;
    QueryPerformanceCounter(&start);
    g_InGameMovieStartTime = start.QuadPart;
}

// FUNCTION: SWBF2 0x004912e0
bool EndDisplay::HasBeenDisplayed()
{
    return mDisplayed;
}

// FUNCTION: SWBF2 0x004912f0
bool EndDisplay::IsReallyFinished()
{
    return HasBeenDisplayed() && (mMinDisplayTime <= 0.0f || IsMusicFinished()) &&
           !GameMovie::IsPlaying();
}

// FUNCTION: SWBF2 0x00491320
bool EndDisplay::IsFinished()
{
    if (!IsReallyFinished())
    {
        return false;
    }
    bool finished = true;
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        finished &= !mEndText[i].IsEnabled();
    }
    return finished;
}

// FUNCTION: SWBF2 0x00491360
bool EndDisplay::IsMusicFinished() const
{
    Snd::Bus* bus = Snd::Bus::FindByHashID(0x9f9c4fd4);
    if (bus && bus->mFinalGain == 0.0f)
    {
        return true;
    }
    for (unsigned int team = 0; team < 2; ++team)
    {
        for (unsigned int outcome = 0; outcome < 2; ++outcome)
        {
            if (mVictoryDefeat[team][outcome].GetPlayState() == GameMusic::playState_playing ||
                mVictoryDefeat[team][outcome].GetCurrentPlayState() == GameMusic::playState_playing)
            {
                return false;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00491400
void EndDisplay::SetVictoryMusic(unsigned int uTeam, const char* pName)
{
    mVictoryDefeat[uTeam][0].SetID(pName);
}

// FUNCTION: SWBF2 0x00491420
void EndDisplay::SetDefeatMusic(unsigned int uTeam, const char* pName)
{
    mVictoryDefeat[uTeam][1].SetID(pName);
}
