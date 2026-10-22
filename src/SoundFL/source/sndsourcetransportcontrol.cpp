#include "sndsourcetransportcontrol.h"

#include "sndglobals.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "sndenginebase.h"
#include "PebbleFL/Common/PblRandom.h"

namespace Snd
{
extern PblRandom sndRandom;
}

namespace Snd
{

// FUNCTION: SWBF2 0x0074b510
SourceTransportControl::SourceTransportControl()
    : mPlayProbability(1.0f),
      mPlayInterval(0.0f),
      mPlayIntervalDev(0.0f),
      mPlayIntervalNext(0.0f),
      mRandomPlaybackPosition(0)
{
}

// FUNCTION: SWBF2 0x0074b650
bool SourceTransportControl::Read(PblFileChunk& chunk, unsigned int uiPropertyID)
{
    PblFile* const pFile = chunk._pFile;
    switch (uiPropertyID)
    {
    case 0xA7930B6Eu:
        pFile->Read8();
        mPlayProbability = Clamp(pFile->ReadFloat(), 0.0f, 1.0f);
        return true;
    case 0x010B9ECAu:
        pFile->Read8();
        mPlayInterval = pFile->ReadFloat();
        return true;
    case 0x3AF79D76u:
        pFile->Read8();
        SetRandomPlayPosition(pFile->ReadFloat() == 1.0f);
        return true;
    case 0xEDCE1E71u:
        pFile->Read8();
        mPlayIntervalDev = pFile->ReadFloat();
        return true;
    }
    return false;
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0074b540
bool SourceTransportControl::DeterminePlaySource(bool bArg) const
{
    const float fTime = EngineBase::GetTimeElapsed();
    if (fTime < mPlayIntervalNext)
    {
        return false;
    }

    if (mPlayInterval != 0.0f || mPlayIntervalDev != 0.0f)
    {
        const_cast<SourceTransportControl*>(this)->mPlayIntervalNext =
            (sndRandom.GetFloat() - 0.5f) * mPlayIntervalDev * 2.0f + (fTime + mPlayInterval);
    }

    if (bArg || mPlayProbability == 1.0f)
    {
        return true;
    }
    return mPlayProbability >= sndRandom.GetFloat();
}
}
