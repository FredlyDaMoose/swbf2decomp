#include <stdio.h>

#include "EventManager_LuaUser.h"
#include "EventManager_Character.h"
#include "NetGame.h"

#include "EventManager_Object.h"

namespace EventManager
{

// FUNCTION: SWBF2 0x005139f0
template <>
bool LuaPushItem<LuaObject>(LuaObject& item)
{
    item.Push();
    return true;
}

// FUNCTION: SWBF2 0x00513a10
template <>
LuaObjectMatches LuaGetItem<LuaObjectMatches>(int iIndex)
{
    return LuaObjectMatches(LuaHelper::mState, iIndex);
}

void LuaObject::Push() const
{
    lua_rawgeti(LuaHelper::mState, LUA_REGISTRYINDEX, mRef);
}

namespace LuaUserEvent
{

// FUNCTION: SWBF2 0x00513a70
static int Trigger(lua_State*)
{
    Event<LuaObject, LuaObject>* pEvent =
        (Event<LuaObject, LuaObject>*)lua_topointer(LuaHelper::mState, lua_upvalueindex(1));
    LuaObject item(LuaHelper::mState, 1);
    LuaObject param(LuaHelper::mState, 2);
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        pEvent->Trigger(item, param);
    }
    return 0;
}
}

namespace LuaUserEvent
{

// FUNCTION: SWBF2 0x00513b70
int Create(lua_State*)
{
    const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
    Event<LuaObject, LuaObject>* pEvent = new Event<LuaObject, LuaObject>((char*)pcName);
    RegisterUnregisterLuaCallback(*pEvent, DummyLuaCallback);
    RegisterRegisterLuaCallback(*pEvent, DummyLuaCallback);
    RegisterRegisterLuaCallback<LuaObject, LuaObject, LuaObjectMatches>(*pEvent, DummyLuaCallback);

    char acName[64];
    _snprintf(acName, sizeof(acName) - 1, "Trigger%s", pcName);
    acName[sizeof(acName) - 1] = 0;
    lua_pushlightuserdata(LuaHelper::mState, pEvent);
    LuaHelper::Register(acName, EventManager::LuaUserEvent::Trigger, 1);
    return 0;
}
}

const char* LuaObjectMatches::GetName()
{
    return "Match";
}

// FUNCTION: SWBF2 0x00513ca0
template <class ITEM, class PARAM>
bool Event<ITEM, PARAM>::Callback::operator()(ITEM&, PARAM&)
{
    return true;
}

// SYNTHETIC: SWBF2 0x00513cb0
// EventManager::Event<EventManager::LuaObject,EventManager::LuaObject>::Callback::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00513d20
template <class ITEM, class PARAM>
void Event<ITEM, PARAM>::Callback::Trigger(ITEM& item, PARAM& param)
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

// FUNCTION: SWBF2 0x005142f0
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

template <class ITEM, class PARAM>
template <class PREDICATE>
Event<ITEM, PARAM>::Filter<PREDICATE>::Filter(PREDICATE& predicate)
    : mPredicate(predicate)
{
}

// FUNCTION: SWBF2 0x005145d0
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

// FUNCTION: SWBF2 0x00514330
template <class ITEM, class PARAM>
LuaCallback<ITEM, PARAM>::LuaCallback(int iRef)
{
    mFunction = -1;
    if (LuaHelper::PushRefProc(iRef))
    {
        mFunction = luaL_ref(LuaHelper::mState, LUA_REGISTRYINDEX);
    }
}

// SYNTHETIC: SWBF2 0x005144a0
// EventManager::LuaCallback<EventManager::LuaObject,EventManager::LuaObject>::`scalar deleting destructor'
template <class ITEM, class PARAM>
LuaCallback<ITEM, PARAM>::~LuaCallback()
{
    if (mFunction != -1)
    {
        lua_unref(LuaHelper::mState, mFunction);
    }
}

// FUNCTION: SWBF2 0x005143c0
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

// FUNCTION: SWBF2 0x00514490
template <class ITEM, class PARAM>
template <class PREDICATE>
const char* Event<ITEM, PARAM>::Filter<PREDICATE>::GetName()
{
    return PREDICATE::GetName();
}

