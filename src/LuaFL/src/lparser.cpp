#include <string.h>

#define lparser_c

#include "lua.h"

#include "lcode.h"
#include "ldebug.h"
#include "lfunc.h"
#include "llex.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lparser.h"
#include "lstate.h"
#include "lstring.h"

#define getlocvar(fs, i)	((fs)->f->locvars[(fs)->actvar[i]])

#define enterlevel(ls)	if (++(ls)->nestlevel > LUA_MAXPARSERLEVEL) \
		luaX_syntaxerror(ls, "too many syntax levels");
#define leavelevel(ls)	((ls)->nestlevel--)

typedef struct BlockCnt {
  struct BlockCnt *previous;
  int breaklist;
  int nactvar;
  int upval;
  int isbreakable;
} BlockCnt;

static void chunk (LexState *ls);
static void expr (LexState *ls, expdesc *v);

// LIBRARY: SWBF2 0x006a9d60
// next
static void next (LexState *ls) {
  ls->lastline = ls->linenumber;
  if (ls->lookahead.token != TK_EOS) {
    ls->t = ls->lookahead;
    ls->lookahead.token = TK_EOS;
  }
  else
    ls->t.token = luaX_lex(ls, &ls->t.seminfo);
}

// LIBRARY: SWBF2 0x006a9ad0
// lookahead
static void lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = luaX_lex(ls, &ls->lookahead.seminfo);
}

// LIBRARY: SWBF2 0x006a8ea0
// error_expected
static void error_expected (LexState *ls, int token) {
  luaX_syntaxerror(ls,
         luaO_pushfstring(ls->L, "`%s' expected", luaX_token2str(ls, token)));
}

// LIBRARY: SWBF2 0x006aa970
// testnext
static int testnext (LexState *ls, int c) {
  if (ls->t.token == c) {
    next(ls);
    return 1;
  }
  else return 0;
}

// LIBRARY: SWBF2 0x006a8930
// check
static void check (LexState *ls, int c) {
  if (!testnext(ls, c))
    error_expected(ls, c);
}

#define check_condition(ls,c,msg)	{ if (!(c)) luaX_syntaxerror(ls, msg); }

