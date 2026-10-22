#include "sndsource2d.h"
#include "sndglobals.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblChunk.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074bd30
void Source2D::CalculateGain(float fGain, float fReverbGain,
                             GainMatrix& rMatrix) const
{
    if (mPan > 0.0f)
    {

        rMatrix.frontRight = fGain;
        rMatrix.frontLeft = (1.0f - mPan) * fGain;

    }
    else
    {
        rMatrix.frontLeft = fGain;
        rMatrix.frontRight = (mPan + 1.0f) * fGain;

    }

    rMatrix.rearLeft = rMatrix.frontLeft;
    rMatrix.rearRight = rMatrix.frontRight;
    rMatrix.reverb = fReverbGain;
}

// FUNCTION: SWBF2 0x0074bda0
bool Source2D::Read(PblFileChunk& chunk, unsigned int uiPropertyID)
{
    if (uiPropertyID != 0x4F55EEECu)
    {
        return false;
    }
    PblFile* const pFile = chunk._pFile;
    pFile->Read8();
    SetPan(Clamp(pFile->ReadFloat(), -1.0f, 1.0f));
    return true;
}

}
