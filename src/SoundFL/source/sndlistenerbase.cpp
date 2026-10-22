#include <float.h>
#include <math.h>

#include "sndlistenerbase.h"
#include "sndspace.h"
#include "sndenginebase.h"
#include "win/sndlistener.h"

namespace Snd
{

unsigned int ListenerBase::smActiveListeners;

// FUNCTION: SWBF2 0x007314c0
ListenerBase* ListenerBase::GetClosestListener(const ::PblVector3* pPos,
                                               ::PblVector3* pDistance,
                                               float* pfDistanceLength)
{
    ListenerBase* pClosest = 0;

    float fDistanceLength = FLT_MAX;
    ::PblVector3 vDistance;

    if (smActiveListeners > 1)
    {
        for (unsigned int i = 0; i < 4; ++i)
        {
            ListenerBase& rListener = EngineBase::smListeners[i];
            if (!(rListener.mFlags & flags_enabled))
            {
                continue;
            }

            ::PblVector3 v;
            v.x = pPos->x - rListener.mPos->x;
            v.y = pPos->y - rListener.mPos->y;
            v.z = pPos->z - rListener.mPos->z;

            float fLength2 = v.x * v.x + v.y * v.y + v.z * v.z;

            if (fDistanceLength > fLength2)
            {
                fDistanceLength = fLength2;
                pClosest = &rListener;
                vDistance = v;
            }
        }
    }

    if (pClosest == 0)
    {
        for (unsigned int i = 0; i < 4; ++i)
        {
            ListenerBase& rListener = EngineBase::smListeners[i];
            if (rListener.mFlags & flags_enabled)
            {
                pClosest = &rListener;
                vDistance.x = pPos->x - rListener.mPos->x;
                vDistance.y = pPos->y - rListener.mPos->y;
                vDistance.z = pPos->z - rListener.mPos->z;
                fDistanceLength = vDistance.x * vDistance.x
                                + vDistance.y * vDistance.y
                                + vDistance.z * vDistance.z;
                break;
            }
        }
    }

    if (pClosest != 0 && pfDistanceLength != 0)
    {
        fDistanceLength = (float)sqrt(fDistanceLength);
    }

    if (pDistance != 0)
    {
        *pDistance = vDistance;
    }

    if (pfDistanceLength != 0)
    {
        *pfDistanceLength = fDistanceLength;
    }

    return pClosest;
}

// FUNCTION: SWBF2 0x00731750
void ListenerBase::SetI3DL2ReverbPreset(const I3DL2ReverbPreset* pReverb)
{
    if (pReverb != mReverb)
    {
        if (pReverb)
        {
            mFlags |= flags_reverbChanged;
        }
        mReverb = const_cast<I3DL2ReverbPreset*>(pReverb);
    }
}

// FUNCTION: SWBF2 0x00731770
void ListenerBase::SetSpace(const Space* pSpace)
{
    if (pSpace != mSpace)
    {
        if (pSpace)
        {
            mFlags |= flags_spaceChanged;
        }
        else
        {
            pSpace = &Space::smDefaultSpace;
        }
        mSpace = const_cast<Space*>(pSpace);
        if (mSpace)
        {
            SetI3DL2ReverbPreset((const I3DL2ReverbPreset*)mSpace->mReverbPreset);
        }
    }
}

// FUNCTION: SWBF2 0x007317b0
void ListenerBase::Update()
{
    mFlags &= ~(flags_spaceChanged | flags_reverbChanged);

    Space* const pSpace = Space::GetSpace(*mSpacePos);
    if (mSpace == 0 || pSpace == &Space::smDefaultSpace
        || !mSpace->IsPointInside(*mSpacePos, 0.0f)
        || pSpace->mPriority > mSpace->mPriority)
    {
        SetSpace(pSpace);
    }
}

}
