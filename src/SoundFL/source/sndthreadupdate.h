#pragma once

#include "win/sndsemaphore.h"
#include "win/sndthread.h"

namespace Snd
{

class ThreadUpdate
{
public:

    struct Node
    {
        Node* mNext;
    };

    class Object
    {
    public:
        Object();
        virtual ~Object();

        virtual void UpdateObject() = 0;

        ThreadUpdate* GetThreadUpdate() const;

        Node mNode;
        ThreadUpdate* mUpdate;
        unsigned char mDestructing;
    };

    ThreadUpdate(unsigned int uiPriority, unsigned int uiStackSize);
    ~ThreadUpdate();

    void AddObject(Object* pObject);
    void RemoveObject(Object* pObject);
    void Exit();

    void Pause(bool bDeferred);
    void Resume();
    void SetSleepTimeMs(unsigned int uiSleepTimeMs);

private:

    static void __cdecl Complete(ThreadUpdate* pThis, void* pData);
    static void __cdecl UpdateObjects(void* pArg);

public:
    Thread mThread;
    Node mUpdateList;
    Semaphore mAccess;
    unsigned int mSleepTime;
    bool mUpdating;
    bool mStopUpdating;
    bool mPausing;
    void(__cdecl* mCompleteCallback)(ThreadUpdate*, void*);
    void* mCompleteCallbackData;
};

 
 

}
