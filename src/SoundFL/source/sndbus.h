#pragma once

#include "PebbleFL/Common/PblListSingle.h"
#include "sndfader.h"
#include "sndfreelist.h"

class PblFileChunk;

namespace Snd
{

class Bus : public Fader
{
public:

    enum Flags
    {
        flags_Solo = 1,
        flags_Mute = 2
    };

    Bus(Bus* pParentBus, unsigned int uiHashID)
        : mParentBus(pParentBus),
          mGain(1.0f),
          mHashID(uiHashID),
          mAttenuateBus(0),
          mAttenuation(0.0f),
          mAttenuationLeadInTime(1.0f),
          mAttenuationLeadOutTime(1.0f),
          mAttenuationRefCount(0),
          mFlags(0)
    {
        if (pParentBus)
        {
            pParentBus->mSubBusList.AddLast(&mNode);
        }
        UpdateFinalGain();
    }

    static void* operator new(unsigned int)
    {
        return smFreeList.Alloc();
    }

    static void operator delete(void* p)
    {
        smFreeList.Free(p);
    }

    ~Bus();

    static Bus smRootBus;

    static FreeList smFreeList;

    static bool smBusSolo;

    static void __cdecl Load(PblFileChunk& chunk, bool bOnce);
    static void __cdecl MuteAll(Bus* pBus);
    void Read(PblFileChunk& chunk);
    Bus* AddSubBus(unsigned int uiHashID);
    Bus* FindSubBusByHashID(unsigned int uiHashID);

    void SetGain(float fGain, bool bStopFade);

    static Bus* FindByHashID(unsigned int uiHashID);

    void DestroySubBuses();

    static void UpdateAll(float fDeltaTime)
    {
        smRootBus.Update(fDeltaTime, false);
    }

protected:
    void Update(float fDeltaTime, bool bForce);

    void UpdateFinalGain()
    {
        mFinalGain = mGain * Fader::mGain;
        if (mParentBus)
        {
            mFinalGain = mParentBus->mFinalGain * mFinalGain;
        }
    }

public:
    PblListSingle::Node mNode;
    Bus* mParentBus;
    PblListSingle mSubBusList;
    float mGain;
    float mFinalGain;
    unsigned int mHashID;
    Bus* mAttenuateBus;
    float mAttenuation;
    float mAttenuationLeadInTime;
    float mAttenuationLeadOutTime;
    unsigned int mAttenuationRefCount;
    unsigned char mFlags;
    bool _mDisableAttenuationCheck;
};

 
}
