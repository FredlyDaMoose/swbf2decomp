#pragma once

class PblFileChunk;

namespace Snd
{

class Sample;
class Source2D3D;
class Source3D;
class SourceI3DL2;
class SourceTransport;

class SourceVirtual
{
public:
    enum Flags
    {
        flags_PriorityChanged = 1,
        flags_BiasChanged = 2
    };

    SourceVirtual()
        : mPriority(1.0f),
          mBias(0.5f)
    {
    }

    void SetPriority(float fPriority)
    {
        mFlags |= flags_PriorityChanged;
        mPriority = fPriority;
    }
    void SetBias(float fBias)
    {
        mFlags |= flags_BiasChanged;
        mBias = fBias;
    }

    float mPriority;
    float mBias;
    unsigned char mFlags;

protected:
    bool Read(PblFileChunk& chunk, unsigned int uiPropertyID);

    bool GetPriorityChanged()
    {
        if (!(mFlags & flags_PriorityChanged))
        {
            return false;
        }
        mFlags &= (unsigned char)~(unsigned int)flags_PriorityChanged;
        return true;
    }
    bool GetBiasChanged()
    {
        if (!(mFlags & flags_BiasChanged))
        {
            return false;
        }
        mFlags &= (unsigned char)~(unsigned int)flags_BiasChanged;
        return true;
    }

public:

    float CalculatePriority(unsigned char ePlayState, float fArg, bool bLooping,
                            float fBusGain) const;
    float CalculatePriority(const SourceTransport* pTransport, const Source2D3D* pSource2D3D,
                            const Source3D* pSource3D, const SourceI3DL2* pSourceI3DL2,
                            const Sample* pSample, float fScale, bool bArg) const;
};

 

}
