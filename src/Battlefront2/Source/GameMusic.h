#pragma once

#include <stddef.h>

class Music;

class GameMusic
{
public:

    GameMusic();

    ~GameMusic();

    enum PlayState
    {
        playState_invalid = 0,
        playState_stopped = 1,
        playState_playing = 2,
    };

    PlayState GetPlayState() const { return (PlayState)mPlayState; }

    Music* GetMusic() const { return mMusic; }

    void Play();
    void Stop();
    PlayState GetCurrentPlayState() const;
    void SetID(const char* pcID);
    bool ReadyToPlay() const;
    GameMusic& operator=(const GameMusic& rhs);

    Music* mMusic;
    unsigned char mPlayState;
};
 
 
 
