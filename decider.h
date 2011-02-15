/* Copyright (c) 2011 Akamai Technologies, Inc. */

class Decider
{
public:
	Decider();
	int Init(const RE2* pat_decider,
		const time_t* last_modified) WARN_RET;

	int Decide(const struct dirent& di,
		const struct stat& st,
		bool* should_scan) const WARN_RET;
private:
	const RE2 * pat_decider_;
	const time_t* last_modified_;

	DISALLOW_COPY_AND_ASSIGN(Decider);
};
