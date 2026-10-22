#pragma once

class PblRandom;

class VarianceValue
{
public:

    float GetValue(PblRandom& rand) const;

    void Setup(float fValue)
    {
        mValue = fValue;
        mVariance = 0.0f;
    }

    void Setup(float fMin, float fMax)
    {
        if (fMin == fMax)
        {
            Setup(fMin);
        }
        else if (fMin > fMax)
        {
            mValue = fMax;
            mVariance = fMin - fMax;
        }
        else
        {
            mValue = fMin;
            mVariance = fMax - fMin;
        }
    }

    float GetMin() const { return mValue; }
    float GetMax() const { return mValue + mVariance; }

    float mValue;
    float mVariance;
};
