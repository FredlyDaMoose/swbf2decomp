#pragma once

#include "sndbus.h"

class PblFileChunk;

namespace Snd
{

class SoundProperties
{
public:

    SoundProperties() : mBus(&Bus::smRootBus), mHashID(0), mGroupID(0) {}

    Bus* mBus;
    unsigned int mHashID;
    unsigned int mGroupID;

protected:
    bool Read(PblFileChunk& chunk, unsigned int uiPropertyID);
};

 

}
