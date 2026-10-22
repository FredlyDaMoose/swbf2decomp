#include "sndsoundlayered.h"

#include <float.h>

#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

namespace Snd
{

template <>
void Allocator<SoundLayered::FireForgetLayer>::operator delete(void* p, unsigned int uiSize)
{
    (void)uiSize;
    switch (((SoundLayered::FireForgetLayer*)p)->GetAllocType())
    {
    case allocType_heap:
        Free(p);
        break;
    case allocType_tempHeap:
        FreeTemp(p);
        break;
    default:
        break;
    }
}
}

namespace Snd
{

SoundLayered::FireForgetLayer* SoundLayered::smFireForgetLayers;
bool SoundLayered::smIsOpen;
unsigned char SoundLayered::smFireForgetLayerStorage[5888];
// GLOBAL: SWBF2 0x007e36bc
// SYNTHETIC: SWBF2 0x0040bbb0
// `dynamic initializer for 'Snd::SoundLayered::FireForgetLayer::smInactiveList''
// SYNTHETIC: SWBF2 0x0076a3c0
// `dynamic atexit destructor for 'Snd::SoundLayered::FireForgetLayer::smInactiveList''
PblListDouble SoundLayered::FireForgetLayer::smInactiveList;

// GLOBAL: SWBF2 0x007e36b4
// SYNTHETIC: SWBF2 0x0040bb40
// `dynamic initializer for 'Snd::SoundLayered::FireForgetLayer::smActiveList''
// SYNTHETIC: SWBF2 0x0076a370
// `dynamic atexit destructor for 'Snd::SoundLayered::FireForgetLayer::smActiveList''
PblListDouble SoundLayered::FireForgetLayer::smActiveList;

// GLOBAL: SWBF2 0x007e3678
// SYNTHETIC: SWBF2 0x0040bb50
// `dynamic initializer for 'Snd::SoundLayered::Properties::Layer::smFreeList''
// SYNTHETIC: SWBF2 0x0076a390
// `dynamic atexit destructor for 'Snd::SoundLayered::Properties::Layer::smFreeList''
FreeList SoundLayered::Properties::Layer::smFreeList(sizeof(SoundLayered::Properties::Layer), 1, 8, 1);

// GLOBAL: SWBF2 0x007e3694
// SYNTHETIC: SWBF2 0x0040bb70
// `dynamic initializer for 'Snd::SoundLayered::Properties::smFreeList''
// SYNTHETIC: SWBF2 0x0076a3a0
// `dynamic atexit destructor for 'Snd::SoundLayered::Properties::smFreeList''
FreeList SoundLayered::Properties::smFreeList(0x0c, 1, 8, 1);

// GLOBAL: SWBF2 0x007e365c
// SYNTHETIC: SWBF2 0x0040bb90
// `dynamic initializer for 'Snd::SoundLayered::SoundLayer::smFreeList''
// SYNTHETIC: SWBF2 0x0076a3b0
// `dynamic atexit destructor for 'Snd::SoundLayered::SoundLayer::smFreeList''
FreeList SoundLayered::SoundLayer::smFreeList(sizeof(SoundLayered::SoundLayer), 1, 0x40, 0);

// GLOBAL: SWBF2 0x007e36b0
// SYNTHETIC: SWBF2 0x0040bbc0
// `dynamic initializer for 'Snd::SoundLayered::Properties::smPropertyList''
// SYNTHETIC: SWBF2 0x0076a3e0
// `dynamic atexit destructor for 'Snd::SoundLayered::Properties::smPropertyList''
PblListSingle SoundLayered::Properties::smPropertyList;

// GLOBAL: SWBF2 0x007e36c4
// SYNTHETIC: SWBF2 0x0040bbd0
// `dynamic initializer for 'Snd::SoundLayered::smSoundLayeredList''
// SYNTHETIC: SWBF2 0x0076a3f0
// `dynamic atexit destructor for 'Snd::SoundLayered::smSoundLayeredList''
PblListDouble SoundLayered::smSoundLayeredList;

// FUNCTION: SWBF2 0x00738cf0
template <>
void Allocator<SoundLayered::FireForgetLayer>::operator delete(void* p,
                                                               void* pMemory,
                                                               bool bTemp)
{
    (void)pMemory;
    (void)bTemp;

    if (!p)
    {
        return;
    }

    switch (((SoundLayered::FireForgetLayer*)p)->GetAllocType())
    {
    case allocType_heap:
        Free(p);
        break;
    case allocType_tempHeap:
        FreeTemp(p);
        break;
    default:

        break;
    }
}

// FUNCTION: SWBF2 0x00738f40
void SoundLayered::Open()
{
    FireForgetLayer* pStorage = (FireForgetLayer*)smFireForgetLayerStorage;

    for (unsigned int i = 0; i < 64; ++i)
    {
        new (&pStorage[i], false) FireForgetLayer();
    }

    smFireForgetLayers = (FireForgetLayer*)smFireForgetLayerStorage;
    smIsOpen = true;
}

void SoundLayered::FireForgetLayer::Update(float fDeltaTime)
{
    if (mTimeElapsed >= mTriggerTime)
    {
        const Sound::Properties* pProps = ((const Properties::Layer*)mLayer)->mSoundProps;
        Sound::Play(pProps->Get3DMode() ? &mSource3D : 0, pProps, 0, 0, true);
        mNode.Remove();
        smInactiveList.AddLast(&mNode);
    }
    mTimeElapsed += fDeltaTime;
}

// FUNCTION: SWBF2 0x00739a70
void SoundLayered::FireForgetLayer::UpdateAll(float fDeltaTime)
{
    PblListDouble::Node* pLink = smActiveList.mTerminator.mNext;
    while (pLink != &smActiveList.mTerminator)
    {
        FireForgetLayer* pLayer = (FireForgetLayer*)((char*)pLink - offsetof(FireForgetLayer, mNode));
        pLink = pLink->mNext;
        pLayer->Update(fDeltaTime);
    }
}

// FUNCTION: SWBF2 0x00739b20
void SoundLayered::UpdateAll(float fDeltaTime)
{
    PblListDouble::Node* pLink = smSoundLayeredList.mTerminator.mNext;
    while (pLink != &smSoundLayeredList.mTerminator)
    {
        ((SoundLayered*)((char*)pLink - offsetof(SoundLayered, mNode)))->Update(fDeltaTime);
        pLink = pLink->mNext;
    }
    FireForgetLayer::UpdateAll(fDeltaTime);
}

void SoundLayered::SoundLayerDestroy(SoundLayer* pLayer)
{
    delete pLayer;
}

void SoundLayered::FreeLayers(void (__cdecl* pLayerDestroy)(SoundLayer*))
{
    if (mProperties)
    {
        mProperties = 0;
    }
    while (!mSoundList.IsEmpty())
    {
        PblListSingle::Node* pFirst = mSoundList.mHead.mNext;
        pLayerDestroy(pFirst == &mSoundList.mHead
                          ? 0
                          : (SoundLayer*)((char*)pFirst - offsetof(SoundLayer, mLayerNode)));
    }
}

// FUNCTION: SWBF2 0x007394b0
void SoundLayered::RemoveAll()
{
    PblListDouble::Node* pLink = smSoundLayeredList.mTerminator.mNext;
    while (pLink != &smSoundLayeredList.mTerminator)
    {
        SoundLayered* pSound = (SoundLayered*)((char*)pLink - offsetof(SoundLayered, mNode));
        pLink = pLink->mNext;
        pSound->Stop();
        pSound->mNode.Remove();
        pSound->FreeLayers(SoundLayerDestroy);
    }
}

// FUNCTION: SWBF2 0x00739780
void SoundLayered::FireForgetLayer::StopAll()
{
    PblListDouble::Node* pLink = smActiveList.mTerminator.mNext;
    while (pLink != &smActiveList.mTerminator)
    {
        FireForgetLayer* pLayer = (FireForgetLayer*)((char*)pLink - offsetof(FireForgetLayer, mNode));
        pLink = pLink->mNext;
        pLayer->Stop();
    }
}

// FUNCTION: SWBF2 0x00738da0
SoundLayered::Properties* __cdecl SoundLayered::Properties::FindByHashID(unsigned int uiHashID)
{
    for (PblListSingle::Node* pNode = smPropertyList.mHead.mNext; pNode != &smPropertyList.mHead;
         pNode = pNode->mNext)
    {
        Properties* pProps = (Properties*)((char*)pNode - offsetof(Properties, mNode));
        if (pProps->mHashID == uiHashID)
        {
            return pProps;
        }
    }
    return 0;
}

extern PblRandom sndRandom;

// FUNCTION: SWBF2 0x00738b10
SoundLayered::SoundLayered()
    : mTimeElapsed(0.0f),
      mStopTime(FLT_MAX),
      mProperties(0)
{

    mPlaybackPosition = 0;
    SourceTransport::mFlags = (unsigned short)flags_stopped;
    smSoundLayeredList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x00738c20
SoundLayered::~SoundLayered()
{
    FreeLayers(SoundLayerDestroy);
    if (mNode.IsAttached())
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x00738e00
Sound* SoundLayered::FindLayerByHashID(unsigned int uiHashID)
{
    PblListSingle::Node* pSoundNode = mSoundList.mHead.mNext;
    for (const PblListSingle::Node* pNode = mProperties->mLayerList.mHead.mNext;
         pNode != &mProperties->mLayerList.mHead;
         pNode = pNode->mNext, pSoundNode = pSoundNode->mNext)
    {
        const Properties::Layer* pLayer =
            (const Properties::Layer*)((const char*)pNode - offsetof(Properties::Layer, mNode));
        if (pLayer->mHashID == uiHashID)
        {
            if (pSoundNode == &mSoundList.mHead)
            {
                return 0;
            }
            return (SoundLayer*)((char*)pSoundNode - offsetof(SoundLayer, mLayerNode));
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x007397e0
void SoundLayered::Update(float fDeltaTime)
{
    PlayState ePlayState = GetCurrentPlayState();
    const PlayState eDesired = QueryDesiredPlayState();
    SourceTransport::mFlags = (unsigned short)
        (SourceTransport::mFlags & ~(unsigned int)flags_updateTransport);

    if (mProperties && eDesired != playState_invalid && ePlayState != eDesired)
    {
        PblListSingle::Node* pSoundNode = mSoundList.mHead.mNext;
        switch (eDesired)
        {
        case playState_playing:
            mTimeElapsed = 0.0f;
            mStopTime = 0.0f;
            for (const PblListSingle::Node* pNode = mProperties->mLayerList.mHead.mNext;
                 pNode != &mProperties->mLayerList.mHead;
                 pNode = pNode->mNext, pSoundNode = pSoundNode->mNext)
            {
                const Properties::Layer* pLayer = (const Properties::Layer*)
                    ((const char*)pNode - offsetof(Properties::Layer, mNode));
                SoundLayer* pSound = pSoundNode == &mSoundList.mHead
                    ? 0
                    : (SoundLayer*)((char*)pSoundNode - offsetof(SoundLayer, mLayerNode));
                pSound->mPlayTime =
                    (sndRandom.GetFloat() - 0.5f) * pLayer->mDelayVar * 2.0f + pLayer->mDelay;
                pSound->mStopTime =
                    (sndRandom.GetFloat() - 0.5f) * pLayer->mPlayTimeVar * 2.0f
                    + pLayer->mPlayTime + pSound->mPlayTime;
                if (pSound->mStopTime > mStopTime)
                {
                    mStopTime = pSound->mStopTime;
                }
            }
            break;

        case playState_paused:
            for (; pSoundNode != &mSoundList.mHead; pSoundNode = pSoundNode->mNext)
            {
                ((SoundLayer*)((char*)pSoundNode - offsetof(SoundLayer, mLayerNode)))->Pause();
            }
            break;

        case playState_stopped:
            for (; pSoundNode != &mSoundList.mHead; pSoundNode = pSoundNode->mNext)
            {
                SoundLayer* pSound =
                    (SoundLayer*)((char*)pSoundNode - offsetof(SoundLayer, mLayerNode));
                pSound->SourceBus::Stop(pSound);
                pSound->VoiceVirtual::Stop();
            }
            break;
        }
        ePlayState = eDesired;
        SetPlayState(eDesired);
    }

    if (ePlayState == playState_playing && mProperties)
    {
        for (PblListSingle::Node* pSoundNode = mSoundList.mHead.mNext;
             pSoundNode != &mSoundList.mHead; pSoundNode = pSoundNode->mNext)
        {
            SoundLayer* pSound =
                (SoundLayer*)((char*)pSoundNode - offsetof(SoundLayer, mLayerNode));
            if (pSound->GetPlayState() == playState_playing)
            {
                if (mTimeElapsed >= pSound->mStopTime)
                {
                    pSound->SourceBus::Stop(pSound);
                    pSound->VoiceVirtual::Stop();
                }
            }
            else if (mTimeElapsed >= pSound->mPlayTime)
            {
                pSound->Play(true);
            }
        }
        if (mTimeElapsed >= mStopTime)
        {
            Stop();
        }
        mTimeElapsed += fDeltaTime;
    }
}

inline float __cdecl SoundLayered::FireForgetLayer::CalculatePriority(Source3D* pSource3D,
                                                                      const Sound::Properties* pProps)
{
    SourceTransport transport;
    transport.mPlaybackPosition = 0;
    transport.mFlags = SourceTransport::flags_stopped;
    transport.Play(true);
    const Sample* pSample = (const Sample*)pProps->mSelObjectWeightList->GetObjectA();
    return pProps->SourceVirtual::CalculatePriority(&transport, pProps, pSource3D, 0, pSample,
                                                    1.0f, false);
}

inline SoundLayered::FireForgetLayer* __cdecl SoundLayered::FireForgetLayer::StealLayer(float fPriority)
{
    FireForgetLayer* pSteal = 0;
    float fLowest = FLT_MAX;
    for (PblListDouble::Node* pLink = smActiveList.mTerminator.mNext; pLink != &smActiveList.mTerminator;
         pLink = pLink->mNext)
    {
        FireForgetLayer* pLayer = pLink == &smInactiveList.mTerminator
            ? 0
            : (FireForgetLayer*)((char*)pLink - offsetof(FireForgetLayer, mNode));
        if (fPriority > pLayer->mPriority && fLowest > pLayer->mPriority)
        {
            fLowest = pLayer->mPriority;
            pSteal = pLayer;
        }
    }
    return pSteal;
}

// FUNCTION: SWBF2 0x00738e50
SoundLayered::FireForgetLayer* __cdecl SoundLayered::FireForgetLayer::GetFreeLayer()
{
    PblListDouble::Node* pFirst = smInactiveList.mTerminator.mNext;
    if (pFirst == &smInactiveList.mTerminator)
    {
        return 0;
    }
    return (FireForgetLayer*)((char*)pFirst - offsetof(FireForgetLayer, mNode));
}

// FUNCTION: SWBF2 0x00739030
void SoundLayered::FireForgetLayer::Play(Source3D* pSource3D, const void* pLayer, float fPriority)
{
    const Properties::Layer* pPropsLayer = (const Properties::Layer*)pLayer;
    if (pSource3D)
    {
        mSource3D = *pSource3D;
        *(::PblVector3*)mPos = *pSource3D->mPos3D;
        *(::PblVector3*)mVel = *pSource3D->mVel3D;
        mSource3D.SetPosition((::PblVector3*)mPos);
        mSource3D.SetVelocity((::PblVector3*)mVel);
    }
    mLayer = const_cast<Properties::Layer*>(pPropsLayer);
    mTimeElapsed = 0.0f;
    mTriggerTime = (sndRandom.GetFloat() - 0.5f) * pPropsLayer->mDelayVar * 2.0f + pPropsLayer->mDelay;
    mNode.Remove();
    smActiveList.AddLast(&mNode);
    if (fPriority == 0.0f)
    {
        mPriority = CalculatePriority(pSource3D, pPropsLayer->mSoundProps);
    }
    else
    {
        mPriority = fPriority;
    }
}

// FUNCTION: SWBF2 0x007391d0
void __cdecl SoundLayered::Play(Source3D* pSource3D, const Properties* pProps)
{
    for (PblListSingle::Itor it(pProps->mLayerList); !it.IsDone(); ++it)
    {
        const Properties::Layer* pLayer =
            (const Properties::Layer*)((char*)it.GetNode() - offsetof(Properties::Layer, mNode));
        Sound::Properties* pSoundProps = pLayer->mSoundProps;
        if (pLayer->mDelay == 0.0f)
        {
            Sound::Play(pSoundProps->Get3DMode() ? pSource3D : 0, pSoundProps, 0, 0, true);
            continue;
        }

        float fPriority = 0.0f;
        FireForgetLayer* pFireForget = FireForgetLayer::GetFreeLayer();
        pSource3D->mFlags |= Source3D::flags_updateListener;
        pSource3D->SetProperties(pSoundProps);
        pSource3D->DetermineClosestListener();
        if (!pFireForget)
        {
            fPriority = FireForgetLayer::CalculatePriority(pSource3D, pSoundProps);
            pFireForget = FireForgetLayer::StealLayer(fPriority);
        }
        if (pFireForget)
        {
            pFireForget->Play(pSource3D, pLayer, fPriority);
        }
        pSource3D->SetProperties(0);
    }
}

// FUNCTION: SWBF2 0x00739620
void SoundLayered::SetPosition(const PblVector3* pPos)
{
    for (PblListSingle::Itor it(mSoundList); !it.IsDone(); ++it)
    {
        PblListSingle::Node* pNode = it.GetNode();
        SoundLayer* pSound = pNode == &mSoundList.mHead
            ? 0
            : (SoundLayer*)((char*)pNode - offsetof(SoundLayer, mLayerNode));
        pSound->Source3D::SetPosition(pPos);
    }
}

// FUNCTION: SWBF2 0x00739660
void SoundLayered::SetProperties(const Properties* pProps, SoundLayer* (__cdecl* pLayerCreate)(void),
                                 void (__cdecl* pLayerDestroy)(SoundLayer*))
{
    FreeLayers(pLayerDestroy);
    if (!pProps)
    {
        return;
    }
    for (PblListSingle::Itor it(pProps->mLayerList); !it.IsDone(); ++it)
    {
        const Properties::Layer* pLayer =
            (const Properties::Layer*)((char*)it.GetNode() - offsetof(Properties::Layer, mNode));
        SoundLayer* pSound = pLayerCreate();
        if (!pSound)
        {
            FreeLayers(pLayerDestroy);
            return;
        }
        mSoundList.AddLast(&pSound->mLayerNode);
        pSound->Sound::SetProperties(pLayer->mSoundProps);
    }
    mProperties = pProps;
}

// FUNCTION: SWBF2 0x00739740
void SoundLayered::SetVelocity(const PblVector3* pVel)
{
    for (PblListSingle::Itor it(mSoundList); !it.IsDone(); ++it)
    {
        PblListSingle::Node* pNode = it.GetNode();
        SoundLayer* pSound = pNode == &mSoundList.mHead
            ? 0
            : (SoundLayer*)((char*)pNode - offsetof(SoundLayer, mLayerNode));
        pSound->Source3D::SetVelocity(pVel);
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x00738b70
SoundLayered::Properties::~Properties()
{
    while (!mLayerList.IsEmpty())
    {
        delete (Layer*)((char*)mLayerList.mHead.mNext - offsetof(Layer, mNode));
    }
    if (mNode.mNext)
    {
        mNode.Remove();
    }
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00738d20
void SoundLayered::Close()
{
    for (unsigned int i = 0; i < 64; ++i)
    {
        delete (FireForgetLayer*)(smFireForgetLayerStorage + i * sizeof(FireForgetLayer));
    }
    smFireForgetLayers = 0;
    smIsOpen = false;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00738dd0
SoundLayered::Properties::Layer* SoundLayered::Properties::FindLayerByHashID(unsigned int uiHashID)
{
    for (PblListSingle::Node* pNode = mLayerList.mHead.mNext; pNode != &mLayerList.mHead;
         pNode = pNode->mNext)
    {
        Layer* const pLayer = (Layer*)((char*)pNode - offsetof(Layer, mNode));
        if (pLayer->mHashID == uiHashID)
        {
            return pLayer;
        }
    }
    return 0;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00738e70
void __cdecl SoundLayered::Properties::Load(PblFileChunk& chunk, bool bOnce)
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
}

namespace Snd {

// FUNCTION: SWBF2 0x00739330
void SoundLayered::Properties::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uiID = child._pFile->Read32();
        if (uiID == 0x07A640F6u)
        {
            const char cVersion = child._pFile->Read8();
            const unsigned int uiHashID = child._pFile->Read32();
            Sound::Properties* const pSoundProps =
                (Sound::Properties*)Sound::Properties::FindByHashID(child._pFile->Read32());
            if (pSoundProps)
            {
                const float fDelay = child._pFile->ReadFloat();
                const float fDelayVar = cVersion >= 4 ? child._pFile->ReadFloat() : 0.0f;
                const float fPlayTime = cVersion >= 5 ? child._pFile->ReadFloat() : FLT_MAX;
                const float fPlayTimeVar = cVersion >= 6 ? child._pFile->ReadFloat() : 0.0f;
                new Layer(pSoundProps, fDelay, fDelayVar, fPlayTime, fPlayTimeVar, uiHashID,
                          mLayerList);
            }
        }
        else if (uiID == 0x8D39BDE6u)
        {
            child._pFile->Read8();
            mHashID = child._pFile->Read32();
        }
    }
}
}
