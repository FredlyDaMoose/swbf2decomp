#pragma once

class PblFileChunk;

#include "sndobjectlist.h"
#include "sndsourcebus.h"
#include "sndsourcei3dl2.h"
#include "sndvoicebase.h"
#include "sndsampleinfo.h"
#include "sndsamplebase.h"
#include "sndvoicevirtualmanager.h"

namespace Snd
{

class StreamBase : public VoiceBase,
                   public SourceBus,
                   public SourceI3DL2Interpolated
{
public:

    class Segment;
    class Info;

    StreamBase();

    ~StreamBase();

    // FUNCTION: SWBF2 0x0059b4c0
    PblListSingle::Itor GetInfoItor() const { return PblListSingle::Itor(mInfoList); }

    bool IsOpen() const { return !mInfoList.IsEmpty(); }
    void Play(bool bRestart);

    // FUNCTION: SWBF2 0x00538770
    void Stop()
    {
        SourceBus::Stop(static_cast<SourceTransport*>(this));
        SourceTransport::Stop();
    }

    // FUNCTION: SWBF2 0x0059b440
    void SetGain(float fGain) { SourceBus::SetGain(static_cast<Source2D3D*>(this), fGain); }
    Segment* FindSegment(unsigned int uiSegmentID) const;
    void SeekSegment(const Segment* pSegment);

protected:

    bool Open(PblFileChunk& chunk, unsigned int uiSegment, const char* pcFileName,
              void (__cdecl* pfnIdle)(StreamBase*, void*), void* pIdleData,
              bool bArg);
    bool LoadSegments(PblFileChunk& chunk, Segment* pSegments,
                      unsigned int uiNumSegments, bool bArg);
    void Close();

    bool AppendSegments(PblFileChunk& chunk, unsigned int uiHashID, const char* pcFileName,
                        void (__cdecl* pfnIdle)(StreamBase*, void*), void* pIdleData,
                        bool bNoPadding);
    void Update(float fDeltaTime);
public:

    Segment* mCurrentSegment;
    unsigned char mUpdateI3DL2Source;
    ObjectList mInfoList;
};

 

class StreamBase::Segment : public SampleBase
{
protected:

    Segment(unsigned int uiHashID, unsigned int uiOffset, unsigned int uiLength,
            unsigned int uiLengthSamples, unsigned int uiFrequency, unsigned int uiPadding)
        : SampleBase(uiHashID, (void*)uiOffset, uiLength, uiLengthSamples,
                     (float)uiFrequency, SampleInfo::format_Unknown, 0),
          mPadding(uiPadding)
    {
    }
    friend class StreamBase;

public:

    static int __cdecl CompareSort(void const* pA, void const* pB);

    static int __cdecl CompareSearch(void const* pKey, void const* pElement);

    unsigned int mPadding;
};

 

class StreamBase::Info : public Allocator<StreamBase::Info>
{
public:

    // FUNCTION: SWBF2 0x0059b340
    static Info* Find(unsigned int uiHashID)
    {
        for (PblListSingle::Node* pLink = smInfoList.mHead.mNext; pLink != &smInfoList.mHead;
             pLink = pLink->mNext)
        {
            Info* pInfo = (Info*)((char*)pLink - offsetof(Info, mNode));
            if (pInfo->mHashID == uiHashID)
            {
                return pInfo;
            }
        }
        return 0;
    }
protected:

    static Info* Find(unsigned int uiHashID, StreamBase* pStream, bool bAddRef);

    Info(unsigned int uiHashID, Segment* pSegments, unsigned int uiNumSegments,
         unsigned int uiChannelInterleave, unsigned int uiNumChannels,
         SampleInfo::Format eFormat, unsigned int uiNumSubstreams,
         unsigned int uiSubstreamInterleave);

    ~Info();
    friend class StreamBase;
    friend class EngineBase;

    static void UpdateAll(float fDeltaTime)
    {
        for (PblListSingle::Node* pLink = smInfoList.mHead.mNext; pLink != &smInfoList.mHead;
             pLink = pLink->mNext)
        {
            Info* pInfo = (Info*)((char*)pLink - offsetof(Info, mNode));
            if (!pInfo->mOverrideManager)
            {
                pInfo->mStreamManager.Update(fDeltaTime);
            }
        }
    }
public:

    VoiceVirtualManager* GetStreamManager()
    {
        return mOverrideManager ? mOverrideManager : &mStreamManager;
    }

    static PblListSingle smInfoList;

    unsigned int mHashID;
    Segment* mSegments;
    unsigned int mNumSegments;
    unsigned int mChannelInterleave;
    unsigned int mNumChannels;
    SampleInfo::Format mFormat;
    unsigned int mNumSubstreams;
    unsigned int mSubstreamInterleave;
    unsigned char mRefCount;
    VoiceVirtualManager mStreamManager;
    VoiceVirtualManager* mOverrideManager;
    PblListSingle::Node mNode;
};

 

}
