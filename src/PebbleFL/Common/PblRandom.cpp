#include "PblRandom.h"

#include <math.h>

// GLOBAL: SWBF2 0x007e2cf4
PblRandom PblRandom::_Global(0x94153A94u);

// GLOBAL: SWBF2 0x009cef48
// SYNTHETIC: SWBF2 0x0040af40
// `dynamic initializer for 'PblRandom::_Sim''
PblRandom PblRandom::_Sim;

// FUNCTION: SWBF2 0x00438570
float PblRandom::GetGlobalFloat()
{
    return _Global.GetFloat();
}

// GLOBAL: SWBF2 0x009cef4c
int sHasCachedNormal;

// GLOBAL: SWBF2 0x009cef50
float sCachedNormal;

// FUNCTION: SWBF2 0x00729e20
float PblRandom::GetNormal()
{
    if (sHasCachedNormal)
    {
        sHasCachedNormal = 0;
        return sCachedNormal;
    }

    float fV1, fV2, fRsq;
    do
    {
        fV1 = GetFloat() * 2.0f - 1.0f;
        fV2 = GetFloat() * 2.0f - 1.0f;
        fRsq = fV2 * fV2 + fV1 * fV1;
    } while (fRsq >= 1.0f || fRsq == 0.0f);

    float fFac = (float)sqrt((double)((float)log((double)fRsq) * -2.0f / fRsq));
    sHasCachedNormal = 1;
    sCachedNormal = fFac * fV1;
    return fFac * fV2;
}
