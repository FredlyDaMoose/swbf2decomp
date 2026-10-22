#include "PblUnicode.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

unsigned int HandleChar(unsigned short* pusDest, unsigned int uiCount, unsigned short usChar);
unsigned int HandleFloat(unsigned short* pusDest, unsigned int uiCount, float fValue);
unsigned int HandleInt(unsigned short* pusDest, unsigned int uiCount, int iValue);
unsigned int HandleString(unsigned short* pusDest, unsigned int uiCount, char const* pcString);

// FUNCTION: SWBF2 0x00726fb0
unsigned int HandleChar(unsigned short* pusDest, unsigned int uiCount, unsigned short usChar)
{
    if (!uiCount)
    {
        return 0;
    }
    *pusDest = usChar;
    return 1;
}

// FUNCTION: SWBF2 0x00726fd0
unsigned int HandleFloat(unsigned short* pusDest, unsigned int uiCount, float fValue)
{
    char buffer[64];
    unsigned int count = uiCount < 64 ? uiCount : 64;
    _snprintf(buffer, count, "%.2f", fValue);
    return PblUnicode::ustrncnv(pusDest, buffer, count);
}

// FUNCTION: SWBF2 0x00727090
unsigned int HandleString(unsigned short* pusDest, unsigned int uiCount, char const* pcString)
{
    return PblUnicode::ustrncnv(pusDest, pcString, uiCount);
}

// FUNCTION: SWBF2 0x007272b0
unsigned short* __cdecl PblUnicode::ustrlwr(unsigned short* pusString)
{
    unsigned short previous = 0;
    for (unsigned short* p = pusString; *p; ++p)
    {
        if (previous != '%'
            && ((*p >= 'A' && *p <= 'Z') || (*p >= 0xC0 && *p <= 0xDD)))
        {
            *p += 32;
        }
        previous = *p;
    }
    return pusString;
}

// FUNCTION: SWBF2 0x007274c0
unsigned short* __cdecl PblUnicode::ustrchr(unsigned short* pusString, unsigned short usChar)
{
    unsigned short* result = 0;
    for (unsigned short* p = pusString; *p; ++p)
    {
        if (*p == usChar)
        {
            result = p;
        }
    }
    return result;
}

// FUNCTION: SWBF2 0x007274f0
unsigned short* __cdecl PblUnicode::ustrupr(unsigned short* pusString)
{
    unsigned short previous = 0;
    for (unsigned short* p = pusString; *p; ++p)
    {
        if (previous != '%'
            && ((*p >= 'a' && *p <= 'z') || (*p >= 0xE0 && *p <= 0xFD)))
        {
            *p -= 32;
        }
        previous = *p;
    }
    return pusString;
}

// FUNCTION: SWBF2 0x007270d0
unsigned int __cdecl strlcat(char* pcDest, char const* pcSource, unsigned int uiSize)
{
    unsigned int uiLen = 0;

    if (uiSize)
    {
        while (pcDest[uiLen])
        {
            if (++uiLen >= uiSize)
            {
                break;
            }
        }

        if (uiLen < uiSize)
        {

            unsigned int uiStart = uiLen;
            while (pcSource[uiLen - uiStart])
            {
                pcDest[uiLen] = pcSource[uiLen - uiStart];
                if (++uiLen >= uiSize)
                {
                    break;
                }
            }
        }
    }

    if (uiSize)
    {
        pcDest[uiLen == uiSize ? uiLen - 1 : uiLen] = '\0';
    }

    return uiLen;
}

// FUNCTION: SWBF2 0x00727120
unsigned int __cdecl strlcpy(char* pcDest, char const* pcSource, unsigned int uiSize)
{
    unsigned int uiLen = 0;

    if (uiSize)
    {
        while (pcSource[uiLen])
        {
            pcDest[uiLen] = pcSource[uiLen];
            if (++uiLen >= uiSize)
            {
                break;
            }
        }
    }

    if (uiSize)
    {
        pcDest[uiLen == uiSize ? uiLen - 1 : uiLen] = '\0';
    }

    return uiLen;
}

// FUNCTION: SWBF2 0x007271a0
int __cdecl PblUnicode::ustrcmp(unsigned short const* pus1, unsigned short const* pus2)
{
    unsigned short us1, us2;
    do
    {
        us1 = *pus1++;
        us2 = *pus2++;
    } while (us1 == us2 && us1);
    return (int)us1 - (int)us2;
}

