#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

class PblTime
{
public:
    PblTime();

    // FUNCTION: SWBF2 0x0052d460
    static unsigned __int64 GetTickCount()
    {
        LARGE_INTEGER PerformanceCount;
        QueryPerformanceCounter(&PerformanceCount);
        return PerformanceCount.QuadPart;
    }

    static unsigned __int64 GetTicksPerSecond() { return ticksPerSecond; }

    static unsigned __int64 ticksPerSecond;
};
