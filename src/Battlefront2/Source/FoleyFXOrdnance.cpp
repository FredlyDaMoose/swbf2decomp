#include "FoleyFXOrdnance.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

// GLOBAL: SWBF2 0x01ebd01c
// SYNTHETIC: SWBF2 0x00403ad0
// `dynamic initializer for 'FoleyFXImpact::rttiHashFoleyFXImpact''
PblHash FoleyFXImpact::rttiHashFoleyFXImpact("FoleyFXImpact");
char FoleyFXImpact::rttiNameFoleyFXImpact[] = "FoleyFXImpact";

// GLOBAL: SWBF2 0x007ec188
// SYNTHETIC: SWBF2 0x00403ab0
// `dynamic initializer for 'FoleyFXImpact::smFreeList''
// SYNTHETIC: SWBF2 0x007687e0
// `dynamic atexit destructor for 'FoleyFXImpact::smFreeList''
Snd::FreeList FoleyFXImpact::smFreeList(sizeof(FoleyFXImpact), 4, 8, 1);

// FUNCTION: SWBF2 0x0052c9e0
bool FoleyFXImpact::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashFoleyFXImpact || FoleyFX::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0052ca10
unsigned int FoleyFXImpact::GetDerivedRtti() const
{
    return rttiHashFoleyFXImpact;
}

// FUNCTION: SWBF2 0x0052ca20
const char* FoleyFXImpact::GetDerivedRttiName() const
{
    return rttiNameFoleyFXImpact;
}

// FUNCTION: SWBF2 0x0052ca30
FoleyFXImpact::FoleyFXImpact()
{
}

// FUNCTION: SWBF2 0x0052ca80
FoleyFXImpact::~FoleyFXImpact()
{
}

// FUNCTION: SWBF2 0x0052ca90
bool FoleyFXImpact::Read(PblFileChunk& chunk, unsigned int uChunkID)
{
    if (uChunkID != 0x1e71b297)
    {
        return FoleyFX::Read(chunk, uChunkID);
    }

    chunk._pFile->Read8();
    mImpact.SetID(chunk._pFile->Read32(), false, false);
    return true;
}

// SYNTHETIC: SWBF2 0x0052ca50
// FoleyFXImpact::`scalar deleting destructor'
