#include "sndvoicevirtual.h"

#include "sndvoicevirtualmanager.h"
#include "sndenginebase.h"
#include "win/sndsamplebank.h"
#include "sndsourcei3dl2.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00737be0
void VoiceVirtual::Pause()
{
    if (mFlags & flags_active)
    {
        mVoiceManager->RemoveVoice(this);
        mFlags &= ~(unsigned int)flags_active;
        Inactive();
        Inactive();
    }
    SourceTransport::Pause();
}

// FUNCTION: SWBF2 0x00737990
VoiceVirtual::VoiceVirtual(unsigned int uiCustomFlags,
                           VoiceVirtualManager* pManager,
                           bool bRestorePlaybackPos)
    : VoiceBase(),
      SourceVirtual(),
      mNode(0.0f),
      mFlags(0),
      mVoice(0),
      mCustomFlags(uiCustomFlags),
      mVoiceManager(pManager),
      mInactiveCallback(0),
      mInactiveCallbackData(0)
{
    if (!mVoiceManager)
    {
        mVoiceManager =
            EngineBase::GetVoiceVirtualManager((Source2D3D::mFlags & 1) != 0);
    }

    mFlags |= (unsigned int)((bRestorePlaybackPos ? flags_restorePlaybackPos : 0)
                             | flags_updatePriority);

    mVoiceManager->mInactiveList.Add(&mNode);
}

// FUNCTION: SWBF2 0x00737b20
float VoiceVirtual::CalculatePriority(bool bForce)
{
    if (!(mFlags & flags_updatePriority)
        && !GetPriorityChanged()
        && !GetBiasChanged()
        && !bForce)
    {
        return mNode.mSortValue;
    }

    if (Get3DMode())
    {
        DetermineClosestListener();
    }

    SourceI3DL2 sourceI3DL2;
    const SourceI3DL2* pSourceI3DL2 = 0;
    if (mClosestListener)
    {
        sourceI3DL2.Update(mClosestListener, mPos3D, Source3D::mProps->mSpaceDistance,
                           true, 0);
        pSourceI3DL2 = &sourceI3DL2;
    }
    mNode.mSortValue = SourceVirtual::CalculatePriority(this, this, this, pSourceI3DL2,
                                                        mSample, 1.0f, bForce);
    mFlags &= ~(unsigned int)flags_updatePriority;
    return mNode.mSortValue;
}

// FUNCTION: SWBF2 0x00737cd0
void VoiceVirtual::Remove()
{
    mVoiceManager->RemoveVoice(this);
    mFlags &= ~(unsigned int)flags_active;
    Inactive();
}

