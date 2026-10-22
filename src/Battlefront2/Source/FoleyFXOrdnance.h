#pragma once

#include <stddef.h>

#include "FoleyFX.h"
#include "GameSound.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "SoundFL/source/sndfreelist.h"

class FoleyFXImpact : public FoleyFX
{
public:
    static void* operator new(unsigned int) { return smFreeList.Alloc(); }
    static void operator delete(void* p) { smFreeList.Free(p); }

    FoleyFXImpact();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~FoleyFXImpact();
    virtual bool Read(PblFileChunk& chunk, unsigned int uChunkID);

    GameSound mImpact;

    static PblHash rttiHashFoleyFXImpact;
    static char rttiNameFoleyFXImpact[];
    static Snd::FreeList smFreeList;
};
 
 
