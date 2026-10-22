#include <math.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblVector.h"
#include "sndglobals.h"
#include "sndlistenerbase.h"
#include "sndsourcei3dl2.h"
#include "sndspace.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00748080
SourceI3DL2::SourceI3DL2()
{
    mSpace = 0;
    mDirectGain = 1.0f;
    mDirectHFGain = 1.0f;
    mRoomGain = 0.0f;
    mRoomHFGain = 0.0f;
    mRoomRollOffFactor = 1.0f;
    mObstructionHFGain = 1.0f;
    mObstructionLFRatio = 1.0f;
    mOcclusionHFGain = 1.0f;
    mOcclusionLFRatio = 1.0f;
    mListenerSpace = 0;

    mFlags = (unsigned short)flags_changed;
}

// FUNCTION: SWBF2 0x007480f0
void SourceI3DL2::CopyParameters(const SourceI3DL2& rSource)
{
    mDirectGain = rSource.mDirectGain;
    mDirectHFGain = rSource.mDirectHFGain;
    mRoomGain = rSource.mRoomGain;
    mRoomHFGain = rSource.mRoomHFGain;
    mRoomRollOffFactor = rSource.mRoomRollOffFactor;
    mObstructionHFGain = rSource.mObstructionHFGain;
    mObstructionLFRatio = rSource.mObstructionLFRatio;
    mOcclusionHFGain = rSource.mOcclusionHFGain;
    mOcclusionLFRatio = rSource.mOcclusionLFRatio;
    mFlags |= flags_changed;
}

