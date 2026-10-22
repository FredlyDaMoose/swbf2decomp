#include "GameMusic.h"

#include "GameLoop.h"
#include "GameState.h"
#include "Music.h"

// FUNCTION: SWBF2 0x00534ea0
GameMusic::GameMusic()
{
    mMusic = 0;
    mPlayState = 0;
}

// FUNCTION: SWBF2 0x00534eb0
GameMusic::~GameMusic()
{
}

// FUNCTION: SWBF2 0x00534ec0
void GameMusic::Play()
{
    if (GameLoop::IsGameOver() && GameState::GetState() == 0x11E1FC01)
    {
        return;
    }
    if (mMusic && (mPlayState == playState_stopped ||
                   mMusic->GetCurrentPlayState() == Music::playState_stopped))
    {
        mMusic->Play();
        mPlayState = playState_playing;
    }
}

// FUNCTION: SWBF2 0x00534f00
void GameMusic::Stop()
{
    if (mMusic && mPlayState == playState_playing)
    {
        mMusic->Stop();
        mPlayState = playState_stopped;
    }
}

// FUNCTION: SWBF2 0x00534f20
GameMusic::PlayState GameMusic::GetCurrentPlayState() const
{
    if (mMusic)
    {
        switch (mMusic->GetCurrentPlayState())
        {
        case Music::playState_starting:
        case Music::playState_playing:
            return playState_playing;
        case Music::playState_stopping:
        case Music::playState_stopped:
        case Music::playState_pausing:
        case Music::playState_paused:
            return playState_stopped;
        }
    }
    return (PlayState)mPlayState;
}

// FUNCTION: SWBF2 0x00534f70
void GameMusic::SetID(const char* pcID)
{
    Stop();
    mMusic = Music::FindByID(pcID);
    mPlayState = mMusic ? playState_stopped : playState_invalid;
}

// FUNCTION: SWBF2 0x00534fa0
bool GameMusic::ReadyToPlay() const
{
    return mMusic && mMusic->GetInactiveTimeLeft() <= 0.0f;
}

// FUNCTION: SWBF2 0x00534fc0
GameMusic& GameMusic::operator=(const GameMusic& rhs)
{
    mMusic = rhs.mMusic;
    mPlayState = rhs.mPlayState;
    return *this;
}
