#include "GameMemory.h"

#include "LuaMemory.h"
#include "RedEngineFL/Memory/RedMemory.h"

namespace GameMemory
{
int ApplicationHeap;
int GamespyHeap;
int RunTimeHeap;
int TempLoadHeap;

int LuaHeap;
int ArtistHeap;

int TEMP_GamespyHeap;
}

char* gArenaStart;
char* gArenaEnd;
char* GamespyHeapstart;
char* GamespyHeapend;
char* g_Heap3Start;
void* gTempHeapBlock;

// GLOBAL: SWBF2 0x01e56158
namespace GameMemory { int gTempGamespyBlock; }
// GLOBAL: SWBF2 0x01e5615c

char* gArtistHeapStart;

// FUNCTION: SWBF2 0x00533b20
void GameMemory::InitApplicationMemory(char* pStart, char* pEnd)
{
    gArenaStart = pStart;
    gArenaEnd = pEnd;
    gArenaStart = RedInitHeap(pStart, pEnd, 9);

    if (ApplicationHeap)
    {
        RedDestroyHeap(ApplicationHeap);
        ApplicationHeap = 0;
    }

    char* pAppEnd = gArenaStart + 0xF40000;
    ApplicationHeap = RedCreateHeap(gArenaStart, pAppEnd, 0x10, "Application");

    GamespyHeapstart = pAppEnd;
    GamespyHeapend = pAppEnd + 0x200000;
    GamespyHeap = RedCreateHeap(pAppEnd, pAppEnd + 0x200000, 0x10, "Gamespy");

    g_Heap3Start = GamespyHeapend;
    RedSetCurrentHeap(ApplicationHeap);
}

// FUNCTION: SWBF2 0x00533bc0
void GameMemory::BuildHeaps(bool bMission)
{
    if (TempLoadHeap)
    {
        RedMarkHeapPermanent(TempLoadHeap, false);
        RedDestroyHeap(TempLoadHeap);
        TempLoadHeap = 0;
    }

    if (RunTimeHeap)
    {
        RedMarkHeapPermanent(RunTimeHeap, false);
        if (TEMP_GamespyHeap)
        {
            RedDestroyHeap(TEMP_GamespyHeap);
            TEMP_GamespyHeap = 0;
            GameMemory::gTempGamespyBlock = 0;
        }
        RedDestroyHeap(RunTimeHeap);
        RunTimeHeap = 0;
    }

    if (LuaHeap)
    {
        RedMarkHeapPermanent(LuaHeap, false);
        RedDestroyHeap(LuaHeap);
        LuaHeap = 0;
    }

    if (ArtistHeap)
    {
        RedMarkHeapPermanent(ArtistHeap, false);
        RedDestroyHeap(ArtistHeap);
        ArtistHeap = 0;
    }

    char* pLuaStart = (char*)(((unsigned int)g_Heap3Start + 0x3F) & 0xFFFFFFC0);
    char* pLuaEnd = pLuaStart + 0x1000000;
    LuaHeap = RedCreateHeap(pLuaStart, pLuaEnd, 4, "Lua");

    char* pArtistStart = (char*)(((unsigned int)pLuaEnd + 0x3F) & 0xFFFFFFC0);
    gArtistHeapStart = pArtistStart;
    char* pArtistEnd = pArtistStart + 0x2000;
    ArtistHeap = RedCreateHeap(pArtistStart, pArtistEnd, 0x10, "Artist");

    char* pRunTimeStart = (char*)(((unsigned int)pArtistEnd + 0x3F) & 0xFFFFFFC0);
    RunTimeHeap = RedCreateHeap(pRunTimeStart, gArenaEnd, 0x10, "Runtime");
    RedSetCurrentHeap(RunTimeHeap);

    LuaMemory::CreatePools(bMission);

    gTempHeapBlock = RedAllocFromHeap(RunTimeHeap, 0x200000, 1);
    TempLoadHeap = RedCreateHeap(gTempHeapBlock, (char*)gTempHeapBlock + 0x1FFF00,
                                 0x10, "Temp");
    RedSetTempHeap(TempLoadHeap);
}

// FUNCTION: SWBF2 0x00533d50
void GameMemory::ReleaseTempHeap()
{
    if (!gTempHeapBlock)
    {
        return;
    }

    if (TempLoadHeap)
    {
        RedDestroyHeap(TempLoadHeap);
        RedSetTempHeap(-1);
        TempLoadHeap = 0;
    }
    RedFreeToHeap(RunTimeHeap, gTempHeapBlock, 0);
    gTempHeapBlock = 0;
}
