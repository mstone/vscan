/* Copyright (c) 2011 Akamai Technologies, Inc. */

class Counter
{
public:
	Counter();
	int Init() WARN_RET;

	/* Statistics counters */
	uint64_t num_dirs;
	uint64_t num_files;
	uint64_t num_interesting;
private:
	DISALLOW_COPY_AND_ASSIGN(Counter);
};
