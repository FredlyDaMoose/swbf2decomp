#include "sndsourcetransport.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00736530
SourceTransport::PlayState SourceTransport::GetCurrentPlayState() const
{

    if (mFlags & flags_playing)
    {
        return playState_playing;
    }

    if (mFlags & flags_paused)
    {
        return playState_paused;
    }

    if (mFlags & flags_pausing)
    {
        return playState_pausing;
    }

    return (PlayState)((mFlags & flags_stopped) >> 4);
}

// FUNCTION: SWBF2 0x00736560
SourceTransport::PlayState SourceTransport::GetPlayState() const
{

    PlayState eState = GetCurrentPlayState();

    PlayState eDesired;
    if (mFlags & flags_updatePlay)
    {
        eDesired = playState_playing;
    }
    else if (mFlags & flags_updatePause)
    {
        eDesired = playState_paused;
    }
    else
    {
        eDesired = (PlayState)((mFlags & flags_updateStop) * 2);
    }

    if (mFlags & flags_returnCurrentPlayState)
    {
        return eState;
    }

    if (eDesired != playState_invalid)
    {
        eState = eDesired;
    }

    return eState;
}

// FUNCTION: SWBF2 0x007365c0
SourceTransport::PlayState SourceTransport::QueryDesiredPlayState() const
{

    if (mFlags & flags_updatePlay)
    {
        return playState_playing;
    }

    if (mFlags & flags_updatePause)
    {
        return playState_paused;
    }

    return (PlayState)((mFlags & flags_updateStop) * 2);
}

// FUNCTION: SWBF2 0x007365f0
void SourceTransport::SetDesiredPlayState(PlayState ePlayState)
{

    mFlags = (unsigned short)(mFlags & ~(unsigned int)flags_updateTransport);

    switch (ePlayState)
    {
    case playState_playing:
        mFlags = (unsigned short)(mFlags | (unsigned int)flags_updatePlay);
        break;

    case playState_pausing:
    case playState_paused:
        mFlags = (unsigned short)(mFlags | (unsigned int)flags_updatePause);
        break;

    case playState_stopped:
        mFlags = (unsigned short)(mFlags | (unsigned int)flags_updateStop);
        break;

    default:

        break;
    }
}

// FUNCTION: SWBF2 0x00736650
void SourceTransport::SetPlayState(PlayState ePlayState)
{

    mFlags = (unsigned short)(mFlags & ~(unsigned int)flags_transport);

    switch (ePlayState)
    {
    case playState_playing:
        mFlags = (unsigned short)(mFlags | (unsigned int)flags_playing);
        break;

    case playState_pausing:
        mFlags = (unsigned short)(mFlags | (unsigned int)flags_pausing);
        break;

    case playState_paused:
        mFlags = (unsigned short)(mFlags | (unsigned int)flags_paused);
        break;

    case playState_stopped:
        mFlags = (unsigned short)(mFlags | (unsigned int)flags_stopped);
        break;

    default:
        break;
    }
}

}
