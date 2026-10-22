#include "sndsound.h"
#include "sndbus.h"

#include "sndglobals.h"
#include "sndsamplebankbase.h"
#include "sndvoicevirtualmanager.h"
#include "sndenginebase.h"
#include "sndlistenerbase.h"
#include "sndsourcei3dl2.h"
#include "win/sndsamplebank.h"
#include "PebbleFL/Common/PblRandom.h"
#include <stddef.h>
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

namespace Snd
{

Sound::FireForget* Sound::smVoiceVirtuals;
unsigned char Sound::smVoiceVirtualStorage[12800];

// SYNTHETIC: SWBF2 0x00739c30
// Snd::Source3D::`default constructor closure'

// GLOBAL: SWBF2 0x01e2e9d0
// SYNTHETIC: SWBF2 0x0040bca0
// `dynamic initializer for 'Snd::Sound::smSource3Ds''
// SYNTHETIC: SWBF2 0x0076a440
// `dynamic atexit destructor for 'Snd::Sound::smSource3Ds''
Source3D Sound::smSource3Ds[64];

// GLOBAL: SWBF2 0x01e2e6c8
// SYNTHETIC: SWBF2 0x0040bc60
// `dynamic initializer for 'Snd::Sound::smSource3DPositions''

::PblVector3 Sound::smSource3DPositions[64];
// GLOBAL: SWBF2 0x01e2f3d0
// SYNTHETIC: SWBF2 0x0040bc80
// `dynamic initializer for 'Snd::Sound::smSource3DVelocities''

::PblVector3 Sound::smSource3DVelocities[64];

// FUNCTION: SWBF2 0x00739f50
void Sound::Open()
{
    smVoiceVirtuals = (FireForget*)smVoiceVirtualStorage;

    for (unsigned int i = 0; i < 64; ++i)
    {
        new (&smVoiceVirtuals[i], false)
            FireForget((unsigned int)(unsigned long)smVoiceVirtuals, 0, true);

        ::PblVector3* pPos = &smSource3DPositions[i];
        smSource3Ds[i].mFlags |= (unsigned char)Source3D::flags_pos3DChanged;
        smSource3Ds[i].mPos3D = pPos ? pPos : &Globals::smNullVector;

        ::PblVector3* pVel = &smSource3DVelocities[i];
        smSource3Ds[i].mFlags |= (unsigned char)Source3D::flags_vel3DChanged;
        smSource3Ds[i].mVel3D = pVel ? pVel : &Globals::smNullVector;
    }
}

// FUNCTION: SWBF2 0x00739c40
void Sound::Close()
{
    for (unsigned int i = 0; i < 64; ++i)
    {
        delete &smVoiceVirtuals[i];
    }

    smVoiceVirtuals = 0;
}

// GLOBAL: SWBF2 0x007e36dc
// SYNTHETIC: SWBF2 0x0040bc20
// `dynamic initializer for 'Snd::Sound::Properties::smFreeList''
// SYNTHETIC: SWBF2 0x0076a410
// `dynamic atexit destructor for 'Snd::Sound::Properties::smFreeList''
FreeList Sound::Properties::smFreeList(sizeof(Sound::Properties), 1, 8, 1);

// GLOBAL: SWBF2 0x007e36f8
// SYNTHETIC: SWBF2 0x0040bc40
// `dynamic initializer for 'Snd::Sound::Properties::smPropertyList''
// SYNTHETIC: SWBF2 0x0076a420
// `dynamic atexit destructor for 'Snd::Sound::Properties::smPropertyList''
PblListSingle Sound::Properties::smPropertyList;

// GLOBAL: SWBF2 0x007e36fc
// SYNTHETIC: SWBF2 0x0040bc50
// `dynamic initializer for 'Snd::Sound::smSoundList''
// SYNTHETIC: SWBF2 0x0076a430
// `dynamic atexit destructor for 'Snd::Sound::smSoundList''
PblListSingle Sound::smSoundList;

// FUNCTION: SWBF2 0x00739b70
Sound::Properties::Properties()
{
    mSelObjectWeightList = &mObjectWeightList;
    smPropertyList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x00739c10
void __cdecl Sound::Properties::operator delete(void* p)
{
    smFreeList.Free(p);
}

// FUNCTION: SWBF2 0x00739e70
void __cdecl Sound::Properties::Load(PblFileChunk& chunk, bool bOnce)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x41544144u)
        {
            continue;
        }

