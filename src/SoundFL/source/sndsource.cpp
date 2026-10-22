#include "sndsource.h"

#include <float.h>

#include "sndglobals.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074dee0
bool Source::Read(PblFileChunk& chunk, unsigned int uiPropertyID)
{
    PblFile* const pFile = chunk._pFile;
    switch (uiPropertyID)
    {
    case 0xBD324AB1u:
        pFile->Read8();
        SetPitch(Clamp(pFile->ReadFloat(), 0.0f, FLT_MAX));
        return true;
    case 0x1B5426FEu:
        pFile->Read8();
        SetGain(Clamp(pFile->ReadFloat(), 0.0f, 1.0f));
        return true;
    case 0x6C408CFEu:
        pFile->Read8();
        SetReverbGain(Clamp(pFile->ReadFloat(), 0.0f, 1.0f));
        return true;
    case 0x7593992Du:
        pFile->Read8();
        SetLooping(pFile->ReadFloat() > 0.0f);
        return true;
    }
    return false;
}

}
