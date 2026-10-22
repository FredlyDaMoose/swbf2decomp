#include <string.h>

#include "LuaHelper.h"
#include "LuaFL/include/lualib.h"
#include "LuaFL/include/lauxlib.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "LuaScript.h"
#include "LuaEvent.h"

namespace LuaHelper
{

lua_State* mState;

bool mbInitialized;

// FUNCTION: SWBF2 0x005a0ce0
void InitState()
{
    if (mbInitialized)
    {
        return;
    }

    mState = lua_open();

    luaopen_base(mState);
    luaopen_table(mState);
    luaopen_io(mState);
    luaopen_string(mState);
    luaopen_math(mState);
    luaopen_debug(mState);

    mbInitialized = true;
}

// FUNCTION: SWBF2 0x005a0e70
void Register(char const* pcName, lua_CFunction pfnCallback, int iNumUpvalues)
{
    lua_pushcclosure(mState, pfnCallback, iNumUpvalues);

    lua_setglobal(mState, pcName);
}

}

// GLOBAL: SWBF2 0x01e5799c
static int mStackPos;

namespace LuaHelper
{

// GLOBAL: SWBF2 0x01e57998
unsigned int m_uiOpenScriptNameHash;

// FUNCTION: SWBF2 0x005a0d40
void CleanupState()
{
    if (mbInitialized)
    {
        mbInitialized = false;
        mState = 0;
    }
}

// FUNCTION: SWBF2 0x005a0d60
bool OpenScript(char const* pcName)
{
    if (m_uiOpenScriptNameHash)
    {
        return false;
    }

    m_uiOpenScriptNameHash = PblHash(pcName);
    LuaScriptFactory* pFactory = LuaScriptFactory::Find(m_uiOpenScriptNameHash);
    if (!pFactory)
    {
        return false;
    }

    unsigned int uiLen = pFactory->GetScriptLen();
    const char* pcScript = pFactory->GetScript();
    if (!pcScript)
    {
        m_uiOpenScriptNameHash = 0;
        return false;
    }

    InitState();
    LuaEventManager::SetLuaState(mState);

    char acName[256];
    strlcpy(acName, pcName, sizeof(acName));
    strlcat(acName, ".lua", sizeof(acName));
    lua_dobuffer(mState, pcScript, uiLen, acName);
    pFactory->Purge();

    lua_setgcthreshold(mState, lua_getgccount(mState));
    return true;
}

// FUNCTION: SWBF2 0x005a0e40
bool CallProc(char const* pcName)
{
    return CallGlobalProc(pcName);
}

// FUNCTION: SWBF2 0x005a0e50
bool CloseScript()
{
    if (!m_uiOpenScriptNameHash)
    {
        return false;
    }
    m_uiOpenScriptNameHash = 0;
    return true;
}

// FUNCTION: SWBF2 0x005a0ec0
bool PushRefProc(int iRef)
{
    lua_getref(mState, iRef);
    if (!lua_isfunction(mState, -1))
    {
        lua_pop(mState, 1);
        return false;
    }
    mStackPos = lua_gettop(mState);
    return true;
}

// FUNCTION: SWBF2 0x005a0f10
bool PushGlobalProc(char const* pcFunc)
{
    lua_pushstring(mState, pcFunc);
    lua_gettable(mState, LUA_GLOBALSINDEX);
    if (!lua_isfunction(mState, -1))
    {
        lua_pop(mState, 1);
        return false;
    }
    mStackPos = lua_gettop(mState);
    return true;
}

// FUNCTION: SWBF2 0x005a0f80
bool PushTableProc(char const* pcTable, char const* pcFunc)
{
    lua_pushstring(mState, pcTable);
    lua_gettable(mState, LUA_GLOBALSINDEX);
    if (!lua_istable(mState, -1))
    {
        lua_pop(mState, 1);
        return false;
    }

    lua_pushstring(mState, pcFunc);
    lua_gettable(mState, -2);
    if (!lua_isfunction(mState, -1))
    {
        lua_pop(mState, 2);
        return false;
    }

    lua_remove(mState, -2);
    mStackPos = lua_gettop(mState);
    return true;
}

// FUNCTION: SWBF2 0x005a1030
bool PushTableProcWThis(char const* pcTable, char const* pcFunc)
{
    if (!PushTableProc(pcTable, pcFunc))
    {
        return false;
    }
    lua_pushstring(mState, pcTable);
    lua_gettable(mState, LUA_GLOBALSINDEX);
    return true;
}

// FUNCTION: SWBF2 0x005a1070
int CallProc(int)
{
    int iResult = lua_pcall(mState, lua_gettop(mState) - mStackPos, 0, 0);
    if (iResult)
    {
        lua_pop(mState, 1);
    }
    mStackPos = 0;
    return iResult;
}

// FUNCTION: SWBF2 0x005a10f0
bool CallGlobalProc(char const* pcFunc)
{
    if (!PushGlobalProc(pcFunc))
    {
        return false;
    }
    CallProc(0);
    return true;
}

// FUNCTION: SWBF2 0x005a1140
bool CallGlobalProc(char const* pcFunc, float fArg)
{
    if (!PushGlobalProc(pcFunc))
    {
        return false;
    }
    lua_pushnumber(mState, fArg);
    CallProc(0);
    return true;
}

// FUNCTION: SWBF2 0x005a11b0
bool CallTableProcWThis(char const* pcTable, char const* pcFunc)
{
    if (!PushTableProcWThis(pcTable, pcFunc))
    {
        return false;
    }
    CallProc(0);
    return true;
}

// FUNCTION: SWBF2 0x005a1210
bool CallTableProcWThis(char const* pcTable, char const* pcFunc, unsigned short const* pusArg)
{
    if (!PushTableProcWThis(pcTable, pcFunc))
    {
        return false;
    }
    PushString(pusArg);
    CallProc(0);
    return true;
}

// FUNCTION: SWBF2 0x005a1290
void TableReadVal(char const* pcKey, float& fVal, bool bRequired)
{
    (void)bRequired;
    lua_pushstring(mState, pcKey);
    lua_rawget(mState, -2);
    if (lua_isnumber(mState, -1))
    {
        fVal = luaL_checknumber(mState, -1);
        lua_settop(mState, -2);
    }
    else
    {
        fVal = 0.0f;
        lua_settop(mState, -2);
    }
}

// FUNCTION: SWBF2 0x005a1300
void TableReadVal(char const* pcKey, int& iVal, bool bRequired)
{
    lua_pushstring(mState, pcKey);
    lua_rawget(mState, -2);
    if (lua_isnumber(mState, -1))
    {
        iVal = (unsigned int)luaL_checknumber(mState, -1);
        lua_settop(mState, -2);
    }
    else
    {
        iVal = 0;
        if (bRequired)
        {
            lua_type(mState, -1);
        }
        lua_settop(mState, -2);
    }
}

// FUNCTION: SWBF2 0x005a13e0
void TableReadVal(char const* pcKey, char* pcVal, unsigned int uSize, bool bRequired)
{
    lua_pushstring(mState, pcKey);
    lua_rawget(mState, -2);
    memset(pcVal, 0, uSize);
    if (lua_isstring(mState, -1))
    {
        strlcpy(pcVal, luaL_checklstring(mState, -1, 0), uSize);
        lua_settop(mState, -2);
    }
    else
    {
        if (bRequired)
        {
            lua_type(mState, -1);
        }
        lua_settop(mState, -2);
    }
}

// FUNCTION: SWBF2 0x005a1480
void TableWriteVal(char const* pcKey, float fVal)
{
    lua_pushstring(mState, pcKey);
    lua_pushnumber(mState, fVal);
    lua_rawset(mState, -3);
}

// FUNCTION: SWBF2 0x005a14d0
void TableWriteVal(char const* pcKey, int iVal)
{
    lua_pushstring(mState, pcKey);
    lua_pushnumber(mState, (lua_Number)iVal);
    lua_rawset(mState, -3);
}

// FUNCTION: SWBF2 0x005a1570
void TableWriteVal(char const* pcKey, unsigned short const* pusVal)
{
    lua_pushstring(mState, pcKey);
    PushString(pusVal);
    lua_rawset(mState, -3);
}

// FUNCTION: SWBF2 0x005a1540
void TableWriteVal(char const* pcKey, char const* pcVal)
{
    lua_pushstring(mState, pcKey);
    lua_pushstring(mState, pcVal);
    lua_rawset(mState, -3);
}

// FUNCTION: SWBF2 0x005a1390
void TableReadVal(char const* pcKey, bool& bVal)
{
    lua_pushstring(mState, pcKey);
    lua_rawget(mState, -2);
    bVal = lua_toboolean(mState, -1) != 0;
    lua_pop(mState, 1);
}

// FUNCTION: SWBF2 0x005a15a0
void TableWriteNil(char const* pcKey)
{
    lua_pushstring(mState, pcKey);
    lua_pushnil(mState);
    lua_rawset(mState, -3);
}

}

