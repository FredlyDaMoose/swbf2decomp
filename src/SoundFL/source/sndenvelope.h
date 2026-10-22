#pragma once

namespace Snd
{

class Envelope
{
public:

    Envelope()
    {
        mStartLevel = 1.0f;
        mEndLevel = 1.0f;
        mLength = 1.0f;
    }

protected:

    float GetLevel(float fTime) const
    {
        if (mLength == 0.0f || fTime > mLength)
        {
            return mEndLevel;
        }
        const float f = (mLength - fTime) / mLength;
        return (1.0f - f) * mEndLevel + mStartLevel * f;
    }

public:
    float mStartLevel;
    float mEndLevel;
    float mLength;
};

}
