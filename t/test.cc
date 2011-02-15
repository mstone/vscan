/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "decode.h"

int main(int, char**)
{
	printf("RE2 tests\n");
	RE2 pat("he.*");
	int ret;
	ret = RE2::FullMatch("he", pat);
	printf("match 'he' %d\n", ret);
	ret = RE2::FullMatch("hello", pat);
	printf("match 'hello' %d\n", ret);
	ret = RE2::FullMatch("ick", pat);
	printf("match 'ick' %d\n", ret);

	printf("\n");

	ostringstream oss;
	printf("base64_decode tests\n");

	ret = b64_decode("YQ==", oss);
	printf("YQ== -> %s == a: %d ret: %d\n", oss.str().c_str(), oss.str() == "a", ret);
	oss.str("");

	ret = b64_decode("YWE=", oss);
	printf("YWE= -> %s == aa: %d ret: %d\n", oss.str().c_str(), oss.str() == "aa", ret);
	oss.str("");

	ret = b64_decode("YWFh", oss);
	printf("YWFh -> %s == aaa: %d ret: %d\n", oss.str().c_str(), oss.str() == "aaa", ret);
	oss.str("");

	ret = b64_decode("RrOzuhGI1Lmy2bXB", oss);
	printf("RrOzuhGI1Lmy2bXB -> %s == 'F\xb3\xb3\xba\x11\x88\xd4\xb9\xb2\xd9\xb5\xc1': %d ret %d\n", oss.str().c_str(), oss.str() == "F\xb3\xb3\xba\x11\x88\xd4\xb9\xb2\xd9\xb5\xc1", ret);
	oss.str("");
	return 0;
}
