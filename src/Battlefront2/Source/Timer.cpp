#include "Timer.h"
#include "NetGame.h"
#include "EventManager_Timer.h"
#include "HUDGameEvents.h"
#include "StringDB.h"

namespace EventManager
{
template <>
void Event<Timer, char>::Callback::Trigger(Timer& item, char& param);
}

// GLOBAL: SWBF2 0x01f7ced8
int Timer::sMapNumEntries;
// GLOBAL: SWBF2 0x01f7cedc
// SYNTHETIC: SWBF2 0x00406aa0
// `dynamic initializer for 'Timer::sMap''

PblHashTable<Timer, 64> Timer::sMap;
// GLOBAL: SWBF2 0x01f99418
// SYNTHETIC: SWBF2 0x00406ac0
// `dynamic initializer for 'Timer::sMemoryPool''
// SYNTHETIC: SWBF2 0x007695f0
// `dynamic atexit destructor for 'Timer::sMemoryPool''
MemoryPool Timer::sMemoryPool("Timer", 0x2c);

// GLOBAL: SWBF2 0x01e51e25
namespace EventManager { static char noContext; }

// FUNCTION: SWBF2 0x00657a80
bool Timer::Update(float fDeltaTime)
{
    if (!NetGame::IsEnabled() || !NetGame::IsPreGame())
    {
        mTime -= mRate * fDeltaTime;
        if (mTime <= 0.0f)
        {
            DeactivateThread();
            if (!NetGame::IsEnabled() || !NetGame::IsClient())
            {
                EventManager::TimerEvent::mElapsed.Trigger(*this, EventManager::noContext);
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00657b20
void Timer::Start()
{
    if (!IsThreadActive())
    {
        ActivateThread(10, PER_LOCAL_TURN);
    }
}

// FUNCTION: SWBF2 0x00657b40
void Timer::Stop()
{
    if (IsThreadActive())
    {
        DeactivateThread();
    }
}

// FUNCTION: SWBF2 0x00657ba0
Timer* Timer::Find(const char* pcName)
{
    return Timer::sMap.Find(PblHash(pcName));
}

// FUNCTION: SWBF2 0x006579e0
Timer::~Timer()
{
    if (HUD::GameEvents::GetObjectiveTimer() == this)
    {
        HUD::GameEvents::SetObjectiveTimer(0);
    }
    if (sMap.Remove(mId))
    {
        --sMapNumEntries;
    }
    StringDB::Remove(mId);
}

// FUNCTION: SWBF2 0x00657900
Timer::Timer(const char* pcName)
{
    mId = PblHash(pcName);
    mTime = 0.0f;
    mRate = 1.0f;
    StringDB::Store(mId, pcName);
    if (sMap.Store(mId, this))
    {
        ++sMapNumEntries;
    }
}

// SYNTHETIC: SWBF2 0x006579b0
// Timer::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00657ce7
// [thunk]:Timer::`scalar deleting destructor' adjustor{24}

namespace EventManager
{

// FUNCTION: SWBF2 0x00657bd0
template <>
void Event<Timer, char>::Callback::Trigger(Timer& item, char& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild->IsAlive())
            {
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
                if (!pChild)
                {
                    break;
                }
            }
            if (pChild)
            {
                pChild->Release();
            }
        }
    }
    Release();
}
}

void Timer::Clear()
{
    sMap.Clear();
    sMapNumEntries = 0;
}
