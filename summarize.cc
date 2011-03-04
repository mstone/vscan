/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"

extern "C" {
#include "nat.h"
}

#include "sqlite3_compat.h"

#include "encode.h"
#include "sensors.h"
#include "scanner_mode.h"
#include "config.h"

#include "binder.h"
#include "sensor_model.h"
#include "path_model.h"
#include "sample_model.h"
#include "hits_model.h"

RE2 pat_f_line("F ([^ ]+) ([^ ]+) ([^ \r\n]+)");

int main(int argc, char** argv)
{
	int ret = 1;
	sqlite3* db;
	sqlite3_stmt *stmt_s_count;

	CHK(sqlite3_open("./summary.db", &db) != SQLITE_OK,
		"unable to open ./summary.db", err_db);

	{ // scope for db models
	SensorModel sens_model;
	PathModel path_model;
	SampleModel sample_model;
	HitsModel hits_model;
	Config config;
	CHK(config.Init("./config.lua") != 0,
		"unable to load sensors", err_db);

	CHK(sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK,
		"unable to BEGIN", err_db);

	CHK(sens_model.InitTable(db) != 0,
		"unable to initialize feats table", err_db);
	CHK(sens_model.InitStatements() != 0,
		"unable to initialize feats statements", err_db);

	CHK(path_model.InitTable(db) != 0,
		"unable to initialize paths table", err_db);
	CHK(path_model.InitStatements() != 0,
		"unable to initialize paths statements", err_db);

	CHK(sample_model.InitTable(db) != 0,
		"unable to initialize samples table", err_db);
	CHK(sample_model.InitStatements() != 0,
		"unable to initialize samples statements", err_db);

	CHK(hits_model.InitTable(db) != 0,
		"unable to initialize hits table", err_db);
	CHK(hits_model.InitStatements() != 0,
		"unable to initialize hits statements", err_db);

	CHK(sqlite3_prepare_v2(db, "SELECT cp, name, COUNT(*) from hits, feats WHERE featid = feats.ROWID GROUP BY cp, name;", -1, &stmt_s_count, NULL) != SQLITE_OK,
		"unable to prepare_v2 stmt_s_count", err_db);


	const_sensors_iterator sens, sens_end;
	for (sens = config.sensors.begin(), sens_end = config.sensors.end();
		sens != sens_end;
		++sens)
	{
		sens_model.set_name(sens->name);
		sens_model.set_pat(sens->pat->pattern());
		CHK(sens_model.Guarantee(NULL) != 0,
			"unable to guarantee feat", err_db, DBERR);
	}

	CHK(sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK,
		"unable to commit after adding features", err_db);

	CHK(sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK,
		"unable to begin a new transaction after adding features", err_db);

	for (int i = 1; i < argc; i++)
	{
		int fd;
		struct stat st;
		void* data;
		off_t data_len;
		StringPiece sp;
		StringPiece path, feature, cpcode, sample;
		unsigned long cp;
		size_t slash_pos;

		LET(fd = open(argv[i], O_RDONLY), fd == -1,
			"unable to open argv[i]", err_file);
		CHK(fstat(fd, &st) == -1,
			"unable to stat fd", err_file);
		data_len = st.st_size;

		LET(data = mmap(NULL, data_len, PROT_READ, MAP_SHARED, fd, 0),
			data == reinterpret_cast<void*>(-1),
			"unable to mmap fd", err_file);

		sp = StringPiece((const char*)data, st.st_size);
		while (RE2::FindAndConsume(&sp, pat_f_line, &path, &feature, &sample))
		{
			LET(slash_pos = path.find('/'), slash_pos == StringPiece::npos,
				"unable to find a / in path", f_line_loop);

			cpcode = path.substr(0, slash_pos);

			CHK(parse_nat(&cp, cpcode.data(), cpcode.size()) != 0,
				"unable to parse cpcode into a nat", f_line_loop);

			sqlite3_int64 pathid;
			path_model.set_path(path);
			CHK(path_model.Guarantee(&pathid) != 0,
				"unable to guarantee path", err_db, DBERR);

			sqlite3_int64 featid;
			sens_model.set_name(feature);
			sens_model.set_pat(NULL);
			CHK(sens_model.Guarantee(&featid) != 0,
				"unable to guarantee feat", err_db, DBERR);

			sqlite3_int64 sampleid;
			sample_model.set_sample(sample);
			CHK(sample_model.Guarantee(&sampleid) != 0,
				"unable to guarantee sample", err_db, DBERR);

			hits_model.set_cp(cp);
			hits_model.set_pathid(pathid);
			hits_model.set_featid(featid);
			hits_model.set_sampleid(sampleid);

			CHK(hits_model.Guarantee(NULL) != 0,
				"unable to guarantee hit", err_db, DBERR);

		f_line_loop:
			continue;
		}

		CHK(munmap(data, data_len) == -1,
			"unable to munmap data", err_file);
	err_file:
		CHK(close(fd) == -1,
			"unable to close fd", file_loop);
	file_loop:
		continue;
	}

	CHK(sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK,
		"unable to commit paths and hits transaction", err_db);

	cout << "Summary: \n";
	while (1)
	{
		LET(ret = sqlite3_step(stmt_s_count), ret != SQLITE_DONE && ret != SQLITE_ROW,
			"unable to step stmt_s_count", err_db);
		if (ret == SQLITE_DONE) break;

		sqlite3_int64 cpcode = sqlite3_column_int64(stmt_s_count, 0);
		const unsigned char* feat = sqlite3_column_text(stmt_s_count, 1);
		sqlite3_int64 count = sqlite3_column_int64(stmt_s_count, 2);

		cout << "  " << cpcode << " " << feat << " " << count << "\n";
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
