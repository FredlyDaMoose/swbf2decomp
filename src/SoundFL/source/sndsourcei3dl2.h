#pragma once

class PblVector3;
class PblFileChunk;

namespace Snd
{

class Space;

class ListenerBase;

class SourceI3DL2
{
public:

    enum Flags
    {
        flags_directGain = 1,
        flags_directHFGain = 2,
        flags_roomGain = 4,
        flags_roomHFGain = 8,
        flags_roomRollOffFactor = 16,
        flags_obstructionHFGain = 32,
        flags_obstructionLFRatio = 64,
        flags_occlusionHFGain = 128,
        flags_occlusionLFRatio = 256,
        flags_changed = 511,
        flags_InsideSpace = 512,
        flags_forceUpdate = 1024
    };

    SourceI3DL2();

    float CalculateGainEstimate() const;

    bool Read(PblFileChunk& chunk, unsigned int uiPropertyID);
    void CopyParameters(const SourceI3DL2& rSource);

    bool Update(const ListenerBase* const pListener,
                const ::PblVector3* const pPos,
                float fRadius,
                bool bSetup,
                bool* const pbInsideSpace);

    Space* mSpace;
    float mDirectGain;
    float mDirectHFGain;
    float mRoomGain;
    float mRoomHFGain;
    float mRoomRollOffFactor;
    float mObstructionHFGain;
    float mObstructionLFRatio;
    float mOcclusionHFGain;
    float mOcclusionLFRatio;
    Space* mListenerSpace;
    unsigned short mFlags;

protected:

    bool GetSourceI3DL2Changed()
    {
        const bool bChanged = (mFlags & 0x1FF) != 0;
        mFlags &= 0xFE00;
        return bChanged;
    }
public:
};

class SourceI3DL2Interpolated : public SourceI3DL2
{
public:

    enum Flags
    {
        flags_interpolate = 1
    };

    SourceI3DL2Interpolated() : mFlags(0) {}

    void Update(const ListenerBase* const pListener,
                const ::PblVector3* const pPos,
                float fRadius,
                bool bInterpolate);

    SourceI3DL2 mTarget;
    unsigned char mFlags;
};

 
 

}
