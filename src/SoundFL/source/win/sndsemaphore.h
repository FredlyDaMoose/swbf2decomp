#pragma once

#include "SoundFL/source/sndsemaphorebase.h"

namespace Snd
{

class Semaphore : public SemaphoreBase
{
public:

    Semaphore()
        : SemaphoreBase()
    {
        mSemaphore = 0;
    }

    Semaphore(unsigned int uiInitialCount, unsigned int uiMaximumCount);

    ~Semaphore();

    bool Create(unsigned int uiInitialCount, unsigned int uiMaximumCount);
    void Destroy();
    bool IsValid();
    void Signal();
    void Wait();

    void* mSemaphore;
};
 

}
