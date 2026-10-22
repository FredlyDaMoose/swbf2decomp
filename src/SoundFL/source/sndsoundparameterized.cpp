#include "sndsoundparameterized.h"

#include "sndglobals.h"
#include "sndparametergraph.h"

#include <float.h>
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

namespace Snd
{

// GLOBAL: SWBF2 0x007e34cc
// SYNTHETIC: SWBF2 0x0040b810
// `dynamic initializer for 'Snd::SoundParameterized::Controller::Properties::smFreeList''
// SYNTHETIC: SWBF2 0x0076a260
// `dynamic atexit destructor for 'Snd::SoundParameterized::Controller::Properties::smFreeList''
FreeList SoundParameterized::Controller::Properties::smFreeList(0x10, 1, 0x10, 1);

// GLOBAL: SWBF2 0x007e34e8
// SYNTHETIC: SWBF2 0x0040b830
// `dynamic initializer for 'Snd::SoundParameterized::Layer::Properties::smFreeList''
// SYNTHETIC: SWBF2 0x0076a270
// `dynamic atexit destructor for 'Snd::SoundParameterized::Layer::Properties::smFreeList''
FreeList SoundParameterized::Layer::Properties::smFreeList(0x0c, 1, 0x10, 1);

// GLOBAL: SWBF2 0x007e3520
// SYNTHETIC: SWBF2 0x0040b850
// `dynamic initializer for 'Snd::SoundParameterized::Properties::smFreeList''
// SYNTHETIC: SWBF2 0x0076a280
// `dynamic atexit destructor for 'Snd::SoundParameterized::Properties::smFreeList''
FreeList SoundParameterized::Properties::smFreeList(0x10, 1, 0x10, 1);

// GLOBAL: SWBF2 0x007e353c
// SYNTHETIC: SWBF2 0x0040b870
// `dynamic initializer for 'Snd::SoundParameterized::Properties::smPropertyList''
// SYNTHETIC: SWBF2 0x0076a290
// `dynamic atexit destructor for 'Snd::SoundParameterized::Properties::smPropertyList''
PblListSingle SoundParameterized::Properties::smPropertyList;

// GLOBAL: SWBF2 0x007e3540
// SYNTHETIC: SWBF2 0x0040b880
// `dynamic initializer for 'Snd::SoundParameterized::smSoundParameterizedList''
// SYNTHETIC: SWBF2 0x0076a2a0
// `dynamic atexit destructor for 'Snd::SoundParameterized::smSoundParameterizedList''
PblListDouble SoundParameterized::smSoundParameterizedList;

// FUNCTION: SWBF2 0x007357f0
Sound::Sound()
    : VoiceVirtualBus(0, 0, true),
      mProperties(0)
{
    smSoundList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x00735790
SoundParameterized::Layer::Layer()
{
    Reset();
}

// FUNCTION: SWBF2 0x00735880
SoundParameterized::SoundParameterized()
{
    mUpdateTime = false;
    mProperties = 0;
    mNode.Remove();
    smSoundParameterizedList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x007358e0
SoundParameterized::Layer::Properties::~Properties()
{
    while (!mControllerPropertyList.IsEmpty())
    {
        delete (Controller::Properties*)((char*)mControllerPropertyList.mHead.mNext
                                         - offsetof(Controller::Properties, mNode));
    }
    if (mNode.mNext)
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x00735a40
SoundParameterized::~SoundParameterized()
{
    Stop();
    SoundLayered::Update(0.0f);
    SetProperties(0);
}

// FUNCTION: SWBF2 0x00735ae0
void __cdecl SoundParameterized::Layer::operator delete(void* p)
{
    Snd::SoundParameterized::Layer::smFreeList.Free(p);
}

// FUNCTION: SWBF2 0x00735b40
void __cdecl SoundParameterized::Properties::DestroyAll()
{
    while (!smPropertyList.IsEmpty())
    {
        delete (Properties*)((char*)smPropertyList.mHead.mNext - offsetof(Properties, mNode));
    }
}

// FUNCTION: SWBF2 0x00735c00
SoundParameterized::Properties* __cdecl SoundParameterized::Properties::FindByHashID(unsigned int uiHashID)
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

// FUNCTION: SWBF2 0x00735c30
void __cdecl SoundParameterized::Free()
{
    unsigned int uiNumBlocks = 0;
    for (const PblListSingle::Node* pNode = Snd::SoundParameterized::Layer::smFreeList.mBlockList.mHead.mNext;
         pNode != &Snd::SoundParameterized::Layer::smFreeList.mBlockList.mHead; pNode = pNode->mNext)
    {
        ++uiNumBlocks;
    }
    if (uiNumBlocks)
    {
        Snd::SoundParameterized::Layer::smFreeList.mFlags &= ~FreeList::flags_locked;
        Snd::SoundParameterized::Layer::smFreeList.PurgeBlocks();
    }
}

void* __cdecl SoundParameterized::Layer::operator new(unsigned int)
{
    return Snd::SoundParameterized::Layer::smFreeList.Alloc();
}

// FUNCTION: SWBF2 0x00735c60
SoundLayered::SoundLayer* __cdecl SoundParameterized::LayerCreate()
{
    return new Layer;
}

// FUNCTION: SWBF2 0x00735cd0
void __cdecl SoundParameterized::LayerDestroy(SoundLayer* pLayer)
{
    delete static_cast<Layer*>(pLayer);
}

// FUNCTION: SWBF2 0x00735d20
void __cdecl SoundParameterized::Properties::Load(PblFileChunk& chunk, bool bOnce)
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

// FUNCTION: SWBF2 0x00735df0
bool SoundParameterized::Properties::QueryParameter(unsigned int uiParameterID) const
{
    for (const PblListSingle::Node* pLayerNode = mLayerList.mHead.mNext;
         pLayerNode != &mLayerList.mHead; pLayerNode = pLayerNode->mNext)
    {
        const Layer::Properties* pLayer =
            (const Layer::Properties*)((const char*)pLayerNode - offsetof(Layer::Properties, mNode));
        for (const PblListSingle::Node* pNode = pLayer->mControllerPropertyList.mHead.mNext;
             pNode != &pLayer->mControllerPropertyList.mHead; pNode = pNode->mNext)
        {
            const Controller::Properties* pController =
                (const Controller::Properties*)((const char*)pNode
                                                - offsetof(Controller::Properties, mNode));
            if (pController->mInputParameterID == uiParameterID)
            {
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00735e40
void SoundParameterized::Layer::Properties::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uiID = child._pFile->Read32();
        child._pFile->Read8();
        if (uiID == 0x8D39BDE6u)
        {
            mHashID = child._pFile->Read32();
        }
        else if (uiID == 0xE657A914u)
        {
            Controller::Properties* const pController = new Controller::Properties;
            pController->mInputParameterID = child._pFile->Read32();
            pController->mGraph = ParameterGraph::FindByHashID(child._pFile->Read32());
            pController->mSoundParameterID = child._pFile->Read32();
            mControllerPropertyList.AddLast(&pController->mNode);
        }
    }
}

// FUNCTION: SWBF2 0x00735f20
void SoundParameterized::Properties::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uiID = child._pFile->Read32();
        child._pFile->Read8();
        if (uiID == 0x07A640F6u)
        {
            Layer::Properties* const pLayer = new Layer::Properties;
            mLayerList.AddLast(&pLayer->mNode);
            child = chunk.ReadNextChild();
            pLayer->Read(child);
        }
        else if (uiID == 0x8D39BDE6u)
        {
            mHashID = child._pFile->Read32();
        }
        else if (uiID == 0xA54A2138u)
        {
            mSoundLayeredProps = SoundLayered::Properties::FindByHashID(child._pFile->Read32());
        }
    }

    if (mSoundLayeredProps)
    {
        for (PblListSingle::Node* pNode = mLayerList.mHead.mNext; pNode != &mLayerList.mHead;
             pNode = pNode->mNext)
        {
            Layer::Properties* pLayer =
                (Layer::Properties*)((char*)pNode - offsetof(Layer::Properties, mNode));
            const_cast<SoundLayered::Properties*>(mSoundLayeredProps)->FindLayerByHashID(pLayer->mHashID);
        }
    }
}

// FUNCTION: SWBF2 0x00736040
void __cdecl SoundParameterized::RemoveAll()
{
    PblListDouble::Node* pNode = smSoundParameterizedList.mTerminator.mNext;
    while (pNode != &smSoundParameterizedList.mTerminator)
    {
        SoundParameterized* pSound =
            (SoundParameterized*)((char*)pNode - offsetof(SoundParameterized, mNode));
        pNode = pNode->mNext;
        pSound->Stop();
        pSound->mNode.Remove();
        pSound->SetProperties(0);
    }
}

// FUNCTION: SWBF2 0x007360c0
void SoundParameterized::ResetParameters()
{
    for (const PblListSingle::Node* pNode = mProperties->mLayerList.mHead.mNext;
         pNode != &mProperties->mLayerList.mHead; pNode = pNode->mNext)
    {
        const Layer::Properties* pLayerProps =
            (const Layer::Properties*)((const char*)pNode - offsetof(Layer::Properties, mNode));
        Layer* pLayer = static_cast<Layer*>(FindLayerByHashID(pLayerProps->mHashID));
        if (pLayer)
        {
            pLayer->Reset();
        }
    }
}

// FUNCTION: SWBF2 0x00736120
void SoundParameterized::SetParameter(unsigned int uiParameterID, float fValue, bool bScale)
{
    for (const PblListSingle::Node* pLayerNode = mProperties->mLayerList.mHead.mNext;
         pLayerNode != &mProperties->mLayerList.mHead; pLayerNode = pLayerNode->mNext)
    {
        const Layer::Properties* pLayerProps =
            (const Layer::Properties*)((const char*)pLayerNode - offsetof(Layer::Properties, mNode));
        for (const PblListSingle::Node* pNode = pLayerProps->mControllerPropertyList.mHead.mNext;
             pNode != &pLayerProps->mControllerPropertyList.mHead; pNode = pNode->mNext)
        {
            const Controller::Properties* pController =
                (const Controller::Properties*)((const char*)pNode
                                                - offsetof(Controller::Properties, mNode));
            if (pController->mInputParameterID != uiParameterID)
            {
                continue;
            }

            Layer* pLayer = static_cast<Layer*>(FindLayerByHashID(pLayerProps->mHashID));
            if (!pLayer)
            {
                continue;
            }

            const unsigned int uIndex =
                Sound::FindParameterIndexByHashID(pController->mSoundParameterID);
            if (bScale)
            {
                pLayer->mParameterValues[uIndex] *= fValue;
            }
            else if (pController->mGraph)
            {
                pLayer->mParameterValues[uIndex] *= pController->mGraph->CalculateOutputValue(fValue);
            }
        }
    }
}

// FUNCTION: SWBF2 0x00736200
void SoundParameterized::SetProperties(const Properties* pProps)
{
    if (mProperties)
    {
        mUpdateTime = false;
        mProperties = 0;
        SoundLayered::SetProperties(0, &LayerCreate, &LayerDestroy);
    }
    if (pProps)
    {
        SoundLayered::SetProperties(pProps->mSoundLayeredProps, &LayerCreate, &LayerDestroy);
        if (SoundLayered::mProperties)
        {
            mProperties = pProps;
            mUpdateTime = pProps->QueryParameter(0x5D3C9BE4u);
        }
    }
}

// FUNCTION: SWBF2 0x007364f0
void __cdecl SoundParameterized::UpdateAll(float fDeltaTime)
{
    for (PblListDouble::Node* pNode = smSoundParameterizedList.mTerminator.mNext;
         pNode != &smSoundParameterizedList.mTerminator; pNode = pNode->mNext)
    {
        ((SoundParameterized*)((char*)pNode - offsetof(SoundParameterized, mNode)))
            ->Update(fDeltaTime);
    }
}

// FUNCTION: SWBF2 0x00736270
void SoundParameterized::Update(float fDeltaTime)
{
    SoundLayered::Update(fDeltaTime);
    if (!mProperties || GetPlayState() != playState_playing)
    {
        return;
    }

    if (mUpdateTime)
    {
        SetParameter(0x5D3C9BE4u, mTimeElapsed, false);
    }

    for (const PblListSingle::Node* pNode = mProperties->mLayerList.mHead.mNext;
         pNode != &mProperties->mLayerList.mHead; pNode = pNode->mNext)
    {
        const Layer::Properties* pLayerProps =
            (const Layer::Properties*)((const char*)pNode - offsetof(Layer::Properties, mNode));
        Layer* pLayer = static_cast<Layer*>(FindLayerByHashID(pLayerProps->mHashID));
        if (!pLayer)
        {
            continue;
        }

        const Sound::Properties* pProps = pLayer->mProperties;
        const float* pValues = pLayer->mParameterValues;
        pLayer->SetGain(Clamp(pProps->mGain * pValues[0], 0.0f, 1.0f));
        pLayer->SetPitch(Clamp(pProps->mPitch * pValues[1], 0.1f, FLT_MAX));
        pLayer->SetReverbGain(Clamp(pProps->mReverbGain * pValues[2], 0.0f, FLT_MAX));
        pLayer->SetPan(Clamp(pProps->mPan * pValues[3], -1.0f, 1.0f));
        const float fMinDistance = Clamp(pProps->mMinDistance * pValues[4], 0.0f, FLT_MAX);
        pLayer->SetMinDistance(fMinDistance);
        const float fMuteDistance = Clamp(pProps->mMuteDistance * pValues[5], fMinDistance, FLT_MAX);
        pLayer->SetMuteDistance(fMuteDistance);
        pLayer->SetMaxDistance(Clamp(pProps->mMaxDistance * pValues[6], fMuteDistance, FLT_MAX));
        pLayer->SetRollOff(Clamp(pProps->mRollOffFactor * pValues[7], 0.0f, FLT_MAX));
        pLayer->SetRollIn(Clamp(pProps->mRollInFactor * pValues[9], 0.0f, FLT_MAX));
        pLayer->SetRollInDistance(Clamp(pProps->mRollInDistance * pValues[10], 0.0f, FLT_MAX));
    }
}

}
