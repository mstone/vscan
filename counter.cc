/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "counter.h"

Counter::Counter()
	:
	num_dirs(0),
	num_files(0),
	num_interesting(0)
	{}

int Counter::Init()
{
	return 0;
}
