/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "encode.h"
#include "decode.h"
#include "traversal.h"
#include "scanner_mode.h"
#include "scanner.h"
#include "scannable.h"

Scannable::Scannable()
	:
	inited_(false),
	destroyed_(false),
	data_len_(0)
	{}

Scannable::~Scannable()
{
	if (inited_ && !destroyed_)
	{
		PERROR("einval-destructing-live-scannable");
	}
}

int Scannable::Init(Traversal* traversal,
	void* traversal_datum,
	uint64_t data_len)
{
	CHK(traversal == NULL, "invalid traversal", out_error);
	traversal_ = traversal;

	CHK(data_len == 0, "zero-length scannable", out_error);
	data_len_ = data_len;

	traversal_datum_ = traversal_datum;

	inited_ = true;
	return 0;
out_error:
	return 1;
}

int Scannable::get_quoted_path(string* s)
{
	return traversal_->get_quoted_path(traversal_datum_, s);
}

uint64_t Scannable::data_len() const
{
	return data_len_;
}
