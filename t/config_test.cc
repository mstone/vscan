/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "sensors.h"
#include "pathtype.h"
#include "scanner_mode.h"
#include "config.h"

int main(int, char**)
{
	Config config;
	CHK(config.Init("./config.lua") != 0,
		"unable to init the config", out_error);

	if (config.filenames != NULL)
		printf("filenames: %s\n", config.filenames->pattern().c_str());
	else
		printf("filenames: nil\n");

	if (config.last_modified != NULL)
		printf("last_modified: %ld\n", *config.last_modified);
	else
		printf("last_modified: nil\n");

	for (const_sensors_iterator it = config.sensors.begin(), ie = config.sensors.end();
		it != ie;
		++it)
		printf("sensor: %s : %s\n", it->name.c_str(), it->pat->pattern().c_str());
	return 0;
out_error:
	return 1;
}
