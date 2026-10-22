#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define lstrlib_c

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#ifndef uchar
#define uchar(c)        ((unsigned char)(c))
#endif

typedef long sint32;

// LIBRARY: SWBF2 0x0069e410
// str_len
static int str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushnumber(L, (lua_Number)l);
  return 1;
}

// LIBRARY: SWBF2 0x0069f340
// posrelat
static sint32 posrelat (sint32 pos, size_t len) {

  return (pos>=0) ? pos : (sint32)len+pos+1;
}

// LIBRARY: SWBF2 0x0069e460
// str_sub
static int str_sub (lua_State *L) {
  size_t l;
  const char *s = luaL_checklstring(L, 1, &l);
  sint32 start = posrelat(luaL_checklong(L, 2), l);
  sint32 end = posrelat(luaL_optlong(L, 3, -1), l);
  if (start < 1) start = 1;
  if (end > (sint32)l) end = (sint32)l;
  if (start <= end)
    lua_pushlstring(L, s+start-1, end-start+1);
  else lua_pushliteral(L, "");
  return 1;
}

// LIBRARY: SWBF2 0x0069e500
// str_lower
static int str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  const char *s = luaL_checklstring(L, 1, &l);
  luaL_buffinit(L, &b);
  for (i=0; i<l; i++)
    luaL_putchar(&b, tolower(uchar(s[i])));
  luaL_pushresult(&b);
  return 1;
}

// LIBRARY: SWBF2 0x0069e5c0
// str_upper
static int str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  const char *s = luaL_checklstring(L, 1, &l);
  luaL_buffinit(L, &b);
  for (i=0; i<l; i++)
    luaL_putchar(&b, toupper(uchar(s[i])));
  luaL_pushresult(&b);
  return 1;
}

// LIBRARY: SWBF2 0x0069e680
// str_rep
static int str_rep (lua_State *L) {
  size_t l;
  luaL_Buffer b;
  const char *s = luaL_checklstring(L, 1, &l);
  int n = luaL_checkint(L, 2);
  luaL_buffinit(L, &b);
  while (n-- > 0)
    luaL_addlstring(&b, s, l);
  luaL_pushresult(&b);
  return 1;
}

// LIBRARY: SWBF2 0x0069daf0
// str_byte
static int str_byte (lua_State *L) {
  size_t l;
  const char *s = luaL_checklstring(L, 1, &l);
  sint32 pos = posrelat(luaL_optlong(L, 2, 1), l);
  if (pos <= 0 || (size_t)(pos) > l)
    return 0;
  lua_pushnumber(L, uchar(s[pos-1]));
  return 1;
}

// LIBRARY: SWBF2 0x0069db70
// str_char
static int str_char (lua_State *L) {
  int n = lua_gettop(L);
  int i;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  for (i=1; i<=n; i++) {
    int c = luaL_checkint(L, i);
    luaL_argcheck(L, uchar(c) == c, i, "invalid value");
    luaL_putchar(&b, uchar(c));
  }
  luaL_pushresult(&b);
  return 1;
}

// FUNCTION: SWBF2 0x0069f580
static int writer (lua_State *L, const void* b, size_t size, void* B) {
  (void)L;
  luaL_addlstring((luaL_Buffer*) B, (const char *)b, size);
  return 1;
}

// LIBRARY: SWBF2 0x0069dc40
// str_dump
static int str_dump (lua_State *L) {
  luaL_Buffer b;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  luaL_buffinit(L,&b);
  if (!lua_dump(L, writer, &b))
    luaL_error(L, "unable to dump given function");
  luaL_pushresult(&b);
  return 1;
}

#ifndef MAX_CAPTURES
#define MAX_CAPTURES 32
#endif

#define CAP_UNFINISHED	(-1)
#define CAP_POSITION	(-2)

typedef struct MatchState {
  const char *src_init;
  const char *src_end;
  lua_State *L;
  int level;
  struct {
    const char *init;
    sint32 len;
  } capture[MAX_CAPTURES];
} MatchState;

#define ESC		'%'
#define SPECIALS	"^$*+?.([%-"

// LIBRARY: SWBF2 0x0069e880
// check_capture
static int check_capture (MatchState *ms, int l) {
  l -= '1';
  if (l < 0 || l >= ms->level || ms->capture[l].len == CAP_UNFINISHED)
    return luaL_error(ms->L, "invalid capture index");
  return l;
}

