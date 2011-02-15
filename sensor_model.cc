/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "sqlite3_compat.h"
#include "binder.h"
#include "sensor_model.h"


static const char* const sql_t_feats = \
	"CREATE TABLE IF NOT EXISTS feats (name TEXT PRIMARY KEY, regex TEXT);";
static const char* const sql_i_feats = "INSERT INTO feats VALUES (?,?);";
static const char* const sql_s_feats = "SELECT ROWID from feats WHERE name = ?;";

SensorModel::SensorModel() : name_(), pat_() {}

const StringPiece& SensorModel::name() const
{
	return name_;
}

void SensorModel::set_name(const StringPiece& name)
{
	name_ = name;
}

const StringPiece& SensorModel::pat() const
{
	return pat_;
}

void SensorModel::set_pat(const StringPiece& pat)
{
	pat_ = pat;
}

int SensorModel::InitTable(sqlite3* db)
{
	db_ = db;
	int ret = 0;

	TST(db == NULL, errno = EINVAL,
		"db must not be NULL", err);

	LET(ret = sqlite3_exec(db_, sql_t_feats, NULL, NULL, NULL), ret != SQLITE_OK,
		"unable to create feats table", err, DBERR);

	return 0;
err:
	return 1;
}

int SensorModel::InitStatements()
{
	sqlite3* const & db = db_;
	int ret = 0;

	LET(ret = sqlite3_prepare(db, sql_i_feats, -1, &insert_, NULL), ret != SQLITE_OK,
		"unable to prepare SensorModel insert_", err, DBERR);
	LET(ret = sqlite3_prepare(db, sql_s_feats, -1, &select_, NULL), ret != SQLITE_OK,
		"unable to prepare SensorModel select_", err, DBERR);

	return 0;
err:
	return 1;

}

int SensorModel::BindInsert()
{
	TST(name_.data() == NULL, errno = EINVAL,
		"name must not be NULL", err);
	TST(name_.size() == 0, errno = EINVAL,
		"name must have positive length", err);
	TST(insert_ == NULL, errno = EINVAL,
		"insert_ must not be NULL", err);

	CHK(sqlite3_bind_text(insert_, 1, name_.data(), name_.size(), SQLITE_STATIC) != SQLITE_OK,
		"unable to bind SensorModel::insert_[1] to name", err);
	if (pat_.data() == NULL)
	{
		CHK(sqlite3_bind_null(insert_, 2) != SQLITE_OK,
			"unable to bind SensorModel::insert_[2] to NULL", err);
	}
	else
	{
		CHK(sqlite3_bind_text(insert_, 2, pat_.data(), pat_.size(), SQLITE_STATIC) != SQLITE_OK,
			"unable to bind SensorModel::insert_[2] to pat", err);
	}
	return 0;
err:
	return 1;
}

int SensorModel::BindSelect()
{
	TST(name_.data() == NULL, errno = EINVAL,
		"name must not be NULL", err);
	TST(name_.size() == 0, errno = EINVAL,
		"name must have positive length", err);
	TST(select_ == NULL, errno = EINVAL,
		"select_ must not be NULL", err);

	CHK(sqlite3_bind_text(select_, 1, name_.data(), name_.size(), SQLITE_STATIC) != SQLITE_OK,
		"unable to bind SensorModel::select_[1] to name", err);
	return 0;
err:
	return 1;
}