// FUNCTION: SWBF2 0x007271f0
unsigned int __cdecl PblUnicode::ustrlcat(unsigned short* pusDest, unsigned short const* pusSource,
                                          unsigned int uiSize)
{
    unsigned int uiLen = 0;
    unsigned short* p = pusDest;

    if (uiSize)
    {
        while (*p)
        {
            ++uiLen;
            ++p;
            if (uiLen >= uiSize)
            {
                goto done;
            }
        }

        if (uiLen < uiSize)
        {
            unsigned short const* pSrc = pusSource;
            while (*pSrc)
            {
                *p++ = *pSrc++;
                if (++uiLen >= uiSize)
                {
                    break;
                }
            }
        }
    }

done:
    *p = 0;
    if (uiSize)
    {
        pusDest[uiSize - 1] = 0;
    }
    return uiLen;
}

// FUNCTION: SWBF2 0x00727290
unsigned int __cdecl PblUnicode::ustrlen(unsigned short const* pusString)
{
    unsigned short const* p = pusString;
    while (*p)
    {
        ++p;
    }
    return (unsigned int)(p - pusString);
}

// FUNCTION: SWBF2 0x00727470
unsigned short* __cdecl PblUnicode::ustrncpy(unsigned short* pusDest, unsigned short const* pusSource,
                                             unsigned int uiCount)
{
    unsigned short* p = pusDest;
    for (unsigned int i = 0; i < uiCount; ++i)
    {
        unsigned short c = pusSource[i];
        if (!c)
        {
            break;
        }
        *p++ = c;
    }
    *p = 0;
    if (uiCount)
    {
        pusDest[uiCount - 1] = 0;
    }
    return pusDest;
}

// FUNCTION: SWBF2 0x00727310
unsigned int __cdecl PblUnicode::ustrncnv(char* pcDest, unsigned short const* pusSource,
                                          unsigned int uiCount)
{
    char* p = pcDest;
    for (unsigned int i = 0; i < uiCount; ++p)
    {
        while (*pusSource >= 0x80)
        {
            ++pusSource;
        }
        if (!*pusSource)
        {
            break;
        }
        ++i;
        *p = (char)*pusSource++;
    }
    *p = 0;
    if (uiCount)
    {
        pcDest[uiCount - 1] = 0;
    }
    return (unsigned int)(p - pcDest);
}

// FUNCTION: SWBF2 0x007273d0
unsigned int __cdecl PblUnicode::ustrncnv(unsigned short* pusDest, char const* pcSource,
                                          unsigned int uiCount)
{
    unsigned short* p = pusDest;
    for (unsigned int i = 0; i < uiCount; ++p)
    {
        if (!*pcSource)
        {
            break;
        }
        ++i;
        *p = (unsigned short)*pcSource++;
    }
    *p = 0;
    if (uiCount)
    {
        pusDest[uiCount - 1] = 0;
    }
    return (unsigned int)(p - pusDest);
}

// FUNCTION: SWBF2 0x00727160
int __cdecl PblUnicode::usnprintf(unsigned short* pusDest, unsigned int uiCount, char const* pcFormat, ...)
{
    va_list args;
    va_start(args, pcFormat);
    return uvsnprintf(pusDest, uiCount, pcFormat, args);
}

// FUNCTION: SWBF2 0x00727550
int __cdecl PblUnicode::uvsnprintf(unsigned short* pusDest, unsigned int uiCount, char const* pcFormat,
                                   char* pArgs)
{
    unsigned int uiLen = 0;
    char const* pcRun = pcFormat;
    for (;;)
    {
        unsigned int n = 0;
        while (*pcFormat != '%' && *pcFormat && uiLen < uiCount)
        {
            ++pcFormat;
            ++n;
            ++uiLen;
        }
        ustrncnv(pusDest, pcRun, n + 1);
        pusDest += n;

        if (*pcFormat == '%')
        {
            switch (pcFormat[1])
            {
            case 'c':
                n = HandleChar(pusDest, uiCount - uiLen, (unsigned short)va_arg(pArgs, char));
                break;
            case 'd':
                n = HandleInt(pusDest, uiCount - uiLen, va_arg(pArgs, int));
                break;
            case 'f':
                n = HandleFloat(pusDest, uiCount - uiLen, va_arg(pArgs, float));
                break;
            case 's':
                n = HandleString(pusDest, uiCount - uiLen, va_arg(pArgs, char const*));
                break;
            default:
                return -1;
            }
            uiLen += n;
            pcFormat += 2;
            pcRun = pcFormat;
            pusDest += n;
        }

        if (!*pcFormat)
        {
            if (uiLen < uiCount)
            {
                *pusDest = 0;
            }
            return uiLen;
        }
        if (uiLen >= uiCount)
        {
            return uiLen;
        }
    }
}

