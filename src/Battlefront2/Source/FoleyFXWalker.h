#pragma once

#include <stddef.h>

#include "FoleyFXOrdnance.h"

// VTABLE: SWBF2 0x0079f2b4
class FoleyFXWalker : public FoleyFXImpact
{
public:
    static void* operator new(unsigned int) { return smFreeList.Alloc(); }
    static void operator delete(void* p) { smFreeList.Free(p); }

    FoleyFXWalker();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~FoleyFXWalker();
    virtual bool Read(PblFileChunk& chunk, unsigned int uChunkID);

    GameSound mFootstep[6];

    static PblHash rttiHashFoleyFXWalker;
    static char rttiNameFoleyFXWalker[];
    static Snd::FreeList smFreeList;
};
 
 
