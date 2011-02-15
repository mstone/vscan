/* Copyright (c) 2011 Akamai Technologies, Inc. */

class Scanner;
class Traversal;
class FDTraversal;

class FDScannable : public Scannable
{
public:
	FDScannable();
	virtual ~FDScannable();

	virtual int Init(Traversal* traversal,
		void* traversal_datum,
		uint64_t data_len,
		int fd) WARN_RET;
	virtual int Destroy() WARN_RET;

	virtual int Scan(Scanner* sc,
		uint64_t offset,
		uint64_t len,
		void* user) WARN_RET;

private:
	int fd_;
	void* data_;

	DISALLOW_COPY_AND_ASSIGN(FDScannable);
};
