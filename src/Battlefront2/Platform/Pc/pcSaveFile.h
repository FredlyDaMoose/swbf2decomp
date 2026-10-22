#pragma once

#include <stdio.h>

class SaveFile
{
public:

    static void __cdecl SplitMetagameFilename(unsigned short const* pusFilename,
                                              unsigned short* pusName,
                                              unsigned short* pusDate,
                                              char* pcArg4,
                                              unsigned int uiArg5,
                                              unsigned int uiArg6);

    static void MakeMetagameFilename(unsigned short* pusDest, unsigned short const* pusSource);

protected:

    static int TimeToString(unsigned short* pusDest, int iYear, int iMonth, int iDay,
                            int iHour, int iMinute, int iSecond);

    static void __cdecl StringToTime(unsigned short const* pusString, int& iYear,
                                     int& iMonth, int& iDay, int& iHour,
                                     int& iMinute, int& iSecond);

public:

    void SetFileInfo(unsigned short const* pusFileName, int iType);
    SaveFile();

    unsigned short const* GetGameName();
    int GetType();

    unsigned short mFileName[260];
    unsigned short mGameName[260];
    FILE* mFileHandle;
    int mType;
};
