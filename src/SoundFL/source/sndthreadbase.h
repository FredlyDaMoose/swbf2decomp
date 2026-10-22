#pragma once

namespace Snd
{

class ThreadBase
{
public:

    ThreadBase()
    {
        mThreadFunc = 0;
        mArgs = 0;
        mPriority = 0;
        mStackSize = 0;
    }

    void Init(void(__cdecl* pfnThreadFunc)(void*), unsigned int uiPriority, unsigned int uiStackSize)
    {
        mThreadFunc = pfnThreadFunc;
        mPriority = uiPriority;
        mStackSize = uiStackSize;
    }

    bool Start(void* pArgs);

protected:
    static void __cdecl ThreadEntry(void* pArg);

public:
    void(__cdecl* mThreadFunc)(void*);
    void* mArgs;
    unsigned int mPriority;
    unsigned int mStackSize;
};

}
