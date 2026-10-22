#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define lauxlib_c

#include "lua.h"

#include "lauxlib.h"

#define RESERVED_REFS	2

#define FREELIST_REF	1
#define ARRAYSIZE_REF	2

#define abs_index(L, i)		((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : \
					lua_gettop(L) + (i) + 1)

// LIBRARY: SWBF2 0x0069ac40
// luaL_argerror
LUALIB_API int luaL_argerror (lua_State *L, int narg, const char *extramsg) {
  lua_Debug ar;
  lua_getstack(L, 0, &ar);
  lua_getinfo(L, "n", &ar);
  if (strcmp(ar.namewhat, "method") == 0) {
    narg--;
    if (narg == 0)
      return luaL_error(L, "calling `%s' on bad self (%s)", ar.name, extramsg);
  }
  if (ar.name == NULL)
    ar.name = "?";
  return luaL_error(L, "bad argument #%d to `%s' (%s)",
                        narg, ar.name, extramsg);
}

// LIBRARY: SWBF2 0x0069b5c0
// luaL_typerror
LUALIB_API int luaL_typerror (lua_State *L, int narg, const char *tname) {
  const char *msg = lua_pushfstring(L, "%s expected, got %s",
                                    tname, lua_typename(L, lua_type(L,narg)));
  return luaL_argerror(L, narg, msg);
}

// LIBRARY: SWBF2 0x0069b760
// tag_error
static void tag_error (lua_State *L, int narg, int tag) {
  luaL_typerror(L, narg, lua_typename(L, tag));
}

// LIBRARY: SWBF2 0x0069b670
// luaL_where
LUALIB_API void luaL_where (lua_State *L, int level) {
  lua_Debug ar;
  if (lua_getstack(L, level, &ar)) {
    lua_getinfo(L, "Snl", &ar);
    if (ar.currentline > 0) {
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      return;
    }
  }
  lua_pushliteral(L, "");
}

// LIBRARY: SWBF2 0x0069aec0
// luaL_error
LUALIB_API int luaL_error (lua_State *L, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  return lua_error(L);
}

LUALIB_API int luaL_findstring (const char *name, const char *const list[]) {
  int i;
  for (i=0; list[i]; i++)
    if (strcmp(list[i], name) == 0)
      return i;
  return -1;
}

// LIBRARY: SWBF2 0x0069b1e0
// luaL_newmetatable
LUALIB_API int luaL_newmetatable (lua_State *L, const char *tname) {
  lua_pushstring(L, tname);
  lua_rawget(L, LUA_REGISTRYINDEX);
  if (!lua_isnil(L, -1))
    return 0;
  lua_pop(L, 1);
  lua_newtable(L);
  lua_pushstring(L, tname);
  lua_pushvalue(L, -2);
  lua_rawset(L, LUA_REGISTRYINDEX);
  lua_pushvalue(L, -1);
  lua_pushstring(L, tname);
  lua_rawset(L, LUA_REGISTRYINDEX);
  return 1;
}

// LIBRARY: SWBF2 0x0069af50
// luaL_getmetatable
LUALIB_API void  luaL_getmetatable (lua_State *L, const char *tname) {
  lua_pushstring(L, tname);
  lua_rawget(L, LUA_REGISTRYINDEX);
}

LUALIB_API void *luaL_checkudata (lua_State *L, int ud, const char *tname) {
  const char *tn;
  if (!lua_getmetatable(L, ud)) return NULL;
  lua_rawget(L, LUA_REGISTRYINDEX);
  tn = lua_tostring(L, -1);
  if (tn && (strcmp(tn, tname) == 0)) {
    lua_pop(L, 1);
    return lua_touserdata(L, ud);
  }
  else {
    lua_pop(L, 1);
    return NULL;
  }
}

// LIBRARY: SWBF2 0x0069ae60
// luaL_checkstack
LUALIB_API void luaL_checkstack (lua_State *L, int space, const char *mes) {
  if (!lua_checkstack(L, space))
    luaL_error(L, "stack overflow (%s)", mes);
}

// LIBRARY: SWBF2 0x0069ae90
// luaL_checktype
LUALIB_API void luaL_checktype (lua_State *L, int narg, int t) {
  if (lua_type(L, narg) != t)
    tag_error(L, narg, t);
}

