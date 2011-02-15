/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include <errno.h>
#include <stdio.h>

#include <re2/stringpiece.h>

using re2::StringPiece;

#include "cgen.h"
#include "sqlite3_compat.h"
#include "binder.h"
#include "sample_model.h"


static const char* sql_t_samples = \
	"CREATE TABLE IF NOT EXISTS samples (sample TEXT PRIMARY KEY);";
static const char* sql_i_samples = "INSERT INTO samples VALUES (?);";
static const char* sql_s_samples = "SELECT ROWID from samples WHERE sample = ?;";

SampleModel::SampleModel() : sample_() {}

int SampleModel::InitTable(sqlite3* db)
{
	db_ = db;
	int ret = 0;

	TST(db == NULL, errno = EINVAL,
		"db must not be NULL", err);

	LET(ret = sqlite3_exec(db_, sql_t_samples, NULL, NULL, NULL), ret != SQLITE_OK,
		"unable to create samples table", err, DBERR);

	return 0;
err:
	return 1;
}

const StringPiece& SampleModel::sample() const
{
	return sample_;
}

void SampleModel::set_sample(const StringPiece& sample)
{
	sample_ = sample;
}

int SampleModel::InitStatements()
{
	sqlite3* const & db = db_;
	int ret = 0;

	TST(db == NULL, errno = EINVAL,
		"db must not be NULL", err);

	LET(ret = sqlite3_prepare(db, sql_i_samples, -1, &insert_, NULL), ret != SQLITE_OK,
		"unable to prepare SampleModel insert_ ", err, DBERR);
	LET(ret = sqlite3_prepare(db, sql_s_samples, -1, &select_, NULL), ret != SQLITE_OK,
		"unable to prepare SampleModel select_", err, DBERR);

	return 0;
err:
	return 1;

}

int SampleModel::Bind(sqlite3_stmt* stmt)
{
	TST(sample_.data() == NULL, errno = EINVAL,
		"SampleModel::sample_ must not be NULL", err);
	TST(sample_.size() == 0, errno = EINVAL,
		"SampleModel::sample_ must have positive length", err);
	TST(stmt == NULL, errno = EINVAL,
		"stmt must not be NULL", err);

	CHK(sqlite3_bind_text(stmt, 1, sample_.data(), sample_.size(), SQLITE_STATIC) != SQLITE_OK,
		"unable to bind stmt[1] to sample_", err);

	return 0;
err:
	return 1;
}

int SampleModel::BindInsert()
{
	return Bind(insert_);
}

int SampleModel::BindSelect()
{
	return Bind(select_);
}
