/* Copyright (c) 2011 Akamai Technologies, Inc. */

class Counter;
class StorageTraversal;
class Scannable;

struct archive;
struct archive_entry;

extern const int WINDOW_LEN;

class Scanner
{
public:
	Scanner();
	virtual ~Scanner();

	virtual int Init(Counter* counter) WARN_RET;
	virtual int Destroy() WARN_RET;

	virtual int scan(Scannable* sc) WARN_RET;

	virtual int scan_buf(Scannable* sc,
		StringPiece buf,
		void* user) WARN_RET = 0;

protected:
	virtual int scan_pre(Scannable* sc) WARN_RET;
	virtual int scan_post(Scannable* sc) WARN_RET;

private:
	bool destroyed_;
	Counter* counter_;

	DISALLOW_COPY_AND_ASSIGN(Scanner);
};
