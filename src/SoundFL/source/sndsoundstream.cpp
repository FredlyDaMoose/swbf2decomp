#include "sndsoundstream.h"

#include <stddef.h>
#include <stdlib.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblVector.h"
#include "sndenginebase.h"
#include "sndvoicevirtualbus.h"
#include "win/sndstream.h"

namespace Snd
{

// FUNCTION: SWBF2 0x007367a0
SoundStream::FireForget::~FireForget()
{
}

unsigned int SoundStream::smCount[6];
SoundStream::Properties* SoundStream::smPlayingProps[6];

// SYNTHETIC: SWBF2 0x007367f0
// PblListDoubleSorted::`default constructor closure'

// GLOBAL: SWBF2 0x01e29818
// SYNTHETIC: SWBF2 0x0040b9b0
// `dynamic initializer for 'Snd::SoundStream::smQueue''
// SYNTHETIC: SWBF2 0x0076a340
// `dynamic atexit destructor for 'Snd::SoundStream::smQueue''
PblListDoubleSorted SoundStream::smQueue[6];

// FUNCTION: SWBF2 0x007366b0
SoundStream::FireForget::FireForget()
    : mProperties(0),
      mPitch(1.0f),
      mGain(0.0f),
      mSegmentID(0)
{
}

// GLOBAL: SWBF2 0x01e29860
// SYNTHETIC: SWBF2 0x0040b8d0
// `dynamic initializer for 'Snd::SoundStream::smFireForget''
// SYNTHETIC: SWBF2 0x0076a2c0
// `dynamic atexit destructor for 'Snd::SoundStream::smFireForget''
SoundStream::FireForget SoundStream::smFireForget[24];

// GLOBAL: SWBF2 0x01e297d0
// SYNTHETIC: SWBF2 0x0040b960
// `dynamic initializer for 'Snd::SoundStream::smPlayingVel''

PblVector3 SoundStream::smPlayingVel[6];
// GLOBAL: SWBF2 0x01e29d40
// SYNTHETIC: SWBF2 0x0040b980
// `dynamic initializer for 'Snd::SoundStream::smPlayingPos''

PblVector3 SoundStream::smPlayingPos[6];

// GLOBAL: SWBF2 0x007e35a8
// SYNTHETIC: SWBF2 0x0040b900
// `dynamic initializer for 'Snd::SoundStream::smFree''
// SYNTHETIC: SWBF2 0x0076a2e0
// `dynamic atexit destructor for 'Snd::SoundStream::smFree''
PblListDoubleSorted SoundStream::smFree;

// FUNCTION: SWBF2 0x00736b40
void SoundStream::Init()
{
    StopAll();

    for (unsigned int i = 0; i < 6; ++i)
    {
        smQueue[i].SetSortOnAdd(true);
        smPlayingProps[i] = 0;
    }

    for (unsigned int i = 0; i < 24; ++i)
    {
        PblListDoubleSorted::Node* pNode = &smFireForget[i].mNode;

        if (pNode->mNode.IsAttached())
        {
            pNode->mNode.Remove();
        }

        smFree.Add(pNode);
    }
}

// FUNCTION: SWBF2 0x00737710
void SoundStream::StopAll()
{
    for (unsigned int i = 0; i < 6; ++i)
    {
        for (;;)
        {
            PblListDoubleSorted::Node* pLink = smQueue[i].GetFirst();
            if (pLink == smQueue[i].GetTerminator())
            {
                break;
            }

            FireForget* pFF = (FireForget*)((char*)pLink
                                            - (int)offsetof(FireForget, mNode));

            --smCount[i];

            pFF->mNode.mNode.Remove();

            smFree.Add(&pFF->mNode);

            if (!pFF)
            {
                break;
            }
        }
    }
}

// GLOBAL: SWBF2 0x007e3568
// SYNTHETIC: SWBF2 0x0040b910
// `dynamic initializer for 'Snd::SoundStream::Properties::smFreeList''
// SYNTHETIC: SWBF2 0x0076a300
// `dynamic atexit destructor for 'Snd::SoundStream::Properties::smFreeList''
FreeList SoundStream::Properties::smFreeList(sizeof(SoundStream::Properties), 1, 8, 1);

// GLOBAL: SWBF2 0x007e3584
// SYNTHETIC: SWBF2 0x0040b9a0
// `dynamic initializer for 'Snd::SoundStream::Properties::smPropertyList''
// SYNTHETIC: SWBF2 0x0076a330
// `dynamic atexit destructor for 'Snd::SoundStream::Properties::smPropertyList''
PblListSingle SoundStream::Properties::smPropertyList;

// GLOBAL: SWBF2 0x007e3588
// SYNTHETIC: SWBF2 0x0040b930
// `dynamic initializer for 'Snd::SoundStream::smFreeList''
// SYNTHETIC: SWBF2 0x0076a310
// `dynamic atexit destructor for 'Snd::SoundStream::smFreeList''
FreeList SoundStream::smFreeList(0xcc, 1, 4, 1);

// GLOBAL: SWBF2 0x007e35a4
// SYNTHETIC: SWBF2 0x0040b950
// `dynamic initializer for 'Snd::SoundStream::smList''
// SYNTHETIC: SWBF2 0x0076a320
// `dynamic atexit destructor for 'Snd::SoundStream::smList''
PblListSingle SoundStream::smList;

// FUNCTION: SWBF2 0x007366f0
SoundStream::Properties::Properties()
    : mStreamID(0)
{
    mSelObjectWeightList = &mObjectWeightList;
    smPropertyList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x007367d0
void __cdecl SoundStream::Properties::operator delete(void* p)
{
    smFreeList.Free(p);
}

// FUNCTION: SWBF2 0x00736c70
void __cdecl SoundStream::Properties::Load(PblFileChunk& chunk, bool bOnce)
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

// FUNCTION: SWBF2 0x00737180
void SoundStream::Properties::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uiID = child._pFile->Read32();
        switch (uiID)
        {
        case 0x0A8325F2u:
            child = chunk.ReadNextChild();
            mSelObjectWeightList = &mObjectWeightList;
            mObjectWeightList.ReadObjectWeights(child, 0xF3C342E6u, 0);
            break;
        case 0x5F6F6D65u:
            child._pFile->Read8();
            mStreamID = child._pFile->Read32();
            break;
        case 0xCA04EFE0u:
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
                mStreamID = pParent->mStreamID;
                break;
            }
            break;
        }
        default:
            if (!SoundProperties::Read(child, uiID)
                && !Source2D3D::Read(child, uiID)
                && !SourceVariation::Read(child, uiID)
                && !SourceVirtual::Read(child, uiID))
            {
                mObjectWeightList.Read(child, uiID);
            }
            break;
        }
    }
}

