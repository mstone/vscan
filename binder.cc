/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "sqlite3_compat.h"
#include "binder.h"

binder::binder() : db_(NULL), select_(NULL), insert_(NULL) {}

binder::~binder()
{
	CHK(DestroyStatements() != 0,
		"unable to destroy paths prepared statements", err);
err:
	{}
}

int binder::DestroyStatements()
{
	sqlite3* const & db = db_;
	int ret = 0;

	LET(ret = sqlite3_finalize(insert_), ret != SQLITE_OK,
		"unable to finalize binder.insert_", err, DBERR);
	insert_ = NULL;

	LET(ret = sqlite3_finalize(select_), ret != SQLITE_OK,
		"unable to finalize binder.select_", err, DBERR);
	select_ = NULL;

	return 0;
err:
	return 1;
}

int binder::Guarantee(sqlite3_int64* rowid)
{
	sqlite3* const & db = db_;
	int ret;

schema:
	CHK(BindInsert() != 0,
		"unable to bind insert_", err, DBERR);

	LET(ret = sqlite3_step(insert_), ret != SQLITE_DONE && ret != SQLITE_ERROR,
		"unable to insert", err, DBERR);

	if (ret == SQLITE_DONE)
	{
		if (rowid != NULL)
			*rowid = sqlite3_last_insert_rowid(db_);
		ret = 0;
	}
	else if (ret == SQLITE_ERROR)
	{
		LET(ret = sqlite3_reset(insert_),
			ret != SQLITE_CONSTRAINT && ret != SQLITE_SCHEMA,
			"real error from insert_", err, DBERR);

		if (ret == SQLITE_SCHEMA)
		{
			CHK(DestroyStatements() != 0,
				"unable to destroy statements prior to recreating them", err, DBERR);
			CHK(InitStatements() != 0,
				"unable to init statements prior to re-running Bind()", err, DBERR);
			goto schema;
		}
		else
		{
			ret = SelectId(rowid);
		}
	}

	CHK(sqlite3_reset(insert_) != SQLITE_OK,
		"unable to reset insert_", err);

	CHK(sqlite3_clear_bindings(insert_) != SQLITE_OK,
		"unable to clear bindings on insert_", err);

	return ret;
err:
	return 1;
}

int binder::SelectId(sqlite3_int64* rowid)
{
	sqlite3* const & db = db_;
	int ret;

	CHK(BindSelect() != 0,
		"unable to bind select_", err, DBERR);

	LET(ret = sqlite3_step(select_), ret != SQLITE_ROW,
		"unable to step select_", err, DBERR);

	if (rowid != NULL)
		*rowid = sqlite3_column_int64(select_, 0);

	LET(ret = sqlite3_reset(select_), ret != SQLITE_OK,
		"real error from select_", err, DBERR);

	CHK(sqlite3_clear_bindings(select_) != SQLITE_OK,
		"unable to clear bindings on select_", err, DBERR);

	return 0;
err:
	return 1;
}
