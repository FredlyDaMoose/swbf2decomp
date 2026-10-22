#include "EventManager_Timer.h"
#include "EventManager_LuaUser.h"
#include "LuaHelper.h"
#include "LuaFL/include/lauxlib.h"
#include "NetGame.h"

namespace EventManager
{

// FUNCTION: SWBF2 0x0051c170
template <>
bool LuaPushItem<Timer>(Timer& timer)
{
    lua_pushlightuserdata(LuaHelper::mState, &timer);
    return true;
}

const char* TimerMatches::GetName()
{
    return "";
}

TimerMatches::TimerMatches(const char* pcName)
    : mTimer(Timer::Find(pcName))
{
}

// FUNCTION: SWBF2 0x0051c190
template <>
TimerMatches LuaGetItem<TimerMatches>(int iIndex)
{
    if (lua_islightuserdata(LuaHelper::mState, iIndex))
    {
        return TimerMatches((const Timer*)lua_topointer(LuaHelper::mState, iIndex));
    }
    return TimerMatches(luaL_checkstring(LuaHelper::mState, iIndex));
}

template <class ITEM, class PARAM>
template <class PREDICATE>
Event<ITEM, PARAM>::Filter<PREDICATE>::Filter(PREDICATE& predicate)
    : mPredicate(predicate)
{
}

// FUNCTION: SWBF2 0x0051c890
template <class ITEM, class PARAM>
template <class PREDICATE>
auto Event<ITEM, PARAM>::GetFilter(PREDICATE& predicate, Callback* pParent)
    -> Filter<PREDICATE>*
{
    if (!pParent)
    {
        pParent = &mRoot;
    }

    for (Node* pNode = pParent->mChild; pNode; pNode = pNode->mSibling)
    {
        if (pNode->GetName() == PREDICATE::GetName() &&
            ((Filter<PREDICATE>*)pNode)->mPredicate == predicate)
        {
            pNode->AddRef();
            return (Filter<PREDICATE>*)pNode;
        }
    }

    Filter<PREDICATE>* pFilter = new Filter<PREDICATE>(predicate);
    pFilter->AttachToParent(pParent);
    return pFilter;
}

// FUNCTION: SWBF2 0x0051c650
template <class ITEM, class PARAM>
LuaCallback<ITEM, PARAM>::LuaCallback(int iRef)
{
    mFunction = -1;
    if (LuaHelper::PushRefProc(iRef))
    {
        mFunction = luaL_ref(LuaHelper::mState, LUA_REGISTRYINDEX);
    }
}

// FUNCTION: SWBF2 0x0051c490
template <class ITEM, class PARAM, class PREDICATE>
int RegisterLuaCallback(lua_State*)
{
    Event<ITEM, PARAM>* pEvent =
        (Event<ITEM, PARAM>*)lua_topointer(LuaHelper::mState, lua_upvalueindex(1));
    if (lua_isfunction(LuaHelper::mState, 1))
    {
        lua_pushvalue(LuaHelper::mState, 1);
        int iRef = luaL_ref(LuaHelper::mState, LUA_REGISTRYINDEX);
        PREDICATE predicate = LuaGetItem<PREDICATE>(2);
        typename Event<ITEM, PARAM>::template Filter<PREDICATE>* pFilter =
            pEvent->GetFilter(predicate);
        LuaCallback<ITEM, PARAM>* pCallback = new LuaCallback<ITEM, PARAM>(iRef);
        pCallback->AttachToParent(pFilter ? (Node*)pFilter : (Node*)&pEvent->mRoot);
        luaL_unref(LuaHelper::mState, LUA_REGISTRYINDEX, iRef);
        pFilter->Release();
        lua_pushlightuserdata(LuaHelper::mState, pCallback);
        pCallback->Release();
        return 1;
    }
    lua_pushnil(LuaHelper::mState);
    return 1;
}

// SYNTHETIC: SWBF2 0x0051c780
// EventManager::LuaCallback<Timer,char>::`scalar deleting destructor'
template <class ITEM, class PARAM>
LuaCallback<ITEM, PARAM>::~LuaCallback()
{
    if (mFunction != -1)
    {
        lua_unref(LuaHelper::mState, mFunction);
    }
}

// FUNCTION: SWBF2 0x0051c6e0
template <class ITEM, class PARAM>
bool LuaCallback<ITEM, PARAM>::operator()(ITEM& item, PARAM& param)
{
    if (mFunction == -1 || !LuaHelper::PushRefProc(mFunction))
    {
        return false;
    }

    if (!LuaPushItem(item))
    {
        lua_pop(LuaHelper::mState, 1);
        return false;
    }

    if (!LuaPushItem(param))
    {
        lua_pop(LuaHelper::mState, 2);
        return false;
    }

    LuaHelper::CallProc(0);
    return true;
}

// SYNTHETIC: SWBF2 0x0051c820
// EventManager::Event<Timer,char>::Filter<EventManager::TimerMatches>::`scalar deleting destructor'

template <class ITEM, class PARAM>
bool Event<ITEM, PARAM>::Callback::operator()(ITEM&, PARAM&)
{
    return true;
}

// TEMPLATE: SWBF2 0x0051c2a0
// EventManager::Event<Timer,char>::Callback::operator()

// FUNCTION: SWBF2 0x0051c770
template <class ITEM, class PARAM>
template <class PREDICATE>
const char* Event<ITEM, PARAM>::Filter<PREDICATE>::GetName()
{
    return PREDICATE::GetName();
}

// FUNCTION: SWBF2 0x0051c740
template <class ITEM, class PARAM>
template <class PREDICATE>
bool Event<ITEM, PARAM>::Filter<PREDICATE>::operator()(ITEM& item, PARAM& param)
{
    return mPredicate(item, param);
}

// FUNCTION: SWBF2 0x0051c610
template <class ITEM, class PARAM>
void Event<ITEM, PARAM>::Unregister(Callback& callback)
{
    Node* pParent = callback.mParent;
    callback.DetachFromParent();
    while (pParent && pParent->mParent && !pParent->mChild)
    {
        Node* pNext = pParent->mParent;
        pParent->DetachFromParent();
        pParent = pNext;
    }
}

// FUNCTION: SWBF2 0x0051c320
template <class ITEM, class PARAM>
int UnregisterLuaCallback(lua_State*)
{
    Event<ITEM, PARAM>* pEvent =
        (Event<ITEM, PARAM>*)lua_topointer(LuaHelper::mState, lua_upvalueindex(1));
    if (lua_islightuserdata(LuaHelper::mState, 1))
    {
        typename Event<ITEM, PARAM>::Callback* pCallback =
            (typename Event<ITEM, PARAM>::Callback*)lua_topointer(LuaHelper::mState, 1);
        if (pCallback)
        {
            pEvent->Unregister(*pCallback);
        }
    }
    return 0;
}

template class LuaCallback<Timer, char>;
template void Event<Timer, char>::Unregister(Callback&);
template int UnregisterLuaCallback<Timer, char>(lua_State*);
template Event<Timer, char>::Filter<TimerMatches>*
Event<Timer, char>::GetFilter<TimerMatches>(TimerMatches&, Callback*);
template int RegisterLuaCallback<Timer, char, TimerMatches>(lua_State*);
template class Event<Timer, char>::Filter<TimerMatches>;
template bool Event<Timer, char>::Callback::operator()(Timer&, char&);

}

