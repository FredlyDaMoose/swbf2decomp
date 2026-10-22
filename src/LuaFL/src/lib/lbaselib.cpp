#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define lbaselib_c

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#include "Battlefront2/Source/LuaMemory.h"

#include "RedEngineFL/Util/RedWarning.h"

// LIBRARY: SWBF2 0x0069f9d0
// luaB_print
static int luaB_print (lua_State *L) {
  int n = lua_gettop(L);
  int i;
  lua_getglobal(L, "tostring");
  for (i=1; i<=n; i++) {
    const char *s;
    lua_pushvalue(L, -1);
    lua_pushvalue(L, i);
    lua_call(L, 1, 1);
    s = lua_tostring(L, -1);
    if (s == NULL)
      return luaL_error(L, "`tostring' must return a string to `print'");
    if (i>1) {
      RedWarning::SetLogData(RedWarning::RED_SEVERITY_INFORM);
      RedWarning::LogMessage("    ");
    }
    RedWarning::SetLogData(RedWarning::RED_SEVERITY_INFORM);
    RedWarning::LogMessage("%s", s);
    lua_pop(L, 1);
  }
  RedWarning::SetLogData(RedWarning::RED_SEVERITY_INFORM);
  RedWarning::LogMessage("\n");
  return 0;
}

// LIBRARY: SWBF2 0x0069faa0
// luaB_tonumber
static int luaB_tonumber (lua_State *L) {
  int base = luaL_optint(L, 2, 10);
  if (base == 10) {
    luaL_checkany(L, 1);
    if (lua_isnumber(L, 1)) {
      lua_pushnumber(L, lua_tonumber(L, 1));
      return 1;
    }
  }
  else {
    const char *s1 = luaL_checkstring(L, 1);
    char *s2;
    unsigned long n;
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    n = strtoul(s1, &s2, base);
    if (s1 != s2) {
      while (isspace((unsigned char)(*s2))) s2++;
      if (*s2 == '\0') {
        lua_pushnumber(L, (lua_Number)n);
        return 1;
      }
    }
  }
  lua_pushnil(L);
  return 1;
}

// LIBRARY: SWBF2 0x0069fbc0
// luaB_error
static int luaB_error (lua_State *L) {
  int level = luaL_optint(L, 2, 1);
  luaL_checkany(L, 1);
  if (!lua_isstring(L, 1) || level == 0)
    lua_pushvalue(L, 1);
  else {
    luaL_where(L, level);
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  return lua_error(L);
}

// LIBRARY: SWBF2 0x0069fc40
// luaB_getmetatable
static int luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  if (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  luaL_getmetafield(L, 1, "__metatable");
  return 1;
}

// LIBRARY: SWBF2 0x0069fc90
// luaB_setmetatable
static int luaB_setmetatable (lua_State *L) {
  int t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argcheck(L, t == LUA_TNIL || t == LUA_TTABLE, 2,
                    "nil or table expected");
  if (luaL_getmetafield(L, 1, "__metatable"))
    luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  return 1;
}

// LIBRARY: SWBF2 0x006a09a0
// getfunc
static void getfunc (lua_State *L) {
  if (lua_isfunction(L, 1)) lua_pushvalue(L, 1);
  else {
    lua_Debug ar;
    int level = luaL_optint(L, 1, 1);
    luaL_argcheck(L, level >= 0, 1, "level must be non-negative");
    if (lua_getstack(L, level, &ar) == 0)
      luaL_argerror(L, 1, "invalid level");
    lua_getinfo(L, "f", &ar);
    if (lua_isnil(L, -1))
      luaL_error(L, "no function environment for tail call at level %d",
                    level);
  }
}

// LIBRARY: SWBF2 0x006a0810
// aux_getfenv
static int aux_getfenv (lua_State *L) {
  lua_getfenv(L, -1);
  lua_pushliteral(L, "__fenv");
  lua_rawget(L, -2);
  return !lua_isnil(L, -1);
}

// LIBRARY: SWBF2 0x0069fd10
// luaB_getfenv
static int luaB_getfenv (lua_State *L) {
  getfunc(L);
  if (!aux_getfenv(L))
    lua_pop(L, 1);
  return 1;
}

// LIBRARY: SWBF2 0x0069fd40
// luaB_setfenv
static int luaB_setfenv (lua_State *L) {
  luaL_checktype(L, 2, LUA_TTABLE);
  getfunc(L);
  if (aux_getfenv(L))
    luaL_error(L, "`setfenv' cannot change a protected environment");
  else
    lua_pop(L, 2);
  lua_pushvalue(L, 2);
  if (lua_isnumber(L, 1) && lua_tonumber(L, 1) == 0)
    lua_replace(L, LUA_GLOBALSINDEX);
  else if (lua_setfenv(L, -2) == 0)
    luaL_error(L, "`setfenv' cannot change environment of given function");
  return 0;
}

// LIBRARY: SWBF2 0x0069fdf0
// luaB_rawequal
static int luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  return 1;
}

