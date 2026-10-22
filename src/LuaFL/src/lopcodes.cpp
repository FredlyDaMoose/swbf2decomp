#define lopcodes_c

#include "lua.h"

#include "lobject.h"
#include "lopcodes.h"

#ifdef LUA_OPNAMES

const char *const luaP_opnames[] = {
  "MOVE",
  "LOADK",
  "LOADBOOL",
  "LOADNIL",
  "GETUPVAL",
  "GETGLOBAL",
  "GETTABLE",
  "SETGLOBAL",
  "SETUPVAL",
  "SETTABLE",
  "NEWTABLE",
  "SELF",
  "ADD",
  "SUB",
  "MUL",
  "DIV",
  "POW",
  "UNM",
  "NOT",
  "CONCAT",
  "JMP",
  "EQ",
  "LT",
  "LE",
  "TEST",
  "CALL",
  "TAILCALL",
  "RETURN",
  "FORLOOP",
  "TFORLOOP",
  "TFORPREP",
  "SETLIST",
  "SETLISTO",
  "CLOSE",
  "CLOSURE"
};

#endif

#define opmode(t,b,bk,ck,sa,k,m) (((t)<<OpModeT) | \
   ((b)<<OpModeBreg) | ((bk)<<OpModeBrk) | ((ck)<<OpModeCrk) | \
   ((sa)<<OpModesetA) | ((k)<<OpModeK) | (m))

const lu_byte luaP_opmodes[NUM_OPCODES] = {

  opmode(0, 1, 0, 0, 1, 0, iABC)
 ,opmode(0, 0, 0, 0, 1, 1, iABx)
 ,opmode(0, 0, 0, 0, 1, 0, iABC)
 ,opmode(0, 1, 0, 0, 1, 0, iABC)
 ,opmode(0, 0, 0, 0, 1, 0, iABC)
 ,opmode(0, 0, 0, 0, 1, 1, iABx)
 ,opmode(0, 1, 0, 1, 1, 0, iABC)
 ,opmode(0, 0, 0, 0, 0, 1, iABx)
 ,opmode(0, 0, 0, 0, 0, 0, iABC)
 ,opmode(0, 0, 1, 1, 0, 0, iABC)
 ,opmode(0, 0, 0, 0, 1, 0, iABC)
 ,opmode(0, 1, 0, 1, 1, 0, iABC)
 ,opmode(0, 0, 1, 1, 1, 0, iABC)
 ,opmode(0, 0, 1, 1, 1, 0, iABC)
 ,opmode(0, 0, 1, 1, 1, 0, iABC)
 ,opmode(0, 0, 1, 1, 1, 0, iABC)
 ,opmode(0, 0, 1, 1, 1, 0, iABC)
 ,opmode(0, 1, 0, 0, 1, 0, iABC)
 ,opmode(0, 1, 0, 0, 1, 0, iABC)
 ,opmode(0, 1, 0, 1, 1, 0, iABC)
 ,opmode(0, 0, 0, 0, 0, 0, iAsBx)
 ,opmode(1, 0, 1, 1, 0, 0, iABC)
 ,opmode(1, 0, 1, 1, 0, 0, iABC)
 ,opmode(1, 0, 1, 1, 0, 0, iABC)
 ,opmode(1, 1, 0, 0, 1, 0, iABC)
 ,opmode(0, 0, 0, 0, 0, 0, iABC)
 ,opmode(0, 0, 0, 0, 0, 0, iABC)
 ,opmode(0, 0, 0, 0, 0, 0, iABC)
 ,opmode(0, 0, 0, 0, 0, 0, iAsBx)
 ,opmode(1, 0, 0, 0, 0, 0, iABC)
 ,opmode(0, 0, 0, 0, 0, 0, iAsBx)
 ,opmode(0, 0, 0, 0, 0, 0, iABx)
 ,opmode(0, 0, 0, 0, 0, 0, iABx)
 ,opmode(0, 0, 0, 0, 0, 0, iABC)
 ,opmode(0, 0, 0, 0, 1, 0, iABx)
};
