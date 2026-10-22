#include "sndsourcebus.h"

#include "sndsourcetransport.h"
#include "sndbus.h"
#include "sndglobals.h"
#include "sndsource.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00738350
SourceBus::SourceBus()
    : mBus(&Bus::smRootBus),
      mGain(1.0f),
      mPlayState((unsigned char)SourceTransport::playState_stopped)
{
}

// FUNCTION: SWBF2 0x00738390
float SourceBus::GetGain() const
{
    return mGain;
}

// FUNCTION: SWBF2 0x00738410
void SourceBus::SetBus(Bus* pBus)
{
    mBus = pBus ? pBus : &Bus::smRootBus;
}

// FUNCTION: SWBF2 0x00738460
void SourceBus::Stop(SourceTransport* pTransport)
{
    if (pTransport->GetPlayState() != SourceTransport::playState_playing)
    {
        return;
    }

    if (mBus->mAttenuateBus && --mBus->mAttenuationRefCount == 0)
    {
        mBus->mAttenuateBus->Fade(mBus->mAttenuationLeadOutTime, 1.0f, -1.0f);
    }
    mPlayState = SourceTransport::playState_stopped;
}

// FUNCTION: SWBF2 0x007384b0
void SourceBus::Update(Source* pSource, SourceTransport* pTransport)
{
    if (pSource->GetGainCopied())
    {
        mGain = 1.0f;
    }
    else
    {
        pSource->SetGain(Clamp(mBus->mFinalGain * mGain, 0.0f, 1.0f));
    }

    if (mPlayState == SourceTransport::playState_playing
        && pTransport->GetPlayState() == SourceTransport::playState_stopped)
    {
        if (mBus->mAttenuateBus && --mBus->mAttenuationRefCount == 0)
        {
            mBus->mAttenuateBus->Fade(mBus->mAttenuationLeadOutTime, 1.0f, -1.0f);
        }
        mPlayState = SourceTransport::playState_stopped;
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x007383a0
void SourceBus::Play(SourceTransport* pTransport)
{
    if (pTransport->GetPlayState() != SourceTransport::playState_stopped)
    {
        return;
    }

    if (mBus->mAttenuateBus && mBus->mAttenuationRefCount++ == 0)
    {
        mBus->mAttenuateBus->Fade(mBus->mAttenuationLeadInTime,
                                  1.0f - mBus->mAttenuation, -1.0f);
    }
    mPlayState = SourceTransport::playState_playing;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00738430
void SourceBus::SetGain(Source* pSource, float fGain)
{
    mGain = fGain;
    pSource->SetGain(mBus->mFinalGain * fGain);
}
}
