#include <windows.h>
#include <stdlib.h>

#include "sndthreadupdate.h"

#include "sndglobals.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0073c210
ThreadUpdate::Object::Object()
{

    mNode.mNext = 0;
    mUpdate = 0;
    mDestructing = 0;
}

// FUNCTION: SWBF2 0x0073c290
ThreadUpdate::Object::~Object()
{
    if (mUpdate)
    {
        mUpdate->RemoveObject(this);
    }

    Node* pNext = mNode.mNext;
    if (pNext)
    {
        Node* pPrev = &mNode;
        Node* pIt = &mNode;
        do
        {
            pPrev = pIt;
            pIt = pIt->mNext;
        } while (pIt != &mNode);

        pPrev->mNext = pNext;
        mNode.mNext = 0;
    }
}

// SYNTHETIC: SWBF2 0x0073c350
// Snd::ThreadUpdate::Object::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0073c4e0
ThreadUpdate* ThreadUpdate::Object::GetThreadUpdate() const
{
    return mUpdate;
}

// FUNCTION: SWBF2 0x0073c230
ThreadUpdate::ThreadUpdate(unsigned int uiPriority, unsigned int uiStackSize)
{

    mThread.Init(UpdateObjects, uiPriority, uiStackSize);

    mUpdateList.mNext = &mUpdateList;

    mSleepTime = 10;
    mUpdating = false;
    mStopUpdating = false;
    mPausing = false;
    mCompleteCallback = Complete;
    mCompleteCallbackData = 0;
}

// FUNCTION: SWBF2 0x0073c300
ThreadUpdate::~ThreadUpdate()
{
    mUpdateList.mNext = 0;
}

// FUNCTION: SWBF2 0x0073c3e0
void ThreadUpdate::AddObject(Object* pObject)
{
    if (mAccess.IsValid())
    {
        mAccess.Wait();
        mUpdating = false;
        mStopUpdating = false;
    }

    if (mUpdateList.mNext == &mUpdateList)
    {
        mUpdating = false;

        if (!mAccess.Create(0, 1))
        {
            PrintDebugMessage("Unable to create semaphore : out of system resources!");
            exit(0);
        }

        if (!mThread.Start(this))
        {
            PrintDebugMessage("Unable to start thread : out of system resources!");
            exit(0);
        }
    }

    Node* pTail = &mUpdateList;
    Node* pIt = &mUpdateList;
    do
    {
        pTail = pIt;
        pIt = pIt->mNext;
    } while (pIt != &mUpdateList);

    pObject->mNode.mNext = pTail->mNext;
    pTail->mNext = &pObject->mNode;
    pObject->mUpdate = this;

    if (mAccess.IsValid())
    {
        mPausing = false;
        mUpdating = true;
        mAccess.Signal();
    }
}

// FUNCTION: SWBF2 0x0073c530
void ThreadUpdate::RemoveObject(Object* pObject)
{
    if (mAccess.IsValid())
    {
        mAccess.Wait();
        mUpdating = false;
        mStopUpdating = false;
    }

    Node* pNode = &pObject->mNode;
    Node* pPrev = pNode;
    Node* pIt = pNode;
    do
    {
        pPrev = pIt;
        pIt = pIt->mNext;
    } while (pIt != pNode);

    pPrev->mNext = pNode->mNext;
    pNode->mNext = 0;
    pObject->mUpdate = 0;

    if (mUpdateList.mNext != &mUpdateList)
    {
        if (mAccess.IsValid())
        {
            mPausing = false;
            mUpdating = true;
            mAccess.Signal();
        }
        return;
    }

    Semaphore done;
    if (!done.Create(0, 1))
    {
        PrintDebugMessage("Unable to create semaphore out of system resources!");
        exit(0);
    }

    mCompleteCallbackData = &done;

    if (mAccess.IsValid())
    {
        mPausing = false;
        mUpdating = true;
        mAccess.Signal();
    }

    done.Wait();
    done.Destroy();
    mCompleteCallbackData = 0;
}

// FUNCTION: SWBF2 0x0073c4b0
void ThreadUpdate::Exit()
{
    Node* pNode = mUpdateList.mNext;
    while (pNode != &mUpdateList)
    {

        RemoveObject(reinterpret_cast<Object*>(reinterpret_cast<char*>(pNode) - offsetof(Object, mNode)));
        pNode = mUpdateList.mNext;
    }
}

// FUNCTION: SWBF2 0x0073c490
void __cdecl ThreadUpdate::Complete(ThreadUpdate* pThis, void* pData)
{
    (void)pThis;

    Semaphore* pDone = (Semaphore*)pData;
    if (pDone)
    {
        pDone->Signal();
    }
}

// FUNCTION: SWBF2 0x0073c4f0
void ThreadUpdate::Pause(bool bDeferred)
{
    if (!mAccess.IsValid())
    {
        return;
    }

    if (bDeferred)
    {
        mPausing = true;
        mStopUpdating = true;
        return;
    }

    mAccess.Wait();
    mUpdating = false;
    mStopUpdating = false;
}

// FUNCTION: SWBF2 0x0073c670
void ThreadUpdate::Resume()
{
    if (!mAccess.IsValid())
    {
        return;
    }

    mPausing = false;
    mUpdating = true;
    mAccess.Signal();
}

// FUNCTION: SWBF2 0x0073c6a0
void ThreadUpdate::SetSleepTimeMs(unsigned int uiSleepTimeMs)
{
    mSleepTime = uiSleepTimeMs;
}

// FUNCTION: SWBF2 0x0073c6b0
void __cdecl ThreadUpdate::UpdateObjects(void* pArg)
{
    ThreadUpdate* pThis = (ThreadUpdate*)pArg;
    int iObjects = -1;

    do
    {
        if (pThis->mUpdating)
        {
            pThis->mAccess.Wait();

            iObjects = 0;
            for (Node* pNode = pThis->mUpdateList.mNext;
                 pNode != &pThis->mUpdateList;
                 pNode = pNode->mNext)
            {

                Object* pObject = reinterpret_cast<Object*>(reinterpret_cast<char*>(pNode) - offsetof(Object, mNode));
                if (!pObject->mDestructing)
                {
                    pObject->UpdateObject();
                }
                ++iObjects;
            }

            if (pThis->mStopUpdating)
            {
                pThis->mStopUpdating = false;
                pThis->mUpdating = false;
            }

            pThis->mAccess.Signal();
        }

        Sleep(pThis->mSleepTime);
    } while (iObjects != 0);

    if (pThis->mAccess.IsValid())
    {
        pThis->mAccess.Destroy();
    }

    if (pThis->mCompleteCallback)
    {
        pThis->mCompleteCallback(pThis, pThis->mCompleteCallbackData);
    }
}

}
