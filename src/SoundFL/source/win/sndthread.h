#pragma once

#include "SoundFL/source/sndthreadbase.h"

namespace Snd
{

class Thread : public ThreadBase
{
public:

    Thread()
        : ThreadBase()
    {
    }

    bool Start(void* pArgs);
};
 

}
