#include <stdlib.h>
#include <math.h>

#define lmathlib_c

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#undef PI
#define PI (3.14159265358979323846)
#define RADIANS_PER_DEGREE (PI/180.0)

#ifdef USE_DEGREES
#define FROMRAD(a)	((a)/RADIANS_PER_DEGREE)
#define TORAD(a)	((a)*RADIANS_PER_DEGREE)
#else
#define FROMRAD(a)	(a)
#define TORAD(a)	(a)
#endif

// LIBRARY: SWBF2 0x0069d1e0
// math_abs
static int math_abs (lua_State *L) {
  lua_pushnumber(L, fabs(luaL_checknumber(L, 1)));
  return 1;
}

// LIBRARY: SWBF2 0x0069d230
// math_sin
static int math_sin (lua_State *L) {
  lua_pushnumber(L, sin(TORAD(luaL_checknumber(L, 1))));
  return 1;
}

// LIBRARY: SWBF2 0x0069d270
// math_cos
static int math_cos (lua_State *L) {
  lua_pushnumber(L, cos(TORAD(luaL_checknumber(L, 1))));
  return 1;
}

// LIBRARY: SWBF2 0x0069d2b0
// math_tan
static int math_tan (lua_State *L) {
  lua_pushnumber(L, tan(TORAD(luaL_checknumber(L, 1))));
  return 1;
}

// LIBRARY: SWBF2 0x0069d2f0
// math_asin
static int math_asin (lua_State *L) {
  lua_pushnumber(L, FROMRAD(asin(luaL_checknumber(L, 1))));
  return 1;
}

// LIBRARY: SWBF2 0x0069d330
// math_acos
static int math_acos (lua_State *L) {
  lua_pushnumber(L, FROMRAD(acos(luaL_checknumber(L, 1))));
  return 1;
}

// LIBRARY: SWBF2 0x0069d370
// math_atan
static int math_atan (lua_State *L) {
  lua_pushnumber(L, FROMRAD(atan(luaL_checknumber(L, 1))));
  return 1;
}

// LIBRARY: SWBF2 0x0069d3b0
// math_atan2
static int math_atan2 (lua_State *L) {
  lua_pushnumber(L, FROMRAD(atan2(luaL_checknumber(L, 1), luaL_checknumber(L, 2))));
  return 1;
}

// LIBRARY: SWBF2 0x0069d420
// math_ceil
static int math_ceil (lua_State *L) {
  lua_pushnumber(L, ceil(luaL_checknumber(L, 1)));
  return 1;
}

// LIBRARY: SWBF2 0x0069d470
// math_floor
static int math_floor (lua_State *L) {
  lua_pushnumber(L, floor(luaL_checknumber(L, 1)));
  return 1;
}

// LIBRARY: SWBF2 0x0069d4c0
// math_mod
static int math_mod (lua_State *L) {
  lua_pushnumber(L, fmod(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
  return 1;
}

// LIBRARY: SWBF2 0x0069d530
// math_sqrt
static int math_sqrt (lua_State *L) {
  lua_pushnumber(L, sqrt(luaL_checknumber(L, 1)));
  return 1;
}

// LIBRARY: SWBF2 0x0069d580
// math_pow
static int math_pow (lua_State *L) {
  lua_pushnumber(L, pow(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
  return 1;
}

// LIBRARY: SWBF2 0x0069d5e0
// math_log
static int math_log (lua_State *L) {
  lua_pushnumber(L, log(luaL_checknumber(L, 1)));
  return 1;
}

// LIBRARY: SWBF2 0x0069d620
// math_log10
static int math_log10 (lua_State *L) {
  lua_pushnumber(L, log10(luaL_checknumber(L, 1)));
  return 1;
}

// LIBRARY: SWBF2 0x0069d660
// math_exp
static int math_exp (lua_State *L) {
  lua_pushnumber(L, exp(luaL_checknumber(L, 1)));
  return 1;
}

// LIBRARY: SWBF2 0x0069d6a0
// math_deg
static int math_deg (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1)/RADIANS_PER_DEGREE);
  return 1;
}

// LIBRARY: SWBF2 0x0069d6d0
// math_rad
static int math_rad (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1)*RADIANS_PER_DEGREE);
  return 1;
}

