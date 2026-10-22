#pragma once

unsigned int __cdecl strlcpy(char* pcDest, char const* pcSource, unsigned int uiSize);

unsigned int __cdecl strlcat(char* pcDest, char const* pcSource, unsigned int uiSize);

namespace PblUnicode
{
unsigned int __cdecl ustrlen(unsigned short const* pusString);

unsigned int __cdecl ustrlcpy(unsigned short* pusDest, unsigned short const* pusSource,
                              unsigned int uiSize);
int __cdecl ustrcmp(unsigned short const* pus1, unsigned short const* pus2);
unsigned short* __cdecl ustrchr(unsigned short* pusString, unsigned short usChar);

unsigned int __cdecl ustrlcat(unsigned short* pusDest, unsigned short const* pusSource,
                              unsigned int uiSize);
unsigned short* __cdecl ustrncpy(unsigned short* pusDest, unsigned short const* pusSource,
                                 unsigned int uiCount);

unsigned short* __cdecl ustrupr(unsigned short* pusString);
unsigned short* __cdecl ustrlwr(unsigned short* pusString);
unsigned int __cdecl ustrncnv(unsigned short* pusDest, char const* pcSource,
                              unsigned int uiCount);
unsigned int __cdecl ustrncnv(char* pcDest, unsigned short const* pusSource,
                              unsigned int uiCount);
unsigned int __cdecl ustrncnv(unsigned short* pusDest, unsigned char const* pucSource,
                              unsigned int uiCount);
int __cdecl usnprintf(unsigned short* pusDest, unsigned int uiCount,
                      char const* pcFormat, ...);
int __cdecl usnprintf(unsigned short* pusDest, unsigned int uiCount,
                      unsigned short const* pusFormat, ...);
int __cdecl uvsnprintf(unsigned short* pusDest, unsigned int uiCount,
                       char const* pcFormat, char* pArgs);
int __cdecl uvsnprintf(unsigned short* pusDest, unsigned int uiCount,
                       unsigned short const* pusFormat, char* pArgs);
}
