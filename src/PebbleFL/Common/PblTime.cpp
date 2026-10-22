#include "PblTime.h"

// GLOBAL: SWBF2 0x009cef78
unsigned __int64 PblTime::ticksPerSecond;

// FUNCTION: SWBF2 0x0072b2d0
PblTime::PblTime()
{
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);
    ticksPerSecond = Frequency.QuadPart;
}

// GLOBAL: SWBF2 0x009cef84
// SYNTHETIC: SWBF2 0x0040b000
// `dynamic initializer for 'pblTime''
PblTime pblTime;

// GLOBAL: SWBF2 0x009cef94

unsigned int sStartupWordSource;

// GLOBAL: SWBF2 0x009cefb8
// SYNTHETIC: SWBF2 0x0040b010
// `dynamic initializer for 'sStartupWordCopy''
unsigned int sStartupWordCopy = sStartupWordSource;
