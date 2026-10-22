#pragma once

#include "sndenvelope.h"

namespace Snd
{

class Fader : public Envelope
{
public:

    Fader() : mTimeElasped(0.0f), mGain(1.0f) {}

    void Fade(float fLength, float fEndLevel, float fStartLevel);

    bool IsActive() const { return mTimeElasped < mLength; }

    void Stop()
    {
        mStartLevel = mGain;
        mEndLevel = 1.0f;
        mLength = 0.0f;
        mGain = 1.0f;
        mTimeElasped = 0.0f;
    }

protected:

    void Update(float fDeltaTime)
    {
        mGain = GetLevel(mTimeElasped);
        mTimeElasped += fDeltaTime;
    }

public:

    float mTimeElasped;
    float mGain;
};

 
 

}