// FUNCTION: SWBF2 0x00727030
unsigned int HandleInt(unsigned short* pusDest, unsigned int uiCount, int iValue)
{
    char acBuffer[64];
    unsigned int uiSize = uiCount < sizeof(acBuffer) ? uiCount : sizeof(acBuffer);
    _snprintf(acBuffer, uiSize, "%d", iValue);
    return PblUnicode::ustrncnv(pusDest, acBuffer, uiSize);
}

// FUNCTION: SWBF2 0x007270b0
static unsigned int HandleString(unsigned short* pusDest, unsigned int uiCount, unsigned short const* pusString)
{
    return PblUnicode::ustrlen(PblUnicode::ustrncpy(pusDest, pusString, uiCount));
}

// FUNCTION: SWBF2 0x00727180
int __cdecl PblUnicode::usnprintf(unsigned short* pusDest, unsigned int uiCount, unsigned short const* pusFormat, ...)
{
    va_list args;
    va_start(args, pusFormat);
    return uvsnprintf(pusDest, uiCount, pusFormat, args);
}

// FUNCTION: SWBF2 0x007276c0
int __cdecl PblUnicode::uvsnprintf(unsigned short* pusDest, unsigned int uiCount, unsigned short const* pusFormat,
                                   char* pArgs)
{
    unsigned int uiLen = 0;
    unsigned short const* pusRun = pusFormat;
    for (;;)
    {
        unsigned int n = 0;
        while (*pusFormat != '%' && *pusFormat && uiLen < uiCount)
        {
            ++pusFormat;
            ++n;
            ++uiLen;
        }
        memcpy(pusDest, pusRun, n * 2);
        pusDest += n;

        if (*pusFormat == '%')
        {
            switch (pusFormat[1])
            {
            case '%':
                *pusDest = '%';
                n = 1;
                break;
            case 'c':
                n = HandleChar(pusDest, uiCount - uiLen, (unsigned short)va_arg(pArgs, int));
                break;
            case 'd':
                n = HandleInt(pusDest, uiCount - uiLen, va_arg(pArgs, int));
                break;
            case 'f':
                n = HandleFloat(pusDest, uiCount - uiLen, va_arg(pArgs, float));
                break;
            case 's':
                n = HandleString(pusDest, uiCount - uiLen, va_arg(pArgs, unsigned short const*));
                break;
            default:
                return -1;
            }
            uiLen += n;
            pusFormat += 2;
            pusRun = pusFormat;
            pusDest += n;
        }

        if (!*pusFormat)
        {
            if (uiLen < uiCount)
            {
                *pusDest = 0;
            }
            return uiLen;
        }
        if (uiLen >= uiCount)
        {
            return uiLen;
        }
    }
}

// FUNCTION: SWBF2 0x00727370
unsigned int __cdecl ConvertUnicodeSkippingWide(char* pDest, const unsigned short* pSrc, unsigned int uiCount)
{
    char* p = pDest;
    for (unsigned int i = 0; i < uiCount; ++p)
    {
        while (*pSrc >= 0x100)
        {
            ++pSrc;
        }
        if (!*pSrc)
        {
            break;
        }
        ++i;
        *p = (char)*pSrc++;
    }
    *p = 0;
    if (uiCount)
    {
        pDest[uiCount - 1] = 0;
    }
    return (unsigned int)(p - pDest);
}

namespace PblUnicode {

// FUNCTION: SWBF2 0x00727420
unsigned int __cdecl ustrncnv(unsigned short* pDest, const unsigned char* pSrc, unsigned int uiCount)
{
    unsigned short* p = pDest;
    for (unsigned int i = 0; i < uiCount; ++p)
    {
        if (!*pSrc)
        {
            break;
        }
        ++i;
        *p = (unsigned char)*pSrc++;
    }
    *p = 0;
    if (uiCount)
    {
        pDest[uiCount - 1] = 0;
    }
    return (unsigned int)(p - pDest);
}

}

// FUNCTION: SWBF2 0x00727250
unsigned int __cdecl PblUnicode::ustrlcpy(unsigned short* pusDest, unsigned short const* pusSource,
                                          unsigned int uiSize)
{
    unsigned int uiCount = 0;
    if (uiSize)
    {
        unsigned short* p = pusDest;
        while (uiCount < uiSize)
        {
            unsigned short c = pusSource[uiCount];
            if (!c)
            {
                break;
            }
            *p++ = c;
            ++uiCount;
        }
        if (uiCount == uiSize)
        {
            --p;
        }
        *p = 0;
    }
    return uiCount;
}
