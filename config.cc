/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "sensors.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "cgen.h"
#include "luaa.h"
#include "scanner_mode.h"
#include "config.h"

static const char* const sdbx = \
"function run(path)\n"
"  local env = {}\n"
"  local untrusted_function, message = loadfile(path)\n"
"  if not untrusted_function then return nil, message end\n"
"  setfenv(untrusted_function, env)\n"
"  untrusted_function()\n"
"  return env\n"
"end\n";

Config::Config()
	: sensors(),
	filenames(NULL),
	last_modified(NULL),
	L(NULL),
	filenames_str_(),
	last_modified_str_()
	{}

int Config::Init(const StringPiece& path)
{
	L = lua_open();
	luaL_openlibs(L);

	CHK(load_config(path) != 0,				// ( -- tbl )
		"unable to load config file", error);

	CHK(load_sensors() != 0,				// ( tbl -- tbl )
		"unable to load config file", error);

	return 0;
error:
	fprintf(stderr, "E lua: %s\n", lua_tostring(L, -1));
	return 1;
}

int Config::load_config(const StringPiece& path)
{
	CHK(luaL_loadstring(L, sdbx) != 0,
		"unable to load the sandbox string", error);

	CHK(lua_pcall(L, 0, 0, 0) != 0,
		"unable to run the sandbox chunk", error);

	lua_getglobal(L, "run");				// ( -- run? )

	CHK(lua_isnoneornil(L, -1) != 0,			// ( run? -- run )
		"unable to get the run() function", error);

	CHK(luaA_pushsp(L, path) != 0,				// ( -- path )
		"unable to push the path", error);

	CHK(lua_pcall(L, 1, 2, 0) != 0,				// ( run path -- _tbl? _msg)
		"unable to pcall run()", error);

	CHK(lua_isnoneornil(L, -2) != 0,			// ( _tbl? _msg -- tbl? _msg )
		"run() failed", error);

	CHK(lua_istable(L, -2) == 0,				// ( tbl _msg -- tbl _msg )
		"run() did not return a table", error);

	lua_pop(L, 1);

	return 0;
error:
	fprintf(stderr, "E lua: %s\n", lua_tostring(L, -1));
	return 1;
}

int Config::load_sensors()
{
	int ret = 1;
	string* strp;

	strp = &filenames_str_;
	CHK(luaA_getstringvalue(L, -1, "filenames", &strp) != 0,
		"filenames was not a string", error);
	if (strp == NULL)
	{
		filenames = NULL;
	}
	else
	{
		// XXX: LEAK
		filenames = new RE2(filenames_str_);
	}


	strp = &last_modified_str_;
	CHK(luaA_getstringvalue(L, -1, "last_modified", &strp) != 0,
		"last_modified was not a string", error);
	if (strp == NULL)
	{
		last_modified = NULL;
	}
	else
	{
		struct tm tm;
		time_t t;
		memset(&tm, 0, sizeof(tm));
		CHK(strptime(last_modified_str_.c_str(), "%Y%m%d", &tm) == NULL,
			"non YYYYmmdd last_modified time", error);
		LET(t = mktime(&tm), t == (time_t)-1,
			"unable to mktime() last_modified time", error);
		// XXX: LEAK
		last_modified = new time_t(t);
	}


	strp = &mode_str_;
	CHK(luaA_getstringvalue(L, -1, "mode", &strp) != 0,
		"mode was not a string", error);
	if (strp == NULL)
		mode_str_ = "search";
	CHK(parse_scannermode(mode_str_, &mode) != 0,
		"einval-unknown-mode-str", error);


	CHK(luaA_pushsp(L, "sensors") != 0,			// ( -- k )
		"unable to push key: \"sensors\"", error);
	lua_gettable(L, -2);					// ( tbl k -- tbl _tbl? )
	CHK(lua_isnil(L, -1) == 1,				// ( _tbl? -- tbl? )
		"nil sensors table", error);
	CHK(lua_istable(L, -2) == 0,				// ( tbl? -- tbl )
		"sensors var does not reference a table", error);

	lua_pushnil(L);						// ( -- nil )

	while (lua_next(L, -2) != 0)				// ( tbl _str -- tbl _str? _str? )
	{
		TST(lua_type(L, -2) != LUA_TSTRING, lua_pop(L, 2), // ( _str? _str? -- str _str? )
			"non-string sensor name", error);
		TST(lua_type(L, -1) != LUA_TSTRING, lua_pop(L, 1), // ( _str? -- str )
			"non-string sensor pattern", error);

		size_t key_size, val_size;
		const char* val_p = lua_tolstring(L, -1, &val_size);
		const char* key_p = lua_tolstring(L, -2, &key_size);

		string key = string(key_p, key_size);
		string val = string(val_p, val_size);

		// XXX: LEAK
		struct sensor* sp = new struct sensor;
		sp->name = key;
		sp->pat = new RE2(val);
		sp->count = 0;
		sensors.push_back(*sp);

		lua_pop(L, 1);					// ( str -- )
	}
	ret = 0;

	goto done;
error:
	fprintf(stderr, "E lua: %s\n", lua_tostring(L, -1));
	ret = 1;
done:
	lua_pop(L, 1);						// ( tbl -- )
	lua_close(L);
	return ret;
}
