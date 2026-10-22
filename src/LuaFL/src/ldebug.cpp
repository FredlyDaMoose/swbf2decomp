#include <stdlib.h>
#include <string.h>

#define ldebug_c

#include "lua.h"

#include "lapi.h"
#include "lcode.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lvm.h"

static const char *getfuncname (CallInfo *ci, const char **name);

#define isLua(ci)	(!((ci)->state & CI_C))

// LIBRARY: SWBF2 0x006a18d0
// currentpc
static int currentpc (CallInfo *ci) {
  if (!isLua(ci)) return -1;
  if (ci->state & CI_HASFRAME)
    ci->u.l.savedpc = *ci->u.l.pc;

  return pcRel(ci->u.l.savedpc, ci_func(ci)->l.p);
}

// LIBRARY: SWBF2 0x006a1890
// currentline
static int currentline (CallInfo *ci) {
  int pc = currentpc(ci);
  if (pc < 0)
    return -1;
  else
    return getline(ci_func(ci)->l.p, pc);
}

// LIBRARY: SWBF2 0x006a1cd0
// luaG_inithooks
void luaG_inithooks (lua_State *L) {
  CallInfo *ci;
  for (ci = L->ci; ci != L->base_ci; ci--)
    currentpc(ci);
  L->hookinit = 1;
}

// LIBRARY: SWBF2 0x006a2340
// lua_sethook
LUA_API int lua_sethook (lua_State *L, lua_Hook func, int mask, int count) {
  if (func == NULL || mask == 0) {
    mask = 0;
    func = NULL;
  }
  L->hook = func;
  L->basehookcount = count;
  resethookcount(L);
  L->hookmask = cast(lu_byte, mask);
  L->hookinit = 0;
  return 1;
}

// LIBRARY: SWBF2 0x006a2170
// lua_gethook
LUA_API lua_Hook lua_gethook (lua_State *L) {
  return L->hook;
}

// LIBRARY: SWBF2 0x006a2190
// lua_gethookmask
LUA_API int lua_gethookmask (lua_State *L) {
  return L->hookmask;
}

// LIBRARY: SWBF2 0x006a2180
// lua_gethookcount
LUA_API int lua_gethookcount (lua_State *L) {
  return L->basehookcount;
}

// LIBRARY: SWBF2 0x006a22c0
// lua_getstack
LUA_API int lua_getstack (lua_State *L, int level, lua_Debug *ar) {
  int status;
  CallInfo *ci;
  lua_lock(L);
  for (ci = L->ci; level > 0 && ci > L->base_ci; ci--) {
    level--;
    if (!(ci->state & CI_C))
      level -= ci->u.l.tailcalls;
  }
  if (level > 0 || ci == L->base_ci) status = 0;
  else if (level < 0) {
    status = 1;
    ar->i_ci = 0;
  }
  else {
    status = 1;
    ar->i_ci = ci - L->base_ci;
  }
  lua_unlock(L);
  return status;
}

// LIBRARY: SWBF2 0x006a19f0
// getluaproto
static Proto *getluaproto (CallInfo *ci) {
  return (isLua(ci) ? ci_func(ci)->l.p : NULL);
}

// LIBRARY: SWBF2 0x006a2250
// lua_getlocal
LUA_API const char *lua_getlocal (lua_State *L, const lua_Debug *ar, int n) {
  const char *name;
  CallInfo *ci;
  Proto *fp;
  lua_lock(L);
  name = NULL;
  ci = L->base_ci + ar->i_ci;
  fp = getluaproto(ci);
  if (fp) {
    name = luaF_getlocalname(fp, n, currentpc(ci));
    if (name)
      luaA_pushobject(L, ci->base+(n-1));
  }
  lua_unlock(L);
  return name;
}

// LIBRARY: SWBF2 0x006a2380
// lua_setlocal
LUA_API const char *lua_setlocal (lua_State *L, const lua_Debug *ar, int n) {
  const char *name;
  CallInfo *ci;
  Proto *fp;
  lua_lock(L);
  name = NULL;
  ci = L->base_ci + ar->i_ci;
  fp = getluaproto(ci);
  L->top--;
  if (fp) {
    name = luaF_getlocalname(fp, n, currentpc(ci));
    if (!name || name[0] == '(')
      name = NULL;
    else
      setobjs2s(ci->base+(n-1), L->top);
  }
  lua_unlock(L);
  return name;
}