// LIBRARY: SWBF2 0x0069ad90
// luaL_checkany
LUALIB_API void luaL_checkany (lua_State *L, int narg) {
  if (lua_type(L, narg) == LUA_TNONE)
    luaL_argerror(L, narg, "value expected");
}

// LIBRARY: SWBF2 0x0069adc0
// luaL_checklstring
LUALIB_API const char *luaL_checklstring (lua_State *L, int narg, size_t *len) {
  const char *s = lua_tostring(L, narg);
  if (!s) tag_error(L, narg, LUA_TSTRING);
  if (len) *len = lua_strlen(L, narg);
  return s;
}

// LIBRARY: SWBF2 0x0069b340
// luaL_optlstring
LUALIB_API const char *luaL_optlstring (lua_State *L, int narg,
                                        const char *def, size_t *len) {
  if (lua_isnoneornil(L, narg)) {
    if (len)
      *len = (def ? strlen(def) : 0);
    return def;
  }
  else return luaL_checklstring(L, narg, len);
}

// LIBRARY: SWBF2 0x0069ae10
// luaL_checknumber
LUALIB_API lua_Number luaL_checknumber (lua_State *L, int narg) {
  lua_Number d = lua_tonumber(L, narg);
  if (d == 0 && !lua_isnumber(L, narg))
    tag_error(L, narg, LUA_TNUMBER);
  return d;
}

// LIBRARY: SWBF2 0x0069b3a0
// luaL_optnumber
LUALIB_API lua_Number luaL_optnumber (lua_State *L, int narg, lua_Number def) {
  if (lua_isnoneornil(L, narg)) return def;
  else return luaL_checknumber(L, narg);
}

// LIBRARY: SWBF2 0x0069aef0
// luaL_getmetafield
LUALIB_API int luaL_getmetafield (lua_State *L, int obj, const char *event) {
  if (!lua_getmetatable(L, obj))
    return 0;
  lua_pushstring(L, event);
  lua_rawget(L, -2);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 2);
    return 0;
  }
  else {
    lua_remove(L, -2);
    return 1;
  }
}

// LIBRARY: SWBF2 0x0069ad30
// luaL_callmeta
LUALIB_API int luaL_callmeta (lua_State *L, int obj, const char *event) {
  obj = abs_index(L, obj);
  if (!luaL_getmetafield(L, obj, event))
    return 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  return 1;
}

// LIBRARY: SWBF2 0x0069b260
// luaL_openlib
LUALIB_API void luaL_openlib (lua_State *L, const char *libname,
                              const luaL_reg *l, int nup) {
  if (libname) {
    lua_pushstring(L, libname);
    lua_gettable(L, LUA_GLOBALSINDEX);
    if (lua_isnil(L, -1)) {
      lua_pop(L, 1);
      lua_newtable(L);
      lua_pushstring(L, libname);
      lua_pushvalue(L, -2);
      lua_settable(L, LUA_GLOBALSINDEX);
    }
    lua_insert(L, -(nup+1));
  }
  for (; l->name; l++) {
    int i;
    lua_pushstring(L, l->name);
    for (i=0; i<nup; i++)
      lua_pushvalue(L, -(nup+1));
    lua_pushcclosure(L, l->func, nup);
    lua_settable(L, -(nup+3));
  }
  lua_pop(L, nup);
}

// LIBRARY: SWBF2 0x0069a980
// checkint
static int checkint (lua_State *L, int topop) {
  int n = (int)lua_tonumber(L, -1);
  if (n == 0 && !lua_isnumber(L, -1)) n = -1;
  lua_pop(L, topop);
  return n;
}

// LIBRARY: SWBF2 0x0069aae0
// getsizes
static void getsizes (lua_State *L) {
  lua_rawgeti(L, LUA_REGISTRYINDEX, ARRAYSIZE_REF);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -2);
    lua_pushliteral(L, "__mode");
    lua_pushliteral(L, "k");
    lua_rawset(L, -3);
    lua_pushvalue(L, -1);
    lua_rawseti(L, LUA_REGISTRYINDEX, ARRAYSIZE_REF);
  }
}

// LIBRARY: SWBF2 0x0069b500
// luaL_setn
void luaL_setn (lua_State *L, int t, int n) {
  t = abs_index(L, t);
  lua_pushliteral(L, "n");
  lua_rawget(L, t);
  if (checkint(L, 1) >= 0) {
    lua_pushliteral(L, "n");
    lua_pushnumber(L, (lua_Number)n);
    lua_rawset(L, t);
  }
  else {
    getsizes(L);
    lua_pushvalue(L, t);
    lua_pushnumber(L, (lua_Number)n);
    lua_rawset(L, -3);
    lua_pop(L, 1);
  }
}

