#ifndef lobject_h
#define lobject_h

#include "llimits.h"
#include "lua.h"

#define NUM_TAGS	LUA_TTHREAD

#define LUA_TPROTO	(NUM_TAGS+1)
#define LUA_TUPVAL	(NUM_TAGS+2)

typedef union GCObject GCObject;

#define CommonHeader	GCObject *next; lu_byte tt; lu_byte marked

typedef struct GCheader {
  CommonHeader;
} GCheader;

typedef union {
  GCObject *gc;
  void *p;
  lua_Number n;
  int b;
} Value;

typedef struct lua_TObject {
  int tt;
  Value value;
} TObject;

#define ttisnil(o)	(ttype(o) == LUA_TNIL)
#define ttisnumber(o)	(ttype(o) == LUA_TNUMBER)
#define ttisstring(o)	(ttype(o) == LUA_TSTRING)
#define ttistable(o)	(ttype(o) == LUA_TTABLE)
#define ttisfunction(o)	(ttype(o) == LUA_TFUNCTION)
#define ttisboolean(o)	(ttype(o) == LUA_TBOOLEAN)
#define ttisuserdata(o)	(ttype(o) == LUA_TUSERDATA)
#define ttisthread(o)	(ttype(o) == LUA_TTHREAD)
#define ttislightuserdata(o)	(ttype(o) == LUA_TLIGHTUSERDATA)

#define ttype(o)	((o)->tt)
#define gcvalue(o)	check_exp(iscollectable(o), (o)->value.gc)
#define pvalue(o)	check_exp(ttislightuserdata(o), (o)->value.p)
#define nvalue(o)	check_exp(ttisnumber(o), (o)->value.n)
#define tsvalue(o)	check_exp(ttisstring(o), &(o)->value.gc->ts)
#define uvalue(o)	check_exp(ttisuserdata(o), &(o)->value.gc->u)
#define clvalue(o)	check_exp(ttisfunction(o), &(o)->value.gc->cl)
#define hvalue(o)	check_exp(ttistable(o), &(o)->value.gc->h)
#define bvalue(o)	check_exp(ttisboolean(o), (o)->value.b)
#define thvalue(o)	check_exp(ttisthread(o), &(o)->value.gc->th)

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0))

#define setnvalue(obj,x) \
  { TObject *i_o=(obj); i_o->tt=LUA_TNUMBER; i_o->value.n=(x); }

#define chgnvalue(obj,x) \
	check_exp(ttype(obj)==LUA_TNUMBER, (obj)->value.n=(x))

#define setpvalue(obj,x) \
  { TObject *i_o=(obj); i_o->tt=LUA_TLIGHTUSERDATA; i_o->value.p=(x); }

#define setbvalue(obj,x) \
  { TObject *i_o=(obj); i_o->tt=LUA_TBOOLEAN; i_o->value.b=(x); }

#define setsvalue(obj,x) \
  { TObject *i_o=(obj); i_o->tt=LUA_TSTRING; \
    i_o->value.gc=cast(GCObject *, (x)); \
    lua_assert(i_o->value.gc->gch.tt == LUA_TSTRING); }

#define setuvalue(obj,x) \
  { TObject *i_o=(obj); i_o->tt=LUA_TUSERDATA; \
    i_o->value.gc=cast(GCObject *, (x)); \
    lua_assert(i_o->value.gc->gch.tt == LUA_TUSERDATA); }

#define setthvalue(obj,x) \
  { TObject *i_o=(obj); i_o->tt=LUA_TTHREAD; \
    i_o->value.gc=cast(GCObject *, (x)); \
    lua_assert(i_o->value.gc->gch.tt == LUA_TTHREAD); }

#define setclvalue(obj,x) \
  { TObject *i_o=(obj); i_o->tt=LUA_TFUNCTION; \
    i_o->value.gc=cast(GCObject *, (x)); \
    lua_assert(i_o->value.gc->gch.tt == LUA_TFUNCTION); }

