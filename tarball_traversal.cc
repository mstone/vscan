/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include <archive.h>
#include <archive_entry.h>

#include "cgen.h"
#include "log.h"
#include "encode.h"
#include "decode.h"
#include "decider.h"
#include "scanner_mode.h"
#include "config.h"
#include "counter.h"
#include "traversal.h"
#include "tarball_traversal.h"
#include "scanner.h"
#include "scannable.h"
#include "tarball_scannable.h"



TarballTraversal::TarballTraversal()
	:
	archive_name_(),
	archive_(NULL),
	entry_(NULL)
	{}

TarballTraversal::~TarballTraversal() {}

int TarballTraversal::Init(StringPiece archive_name,
	struct archive* archive,
	Scanner* scanner)
{
	CHK(archive_name.empty(), "empty archive name", out_error)
	archive_name_ = archive_name;

	CHK(archive == NULL, "null archive", out_error);
	archive_ = archive;

	CHK(scanner == NULL, "null scanner", out_error);
	scanner_ = scanner;

	return 0;
out_error:
	return 1;
}

int TarballTraversal::go(bool* done)
{
	int ret;
	ssize_t len;
	StringPiece bufp;
	TarballScannable sc_tb;

	ret = archive_read_next_header(archive_, &entry_);
	if (ret == ARCHIVE_EOF) goto out_done;
	TST( ret != ARCHIVE_OK, errno = EINVAL,
		"unable to read next archive header", out_error);

	len = archive_read_data(archive_, buf_, sizeof(buf_));
	TST( len == 0, errno = EINVAL, "zero-length tarball entry", out_skip);
	TST( len < 0, errno = EINVAL, "negative-length tarball entry", out_error);
	TST( len >= (ssize_t)sizeof(buf_), errno = EINVAL,
		"tarball read overflow", out_error);

	buf_[len] = 0;
	bufp = StringPiece (buf_, len);

	CHK(sc_tb.Init(this, NULL, bufp) != 0,
		"unable to init tarball scannable", out_error);

	CHK(scanner_->scan(&sc_tb) != 0,
		"unable to scan tarball entry", out_error);

	CHK(sc_tb.Destroy() != 0,
		"unable to destroy tarball scannable", out_error);

out_skip:
	*done = false;
	return 0;
out_done:
	*done = true;
	return 0;
out_error:
	*done = true;
	if (ret != ARCHIVE_EOF)
		printf("E %s %d %d %s\n",
			archive_name_.as_string().c_str(),
			ret,
			archive_errno(archive_),
			archive_error_string(archive_));
	return 1;
}

int TarballTraversal::dbg_print_current_path(const char* msg, const char* frag) const
{
	fprintf(stderr, "E %s path %s\n", msg, frag);
	return 0;
}

int TarballTraversal::get_quoted_path(void*, string* qpath) const
{
	*qpath = string(archive_entry_pathname(entry_));
	return 0;
}
