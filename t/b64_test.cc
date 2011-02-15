/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "decode.h"

int main(int, char**)
{
	string line;
	string result;
	ostringstream oss;
	ssize_t amt;
	StringPiece sp;

	while(!cin.eof())
	{
		getline(cin, line);
		LET(amt = b64_decode(line, oss), amt != 0, "decode failure", out_error, "ret: %zd, %s\n", amt, strerror(amt));
		result = oss.str();
		LET(amt = write(1, result.data(), result.size()), amt != (ssize_t)result.size(),
			"unable to write out decoded chars", out_error);
		oss.str("");
	}

	return 0;
out_error:
	return 1;
}
