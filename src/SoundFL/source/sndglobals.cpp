#include "sndglobals.h"

#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblVector.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace Snd
{

// FUNCTION: SWBF2 0x00733340
static void __cdecl PrintDebugMessageDefault(char const*)
{
}

// GLOBAL: SWBF2 0x009d83b0
// SYNTHETIC: SWBF2 0x0040b600
// `dynamic initializer for 'Snd::sndRandom''
PblRandom sndRandom;

namespace Globals
{

void* (__cdecl* smAlloc)(size_t) = 0;
void (__cdecl* smFree)(void*) = 0;
void* (__cdecl* smTempAlloc)(size_t) = 0;
void (__cdecl* smTempFree)(void*) = 0;

void* (__cdecl* smAllocPerm)(size_t) = 0;
void (__cdecl* smFreePerm)(void*) = 0;

void (__cdecl* smWarningFunction)(char const*) = PrintDebugMessageDefault;

// GLOBAL: SWBF2 0x009d83b4
::PblVector3 smNullVector;

// GLOBAL: SWBF2 0x007e338c
::PblVector3 smUpVector(0.0f, 1.0f, 0.0f);
// GLOBAL: SWBF2 0x007e3398
::PblVector3 smFrontVector(0.0f, 0.0f, 1.0f);

}

// FUNCTION: SWBF2 0x00733350
void* __cdecl Alloc(unsigned int uiSize)
{
    return Globals::smAlloc(uiSize);
}

// FUNCTION: SWBF2 0x00733360
void* __cdecl AllocPerm(unsigned int uiSize)
{
    return Globals::smAllocPerm(uiSize);
}

// FUNCTION: SWBF2 0x00733370
void* __cdecl AllocTemp(unsigned int uiSize)
{
    return Globals::smTempAlloc(uiSize);
}

// FUNCTION: SWBF2 0x00733380
int __cdecl ConvertLinearToLog(float fLinear)
{
    if (1.0e-5f > fLinear)
    {
        return -10000;
    }

    int iLog = (int)((float)log10((double)fLinear) * 2000.0f);
    if (iLog > 0)
    {
        iLog = 0;
    }

    return iLog;
}

// FUNCTION: SWBF2 0x007333c0
void __cdecl Free(void* p)
{
    Globals::smFree(p);
}

// FUNCTION: SWBF2 0x007333d0
void __cdecl FreePerm(void* p)
{
    Globals::smFreePerm(p);
}

// FUNCTION: SWBF2 0x007333e0
void __cdecl FreeTemp(void* p)
{
    Globals::smTempFree(p);
}

// FUNCTION: SWBF2 0x007333f0
bool __cdecl IsBase2(unsigned int uiValue)
{
    while ((uiValue & 1) == 0)
    {
        uiValue >>= 1;
    }

    return uiValue == 1;
}

// FUNCTION: SWBF2 0x00733420
void __cdecl PrintDebugMessage(char const* pcFormat, ...)
{
    char acMessage[0x800];
    va_list args;

    va_start(args, pcFormat);
    _vsnprintf(acMessage, 0x7fe, pcFormat, args);
    va_end(args);

    strcat(acMessage, "\n");
    Globals::smWarningFunction(acMessage);
}

}

// LIBRARY: SWBF2 0x00753152
// _libm_sse2_log10_precise
