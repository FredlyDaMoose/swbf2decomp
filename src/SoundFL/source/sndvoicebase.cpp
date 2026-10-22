#include "sndvoicebase.h"
#include "sndvoicevirtualmanager.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0073bfc0
VoiceBase::VoiceBase()
    : Source3D(0)
{

    SourceTransport::mFlags = (unsigned short)SourceTransport::flags_stopped;
    SourceTransport::mPlaybackPosition = 0;

    mNode.mNext = 0;
    mManager = 0;
    mSample = 0;
    mFlags = 0;

    SetGain(1.0f);
    SetPitch(1.0f);
    Source::mFlags = (unsigned char)((Source::mFlags & ~Source::flags_looping)
                                     | Source::flags_loopingChanged);
    SetReverbGain(0.0f);
    SetPan(0.0f);
    SetMinDistance(0.0f);
    SetMuteDistance(1.0f);
    SetMaxDistance(1.0f);

    Source3D::mProps = static_cast<Source3D::Properties*>(this);
    SetPosVel3D(&Globals::smNullVector, &Globals::smNullVector);

    Set3DMode(true);
}

// FUNCTION: SWBF2 0x0073c0b0
VoiceBase::~VoiceBase()
{
    SourceTransport::Stop();
    SetSample(0);
    if (mNode.mNext != 0)
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x0073c100
void VoiceBase::CopyParameters(const VoiceBase& rOther, bool bSample)
{
    Source::CopyParameters(rOther);
    Source2D::CopyParameters(rOther);
    Source3D::CopyParameters(rOther);
    Set3DMode(rOther.Get3DMode());
    if (bSample)
    {
        SetSample(rOther.mSample);
    }
}

// FUNCTION: SWBF2 0x0073c1a0
void VoiceBase::Lock(bool bLocked)
{
    if (bLocked)
    {
        mFlags = (unsigned char)(mFlags | flags_locked);
    }
    else
    {
        mFlags = (unsigned char)(mFlags & ~flags_locked);
    }
}

// FUNCTION: SWBF2 0x0073c1c0
void VoiceBase::RemoveFromManager()
{
    VoiceVirtualManager* const pManager = mManager;

    mNode.Remove();

    mManager = 0;
    --pManager->mNumManagedVoices;
}

// FUNCTION: SWBF2 0x0073c1f0
void VoiceBase::SetSample(const Sample* pSample)
{
    if (pSample != mSample)
    {
        mFlags |= flags_updateSample;
        mSample = const_cast<Sample*>(pSample);
    }
}

}
