#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>

char lastTraceMessage[1024];

// FUNCTION: SWBF2 0x0072b3a0
void __cdecl PblTraceP(const char* Format, ...)
{
    va_list ArgList;
    va_start(ArgList, Format);
    _vsnprintf(lastTraceMessage, 0x400, Format, ArgList);
    va_end(ArgList);

    OutputDebugStringA(lastTraceMessage);
}
