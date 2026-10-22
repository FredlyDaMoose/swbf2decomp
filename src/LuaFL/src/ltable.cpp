#include <string.h>

#define ltable_c

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "ltable.h"

#if BITS_INT > 26
#define MAXBITS		24
#else
#define MAXBITS		(BITS_INT-2)
#endif

#define toobig(x)	((((x)-1) >> MAXBITS) != 0)

#ifndef lua_number2int
#define lua_number2int(i,n)	((i)=(int)(n))
#endif

#define hashpow2(t,n)      (gnode(t, lmod((n), sizenode(t))))

#define hashstr(t,str)  hashpow2(t, (str)->tsv.hash)
#define hashboolean(t,p)        hashpow2(t, p)

#define hashmod(t,n)	(gnode(t, ((n) % ((sizenode(t)-1)|1))))

#define hashpointer(t,p)	hashmod(t, IntPoint(p))

#define numints		cast(int, sizeof(lua_Number)/sizeof(int))

// LIBRARY: SWBF2 0x006a4910
// hashnum
static Node *hashnum (const Table *t, lua_Number n) {
  unsigned int a[numints];
  int i;
  n += 1;
  lua_assert(sizeof(a) <= sizeof(n));
  memcpy(a, &n, sizeof(a));
  for (i = 1; i < numints; i++) a[0] += a[i];
  return hashmod(t, cast(lu_hash, a[0]));
}

// LIBRARY: SWBF2 0x006a4b60
// luaH_mainposition
Node *luaH_mainposition (const Table *t, const TObject *key) {
  switch (ttype(key)) {
    case LUA_TNUMBER:
      return hashnum(t, nvalue(key));
    case LUA_TSTRING:
      return hashstr(t, tsvalue(key));
    case LUA_TBOOLEAN:
      return hashboolean(t, bvalue(key));
    case LUA_TLIGHTUSERDATA:
      return hashpointer(t, pvalue(key));
    default:
      return hashpointer(t, gcvalue(key));
  }
}

// LIBRARY: SWBF2 0x006a4840
// arrayindex
static int arrayindex (const TObject *key) {
  if (ttisnumber(key)) {
    int k;
    lua_number2int(k, (nvalue(key)));
    if (cast(lua_Number, k) == nvalue(key) && k >= 1 && !toobig(k))
      return k;
  }
  return -1;
}

// LIBRARY: SWBF2 0x006a4af0
// luaH_index
static int luaH_index (lua_State *L, Table *t, StkId key) {
  int i;
  if (ttisnil(key)) return -1;
  i = arrayindex(key);
  if (0 <= i && i <= t->sizearray) {
    return i-1;
  }
  else {
    const TObject *v = luaH_get(t, key);
    if (v == &luaO_nilobject)
      luaG_runerror(L, "invalid key for `next'");
    i = cast(int, (cast(const lu_byte *, v) -
                   cast(const lu_byte *, gval(gnode(t, 0)))) / sizeof(Node));
    return i + t->sizearray;
  }
}

// LIBRARY: SWBF2 0x006a4c70
// luaH_next
int luaH_next (lua_State *L, Table *t, StkId key) {
  int i = luaH_index(L, t, key);
  for (i++; i < t->sizearray; i++) {
    if (!ttisnil(&t->array[i])) {
      setnvalue(key, cast(lua_Number, i+1));
      setobj2s(key+1, &t->array[i]);
      return 1;
    }
  }
  for (i -= t->sizearray; i < sizenode(t); i++) {
    if (!ttisnil(gval(gnode(t, i)))) {
      setobj2s(key, gkey(gnode(t, i)));
      setobj2s(key+1, gval(gnode(t, i)));
      return 1;
    }
  }
  return 0;
}

// LIBRARY: SWBF2 0x006a4880
// computesizes
static void computesizes  (int nums[], int ntotal, int *narray, int *nhash) {
  int i;
  int a = nums[0];
  int na = a;
  int n = (na == 0) ? -1 : 0;
  for (i = 1; a < *narray && *narray >= twoto(i-1); i++) {
    if (nums[i] > 0) {
      a += nums[i];
      if (a >= twoto(i-1)) {
        n = i;
        na = a;
      }
    }
  }
  lua_assert(na <= *narray && *narray <= ntotal);
  *nhash = ntotal - na;
  *narray = (n == -1) ? 0 : twoto(n);
  lua_assert(na <= *narray && na >= *narray/2);
}

