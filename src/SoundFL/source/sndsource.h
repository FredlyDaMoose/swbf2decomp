#pragma once

class PblFileChunk;

namespace Snd
{

struct GainMatrix
{
    float frontLeft;
    float frontRight;
    float rearLeft;
    float rearRight;
    float reverb;
};

class Source
{
public:
    enum Flags
    {

        flags_pitchChanged = 1,
        flags_gainChanged = 4,
        flags_reverbGainChanged = 8,

        flags_loopingChanged = 2,
        flags_looping = 16,

        flags_gainCopied = 32
    };

    Source() : mPitch(1.0f), mGain(1.0f), mReverbGain(0.0f), mFlags(0) {}

    void SetPitch(float fPitch)
    {
        mFlags |= flags_pitchChanged;
        mPitch = fPitch;
    }

    void SetGain(float fGain)
    {
        mFlags |= flags_gainChanged;
        mGain = fGain;
    }

    void SetReverbGain(float fGain)
    {
        mFlags |= flags_reverbGainChanged;
        mReverbGain = fGain;
    }

    bool GetLooping() const { return (mFlags & flags_looping) != 0; }

    bool GetGainChanged()
    {
        const bool bChanged = (mFlags & flags_gainChanged) != 0;
        mFlags &= (unsigned char)~(unsigned int)flags_gainChanged;
        return bChanged;
    }

    bool GetGainCopied()
    {
        const bool bCopied = (mFlags & flags_gainCopied) != 0;
        mFlags &= (unsigned char)~(unsigned int)flags_gainCopied;
        return bCopied;
    }
    bool GetReverbGainChanged()
    {
        const bool bChanged = (mFlags & flags_reverbGainChanged) != 0;
        mFlags &= (unsigned char)~(unsigned int)flags_reverbGainChanged;
        return bChanged;
    }

    bool GetPitchChanged()
    {
        const bool bChanged = (mFlags & flags_pitchChanged) != 0;
        mFlags &= (unsigned char)~(unsigned int)flags_pitchChanged;
        return bChanged;
    }
    void SetLooping(bool bLooping)
    {
        mFlags |= flags_loopingChanged;
        if (bLooping)
        {
            mFlags |= flags_looping;
        }
        else
        {
            mFlags &= (unsigned char)~(unsigned int)flags_looping;
        }
    }

    float mPitch;
    float mGain;
    float mReverbGain;
    unsigned char mFlags;

protected:
    bool Read(PblFileChunk& chunk, unsigned int uiPropertyID);

    void CopyParameters(const Source& rOther)
    {
        mPitch = rOther.mPitch;
        mGain = rOther.mGain;
        mReverbGain = rOther.mReverbGain;
        mFlags = (unsigned char)(rOther.mFlags | flags_reverbGainChanged | 0x20);
    }
};

 
 

}
