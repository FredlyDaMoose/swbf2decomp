#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

#define ldo_c

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lparser.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lundump.h"
#include "lvm.h"
#include "lzio.h"

struct lua_longjmp {
  struct lua_longjmp *previous;
  jmp_buf b;
  volatile int status;
};

// LIBRARY: SWBF2 0x006a36b0
// seterrorobj
static void seterrorobj (lua_State *L, int errcode, StkId oldtop) {
  switch (errcode) {
    case LUA_ERRMEM: {
      setsvalue2s(oldtop, luaS_new(L, MEMERRMSG));
      break;
    }
    case LUA_ERRERR: {
      setsvalue2s(oldtop, luaS_new(L, "error in error handling"));
      break;
    }
    case LUA_ERRSYNTAX:
    case LUA_ERRRUN: {
      setobjs2s(oldtop, L->top - 1);
      break;
    }
  }
  L->top = oldtop + 1;
}

// LIBRARY: SWBF2 0x006a33f0
// luaD_throw
void luaD_throw (lua_State *L, int errcode) {
  if (L->errorJmp) {
    L->errorJmp->status = errcode;
    longjmp(L->errorJmp->b, 1);
  }
  else {

    G(L)->panic(L);
  }
}

// LIBRARY: SWBF2 0x006a32a0
// luaD_rawrunprotected
int luaD_rawrunprotected (lua_State *L, Pfunc f, void *ud) {
  struct lua_longjmp lj;
  lj.status = 0;
  lj.previous = L->errorJmp;
  L->errorJmp = &lj;
  if (setjmp(lj.b) == 0)
    (*f)(L, ud);
  L->errorJmp = lj.previous;
  return lj.status;
}

// LIBRARY: SWBF2 0x006a3560
// restore_stack_limit
static void restore_stack_limit (lua_State *L) {
  L->stack_last = L->stack+L->stacksize-1;
  if (L->size_ci > LUA_MAXCALLS) {
    int inuse = (L->ci - L->base_ci);
    if (inuse + 1 < LUA_MAXCALLS)
      luaD_reallocCI(L, LUA_MAXCALLS);
  }
}

// FUNCTION: SWBF2 0x006a2ca0
static void correctstack (lua_State *L, TObject *oldstack) {
  CallInfo *ci;
  GCObject *up;
  L->top = (L->top - oldstack) + L->stack;
  for (up = L->openupval; up != NULL; up = up->gch.next)
    gcotouv(up)->v = (gcotouv(up)->v - oldstack) + L->stack;
  for (ci = L->base_ci; ci <= L->ci; ci++) {
    ci->top = (ci->top - oldstack) + L->stack;
    ci->base = (ci->base - oldstack) + L->stack;
  }
  L->base = L->ci->base;
}

// LIBRARY: SWBF2 0x006a33a0
// luaD_reallocstack
void luaD_reallocstack (lua_State *L, int newsize) {
  TObject *oldstack = L->stack;
  luaM_reallocvector(L, L->stack, L->stacksize, newsize, TObject);
  L->stacksize = newsize;
  L->stack_last = L->stack+newsize-1-EXTRA_STACK;
  correctstack(L, oldstack);
}

// LIBRARY: SWBF2 0x006a3330
// luaD_reallocCI
void luaD_reallocCI (lua_State *L, int newsize) {
  CallInfo *oldci = L->base_ci;
  luaM_reallocvector(L, L->base_ci, L->size_ci, newsize, CallInfo);
  L->size_ci = cast(unsigned short, newsize);
  L->ci = (L->ci - oldci) + L->base_ci;
  L->end_ci = L->base_ci + L->size_ci;
}

// LIBRARY: SWBF2 0x006a2f60
// luaD_growstack
void luaD_growstack (lua_State *L, int n) {
  if (n <= L->stacksize)
    luaD_reallocstack(L, 2*L->stacksize);
  else
    luaD_reallocstack(L, L->stacksize + n + EXTRA_STACK);
}

