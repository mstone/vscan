/* Copyright (c) 2011 Akamai Technologies, Inc. */

class TarballScanner : public Scanner
{
public:
	TarballScanner();
	virtual ~TarballScanner();

	virtual int Init(Counter* counter,
		struct archive* archive) WARN_RET;

	virtual int Destroy() WARN_RET;

	virtual int scan_buf(Scannable* sc,
		StringPiece buf,
		void* user) WARN_RET;

protected:
	virtual int scan_pre(Scannable* sc) WARN_RET;
	virtual int scan_post(Scannable* sc) WARN_RET;

private:
	struct archive* archive_;
	struct archive_entry* entry_;

	DISALLOW_COPY_AND_ASSIGN(TarballScanner);
};
