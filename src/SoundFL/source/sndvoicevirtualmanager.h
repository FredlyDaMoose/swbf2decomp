#pragma once

#include "PebbleFL/Common/PblListDoubleSorted.h"
#include "PebbleFL/Common/PblListSingle.h"

namespace Snd
{

class VoiceBase;
class VoiceVirtual;

class VoiceVirtualManager
{
public:

    VoiceVirtualManager()
        : mActiveList(true), mInactiveList(false), mNumManagedVoices(0)
    {
    }

    PblListDoubleSorted mActiveList;
    PblListDoubleSorted mInactiveList;
    PblListSingle mVoiceList;
    unsigned int mNumManagedVoices;

    void Update(float fDeltaTime);

private:

    void InsertVoice(VoiceVirtual* pVoice);
    void RemoveVoice(VoiceVirtual* pVoice);

    VoiceBase* GetFreeVoice();
    friend class VoiceVirtual;
};

 

}
