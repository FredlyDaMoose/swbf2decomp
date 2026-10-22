#include <windows.h>

#include "sndthread.h"

namespace Snd
{

// FUNCTION: SWBF2 0x007451d0
bool Thread::Start(void* pArgs)
{
    DWORD id;

    mArgs = pArgs;

    HANDLE hThread = CreateThread(0, mStackSize, (LPTHREAD_START_ROUTINE)ThreadEntry, this, 0, &id);
    if (!hThread)
    {
        return false;
    }

    unsigned int uiPriority = mPriority;
    if (uiPriority > 0xFFFFFFFB)
    {
        uiPriority = 0xFFFFFFFB;
    }

    switch (uiPriority / 0x24924925)
    {
    case 0:
        SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
        break;
    case 1:
        SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
        break;
    case 2:
        SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
        break;
    case 3:
        SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
        break;
    case 4:
        SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
        break;
    case 5:
        SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
        break;
    default:
        SetThreadPriority(hThread, THREAD_PRIORITY_IDLE);
        break;
    }

    return true;
}

// FUNCTION: SWBF2 0x007452d0
void __cdecl ThreadBase::ThreadEntry(void* pArg)
{
    ThreadBase* pThread = (ThreadBase*)pArg;
    pThread->mThreadFunc(pThread->mArgs);
}

}
