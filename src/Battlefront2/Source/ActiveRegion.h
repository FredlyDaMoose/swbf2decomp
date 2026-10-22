#pragma once

#include <stddef.h>

#include "Thread.h"
#include "PebbleFL/Common/PblBitVector.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "RedEngineFL/Util/RedRegion.h"

struct lua_State;

namespace ActiveRegions
{

// VTABLE: SWBF2 0x00796b44
class ActiveRegion : public Thread
{
public:

    ActiveRegion(RedRegion& region);

    virtual ~ActiveRegion();

    virtual bool Update(float fDeltaTime);

    PblBitVector<600> GetPlayersInRegion();

    static void* operator new(unsigned int uSize);

    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    RedRegion& mRegion;
    PblBitVector<600> mPlayers;

    static MemoryPool sMemoryPool;
};

 
 
 

extern int sMapNumEntries;
extern PblHashTable<ActiveRegion, 64> sMap;

void Cleanup();

void Init();

int Get(lua_State* L);
int Activate(lua_State* L);
int Deactivate(lua_State* L);
int Name(lua_State* L);
int IsInside(lua_State* L);
ActiveRegion* __fastcall Activate(RedRegion* pRegion);
void __fastcall Deactivate(RedRegion* pRegion);
RedRegion* __fastcall Find(lua_State* L, int iIndex);

}

void* __fastcall InitRawListIterator(void** pCurrent, void* pEdx, const void* pSource);
void* __fastcall sub_437DD0(void** pCurrent);
void* __fastcall sub_437DE0(void** pCurrent);
void* __fastcall AdvanceRawListIterator(void** pCurrent);