// LIBRARY: SWBF2 0x0069af70
// luaL_getn
int luaL_getn (lua_State *L, int t) {
  int n;
  t = abs_index(L, t);
  lua_pushliteral(L, "n");
  lua_rawget(L, t);
  if ((n = checkint(L, 1)) >= 0) return n;
  getsizes(L);
  lua_pushvalue(L, t);
  lua_rawget(L, -2);
  if ((n = checkint(L, 2)) >= 0) return n;
  for (n = 1; ; n++) {
    lua_rawgeti(L, t, n);
    if (lua_isnil(L, -1)) break;
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
  return n - 1;
}

#define bufflen(B)	((B)->p - (B)->buffer)
#define bufffree(B)	((size_t)(LUAL_BUFFERSIZE - bufflen(B)))

#define LIMIT	(LUA_MINSTACK/2)

// LIBRARY: SWBF2 0x0069a9d0
// emptybuffer
static int emptybuffer (luaL_Buffer *B) {
  size_t l = bufflen(B);
  if (l == 0) return 0;
  else {
    lua_pushlstring(B->L, B->buffer, l);
    B->p = B->buffer;
    B->lvl++;
    return 1;
  }
}

// LIBRARY: SWBF2 0x0069a850
// adjuststack
static void adjuststack (luaL_Buffer *B) {
  if (B->lvl > 1) {
    lua_State *L = B->L;
    int toget = 1;
    size_t toplen = lua_strlen(L, -1);
    do {
      size_t l = lua_strlen(L, -(toget+1));
      if (B->lvl - toget + 1 >= LIMIT || toplen > l) {
        toplen += l;
        toget++;
      }
      else break;
    } while (toget < B->lvl);
    lua_concat(L, toget);
    B->lvl = B->lvl - toget + 1;
  }
}

// LIBRARY: SWBF2 0x0069b3d0
// luaL_prepbuffer
LUALIB_API char *luaL_prepbuffer (luaL_Buffer *B) {
  if (emptybuffer(B))
    adjuststack(B);
  return B->buffer;
}

// LIBRARY: SWBF2 0x0069ab70
// luaL_addlstring
LUALIB_API void luaL_addlstring (luaL_Buffer *B, const char *s, size_t l) {
  while (l--)
    luaL_putchar(B, *s++);
}

LUALIB_API void luaL_addstring (luaL_Buffer *B, const char *s) {
  luaL_addlstring(B, s, strlen(s));
}

// LIBRARY: SWBF2 0x0069b400
// luaL_pushresult
LUALIB_API void luaL_pushresult (luaL_Buffer *B) {
  emptybuffer(B);
  lua_concat(B->L, B->lvl);
  B->lvl = 1;
}

// LIBRARY: SWBF2 0x0069abc0
// luaL_addvalue
LUALIB_API void luaL_addvalue (luaL_Buffer *B) {
  lua_State *L = B->L;
  size_t vl = lua_strlen(L, -1);
  if (vl <= bufffree(B)) {
    memcpy(B->p, lua_tostring(L, -1), vl);
    B->p += vl;
    lua_pop(L, 1);
  }
  else {
    if (emptybuffer(B))
      lua_insert(L, -2);
    B->lvl++;
    adjuststack(B);
  }
}

// LIBRARY: SWBF2 0x0069ad10
// luaL_buffinit
LUALIB_API void luaL_buffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->p = B->buffer;
  B->lvl = 0;
}

// LIBRARY: SWBF2 0x0069b430
// luaL_ref
LUALIB_API int luaL_ref (lua_State *L, int t) {
  int ref;
  t = abs_index(L, t);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    return LUA_REFNIL;
  }
  lua_rawgeti(L, t, FREELIST_REF);
  ref = (int)lua_tonumber(L, -1);
  lua_pop(L, 1);
  if (ref != 0) {
    lua_rawgeti(L, t, ref);
    lua_rawseti(L, t, FREELIST_REF);
  }
  else {
    ref = luaL_getn(L, t);
    if (ref < RESERVED_REFS)
      ref = RESERVED_REFS;
    ref++;
    luaL_setn(L, t, ref);
  }
  lua_rawseti(L, t, ref);
  return ref;
}