// LIBRARY: SWBF2 0x006a1910
// funcinfo
static void funcinfo (lua_Debug *ar, StkId func) {
  Closure *cl = clvalue(func);
  if (cl->c.isC) {
    ar->source = "=[C]";
    ar->linedefined = -1;
    ar->what = "C";
  }
  else {
    ar->source = getstr(cl->l.p->source);
    ar->linedefined = cl->l.p->lineDefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, LUA_IDSIZE);
}

// LIBRARY: SWBF2 0x006a2440
// travglobals
static const char *travglobals (lua_State *L, const TObject *o) {
  Table *g = hvalue(gt(L));
  int i = sizenode(g);
  while (i--) {
    Node *n = gnode(g, i);
    if (luaO_rawequalObj(o, gval(n)) && ttisstring(gkey(n)))
      return getstr(tsvalue(gkey(n)));
  }
  return NULL;
}

// LIBRARY: SWBF2 0x006a1b20
// info_tailcall
static void info_tailcall (lua_State *L, lua_Debug *ar) {
  ar->name = ar->namewhat = "";
  ar->what = "tail";
  ar->linedefined = ar->currentline = -1;
  ar->source = "=(tail call)";
  luaO_chunkid(ar->short_src, ar->source, LUA_IDSIZE);
  ar->nups = 0;
  setnilvalue(L->top);
}

// LIBRARY: SWBF2 0x006a16f0
// auxgetinfo
static int auxgetinfo (lua_State *L, const char *what, lua_Debug *ar,
                    StkId f, CallInfo *ci) {
  int status = 1;
  for (; *what; what++) {
    switch (*what) {
      case 'S': {
        funcinfo(ar, f);
        break;
      }
      case 'l': {
        ar->currentline = (ci) ? currentline(ci) : -1;
        break;
      }
      case 'u': {
        ar->nups = clvalue(f)->c.nupvalues;
        break;
      }
      case 'n': {
        ar->namewhat = (ci) ? getfuncname(ci, &ar->name) : NULL;
        if (ar->namewhat == NULL) {

          if ((ar->name = travglobals(L, f)) != NULL)
            ar->namewhat = "global";
          else ar->namewhat = "";
        }
        break;
      }
      case 'f': {
        setobj2s(L->top, f);
        break;
      }
      default: status = 0;
    }
  }
  return status;
}

// LIBRARY: SWBF2 0x006a21a0
// lua_getinfo
LUA_API int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar) {
  int status = 1;
  lua_lock(L);
  if (*what == '>') {
    StkId f = L->top - 1;
    if (!ttisfunction(f))
      luaG_runerror(L, "value for `lua_getinfo' is not a function");
    status = auxgetinfo(L, what + 1, ar, f, NULL);
    L->top--;
  }
  else if (ar->i_ci != 0) {
    CallInfo *ci = L->base_ci + ar->i_ci;
    lua_assert(ttisfunction(ci->base - 1));
    status = auxgetinfo(L, what, ar, ci->base - 1, ci);
  }
  else
    info_tailcall(L, ar);
  if (strchr(what, 'f')) incr_top(L);
  lua_unlock(L);
  return status;
}

#define check(x)		if (!(x)) return 0;

#define checkjump(pt,pc)	check(0 <= pc && pc < pt->sizecode)

#define checkreg(pt,reg)	check((reg) < (pt)->maxstacksize)

// LIBRARY: SWBF2 0x006a2400
// precheck
static int precheck (const Proto *pt) {
  check(pt->maxstacksize <= MAXSTACK);
  check(pt->sizelineinfo == pt->sizecode || pt->sizelineinfo == 0);
  lua_assert(pt->numparams+pt->is_vararg <= pt->maxstacksize);
  check(GET_OPCODE(pt->code[pt->sizecode-1]) == OP_RETURN);
  return 1;
}

// LIBRARY: SWBF2 0x006a1850
// checkopenop
static int checkopenop (const Proto *pt, int pc) {
  Instruction i = pt->code[pc+1];
  switch (GET_OPCODE(i)) {
    case OP_CALL:
    case OP_TAILCALL:
    case OP_RETURN: {
      check(GETARG_B(i) == 0);
      return 1;
    }
    case OP_SETLISTO: return 1;
    default: return 0;
  }
}

// LIBRARY: SWBF2 0x006a1820
// checkRK
static int checkRK (const Proto *pt, int r) {
  return (r < pt->maxstacksize || (r >= MAXSTACK && r-MAXSTACK < pt->sizek));
}