// LIBRARY: SWBF2 0x006a89c0
// check_match
static void check_match (LexState *ls, int what, int who, int where) {
  if (!testnext(ls, what)) {
    if (where == ls->linenumber)
      error_expected(ls, what);
    else {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "`%s' expected (to close `%s' at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}

// LIBRARY: SWBF2 0x006aa810
// str_checkname
static TString *str_checkname (LexState *ls) {
  TString *ts;
  check_condition(ls, (ls->t.token == TK_NAME), "<name> expected");
  ts = ls->t.seminfo.ts;
  next(ls);
  return ts;
}

// LIBRARY: SWBF2 0x006a9860
// init_exp
static void init_exp (expdesc *e, expkind k, int i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->info = i;
}

// LIBRARY: SWBF2 0x006a8c80
// codestring
static void codestring (LexState *ls, expdesc *e, TString *s) {
  init_exp(e, VK, luaK_stringK(ls->fs, s));
}

// LIBRARY: SWBF2 0x006a8a30
// checkname
static void checkname(LexState *ls, expdesc *e) {
  codestring(ls, e, str_checkname(ls));
}

// LIBRARY: SWBF2 0x006a9af0
// luaI_registerlocalvar
static int luaI_registerlocalvar (LexState *ls, TString *varname) {
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  luaM_growvector(ls->L, f->locvars, fs->nlocvars, f->sizelocvars,
                  LocVar, MAX_INT, "");
  f->locvars[fs->nlocvars].varname = varname;
  return fs->nlocvars++;
}

// LIBRARY: SWBF2 0x006a9ce0
// new_localvar
static void new_localvar (LexState *ls, TString *name, int n) {
  FuncState *fs = ls->fs;
  luaX_checklimit(ls, fs->nactvar+n+1, MAXVARS, "local variables");
  fs->actvar[fs->nactvar+n] = luaI_registerlocalvar(ls, name);
}

// LIBRARY: SWBF2 0x006a8630
// adjustlocalvars
static void adjustlocalvars (LexState *ls, int nvars) {
  FuncState *fs = ls->fs;
  fs->nactvar += nvars;
  for (; nvars; nvars--) {
    getlocvar(fs, fs->nactvar - nvars).startpc = fs->pc;
  }
}

// LIBRARY: SWBF2 0x006aa280
// removevars
static void removevars (LexState *ls, int tolevel) {
  FuncState *fs = ls->fs;
  while (fs->nactvar > tolevel)
    getlocvar(fs, --fs->nactvar).endpc = fs->pc;
}

// LIBRARY: SWBF2 0x006a9d20
// new_localvarstr
static void new_localvarstr (LexState *ls, const char *name, int n) {
  new_localvar(ls, luaS_new(ls->L, name), n);
}

// LIBRARY: SWBF2 0x006a8e50
// create_local
static void create_local (LexState *ls, const char *name) {
  new_localvarstr(ls, name, 0);
  adjustlocalvars(ls, 1);
}

// LIBRARY: SWBF2 0x006a97a0
// indexupvalue
static int indexupvalue (FuncState *fs, TString *name, expdesc *v) {
  int i;
  Proto *f = fs->f;
  for (i=0; i<f->nups; i++) {
    if (fs->upvalues[i].k == v->k && fs->upvalues[i].info == v->info) {
      lua_assert(fs->f->upvalues[i] == name);
      return i;
    }
  }

  luaX_checklimit(fs->ls, f->nups + 1, MAXUPVALUES, "upvalues");
  luaM_growvector(fs->L, fs->f->upvalues, f->nups, fs->f->sizeupvalues,
                  TString *, MAX_INT, "");
  fs->f->upvalues[f->nups] = name;
  fs->upvalues[f->nups] = *v;
  return f->nups++;
}

// LIBRARY: SWBF2 0x006aa400
// searchvar
static int searchvar (FuncState *fs, TString *n) {
  int i;
  for (i=fs->nactvar-1; i >= 0; i--) {
    if (n == getlocvar(fs, i).varname)
      return i;
  }
  return -1;
}

// LIBRARY: SWBF2 0x006a9cb0
// markupval
static void markupval (FuncState *fs, int level) {
  BlockCnt *bl = fs->bl;
  while (bl && bl->nactvar > level) bl = bl->previous;
  if (bl) bl->upval = 1;
}

// LIBRARY: SWBF2 0x006aa620
// singlevaraux
static void singlevaraux (FuncState *fs, TString *n, expdesc *var, int base) {
  if (fs == NULL)
    init_exp(var, VGLOBAL, NO_REG);
  else {
    int v = searchvar(fs, n);
    if (v >= 0) {
      init_exp(var, VLOCAL, v);
      if (!base)
        markupval(fs, v);
    }
    else {
      singlevaraux(fs->prev, n, var, 0);
      if (var->k == VGLOBAL) {
        if (base)
          var->info = luaK_stringK(fs, n);
      }
      else {
        var->info = indexupvalue(fs, n, var);
        var->k = VUPVAL;
      }
    }
  }
}

// LIBRARY: SWBF2 0x006aa5f0
// singlevar
static TString *singlevar (LexState *ls, expdesc *var, int base) {
  TString *varname = str_checkname(ls);
  singlevaraux(ls->fs, varname, var, base);
  return varname;
}

// LIBRARY: SWBF2 0x006a85b0
// adjust_assign
static void adjust_assign (LexState *ls, int nvars, int nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  int extra = nvars - nexps;
  if (e->k == VCALL) {
    extra++;
    if (extra <= 0) extra = 0;
    else luaK_reserveregs(fs, extra-1);
    luaK_setcallreturns(fs, e, extra);
  }
  else {
    if (e->k != VVOID) luaK_exp2nextreg(fs, e);
    if (extra > 0) {
      int reg = fs->freereg;
      luaK_reserveregs(fs, extra);
      luaK_nil(fs, reg, extra);
    }
  }
}

// LIBRARY: SWBF2 0x006a8c20
// code_params
static void code_params (LexState *ls, int nparams, int dots) {
  FuncState *fs = ls->fs;
  adjustlocalvars(ls, nparams);
  luaX_checklimit(ls, fs->nactvar, MAXPARAMS, "parameters");
  fs->f->numparams = cast(lu_byte, fs->nactvar);
  fs->f->is_vararg = cast(lu_byte, dots);
  if (dots)
    create_local(ls, "arg");
  luaK_reserveregs(fs, fs->nactvar);
}

// LIBRARY: SWBF2 0x006a8e70
// enterblock
static void enterblock (FuncState *fs, BlockCnt *bl, int isbreakable) {
  bl->breaklist = NO_JUMP;
  bl->isbreakable = isbreakable;
  bl->nactvar = fs->nactvar;
  bl->upval = 0;
  bl->previous = fs->bl;
  fs->bl = bl;
  lua_assert(fs->freereg == fs->nactvar);
}

// LIBRARY: SWBF2 0x006a9910
// leaveblock
static void leaveblock (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  fs->bl = bl->previous;
  removevars(fs->ls, bl->nactvar);
  if (bl->upval)
    luaK_codeABC(fs, OP_CLOSE, bl->nactvar, 0, 0);
  lua_assert(bl->nactvar == fs->nactvar);
  fs->freereg = fs->nactvar;
  luaK_patchtohere(fs, bl->breaklist);
}

// LIBRARY: SWBF2 0x006aa110
// pushclosure
static void pushclosure (LexState *ls, FuncState *func, expdesc *v) {
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  int i;
  luaM_growvector(ls->L, f->p, fs->np, f->sizep, Proto *,
                  MAXARG_Bx, "constant table overflow");
  f->p[fs->np++] = func->f;
  init_exp(v, VRELOCABLE, luaK_codeABx(fs, OP_CLOSURE, 0, fs->np-1));
  for (i=0; i<func->f->nups; i++) {
    OpCode o = (func->upvalues[i].k == VLOCAL) ? OP_MOVE : OP_GETUPVAL;
    luaK_codeABC(fs, o, 0, func->upvalues[i].info, 0);
  }
}

// LIBRARY: SWBF2 0x006a9da0
// open_func
static void open_func (LexState *ls, FuncState *fs) {
  Proto *f = luaF_newproto(ls->L);
  fs->f = f;
  fs->prev = ls->fs;
  fs->ls = ls;
  fs->L = ls->L;
  ls->fs = fs;
  fs->pc = 0;
  fs->lasttarget = 0;
  fs->jpc = NO_JUMP;
  fs->freereg = 0;
  fs->nk = 0;
  fs->h = luaH_new(ls->L, 0, 0);
  fs->np = 0;
  fs->nlocvars = 0;
  fs->nactvar = 0;
  fs->bl = NULL;
  f->source = ls->source;
  f->maxstacksize = 2;
}

// LIBRARY: SWBF2 0x006a8ab0
// close_func
static void close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  removevars(ls, 0);
  luaK_codeABC(fs, OP_RETURN, 0, 1, 0);
  luaM_reallocvector(L, f->code, f->sizecode, fs->pc, Instruction);
  f->sizecode = fs->pc;
  luaM_reallocvector(L, f->lineinfo, f->sizelineinfo, fs->pc, int);
  f->sizelineinfo = fs->pc;
  luaM_reallocvector(L, f->k, f->sizek, fs->nk, TObject);
  f->sizek = fs->nk;
  luaM_reallocvector(L, f->p, f->sizep, fs->np, Proto *);
  f->sizep = fs->np;
  luaM_reallocvector(L, f->locvars, f->sizelocvars, fs->nlocvars, LocVar);
  f->sizelocvars = fs->nlocvars;
  luaM_reallocvector(L, f->upvalues, f->sizeupvalues, f->nups, TString *);
  f->sizeupvalues = f->nups;
  lua_assert(luaG_checkcode(f));
  lua_assert(fs->bl == NULL);
  ls->fs = fs->prev;
}

// LIBRARY: SWBF2 0x006a9be0
// luaY_parser
Proto *luaY_parser (lua_State *L, ZIO *z, Mbuffer *buff) {
  struct LexState lexstate;
  struct FuncState funcstate;
  lexstate.buff = buff;
  lexstate.nestlevel = 0;
  luaX_setinput(L, &lexstate, z, luaS_new(L, zname(z)));
  open_func(&lexstate, &funcstate);
  next(&lexstate);
  chunk(&lexstate);
  check_condition(&lexstate, (lexstate.t.token == TK_EOS), "<eof> expected");
  close_func(&lexstate);
  lua_assert(funcstate.prev == NULL);
  lua_assert(funcstate.f->nups == 0);
  lua_assert(lexstate.nestlevel == 0);
  return funcstate.f;
}

// LIBRARY: SWBF2 0x006a9b50
// luaY_field
static void luaY_field (LexState *ls, expdesc *v) {

  FuncState *fs = ls->fs;
  expdesc key;
  luaK_exp2anyreg(fs, v);
  next(ls);
  checkname(ls, &key);
  luaK_indexed(fs, v, &key);
}

// LIBRARY: SWBF2 0x006a9bb0
// luaY_index
static void luaY_index (LexState *ls, expdesc *v) {

  next(ls);
  expr(ls, v);
  luaK_exp2val(ls->fs, v);
  check(ls, ']');
}

struct ConsControl {
  expdesc v;
  expdesc *t;
  int nh;
  int na;
  int tostore;
};

// LIBRARY: SWBF2 0x006aa1c0
// recfield
static void recfield (LexState *ls, struct ConsControl *cc) {

  FuncState *fs = ls->fs;
  int reg = ls->fs->freereg;
  expdesc key, val;
  if (ls->t.token == TK_NAME) {
    luaX_checklimit(ls, cc->nh, MAX_INT, "items in a constructor");
    cc->nh++;
    checkname(ls, &key);
  }
  else
    luaY_index(ls, &key);
  check(ls, '=');
  luaK_exp2RK(fs, &key);
  expr(ls, &val);
  luaK_codeABC(fs, OP_SETTABLE, cc->t->info, luaK_exp2RK(fs, &key),
                                             luaK_exp2RK(fs, &val));
  fs->freereg = reg;
}

// LIBRARY: SWBF2 0x006a8bc0
// closelistfield
static void closelistfield (FuncState *fs, struct ConsControl *cc) {
  if (cc->v.k == VVOID) return;
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  if (cc->tostore == LFIELDS_PER_FLUSH) {
    luaK_codeABx(fs, OP_SETLIST, cc->t->info, cc->na-1);
    cc->tostore = 0;
    fs->freereg = cc->t->info + 1;
  }
}

// LIBRARY: SWBF2 0x006a9890
// lastlistfield
static void lastlistfield (FuncState *fs, struct ConsControl *cc) {
  if (cc->tostore == 0) return;
  if (cc->v.k == VCALL) {
    luaK_setcallreturns(fs, &cc->v, LUA_MULTRET);
    luaK_codeABx(fs, OP_SETLISTO, cc->t->info, cc->na-1);
  }
  else {
    if (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_codeABx(fs, OP_SETLIST, cc->t->info, cc->na-1);
  }
  fs->freereg = cc->t->info + 1;
}

// LIBRARY: SWBF2 0x006a9960
// listfield
static void listfield (LexState *ls, struct ConsControl *cc) {
  expr(ls, &cc->v);
  luaX_checklimit(ls, cc->na, MAXARG_Bx, "items in a constructor");
  cc->na++;
  cc->tostore++;
}

// LIBRARY: SWBF2 0x006a8cf0
// constructor
static void constructor (LexState *ls, expdesc *t) {

  FuncState *fs = ls->fs;
  int line = ls->linenumber;
  int pc = luaK_codeABC(fs, OP_NEWTABLE, 0, 0, 0);
  struct ConsControl cc;
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VRELOCABLE, pc);
  init_exp(&cc.v, VVOID, 0);
  luaK_exp2nextreg(ls->fs, t);
  check(ls, '{');
  do {
    lua_assert(cc.v.k == VVOID || cc.tostore > 0);
    testnext(ls, ';');
    if (ls->t.token == '}') break;
    closelistfield(fs, &cc);
    switch(ls->t.token) {
      case TK_NAME: {
        lookahead(ls);
        if (ls->lookahead.token != '=')
          listfield(ls, &cc);
        else
          recfield(ls, &cc);
        break;
      }
      case '[': {
        recfield(ls, &cc);
        break;
      }
      default: {
        listfield(ls, &cc);
        break;
      }
    }
  } while (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, '}', '{', line);
  lastlistfield(fs, &cc);
  SETARG_B(fs->f->code[pc], luaO_int2fb(cc.na));
  SETARG_C(fs->f->code[pc], luaO_log2(cc.nh)+1);
}

// LIBRARY: SWBF2 0x006a9e30
// parlist
static void parlist (LexState *ls) {

  int nparams = 0;
  int dots = 0;
  if (ls->t.token != ')') {
    do {
      switch (ls->t.token) {
        case TK_DOTS: dots = 1; next(ls); break;
        case TK_NAME: new_localvar(ls, str_checkname(ls), nparams++); break;
        default: luaX_syntaxerror(ls, "<name> or `...' expected");
      }
    } while (!dots && testnext(ls, ','));
  }
  code_params(ls, nparams, dots);
}

// LIBRARY: SWBF2 0x006a8810
// body
static void body (LexState *ls, expdesc *e, int needself, int line) {

  FuncState new_fs;
  open_func(ls, &new_fs);
  new_fs.f->lineDefined = line;
  check(ls, '(');
  if (needself)
    create_local(ls, "self");
  parlist(ls);
  check(ls, ')');
  chunk(ls);
  check_match(ls, TK_END, TK_FUNCTION, line);
  close_func(ls);
  pushclosure(ls, &new_fs, e);
}

// LIBRARY: SWBF2 0x006a8f20
// explist1
static int explist1 (LexState *ls, expdesc *v) {

  int n = 1;
  expr(ls, v);
  while (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    expr(ls, v);
    n++;
  }
  return n;
}

// LIBRARY: SWBF2 0x006a9300
// funcargs
static void funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc args;
  int base, nparams;
  int line = ls->linenumber;
  switch (ls->t.token) {
    case '(': {
      if (line != ls->lastline)
        luaX_syntaxerror(ls,"ambiguous syntax (function call x new statement)");
      next(ls);
      if (ls->t.token == ')')
        args.k = VVOID;
      else {
        explist1(ls, &args);
        luaK_setcallreturns(fs, &args, LUA_MULTRET);
      }
      check_match(ls, ')', '(', line);
      break;
    }
    case '{': {
      constructor(ls, &args);
      break;
    }
    case TK_STRING: {
      codestring(ls, &args, ls->t.seminfo.ts);
      next(ls);
      break;
    }
    default: {
      luaX_syntaxerror(ls, "function arguments expected");
      return;
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->info;
  if (args.k == VCALL)
    nparams = LUA_MULTRET;
  else {
    if (args.k != VVOID)
      luaK_exp2nextreg(fs, &args);
    nparams = fs->freereg - (base+1);
  }
  init_exp(f, VCALL, luaK_codeABC(fs, OP_CALL, base, nparams+1, 2));
  luaK_fixline(fs, line);
  fs->freereg = base+1;
}

// LIBRARY: SWBF2 0x006a9ec0
// prefixexp
static void prefixexp (LexState *ls, expdesc *v) {

  switch (ls->t.token) {
    case '(': {
      int line = ls->linenumber;
      next(ls);
      expr(ls, v);
      check_match(ls, ')', '(', line);
      luaK_dischargevars(ls->fs, v);
      return;
    }
    case TK_NAME: {
      singlevar(ls, v, 1);
      return;
    }
#ifdef LUA_COMPATUPSYNTAX
    case '%': {
      TString *varname;
      int line = ls->linenumber;
      next(ls);
      varname = singlevar(ls, v, 1);
      if (v->k != VUPVAL)
        luaX_errorline(ls, "global upvalues are obsolete",
                           getstr(varname), line);
      return;
    }
#endif
    default: {
      luaX_syntaxerror(ls, "unexpected symbol");
      return;
    }
  }
}

// LIBRARY: SWBF2 0x006a9f30
// primaryexp
static void primaryexp (LexState *ls, expdesc *v) {

  FuncState *fs = ls->fs;
  prefixexp(ls, v);
  for (;;) {
    switch (ls->t.token) {
      case '.': {
        luaY_field(ls, v);
        break;
      }
      case '[': {
        expdesc key;
        luaK_exp2anyreg(fs, v);
        luaY_index(ls, &key);
        luaK_indexed(fs, v, &key);
        break;
      }
      case ':': {
        expdesc key;
        next(ls);
        checkname(ls, &key);
        luaK_self(fs, v, &key);
        funcargs(ls, v);
        break;
      }
      case '(': case TK_STRING: case '{': {
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        break;
      }
      default: return;
    }
  }
}

// LIBRARY: SWBF2 0x006aa440
// simpleexp
static void simpleexp (LexState *ls, expdesc *v) {

  switch (ls->t.token) {
    case TK_NUMBER: {
      init_exp(v, VK, luaK_numberK(ls->fs, ls->t.seminfo.r));
      next(ls);
      break;
    }
    case TK_STRING: {
      codestring(ls, v, ls->t.seminfo.ts);
      next(ls);
      break;
    }
    case TK_NIL: {
      init_exp(v, VNIL, 0);
      next(ls);
      break;
    }
    case TK_TRUE: {
      init_exp(v, VTRUE, 0);
      next(ls);
      break;
    }
    case TK_FALSE: {
      init_exp(v, VFALSE, 0);
      next(ls);
      break;
    }
    case '{': {
      constructor(ls, v);
      break;
    }
    case TK_FUNCTION: {
      next(ls);
      body(ls, v, 0, ls->linenumber);
      break;
    }
    default: {
      primaryexp(ls, v);
      break;
    }
  }
}

// LIBRARY: SWBF2 0x006a96a0
// getunopr
static UnOpr getunopr (int op) {
  switch (op) {
    case TK_NOT: return OPR_NOT;
    case '-': return OPR_MINUS;
    default: return OPR_NOUNOPR;
  }
}

// LIBRARY: SWBF2 0x006a94e0
// getbinopr
static BinOpr getbinopr (int op) {
  switch (op) {
    case '+': return OPR_ADD;
    case '-': return OPR_SUB;
    case '*': return OPR_MULT;
    case '/': return OPR_DIV;
    case '^': return OPR_POW;
    case TK_CONCAT: return OPR_CONCAT;
    case TK_NE: return OPR_NE;
    case TK_EQ: return OPR_EQ;
    case '<': return OPR_LT;
    case TK_LE: return OPR_LE;
    case '>': return OPR_GT;
    case TK_GE: return OPR_GE;
    case TK_AND: return OPR_AND;
    case TK_OR: return OPR_OR;
    default: return OPR_NOBINOPR;
  }
}

static const struct {
  lu_byte left;
  lu_byte right;
} priority[] = {
   {6, 6}, {6, 6}, {7, 7}, {7, 7},
   {10, 9}, {5, 4},
   {3, 3}, {3, 3},
   {3, 3}, {3, 3}, {3, 3}, {3, 3},
   {2, 2}, {1, 1}
};

#define UNARY_PRIORITY	8

// LIBRARY: SWBF2 0x006aa850
// subexpr
static BinOpr subexpr (LexState *ls, expdesc *v, int limit) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  if (uop != OPR_NOUNOPR) {
    next(ls);
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v);
  }
  else simpleexp(ls, v);

  op = getbinopr(ls->t.token);
  while (op != OPR_NOBINOPR && cast(int, priority[op].left) > limit) {
    expdesc v2;
    BinOpr nextop;
    next(ls);
    luaK_infix(ls->fs, op, v);

    nextop = subexpr(ls, &v2, cast(int, priority[op].right));
    luaK_posfix(ls->fs, op, v, &v2);
    op = nextop;
  }
  leavelevel(ls);
  return op;
}

// LIBRARY: SWBF2 0x006a8f70
// expr
static void expr (LexState *ls, expdesc *v) {
  subexpr(ls, v, -1);
}

// LIBRARY: SWBF2 0x006a87c0
// block_follow
static int block_follow (int token) {
  switch (token) {
    case TK_ELSE: case TK_ELSEIF: case TK_END:
    case TK_UNTIL: case TK_EOS:
      return 1;
    default: return 0;
  }
}

// LIBRARY: SWBF2 0x006a8790
// block
static void block (LexState *ls) {

  FuncState *fs = ls->fs;
  BlockCnt bl;
  enterblock(fs, &bl, 0);
  chunk(ls);
  lua_assert(bl.breaklist == NO_JUMP);
  leaveblock(fs);
}

struct LHS_assign {
  struct LHS_assign *prev;
  expdesc v;
};

// LIBRARY: SWBF2 0x006a8950
// check_conflict
static void check_conflict (LexState *ls, struct LHS_assign *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  int extra = fs->freereg;
  int conflict = 0;
  for (; lh; lh = lh->prev) {
    if (lh->v.k == VINDEXED) {
      if (lh->v.info == v->info) {
        conflict = 1;
        lh->v.info = extra;
      }
      if (lh->v.aux == v->info) {
        conflict = 1;
        lh->v.aux = extra;
      }
    }
  }
  if (conflict) {
    luaK_codeABC(fs, OP_MOVE, fs->freereg, v->info, 0);
    luaK_reserveregs(fs, 1);
  }
}

// LIBRARY: SWBF2 0x006a8680
// assignment
static void assignment (LexState *ls, struct LHS_assign *lh, int nvars) {
  expdesc e;
  check_condition(ls, VLOCAL <= lh->v.k && lh->v.k <= VINDEXED,
                      "syntax error");
  if (testnext(ls, ',')) {
    struct LHS_assign nv;
    nv.prev = lh;
    primaryexp(ls, &nv.v);
    if (nv.v.k == VLOCAL)
      check_conflict(ls, lh, &nv.v);
    assignment(ls, &nv, nvars+1);
  }
  else {
    int nexps;
    check(ls, '=');
    nexps = explist1(ls, &e);
    if (nexps != nvars) {
      adjust_assign(ls, nvars, nexps, &e);
      if (nexps > nvars)
        ls->fs->freereg -= nexps - nvars;
    }
    else {
      luaK_setcallreturns(ls->fs, &e, 1);
      luaK_storevar(ls->fs, &lh->v, &e);
      return;
    }
  }
  init_exp(&e, VNONRELOC, ls->fs->freereg-1);
  luaK_storevar(ls->fs, &lh->v, &e);
}

// LIBRARY: SWBF2 0x006a8cb0
// cond
static void cond (LexState *ls, expdesc *v) {

  expr(ls, v);
  if (v->k == VNIL) v->k = VFALSE;
  luaK_goiftrue(ls->fs, v);
  luaK_patchtohere(ls->fs, v->t);
}

#ifndef MAXEXPWHILE
#define MAXEXPWHILE	100
#endif

#define EXTRAEXP	5

// LIBRARY: SWBF2 0x006aa9a0
// whilestat
static void whilestat (LexState *ls, int line) {

  Instruction codeexp[MAXEXPWHILE + EXTRAEXP];
  int lineexp;
  int i;
  int sizeexp;
  FuncState *fs = ls->fs;
  int whileinit, blockinit, expinit;
  expdesc v;
  BlockCnt bl;
  next(ls);
  whileinit = luaK_jump(fs);
  expinit = luaK_getlabel(fs);
  expr(ls, &v);
  if (v.k == VK) v.k = VTRUE;
  lineexp = ls->linenumber;
  luaK_goiffalse(fs, &v);
  luaK_concat(fs, &v.f, fs->jpc);
  fs->jpc = NO_JUMP;
  sizeexp = fs->pc - expinit;
  if (sizeexp > MAXEXPWHILE)
    luaX_syntaxerror(ls, "`while' condition too complex");
  for (i = 0; i < sizeexp; i++)
    codeexp[i] = fs->f->code[expinit + i];
  fs->pc = expinit;
  enterblock(fs, &bl, 1);
  check(ls, TK_DO);
  blockinit = luaK_getlabel(fs);
  block(ls);
  luaK_patchtohere(fs, whileinit);

  if (v.t != NO_JUMP) v.t += fs->pc - expinit;
  if (v.f != NO_JUMP) v.f += fs->pc - expinit;
  for (i=0; i<sizeexp; i++)
    luaK_code(fs, codeexp[i], lineexp);
  check_match(ls, TK_END, TK_WHILE, line);
  leaveblock(fs);
  luaK_patchlist(fs, v.t, blockinit);
  luaK_patchtohere(fs, v.f);
}

// LIBRARY: SWBF2 0x006aa2c0
// repeatstat
static void repeatstat (LexState *ls, int line) {

  FuncState *fs = ls->fs;
  int repeat_init = luaK_getlabel(fs);
  expdesc v;
  BlockCnt bl;
  enterblock(fs, &bl, 1);
  next(ls);
  block(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, line);
  cond(ls, &v);
  luaK_patchlist(fs, v.f, repeat_init);
  leaveblock(fs);
}

// LIBRARY: SWBF2 0x006a8ed0
// exp1
static int exp1 (LexState *ls) {
  expdesc e;
  int k;
  expr(ls, &e);
  k = e.k;
  luaK_exp2nextreg(ls->fs, &e);
  return k;
}

// LIBRARY: SWBF2 0x006a9000
// forbody
static void forbody (LexState *ls, int base, int line, int nvars, int isnum) {
  BlockCnt bl;
  FuncState *fs = ls->fs;
  int prep, endfor;
  adjustlocalvars(ls, nvars);
  check(ls, TK_DO);
  enterblock(fs, &bl, 1);
  prep = luaK_getlabel(fs);
  block(ls);
  luaK_patchtohere(fs, prep-1);
  endfor = (isnum) ? luaK_codeAsBx(fs, OP_FORLOOP, base, NO_JUMP) :
                     luaK_codeABC(fs, OP_TFORLOOP, base, 0, nvars - 3);
  luaK_fixline(fs, line);
  luaK_patchlist(fs, (isnum) ? endfor : luaK_jump(fs), prep);
  leaveblock(fs);
}

// LIBRARY: SWBF2 0x006a91b0
// fornum
static void fornum (LexState *ls, TString *varname, int line) {

  FuncState *fs = ls->fs;
  int base = fs->freereg;
  new_localvar(ls, varname, 0);
  new_localvarstr(ls, "(for limit)", 1);
  new_localvarstr(ls, "(for step)", 2);
  check(ls, '=');
  exp1(ls);
  check(ls, ',');
  exp1(ls);
  if (testnext(ls, ','))
    exp1(ls);
  else {
    luaK_codeABx(fs, OP_LOADK, fs->freereg, luaK_numberK(fs, 1));
    luaK_reserveregs(fs, 1);
  }
  luaK_codeABC(fs, OP_SUB, fs->freereg - 3, fs->freereg - 3, fs->freereg - 1);
  luaK_jump(fs);
  forbody(ls, base, line, 3, 1);
}

// LIBRARY: SWBF2 0x006a90c0
// forlist
static void forlist (LexState *ls, TString *indexname) {

  FuncState *fs = ls->fs;
  expdesc e;
  int nvars = 0;
  int line;
  int base = fs->freereg;
  new_localvarstr(ls, "(for generator)", nvars++);
  new_localvarstr(ls, "(for state)", nvars++);
  new_localvar(ls, indexname, nvars++);
  while (testnext(ls, ','))
    new_localvar(ls, str_checkname(ls), nvars++);
  check(ls, TK_IN);
  line = ls->linenumber;
  adjust_assign(ls, nvars, explist1(ls, &e), &e);
  luaK_checkstack(fs, 3);
  luaK_codeAsBx(fs, OP_TFORPREP, base, NO_JUMP);
  forbody(ls, base, line, nvars, 0);
}

// LIBRARY: SWBF2 0x006a9270
// forstat
static void forstat (LexState *ls, int line) {

  FuncState *fs = ls->fs;
  TString *varname;
  BlockCnt bl;
  enterblock(fs, &bl, 0);
  next(ls);
  varname = str_checkname(ls);
  switch (ls->t.token) {
    case '=': fornum(ls, varname, line); break;
    case ',': case TK_IN: forlist(ls, varname); break;
    default: luaX_syntaxerror(ls, "`=' or `in' expected");
  }
  check_match(ls, TK_END, TK_FOR, line);
  leaveblock(fs);
}

// LIBRARY: SWBF2 0x006aa940
// test_then_block
static void test_then_block (LexState *ls, expdesc *v) {

  next(ls);
  cond(ls, v);
  check(ls, TK_THEN);
  block(ls);
}

// LIBRARY: SWBF2 0x006a96c0
// ifstat
static void ifstat (LexState *ls, int line) {

  FuncState *fs = ls->fs;
  expdesc v;
  int escapelist = NO_JUMP;
  test_then_block(ls, &v);
  while (ls->t.token == TK_ELSEIF) {
    luaK_concat(fs, &escapelist, luaK_jump(fs));
    luaK_patchtohere(fs, v.f);
    test_then_block(ls, &v);
  }
  if (ls->t.token == TK_ELSE) {
    luaK_concat(fs, &escapelist, luaK_jump(fs));
    luaK_patchtohere(fs, v.f);
    next(ls);
    block(ls);
  }
  else
    luaK_concat(fs, &escapelist, v.f);
  luaK_patchtohere(fs, escapelist);
  check_match(ls, TK_END, TK_IF, line);
}

// LIBRARY: SWBF2 0x006a99a0
// localfunc
static void localfunc (LexState *ls) {
  expdesc v, b;
  FuncState *fs = ls->fs;
  new_localvar(ls, str_checkname(ls), 0);
  init_exp(&v, VLOCAL, fs->freereg);
  luaK_reserveregs(fs, 1);
  adjustlocalvars(ls, 1);
  body(ls, &b, 0, ls->linenumber);
  luaK_storevar(fs, &v, &b);

  getlocvar(fs, fs->nactvar - 1).startpc = fs->pc;
}

// LIBRARY: SWBF2 0x006a9a40
// localstat
static void localstat (LexState *ls) {

  int nvars = 0;
  int nexps;
  expdesc e;
  do {
    new_localvar(ls, str_checkname(ls), nvars++);
  } while (testnext(ls, ','));
  if (testnext(ls, '='))
    nexps = explist1(ls, &e);
  else {
    e.k = VVOID;
    nexps = 0;
  }
  adjust_assign(ls, nvars, nexps, &e);
  adjustlocalvars(ls, nvars);
}

// LIBRARY: SWBF2 0x006a9430
// funcname
static int funcname (LexState *ls, expdesc *v) {

  int needself = 0;
  singlevar(ls, v, 1);
  while (ls->t.token == '.')
    luaY_field(ls, v);
  if (ls->t.token == ':') {
    needself = 1;
    luaY_field(ls, v);
  }
  return needself;
}

// LIBRARY: SWBF2 0x006a9480
// funcstat
static void funcstat (LexState *ls, int line) {

  int needself;
  expdesc v, b;
  next(ls);
  needself = funcname(ls, &v);
  body(ls, &b, needself, line);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, line);
}

