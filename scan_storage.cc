/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include <archive.h>

#include "cgen.h"
#include "encode.h"
#include "decode.h"
#include "decider.h"
#include "path_dir_pair.h"
#include "traversal.h"
#include "fd_traversal.h"
#include "storage_traversal.h"
#include "sensors.h"
#include "counter.h"
#include "scanner_mode.h"
#include "scanner.h"
#include "regex_scanner.h"
#include "tarball_scanner.h"
#include "config.h"

int main(int argc, char** argv)
{
	Config config;
	StorageTraversal traversal;
	Decider decider;
	RegexScanner re_scanner;
	TarballScanner tb_scanner;
	Scanner* scanner;
	Counter counter;

	string pat_all_str;
	RE2* pat_all;

	DIR* dir;
	bool done = false;
	struct path_dir_pair root;

	const RE2 pat_cpcode("^.*/([1-9][0-9]*)/?$");
	StringPiece cpcode;

	struct archive* archive = NULL;
	string archive_name("./examples.tar.gz");

	CHK(config.Init("./config.lua") != 0,
		"unable to load config", out_error);

	CHK(combine_sensors(config.sensors.begin(),
		config.sensors.end(),
		&pat_all_str) != 0,
		"unable to combine sensors", out_error);

	pat_all = new RE2(pat_all_str);

	CHK(decider.Init(config.filenames, config.last_modified) != 0,
		"unable to initialize decider", out_error);

	CHK(counter.Init() != 0,
		"unable to initialize counter", out_error);

	switch (config.mode)
	{
	case kCollect:
		LET(archive = archive_write_new(), archive == NULL,
			"unable to alloc archive", out_error);
		CHK(archive_write_set_compression_gzip(archive) != ARCHIVE_OK,
			"unable to turn on gzip compression", out_error);
		CHK(archive_write_set_format_pax_restricted(archive) != ARCHIVE_OK,
			"unable to turn on restricted pax format", out_error);
		CHK(archive_write_open_filename(archive, archive_name.c_str()) != ARCHIVE_OK,
			"unable to open archive for writing", out_error);
		break;
	}

	for (int argi = 1; argi < argc; argi++)
	{
		LET(dir = opendir(argv[argi]), dir == NULL,
			"unable to open argv[argi]", out_error,
			"arg: %s\n", argv[argi]);
		if (RE2::FullMatch(argv[argi], pat_cpcode, &cpcode))
			root.path = cpcode.as_string();
		else
			root.path = string(argv[argi]);
		root.dir = dir;

		switch (config.mode)
		{
		case kSearch:
			CHK(re_scanner.Init(&counter, &config.sensors, pat_all) != 0,
				"unable to initialize regex scanner", out_error);
			scanner = &re_scanner;
			break;
		case kCollect:
			CHK(tb_scanner.Init(&counter, archive) != 0,
				"unable to initialize tarball scanner", out_error);
			scanner = &tb_scanner;
			break;
		}

		CHK(traversal.Init(&counter, root, &decider, scanner) != 0,
			"unable to init traversal", out_error);

		done = false;

		while (!done) {
			CHK(traversal.go(&done) != 0,
				"unable to traverse", out_error);
		}
	}

	switch (config.mode)
	{
	case kCollect:
		CHK(archive_write_close(archive) != ARCHIVE_OK,
			"unable to close archive", out_error);
		CHK(archive_write_finish(archive) != ARCHIVE_OK,
			"unable to free archive", out_error);
	}

	re_scanner.Destroy();
	tb_scanner.Destroy();

	return 0;
out_error:
	return 1;
}
