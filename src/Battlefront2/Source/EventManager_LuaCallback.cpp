#include "EventManager_Timer.h"
#include "LuaHelper.h"
#include "LuaFL/include/lauxlib.h"

namespace EventManager
{

// FUNCTION: SWBF2 0x00513980
template <>
bool LuaPushItem<int>(int& item)
{
    lua_pushnumber(LuaHelper::mState, (lua_Number)item);
    return true;
}

// FUNCTION: SWBF2 0x005139a0
template <>
bool LuaPushItem<float>(float& item)
{
    lua_pushnumber(LuaHelper::mState, (lua_Number)item);
    return true;
}

// FUNCTION: SWBF2 0x005139c0
int DummyLuaCallback(lua_State*)
{
    return 0;
}

}
