#include <string.h>

#define lgc_c

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"

typedef struct GCState {
  GCObject *tmark;
  GCObject *wk;
  GCObject *wv;
  GCObject *wkv;
  global_State *g;
} GCState;

#define setbit(x,b)	((x) |= (1<<(b)))
#define resetbit(x,b)	((x) &= cast(lu_byte, ~(1<<(b))))
#define testbit(x,b)	((x) & (1<<(b)))

#define unmark(x)	resetbit((x)->gch.marked, 0)
#define ismarked(x)	((x)->gch.marked & ((1<<4)|1))

#define stringmark(s)	setbit((s)->tsv.marked, 0)

#define isfinalized(u)		(!testbit((u)->uv.marked, 1))
#define markfinalized(u)	resetbit((u)->uv.marked, 1)

#define KEYWEAKBIT    1
#define VALUEWEAKBIT  2
#define KEYWEAK         (1<<KEYWEAKBIT)
#define VALUEWEAK       (1<<VALUEWEAKBIT)

#define markobject(st,o) { checkconsistency(o); \
  if (iscollectable(o) && !ismarked(gcvalue(o))) reallymarkobject(st,gcvalue(o)); }

#define condmarkobject(st,o,c) { checkconsistency(o); \
  if (iscollectable(o) && !ismarked(gcvalue(o)) && (c)) \
    reallymarkobject(st,gcvalue(o)); }

#define markvalue(st,t) { if (!ismarked(valtogco(t))) \
		reallymarkobject(st, valtogco(t)); }

// LIBRARY: SWBF2 0x006a4100
// reallymarkobject
static void reallymarkobject (GCState *st, GCObject *o) {
  lua_assert(!ismarked(o));
  setbit(o->gch.marked, 0);
  switch (o->gch.tt) {
    case LUA_TUSERDATA: {
      markvalue(st, gcotou(o)->uv.metatable);
      break;
    }
    case LUA_TFUNCTION: {
      gcotocl(o)->c.gclist = st->tmark;
      st->tmark = o;
      break;
    }
    case LUA_TTABLE: {
      gcotoh(o)->gclist = st->tmark;
      st->tmark = o;
      break;
    }
    case LUA_TTHREAD: {
      gcototh(o)->gclist = st->tmark;
      st->tmark = o;
      break;
    }
    case LUA_TPROTO: {
      gcotop(o)->gclist = st->tmark;
      st->tmark = o;
      break;
    }
    default: lua_assert(o->gch.tt == LUA_TSTRING);
  }
}

// LIBRARY: SWBF2 0x006a4050
// marktmu
static void marktmu (GCState *st) {
  GCObject *u;
  for (u = st->g->tmudata; u; u = u->gch.next) {
    unmark(u);
    reallymarkobject(st, u);
  }
}

// LIBRARY: SWBF2 0x006a3e50
// luaC_separateudata
size_t luaC_separateudata (lua_State *L) {
  size_t deadmem = 0;
  GCObject **p = &G(L)->rootudata;
  GCObject *curr;
  GCObject *collected = NULL;
  GCObject **lastcollected = &collected;
  while ((curr = *p) != NULL) {
    lua_assert(curr->gch.tt == LUA_TUSERDATA);
    if (ismarked(curr) || isfinalized(gcotou(curr)))
      p = &curr->gch.next;

    else if (fasttm(L, gcotou(curr)->uv.metatable, TM_GC) == NULL) {
      markfinalized(gcotou(curr));
      p = &curr->gch.next;
    }
    else {
      deadmem += sizeudata(gcotou(curr)->uv.len);
      *p = curr->gch.next;
      curr->gch.next = NULL;
      *lastcollected = curr;
      lastcollected = &curr->gch.next;
    }
  }

  *lastcollected = G(L)->tmudata;
  G(L)->tmudata = collected;
  return deadmem;
}

// LIBRARY: SWBF2 0x006a4180
// removekey
static void removekey (Node *n) {
  setnilvalue(gval(n));
  if (iscollectable(gkey(n)))
    setttype(gkey(n), LUA_TNONE);
}