extern PblRandom sndRandom;

// FUNCTION: SWBF2 0x00736ac0
SoundStream::FireForget* __cdecl SoundStream::GetFireForget(unsigned int uiIndex)
{
    PblListDoubleSorted::Node* pLink = smQueue[uiIndex].GetFirst();
    if (pLink == smQueue[uiIndex].GetTerminator())
    {
        return 0;
    }
    FireForget* pFF = (FireForget*)((char*)pLink - (int)offsetof(FireForget, mNode));
    --smCount[uiIndex];
    pFF->mNode.mNode.Remove();
    smFree.Add(&pFF->mNode);
    return pFF;
}

// FUNCTION: SWBF2 0x00737560
bool SoundStream::Properties::SetupStream(Stream* pStream, unsigned int uiSegmentID) const
{
    if (!pStream->IsOpen())
    {
        return false;
    }
    if (!uiSegmentID)
    {
        uiSegmentID = (unsigned int)mSelObjectWeightList->GetObjectA();
    }

    const StreamBase::Segment* pSegment = pStream->FindSegment(uiSegmentID);
    if (!pSegment)
    {
        return false;
    }

    pStream->SetLooping(GetLooping());
    pStream->SourceBus::SetBus(mBus);
    pStream->SetReverbGain(mReverbGain);
    pStream->SetPan(mPan);
    pStream->SetRollIn(mRollInFactor);
    pStream->SetRollInDistance(mRollInDistance);
    pStream->SetMinDistance(mMinDistance);
    pStream->SetMuteDistance(mMuteDistance);
    pStream->SetMaxDistance(mMaxDistance);
    pStream->SetRollOff(mRollOffFactor);
    pStream->Set3DMode(Get3DMode());
    pStream->SetDopplerMode(GetDopplerMode());
    pStream->SeekSegment(pSegment);

    if (GetRandomPlayPosition())
    {
        pStream->mPlaybackPosition =
            (unsigned int)(sndRandom.GetFloat() * (float)pSegment->mLengthSamples);
        pStream->SourceTransport::mFlags |= SourceTransport::flags_updatePlaybackPosition;
    }
    return true;
}

