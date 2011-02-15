/* Copyright (c) 2011 Akamai Technologies, Inc. */

class Decider;
class Counter;
class Scanner;

class Traversal
{
public:

	Traversal();
	virtual ~Traversal();

	virtual int Init() WARN_RET;

	virtual int go(bool* done) WARN_RET = 0;

	virtual int dbg_print_current_path(const char* msg,
					   const char* frag) const = 0;

	virtual int get_quoted_path(void* traversal_datum,
				    string* qpath) const WARN_RET = 0;

private:
	DISALLOW_COPY_AND_ASSIGN(Traversal);
};