// LIBRARY: SWBF2 0x0069fe30
// luaB_rawget
static int luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_rawget(L, 1);
  return 1;
}

// LIBRARY: SWBF2 0x0069fe60
// luaB_rawset
static int luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_rawset(L, 1);
  return 1;
}

// LIBRARY: SWBF2 0x0069fea0
// luaB_gcinfo
static int luaB_gcinfo (lua_State *L) {
  lua_pushnumber(L, (lua_Number)lua_getgccount(L));
  lua_pushnumber(L, (lua_Number)lua_getgcthreshold(L));
  return 2;
}

// LIBRARY: SWBF2 0x0069fef0
// luaB_collectgarbage
static int luaB_collectgarbage (lua_State *L) {
  lua_setgcthreshold(L, luaL_optint(L, 1, 0));
  return 0;
}

// LIBRARY: SWBF2 0x0069ff20
// luaB_type
static int luaB_type (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushstring(L, lua_typename(L, lua_type(L, 1)));
  return 1;
}

// LIBRARY: SWBF2 0x0069ff50
// luaB_next
static int luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);
  if (lua_next(L, 1))
    return 2;
  else {
    lua_pushnil(L);
    return 1;
  }
}

// LIBRARY: SWBF2 0x0069ffa0
// luaB_pairs
static int luaB_pairs (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_pushliteral(L, "next");
  lua_rawget(L, LUA_GLOBALSINDEX);
  lua_pushvalue(L, 1);
  lua_pushnil(L);
  return 3;
}