// LIBRARY: SWBF2 0x006a2f10
// luaD_growCI
static void luaD_growCI (lua_State *L) {
  if (L->size_ci > LUA_MAXCALLS)
    luaD_throw(L, LUA_ERRERR);
  else {
    luaD_reallocCI(L, 2*L->size_ci);
    if (L->size_ci > LUA_MAXCALLS)
      luaG_runerror(L, "stack overflow");
  }
}

// LIBRARY: SWBF2 0x006a2e30
// luaD_callhook
void luaD_callhook (lua_State *L, int event, int line) {
  lua_Hook hook = L->hook;
  if (hook && L->allowhook) {
    ptrdiff_t top = savestack(L, L->top);
    ptrdiff_t ci_top = savestack(L, L->ci->top);
    lua_Debug ar;
    ar.event = event;
    ar.currentline = line;
    if (event == LUA_HOOKTAILRET)
      ar.i_ci = 0;
    else
      ar.i_ci = L->ci - L->base_ci;
    luaD_checkstack(L, LUA_MINSTACK);
    L->ci->top = L->top + LUA_MINSTACK;
    L->allowhook = 0;
    lua_unlock(L);
    (*hook)(L, &ar);
    lua_lock(L);
    lua_assert(!L->allowhook);
    L->allowhook = 1;
    L->ci->top = restorestack(L, ci_top);
    L->top = restorestack(L, top);
  }
}

// FUNCTION: SWBF2 0x006a2b20
static void adjust_varargs (lua_State *L, int nfixargs, StkId base) {
  int i;
  Table *htab;
  TObject nname;
  int actual = L->top - base;
  if (actual < nfixargs) {
    luaD_checkstack(L, nfixargs - actual);
    for (; actual < nfixargs; ++actual)
      setnilvalue(L->top++);
  }
  actual -= nfixargs;
  htab = luaH_new(L, actual, 1);
  for (i=0; i<actual; i++)
    setobj2n(luaH_setnum(L, htab, i+1), L->top - actual + i);

  setsvalue(&nname, luaS_newliteral(L, "n"));
  setnvalue(luaH_set(L, htab, &nname), cast(lua_Number, actual));
  L->top -= actual;
  sethvalue(L->top, htab);
  incr_top(L);
}

// LIBRARY: SWBF2 0x006a3740
// tryfuncTM
static StkId tryfuncTM (lua_State *L, StkId func) {
  const TObject *tm = luaT_gettmbyobj(L, func, TM_CALL);
  StkId p;
  ptrdiff_t funcr = savestack(L, func);
  if (!ttisfunction(tm))
    luaG_typeerror(L, func, "call");

  for (p = L->top; p > func; p--) setobjs2s(p, p-1);
  incr_top(L);
  func = restorestack(L, funcr);
  setobj2s(func, tm);
  return func;
}

// LIBRARY: SWBF2 0x006a30c0
// luaD_precall
StkId luaD_precall (lua_State *L, StkId func) {
  LClosure *cl;
  ptrdiff_t funcr = savestack(L, func);
  if (!ttisfunction(func))
    func = tryfuncTM(L, func);
  if (L->ci + 1 == L->end_ci) luaD_growCI(L);
  else condhardstacktests(luaD_reallocCI(L, L->size_ci));
  cl = &clvalue(func)->l;
  if (!cl->isC) {
    CallInfo *ci;
    Proto *p = cl->p;
    if (p->is_vararg)
      adjust_varargs(L, p->numparams, func+1);
    luaD_checkstack(L, p->maxstacksize);
    ci = ++L->ci;
    L->base = L->ci->base = restorestack(L, funcr) + 1;
    ci->top = L->base + p->maxstacksize;
    ci->u.l.savedpc = p->code;
    ci->u.l.tailcalls = 0;
    ci->state = CI_SAVEDPC;
    while (L->top < ci->top)
      setnilvalue(L->top++);
    L->top = ci->top;
    return NULL;
  }
  else {
    CallInfo *ci;
    int n;
    luaD_checkstack(L, LUA_MINSTACK);
    ci = ++L->ci;
    L->base = L->ci->base = restorestack(L, funcr) + 1;
    ci->top = L->top + LUA_MINSTACK;
    ci->state = CI_C;
    if (L->hookmask & LUA_MASKCALL)
      luaD_callhook(L, LUA_HOOKCALL, -1);
    lua_unlock(L);
#ifdef LUA_COMPATUPVALUES
    lua_pushupvalues(L);
#endif
    n = (*clvalue(L->base - 1)->c.f)(L);
    lua_lock(L);
    return L->top - n;
  }
}

