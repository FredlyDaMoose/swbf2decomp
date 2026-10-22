#pragma once

#include <float.h>
#include <stddef.h>

#include "sndallocator.h"
#include "sndsound.h"
#include "sndsource3d.h"
#include "sndsourcetransport.h"

#include "PebbleFL/Common/PblListSingle.h"

#include "PebbleFL/Common/PblListDouble.h"

class PblFileChunk;
class PblVector3;

namespace Snd
{

class SoundLayered : public SourceTransport
{
public:
    class Properties;
    class SoundLayer;

    SoundLayered();

    Sound* FindLayerByHashID(unsigned int uiHashID);

    Sound* GetLayerByIndex(unsigned int uIndex);
    ~SoundLayered();

    static void __cdecl Play(Source3D* pSource3D, const Properties* pProps);

    void SetPosition(const PblVector3* pPos);
    void SetVelocity(const PblVector3* pVel);

    void Play(bool bRestart) { SourceTransport::Play(bRestart); }

    class FireForgetLayer : public Allocator<FireForgetLayer>
    {
    public:

        FireForgetLayer()
            : mTimeElapsed(0.0f),
              mTriggerTime(0.0f),
              mSource3D(0),
              mLayer(0)
        {
            smInactiveList.AddLast(&mNode);
        }

        ~FireForgetLayer()
        {
            if (mNode.IsAttached())
            {
                mNode.Remove();
            }
        }

        void Stop()
        {
            mNode.Remove();
            smInactiveList.AddLast(&mNode);
        }

        static void StopAll();

        static FireForgetLayer* __cdecl GetFreeLayer();

        void Play(Source3D* pSource3D, const void* pLayer, float fPriority);

        static float __cdecl CalculatePriority(Source3D* pSource3D,
                                               const Sound::Properties* pProps);
        static FireForgetLayer* __cdecl StealLayer(float fPriority);

        static void UpdateAll(float fDeltaTime);

    protected:

        void Update(float fDeltaTime);

    public:

        float mTimeElapsed;
        float mTriggerTime;
        float mPriority;
        Source3D mSource3D;
        void* mLayer;
        PblListDouble::Node mNode;

        unsigned char mPos[12];
        unsigned char mVel[12];

        static PblListDouble smInactiveList;

        static PblListDouble smActiveList;
    };

    static void Open();

    static void Close();

    static void __cdecl UpdateAll(float fDeltaTime);

    static void __cdecl RemoveAll();

    static FireForgetLayer* smFireForgetLayers;
    static bool smIsOpen;
    static unsigned char smFireForgetLayerStorage[5888];

    static PblListDouble smSoundLayeredList;

protected:

    void SetProperties(const Properties* pProps, SoundLayer* (__cdecl* pLayerCreate)(void),
                       void (__cdecl* pLayerDestroy)(SoundLayer*));

    void Update(float fDeltaTime);

    void FreeLayers(void (__cdecl* pLayerDestroy)(SoundLayer*));

private:

    static void __cdecl SoundLayerDestroy(SoundLayer* pLayer);

public:
    PblListSingle mSoundList;
    PblListDouble::Node mNode;
    float mTimeElapsed;
    float mStopTime;
    const Properties* mProperties;
};
 
 
 

 

class SoundLayered::SoundLayer : public Sound
{
public:

    SoundLayer() : mPlayTime(0.0f), mStopTime(FLT_MAX) {}

    ~SoundLayer()
    {
        if (mLayerNode.mNext)
        {
            mLayerNode.Remove();
        }
    }

    static void operator delete(void* p) { smFreeList.Free(p); }

    PblListSingle::Node mLayerNode;
    float mPlayTime;
    float mStopTime;

    static FreeList smFreeList;
};
 
 

class SoundLayered::Properties
{
public:

    class Layer
    {
    public:

        Layer(Sound::Properties* pSoundProps, float fDelay, float fDelayVar, float fPlayTime,
              float fPlayTimeVar, unsigned int uiHashID, PblListSingle& list)
            : mSoundProps(pSoundProps), mDelay(fDelay), mDelayVar(fDelayVar),
              mPlayTime(fPlayTime), mPlayTimeVar(fPlayTimeVar), mHashID(uiHashID)
        {
            list.AddLast(&mNode);
        }

        ~Layer()
        {
            if (mNode.mNext)
            {
                mNode.Remove();
            }
        }

        static void* operator new(unsigned int) { return smFreeList.Alloc(); }
        static void operator delete(void* p) { smFreeList.Free(p); }

        Sound::Properties* mSoundProps;
        float mDelay;
        float mDelayVar;
        float mPlayTime;
        float mPlayTimeVar;
        unsigned int mHashID;
        PblListSingle::Node mNode;

        static FreeList smFreeList;
    };

    static Properties* __cdecl FindByHashID(unsigned int uiHashID);

    Layer* FindLayerByHashID(unsigned int uiHashID);

    unsigned int GetHashID() const { return mHashID; }

    Layer* FindLayerByIndex(unsigned int uIndex) const
    {
        const PblListSingle::Node* pNode =
            mLayerList.IsEmpty() ? 0 : mLayerList.mHead.mNext;
        while (pNode != &mLayerList.mHead)
        {
            if (uIndex-- == 0)
            {
                return (Layer*)((char*)pNode - offsetof(Layer, mNode));
            }
            pNode = pNode->mNext;
        }
        return 0;
    }

    PblListSingle mLayerList;
    unsigned int mHashID;

    static FreeList smFreeList;
    static PblListSingle smPropertyList;

    Properties() : mHashID(0)
    {
        smPropertyList.AddLast(&mNode);
    }
    static void* operator new(unsigned int) { return smFreeList.Alloc(); }
    static void operator delete(void* p) { smFreeList.Free(p); }

    ~Properties();
    static void __cdecl DestroyAll();

    void Read(PblFileChunk& chunk);

    PblListSingle::Node mNode;

    static void __cdecl Load(PblFileChunk& chunk, bool bOnce);
};

inline Sound* SoundLayered::GetLayerByIndex(unsigned int uIndex)
{
    PblListSingle::Node* pNode = mSoundList.IsEmpty() ? 0 : mSoundList.mHead.mNext;
    if (pNode)
    {
        for (; uIndex; --uIndex)
        {
            pNode = pNode->mNext;
        }
    }
    if (pNode == &mSoundList.mHead)
    {
        return 0;
    }
    return (SoundLayer*)((char*)pNode - offsetof(SoundLayer, mLayerNode));
}

}
