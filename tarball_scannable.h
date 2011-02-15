/* Copyright (c) 2011 Akamai Technologies, Inc. */

class Scanner;
class Traversal;
class TarballTraversal;

class TarballScannable : public Scannable
{
public:
	TarballScannable();
	virtual ~TarballScannable();

	virtual int Init(TarballTraversal* traversal,
		void* traversal_datum,
		StringPiece bufp) WARN_RET;
	virtual int Destroy() WARN_RET;

	virtual int Scan(Scanner* sc,
		uint64_t offset,
		uint64_t len,
		void* user) WARN_RET;

private:
	StringPiece bufp_;

	DISALLOW_COPY_AND_ASSIGN(TarballScannable);
};