// LIBRARY: SWBF2 0x006a4430
// traversetable
static void traversetable (GCState *st, Table *h) {
  int i;
  int weakkey = 0;
  int weakvalue = 0;
  const TObject *mode;
  markvalue(st, h->metatable);
  lua_assert(h->lsizenode || h->node == st->g->dummynode);
  mode = gfasttm(st->g, h->metatable, TM_MODE);
  if (mode && ttisstring(mode)) {
    weakkey = (strchr(svalue(mode), 'k') != NULL);
    weakvalue = (strchr(svalue(mode), 'v') != NULL);
    if (weakkey || weakvalue) {
      GCObject **weaklist;
      h->marked &= ~(KEYWEAK | VALUEWEAK);
      h->marked |= cast(lu_byte, (weakkey << KEYWEAKBIT) |
                                 (weakvalue << VALUEWEAKBIT));
      weaklist = (weakkey && weakvalue) ? &st->wkv :
                              (weakkey) ? &st->wk :
                                          &st->wv;
      h->gclist = *weaklist;
      *weaklist = valtogco(h);
    }
  }
  if (!weakvalue) {
    i = h->sizearray;
    while (i--)
      markobject(st, &h->array[i]);
  }
  i = sizenode(h);
  while (i--) {
    Node *n = gnode(h, i);
    if (!ttisnil(gval(n))) {
      lua_assert(!ttisnil(gkey(n)));
      condmarkobject(st, gkey(n), !weakkey);
      condmarkobject(st, gval(n), !weakvalue);
    }
  }
}

// LIBRARY: SWBF2 0x006a4300
// traverseproto
static void traverseproto (GCState *st, Proto *f) {
  int i;
  stringmark(f->source);
  for (i=0; i<f->sizek; i++) {
    if (ttisstring(f->k+i))
      stringmark(tsvalue(f->k+i));
  }
  for (i=0; i<f->sizeupvalues; i++)
    stringmark(f->upvalues[i]);
  for (i=0; i<f->sizep; i++)
    markvalue(st, f->p[i]);
  for (i=0; i<f->sizelocvars; i++)
    stringmark(f->locvars[i].varname);
  lua_assert(luaG_checkcode(f));
}

// LIBRARY: SWBF2 0x006a4230
// traverseclosure
static void traverseclosure (GCState *st, Closure *cl) {
  if (cl->c.isC) {
    int i;
    for (i=0; i<cl->c.nupvalues; i++)
      markobject(st, &cl->c.upvalue[i]);
  }
  else {
    int i;
    lua_assert(cl->l.nupvalues == cl->l.p->nups);
    markvalue(st, hvalue(&cl->l.g));
    markvalue(st, cl->l.p);
    for (i=0; i<cl->l.nupvalues; i++) {
      UpVal *u = cl->l.upvals[i];
      if (!u->marked) {
        markobject(st, &u->value);
        u->marked = 1;
      }
    }
  }
}

// LIBRARY: SWBF2 0x006a3b20
// checkstacksizes
static void checkstacksizes (lua_State *L, StkId max) {
  int used = L->ci - L->base_ci;
  if (4*used < L->size_ci && 2*BASIC_CI_SIZE < L->size_ci)
    luaD_reallocCI(L, L->size_ci/2);
  else condhardstacktests(luaD_reallocCI(L, L->size_ci));
  used = max - L->stack;
  if (4*used < L->stacksize && 2*(BASIC_STACK_SIZE+EXTRA_STACK) < L->stacksize)
    luaD_reallocstack(L, L->stacksize/2);
  else condhardstacktests(luaD_reallocstack(L, L->stacksize));
}

// LIBRARY: SWBF2 0x006a4390
// traversestack
static void traversestack (GCState *st, lua_State *L1) {
  StkId o, lim;
  CallInfo *ci;
  markobject(st, gt(L1));
  lim = L1->top;
  for (ci = L1->base_ci; ci <= L1->ci; ci++) {
    lua_assert(ci->top <= L1->stack_last);
    lua_assert(ci->state & (CI_C | CI_HASFRAME | CI_SAVEDPC));
    if (lim < ci->top)
      lim = ci->top;
  }
  for (o = L1->stack; o < L1->top; o++)
    markobject(st, o);
  for (; o <= lim; o++)
    setnilvalue(o);
  checkstacksizes(L1, lim);
}

