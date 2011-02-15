/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "sqlite3_compat.h"
#include "binder.h"
#include "hits_model.h"

static const char* const sql_t_hits = \
	"CREATE TABLE IF NOT EXISTS hits ("
	"cp INTEGER, pathid INTEGER, featid INTEGER, sampleid INTEGER,"
	"PRIMARY KEY (pathid, featid, sampleid));";
static const char* const sql_i_hits = "INSERT INTO hits VALUES (?,?,?,?);";
static const char* const sql_s_hits = "SELECT ROWID from hits WHERE "
	"cp = ? and pathid = ? and featid = ? and sampleid = ?;";

static const char* const sql_idx_hits_cp = "CREATE INDEX "
	"IF NOT EXISTS idx_hits_cp ON hits (cp);";
static const char* const sql_idx_hits_pathid = "CREATE INDEX "
	"IF NOT EXISTS idx_hits_pathid ON hits (pathid);";
static const char* const sql_idx_hits_featid = "CREATE INDEX "
	"IF NOT EXISTS idx_hits_featid ON hits (featid);";

HitsModel::HitsModel() : cp_(-1), pathid_(-1), featid_(-1), sampleid_(-1) {}

int HitsModel::InitTable(sqlite3* db)
{
	db_ = db;
	int ret = 0;

	TST(db == NULL, errno = EINVAL,
		"db must not be NULL", err);

	LET(ret = sqlite3_exec(db_, sql_t_hits, NULL, NULL, NULL), ret != SQLITE_OK,
		"unable to create hits table", err, DBERR);
	LET(ret = sqlite3_exec(db_, sql_idx_hits_cp, NULL, NULL, NULL), ret != SQLITE_OK,
		"unable to create hits cp index", err, DBERR);
	LET(ret = sqlite3_exec(db_, sql_idx_hits_pathid, NULL, NULL, NULL), ret != SQLITE_OK,
		"unable to create hits pathid index", err, DBERR);
	LET(ret = sqlite3_exec(db_, sql_idx_hits_featid, NULL, NULL, NULL), ret != SQLITE_OK,
		"unable to create hits featid index", err, DBERR);

	return 0;
err:
	return 1;
}

int HitsModel::cp() const
{
	return cp_;
}

void HitsModel::set_cp(int cp)
{
	cp_ = cp;
}

sqlite3_int64 HitsModel::pathid() const
{
	return pathid_;
}

void HitsModel::set_pathid(sqlite3_int64 pathid)
{
	pathid_ = pathid;
}

sqlite3_int64 HitsModel::featid() const
{
	return featid_;
}

void HitsModel::set_featid(sqlite3_int64 featid)
{
	featid_ = featid;
}

sqlite3_int64 HitsModel::sampleid() const
{
	return sampleid_;
}

void HitsModel::set_sampleid(sqlite3_int64 sampleid)
{
	sampleid_ = sampleid;
}

int HitsModel::InitStatements()
{
	sqlite3* const & db = db_;
	int ret = 0;

	TST(db == NULL, errno = EINVAL,
		"db must not be NULL", err);

	LET(ret = sqlite3_prepare_v2(db, sql_i_hits, -1, &insert_, NULL), ret != SQLITE_OK,
		"unable to HitsModel insert_", err, DBERR);
	LET(ret = sqlite3_prepare_v2(db, sql_s_hits, -1, &select_, NULL), ret != SQLITE_OK,
		"unable to HitsModel select_", err, DBERR);

	return 0;
err:
	return 1;
}

int HitsModel::Bind(sqlite3_stmt* stmt)
{
	TST(cp_ == -1, errno = EINVAL,
		"HitsModel::cp_ must not be -1", err);
	TST(pathid_ == -1, errno = EINVAL,
		"HitsModel::pathid_ must not be -1", err);
	TST(featid_ == -1, errno = EINVAL,
		"HitsModel::featid_ must not be -1", err);
	TST(sampleid_ == -1, errno = EINVAL,
		"HitsModel::sampleid_ must not be -1", err);
	TST(stmt == NULL, errno = EINVAL,
		"stmt must not be NULL", err);

	CHK(sqlite3_bind_int(stmt, 1, cp_) != SQLITE_OK,
		"unable to bind cp to stmt[1]", err);
	CHK(sqlite3_bind_int64(stmt, 2, pathid_) != SQLITE_OK,
		"unable to bind pathid to stmt[2]", err);
	CHK(sqlite3_bind_int64(stmt, 3, featid_) != SQLITE_OK,
		"unable to bind featid to stmt[3]", err);
	CHK(sqlite3_bind_int64(stmt, 4, sampleid_) != SQLITE_OK,
		"unable to bind sampleid to stmt[4]", err);

	return 0;
err:
	return 1;
}

int HitsModel::BindInsert()
{
	return Bind(insert_);
}

int HitsModel::BindSelect()
{
	return Bind(select_);
}