        child._pFile->Read32();
        child = chunk.ReadNextChild();

        Properties* const pProperties = new Properties;
        pProperties->Read(child);
        if (bOnce)
        {
            break;
        }
    }
}

// FUNCTION: SWBF2 0x0073a750
void Sound::Properties::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uiID = child._pFile->Read32();
        if (uiID == 0x4143ACAFu)
        {
            child = chunk.ReadNextChild();
            if (mObjectWeightList.ReadObjectWeights(child, 0x96E382A7u,
                                                    &SampleFindByHashID))
            {
                mSelObjectWeightList = &mObjectWeightList;
            }
        }
        else if (uiID == 0xCA04EFE0u)
        {
            child._pFile->Read8();
            const unsigned int uiParentID = child._pFile->Read32();
            for (PblListSingle::Node* pNode = smPropertyList.mHead.mNext;
                 pNode != &smPropertyList.mHead; pNode = pNode->mNext)
            {
                Properties* const pParent = reinterpret_cast<Properties*>(reinterpret_cast<char*>(pNode) - offsetof(Properties, mNode));
                if (pParent->mHashID != uiParentID)
                {
                    continue;
                }

                mSelObjectWeightList = &pParent->mObjectWeightList;
                SetPitch(pParent->mPitch);
                mPitchDev = pParent->mPitchDev;
                SetGain(pParent->mGain);
                mGainDev = pParent->mGainDev;
                SetReverbGain(pParent->mReverbGain);
                mBus = pParent->mBus;
                SetLooping(pParent->GetLooping());
                SetPan(pParent->mPan);
                SetRollIn(pParent->mRollInFactor);
                SetRollInDistance(pParent->mRollInDistance);
                SetMinDistance(pParent->mMinDistance);
                SetMuteDistance(pParent->mMuteDistance);
                SetMaxDistance(pParent->mMaxDistance);
                SetRollOff(pParent->mRollOffFactor);
                Set3DMode(pParent->Get3DMode());
                SetDopplerMode(pParent->GetDopplerMode());
                SetBias(pParent->mBias);
                SetPriority(pParent->mPriority);
                mPlayProbability = pParent->mPlayProbability;
                mPlayInterval = pParent->mPlayInterval;
                mPlayIntervalDev = pParent->mPlayIntervalDev;
                SetRandomPlayPosition(pParent->GetRandomPlayPosition());
                break;
            }
        }
        else if (!SoundProperties::Read(child, uiID)
                 && !Source2D3D::Read(child, uiID)
                 && !SourceVirtual::Read(child, uiID)
                 && !SourceVariation::Read(child, uiID))
        {
            mObjectWeightList.Read(child, uiID);
        }
    }
}

// FUNCTION: SWBF2 0x0073aa60
void const* __cdecl Sound::Properties::SampleFindByHashID(unsigned int uiHashID)
{
    return SampleBankBase::AllBanksFindSampleByHashID(uiHashID, false);
}

