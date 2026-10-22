#pragma once

#include <math.h>

class PblAngle
{
public:

    PblAngle() : mCos(1.0f), mSin(0.0f) {}
    PblAngle(float fCos, float fSin) : mCos(fCos), mSin(fSin) {}

    explicit PblAngle(float fAngle) : mCos(cosf(fAngle)), mSin(sinf(fAngle)) {}

    PblAngle& operator-=(const PblAngle& a)
    {
        float fSin = mSin * a.mCos - mCos * a.mSin;
        mCos = mSin * a.mSin + mCos * a.mCos;
        mSin = fSin;
        return *this;
    }

    void Normalize()
    {
        float fInv = 1.0f / sqrtf(mCos * mCos + mSin * mSin);
        mCos = mCos * fInv;
        mSin = mSin * fInv;
    }

    void Renormalize()
    {
        float fLength2 = mCos * mCos + mSin * mSin;
        float fHalf = (3.0f - fLength2) * 0.5f;
        float fScale = (3.0f - fHalf * fLength2 * fHalf) * (fHalf * 0.5f);
        mSin = fScale * mSin;
        mCos = fScale * mCos;
    }

    PblAngle& operator+=(const PblAngle& a)
    {
        float fCos = mCos * a.mCos - mSin * a.mSin;
        mSin = mCos * a.mSin + mSin * a.mCos;
        mCos = fCos;
        return *this;
    }

    float GetCos() const { return mCos; }
    float GetSin() const { return mSin; }

    float mCos;
    float mSin;
};

 