// LIBRARY: SWBF2 0x0069e850
// capture_to_close
static int capture_to_close (MatchState *ms) {
  int level = ms->level;
  for (level--; level>=0; level--)
    if (ms->capture[level].len == CAP_UNFINISHED) return level;
  return luaL_error(ms->L, "invalid pattern capture");
}

// LIBRARY: SWBF2 0x0069ec10
// luaI_classend
static const char *luaI_classend (MatchState *ms, const char *p) {
  switch (*p++) {
    case ESC: {
      if (*p == '\0')
        luaL_error(ms->L, "malformed pattern (ends with `%')");
      return p+1;
    }
    case '[': {
      if (*p == '^') p++;
      do {
        if (*p == '\0')
          luaL_error(ms->L, "malformed pattern (missing `]')");
        if (*(p++) == ESC && *p != '\0')
          p++;
      } while (*p != ']');
      return p+1;
    }
    default: {
      return p;
    }
  }
}

// FUNCTION: SWBF2 0x0069f030
static int match_class (int c, int cl) {
  int res;
  switch (tolower(cl)) {
    case 'a' : res = isalpha(c); break;
    case 'c' : res = iscntrl(c); break;
    case 'd' : res = isdigit(c); break;
    case 'l' : res = islower(c); break;
    case 'p' : res = ispunct(c); break;
    case 's' : res = isspace(c); break;
    case 'u' : res = isupper(c); break;
    case 'w' : res = isalnum(c); break;
    case 'x' : res = isxdigit(c); break;
    case 'z' : res = (c == 0); break;
    default: return (cl == c);
  }
  return (islower(cl) ? res : !res);
}

// LIBRARY: SWBF2 0x0069f1e0
// matchbracketclass
static int matchbracketclass (int c, const char *p, const char *ec) {
  int sig = 1;
  if (*(p+1) == '^') {
    sig = 0;
    p++;
  }
  while (++p < ec) {
    if (*p == ESC) {
      p++;
      if (match_class(c, *p))
        return sig;
    }
    else if ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      if (uchar(*(p-2)) <= c && c <= uchar(*p))
        return sig;
    }
    else if (uchar(*p) == c) return sig;
  }
  return !sig;
}

// FUNCTION: SWBF2 0x0069ec80
static int luaI_singlematch (int c, const char *p, const char *ep) {
  switch (*p) {
    case '.': return 1;
    case ESC: return match_class(c, *(p+1));
    case '[': return matchbracketclass(c, p, ep-1);
    default:  return (uchar(*p) == c);
  }
}

static const char *match (MatchState *ms, const char *s, const char *p);

// LIBRARY: SWBF2 0x0069f160
// matchbalance
static const char *matchbalance (MatchState *ms, const char *s,
                                   const char *p) {
  if (*p == 0 || *(p+1) == 0)
    luaL_error(ms->L, "unbalanced pattern");
  if (*s != *p) return NULL;
  else {
    int b = *p;
    int e = *(p+1);
    int cont = 1;
    while (++s < ms->src_end) {
      if (*s == e) {
        if (--cont == 0) return s+1;
      }
      else if (*s == b) cont++;
    }
  }
  return NULL;
}

// LIBRARY: SWBF2 0x0069f280
// max_expand
static const char *max_expand (MatchState *ms, const char *s,
                                 const char *p, const char *ep) {
  sint32 i = 0;
  while ((s+i)<ms->src_end && luaI_singlematch(uchar(*(s+i)), p, ep))
    i++;

  while (i>=0) {
    const char *res = match(ms, (s+i), ep+1);
    if (res) return res;
    i--;
  }
  return NULL;
}

// FUNCTION: SWBF2 0x0069f2e0
static const char *min_expand (MatchState *ms, const char *s,
                                 const char *p, const char *ep) {
  for (;;) {
    const char *res = match(ms, s, ep+1);
    if (res != NULL)
      return res;
    else if (s<ms->src_end && luaI_singlematch(uchar(*s), p, ep))
      s++;
    else return NULL;
  }
}

