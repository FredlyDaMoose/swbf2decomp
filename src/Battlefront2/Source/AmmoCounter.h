#pragma once

#include <stddef.h>

class AmmoCounterClass
{
public:
    AmmoCounterClass()
    {
        mRoundsPerClip = 1;
        mAmmoPerRound = 1.0f;
    }

    bool SetProperty(unsigned int uiHash, const char* pcValue);

    int mRoundsPerClip;
    float mAmmoPerRound;
};
 
 
 