// LIBRARY: SWBF2 0x006a1d80
// luaG_symbexec
static Instruction luaG_symbexec (const Proto *pt, int lastpc, int reg) {
  int pc;
  int last;
  last = pt->sizecode-1;
  check(precheck(pt));
  for (pc = 0; pc < lastpc; pc++) {
    const Instruction i = pt->code[pc];
    OpCode op = GET_OPCODE(i);
    int a = GETARG_A(i);
    int b = 0;
    int c = 0;
    checkreg(pt, a);
    switch (getOpMode(op)) {
      case iABC: {
        b = GETARG_B(i);
        c = GETARG_C(i);
        if (testOpMode(op, OpModeBreg)) {
          checkreg(pt, b);
        }
        else if (testOpMode(op, OpModeBrk))
          check(checkRK(pt, b));
        if (testOpMode(op, OpModeCrk))
          check(checkRK(pt, c));
        break;
      }
      case iABx: {
        b = GETARG_Bx(i);
        if (testOpMode(op, OpModeK)) check(b < pt->sizek);
        break;
      }
      case iAsBx: {
        b = GETARG_sBx(i);
        break;
      }
    }
    if (testOpMode(op, OpModesetA)) {
      if (a == reg) last = pc;
    }
    if (testOpMode(op, OpModeT)) {
      check(pc+2 < pt->sizecode);
      check(GET_OPCODE(pt->code[pc+1]) == OP_JMP);
    }
    switch (op) {
      case OP_LOADBOOL: {
        check(c == 0 || pc+2 < pt->sizecode);
        break;
      }
      case OP_LOADNIL: {
        if (a <= reg && reg <= b)
          last = pc;
        break;
      }
      case OP_GETUPVAL:
      case OP_SETUPVAL: {
        check(b < pt->nups);
        break;
      }
      case OP_GETGLOBAL:
      case OP_SETGLOBAL: {
        check(ttisstring(&pt->k[b]));
        break;
      }
      case OP_SELF: {
        checkreg(pt, a+1);
        if (reg == a+1) last = pc;
        break;
      }
      case OP_CONCAT: {

        check(c < MAXSTACK && b < c);
        break;
      }
      case OP_TFORLOOP:
        checkreg(pt, a+c+5);
        if (reg >= a) last = pc;

      case OP_FORLOOP:
        checkreg(pt, a+2);

      case OP_JMP: {
        int dest = pc+1+b;
	check(0 <= dest && dest < pt->sizecode);

        if (reg != NO_REG && pc < dest && dest <= lastpc)
          pc += b;
        break;
      }
      case OP_CALL:
      case OP_TAILCALL: {
        if (b != 0) {
          checkreg(pt, a+b-1);
        }
        c--;
        if (c == LUA_MULTRET) {
          check(checkopenop(pt, pc));
        }
        else if (c != 0)
          checkreg(pt, a+c-1);
        if (reg >= a) last = pc;
        break;
      }
      case OP_RETURN: {
        b--;
        if (b > 0) checkreg(pt, a+b-1);
        break;
      }
      case OP_SETLIST: {
        checkreg(pt, a + (b&(LFIELDS_PER_FLUSH-1)) + 1);
        break;
      }
      case OP_CLOSURE: {
        int nup;
        check(b < pt->sizep);
        nup = pt->p[b]->nups;
        check(pc + nup < pt->sizecode);
        for (; nup>0; nup--) {
          OpCode op1 = GET_OPCODE(pt->code[pc+nup]);
          check(op1 == OP_GETUPVAL || op1 == OP_MOVE);
        }
        break;
      }
      default: break;
    }
  }
  return pt->code[last];
}

#undef check
#undef checkjump
#undef checkreg

// LIBRARY: SWBF2 0x006a1c10
// luaG_checkcode
int luaG_checkcode (const Proto *pt) {
  return luaG_symbexec(pt, pt->sizecode, NO_REG);
}

// LIBRARY: SWBF2 0x006a1bb0
// kname
static const char *kname (Proto *p, int c) {
  c = c - MAXSTACK;
  if (c >= 0 && ttisstring(&p->k[c]))
    return svalue(&p->k[c]);
  else
    return "?";
}

