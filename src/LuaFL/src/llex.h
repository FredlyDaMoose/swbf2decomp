#ifndef llex_h
#define llex_h

#include "lobject.h"
#include "lzio.h"

#define FIRST_RESERVED	257

#define TOKEN_LEN	(sizeof("function")/sizeof(char))

enum RESERVED {

  TK_AND = FIRST_RESERVED, TK_BREAK,
  TK_DO, TK_ELSE, TK_ELSEIF, TK_END, TK_FALSE, TK_FOR, TK_FUNCTION,
  TK_IF, TK_IN, TK_LOCAL, TK_NIL, TK_NOT, TK_OR, TK_REPEAT,
  TK_RETURN, TK_THEN, TK_TRUE, TK_UNTIL, TK_WHILE,

  TK_NAME, TK_CONCAT, TK_DOTS, TK_EQ, TK_GE, TK_LE, TK_NE, TK_NUMBER,
  TK_STRING, TK_EOS
};

#define NUM_RESERVED	(cast(int, TK_WHILE-FIRST_RESERVED+1))

typedef union {
  lua_Number r;
  TString *ts;
} SemInfo;

typedef struct Token {
  int token;
  SemInfo seminfo;
} Token;

typedef struct LexState {
  int current;
  int linenumber;
  int lastline;
  Token t;
  Token lookahead;
  struct FuncState *fs;
  struct lua_State *L;
  ZIO *z;
  Mbuffer *buff;
  TString *source;
  int nestlevel;
} LexState;

void luaX_init (lua_State *L);
void luaX_setinput (lua_State *L, LexState *LS, ZIO *z, TString *source);
int luaX_lex (LexState *LS, SemInfo *seminfo);
void luaX_checklimit (LexState *ls, int val, int limit, const char *msg);
void luaX_syntaxerror (LexState *ls, const char *s);
void luaX_errorline (LexState *ls, const char *s, const char *token, int line);
const char *luaX_token2str (LexState *ls, int token);

#endif
