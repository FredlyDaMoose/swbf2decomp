#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdio.h>

#include "GameLoop.h"
#include "LoadUtil.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblUnicode.h"

class CustomContent
{
public:
    static void Create(bool bMission);
};

bool HasHD();
void CopyFiles();

// FUNCTION: SWBF2 0x00488350
void CopyFiles()
{
    char source[0x4000];
    char unusedPath[64];
    char destination[64];
    char fullName[64];

    _snprintf(unusedPath, sizeof(unusedPath), "%s\\%s", "Data\\_LVL_PS2", "TAT");
    _snprintf(source, sizeof(source), "%s/%s", "TAT", "tat2");
    LoadUtil::MakeFullName(source, LoadUtil::FILE_TYPE_LEVEL, fullName, sizeof(fullName), false);

    PblFile input;
    input.Open(fullName, true, false, true);
    if (!input.IsOpen())
    {
        return;
    }

    strlcpy(destination, fullName, sizeof(destination));
    PblFile output;
    output.OpenCreate(destination, true, false, true);
    if (!output.IsOpen())
    {
        input.Close();
        return;
    }

    for (int remaining = input.GetSize(); remaining > 0; )
    {
        int count = remaining < (int)sizeof(source) ? remaining : (int)sizeof(source);
        input.ReadData(source, count);
        output.WriteData(source, count);
        remaining -= count;
    }
    input.Close();
    output.Close();
}

// GLOBAL: SWBF2 0x007e6321
static bool firstTime = true;

// FUNCTION: SWBF2 0x004882e0
void CustomContent::Create(bool bMission)
{
    if (HasHD())
    {
        if (bMission)
        {
            char acMission[64];
            strlcpy(acMission, GameLoop::mMissionScript, sizeof(acMission));
            acMission[sizeof(acMission) - 1] = 0;
            _strlwr(acMission);
            strncmp(acMission, "tat2", 4);
        }
        else
        {
            if (firstTime)
            {
                CopyFiles();
            }
            firstTime = false;
        }
    }
}

// FUNCTION: SWBF2 0x004884e0
bool HasHD()
{
    return false;
}
