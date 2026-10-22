#include "ZephyrEvent.h"

// FUNCTION: SWBF2 0x0072ee90
void ZephyrEventBank::Init(short iNumLists)
{
    _Lists = new ZephyrEventList[iNumLists];
    _iMaxLists = iNumLists;
    _iNumLists = 0;
}

// FUNCTION: SWBF2 0x0072ef70
ZephyrEventList* ZephyrEventBank::New(unsigned int uiHash)
{
    if (_iNumLists >= _iMaxLists)
    {
        return 0;
    }

    short i = _iNumLists++;
    if (_HashTable.Store(uiHash, &_Lists[i]))
    {
        ++_HashTableNumEntries;
    }
    return &_Lists[i];
}
