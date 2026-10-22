#pragma once

#include <stddef.h>

#include "sndfreelist.h"
#include "sndsoundlayered.h"

#include "PebbleFL/Common/PblListDouble.h"
#include "PebbleFL/Common/PblListSingle.h"

class PblFileChunk;

namespace Snd
{

class ParameterGraph;

class SoundParameterized : public SoundLayered
{
public:
    class Controller
    {
    public:

        class Properties
        {
        public:
            Properties() {}
            ~Properties()
            {
                if (mNode.mNext)
                {
                    mNode.Remove();
                }
            }
            static void* operator new(unsigned int) { return smFreeList.Alloc(); }
            static void operator delete(void* p) { smFreeList.Free(p); }

            const ParameterGraph* mGraph;
            unsigned int mInputParameterID;
            unsigned int mSoundParameterID;
            PblListSingle::Node mNode;

            static FreeList smFreeList;
        };
    };

    class Layer : public SoundLayered::SoundLayer
    {
    public:

        Layer();

        static void* __cdecl operator new(unsigned int uiSize);
        static void __cdecl operator delete(void* p);

        void Reset()
        {
            for (int i = 0; i < 11; ++i)
            {
                mParameterValues[i] = 1.0f;
            }
        }

        float mParameterValues[11];

        class Properties
        {
        public:
            Properties() : mHashID(0) {}
            ~Properties();
            static void* operator new(unsigned int) { return smFreeList.Alloc(); }
            static void operator delete(void* p) { smFreeList.Free(p); }

            void Read(PblFileChunk& chunk);

            unsigned int mHashID;
            PblListSingle mControllerPropertyList;
            PblListSingle::Node mNode;

            static FreeList smFreeList;
        };
public:
    static Snd::FreeList smFreeList;
    };

    class Properties
    {
    public:

        Properties() : mHashID(0), mSoundLayeredProps(0)
        {
            smPropertyList.AddLast(&mNode);
        }

        ~Properties()
        {
            while (!mLayerList.IsEmpty())
            {
                delete (Layer::Properties*)((char*)mLayerList.mHead.mNext
                                            - offsetof(Layer::Properties, mNode));
            }
            if (mNode.mNext)
            {
                mNode.Remove();
            }
        }
        static void* operator new(unsigned int) { return smFreeList.Alloc(); }
        static void operator delete(void* p) { smFreeList.Free(p); }

        static void __cdecl Load(PblFileChunk& chunk, bool bOnce);
        static void __cdecl DestroyAll();
        static Properties* __cdecl FindByHashID(unsigned int uiHashID);
        bool QueryParameter(unsigned int uiParameterID) const;
        void Read(PblFileChunk& chunk);

        unsigned int mHashID;
        PblListSingle mLayerList;
        const SoundLayered::Properties* mSoundLayeredProps;
        PblListSingle::Node mNode;

        static FreeList smFreeList;
        static PblListSingle smPropertyList;
    };

    SoundParameterized();
    ~SoundParameterized();

    void SetProperties(const Properties* pProps);
    void Update(float fDeltaTime);
    void ResetParameters();
    void SetParameter(unsigned int uiParameterID, float fValue, bool bScale);

    static void __cdecl RemoveAll();

    static void __cdecl UpdateAll(float fDeltaTime);
    static void __cdecl Free();
    static SoundLayer* __cdecl LayerCreate();
    static void __cdecl LayerDestroy(SoundLayer* pLayer);

    const Properties* mProperties;
    bool mUpdateTime : 1;

    static PblListDouble smSoundParameterizedList;
};

 
 
 
 
 
 
 

}
