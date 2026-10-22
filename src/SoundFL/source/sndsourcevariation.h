#pragma once

#include "sndsourcetransportcontrol.h"

namespace Snd
{

class SourceVariation : public SourceTransportControl
{
public:
    SourceVariation();

    float CalculateGain(float fGain) const;
    float CalculatePitch(float fPitch) const;

    float mGainDev;
    float mPitchDev;

protected:
    bool Read(PblFileChunk& chunk, unsigned int uiPropertyID);
};

 

}