// LIBRARY: SWBF2 0x006a1a10
// getobjname
static const char *getobjname (CallInfo *ci, int stackpos, const char **name) {
  if (isLua(ci)) {
    Proto *p = ci_func(ci)->l.p;
    int pc = currentpc(ci);
    Instruction i;
    *name = luaF_getlocalname(p, stackpos+1, pc);
    if (*name)
      return "local";
    i = luaG_symbexec(p, pc, stackpos);
    lua_assert(pc != -1);
    switch (GET_OPCODE(i)) {
      case OP_GETGLOBAL: {
        int g = GETARG_Bx(i);
        lua_assert(ttisstring(&p->k[g]));
        *name = svalue(&p->k[g]);
        return "global";
      }
      case OP_MOVE: {
        int a = GETARG_A(i);
        int b = GETARG_B(i);
        if (b < a)
          return getobjname(ci, b, name);
        break;
      }
      case OP_GETTABLE: {
        int k = GETARG_C(i);
        *name = kname(p, k);
        return "field";
      }
      case OP_SELF: {
        int k = GETARG_C(i);
        *name = kname(p, k);
        return "method";
      }
      default: break;
    }
  }
  return NULL;
}

// LIBRARY: SWBF2 0x006a1990
// getfuncname
static const char *getfuncname (CallInfo *ci, const char **name) {
  Instruction i;
  if ((isLua(ci) && ci->u.l.tailcalls > 0) || !isLua(ci - 1))
    return NULL;
  ci--;
  i = ci_func(ci)->l.p->code[currentpc(ci)];
  if (GET_OPCODE(i) == OP_CALL || GET_OPCODE(i) == OP_TAILCALL)
    return getobjname(ci, GETARG_A(i), name);
  else
    return NULL;
}

// LIBRARY: SWBF2 0x006a1b80
// isinstack
static int isinstack (CallInfo *ci, const TObject *o) {
  StkId p;
  for (p = ci->base; p < ci->top; p++)
    if (o == p) return 1;
  return 0;
}

// LIBRARY: SWBF2 0x006a20f0
// luaG_typeerror
void luaG_typeerror (lua_State *L, const TObject *o, const char *op) {
  const char *name = NULL;
  const char *t = luaT_typenames[ttype(o)];
  const char *kind = (isinstack(L->ci, o)) ?
                         getobjname(L->ci, o - L->base, &name) : NULL;
  if (kind)
    luaG_runerror(L, "attempt to %s %s `%s' (a %s value)",
                op, kind, name, t);
  else
    luaG_runerror(L, "attempt to %s a %s value", op, t);
}

// LIBRARY: SWBF2 0x006a1c30
// luaG_concaterror
void luaG_concaterror (lua_State *L, StkId p1, StkId p2) {
  if (ttisstring(p1)) p1 = p2;
  lua_assert(!ttisstring(p1));
  luaG_typeerror(L, p1, "concatenate");
}

// LIBRARY: SWBF2 0x006a1be0
// luaG_aritherror
void luaG_aritherror (lua_State *L, const TObject *p1, const TObject *p2) {
  TObject temp;
  if (luaV_tonumber(p1, &temp) == NULL)
    p2 = p1;
  luaG_typeerror(L, p2, "perform arithmetic on");
}

// LIBRARY: SWBF2 0x006a1d00
// luaG_ordererror
int luaG_ordererror (lua_State *L, const TObject *p1, const TObject *p2) {
  const char *t1 = luaT_typenames[ttype(p1)];
  const char *t2 = luaT_typenames[ttype(p2)];
  if (t1[2] == t2[2])
    luaG_runerror(L, "attempt to compare two %s values", t1);
  else
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
  return 0;
}

// LIBRARY: SWBF2 0x006a1670
// addinfo
static void addinfo (lua_State *L, const char *msg) {
  CallInfo *ci = L->ci;
  if (isLua(ci)) {
    char buff[LUA_IDSIZE];
    int line = currentline(ci);
    luaO_chunkid(buff, getstr(getluaproto(ci)->source), LUA_IDSIZE);
    luaO_pushfstring(L, "%s:%d: %s", buff, line, msg);
  }
}

// LIBRARY: SWBF2 0x006a1c50
// luaG_errormsg
void luaG_errormsg (lua_State *L) {
  if (L->errfunc != 0) {
    StkId errfunc = restorestack(L, L->errfunc);
    if (!ttisfunction(errfunc)) luaD_throw(L, LUA_ERRERR);
    setobjs2s(L->top, L->top - 1);
    setobjs2s(L->top - 1, errfunc);
    incr_top(L);
    luaD_call(L, L->top - 2, 1);
  }
  luaD_throw(L, LUA_ERRRUN);
}

// LIBRARY: SWBF2 0x006a1d50
// luaG_runerror
void luaG_runerror (lua_State *L, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  addinfo(L, luaO_pushvfstring(L, fmt, argp));
  va_end(argp);
  luaG_errormsg(L);
}