// LIBRARY: SWBF2 0x006a4080
// propagatemarks
static void propagatemarks (GCState *st) {
  while (st->tmark) {
    switch (st->tmark->gch.tt) {
      case LUA_TTABLE: {
        Table *h = gcotoh(st->tmark);
        st->tmark = h->gclist;
        traversetable(st, h);
        break;
      }
      case LUA_TFUNCTION: {
        Closure *cl = gcotocl(st->tmark);
        st->tmark = cl->c.gclist;
        traverseclosure(st, cl);
        break;
      }
      case LUA_TTHREAD: {
        lua_State *th = gcototh(st->tmark);
        st->tmark = th->gclist;
        traversestack(st, th);
        break;
      }
      case LUA_TPROTO: {
        Proto *p = gcotop(st->tmark);
        st->tmark = p->gclist;
        traverseproto(st, p);
        break;
      }
      default: lua_assert(0);
    }
  }
}

// LIBRARY: SWBF2 0x006a45a0
// valismarked
static int valismarked (const TObject *o) {
  if (ttisstring(o))
    stringmark(tsvalue(o));
  return !iscollectable(o) || testbit(o->value.gc->gch.marked, 0);
}

// LIBRARY: SWBF2 0x006a3ba0
// cleartablekeys
static void cleartablekeys (GCObject *l) {
  while (l) {
    Table *h = gcotoh(l);
    int i = sizenode(h);
    lua_assert(h->marked & KEYWEAK);
    while (i--) {
      Node *n = gnode(h, i);
      if (!valismarked(gkey(n)))
        removekey(n);
    }
    l = h->gclist;
  }
}

// LIBRARY: SWBF2 0x006a3c00
// cleartablevalues
static void cleartablevalues (GCObject *l) {
  while (l) {
    Table *h = gcotoh(l);
    int i = h->sizearray;
    lua_assert(h->marked & VALUEWEAK);
    while (i--) {
      TObject *o = &h->array[i];
      if (!valismarked(o))
        setnilvalue(o);
    }
    i = sizenode(h);
    while (i--) {
      Node *n = gnode(h, i);
      if (!valismarked(gval(n)))
        removekey(n);
    }
    l = h->gclist;
  }
}

// LIBRARY: SWBF2 0x006a3ce0
// freeobj
static void freeobj (lua_State *L, GCObject *o) {
  switch (o->gch.tt) {
    case LUA_TPROTO: luaF_freeproto(L, gcotop(o)); break;
    case LUA_TFUNCTION: luaF_freeclosure(L, gcotocl(o)); break;
    case LUA_TUPVAL: luaM_freelem(L, gcotouv(o)); break;
    case LUA_TTABLE: luaH_free(L, gcotoh(o)); break;
    case LUA_TTHREAD: {
      lua_assert(gcototh(o) != L && gcototh(o) != G(L)->mainthread);
      luaE_freethread(L, gcototh(o));
      break;
    }
    case LUA_TSTRING: {
      luaM_free(L, o, sizestring(gcotots(o)->tsv.len));
      break;
    }
    case LUA_TUSERDATA: {
      luaM_free(L, o, sizeudata(gcotou(o)->uv.len));
      break;
    }
    default: lua_assert(0);
  }
}

// LIBRARY: SWBF2 0x006a41a0
// sweeplist
static int sweeplist (lua_State *L, GCObject **p, int limit) {
  GCObject *curr;
  int count = 0;
  while ((curr = *p) != NULL) {
    if (curr->gch.marked > limit) {
      unmark(curr);
      p = &curr->gch.next;
    }
    else {
      count++;
      *p = curr->gch.next;
      freeobj(L, curr);
    }
  }
  return count;
}