// FUNCTION: SWBF2 0x00737800
void __cdecl SoundStream::UpdateAll()
{
    for (unsigned int i = 0; i < 6; ++i)
    {
        Stream* pStream = &EngineBase::smStreams[i];
        if (pStream->GetPlayState() != SourceTransport::playState_stopped)
        {
            continue;
        }
        FireForget* pFF = GetFireForget(i);
        if (!pFF)
        {
            continue;
        }
        Properties* pProps = pFF->mProperties;
        if (!pProps->SetupStream(pStream, pFF->mSegmentID))
        {
            continue;
        }

        smPlayingProps[i] = pProps;
        smPlayingVel[i] = *(const PblVector3*)pFF->mPos;
        smPlayingPos[i] = *(const PblVector3*)pFF->mVel;
        pStream->SetPitch(pFF->mPitch);
        pStream->SetGain(pFF->mGain);
        pStream->SetPosition(&smPlayingVel[i]);
        pStream->SetVelocity(&smPlayingPos[i]);
        pStream->Play(true);
    }

    for (PblListSingle::Node* pNode = smList.mHead.mNext; pNode != &smList.mHead;
         pNode = pNode->mNext)
    {
        reinterpret_cast<SoundStream*>(reinterpret_cast<char*>(pNode) - offsetof(SoundStream, mNode))->VoiceVirtualBus::Update();
    }
}

// FUNCTION: SWBF2 0x00736970
float SoundStream::Properties::CalculatePriority(const PblVector3* const pPos,
                                                 const PblVector3* const pVel) const
{
    Source3D source3D;

    SourceTransport transport;
    transport.mPlaybackPosition = 0;
    transport.mFlags = SourceTransport::flags_stopped;
    transport.Play(true);

    const Source3D* pSource3D = 0;
    if (Get3DMode())
    {
        source3D.SetProperties(const_cast<Properties*>(this));
        source3D.SetPosition(pPos);
        source3D.SetVelocity(pVel);
        source3D.mFlags |= Source3D::flags_updateListener;
        source3D.DetermineClosestListener();
        pSource3D = &source3D;
    }
    return SourceVirtual::CalculatePriority(&transport, this, pSource3D, 0, 0,
                                            mBus->mFinalGain, false);
}

