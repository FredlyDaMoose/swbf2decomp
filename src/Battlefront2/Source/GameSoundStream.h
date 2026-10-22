#pragma once

#include "GameSound.h"
#include "SoundFL/source/sndsoundstream.h"

class GameSoundStream : public GameSound
{
public:
    GameSoundStream();

    // FUNCTION: SWBF2 0x0044de60
    ~GameSoundStream() {}

    const Snd::SoundStream::Properties* GetProperties(bool);
};
 
