#pragma once

namespace PblMath
{
    static const float fPi = 3.141592654f;
    static const float fTwoPi = fPi * 2.0f;
    static const float fHalfPi = fPi * 0.5f;

    static const float fOneOverPi = 1.0f / fPi;

    static const float fOneOverTwoPi = 1.0f / fTwoPi;

    static const float fOneOverHalfPi = 1.0f / fHalfPi;

    float __cdecl Atan2(float y, float x);
    float __cdecl Sin(float fAngle);
    float __cdecl Cos(float fAngle);
    void __cdecl SinCos(float* pSin, float* pCos, float fAngle);

    inline int Ceiling(float f) { return (int)(f - 4194304.0f) + 4194304; }

    inline int Floor(float f) { return (int)(f + 4194304.0f) - 4194304; }
    inline int Round(float f) { return Floor(f + 0.5f); }
}
