#pragma once

namespace Snd
{

class SemaphoreBase
{
public:

    SemaphoreBase()
    {
        mValid = false;
    }

    SemaphoreBase(unsigned int uiInitialCount, unsigned int uiMaximumCount);

    ~SemaphoreBase()
    {
        mValid = false;
    }

    bool Create(unsigned int uiInitialCount, unsigned int uiMaximumCount)
    {
        mValid = uiMaximumCount != 0;
        return mValid;
    }

    void Destroy()
    {
        mValid = false;
    }

    bool IsValid()
    {
        return mValid;
    }

    void Signal()
    {
    }

    void Wait()
    {
    }

    bool mValid;
};

}
