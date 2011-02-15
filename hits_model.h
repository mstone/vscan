/* Copyright (c) 2011 Akamai Technologies, Inc. */


class HitsModel : public binder
{
public:
	HitsModel();

	virtual int InitTable(sqlite3* db);
	virtual int InitStatements();

	int cp() const;
	void set_cp(int cp);

	sqlite3_int64 pathid() const;
	void set_pathid(sqlite3_int64 pathid);

	sqlite3_int64 featid() const;
	void set_featid(sqlite3_int64 featid);

	sqlite3_int64 sampleid() const;
	void set_sampleid(sqlite3_int64 sampleid);

private:
	virtual int BindInsert();
	virtual int BindSelect();

	int cp_;
	sqlite3_int64 pathid_;
	sqlite3_int64 featid_;
	sqlite3_int64 sampleid_;
	int Bind(sqlite3_stmt* stmt);

	DISALLOW_COPY_AND_ASSIGN(HitsModel);
};
