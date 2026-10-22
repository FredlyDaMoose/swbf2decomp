#pragma once

#include <stddef.h>

class PblVector3;

namespace Snd
{
namespace Globals
{

extern void* (__cdecl* smAlloc)(size_t);
extern void (__cdecl* smFree)(void*);

extern void* (__cdecl* smTempAlloc)(size_t);
extern void (__cdecl* smTempFree)(void*);

extern void* (__cdecl* smAllocPerm)(size_t);
extern void (__cdecl* smFreePerm)(void*);

extern void (__cdecl* smWarningFunction)(char const* pcMessage);

extern ::PblVector3 smNullVector;

extern ::PblVector3 smUpVector;
extern ::PblVector3 smFrontVector;

}

void* __cdecl Alloc(unsigned int uiSize);
void* __cdecl AllocPerm(unsigned int uiSize);
void* __cdecl AllocTemp(unsigned int uiSize);
int __cdecl ConvertLinearToLog(float fLinear);
void __cdecl Free(void* p);
void __cdecl FreePerm(void* p);
void __cdecl FreeTemp(void* p);
bool __cdecl IsBase2(unsigned int uiValue);
void __cdecl PrintDebugMessage(char const* pcFormat, ...);

inline float Clamp(float fValue, float fMin, float fMax)
{
    return (fMin > fValue) ? fMin : ((fValue > fMax) ? fMax : fValue);
}

}
