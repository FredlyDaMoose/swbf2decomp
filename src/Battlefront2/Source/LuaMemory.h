#pragma once

#include <stddef.h>

class MemoryPool;

struct MemPoolInfo
{
    unsigned short mCount;
    unsigned short mBin;
    unsigned int mTotal;
};
 
 
 

namespace LuaMemory
{

void CreatePools(bool bMission);

MemoryPool* LocatePool(unsigned int uiSize, void* pBlock);

void Reset();

}

void Lua_Red_Free(void* pBlock, unsigned int uiSize);
void* Lua_Red_realloc(void* pBlock, unsigned int uiOldSize, unsigned int uiNewSize);

void __cdecl LuaScript_CallWARNINGF(char const* pcMessage);