// FUNCTION: SWBF2 0x00736800
SoundStream::FireForget* __cdecl SoundStream::AllocFireForget(unsigned int uiIndex, float fPriority,
                                                              bool bNoSteal)
{
    PblListDoubleSorted& rQueue = smQueue[uiIndex];

    PblListDoubleSorted::Node* pFreeNode = smFree.GetFirst();
    FireForget* const pFree = (pFreeNode && pFreeNode != smFree.GetTerminator())
        ? (FireForget*)((char*)pFreeNode - (int)offsetof(FireForget, mNode))
        : 0;

    for (PblListDouble::Node* pLink = rQueue.mTerminator.mNext; pLink != &rQueue.mTerminator;)
    {
        PblListDoubleSorted::Node* const pNode =
            (PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode));
        if (pNode == rQueue.GetTerminator())
        {
            break;
        }
        FireForget* const pFF = (FireForget*)((char*)pNode - (int)offsetof(FireForget, mNode));
        pNode->mSortValue = pFF->mProperties->CalculatePriority((const PblVector3*)pFF->mPos,
                                                                (const PblVector3*)pFF->mVel);
        pLink = pLink->mNext;
        if (pNode->mSortValue <= 0.0f)
        {
            --smCount[uiIndex];
            pNode->mNode.Remove();
            smFree.Add(pNode);
        }
    }

    pFree->mNode.mNode.Remove();
    pFree->mNode.mSortValue = fPriority;
    rQueue.Add(&pFree->mNode);
    if (++smCount[uiIndex] <= 4)
    {
        return pFree;
    }

    FireForget* pResult = pFree;
    PblListDouble::Node* const pLastLink = rQueue.mTerminator.mPrev;
    const PblListDoubleSorted::Node* pVictim = (pLastLink == &rQueue.mTerminator)
        ? rQueue.GetTerminator()
        : (const PblListDoubleSorted::Node*)((char*)pLastLink
                                             - offsetof(PblListDoubleSorted::Node, mNode));
    if (pVictim == &pFree->mNode)
    {
        if (bNoSteal)
        {
            pResult = 0;
        }
        else
        {
            PblListDouble::Node* const pPrevLink = pVictim->mNode.mPrev;
            pVictim = (pPrevLink == &rQueue.mTerminator)
                ? 0
                : (const PblListDoubleSorted::Node*)((char*)pPrevLink
                                                     - offsetof(PblListDoubleSorted::Node, mNode));
        }
    }

    FireForget* const pDrop = (pVictim == rQueue.GetTerminator())
        ? 0
        : (FireForget*)((char*)pVictim - (int)offsetof(FireForget, mNode));
    --smCount[uiIndex];
    pDrop->mNode.mNode.Remove();
    smFree.Add(&pDrop->mNode);
    return pResult;
}

// FUNCTION: SWBF2 0x00736ed0
void __cdecl SoundStream::Play(const Properties* pProps, Source3D* pSource3D, bool bQueue)
{
    Source3D source3D;

    Stream* const pStream = EngineBase::GetOpenStream(pProps->mStreamID);
    if (!pStream || !pProps->mBus || pProps->mBus->mFinalGain <= 0.0f)
    {
        return;
    }

    const unsigned int uiSegmentID = (unsigned int)pProps->mSelObjectWeightList->GetObjectA();
    if (!uiSegmentID)
    {
        return;
    }

    const float fGain = pProps->SourceVariation::CalculateGain(pProps->mGain);
    if (fGain <= 0.0f)
    {
        return;
    }
    if (!pProps->DeterminePlaySource(false))
    {
        return;
    }

    if (pSource3D)
    {
        source3D.CopyParameters(*pSource3D);
        source3D.SetProperties(const_cast<Properties*>(pProps));
    }

    const float fPriority = pProps->CalculatePriority(source3D.mPos3D, source3D.mVel3D);
    if (fPriority <= 0.0f)
    {
        return;
    }

    const unsigned int uiIndex = (unsigned int)(pStream - EngineBase::smStreams);

    if (bQueue)
    {
        FireForget* const pFF = AllocFireForget(uiIndex, fPriority, pProps->Get3DMode());
        if (!pFF)
        {
            return;
        }
        pFF->mProperties = const_cast<Properties*>(pProps);
        *(PblVector3*)pFF->mPos = *source3D.mPos3D;
        *(PblVector3*)pFF->mVel = *source3D.mVel3D;
        pFF->mSegmentID = uiSegmentID;
        pFF->mGain = fGain;
        pFF->mPitch = pProps->SourceVariation::CalculatePitch(pProps->mPitch);
        return;
    }

    Stream* const pPlaying = &EngineBase::smStreams[uiIndex];
    if (pPlaying->GetPlayState() != SourceTransport::playState_stopped && smPlayingProps[uiIndex])
    {
        if (fPriority < smPlayingProps[uiIndex]->CalculatePriority(&smPlayingVel[uiIndex],
                                                                   &smPlayingPos[uiIndex]))
        {
            return;
        }
    }

    if (!pProps->SetupStream(pPlaying, uiSegmentID))
    {
        return;
    }

    smPlayingProps[uiIndex] = const_cast<Properties*>(pProps);
    smPlayingVel[uiIndex] = *source3D.mPos3D;
    smPlayingPos[uiIndex] = *source3D.mVel3D;
    pPlaying->SetPitch(pProps->SourceVariation::CalculatePitch(pProps->mPitch));
    pPlaying->SetGain(fGain);
    pPlaying->SetPosition(&smPlayingVel[uiIndex]);
    pPlaying->SetVelocity(&smPlayingPos[uiIndex]);
    pPlaying->Play(true);
}