// FUNCTION: SWBF2 0x005a10c0
void LuaHelper::PushString(unsigned short const* pusString)
{
    lua_pushlstring(mState, (const char*)pusString, PblUnicode::ustrlen(pusString) * 2 + 2);
}

namespace LuaHelper {

// FUNCTION: SWBF2 0x005a1110
bool CallGlobalProc(char const* pcFunc, bool bArg)
{
    if (!PushGlobalProc(pcFunc))
    {
        return false;
    }
    lua_pushboolean(mState, bArg);
    CallProc(0);
    return true;
}
}

namespace LuaHelper {

// FUNCTION: SWBF2 0x005a1180
bool CallGlobalProc(char const* pcFunc, unsigned short const* pusArg)
{
    if (!PushGlobalProc(pcFunc))
    {
        return false;
    }
    PushString(pusArg);
    CallProc(0);
    return true;
}
}

// FUNCTION: SWBF2 0x005a11d0
bool LuaHelper::CallTableProcWThis(char const* pcTable, char const* pcFunc, float fArg)
{
    if (!PushTableProcWThis(pcTable, pcFunc))
    {
        return false;
    }
    lua_pushnumber(mState, fArg);
    CallProc(0);
    return true;
}

// FUNCTION: SWBF2 0x005a1230
bool LuaHelper::CallTableProcWThis(char const* pcTable, char const* pcFunc, float fArg1, float fArg2)
{
    if (!PushTableProcWThis(pcTable, pcFunc))
    {
        return false;
    }
    lua_pushnumber(mState, fArg1);
    lua_pushnumber(mState, fArg2);
    CallProc(0);
    return true;
}

namespace LuaHelper {

// FUNCTION: SWBF2 0x005a1510
void TableWriteVal(char const* pcKey, bool bVal)
{
    lua_pushstring(mState, pcKey);
    lua_pushboolean(mState, bVal);
    lua_rawset(mState, -3);
}
}