// FUNCTION: SWBF2 0x006a2c40
static StkId callrethooks (lua_State *L, StkId firstResult) {
  ptrdiff_t fr = savestack(L, firstResult);
  luaD_callhook(L, LUA_HOOKRET, -1);
  if (!(L->ci->state & CI_C)) {
    while (L->ci->u.l.tailcalls--)
      luaD_callhook(L, LUA_HOOKTAILRET, -1);
  }
  return restorestack(L, fr);
}

// LIBRARY: SWBF2 0x006a3030
// luaD_poscall
void luaD_poscall (lua_State *L, int wanted, StkId firstResult) {
  StkId res;
  if (L->hookmask & LUA_MASKRET)
    firstResult = callrethooks(L, firstResult);
  res = L->base - 1;
  L->ci--;
  L->base = L->ci->base;

  while (wanted != 0 && firstResult < L->top) {
    setobjs2s(res++, firstResult++);
    wanted--;
  }
  while (wanted-- > 0)
    setnilvalue(res++);
  L->top = res;
}

// LIBRARY: SWBF2 0x006a2da0
// luaD_call
void luaD_call (lua_State *L, StkId func, int nResults) {
  StkId firstResult;
  lua_assert(!(L->ci->state & CI_CALLING));
  if (++L->nCcalls >= LUA_MAXCCALLS) {
    if (L->nCcalls == LUA_MAXCCALLS)
      luaG_runerror(L, "C stack overflow");
    else if (L->nCcalls >= (LUA_MAXCCALLS + (LUA_MAXCCALLS>>3)))
      luaD_throw(L, LUA_ERRERR);
  }
  firstResult = luaD_precall(L, func);
  if (firstResult == NULL)
    firstResult = luaV_execute(L);
  luaD_poscall(L, nResults, firstResult);
  L->nCcalls--;
  luaC_checkGC(L);
}

// LIBRARY: SWBF2 0x006a35c0
// resume
static void resume (lua_State *L, void *ud) {
  StkId firstResult;
  int nargs = *cast(int *, ud);
  CallInfo *ci = L->ci;
  if (ci == L->base_ci) {
    lua_assert(nargs < L->top - L->base);
    luaD_precall(L, L->top - (nargs + 1));
  }
  else {
    lua_assert(ci->state & CI_YIELD);
    if (ci->state & CI_C) {

      int nresults;
      lua_assert((ci-1)->state & CI_SAVEDPC);
      lua_assert(GET_OPCODE(*((ci-1)->u.l.savedpc - 1)) == OP_CALL ||
                 GET_OPCODE(*((ci-1)->u.l.savedpc - 1)) == OP_TAILCALL);
      nresults = GETARG_C(*((ci-1)->u.l.savedpc - 1)) - 1;
      luaD_poscall(L, nresults, L->top - nargs);
      if (nresults >= 0) L->top = L->ci->top;
    }
    else {
      ci->state &= ~CI_YIELD;
    }
  }
  firstResult = luaV_execute(L);
  if (firstResult != NULL)
    luaD_poscall(L, LUA_MULTRET, firstResult);
}

// LIBRARY: SWBF2 0x006a3650
// resume_error
static int resume_error (lua_State *L, const char *msg) {
  L->top = L->ci->base;
  setsvalue2s(L->top, luaS_new(L, msg));
  incr_top(L);
  lua_unlock(L);
  return LUA_ERRRUN;
}

