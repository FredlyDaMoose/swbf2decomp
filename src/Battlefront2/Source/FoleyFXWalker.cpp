#include "FoleyFXWalker.h"

#include <stdio.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

// GLOBAL: SWBF2 0x01ebdc4c
// SYNTHETIC: SWBF2 0x00403b40
// `dynamic initializer for 'FoleyFXWalker::rttiHashFoleyFXWalker''
PblHash FoleyFXWalker::rttiHashFoleyFXWalker("FoleyFXWalker");
char FoleyFXWalker::rttiNameFoleyFXWalker[] = "FoleyFXWalker";

// GLOBAL: SWBF2 0x007ec1c0
// SYNTHETIC: SWBF2 0x00403b20
// `dynamic initializer for 'FoleyFXWalker::smFreeList''
// SYNTHETIC: SWBF2 0x00768810
// `dynamic atexit destructor for 'FoleyFXWalker::smFreeList''
Snd::FreeList FoleyFXWalker::smFreeList(sizeof(FoleyFXWalker), 4, 8, 1);

// FUNCTION: SWBF2 0x0052d000
bool FoleyFXWalker::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashFoleyFXWalker || FoleyFX::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0052d030
unsigned int FoleyFXWalker::GetDerivedRtti() const
{
    return rttiHashFoleyFXWalker;
}

// FUNCTION: SWBF2 0x0052d040
const char* FoleyFXWalker::GetDerivedRttiName() const
{
    return rttiNameFoleyFXWalker;
}

// FUNCTION: SWBF2 0x0052d050
FoleyFXWalker::FoleyFXWalker()
{
}

// SYNTHETIC: SWBF2 0x0052d0b0
// FoleyFXWalker::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0052d0e0
FoleyFXWalker::~FoleyFXWalker()
{
}

// FUNCTION: SWBF2 0x0052d140
bool FoleyFXWalker::Read(PblFileChunk& chunk, unsigned int uChunkID)
{
    for (unsigned int i = 0; i < 6; i++)
    {
        char acName[64];
        _snprintf(acName, sizeof(acName), "Walker%d", i);
        if (uChunkID == PblHash(acName))
        {
            mFootstep[i].SetID(chunk._pFile->Read32(), false, false);
            return true;
        }
    }

    return FoleyFXImpact::Read(chunk, uChunkID);
}
