/* Copyright (c) 2011 Akamai Technologies, Inc. */

class SampleModel : public binder
{
public:
	SampleModel();

	virtual int InitTable(sqlite3* db);
	virtual int InitStatements();

	const StringPiece& sample() const;
	void set_sample(const StringPiece& sample);

private:
	virtual int BindInsert();
	virtual int BindSelect();

	StringPiece sample_;
	int Bind(sqlite3_stmt* stmt);

	DISALLOW_COPY_AND_ASSIGN(SampleModel);
};
