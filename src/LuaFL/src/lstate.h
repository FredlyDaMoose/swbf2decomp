#ifndef lstate_h
#define lstate_h

#include "lua.h"

#include "lobject.h"
#include "ltm.h"
#include "lzio.h"

#ifndef lua_lock
#define lua_lock(L)	((void) 0)
#endif

#ifndef lua_unlock
#define lua_unlock(L)	((void) 0)
#endif

#ifndef lua_userstateopen
#define lua_userstateopen(l)
#endif

struct lua_longjmp;

#define defaultmeta(L)	(&G(L)->_defaultmeta)

#define gt(L)	(&L->_gt)

#define registry(L)	(&G(L)->_registry)

#define EXTRA_STACK   5

#define BASIC_CI_SIZE           8

#define BASIC_STACK_SIZE        (2*LUA_MINSTACK)

typedef struct stringtable {
  GCObject **hash;
  ls_nstr nuse;
  int size;
} stringtable;

typedef struct CallInfo {
  StkId base;
  StkId	top;
  int state;
  union {
    struct {
      const Instruction *savedpc;
      const Instruction **pc;
      int tailcalls;
    } l;
    struct {
      int dummy;
    } c;
  } u;
} CallInfo;

#define CI_C		(1<<0)

#define CI_HASFRAME	(1<<1)

#define CI_CALLING	(1<<2)
#define CI_SAVEDPC	(1<<3)
#define CI_YIELD	(1<<4)

#define ci_func(ci)	(clvalue((ci)->base - 1))

typedef struct global_State {
  stringtable strt;
  GCObject *rootgc;
  GCObject *rootudata;
  GCObject *tmudata;
  Mbuffer buff;
  lu_mem GCthreshold;
  lu_mem nblocks;
  lua_CFunction panic;
  TObject _registry;
  TObject _defaultmeta;
  struct lua_State *mainthread;
  Node dummynode[1];
  TString *tmname[TM_N];
} global_State;

struct lua_State {
  CommonHeader;
  StkId top;
  StkId base;
  global_State *l_G;
  CallInfo *ci;
  StkId stack_last;
  StkId stack;
  int stacksize;
  CallInfo *end_ci;
  CallInfo *base_ci;
  unsigned short size_ci;
  unsigned short nCcalls;
  lu_byte hookmask;
  lu_byte allowhook;
  lu_byte hookinit;
  int basehookcount;
  int hookcount;
  lua_Hook hook;
  TObject _gt;
  GCObject *openupval;
  GCObject *gclist;
  struct lua_longjmp *errorJmp;
  ptrdiff_t errfunc;
};

#define G(L)	(L->l_G)

union GCObject {
  GCheader gch;
  union TString ts;
  union Udata u;
  union Closure cl;
  struct Table h;
  struct Proto p;
  struct UpVal uv;
  struct lua_State th;
};

#define gcotots(o)	check_exp((o)->gch.tt == LUA_TSTRING, &((o)->ts))
#define gcotou(o)	check_exp((o)->gch.tt == LUA_TUSERDATA, &((o)->u))
#define gcotocl(o)	check_exp((o)->gch.tt == LUA_TFUNCTION, &((o)->cl))
#define gcotoh(o)	check_exp((o)->gch.tt == LUA_TTABLE, &((o)->h))
#define gcotop(o)	check_exp((o)->gch.tt == LUA_TPROTO, &((o)->p))
#define gcotouv(o)	check_exp((o)->gch.tt == LUA_TUPVAL, &((o)->uv))
#define ngcotouv(o) \
	check_exp((o) == NULL || (o)->gch.tt == LUA_TUPVAL, &((o)->uv))
#define gcototh(o)	check_exp((o)->gch.tt == LUA_TTHREAD, &((o)->th))

#define valtogco(v)	(cast(GCObject *, (v)))

lua_State *luaE_newthread (lua_State *L);
void luaE_freethread (lua_State *L, lua_State *L1);

#endif
