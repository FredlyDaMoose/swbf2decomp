#pragma once

#include "PebbleFL/Common/PblHashTable.h"
#include "Zephyr/Source/ZephyrAnimBankReader.h"
#include "Zephyr/Source/ZephyrSkinReader.h"

class PblMemChunk;

class RedSkeleton;
class ZephyrSkeletonShared;
class ZephyrAnimBank;
class ZephyrEventBank;
class ZephyrAnim;

extern int RedAnimation_HashTableNumEntries;

// VTABLE: SWBF2 0x007834a0
class RedAnimation
{
public:
    RedAnimation();
    virtual ~RedAnimation();

    // SYNTHETIC: SWBF2 0x006dd6b0
    // RedAnimation::`scalar deleting destructor'

    static void ClearAll();

    // FUNCTION: SWBF2 0x00443970
    static RedAnimation* Find(unsigned int uiHash) { return _HashTable.Find(uiHash); }

    static PblHashTable<RedAnimation, 1024> _HashTable;

    const ZephyrAnim* FindAnimation(const char* pszName) const;

    void _ReadEvents(PblMemChunk& kChunk);
    void _ReadZaa(PblMemChunk& kChunk);
    void _ReadZaf(PblMemChunk& kChunk);

    static void ReadZaa(PblMemChunk& kChunk);
    static void ReadZaf(PblMemChunk& kChunk);

    ZephyrSkeletonShared* GetZephyrSkeletonShared() const { return _pSkeletonShared; }

    unsigned int _uiNameHash;
    RedSkeleton* _pSkeleton;
    ZephyrSkeletonShared* _pSkeletonShared;
    ZephyrSkinReader _kZephyrSkinReader;
    ZephyrAnimBank* _pZephyrAnimBank;
    ZephyrAnimBankReader _kZephyrAnimBankReader;
    ZephyrEventBank* _pEventBank;
};
