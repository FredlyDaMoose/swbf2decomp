#include "sndstreambase.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include <stdlib.h>

namespace Snd
{

StreamBase::Info::~Info()
{
    if (mNode.mNext)
    {
        mNode.Remove();
    }
}

template <>
void Allocator<StreamBase::Info>::operator delete(void* p, unsigned int uiSize)
{
    (void)uiSize;
    switch (((StreamBase::Info*)p)->GetAllocType())
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

// FUNCTION: SWBF2 0x00742f20
StreamBase::StreamBase()
    : mCurrentSegment(0),
      mUpdateI3DL2Source(1)
{

    const unsigned char f = Source2D3D::mFlags;
    Source2D3D::mFlags =
        (unsigned char)((f & (unsigned char)~3u)
                        | (unsigned char)((f & 1) << 1)
                        | 4u);
}

// GLOBAL: SWBF2 0x007e3860
// SYNTHETIC: SWBF2 0x0040c0b0
// `dynamic initializer for 'Snd::StreamBase::Info::smInfoList''
// SYNTHETIC: SWBF2 0x0076a480
// `dynamic atexit destructor for 'Snd::StreamBase::Info::smInfoList''
PblListSingle StreamBase::Info::smInfoList;

}

namespace Snd {

// FUNCTION: SWBF2 0x00742e90
StreamBase::Info::Info(unsigned int uiHashID, Segment* pSegments, unsigned int uiNumSegments,
                       unsigned int uiChannelInterleave, unsigned int uiNumChannels,
                       SampleInfo::Format eFormat, unsigned int uiNumSubstreams,
                       unsigned int uiSubstreamInterleave)
    : mHashID(uiHashID),
      mSegments(pSegments),
      mNumSegments(uiNumSegments),
      mChannelInterleave(uiChannelInterleave),
      mNumChannels(uiNumChannels),
      mFormat(eFormat),
      mNumSubstreams(uiNumSubstreams),
      mSubstreamInterleave(uiSubstreamInterleave),
      mRefCount(0),
      mOverrideManager(0)
{
    ++mRefCount;
    smInfoList.AddLast(&mNode);
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00742fd0
StreamBase::~StreamBase()
{
}
}

namespace Snd {

// FUNCTION: SWBF2 0x007430c0
bool StreamBase::AppendSegments(PblFileChunk& chunk, unsigned int uiHashID, const char* pcFileName,
                                void (__cdecl* pfnIdle)(StreamBase*, void*), void* pIdleData,
                                bool bNoPadding)
{
    StreamBase stream;
    if (!stream.Open(chunk, uiHashID, pcFileName, pfnIdle, pIdleData, bNoPadding))
    {
        return false;
    }

    mInfoList.Add(Info::Find(uiHashID, this, true));
    stream.Close();
    chunk._pFile->Close();
    return true;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x007431f0
void StreamBase::Close()
{
    if (mNode.mNext)
    {
        RemoveFromManager();
    }

    PblListSingle::Node* pNode = mInfoList.mHead.mNext;
    while (pNode != &mInfoList.mHead)
    {
        Info* const pInfo = (Info*)((ObjectList::Node*)pNode)->mObject;
        pNode = pNode->mNext;
        mInfoList.Remove(pInfo);
        if (--pInfo->mRefCount == 0)
        {
            delete pInfo;
            Free(pInfo);
        }
    }
    mCurrentSegment = 0;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x007432d0
int __cdecl StreamBase::Segment::CompareSearch(void const* pKey, void const* pElement)
{
    const unsigned int uiKey = *(const unsigned int*)pKey;
    const unsigned int uiHash = ((const Segment*)pElement)->mHashID;
    if (uiKey > uiHash)
    {
        return 1;
    }
    return uiKey < uiHash ? -1 : 0;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x007432f0
int __cdecl StreamBase::Segment::CompareSort(void const* pA, void const* pB)
{
    const unsigned int uiA = ((const Segment*)pA)->mHashID;
    const unsigned int uiB = ((const Segment*)pB)->mHashID;
    if (uiA > uiB)
    {
        return 1;
    }
    return uiA < uiB ? -1 : 0;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00743310
bool StreamBase::LoadSegments(PblFileChunk& chunk, Segment* pSegments,
                              unsigned int uiNumSegments, bool bNoPadding)
{
    unsigned int uiCount = 0;
    unsigned int uiOffset = 0;
    unsigned int uiRemaining = uiNumSegments;
    Segment* pNext = pSegments;

    while (chunk.HasMoreChildren())
    {
        if (!uiRemaining)
        {
            break;
        }

        if (chunk._pFile->Read32() == 0x96E382A7u)
        {
            unsigned int uiHashID = 0;
            unsigned int uiFrequency = 0;
            unsigned int uiLength = 0;
            unsigned int uiLengthSamples = 0;
            unsigned int uiPadding = 0;
            unsigned int uiAliasOffset = 0;
            unsigned int uiSize = 0;
            bool bContinue = true;
            bool bAlias = false;

            while (chunk.HasMoreChildren() && bContinue)
            {
                switch ((unsigned int)chunk._pFile->Read32())
                {
                case 0x2E789FB4u:
                    bContinue = false;
                    break;
                case 0x23A0D95Cu:
                    uiLength = chunk._pFile->Read32();
                    break;
                case 0x1D48FEEFu:
                    uiLengthSamples = chunk._pFile->Read32();
                    break;
                case 0x2FB31C01u:
                    uiFrequency = chunk._pFile->Read32();
                    break;
                case 0x37386AE0u:
                    uiHashID = chunk._pFile->Read32();
                    break;
                case 0x7D268157u:
                {
                    const unsigned int uiAliasID = chunk._pFile->Read32();
                    for (unsigned int i = 0; i < uiCount; ++i)
                    {
                        if (pSegments[i].mHashID == uiAliasID)
                        {
                            uiLengthSamples = pSegments[i].mLengthSamples;
                            uiLength = pSegments[i].mLength;
                            uiAliasOffset = (unsigned int)pSegments[i].mAddress;
                            break;
                        }
                    }
                    bAlias = true;
                    break;
                }
                case 0x809608B6u:
                    uiPadding = chunk._pFile->Read32();
                    break;
                }
            }

            if (!bAlias)
            {
                uiSize = bNoPadding ? uiLength : uiPadding + uiLength;
            }
            ++uiCount;
            uiOffset += uiSize;
            new (pNext++, false) Segment(uiHashID, bAlias ? uiAliasOffset : uiOffset - uiSize,
                                         uiLength, uiLengthSamples, uiFrequency, uiPadding);
        }
        --uiRemaining;
    }

    qsort(pSegments, uiNumSegments, sizeof(Segment), Segment::CompareSort);
    return true;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00743580
bool StreamBase::Open(PblFileChunk& chunk, unsigned int uiHashID, const char* pcFileName,
                      void (__cdecl* pfnIdle)(StreamBase*, void*), void* pIdleData,
                      bool bNoPadding)
{
    Info* pInfo = 0;
    bool bInfo = false;
    bool bNewInfo = true;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x5F6F6D65u:
        case 0x5BB97F21u:
            if (Open(child, uiHashID, pcFileName, pfnIdle, pIdleData, bNoPadding))
            {
                return true;
            }
            break;

        case 0x0FB40705u:
        {
            unsigned int uiName = 0;
            unsigned int uiNumSegments = 0;
            unsigned int uiNumChannels = 0;
            unsigned int uiChannelInterleave = 0;
            unsigned int uiNumSubstreams = 0;
            unsigned int uiSubstreamInterleave = 0;
            unsigned int uiFormat = 0;
            bool bContinue = true;
            bInfo = true;

            while (child.HasMoreChildren() && bContinue)
            {
                switch ((unsigned int)child._pFile->Read32())
                {
                case 0x23A0D95Cu:
                    child._pFile->Read32();
                    break;
                case 0x182FD58Du:
                    uiChannelInterleave = child._pFile->Read32();
                    break;
                case 0x40FBDEBDu:
                    uiNumSegments = child._pFile->Read32();
                    break;
                case 0x740FDB0Cu:
                    uiSubstreamInterleave = child._pFile->Read32();
                    break;
                case 0x7816084Bu:
                    uiNumChannels = child._pFile->Read32();
                    break;
                case 0x8D39BDE6u:
                    uiName = child._pFile->Read32();
                    if (uiName != uiHashID && uiHashID)
                    {
                        return false;
                    }
                    break;
                case 0x7AAF1A1Cu:
                    uiNumSubstreams = child._pFile->Read32();
                    break;
                case 0xB99D8552u:
                    uiFormat = child._pFile->Read32();
                    break;
                case 0xB969BE96u:
                    pInfo = Info::Find(uiName, this, true);
                    if (pInfo)
                    {
                        mInfoList.Add(pInfo);
                        bContinue = false;
                        bNewInfo = false;
                    }
                    else
                    {
                        void* pMem = Alloc(sizeof(Info) + uiNumSegments * sizeof(Segment));
                        Segment* pSegments = (Segment*)((char*)pMem + sizeof(Info));
                        pInfo = new (pMem, false) Info(uiName, pSegments, uiNumSegments,
                                                       uiChannelInterleave, uiNumChannels,
                                                       (SampleInfo::Format)uiFormat,
                                                       uiNumSubstreams, uiSubstreamInterleave);
                        if (!mNode.mNext)
                        {

                            VoiceVirtualManager* pManager = pInfo->GetStreamManager();
                            pManager->mVoiceList.AddLast(&mNode);
                            mManager = pManager;
                            ++pManager->mNumManagedVoices;
                        }
                        if (!LoadSegments(child, pSegments, uiNumSegments, bNoPadding))
                        {
                            delete pInfo;
                            Free(pInfo);
                            return false;
                        }
                        mInfoList.Add(pInfo);
                        bContinue = false;
                    }
                    break;
                }
            }
            break;
        }

        case 0xD872E2A5u:
            if (bInfo)
            {
                const int iPos = child._pFile->Tell();
                if (bNewInfo)
                {
                    for (unsigned int i = 0; i < pInfo->mNumSegments; ++i)
                    {
                        pInfo->mSegments[i].mAddress =
                            (char*)pInfo->mSegments[i].mAddress + iPos;
                    }
                }
            }
            break;
        }
    }

    if (!pInfo)
    {
        return false;
    }
    mCurrentSegment = pInfo->mSegments;
    return true;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00743900
void StreamBase::Update(float fDeltaTime)
{
    (void)fDeltaTime;
    Segment* const pSegment = (Segment*)mSample;

    SourceBus::Update(this, this);

    if (Get3DMode() && mUpdateI3DL2Source)
    {
        Source3D::mFlags |= (unsigned char)Source3D::flags_updateListener;
        Source3D::DetermineClosestListener();
        SourceI3DL2Interpolated::Update(Source3D::mClosestListener, Source3D::mPos3D,
                                        Source3D::mProps->mSpaceDistance, true);
    }

    if (pSegment && pSegment != mCurrentSegment)
    {
        SeekSegment(pSegment);
    }
}
}

// SYNTHETIC: SWBF2 0x00743040
// Snd::StreamBase::Info::`scalar deleting destructor'
