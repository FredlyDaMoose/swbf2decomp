#pragma once

#include <stddef.h>

namespace HUD {
class Envelope
{
public:

    enum Phase
    {
        phase_Inactive = 0,
        phase_Attack = 1,
        phase_Hold = 2,
        phase_Sustain = 3,
        phase_Release = 4,
        phase_MAX = 5
    };

    Envelope();
    Phase Update(float fTime);
    float CalculateValue() const;
    Envelope& operator=(const Envelope& other);

    void Start();
    void Stop();

    // FUNCTION: SWBF2 0x0055da60
    Phase GetPhase() const { return mPhase; }
    float GetElapsed() const { return mTime; }
    // FUNCTION: SWBF2 0x0055daf0
    float GetPhaseTime(Phase ePhase) const { return mPhaseTime[ePhase]; }

    // FUNCTION: SWBF2 0x0055dae0
    void SetPhaseTime(Phase ePhase, float fTime) { mPhaseTime[ePhase] = fTime; }

    float mPhaseTime[phase_MAX];
    float mTime;
    Phase mPhase;
};
}

 
 
