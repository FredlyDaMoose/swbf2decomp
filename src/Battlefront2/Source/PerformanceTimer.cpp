#include <windows.h>

#include "PerformanceTimer.h"

unsigned __int64 PblPerformanceTimer::_GetCycleCounter()
{
    LARGE_INTEGER iCount;
    QueryPerformanceCounter(&iCount);
    return iCount.QuadPart;
}

// FUNCTION: SWBF2 0x00530e70
PblPerformanceTimer::~PblPerformanceTimer()
{
}

// FUNCTION: SWBF2 0x00530e10
void PblPerformanceTimer::Start()
{
    _iValue = _GetCycleCounter();
}

// FUNCTION: SWBF2 0x00530e40
void PblPerformanceTimer::Stop()
{
    _iAccum += _GetCycleCounter() - _iValue;
}