// LIBRARY: SWBF2 0x006a41f0
// sweepstrings
static void sweepstrings (lua_State *L, int all) {
  int i;
  for (i=0; i<G(L)->strt.size; i++) {
    G(L)->strt.nuse -= sweeplist(L, &G(L)->strt.hash[i], all);
  }
}

// LIBRARY: SWBF2 0x006a3ab0
// checkSizes
static void checkSizes (lua_State *L, size_t deadmem) {

  if (G(L)->strt.nuse < cast(ls_nstr, G(L)->strt.size/4) &&
      G(L)->strt.size > MINSTRTABSIZE*2)
    luaS_resize(L, G(L)->strt.size/2);

  if (luaZ_sizebuffer(&G(L)->buff) > LUA_MINBUFFER*2) {
    size_t newsize = luaZ_sizebuffer(&G(L)->buff) / 2;
    luaZ_resizebuffer(L, &G(L)->buff, newsize);
  }
  G(L)->GCthreshold = 2*G(L)->nblocks - deadmem;
}

// LIBRARY: SWBF2 0x006a3c80
// do1gcTM
static void do1gcTM (lua_State *L, Udata *udata) {
  const TObject *tm = fasttm(L, udata->uv.metatable, TM_GC);
  if (tm != NULL) {
    setobj2s(L->top, tm);
    setuvalue(L->top+1, udata);
    L->top += 2;
    luaD_call(L, L->top - 2, 0);
  }
}

// LIBRARY: SWBF2 0x006a3d80
// luaC_callGCTM
void luaC_callGCTM (lua_State *L) {
  lu_byte oldah = L->allowhook;
  L->allowhook = 0;
  L->top++;
  while (G(L)->tmudata != NULL) {
    GCObject *o = G(L)->tmudata;
    Udata *udata = gcotou(o);
    G(L)->tmudata = udata->uv.next;
    udata->uv.next = G(L)->rootudata;
    G(L)->rootudata = o;
    setuvalue(L->top - 1, udata);
    unmark(o);
    markfinalized(udata);
    do1gcTM(L, udata);
  }
  L->top--;
  L->allowhook = oldah;
}

// LIBRARY: SWBF2 0x006a3ef0
// luaC_sweep
void luaC_sweep (lua_State *L, int all) {
  if (all) all = 256;
  sweeplist(L, &G(L)->rootudata, all);
  sweepstrings(L, all);
  sweeplist(L, &G(L)->rootgc, all);
}

// LIBRARY: SWBF2 0x006a3fe0
// markroot
static void markroot (GCState *st, lua_State *L) {
  global_State *g = st->g;
  markobject(st, defaultmeta(L));
  markobject(st, registry(L));
  traversestack(st, g->mainthread);
  if (L != g->mainthread)
    markvalue(st, L);
}

// LIBRARY: SWBF2 0x006a3f30
// mark
static size_t mark (lua_State *L) {
  size_t deadmem;
  GCState st;
  GCObject *wkv;
  st.g = G(L);
  st.tmark = NULL;
  st.wkv = st.wk = st.wv = NULL;
  markroot(&st, L);
  propagatemarks(&st);
  cleartablevalues(st.wkv);
  cleartablevalues(st.wv);
  wkv = st.wkv;
  st.wkv = NULL;
  st.wv = NULL;
  deadmem = luaC_separateudata(L);
  marktmu(&st);
  propagatemarks(&st);
  cleartablekeys(wkv);

  cleartablekeys(st.wk);
  cleartablevalues(st.wv);
  cleartablekeys(st.wkv);
  cleartablevalues(st.wkv);
  return deadmem;
}

// LIBRARY: SWBF2 0x006a3df0
// luaC_collectgarbage
void luaC_collectgarbage (lua_State *L) {
  size_t deadmem = mark(L);
  luaC_sweep(L, 0);
  checkSizes(L, deadmem);
  luaC_callGCTM(L);
}

// LIBRARY: SWBF2 0x006a3e20
// luaC_link
void luaC_link (lua_State *L, GCObject *o, lu_byte tt) {
  o->gch.next = G(L)->rootgc;
  G(L)->rootgc = o;
  o->gch.marked = 0;
  o->gch.tt = tt;
}
