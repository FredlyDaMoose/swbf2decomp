#ifndef lparser_h
#define lparser_h

#include "llimits.h"
#include "lobject.h"
#include "ltable.h"
#include "lzio.h"

typedef enum {
  VVOID,
  VNIL,
  VTRUE,
  VFALSE,
  VK,
  VLOCAL,
  VUPVAL,
  VGLOBAL,
  VINDEXED,
  VJMP,
  VRELOCABLE,
  VNONRELOC,
  VCALL
} expkind;

typedef struct expdesc {
  expkind k;
  int info, aux;
  int t;
  int f;
} expdesc;

struct BlockCnt;

typedef struct FuncState {
  Proto *f;
  Table *h;
  struct FuncState *prev;
  struct LexState *ls;
  struct lua_State *L;
  struct BlockCnt *bl;
  int pc;
  int lasttarget;
  int jpc;
  int freereg;
  int nk;
  int np;
  int nlocvars;
  int nactvar;
  expdesc upvalues[MAXUPVALUES];
  int actvar[MAXVARS];
} FuncState;

Proto *luaY_parser (lua_State *L, ZIO *z, Mbuffer *buff);

#endif
