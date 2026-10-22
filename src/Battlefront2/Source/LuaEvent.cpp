#include "LuaEvent.h"
#include "LuaFL/include/lua.h"

// GLOBAL: SWBF2 0x007ebb3c
LuaEvent::List LuaEventManager::_ActiveEventList;

// GLOBAL: SWBF2 0x007ebb68
LuaEvent::List LuaEventManager::_InactiveEventList;

// GLOBAL: SWBF2 0x01e57994
lua_State* LuaEventManager::_pLuaState;

// GLOBAL: SWBF2 0x01ec9208
// SYNTHETIC: SWBF2 0x00404260
// `dynamic initializer for 'LuaEventManager::_EventInstances''
// SYNTHETIC: SWBF2 0x00768cf0
// `dynamic atexit destructor for 'LuaEventManager::_EventInstances''
LuaEvent LuaEventManager::_EventInstances[100];

// FUNCTION: SWBF2 0x005a0c20
LuaEventManager::LuaEventManager()
{
    for (int i = 0; i < 100; i++)
    {
        _InactiveEventList.Add(&_EventInstances[i]);
    }
    _pLuaState = 0;
}

// GLOBAL: SWBF2 0x01ec0c13
// SYNTHETIC: SWBF2 0x00404290
// `dynamic initializer for 'LuaEventManager::_Instance''
// SYNTHETIC: SWBF2 0x00768d10
// `dynamic atexit destructor for 'LuaEventManager::_Instance''
LuaEventManager LuaEventManager::_Instance;

// FUNCTION: SWBF2 0x005a0ae0
LuaEvent::LuaEvent()
{
    Reset();
}

// FUNCTION: SWBF2 0x005a0aa0
LuaEvent::~LuaEvent()
{
}

// SYNTHETIC: SWBF2 0x005a0ab0
// LuaEvent::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005a0b20
void LuaEvent::Reset()
{
    _eEventType = EVENT_TYPE_UNDEFINED;
    _fRelativeTimer = 0.0f;
    _fProximitySq = 0.0f;
}

// FUNCTION: SWBF2 0x005a0b10
void LuaEvent::Deinit()
{
    DeactivateThread();
    LuaEventManager::DestroyEvent(this);
}

// FUNCTION: SWBF2 0x005a0b40
bool LuaEvent::Update(float fDeltaTime)
{
    if (_eEventType == EVENT_TYPE_RELATIVE_TIMER)
    {
        _fRelativeTimer -= fDeltaTime;
        if (_fRelativeTimer < 0.0f)
        {
            SendEvent();
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x005a0b70
void LuaEvent::SendEvent()
{
    lua_State* pState = LuaEventManager::_pLuaState;
    if (!pState)
    {
        return;
    }

    lua_pushstring(pState, _pcFlagName);
    lua_gettable(pState, LUA_GLOBALSINDEX);
    if (lua_isstring(pState, 1))
    {
        lua_pushstring(pState, "true");
        lua_pushstring(pState, _pcFlagName);
        lua_insert(pState, -2);
        lua_settable(pState, LUA_GLOBALSINDEX);
    }
    Deinit();
    lua_pushstring(pState, _pcCallbackFunctionName);
    lua_gettable(pState, LUA_GLOBALSINDEX);
    lua_pcall(pState, 0, 0, 0);
}

// FUNCTION: SWBF2 0x005a0c80
void LuaEventManager::DestroyEvent(LuaEvent* pEvent)
{
    _ActiveEventList.Remove(pEvent);
    _InactiveEventList.Add(pEvent);
}
