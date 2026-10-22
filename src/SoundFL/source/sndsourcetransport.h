#pragma once

namespace Snd
{

class SourceTransport
{
public:
    enum Flags
    {
        flags_updatePlay = 1,
        flags_updateStop = 2,
        flags_updatePause = 4,
        flags_updateTransport = 7,
        flags_playing = 8,
        flags_paused = 16,
        flags_pausing = 32,
        flags_stopped = 64,
        flags_transport = 120,
        flags_updatePlaybackPosition = 128,
        flags_returnCurrentPlayState = 256
    };

    enum PlayState
    {
        playState_invalid = 0,
        playState_playing = 1,
        playState_pausing = 2,
        playState_paused = 3,
        playState_stopped = 4
    };

    PlayState GetCurrentPlayState() const;

    PlayState GetPlayState() const;

    void Play(bool  )
    {
        mFlags = (unsigned short)((mFlags
                                   & ~(unsigned int)(flags_updateStop
                                                     | flags_updatePause))
                                  | (unsigned int)flags_updatePlay);

        if ((mFlags & flags_updatePlaybackPosition) == 0)
        {
            mPlaybackPosition = 0;
            mFlags = (unsigned short)(mFlags
                                      | (unsigned int)
                                            flags_updatePlaybackPosition);
        }
    }

    void Pause()
    {
        mFlags = (unsigned short)((mFlags
                                   & ~(unsigned int)(flags_updatePlay
                                                     | flags_updateStop))
                                  | (unsigned int)flags_updatePause);
    }

    void Stop()
    {
        mFlags = (unsigned short)((mFlags
                                   & ~(unsigned int)(flags_updatePlay
                                                     | flags_updatePause))
                                  | (unsigned int)flags_updateStop);
    }

protected:

    PlayState QueryDesiredPlayState() const;

    PlayState GetDesiredPlayState()
    {
        const PlayState ePlayState = QueryDesiredPlayState();
        mFlags = (unsigned short)(mFlags & ~(unsigned int)flags_updateTransport);
        return ePlayState;
    }

    bool GetPlaybackPositionChanged()
    {
        const bool bChanged = (mFlags & flags_updatePlaybackPosition) != 0;
        mFlags = (unsigned short)(mFlags & ~(unsigned int)flags_updatePlaybackPosition);
        return bChanged;
    }

public:

    unsigned int GetPlaybackPosition() const { return mPlaybackPosition; }
    void SetPlaybackPosition(unsigned int uiPosition)
    {
        mPlaybackPosition = uiPosition;
        mFlags = (unsigned short)(mFlags | flags_updatePlaybackPosition);
    }

protected:

    void SetDesiredPlayState(PlayState ePlayState);

    void SetPlayState(PlayState ePlayState);

public:
    unsigned int mPlaybackPosition;
    unsigned short mFlags;
};

 

}
