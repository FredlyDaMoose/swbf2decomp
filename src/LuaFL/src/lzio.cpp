#include <string.h>

#define lzio_c

#include "lua.h"

#include "llimits.h"
#include "lmem.h"
#include "lzio.h"

// LIBRARY: SWBF2 0x006a29c0
// luaZ_fill
int luaZ_fill (ZIO *z) {
  size_t size;
  const char *buff = z->reader(NULL, z->data, &size);
  if (buff == NULL || size == 0) return EOZ;
  z->n = size - 1;
  z->p = buff;
  return char2int(*(z->p++));
}

int luaZ_lookahead (ZIO *z) {
  if (z->n == 0) {
    int c = luaZ_fill(z);
    if (c == EOZ) return c;
    z->n++;
    z->p--;
  }
  return char2int(*z->p);
}

void luaZ_init (ZIO *z, lua_Chunkreader reader, void *data, const char *name) {
  z->reader = reader;
  z->data = data;
  z->name = name;
  z->n = 0;
  z->p = NULL;
}

// LIBRARY: SWBF2 0x006a2ab0
// luaZ_read
size_t luaZ_read (ZIO *z, void *b, size_t n) {
  while (n) {
    size_t m;
    if (z->n == 0) {
      if (luaZ_fill(z) == EOZ)
        return n;
      else {
        ++z->n;
        --z->p;
      }
    }
    m = (n <= z->n) ? n : z->n;
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (char *)b + m;
    n -= m;
  }
  return 0;
}

// LIBRARY: SWBF2 0x006a2a70
// luaZ_openspace
char *luaZ_openspace (lua_State *L, Mbuffer *buff, size_t n) {
  if (n > buff->buffsize) {
    if (n < LUA_MINBUFFER) n = LUA_MINBUFFER;
    luaM_reallocvector(L, buff->buffer, buff->buffsize, n, char);
    buff->buffsize = n;
  }
  return buff->buffer;
}
