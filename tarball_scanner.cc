/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include <archive.h>
#include <archive_entry.h>

#include "cgen.h"
#include "sensors.h"
#include "log.h"
#include "encode.h"
#include "decode.h"
#include "scanner.h"
#include "tarball_scanner.h"
#include "scannable.h"

TarballScanner::TarballScanner() : archive_(NULL), entry_(NULL) {}

TarballScanner::~TarballScanner() {}

int TarballScanner::Init(Counter* counter, struct archive* archive)
{
	CHK(Scanner::Init(counter) != 0,
		"unable to init scanner", out_error);
	CHK(archive == NULL, "null archive", out_error);
	archive_ = archive;

	LET(entry_ = archive_entry_new(), entry_ == NULL,
		"unable to alloc archive entry", out_error)

	return 0;
out_error:
	return 1;
}

int TarballScanner::Destroy()
{
	archive_entry_free(entry_);

	CHK(Scanner::Destroy() != 0,
		"unable to destroy scanner", out_error);
	return 0;
out_error:
	return 1;
}

int TarballScanner::scan_pre(Scannable* sc)
{
	string qpath;
	uint64_t entry_len;

	CHK(Scanner::scan_pre(sc) != 0,
		"unable to pre-scan tarball entry", out_error);

	CHK(sc->get_quoted_path(&qpath) != 0,
		"unable to get entry path", out_error);
	entry_len = min((uint64_t)2*WINDOW_LEN, sc->data_len());

	archive_entry_clear(entry_);

	archive_entry_set_pathname(entry_, qpath.c_str());
	archive_entry_set_size(entry_, entry_len);
	archive_entry_set_filetype(entry_, AE_IFREG);

	CHK(archive_write_header(archive_, entry_) != ARCHIVE_OK,
		"unable to write archive entry header", out_error);

	return 0;
out_error:
	return 1;
}

int TarballScanner::scan_buf(Scannable*, StringPiece buf, void*)
{
	CHK(archive_write_data(archive_, buf.data(), buf.size()) == -1,
		"unable to write to archive", out_error);
	return 0;
out_error:
	return 1;
}

int TarballScanner::scan_post(Scannable* sc)
{
	CHK(Scanner::scan_post(sc) != 0,
		"unable to post-scan tarball entry", out_error);
	return 0;
out_error:
	return 1;
}