namespace EventManager {

// FUNCTION: SWBF2 0x0051c240
template Event<Timer, char>::Callback::~Callback();
}

namespace EventManager
{

int DummyLuaCallback(lua_State* L);
namespace TimerEvent { void Init(); }

template <class ITEM, class PARAM>
void RegisterUnregisterLuaCallback(Event<ITEM, PARAM>& event, lua_CFunction pfnClient)
{
    char acName[64];
    _snprintf(acName, sizeof(acName) - 1, "Release%s", event.mName);
    acName[sizeof(acName) - 1] = 0;
    lua_pushlightuserdata(LuaHelper::mState, &event);
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        LuaHelper::Register(acName, pfnClient, 0);
    }
    else
    {
        LuaHelper::Register(acName, UnregisterLuaCallback<ITEM, PARAM>, 1);
    }
}

template <class ITEM, class PARAM>
void RegisterRegisterLuaCallback(Event<ITEM, PARAM>& event, lua_CFunction pfnClient)
{
    char acName[64];
    _snprintf(acName, sizeof(acName) - 1, "On%s", event.mName);
    acName[sizeof(acName) - 1] = 0;
    lua_pushlightuserdata(LuaHelper::mState, &event);
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        LuaHelper::Register(acName, pfnClient, 0);
    }
    else
    {
        LuaHelper::Register(acName, RegisterLuaCallback<ITEM, PARAM>, 1);
    }
}

template <class ITEM, class PARAM, class PREDICATE>
void RegisterRegisterLuaCallback(Event<ITEM, PARAM>& event, lua_CFunction pfnClient)
{
    char acName[64];
    _snprintf(acName, sizeof(acName) - 1, "On%s%s", event.mName, PREDICATE::GetName());
    acName[sizeof(acName) - 1] = 0;
    lua_pushlightuserdata(LuaHelper::mState, &event);
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        LuaHelper::Register(acName, pfnClient, 0);
    }
    else
    {
        LuaHelper::Register(acName, RegisterLuaCallback<ITEM, PARAM, PREDICATE>, 1);
    }
}

void TimerEvent::Init()
{
    mElapsed.Init();
    RegisterUnregisterLuaCallback(mElapsed, DummyLuaCallback);
    RegisterRegisterLuaCallback<Timer, char, TimerMatches>(mElapsed, DummyLuaCallback);
    Timer::sMemoryPool.Setup(64, Timer::sMemoryPool.GetSize(), Timer::sMemoryPool.GetGrow());
}

// TEMPLATE: SWBF2 0x0051c370
// EventManager::RegisterUnregisterLuaCallback<Timer,char>

// TEMPLATE: SWBF2 0x0051c400
// EventManager::RegisterRegisterLuaCallback<Timer,char,EventManager::TimerMatches>

}

namespace EventManager
{

void TimerEvent::Cleanup()
{
    mElapsed.Cleanup();
    Timer::Clear();
}
}

namespace EventManager
{
namespace TimerEvent
{

// GLOBAL: SWBF2 0x007ec120
// SYNTHETIC: SWBF2 0x004037a0
// `dynamic initializer for 'EventManager::TimerEvent::mElapsed''
// SYNTHETIC: SWBF2 0x007686a0
// `dynamic atexit destructor for 'EventManager::TimerEvent::mElapsed''
Event<Timer, char> mElapsed("TimerElapse");

}
}

// SYNTHETIC: SWBF2 0x0051c2b0
// EventManager::Event<Timer,char>::Callback::`scalar deleting destructor'
