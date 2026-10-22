#include <string.h>

#include "StringDB.h"

namespace StringDB
{

int mMapNumEntries;
// GLOBAL: SWBF2 0x01f6df0c
// SYNTHETIC: SWBF2 0x004064d0
// `dynamic initializer for 'StringDB::mMap''

PblHashTable<char, 4096> mMap;

char* mPool;
char* mNext;
int mPoolSize;

// FUNCTION: SWBF2 0x00651ed0
void Init()
{
    mMap.Clear();
    mMapNumEntries = 0;

    mPoolSize = 0x8000;

    mPool = (char*)operator new(0x8000);
    memset(mPool, 0, 0x8000);
    mNext = mPool;
}

// FUNCTION: SWBF2 0x00651f20
void Cleanup()
{
    mMap.Clear();
    mMapNumEntries = 0;

    operator delete(mPool);
    mPool = 0;
    mNext = 0;
}

// FUNCTION: SWBF2 0x00651ff0
bool Remove(unsigned int uiHash)
{
    if (!mMap.Find(uiHash))
    {
        return false;
    }
    if (mMap.Remove(uiHash))
    {
        --mMapNumEntries;
    }
    return true;
}

// FUNCTION: SWBF2 0x00651f60
const char* Store(unsigned int uiHash, const char* pcText)
{
    if (!uiHash)
    {
        return 0;
    }
    const char* pcFound = mMap.Find(uiHash);
    if (pcFound)
    {
        return pcFound;
    }
    unsigned int uiLen = strlen(pcText);
    char* pcCopy = mNext;
    memcpy(pcCopy, pcText, uiLen + 1);
    pcCopy[uiLen + 1] = 0;
    if (mMap.Store(uiHash, pcCopy))
    {
        ++mMapNumEntries;
    }
    mNext += uiLen + 1;
    return pcCopy;
}

// FUNCTION: SWBF2 0x00652030
const char* Find(unsigned int uiHash)
{
    return mMap.Find(uiHash);
}

}
