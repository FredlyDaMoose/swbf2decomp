#pragma once

#include "LuaFL/include/lua.h"
#include "LuaFL/include/lauxlib.h"

namespace LuaHelper
{

extern lua_State* mState;

extern bool mbInitialized;

void InitState();

void Register(char const* pcName, lua_CFunction pfnCallback, int iNumUpvalues);

// FUNCTION: SWBF2 0x00437570
inline char const* GetString(int iIndex) { return luaL_checklstring(mState, iIndex, 0); }

// FUNCTION: SWBF2 0x004375a0
inline int GetInt(int iIndex) { return (int)luaL_checknumber(mState, iIndex); }

// FUNCTION: SWBF2 0x00513960
inline float GetFloat(int iIndex) { return (float)luaL_checknumber(mState, iIndex); }
inline float GetFloat(int iIndex, float fDefault) { return (float)luaL_optnumber(mState, iIndex, fDefault); }

// FUNCTION: SWBF2 0x00513940
inline bool GetBool(int iIndex) { return lua_toboolean(mState, iIndex) != 0; }

// FUNCTION: SWBF2 0x0057dc30
inline bool IsNumber(int iIndex) { return lua_isnumber(mState, iIndex) != 0; }

// FUNCTION: SWBF2 0x00580860
inline bool IsNoneOrNil(int iIndex) { return lua_type(mState, iIndex) <= LUA_TNIL; }
// FUNCTION: SWBF2 0x00580880
inline bool IsBool(int iIndex) { return lua_type(mState, iIndex) == LUA_TBOOLEAN; }

inline bool Equal(int iIndex1, int iIndex2) { return lua_equal(mState, iIndex1, iIndex2) != 0; }

extern unsigned int m_uiOpenScriptNameHash;

void CleanupState();
bool OpenScript(char const* pcName);
bool CloseScript();

bool PushRefProc(int iRef);

void PushString(unsigned short const* pusString);

void PushString(const char* pString);

bool __fastcall IsString(int iIndex);
void PushInt(int iValue);
int GetTop();
int TableGetN(int iStack);
void TableGetByPosition(int iStack, int iIndex);
void RawGetFromTable(int iStack);
bool PushGlobalProc(char const* pcFunc);
bool PushTableProc(char const* pcTable, char const* pcFunc);
bool PushTableProcWThis(char const* pcTable, char const* pcFunc);

int CallProc(int);
bool CallProc(char const* pcName);
bool CallGlobalProc(char const* pcFunc);

bool CallGlobalProc(char const* pcFunc, float fArg);

bool CallGlobalProc(char const* pcFunc, unsigned short const* pusArg);
bool CallTableProcWThis(char const* pcTable, char const* pcFunc);

bool CallTableProcWThis(char const* pcTable, char const* pcFunc, float fArg);
bool CallTableProcWThis(char const* pcTable, char const* pcFunc, unsigned short const* pusArg);
bool CallTableProcWThis(char const* pcTable, char const* pcFunc, float fArg1, float fArg2);
void TableWriteNil(char const* pcKey);

// FUNCTION: SWBF2 0x00527f80
inline void GetGlobal(char const* pcName)
{
    lua_pushstring(mState, pcName);
    lua_gettable(mState, LUA_GLOBALSINDEX);
}

// FUNCTION: SWBF2 0x0050b450
inline void Pop(int iCount)
{
    lua_settop(mState, -1 - iCount);
}

void TableWriteVal(char const* pcKey, int iVal);
void TableWriteVal(char const* pcKey, unsigned short const* pusVal);

void TableWriteVal(char const* pcKey, float fVal);
void TableWriteVal(char const* pcKey, bool bVal);

void TableWriteVal(char const* pcKey, char const* pcVal);
void SetGlobal(char const* pcName);
void TableReadVal(char const* pcKey, float& fVal, bool bRequired);
void TableReadVal(char const* pcKey, int& iVal, bool bRequired);
void TableReadVal(char const* pcKey, bool& bVal);
void TableReadVal(char const* pcKey, char* pcVal, unsigned int uSize, bool bRequired);

}
