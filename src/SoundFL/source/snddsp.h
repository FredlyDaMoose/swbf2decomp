#pragma once

namespace Snd
{
namespace DSP
{

class HighLowPassFilter
{
public:

    enum Mode
    {
        mode_Invalid  = 0,
        mode_LowPass  = 1,
        mode_HighPass = 2
    };

    struct ChannelState
    {
        float mInDelay[2];
        float mOutDelay[2];
        float r;
        float c;
        float a1;
        float a2;
        float a3;
        float b1;
        float b2;
    };

    HighLowPassFilter();

    void SetSampleRate(float fSampleRate);
    void SetMode(Mode eMode);
    void SetNumChannels(unsigned int uiChannels);
    void SetCutOffFrequency(float fCutOff);

    void ResetState();

    void Process16(const short* const pIn, unsigned int uiNumSamples,
                   short* const pOut);

private:

    void CalculateCoefficents();

public:
    float mSampleRate;
    float mCutOffFreq;
    Mode mMode;
    unsigned int mNumChannels;
    ChannelState mState[2];
};

 
 

unsigned __int64 SignalIdentity16(const short* pSamples, unsigned int uiCount);
unsigned __int64 SignalPower16(const short* pSamples, unsigned int uiCount);
unsigned int TurningPointsFrequency16(const short* pSamples, unsigned int uiCount,
                                      unsigned int uiSampleRate);

}
}