// LIBRARY: SWBF2 0x006a4ed0
// numuse
static void numuse (const Table *t, int *narray, int *nhash) {
  int nums[MAXBITS+1];
  int i, lg;
  int totaluse = 0;

  for (i=0, lg=0; lg<=MAXBITS; lg++) {
    int ttlg = twoto(lg);
    if (ttlg > t->sizearray) {
      ttlg = t->sizearray;
      if (i >= ttlg) break;
    }
    nums[lg] = 0;
    for (; i<ttlg; i++) {
      if (!ttisnil(&t->array[i])) {
        nums[lg]++;
        totaluse++;
      }
    }
  }
  for (; lg<=MAXBITS; lg++) nums[lg] = 0;
  *narray = totaluse;

  i = sizenode(t);
  while (i--) {
    Node *n = &t->node[i];
    if (!ttisnil(gval(n))) {
      int k = arrayindex(gkey(n));
      if (k >= 0) {
        nums[luaO_log2(k-1)+1]++;
        (*narray)++;
      }
      totaluse++;
    }
  }
  computesizes(nums, totaluse, narray, nhash);
}

// LIBRARY: SWBF2 0x006a5190
// setarrayvector
static void setarrayvector (lua_State *L, Table *t, int size) {
  int i;
  luaM_reallocvector(L, t->array, t->sizearray, size, TObject);
  for (i=t->sizearray; i<size; i++)
     setnilvalue(&t->array[i]);
  t->sizearray = size;
}

// LIBRARY: SWBF2 0x006a51e0
// setnodevector
static void setnodevector (lua_State *L, Table *t, int lsize) {
  int i;
  int size = twoto(lsize);
  if (lsize > MAXBITS)
    luaG_runerror(L, "table overflow");
  if (lsize == 0) {
    t->node = G(L)->dummynode;
    lua_assert(ttisnil(gkey(t->node)));
    lua_assert(ttisnil(gval(t->node)));
    lua_assert(t->node->next == NULL);
  }
  else {
    t->node = luaM_newvector(L, size, Node);
    for (i=0; i<size; i++) {
      t->node[i].next = NULL;
      setnilvalue(gkey(gnode(t, i)));
      setnilvalue(gval(gnode(t, i)));
    }
  }
  t->lsizenode = cast(lu_byte, lsize);
  t->firstfree = gnode(t, size-1);
}

// LIBRARY: SWBF2 0x006a5050
// resize
static void resize (lua_State *L, Table *t, int nasize, int nhsize) {
  int i;
  int oldasize = t->sizearray;
  int oldhsize = t->lsizenode;
  Node *nold;
  Node temp[1];
  if (oldhsize)
    nold = t->node;
  else {
    lua_assert(t->node == G(L)->dummynode);
    temp[0] = t->node[0];
    nold = temp;
    setnilvalue(gkey(G(L)->dummynode));
    setnilvalue(gval(G(L)->dummynode));
    lua_assert(G(L)->dummynode->next == NULL);
  }
  if (nasize > oldasize)
    setarrayvector(L, t, nasize);

  setnodevector(L, t, nhsize);

  if (nasize < oldasize) {
    t->sizearray = nasize;

    for (i=nasize; i<oldasize; i++) {
      if (!ttisnil(&t->array[i]))
        setobjt2t(luaH_setnum(L, t, i+1), &t->array[i]);
    }

    luaM_reallocvector(L, t->array, oldasize, nasize, TObject);
  }

  for (i = twoto(oldhsize) - 1; i >= 0; i--) {
    Node *old = nold+i;
    if (!ttisnil(gval(old)))
      setobjt2t(luaH_set(L, t, gkey(old)), gval(old));
  }
  if (oldhsize)
    luaM_freearray(L, nold, twoto(oldhsize), Node);
}

// LIBRARY: SWBF2 0x006a5010
// rehash
static void rehash (lua_State *L, Table *t) {
  int nasize, nhsize;
  numuse(t, &nasize, &nhsize);
  resize(L, t, nasize, luaO_log2(nhsize)+1);
}

// LIBRARY: SWBF2 0x006a4c10
// luaH_new
Table *luaH_new (lua_State *L, int narray, int lnhash) {
  Table *t = luaM_new(L, Table);
  luaC_link(L, valtogco(t), LUA_TTABLE);
  t->metatable = hvalue(defaultmeta(L));
  t->flags = cast(lu_byte, ~0);

  t->array = NULL;
  t->sizearray = 0;
  t->lsizenode = 0;
  t->node = NULL;
  setarrayvector(L, t, narray);
  setnodevector(L, t, lnhash);
  return t;
}

