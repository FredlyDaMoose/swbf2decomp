#pragma once

class PblFileChunk;

namespace Snd
{

class SourceTransportControl
{
public:
    SourceTransportControl();

    bool DeterminePlaySource(bool bArg) const;

    bool GetRandomPlayPosition() const { return mRandomPlaybackPosition == 1; }
    void SetRandomPlayPosition(bool b) { mRandomPlaybackPosition = b; }

    float mPlayProbability;
    float mPlayInterval;
    float mPlayIntervalDev;
    float mPlayIntervalNext;
    unsigned char mRandomPlaybackPosition;

protected:
    bool Read(PblFileChunk& chunk, unsigned int uiPropertyID);
};

 

}
