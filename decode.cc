/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "decode.h"

static char decode_hex_char(char c)
{
	if (c <= 'f' && c >= 'a')
		return c - 'a' + 10;
	else if (c <= 'F' && c >= 'A')
		return c - 'A' + 10;
	else
		return c - '0';
}


int uri_decode(const StringPiece& sp, ostream& os)
{
	for (StringPiece::const_iterator it = sp.begin(); it != sp.end(); ++it)
	{
		if (!os.good())
			return EIO;

		char c = *it;

		if (c != '%')
			os << c;
		else
		{
			++it;
			if (it == sp.end())
				return EINVAL;

			char c1 = *it;
			if (!isxdigit(c1))
				return EINVAL;

			++it;
			if (it == sp.end())
				return EINVAL;

			char c2 = *it;
			if (!isxdigit(c2))
				return EINVAL;

			char d1 = decode_hex_char(c1);
			char d2 = decode_hex_char(c2);
			char d3 = d1 << 4;
			char d4 = d3 | d2;

			os << d4;
		}
	}

	if (os.bad())
		return EIO;

	return 0;
}

static char isb64digit(char c)
{
	if (c <= 'Z' && c >= 'A')
		return 1;
	else if (c <= 'z' && c >= 'a')
		return 1;
	else if (c <= '9' && c >= '0')
		return 1;
	else if (c == '+')
		return 1;
	else if (c == '-')
		return 1;
	return 0;
}

static char unsafe_decode_b64_char(char c)
{
	if (c <= 'Z' && c >= 'A')
		return c - 'A';
	else if (c <= 'z' && c >= 'a')
		return c - 'a' + 26;
	else if (c <= '9' && c >= '0')
		return c - '0' + 52;
	else if (c == '+')
		return 62;
	else if (c == '-')
		return 63;
	return 0;
}

#define READCHAR(v) \
	char c##v = *it++; \
	if (!isb64digit(c##v)) { \
		if (len == 1 && (v == 2 || v == 3) && c##v == '=') {} \
		else if (len == 2 && (v == 3) && c##v == '=') {} \
		else return EINVAL; \
	} \
	char d##v = unsafe_decode_b64_char(c##v);

#define TESTITER(v) \
	if (it == sp.end()) {\
		return EINVAL;\
	}

#define MAYBE_OUTPUT(v) \
	os << v; \
	if (len == 1) { \
		break;\
	} \
	--len;

int b64_decode(const StringPiece& sp, ostream& os)
{
	size_t len = sp.size();
	size_t pad_len = 0;

	if (len == 0)
		return 0;

	if (len % 4 != 0)
		return EINVAL;

	if (sp[len-1] == '=' && sp[len-2] == '=')
		pad_len = 2;
	else if (sp[len-1] == '=')
		pad_len = 1;

	len /= 4;
	len *= 3;
	len -= pad_len;

	for (StringPiece::const_iterator it = sp.begin(); it != sp.end();)
	{
		if (!os.good())
			return EIO;

		READCHAR(0); TESTITER(0);
		READCHAR(1); TESTITER(1);
		READCHAR(2); TESTITER(2);
		READCHAR(3);

		/*
		    |......|.. ....|.... ..|......|
		    |...... ..|.... ....|.. ......|
		*/

		char m0 = (d0 & 0x3F) << 2 | ((d1 & 0x30) >> 4);
		char m1 = (d1 & 0x0F) << 4 | ((d2 & 0x3C) >> 2);
		char m2 = (d2 & 0x03) << 6 | ((d3 & 0x3F) >> 0);

		MAYBE_OUTPUT(m0);
		MAYBE_OUTPUT(m1);
		MAYBE_OUTPUT(m2);
	}

	if (os.bad())
		return EIO;

	return 0;
}

#undef READCHAR
#undef TESTITER
#undef MAYBE_OUTPUT
