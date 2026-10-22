#include <math.h>

#include "snddsp.h"

namespace Snd
{
namespace DSP
{

// FUNCTION: SWBF2 0x00747a80
void HighLowPassFilter::Process16(const short* const pIn, unsigned int uiNumSamples,
                                  short* const pOut)
{
    for (unsigned int channel = 0; channel < mNumChannels; ++channel)
    {
        ChannelState& state = mState[channel];
        for (unsigned int sample = channel; sample < uiNumSamples; sample += mNumChannels)
        {
            float input = (float)pIn[sample] / 32767.0f;
            float output = state.mInDelay[0] * state.a2 + input * state.a1;
            output += state.a3 * state.mInDelay[1];
            output -= state.b1 * state.mOutDelay[0];
            output -= state.b2 * state.mOutDelay[1];
            if (output < -1.0f)
            {
                output = -1.0f;
            }
            else if (output > 1.0f)
            {
                output = 1.0f;
            }
            pOut[sample] = (short)(int)(output * 32767.0f);
            state.mInDelay[1] = state.mInDelay[0];
            state.mInDelay[0] = input;
            state.mOutDelay[1] = state.mOutDelay[0];
            state.mOutDelay[0] = output;
        }
    }
}

// FUNCTION: SWBF2 0x00747cf0
unsigned int TurningPointsFrequency16(const short* pSamples, unsigned int uiCount,
                                       unsigned int uiSampleRate)
{
    short previous = 0;
    short beforePrevious = 0;
    unsigned int turns = 0;
    for (unsigned int i = 0; i < uiCount; ++i)
    {
        short current = pSamples[i];
        if ((beforePrevious <= previous && current < previous) ||
            (beforePrevious >= previous && current > previous))
        {
            ++turns;
        }
        beforePrevious = previous;
        previous = current;
    }
    return (turns >> 1) * uiSampleRate / uiCount;
}

static const float kPi = 3.1415927f;

// FUNCTION: SWBF2 0x007478b0
HighLowPassFilter::HighLowPassFilter()
    : mSampleRate(48000.0f)
    , mCutOffFreq(24000.0f)
    , mMode(mode_LowPass)
    , mNumChannels(1)
{
    mState[0].r = 1.41421356f;
    mState[1].r = 1.41421356f;
    ResetState();
    CalculateCoefficents();
}

// FUNCTION: SWBF2 0x00747920
void HighLowPassFilter::CalculateCoefficents()
{

    for (unsigned int i = 0; i < mNumChannels; ++i)
    {
        ChannelState& rState = mState[i];

        float fC;
        float fNumerator;

        if (mMode == mode_LowPass)
        {
            fC = 1.0f / (float)tan((double)(mCutOffFreq * kPi / mSampleRate));
            rState.c = fC;
            rState.a1 = 1.0f / (rState.r * fC + 1.0f + fC * fC);
            rState.a2 = rState.a1 * 2.0f;
            fNumerator = 1.0f - fC * fC;
        }
        else if (mMode == mode_HighPass)
        {
            fC = (float)tan((double)(mCutOffFreq * kPi / mSampleRate));
            rState.c = fC;
            rState.a1 = 1.0f / (rState.r * fC + 1.0f + fC * fC);
            rState.a2 = rState.a1 * -2.0f;
            fNumerator = fC * fC - 1.0f;
        }
        else
        {
            continue;
        }

        rState.a3 = rState.a1;
        rState.b1 = fNumerator * 2.0f * rState.a1;
        rState.b2 = (1.0f - fC * rState.r + fC * fC) * rState.a1;
    }
}

// FUNCTION: SWBF2 0x00747c10
void HighLowPassFilter::SetSampleRate(float fSampleRate)
{
    mSampleRate = fSampleRate;
}

// FUNCTION: SWBF2 0x00747bf0
void HighLowPassFilter::SetMode(Mode eMode)
{
    mMode = eMode;
}

// FUNCTION: SWBF2 0x00747c00
void HighLowPassFilter::SetNumChannels(unsigned int uiChannels)
{
    mNumChannels = uiChannels;
}

// FUNCTION: SWBF2 0x00747bc0
void HighLowPassFilter::SetCutOffFrequency(float fCutOff)
{
    const float fLimit = mSampleRate * 0.5f;

    mCutOffFreq = (fCutOff > fLimit) ? fLimit : fCutOff;

    CalculateCoefficents();
}

// FUNCTION: SWBF2 0x00747b80
void HighLowPassFilter::ResetState()
{
    mState[0].mInDelay[0] = 0.0f;
    mState[0].mInDelay[1] = 0.0f;
    mState[0].mOutDelay[0] = 0.0f;
    mState[0].mOutDelay[1] = 0.0f;

    mState[1].mInDelay[0] = 0.0f;
    mState[1].mInDelay[1] = 0.0f;
    mState[1].mOutDelay[0] = 0.0f;
    mState[1].mOutDelay[1] = 0.0f;
}

// FUNCTION: SWBF2 0x00747c20
unsigned __int64 SignalIdentity16(const short* pSamples, unsigned int uiCount)
{
    unsigned __int64 uiSum = 0;
    for (unsigned int i = 0; i < uiCount; ++i)
    {
        uiSum += pSamples[i] * pSamples[i];
    }
    return uiSum;
}

// FUNCTION: SWBF2 0x00747cd0
unsigned __int64 SignalPower16(const short* pSamples, unsigned int uiCount)
{
    return SignalIdentity16(pSamples, uiCount) / uiCount;
}

}
}
