#include "sndvoicehardware.h"

#include "win/sndvoice.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074c5b0
void VoiceHardware::Update(Voice* pVoice)
{
    const PlayState eDesiredPlayState = GetDesiredPlayState();
    const PlayState eCurrentPlayState = GetCurrentPlayState();
    const Sample* const pSample = mSample;

    if (eDesiredPlayState == playState_playing)
    {
        SourceI3DL2::mFlags |= SourceI3DL2::flags_forceUpdate;
        if (eCurrentPlayState == playState_playing)
        {
            Voice::UpdatePlayState(pVoice, playState_stopped);
        }
    }

    if (pSample != 0
        && (eCurrentPlayState == playState_playing
            || eDesiredPlayState == playState_playing))
    {
        if (Get3DMode())
        {
            Source3D::mFlags |= flags_updateListener;
            DetermineClosestListener();

            GainMatrix gainMatrix;
            Source3D::CalculateGain(gainMatrix.frontLeft);
            gainMatrix.frontLeft *= mGain;
            SourceI3DL2Interpolated::Update(mClosestListener, mPos3D,
                                            mProps->mSpaceDistance,
                                            eDesiredPlayState != playState_playing);
            Voice::UpdateGain(pVoice, gainMatrix, false);
            Voice::UpdatePitch(pVoice, mPitch * Source3D::CalculatePitch());
        }
        else
        {
            const bool bPanChanged = GetPanChanged();
            if (GetGainChanged() || GetReverbGainChanged() || bPanChanged)
            {
                GainMatrix gainMatrix;
                Source2D::CalculateGain(mGain, mReverbGain, gainMatrix);
                Voice::UpdateGain(pVoice, gainMatrix, bPanChanged);
            }
            if (GetPitchChanged())
            {
                Voice::UpdatePitch(pVoice, mPitch);
            }
        }
    }

    if (eDesiredPlayState != playState_invalid
        && (eDesiredPlayState != playState_playing || pSample != 0))
    {
        Voice::UpdatePlayState(pVoice, eDesiredPlayState);
    }
}

}
