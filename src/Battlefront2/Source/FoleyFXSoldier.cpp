#include "FoleyFXSoldier.h"

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

// GLOBAL: SWBF2 0x01ebd13c
// SYNTHETIC: SWBF2 0x00403b10
// `dynamic initializer for 'FoleyFXSoldier::rttiHashFoleyFXSoldier''
PblHash FoleyFXSoldier::rttiHashFoleyFXSoldier("FoleyFXSoldier");
char FoleyFXSoldier::rttiNameFoleyFXSoldier[] = "FoleyFXSoldier";

// GLOBAL: SWBF2 0x007ec1a4
// SYNTHETIC: SWBF2 0x00403af0
// `dynamic initializer for 'FoleyFXSoldier::smFreeList''
// SYNTHETIC: SWBF2 0x00768800
// `dynamic atexit destructor for 'FoleyFXSoldier::smFreeList''
Snd::FreeList FoleyFXSoldier::smFreeList(sizeof(FoleyFXSoldier), 4, 8, 1);

// FUNCTION: SWBF2 0x0052cc40
bool FoleyFXSoldier::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashFoleyFXSoldier || FoleyFX::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0052cc70
unsigned int FoleyFXSoldier::GetDerivedRtti() const
{
    return rttiHashFoleyFXSoldier;
}

// FUNCTION: SWBF2 0x0052cc80
const char* FoleyFXSoldier::GetDerivedRttiName() const
{
    return rttiNameFoleyFXSoldier;
}

// FUNCTION: SWBF2 0x0052cc90
FoleyFXSoldier::FoleyFXSoldier()
{
}

// SYNTHETIC: SWBF2 0x0052cd20
// FoleyFXSoldier::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0052cd50
FoleyFXSoldier::~FoleyFXSoldier()
{
}

// FUNCTION: SWBF2 0x0052cd60
bool FoleyFXSoldier::Read(PblFileChunk& chunk, unsigned int uChunkID)
{
    GameSound* pSound;
    switch (uChunkID)
    {
    case 0x2a9f6673: pSound = &mFootstepLeftWalk;  break;
    case 0xe79eb7c2: pSound = &mFootstepRightWalk; break;
    case 0x640f6efb: pSound = &mFootstepLeftRun;   break;
    case 0xa4df1b00: pSound = &mFootstepRightRun;  break;
    case 0xb59a8702: pSound = &mFootstepLeftStop;  break;
    case 0x54e8af1f: pSound = &mFootstepRightStop; break;
    case 0xa73f5c0d: pSound = &mJump;              break;
    case 0x6dea369c: pSound = &mLand;              break;
    case 0x16ff3d1e: pSound = &mRoll;              break;
    case 0x6dd99a25: pSound = &mProne;             break;
    case 0x65bfa4e1: pSound = &mSquat;             break;
    case 0x75493323: pSound = &mStand;             break;
    case 0x70dc49d0: pSound = &mBodyFall;          break;
    default:
        return FoleyFXImpact::Read(chunk, uChunkID);
    }

    chunk._pFile->Read8();
    pSound->SetID(chunk._pFile->Read32(), false, false);
    return true;
}
