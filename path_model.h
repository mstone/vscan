/* Copyright (c) 2011 Akamai Technologies, Inc. */

class PathModel : public binder
{
public:
	PathModel();

	virtual int InitTable(sqlite3* db);
	virtual int InitStatements();

	const StringPiece& path() const;
	void set_path(const StringPiece& path);

private:
	virtual int BindInsert();
	virtual int BindSelect();

	StringPiece path_;
	int Bind(sqlite3_stmt* stmt);

	DISALLOW_COPY_AND_ASSIGN(PathModel);
};