// LIBRARY: SWBF2 0x0069d700
// math_frexp
static int math_frexp (lua_State *L) {
  int e;
  lua_pushnumber(L, frexp(luaL_checknumber(L, 1), &e));
  lua_pushnumber(L, e);
  return 2;
}

// LIBRARY: SWBF2 0x0069d770
// math_ldexp
static int math_ldexp (lua_State *L) {
  lua_pushnumber(L, ldexp(luaL_checknumber(L, 1), luaL_checkint(L, 2)));
  return 1;
}

// LIBRARY: SWBF2 0x0069d7d0
// math_min
static int math_min (lua_State *L) {
  int n = lua_gettop(L);
  lua_Number dmin = luaL_checknumber(L, 1);
  int i;
  for (i=2; i<=n; i++) {
    lua_Number d = luaL_checknumber(L, i);
    if (d < dmin)
      dmin = d;
  }
  lua_pushnumber(L, dmin);
  return 1;
}

// LIBRARY: SWBF2 0x0069d850
// math_max
static int math_max (lua_State *L) {
  int n = lua_gettop(L);
  lua_Number dmax = luaL_checknumber(L, 1);
  int i;
  for (i=2; i<=n; i++) {
    lua_Number d = luaL_checknumber(L, i);
    if (d > dmax)
      dmax = d;
  }
  lua_pushnumber(L, dmax);
  return 1;
}

// LIBRARY: SWBF2 0x0069d8d0
// math_random
static int math_random (lua_State *L) {

  lua_Number r = (lua_Number)(rand()%RAND_MAX) / (lua_Number)RAND_MAX;
  switch (lua_gettop(L)) {
    case 0: {
      lua_pushnumber(L, r);
      break;
    }
    case 1: {
      int u = luaL_checkint(L, 1);
      luaL_argcheck(L, 1<=u, 1, "interval is empty");
      lua_pushnumber(L, (int)floor(r*u)+1);
      break;
    }
    case 2: {
      int l = luaL_checkint(L, 1);
      int u = luaL_checkint(L, 2);
      luaL_argcheck(L, l<=u, 2, "interval is empty");
      lua_pushnumber(L, (int)floor(r*(u-l+1))+l);
      break;
    }
    default: return luaL_error(L, "wrong number of arguments");
  }
  return 1;
}

// LIBRARY: SWBF2 0x0069da60
// math_randomseed
static int math_randomseed (lua_State *L) {
  srand(luaL_checkint(L, 1));
  return 0;
}

static const luaL_reg mathlib[] = {
  {"abs",   math_abs},
  {"sin",   math_sin},
  {"cos",   math_cos},
  {"tan",   math_tan},
  {"asin",  math_asin},
  {"acos",  math_acos},
  {"atan",  math_atan},
  {"atan2", math_atan2},
  {"ceil",  math_ceil},
  {"floor", math_floor},
  {"mod",   math_mod},
  {"frexp", math_frexp},
  {"ldexp", math_ldexp},
  {"sqrt",  math_sqrt},
  {"min",   math_min},
  {"max",   math_max},
  {"log",   math_log},
  {"log10", math_log10},
  {"exp",   math_exp},
  {"deg",   math_deg},
  {"pow",   math_pow},
  {"rad",   math_rad},
  {"random",     math_random},
  {"randomseed", math_randomseed},
  {NULL, NULL}
};

// LIBRARY: SWBF2 0x0069da80
// luaopen_math
LUALIB_API int luaopen_math (lua_State *L) {
  luaL_openlib(L, LUA_MATHLIBNAME, mathlib, 0);
  lua_pushliteral(L, "pi");
  lua_pushnumber(L, PI);
  lua_settable(L, -3);
  lua_pushliteral(L, "__pow");
  lua_pushcfunction(L, math_pow);
  lua_settable(L, LUA_GLOBALSINDEX);
  return 1;
}

// LIBRARY: SWBF2 0x0075317c
// floor

// LIBRARY: SWBF2 0x0075312e
// _CIfmod
