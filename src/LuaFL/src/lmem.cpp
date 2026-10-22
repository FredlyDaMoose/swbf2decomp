#include <stdlib.h>

#define lmem_c

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"

#include "Battlefront2/Source/LuaMemory.h"

#ifndef l_realloc
#define l_realloc(b,os,s)	Lua_Red_realloc(b, os, s)
#endif

#ifndef l_free
#define l_free(b,os)	Lua_Red_Free(b, os)
#endif

#define MINSIZEARRAY	4

// LIBRARY: SWBF2 0x006a84b0
// luaM_growaux
void *luaM_growaux (lua_State *L, void *block, int *size, int size_elems,
                    int limit, const char *errormsg) {
  void *newblock;
  int newsize = (*size)*2;
  if (newsize < MINSIZEARRAY)
    newsize = MINSIZEARRAY;
  else if (*size >= limit/2) {
    if (*size < limit - MINSIZEARRAY)
      newsize = limit;
    else luaG_runerror(L, errormsg);
  }
  newblock = luaM_realloc(L, block,
                          cast(lu_mem, *size)*cast(lu_mem, size_elems),
                          cast(lu_mem, newsize)*cast(lu_mem, size_elems));
  *size = newsize;
  return newblock;
}

// LIBRARY: SWBF2 0x006a8520
// luaM_realloc
void *luaM_realloc (lua_State *L, void *block, lu_mem oldsize, lu_mem size) {
  lua_assert((oldsize == 0) == (block == NULL));
  if (size == 0) {
    if (block != NULL) {
      l_free(block, oldsize);
      block = NULL;
    }
    else return NULL;
  }
  else if (size >= MAX_SIZET)
    luaG_runerror(L, "memory allocation error: block too big");
  else {
    block = l_realloc(block, oldsize, size);
    if (block == NULL) {
      if (L)
        luaD_throw(L, LUA_ERRMEM);
      else return NULL;
    }
  }
  if (L) {
    lua_assert(G(L) != NULL && G(L)->nblocks > 0);
    G(L)->nblocks -= oldsize;
    G(L)->nblocks += size;
  }
  return block;
}
