/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "sqlite3_compat.h"
#include "binder.h"
#include "path_model.h"


static const char* sql_t_paths = \
	"CREATE TABLE IF NOT EXISTS paths (path TEXT PRIMARY KEY);";
static const char* sql_i_paths = "INSERT INTO paths VALUES (?);";
static const char* sql_s_paths = "SELECT ROWID from paths WHERE path = ?;";

PathModel::PathModel() : path_() {}

int PathModel::InitTable(sqlite3* db)
{
	db_ = db;
	int ret = 0;

	TST(db == NULL, errno = EINVAL,
		"db must not be NULL", err);

	LET(ret = sqlite3_exec(db_, sql_t_paths, NULL, NULL, NULL), ret != SQLITE_OK,
		"unable to create paths table", err, DBERR);

	return 0;
err:
	return 1;
}

const StringPiece& PathModel::path() const
{
	return path_;
}

void PathModel::set_path(const StringPiece& path)
{
	path_ = path;
}

int PathModel::InitStatements()
{
	sqlite3* const & db = db_;
	int ret = 0;

	TST(db == NULL, errno = EINVAL,
		"db must not be NULL", err);

	LET(ret = sqlite3_prepare(db, sql_i_paths, -1, &insert_, NULL), ret != SQLITE_OK,
		"unable to prepare PathModel insert_ ", err, DBERR);
	LET(ret = sqlite3_prepare(db, sql_s_paths, -1, &select_, NULL), ret != SQLITE_OK,
		"unable to prepare PathModel select_", err, DBERR);

	return 0;
err:
	return 1;

}

int PathModel::Bind(sqlite3_stmt* stmt)
{
	TST(path_.data() == NULL, errno = EINVAL,
		"PathModel::path_ must not be NULL", err);
	TST(path_.size() == 0, errno = EINVAL,
		"PathModel::path_ must have positive length", err);
	TST(stmt == NULL, errno = EINVAL,
		"stmt must not be NULL", err);

	CHK(sqlite3_bind_text(stmt, 1, path_.data(), path_.size(), SQLITE_STATIC) != SQLITE_OK,
		"unable to bind stmt[1] to path_", err);

	return 0;
err:
	return 1;
}

int PathModel::BindInsert()
{
	return Bind(insert_);
}

int PathModel::BindSelect()
{
	return Bind(select_);
}
