#pragma once

class PblFileChunk;

namespace Snd
{

class I3DL2Reverb
{
public:

    enum Flags
    {
        flags_roomGainChanged = 1,
        flags_roomHFGainChanged = 2,
        flags_roomRollOffChanged = 4,
        flags_decayTimeChanged = 8,
        flags_decayHFRatioChanged = 16,
        flags_erGainChanged = 32,
        flags_erDelayChanged = 64,
        flags_reverbGainChanged = 128,
        flags_reverbDelayChanged = 256,
        flags_diffusionChanged = 512,
        flags_densityChanged = 1024,
        flags_hrReferenceChanged = 2048
    };

    I3DL2Reverb();

    bool Read(PblFileChunk& chunk, unsigned int uiPropertyID);

    void SetRoomGain(float f) { mRoomGain = f; mFlags |= flags_roomGainChanged; }
    void SetRoomHFGain(float f) { mRoomHFGain = f; mFlags |= flags_roomHFGainChanged; }
    void SetERGain(float f) { mERGain = f; mFlags |= flags_erGainChanged; }
    void SetReverbGain(float f) { mReverbGain = f; mFlags |= flags_reverbGainChanged; }

    float mRoomGain;
    float mRoomHFGain;
    float mRoomRollOff;
    float mDecayTime;
    float mDecayHFRatio;
    float mERGain;
    float mERDelay;
    float mReverbGain;
    float mReverbDelay;
    float mDiffusion;
    float mDensity;
    float mHFReference;
    unsigned short mFlags;
};
 

}
