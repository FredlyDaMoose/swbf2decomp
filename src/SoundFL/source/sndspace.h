#pragma once

#include <stddef.h>
#include "PebbleFL/Common/PblListSingle.h"
#include "PebbleFL/Common/PblRegion.h"
#include "PebbleFL/Common/PblVector.h"
#include "sndfreelist.h"
#include "sndobjectlist.h"
#include "sndsourcei3dl2.h"

class PblFileChunk;

namespace Snd
{
class I3DL2ReverbPreset;

class Space : public SourceI3DL2
{
public:

    class Connected
    {
    public:

        static FreeList smFreeList;

        Space* mSpace;
        float mGainFactor;
    };

    Space(bool bAddToList = false)
        : mReverbPreset(0),
          mPriority(0.0f)
    {
        if (bAddToList)
        {
            smSpaceList.AddLast(&mNode);
        }
    }

    ~Space();

    static void* operator new(unsigned int)
    {
        return smFreeList.Alloc();
    }
    static void __cdecl operator delete(void* p);

    static bool Open();

    static void Close();
    static void __cdecl DestroyAll();

    static void __cdecl Load(PblFileChunk& chunk, bool bOnce);
    void Read(PblFileChunk& chunk);

    static Space* __cdecl Find(unsigned int uiHashID);

    class Region;
    static unsigned int __cdecl GetCount();
    static Space* __cdecl FindByIndex(unsigned int uIndex);
    unsigned int GetRegionCount() const;
    Region* FindRegionByIndex(unsigned int uIndex);
    void ConnectSpace(Space* const pSpace, float fGainFactor);
    void SetReverbPreset(I3DL2ReverbPreset* const pPreset);

    static FreeList smFreeList;

    static Space* __cdecl GetSpace(const ::PblVector3& rPos);

    bool IsPointInside(const ::PblVector3& rPoint, float fRadius) const;

    void SetupSource(SourceI3DL2* pSource, bool bInsideSpace) const;

    void SetupSourceInterpolated(SourceI3DL2Interpolated* pSource,
                                 bool bInsideSpace) const;

    static PblListSingle smSpaceList;

    static Space smDefaultSpace;

    unsigned int mHashID;
    PblListSingle mList;
    void* mReverbPreset;
    PblListSingle::Node mNode;
    ObjectList mConnected;
    float mPriority;

protected:

    void CalculateObstructionOcclusion(SourceI3DL2* pSource,
                                       float& rfObstructionHFGain,
                                       float& rfOcclusionHFGain,
                                       float& rfObstructionLFRatio,
                                       float& rfOcclusionLFRatio,
                                       bool bInsideSpace) const;
};

 

 
 
 
 
 
 

class Space::Region : public PblRegion
{
public:

    Region(const PblRegion& rRegion) : PblRegion(rRegion) {}

    ~Region()
    {
        if (mSpaceNode.mNext)
        {
            mSpaceNode.Remove();
        }
    }

    PblListSingle::Node mSpaceNode;
};
 

 

}