// LIBRARY: SWBF2 0x0069b600
// luaL_unref
LUALIB_API void luaL_unref (lua_State *L, int t, int ref) {
  if (ref >= 0) {
    t = abs_index(L, t);
    lua_rawgeti(L, t, FREELIST_REF);
    lua_rawseti(L, t, ref);
    lua_pushnumber(L, (lua_Number)ref);
    lua_rawseti(L, t, FREELIST_REF);
  }
}

typedef struct LoadF {
  FILE *f;
  char buff[LUAL_BUFFERSIZE];
} LoadF;

// LIBRARY: SWBF2 0x0069aa60
// getF
static const char *getF (lua_State *L, void *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  (void)L;
  if (feof(lf->f)) return NULL;
  *size = fread(lf->buff, 1, LUAL_BUFFERSIZE, lf->f);
  return (*size > 0) ? lf->buff : NULL;
}

// LIBRARY: SWBF2 0x0069aa10
// errfile
static int errfile (lua_State *L, int fnameindex) {
  const char *filename = lua_tostring(L, fnameindex) + 1;
  lua_pushfstring(L, "cannot read %s: %s", filename, strerror(errno));
  lua_remove(L, fnameindex);
  return LUA_ERRFILE;
}

// LIBRARY: SWBF2 0x0069b060
// luaL_loadfile
LUALIB_API int luaL_loadfile (lua_State *L, const char *filename) {
  LoadF lf;
  int status, readstatus;
  int c;
  int fnameindex = lua_gettop(L) + 1;
  if (filename == NULL) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  else {
    lua_pushfstring(L, "@%s", filename);
    lf.f = fopen(filename, "r");
  }
  if (lf.f == NULL) return errfile(L, fnameindex);
  c = ungetc(getc(lf.f), lf.f);
  if (!(isspace(c) || isprint(c)) && lf.f != stdin) {
    fclose(lf.f);
    lf.f = fopen(filename, "rb");
    if (lf.f == NULL) return errfile(L, fnameindex);
  }
  status = lua_load(L, getF, &lf, lua_tostring(L, -1));
  readstatus = ferror(lf.f);
  if (lf.f != stdin) fclose(lf.f);
  if (readstatus) {
    lua_settop(L, fnameindex);
    return errfile(L, fnameindex);
  }
  lua_remove(L, fnameindex);
  return status;
}

typedef struct LoadS {
  const char *s;
  size_t size;
} LoadS;

// LIBRARY: SWBF2 0x0069aab0
// getS
static const char *getS (lua_State *L, void *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  (void)L;
  if (ls->size == 0) return NULL;
  *size = ls->size;
  ls->size = 0;
  return ls->s;
}

// LIBRARY: SWBF2 0x0069b030
// luaL_loadbuffer
LUALIB_API int luaL_loadbuffer (lua_State *L, const char *buff, size_t size,
                                const char *name) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  return lua_load(L, getS, &ls, name);
}

// LIBRARY: SWBF2 0x0069a900
// callalert
static void callalert (lua_State *L, int status) {
  if (status != 0) {
    lua_getglobal(L, "_ALERT");
    if (lua_isfunction(L, -1)) {
      lua_insert(L, -2);
      lua_call(L, 1, 0);
    }
    else {
      fprintf(stderr, "%s\n", lua_tostring(L, -2));
      lua_pop(L, 2);
    }
  }
}

// LIBRARY: SWBF2 0x0069a8d0
// aux_do
static int aux_do (lua_State *L, int status) {
  if (status == 0) {
    status = lua_pcall(L, 0, LUA_MULTRET, 0);
  }
  callalert(L, status);
  return status;
}

LUALIB_API int lua_dofile (lua_State *L, const char *filename) {
  return aux_do(L, luaL_loadfile(L, filename));
}

// LIBRARY: SWBF2 0x0069b700
// lua_dobuffer
LUALIB_API int lua_dobuffer (lua_State *L, const char *buff, size_t size,
                          const char *name) {
  return aux_do(L, luaL_loadbuffer(L, buff, size, name));
}

// LIBRARY: SWBF2 0x0069b730
// lua_dostring
LUALIB_API int lua_dostring (lua_State *L, const char *str) {
  return lua_dobuffer(L, str, strlen(str), str);
}

// LIBRARY: SWBF2 0x00753032
// __iob_func
