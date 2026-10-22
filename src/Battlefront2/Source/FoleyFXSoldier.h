#pragma once

#include <stddef.h>

#include "FoleyFXOrdnance.h"

// VTABLE: SWBF2 0x0079f280
class FoleyFXSoldier : public FoleyFXImpact
{
public:
    static void* operator new(unsigned int) { return smFreeList.Alloc(); }
    static void operator delete(void* p) { smFreeList.Free(p); }

    FoleyFXSoldier();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~FoleyFXSoldier();
    virtual bool Read(PblFileChunk& chunk, unsigned int uChunkID);

    GameSound mFootstepLeftWalk;
    GameSound mFootstepRightWalk;
    GameSound mFootstepLeftRun;
    GameSound mFootstepRightRun;
    GameSound mFootstepLeftStop;
    GameSound mFootstepRightStop;
    GameSound mJump;
    GameSound mLand;
    GameSound mRoll;
    GameSound mProne;
    GameSound mSquat;
    GameSound mStand;
    GameSound mBodyFall;

    static PblHash rttiHashFoleyFXSoldier;
    static char rttiNameFoleyFXSoldier[];
    static Snd::FreeList smFreeList;
};
 
 
 
