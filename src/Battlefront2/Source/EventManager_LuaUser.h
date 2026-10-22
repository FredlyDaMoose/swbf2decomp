#pragma once

#include "EventManager_Timer.h"
#include "LuaHelper.h"

namespace EventManager
{

namespace LuaUserEvent
{
int Create(lua_State* L);
}

struct LuaObject
{

    LuaObject(lua_State* L, int iIndex)
    {
        lua_pushvalue(L, iIndex);
        mRef = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    LuaObject(const LuaObject& other)
    {
        other.Push();
        mRef = luaL_ref(LuaHelper::mState, LUA_REGISTRYINDEX);
    }

    ~LuaObject()
    {
        luaL_unref(LuaHelper::mState, LUA_REGISTRYINDEX, mRef);
    }

    void Push() const;

    int mRef;
};
 

inline bool operator==(const LuaObject& a, const LuaObject& b)
{
    a.Push();
    b.Push();
    bool bResult = LuaHelper::Equal(-1, -2);
    lua_pop(LuaHelper::mState, 2);
    return bResult;
}

class LuaObjectMatches
{
public:

    LuaObjectMatches(lua_State* L, int iIndex) : mMatch(L, iIndex) {}

    static const char* GetName();

    bool operator()(LuaObject& item, LuaObject&) const
    {
        return item == mMatch;
    }

    bool operator==(const LuaObjectMatches& other) const
    {
        return mMatch == other.mMatch;
    }

    LuaObject mMatch;
};
 
 
 

template <class ITEM>
ITEM LuaGetItem(int iIndex);

template <>
LuaObjectMatches LuaGetItem<LuaObjectMatches>(int iIndex);

template <>
bool LuaPushItem<LuaObject>(LuaObject& item);

template <class ITEM, class PARAM>
int UnregisterLuaCallback(lua_State* L);
template <class ITEM, class PARAM>
int RegisterLuaCallback(lua_State* L);
template <class ITEM, class PARAM, class PREDICATE>
int RegisterLuaCallback(lua_State* L);
template <class ITEM, class PARAM>
void RegisterUnregisterLuaCallback(Event<ITEM, PARAM>& event, lua_CFunction pfnClient);
template <class ITEM, class PARAM>
void RegisterRegisterLuaCallback(Event<ITEM, PARAM>& event, lua_CFunction pfnClient);
template <class ITEM, class PARAM, class PREDICATE>
void RegisterRegisterLuaCallback(Event<ITEM, PARAM>& event, lua_CFunction pfnClient);

}