// LIBRARY: SWBF2 0x006a3420
// lua_resume
LUA_API int lua_resume (lua_State *L, int nargs) {
  int status;
  lu_byte old_allowhooks;
  lua_lock(L);
  if (L->ci == L->base_ci) {
    if (nargs >= L->top - L->base)
      return resume_error(L, "cannot resume dead coroutine");
  }
  else if (!(L->ci->state & CI_YIELD))
    return resume_error(L, "cannot resume non-suspended coroutine");
  old_allowhooks = L->allowhook;
  lua_assert(L->errfunc == 0 && L->nCcalls == 0);
  status = luaD_rawrunprotected(L, resume, &nargs);
  if (status != 0) {
    L->ci = L->base_ci;
    L->base = L->ci->base;
    L->nCcalls = 0;
    luaF_close(L, L->base);
    seterrorobj(L, status, L->base);
    L->allowhook = old_allowhooks;
    restore_stack_limit(L);
  }
  lua_unlock(L);
  return status;
}

// LIBRARY: SWBF2 0x006a34c0
// lua_yield
LUA_API int lua_yield (lua_State *L, int nresults) {
  CallInfo *ci;
  lua_lock(L);
  ci = L->ci;
  if (L->nCcalls > 0)
    luaG_runerror(L, "attempt to yield across metamethod/C-call boundary");
  if (ci->state & CI_C) {
    if ((ci-1)->state & CI_C)
      luaG_runerror(L, "cannot yield a C function");
    if (L->top - nresults > L->base) {
      int i;
      for (i=0; i<nresults; i++)
        setobjs2s(L->base + i, L->top - nresults + i);
      L->top = L->base + nresults;
    }
  }
  ci->state |= CI_YIELD;
  lua_unlock(L);
  return -1;
}

// LIBRARY: SWBF2 0x006a2f90
// luaD_pcall
int luaD_pcall (lua_State *L, Pfunc func, void *u,
                ptrdiff_t old_top, ptrdiff_t ef) {
  int status;
  unsigned short oldnCcalls = L->nCcalls;
  ptrdiff_t old_ci = saveci(L, L->ci);
  lu_byte old_allowhooks = L->allowhook;
  ptrdiff_t old_errfunc = L->errfunc;
  L->errfunc = ef;
  status = luaD_rawrunprotected(L, func, u);
  if (status != 0) {
    StkId oldtop = restorestack(L, old_top);
    luaF_close(L, oldtop);
    seterrorobj(L, status, oldtop);
    L->nCcalls = oldnCcalls;
    L->ci = restoreci(L, old_ci);
    L->base = L->ci->base;
    L->allowhook = old_allowhooks;
    restore_stack_limit(L);
  }
  L->errfunc = old_errfunc;
  return status;
}

struct SParser {
  ZIO *z;
  Mbuffer buff;
  int bin;
};

// LIBRARY: SWBF2 0x006a2d20
// f_parser
static void f_parser (lua_State *L, void *ud) {
  struct SParser *p;
  Proto *tf;
  Closure *cl;
  luaC_checkGC(L);
  p = cast(struct SParser *, ud);
  tf = p->bin ? luaU_undump(L, p->z, &p->buff) : luaY_parser(L, p->z, &p->buff);
  cl = luaF_newLclosure(L, 0, gt(L));
  cl->l.p = tf;
  setclvalue(L->top, cl);
  incr_top(L);
}

// LIBRARY: SWBF2 0x006a3220
// luaD_protectedparser
int luaD_protectedparser (lua_State *L, ZIO *z, int bin) {
  struct SParser p;
  int status;
  ptrdiff_t oldtopr = savestack(L, L->top);
  p.z = z; p.bin = bin;
  luaZ_initbuffer(L, &p.buff);
  status = luaD_rawrunprotected(L, f_parser, &p);
  luaZ_freebuffer(L, &p.buff);
  if (status != 0) {
    StkId oldtop = restorestack(L, oldtopr);
    seterrorobj(L, status, oldtop);
  }
  return status;
}

// LIBRARY: SWBF2 0x007535a6
// _setjmp3
