#include "Thread.h"
#include "NetGame.h"

extern ThreadGroup sHostFrameThreadList;
extern ThreadGroup sHostTurnThreadList;
extern ThreadGroup sClientFrameThreadList;
extern ThreadGroup sClientTurnThreadList;
extern ThreadGroup sClientLocalTurnThreadList;
extern ThreadGroup* sFrameThreadList;
extern ThreadGroup* sTurnThreadList;
extern bool gThreadIsEven;

namespace FrameUpdate { void Update(); }

// FUNCTION: SWBF2 0x00657400
ThreadGroup::ThreadGroup(int iPriority)
{
    mPriority = iPriority;
    mNext = this;
    mPrev = this;
}

// FUNCTION: SWBF2 0x00657440
ThreadGroup::~ThreadGroup()
{
    Remove();
    mThreads.Clear();
}

// FUNCTION: SWBF2 0x00657460
void ThreadGroup::Clear()
{
    while (mNext != this)
    {
        delete mNext;
    }
    mThreads.Clear();
}

// FUNCTION: SWBF2 0x006574a0
void ThreadGroup::AddBefore(ThreadGroup* pNext)
{
    mNext = pNext;
    mPrev = pNext->mPrev;
    mPrev->mNext = this;
    mNext->mPrev = this;
}

// FUNCTION: SWBF2 0x006574c0
void ThreadGroup::Remove()
{
    mNext->mPrev = mPrev;
    mPrev->mNext = mNext;
}

// FUNCTION: SWBF2 0x006574e0
ThreadGroup* ThreadGroup::FindCreate(int iPriority)
{
    ThreadGroup* pGroup = mNext;
    while (iPriority > pGroup->mPriority)
    {
        pGroup = pGroup->mNext;
    }

    if (iPriority < pGroup->mPriority)
    {
        ThreadGroup* pNew = new ThreadGroup(iPriority);
        pNew->AddBefore(pGroup);
        return pNew;
    }
    return pGroup;
}

// FUNCTION: SWBF2 0x00657540
void ThreadGroup::Update(float fDeltaTime)
{
    Thread::sExecutingThread = mThreads._head._pNext;
    while (Thread* pThread = Thread::sExecutingThread->_pObject)
    {
        Thread::sExecutingThread = Thread::sExecutingThread->_pNext;
        if (!pThread->Update(fDeltaTime))
        {
            delete pThread;
        }
    }
}

// FUNCTION: SWBF2 0x006575a0
void ThreadGroup::UpdateAll(float fDeltaTime)
{
    for (ThreadGroup* pGroup = mNext; pGroup != this; pGroup = pGroup->mNext)
    {
        pGroup->Update(fDeltaTime);
    }
}

// FUNCTION: SWBF2 0x006575e0
Thread::Thread()
{
}

// SYNTHETIC: SWBF2 0x00657610
// Thread::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00657640
Thread::~Thread()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x006576f0
void Thread::ActivateThread(int iOrder, ThreadType eThreadType)
{
    ThreadGroup* pGroup;
    if (eThreadType == PER_FRAME)
    {
        pGroup = sFrameThreadList;
    }
    else if (NetGame::IsEnabled() && sTurnThreadList == &sClientTurnThreadList &&
             eThreadType != PER_TURN)
    {
        pGroup = &sClientLocalTurnThreadList;
    }
    else
    {
        pGroup = sTurnThreadList;
    }

    pGroup = pGroup->FindCreate(-iOrder);
    pGroup->mThreads.Add(&mThreadListNode, this, (int)this);
}

// FUNCTION: SWBF2 0x00657760
void Thread::DeactivateThread()
{
    if (!IsThreadActive())
    {
        return;
    }

    if (sExecutingThread->_pObject != 0 && sExecutingThread->_pObject == this)
    {
        sExecutingThread = sExecutingThread->_pNext;
    }

    mThreadListNode._pList->Remove(&mThreadListNode);
}

// FUNCTION: SWBF2 0x006577b0
void Thread::UpdateAll(float fDeltaTime, bool bFrame, bool bLocalTurn)
{
    ThreadGroup* pLocalGroup = 0;
    ThreadGroup* pGroup;
    if (bFrame)
    {
        pGroup = sFrameThreadList;
    }
    else
    {
        gThreadIsEven = !gThreadIsEven;
        if (NetGame::IsEnabled() && NetGame::IsClient() && bLocalTurn)
        {
            pLocalGroup = &sClientLocalTurnThreadList;
        }
        pGroup = sTurnThreadList;
    }

    if (pGroup || pLocalGroup)
    {
        FrameUpdate::Update();
        if (pGroup)
        {
            pGroup->UpdateAll(fDeltaTime);
            FrameUpdate::Update();
        }
        if (pLocalGroup)
        {
            pLocalGroup->UpdateAll(fDeltaTime);
            FrameUpdate::Update();
        }
    }
}

