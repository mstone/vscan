/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include <archive.h>
#include <archive_entry.h>

#include "cgen.h"
#include "encode.h"
#include "sensors.h"
#include "log.h"
#include "scanner_mode.h"
#include "traversal.h"
#include "tarball_traversal.h"
#include "scanner.h"
#include "regex_scanner.h"
#include "scannable.h"
#include "counter.h"
#include "config.h"

int main(int argc, char** argv)
{
	Config config;
	TarballTraversal traversal;
	RegexScanner scanner;
	Counter counter;

	string pat_all_str;
	RE2* pat_all;

	int ret = 0;
	struct archive* ar = NULL;
	bool done = false;

	CHK(config.Init("./config.lua") != 0,
		"unable to load config", out_error);

	CHK(combine_sensors(config.sensors.begin(),
		config.sensors.end(),
		&pat_all_str) != 0,
		"unable to combine sensors", out_error);

	pat_all = new RE2(pat_all_str);

	CHK(counter.Init() != 0,
		"unable to init counter", out_error);

	for (int argi = 1; argi < argc; argi++)
	{
		/* 1. Prepare to read the archive named in argv[1]. */
		ar = archive_read_new();
		ret = archive_read_support_compression_all(ar);
		ret = archive_read_support_format_all(ar);
		ret = archive_read_open_filename(ar, argv[argi], 10240);
		if (ret != ARCHIVE_OK) goto out_next_tarball;

		CHK(traversal.Init(argv[argi], ar, &scanner) != 0,
			"unable to init tarball traversal", out_error);

		CHK(scanner.Init(&counter, &config.sensors, pat_all) != 0,
			"unable to init tarball scanner", out_error);

		done = false;

		while (!done) {
			CHK(traversal.go(&done) != 0,
				"unable to traverse", out_error);
		}
	out_next_tarball:
		continue;
	}

	ret = 0;
	goto cleanup;
out_error:
	ret = 1;
	goto cleanup;
cleanup:
	if (ar != NULL)
	{
		TST(archive_read_finish(ar) != ARCHIVE_OK, ar = NULL,
			"unable to cleanup archive", out_error);
	}

	scanner.Destroy();

	return ret;
}
