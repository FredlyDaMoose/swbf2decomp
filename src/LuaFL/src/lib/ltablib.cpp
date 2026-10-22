#include <stddef.h>

#define ltablib_c

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#define aux_getn(L,n)	(luaL_checktype(L, n, LUA_TTABLE), luaL_getn(L, n))

// LIBRARY: SWBF2 0x006a0ec0
// luaB_foreachi
static int luaB_foreachi (lua_State *L) {
  int i;
  int n = aux_getn(L, 1);
  luaL_checktype(L, 2, LUA_TFUNCTION);
  for (i=1; i<=n; i++) {
    lua_pushvalue(L, 2);
    lua_pushnumber(L, (lua_Number)i);
    lua_rawgeti(L, 1, i);
    lua_call(L, 2, 1);
    if (!lua_isnil(L, -1))
      return 1;
    lua_pop(L, 1);
  }
  return 0;
}

// LIBRARY: SWBF2 0x006a0f60
// luaB_foreach
static int luaB_foreach (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checktype(L, 2, LUA_TFUNCTION);
  lua_pushnil(L);
  for (;;) {
    if (lua_next(L, 1) == 0)
      return 0;
    lua_pushvalue(L, 2);
    lua_pushvalue(L, -3);
    lua_pushvalue(L, -3);
    lua_call(L, 2, 1);
    if (!lua_isnil(L, -1))
      return 1;
    lua_pop(L, 2);
  }
}

// LIBRARY: SWBF2 0x006a0ff0
// luaB_getn
static int luaB_getn (lua_State *L) {
  lua_pushnumber(L, (lua_Number)aux_getn(L, 1));
  return 1;
}

// LIBRARY: SWBF2 0x006a1030
// luaB_setn
static int luaB_setn (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_setn(L, 1, luaL_checkint(L, 2));
  return 0;
}

// LIBRARY: SWBF2 0x006a1060
// luaB_tinsert
static int luaB_tinsert (lua_State *L) {
  int v = lua_gettop(L);
  int n = aux_getn(L, 1) + 1;
  int pos;
  if (v == 2)
    pos = n;
  else {
    pos = luaL_checkint(L, 2);
    if (pos > n) n = pos;
    v = 3;
  }
  luaL_setn(L, 1, n);
  while (--n >= pos) {
    lua_rawgeti(L, 1, n);
    lua_rawseti(L, 1, n+1);
  }
  lua_pushvalue(L, v);
  lua_rawseti(L, 1, pos);
  return 0;
}

// LIBRARY: SWBF2 0x006a1110
// luaB_tremove
static int luaB_tremove (lua_State *L) {
  int n = aux_getn(L, 1);
  int pos = luaL_optint(L, 2, n);
  if (n <= 0) return 0;
  luaL_setn(L, 1, n-1);
  lua_rawgeti(L, 1, pos);
  for ( ;pos<n; pos++) {
    lua_rawgeti(L, 1, pos+1);
    lua_rawseti(L, 1, pos);
  }
  lua_pushnil(L);
  lua_rawseti(L, 1, n);
  return 1;
}

// LIBRARY: SWBF2 0x006a11c0
// str_concat
static int str_concat (lua_State *L) {
  luaL_Buffer b;
  size_t lsep;
  const char *sep = luaL_optlstring(L, 2, "", &lsep);
  int i = luaL_optint(L, 3, 1);
  int n = luaL_optint(L, 4, 0);
  luaL_checktype(L, 1, LUA_TTABLE);
  if (n == 0) n = luaL_getn(L, 1);
  luaL_buffinit(L, &b);
  for (; i <= n; i++) {
    lua_rawgeti(L, 1, i);
    luaL_argcheck(L, lua_isstring(L, -1), 1, "table contains non-strings");
    luaL_addvalue(&b);
    if (i != n)
      luaL_addlstring(&b, sep, lsep);
  }
  luaL_pushresult(&b);
  return 1;
}

// LIBRARY: SWBF2 0x006a15e0
// set2
static void set2 (lua_State *L, int i, int j) {
  lua_rawseti(L, 1, i);
  lua_rawseti(L, 1, j);
}

// LIBRARY: SWBF2 0x006a1600
// sort_comp
static int sort_comp (lua_State *L, int a, int b) {
  if (!lua_isnil(L, 2)) {
    int res;
    lua_pushvalue(L, 2);
    lua_pushvalue(L, a-1);
    lua_pushvalue(L, b-2);
    lua_call(L, 2, 1);
    res = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return res;
  }
  else
    return lua_lessthan(L, a, b);
}

// LIBRARY: SWBF2 0x006a1350
// auxsort
static void auxsort (lua_State *L, int l, int u) {
  while (l < u) {
    int i, j;

    lua_rawgeti(L, 1, l);
    lua_rawgeti(L, 1, u);
    if (sort_comp(L, -1, -2))
      set2(L, l, u);
    else
      lua_pop(L, 2);
    if (u-l == 1) break;
    i = (l+u)/2;
    lua_rawgeti(L, 1, i);
    lua_rawgeti(L, 1, l);
    if (sort_comp(L, -2, -1))
      set2(L, i, l);
    else {
      lua_pop(L, 1);
      lua_rawgeti(L, 1, u);
      if (sort_comp(L, -1, -2))
        set2(L, i, u);
      else
        lua_pop(L, 2);
    }
    if (u-l == 2) break;
    lua_rawgeti(L, 1, i);
    lua_pushvalue(L, -1);
    lua_rawgeti(L, 1, u-1);
    set2(L, i, u-1);

    i = l; j = u-1;
    for (;;) {

      while (lua_rawgeti(L, 1, ++i), sort_comp(L, -1, -2)) {
        if (i>u) luaL_error(L, "invalid order function for sorting");
        lua_pop(L, 1);
      }

      while (lua_rawgeti(L, 1, --j), sort_comp(L, -3, -1)) {
        if (j<l) luaL_error(L, "invalid order function for sorting");
        lua_pop(L, 1);
      }
      if (j<i) {
        lua_pop(L, 3);
        break;
      }
      set2(L, i, j);
    }
    lua_rawgeti(L, 1, u-1);
    lua_rawgeti(L, 1, i);
    set2(L, u-1, i);

    if (i-l < u-i) {
      j=l; i=i-1; l=i+2;
    }
    else {
      j=i+1; i=u; u=j-2;
    }
    auxsort(L, j, i);
  }
}

// LIBRARY: SWBF2 0x006a12f0
// luaB_sort
static int luaB_sort (lua_State *L) {
  int n = aux_getn(L, 1);
  luaL_checkstack(L, 40, "");
  if (!lua_isnoneornil(L, 2))
    luaL_checktype(L, 2, LUA_TFUNCTION);
  lua_settop(L, 2);
  auxsort(L, 1, n);
  return 0;
}

static const luaL_reg tab_funcs[] = {
  {"concat", str_concat},
  {"foreach", luaB_foreach},
  {"foreachi", luaB_foreachi},
  {"getn", luaB_getn},
  {"setn", luaB_setn},
  {"sort", luaB_sort},
  {"insert", luaB_tinsert},
  {"remove", luaB_tremove},
  {NULL, NULL}
};

// LIBRARY: SWBF2 0x006a15b0
// luaopen_table
LUALIB_API int luaopen_table (lua_State *L) {
  luaL_openlib(L, LUA_TABLIBNAME, tab_funcs, 0);
  return 1;
}
