/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "scanner_mode.h"

const char* mode_to_str[] = {
	"search",
	"collect"
};

int parse_scannermode(const StringPiece& sp, ScannerMode* mode)
{
	if (sp == "search")
		*mode = kSearch;
	else if (sp == "collect")
		*mode = kCollect;
	else
		goto err;

	return 0;
err:
	return 1;
}

