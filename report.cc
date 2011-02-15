/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "sqlite3_compat.h"
#include "binder.h"
#include "sensor_model.h"

const char sql_s_feats[] = \
	"SELECT DISTINCT name, ROWID FROM feats ORDER BY name ASC;";

const char sql_s_cp[] = \
	"SELECT -1 " \
	"UNION ALL " \
	"SELECT DISTINCT cp FROM hits c ORDER BY cp ASC;";

const char sql_s_count[] = \
	"SELECT -1, COUNT(DISTINCT pathid) FROM hits WHERE featid = ?1 " \
	"UNION ALL " \
	"SELECT c.cp, IFNULL(m.path_count, 0) FROM " \
	"  (SELECT DISTINCT cp FROM hits ORDER BY cp ASC) c " \
	"  LEFT OUTER JOIN "\
	"  (SELECT cp, COUNT(DISTINCT pathid) as path_count FROM hits " \
	"   WHERE hits.featid = ?1 GROUP BY cp) m " \
	"ON c.cp = m.cp;";

int main(int, char**)
{
	int ret = 1;
	sqlite3* db;
	sqlite3_stmt *stmt_s_feats, *stmt_s_count, *stmt_s_cp;

	CHK(sqlite3_open("./summary.db", &db) != SQLITE_OK,
		"unable to open ./summary.db", err_db);

	{ // scope for db models
	SensorModel sens_model;

	CHK(sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK,
		"unable to BEGIN", err_db);

	CHK(sens_model.InitTable(db) != 0,
		"unable to initialize feats table", err_db);
	CHK(sens_model.InitStatements() != 0,
		"unable to initialize feats statements", err_db);

	CHK(sqlite3_prepare_v2(db, sql_s_feats, -1, &stmt_s_feats, NULL) != SQLITE_OK,
		"unable to prepare_v2 stmt_s_feats", err_db);
	CHK(sqlite3_prepare_v2(db, sql_s_count, -1, &stmt_s_count, NULL) != SQLITE_OK,
		"unable to prepare_v2 stmt_s_count", err_db);
	CHK(sqlite3_prepare_v2(db, sql_s_cp, -1, &stmt_s_cp, NULL) != SQLITE_OK,
		"unable to prepare_v2 stmt_s_cp", err_db);

	CHK(sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK,
		"unable to commit after adding features", err_db);

	CHK(sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK,
		"unable to begin a new transaction after adding features", err_db);

	//   output a row of cpcode names
	fprintf(stdout, "     [sig]   ");
	while (1)
	{
		LET(ret = sqlite3_step(stmt_s_cp),
			ret != SQLITE_DONE && ret != SQLITE_ROW,
			"unable to step stmt_s_cp", err_db);
		if (ret == SQLITE_DONE) break;

		sqlite3_int64 cpcode = sqlite3_column_int64(stmt_s_cp, 0);

		if (cpcode != -1)
			fprintf(stdout, "  [%6lld]", cpcode);
		else fprintf(stdout, "      total");
	}
	fprintf(stdout, "\n");

	LET(ret = sqlite3_reset(stmt_s_cp),
		ret != SQLITE_OK,
		"real error from stmt_s_cp", err_db, DBERR);

	CHK(sqlite3_clear_bindings(stmt_s_cp) != SQLITE_OK,
		"unable to clear bindings on stmt_s_cp", err_db);

	CHK(sqlite3_finalize(stmt_s_cp) != SQLITE_OK,
		"unable to finalize stmt_s_cp", err_db);

	// for each feat:
	//   count paths for this feat in gross and grouped by cp
	//   skipping feats which have no hits at all
	while (1)
	{
		LET(ret = sqlite3_step(stmt_s_feats),
			ret != SQLITE_DONE && ret != SQLITE_ROW,
			"unable to step stmt_s_feats", err_db);
		if (ret == SQLITE_DONE) break;

		const unsigned char* name = sqlite3_column_text(stmt_s_feats, 0);
		sqlite3_int64 featid = sqlite3_column_int64(stmt_s_feats, 1);

		CHK(sqlite3_bind_int64(stmt_s_count, 1, featid) != SQLITE_OK,
			"unable to bind pathid to stmt[2]", err_db);

		int col = 0;
		while (1)
		{
			LET(ret = sqlite3_step(stmt_s_count),
				ret != SQLITE_DONE && ret != SQLITE_ROW,
				"unable to step stmt_s_count", err_db);
			if (ret == SQLITE_DONE) break;

			//sqlite3_int64 cpcode = sqlite3_column_int64(stmt_s_count, 0);
			sqlite3_int64 count = sqlite3_column_int64(stmt_s_count, 1);

			if (col == 0)
			{
				if (count > 0)
					fprintf(stdout, "%14s", name);
				else
					break; // skip sensors which have no hits
			}

			if (count != 0)
				fprintf(stdout, "  %8lld", count);
			else
				fprintf(stdout, "          ");

			col++;
		}
		if (col > 0)
			fprintf(stdout, "\n");


		LET(ret = sqlite3_reset(stmt_s_count),
			ret != SQLITE_OK,
			"real error from stmt_s_count", err_db, DBERR);
		CHK(sqlite3_clear_bindings(stmt_s_count) != SQLITE_OK,
			"unable to clear bindings on stmt_s_count", err_db);
	}


	CHK(sqlite3_finalize(stmt_s_count) != SQLITE_OK,
		"unable to finalize stmt_s_count", err_db);

	} // end scope for db models

	sqlite3_close(db);
	return 0;
err_db:
	fprintf(stderr, DBERR);
	sqlite3_close(db);
	exit(1);
}
