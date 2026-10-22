#pragma once

#include <stddef.h>

class GameSoundParameterized
{
public:

    GameSoundParameterized();

    void SetID(const char* pName);

    GameSoundParameterized& operator=(const GameSoundParameterized& other);

    void* mProps;
};
 
 
