#include <stdio.h>

#include "EventManager_Region.h"
#include "ActiveRegion.h"
#include "Controllable.h"
#include "Entity.h"
#include "NetGame.h"
#include "LuaFL/include/lauxlib.h"

namespace EventManager
{

// FUNCTION: SWBF2 0x00519a90
template <>
bool LuaPushItem<RedRegion>(RedRegion& region)
{
    lua_pushlightuserdata(LuaHelper::mState, &region);
    return true;
}

// FUNCTION: SWBF2 0x00519ab0
template <>
bool LuaPushItem<Character>(Character& character)
{
    lua_pushnumber(LuaHelper::mState, (lua_Number)(&character - Character::sCharacters));
    return true;
}

RegionMatches::RegionMatches(const char* pcName)
    : mRegion(RedRegion::Find(PblHash(pcName)))
{
}

const char* RegionMatches::GetName()
{
    return "";
}

CharacterMatchesName::CharacterMatchesName(const char* pcName)
    : mId(PblHash(pcName))
{
}

CharacterMatchesClass::CharacterMatchesClass(const char* pcName)
    : mClass(EntityClass::Find(PblHash(pcName)))
{
}

bool CharacterMatchesName::operator()(const RedRegion&, const Character& character)
{
    const Controllable* pControllable = character.GetControllable();
    if (!pControllable)
    {
        return false;
    }
    const EntityEx* pEntity = (const EntityEx*)pControllable->GetEntity();
    if (!pEntity)
    {
        return false;
    }
    return pEntity->mId == mId;
}

bool CharacterMatchesClass::operator()(const RedRegion&, const Character& character)
{
    const Controllable* pControllable = character.GetControllable();
    if (!pControllable)
    {
        return false;
    }
    const Entity* pEntity = pControllable->GetEntity();
    if (!pEntity)
    {
        return false;
    }
    return pEntity->GetEntityClass() == mClass;
}

// FUNCTION: SWBF2 0x00519af0
template <>
RegionMatches LuaGetItem<RegionMatches>(int iIndex)
{
    if (lua_islightuserdata(LuaHelper::mState, iIndex))
    {
        return RegionMatches((RedRegion*)lua_topointer(LuaHelper::mState, iIndex));
    }
    return RegionMatches(luaL_checkstring(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x00519b60
template <>
CharacterMatchesName LuaGetItem<CharacterMatchesName>(int iIndex)
{
    return CharacterMatchesName(luaL_checkstring(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x00519b90
template <>
CharacterMatchesTeam LuaGetItem<CharacterMatchesTeam>(int iIndex)
{
    return CharacterMatchesTeam((unsigned int)luaL_checknumber(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x00519bb0
template <>
CharacterMatchesClass LuaGetItem<CharacterMatchesClass>(int iIndex)
{
    return CharacterMatchesClass(luaL_checkstring(LuaHelper::mState, iIndex));
}

// TEMPLATE: SWBF2 0x00519d00
template <class ITEM, class PARAM>
bool Event<ITEM, PARAM>::Callback::operator()(ITEM&, PARAM&)
{
    return true;
}

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

// TEMPLATE: SWBF2 0x0051ac50
// EventManager::Event<RedRegion,Character>::GetFilter<EventManager::RegionMatches>

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

// TEMPLATE: SWBF2 0x0051ad00
// EventManager::Event<RedRegion,Character>::GetFilter<EventManager::CharacterMatchesName>
// TEMPLATE: SWBF2 0x0051adc0
// EventManager::Event<RedRegion,Character>::GetFilter<EventManager::CharacterMatchesTeam>
// TEMPLATE: SWBF2 0x0051ae80
// EventManager::Event<RedRegion,Character>::GetFilter<EventManager::CharacterMatchesClass>

template <class ITEM, class PARAM>
LuaCallback<ITEM, PARAM>::LuaCallback(int iRef)
{
    mFunction = -1;
    if (LuaHelper::PushRefProc(iRef))
    {
        mFunction = luaL_ref(LuaHelper::mState, LUA_REGISTRYINDEX);
    }
}

template <class ITEM, class PARAM>
LuaCallback<ITEM, PARAM>::~LuaCallback()
{
    if (mFunction != -1)
    {
        lua_unref(LuaHelper::mState, mFunction);
    }
}

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

template <class ITEM, class PARAM>
template <class PREDICATE>
const char* Event<ITEM, PARAM>::Filter<PREDICATE>::GetName()
{
    return PREDICATE::GetName();
}

template <class ITEM, class PARAM>
template <class PREDICATE>
bool Event<ITEM, PARAM>::Filter<PREDICATE>::operator()(ITEM& item, PARAM& param)
{
    return mPredicate(item, param);
}

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

template <class ITEM, class PARAM, class P1, class P2>
int RegisterLuaCallback(lua_State*)
{
    Event<ITEM, PARAM>* pEvent =
        (Event<ITEM, PARAM>*)lua_topointer(LuaHelper::mState, lua_upvalueindex(1));
    if (lua_isfunction(LuaHelper::mState, 1))
    {
        lua_pushvalue(LuaHelper::mState, 1);
        int iRef = luaL_ref(LuaHelper::mState, LUA_REGISTRYINDEX);
        P1 predicate1 = LuaGetItem<P1>(2);
        typename Event<ITEM, PARAM>::template Filter<P1>* pFilter1 =
            pEvent->GetFilter(predicate1);
        P2 predicate2 = LuaGetItem<P2>(3);
        typename Event<ITEM, PARAM>::template Filter<P2>* pFilter2 =
            pEvent->GetFilter(predicate2, pFilter1);
        LuaCallback<ITEM, PARAM>* pCallback = new LuaCallback<ITEM, PARAM>(iRef);
        pCallback->AttachToParent(pFilter2 ? (Node*)pFilter2 : (Node*)&pEvent->mRoot);
        luaL_unref(LuaHelper::mState, LUA_REGISTRYINDEX, iRef);
        pFilter1->Release();
        pFilter2->Release();
        lua_pushlightuserdata(LuaHelper::mState, pCallback);
        pCallback->Release();
        return 1;
    }
    lua_pushnil(LuaHelper::mState);
    return 1;
}

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

template <class ITEM, class PARAM, class P1, class P2>
void RegisterRegisterLuaCallback(Event<ITEM, PARAM>& event, lua_CFunction pfnClient)
{
    char acName[64];
    _snprintf(acName, sizeof(acName) - 1, "On%s%s%s", event.mName, P1::GetName(), P2::GetName());
    acName[sizeof(acName) - 1] = 0;
    lua_pushlightuserdata(LuaHelper::mState, &event);
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        LuaHelper::Register(acName, pfnClient, 0);
    }
    else
    {
        LuaHelper::Register(acName, RegisterLuaCallback<ITEM, PARAM, P1, P2>, 1);
    }
}

namespace RegionEvent
{

// FUNCTION: SWBF2 0x00519c10
void Init()
{
    mEnterRegion.Init();
    RegisterUnregisterLuaCallback(mEnterRegion, DummyLuaCallback);
    RegisterRegisterLuaCallback<RedRegion, Character, RegionMatches>(mEnterRegion, DummyLuaCallback);
    RegisterRegisterLuaCallback<RedRegion, Character, RegionMatches, CharacterMatchesName>(mEnterRegion, DummyLuaCallback);
    RegisterRegisterLuaCallback<RedRegion, Character, RegionMatches, CharacterMatchesTeam>(mEnterRegion, DummyLuaCallback);
    RegisterRegisterLuaCallback<RedRegion, Character, RegionMatches, CharacterMatchesClass>(mEnterRegion, DummyLuaCallback);

    mLeaveRegion.Init();
    RegisterUnregisterLuaCallback(mLeaveRegion, DummyLuaCallback);
    RegisterRegisterLuaCallback<RedRegion, Character, RegionMatches>(mLeaveRegion, DummyLuaCallback);
    RegisterRegisterLuaCallback<RedRegion, Character, RegionMatches, CharacterMatchesName>(mLeaveRegion, DummyLuaCallback);
    RegisterRegisterLuaCallback<RedRegion, Character, RegionMatches, CharacterMatchesTeam>(mLeaveRegion, DummyLuaCallback);
    RegisterRegisterLuaCallback<RedRegion, Character, RegionMatches, CharacterMatchesClass>(mLeaveRegion, DummyLuaCallback);

    ActiveRegions::Init();
}

// FUNCTION: SWBF2 0x00519c80
void Cleanup()
{
    ActiveRegions::Cleanup();
    mEnterRegion.Cleanup();
    mLeaveRegion.Cleanup();
}

}

// SYNTHETIC: SWBF2 0x00519ca0
// EventManager::Event<RedRegion,Character>::Callback::~Callback

// SYNTHETIC: SWBF2 0x00519d10
// EventManager::Event<RedRegion,Character>::Callback::`scalar deleting destructor'

// TEMPLATE: SWBF2 0x00519d80
// EventManager::UnregisterLuaCallback<RedRegion,Character>

// TEMPLATE: SWBF2 0x00519dd0
// EventManager::RegisterUnregisterLuaCallback<RedRegion,Character>

// TEMPLATE: SWBF2 0x00519e60
// EventManager::RegisterRegisterLuaCallback<RedRegion,Character,EventManager::RegionMatches>

// TEMPLATE: SWBF2 0x00519ef0
// EventManager::RegisterLuaCallback<RedRegion,Character,EventManager::RegionMatches>

// TEMPLATE: SWBF2 0x0051a070
// EventManager::RegisterRegisterLuaCallback<RedRegion,Character,EventManager::RegionMatches,EventManager::CharacterMatchesName>

// TEMPLATE: SWBF2 0x0051a110
// EventManager::RegisterLuaCallback<RedRegion,Character,EventManager::RegionMatches,EventManager::CharacterMatchesName>

// TEMPLATE: SWBF2 0x0051a2d0
// EventManager::RegisterRegisterLuaCallback<RedRegion,Character,EventManager::RegionMatches,EventManager::CharacterMatchesTeam>

// TEMPLATE: SWBF2 0x0051a370
// EventManager::RegisterLuaCallback<RedRegion,Character,EventManager::RegionMatches,EventManager::CharacterMatchesTeam>

// TEMPLATE: SWBF2 0x0051a530
// EventManager::RegisterRegisterLuaCallback<RedRegion,Character,EventManager::RegionMatches,EventManager::CharacterMatchesClass>

// TEMPLATE: SWBF2 0x0051a5d0
// EventManager::RegisterLuaCallback<RedRegion,Character,EventManager::RegionMatches,EventManager::CharacterMatchesClass>

// TEMPLATE: SWBF2 0x0051a790
// EventManager::Event<RedRegion,Character>::Unregister

// TEMPLATE: SWBF2 0x0051a7d0
// EventManager::LuaCallback<RedRegion,Character>::LuaCallback<RedRegion,Character>

// TEMPLATE: SWBF2 0x0051a860
// EventManager::LuaCallback<RedRegion,Character>::operator()

// TEMPLATE: SWBF2 0x0051a8d0
// EventManager::Event<RedRegion,Character>::Filter<EventManager::RegionMatches>::operator()

// TEMPLATE: SWBF2 0x0051a8f0
// EventManager::Event<RedRegion,Character>::Filter<EventManager::RegionMatches>::GetName

// TEMPLATE: SWBF2 0x0051a900
// EventManager::Event<RedRegion,Character>::Filter<EventManager::CharacterMatchesName>::operator()

// TEMPLATE: SWBF2 0x0051a950
// EventManager::Event<RedRegion,Character>::Filter<EventManager::CharacterMatchesName>::GetName

// TEMPLATE: SWBF2 0x0051a960
// EventManager::Event<RedRegion,Character>::Filter<EventManager::CharacterMatchesTeam>::operator()

// TEMPLATE: SWBF2 0x0051a980
// EventManager::Event<RedRegion,Character>::Filter<EventManager::CharacterMatchesTeam>::GetName

// TEMPLATE: SWBF2 0x0051a990
// EventManager::Event<RedRegion,Character>::Filter<EventManager::CharacterMatchesClass>::operator()

// TEMPLATE: SWBF2 0x0051a9e0
// EventManager::Event<RedRegion,Character>::Filter<EventManager::CharacterMatchesClass>::GetName

// SYNTHETIC: SWBF2 0x0051a9f0
// EventManager::LuaCallback<RedRegion,Character>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0051aa90
// EventManager::Event<RedRegion,Character>::Filter<EventManager::RegionMatches>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0051ab00
// EventManager::Event<RedRegion,Character>::Filter<EventManager::CharacterMatchesName>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0051ab70
// EventManager::Event<RedRegion,Character>::Filter<EventManager::CharacterMatchesTeam>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0051abe0
// EventManager::Event<RedRegion,Character>::Filter<EventManager::CharacterMatchesClass>::`scalar deleting destructor'

template class LuaCallback<RedRegion, Character>;
template class Event<RedRegion, Character>::Filter<RegionMatches>;
template class Event<RedRegion, Character>::Filter<CharacterMatchesName>;
template class Event<RedRegion, Character>::Filter<CharacterMatchesTeam>;
template class Event<RedRegion, Character>::Filter<CharacterMatchesClass>;
template Event<RedRegion, Character>::Filter<RegionMatches>*
Event<RedRegion, Character>::GetFilter<RegionMatches>(RegionMatches&, Callback*);
template Event<RedRegion, Character>::Filter<CharacterMatchesName>*
Event<RedRegion, Character>::GetFilter<CharacterMatchesName>(CharacterMatchesName&, Callback*);
template Event<RedRegion, Character>::Filter<CharacterMatchesTeam>*
Event<RedRegion, Character>::GetFilter<CharacterMatchesTeam>(CharacterMatchesTeam&, Callback*);
template Event<RedRegion, Character>::Filter<CharacterMatchesClass>*
Event<RedRegion, Character>::GetFilter<CharacterMatchesClass>(CharacterMatchesClass&, Callback*);

}

namespace EventManager
{
namespace RegionEvent
{

// GLOBAL: SWBF2 0x007ec080
// SYNTHETIC: SWBF2 0x004036b0
// `dynamic initializer for 'EventManager::RegionEvent::mEnterRegion''
// SYNTHETIC: SWBF2 0x00768420
// `dynamic atexit destructor for 'EventManager::RegionEvent::mEnterRegion''
Event<RedRegion, Character> mEnterRegion("EnterRegion");

// GLOBAL: SWBF2 0x007ec0a0
// SYNTHETIC: SWBF2 0x004036e0
// `dynamic initializer for 'EventManager::RegionEvent::mLeaveRegion''
// SYNTHETIC: SWBF2 0x007684a0
// `dynamic atexit destructor for 'EventManager::RegionEvent::mLeaveRegion''
Event<RedRegion, Character> mLeaveRegion("LeaveRegion");

}
}
