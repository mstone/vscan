/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "encode.h"
#include "decode.h"
#include "traversal.h"
#include "scanner_mode.h"
#include "scanner.h"
#include "scannable.h"
#include "fd_scannable.h"

#include "compat_lseek64.h"

FDScannable::FDScannable()
	:
	fd_(-1),
	data_(NULL)
	{}

FDScannable::~FDScannable() {}

int FDScannable::Init(
	Traversal* traversal,
	void* traversal_datum,
	uint64_t data_len,
	int fd)
{
	CHK(Scannable::Init(traversal, traversal_datum, data_len) != 0,
		"unable to init base class", out_error);

	CHK(fd == -1, "invalid fd", out_error);
	fd_ = fd;

	// TEST: comment out the next line to test read() vs. mmap().
	data_ = mmap(NULL, this->data_len(), PROT_READ, MAP_SHARED, fd_, 0);
	CHK(data_ == reinterpret_cast<void*>(-1), "unable to mmap fd", out_no_mmap);

	return 0;

out_no_mmap:
	data_ = NULL;
	return 0;

out_error:
	return 1;
}

int FDScannable::Destroy()
{
	if (data_ != NULL)
	{
		TST(munmap(data_, data_len()) == -1, data_ = NULL,
			"unable to munmap data", out_error);
	}
	destroyed_ = true;

	return 0;
out_error:
	return 1;
}

int FDScannable::Scan(Scanner* sc, uint64_t offset, uint64_t len, void* user)
{
	StringPiece buf;
	char* start, *pagestart;
	long pagesize;

	if (data_ != NULL)
	{
		pagesize = sysconf(_SC_PAGE_SIZE);
		CHK(pagesize < 1,
			"page size too small", out_error)

		start     = reinterpret_cast<char*>(data_) + offset;
		pagestart = reinterpret_cast<char*>(data_) +
				(offset / pagesize) * pagesize;

		buf = StringPiece(start, len);

	}
	else
	{
		CHK(lseek(fd_, offset, SEEK_SET) == -1,
			"unable to lseek", out_error);

		vector<char> v(len+1);
		ssize_t c, idx;
		c = idx = 0;

		while (true) {
			c = read(fd_, &v[idx], len-idx);
			CHK(c < 0, "unable to read", out_error);
			idx += c;
			if (c == 0) break;
			if (idx >= len) break;
		}

		CHK(idx != len, "unable to read len bytes", out_error);

		buf = StringPiece(&v[0], v.size());
	}

	CHK(sc->scan_buf(this, buf, user) != 0,
		"unable to scan mmapped data", out_error);

	return 0;
out_error:
	return 1;
}
