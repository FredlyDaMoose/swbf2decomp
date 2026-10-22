#include "DownloadableContent.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "LoadUtil.h"
#include "LuaHelper.h"
#include "LuaScript.h"

namespace DownloadableContent
{

// GLOBAL: SWBF2 0x01e30948
int mNumMissions;

// GLOBAL: SWBF2 0x01e30950
DownloadableMissionType mMissions[500];

// GLOBAL: SWBF2 0x007e632c
int mCurMissionIndex = -1;

// GLOBAL: SWBF2 0x01e51c90
char mCurMissionDir[260];

}

static char* PC_DOWNLOADABLECONTENT_DIR = "AddOn";

namespace DownloadableContent
{

// FUNCTION: SWBF2 0x0048eb50
void SetCurrentMission(unsigned int uiHash)
{
    mCurMissionIndex = -1;

    if (mNumMissions > 0)
    {
        for (int i = 0; i < mNumMissions; ++i)
        {
            if (mMissions[i].mMissionName == uiHash)
            {
                mCurMissionIndex = i;
                break;
            }
        }
    }
}

// FUNCTION: SWBF2 0x0048e750
bool ReadDataFile(char const* pcFileName)
{
    PblFile file;
    bool bResult;
    if (file.Open(pcFileName, false, false, true))
    {
        bResult = LoadUtil::ReadDataFile(file, 0, 0, 0, 0);
        file.Close();
    }
    else
    {
        bResult = false;
    }
    return bResult;
}

// FUNCTION: SWBF2 0x0048e7d0
void AddMissionCommon(LuaScript* pScript, char const* pcDir)
{
    strcpy(mCurMissionDir, pcDir);
    LuaScript::LoadScript("addme");
    pScript->RemoveScript("addme");

    char acFileName[300];
    LoadUtil::MakeFullName("shell\\eng\\shell.lvl", LoadUtil::FILE_TYPE_NONE, acFileName,
                           sizeof(acFileName), true);
    ReadDataFile(acFileName);
}

// FUNCTION: SWBF2 0x0048ea30
static int AddDownloadableContent(lua_State* L)
{
    char const* pcMapName = LuaHelper::GetString(1);
    char const* pcMissionName = LuaHelper::GetString(2);
    unsigned int uiLevelMemory = (LuaHelper::GetInt(3) + 32) << 16;

    if (mNumMissions < 500)
    {
        mMissions[mNumMissions].mMapName = PblHash(pcMapName);
        mMissions[mNumMissions].mMissionName = PblHash(pcMissionName);
        strncpy(mMissions[mNumMissions].mDir, mCurMissionDir, sizeof(mMissions[0].mDir));
        mMissions[mNumMissions].mDir[sizeof(mMissions[0].mDir) - 1] = 0;
        mMissions[mNumMissions].mLevelMemory = uiLevelMemory;
        ++mNumMissions;
        SetCurrentMission(PblHash(pcMissionName));
    }
    return 0;
}

// FUNCTION: SWBF2 0x0048e830
void AddMissions(LuaScript* pScript)
{
    mNumMissions = 0;
    LuaHelper::Register("AddDownloadableContent", AddDownloadableContent, 0);

    char acBuffer[260];
    char acFileName[260];
    _makepath(acBuffer, 0, PC_DOWNLOADABLECONTENT_DIR, "*", 0);
    _fullpath(acFileName, acBuffer, sizeof(acFileName));

    _finddata_t findData;
    intptr_t hFind = _findfirst(acFileName, &findData);
    if (hFind == -1)
    {
        return;
    }

    do
    {
        if (strcmp(findData.name, ".") != 0 && strcmp(findData.name, "..") != 0 &&
            (findData.attrib & _A_SUBDIR))
        {
            _snprintf(acFileName, sizeof(acFileName) - 1, "%s/%s", PC_DOWNLOADABLECONTENT_DIR,
                      findData.name);
            acFileName[sizeof(acFileName) - 1] = 0;
            _makepath(acBuffer, 0, acFileName, "addme", "script");
            _fullpath(acFileName, acBuffer, sizeof(acFileName));
            if (ReadDataFile(acFileName))
            {
                _snprintf(acBuffer, sizeof(acBuffer) - 1, "%s/%s", PC_DOWNLOADABLECONTENT_DIR,
                          findData.name);
                _fullpath(acFileName, acBuffer, sizeof(acFileName));
                AddMissionCommon(pScript, acFileName);
            }
        }
    } while (_findnext(hFind, &findData) != -1);

    _findclose(hFind);
    SetCurrentMission(0);
}

// FUNCTION: SWBF2 0x0048eb10
void SetCurrentMap(unsigned int uiHash)
{
    mCurMissionIndex = -1;

    if (mNumMissions > 0)
    {
        for (int i = 0; i < mNumMissions; ++i)
        {
            if (mMissions[i].mMapName == uiHash)
            {
                mCurMissionIndex = i;
                break;
            }
        }
    }
}

// FUNCTION: SWBF2 0x0048eb90
bool IsDownloadableContent()
{
    return mCurMissionIndex >= 0;
}

// FUNCTION: SWBF2 0x0048eba0
char const* GetContentDirectory()
{
    if (IsDownloadableContent())
    {
        return mMissions[mCurMissionIndex].mDir;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0048ebc0
bool IsMissionDownloaded(unsigned int uiHash)
{
    for (int i = 0; i < mNumMissions; ++i)
    {
        if (mMissions[i].mMissionName == uiHash)
        {
            return true;
        }
    }
    return false;
}

}
