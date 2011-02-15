/* Copyright (c) 2011 Akamai Technologies, Inc. */

int luaA_pushsp(lua_State* L, const StringPiece& sp) WARN_RET;
int luaA_getstringvalue(lua_State* L, int idx, const string& key, string** valp) WARN_RET;
void luaPIL_dumpstack(lua_State *L);