// LIBRARY: SWBF2 0x006a4950
// luaH_free
void luaH_free (lua_State *L, Table *t) {
  if (t->lsizenode)
    luaM_freearray(L, t->node, sizenode(t), Node);
  luaM_freearray(L, t->array, t->sizearray, TObject);
  luaM_freelem(L, t);
}

#if 0

void luaH_remove (Table *t, Node *e) {
  Node *mp = luaH_mainposition(t, gkey(e));
  if (e != mp) {
    while (mp->next != e) mp = mp->next;
    mp->next = e->next;
  }
  else {
    if (e->next != NULL) ??
  }
  lua_assert(ttisnil(gval(node)));
  setnilvalue(gkey(e));
  e->next = NULL;
}
#endif

// LIBRARY: SWBF2 0x006a4e00
// newkey
static TObject *newkey (lua_State *L, Table *t, const TObject *key) {
  TObject *val;
  Node *mp = luaH_mainposition(t, key);
  if (!ttisnil(gval(mp))) {
    Node *othern = luaH_mainposition(t, gkey(mp));
    Node *n = t->firstfree;
    if (othern != mp) {

      while (othern->next != mp) othern = othern->next;
      othern->next = n;
      *n = *mp;
      mp->next = NULL;
      setnilvalue(gval(mp));
    }
    else {

      n->next = mp->next;
      mp->next = n;
      mp = n;
    }
  }
  setobj2t(gkey(mp), key);
  lua_assert(ttisnil(gval(mp)));
  for (;;) {
    if (ttisnil(gkey(t->firstfree)))
      return gval(mp);
    else if (t->firstfree == t->node) break;
    else (t->firstfree)--;
  }

  setbvalue(gval(mp), 0);
  rehash(L, t);
  val = cast(TObject *, luaH_get(t, key));
  lua_assert(ttisboolean(val));
  setnilvalue(val);
  return val;
}

static const TObject *luaH_getany (Table *t, const TObject *key) {
  if (ttisnil(key)) return &luaO_nilobject;
  else {
    Node *n = luaH_mainposition(t, key);
    do {
      if (luaO_rawequalObj(gkey(n), key)) return gval(n);
      else n = n->next;
    } while (n);
    return &luaO_nilobject;
  }
}

// LIBRARY: SWBF2 0x006a4a50
// luaH_getnum
const TObject *luaH_getnum (Table *t, int key) {
  if (1 <= key && key <= t->sizearray)
    return &t->array[key-1];
  else {
    lua_Number nk = cast(lua_Number, key);
    Node *n = hashnum(t, nk);
    do {
      if (ttisnumber(gkey(n)) && nvalue(gkey(n)) == nk)
        return gval(n);
      else n = n->next;
    } while (n);
    return &luaO_nilobject;
  }
}

// LIBRARY: SWBF2 0x006a4ab0
// luaH_getstr
const TObject *luaH_getstr (Table *t, TString *key) {
  Node *n = hashstr(t, key);
  do {
    if (ttisstring(gkey(n)) && tsvalue(gkey(n)) == key)
      return gval(n);
    else n = n->next;
  } while (n);
  return &luaO_nilobject;
}

// LIBRARY: SWBF2 0x006a49b0
// luaH_get
const TObject *luaH_get (Table *t, const TObject *key) {
  switch (ttype(key)) {
    case LUA_TSTRING: return luaH_getstr(t, tsvalue(key));
    case LUA_TNUMBER: {
      int k;
      lua_number2int(k, (nvalue(key)));
      if (cast(lua_Number, k) == nvalue(key))
        return luaH_getnum(t, k);

    }
    default: return luaH_getany(t, key);
  }
}

// LIBRARY: SWBF2 0x006a4d40
// luaH_set
TObject *luaH_set (lua_State *L, Table *t, const TObject *key) {
  const TObject *p = luaH_get(t, key);
  t->flags = 0;
  if (p != &luaO_nilobject)
    return cast(TObject *, p);
  else {
    if (ttisnil(key)) luaG_runerror(L, "table index is nil");
    else if (ttisnumber(key) && nvalue(key) != nvalue(key))
      luaG_runerror(L, "table index is NaN");
    return newkey(L, t, key);
  }
}

// LIBRARY: SWBF2 0x006a4db0
// luaH_setnum
TObject *luaH_setnum (lua_State *L, Table *t, int key) {
  const TObject *p = luaH_getnum(t, key);
  if (p != &luaO_nilobject)
    return cast(TObject *, p);
  else {
    TObject k;
    setnvalue(&k, cast(lua_Number, key));
    return newkey(L, t, &k);
  }
}
