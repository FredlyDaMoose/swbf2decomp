#include "GameSoundParameterized.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "SoundFL/source/sndsoundparameterized.h"

// FUNCTION: SWBF2 0x00539110
GameSoundParameterized::GameSoundParameterized()
{
    mProps = 0;
}

// FUNCTION: SWBF2 0x00539120
void GameSoundParameterized::SetID(const char* pName)
{
    if (pName)
    {
        mProps = Snd::SoundParameterized::Properties::FindByHashID(PblHash(pName));
    }
}

// FUNCTION: SWBF2 0x00539150
GameSoundParameterized& GameSoundParameterized::operator=(const GameSoundParameterized& other)
{
    mProps = other.mProps;
    return *this;
}
