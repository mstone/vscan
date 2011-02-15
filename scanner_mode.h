/* Copyright (c) 2011 Akamai Technologies, Inc. */

enum ScannerMode
{
	kSearch = 0,
	kCollect,
	kMaxScannerMode,
};

extern const char* mode_to_str[kMaxScannerMode];

int parse_scannermode(const StringPiece& sp, ScannerMode* mode);

