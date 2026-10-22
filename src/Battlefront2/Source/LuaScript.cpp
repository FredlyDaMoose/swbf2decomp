#include "LuaScript.h"

#include "GameMemory.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "LoadDisplay.h"
#include "LuaHelper.h"
#include "LuaFL/include/lauxlib.h"

// GLOBAL: SWBF2 0x007ebb7c
LuaScriptFactory::List LuaScriptFactory::_GlobalList;

// GLOBAL: SWBF2 0x01e579ac
int LuaScriptFactory::sScriptMemUsage;

// FUNCTION: SWBF2 0x005a18c0
LuaScriptFactory::LuaScriptFactory() : mNameHash(0), mDataLen(0), mData(0)
{
}

// FUNCTION: SWBF2 0x005a1910
void LuaScriptFactory::Read(PblFileChunk& chunk)
{
    LuaScriptFactory* pFactory = new LuaScriptFactory;
    pFactory->_Read(chunk);
    _GlobalList.Add(pFactory);
}

// FUNCTION: SWBF2 0x005a19d0
void LuaScriptFactory::_Read(PblFileChunk& chunk)
{
    char acName[256];
    acName[0] = 0;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId == 0x454D414Eu)
        {
            child.ReadString(acName, 256);
            mNameHash = PblHash(acName);
        }
        else if (child._iId == 0x59444F42u)
        {
            const int iSize = child.GetSize();
            mData = (char*)RedAllocFromHeap(GameMemory::RunTimeHeap, iSize + 1, 1);
            child._pFile->ReadData(mData, iSize);
            sScriptMemUsage += iSize;
            mDataLen = iSize;
        }
    }
}

// FUNCTION: SWBF2 0x005a1a90
void LuaScriptFactory::List::Add(LuaScriptFactory* pFactory)
{
    PblList<LuaScriptFactory>::Add(&pFactory->_Node, pFactory);
}

// FUNCTION: SWBF2 0x005a1990
void LuaScriptFactory::Purge()
{
    if (mData)
    {
        sScriptMemUsage -= mDataLen;
        RedFreeToHeap(GameMemory::RunTimeHeap, mData, 0);
        mData = 0;
        mDataLen = 0;
    }
}

// FUNCTION: SWBF2 0x005a1b00
LuaScriptFactory* LuaScriptFactory::List::Find(unsigned int uiNameHash) const
{
    for (Itor it(*this); (LuaScriptFactory*)it != 0; ++it)
    {
        if (it->mNameHash == uiNameHash)
        {
            return it;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005a1b80
LuaScript::LuaScript()
{
}

// FUNCTION: SWBF2 0x005a1b90
unsigned int LuaScript::LoadScript(const char* pcName)
{
    char acChunkName[260];
    strlcpy(acChunkName, pcName, 256);
    strlcat(acChunkName, ".lua", 256);

    if (LoadDisplay::sInstance)
    {
        LoadDisplay::sInstance->Update();
    }

    LuaScriptFactory* pFactory = LuaScriptFactory::_GlobalList.Find(PblHash(pcName));
    if (!pFactory)
    {
        return 0;
    }

    unsigned int uiResult = lua_dobuffer(LuaHelper::mState, pFactory->mData,
                                         pFactory->mDataLen, acChunkName);
    pFactory->Purge();
    lua_setgcthreshold(LuaHelper::mState, lua_getgccount(LuaHelper::mState));
    return uiResult;
}

// FUNCTION: SWBF2 0x005a1ad0
void LuaScriptFactory::List::Remove(LuaScriptFactory* pFactory)
{
    PblList<LuaScriptFactory>::Remove(&pFactory->_Node);
}

// FUNCTION: SWBF2 0x005a1c40
unsigned int LuaScript::RemoveScript(const char* pcName)
{
    LuaScriptFactory* pFactory = LuaScriptFactory::_GlobalList.Find(PblHash(pcName));
    LuaScriptFactory::_GlobalList.Remove(pFactory);
    return 0;
}

// FUNCTION: SWBF2 0x005a1900
void LuaScriptFactory::Cleanup()
{
    _GlobalList.RemoveAll();
    sScriptMemUsage = 0;
}

// FUNCTION: SWBF2 0x005a1950
void LuaScriptFactory::PurgeUnused()
{
    for (PblList<LuaScriptFactory>::Itor it(_GlobalList); (LuaScriptFactory*)it != 0; ++it)
    {
        if (it->mData)
        {
            it->Purge();
        }
    }
}

// FUNCTION: SWBF2 0x005a1b30
template void PblList<LuaScriptFactory>::RemoveAll();