// LIBRARY: SWBF2 0x0069fff0
// luaB_ipairs
static int luaB_ipairs (lua_State *L) {
  lua_Number i = lua_tonumber(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  if (i == 0 && lua_isnone(L, 2)) {
    lua_pushliteral(L, "ipairs");
    lua_rawget(L, LUA_GLOBALSINDEX);
    lua_pushvalue(L, 1);
    lua_pushnumber(L, 0);
    return 3;
  }
  else {
    i++;
    lua_pushnumber(L, i);
    lua_rawgeti(L, 1, (int)i);
    return (lua_isnil(L, -1)) ? 0 : 2;
  }
}

// FUNCTION: SWBF2 0x006a0a80
static int load_aux (lua_State *L, int status) {
  if (status == 0)
    return 1;
  else {
    lua_pushnil(L);
    lua_insert(L, -2);
    return 2;
  }
}

// LIBRARY: SWBF2 0x006a00c0
// luaB_loadstring
static int luaB_loadstring (lua_State *L) {
  size_t l;
  const char *s = luaL_checklstring(L, 1, &l);
  const char *chunkname = luaL_optstring(L, 2, s);
  return load_aux(L, luaL_loadbuffer(L, s, l, chunkname));
}

// LIBRARY: SWBF2 0x006a0100
// luaB_loadfile
static int luaB_loadfile (lua_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  return load_aux(L, luaL_loadfile(L, fname));
}

// LIBRARY: SWBF2 0x006a0130
// luaB_dofile
static int luaB_dofile (lua_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  int n = lua_gettop(L);
  int status = luaL_loadfile(L, fname);
  if (status != 0) lua_error(L);
  lua_call(L, 0, LUA_MULTRET);
  return lua_gettop(L) - n;
}

// LIBRARY: SWBF2 0x006a0180
// luaB_assert
static int luaB_assert (lua_State *L) {
  luaL_checkany(L, 1);
  if (!lua_toboolean(L, 1))
    return luaL_error(L, "%s", luaL_optstring(L, 2, "assertion failed!"));
  lua_settop(L, 1);
  return 1;
}

// LIBRARY: SWBF2 0x006a01e0
// luaB_unpack
static int luaB_unpack (lua_State *L) {
  int n, i;
  luaL_checktype(L, 1, LUA_TTABLE);
  n = luaL_getn(L, 1);
  luaL_checkstack(L, n, "table too big to unpack");
  for (i=1; i<=n; i++)
    lua_rawgeti(L, 1, i);
  return n;
}

// LIBRARY: SWBF2 0x006a0230
// luaB_pcall
static int luaB_pcall (lua_State *L) {
  int status;
  luaL_checkany(L, 1);
  status = lua_pcall(L, lua_gettop(L) - 1, LUA_MULTRET, 0);
  lua_pushboolean(L, (status == 0));
  lua_insert(L, 1);
  return lua_gettop(L);
}

// LIBRARY: SWBF2 0x006a0280
// luaB_xpcall
static int luaB_xpcall (lua_State *L) {
  int status;
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_insert(L, 1);
  status = lua_pcall(L, 0, LUA_MULTRET, 1);
  lua_pushboolean(L, (status == 0));
  lua_replace(L, 1);
  return lua_gettop(L);
}

// LIBRARY: SWBF2 0x006a02d0
// luaB_tostring
static int luaB_tostring (lua_State *L) {
  char buff[128];
  luaL_checkany(L, 1);
  if (luaL_callmeta(L, 1, "__tostring"))
    return 1;
  switch (lua_type(L, 1)) {
    case LUA_TNUMBER:
      lua_pushstring(L, lua_tostring(L, 1));
      return 1;
    case LUA_TSTRING:
      lua_pushvalue(L, 1);
      return 1;
    case LUA_TBOOLEAN:
      lua_pushstring(L, (lua_toboolean(L, 1) ? "true" : "false"));
      return 1;
    case LUA_TTABLE:
      sprintf(buff, "table: %p", lua_topointer(L, 1));
      break;
    case LUA_TFUNCTION:
      sprintf(buff, "function: %p", lua_topointer(L, 1));
      break;
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
      sprintf(buff, "userdata: %p", lua_touserdata(L, 1));
      break;
    case LUA_TTHREAD:
      sprintf(buff, "thread: %p", (void *)lua_tothread(L, 1));
      break;
    case LUA_TNIL:
      lua_pushliteral(L, "nil");
      return 1;
  }
  lua_pushstring(L, buff);
  return 1;
}

// LIBRARY: SWBF2 0x006a0b20
// luaB_newproxy
static int luaB_newproxy (lua_State *L) {
  lua_settop(L, 1);
  lua_newuserdata(L, 0);
  if (lua_toboolean(L, 1) == 0)
    return 1;
  else if (lua_isboolean(L, 1)) {
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_pushboolean(L, 1);
    lua_rawset(L, lua_upvalueindex(1));
  }
  else {
    int validproxy = 0;
    if (lua_getmetatable(L, 1)) {
      lua_rawget(L, lua_upvalueindex(1));
      validproxy = lua_toboolean(L, -1);
      lua_pop(L, 1);
    }
    luaL_argcheck(L, validproxy, 1, "boolean or proxy expected");
    lua_getmetatable(L, 1);
  }
  lua_setmetatable(L, 2);
  return 1;
}

#define REQTAB		"_LOADED"

#define LUA_PATH	"LUA_PATH"

#ifndef LUA_PATH_SEP
#define LUA_PATH_SEP	';'
#endif

#ifndef LUA_PATH_MARK
#define LUA_PATH_MARK	'?'
#endif

#ifndef LUA_PATH_DEFAULT
#define LUA_PATH_DEFAULT	"?;?.lua"
#endif

// LIBRARY: SWBF2 0x006a0a70
// getpath
static const char *getpath (lua_State *L) {
  (void)L;
  return LUA_PATH_DEFAULT;
}
#if 0
static const char *getpath (lua_State *L) {
  const char *path;
  lua_getglobal(L, LUA_PATH);
  path = lua_tostring(L, -1);
  lua_pop(L, 1);
  if (path) return path;
  path = getenv(LUA_PATH);
  if (path) return path;
  return LUA_PATH_DEFAULT;
}
#endif

// LIBRARY: SWBF2 0x006a0ce0
// pushnextpath
static const char *pushnextpath (lua_State *L, const char *path) {
  const char *l;
  if (*path == '\0') return NULL;
  if (*path == LUA_PATH_SEP) path++;
  l = strchr(path, LUA_PATH_SEP);
  if (l == NULL) l = path+strlen(path);
  lua_pushlstring(L, path, l - path);
  return l;
}

// LIBRARY: SWBF2 0x006a0c50
// pushcomposename
static void pushcomposename (lua_State *L) {
  const char *path = lua_tostring(L, -1);
  const char *wild;
  int n = 1;
  while ((wild = strchr(path, LUA_PATH_MARK)) != NULL) {

    luaL_checkstack(L, 3, "too many marks in a path component");
    lua_pushlstring(L, path, wild - path);
    lua_pushvalue(L, 1);
    path = wild + 1;
    n += 2;
  }
  lua_pushstring(L, path);
  lua_concat(L, n);
}

// LIBRARY: SWBF2 0x006a0440
// luaB_require
static int luaB_require (lua_State *L) {
  const char *path;
  int status = LUA_ERRFILE;
  luaL_checkstring(L, 1);
  lua_settop(L, 1);
  lua_getglobal(L, REQTAB);
  if (!lua_istable(L, 2)) return luaL_error(L, "`" REQTAB "' is not a table");
  path = getpath(L);
  lua_pushvalue(L, 1);
  lua_rawget(L, 2);
  if (lua_toboolean(L, -1))
    return 1;
  else {
    while (status == LUA_ERRFILE) {
      lua_settop(L, 3);
      if ((path = pushnextpath(L, path)) == NULL) break;
      pushcomposename(L);
      status = luaL_loadfile(L, lua_tostring(L, -1));
    }
  }
  switch (status) {
    case 0: {
      lua_getglobal(L, "_REQUIREDNAME");
      lua_insert(L, -2);
      lua_pushvalue(L, 1);
      lua_setglobal(L, "_REQUIREDNAME");
      lua_call(L, 0, 1);
      lua_insert(L, -2);
      lua_setglobal(L, "_REQUIREDNAME");
      if (lua_isnil(L, -1)) {
        lua_pushboolean(L, 1);
        lua_replace(L, -2);
      }
      lua_pushvalue(L, 1);
      lua_pushvalue(L, -2);
      lua_rawset(L, 2);
      return 1;
    }
    case LUA_ERRFILE: {
      return luaL_error(L, "could not load package `%s' from path `%s'",
                            lua_tostring(L, 1), getpath(L));
    }
    default: {
      return luaL_error(L, "error loading package `%s' (%s)",
                           lua_tostring(L, 1), lua_tostring(L, -1));
    }
  }
}

// FUNCTION: SWBF2 0x006a0620
static int luaB__ALERT (lua_State *L) {
  LuaScript_CallWARNINGF(luaL_checkstring(L, 1));
  return 0;
}

static const luaL_reg base_funcs[] = {
  {"_ALERT", luaB__ALERT},
  {"error", luaB_error},
  {"getmetatable", luaB_getmetatable},
  {"setmetatable", luaB_setmetatable},
  {"getfenv", luaB_getfenv},
  {"setfenv", luaB_setfenv},
  {"next", luaB_next},
  {"ipairs", luaB_ipairs},
  {"pairs", luaB_pairs},
  {"print", luaB_print},
  {"tonumber", luaB_tonumber},
  {"tostring", luaB_tostring},
  {"type", luaB_type},
  {"assert", luaB_assert},
  {"unpack", luaB_unpack},
  {"rawequal", luaB_rawequal},
  {"rawget", luaB_rawget},
  {"rawset", luaB_rawset},
  {"pcall", luaB_pcall},
  {"xpcall", luaB_xpcall},
  {"collectgarbage", luaB_collectgarbage},
  {"gcinfo", luaB_gcinfo},
  {"loadfile", luaB_loadfile},
  {"dofile", luaB_dofile},
  {"loadstring", luaB_loadstring},
  {"require", luaB_require},
  {NULL, NULL}
};

// LIBRARY: SWBF2 0x006a0850
// auxresume
static int auxresume (lua_State *L, lua_State *co, int narg) {
  int status;
  if (!lua_checkstack(co, narg))
    luaL_error(L, "too many arguments to resume");
  lua_xmove(L, co, narg);
  status = lua_resume(co, narg);
  if (status == 0) {
    int nres = lua_gettop(co);
    if (!lua_checkstack(L, nres))
      luaL_error(L, "too many results to resume");
    lua_xmove(co, L, nres);
    return nres;
  }
  else {
    lua_xmove(co, L, 1);
    return -1;
  }
}

// LIBRARY: SWBF2 0x006a0640
// luaB_coresume
static int luaB_coresume (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  int r;
  luaL_argcheck(L, co, 1, "coroutine expected");
  r = auxresume(L, co, lua_gettop(L) - 1);
  if (r < 0) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    return 2;
  }
  else {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    return r + 1;
  }
}

