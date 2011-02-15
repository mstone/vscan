/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include <sqlite3.h>

#ifndef sqlite3_int64
#define sqlite3_int64 sqlite_int64
#endif

#ifndef sqlite3_prepare_v2
#define sqlite3_prepare_v2 sqlite3_prepare
#endif

#ifndef sqlite3_clear_bindings
#define sqlite3_clear_bindings(...) 0
#endif