// FUNCTION: SWBF2 0x00657860
template void PblOrderedList<Thread, int>::Add(PblOrderedList<Thread, int>::Node*, Thread*, int);

// FUNCTION: SWBF2 0x006578c0
template void PblOrderedList<Thread, int>::Clear();

// GLOBAL: SWBF2 0x01f993f0
// SYNTHETIC: SWBF2 0x004069f0
// `dynamic initializer for 'sHostFrameThreadList''
// SYNTHETIC: SWBF2 0x007695a0
// `dynamic atexit destructor for 'sHostFrameThreadList''
ThreadGroup sHostFrameThreadList(0x7FFFFFFF);
// GLOBAL: SWBF2 0x01f99330
// SYNTHETIC: SWBF2 0x00406a10
// `dynamic initializer for 'sHostTurnThreadList''
// SYNTHETIC: SWBF2 0x007695b0
// `dynamic atexit destructor for 'sHostTurnThreadList''
ThreadGroup sHostTurnThreadList(0x7FFFFFFF);
// GLOBAL: SWBF2 0x01f99390
// SYNTHETIC: SWBF2 0x00406a30
// `dynamic initializer for 'sClientFrameThreadList''
// SYNTHETIC: SWBF2 0x007695c0
// `dynamic atexit destructor for 'sClientFrameThreadList''
ThreadGroup sClientFrameThreadList(0x7FFFFFFF);
// GLOBAL: SWBF2 0x01f99360
// SYNTHETIC: SWBF2 0x00406a50
// `dynamic initializer for 'sClientTurnThreadList''
// SYNTHETIC: SWBF2 0x007695d0
// `dynamic atexit destructor for 'sClientTurnThreadList''
ThreadGroup sClientTurnThreadList(0x7FFFFFFF);
// GLOBAL: SWBF2 0x01f993c0
// SYNTHETIC: SWBF2 0x00406a70
// `dynamic initializer for 'sClientLocalTurnThreadList''
// SYNTHETIC: SWBF2 0x007695e0
// `dynamic atexit destructor for 'sClientLocalTurnThreadList''
ThreadGroup sClientLocalTurnThreadList(0x7FFFFFFF);

// GLOBAL: SWBF2 0x007e9ab4
ThreadGroup* sFrameThreadList = &sHostFrameThreadList;
// GLOBAL: SWBF2 0x007e9ab0
ThreadGroup* sTurnThreadList = &sHostTurnThreadList;

// GLOBAL: SWBF2 0x007e9ab9
bool gThreadIsEven = true;
// GLOBAL: SWBF2 0x007e904b
bool sHostThreadIsEven = true;
// GLOBAL: SWBF2 0x007e9ab8
bool sClientThreadIsEven = true;

// GLOBAL: SWBF2 0x01f7ce84
// SYNTHETIC: SWBF2 0x00406a90
// `dynamic initializer for 'Thread::sExecutingThread''
Thread::ThreadListNode* Thread::sExecutingThread = sFrameThreadList->mThreads._head._pNext;

// FUNCTION: SWBF2 0x00657650
void Thread::Reset()
{
    sHostFrameThreadList.Clear();
    sHostTurnThreadList.Clear();
    sClientFrameThreadList.Clear();
    sClientLocalTurnThreadList.Clear();
    sClientTurnThreadList.Clear();
}

// FUNCTION: SWBF2 0x00657690
void Thread::EnterClient()
{
    sHostThreadIsEven = gThreadIsEven;
    sFrameThreadList = &sClientFrameThreadList;
    sTurnThreadList = &sClientTurnThreadList;
    gThreadIsEven = sClientThreadIsEven;
}

// FUNCTION: SWBF2 0x006576c0
void Thread::LeaveClient()
{
    sClientThreadIsEven = gThreadIsEven;
    sFrameThreadList = &sHostFrameThreadList;
    sTurnThreadList = &sHostTurnThreadList;
    gThreadIsEven = sHostThreadIsEven;
}

// FUNCTION: SWBF2 0x00429200
bool Thread::IsThreadActive()
{
    return mThreadListNode._pList != 0;
}
