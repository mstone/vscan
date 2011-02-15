/* Copyright (c) 2011 Akamai Technologies, Inc. */

class encode
{
protected:
	const StringPiece& sp;
	explicit encode(const StringPiece& sp);
};

class uri_encode : public encode
{
protected:
	friend ostream& operator <<(ostream&, const uri_encode&);

public:
	explicit uri_encode(const StringPiece& sp);
};

class html_encode : public encode
{
protected:
	friend ostream& operator <<(ostream&, const html_encode&);

public:
	explicit html_encode(const StringPiece& sp);
};

class highlight_encode : public encode
{
protected:
	friend ostream& operator <<(ostream&, const highlight_encode&);

public:
	explicit highlight_encode(const StringPiece& sp);
};
