#include "GameSoundStream.h"

// FUNCTION: SWBF2 0x00539160
GameSoundStream::GameSoundStream()
{
}

// FUNCTION: SWBF2 0x00539170
const Snd::SoundStream::Properties* GameSoundStream::GetProperties(bool)
{
    if (GetPropertiesType() == type_soundStream)
    {
        return (const Snd::SoundStream::Properties*)GameSound::GetProperties();
    }
    return 0;
}
