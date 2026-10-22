#pragma once

#include "sndsource.h"

namespace Snd
{

class Source2D
{
public:
    enum Flags
    {
        flags_panChanged = 1
    };

    Source2D() : mPan(0.0f), mFlags(0) {}

    bool GetPanChanged()
    {
        const bool bChanged = (mFlags & flags_panChanged) != 0;
        mFlags &= (unsigned char)~(unsigned int)flags_panChanged;
        return bChanged;
    }

    void SetPan(float fPan)
    {
        mFlags |= flags_panChanged;
        mPan = fPan;
    }

protected:

    void CalculateGain(float fGain, float fReverbGain,
                       GainMatrix& rMatrix) const;

    bool Read(PblFileChunk& chunk, unsigned int uiPropertyID);

    void CopyParameters(const Source2D& rOther)
    {
        mPan = rOther.mPan;
        mFlags = rOther.mFlags;
    }

public:
    float mPan;
    unsigned char mFlags;
};

 

}
