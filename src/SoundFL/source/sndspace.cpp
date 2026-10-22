#include <math.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblRegion.h"
#include "sndi3dl2reverbpreset.h"
#include "sndspace.h"

namespace Snd
{

// GLOBAL: SWBF2 0x009d83c8
// SYNTHETIC: SWBF2 0x0040b640
// `dynamic initializer for 'Snd::Space::smDefaultSpace''
// SYNTHETIC: SWBF2 0x0076a1b0
// `dynamic atexit destructor for 'Snd::Space::smDefaultSpace''
Space Space::smDefaultSpace;

// GLOBAL: SWBF2 0x007e33bc
// SYNTHETIC: SWBF2 0x0040b6d0
// `dynamic initializer for 'Snd::Space::smSpaceList''
// SYNTHETIC: SWBF2 0x0076a1e0
// `dynamic atexit destructor for 'Snd::Space::smSpaceList''
PblListSingle Space::smSpaceList;

// GLOBAL: SWBF2 0x007e341c
// SYNTHETIC: SWBF2 0x0040b690
// `dynamic initializer for 'Snd::Space::Connected::smFreeList''
// SYNTHETIC: SWBF2 0x0076a1c0
// `dynamic atexit destructor for 'Snd::Space::Connected::smFreeList''
FreeList Space::Connected::smFreeList(8, 1, 8, 1);

// GLOBAL: SWBF2 0x007e3400
// SYNTHETIC: SWBF2 0x0040b6b0
// `dynamic initializer for 'Snd::Space::smFreeList''
// SYNTHETIC: SWBF2 0x0076a1d0
// `dynamic atexit destructor for 'Snd::Space::smFreeList''
FreeList Space::smFreeList(sizeof(Space), 1, 8, 1);

// FUNCTION: SWBF2 0x00733a70
bool Space::Open()
{
    smDefaultSpace.mHashID = 0x933B5BDEu;
    smDefaultSpace.mPriority = -1.0f;
    return true;
}

// FUNCTION: SWBF2 0x007336a0
Space* __cdecl Space::GetSpace(const ::PblVector3& rPos)
{
    Space* pBest = &smDefaultSpace;

    for (PblListSingle::Node* pNode = smSpaceList.mHead.mNext;
         pNode != &smSpaceList.mHead;
         pNode = pNode->mNext)
    {

        Space* const pSpace = reinterpret_cast<Space*>(reinterpret_cast<char*>(pNode) - offsetof(Space, mNode));

        if (pSpace->mPriority > pBest->mPriority)
        {
            if (pSpace->IsPointInside(rPos, 0.0f))
            {
                pBest = pSpace;
            }
        }
    }
    return pBest;
}

// FUNCTION: SWBF2 0x007336f0
bool Space::IsPointInside(const ::PblVector3& rPoint, float fRadius) const
{
    if (mList.IsEmpty())
    {
        if (this != &smDefaultSpace)
        {
            return false;
        }
        return GetSpace(rPoint) == &smDefaultSpace;
    }

    const float fLength2EstAlpha = 0.94805944f;
    const float fLength2EstBeta = 0.39269909f;

    const bool bHasRadius = fRadius > 0.0f;
    float fDistance = 0.0f;

    for (const PblListSingle::Node* pNode = mList.mHead.mNext;
         pNode != &mList.mHead;
         pNode = pNode->mNext)
    {

        const PblRegion* const pRegion = reinterpret_cast<const Space::Region*>(
            reinterpret_cast<const char*>(pNode) - offsetof(Space::Region, mSpaceNode));

        float fDeltaX = 0.0f;
        float fDeltaY = 0.0f;
        float fDeltaZ = 0.0f;

        if (bHasRadius)
        {

            fDeltaX = pRegion->mMatrix.m[3][0] - rPoint.x;
            fDeltaY = pRegion->mMatrix.m[3][1] - rPoint.y;
            fDeltaZ = pRegion->mMatrix.m[3][2] - rPoint.z;

            float fA = (float)fabs(fDeltaX);
            float fB = (float)fabs(fDeltaY);
            float fEst = fA > fB ? fA * fLength2EstAlpha + fB * fLength2EstBeta
                                 : fA * fLength2EstBeta + fB * fLength2EstAlpha;

            fA = (float)fabs(fEst);
            fB = (float)fabs(fDeltaZ);
            fDistance = fA > fB ? fA * fLength2EstAlpha + fB * fLength2EstBeta
                                : fA * fLength2EstBeta + fB * fLength2EstAlpha;

            if (fRadius >= fDistance)
            {
                return true;
            }
        }

        if (pRegion->IsPointInside(rPoint))
        {
            return true;
        }

        if (bHasRadius)
        {
            const float fScale = 1.0f / fDistance;
            ::PblVector3 vProbe;
            vProbe.x = rPoint.x + fDeltaX * fScale * fRadius;
            vProbe.y = rPoint.y + fDeltaY * fScale * fRadius;
            vProbe.z = rPoint.z + fDeltaZ * fScale * fRadius;
            if (pRegion->IsPointInside(vProbe))
            {
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00733560
void Space::CalculateObstructionOcclusion(SourceI3DL2* pSource,
                                          float& rfObstructionHFGain,
                                          float& rfOcclusionHFGain,
                                          float& rfObstructionLFRatio,
                                          float& rfOcclusionLFRatio,
                                          bool bInsideSpace) const
{
    if (bInsideSpace)
    {
        rfObstructionHFGain = 1.0f;
        rfObstructionLFRatio = 1.0f;
        rfOcclusionHFGain = 1.0f;
        rfOcclusionLFRatio = 1.0f;
        return;
    }

    const Space* const pSourceSpace = pSource->mSpace;
    float fGainFactor = 1.0f;

    for (const PblListSingle::Node* pNode = mConnected.mHead.mNext;
         pNode != &mConnected.mHead;
         pNode = pNode->mNext)
    {

        const Connected* const pConnected =
            (const Connected*)((const ObjectList::Node*)pNode)->mObject;
        if (pConnected->mSpace == pSourceSpace)
        {
            fGainFactor = pConnected->mGainFactor;
            break;
        }
    }

    rfObstructionHFGain = 1.0f - (1.0f - mObstructionHFGain) * fGainFactor;
    rfObstructionLFRatio = mObstructionLFRatio * fGainFactor;
    rfOcclusionHFGain = 1.0f - (1.0f - mOcclusionHFGain) * fGainFactor;
    rfOcclusionLFRatio = mOcclusionLFRatio * fGainFactor;
}

// FUNCTION: SWBF2 0x00733cd0
void Space::SetupSource(SourceI3DL2* pSource, bool bInsideSpace) const
{
    float fObstructionHFGain;
    float fOcclusionHFGain;
    float fObstructionLFRatio;
    float fOcclusionLFRatio;
    CalculateObstructionOcclusion(pSource, fObstructionHFGain, fOcclusionHFGain,
                                  fObstructionLFRatio, fOcclusionLFRatio,
                                  bInsideSpace);

    pSource->mFlags |= (unsigned short)(SourceI3DL2::flags_obstructionHFGain
                                        | SourceI3DL2::flags_obstructionLFRatio
                                        | SourceI3DL2::flags_occlusionHFGain
                                        | SourceI3DL2::flags_occlusionLFRatio);
    pSource->mObstructionHFGain = fObstructionHFGain;
    pSource->mObstructionLFRatio = fObstructionLFRatio;
    pSource->mOcclusionHFGain = fOcclusionHFGain;
    pSource->mOcclusionLFRatio = fOcclusionLFRatio;

    pSource->mFlags |= (unsigned short)SourceI3DL2::flags_directGain;
    pSource->mDirectGain = mDirectGain;
    pSource->mFlags |= (unsigned short)SourceI3DL2::flags_directHFGain;
    pSource->mDirectHFGain = mDirectHFGain;
    pSource->mFlags |= (unsigned short)SourceI3DL2::flags_roomGain;
    pSource->mRoomGain = mRoomGain;
    pSource->mFlags |= (unsigned short)SourceI3DL2::flags_roomHFGain;
    pSource->mRoomHFGain = mRoomHFGain;
    pSource->mFlags |= (unsigned short)SourceI3DL2::flags_roomRollOffFactor;
    pSource->mRoomRollOffFactor = mRoomRollOffFactor;
}

// FUNCTION: SWBF2 0x00733d90
void Space::SetupSourceInterpolated(SourceI3DL2Interpolated* pSource,
                                    bool bInsideSpace) const
{
    float fObstructionHFGain;
    float fOcclusionHFGain;
    float fObstructionLFRatio;
    float fOcclusionLFRatio;
    CalculateObstructionOcclusion(pSource, fObstructionHFGain, fOcclusionHFGain,
                                  fObstructionLFRatio, fOcclusionLFRatio,
                                  bInsideSpace);

    pSource->mTarget.mFlags |=
        (unsigned short)SourceI3DL2::flags_obstructionHFGain;
    pSource->mTarget.mObstructionHFGain = fObstructionHFGain;
    pSource->mFlags |=
        (unsigned char)SourceI3DL2Interpolated::flags_interpolate;

    pSource->mTarget.mFlags |=
        (unsigned short)SourceI3DL2::flags_obstructionLFRatio;
    pSource->mTarget.mObstructionLFRatio = fObstructionLFRatio;
    pSource->mFlags |=
        (unsigned char)SourceI3DL2Interpolated::flags_interpolate;

    pSource->mTarget.mFlags |=
        (unsigned short)SourceI3DL2::flags_occlusionHFGain;
    pSource->mTarget.mOcclusionHFGain = fOcclusionHFGain;
    pSource->mFlags |=
        (unsigned char)SourceI3DL2Interpolated::flags_interpolate;

    pSource->mTarget.mFlags |=
        (unsigned short)SourceI3DL2::flags_occlusionLFRatio;
    pSource->mTarget.mOcclusionLFRatio = fOcclusionLFRatio;
    pSource->mFlags |=
        (unsigned char)SourceI3DL2Interpolated::flags_interpolate;

    pSource->mTarget.mFlags |= (unsigned short)SourceI3DL2::flags_directGain;
    pSource->mTarget.mDirectGain = mDirectGain;
    pSource->mFlags |=
        (unsigned char)SourceI3DL2Interpolated::flags_interpolate;

    pSource->mTarget.mFlags |= (unsigned short)SourceI3DL2::flags_directHFGain;
    pSource->mTarget.mDirectHFGain = mDirectHFGain;
    pSource->mFlags |=
        (unsigned char)SourceI3DL2Interpolated::flags_interpolate;

    pSource->mTarget.mFlags |= (unsigned short)SourceI3DL2::flags_roomGain;
    pSource->mTarget.mRoomGain = mRoomGain;
    pSource->mFlags |=
        (unsigned char)SourceI3DL2Interpolated::flags_interpolate;

    pSource->mTarget.mFlags |= (unsigned short)SourceI3DL2::flags_roomHFGain;
    pSource->mTarget.mRoomHFGain = mRoomHFGain;
    pSource->mFlags |=
        (unsigned char)SourceI3DL2Interpolated::flags_interpolate;

    pSource->mTarget.mFlags |=
        (unsigned short)SourceI3DL2::flags_roomRollOffFactor;
    pSource->mTarget.mRoomRollOffFactor = mRoomRollOffFactor;
    pSource->mFlags |=
        (unsigned char)SourceI3DL2Interpolated::flags_interpolate;
}

// FUNCTION: SWBF2 0x00733490
Space::~Space()
{

    mList.mHead.mNext = &mList.mHead;

    while (!mConnected.IsEmpty())
    {
        Connected* const pConnected =
            (Connected*)((ObjectList::Node*)mConnected.mHead.mNext)->mObject;

        mConnected.Remove(pConnected);
        Connected::smFreeList.Free(pConnected);
    }

    if (mReverbPreset != 0)
    {
        mReverbPreset = 0;
    }

    if (mNode.mNext != 0)
    {
        mNode.Remove();
    }

}

// FUNCTION: SWBF2 0x00733540
void __cdecl Space::operator delete(void* p)
{
    smFreeList.Free(p);
}

// FUNCTION: SWBF2 0x00733670
Space* __cdecl Space::Find(unsigned int uiHashID)
{
    for (PblListSingle::Node* pNode = smSpaceList.mHead.mNext;
         pNode != &smSpaceList.mHead; pNode = pNode->mNext)
    {
        Space* const pSpace = reinterpret_cast<Space*>(reinterpret_cast<char*>(pNode) - offsetof(Space, mNode));
        if (pSpace->mHashID == uiHashID)
        {
            return pSpace;
        }
    }
    return 0;
}

void Space::SetReverbPreset(I3DL2ReverbPreset* const pPreset)
{
    if (mReverbPreset)
    {
        mReverbPreset = 0;
    }
    if (pPreset)
    {
        mReverbPreset = pPreset;
    }
}

void Space::ConnectSpace(Space* const pSpace, float fGainFactor)
{
    if (mConnected.FindByObject(pSpace))
    {
        return;
    }

    Connected* const pConnected = (Connected*)Connected::smFreeList.Alloc();
    if (pConnected)
    {
        pConnected->mSpace = pSpace;
        pConnected->mGainFactor = fGainFactor;
        mConnected.Add(pConnected);
    }
}

// FUNCTION: SWBF2 0x00733a90
void Space::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uiID = child._pFile->Read32();
        switch (uiID)
        {
        case 0x94E4E309u:
            child._pFile->Read8();
            mPriority = child._pFile->ReadFloat();
            break;
        case 0x8D39BDE6u:
            child._pFile->Read8();
            mHashID = child._pFile->Read32();
            break;
        case 0xABD36816u:
            child._pFile->Read8();
            SetReverbPreset(I3DL2ReverbPreset::Find(child._pFile->Read32()));
            break;
        case 0x148F0D58u:
        {
            PblFileChunk list = chunk.ReadNextChild();
            while (list.HasMoreChildren())
            {
                PblFileChunk entry = list.ReadNextChild();
                if (entry._pFile->Read32() != 0x3553E285u)
                {
                    continue;
                }
                entry._pFile->Read8();
                const unsigned int uiHashID = entry._pFile->Read32();
                const float fGainFactor = entry._pFile->ReadFloat();
                Space* const pOther = Find(uiHashID);
                if (pOther)
                {
                    ConnectSpace(pOther, fGainFactor);
                    pOther->ConnectSpace(this, fGainFactor);
                }
            }
            break;
        }
        default:
            SourceI3DL2::Read(child, uiID);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00733920
void __cdecl Space::Load(PblFileChunk& chunk, bool bOnce)
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

        Space* const pSpace = new Space(true);
        pSpace->Read(child);
        if (pSpace->mHashID == smDefaultSpace.mHashID)
        {
            smDefaultSpace.SetReverbPreset((I3DL2ReverbPreset*)pSpace->mReverbPreset);
            smDefaultSpace.CopyParameters(*pSpace);
            delete pSpace;
        }
        if (bOnce)
        {
            break;
        }
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x00733620
void Space::Close()
{
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00733630
void __cdecl Space::DestroyAll()
{
    for (PblListSingle::Node* pNode = smSpaceList.mHead.mNext; pNode != &smSpaceList.mHead;)
    {
        Space* const pSpace = (Space*)((char*)pNode - offsetof(Space, mNode));
        pNode = pNode->mNext;
        delete pSpace;
    }
    smDefaultSpace.mReverbPreset = 0;
}
}
