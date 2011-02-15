/* Copyright (c) 2011 Akamai Technologies, Inc. */

class SensorModel : public binder
{
public:
	SensorModel();

	virtual int InitTable(sqlite3* db);
	virtual int InitStatements();

	const StringPiece& name() const;
	void set_name(const StringPiece& name);

	const StringPiece& pat() const;
	void set_pat(const StringPiece& pat);
private:
	virtual int BindInsert();
	virtual int BindSelect();

	StringPiece name_;
	StringPiece pat_;

	DISALLOW_COPY_AND_ASSIGN(SensorModel);
};
