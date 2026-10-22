#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblListDoubleSorted.h"
#include "sndobjectweightlist.h"
#include "sndsoundproperties.h"
#include "sndsource2d3d.h"
#include "sndsourcevariation.h"
#include "sndsourcevirtual.h"
#include "sndstreambase.h"
#include "sndvoicevirtualbus.h"

class PblVector3;

namespace Snd
{

class Stream;

class SoundStream : public VoiceVirtualBus
{
public:

    class Properties;

    SoundStream();
    ~SoundStream();

    void SetProperties(const Properties* pProps);

    void Play(bool bQueue);

    static void __cdecl Play(const Properties* pProps, Source3D* pSource3D, bool bQueue);
    static void __cdecl Play(const Properties* pProps, bool bQueue);

    class FireForget
    {
    public:

        FireForget();
        ~FireForget();

        Properties* mProperties;

        unsigned char mPos[12];
        unsigned char mVel[12];

        float mPitch;
        float mGain;
        unsigned int mSegmentID;
        PblListDoubleSorted::Node mNode;
    };

    static void Init();

    static void StopAll();

    static void __cdecl UpdateAll();

private:

    static FireForget* __cdecl GetFireForget(unsigned int uiIndex);

    static FireForget* __cdecl AllocFireForget(unsigned int uiIndex, float fPriority, bool bNoSteal);
public:

    static unsigned int smCount[6];
    static Properties* smPlayingProps[6];
    static PblListDoubleSorted smQueue[6];
    static FireForget smFireForget[24];
    static PblListDoubleSorted smFree;

    static PblVector3 smPlayingVel[6];
    static PblVector3 smPlayingPos[6];

    static FreeList smFreeList;
    static PblListSingle smList;

    const Properties* mProperties;
    StreamBase::Info* mInfo;
    PblListSingle::Node mNode;
};

class SoundStream::Properties : public SourceVirtual,
                                public SoundProperties,
                                public Source2D3D,
                                public SourceVariation
{
public:
    Properties();

    static void* operator new(unsigned int)
    {
        return smFreeList.Alloc();
    }
    static void __cdecl operator delete(void* p);

    ~Properties();
    static void __cdecl DestroyAll();

    static void __cdecl Load(PblFileChunk& chunk, bool bOnce);
    void Read(PblFileChunk& chunk);

    static const Properties* __cdecl FindByHashID(unsigned int uiHashID);

    bool SetupStream(Stream* pStream, unsigned int uiSegmentID) const;

    float CalculatePriority(const PblVector3* const pPos, const PblVector3* const pVel) const;

    void SetupSoundStream(SoundStream* pStream) const;

    static FreeList smFreeList;

    static PblListSingle smPropertyList;

    ObjectWeightList mObjectWeightList;
    ObjectWeightList* mSelObjectWeightList;
    unsigned int mStreamID;
    PblListSingle::Node mNode;
};

 

inline void SoundStream::SetProperties(const Properties* pProps)
{
    mInfo = 0;
    pProps->SetupSoundStream(this);
    mProperties = pProps;
}

 
 
 
 

 

}
