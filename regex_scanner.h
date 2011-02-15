/* Copyright (c) 2011 Akamai Technologies, Inc. */

class RegexScanner : public Scanner
{
public:
	RegexScanner();
	virtual ~RegexScanner();

	virtual int Init(Counter* counter,
		list<struct sensor>* sensors,
		const RE2* pat_all) WARN_RET;

	/* Scans buf using the combined pattern in pat_all_. Logs hits. */
	virtual int scan_buf(Scannable* sc,
		StringPiece buf,
		void* user) WARN_RET;

private:
	list<struct sensor>* sensors_;

	/* pat_all_ lets us find hits within a buffer. */
	RE2 const * pat_all_;

	DISALLOW_COPY_AND_ASSIGN(RegexScanner);
};
