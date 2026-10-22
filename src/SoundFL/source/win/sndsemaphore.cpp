#include <windows.h>

#include "sndsemaphore.h"

namespace Snd
{

// FUNCTION: SWBF2 0x007450e0
Semaphore::~Semaphore()
{
    if (IsValid())
    {
        Destroy();
    }
}

// FUNCTION: SWBF2 0x00745110
bool Semaphore::Create(unsigned int uiInitialCount, unsigned int uiMaximumCount)
{
    if (IsValid())
    {
        Destroy();
    }

    if (SemaphoreBase::Create(uiInitialCount, uiMaximumCount))
    {
        mSemaphore = CreateSemaphoreA(0, uiInitialCount, uiMaximumCount, 0);
    }

    return IsValid();
}

// FUNCTION: SWBF2 0x00745170
void Semaphore::Destroy()
{
    CloseHandle(mSemaphore);
    mSemaphore = 0;
    SemaphoreBase::Destroy();
}

// FUNCTION: SWBF2 0x00745190
bool Semaphore::IsValid()
{
    return SemaphoreBase::IsValid() && mSemaphore != 0;
}

// FUNCTION: SWBF2 0x007451b0
void Semaphore::Signal()
{
    ReleaseSemaphore(mSemaphore, 1, 0);
}

// FUNCTION: SWBF2 0x007451c0
void Semaphore::Wait()
{
    WaitForSingleObject(mSemaphore, INFINITE);
}

}