// FUNCTION: SWBF2 0x00748140
bool SourceI3DL2::Read(PblFileChunk& chunk, unsigned int uiPropertyID)
{
    PblFile* const pFile = chunk._pFile;
    switch (uiPropertyID)
    {
    case 0x116C5DFFu:
        pFile->Read8();
        mDirectGain = Clamp(pFile->ReadFloat(), 0.0f, 3.16f);
        mFlags |= flags_directGain;
        return true;
    case 0x74C54AE1u:
        pFile->Read8();
        mDirectHFGain = Clamp(pFile->ReadFloat(), 0.0f, 1.0f);
        mFlags |= flags_directHFGain;
        return true;
    case 0x5676AB7Du:
        pFile->Read8();
        mRoomGain = Clamp(pFile->ReadFloat(), 0.0f, 3.16f);
        mFlags |= flags_roomGain;
        return true;
    case 0x89695A93u:
        pFile->Read8();
        mRoomHFGain = Clamp(pFile->ReadFloat(), 0.0f, 1.0f);
        mFlags |= flags_roomHFGain;
        return true;
    case 0xDFF9620Du:
        pFile->Read8();
        mRoomRollOffFactor = Clamp(pFile->ReadFloat(), 0.0f, 10.0f);
        mFlags |= flags_roomRollOffFactor;
        return true;
    case 0x0DB31274u:
        pFile->Read8();
        mObstructionHFGain = Clamp(pFile->ReadFloat(), 0.0f, 1.0f);
        mFlags |= flags_obstructionHFGain;
        return true;
    case 0xA0A3A78Au:
        pFile->Read8();
        mObstructionLFRatio = Clamp(pFile->ReadFloat(), 0.0f, 1.0f);
        mFlags |= flags_obstructionLFRatio;
        return true;
    case 0x7C8A1B8Bu:
        pFile->Read8();
        mOcclusionHFGain = Clamp(pFile->ReadFloat(), 0.0f, 1.0f);
        mFlags |= flags_occlusionHFGain;
        return true;
    case 0xA96B06AFu:
        pFile->Read8();
        mOcclusionLFRatio = Clamp(pFile->ReadFloat(), 0.0f, 1.0f);
        mFlags |= flags_occlusionLFRatio;
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x007480e0
float SourceI3DL2::CalculateGainEstimate() const
{
    return (mOcclusionHFGain + mObstructionHFGain) * 0.5f;
}

// FUNCTION: SWBF2 0x00748440
bool SourceI3DL2::Update(const ListenerBase* const pListener,
                         const ::PblVector3* const pPos,
                         float fRadius,
                         bool bSetup,
                         bool* const pbInsideSpace)
{
    Space* const pListenerSpace = pListener->mSpace;
    const bool bWasInside = (mFlags & flags_InsideSpace) != 0;
    Space* pSpace;

    if (pListenerSpace == &Space::smDefaultSpace)
    {
        pSpace = Space::GetSpace(*pPos);
        if (pListenerSpace != pSpace && fRadius > 0.0f)
        {

            const ::PblVector3* const pListenerPos = pListener->mSpacePos;
            ::PblVector3 vDelta;
            vDelta.x = pListenerPos->x - pPos->x;
            vDelta.y = pListenerPos->y - pPos->y;
            vDelta.z = pListenerPos->z - pPos->z;

            const ::PblVector3 vDir = ::NormalizeEst(vDelta);
            ::PblVector3 vProbe;
            vProbe.x = pPos->x + vDir.x * fRadius;
            vProbe.y = pPos->y + vDir.y * fRadius;
            vProbe.z = pPos->z + vDir.z * fRadius;

            if (!pSpace->IsPointInside(vProbe, 0.0f))
            {
                pSpace = pListenerSpace;
            }
        }
    }
    else if (pListenerSpace->IsPointInside(*pPos, fRadius))
    {
        pSpace = pListenerSpace;
    }
    else
    {
        pSpace = Space::GetSpace(*pPos);
    }

    const bool bInsideSpace = (pListenerSpace == pSpace);
    if (pbInsideSpace)
    {
        *pbInsideSpace = bInsideSpace;
    }

    if (pSpace == mSpace && mListenerSpace == pListenerSpace
        && bWasInside == bInsideSpace && !(mFlags & flags_forceUpdate))
    {
        return false;
    }

    mFlags &= (unsigned short)~flags_forceUpdate;
    if (bInsideSpace)
    {
        mFlags |= (unsigned short)flags_InsideSpace;
    }
    else
    {
        mFlags &= (unsigned short)~flags_InsideSpace;
    }
    mListenerSpace = pListenerSpace;
    mSpace = pSpace;

    if (bSetup)
    {
        pListenerSpace->SetupSource(this, bInsideSpace);
    }
    return true;
}

// FUNCTION: SWBF2 0x00748610
void SourceI3DL2Interpolated::Update(const ListenerBase* const pListener,
                                     const ::PblVector3* const pPos,
                                     float fRadius,
                                     bool bInterpolate)
{
    bool bInsideSpace;
    const bool bChanged = SourceI3DL2::Update(pListener, pPos, fRadius,
                                              !bInterpolate, &bInsideSpace);
    if (bInterpolate)
    {
        if (bChanged)
        {
            pListener->mSpace->SetupSourceInterpolated(this, bInsideSpace);
        }
    }
    else
    {
        mFlags &= (unsigned char)~flags_interpolate;
    }

    if (!(mFlags & flags_interpolate))
    {
        return;
    }

    const float fRate = 0.05f;
    const float fEpsilon = 0.001f;
    float fDelta;

    fDelta = mTarget.mDirectGain - mDirectGain;
    bool bSettled = fEpsilon >= fabs(fDelta);
    mDirectGain += fDelta * fRate;
    SourceI3DL2::mFlags |= (unsigned short)flags_directGain;

    fDelta = mTarget.mDirectHFGain - mDirectHFGain;
    bSettled = bSettled && fEpsilon >= fabs(fDelta);
    mDirectHFGain += fDelta * fRate;
    SourceI3DL2::mFlags |= (unsigned short)flags_directHFGain;

    fDelta = mTarget.mRoomGain - mRoomGain;
    bSettled = bSettled && fEpsilon >= fabs(fDelta);
    mRoomGain += fDelta * fRate;
    SourceI3DL2::mFlags |= (unsigned short)flags_roomGain;

    fDelta = mTarget.mRoomHFGain - mRoomHFGain;
    bSettled = bSettled && fEpsilon >= fabs(fDelta);
    mRoomHFGain += fDelta * fRate;
    SourceI3DL2::mFlags |= (unsigned short)flags_roomHFGain;

    fDelta = mTarget.mRoomRollOffFactor - mRoomRollOffFactor;
    bSettled = bSettled && fEpsilon >= fabs(fDelta);
    mRoomRollOffFactor += fDelta * fRate;
    SourceI3DL2::mFlags |= (unsigned short)flags_roomRollOffFactor;

    fDelta = mTarget.mObstructionHFGain - mObstructionHFGain;
    bSettled = bSettled && fEpsilon >= fabs(fDelta);
    mObstructionHFGain += fDelta * fRate;
    SourceI3DL2::mFlags |= (unsigned short)flags_obstructionHFGain;

    fDelta = mTarget.mObstructionLFRatio - mObstructionLFRatio;
    bSettled = bSettled && fEpsilon >= fabs(fDelta);
    mObstructionLFRatio += fDelta * fRate;
    SourceI3DL2::mFlags |= (unsigned short)flags_obstructionLFRatio;

    fDelta = mTarget.mOcclusionHFGain - mOcclusionHFGain;
    bSettled = bSettled && fEpsilon >= fabs(fDelta);
    mOcclusionHFGain += fDelta * fRate;
    SourceI3DL2::mFlags |= (unsigned short)flags_occlusionHFGain;

    fDelta = mTarget.mOcclusionLFRatio - mOcclusionLFRatio;
    bSettled = bSettled && fEpsilon >= fabs(fDelta);
    mOcclusionLFRatio += fDelta * fRate;
    SourceI3DL2::mFlags |= (unsigned short)flags_occlusionLFRatio;

    if (bSettled)
    {
        mFlags &= (unsigned char)~flags_interpolate;
    }
}

}
