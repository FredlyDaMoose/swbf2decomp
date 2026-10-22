#ifndef llimits_h
#define llimits_h

#include <limits.h>
#include <stddef.h>

#include "lua.h"

#ifndef BITS_INT

#if INT_MAX-20 < 32760
#define	BITS_INT	16
#else
#if INT_MAX > 2147483640L

#define BITS_INT	32
#else
#error "you must define BITS_INT with number of bits in an integer"
#endif
#endif
#endif

typedef unsigned int lu_hash;

typedef int ls_hash;

typedef unsigned long lu_mem;

#define MAX_LUMEM	ULONG_MAX

typedef long ls_nstr;

typedef unsigned char lu_byte;

#define MAX_SIZET	((size_t)(~(size_t)0)-2)

#define MAX_INT (INT_MAX-2)

#define IntPoint(p)  ((lu_hash)(p))

#ifndef LUSER_ALIGNMENT_T
typedef union { double u; void *s; long l; } L_Umaxalign;
#else
typedef LUSER_ALIGNMENT_T L_Umaxalign;
#endif

#ifndef LUA_UACNUMBER
typedef double l_uacNumber;
#else
typedef LUA_UACNUMBER l_uacNumber;
#endif

#ifndef lua_assert
#define lua_assert(c)
#endif

#ifndef check_exp
#define check_exp(c,e)	(e)
#endif

#ifndef UNUSED
#define UNUSED(x)	((void)(x))
#endif

#ifndef cast
#define cast(t, exp)	((t)(exp))
#endif

typedef unsigned long Instruction;

#ifndef LUA_MAXCALLS
#define LUA_MAXCALLS        4096
#endif

#ifndef LUA_MAXCCALLS
#define LUA_MAXCCALLS        200
#endif

#ifndef LUA_MAXCSTACK
#define LUA_MAXCSTACK        2048
#endif

#define MAXSTACK	128

#ifndef MAXVARS
#define MAXVARS 100
#endif

#ifndef MAXUPVALUES
#define MAXUPVALUES	32
#endif

#ifndef MAXPARAMS
#define MAXPARAMS 100
#endif

#ifndef MINSTRTABSIZE
#define MINSTRTABSIZE	32
#endif

#ifndef LUA_MINBUFFER
#define LUA_MINBUFFER	32
#endif

#ifndef LUA_MAXPARSERLEVEL
#define LUA_MAXPARSERLEVEL	200
#endif

#endif
