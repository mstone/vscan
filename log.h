/* Copyright (c) 2011 Akamai Technologies, Inc. */

struct sensor;
class Counter;

void write_f_line(ostream* out,
	const StringPiece& qpath,
	const sensor* sens,
	const StringPiece& capture);

void write_p_line(const list<struct sensor>* sensors,
	ostream* out,
	const StringPiece& qpath);

void write_h_line(ostream* out,
	const StringPiece& qpath,
	const StringPiece& text,
	const RE2* pat_all);

void write_c_line(ostream* out,
	const StringPiece& qpath,
	const Counter* counter);
