/* Copyright (c) 2011 Akamai Technologies, Inc. */

struct archive;

class TarballTraversal : public Traversal
{
public:
	TarballTraversal();
	virtual ~TarballTraversal();

	virtual int Init(StringPiece archive_name,
		struct archive* archive,
		Scanner* scanner) WARN_RET;

	virtual int go(bool* done) WARN_RET;

	int dbg_print_current_path(const char* msg, const char* frag) const;

	int get_quoted_path(void* traversal_datum, string* qpath) const WARN_RET;

private:
	StringPiece archive_name_;
	struct archive* archive_;
	struct archive_entry* entry_;
	Scanner* scanner_;
	char buf_[1024*1024];

	DISALLOW_COPY_AND_ASSIGN(TarballTraversal);
};