// FUNCTION: SWBF2 0x006a0ab0
static int luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  int r = auxresume(L, co, lua_gettop(L));
  if (r < 0) {
    if (lua_isstring(L, -1)) {
      luaL_where(L, 1);
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    lua_error(L);
  }
  return r;
}

// LIBRARY: SWBF2 0x006a06c0
// luaB_cocreate
static int luaB_cocreate (lua_State *L) {
  lua_State *NL = lua_newthread(L);
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1), 1,
    "Lua function expected");
  lua_pushvalue(L, 1);
  lua_xmove(L, NL, 1);
  return 1;
}

// LIBRARY: SWBF2 0x006a0720
// luaB_cowrap
static int luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  return 1;
}

// LIBRARY: SWBF2 0x006a0750
// luaB_yield
static int luaB_yield (lua_State *L) {
  return lua_yield(L, lua_gettop(L));
}

// LIBRARY: SWBF2 0x006a0770
// luaB_costatus
static int luaB_costatus (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  if (L == co) lua_pushliteral(L, "running");
  else {
    lua_Debug ar;
    if (lua_getstack(co, 0, &ar) == 0 && lua_gettop(co) == 0)
      lua_pushliteral(L, "dead");
    else
      lua_pushliteral(L, "suspended");
  }
  return 1;
}