#define sethvalue(obj,x) \
  { TObject *i_o=(obj); i_o->tt=LUA_TTABLE; \
    i_o->value.gc=cast(GCObject *, (x)); \
    lua_assert(i_o->value.gc->gch.tt == LUA_TTABLE); }

#define setnilvalue(obj) ((obj)->tt=LUA_TNIL)

#define checkconsistency(obj) \
  lua_assert(!iscollectable(obj) || (ttype(obj) == (obj)->value.gc->gch.tt))

#define setobj(obj1,obj2) \
  { const TObject *o2=(obj2); TObject *o1=(obj1); \
    checkconsistency(o2); \
    o1->tt=o2->tt; o1->value = o2->value; }

#define setobjs2s	setobj

#define setobj2s	setobj
#define setsvalue2s	setsvalue

#define setobjt2t	setobj

#define setobj2t	setobj

#define setobj2n	setobj
#define setsvalue2n	setsvalue

#define setttype(obj, tt) (ttype(obj) = (tt))

#define iscollectable(o)	(ttype(o) >= LUA_TSTRING)

typedef TObject *StkId;

typedef union TString {
  L_Umaxalign dummy;
  struct {
    CommonHeader;
    lu_byte reserved;
    lu_hash hash;
    size_t len;
  } tsv;
} TString;

#define getstr(ts)	cast(const char *, (ts) + 1)
#define svalue(o)       getstr(tsvalue(o))

typedef union Udata {
  L_Umaxalign dummy;
  struct {
    CommonHeader;
    struct Table *metatable;
    size_t len;
  } uv;
} Udata;

typedef struct Proto {
  CommonHeader;
  TObject *k;
  Instruction *code;
  struct Proto **p;
  int *lineinfo;
  struct LocVar *locvars;
  TString **upvalues;
  TString  *source;
  int sizeupvalues;
  int sizek;
  int sizecode;
  int sizelineinfo;
  int sizep;
  int sizelocvars;
  int lineDefined;
  GCObject *gclist;
  lu_byte nups;
  lu_byte numparams;
  lu_byte is_vararg;
  lu_byte maxstacksize;
} Proto;

typedef struct LocVar {
  TString *varname;
  int startpc;
  int endpc;
} LocVar;

typedef struct UpVal {
  CommonHeader;
  TObject *v;
  TObject value;
} UpVal;

#define ClosureHeader \
	CommonHeader; lu_byte isC; lu_byte nupvalues; GCObject *gclist

typedef struct CClosure {
  ClosureHeader;
  lua_CFunction f;
  TObject upvalue[1];
} CClosure;

typedef struct LClosure {
  ClosureHeader;
  struct Proto *p;
  TObject g;
  UpVal *upvals[1];
} LClosure;

typedef union Closure {
  CClosure c;
  LClosure l;
} Closure;

#define iscfunction(o)	(ttype(o) == LUA_TFUNCTION && clvalue(o)->c.isC)
#define isLfunction(o)	(ttype(o) == LUA_TFUNCTION && !clvalue(o)->c.isC)

typedef struct Node {
  TObject i_key;
  TObject i_val;
  struct Node *next;
} Node;

typedef struct Table {
  CommonHeader;
  lu_byte flags;
  lu_byte lsizenode;
  struct Table *metatable;
  TObject *array;
  Node *node;
  Node *firstfree;
  GCObject *gclist;
  int sizearray;
} Table;

#define lmod(s,size) \
	check_exp((size&(size-1))==0, (cast(int, (s) & ((size)-1))))

#define twoto(x)	(1<<(x))
#define sizenode(t)	(twoto((t)->lsizenode))

extern const TObject luaO_nilobject;

int luaO_log2 (unsigned int x);
int luaO_int2fb (unsigned int x);
#define fb2int(x)	(((x) & 7) << ((x) >> 3))

int luaO_rawequalObj (const TObject *t1, const TObject *t2);
int luaO_str2d (const char *s, lua_Number *result);

const char *luaO_pushvfstring (lua_State *L, const char *fmt, va_list argp);
const char *luaO_pushfstring (lua_State *L, const char *fmt, ...);
void luaO_chunkid (char *out, const char *source, int len);

#endif
