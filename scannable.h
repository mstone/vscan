/* Copyright (c) 2011 Akamai Technologies, Inc. */

class Scanner;
class Traversal;

class Scannable
{
public:
	Scannable();
	virtual ~Scannable();

	virtual int Init(Traversal* traversal,
		void* traversal_datum,
		uint64_t data_len) WARN_RET;
	virtual int Destroy() WARN_RET = 0;

	virtual int Scan(Scanner* sc,
		uint64_t offset,
		uint64_t len,
		void* user) WARN_RET = 0;

	uint64_t data_len() const;

	int get_quoted_path(string* s) WARN_RET;

protected:
	bool inited_, destroyed_;
	Traversal* traversal_;
	void* traversal_datum_;

private:
	uint64_t data_len_;

	DISALLOW_COPY_AND_ASSIGN(Scannable);
};