// FUNCTION: SWBF2 0x00737160
void __cdecl SoundStream::Play(const Properties* pProps, bool bQueue)
{
    Play(pProps, 0, bQueue);
}

// FUNCTION: SWBF2 0x00737420
void SoundStream::Properties::SetupSoundStream(SoundStream* pStream) const
{
    pStream->mInfo = StreamBase::Info::Find(mStreamID);
    if (!pStream->mInfo)
    {
        return;
    }

    pStream->SetVoiceVirtualManager(pStream->mInfo->GetStreamManager());
    pStream->SetLooping(GetLooping());
    pStream->SourceBus::SetBus(mBus);
    pStream->SetReverbGain(mReverbGain);
    pStream->SetPan(mPan);
    pStream->SetRollIn(mRollInFactor);
    pStream->SetRollInDistance(mRollInDistance);
    pStream->SetMinDistance(mMinDistance);
    pStream->SetMuteDistance(mMuteDistance);
    pStream->SetMaxDistance(mMaxDistance);
    pStream->SetRollOff(mRollOffFactor);
    pStream->Set3DMode(Get3DMode());
    pStream->SetDopplerMode(GetDopplerMode());
    pStream->SetBias(mBias);
    pStream->SetPriority(mPriority);
    pStream->SetSpaceDistance(mSpaceDistance);
}

// FUNCTION: SWBF2 0x00736d50
void SoundStream::Play(bool bRestart)
{
    if (!mProperties->DeterminePlaySource(false))
    {
        return;
    }

    unsigned int uiSegmentID = (unsigned int)mProperties->mSelObjectWeightList->GetObjectA();
    const StreamBase::Info* pInfo = StreamBase::Info::Find(mProperties->mStreamID);
    if (!pInfo)
    {
        return;
    }
    const StreamBase::Segment* pSegment = (const StreamBase::Segment*)bsearch(
        &uiSegmentID, pInfo->mSegments, pInfo->mNumSegments, sizeof(StreamBase::Segment),
        StreamBase::Segment::CompareSearch);
    if (!pSegment)
    {
        return;
    }

    SetPitch(mProperties->SourceVariation::CalculatePitch(mProperties->mPitch));
    SetGain(mProperties->SourceVariation::CalculateGain(mProperties->mGain));
    SetSample((const Sample*)pSegment);
    if (mProperties->GetRandomPlayPosition())
    {
        mPlaybackPosition = (unsigned int)(sndRandom.GetFloat() * (float)pSegment->mLength);
        SourceTransport::mFlags |= SourceTransport::flags_updatePlaybackPosition;
    }

    SourceBus::Play(this);
    VoiceVirtual::Play(bRestart);
}

}

namespace Snd {

// FUNCTION: SWBF2 0x00736a20
void __cdecl SoundStream::Properties::DestroyAll()
{
    StopAll();
    while (!smPropertyList.IsEmpty())
    {
        delete (Properties*)((char*)smPropertyList.mHead.mNext - offsetof(Properties, mNode));
    }
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00736a90
const SoundStream::Properties* __cdecl SoundStream::Properties::FindByHashID(unsigned int uiHashID)
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

// LIBRARY: SWBF2 0x00752af8
// bsearch

namespace Snd
{

SoundStream::Properties::~Properties()
{
    if (mNode.mNext)
    {
        mNode.Remove();
    }
}
}
