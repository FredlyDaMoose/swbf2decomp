#pragma once

#ifndef SWBF2_BATTLEFRONT2_STRINGDB_H
#define SWBF2_BATTLEFRONT2_STRINGDB_H

#include "PebbleFL/Common/PblHashTable.h"

namespace StringDB
{

void Init();

void Cleanup();

const char* Store(unsigned int uiHash, const char* pcText);
bool Remove(unsigned int uiHash);
const char* Find(unsigned int uiHash);

extern PblHashTable<char, 4096> mMap;
extern char* mPool;
extern char* mNext;

}

#endif
