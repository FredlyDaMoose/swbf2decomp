#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHashTable.h"

class ZephyrEventList
{
public:

    ZephyrEventList() : _iNumEvents(0), _pEventTimes(0), _pEventMessages(0) {}
    void AddEvent(float fTime, unsigned int uiMessage);
    void Init(short iNumEvents);
    short _iNumEvents;
    float* _pEventTimes;
    unsigned int* _pEventMessages;
};
 

class ZephyrEventBank
{
public:

    ZephyrEventBank();
    void Init(short iNumLists);
    ZephyrEventList* New(unsigned int uiHash);

    int _HashTableNumEntries;
    PblHashTable<ZephyrEventList, 128> _HashTable;
    ZephyrEventList* _Lists;
    short _iNumLists;
    short _iMaxLists;
};
 
 
 
 
