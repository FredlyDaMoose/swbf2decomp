#ifndef lundump_h
#define lundump_h

#include "lobject.h"
#include "lzio.h"

Proto* luaU_undump (lua_State* L, ZIO* Z, Mbuffer* buff);

int luaU_endianness (void);

void luaU_dump (lua_State* L, const Proto* Main, lua_Chunkwriter w, void* data);

void luaU_print (const Proto* Main);

#define	LUA_SIGNATURE	"\033Lua"
#define	VERSION		0x50
#define	VERSION0	0x50

#define	TEST_NUMBER	((lua_Number)3.14159265358979323846E7f)

#endif
