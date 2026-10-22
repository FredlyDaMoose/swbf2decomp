#include "HUDEnvelope.h"

#include <float.h>

// FUNCTION: SWBF2 0x0055d900
HUD::Envelope::Envelope()
    : mTime(0.0f),
      mPhase(phase_Inactive)
{
    for (int i = 0; i < phase_MAX; ++i)
    {
        mPhaseTime[i] = 0.0f;
    }
    mPhaseTime[phase_Hold] = FLT_MAX;
}

// FUNCTION: SWBF2 0x0055d930
void HUD::Envelope::Start()
{
    switch (mPhase)
    {
    case phase_Inactive:
        mTime = 0.0f;
        mPhase = phase_Attack;
        break;
    case phase_Attack:
        break;
    case phase_Hold:
        mTime = 0.0f;
        break;
    case phase_Sustain:
        mTime = 0.0f;
        mPhase = phase_Hold;
        break;
    case phase_Release:
        mTime = mPhaseTime[phase_Release] != 0.0f
                    ? mTime / mPhaseTime[phase_Release] * mPhaseTime[phase_Attack]
                    : 0.0f;
        mPhase = phase_Attack;
        break;
    }
}

// FUNCTION: SWBF2 0x0055d9b0
void HUD::Envelope::Stop()
{
    switch (mPhase)
    {
    case phase_Attack:
        mTime = mPhaseTime[phase_Attack] != 0.0f
                    ? mTime / mPhaseTime[phase_Attack] * mPhaseTime[phase_Release]
                    : 0.0f;
        mPhase = phase_Release;
        break;
    case phase_Hold:
        mTime = 0.0f;
        mPhase = phase_Sustain;
        break;
    }
}

// FUNCTION: SWBF2 0x0055d9f0
HUD::Envelope::Phase HUD::Envelope::Update(float fTime)
{
    while (mPhase > phase_Inactive &&
           (mPhaseTime[mPhase] == 0.0f || mTime > mPhaseTime[mPhase]))
    {
        mTime -= mPhaseTime[mPhase];
        mPhase = (Phase)(mPhase + 1);
        if (mPhase >= phase_MAX)
        {
            mPhase = phase_Inactive;
        }
    }
    mTime += fTime;
    return mPhase;
}

// FUNCTION: SWBF2 0x0055da70
float HUD::Envelope::CalculateValue() const
{
    switch (mPhase)
    {
    case phase_Attack:
        if (mPhaseTime[phase_Attack] > mTime)
        {
            return mTime / mPhaseTime[phase_Attack];
        }
        return 1.0f;
    case phase_Hold:
    case phase_Sustain:
        return 1.0f;
    case phase_Release:
        if (mPhaseTime[phase_Release] > mTime)
        {
            return 1.0f - mTime / mPhaseTime[phase_Release];
        }
        return 0.0f;
    default:
        return 0.0f;
    }
}

// FUNCTION: SWBF2 0x0055db00
HUD::Envelope& HUD::Envelope::operator=(const Envelope& other)
{
    for (int i = 0; i < phase_MAX; ++i)
    {
        mPhaseTime[i] = other.mPhaseTime[i];
    }
    return *this;
}
