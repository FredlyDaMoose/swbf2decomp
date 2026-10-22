#pragma once

namespace GameMemory
{

void InitApplicationMemory(char* pStart, char* pEnd);

void BuildHeaps(bool bMission);

void ReleaseTempHeap();

extern int ApplicationHeap;
extern int GamespyHeap;
extern int RunTimeHeap;
extern int TempLoadHeap;
extern int LuaHeap;
extern int ArtistHeap;
extern int TEMP_GamespyHeap;
}
