/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "encode.h"
#include "decode.h"
#include "traversal.h"
#include "tarball_traversal.h"
#include "scanner_mode.h"
#include "scanner.h"
#include "scannable.h"
#include "tarball_scannable.h"

#include "compat_lseek64.h"

TarballScannable::TarballScannable()
	:
	bufp_()
	{}

TarballScannable::~TarballScannable() {}

int TarballScannable::Init(
	TarballTraversal* traversal,
	void* traversal_datum,
	StringPiece bufp)
{
	CHK(Scannable::Init(static_cast<Traversal*>(traversal),
		traversal_datum,
		bufp.size()) != 0,
		"unable to init base class", out_error);

	bufp_ = bufp;
	return 0;
out_error:
	return 1;
}

int TarballScannable::Destroy()
{
	destroyed_ = true;
	return 0;
}

int TarballScannable::Scan(Scanner* sc, uint64_t offset, uint64_t len, void* user)
{
	// XXX: bounds-check required?
	CHK(sc->scan_buf(this, bufp_.substr(offset, len), user) != 0,
		"unable to scan tarball entry data", out_error);
	return 0;
out_error:
	return 1;
}

