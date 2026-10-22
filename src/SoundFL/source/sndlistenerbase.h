#pragma once

#include "PebbleFL/Common/PblVector.h"
#include "sndglobals.h"

namespace Snd
{

class Space;
class I3DL2ReverbPreset;

class ListenerBase
{
public:

    ListenerBase();
    ~ListenerBase();

    enum Flags
    {
        flags_enabled = 1,
        flags_spaceChanged = 2,
        flags_reverbChanged = 4,
        flags_reverbUpdate = 8
    };

    static ListenerBase* __cdecl GetClosestListener(const ::PblVector3* pPos,
                                                    ::PblVector3* pDistance,
                                                    float* pfDistanceLength);

    static unsigned int smActiveListeners;

    bool Enabled() const { return (mFlags & flags_enabled) != 0; }

    const Space* GetSpace() const { return mSpace; }
    void Enable(bool bEnable)
    {
        if (bEnable)
        {
            if (!Enabled())
            {
                mFlags |= flags_enabled;
                ++smActiveListeners;
            }
        }
        else if (Enabled())
        {
            mFlags = (mFlags & ~flags_enabled) | flags_spaceChanged | flags_reverbChanged;
            SetSpace(0);
            SetI3DL2ReverbPreset(0);
            --smActiveListeners;
        }
    }

    // FUNCTION: SWBF2 0x0044e410
    void SetPositionOrientation(const ::PblVector3* pPos, const ::PblVector3* pFront,
                                const ::PblVector3* pUp, const ::PblVector3* pRight)
    {
        const ::PblVector3* pP = pPos ? pPos : &Globals::smNullVector;
        mPos = const_cast< ::PblVector3*>(pP);
        mSpacePos = const_cast< ::PblVector3*>(pP);
        const ::PblVector3* pF = pFront ? pFront : &Globals::smFrontVector;
        mFront = const_cast< ::PblVector3*>(pF);
        const ::PblVector3* pU = pUp ? pUp : &Globals::smUpVector;
        mUp = const_cast< ::PblVector3*>(pU);
        ::PblVector3 vRight;
        if (pRight)
        {
            vRight = *pRight;
        }
        else
        {
            vRight.x = pU->y * pF->z - pU->z * pF->y;
            vRight.y = pU->z * pF->x - pU->x * pF->z;
            vRight.z = pU->x * pF->y - pU->y * pF->x;
        }
        mRight = vRight;
    }

    void SetPositonSpace(const ::PblVector3* pPos)
    {
        mSpacePos = const_cast< ::PblVector3*>(pPos ? pPos : &Globals::smNullVector);
    }
    void SetVelocity(const ::PblVector3* pVel)
    {
        mVel = const_cast< ::PblVector3*>(pVel ? pVel : &Globals::smNullVector);
    }

    void SetSpace(const Space* pSpace);

    void Update();

    bool GetSpaceChanged() const { return (mFlags & flags_spaceChanged) != 0; }
    bool GetReverbPresetChanged() const { return (mFlags & flags_reverbChanged) != 0; }
    bool GetReverbEnable() const { return (mFlags & flags_reverbUpdate) != 0; }

private:

    void SetI3DL2ReverbPreset(const I3DL2ReverbPreset* pReverb);

public:

    unsigned int mFlags;
    ::PblVector3* mPos;
    ::PblVector3* mSpacePos;
    ::PblVector3* mFront;
    ::PblVector3* mUp;
    ::PblVector3* mVel;
    float mDopplerFactor;

    void SetDopplerFactor(float fFactor) { mDopplerFactor = fFactor; }
    float mRollOffFactor;
    float mDistanceFactor;
    ::PblVector3 mRight;
    Space* mSpace;
    I3DL2ReverbPreset* mReverb;
};

 

}