// FUNCTION: SWBF2 0x0073a9d0
void __cdecl Sound::RemoveAll()
{
    while (!smSoundList.IsEmpty())
    {
        Sound* const pSound = (Sound*)((char*)smSoundList.mHead.mNext - offsetof(Sound, mNode));
        pSound->VoiceVirtual::RemoveAll();
        pSound->SourceBus::Stop(pSound);
        pSound->VoiceVirtual::Stop();
        pSound->SetBus(&Bus::smRootBus);
        pSound->mProperties = 0;
        pSound->mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x0073ad90
void __cdecl Sound::StopAll()
{
    for (unsigned int i = 0; i < 64; ++i)
    {
        if (smVoiceVirtuals[i].GetPlayState() != SourceTransport::playState_stopped)
        {
            FireForget* const pVoice = &smVoiceVirtuals[i];
            pVoice->SourceBus::Stop(pVoice);
            pVoice->VoiceVirtual::Stop();
        }
        smVoiceVirtuals[i].SetSample(0);
        smVoiceVirtuals[i].SetBus(0);
    }
}

extern PblRandom sndRandom;

// FUNCTION: SWBF2 0x00739df0
VoiceVirtual* __cdecl Sound::GetFreeVoice(VoiceVirtualManager* pManager)
{
    for (PblListDouble::Node* pLink = pManager->mInactiveList.mTerminator.mNext;
         pLink != &pManager->mInactiveList.mTerminator; pLink = pLink->mNext)
    {
        PblListDoubleSorted::Node* pNode =
            (PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode));
        if (pNode == pManager->mInactiveList.GetTerminator())
        {
            break;
        }
        VoiceVirtual* pVoice = (VoiceVirtual*)((char*)pNode - offsetof(VoiceVirtual, mNode));
        if (pVoice->mCustomFlags == (unsigned int)smVoiceVirtuals)
        {
            return pVoice;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0073acf0
VoiceVirtual* __cdecl Sound::StealVoice(VoiceVirtualManager* pManager, float fPriority)
{
    PblListDoubleSorted& list = pManager->mActiveList;
    PblListDouble::Node* pTail = list.mTerminator.mPrev;
    PblListDoubleSorted::Node* pTailNode = pTail == &list.mTerminator
        ? (PblListDoubleSorted::Node*)list.GetTerminator()
        : (PblListDoubleSorted::Node*)((char*)pTail - offsetof(PblListDoubleSorted::Node, mNode));
    for (PblListDouble::Node* pLink = &pTailNode->mNode; pLink != &list.mTerminator; pLink = pLink->mPrev)
    {
        PblListDoubleSorted::Node* pNode =
            (PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode));
        if (pNode == list.GetTerminator())
        {
            break;
        }
        VoiceVirtual* pVoice = (VoiceVirtual*)((char*)pNode - offsetof(VoiceVirtual, mNode));
        if (pVoice->mCustomFlags != (unsigned int)smVoiceVirtuals)
        {
            continue;
        }

        if (!(fPriority > pVoice->CalculatePriority(false)))
        {
            return 0;
        }
        FireForget* pFireForget = (FireForget*)pVoice;
        if (pFireForget->mCallback)
        {
            pFireForget->mCallback(pVoice, pFireForget->mCallbackData);
            pFireForget->mCallback = 0;
            pFireForget->mCallbackData = 0;
        }
        pVoice->Stop();
        pVoice->ResetPriority();
        return pVoice;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00739e30
void __cdecl Sound::FireForget::InactiveCallback(const VoiceVirtual* const pVoice, void* const pData)
{
    FireForget* pThis = (FireForget*)pData;
    if (pThis->mCallback)
    {
        pThis->mCallback(pThis, pThis->mCallbackData);
        pThis->SetStolenCallback(0, 0);
    }
}

// FUNCTION: SWBF2 0x0073af80
void __cdecl Sound::VoiceVirtualRelease(VoiceVirtual* pVoice)
{
    FireForget* pThis = static_cast<FireForget*>(pVoice);
    pThis->SetStolenCallback(0, 0);
    pVoice->SetInactiveCallback(&FireForget::InactiveCallback, pVoice);
}

// FUNCTION: SWBF2 0x0073a430
VoiceVirtual* __cdecl Sound::Play(Source3D* pSource3D, const Properties* pProps,
                                  void (__cdecl* pStolenCallback)(VoiceVirtual*, void*),
                                  void* pStolenCallbackData, bool bArg)
{
    SourceTransport transport;
    transport.mPlaybackPosition = 0;
    transport.mFlags = SourceTransport::flags_stopped;

    if (!pProps->DeterminePlaySource(false))
    {
        return 0;
    }
    const Sample* pSample = (const Sample*)pProps->mSelObjectWeightList->GetObjectA();
    if (!pSample)
    {
        return 0;
    }
    transport.Play(true);

    float fGain;
    if (pSource3D)
    {
        pSource3D->mFlags |= Source3D::flags_updateListener;
        pSource3D->SetProperties(const_cast<Properties*>(pProps));
        pSource3D->DetermineClosestListener();
        pSource3D->CalculateGain(fGain);
        if (fGain != 0.0f)
        {
            SourceI3DL2 sourceI3DL2;
            sourceI3DL2.Update(ListenerBase::GetClosestListener(pSource3D->mPos3D, 0, 0),
                               pSource3D->mPos3D, pProps->mSpaceDistance, true, 0);
            fGain *= sourceI3DL2.CalculateGainEstimate();
        }
    }
    else
    {
        fGain = pProps->mGain;
    }
    if (fGain == 0.0f && bArg)
    {
        return 0;
    }

    bool bNewVoice = true;
    VoiceVirtualManager* p2DManager = EngineBase::GetVoiceVirtualManager(false);
    VoiceVirtualManager* p3DManager = EngineBase::GetVoiceVirtualManager(true);
    const bool bTwoManagers = p2DManager != p3DManager;
    const bool b3D = pProps->Get3DMode();

    FireForget* pVoice = (FireForget*)GetFreeVoice(EngineBase::GetVoiceVirtualManager(b3D));
    if (!pVoice && bTwoManagers)
    {
        pVoice = (FireForget*)GetFreeVoice(EngineBase::GetVoiceVirtualManager(!b3D));
    }
    if (!pVoice)
    {
        const float fPriority = pProps->SourceVirtual::CalculatePriority(
            (unsigned char)transport.GetPlayState(), 0.0f, pProps->GetLooping(),
            pProps->mBus->mFinalGain);
        FireForget* pVoice2D = (FireForget*)StealVoice(p2DManager, fPriority);
        FireForget* pVoice3D = bTwoManagers ? (FireForget*)StealVoice(p3DManager, fPriority) : 0;
        if (!pVoice2D)
        {
            if (!pVoice3D)
            {
                return 0;
            }
            pVoice = pVoice3D;
        }
        else if (pVoice3D)
        {
            const float fPriority2D = pVoice2D->CalculatePriority(false);
            const float fPriority3D = pVoice3D->CalculatePriority(false);
            pVoice = fPriority2D > fPriority3D ? pVoice3D : pVoice2D;
        }
        else
        {
            pVoice = pVoice2D;
        }
        bNewVoice = false;
    }

    if (pSource3D)
    {
        const unsigned int i = (unsigned int)(pVoice - smVoiceVirtuals);
        smSource3DPositions[i] = *pSource3D->mPos3D;
        smSource3DVelocities[i] = *pSource3D->mVel3D;
        smSource3Ds[i].CopyParameters(*pSource3D);
        pSource3D = &smSource3Ds[i];
        pSource3D->SetPosition(&smSource3DPositions[i]);
        pSource3D->SetVelocity(&smSource3DVelocities[i]);
    }

    pProps->SetupVoiceVirtual(pVoice, pSource3D, pSample);
    if (bNewVoice)
    {
        pVoice->CalculatePriority(true);
    }
    pVoice->SetStolenCallback(pStolenCallback, pStolenCallbackData);
    pVoice->SetInactiveCallback(FireForget::InactiveCallback, pVoice);
    pVoice->VoiceVirtual::Play(true);
    if (pSource3D)
    {
        pSource3D->SetProperties(0);
    }

    return pStolenCallback ? pVoice : 0;
}

// FUNCTION: SWBF2 0x0073aa80
void Sound::Properties::SetupVoiceVirtual(VoiceVirtualBus* pVoice, Source3D* pSource3D,
                                          const Sample* pSample) const
{
    if (!pSample)
    {
        pSample = (const Sample*)mSelObjectWeightList->GetObjectA();
    }

    pVoice->SourceBus::Stop(pVoice);
    pVoice->VoiceVirtual::Stop();
    pVoice->SetSample(pSample);
    pVoice->SetLooping(GetLooping());
    pVoice->SetPitch(SourceVariation::CalculatePitch(mPitch));
    pVoice->SetGain(SourceVariation::CalculateGain(mGain));
    pVoice->SourceBus::SetBus(mBus);
    pVoice->SetReverbGain(mReverbGain);
    pVoice->SetPan(mPan);
    pVoice->SetMinDistance(mMinDistance);
    pVoice->SetMuteDistance(mMuteDistance);
    pVoice->SetMaxDistance(mMaxDistance);
    pVoice->SetRollOff(mRollOffFactor);
    pVoice->SetRollIn(mRollInFactor);
    pVoice->SetRollInDistance(mRollInDistance);
    pVoice->Set3DMode(Get3DMode());
    pVoice->SetDopplerMode(GetDopplerMode());
    pVoice->SetPriority(mPriority);
    pVoice->SetBias(mBias);
    pVoice->SetSpaceDistance(mSpaceDistance);

    if (GetRandomPlayPosition())
    {
        pVoice->mPlaybackPosition =
            (unsigned int)(sndRandom.GetFloat() * (float)pSample->mLengthSamples);
        pVoice->SourceTransport::mFlags |= SourceTransport::flags_updatePlaybackPosition;
    }

    if (pSource3D)
    {
        pVoice->SetPosition(pSource3D->mPos3D);
        pVoice->SetVelocity(pSource3D->mVel3D);
        if (pSource3D->mClosestListener)
        {
            pVoice->AssignClosestListener(pSource3D->mClosestListener, pSource3D->mDistance,
                                          pSource3D->mDistanceLength);
        }
    }
}

// FUNCTION: SWBF2 0x0073ae10
void __cdecl Sound::UpdateAll(float fDeltaTime)
{
    for (PblListSingle::Node* pNode = smSoundList.mHead.mNext; pNode != &smSoundList.mHead;
         pNode = pNode->mNext)
    {
        VoiceVirtualBus* pSound = reinterpret_cast<Sound*>(reinterpret_cast<char*>(pNode) - offsetof(Sound, mNode));
        if (pSound->GetPlayState() == SourceTransport::playState_playing)
        {
            pSound->VoiceVirtualBus::Update();
        }
    }

    VoiceVirtualManager* p3DManager = EngineBase::GetVoiceVirtualManager(true);
    VoiceVirtualManager* p2DManager = EngineBase::GetVoiceVirtualManager(false);
    UpdateVoices(p3DManager, fDeltaTime);
    if (p2DManager != p3DManager)
    {
        UpdateVoices(p2DManager, fDeltaTime);
    }
}

// FUNCTION: SWBF2 0x0073aeb0
void __cdecl Sound::UpdateVoices(VoiceVirtualManager* pManager, float fDeltaTime)
{
    for (PblListDouble::Node* pLink = pManager->mActiveList.mTerminator.mNext;
         pLink != &pManager->mActiveList.mTerminator; pLink = pLink->mNext)
    {
        PblListDoubleSorted::Node* pNode =
            (PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode));
        if (pNode == pManager->mActiveList.GetTerminator())
        {
            break;
        }
        FireForget* pVoice = (FireForget*)((char*)pNode - offsetof(VoiceVirtual, mNode));
        if (pVoice->mCustomFlags != (unsigned int)smVoiceVirtuals)
        {
            continue;
        }

        pVoice->VoiceVirtualBus::Update();
        const ::PblVector3* pPos = pVoice->mPos3D;
        if (pPos < &smSource3DPositions[0] || pPos >= &smSource3DPositions[64])
        {
            const unsigned int i = (unsigned int)(pVoice - smVoiceVirtuals);
            smSource3DPositions[i] = *pPos;
            pVoice->SetPosition(&smSource3DPositions[i]);
        }
    }
}

// FUNCTION: SWBF2 0x0073af60
VoiceVirtual* __cdecl Sound::VoiceVirtualHandleToVoiceVirtual(unsigned short uHandle)
{
    return &smVoiceVirtuals[uHandle - 1];
}

// FUNCTION: SWBF2 0x0073afb0
unsigned short __cdecl Sound::VoiceVirtualToVoiceVirtualHandle(const VoiceVirtual* const pVoice)
{
    return (unsigned short)(static_cast<const FireForget*>(pVoice) - smVoiceVirtuals + 1);
}

// FUNCTION: SWBF2 0x0073a2f0
void Sound::Play(bool bRestart)
{
    if (!mProperties->DeterminePlaySource(false))
    {
        return;
    }

    SetSample((const Sample*)mProperties->mSelObjectWeightList->GetObjectA());
    if (mProperties->GetRandomPlayPosition())
    {
        mPlaybackPosition = (unsigned int)(sndRandom.GetFloat() * (float)mSample->mLengthSamples);
        SourceTransport::mFlags |= SourceTransport::flags_updatePlaybackPosition;
    }

    SetGain(mProperties->SourceVariation::CalculateGain(mProperties->mGain));
    SetPitch(mProperties->SourceVariation::CalculatePitch(mProperties->mPitch));
    SourceBus::Play(this);
    VoiceVirtual::Play(bRestart);
}

// GLOBAL: SWBF2 0x007e3700
Sound::ParameterID Sound::smParameterFunctions[11] = {
    Sound::ParameterID(&Sound::ParamSetGain, 0x1B5426FEu),
    Sound::ParameterID(&Sound::ParamSetPitch, 0xBD324AB1u),
    Sound::ParameterID(&Sound::ParamSetReverbGain, 0x6C408CFEu),
    Sound::ParameterID(&Sound::ParamSetPan, 0x4F55EEECu),
    Sound::ParameterID(&Sound::ParamSetMinDistance, 0xB835CF50u),
    Sound::ParameterID(&Sound::ParamSetMuteDistance, 0xE4B12EDFu),
    Sound::ParameterID(&Sound::ParamSetMaxDistance, 0xE6915CF6u),
    Sound::ParameterID(&Sound::ParamSetRollOff, 0xAB59D4BBu),
    Sound::ParameterID(&Sound::ParamSetTrigger, 0x75413203u),
    Sound::ParameterID(&Sound::ParamSetRollIn, 0x1CE8C2E1u),
    Sound::ParameterID(&Sound::ParamSetRollInDistance, 0x80A163FEu),
};

namespace
{
float ClampParameter(float value, float low, float high)
{
    if (value < low)
    {
        return low;
    }
    if (value > high)
    {
        return high;
    }
    return value;
}
float ClampNonnegative(float value)
{
    return value >= 0.0f ? value : 0.0f;
}
}

// FUNCTION: SWBF2 0x0073a070
void __cdecl Sound::ParamSetGain(Sound* pSound, float fValue)
{
    pSound->VoiceVirtualBus::SetGain(ClampParameter(pSound->mProperties->mGain * fValue, 0.0f, 1.0f));
}

// FUNCTION: SWBF2 0x0073a0c0
void __cdecl Sound::ParamSetMaxDistance(Sound* pSound, float fValue)
{
    static_cast<Source3D::Properties*>(static_cast<Source2D3D*>(pSound))->SetMaxDistance(
        ClampNonnegative(pSound->mProperties->mMaxDistance * fValue));
}

// FUNCTION: SWBF2 0x0073a0f0
void __cdecl Sound::ParamSetMinDistance(Sound* pSound, float fValue)
{
    static_cast<Source3D::Properties*>(static_cast<Source2D3D*>(pSound))->SetMinDistance(
        ClampNonnegative(pSound->mProperties->mMinDistance * fValue));
}

// FUNCTION: SWBF2 0x0073a120
void __cdecl Sound::ParamSetMuteDistance(Sound* pSound, float fValue)
{
    static_cast<Source3D::Properties*>(static_cast<Source2D3D*>(pSound))->SetMuteDistance(
        ClampNonnegative(pSound->mProperties->mMuteDistance * fValue));
}

// FUNCTION: SWBF2 0x0073a150
void __cdecl Sound::ParamSetPan(Sound* pSound, float fValue)
{
    static_cast<Source2D*>(static_cast<Source2D3D*>(pSound))->SetPan(
        ClampParameter(pSound->mProperties->mPan * fValue, -1.0f, 1.0f));
}

// FUNCTION: SWBF2 0x0073a190
void __cdecl Sound::ParamSetPitch(Sound* pSound, float fValue)
{
    static_cast<Source*>(static_cast<Source2D3D*>(pSound))->SetPitch(
        ClampNonnegative(pSound->mProperties->mPitch * fValue));
}

// FUNCTION: SWBF2 0x0073a1c0
void __cdecl Sound::ParamSetReverbGain(Sound* pSound, float fValue)
{
    static_cast<Source*>(static_cast<Source2D3D*>(pSound))->SetReverbGain(
        ClampParameter(pSound->mProperties->mReverbGain * fValue, 0.0f, 1.0f));
}

// FUNCTION: SWBF2 0x0073a200
void __cdecl Sound::ParamSetRollIn(Sound* pSound, float fValue)
{
    static_cast<Source3D::Properties*>(static_cast<Source2D3D*>(pSound))->SetRollIn(
        ClampNonnegative(pSound->mProperties->mRollInFactor * fValue));
}

// FUNCTION: SWBF2 0x0073a230
void __cdecl Sound::ParamSetRollInDistance(Sound* pSound, float fValue)
{
    static_cast<Source3D::Properties*>(static_cast<Source2D3D*>(pSound))->SetRollInDistance(
        ClampNonnegative(pSound->mProperties->mRollInDistance * fValue));
}

// FUNCTION: SWBF2 0x0073a260
void __cdecl Sound::ParamSetRollOff(Sound* pSound, float fValue)
{
    static_cast<Source3D::Properties*>(static_cast<Source2D3D*>(pSound))->SetRollOff(
        ClampNonnegative(pSound->mProperties->mRollOffFactor * fValue));
}

// FUNCTION: SWBF2 0x0073a290
void __cdecl Sound::ParamSetTrigger(Sound* pSound, float fValue)
{
    SourceTransport* pTransport = static_cast<SourceTransport*>(pSound);
    if (pTransport->GetPlayState() == SourceTransport::playState_stopped && fValue > 0.5f)
    {
        pSound->Play(true);
    }
    else if (pTransport->GetPlayState() == SourceTransport::playState_playing)
    {
        pSound->SourceBus::Stop(pTransport);
        pSound->VoiceVirtual::Stop();
    }
}

// FUNCTION: SWBF2 0x00739dc0
unsigned int __cdecl Sound::FindParameterIndexByHashID(unsigned int uiHashID)
{
    for (unsigned int i = 0; i < 11; ++i)
    {
        if (smParameterFunctions[i].GetHashID() == uiHashID)
        {
            return i;
        }
    }
    return ~0u;
}

// FUNCTION: SWBF2 0x00537ef0
Source3D::~Source3D()
{
}

}

namespace Snd {

// FUNCTION: SWBF2 0x00739d20
void __cdecl Sound::Properties::DestroyAll()
{
    while (!smPropertyList.IsEmpty())
    {
        delete (Properties*)((char*)smPropertyList.mHead.mNext - offsetof(Properties, mNode));
    }
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00739d90
const Sound::Properties* __cdecl Sound::Properties::FindByHashID(unsigned int uiHashID)
{
    for (PblListSingle::Node* pNode = smPropertyList.mHead.mNext;
         pNode != &smPropertyList.mHead; pNode = pNode->mNext)
    {
        const Properties* const pProps =
            (const Properties*)((char*)pNode - offsetof(Properties, mNode));
        if (pProps->mHashID == uiHashID)
        {
            return pProps;
        }
    }
    return 0;
}
}

namespace Snd
{

Sound::Properties::~Properties()
{
    if (mNode.mNext)
    {
        mNode.Remove();
    }
}
}
