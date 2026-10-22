#include <stdio.h>

#include "EventManager_Character.h"
#include "Character.h"
#include "Controllable.h"
#include "Entity.h"
#include "NetGame.h"
#include "LuaFL/include/lauxlib.h"

#include "EventManager_CommandPost.h"

namespace EventManager
{

// FUNCTION: SWBF2 0x0050b490
const char* Node::GetName()
{
    return 0;
}

Node::~Node()
{
    Clear();
    DetachFromParent();
}

// SYNTHETIC: SWBF2 0x0050b4a0
// EventManager::Node::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0050b510
template <>
bool LuaPushItem<AI::EventType>(AI::EventType& eType)
{
    lua_pushnumber(LuaHelper::mState, (lua_Number)eType);
    return true;
}

template <class PARAM>
CharacterMatchesName<PARAM>::CharacterMatchesName(const char* pcName)
    : mId(PblHash(pcName))
{
}

template <class PARAM>
CharacterMatchesClass<PARAM>::CharacterMatchesClass(const char* pcName)
    : mClass(EntityClass::Find(PblHash(pcName)))
{
}

template <class PARAM>
const char* CharacterMatchesName<PARAM>::GetName()
{
    return "Name";
}

template <class PARAM>
const char* CharacterMatchesTeam<PARAM>::GetName()
{
    return "Team";
}

template <class PARAM>
const char* CharacterMatchesClass<PARAM>::GetName()
{
    return "Class";
}

template <class PARAM>
bool CharacterMatchesName<PARAM>::operator()(const Character& character, const PARAM&)
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

template <class PARAM>
bool CharacterMatchesTeam<PARAM>::operator()(const Character& character, const PARAM&) const
{
    return character.mTeamNumber == mTeam;
}

template <class PARAM>
bool CharacterMatchesClass<PARAM>::operator()(const Character& character, const PARAM&)
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

// FUNCTION: SWBF2 0x0050b530
template <>
CharacterMatchesName<DamageOwner> LuaGetItem<CharacterMatchesName<DamageOwner> >(int iIndex)
{
    return CharacterMatchesName<DamageOwner>(luaL_checkstring(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x0050b560
template <>
CharacterMatchesName<char> LuaGetItem<CharacterMatchesName<char> >(int iIndex)
{
    return CharacterMatchesName<char>(luaL_checkstring(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x0050b590
template <>
CharacterMatchesName<GameObject> LuaGetItem<CharacterMatchesName<GameObject> >(int iIndex)
{
    return CharacterMatchesName<GameObject>(luaL_checkstring(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x0050b5c0
template <>
CharacterMatchesName<AI::EventType> LuaGetItem<CharacterMatchesName<AI::EventType> >(int iIndex)
{
    return CharacterMatchesName<AI::EventType>(luaL_checkstring(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x0050b5f0
template <>
CharacterMatchesTeam<DamageOwner> LuaGetItem<CharacterMatchesTeam<DamageOwner> >(int iIndex)
{
    return CharacterMatchesTeam<DamageOwner>((unsigned int)luaL_checknumber(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x0050b610
template <>
CharacterMatchesTeam<char> LuaGetItem<CharacterMatchesTeam<char> >(int iIndex)
{
    return CharacterMatchesTeam<char>((unsigned int)luaL_checknumber(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x0050b630
template <>
CharacterMatchesTeam<GameObject> LuaGetItem<CharacterMatchesTeam<GameObject> >(int iIndex)
{
    return CharacterMatchesTeam<GameObject>((unsigned int)luaL_checknumber(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x0050b650
template <>
CharacterMatchesTeam<AI::EventType> LuaGetItem<CharacterMatchesTeam<AI::EventType> >(int iIndex)
{
    return CharacterMatchesTeam<AI::EventType>((unsigned int)luaL_checknumber(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x0050b670
template <>
CharacterMatchesClass<DamageOwner> LuaGetItem<CharacterMatchesClass<DamageOwner> >(int iIndex)
{
    return CharacterMatchesClass<DamageOwner>(luaL_checkstring(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x0050b6d0
template <>
CharacterMatchesClass<char> LuaGetItem<CharacterMatchesClass<char> >(int iIndex)
{
    return CharacterMatchesClass<char>(luaL_checkstring(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x0050b730
template <>
CharacterMatchesClass<GameObject> LuaGetItem<CharacterMatchesClass<GameObject> >(int iIndex)
{
    return CharacterMatchesClass<GameObject>(luaL_checkstring(LuaHelper::mState, iIndex));
}

// FUNCTION: SWBF2 0x0050b790
template <>
CharacterMatchesClass<AI::EventType> LuaGetItem<CharacterMatchesClass<AI::EventType> >(int iIndex)
{
    return CharacterMatchesClass<AI::EventType>(luaL_checkstring(LuaHelper::mState, iIndex));
}

template <class ITEM, class PARAM>
bool Event<ITEM, PARAM>::Callback::operator()(ITEM&, PARAM&)
{
    return true;
}

// TEMPLATE: SWBF2 0x0050e0c0
// EventManager::Event<Character,DamageOwner>::Unregister

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

// TEMPLATE: SWBF2 0x0050f130
// EventManager::Event<Character,DamageOwner>::GetFilter<EventManager::CharacterMatchesName<DamageOwner> >

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

// TEMPLATE: SWBF2 0x0050e1c0
// EventManager::LuaCallback<Character,DamageOwner>::LuaCallback<Character,DamageOwner>

// TEMPLATE: SWBF2 0x0050e3b0
// EventManager::LuaCallback<Character,char>::LuaCallback<Character,char>

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

// SYNTHETIC: SWBF2 0x0050e970
// EventManager::LuaCallback<Character,DamageOwner>::`scalar deleting destructor'

// TEMPLATE: SWBF2 0x0050e250
// EventManager::LuaCallback<Character,DamageOwner>::operator()

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

// TEMPLATE: SWBF2 0x0050e310
// EventManager::Event<Character,DamageOwner>::Filter<EventManager::CharacterMatchesName<DamageOwner> >::GetName

template <class ITEM, class PARAM>
template <class PREDICATE>
const char* Event<ITEM, PARAM>::Filter<PREDICATE>::GetName()
{
    return PREDICATE::GetName();
}

// TEMPLATE: SWBF2 0x0050e2c0
// EventManager::Event<Character,DamageOwner>::Filter<EventManager::CharacterMatchesName<DamageOwner> >::operator()

template <class ITEM, class PARAM>
template <class PREDICATE>
bool Event<ITEM, PARAM>::Filter<PREDICATE>::operator()(ITEM& item, PARAM& param)
{
    return mPredicate(item, param);
}

// TEMPLATE: SWBF2 0x0050cf40
// EventManager::UnregisterLuaCallback<Character,GameObject>

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

// TEMPLATE: SWBF2 0x0050d0b0
// EventManager::RegisterLuaCallback<Character,GameObject>

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

// TEMPLATE: SWBF2 0x0050cdc0
// EventManager::RegisterLuaCallback<Character,char,EventManager::CharacterMatchesClass<char> >

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

namespace CharacterEvent
{

// FUNCTION: SWBF2 0x0050b7f0
void Init()
{
    mDeath.Init();
    RegisterUnregisterLuaCallback(mDeath, DummyLuaCallback);
    RegisterRegisterLuaCallback(mDeath, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, DamageOwner, CharacterMatchesName<DamageOwner> >(mDeath, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, DamageOwner, CharacterMatchesTeam<DamageOwner> >(mDeath, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, DamageOwner, CharacterMatchesClass<DamageOwner> >(mDeath, DummyLuaCallback);

    mSpawn.Init();
    RegisterUnregisterLuaCallback(mSpawn, DummyLuaCallback);
    RegisterRegisterLuaCallback(mSpawn, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, char, CharacterMatchesName<char> >(mSpawn, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, char, CharacterMatchesTeam<char> >(mSpawn, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, char, CharacterMatchesClass<char> >(mSpawn, DummyLuaCallback);

    mChangeClass.Init();
    RegisterUnregisterLuaCallback(mChangeClass, DummyLuaCallback);
    RegisterRegisterLuaCallback(mChangeClass, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, char, CharacterMatchesName<char> >(mChangeClass, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, char, CharacterMatchesTeam<char> >(mChangeClass, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, char, CharacterMatchesClass<char> >(mChangeClass, DummyLuaCallback);

    mDispensePowerup.Init();
    RegisterUnregisterLuaCallback(mDispensePowerup, DummyLuaCallback);
    RegisterRegisterLuaCallback(mDispensePowerup, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, GameObject, CharacterMatchesName<GameObject> >(mDispensePowerup, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, GameObject, CharacterMatchesTeam<GameObject> >(mDispensePowerup, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, GameObject, CharacterMatchesClass<GameObject> >(mDispensePowerup, DummyLuaCallback);

    mDispenseControllable.Init();
    RegisterUnregisterLuaCallback(mDispenseControllable, DummyLuaCallback);
    RegisterRegisterLuaCallback(mDispenseControllable, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, GameObject, CharacterMatchesName<GameObject> >(mDispenseControllable, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, GameObject, CharacterMatchesTeam<GameObject> >(mDispenseControllable, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, GameObject, CharacterMatchesClass<GameObject> >(mDispenseControllable, DummyLuaCallback);

    mLandedFlyer.Init();
    RegisterUnregisterLuaCallback(mLandedFlyer, DummyLuaCallback);
    RegisterRegisterLuaCallback(mLandedFlyer, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, GameObject, CharacterMatchesName<GameObject> >(mLandedFlyer, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, GameObject, CharacterMatchesTeam<GameObject> >(mLandedFlyer, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, GameObject, CharacterMatchesClass<GameObject> >(mLandedFlyer, DummyLuaCallback);

    mEnterVehicle.Init();
    RegisterUnregisterLuaCallback(mEnterVehicle, DummyLuaCallback);
    RegisterRegisterLuaCallback(mEnterVehicle, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, GameObject, CharacterMatchesName<GameObject> >(mEnterVehicle, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, GameObject, CharacterMatchesTeam<GameObject> >(mEnterVehicle, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, GameObject, CharacterMatchesClass<GameObject> >(mEnterVehicle, DummyLuaCallback);

    mIssueAICommand.Init();
    RegisterUnregisterLuaCallback(mIssueAICommand, DummyLuaCallback);
    RegisterRegisterLuaCallback(mIssueAICommand, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, AI::EventType, CharacterMatchesName<AI::EventType> >(mIssueAICommand, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, AI::EventType, CharacterMatchesTeam<AI::EventType> >(mIssueAICommand, DummyLuaCallback);
    RegisterRegisterLuaCallback<Character, AI::EventType, CharacterMatchesClass<AI::EventType> >(mIssueAICommand, DummyLuaCallback);

    mKillsChange.Init();
    mDeathsChange.Init();
    mScoreChange.Init();
}

// FUNCTION: SWBF2 0x0050b950
void Cleanup()
{
    mDeath.Cleanup();
    mSpawn.Cleanup();
    mChangeClass.Cleanup();
    mDispensePowerup.Cleanup();
    mDispenseControllable.Cleanup();
    mLandedFlyer.Cleanup();
    mEnterVehicle.Cleanup();
    mIssueAICommand.Cleanup();
    mKillsChange.Cleanup();
    mDeathsChange.Cleanup();
    mScoreChange.Cleanup();
}

}

// SYNTHETIC: SWBF2 0x0050b9c0
// EventManager::Event<Character,DamageOwner>::Callback::~Callback

// TEMPLATE: SWBF2 0x0050ba20
// EventManager::Event<Character,DamageOwner>::Callback::operator()

// SYNTHETIC: SWBF2 0x0050ba30
// EventManager::Event<Character,char>::Callback::~Callback

// TEMPLATE: SWBF2 0x0050ba90
// EventManager::Event<Character,char>::Callback::operator()

// SYNTHETIC: SWBF2 0x0050baa0
// EventManager::Event<Character,GameObject>::Callback::~Callback

// TEMPLATE: SWBF2 0x0050bb00
// EventManager::Event<Character,GameObject>::Callback::operator()

// SYNTHETIC: SWBF2 0x0050bb10
// EventManager::Event<Character,enum AI::EventType>::Callback::~Callback

// TEMPLATE: SWBF2 0x0050bb70
// EventManager::Event<Character,enum AI::EventType>::Callback::operator()

// TEMPLATE: SWBF2 0x0050bb80
// EventManager::Event<Character,int>::Callback::operator()

// SYNTHETIC: SWBF2 0x0050bb90
// EventManager::Event<Character,DamageOwner>::Callback::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050bc00
// EventManager::Event<Character,char>::Callback::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050bc70
// EventManager::Event<Character,GameObject>::Callback::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050bce0
// EventManager::Event<Character,enum AI::EventType>::Callback::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050bd50
// EventManager::Event<Character,int>::Callback::`scalar deleting destructor'

// TEMPLATE: SWBF2 0x0050bdc0
// EventManager::UnregisterLuaCallback<Character,DamageOwner>

// TEMPLATE: SWBF2 0x0050be10
// EventManager::RegisterUnregisterLuaCallback<Character,DamageOwner>

// TEMPLATE: SWBF2 0x0050bea0
// EventManager::RegisterRegisterLuaCallback<Character,DamageOwner>

// TEMPLATE: SWBF2 0x0050bf30
// EventManager::RegisterLuaCallback<Character,DamageOwner>

// TEMPLATE: SWBF2 0x0050c050
// EventManager::RegisterRegisterLuaCallback<Character,DamageOwner,EventManager::CharacterMatchesName<DamageOwner> >

// TEMPLATE: SWBF2 0x0050c0e0
// EventManager::RegisterLuaCallback<Character,DamageOwner,EventManager::CharacterMatchesName<DamageOwner> >

// TEMPLATE: SWBF2 0x0050c260
// EventManager::RegisterRegisterLuaCallback<Character,DamageOwner,EventManager::CharacterMatchesTeam<DamageOwner> >

// TEMPLATE: SWBF2 0x0050c2f0
// EventManager::RegisterLuaCallback<Character,DamageOwner,EventManager::CharacterMatchesTeam<DamageOwner> >

// TEMPLATE: SWBF2 0x0050c470
// EventManager::RegisterRegisterLuaCallback<Character,DamageOwner,EventManager::CharacterMatchesClass<DamageOwner> >

// TEMPLATE: SWBF2 0x0050c500
// EventManager::RegisterLuaCallback<Character,DamageOwner,EventManager::CharacterMatchesClass<DamageOwner> >

// TEMPLATE: SWBF2 0x0050c680
// EventManager::UnregisterLuaCallback<Character,char>

// TEMPLATE: SWBF2 0x0050c6d0
// EventManager::RegisterUnregisterLuaCallback<Character,char>

// TEMPLATE: SWBF2 0x0050c760
// EventManager::RegisterRegisterLuaCallback<Character,char>

// TEMPLATE: SWBF2 0x0050c7f0
// EventManager::RegisterLuaCallback<Character,char>

// TEMPLATE: SWBF2 0x0050c910
// EventManager::RegisterRegisterLuaCallback<Character,char,EventManager::CharacterMatchesName<char> >

// TEMPLATE: SWBF2 0x0050c9a0
// EventManager::RegisterLuaCallback<Character,char,EventManager::CharacterMatchesName<char> >

// TEMPLATE: SWBF2 0x0050cb20
// EventManager::RegisterRegisterLuaCallback<Character,char,EventManager::CharacterMatchesTeam<char> >

// TEMPLATE: SWBF2 0x0050cbb0
// EventManager::RegisterLuaCallback<Character,char,EventManager::CharacterMatchesTeam<char> >

// TEMPLATE: SWBF2 0x0050cd30
// EventManager::RegisterRegisterLuaCallback<Character,char,EventManager::CharacterMatchesClass<char> >

// TEMPLATE: SWBF2 0x0050cf90
// EventManager::RegisterUnregisterLuaCallback<Character,GameObject>

// TEMPLATE: SWBF2 0x0050d020
// EventManager::RegisterRegisterLuaCallback<Character,GameObject>

// TEMPLATE: SWBF2 0x0050d1d0
// EventManager::RegisterRegisterLuaCallback<Character,GameObject,EventManager::CharacterMatchesName<GameObject> >

// TEMPLATE: SWBF2 0x0050d3e0
// EventManager::RegisterRegisterLuaCallback<Character,GameObject,EventManager::CharacterMatchesTeam<GameObject> >

// TEMPLATE: SWBF2 0x0050d5f0
// EventManager::RegisterRegisterLuaCallback<Character,GameObject,EventManager::CharacterMatchesClass<GameObject> >

// TEMPLATE: SWBF2 0x0050d850
// EventManager::RegisterUnregisterLuaCallback<Character,enum AI::EventType>

// TEMPLATE: SWBF2 0x0050d8e0
// EventManager::RegisterRegisterLuaCallback<Character,enum AI::EventType>

// TEMPLATE: SWBF2 0x0050da90
// EventManager::RegisterRegisterLuaCallback<Character,enum AI::EventType,EventManager::CharacterMatchesName<enum AI::EventType> >

// TEMPLATE: SWBF2 0x0050dca0
// EventManager::RegisterRegisterLuaCallback<Character,enum AI::EventType,EventManager::CharacterMatchesTeam<enum AI::EventType> >

// TEMPLATE: SWBF2 0x0050deb0
// EventManager::RegisterRegisterLuaCallback<Character,enum AI::EventType,EventManager::CharacterMatchesClass<enum AI::EventType> >

template bool Event<Character, int>::Callback::operator()(Character&, int&);

// FUNCTION: SWBF2 0x004faa30
template <>
void Event<Character, AI::EventType>::Callback::Trigger(Character& item, AI::EventType& param)
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

// TEMPLATE: SWBF2 0x0050d260
// EventManager::RegisterLuaCallback<Character,GameObject,EventManager::CharacterMatchesName<GameObject> >

// TEMPLATE: SWBF2 0x0050d470
// EventManager::RegisterLuaCallback<Character,GameObject,EventManager::CharacterMatchesTeam<GameObject> >

// TEMPLATE: SWBF2 0x0050d680
// EventManager::RegisterLuaCallback<Character,GameObject,EventManager::CharacterMatchesClass<GameObject> >

// TEMPLATE: SWBF2 0x0050d800
// EventManager::UnregisterLuaCallback<Character,enum AI::EventType>

// TEMPLATE: SWBF2 0x0050d970
// EventManager::RegisterLuaCallback<Character,enum AI::EventType>

// TEMPLATE: SWBF2 0x0050db20
// EventManager::RegisterLuaCallback<Character,enum AI::EventType,EventManager::CharacterMatchesName<enum AI::EventType> >

// TEMPLATE: SWBF2 0x0050dd30
// EventManager::RegisterLuaCallback<Character,enum AI::EventType,EventManager::CharacterMatchesTeam<enum AI::EventType> >

// TEMPLATE: SWBF2 0x0050df40
// EventManager::RegisterLuaCallback<Character,enum AI::EventType,EventManager::CharacterMatchesClass<enum AI::EventType> >

// TEMPLATE: SWBF2 0x0050e100
// EventManager::Event<Character,char>::Unregister

// TEMPLATE: SWBF2 0x0050e140
// EventManager::Event<Character,GameObject>::Unregister

// TEMPLATE: SWBF2 0x0050e180
// EventManager::Event<Character,enum AI::EventType>::Unregister

// TEMPLATE: SWBF2 0x0050e320
// EventManager::Event<Character,DamageOwner>::Filter<EventManager::CharacterMatchesTeam<DamageOwner> >::operator()

// TEMPLATE: SWBF2 0x0050e340
// EventManager::Event<Character,DamageOwner>::Filter<EventManager::CharacterMatchesTeam<DamageOwner> >::GetName

// TEMPLATE: SWBF2 0x0050e350
// EventManager::Event<Character,DamageOwner>::Filter<EventManager::CharacterMatchesClass<DamageOwner> >::operator()

// TEMPLATE: SWBF2 0x0050e3a0
// EventManager::Event<Character,DamageOwner>::Filter<EventManager::CharacterMatchesClass<DamageOwner> >::GetName

// TEMPLATE: SWBF2 0x0050e440
// EventManager::LuaCallback<Character,char>::operator()

// TEMPLATE: SWBF2 0x0050e4a0
// EventManager::Event<Character,char>::Filter<EventManager::CharacterMatchesName<char> >::operator()

// TEMPLATE: SWBF2 0x0050e4f0
// EventManager::Event<Character,char>::Filter<EventManager::CharacterMatchesName<char> >::GetName

// TEMPLATE: SWBF2 0x0050e500
// EventManager::Event<Character,char>::Filter<EventManager::CharacterMatchesTeam<char> >::operator()

// TEMPLATE: SWBF2 0x0050e520
// EventManager::Event<Character,char>::Filter<EventManager::CharacterMatchesTeam<char> >::GetName

// TEMPLATE: SWBF2 0x0050e530
// EventManager::Event<Character,char>::Filter<EventManager::CharacterMatchesClass<char> >::operator()

// TEMPLATE: SWBF2 0x0050e580
// EventManager::Event<Character,char>::Filter<EventManager::CharacterMatchesClass<char> >::GetName

// TEMPLATE: SWBF2 0x0050e590
// EventManager::LuaCallback<Character,GameObject>::LuaCallback<Character,GameObject>

// TEMPLATE: SWBF2 0x0050e620
// EventManager::LuaCallback<Character,GameObject>::operator()

// TEMPLATE: SWBF2 0x0050e690
// EventManager::Event<Character,GameObject>::Filter<EventManager::CharacterMatchesName<GameObject> >::operator()

// TEMPLATE: SWBF2 0x0050e6e0
// EventManager::Event<Character,GameObject>::Filter<EventManager::CharacterMatchesName<GameObject> >::GetName

// TEMPLATE: SWBF2 0x0050e6f0
// EventManager::Event<Character,GameObject>::Filter<EventManager::CharacterMatchesTeam<GameObject> >::operator()

// TEMPLATE: SWBF2 0x0050e710
// EventManager::Event<Character,GameObject>::Filter<EventManager::CharacterMatchesTeam<GameObject> >::GetName

// TEMPLATE: SWBF2 0x0050e720
// EventManager::Event<Character,GameObject>::Filter<EventManager::CharacterMatchesClass<GameObject> >::operator()

// TEMPLATE: SWBF2 0x0050e770
// EventManager::Event<Character,GameObject>::Filter<EventManager::CharacterMatchesClass<GameObject> >::GetName

// TEMPLATE: SWBF2 0x0050e780
// EventManager::LuaCallback<Character,enum AI::EventType>::LuaCallback<Character,enum AI::EventType>

// TEMPLATE: SWBF2 0x0050e810
// EventManager::LuaCallback<Character,enum AI::EventType>::operator()

// TEMPLATE: SWBF2 0x0050e880
// EventManager::Event<Character,enum AI::EventType>::Filter<EventManager::CharacterMatchesName<enum AI::EventType> >::operator()

// TEMPLATE: SWBF2 0x0050e8d0
// EventManager::Event<Character,enum AI::EventType>::Filter<EventManager::CharacterMatchesName<enum AI::EventType> >::GetName

// TEMPLATE: SWBF2 0x0050e8e0
// EventManager::Event<Character,enum AI::EventType>::Filter<EventManager::CharacterMatchesTeam<enum AI::EventType> >::operator()

// TEMPLATE: SWBF2 0x0050e900
// EventManager::Event<Character,enum AI::EventType>::Filter<EventManager::CharacterMatchesTeam<enum AI::EventType> >::GetName

// TEMPLATE: SWBF2 0x0050e910
// EventManager::Event<Character,enum AI::EventType>::Filter<EventManager::CharacterMatchesClass<enum AI::EventType> >::operator()

// TEMPLATE: SWBF2 0x0050e960
// EventManager::Event<Character,enum AI::EventType>::Filter<EventManager::CharacterMatchesClass<enum AI::EventType> >::GetName

// SYNTHETIC: SWBF2 0x0050ea10
// EventManager::Event<Character,DamageOwner>::Filter<EventManager::CharacterMatchesName<DamageOwner> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050ea80
// EventManager::Event<Character,DamageOwner>::Filter<EventManager::CharacterMatchesTeam<DamageOwner> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050eaf0
// EventManager::Event<Character,DamageOwner>::Filter<EventManager::CharacterMatchesClass<DamageOwner> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050eb60
// EventManager::LuaCallback<Character,char>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050ec00
// EventManager::Event<Character,char>::Filter<EventManager::CharacterMatchesName<char> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050ec70
// EventManager::Event<Character,char>::Filter<EventManager::CharacterMatchesTeam<char> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050ece0
// EventManager::Event<Character,char>::Filter<EventManager::CharacterMatchesClass<char> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050ed50
// EventManager::LuaCallback<Character,GameObject>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050edf0
// EventManager::Event<Character,GameObject>::Filter<EventManager::CharacterMatchesName<GameObject> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050ee60
// EventManager::Event<Character,GameObject>::Filter<EventManager::CharacterMatchesTeam<GameObject> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050eed0
// EventManager::Event<Character,GameObject>::Filter<EventManager::CharacterMatchesClass<GameObject> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050ef40
// EventManager::LuaCallback<Character,enum AI::EventType>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050efe0
// EventManager::Event<Character,enum AI::EventType>::Filter<EventManager::CharacterMatchesName<enum AI::EventType> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050f050
// EventManager::Event<Character,enum AI::EventType>::Filter<EventManager::CharacterMatchesTeam<enum AI::EventType> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0050f0c0
// EventManager::Event<Character,enum AI::EventType>::Filter<EventManager::CharacterMatchesClass<enum AI::EventType> >::`scalar deleting destructor'

// TEMPLATE: SWBF2 0x0050f1e0
// EventManager::Event<Character,DamageOwner>::GetFilter<EventManager::CharacterMatchesTeam<DamageOwner> >

// TEMPLATE: SWBF2 0x0050f290
// EventManager::Event<Character,DamageOwner>::GetFilter<EventManager::CharacterMatchesClass<DamageOwner> >

// TEMPLATE: SWBF2 0x0050f340
// EventManager::Event<Character,char>::GetFilter<EventManager::CharacterMatchesName<char> >

// TEMPLATE: SWBF2 0x0050f3f0
// EventManager::Event<Character,char>::GetFilter<EventManager::CharacterMatchesTeam<char> >

// TEMPLATE: SWBF2 0x0050f4a0
// EventManager::Event<Character,char>::GetFilter<EventManager::CharacterMatchesClass<char> >

// TEMPLATE: SWBF2 0x0050f550
// EventManager::Event<Character,GameObject>::GetFilter<EventManager::CharacterMatchesName<GameObject> >

// TEMPLATE: SWBF2 0x0050f600
// EventManager::Event<Character,GameObject>::GetFilter<EventManager::CharacterMatchesTeam<GameObject> >

// TEMPLATE: SWBF2 0x0050f6b0
// EventManager::Event<Character,GameObject>::GetFilter<EventManager::CharacterMatchesClass<GameObject> >

// TEMPLATE: SWBF2 0x0050f760
// EventManager::Event<Character,enum AI::EventType>::GetFilter<EventManager::CharacterMatchesName<enum AI::EventType> >

// TEMPLATE: SWBF2 0x0050f810
// EventManager::Event<Character,enum AI::EventType>::GetFilter<EventManager::CharacterMatchesTeam<enum AI::EventType> >

// TEMPLATE: SWBF2 0x0050f8c0
// EventManager::Event<Character,enum AI::EventType>::GetFilter<EventManager::CharacterMatchesClass<enum AI::EventType> >

template class LuaCallback<Character, DamageOwner>;
template class LuaCallback<Character, char>;
template class LuaCallback<Character, GameObject>;
template class LuaCallback<Character, AI::EventType>;
template class Event<Character, DamageOwner>::Filter<CharacterMatchesName<DamageOwner> >;
template class Event<Character, DamageOwner>::Filter<CharacterMatchesTeam<DamageOwner> >;
template class Event<Character, DamageOwner>::Filter<CharacterMatchesClass<DamageOwner> >;
template class Event<Character, char>::Filter<CharacterMatchesName<char> >;
template class Event<Character, char>::Filter<CharacterMatchesTeam<char> >;
template class Event<Character, char>::Filter<CharacterMatchesClass<char> >;
template class Event<Character, GameObject>::Filter<CharacterMatchesName<GameObject> >;
template class Event<Character, GameObject>::Filter<CharacterMatchesTeam<GameObject> >;
template class Event<Character, GameObject>::Filter<CharacterMatchesClass<GameObject> >;
template class Event<Character, AI::EventType>::Filter<CharacterMatchesName<AI::EventType> >;
template class Event<Character, AI::EventType>::Filter<CharacterMatchesTeam<AI::EventType> >;
template class Event<Character, AI::EventType>::Filter<CharacterMatchesClass<AI::EventType> >;

}

namespace EventManager
{
// TEMPLATE: SWBF2 0x00511390
// EventManager::Event<CommandPost,DamageOwner>::Unregister
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x00510900
// EventManager::RegisterLuaCallback<CommandPost,DamageOwner,EventManager::PostMatchesName<DamageOwner> >
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x00510b10
// EventManager::RegisterLuaCallback<CommandPost,DamageOwner,EventManager::PostMatchesTeam<DamageOwner> >
}

namespace EventManager
{
namespace CharacterEvent
{

// GLOBAL: SWBF2 0x007eba20
// SYNTHETIC: SWBF2 0x00402f30
// `dynamic initializer for 'EventManager::CharacterEvent::mDeath''
// SYNTHETIC: SWBF2 0x00767020
// `dynamic atexit destructor for 'EventManager::CharacterEvent::mDeath''
Event<Character, DamageOwner> mDeath("CharacterDeath");

// GLOBAL: SWBF2 0x007eba00
// SYNTHETIC: SWBF2 0x00402f60
// `dynamic initializer for 'EventManager::CharacterEvent::mSpawn''
// SYNTHETIC: SWBF2 0x007670a0
// `dynamic atexit destructor for 'EventManager::CharacterEvent::mSpawn''
Event<Character, char> mSpawn("CharacterSpawn");

// GLOBAL: SWBF2 0x007eba40
// SYNTHETIC: SWBF2 0x00402f90
// `dynamic initializer for 'EventManager::CharacterEvent::mChangeClass''
// SYNTHETIC: SWBF2 0x00767120
// `dynamic atexit destructor for 'EventManager::CharacterEvent::mChangeClass''
Event<Character, char> mChangeClass("CharacterChangeClass");

// GLOBAL: SWBF2 0x007eb900
// SYNTHETIC: SWBF2 0x00402fc0
// `dynamic initializer for 'EventManager::CharacterEvent::mDispensePowerup''
// SYNTHETIC: SWBF2 0x007671a0
// `dynamic atexit destructor for 'EventManager::CharacterEvent::mDispensePowerup''
Event<Character, GameObject> mDispensePowerup("CharacterDispensePowerup");

// GLOBAL: SWBF2 0x007eb8d0
// SYNTHETIC: SWBF2 0x00402ff0
// `dynamic initializer for 'EventManager::CharacterEvent::mDispenseControllable''
// SYNTHETIC: SWBF2 0x00767220
// `dynamic atexit destructor for 'EventManager::CharacterEvent::mDispenseControllable''
Event<Character, GameObject> mDispenseControllable("CharacterDispenseControllable");

// GLOBAL: SWBF2 0x007eb9d0
// SYNTHETIC: SWBF2 0x00403020
// `dynamic initializer for 'EventManager::CharacterEvent::mLandedFlyer''
// SYNTHETIC: SWBF2 0x007672a0
// `dynamic atexit destructor for 'EventManager::CharacterEvent::mLandedFlyer''
Event<Character, GameObject> mLandedFlyer("CharacterLandedFlyer");

// GLOBAL: SWBF2 0x007eb980
// SYNTHETIC: SWBF2 0x00403050
// `dynamic initializer for 'EventManager::CharacterEvent::mEnterVehicle''
// SYNTHETIC: SWBF2 0x00767320
// `dynamic atexit destructor for 'EventManager::CharacterEvent::mEnterVehicle''
Event<Character, GameObject> mEnterVehicle("CharacterEnterVehicle");

// GLOBAL: SWBF2 0x007eb9a0
// SYNTHETIC: SWBF2 0x00403080
// `dynamic initializer for 'EventManager::CharacterEvent::mIssueAICommand''
// SYNTHETIC: SWBF2 0x007673a0
// `dynamic atexit destructor for 'EventManager::CharacterEvent::mIssueAICommand''
Event<Character, AI::EventType> mIssueAICommand("CharacterIssueAICommand");

// GLOBAL: SWBF2 0x007eb930
// SYNTHETIC: SWBF2 0x004030b0
// `dynamic initializer for 'EventManager::CharacterEvent::mKillsChange''
// SYNTHETIC: SWBF2 0x00767420
// `dynamic atexit destructor for 'EventManager::CharacterEvent::mKillsChange''
Event<Character, int> mKillsChange("KillsChange");

// GLOBAL: SWBF2 0x007eb960
// SYNTHETIC: SWBF2 0x004030e0
// `dynamic initializer for 'EventManager::CharacterEvent::mDeathsChange''
// SYNTHETIC: SWBF2 0x007674a0
// `dynamic atexit destructor for 'EventManager::CharacterEvent::mDeathsChange''
Event<Character, int> mDeathsChange("DeathsChange");

// GLOBAL: SWBF2 0x007eb8b0
// SYNTHETIC: SWBF2 0x00403110
// `dynamic initializer for 'EventManager::CharacterEvent::mScoreChange''
// SYNTHETIC: SWBF2 0x00767520
// `dynamic atexit destructor for 'EventManager::CharacterEvent::mScoreChange''
Event<Character, int> mScoreChange("ScoreChange");

}
}
