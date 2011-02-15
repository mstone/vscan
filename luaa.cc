/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "cgen.h"
#include "luaa.h"

static int pushsp_trampoline(lua_State* L)
{
	const StringPiece* s;
	CHK(lua_isnone(L, -1) == 1,
		"no element on the top of the lua stack", err);

	CHK(lua_islightuserdata(L, -1) == 0,
		"top of the lua stack is not a lightuserdata", err);

	s = (const StringPiece*) lua_touserdata(L, -1);

	// may longjmp() or throw on allocation failure, hence our trampoline
	lua_pushlstring(L, s->data(), s->size());

	return 1;
err:
	return 0;
}

int luaA_pushsp(lua_State* L, const StringPiece& s)
{
	/* XXX: we should check that we have two stack slots free but, so far
	 * as I know, there's no safe way to do so. */

	lua_pushcfunction(L, &pushsp_trampoline);
	void* vs = reinterpret_cast<void*>(const_cast<StringPiece*>(&s));
	lua_pushlightuserdata(L, vs);
	return lua_pcall(L, 1, 1, 0);
}

int luaA_getstringvalue(lua_State* L, int idx, const string& key, string** valp)
{
	int ret = 1;

	CHK(lua_isnoneornil(L, idx) != 0,			// ( _tbl? ... -- tbl? ... )
		"value at idx is none or nil", out);
	CHK(lua_istable(L, idx) == 0,				// ( tbl? ... -- tbl ... )
		"value at idx is not a table", out);

	lua_pushvalue(L, idx);					// ( -- tbl )

	CHK(luaA_pushsp(L, key) != 0,				// ( -- k )
		"unable to push key", pop_table);

	lua_gettable(L, -2);					// ( tbl k -- tbl _str? )
	if (lua_isnil(L, -1))					// ( _str? -- nil )
	{
		*valp = NULL;
	}
	else							// ( _str? -- str? )
	{
		CHK(lua_type(L, -1) != LUA_TSTRING,		// ( str? -- str )
			"non-string filenames pattern", pop_key);
		size_t size;
		const char* val = lua_tolstring(L, -1, &size);
		string tmp(val, size);
		(*valp)->swap(tmp);
	}
	ret = 0;
pop_key:
	lua_pop(L, 1);
pop_table:
	lua_pop(L, 1);
out:
	return ret;
}

/* From PIL */
void luaPIL_dumpstack(lua_State *L) {
	int i;
	int top = lua_gettop(L);
	for (i = 1; i <= top; i++) {
		int t = lua_type(L, i);
		switch (t) {
			case LUA_TSTRING:
				printf("`%s'", lua_tostring(L, i));
				break;
			case LUA_TBOOLEAN:
				printf(lua_toboolean(L, i) ? "true" : "false");
				break;
			case LUA_TNUMBER:
				printf("%g", lua_tonumber(L, i));
				break;
			default:
				printf("%s", lua_typename(L, t));
				break;
		}
		printf("  ");
	}
	printf("\n");
}
