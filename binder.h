/* Copyright (c) 2011 Akamai Technologies, Inc. */

class binder
{
public:
	virtual ~binder();
	virtual int InitTable(sqlite3* db) = 0;
	virtual int InitStatements() = 0;
	virtual int DestroyStatements();
	virtual int Guarantee(sqlite3_int64* rowid);
	virtual int SelectId(sqlite3_int64* rowid);

protected:
	virtual int BindInsert() = 0;
	virtual int BindSelect() = 0;
	binder();

	sqlite3* db_;
	sqlite3_stmt* select_;
	sqlite3_stmt* insert_;

private:
	DISALLOW_COPY_AND_ASSIGN(binder);
};

