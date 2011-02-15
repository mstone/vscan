/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "decider.h"


Decider::Decider() : pat_decider_(NULL), last_modified_(NULL) {}

int Decider::Init(const RE2* pat_decider, const time_t* last_modified)
{
	pat_decider_ = pat_decider;
	last_modified_ = last_modified;
	return 0;
}

int Decider::Decide(const struct dirent& di,
	const struct stat& st,
	bool* should_scan) const
{
	bool decision = true;
	if (decision && last_modified_ != NULL)
		decision &= (st.st_mtime > *last_modified_);
	if (decision && pat_decider_ != NULL)
		decision &= RE2::FullMatch(di.d_name, *pat_decider_);
	if (decision)
		decision &= (st.st_size > 0);
	*should_scan = decision;
	return 0;
}
