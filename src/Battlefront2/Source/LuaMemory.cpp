#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>

#include "LuaMemory.h"

#include "GameMemory.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

MemPoolInfo gGamePoolInfo[48] = {
    {  6933,   8, 0 }, {  3427,   6, 0 }, {  1764,  10, 0 }, {  1622,  18, 0 },
    {  1567,  80, 0 }, {  1405,   4, 0 }, {  1420,   7, 0 }, {   895, 160, 0 },
    {   750,  20, 0 }, {   895,   9, 0 }, {   733,  11, 0 }, {   728,  12, 0 },
    {   632,  40, 0 }, {   445,   5, 0 }, {   343,  13, 0 }, {   558,   3, 0 },
    {   284,  14, 0 }, {   349,   1, 0 }, {   184,  16, 0 }, {   146,  15, 0 },
    {   134,   2, 0 }, {   109,  24, 0 }, {    78,  28, 0 }, {    84,  22, 0 },
    {    90,  17, 0 }, {    73,  26, 0 }, {    59,  32, 0 }, {    60,  36, 0 },
    {    59,  30, 0 }, {    64,  21, 0 }, {    53,  34, 0 }, {    49,  19, 0 },
    {    43,  38, 0 }, {    33, 320, 0 }, {    34,  50, 0 }, {    34,  23, 0 },
    {    32,  42, 0 }, {    31,  29, 0 }, {    25,  44, 0 }, {    22,  46, 0 },
    {    21,  64, 0 }, {    18,  41, 0 }, {    18,  31, 0 }, {    16,  52, 0 },
    {    16,  68, 0 }, {    16,  56, 0 }, {    13,  58, 0 }, {    12,  53, 0 },
};

MemPoolInfo gShellPoolInfo[48] = {
    {  2648,   8, 0 }, {  1648,   6, 0 }, {  1150,   4, 0 }, {  1070,  10, 0 },
    {   578,   7, 0 }, {   551,  18, 0 }, {   520,   9, 0 }, {   449,  40, 0 },
    {   399,  20, 0 }, {   383,  11, 0 }, {   371,  12, 0 }, {   232,  80, 0 },
    {   206,   5, 0 }, {   195,   3, 0 }, {   131,  13, 0 }, {   121,  14, 0 },
    {    89,   1, 0 }, {    68,  15, 0 }, {    54,  16, 0 }, {    43, 160, 0 },
    {    40,   2, 0 }, {    35,  24, 0 }, {    34,  28, 0 }, {    34,  21, 0 },
    {    33,  22, 0 }, {    28,  34, 0 }, {    26,  26, 0 }, {    23,  36, 0 },
    {    24,  19, 0 }, {    22,  32, 0 }, {    20,  30, 0 }, {    18,  17, 0 },
    {    16,  38, 0 }, {    16,  64, 0 }, {    15,  23, 0 }, {    13,  60, 0 },
    {    10,  54, 0 }, {     8,  44, 0 }, {     8,  41, 0 }, {     8,  58, 0 },
    {     7,  53, 0 }, {     6,  68, 0 }, {     6,  46, 0 }, {     5,  66, 0 },
    {     5,  67, 0 }, {     5, 320, 0 }, {     4,  98, 0 }, {     4,  56, 0 },
};

MemPoolInfo gPoolInfo[48];
MemoryPool* gPools[48];

unsigned int gLuaUsedMemory;
namespace LuaMemory { unsigned int gLuaNumBlocks; }

// FUNCTION: SWBF2 0x005a15d0
void LuaMemory::CreatePools(bool bMission)
{
    int iOldHeap = RedSetCurrentHeap(GameMemory::LuaHeap);
    RedAllocFromHeap(iOldHeap, 4, 0);

    MemPoolInfo* pInfo = bMission ? gGamePoolInfo : gShellPoolInfo;
    int i;
    for (i = 0; i < 48; ++i)
    {

        gPoolInfo[i] = pInfo[i];
        if (gPoolInfo[i].mCount == 0)
        {
            gPools[i] = 0;
            continue;
        }

        char acName[16];
        sprintf(acName, "Lua%d", pInfo[i].mBin);
        MemoryPool* pPool = new MemoryPool(acName, 0);
        gPools[i] = pPool;
        pPool->Create(pInfo[i].mCount, pInfo[i].mBin * 4, 0);
    }

    RedSetCurrentHeap(iOldHeap);
}

// FUNCTION: SWBF2 0x005a1760
MemoryPool* LuaMemory::LocatePool(unsigned int uiSize, void* pBlock)
{
    unsigned int uiBin = (uiSize + 3) >> 2;

    MemoryPool* pPool = 0;
    int i;
    for (i = 0; i < 48; ++i)
    {
        if ((unsigned int)gPoolInfo[i].mBin == uiBin)
        {
            pPool = gPools[i];
            break;
        }
    }

    if (pBlock)
    {
        return (pPool && pPool->IsPresent(pBlock)) ? pPool : 0;
    }

    return (pPool && pPool->mUsed < pPool->mCount) ? pPool : 0;
}

// FUNCTION: SWBF2 0x005a17d0
void Lua_Red_Free(void* pBlock, unsigned int uiSize)
{
    if (!pBlock)
    {
        return;
    }

    MemoryPool* pPool = LuaMemory::LocatePool(uiSize, pBlock);
    if (pPool)
    {
        pPool->Free(pBlock);
    }
    else
    {
        RedFreeToHeap(GameMemory::LuaHeap, pBlock, 1);
    }
}

// FUNCTION: SWBF2 0x005a1810
void* Lua_Red_realloc(void* pBlock, unsigned int uiOldSize, unsigned int uiNewSize)
{
    char acMsg[0x80];

    MemoryPool* pPool = LuaMemory::LocatePool(uiNewSize, 0);
    void* pNew = pPool ? pPool->Allocate(uiNewSize)
                       : RedAllocFromHeap(GameMemory::LuaHeap, uiNewSize, 0);

    if (!pNew)
    {
        _snprintf(acMsg, 0x80,
                  "Lua out of memory (%d used in %d blocks, trying to alloc %d)\n",
                  gLuaUsedMemory, LuaMemory::gLuaNumBlocks, uiNewSize);
        *(char*)0 = 0;
    }

    if (pBlock)
    {
        memcpy(pNew, pBlock, uiNewSize < uiOldSize ? uiNewSize : uiOldSize);
        Lua_Red_Free(pBlock, uiOldSize);
    }

    return pNew;
}

// FUNCTION: SWBF2 0x005a18b0
void __cdecl LuaScript_CallWARNINGF(char const* pcMessage)
{
    (void)pcMessage;
}

// FUNCTION: SWBF2 0x005a16e0
void LuaMemory::Reset()
{
    int iOldHeap = RedSetCurrentHeap(GameMemory::LuaHeap);
    int i;
    for (i = 0; i < 48; ++i)
    {
        delete gPools[i];
        gPools[i] = 0;
    }
    RedSetCurrentHeap(iOldHeap);

    gLuaUsedMemory = 0;
    LuaMemory::gLuaNumBlocks = 0;
    memset(gPoolInfo, 0, sizeof(gPoolInfo));
}