// FUNCTION: SWBF2 0x00737d20
void VoiceVirtual::RemoveAll()
{
    Stop();
    SetSample(0);
    SetVoice(0);
    if (mNode.mNode.IsAttached())
    {
        mNode.mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x00737e30
void VoiceVirtual::SetVoice(VoiceBase* pVoice)
{
    mFlags |= flags_updateVoice;
    if (mVoice)
    {
        mVoice->SourceTransport::Stop();
        mVoice->Lock(false);
    }
    if (pVoice)
    {
        pVoice->Lock(true);
    }
    mVoice = pVoice;
}

// FUNCTION: SWBF2 0x00737f30
void VoiceVirtual::Stop()
{
    SourceTransport::Stop();
    if (mVoice)
    {
        mVoice->SourceTransport::Stop();
    }
    if (GetActive())
    {
        Remove();
    }
    Inactive();
}

// FUNCTION: SWBF2 0x00737ff0
void VoiceVirtual::Update3DMode()
{
    VoiceVirtualManager* p2DManager = EngineBase::GetVoiceVirtualManager(false);
    VoiceVirtualManager* p3DManager = EngineBase::GetVoiceVirtualManager(true);
    VoiceVirtualManager* pManager = Get3DMode() ? p3DManager : p2DManager;

    if ((mVoiceManager == p2DManager || mVoiceManager == p3DManager)
        && pManager != mVoiceManager)
    {
        Sample* pSample = mSample;
        RemoveAll();
        mVoiceManager = pManager;
        pManager->mInactiveList.Add(&mNode);
        Stop();
        SetSample(pSample);
        if (GetActive())
        {

            SourceTransport::Play(true);
            mVoiceManager->InsertVoice(this);
            mFlags |= flags_active;
        }
    }
}

// FUNCTION: SWBF2 0x007380b0
void VoiceVirtual::Update(float fDeltaTime)
{
    const Sample* pSample = mSample;

    if (Get3DModeChanged())
    {
        Update3DMode();
    }
    ResetPriority();

    if (GetPlayState() == playState_playing && mVoice)
    {
        if (GetVoiceChanged())
        {
            mVoice->SourceTransport::Stop();
            mVoice->SetSample(mSample);
            mVoice->SourceTransport::Play(true);
            if (mFlags & flags_restorePlaybackPos)
            {
                mVoice->mPlaybackPosition = mPlaybackPosition;
                mVoice->SourceTransport::mFlags |= flags_updatePlaybackPosition;
            }
        }
        mVoice->CopyParameters(*this, false);
    }

    switch (GetDesiredPlayState())
    {
    case playState_stopped:
        SetPlayState(playState_stopped);
        mPlaybackPosition = 0;
        Inactive();
        break;
    case playState_paused:
        SetPlayState(playState_paused);
        if (mVoice)
        {
            mVoice->SourceTransport::Pause();
        }
        break;
    case playState_playing:
        SetPlayState(playState_playing);
        if (mVoice && GetPlaybackPositionChanged())
        {
            mVoice->mPlaybackPosition = mPlaybackPosition;
            mVoice->SourceTransport::mFlags |= flags_updatePlaybackPosition;
        }
        break;
    }

    if (!pSample)
    {
        return;
    }

    const PlayState ePlayState = GetPlayState();
    if (mVoice)
    {
        mPlaybackPosition = mVoice->mPlaybackPosition;
        SetPlayState(mVoice->GetPlayState());
    }
    else if (ePlayState == playState_playing)
    {
        unsigned int uiPos = mPlaybackPosition;
        const unsigned int uiLength = pSample->mLengthSamples;
        DetermineClosestListener();
        uiPos += (unsigned int)(Source2D3D::CalculatePitch(this) * fDeltaTime
                                * (float)pSample->mFrequency);
        if (uiPos >= uiLength)
        {
            if (GetLooping())
            {
                uiPos -= uiLength;
            }
            else
            {
                uiPos = 0;
                Stop();
            }
        }
        mPlaybackPosition = uiPos;
    }

    if (GetActive() && ePlayState == playState_playing
        && GetPlayState() == playState_stopped)
    {
        Stop();
        mPlaybackPosition = 0;
        Remove();
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x00737a90
VoiceVirtual::~VoiceVirtual()
{
    RemoveAll();
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00737c80
void VoiceVirtual::Play(bool bRestart)
{
    SourceTransport::mFlags = (unsigned short)((SourceTransport::mFlags
                                                & ~(unsigned int)(flags_updateStop | flags_updatePause))
                                               | (unsigned int)flags_updatePlay);
    if ((SourceTransport::mFlags & flags_updatePlaybackPosition) == 0 && bRestart)
    {
        mPlaybackPosition = 0;
        SourceTransport::mFlags = (unsigned short)(SourceTransport::mFlags
                                                   | (unsigned int)flags_updatePlaybackPosition);
    }
    mVoiceManager->InsertVoice(this);
    mFlags |= flags_active;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00737dd0
void VoiceVirtual::Set3DMode(bool b3DMode)
{
    Source2D3D::Set3DMode(b3DMode);
    Update3DMode();
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00737e10
void VoiceVirtual::SetInactiveCallback(void (__cdecl* pfnCallback)(const VoiceVirtual* const, void* const),
                                       void* const pData)
{
    mInactiveCallback = (void (*)())pfnCallback;
    mInactiveCallbackData = pData;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00737e90
void VoiceVirtual::SetVoiceVirtualManager(VoiceVirtualManager* pManager)
{
    Stop();
    SetVoice(0);
    mNode.mNode.Remove();
    mVoiceManager = pManager;
    pManager->mInactiveList.Add(&mNode);
}
}