// LIBRARY: SWBF2 0x0069f530
// start_capture
static const char *start_capture (MatchState *ms, const char *s,
                                    const char *p, int what) {
  const char *res;
  int level = ms->level;
  if (level >= MAX_CAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  if ((res=match(ms, s, p)) == NULL)
    ms->level--;
  return res;
}

// FUNCTION: SWBF2 0x0069e8b0
static const char *end_capture (MatchState *ms, const char *s,
                                  const char *p) {
  int l = capture_to_close(ms);
  const char *res;
  ms->capture[l].len = s - ms->capture[l].init;
  if ((res = match(ms, s, p)) == NULL)
    ms->capture[l].len = CAP_UNFINISHED;
  return res;
}

// FUNCTION: SWBF2 0x0069efa0
static const char *match_capture (MatchState *ms, const char *s, int l) {
  size_t len;
  l = check_capture(ms, l);
  len = ms->capture[l].len;
  if ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    return s+len;
  else return NULL;
}

// LIBRARY: SWBF2 0x0069ed00
// match
static const char *match (MatchState *ms, const char *s, const char *p) {
  init:
  switch (*p) {
    case '(': {
      if (*(p+1) == ')')
        return start_capture(ms, s, p+2, CAP_POSITION);
      else
        return start_capture(ms, s, p+1, CAP_UNFINISHED);
    }
    case ')': {
      return end_capture(ms, s, p+1);
    }
    case ESC: {
      switch (*(p+1)) {
        case 'b': {
          s = matchbalance(ms, s, p+2);
          if (s == NULL) return NULL;
          p+=4; goto init;
        }
        case 'f': {
          const char *ep; char previous;
          p += 2;
          if (*p != '[')
            luaL_error(ms->L, "missing `[' after `%%f' in pattern");
          ep = luaI_classend(ms, p);
          previous = (s == ms->src_init) ? '\0' : *(s-1);
          if (matchbracketclass(uchar(previous), p, ep-1) ||
             !matchbracketclass(uchar(*s), p, ep-1)) return NULL;
          p=ep; goto init;
        }
        default: {
          if (isdigit(uchar(*(p+1)))) {
            s = match_capture(ms, s, *(p+1));
            if (s == NULL) return NULL;
            p+=2; goto init;
          }
          goto dflt;
        }
      }
    }
    case '\0': {
      return s;
    }
    case '$': {
      if (*(p+1) == '\0')
        return (s == ms->src_end) ? s : NULL;
      else goto dflt;
    }
    default: dflt: {
      const char *ep = luaI_classend(ms, p);
      int m = s<ms->src_end && luaI_singlematch(uchar(*s), p, ep);
      switch (*ep) {
        case '?': {
          const char *res;
          if (m && ((res=match(ms, s+1, ep+1)) != NULL))
            return res;
          p=ep+1; goto init;
        }
        case '*': {
          return max_expand(ms, s, p, ep);
        }
        case '+': {
          return (m ? max_expand(ms, s+1, p, ep) : NULL);
        }
        case '-': {
          return min_expand(ms, s, p, ep);
        }
        default: {
          if (!m) return NULL;
          s++; p=ep; goto init;
        }
      }
    }
  }
}

// FUNCTION: SWBF2 0x0069ea20
static const char *lmemfind (const char *s1, size_t l1,
                               const char *s2, size_t l2) {
  if (l2 == 0) return s1;
  else if (l2 > l1) return NULL;
  else {
    const char *init;
    l2--;
    l1 = l1-l2;
    while (l1 > 0 && (init = (const char *)memchr(s1, *s2, l1)) != NULL) {
      init++;
      if (memcmp(init, s2+1, l2) == 0)
        return init-1;
      else {
        l1 -= init-s1;
        s1 = init;
      }
    }
    return NULL;
  }
}

// LIBRARY: SWBF2 0x0069f3d0
// push_onecapture
static void push_onecapture (MatchState *ms, int i) {
  int l = ms->capture[i].len;
  if (l == CAP_UNFINISHED) luaL_error(ms->L, "unfinished capture");
  if (l == CAP_POSITION)
    lua_pushnumber(ms->L, (lua_Number)(ms->capture[i].init - ms->src_init + 1));
  else
    lua_pushlstring(ms->L, ms->capture[i].init, l);
}

// FUNCTION: SWBF2 0x0069f360
static int push_captures (MatchState *ms, const char *s, const char *e) {
  int i;
  luaL_checkstack(ms->L, ms->level, "too many captures");
  if (ms->level == 0 && s) {
    lua_pushlstring(ms->L, s, e-s);
    return 1;
  }
  else {
    for (i=0; i<ms->level; i++)
      push_onecapture(ms, i);
    return ms->level;
  }
}

// LIBRARY: SWBF2 0x0069dcc0
// str_find
static int str_find (lua_State *L) {
  size_t l1, l2;
  const char *s = luaL_checklstring(L, 1, &l1);
  const char *p = luaL_checklstring(L, 2, &l2);
  sint32 init = posrelat(luaL_optlong(L, 3, 1), l1) - 1;
  if (init < 0) init = 0;
  else if ((size_t)(init) > l1) init = (sint32)l1;
  if (lua_toboolean(L, 4) ||
      strpbrk(p, SPECIALS) == NULL) {

    const char *s2 = lmemfind(s+init, l1-init, p, l2);
    if (s2) {
      lua_pushnumber(L, (lua_Number)(s2-s+1));
      lua_pushnumber(L, (lua_Number)(s2-s+l2));
      return 2;
    }
  }
  else {
    MatchState ms;
    int anchor = (*p == '^') ? (p++, 1) : 0;
    const char *s1=s+init;
    ms.L = L;
    ms.src_init = s;
    ms.src_end = s+l1;
    do {
      const char *res;
      ms.level = 0;
      if ((res=match(&ms, s1, p)) != NULL) {
        lua_pushnumber(L, (lua_Number)(s1-s+1));
        lua_pushnumber(L, (lua_Number)(res-s));
        return push_captures(&ms, NULL, 0) + 2;
      }
    } while (s1++<ms.src_end && !anchor);
  }
  lua_pushnil(L);
  return 1;
}

// FUNCTION: SWBF2 0x0069e8f0
static int gfind_aux (lua_State *L) {
  MatchState ms;
  const char *s = lua_tostring(L, lua_upvalueindex(1));
  size_t ls = lua_strlen(L, lua_upvalueindex(1));
  const char *p = lua_tostring(L, lua_upvalueindex(2));
  const char *src;
  ms.L = L;
  ms.src_init = s;
  ms.src_end = s+ls;
  for (src = s + (size_t)lua_tonumber(L, lua_upvalueindex(3));
       src <= ms.src_end;
       src++) {
    const char *e;
    ms.level = 0;
    if ((e = match(&ms, src, p)) != NULL) {
      int newstart = e-s;
      if (e == src) newstart++;
      lua_pushnumber(L, (lua_Number)newstart);
      lua_replace(L, lua_upvalueindex(3));
      return push_captures(&ms, src, e);
    }
  }
  return 0;
}

// LIBRARY: SWBF2 0x0069df00
// gfind
static int gfind (lua_State *L) {
  luaL_checkstring(L, 1);
  luaL_checkstring(L, 2);
  lua_settop(L, 2);
  lua_pushnumber(L, 0);
  lua_pushcclosure(L, gfind_aux, 3);
  return 1;
}

// FUNCTION: SWBF2 0x0069e720
static void add_s (MatchState *ms, luaL_Buffer *b,
                   const char *s, const char *e) {
  lua_State *L = ms->L;
  if (lua_isstring(L, 3)) {
    const char *news = lua_tostring(L, 3);
    size_t l = lua_strlen(L, 3);
    size_t i;
    for (i=0; i<l; i++) {
      if (news[i] != ESC)
        luaL_putchar(b, news[i]);
      else {
        i++;
        if (!isdigit(uchar(news[i])))
          luaL_putchar(b, news[i]);
        else {
          int level = check_capture(ms, news[i]);
          push_onecapture(ms, level);
          luaL_addvalue(b);
        }
      }
    }
  }
  else {
    int n;
    lua_pushvalue(L, 3);
    n = push_captures(ms, s, e);
    lua_call(L, n, 1);
    if (lua_isstring(L, -1))
      luaL_addvalue(b);
    else
      lua_pop(L, 1);
  }
}

// LIBRARY: SWBF2 0x0069df50
// str_gsub
static int str_gsub (lua_State *L) {
  size_t srcl;
  const char *src = luaL_checklstring(L, 1, &srcl);
  const char *p = luaL_checkstring(L, 2);
  int max_s = luaL_optint(L, 4, srcl+1);
  int anchor = (*p == '^') ? (p++, 1) : 0;
  int n = 0;
  MatchState ms;
  luaL_Buffer b;
  luaL_argcheck(L,
    lua_gettop(L) >= 3 && (lua_isstring(L, 3) || lua_isfunction(L, 3)),
    3, "string or function expected");
  luaL_buffinit(L, &b);
  ms.L = L;
  ms.src_init = src;
  ms.src_end = src+srcl;
  while (n < max_s) {
    const char *e;
    ms.level = 0;
    e = match(&ms, src, p);
    if (e) {
      n++;
      add_s(&ms, &b, src, e);
    }
    if (e && e>src)
      src = e;
    else if (src < ms.src_end)
      luaL_putchar(&b, *src++);
    else break;
    if (anchor) break;
  }
  luaL_addlstring(&b, src, ms.src_end-src);
  luaL_pushresult(&b);
  lua_pushnumber(L, (lua_Number)n);
  return 2;
}

#define MAX_ITEM	512

#define MAX_FORMAT	20

// LIBRARY: SWBF2 0x0069eae0
// luaI_addquoted
static void luaI_addquoted (lua_State *L, luaL_Buffer *b, int arg) {
  size_t l;
  const char *s = luaL_checklstring(L, arg, &l);
  luaL_putchar(b, '"');
  while (l--) {
    switch (*s) {
      case '"': case '\\': case '\n': {
        luaL_putchar(b, '\\');
        luaL_putchar(b, *s);
        break;
      }
      case '\0': {
        luaL_addlstring(b, "\\000", 4);
        break;
      }
      default: {
        luaL_putchar(b, *s);
        break;
      }
    }
    s++;
  }
  luaL_putchar(b, '"');
}

// LIBRARY: SWBF2 0x0069f440
// scanformat
static const char *scanformat (lua_State *L, const char *strfrmt,
                                 char *form, int *hasprecision) {
  const char *p = strfrmt;
  while (strchr("-+ #0", *p)) p++;
  if (isdigit(uchar(*p))) p++;
  if (isdigit(uchar(*p))) p++;
  if (*p == '.') {
    p++;
    *hasprecision = 1;
    if (isdigit(uchar(*p))) p++;
    if (isdigit(uchar(*p))) p++;
  }
  if (isdigit(uchar(*p)))
    luaL_error(L, "invalid format (width or precision too long)");
  if (p-strfrmt+2 > MAX_FORMAT)
    luaL_error(L, "invalid format (too long)");
  form[0] = '%';
  strncpy(form+1, strfrmt, p-strfrmt+1);
  form[p-strfrmt+2] = 0;
  return p;
}

// LIBRARY: SWBF2 0x0069e160
// str_format
static int str_format (lua_State *L) {
  int arg = 1;
  size_t sfl;
  const char *strfrmt = luaL_checklstring(L, arg, &sfl);
  const char *strfrmt_end = strfrmt+sfl;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  while (strfrmt < strfrmt_end) {
    if (*strfrmt != '%')
      luaL_putchar(&b, *strfrmt++);
    else if (*++strfrmt == '%')
      luaL_putchar(&b, *strfrmt++);
    else {
      char form[MAX_FORMAT];
      char buff[MAX_ITEM];
      int hasprecision = 0;
      if (isdigit(uchar(*strfrmt)) && *(strfrmt+1) == '$')
        return luaL_error(L, "obsolete option (d$) to `format'");
      arg++;
      strfrmt = scanformat(L, strfrmt, form, &hasprecision);
      switch (*strfrmt++) {
        case 'c':  case 'd':  case 'i': {
          sprintf(buff, form, luaL_checkint(L, arg));
          break;
        }
        case 'o':  case 'u':  case 'x':  case 'X': {
          sprintf(buff, form, (unsigned int)(luaL_checknumber(L, arg)));
          break;
        }
        case 'e':  case 'E': case 'f':
        case 'g': case 'G': {
          sprintf(buff, form, luaL_checknumber(L, arg));
          break;
        }
        case 'q': {
          luaI_addquoted(L, &b, arg);
          continue;
        }
        case 's': {
          size_t l;
          const char *s = luaL_checklstring(L, arg, &l);
          if (!hasprecision && l >= 100) {

            lua_pushvalue(L, arg);
            luaL_addvalue(&b);
            continue;
          }
          else {
            sprintf(buff, form, s);
            break;
          }
        }
        default: {
          return luaL_error(L, "invalid option to `format'");
        }
      }
      luaL_addlstring(&b, buff, strlen(buff));
    }
  }
  luaL_pushresult(&b);
  return 1;
}

static const luaL_reg strlib[] = {
  {"len", str_len},
  {"sub", str_sub},
  {"lower", str_lower},
  {"upper", str_upper},
  {"char", str_char},
  {"rep", str_rep},
  {"byte", str_byte},
  {"format", str_format},
  {"dump", str_dump},
  {"find", str_find},
  {"gfind", gfind},
  {"gsub", str_gsub},
  {NULL, NULL}
};

// LIBRARY: SWBF2 0x0069ecd0
// luaopen_string
LUALIB_API int luaopen_string (lua_State *L) {
  luaL_openlib(L, LUA_STRLIBNAME, strlib, 0);
  return 1;
}

// LIBRARY: SWBF2 0x0075297a
// strncpy

// LIBRARY: SWBF2 0x00752b22
// isdigit