// LIBRARY: SWBF2 0x006a8f90
// exprstat
static void exprstat (LexState *ls) {

  FuncState *fs = ls->fs;
  struct LHS_assign v;
  primaryexp(ls, &v.v);
  if (v.v.k == VCALL) {
    luaK_setcallreturns(fs, &v.v, 0);
  }
  else {
    v.prev = NULL;
    assignment(ls, &v, 1);
  }
}

// LIBRARY: SWBF2 0x006aa340
// retstat
static void retstat (LexState *ls) {

  FuncState *fs = ls->fs;
  expdesc e;
  int first, nret;
  next(ls);
  if (block_follow(ls->t.token) || ls->t.token == ';')
    first = nret = 0;
  else {
    nret = explist1(ls, &e);
    if (e.k == VCALL) {
      luaK_setcallreturns(fs, &e, LUA_MULTRET);
      if (nret == 1) {
        SET_OPCODE(getcode(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getcode(fs,&e)) == fs->nactvar);
      }
      first = fs->nactvar;
      nret = LUA_MULTRET;
    }
    else {
      if (nret == 1)
        first = luaK_exp2anyreg(fs, &e);
      else {
        luaK_exp2nextreg(fs, &e);
        first = fs->nactvar;
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  luaK_codeABC(fs, OP_RETURN, first, nret+1, 0);
}

// LIBRARY: SWBF2 0x006a88c0
// breakstat
static void breakstat (LexState *ls) {

  FuncState *fs = ls->fs;
  BlockCnt *bl = fs->bl;
  int upval = 0;
  next(ls);
  while (bl && !bl->isbreakable) {
    upval |= bl->upval;
    bl = bl->previous;
  }
  if (!bl)
    luaX_syntaxerror(ls, "no loop to break");
  if (upval)
    luaK_codeABC(fs, OP_CLOSE, bl->nactvar, 0, 0);
  luaK_concat(fs, &bl->breaklist, luaK_jump(fs));
}

// LIBRARY: SWBF2 0x006aa6c0
// statement
static int statement (LexState *ls) {
  int line = ls->linenumber;
  switch (ls->t.token) {
    case TK_IF: {
      ifstat(ls, line);
      return 0;
    }
    case TK_WHILE: {
      whilestat(ls, line);
      return 0;
    }
    case TK_DO: {
      next(ls);
      block(ls);
      check_match(ls, TK_END, TK_DO, line);
      return 0;
    }
    case TK_FOR: {
      forstat(ls, line);
      return 0;
    }
    case TK_REPEAT: {
      repeatstat(ls, line);
      return 0;
    }
    case TK_FUNCTION: {
      funcstat(ls, line);
      return 0;
    }
    case TK_LOCAL: {
      next(ls);
      if (testnext(ls, TK_FUNCTION))
        localfunc(ls);
      else
        localstat(ls);
      return 0;
    }
    case TK_RETURN: {
      retstat(ls);
      return 1;
    }
    case TK_BREAK: {
      breakstat(ls);
      return 1;
    }
    default: {
      exprstat(ls);
      return 0;
    }
  }
}

// LIBRARY: SWBF2 0x006a8a50
// chunk
static void chunk (LexState *ls) {

  int islast = 0;
  enterlevel(ls);
  while (!islast && !block_follow(ls->t.token)) {
    islast = statement(ls);
    testnext(ls, ';');
    lua_assert(ls->fs->freereg >= ls->fs->nactvar);
    ls->fs->freereg = ls->fs->nactvar;
  }
  leavelevel(ls);
}
