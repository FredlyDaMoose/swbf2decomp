#pragma once

#include <stddef.h>

class LuaScript;

namespace DownloadableContent
{

struct DownloadableMissionType
{
    unsigned int mMapName;
    unsigned int mMissionName;
    char mDir[260];
    unsigned int mLevelMemory;
};
 
 
 

void SetCurrentMission(unsigned int uiHash);

void AddMissionCommon(LuaScript* pScript, char const* pcDir);
void AddMissions(LuaScript* pScript);
void SetCurrentMap(unsigned int uiHash);
bool IsDownloadableContent();
char const* GetContentDirectory();
bool IsMissionDownloaded(unsigned int uiHash);
bool ReadDataFile(char const* pcFileName);

extern DownloadableMissionType mMissions[500];

}
