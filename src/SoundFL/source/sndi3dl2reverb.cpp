#include "sndi3dl2reverb.h"

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "sndglobals.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0073bb50
I3DL2Reverb::I3DL2Reverb()
{
    mRoomGain = 0.0f;
    mRoomHFGain = 0.0f;
    mRoomRollOff = 0.0f;
    mDecayTime = 1.0f;
    mDecayHFRatio = 0.5f;
    mERGain = 0.0f;
    mERDelay = 0.02f;
    mReverbGain = 0.0f;
    mReverbDelay = 0.04f;
    mDiffusion = 100.0f;
    mDensity = 100.0f;
    mHFReference = 5000.0f;
}

// FUNCTION: SWBF2 0x0073bbb0
bool I3DL2Reverb::Read(PblFileChunk& chunk, unsigned int uiPropertyID)
{
    PblFile* const pFile = chunk._pFile;
    switch (uiPropertyID)
    {
    case 0x5676AB7Du:
        pFile->Read8();
        mRoomGain = Clamp(pFile->ReadFloat(), 0.0f, 1.0f);
        mFlags |= flags_roomGainChanged;
        return true;
    case 0x89695A93u:
        pFile->Read8();
        mRoomHFGain = Clamp(pFile->ReadFloat(), 0.0f, 1.0f);
        mFlags |= flags_roomHFGainChanged;
        return true;
    case 0xB3571EDAu:
        pFile->Read8();
        mRoomRollOff = Clamp(pFile->ReadFloat(), 0.0f, 10.0f);
        mFlags |= flags_roomRollOffChanged;
        return true;
    case 0x1C098B3Cu:
        pFile->Read8();
        mDecayTime = Clamp(pFile->ReadFloat(), 0.01f, 20.0f);
        mFlags |= flags_decayTimeChanged;
        return true;
    case 0x80D9B27Eu:
        pFile->Read8();
        mDecayHFRatio = Clamp(pFile->ReadFloat(), 0.01f, 2.0f);
        mFlags |= flags_decayHFRatioChanged;
        return true;
    case 0xF2C65989u:
        pFile->Read8();
        mERGain = Clamp(pFile->ReadFloat(), 0.0f, 1.0f);
        mFlags |= flags_erGainChanged;
        return true;
    case 0xBE463E19u:
        pFile->Read8();
        mERDelay = Clamp(pFile->ReadFloat(), 0.0f, 0.3f);
        mFlags |= flags_erDelayChanged;
        return true;
    case 0x6C408CFEu:
        pFile->Read8();
        mReverbGain = Clamp(pFile->ReadFloat(), 0.0f, 10.0f);
        mFlags |= flags_reverbGainChanged;
        return true;
    case 0xB2F683D8u:
        pFile->Read8();
        mReverbDelay = Clamp(pFile->ReadFloat(), 0.0f, 0.1f);
        mFlags |= flags_reverbDelayChanged;
        return true;
    case 0xC8A3E804u:
        pFile->Read8();
        mDiffusion = Clamp(pFile->ReadFloat(), 0.0f, 100.0f);
        mFlags |= flags_diffusionChanged;
        return true;
    case 0x72B9059Bu:
        pFile->Read8();
        mDensity = Clamp(pFile->ReadFloat(), 0.0f, 100.0f);
        mFlags |= flags_densityChanged;
        return true;
    case 0xE1A099A8u:
        pFile->Read8();
        mHFReference = Clamp(pFile->ReadFloat(), 20.0f, 20000.0f);
        mFlags |= flags_hrReferenceChanged;
        return true;
    }
    return false;
}

}
