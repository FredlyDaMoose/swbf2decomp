#pragma once

#include "PebbleFL/Common/PblList.h"

class PblFileChunk;

class LuaScriptFactory
{
public:

    class List : public PblList<LuaScriptFactory>
    {
    public:

        List() {}

        void Add(LuaScriptFactory* pFactory);

        LuaScriptFactory* Find(unsigned int uiNameHash) const;

        void Remove(LuaScriptFactory* pFactory);
        using PblList<LuaScriptFactory>::Remove;
    };

    static void Read(PblFileChunk& chunk);

    void Purge();

    static void PurgeUnused();
    static void Cleanup();

    static LuaScriptFactory* Find(unsigned int uiNameHash) { return _GlobalList.Find(uiNameHash); }
    const char* GetScript() const { return mData; }
    unsigned int GetScriptLen() const { return mDataLen; }

    static List _GlobalList;
    static int sScriptMemUsage;

    PblList<LuaScriptFactory>::Node _Node;
    unsigned int mNameHash;
    unsigned int mDataLen;
    char* mData;

private:

    LuaScriptFactory();

    void _Read(PblFileChunk& chunk);
};
 
 

class LuaScript
{
public:

    LuaScript();

    static unsigned int LoadScript(const char* pcName);

    unsigned int RemoveScript(const char* pcName);
};