static const luaL_reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"wrap", luaB_cowrap},
  {"resume", luaB_coresume},
  {"yield", luaB_yield},
  {"status", luaB_costatus},
  {NULL, NULL}
};

// LIBRARY: SWBF2 0x006a08e0
// base_open
static void base_open (lua_State *L) {
  lua_pushliteral(L, "_G");
  lua_pushvalue(L, LUA_GLOBALSINDEX);
  luaL_openlib(L, NULL, base_funcs, 0);
  lua_pushliteral(L, "_VERSION");
  lua_pushliteral(L, LUA_VERSION);
  lua_rawset(L, -3);

  lua_pushliteral(L, "newproxy");
  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setmetatable(L, -2);
  lua_pushliteral(L, "__mode");
  lua_pushliteral(L, "k");
  lua_rawset(L, -3);
  lua_pushcclosure(L, luaB_newproxy, 1);
  lua_rawset(L, -3);
  lua_rawset(L, -1);
}

// LIBRARY: SWBF2 0x006a0c00
// luaopen_base
LUALIB_API int luaopen_base (lua_State *L) {
  base_open(L);
  luaL_openlib(L, LUA_COLIBNAME, co_funcs, 0);
  lua_newtable(L);
  lua_setglobal(L, REQTAB);
  return 0;
}
