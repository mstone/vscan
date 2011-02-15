/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "sensors.h"

#include "cgen.h"

int combine_sensors(const_sensors_iterator sens, const_sensors_iterator end, string* out)
{
	const_sensors_iterator begin = sens;
	ostringstream oss_pat;
	oss_pat << "(";
	for (; sens != end; ++sens)
	{
		if (sens != begin)
			oss_pat << "|";
		oss_pat << "(?:" << sens->pat->pattern() << ")";
	}
	oss_pat << ")";
	*out = oss_pat.str();
	return 0;
}