// FUNCTION: SWBF2 0x00514430
template <class ITEM, class PARAM>
template <class PREDICATE>
bool Event<ITEM, PARAM>::Filter<PREDICATE>::operator()(ITEM& item, PARAM& param)
{
    return mPredicate(item, param);
}

// SYNTHETIC: SWBF2 0x00514540
// EventManager::Event<EventManager::LuaObject,EventManager::LuaObject>::Filter<EventManager::LuaObjectMatches>::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00513e40
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

// FUNCTION: SWBF2 0x00513fb0
template <class ITEM, class PARAM>
int RegisterLuaCallback(lua_State*)
{
    Event<ITEM, PARAM>* pEvent =
        (Event<ITEM, PARAM>*)lua_topointer(LuaHelper::mState, lua_upvalueindex(1));
    if (lua_isfunction(LuaHelper::mState, 1))
    {
        lua_pushvalue(LuaHelper::mState, 1);
        int iRef = luaL_ref(LuaHelper::mState, LUA_REGISTRYINDEX);
        LuaCallback<ITEM, PARAM>* pCallback = new LuaCallback<ITEM, PARAM>(iRef);
        pCallback->AttachToParent(&pEvent->mRoot);
        luaL_unref(LuaHelper::mState, LUA_REGISTRYINDEX, iRef);
        lua_pushlightuserdata(LuaHelper::mState, pCallback);
        pCallback->Release();
        return 1;
    }
    lua_pushnil(LuaHelper::mState);
    return 1;
}

// FUNCTION: SWBF2 0x00514160
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

// FUNCTION: SWBF2 0x00513e90
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

// FUNCTION: SWBF2 0x00513f20
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

// FUNCTION: SWBF2 0x005140d0
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

template class Event<LuaObject, LuaObject>::Callback;
template class LuaCallback<LuaObject, LuaObject>;
template class Event<LuaObject, LuaObject>::Filter<LuaObjectMatches>;
template void Event<LuaObject, LuaObject>::Unregister(Callback&);
template Event<LuaObject, LuaObject>::Filter<LuaObjectMatches>*
Event<LuaObject, LuaObject>::GetFilter<LuaObjectMatches>(LuaObjectMatches&, Callback*);
template void RegisterUnregisterLuaCallback<LuaObject, LuaObject>(Event<LuaObject, LuaObject>&, lua_CFunction);
template void RegisterRegisterLuaCallback<LuaObject, LuaObject>(Event<LuaObject, LuaObject>&, lua_CFunction);
template void RegisterRegisterLuaCallback<LuaObject, LuaObject, LuaObjectMatches>(Event<LuaObject, LuaObject>&, lua_CFunction);

}

namespace EventManager {

// FUNCTION: SWBF2 0x00513c40
template Event<LuaObject, LuaObject>::Callback::~Callback();
}

namespace EventManager
{

int DummyLuaCallback(lua_State* L);
namespace LuaUserEvent { int Create(lua_State* L); void Init(); }

// FUNCTION: SWBF2 0x00513c20
void LuaUserEvent::Init()
{
    LuaHelper::Register("CreateUserEvent", Create, 0);
}

}

namespace EventManager
{
// TEMPLATE: SWBF2 0x00515080
// EventManager::Event<GameObject,int>::Callback::operator()
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x00517e60
// EventManager::Event<GameObject,DamageOwner>::Unregister
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x005160b0
// EventManager::RegisterLuaCallback<GameObject,DamageOwner,EventManager::ObjectMatches<DamageOwner> >
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x005162c0
// EventManager::RegisterLuaCallback<GameObject,DamageOwner,EventManager::ObjectMatchesName<DamageOwner> >
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x005164d0
// EventManager::RegisterLuaCallback<GameObject,DamageOwner,EventManager::ObjectMatchesTeam<DamageOwner> >
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x005166e0
// EventManager::RegisterLuaCallback<GameObject,DamageOwner,EventManager::ObjectMatchesClass<DamageOwner> >
}
