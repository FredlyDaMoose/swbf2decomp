#ifndef lopcodes_h
#define lopcodes_h

#include "llimits.h"

enum OpMode {iABC, iABx, iAsBx};

#define SIZE_C		9
#define SIZE_B		9
#define SIZE_Bx		(SIZE_C + SIZE_B)
#define SIZE_A		8

#define SIZE_OP		6

#define POS_C		SIZE_OP
#define POS_B		(POS_C + SIZE_C)
#define POS_Bx		POS_C
#define POS_A		(POS_B + SIZE_B)

#if SIZE_Bx < BITS_INT-1
#define MAXARG_Bx        ((1<<SIZE_Bx)-1)
#define MAXARG_sBx        (MAXARG_Bx>>1)
#else
#define MAXARG_Bx        MAX_INT
#define MAXARG_sBx        MAX_INT
#endif

#define MAXARG_A        ((1<<SIZE_A)-1)
#define MAXARG_B        ((1<<SIZE_B)-1)
#define MAXARG_C        ((1<<SIZE_C)-1)

#define MASK1(n,p)	((~((~(Instruction)0)<<n))<<p)

#define MASK0(n,p)	(~MASK1(n,p))

#define GET_OPCODE(i)	(cast(OpCode, (i)&MASK1(SIZE_OP,0)))
#define SET_OPCODE(i,o)	((i) = (((i)&MASK0(SIZE_OP,0)) | cast(Instruction, o)))

#define GETARG_A(i)	(cast(int, (i)>>POS_A))
#define SETARG_A(i,u)	((i) = (((i)&MASK0(SIZE_A,POS_A)) | \
		((cast(Instruction, u)<<POS_A)&MASK1(SIZE_A,POS_A))))

#define GETARG_B(i)	(cast(int, ((i)>>POS_B) & MASK1(SIZE_B,0)))
#define SETARG_B(i,b)	((i) = (((i)&MASK0(SIZE_B,POS_B)) | \
		((cast(Instruction, b)<<POS_B)&MASK1(SIZE_B,POS_B))))

#define GETARG_C(i)	(cast(int, ((i)>>POS_C) & MASK1(SIZE_C,0)))
#define SETARG_C(i,b)	((i) = (((i)&MASK0(SIZE_C,POS_C)) | \
		((cast(Instruction, b)<<POS_C)&MASK1(SIZE_C,POS_C))))

#define GETARG_Bx(i)	(cast(int, ((i)>>POS_Bx) & MASK1(SIZE_Bx,0)))
#define SETARG_Bx(i,b)	((i) = (((i)&MASK0(SIZE_Bx,POS_Bx)) | \
		((cast(Instruction, b)<<POS_Bx)&MASK1(SIZE_Bx,POS_Bx))))

#define GETARG_sBx(i)	(GETARG_Bx(i)-MAXARG_sBx)
#define SETARG_sBx(i,b)	SETARG_Bx((i),cast(unsigned int, (b)+MAXARG_sBx))

#define CREATE_ABC(o,a,b,c)	(cast(Instruction, o) \
			| (cast(Instruction, a)<<POS_A) \
			| (cast(Instruction, b)<<POS_B) \
			| (cast(Instruction, c)<<POS_C))

#define CREATE_ABx(o,a,bc)	(cast(Instruction, o) \
			| (cast(Instruction, a)<<POS_A) \
			| (cast(Instruction, bc)<<POS_Bx))

#define NO_REG		MAXARG_A

typedef enum {

OP_MOVE,
OP_LOADK,
OP_LOADBOOL,
OP_LOADNIL,
OP_GETUPVAL,

OP_GETGLOBAL,
OP_GETTABLE,

OP_SETGLOBAL,
OP_SETUPVAL,
OP_SETTABLE,

OP_NEWTABLE,

OP_SELF,

OP_ADD,
OP_SUB,
OP_MUL,
OP_DIV,
OP_POW,
OP_UNM,
OP_NOT,

OP_CONCAT,

OP_JMP,

OP_EQ,
OP_LT,
OP_LE,

OP_TEST,

OP_CALL,
OP_TAILCALL,
OP_RETURN,

OP_FORLOOP,

OP_TFORLOOP,
OP_TFORPREP,

OP_SETLIST,
OP_SETLISTO,

OP_CLOSE,
OP_CLOSURE
} OpCode;

#define NUM_OPCODES	(cast(int, OP_CLOSURE+1))

enum OpModeMask {
  OpModeBreg = 2,
  OpModeBrk,
  OpModeCrk,
  OpModesetA,
  OpModeK,
  OpModeT

};

extern const lu_byte luaP_opmodes[NUM_OPCODES];

#define getOpMode(m)            (cast(enum OpMode, luaP_opmodes[m] & 3))
#define testOpMode(m, b)        (luaP_opmodes[m] & (1 << (b)))

#ifdef LUA_OPNAMES
extern const char *const luaP_opnames[];
#endif

#define LFIELDS_PER_FLUSH	32

#endif
