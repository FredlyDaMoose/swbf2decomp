#include "sndsourcevirtual.h"

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "sndsourcetransport.h"
#include "sndsource2d3d.h"
#include "sndsourcei3dl2.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074b4a0
bool SourceVirtual::Read(PblFileChunk& chunk, unsigned int uiPropertyID)
{
    PblFile* const pFile = chunk._pFile;
    if (uiPropertyID == 0x94E4E309u)
    {
        pFile->Read8();
        SetPriority(pFile->ReadFloat());
        return true;
    }
    if (uiPropertyID == 0xBA467EC4u)
    {
        pFile->Read8();
        SetBias(pFile->ReadFloat());
        return true;
    }
    return false;
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0074b380
float SourceVirtual::CalculatePriority(unsigned char ePlayState, float fArg, bool bLooping,
                                       float fBusGain) const
{
    if (ePlayState != SourceTransport::playState_playing)
    {
        return 0.0f;
    }
    float fScale = 1.0f;
    if (!bLooping)
    {
        fScale = (1.0f - fArg) * 0.0099999998f + 0.99000001f;
    }
    return ((1.0f - mBias) * fBusGain + mBias * mPriority) * fScale;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0074b3e0
float SourceVirtual::CalculatePriority(const SourceTransport* pTransport,
                                       const Source2D3D* pSource2D3D,
                                       const Source3D* pSource3D,
                                       const SourceI3DL2* pSourceI3DL2,
                                       const Sample* pSample, float fScale,
                                       bool bArg) const
{
    (void)pSample;
    float fGain = pSource2D3D->mGain;
    if (pSource2D3D->Get3DMode() && fGain > 0.0f)
    {
        float f3DGain = fGain;
        pSource3D->CalculateGain(f3DGain);
        fGain = f3DGain * fGain;
    }
    if (pSourceI3DL2)
    {
        fGain = pSourceI3DL2->CalculateGainEstimate() * fGain;
    }
    if (!bArg && (unsigned char)pTransport->GetPlayState() != SourceTransport::playState_playing)
    {
        return 0.0f;
    }
    return (1.0f - mBias) * (fGain * fScale) + mBias * mPriority;
}
}
